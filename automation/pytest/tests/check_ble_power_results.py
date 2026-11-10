#!/usr/bin/env python3
"""
Check BLE advertisement power results written by BLEPowerHelper (test_power.py).

Reads either output file:
    ble_power_results.txt  lines like "mode: STOP, Advertisement energy: 23.08uJ Base consumption: 62.54uW"
    ble_power_results.csv  columns: Off profile, Measurement time, Advertisement energy (J), Base power (W)

Check results against DEFAULT_LIMITS (exit code 0 = all PASS, 1 = any FAIL):
    python check_ble_power_results.py ble_power_results.txt

Check against a limits JSON file instead:
    python check_ble_power_results.py ble_power_results.txt --limits ble_power_limits.json

Create a limits JSON from a known-good results file (min/max widened by --tolerance percent):
    python check_ble_power_results.py ble_power_results.txt --make-limits ble_power_limits.json --tolerance 20
"""

import argparse
import csv
import json
import re
import sys

PROFILE_COL = "Off profile"
CHECKED_COLUMNS = {
    "Advertisement energy (J)": "J",
    "Base power (W)": "W",
}


DEFAULT_LIMITS = {
    "STOP": {
        "Advertisement energy (J)": {"min": 12e-6, "max": 40e-6},
        "Base power (W)": {"min": 30e-6, "max": 90e-6},
    },
    "STANDBY": {
        "Advertisement energy (J)": {"min": 25e-6, "max": 70e-6},
        "Base power (W)": {"min": 300e-6, "max": 700e-6},
    },
    "IDLE": {
        "Advertisement energy (J)": {"min": 12e-6, "max": 40e-6},
        "Base power (W)": {"min": 1.5e-3, "max": 2.5e-3},
    },
}

SI_PREFIXES = {"f": 1e-15, "p": 1e-12, "n": 1e-9, "u": 1e-6, "m": 1e-3, "": 1.0,
               "k": 1e3, "M": 1e6, "G": 1e9, "T": 1e12, "P": 1e15}

# e.g. "mode: STOP, Advertisement energy: 23.08uJ Base consumption: 62.54uW"
TXT_LINE = re.compile(r"mode:\s*(?P<mode>\w+),\s*Advertisement energy:\s*(?P<energy>-?[\d.]+)(?P<energy_prefix>[a-zA-Z]?)J"
                      r"\s*Base consumption:\s*(?P<power>-?[\d.]+)(?P<power_prefix>[a-zA-Z]?)W")


def si_string(value):
    for prefix, scale in sorted(SI_PREFIXES.items(), key=lambda p: -p[1]):
        if abs(value) >= scale:
            return f"{value / scale:.2f}{prefix}"
    return f"{value / SI_PREFIXES['f']:.2f}f"


def read_txt_results(f):
    rows = []
    for line in f:
        m = TXT_LINE.search(line)
        if m:
            rows.append({
                PROFILE_COL: m["mode"],
                "Advertisement energy (J)": float(m["energy"]) * SI_PREFIXES[m["energy_prefix"]],
                "Base power (W)": float(m["power"]) * SI_PREFIXES[m["power_prefix"]],
            })
    return rows


def read_results(path):
    with open(path, newline="") as f:
        if path.endswith(".txt"):
            rows = read_txt_results(f)
        else:
            rows = list(csv.DictReader(f))
    if not rows:
        sys.exit(f"No results found in {path}")
    return rows


def make_limits(rows, tolerance_pct):
    """Per off profile: min/max of observed values, widened by tolerance_pct."""
    tol = tolerance_pct / 100.0
    observed = {}
    for row in rows:
        profile = observed.setdefault(row[PROFILE_COL], {})
        for col in CHECKED_COLUMNS:
            profile.setdefault(col, []).append(float(row[col]))

    limits = {}
    for profile, columns in observed.items():
        limits[profile] = {}
        for col, values in columns.items():
            lo, hi = min(values), max(values)
            limits[profile][col] = {"min": lo - abs(lo) * tol, "max": hi + abs(hi) * tol}
    return limits


def check_results(rows, limits):
    all_pass = True
    for row in rows:
        profile = row[PROFILE_COL]
        if profile not in limits:
            print(f"FAIL  {profile}: no limits defined for this off profile")
            all_pass = False
            continue

        for col, unit in CHECKED_COLUMNS.items():
            value = float(row[col])
            lo = limits[profile][col]["min"]
            hi = limits[profile][col]["max"]
            ok = lo <= value <= hi
            all_pass &= ok
            print(f"{'PASS' if ok else 'FAIL'}  {profile:8s} {col:26s} "
                  f"{si_string(value)}{unit}  (range {si_string(lo)}{unit} .. {si_string(hi)}{unit})")

    print("\nRESULT:", "PASS" if all_pass else "FAIL")
    return all_pass


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("results", nargs="?", default="ble_power_results.txt", help="results .txt or .csv from test_power.py")
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument("--make-limits", metavar="JSON", help="write min/max limits computed from the results")
    mode.add_argument("--limits", metavar="JSON", help="check against this limits file instead of DEFAULT_LIMITS")
    parser.add_argument("--tolerance", type=float, default=20.0, help="percent to widen min/max (default 20)")
    args = parser.parse_args()

    rows = read_results(args.results)

    if args.make_limits:
        limits = make_limits(rows, args.tolerance)
        with open(args.make_limits, "w") as f:
            json.dump(limits, f, indent=4)
        print(f"Wrote limits for {', '.join(limits)} to {args.make_limits}")
        return 0

    limits = DEFAULT_LIMITS
    if args.limits:
        with open(args.limits) as f:
            limits = json.load(f)
    return 0 if check_results(rows, limits) else 1


if __name__ == "__main__":
    sys.exit(main())

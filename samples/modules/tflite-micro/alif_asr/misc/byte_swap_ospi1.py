#!/usr/bin/env python3
# Copyright (C) Alif Semiconductor - All Rights Reserved.
# Use, distribution and modification of this code is permitted under the
# terms stated in the Alif Semiconductor Software License Agreement
#
# You should have received a copy of the Alif Semiconductor Software
# License Agreement with this file. If not, please write to:
# contact@alifsemi.com, or visit: https://alifsemi.com/license
#

"""Swap byte order for 16-bit words in a binary file.

Typical use case:
- Convert zephyr/ospi1.bin when target flash reader accesses data as 16-bit words.
- Each word AB becomes BA.
"""

from __future__ import annotations

import argparse
from pathlib import Path


def swap_16bit_words(data: bytes) -> bytes:
    """Return data with bytes swapped inside each 16-bit word (AB -> BA)."""
    if len(data) % 2 != 0:
        raise ValueError("Input size must be even for 16-bit byte swapping")

    swapped = bytearray(len(data))
    swapped[0::2] = data[1::2]
    swapped[1::2] = data[0::2]
    return bytes(swapped)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Swap bytes inside each 16-bit word of a binary file"
    )
    parser.add_argument(
        "input",
        type=Path,
        help="Input binary file (e.g. ~/alif/ad-2214_2/build/zephyr/ospi1.bin)",
    )

    out_group = parser.add_mutually_exclusive_group()
    out_group.add_argument(
        "-o",
        "--output",
        type=Path,
        help="Output file path (default: swab16.bin)",
    )
    out_group.add_argument(
        "--in-place",
        action="store_true",
        help="Overwrite input file in place",
    )

    return parser.parse_args()


def main() -> int:
    args = parse_args()
    in_path = args.input.expanduser().resolve()

    if not in_path.is_file():
        raise FileNotFoundError(f"Input file not found: {in_path}")

    if args.in_place:
        out_path = in_path
    elif args.output is not None:
        out_path = args.output.expanduser().resolve()
    else:
        out_path = in_path.with_suffix(in_path.suffix + ".swab16.bin")

    data = in_path.read_bytes()
    swapped = swap_16bit_words(data)

    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_bytes(swapped)

    print(f"Input : {in_path}")
    print(f"Output: {out_path}")
    print(f"Size  : {len(swapped)} bytes")
    print("Mode  : 16-bit byte swap (AB -> BA)")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())

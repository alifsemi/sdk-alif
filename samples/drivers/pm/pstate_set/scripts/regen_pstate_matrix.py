#!/usr/bin/env python3
# SPDX-FileCopyrightText: Copyright Analog Devices, Inc.
# SPDX-License-Identifier: Apache-2.0
"""Regenerate snippets/pstate-*-full/pstate_matrix_*.dtsi."""

from pathlib import Path

SNIPPETS = Path(__file__).resolve().parents[1] / "snippets"

RC_SCALED = [
    ("ALIF_SCALED_FREQ_RC_ACTIVE_76_8_MHZ", "r0"),
    ("ALIF_SCALED_FREQ_RC_ACTIVE_38_4_MHZ", "r1"),
    ("ALIF_SCALED_FREQ_RC_ACTIVE_19_2_MHZ", "r2"),
    ("ALIF_SCALED_FREQ_RC_ACTIVE_9_6_MHZ", "r3"),
    ("ALIF_SCALED_FREQ_RC_ACTIVE_4_8_MHZ", "r4"),
    ("ALIF_SCALED_FREQ_RC_ACTIVE_2_4_MHZ", "r5"),
    ("ALIF_SCALED_FREQ_RC_ACTIVE_1_2_MHZ", "r6"),
    ("ALIF_SCALED_FREQ_RC_ACTIVE_0_6_MHZ", "r7"),
]

XO_SCALED = [
    ("ALIF_SCALED_FREQ_XO_LOW_DIV_38_4_MHZ", "x16"),
    ("ALIF_SCALED_FREQ_XO_LOW_DIV_19_2_MHZ", "x17"),
    ("ALIF_SCALED_FREQ_XO_LOW_DIV_9_6_MHZ", "x18"),
    ("ALIF_SCALED_FREQ_XO_LOW_DIV_4_8_MHZ", "x19"),
    ("ALIF_SCALED_FREQ_XO_LOW_DIV_2_4_MHZ", "x20"),
    ("ALIF_SCALED_FREQ_XO_LOW_DIV_1_2_MHZ", "x21"),
    ("ALIF_SCALED_FREQ_XO_LOW_DIV_0_6_MHZ", "x22"),
    ("ALIF_SCALED_FREQ_XO_LOW_DIV_0_3_MHZ", "x23"),
    ("ALIF_SCALED_FREQ_XO_HIGH_DIV_38_4_MHZ", "x24"),
    ("ALIF_SCALED_FREQ_XO_HIGH_DIV_19_2_MHZ", "x25"),
    ("ALIF_SCALED_FREQ_XO_HIGH_DIV_9_6_MHZ", "x26"),
    ("ALIF_SCALED_FREQ_XO_HIGH_DIV_2_4_MHZ", "x27"),
    ("ALIF_SCALED_FREQ_XO_HIGH_DIV_0_6_MHZ", "x28"),
    ("ALIF_SCALED_FREQ_XO_HIGH_DIV_0_3_MHZ", "x29"),
    ("ALIF_SCALED_FREQ_XO_HIGH_DIV_0_15_MHZ", "x30"),
    ("ALIF_SCALED_FREQ_XO_HIGH_DIV_0_0375_MHZ", "x31"),
]

# E7/E8 (and E3/E4) HE: SYSPLL taps 160 and 80 only.
HE_PLL_E7E8 = [
    ("160", "ALIF_CLOCK_FREQ_160MHZ"),
    ("80", "ALIF_CLOCK_FREQ_80MHZ"),
]

# E1C/B1 HE: full PLL ladder (160/80/120/60) plus the same oscillator matrix as E7/E8.
HE_PLL_E1C_B1 = [
    ("160", "ALIF_CLOCK_FREQ_160MHZ"),
    ("80", "ALIF_CLOCK_FREQ_80MHZ"),
    ("120", "ALIF_CLOCK_FREQ_120MHZ"),
    ("60", "ALIF_CLOCK_FREQ_60MHZ"),
]

# E7/E8 HP SYSPLL taps.
HP_PLL = [
    ("400", "ALIF_CLOCK_FREQ_400MHZ"),
    ("200", "ALIF_CLOCK_FREQ_200MHZ"),
    ("100", "ALIF_CLOCK_FREQ_100MHZ"),
]


# Omitted from the default okay walk (set status okay to verify on hardware).
DISABLED_SUFFIXES = frozenset(
    {
        "pllrc768_r7",
	"pllrc384_r6",
        "pllrc384_r7",
	"pllxo384_x22",
        "pllxo384_x23",
        "pllxo384_x28",
        "pllxo384_x29",
        "pllxo384_x30",
        "pllxo384_x31",
        "hfrc768_r7",
        "hfrc384_r6",
        "hfrc384_r7",
        "hfxo768_x22",
        "hfxo768_x23",
        "hfxo768_x28",
        "hfxo768_x29",
        "hfxo768_x30",
        "hfxo384_x22",
        "hfxo384_x23",
        "hfxo384_x28",
        "hfxo384_x29",
        "hfxo384_x30",
        "hfxo384_x31",
    }
)

DISABLED_NODE_COMMENT = (
    "Not in the default walk. Set status okay to verify this scaled frequency."
)


def node_disabled(core: str, label: str) -> bool:
    prefix = f"pstate_mx_{core}_"
    if not label.startswith(prefix):
        return False
    return label[len(prefix) :] in DISABLED_SUFFIXES


def emit_node(label, body_lines, comment=None):
    core = label.split("_")[2]  # pstate_mx_<core>_...
    status = "disabled" if node_disabled(core, label) else "okay"
    lines = []
    if comment:
        lines.append(f"\t\t/* {comment} */")
    elif status == "disabled":
        lines.append(f"\t\t/* {DISABLED_NODE_COMMENT} */")
    lines.extend(
        [
            f"\t\t{label}: {label} {{",
            '\t\t\tcompatible = "alif,rtss-pstate";',
            "\t\t\tload-threshold = <0>;",
        ]
    )
    lines.extend(f"\t\t\t{line}" for line in body_lines)
    lines.extend((f'\t\t\tstatus = "{status}";', "\t\t};"))
    return "\n".join(lines)


def emit_pll_nodes(out, core, pll_list):
    for mhz, cpu in pll_list:
        for sym, sfx in RC_SCALED + XO_SCALED:
            lab = f"pstate_mx_{core}_pll{mhz}_{sfx}"
            out.append(
                emit_node(
                    lab,
                    [
                        f"cpu-clk-freq = <{cpu}>;",
                        'clk-src = "pll";',
                        f"scaled-clk-freq = <{sym}>;",
                    ],
                )
            )


def emit_osc_family(out, core, src, prefix, taps, scaled):
    for tap, cpu in taps:
        for sym, sfx in scaled:
            lab = f"pstate_mx_{core}_{prefix}{tap}_{sfx}"
            out.append(
                emit_node(
                    lab,
                    [
                        f"cpu-clk-freq = <{cpu}>;",
                        f'clk-src = "{src}";',
                        f"scaled-clk-freq = <{sym}>;",
                    ],
                )
            )


def emit_osc_nodes(out, core):
    rc_taps = (
        ("768", "ALIF_CLOCK_FREQ_76_8_RC_MHZ"),
        ("384", "ALIF_CLOCK_FREQ_38_4_RC_MHZ"),
    )
    xo_taps = (
        ("768", "ALIF_CLOCK_FREQ_76_8_XO_MHZ"),
        ("384", "ALIF_CLOCK_FREQ_38_4_XO_MHZ"),
    )

    # clk-src PLL with oscillator cpu-clk-freq (same scaled pairing as HFRC/HFXO).
    emit_osc_family(out, core, "pll", "pllrc", rc_taps, RC_SCALED)
    emit_osc_family(out, core, "pll", "pllxo", xo_taps, XO_SCALED)
    emit_osc_family(out, core, "hfrc", "hfrc", rc_taps, RC_SCALED)
    emit_osc_family(out, core, "hfxo", "hfxo", xo_taps, XO_SCALED)


def generate(comment_lines, pll_list, core, include_osc):
    out = [
        "/* SPDX-FileCopyrightText: Copyright Alif Semiconductor */",
        "/* SPDX-License-Identifier: Apache-2.0 */",
        "/* Generated by scripts/regen_pstate_matrix.py */",
    ]
    out.extend(comment_lines)
    out.extend(
        [
            "#include <dt-bindings/misc/alif_aipm_common.h>",
            "",
            "/ {",
            "\tperformance-states {",
        ]
    )

    emit_pll_nodes(out, core, pll_list)
    if include_osc:
        emit_osc_nodes(out, core)

    out.extend(("\t};", "};", ""))
    return "\n".join(out)


def main():
    he_dir = SNIPPETS / "pstate-he-full"
    hp_dir = SNIPPETS / "pstate-hp-full"

    he = he_dir / "pstate_matrix_he.dtsi"
    he.write_text(
        generate(
            [
                "/* HE matrix for E7/E8 (also E3/E4): PLL 160 and 80 MHz,",
                " * PLL x HFRC/HFXO cpu-clk-freq, HFRC src, and HFXO src.",
                " * E1C/B1 use pstate_matrix_he_e1c_b1.dtsi (160/80/120/60 + osc).",
                " */",
            ],
            HE_PLL_E7E8,
            "he",
            True,
        )
    )
    print(f"wrote {he}")

    he_extra = he_dir / "pstate_matrix_he_e1c_b1.dtsi"
    he_extra.write_text(
        generate(
            [
                "/* HE full matrix for E1C/B1: PLL 160/80/120/60,",
                " * PLL x HFRC/HFXO cpu-clk-freq, HFRC src, and HFXO src.",
                " */",
            ],
            HE_PLL_E1C_B1,
            "he",
            True,
        )
    )
    print(f"wrote {he_extra}")

    hp = hp_dir / "pstate_matrix_hp.dtsi"
    hp.write_text(
        generate(
            [
                "/* HP matrix for E7/E8 (also E3/E4): PLL 400/200/100 MHz,",
                " * PLL x HFRC/HFXO cpu-clk-freq, HFRC src, and HFXO src.",
                " * E1C/B1 have no RTSS-HP.",
                " */",
            ],
            HP_PLL,
            "hp",
            True,
        )
    )
    print(f"wrote {hp}")


if __name__ == "__main__":
    main()

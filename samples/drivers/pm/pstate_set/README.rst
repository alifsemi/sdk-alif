.. _alif-pstate-set-sample:

Alif RTSS P-state set
#####################

Overview
********

Walks every enabled child of ``/performance-states`` with
``cpu_freq_pstate_set()``. Uses the board default console (uart2 on HE,
uart4 on HP). No extra UART or pinctrl.

After each P-state the sample sleeps 1 s and compares that interval to
RTC (``rtc0``). SysTick drives ``k_uptime``, so RTC is the independent
check. All snippets use the shared ``snippets/pstate_rtc0.overlay``
(HE, HP, E1C, B1).

The SoC dtsi lists a **small PLL ladder** (boot PLL ``okay``, rest
``disabled``) plus three HFRC rows in ``rtss_pstate_hfrc.dtsi``
(disabled). Snippets enable a subset; they do **not** grow the SoC
dtsi.

* ``pstate-he`` / ``pstate-hp``: demo subset (PLL step + three HFRC
  scaled rows).
* ``pstate-he-full`` / ``pstate-hp-full``: application overlay adds
  the **SoC PLL ladder** × scaled clocks, plus HFRC/HFXO:

  - HE E7/E8 (also E3/E4): PLL **160 and 80** × scaled, plus oscillator
    matrix (``pstate_matrix_he.dtsi``).
  - HE E1C/B1: PLL **160/80/120/60** × scaled, plus the same PLL × HFRC/HFXO,
    HFRC, and HFXO combinations (``pstate_matrix_he_e1c_b1.dtsi``).
  - HP E7/E8 (also E3/E4): PLL **400/200/100** × 24 scaled.
    E1C/B1 have no RTSS-HP.
  - PLL ``clk-src`` with HFRC/HFXO ``cpu-clk-freq`` (76.8/38.4 RC ×
    RC Active, 76.8/38.4 XO × XO dividers).
  - HFRC ``clk-src`` ``76.8 RC`` / ``38.4 RC`` × RC Active 0–7.
  - HFXO ``clk-src`` ``76.8 XO`` / ``38.4 XO`` × all XO divider enums.
  - Very low scaled frequencies are ``disabled`` in the default walk;
    set ``status = \"okay\"`` on a row to verify it on hardware.

  Matrix nodes live under
  ``samples/.../snippets/pstate-*-full/pstate_matrix_*.dtsi``
  Regenerate after ``alif_aipm_common.h`` changes::

    python3 scripts/regen_pstate_matrix.py

After each P-state the sample restores the boot PLL, then logs a walk
line (``LOG_INF``) so a long matrix does not look hung. UART is back at
boot baud before that line. After the walk it prints a summary plus
grouped PASS/FAIL lists. Per-row clocks, SE vote mismatch, and sleep
checks are ``LOG_DBG``; enable with ``-DCONFIG_PSTATE_SET_LOG_LEVEL_DBG=y``.
``CONFIG_LOG_DEFAULT_LEVEL`` stays 0 so other modules stay quiet.

SE may reject a combination; that row is recorded as APPLY_FAIL and the
walk continues.

Requirements
************

* Alif Ensemble or Balletto RTSS board
* ``CONFIG_ALIF_CPU_FREQ_PSTATE`` (set in ``prj.conf``)
* Matching snippet: ``pstate-he``, ``pstate-hp``, ``pstate-he-full``, or
  ``pstate-hp-full``

Building and Running
********************

HE
==

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/pstate_set
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_he
   :goals: build
   :west-args: -p auto
   :snippets: pstate-he

HP
==

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/pstate_set
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_hp
   :goals: build
   :west-args: -p auto
   :snippets: pstate-hp

HE full matrix
==============

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/pstate_set
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_he
   :goals: build
   :west-args: -p auto
   :snippets: pstate-he-full

HP full matrix
==============

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/pstate_set
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_hp
   :goals: build
   :west-args: -p auto
   :snippets: pstate-hp-full

Expected console
****************

::

   pstate_set: walking 5 states (EXTSYS1, rtc0)
   [00:00:01.234,000] <inf> pstate_set: [1/5] pstate-pll-160: PASS
   [00:00:02.456,000] <inf> pstate_set: [2/5] pstate-pll-80: PASS
   ...
   pstate_set: walked 5 states (EXTSYS1, rtc0)

   Summary
     PASS  5
     FAIL  0
     boot restore ok

   PASS
     pstate-pll-160
     pstate-pll-80
     pstate-scaled-hfrc-76p8
     pstate-scaled-hfrc-38p4
     pstate-scaled-hfrc-19p2

   pstate_set: PASSED

A FAIL row looks like ``pstate-scaled-hfrc-19p2  VERIFY_FAIL (err=-5)``.
The empty FAIL section is omitted. Full-matrix snippets use the same
layout with longer lists.

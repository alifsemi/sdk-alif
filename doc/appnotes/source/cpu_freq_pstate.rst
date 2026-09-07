.. _appnote-cpu-freq-pstate:

===========================
RTSS CPU Frequency P-states
===========================

Introduction
============

RTSS cores can change the Secure Enclave (SE) run profile at runtime with
the upstream Zephyr API ``cpu_freq_pstate_set()``. Each Devicetree child of
``/performance-states`` is a **destination**: the SoC copies
``aipm_run_default`` and overlays only the properties that P-state sets,
then sends the composed ``run_profile_t`` to SE.

This is a **manual** call from the application. Do **not** set
``CONFIG_CPU_FREQ=y``. That option starts the Zephyr frequency governor,
which this tree does not use. The Alif hook is
``CONFIG_ALIF_CPU_FREQ_PSTATE``.

Cold-boot clocks and power domains stay with the run profile. See
:ref:`appnote-se-aipm-profiles` for ``aipm_run`` / ``aipm_off``. Clock IDs
and ``clock_control_get_rate()`` are covered in
:ref:`appnote-zephyr-clock-control`.

Enable the Feature
==================

In the application ``prj.conf``:

.. code-block:: cfg

   CONFIG_ALIF_CPU_FREQ_PSTATE=y

``CONFIG_ALIF_CPU_FREQ_PSTATE`` implements ``cpu_freq_pstate_set()`` and
updates SysTick when the CPU clock changes. It also selects
``CONFIG_PM_DEVICE`` so the hook can suspend and resume clock consumers
(UART baud and similar rates are rewritten after the move).

Leave ``CONFIG_CPU_FREQ`` unset (``n``).

P-state Properties
==================

Compatible is ``alif,rtss-pstate``. Property names and values match
``alif,aipm-run`` children. Use ``ALIF_CLOCK_FREQ_*`` and
``ALIF_SCALED_FREQ_*`` from ``dt-bindings/misc/alif_aipm_common.h``.

.. list-table::
   :widths: 25 10 65
   :header-rows: 1

   * - Property
     - Required
     - Description
   * - ``cpu-clk-freq``
     - **Yes**
     - CPU / EXTSYS frequency token.
   * - ``clk-src``
     - No
     - ``"pll"``, ``"hfrc"``, or ``"hfxo"``. Omitted: keep
       ``aipm_run_default``.
   * - ``scaled-clk-freq``
     - No
     - Scaled HF clock (HFRC / HFXO rows). Omitted: keep the run default.
   * - ``dcdc-voltage``
     - No
     - DCDC millivolts: 800, 825, or 850.
   * - ``dcdc-mode``
     - No
     - ``"off"``, ``"pfm-auto"``, ``"pfm-forced"``, ``"pwm"``.
   * - ``load-threshold``
     - No
     - Unused without the governor. SoC rows set ``<0>``.

Omit a property to keep ``aipm_run_default``. Do not put
``aipm-power-domains``, memory-block, gating, ioflex, or aon-clk on a
P-state; those stay with the run profile. Clock and DCDC enumerations are
the same as in :ref:`appnote-se-aipm-profiles`.

SoC Menu
========

The SoC DTSI lists the PLL and HFRC rows. Only the **boot PLL** is okay.
Other rows are ``status = "disabled"`` so they do not create
``struct pstate`` objects until an overlay enables them.

.. list-table::
   :widths: 22 28 50
   :header-rows: 1

   * - Core
     - Okay at boot
     - Disabled until overlay
   * - HE / E1C / B1
     - ``pstate_pll_160`` (160 MHz PLL)
     - PLL 120 / 80 / 60 MHz; HFRC 76.8 / 38.4 / 19.2 MHz
   * - HP
     - ``pstate_pll_400`` (400 MHz PLL)
     - PLL 200 / 100 MHz; same three HFRC rows

HFRC rows live in ``dts/arm/alif/common/rtss_pstate_hfrc.dtsi``. Labels:

* PLL: ``pstate_pll_<mhz>`` (node name ``pstate-pll-<mhz>``)
* HFRC: ``pstate_scaled_hfrc_76p8``, ``pstate_scaled_hfrc_38p4``,
  ``pstate_scaled_hfrc_19p2``


Enable a SoC Row
================

Set ``status = "okay"`` on a disabled child in the board or application
overlay. The sample snippets do this for the demo subset:

.. code-block:: dts

   &pstate_pll_80 {
           status = "okay";
   };

   &pstate_scaled_hfrc_76p8 {
           status = "okay";
   };

Add a Custom P-state
====================

Add a new child under ``/performance-states`` in the application overlay.
The node merges with the SoC menu. Constants are already visible from the
SoC include of ``alif_aipm_common.h``.

PLL example (destination 80 MHz; DCDC left at the run default):

.. code-block:: dts

   / {
           performance-states {
                   pstate_app_pll_80: pstate-app-pll-80 {
                           compatible = "alif,rtss-pstate";
                           load-threshold = <0>;
                           cpu-clk-freq = <ALIF_CLOCK_FREQ_80MHZ>;
                           clk-src = "pll";
                   };
           };
   };

HFRC example that is **not** in the SoC menu (9.6 MHz scaled). ``cpu-clk-freq``
stays the 76.8 MHz RC token; ``scaled-clk-freq`` selects the divider:

.. code-block:: dts

   / {
           performance-states {
                   pstate_app_hfrc_9p6: pstate-app-hfrc-9p6 {
                           compatible = "alif,rtss-pstate";
                           load-threshold = <0>;
                           cpu-clk-freq = <ALIF_CLOCK_FREQ_76_8_RC_MHZ>;
                           clk-src = "hfrc";
                           scaled-clk-freq = <ALIF_SCALED_FREQ_RC_ACTIVE_9_6_MHZ>;
                   };
           };
   };

A custom row is still **one core’s vote**. SE merges ``run_clk_src`` and
the scaled clock across cores. See `SE Vote`_.

Apply a P-state from the Application
====================================

Look up the node by **label**, not by walk order:

.. code-block:: c

   #include <zephyr/cpu_freq/cpu_freq.h>
   #include <zephyr/cpu_freq/pstate.h>
   #include <zephyr/devicetree.h>

   int err = cpu_freq_pstate_set(
           PSTATE_DT_GET(DT_NODELABEL(pstate_pll_80)));

Use ``DT_NODELABEL(pstate_app_pll_80)`` (or the HFRC label) for a custom
node. Restore the boot PLL the same way
(``pstate_pll_160`` on HE, ``pstate_pll_400`` on HP).

A return of **0** means this core’s vote was **accepted**. It does not
mean AXI, AHB, APB, or the other core’s EXTSYS now match the request.

SE Vote
=======

SE combines run-clock source and scaled frequency from the cores that
have voted. If HP stays on PLL while HE requests HFRC, SYST buses can
remain at PLL rates and ``se_service_get_run_cfg()`` can differ from
``se_service_get_last_set_run_cfg()`` on the requesting core.

To see whether the system adopted the request, compare those two
profiles (``run_clk_src``, ``cpu_clk_freq``, ``scaled_clk_freq``) or read
clocks with ``se_service_clock_setting_get()``. Do not treat a mismatch
as an API failure; the vote still succeeded.

What the Hook Updates
=====================

On success the SoC hook:

* Suspends clock-control dependents (EXTSYS-only when only
  ``cpu-clk-freq`` changes; all ``clockctrl`` PM devices when
  ``clk-src`` or ``scaled-clk-freq`` changes).
* Sends the composed run profile to SE.
* Invalidates the Zephyr ``get_rate`` cache for the clocks that moved.
* Programs SysTick. Hertz is ``cpu-clk-freq``, except when the destination
  ``clk-src`` is not PLL **and** ``scaled-clk-freq`` is set, in which case
  SysTick uses the scaled Hertz.

HFRC is an RC oscillator. A one-second ``k_msleep()`` measured against
LFXO RTC can be a few milliseconds short of 1000 ms even when SysTick
matches EXTSYS.

Concurrency
===========

The hook is **not** safe for overlapping calls. Only one
``cpu_freq_pstate_set()`` may be in progress on a core. The
application is the owner: serialize P-state changes (one thread, or
an application lock). Do not call the hook from an ISR.

If a clock consumer is marked busy (in-flight SPI, I2C, PDM, …), the
hook returns ``-EBUSY`` and rolls back, same busy bit as system PM.
Wait for that transfer to finish, then retry. A driver that neither
sets busy nor returns ``-EBUSY`` from suspend can still be forced
down.

``irq_lock()`` covers only the Secure Enclave apply, clock-cache
invalidate, and SysTick update. Deciding which dependents to suspend
and parking those devices happen **before** that lock. Two threads
that interleave there can park the wrong set and drop the wrong cache
slots.

Using with Power Management
===========================

``cpu_freq_pstate_set()`` can be used with ``CONFIG_PM``. The hook does
not take a policy lock against sleep. If a P-state change must not race
idle or ``SUSPEND_TO_RAM``, the application holds that lock itself
(the same lock can serialize P-state callers).

On ``SUSPEND_TO_RAM`` and ``SOFT_OFF`` entry, AIPM drops its cached run
profile so resume can re-sync with SE. Resume then applies the Devicetree
run profile (``aipm_run_default``, or a matching ``aipm_run`` child). If
the application still wants a P-state after wakeup, it calls
``cpu_freq_pstate_set()`` again.

Limitations
===========

* **APSS:** not covered.
* **Governor:** ``CONFIG_CPU_FREQ`` stays off. A later mix of the
  on-demand governor and reserved manual P-states must share **one**
  workqueue owner. The hook is not IRQ-safe; the governor timer cannot
  call it directly.

Sample
======

``samples/drivers/pm/pstate_set`` walks every okay ``/performance-states``
child, prints EXTSYS / AXI / AHB / APB, and checks SysTick against LPRTC.
Snippets ``pstate-he`` and ``pstate-hp`` enable the demo rows only.

.. code-block:: console

   west build -p auto -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/pstate_set -S pstate-he

   west build -p auto -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/pm/pstate_set -S pstate-hp

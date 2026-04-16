.. _appnote-se-aipm-profiles:

==================================
SE aiPM Run and Off Profile (DTS)
==================================

Introduction
============

Alif SoCs use Secure Enclave (SE) to manage global power and clock settings
through the aiPM (Autonomous Intelligent Power Management) framework. Two
profiles control this:

* **Run profile** — applied at cold boot and on each PM resume. Sets the CPU
  clock, DCDC mode, power domains, and other active-state parameters.

* **Off profile** — applied on PM state entry (``SUSPEND_TO_RAM``,
  ``SOFT_OFF``). Sets memory retention, wakeup sources, EWIC configuration,
  and standby clocks before the CPU is power-gated.

Both profiles are configured via Devicetree nodes (``aipm_run``,
``aipm_off``) defined in the SoC DTSI. The run profile is active by default
(``status = "okay"``); board overlays override only the properties that
differ from the SoC defaults. The off profile is ``status = "disabled"`` by
default and must be explicitly enabled in a board overlay. No application
code changes are required — the SE service driver applies the profiles
automatically.

Run Profile
===========

Overview
--------

The ``aipm_run`` node (compatible ``alif,aipm-run``) holds one child node
per PM state/substate. A child without ``pm-state`` acts as the cold-boot
default and universal fallback.

The profile is applied:

* At PRE_KERNEL_1 priority 45 (``se_service_mhuv2_nodes_init``), just after
  the SE communication channel is ready.
* In the ``pre_device_resume`` PM notifier before devices are resumed,
  ensuring the correct power domains are active before driver callbacks run.

.. note::
   Any driver or ``SYS_INIT`` callback that needs a power domain enabled
   (e.g. SYSTOP for peripheral register access) must run at PRE_KERNEL_1
   priority **greater than 45** so the run profile — and its power domains —
   is already active.

Key Properties (per child node)
---------------------------------

.. list-table::
   :widths: 25 10 65
   :header-rows: 1

   * - Property
     - Required
     - Description
   * - ``pm-state``
     - No
     - PM state this child targets on resume. Omit for cold-boot default.
   * - ``pm-substate``
     - No
     - Substate index. Default 0.
   * - ``aipm-power-domains``
     - No
     - Power domain bitmask (``ALIF_PD_*_MASK``). Domains kept powered.
   * - ``cpu-clk-freq``
     - **Yes**
     - CPU clock frequency. Use ``ALIF_CLOCK_FREQ_*`` constants.
   * - ``clk-src``
     - No
     - HF run clock source: ``"hfrc"``, ``"hfxo"``, ``"pll"``.
   * - ``dcdc-mode``
     - No
     - DCDC mode: ``"off"``, ``"pfm-auto"``, ``"pfm-forced"``, ``"pwm"``.
   * - ``dcdc-voltage``
     - No
     - DCDC voltage in mV: 800, 825, or 850.
   * - ``aon-clk-src``
     - No
     - Always-on LF clock: ``"lfrc"`` or ``"lfxo"``.
   * - ``memory-blocks``
     - No
     - Memory retention bitmask. See `Memory Block Reference`_.
   * - ``vdd-ioflex``
     - No
     - GPIO voltage: ``ALIF_IOFLEX_LEVEL_1V8`` (default) or ``ALIF_IOFLEX_LEVEL_3V3``.

Example — Enabling and Customising the Run Profile
----------------------------------------------------

The SoC DTSI defines the ``aipm_run`` node with ``status = "okay"`` and a
default child. A board overlay overrides only the child properties that
differ from the SoC defaults:

.. code-block:: dts

   /* Override the default cold-boot profile */
   &run_profile_default {
       status = "okay";
       aipm-power-domains = <(ALIF_PD_SSE700_AON_MASK |
                              ALIF_PD_VBAT_AON_MASK   |
                              ALIF_PD_SYST_AON_MASK   |
                              ALIF_PD_SYST_MEM_MASK   |
                              ALIF_PD_SYST_FULL_MASK)>;
       cpu-clk-freq  = <ALIF_CLOCK_FREQ_400_MHZ>;
       clk-src       = "pll";
       dcdc-mode     = "pwm";
       dcdc-voltage  = <850>;
       aon-clk-src   = "lfxo";
   };

Off Profile
===========

Overview
--------

The ``aipm_off`` node (compatible ``alif,aipm-off``) holds one child node
per PM state/substate for which an off profile is needed. The SE service
matches by ``(pm-state, pm-substate)`` in the ``state_entry`` PM notifier —
**before** the SE communication channel is closed.

The parent node carries ``vtor-address``: the vector table base address the
SE uses to resume CPU execution after wakeup.

.. list-table:: Default ``vtor-address`` values by core
   :widths: 40 30 30
   :header-rows: 1

   * - Core / SoC
     - Default (MRAM boot)
     - TCM boot override
   * - RTSS_HE (Ensemble, E1C, B1)
     - ``0x80000000``
     - ``0x00000000``
   * - RTSS_HP (Ensemble)
     - ``0x80200000``
     - N/A (HP has no TCM Retention mode)

Key Properties (parent node)
------------------------------

.. list-table::
   :widths: 25 10 65
   :header-rows: 1

   * - Property
     - Required
     - Description
   * - ``vtor-address``
     - **Yes**
     - Vector table base address. Set in SoC DTSI; override in board overlay
       for TCM or SRAM-relocation boot.

Key Properties (per child node)
---------------------------------

.. list-table::
   :widths: 25 10 65
   :header-rows: 1

   * - Property
     - Required
     - Description
   * - ``pm-state``
     - **Yes**
     - PM state this child applies to. ``4`` = SUSPEND_TO_RAM,
       ``6`` = SOFT_OFF.
   * - ``pm-substate``
     - No
     - Substate index. Default 0.
   * - ``aipm-power-domains``
     - No
     - Power domains kept powered during sleep.
   * - ``memory-blocks``
     - No
     - Memory retention bitmask. Default 0 (no retention).
       See `Memory Block Reference`_.
   * - ``wakeup-events``
     - No
     - Wakeup source bitmask (``ALIF_WE_*``). Default 0 — **must** be set
       in board overlay to enable wakeup.
   * - ``ewic-cfg``
     - No
     - EWIC configuration bitmask (``ALIF_EWIC_*``). Default 0 — **must**
       be set in board overlay to enable EWIC wakeup.
   * - ``dcdc-mode``
     - No
     - DCDC mode during sleep. Typically ``"off"``.
   * - ``aon-clk-src``
     - No
     - Always-on LF clock: ``"lfrc"`` or ``"lfxo"`` (default).
   * - ``stby-clk-src``
     - No
     - Standby HF clock: ``"hfrc"`` (default), ``"hfxo"``, or ``"pll"``.
   * - ``stby-clk-freq``
     - No
     - Scaled standby clock frequency. Use ``ALIF_SCALED_FREQ_RC_STDBY_*``.
   * - ``vdd-ioflex``
     - No
     - GPIO voltage level. Default ``ALIF_IOFLEX_LEVEL_1V8``.

Example — Enabling the Off Profile in a Board Overlay
------------------------------------------------------

.. code-block:: dts

   /* Override vtor-address for TCM boot */
   &aipm_off {
       vtor-address = <0x00000000>;
       status = "okay";
   };

   /* SUSPEND_TO_RAM substate 0 (STANDBY) */
   &off_profile_standby {
       status = "okay";
       wakeup-events = <ALIF_WE_LPRTC>;
       ewic-cfg      = <ALIF_EWIC_RTC_A>;
       memory-blocks = <(ALIF_SRAM4_1_MASK | ALIF_SRAM4_2_MASK |
                         ALIF_SRAM5_1_MASK | ALIF_SRAM5_2_MASK |
                         ALIF_SERAM_MASK   | ALIF_MRAM_MASK)>;
   };

   /* SUSPEND_TO_RAM substate 1 (STOP) */
   &off_profile_stop {
       status = "okay";
       wakeup-events = <ALIF_WE_LPRTC>;
       ewic-cfg      = <ALIF_EWIC_RTC_A>;
       memory-blocks = <(ALIF_SRAM4_1_MASK | ALIF_SRAM4_2_MASK |
                         ALIF_SRAM5_1_MASK | ALIF_SRAM5_2_MASK |
                         ALIF_SERAM_MASK   | ALIF_MRAM_MASK)>;
   };

.. note::
   ``wakeup-events`` and ``ewic-cfg`` default to ``0`` in the SoC DTSI
   (safe default — no spurious wakeups). Always set them explicitly in the
   board overlay for the desired wakeup source.

Memory Block Reference
======================

The ``memory-blocks`` bitmask selects which memories the SE keeps powered
and retained during sleep. Use constants from the per-SoC header included in
the SoC DTSI.

.. list-table:: HE Core Memory Blocks by SoC Family
   :widths: 30 35 35
   :header-rows: 1

   * - Block
     - Ensemble / Gen2 (E3–E8)
     - E1C / B1
   * - ITCM (SRAM4)
     - ``ALIF_SRAM4_1_MASK | ALIF_SRAM4_2_MASK``
     - ``ALIF_SRAM4_1_MASK`` … ``ALIF_SRAM4_4_MASK``
   * - DTCM (SRAM5)
     - ``ALIF_SRAM5_1_MASK | ALIF_SRAM5_2_MASK``
     - ``ALIF_SRAM5_1_MASK`` … ``ALIF_SRAM5_5_MASK``
   * - SERAM
     - ``ALIF_SERAM_MASK`` (1 bit / 2 bits)
     - ``ALIF_SERAM_MASK`` (4 bits)
   * - MRAM
     - ``ALIF_MRAM_MASK``
     - ``ALIF_MRAM_MASK``

.. note::
   ``ALIF_SERAM_MASK`` resolves to the correct width automatically via the
   per-SoC header pulled in by the SoC DTSI. No manual bit-width selection
   is needed.

PM System Off Snippet
======================

The ``pm-system-off-he`` snippet provides ready-to-use overlay files for the
HE core. See the :ref:`alif-pm-states-sample` sample for build instructions
and per-SoC overlay selection details.

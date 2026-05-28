.. _alif-pm-states-sample:

Alif Power Management States Demo
##################################

Overview
********

This sample demonstrates Zephyr power management states on Alif RTSS cores,
showcasing different PM state transitions with RTC/Timer wakeup capabilities.
The PM states exercised depend on the *capability* of the target, selected via
the build snippet:

* **PM_STATE_RUNTIME_IDLE**: Light sleep (WFI); all clocks and retention intact
* **PM_STATE_SUSPEND_TO_IDLE**: CPU sleep with IWIC; devices stay active
* **PM_STATE_SUSPEND_TO_RAM (S2RAM)**: Deep sleep with retention(SERAM & TCM/SRAM0)

  * Substate 0 (STANDBY): PD0, PD1 & PD2 will be ON
  * Substate 1 (STOP): Deeper power savings, VBAT-AON(PD0) only

* **PM_STATE_SOFT_OFF**: Deepest sleep; no retention, full system reset on wakeup

Two compile-time predicates drive the state machine in ``main.c``;

``S2RAM_SUPPORTED``
  True when the DTS ``chosen`` node ``zephyr,sram`` points at ``sram0``
  (set by the snippet overlay) OR when the HE core boots from TCM (with retention).
  The snippet overlay is responsible for ensuring the target has SRAM0 retention capability.

``SOFT_OFF_SUPPORTED``
  True when ``S2RAM_SUPPORTED`` is false (mutually exclusive).

Capability Matrix
=================

.. list-table::
   :header-rows: 1
   :widths: 30 15 15 40

   * - Snippet
     - Core(s)
     - Data RAM
     - PM states exercised
   * - ``pm-system-off-s2ram-tcm``
     - HE only
     - TCM (SRAM4/5)
     - RUNTIME_IDLE → SUSPEND_TO_IDLE → S2RAM STANDBY → S2RAM STOP
   * - ``pm-system-off-mram``
     - HE + HP
     - TCM / MRAM boot
     - RUNTIME_IDLE → SUSPEND_TO_IDLE → SOFT_OFF
   * - ``pm-system-off-s2ram-sram0``
     - HE + HP
     - SRAM0 (E8 only)
     - RUNTIME_IDLE → SUSPEND_TO_IDLE → S2RAM STANDBY → S2RAM STOP

Requirements
************

* Alif Ensemble or Balletto development board
* RTC or LPTIMER0 peripheral enabled for wakeup (configured by snippet)
* SE Services for power profile configuration (configured via DTS overlay)

Building and Running
********************

HE Core — TCM boot S2RAM (E7/E8/E1C/B1)
=========================================

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/system_off
   :board: alif_e7_dk/ae722f80f55d5xx/rtss_he
   :goals: build
   :west-args: -p auto
   :snippets: pm-system-off-s2ram-tcm
   :gen-args: -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

HE Core — MRAM boot SOFT_OFF
=============================

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/system_off
   :board: alif_e7_dk/ae722f80f55d5xx/rtss_he
   :goals: build
   :west-args: -p auto
   :snippets: pm-system-off-mram

HP Core — MRAM boot SOFT_OFF
=============================

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/system_off
   :board: alif_e7_dk/ae722f80f55d5xx/rtss_hp
   :goals: build
   :west-args: -p auto
   :snippets: pm-system-off-mram

HE Core — SRAM0 S2RAM (E8 only)
================================

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/system_off
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_he
   :goals: build
   :west-args: -p auto
   :snippets: pm-system-off-s2ram-sram0

HP Core — SRAM0 S2RAM (E8 only)
================================

Both HE and HP require a first-stage loader in MRAM to power up SRAM0
before the Zephyr image runs.  The loader placement is user-defined; the
``aipm_off`` ``vtor-address`` must be set to the address where the loader
is placed so the SE restores execution there on wakeup.  The Zephyr
application uses RTC as the idle timer; ensure no other core is using RTC
when this application runs.

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/system_off
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_hp
   :goals: build
   :west-args: -p auto
   :snippets: pm-system-off-s2ram-sram0

Flash the binary using SE Tools. See :ref:`programming_an_application` for
details.

Sample Output
*************

HE Core — TCM boot (S2RAM STANDBY → STOP)
==========================================

.. code-block:: console

   *** Booting Zephyr OS build v4.1.0 ***
   [00:00:00.004,000] <inf> pm_system_off: alif_e7_dk (S2RAM): PM states demo (RUNTIME_IDLE, SUSPEND_TO_IDLE, S2RAM STANDBY, S2RAM STOP)
   [00:00:00.016,000] <inf> pm_system_off: POWER STATE SEQUENCE:
   [00:00:00.022,000] <inf> pm_system_off:   1. PM_STATE_RUNTIME_IDLE
   [00:00:00.029,000] <inf> pm_system_off:   2. PM_STATE_SUSPEND_TO_IDLE
   [00:00:00.036,000] <inf> pm_system_off:   3. PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY)
   [00:00:00.044,000] <inf> pm_system_off:   4. PM_STATE_SUSPEND_TO_RAM (substate 1: STOP)
   [00:00:00.062,000] <inf> pm_system_off: Enter RUNTIME_IDLE sleep for (18000000 microseconds)
   [00:00:18.071,000] <inf> pm_system_off: Exited from RUNTIME_IDLE sleep
   [00:00:18.077,000] <inf> pm_system_off: Enter PM_STATE_SUSPEND_TO_IDLE for (4000 microseconds)
   [00:00:18.092,000] <inf> pm_system_off: Exited from PM_STATE_SUSPEND_TO_IDLE
   [00:00:18.099,000] <inf> pm_system_off: Enter PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) for (6000000 microseconds)
   [00:00:24.115,000] <inf> pm_system_off: === Resumed from PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) ===
   [00:00:24.125,000] <inf> pm_system_off: Main thread running - iteration 0 - tick: 24125
   [00:00:26.135,000] <inf> pm_system_off: Main thread running - iteration 1 - tick: 26135
   [00:00:28.145,000] <inf> pm_system_off: Main thread running - iteration 2 - tick: 28145
   [00:00:30.154,000] <inf> pm_system_off: Enter PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) for (9000000 microseconds)
   [00:00:39.169,000] <inf> pm_system_off: === Resumed from PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) ===
   [00:00:39.178,000] <inf> pm_system_off: Main thread running - iteration 0 - tick: 39178
   [00:00:41.188,000] <inf> pm_system_off: Main thread running - iteration 1 - tick: 41188
   [00:00:43.198,000] <inf> pm_system_off: Main thread running - iteration 2 - tick: 43198
   [00:00:45.207,000] <inf> pm_system_off: === POWER STATE SEQUENCE COMPLETED ===

HP Core — MRAM boot (SOFT_OFF)
===============================

.. code-block:: console

   *** Booting Zephyr OS build v4.1.0 ***
   [00:00:00.005,000] <inf> pm_system_off: alif_e7_dk (SOFT_OFF): PM states demo (RUNTIME_IDLE, SUSPEND_TO_IDLE, SOFT_OFF)
   [00:00:00.017,000] <inf> pm_system_off: POWER STATE SEQUENCE:
   [00:00:00.023,000] <inf> pm_system_off:   1. PM_STATE_RUNTIME_IDLE
   [00:00:00.030,000] <inf> pm_system_off:   2. PM_STATE_SUSPEND_TO_IDLE
   [00:00:00.037,000] <inf> pm_system_off:   3. PM_STATE_SOFT_OFF
   [00:00:00.043,000] <inf> pm_system_off: Enter RUNTIME_IDLE sleep for (18000000 microseconds)
   [00:00:18.054,000] <inf> pm_system_off: Exited from RUNTIME_IDLE sleep
   [00:00:18.060,000] <inf> pm_system_off: PM_STATE_SUSPEND_TO_IDLE (skipped - LPM timer not enabled)
   [00:00:18.070,000] <inf> pm_system_off: Enter PM_STATE_SOFT_OFF for (10000000 microseconds)
   [00:00:18.078,000] <inf> pm_system_off: Note: SOFT_OFF has no retention - system will reset on wakeup

   <-- System resets after 10 seconds -->

   *** Booting Zephyr OS build v4.1.0 ***
   [00:00:00.005,000] <inf> pm_system_off: alif_e7_dk (SOFT_OFF): PM states demo (RUNTIME_IDLE, SUSPEND_TO_IDLE, SOFT_OFF)
   [Cycle repeats...]

Notes
*****

* **Debugger**: Disconnect the debugger before testing — it prevents cores
  from entering OFF states.
* **UART Hub**: If using a USB hub for UART, set ``CONFIG_BOOT_DELAY`` to
  avoid missing logs after a power cycle.
* **Sleep durations** (all below the 10.7 s uint32 overflow ceiling at
  400 MHz):

  * RUNTIME_IDLE: 18 s (WFI, not subject to overflow)
  * SUSPEND_TO_IDLE: 4 ms
  * S2RAM STANDBY: 6 s
  * S2RAM STOP: 9 s
  * SOFT_OFF: 10 s

* **SUSPEND_TO_IDLE**:

  * Requires ``CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER`` (LPM timer).
  * HE core: enabled by default with RTC as LPM timer.
  * HP core: skipped by default (enabled when ``-S pm-system-off-s2ram-sram0``
    is used, with RTC as idle timer).
  * Uses IWIC (Internal WIC) — lighter than EWIC.
  * Only interrupts 0–63 can wake from IWIC mode.
  * Devices remain active (no suspend/resume overhead).

* **SRAM0 S2RAM — E8 only**: SRAM0 retention masks
  (``ALIF_SRAM0_*_RET_MASK``) are defined only in the Ensemble Gen2
  DT-bindings header.  Using ``-S pm-system-off-s2ram-sram0`` on E7 or
  earlier will fail with a DTS compile error.
* **SRAM0 S2RAM — RTC as idle timer**: This application configures RTC as
  the idle timer (``zephyr,cortex-m-idle-timer``).  Ensure no other core
  is using RTC while this application runs.
* **CONFIG_POWEROFF**: Alternative mode to test ``sys_poweroff()`` instead
  of the PM state sequence.
* **Power measurement**: Disable all unused peripherals for accurate numbers.

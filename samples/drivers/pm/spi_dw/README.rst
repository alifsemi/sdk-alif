.. _spi-dw-pm-sample:

SPI DW Power Management Demo
#############################

Overview
********

This sample demonstrates Zephyr power management states combined with SPI DW
loopback transfers on Alif RTSS cores. The application cycles through PM states
and runs an SPI master/slave loopback after each wake, verifying that the SPI
peripheral resumes correctly.

The PM states exercised depend on the *capability* of the target, selected via
the build snippet:

* **PM_STATE_RUNTIME_IDLE**: Light sleep (WFI); all clocks and retention intact
* **PM_STATE_SUSPEND_TO_IDLE**: CPU sleep with IWIC; devices stay active
* **PM_STATE_SUSPEND_TO_RAM (S2RAM)**: Deep sleep with retention (SERAM & TCM/SRAM0)

  * Substate 0 (STANDBY): PD0, PD1 & PD2 will be ON
  * Substate 1 (STOP): Deeper power savings, VBAT-AON (PD0) only

* **PM_STATE_SOFT_OFF**: Deepest sleep; no retention, full system reset on wakeup

Two compile-time predicates drive the state machine in ``main.c``:

``S2RAM_SUPPORTED``
  True when the DTS ``chosen`` node ``zephyr,sram`` points at ``sram0``
  (set by the snippet overlay) OR when the HE core boots from TCM (with retention).
  The snippet overlay is responsible for ensuring the target has sram retention capability.

``SOFT_OFF_SUPPORTED``
  True when ``S2RAM_SUPPORTED`` is false (mutually exclusive).

SPI master and slave threads run DMA loopback transfers before each deep-sleep
phase. Transfers are guarded with ``pm_policy_state_lock`` so the CPU cannot
enter ``SUSPEND_TO_IDLE`` while DMA is active (SPI and DMA IRQs fall outside
the IWIC wakeup range). Threads and semaphores live in retained RAM so they
resume from ``k_sem_take()`` after S2RAM without re-creation.

Capability Matrix
=================

.. list-table::
   :header-rows: 1
   :widths: 30 15 15 40

   * - Snippet
     - Core(s)
     - Data RAM
     - PM states exercised
   * - ``spi-dw-pm-s2ram-tcm``
     - HE only
     - TCM (SRAM4/5)
     - RUNTIME_IDLE → SUSPEND_TO_IDLE → S2RAM STANDBY → S2RAM STOP
   * - ``spi-dw-pm-mram``
     - HE + HP
     - TCM / MRAM boot
     - RUNTIME_IDLE → SUSPEND_TO_IDLE → SOFT_OFF
   * - ``spi-dw-pm-s2ram-sram0``
     - HE + HP
     - SRAM0 (E8 only)
     - RUNTIME_IDLE → SUSPEND_TO_IDLE → S2RAM STANDBY → S2RAM STOP

SPI Instances
=============

.. list-table::
   :header-rows: 1
   :widths: 22 20 20 38

   * - Target
     - Master
     - Slave
     - DMA / event router
   * - E7 HE
     - SPI4
     - SPI0
     - DMA2 + DMA0, evtrtr2 + evtrtr0
   * - E8 HE
     - SPI4
     - SPI1
     - DMA2 + DMA0, evtrtr2 + evtrtr0
   * - E7 / E8 HP
     - SPI1
     - SPI0
     - DMA0, evtrtr0
   * - E1C HE
     - LPSPI0
     - SPI0
     - DMA2, evtrtr2
   * - B1 HE
     - LPSPI0
     - SPI1
     - DMA2, evtrtr2

Wire the chosen master and slave instances in loopback (MOSI↔MISO, SCLK, SS).

.. note::

   Use the LPUART port for console logs on E1C and B1 DevKits.

.. note::

   On E8 HP, UART2 (P1_0 / P1_1) shares pins with SPI0. Both the
   ``spi-dw-pm-mram`` and ``spi-dw-pm-s2ram-sram0`` E8 HP overlays move the
   console to UART4 (P12_1 / P12_2). Connect the UART hub to UART4 for those
   builds. E7 builds keep UART2.

Requirements
************

* Alif Ensemble or Balletto development board
* RTC or LPTIMER0 peripheral enabled for wakeup (configured by snippet)
* SE Services for power profile configuration (configured via DTS overlay)
* External loopback wiring between the master and slave SPI instances

Building and Running
********************

HE Core — TCM boot S2RAM (E7/E8/E1C/B1)
=========================================

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/spi_dw
   :board: alif_e7_dk/ae722f80f55d5xx/rtss_he
   :goals: build
   :west-args: -p auto
   :snippets: spi-dw-pm-s2ram-tcm
   :gen-args: -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

HE Core — MRAM boot SOFT_OFF
=============================

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/spi_dw
   :board: alif_e7_dk/ae722f80f55d5xx/rtss_he
   :goals: build
   :west-args: -p auto
   :snippets: spi-dw-pm-mram

HP Core — MRAM boot SOFT_OFF
=============================

On E8 HP, use UART4 for console (see the Overview note). The E7 HP example
below keeps UART2.

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/spi_dw
   :board: alif_e7_dk/ae722f80f55d5xx/rtss_hp
   :goals: build
   :west-args: -p auto
   :snippets: spi-dw-pm-mram

E8 HP — MRAM boot SOFT_OFF (UART4 console)
==========================================

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/spi_dw
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_hp
   :goals: build
   :west-args: -p auto
   :snippets: spi-dw-pm-mram

HE Core — SRAM0 S2RAM (E8 only)
================================

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/spi_dw
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_he
   :goals: build
   :west-args: -p auto
   :snippets: spi-dw-pm-s2ram-sram0

HP Core — SRAM0 S2RAM (E8 only)
================================

On E8 HP, use UART4 for console (see the Overview note).

Both HE and HP require a first-stage loader in MRAM to power up SRAM0
before the Zephyr image runs.  The loader placement is user-defined; the
``aipm_off`` ``vtor-address`` must be set to the address where the loader
is placed so the SE restores execution there on wakeup.  The Zephyr
application uses RTC as the idle timer; ensure no other core is using RTC
when this application runs.

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/spi_dw
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_hp
   :goals: build
   :west-args: -p auto
   :snippets: spi-dw-pm-s2ram-sram0

Flash the binary using SE Tools. See :ref:`programming_an_application` for
details.

Sample Output
*************

HE Core — TCM boot (S2RAM STANDBY → STOP)
==========================================

The output below is from an E7 DK HE core TCM-boot run (``APP_PM_WAKEUP_DEBUG 0``,
the default). Setting ``APP_PM_WAKEUP_DEBUG 1`` in ``main.c`` additionally prints
``PM wakeup: NVIC ISPR[x] = 0x...`` lines on each resume.

.. code-block:: console

   *** Booting Zephyr OS build v4.1.0-607-g7fafce9cf3ee ***
   [00:00:00.000,000] <inf> pm_spi_dw: alif_e7_dk (S2RAM): SPI DW PM demo (RUNTIME_IDLE, SUSPEND_TO_IDLE, S2RAM STANDBY, S2RAM STOP)
   [00:00:00.000,000] <inf> pm_spi_dw: POWER STATE SEQUENCE:
   [00:00:00.000,000] <inf> pm_spi_dw:   1. PM_STATE_RUNTIME_IDLE
   [00:00:00.000,000] <inf> pm_spi_dw:   2. PM_STATE_SUSPEND_TO_IDLE
   [00:00:00.000,000] <inf> pm_spi_dw:   3. PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY)
   [00:00:00.000,000] <inf> pm_spi_dw:   4. PM_STATE_SUSPEND_TO_RAM (substate 1: STOP)
   [00:00:00.000,000] <inf> pm_spi_dw: [SPI Demo] before RUNTIME_IDLE
   [00:00:00.000,000] <inf> pm_spi_dw: Slave Transceive Iter= 1
   [00:00:00.101,000] <inf> pm_spi_dw: Master Transceive Iter= 1
   [00:00:00.107,000] <inf> pm_spi_dw: Master wrote:   a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
   [00:00:00.107,000] <inf> pm_spi_dw: Master receive: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
   [00:00:00.107,000] <inf> pm_spi_dw: SUCCESS: SPI Master RX & Slave TX DATA IS MATCHING
   [00:00:00.107,000] <inf> pm_spi_dw: slave wrote: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
   [00:00:00.107,000] <inf> pm_spi_dw: slave read:  a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
   [00:00:00.107,000] <inf> pm_spi_dw: SUCCESS: SPI Master TX & Slave RX DATA IS MATCHING
   ... (iterations 2-10, one per second) ...
   [00:00:09.170,000] <inf> pm_spi_dw: Slave Transfer Successfully Completed
   [00:00:10.171,000] <inf> pm_spi_dw: Master Transfer Successfully Completed
   [00:00:10.171,000] <inf> pm_spi_dw: Enter RUNTIME_IDLE sleep for (18000000 microseconds)
   [00:00:28.172,000] <inf> pm_spi_dw: Exited from RUNTIME_IDLE sleep
   [00:00:28.172,000] <inf> pm_spi_dw: Request SUSPEND_TO_IDLE for 10000 us
   [00:00:28.173,000] <inf> pm_spi_dw: PM enter: SUSPEND_TO_IDLE (substate 0)
   [00:00:28.173,000] <inf> pm_spi_dw: PM wakeup: SUSPEND_TO_IDLE (substate 0)
   [00:00:28.173,000] <inf> pm_spi_dw: PM exit:  SUSPEND_TO_IDLE (substate 0)
   [00:00:28.183,000] <inf> pm_spi_dw: [SPI Demo] after SUSPEND_TO_IDLE
   ... (10 transfer iterations, one per second) ...
   [00:00:38.354,000] <inf> pm_spi_dw: Master Transfer Successfully Completed
   [00:00:38.354,000] <inf> pm_spi_dw: Request S2RAM STANDBY for 6000000 us
   [00:00:38.452,000] <inf> pm_spi_dw: PM enter: SUSPEND_TO_RAM (substate 0)
   [00:00:38.452,000] <inf> pm_spi_dw: PM wakeup: SUSPEND_TO_RAM (substate 0)
   [00:00:38.452,000] <inf> pm_spi_dw: PM exit:  SUSPEND_TO_RAM (substate 0)
   [00:00:44.405,000] <inf> pm_spi_dw: [SPI Demo] after S2RAM STANDBY
   ... (10 transfer iterations, one per second) ...
   [00:00:54.576,000] <inf> pm_spi_dw: Master Transfer Successfully Completed
   [00:00:54.576,000] <inf> pm_spi_dw: Request S2RAM STOP for 9000000 us
   [00:00:54.674,000] <inf> pm_spi_dw: PM enter: SUSPEND_TO_RAM (substate 1)
   [00:00:54.674,000] <inf> pm_spi_dw: PM wakeup: SUSPEND_TO_RAM (substate 1)
   [00:00:54.674,000] <inf> pm_spi_dw: PM exit:  SUSPEND_TO_RAM (substate 1)
   [00:01:03.622,000] <inf> pm_spi_dw: [SPI Demo] after S2RAM STOP
   ... (10 transfer iterations, one per second) ...
   [00:01:13.793,000] <inf> pm_spi_dw: Master Transfer Successfully Completed
   [00:01:13.793,000] <inf> pm_spi_dw: === SPI DW PM SEQUENCE COMPLETED ===

HP Core — MRAM boot (SOFT_OFF)
===============================

On HP MRAM the LPM idle timer is not configured, so ``SUSPEND_TO_IDLE`` is
skipped. After SOFT_OFF the system resets and restarts from ``main()``.

.. code-block:: console

   *** Booting Zephyr OS build v4.1.0 ***
   [00:00:00.005,000] <inf> pm_spi_dw: alif_e7_dk (SOFT_OFF): SPI DW PM demo (RUNTIME_IDLE, SUSPEND_TO_IDLE, SOFT_OFF)
   [00:00:00.017,000] <inf> pm_spi_dw: POWER STATE SEQUENCE:
   [00:00:00.023,000] <inf> pm_spi_dw:   1. PM_STATE_RUNTIME_IDLE
   [00:00:00.030,000] <inf> pm_spi_dw:   2. PM_STATE_SUSPEND_TO_IDLE
   [00:00:00.037,000] <inf> pm_spi_dw:   3. PM_STATE_SOFT_OFF
   [00:00:00.043,000] <inf> pm_spi_dw: [SPI Demo] before RUNTIME_IDLE
   ... (10 transfer iterations, one per second) ...
   [00:00:10.171,000] <inf> pm_spi_dw: Master Transfer Successfully Completed
   [00:00:10.171,000] <inf> pm_spi_dw: Enter RUNTIME_IDLE sleep for (18000000 microseconds)
   [00:00:28.172,000] <inf> pm_spi_dw: Exited from RUNTIME_IDLE sleep
   [00:00:28.172,000] <inf> pm_spi_dw: PM_STATE_SUSPEND_TO_IDLE (skipped - LPM timer not enabled)
   [00:00:28.172,000] <inf> pm_spi_dw: Request SOFT_OFF for 10000000 us (no retention - system resets on wakeup)

   <-- System resets after 10 seconds -->

   *** Booting Zephyr OS build v4.1.0 ***
   [00:00:00.005,000] <inf> pm_spi_dw: alif_e7_dk (SOFT_OFF): SPI DW PM demo (RUNTIME_IDLE, SUSPEND_TO_IDLE, SOFT_OFF)
   [Cycle repeats...]

Notes
*****

* **Debugger**: Disconnect the debugger before testing — it prevents cores
  from entering OFF states.
* **UART Hub**: If using a USB hub for UART, set ``CONFIG_BOOT_DELAY`` to
  avoid missing logs after a power cycle.
* **E8 HP console**: UART2 clashes with SPI0 on P1_0 / P1_1. The E8 HP
  ``spi-dw-pm-mram`` and ``spi-dw-pm-s2ram-sram0`` overlays select UART4.
  Use the UART4 port on the hub. E7 builds keep UART2.
* **Sleep durations** (all below the 10.7 s uint32 overflow ceiling at
  400 MHz):

  * RUNTIME_IDLE: 18 s (WFI, not subject to overflow)
  * SUSPEND_TO_IDLE: 10 ms
  * S2RAM STANDBY: 6 s
  * S2RAM STOP: 9 s
  * SOFT_OFF: 10 s

* **SUSPEND_TO_IDLE**:

  * Requires ``CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER`` (LPM timer).
  * HE core: enabled by default with RTC as LPM timer.
  * HP core: skipped by default (enabled when ``-S spi-dw-pm-s2ram-sram0``
    is used, with RTC as idle timer).
  * Uses IWIC (Internal WIC) — lighter than EWIC.
  * Only interrupts 0–63 can wake from IWIC mode.
  * Devices remain active (no suspend/resume overhead).

* **SPI DMA**: ``spi_loopback_run()`` locks every state deeper than
  ``RUNTIME_IDLE`` for the duration of the transfer. SPI and DMA IRQs are
  outside the IWIC range, so an accidental ``SUSPEND_TO_IDLE`` would stall
  the transfer.
* **SRAM0 S2RAM — E8 only**: SRAM0 retention masks
  (``ALIF_SRAM0_*_RET_MASK``) are defined only in the Ensemble Gen2
  DT-bindings header.  Using ``-S spi-dw-pm-s2ram-sram0`` on E7 or
  earlier will fail with a DTS compile error.
* **SRAM0 S2RAM — RTC as idle timer**: This application configures RTC as
  the idle timer (``zephyr,cortex-m-idle-timer``).  Ensure no other core
  is using RTC while this application runs.
* **Power measurement**: Disable all unused peripherals for accurate numbers.

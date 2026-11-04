.. SPDX-License-Identifier: Apache-2.0

.. _spi_pm_test:

Alif SPI Power Management Test
##############################

Overview
********

This ztest application validates Zephyr power management (PM) state transitions
on Alif RTSS cores. An SPI DesignWare DMA controller/peripheral loopback runs
before and after each resumable sleep; SOFT_OFF only runs it before sleep because
wakeup resets the core. Each resumable case checks two things:

* transferred data matches in both directions;
* the PM notifier saw the expected state (and no deeper state) being entered.

The test creates two threads that exchange data over an on-board SPI loopback
(one instance as controller, the other as peripheral):

* **Controller thread**: waits on its start semaphore, then performs SPI
  transceives.
* **Peripheral thread**: waits on its start semaphore, then performs the
  matching SPI transceives.

``spi_loopback_run()`` arms the peripheral, waits briefly, then starts the
controller. The threads block on those semaphores between rounds. They are
not suspended with ``k_thread_suspend()``. During each transfer,
``SUSPEND_TO_IDLE``, ``SUSPEND_TO_RAM`` and ``SOFT_OFF`` are locked because
SPI/DMA interrupts are not wakeup sources. If a done-semaphore times out,
those locks stay held and later SPI rounds are skipped.

PM states under test:

* **PM_STATE_RUNTIME_IDLE**: Light sleep with quick wakeup
* **PM_STATE_SUSPEND_TO_IDLE**: Light sleep, no retention requirements
* **PM_STATE_SUSPEND_TO_RAM (S2RAM)**: Deep sleep with retention

  * Substate 0 (STANDBY): Medium power savings
  * Substate 1 (STOP): Higher power savings

* **PM_STATE_SOFT_OFF**: Deepest sleep, no retention, full reset on wakeup

Which cases run depends on the snippet and boot mode:

* **S2RAM TCM** (``spi-dw-pm-s2ram-tcm``, HE only): RUNTIME_IDLE,
  SUSPEND_TO_IDLE, S2RAM STANDBY, S2RAM STOP. SOFT_OFF is skipped.
* **MRAM** (``spi-dw-pm-mram``, HE or HP): RUNTIME_IDLE, SUSPEND_TO_IDLE (when
  enabled in the overlay), SOFT_OFF. S2RAM suites are skipped
  (``ztest_test_skip``). SOFT_OFF resets the core, so the run restarts from
  ``main()``.

SUSPEND_TO_IDLE is also skipped when the LPM idle timer is not enabled or the
``suspend_idle`` node is disabled in the overlay.

Test Suites
***********

The application registers the following ztest suites (see ``src/main.c``):

* ``pm_POS_01_Runtime_idle_testing_for_SPI`` - RUNTIME_IDLE basic, repeated, min/max sleep
* ``pm_POS_02_Suspend_to_idle_testing_for_SPI`` - SUSPEND_TO_IDLE basic, repeated, min/max, and RUNTIME_IDLE then SUSPEND_TO_IDLE
* ``pm_POS_04_STANDBY_testing_for_SPI`` - S2RAM STANDBY basic, repeated, min/max, RUNTIME_IDLE then STANDBY, and SOFT_OFF blocked during STANDBY
* ``pm_POS_05_STOP_testing_for_SPI`` - S2RAM STOP basic, repeated, min/max, sequences, SOFT_OFF blocked during STOP, and below-STOP residency (policy selects STANDBY)
* ``pm_POS_03_SOFT_OFF__testing_for_SPI`` - SOFT_OFF basic (MRAM builds; skipped when retained RAM is available)

ztest runs suites in alphabetical order (``01``, ``02``, ``03`` SOFT_OFF,
``04``, ``05``). Before and after each case the fixtures restore the PM lock
baseline so every test starts from a known state.

Requirements
************

* Alif Ensemble or Balletto development board
* Two SPI instances wired for loopback as controller and peripheral
  (``controller-spi`` / ``peripheral-spi`` aliases in the snippet overlays)
* RTC (or timer) enabled for wakeup
* SE Services for power profile configuration

Supported Boards
****************

==========  =================  ======================  =================================
Board/core  Controller         Peripheral              Snippets
==========  =================  ======================  =================================
E7 HE       SPI4               SPI0                    ``spi-dw-pm-s2ram-tcm``, ``spi-dw-pm-mram``
E8 HE       SPI4               SPI1                    ``spi-dw-pm-s2ram-tcm``, ``spi-dw-pm-mram``
E7 HP       SPI1               SPI0                    ``spi-dw-pm-mram``
E8 HP       SPI1               SPI0                    ``spi-dw-pm-mram``
E1C HE      LPSPI0             SPI0                    ``spi-dw-pm-s2ram-tcm``, ``spi-dw-pm-mram``
B1 HE       LPSPI0             SPI1                    ``spi-dw-pm-s2ram-tcm``, ``spi-dw-pm-mram``
==========  =================  ======================  =================================

Connect the controller and peripheral SCLK, SDO, SDI and CS pins together.
E8 HP MRAM builds move the console to UART4 because UART2 shares pins
with SPI0.

Building and Running
********************

Board-specific SPI and PM configuration comes from the snippets under
``snippets/``, selected with ``-S``.

HE TCM (S2RAM):

.. code-block:: console

   west build -- \
       ../alif/tests/drivers/pm/spi \
       -S spi-dw-pm-s2ram-tcm

HE / HP MRAM (SOFT_OFF):

.. code-block:: console

   west build -- \
       ../alif/tests/drivers/pm/spi \
       -S spi-dw-pm-mram

Flash the binary using SE Tools. See :ref:`programming_an_application` for details.

Test Output
***********

.. code-block:: console

   *** Booting Zephyr OS build v4.1.0 ***
   [00:00:00.004,000] <inf> spi_pm: alif_e7_dk (S2RAM): SPI DW PM test
   [00:00:00.005,000] <inf> spi_pm: RUNTIME_IDLE, S2I, S2RAM STANDBY, S2RAM STOP
   [00:00:00.015,000] <inf> spi_pm: Boot: TCM, VTOR 0x00000000
   [00:00:00.020,000] <inf> spi_pm: SPI threads started (controller SPI_4, peripheral SPI_0)
   RUNNING TEST SUITE pm_POS_01_Runtime_idle_testing_for_SPI
   ===================================================================
   START - test_SPI_POS_01_runtime_idle_basic_test
   [00:00:00.030,000] <inf> spi_pm: === TEST 1: Basic RUNTIME_IDLE ===
   [00:00:00.040,000] <inf> spi_pm: [SPI Test] before RUNTIME_IDLE
   [00:00:01.050,000] <inf> spi_pm: [SPI Test] before RUNTIME_IDLE: PASS
   [00:00:01.060,000] <inf> spi_pm: Enter RUNTIME_IDLE sleep for (10000000 microseconds)
   [00:00:11.070,000] <inf> spi_pm: Exited from RUNTIME_IDLE sleep
   [00:00:11.080,000] <inf> spi_pm: [SPI Test] after RUNTIME_IDLE
    PASS - test_SPI_POS_01_runtime_idle_basic_test in 17.100 seconds
   ...

Notes
*****

* **Debugger**: Disconnect the debugger before testing - it prevents cores from entering OFF states
* **SPI Loopback**: The controller and peripheral SPI instances must be wired for
  loopback as configured by the snippet overlays
* **CONFIG_ZTEST**: Test suites run automatically and log PASS/FAIL/SKIP per case
* **Retention Memory**: HE TCM retains TCM/SERAM
* **Power Measurement**: Disable unused peripherals for accurate measurements
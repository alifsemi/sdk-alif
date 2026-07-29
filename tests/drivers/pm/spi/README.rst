.. _alif-spi-pm-testcode-sample:

Alif SPI Power Management Test
###############################

Overview
********

This sample validates Zephyr power management (PM) state transitions on Alif RTSS
cores while an SPI controller/peripheral loopback transfer runs concurrently in the
background. It uses the Zephyr ztest framework to exercise each PM state as an
individual, automatically-run test case, and confirms that ongoing SPI transactions
are correctly suspended before a low-power state is entered and correctly resumed
afterwards.

The sample sets up two threads that continuously exchange data over an on-board SPI
loopback (one SPI instance acting as controller, another as peripheral):

* **Controller thread**: waits for the peripheral, then performs an SPI transceive.
* **Peripheral thread**: signals the controller and performs the matching SPI transceive.

Before entering any sleep/PM state, the SPI threads are suspended
(``spi_pm_thread_suspend``); after waking up they are resumed
(``spi_pm_thread_resume``). Received data is compared against the expected pattern to
detect any corruption caused by the power state transition.

PM states under test:

* **PM_STATE_RUNTIME_IDLE**: Light sleep state with quick wakeup
* **PM_STATE_SUSPEND_TO_IDLE**: Light sleep, no retention requirements
* **PM_STATE_SUSPEND_TO_RAM (S2RAM)**: Deep sleep with retention (HE core only)

  * Substate 0 (STANDBY): Medium power savings
  * Substate 1 (STOP): Higher power savings

* **PM_STATE_SOFT_OFF**: Deepest sleep, no retention, full system reset on wakeup

The behavior differs between HP and HE cores:

**HE Core (with retention support)**:

* When booting from **TCM** (VTOR = 0x0):

  * Runs RUNTIME_IDLE, SUSPEND_TO_IDLE, S2RAM STANDBY, S2RAM STOP test suites
  * Skips SOFT_OFF (uses retention instead)

* When booting from **MRAM** (VTOR >= 0x80000000):

  * Runs RUNTIME_IDLE, SUSPEND_TO_IDLE, SOFT_OFF test suites
  * S2RAM tests are skipped (``ztest_test_skip``)
  * System resets and restarts from ``main()`` after SOFT_OFF

**HP Core (no retention support)**:

* S2RAM test suites are skipped (no retention capability)
* Runs RUNTIME_IDLE, SUSPEND_TO_IDLE, SOFT_OFF test suites
* System resets and restarts from ``main()`` after SOFT_OFF

Test Suites
***********

The application registers the following ztest suites (see ``src/main.c``):

* ``pm_POS_01_Runtime_idle_testing_for_SPI`` - RUNTIME_IDLE basic, repeated, min/max
  sleep duration tests
* ``pm_POS_02_Suspend_to_idle_testing_for_SPI`` - SUSPEND_TO_IDLE basic, repeated,
  min/max duration, and combined RUNTIME_IDLE + SUSPEND_TO_IDLE sequence tests
* ``pm_POS_04_STANDBY_testing_for_SPI`` - S2RAM STANDBY basic, repeated, min/max
  sleep, and "no SOFT_OFF while in STANDBY" tests (HE/TCM boot only)
* ``pm_POS_05_STOP_testing_for_SPI`` - S2RAM STOP basic test (HE/TCM boot only)
* ``pm_POS_03_SOFT_OFF__testing_for_SPI`` - SOFT_OFF test (HP core, or HE/MRAM boot)

Before/after each test case, the SPI threads are resumed and re-suspended
respectively so every test starts and ends from a known state.

Requirements
************

* Alif Ensemble or Balletto development board
* Two SPI instances wired/loopback-configured as controller and peripheral
  (``controller-spi`` / ``peripheral-spi`` aliases, see the snippet overlay files)
* RTC (or timer) peripheral enabled for wakeup
* SE Services for power profile configuration

Supported Boards
****************

Board overlays are provided via the ``snippets/`` directory and are selected
automatically based on board name:

* ``alif_e7_dk_rtss_he`` / ``alif_e8_dk_rtss_he`` - ``-S spi-pm-he``
  (``snippets/spi-pm-he/spi_pm_he_ensemble.overlay``)
* ``alif_b1_dk_rtss_he`` (Balletto) - ``-S spi-pm-he``
  (``snippets/spi-pm-he/spi_pm_he_balletto.overlay``)
* ``alif_e7_dk_rtss_hp`` / ``alif_e8_dk_rtss_hp`` - ``-S spi-pm-hp``
  (``snippets/spi-pm-hp/spi_pm_hp_ensemble.overlay``)

Building and Running
********************

Board-specific SPI pin/interrupt configuration is supplied via the ``spi-pm-he``
and ``spi-pm-hp`` snippets (see ``snippets/``), selected on the command line with
``-S``. The snippet automatically picks the correct overlay for the target board
(Ensemble E7/E8 or Balletto B1).

Build for HE core:

.. code-block:: console

   west build -- \
       ../alif/tests/drivers/pm/spi \
       -S spi-pm-he

Build for HP core:

.. code-block:: console

   west build -- \
       ../alif/tests/drivers/pm/spi \
       -S spi-pm-hp

Flash the binary using SE Tools. See :ref:`programming_an_application` for details.

Sample Output
*************

.. code-block:: console

   *** Booting Zephyr OS build v4.1.0 ***
   [00:00:00.004,000] <inf> spi_pm:
   alif_e7_dk RTSS_HE (TCM boot): PM states demo (RUNTIME_IDLE, S2RAM)
   [00:00:00.015,000] <inf> spi_pm: RTSS_HE: PM states demo WITH SPI Transaction
   [00:00:00.020,000] <inf> spi_pm: SPI threads started
   RUNNING TEST SUITE pm_POS_01_Runtime_idle_testing_for_SPI
   ===================================================================
   START - test_SPI_POS_01_runtime_idle_basic_test
   [00:00:00.030,000] <inf> spi_pm: === TEST 1: Basic RUNTIME_IDLE ===
   [00:00:05.040,000] <inf> spi_pm: Enter RUNTIME_IDLE sleep for (10000000 microseconds)
   [00:00:15.050,000] <inf> spi_pm: Exited from RUNTIME_IDLE sleep
   [00:00:15.060,000] <inf> spi_pm:  ====== SPI Transactions are starting...
    PASS - test_SPI_POS_01_runtime_idle_basic_test in 17.100 seconds
   ...

Notes
*****

* **Debugger**: Disconnect debugger before testing - it prevents cores from entering
  OFF states
* **SPI Loopback**: The controller and peripheral SPI instances must be wired for loopback
  (or use an SoC-internal loopback) as configured by the board overlays
* **CONFIG_ZTEST**: This application is built as a ztest binary; test suites run
  automatically and log ``PASS``/``FAIL``/``SKIP`` per test case
* **Retention Memory**: HE core retains SERAM and optionally TCM (when booting from
  TCM)
* **Power Measurement**: For accurate power measurements, ensure all unused
  peripherals are disabled

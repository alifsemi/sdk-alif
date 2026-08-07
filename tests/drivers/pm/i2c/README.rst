.. _alif-i2c-pm-test-code:

I2C Power Management Test Application
#######################################

Overview
********

This Zephyr test application exercises Alif RTSS power management states while
performing I2C transfers before and after each sleep/resume cycle. The tests use
the standard Zephyr ``ztest`` framework and cover:

* **RUNTIME_IDLE** basic, multiple cycles, maximum and minimum duration tests
* **SUSPEND_TO_IDLE** basic, multiple cycles, maximum and minimum duration tests
* **SOFT_OFF** basic test (HP/MRAM boot only; reset on wakeup)
* **S2RAM STANDBY** basic, multiple cycles, maximum and minimum duration tests
* **S2RAM STOP** basic, multiple cycles, maximum and minimum duration tests

For each test, the application performs an I2C write-read transfer before
entering the target power state and again after resuming, verifying that the I2C
peripheral remains functional across PM state transitions.

Requirements
************

* Alif Ensemble or Balletto development board
* I2C peripheral enabled via the ``alif-i2c-pm`` snippet
* An I2C target device at address ``0x76`` with a readable register at ``0xD0``
  (default BME280 chip ID register)
* RTC0 or Timer0 enabled as the low-power wakeup/sleep timer
* SE Services for power profile configuration

Supported Boards
****************

* alif_e7_dk_rtss_he
* alif_e1c_dk_rtss_he
* alif_b1_dk_rtss_he
* alif_e8_dk_rtss_he
* alif_e7_dk_rtss_hp
* alif_e8_dk_rtss_hp

Building and Running
********************

Build for HE core (TCM boot with retention, Ensemble DK, ``i2c0``):

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
       ../alif/tests/drivers/pm/i2c \
       -S alif-i2c-pm \
       -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
       -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
       -DCONFIG_FLASH_SIZE=256

Build for HE core (MRAM boot, Ensemble DK, ``i2c0``):

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
       ../alif/tests/drivers/pm/i2c \
       -S alif-i2c-pm

Build for HP core (Ensemble DK, ``i2c0``):

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
       ../alif/tests/drivers/pm/i2c \
       -S alif-i2c-pm

For E1C/B1 DK (uses ``i2c1`` by default):

.. code-block:: console

   west build -p auto -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
       ../alif/tests/drivers/pm/i2c \
       -S alif-i2c-pm

Flash the binary using SE Tools and connect the target I2C device to the board.

Sample Output
*************

HE Core - TCM boot (excerpt of ``ztest`` output)
==================================================

.. code-block:: console

   *** Booting Zephyr OS build v4.1.0-415-g8a0d36191e14 ***
   Running TESTSUITE i2c_test_POS_01_RUNTIME_IDLE
   ===================================================================
   START - test_POS_01_RUNTIME_IDLE_basic
   [00:00:00.005,000] <inf> i2c_pm: alif_e7_dk RTSS_HE (TCM boot): I2C PM states demo (RUNTIME_IDLE, SUSPEND_TO_IDLE, S2RAM)
   [00:00:00.012,000] <inf> i2c_pm: --- I2C transfer before sleep ---
   [00:00:00.018,000] <inf> i2c_pm: I2C read reg 0xD0 = 0x60
   [00:00:00.025,000] <inf> i2c_pm: === TEST 1: Basic RUNTIME_IDLE ===
   [00:00:00.031,000] <inf> i2c_pm: Enter RUNTIME_IDLE sleep for (18000000 microseconds)
   [00:00:18.041,000] <inf> i2c_pm: Exited from RUNTIME_IDLE sleep
   [00:00:18.047,000] <inf> i2c_pm: --- I2C transfer after sleep ---
   [00:00:18.053,000] <inf> i2c_pm: I2C read reg 0xD0 = 0x60
   PASS - test_POS_01_RUNTIME_IDLE_basic in 18024 ms
   ===================================================================
   TESTSUITE i2c_test_POS_01_RUNTIME_IDLE succeeded
   ...
   Running TESTSUITE i2c_test_POS_04_S2RAM_STANDBY
   ===================================================================
   START - test_POS_01_S2RAM_STANDBY_basic
   [00:01:10.123,000] <inf> i2c_pm: --- I2C transfer before sleep ---
   [00:01:10.129,000] <inf> i2c_pm: I2C read reg 0xD0 = 0x60
   [00:01:10.135,000] <inf> i2c_pm: Enter PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) for (20000000 microseconds)
   [00:01:30.152,000] <inf> i2c_pm: Resumed from PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY)
   [00:01:30.159,000] <inf> i2c_pm: --- I2C transfer after sleep ---
   [00:01:30.165,000] <inf> i2c_pm: I2C read reg 0xD0 = 0x60
   PASS - test_POS_01_S2RAM_STANDBY_basic in 20021 ms
   ===================================================================
   TESTSUITE i2c_test_POS_04_S2RAM_STANDBY succeeded
   ...
   RunID: ...
   PROJECT EXECUTION SUCCESSFUL

Notes
*****

* **I2C target device**: The default code reads register ``0xD0`` from address
  ``0x76``. If no device is connected, the test logs a warning before sleep and
  an error after sleep, but the test itself continues.
* **I2C instance**: The snippet selects ``i2c0`` for Ensemble and ``i2c1`` for
  E1C/B1 via the ``i2c-instance`` devicetree alias. Edit the overlay if a
  different I2C controller is required.
* **Sleep durations**: Test defines values such as 18 s for RUNTIME_IDLE,
  1 ms to 18 ms for SUSPEND_TO_IDLE, 20 s for S2RAM STANDBY, and 22 s for
  S2RAM STOP.
* **HP core**: S2RAM tests are skipped because the HP core has no retention.
  SOFT_OFF tests reset the core on wakeup.
* **Debugger**: Disconnect the debugger when testing OFF states; it can
  prevent the core from entering the lowest power modes.
* **Power measurement**: Disable all unused peripherals in the devicetree for
  accurate current measurements.

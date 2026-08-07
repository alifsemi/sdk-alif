.. _alif-i2s-pm-test-code:

Alif I2S Power Management Test Application
############################################

Overview
********

This Zephyr ``ztest`` application verifies I2S audio streaming across Alif RTSS
power management states. It uses the I2S RX/TX loopback (or microphone-to-speaker)
path to exercise peripheral suspend and resume while the system enters and exits
low-power states.

The test suites cover:

* **RUNTIME_IDLE** basic, multiple cycles, maximum and minimum duration tests
* **SUSPEND_TO_IDLE** basic, multiple cycles, maximum and minimum duration tests
* **SOFT_OFF** basic test (HP/MRAM boot only; reset on wakeup)
* **S2RAM STANDBY** basic, multiple cycles, maximum and minimum duration tests
* **S2RAM STOP** basic, multiple cycles, maximum, minimum, and combined
  STANDBY+STOP tests

Before and after each sleep/resume cycle the application starts the I2S
peripheral, streams a short audio block, and stops the peripheral. This verifies
that the I2S device context is preserved across S2RAM states and that the
peripheral is correctly suspended/resumed.

Requirements
************

* Alif Ensemble or Balletto development board
* I2S RX/TX devices enabled via the appropriate ``devkit-he`` or ``devkit-hp``
  snippet (e.g., ``i2s3`` + ``i2s4`` on Ensemble HE, ``i2s1`` + ``i2s3`` on HP)
* RTC0 or Timer0 enabled as the low-power wakeup/sleep timer
* SE Services for power profile configuration
* For B1 DK: ``i2s0`` + ``i2s4`` is selected by the B1 overlay

Supported Boards
****************

* alif_e7_dk_rtss_he
* alif_e8_dk_rtss_he
* alif_e7_dk_rtss_hp
* alif_e8_dk_rtss_hp
* alif_b1_dk_rtss_he (via ``alif_b1_dk_rtss_he.overlay``)

Building and Running
**********************

Build for HE core (TCM boot with retention, Ensemble DK):

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
       ../alif/tests/drivers/pm/i2s \
       -S devkit-he \
       -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
       -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
       -DCONFIG_FLASH_SIZE=256

Build for HE core (MRAM boot, Ensemble DK):

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
       ../alif/tests/drivers/pm/i2s \
       -S devkit-he

Build for HP core (Ensemble DK):

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
       ../alif/tests/drivers/pm/i2s \
       -S devkit-hp

Build for B1 DK HE core:

.. code-block:: console

   west build -p auto -b alif_b1_dk/ae1c1f4051920hh/rtss_he \
       ../alif/tests/drivers/pm/i2s \
       -S devkit-he

Flash the binary using SE Tools. See :ref:`programming_an_application` for details.

Sample Output
*************

Excerpt of console output (HE TCM boot):

.. code-block:: console

   *** Booting Zephyr OS build v4.1.0-415-g8a0d36191e14 ***
   Running TESTSUITE i2s_test_POS_01_RUNTIME_IDLE
   ===================================================================
   START - test_POS_01_RUNTIME_IDLE_basic
   [00:00:00.005,000] <inf> i2s_pm: alif_e7_dk RTSS_HE (TCM boot): I2S PM demo (RUNTIME_IDLE, SUSPEND_TO_IDLE, S2RAM)
   [00:00:00.012,000] <inf> i2s_pm: --- I2S stream before sleep ---
   [00:00:00.025,000] <inf> i2s_pm: === TEST 1: Basic RUNTIME_IDLE ===
   [00:00:00.031,000] <inf> i2s_pm: Enter RUNTIME_IDLE sleep for (18000000 microseconds)
   [00:00:18.041,000] <inf> i2s_pm: Exited from RUNTIME_IDLE sleep
   [00:00:18.047,000] <inf> i2s_pm: --- I2S stream after sleep ---
   PASS - test_POS_01_RUNTIME_IDLE_basic in 18023 ms
   ===================================================================
   TESTSUITE i2s_test_POS_01_RUNTIME_IDLE succeeded
   ...
   Running TESTSUITE i2s_test_POS_04_S2RAM_STANDBY
   ===================================================================
   START - test_POS_01_S2RAM_STANDBY_basic
   [00:01:10.123,000] <inf> i2s_pm: --- I2S stream before sleep ---
   [00:01:10.135,000] <inf> i2s_pm: Enter PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) for (20000000 microseconds)
   [00:01:30.152,000] <inf> i2s_pm: Resumed from PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY)
   [00:01:30.159,000] <inf> i2s_pm: --- I2S stream after sleep ---
   PASS - test_POS_01_S2RAM_STANDBY_basic in 20021 ms
   ===================================================================
   TESTSUITE i2s_test_POS_04_S2RAM_STANDBY succeeded
   ...
   RunID: ...
   PROJECT EXECUTION SUCCESSFUL

Notes
*****

* **I2S hardware**: Ensure the correct I2S TX/RX pins are connected for loopback
  or for microphone + speaker operation on the target board.
* **Snippets**:

  * ``devkit-he`` enables ``i2s3`` + ``i2s4`` for Ensemble E7/E8 and adds
    ``prj.conf`` with ``CONFIG_PM_S2RAM=y``.
  * ``devkit-he`` also selects ``i2s0`` + ``i2s4`` for B1 DK via
    ``alif_b1_dk_rtss_he.overlay``.
  * ``devkit-hp`` enables ``i2s1`` + ``i2s3`` for HP cores.

* **I2S stream handling**: ``before()`` starts and runs the I2S stream;
  ``after()`` restarts it after sleep. ``setup()`` initializes the I2S and
  wakeup counter devices.
* **SUSPEND_TO_IDLE lock**: The ``after()`` fixture temporarily locks
  ``SUSPEND_TO_IDLE`` around the ``i2s_stop()`` delay so that the cleanup
  ``k_sleep(100ms)`` does not accidentally enter an unrecoverable low-power mode.
* **HP core**: S2RAM tests are skipped because the HP core has no retention.
* **Debugger**: Disconnect the debugger when testing OFF states; it can prevent
  the core from entering the lowest power modes.

.. _alif-i2s-pm-test:

Alif I2S Power Management Test
##############################

Overview
********

This Zephyr ``ztest`` verifies I2S streaming across Alif RTSS power management
states. It uses the I2S RX/TX path (loopback or microphone to speaker) to
exercise peripheral suspend and resume while the system enters and exits
low-power states.

The I2S instance that drives the bit clock and frame clock is the **controller**
(Zephyr ``I2S_OPT_BIT_CLK_MASTER`` / ``I2S_OPT_FRAME_CLK_MASTER``). The RX
instance is the **target**. This wording follows Zephyr coding guideline
Rule A.2 (inclusive language / OSHWA terms).

The test suites cover:

* **RUNTIME_IDLE** basic, multiple cycles, maximum and minimum duration tests
* **SUSPEND_TO_IDLE** basic, multiple cycles, maximum and minimum duration tests
* **SOFT_OFF** basic test (HP or HE MRAM boot only; reset on wakeup)
* **S2RAM STANDBY** basic, multiple cycles, maximum and minimum duration tests
* **S2RAM STOP** basic, multiple cycles, maximum, minimum, and combined
  STANDBY+STOP tests

Before and after each sleep/resume cycle the test starts the I2S
peripheral, streams a short audio block, and stops the peripheral. This verifies
that the I2S device context is preserved across S2RAM states and that the
peripheral is correctly suspended and resumed.

Requirements
************

* Alif Ensemble or Balletto development board
* I2S RX/TX devices enabled via the ``i2s-pm-he`` or ``i2s-pm-hp`` snippet
  (``i2s3`` + ``i2s4`` on Ensemble HE, ``i2s1`` + ``i2s3`` on E7 HP only)
* CPU PM states, wakeup timer, and SE off profiles via
  ``pm-system-off-he`` or ``pm-system-off-hp``
* RTC0 or Timer0 enabled as the low-power wakeup/sleep timer
* SE Services for power profile configuration
* For B1 DK: ``i2s0`` + ``i2s4`` is selected by ``i2s_pm_he_balletto.overlay``

Supported Boards
****************

* ``alif_e7_dk_rtss_he`` / ``alif_e8_dk_rtss_he`` - ``-S i2s-pm-he``
  (``snippets/i2s-pm-he/i2s_pm_he_ensemble.overlay``)
* ``alif_b1_dk_rtss_he`` (Balletto) - ``-S i2s-pm-he``
  (``snippets/i2s-pm-he/i2s_pm_he_balletto.overlay``)
* ``alif_e7_dk_rtss_hp`` - ``-S i2s-pm-hp``
  (``snippets/i2s-pm-hp/i2s_pm_hp_ensemble.overlay``)

Building and Running
********************

Build for HE core:

.. code-block:: console

   west build -- \
       ../alif/tests/drivers/pm/i2s \
       -S i2s-pm-he -S pm-system-off-he

Build for HP core:

.. code-block:: console

   west build -- \
       ../alif/tests/drivers/pm/i2s \
       -S i2s-pm-hp -S pm-system-off-hp

Flash the binary using SE Tools. See :ref:`programming_an_application` for details.

Test Output
***********

Excerpt of console output (HE TCM boot):

.. code-block:: console

   *** Booting Zephyr OS build v4.1.0-415-g8a0d36191e14 ***
   Running TESTSUITE i2s_test_POS_01_RUNTIME_IDLE
   ===================================================================
   START - test_POS_01_RUNTIME_IDLE_basic
   [00:00:00.005,000] <inf> i2s_pm: alif_e7_dk RTSS_HE (TCM boot): I2S PM test (RUNTIME_IDLE, SUSPEND_TO_IDLE, S2RAM)
   [00:00:00.025,000] <inf> i2s_pm: === TEST 1: Basic RUNTIME_IDLE ===
   [00:00:00.031,000] <inf> i2s_pm: Enter RUNTIME_IDLE sleep for (18000000 microseconds)
   [00:00:18.041,000] <inf> i2s_pm: Exited from RUNTIME_IDLE sleep
   PASS - test_POS_01_RUNTIME_IDLE_basic in 18023 ms
   ===================================================================
   TESTSUITE i2s_test_POS_01_RUNTIME_IDLE succeeded
   ...
   Running TESTSUITE i2s_test_POS_04_S2RAM_STANDBY
   ===================================================================
   START - test_POS_01_S2RAM_STANDBY_basic
   [00:01:10.135,000] <inf> i2s_pm: Enter PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) for (20000000 microseconds)
   [00:01:30.152,000] <inf> i2s_pm: Resumed from PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY)
   PASS - test_POS_01_S2RAM_STANDBY_basic in 20021 ms
   ===================================================================
   TESTSUITE i2s_test_POS_04_S2RAM_STANDBY succeeded
   ...
   RunID: ...
   PROJECT EXECUTION SUCCESSFUL

Notes
*****

* **I2S hardware**: Connect the I2S TX/RX pins for loopback or for
  microphone and speaker operation on the target board.
* **Snippets**:

  * ``i2s-pm-he`` enables ``i2s3`` + ``i2s4`` for Ensemble E7/E8 and adds
    ``prj.conf`` with ``CONFIG_PM_S2RAM=y``.
  * ``i2s-pm-he`` selects ``i2s0`` + ``i2s4`` for B1 DK via
    ``i2s_pm_he_balletto.overlay``.
  * ``i2s-pm-hp`` enables ``i2s1`` + ``i2s3`` on E7 HP only.
      E8 HP is not supported: the E8 onboard mic is routed to
      LPI2S (``i2s4``), not ``i2s1``/``i2s3``.
  * ``pm-system-off-he`` / ``pm-system-off-hp`` enable SOFT_OFF, S2RAM,
      SUSPEND_TO_IDLE, the low-power wakeup timer, and SE off profiles.

* **I2S stream handling**: ``before()`` starts and runs the I2S stream;
  ``after()`` restarts it after sleep. ``setup()`` initializes the I2S and
  wakeup counter devices.
* **SUSPEND_TO_IDLE lock**: The ``after()`` fixture temporarily locks
  ``SUSPEND_TO_IDLE`` around the ``i2s_stop()`` delay so that the cleanup
  ``k_sleep(100ms)`` does not accidentally enter an unrecoverable low-power mode.
* **HP core**: S2RAM tests are skipped because the HP core has no retention.
* **Debugger**: Disconnect the debugger when testing OFF states; it can prevent
  the core from entering the lowest power modes.

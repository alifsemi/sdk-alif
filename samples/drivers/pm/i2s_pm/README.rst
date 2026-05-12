.. _alif-i2s-pm-sample:

Alif I2S Power Management Demo
################################

Overview
********

This sample demonstrates I2S audio streaming combined with Zephyr power management
states on Alif RTSS cores. It shows how to properly stop, suspend, resume, and
restart I2S streaming across different PM state transitions.

The sample performs:

1. **Initial I2S streaming** (Phase 1): Verifies basic I2S RX/TX echo operation
2. **PM state transitions**: Exercises power states with I2S stop/restart around each
3. **I2S restart after each PM state**: Validates that I2S resumes correctly

Power states demonstrated:

* **PM_STATE_RUNTIME_IDLE**: Light sleep (CPU clock gating via WFI)
* **PM_STATE_SUSPEND_TO_IDLE**: CPU sleep with IWIC, devices remain active
* **PM_STATE_SUSPEND_TO_RAM (S2RAM)**: Deep sleep with retention (HE core, TCM boot only)

  * Substate 0 (STANDBY): Medium power savings
  * Substate 1 (STOP): Higher power savings

* **PM_STATE_SOFT_OFF**: Deepest sleep, no retention, full system reset on wakeup

The behavior differs between HP and HE cores:

**HE Core (with retention support)**:

* When booting from **TCM** (VTOR = 0x0):

  * Streams I2S, then exercises RUNTIME_IDLE, SUSPEND_TO_IDLE, S2RAM STANDBY, S2RAM STOP
  * Restarts I2S after each PM state to verify peripheral recovery
  * Skips SOFT_OFF (uses retention instead)

* When booting from **MRAM** (VTOR >= 0x80000000):

  * Streams I2S, then exercises RUNTIME_IDLE, SUSPEND_TO_IDLE, SOFT_OFF
  * Restarts I2S after light sleep and before SOFT_OFF
  * System resets and restarts from main() after SOFT_OFF

**HP Core (no retention support)**:

* Streams I2S, then exercises RUNTIME_IDLE and SOFT_OFF
* Restarts I2S after RUNTIME_IDLE
* System resets and restarts from main() after SOFT_OFF

Requirements
************

* Alif Ensemble development board with I2S codec connected
* RTC peripheral enabled for wakeup
* SE Services for power profile configuration

Supported Boards
****************

* alif_e7_dk_rtss_he
* alif_e8_dk_rtss_he
* alif_e7_dk_rtss_hp
* alif_e8_dk_rtss_hp

Building and Running
********************

Build for alif_e7_dk
=====================

HE core (TCM boot with retention):

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
       ../alif/samples/drivers/pm/i2s_pm/ \
       -S devkit-he \
       -DCONFIG_FLASH_BASE_ADDRESS=0 \
       -DCONFIG_FLASH_LOAD_OFFSET=0 \
       -DCONFIG_FLASH_SIZE=256

HE core (MRAM boot):

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
       ../alif/samples/drivers/pm/i2s_pm/ \
       -S devkit-he

HP core (MRAM boot):

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
       ../alif/samples/drivers/pm/i2s_pm/ \
       -S devkit-hp

Build for alif_e8_dk
=====================

HE core (TCM boot with retention):

.. code-block:: console

   west build -p auto -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
       ../alif/samples/drivers/pm/i2s_pm/ \
       -S devkit-he \
       -DCONFIG_FLASH_BASE_ADDRESS=0 \
       -DCONFIG_FLASH_LOAD_OFFSET=0 \
       -DCONFIG_FLASH_SIZE=256

HE core (MRAM boot):

.. code-block:: console

   west build -p auto -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
       ../alif/samples/drivers/pm/i2s_pm/ \
       -S devkit-he

HP core (MRAM boot):

.. code-block:: console

   west build -p auto -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
       ../alif/samples/drivers/pm/i2s_pm/ \
       -S devkit-hp

Flash the binary using SE Tools. See :ref:`programming_an_application` for details.

Sample Output
*************

HE Core - TCM Boot (with retention, SOFT_OFF skipped)
======================================================

.. code-block:: console

   *** Booting Zephyr OS build 9bca98ae74dc ***
   [00:00:00.005,000] <inf> i2s_pm: alif_e8_dk RTSS_HE (TCM boot): I2S PM demo (RUNTIME_IDLE, SUSPEND_TO_I)
   [00:00:00.016,000] <inf> i2s_pm: === Phase 1: Initial I2S streaming ===
   [00:00:02.011,000] <inf> i2s_pm: stream loop exited
   [00:00:02.016,000] <inf> i2s_pm: i2s_stop: RX drop returned 0
   [00:00:02.022,000] <inf> i2s_pm: i2s_stop: TX drop returned 0
   [00:00:02.130,000] <inf> i2s_pm: i2s_stop: done
   [00:00:02.134,000] <inf> i2s_pm: POWER STATE SEQUENCE:
   [00:00:02.140,000] <inf> i2s_pm:   1. PM_STATE_RUNTIME_IDLE
   [00:00:02.146,000] <inf> i2s_pm:   2. PM_STATE_SUSPEND_TO_IDLE
   [00:00:02.152,000] <inf> i2s_pm:   3. PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY)
   [00:00:02.160,000] <inf> i2s_pm:   4. PM_STATE_SUSPEND_TO_RAM (substate 1: STOP)
   [00:00:02.168,000] <inf> i2s_pm:   5. (SOFT_OFF skipped - TCM boot, using retention)
   [00:00:02.176,000] <inf> i2s_pm: Enter RUNTIME_IDLE sleep for (18000000 microseconds)
   [00:00:20.185,000] <inf> i2s_pm: Exited from RUNTIME_IDLE sleep
   [00:00:20.191,000] <inf> i2s_pm: Enter PM_STATE_SUSPEND_TO_IDLE for (4000 microseconds)
   [00:00:20.206,000] <inf> i2s_pm: Exited from PM_STATE_SUSPEND_TO_IDLE
   [00:00:20.212,000] <inf> i2s_pm: === Restarting I2S after light sleep ===
   [00:00:22.202,000] <inf> i2s_pm: stream loop exited
   [00:00:22.207,000] <inf> i2s_pm: i2s_stop: RX drop returned 0
   [00:00:22.213,000] <inf> i2s_pm: i2s_stop: TX drop returned 0
   [00:00:22.320,000] <inf> i2s_pm: i2s_stop: done
   [00:00:22.324,000] <inf> i2s_pm: Enter PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) for (20000000 micr)
   [00:00:42.336,000] <inf> i2s_pm: === Resumed from PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) ===
   [00:00:48.674,000] <inf> i2s_pm: stream loop exited
   [00:00:48.679,000] <inf> i2s_pm: i2s_stop: RX drop returned 0
   [00:00:48.685,000] <inf> i2s_pm: i2s_stop: TX drop returned 0
   [00:00:48.795,000] <inf> i2s_pm: i2s_stop: done
   [00:00:48.799,000] <inf> i2s_pm: Enter PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) for (22000000 microse)
   [00:01:15.814,000] <inf> i2s_pm: === Resumed from PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) ===
   [00:01:22.147,000] <inf> i2s_pm: stream loop exited
   [00:01:22.153,000] <inf> i2s_pm: i2s_stop: RX drop returned 0
   [00:01:22.159,000] <inf> i2s_pm: i2s_stop: TX drop returned 0
   [00:01:22.268,000] <inf> i2s_pm: i2s_stop: done
   [00:01:22.273,000] <inf> i2s_pm: Skipping PM_STATE_SOFT_OFF (TCM boot, using retention instead)
   [00:01:22.282,000] <inf> i2s_pm: === POWER STATE SEQUENCE COMPLETED ===

HE Core - MRAM Boot (SOFT_OFF with system reset)
==================================================

.. code-block:: console

   *** Booting Zephyr OS build 9bca98ae74dc ***
   [00:00:00.005,000] <inf> i2s_pm: alif_e8_dk RTSS_HE (MRAM boot): I2S PM demo (RUNTIME_IDLE, SUSPEND_TO_)
   [00:00:00.016,000] <inf> i2s_pm: === Phase 1: Initial I2S streaming ===
   [00:00:02.011,000] <inf> i2s_pm: stream loop exited
   [00:00:02.017,000] <inf> i2s_pm: i2s_stop: RX drop returned 0
   [00:00:02.023,000] <inf> i2s_pm: i2s_stop: TX drop returned 0
   [00:00:02.130,000] <inf> i2s_pm: i2s_stop: done
   [00:00:02.134,000] <inf> i2s_pm: POWER STATE SEQUENCE:
   [00:00:02.140,000] <inf> i2s_pm:   1. PM_STATE_RUNTIME_IDLE
   [00:00:02.146,000] <inf> i2s_pm:   2. PM_STATE_SUSPEND_TO_IDLE
   [00:00:02.152,000] <inf> i2s_pm:   3. (S2RAM skipped - MRAM boot)
   [00:00:02.159,000] <inf> i2s_pm:   4. PM_STATE_SOFT_OFF
   [00:00:02.164,000] <inf> i2s_pm: Enter RUNTIME_IDLE sleep for (18000000 microseconds)
   [00:00:20.174,000] <inf> i2s_pm: Exited from RUNTIME_IDLE sleep
   [00:00:20.180,000] <inf> i2s_pm: Enter PM_STATE_SUSPEND_TO_IDLE for (4000 microseconds)
   [00:00:20.195,000] <inf> i2s_pm: Exited from PM_STATE_SUSPEND_TO_IDLE
   [00:00:20.201,000] <inf> i2s_pm: === Restarting I2S after light sleep ===
   [00:00:22.191,000] <inf> i2s_pm: stream loop exited
   [00:00:22.196,000] <inf> i2s_pm: i2s_stop: RX drop returned 0
   [00:00:22.202,000] <inf> i2s_pm: i2s_stop: TX drop returned 0
   [00:00:22.309,000] <inf> i2s_pm: i2s_stop: done
   [00:00:22.313,000] <inf> i2s_pm: Skipping PM_STATE_SUSPEND_TO_RAM (MRAM boot)
   [00:00:28.651,000] <inf> i2s_pm: stream loop exited
   [00:00:28.777,000] <inf> i2s_pm: Enter PM_STATE_SOFT_OFF for (26000000 microseconds)
   [00:00:28.785,000] <inf> i2s_pm: Note: SOFT_OFF has no retention - system will reset on wakeup

   <-- System resets here after 26 seconds -->

   *** Booting Zephyr OS build 9bca98ae74dc ***
   [00:00:00.005,000] <inf> i2s_pm: alif_e8_dk RTSS_HE (MRAM boot): I2S PM demo (RUNTIME_IDLE, SUSPEND_TO_)
   [Cycle repeats...]

HP Core - MRAM Boot (no retention, SOFT_OFF only)
==================================================

.. code-block:: console

   *** Booting Zephyr OS build 9bca98ae74dc ***
   [00:00:00.004,000] <inf> i2s_pm: alif_e8_dk RTSS_HP: I2S PM demo (RUNTIME_IDLE, SUSPEND_TO_IDLE, SOFT_O)
   [00:00:00.014,000] <inf> i2s_pm: === Phase 1: Initial I2S streaming ===
   [00:00:02.007,000] <inf> i2s_pm: stream loop exited
   [00:00:02.012,000] <inf> i2s_pm: i2s_stop: RX drop returned 0
   [00:00:02.018,000] <inf> i2s_pm: i2s_stop: TX drop returned 0
   [00:00:02.125,000] <inf> i2s_pm: i2s_stop: done
   [00:00:02.130,000] <inf> i2s_pm: POWER STATE SEQUENCE:
   [00:00:02.135,000] <inf> i2s_pm:   1. PM_STATE_RUNTIME_IDLE
   [00:00:02.141,000] <inf> i2s_pm:   2. PM_STATE_SUSPEND_TO_IDLE
   [00:00:02.148,000] <inf> i2s_pm:   3. PM_STATE_SOFT_OFF
   [00:00:02.153,000] <inf> i2s_pm: Enter RUNTIME_IDLE sleep for (18000000 microseconds)
   [00:00:20.163,000] <inf> i2s_pm: Exited from RUNTIME_IDLE sleep
   [00:00:20.169,000] <inf> i2s_pm: PM_STATE_SUSPEND_TO_IDLE (skipped - LPM timer not enabled)
   [00:00:20.177,000] <inf> i2s_pm: === Restarting I2S after light sleep ===
   [00:00:22.165,000] <inf> i2s_pm: stream loop exited
   [00:00:22.171,000] <inf> i2s_pm: i2s_stop: RX drop returned 0
   [00:00:22.177,000] <inf> i2s_pm: i2s_stop: TX drop returned 0
   [00:00:22.284,000] <inf> i2s_pm: i2s_stop: done
   [00:00:22.288,000] <inf> i2s_pm: Enter PM_STATE_SOFT_OFF for (26000000 microseconds)
   [00:00:22.296,000] <inf> i2s_pm: Note: SOFT_OFF has no retention - system will reset on wakeup

   <-- System resets here after 26 seconds -->

   *** Booting Zephyr OS build 9bca98ae74dc ***
   [00:00:00.004,000] <inf> i2s_pm: alif_e8_dk RTSS_HP: I2S PM demo (RUNTIME_IDLE, SUSPEND_TO_IDLE, SOFT_O)
   [Cycle repeats...]

Notes
*****

* **Debugger**: Disconnect debugger before testing - it prevents cores from entering OFF states
* **UART Hub**: If using USB hub for UART, set BOOT_DELAY to avoid missing logs after power cycle
* **I2S Configuration**:

  * Sample rate: 44.1 kHz, 16-bit stereo
  * Block size: 17,640 bytes (~100 ms of audio per block)
  * I2S is stopped before entering any PM state and restarted after wakeup

* **Sleep Durations**:

  * RUNTIME_IDLE: 18 seconds
  * SUSPEND_TO_IDLE: 4 milliseconds
  * S2RAM STANDBY: 20 seconds
  * S2RAM STOP: 22 seconds
  * SOFT_OFF: 26 seconds

* **SUSPEND_TO_IDLE Details**:

  * Requires LPM timer support (``CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER``)
  * HE core: Enabled by default with RTC as LPM timer
  * HP core: Skipped by default (RTC shared with HE, can be enabled if needed)
  * Uses IWIC (Internal WIC) for wake sources
  * Devices remain active - no suspend/resume overhead

* **Retention Memory**: HE core retains SERAM and optionally TCM (when booting from TCM)
* **Power Measurement**: For accurate power measurements, ensure all unused peripherals are disabled

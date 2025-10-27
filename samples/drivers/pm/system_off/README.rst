.. _alif-pm-states-sample:

Alif Power Management States Demo
##################################

Overview
********

This sample demonstrates Zephyr power management states on Alif RTSS cores, showcasing
different PM state transitions with RTC wakeup capabilities. The sample illustrates:

* **PM_STATE_RUNTIME_IDLE**: Light sleep state with quick wakeup
* **PM_STATE_SUSPEND_TO_RAM (S2RAM)**: Deep sleep with retention (HE core only)

  * Substate 0 (STANDBY): Medium power savings
  * Substate 1 (STOP): Higher power savings

* **PM_STATE_SOFT_OFF**: Deepest sleep, no retention, full system reset on wakeup

The behavior differs between HP and HE cores:

**HE Core (with retention support)**:

* When booting from **TCM** (VTOR = 0x0):

  * Demonstrates RUNTIME_IDLE, S2RAM STANDBY, S2RAM STOP
  * Skips SOFT_OFF (uses retention instead)
  * Resumes execution after each state

* When booting from **MRAM** (VTOR >= 0x80000000):

  * Demonstrates RUNTIME_IDLE, SOFT_OFF
  * System resets and restarts from main() after SOFT_OFF

**HP Core (no retention support)**:

* Only SOFT_OFF is available (no S2RAM support)
* Demonstrates RUNTIME_IDLE, then SOFT_OFF
* System resets and restarts from main() after SOFT_OFF

Requirements
************

* Alif Ensemble or Balletto development board
* RTC peripheral enabled for wakeup
* SE Services for power profile configuration

Supported Boards
****************

* alif_e3_dk_rtss_he
* alif_e7_dk_rtss_he
* alif_e1c_dk_rtss_he
* alif_b1_dk_rtss_he
* alif_e4_dk_rtss_he
* alif_e8_dk_rtss_he
* alif_e3_dk_rtss_hp (HP core)
* alif_e7_dk_rtss_hp (HP core)
* alif_e4_dk_rtss_hp (HP core)
* alif_e8_dk_rtss_hp (HP core)

Building and Running
********************

Build for HE core (TCM boot with retention):

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
       ../alif/samples/drivers/pm/system_off \
       -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
       -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
       -DCONFIG_FLASH_SIZE=256

Build for HE core (MRAM boot):

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
       ../alif/samples/drivers/pm/system_off

Build for HP core:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
       ../alif/samples/drivers/pm/system_off

Flash the binary using SE Tools. See :ref:`programming_an_application` for details.

Sample Output
*************

HE Core - TCM Boot (with retention, SOFT_OFF skipped)
======================================================

.. code-block:: console

   *** Booting Zephyr OS build v4.1.0-231-g97f5a925a760 ***
   [00:00:00.004,000] <inf> pm_system_off:
   alif_e7_dk RTSS_HE (TCM boot): PM states demo (RUNTIME_IDLE, S2RAM)
   [00:00:00.015,000] <inf> pm_system_off: POWER STATE SEQUENCE:
   [00:00:00.021,000] <inf> pm_system_off:   1. PM_STATE_RUNTIME_IDLE
   [00:00:00.027,000] <inf> pm_system_off:   2. PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY)
   [00:00:00.036,000] <inf> pm_system_off:   3. PM_STATE_SUSPEND_TO_RAM (substate 1: STOP)
   [00:00:00.044,000] <inf> pm_system_off:   4. (SOFT_OFF skipped - TCM boot, using retention)
   [00:00:00.053,000] <inf> pm_system_off: Enter RUNTIME_IDLE sleep for (18000000 microseconds)
   [00:00:18.063,000] <inf> pm_system_off: Exited from RUNTIME_IDLE sleep
   [00:00:18.069,000] <inf> pm_system_off: Enter PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) for (20000000 microseconds)
   [00:00:38.081,000] <inf> pm_system_off: === Resumed from PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) ===
   [00:00:38.090,000] <inf> pm_system_off: Main thread running - iteration 0 - tick: 38090
   [00:00:40.100,000] <inf> pm_system_off: Main thread running - iteration 1 - tick: 40100
   [00:00:42.109,000] <inf> pm_system_off: Main thread running - iteration 2 - tick: 42109
   [00:00:44.118,000] <inf> pm_system_off: Enter PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) for (22000000 microseconds)
   [00:01:06.129,000] <inf> pm_system_off: === Resumed from PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) ===
   [00:01:06.138,000] <inf> pm_system_off: Main thread running - iteration 0 - tick: 66138
   [00:01:08.148,000] <inf> pm_system_off: Main thread running - iteration 1 - tick: 68148
   [00:01:10.157,000] <inf> pm_system_off: Main thread running - iteration 2 - tick: 70157
   [00:01:12.166,000] <inf> pm_system_off: Skipping PM_STATE_SOFT_OFF (TCM boot, using retention instead)
   [00:01:12.175,000] <inf> pm_system_off: === POWER STATE SEQUENCE COMPLETED ===

HP Core - MRAM Boot (no retention, only SOFT_OFF)
==================================================

.. code-block:: console

   *** Booting Zephyr OS build v4.1.0-231-g97f5a925a760 ***
   [00:00:00.007,000] <inf> pm_system_off:
   alif_e7_dk RTSS_HP: PM states demo (RUNTIME_IDLE, SOFT_OFF)
   [00:00:00.018,000] <inf> pm_system_off: POWER STATE SEQUENCE:
   [00:00:00.024,000] <inf> pm_system_off:   1. PM_STATE_RUNTIME_IDLE
   [00:00:00.031,000] <inf> pm_system_off:   2. PM_STATE_SOFT_OFF
   [00:00:00.037,000] <inf> pm_system_off: Enter RUNTIME_IDLE sleep for (18000000 microseconds)
   [00:00:18.048,000] <inf> pm_system_off: Exited from RUNTIME_IDLE sleep
   [00:00:18.054,000] <inf> pm_system_off: Enter PM_STATE_SOFT_OFF for (24000000 microseconds)
   [00:00:18.063,000] <inf> pm_system_off: Note: SOFT_OFF has no retention - system will reset on wakeup

   <-- System resets here after 24 seconds -->

   *** Booting Zephyr OS build v4.1.0-231-g97f5a925a760 ***
   [00:00:00.006,000] <inf> pm_system_off:
   alif_e7_dk RTSS_HP: PM states demo (RUNTIME_IDLE, SOFT_OFF)
   [00:00:00.017,000] <inf> pm_system_off: POWER STATE SEQUENCE:
   [00:00:00.023,000] <inf> pm_system_off:   1. PM_STATE_RUNTIME_IDLE
   [00:00:00.030,000] <inf> pm_system_off:   2. PM_STATE_SOFT_OFF
   [00:00:00.036,000] <inf> pm_system_off: Enter RUNTIME_IDLE sleep for (18000000 microseconds)
   [Cycle repeats...]

Notes
*****

* **Debugger**: Disconnect debugger before testing - it prevents cores from entering OFF states
* **UART Hub**: If using USB hub for UART, set BOOT_DELAY to avoid missing logs after power cycle
* **Sleep Durations**:

  * RUNTIME_IDLE: 18 seconds
  * S2RAM STANDBY: 20 seconds
  * S2RAM STOP: 22 seconds
  * SOFT_OFF: 24 seconds

* **CONFIG_POWEROFF**: Alternative mode to test sys_poweroff() instead of PM state sequence
* **Retention Memory**: HE core retains SERAM and optionally TCM (when booting from TCM)
* **Power Measurement**: For accurate power measurements, ensure all unused peripherals are disabled

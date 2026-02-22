.. _alif-pm-states-sample:

Alif Power Management States Demo
##################################

Overview
********

This sample demonstrates Zephyr power management states on Alif RTSS cores, showcasing
different PM state transitions with RTC wakeup capabilities. The sample illustrates:

* **PM_STATE_RUNTIME_IDLE**: Light sleep state with quick wakeup (CPU clock gating via WFI)
* **PM_STATE_SUSPEND_TO_IDLE**: CPU sleep with IWIC (Internal WIC), devices remain active (no device PM overhead)
* **PM_STATE_SUSPEND_TO_RAM (S2RAM)**: Deep sleep with retention (HE core only)

  * Substate 0 (STANDBY): Medium power savings
  * Substate 1 (STOP): Higher power savings

* **PM_STATE_SOFT_OFF**: Deepest sleep, no retention, full system reset on wakeup

The behavior differs between HP and HE cores:

**HE Core (with retention support)**:

* When booting from **TCM** (VTOR = 0x0):

  * Demonstrates RUNTIME_IDLE, SUSPEND_TO_IDLE, S2RAM STANDBY, S2RAM STOP
  * Skips SOFT_OFF (uses retention instead)
  * Resumes execution after each state

* When booting from **MRAM** (VTOR >= 0x80000000):

  * Demonstrates RUNTIME_IDLE, SUSPEND_TO_IDLE, SOFT_OFF
  * System resets and restarts from main() after SOFT_OFF

**HP Core (no retention support)**:

* Only SOFT_OFF is available for deeper sleep (no S2RAM support)
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
       -S pm-system-off-he \
       -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
       -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
       -DCONFIG_FLASH_SIZE=256

Build for HE core (MRAM boot):

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
       ../alif/samples/drivers/pm/system_off \
       -S pm-system-off-he

Build for HP core:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
       ../alif/samples/drivers/pm/system_off \
       -S pm-system-off-hp

Flash the binary using SE Tools. See :ref:`programming_an_application` for details.

Sample Output
*************

HE Core - TCM Boot (with retention, SOFT_OFF skipped)
======================================================

.. code-block:: console

   *** Booting Zephyr OS build v4.1.0-415-g8a0d36191e14 ***
   [00:00:00.004,000] <inf> pm_system_off: alif_e7_dk RTSS_HE (TCM boot): PM states demo (RUNTIME_IDLE, SUSPEND_TO_IDLE, S2RAM)
   [00:00:00.016,000] <inf> pm_system_off: POWER STATE SEQUENCE:
   [00:00:00.022,000] <inf> pm_system_off:   1. PM_STATE_RUNTIME_IDLE
   [00:00:00.029,000] <inf> pm_system_off:   2. PM_STATE_SUSPEND_TO_IDLE
   [00:00:00.036,000] <inf> pm_system_off:   3. PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY)
   [00:00:00.044,000] <inf> pm_system_off:   4. PM_STATE_SUSPEND_TO_RAM (substate 1: STOP)
   [00:00:00.053,000] <inf> pm_system_off:   5. (SOFT_OFF skipped - TCM boot, using retention)
   [00:00:00.062,000] <inf> pm_system_off: Enter RUNTIME_IDLE sleep for (18000000 microseconds)
   [00:00:18.071,000] <inf> pm_system_off: Exited from RUNTIME_IDLE sleep
   [00:00:18.077,000] <inf> pm_system_off: Enter PM_STATE_SUSPEND_TO_IDLE for (4000 microseconds)
   [00:00:18.092,000] <inf> pm_system_off: Exited from PM_STATE_SUSPEND_TO_IDLE
   [00:00:18.099,000] <inf> pm_system_off: Enter PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) for (20000000 microseconds)
   [00:00:38.115,000] <inf> pm_system_off: === Resumed from PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) ===
   [00:00:38.125,000] <inf> pm_system_off: Main thread running - iteration 0 - tick: 38125
   [00:00:40.135,000] <inf> pm_system_off: Main thread running - iteration 1 - tick: 40135
   [00:00:42.145,000] <inf> pm_system_off: Main thread running - iteration 2 - tick: 42145
   [00:00:44.154,000] <inf> pm_system_off: Enter PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) for (22000000 microseconds)
   [00:01:06.169,000] <inf> pm_system_off: === Resumed from PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) ===
   [00:01:06.178,000] <inf> pm_system_off: Main thread running - iteration 0 - tick: 66178
   [00:01:08.188,000] <inf> pm_system_off: Main thread running - iteration 1 - tick: 68188
   [00:01:10.198,000] <inf> pm_system_off: Main thread running - iteration 2 - tick: 70198
   [00:01:12.207,000] <inf> pm_system_off: Skipping PM_STATE_SOFT_OFF (TCM boot, using retention instead)
   [00:01:12.217,000] <inf> pm_system_off: === POWER STATE SEQUENCE COMPLETED ===

HP Core - MRAM Boot (no retention, only SOFT_OFF)
==================================================

.. code-block:: console

   *** Booting Zephyr OS build v4.1.0-415-g8a0d36191e14 ***
   [00:00:00.005,000] <inf> pm_system_off: alif_e7_dk RTSS_HP: PM states demo (RUNTIME_IDLE, SUSPEND_TO_IDLE, SOFT_OFF)
   [00:00:00.017,000] <inf> pm_system_off: POWER STATE SEQUENCE:
   [00:00:00.023,000] <inf> pm_system_off:   1. PM_STATE_RUNTIME_IDLE
   [00:00:00.030,000] <inf> pm_system_off:   2. PM_STATE_SUSPEND_TO_IDLE
   [00:00:00.037,000] <inf> pm_system_off:   3. PM_STATE_SOFT_OFF
   [00:00:00.043,000] <inf> pm_system_off: Enter RUNTIME_IDLE sleep for (18000000 microseconds)
   [00:00:18.054,000] <inf> pm_system_off: Exited from RUNTIME_IDLE sleep
   [00:00:18.060,000] <inf> pm_system_off: PM_STATE_SUSPEND_TO_IDLE (skipped - LPM timer not enabled)
   [00:00:18.070,000] <inf> pm_system_off: Enter PM_STATE_SOFT_OFF for (26000000 microseconds)
   [00:00:18.078,000] <inf> pm_system_off: Note: SOFT_OFF has no retention - system will reset on wakeup

   <-- System resets here after 26 seconds -->

   *** Booting Zephyr OS build v4.1.0-415-g8a0d36191e14 ***
   [00:00:00.005,000] <inf> pm_system_off: alif_e7_dk RTSS_HP: PM states demo (RUNTIME_IDLE, SUSPEND_TO_IDLE, SOFT_OFF)
   [00:00:00.018,000] <inf> pm_system_off: POWER STATE SEQUENCE:
   [00:00:00.024,000] <inf> pm_system_off:   1. PM_STATE_RUNTIME_IDLE
   [00:00:00.030,000] <inf> pm_system_off:   2. PM_STATE_SUSPEND_TO_IDLE
   [00:00:00.037,000] <inf> pm_system_off:   3. PM_STATE_SOFT_OFF
   [00:00:00.043,000] <inf> pm_system_off: Enter RUNTIME_IDLE sleep for (18000000 microseconds)
   [Cycle repeats...]

Notes
*****

* **Debugger**: Disconnect debugger before testing - it prevents cores from entering OFF states
* **UART Hub**: If using USB hub for UART, set BOOT_DELAY to avoid missing logs after power cycle
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
  * Uses IWIC (Internal WIC) for wake sources - lighter than EWIC
  * Only interrupts 0-63 can wake from IWIC mode
  * Devices remain active - no suspend/resume overhead (zephyr,pm-device-disabled)
  * Exit latency (~500µs) primarily from RTC register read/write for time compensation
  * Suitable for frequent, low-latency wake scenarios
  * Enable in device tree with ``suspend_idle`` node status = "okay"

* **CONFIG_POWEROFF**: Alternative mode to test sys_poweroff() instead of PM state sequence
* **Retention Memory**: HE core retains SERAM and optionally TCM (when booting from TCM)
* **Power Measurement**: For accurate power measurements, ensure all unused peripherals are disabled

.. _display-pm-sample:

Display Power Management Demo
#############################

Overview
********

This sample demonstrates Zephyr power management states combined with a
CDC200 + MIPI DSI display pipeline on Alif RTSS cores. The application
cycles through PM states (RUNTIME_IDLE, SUSPEND_TO_IDLE, and S2RAM
STANDBY/STOP, or SOFT_OFF on MRAM boot) and streams stored data in buffer
to the display after each wake, verifying that the CDC200, DSI host, D-PHY,
and panel driver all resume correctly and reproduce a valid frame.

PM states exercised (determined at runtime by capability predicates):

- **S2RAM path** (TCM or SRAM0 retention): RUNTIME_IDLE → SUSPEND_TO_IDLE →
  S2RAM STANDBY → S2RAM STOP → idle loop
- **SOFT_OFF path** (MRAM boot, no retention): RUNTIME_IDLE → SUSPEND_TO_IDLE →
  SOFT_OFF (system resets on wakeup)

Building and Running
********************

HE Core — TCM boot S2RAM (E8)
=========================================

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/display_pm
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_he
   :goals: build
   :west-args: -p auto
   :snippets: display-pm-s2ram-tcm
   :gen-args: -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

HE Core — MRAM boot SOFT_OFF (E8)
=============================================

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/display_pm
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_he
   :goals: build
   :west-args: -p auto
   :snippets: display-pm-mram

HP Core — MRAM boot SOFT_OFF (E8)
======================================

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/display_pm
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_hp
   :goals: build
   :west-args: -p auto
   :snippets: display-pm-mram

Sample Output (S2RAM path)
**************************

The output below is from an E8 DK HE core TCM-boot run (``APP_PM_WAKEUP_DEBUG 0``,
the default). Setting ``APP_PM_WAKEUP_DEBUG 1`` in ``main.c`` additionally prints
``PM wakeup: NVIC ISPR[x] = 0x...`` lines on each resume.

.. code-block:: console

   *** Booting Zephyr OS build 3a2b84d96961 ***
   [00:00:00.262,000] <inf> disp_pm: alif_e8_dk (S2RAM): CAM PM demo (RUNTIME_IDLE, SUSPEND_TO_IDLE, S2RAM STANDBY, S2RAM STOP)
   [00:00:00.262,000] <inf> disp_pm: POWER STATE SEQUENCE:
   [00:00:00.262,000] <inf> disp_pm:   1. PM_STATE_RUNTIME_IDLE
   [00:00:00.262,000] <inf> disp_pm:   2. PM_STATE_SUSPEND_TO_IDLE
   [00:00:00.262,000] <inf> disp_pm:   3. PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY)
   [00:00:00.262,000] <inf> disp_pm:   4. PM_STATE_SUSPEND_TO_RAM (substate 1: STOP)
   [00:00:00.262,000] <inf> disp_pm: Enter RUNTIME_IDLE sleep for (18000000 microseconds)
   [00:00:00.262,000] <inf> disp_pm: skip starting a new stream cycle this round
   [00:00:00.262,000] <inf> disp_pm: Enabling CDC200 Device.
   [00:00:00.262,000] <inf> disp_pm: Display init: cdc200@49031000, panel res (480, 800), fmt 25
   [00:00:00.262,000] <dbg> disp_pm: display_init: Layer 1: en=1 res=(480,800) fmt=1
   [00:00:00.262,000] <dbg> disp_pm: display_init: Layer 2: en=0 res=(0,0) fmt=0
   [00:00:00.262,000] <inf> disp_pm: Display: suspending for PM cycle
   [00:00:00.383,000] <inf> disp_pm: Display thread is now suspended (polling for resume)
   [00:00:00.383,000] <inf> disp_pm: Display streaming is Suspended
   [00:00:18.384,000] <inf> disp_pm: Exited from RUNTIME_IDLE sleep
   [00:00:18.384,000] <dbg> disp_pm: display_pm_thread_resume: Display: Try to Resume...
   [00:00:18.384,000] <inf> disp_pm: Display: resume signal sent
   [00:00:18.384,000] <inf> disp_pm: Display: resuming after PM wake
   [00:00:18.696,000] <dbg> disp_pm: display_streaming_thread: Display: reinit complete, resuming streaming
   [00:00:18.696,000] <inf> disp_pm: FB0 - 0x02000000, size - 1152000
   [00:00:28.827,000] <inf> disp_pm: Display: streaming cycle complete, waiting for next PM cycle
   [00:00:28.827,000] <inf> disp_pm: Display streaming cycle OK (RUNTIME_IDLE)
   [00:00:28.827,000] <inf> disp_pm: Request SUSPEND_TO_IDLE for 10000 us
   [00:00:28.827,000] <inf> disp_pm: Display: suspending for PM cycle
   [00:00:28.838,000] <inf> disp_pm: Exited from SUSPEND_TO_IDLE sleep
   [00:00:28.838,000] <inf> disp_pm: Request S2RAM STANDBY for 6000000 us
   [00:00:28.948,000] <inf> disp_pm: Display thread is now suspended (polling for resume)
   [00:00:28.948,000] <inf> disp_pm: Display streaming is Suspended: for Deep Sleep
   [00:00:28.988,000] <inf> disp_pm: PM enter: SUSPEND_TO_RAM (substate 0)
   [00:00:28.988,000] <inf> disp_pm: PM wakeup: SUSPEND_TO_RAM (substate 0)
   [00:00:28.988,000] <inf> disp_pm: PM exit:  SUSPEND_TO_RAM (substate 0)
   [00:00:34.958,000] <inf> disp_pm: Display streaming is Suspended: for Deep Sleep
   [00:00:34.958,000] <dbg> disp_pm: display_pm_thread_resume: Display: Try to Resume...
   [00:00:34.958,000] <inf> disp_pm: Display: resume signal sent
   [00:00:34.958,000] <inf> disp_pm: Display stream is Resumed: for Deep Sleep
   [00:00:34.958,000] <inf> disp_pm: === Resumed from PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) ===
   [00:00:34.958,000] <inf> disp_pm: Display: resuming after PM wake
   [00:00:35.271,000] <dbg> disp_pm: display_streaming_thread: Display: reinit complete, resuming streaming
   [00:00:35.271,000] <inf> disp_pm: FB0 - 0x02000000, size - 1152000
   [00:00:45.402,000] <inf> disp_pm: Display: streaming cycle complete, waiting for next PM cycle
   [00:00:45.402,000] <inf> disp_pm: Display streaming cycle OK (RUNTIME_IDLE)
   [00:00:45.402,000] <inf> disp_pm: Main thread running - iteration 0 - tick: 45402
   [00:00:45.402,000] <inf> disp_pm: Display: suspending for PM cycle
   [00:00:47.403,000] <inf> disp_pm: Main thread running - iteration 1 - tick: 47403
   [00:00:49.404,000] <inf> disp_pm: Main thread running - iteration 2 - tick: 49404
   [00:00:51.405,000] <inf> disp_pm: Request S2RAM STOP for 9000000 us
   [00:00:51.405,000] <inf> disp_pm: Display thread is now suspended (polling for resume)
   [00:00:51.405,000] <inf> disp_pm: Display streaming is Suspended: for Deep Sleep
   [00:00:51.458,000] <inf> disp_pm: PM enter: SUSPEND_TO_RAM (substate 1)
   [00:00:51.458,000] <inf> disp_pm: PM wakeup: SUSPEND_TO_RAM (substate 1)
   [00:00:51.458,000] <inf> disp_pm: PM exit:  SUSPEND_TO_RAM (substate 1)
   [00:01:00.418,000] <inf> disp_pm: Display streaming is Suspended: for Deep Sleep
   [00:01:00.418,000] <dbg> disp_pm: display_pm_thread_resume: Display: Try to Resume...
   [00:01:00.418,000] <inf> disp_pm: Display: resume signal sent
   [00:01:00.418,000] <inf> disp_pm: Display stream is Resumed: for Deep Sleep
   [00:01:00.418,000] <inf> disp_pm: === Resumed from PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) ===
   [00:01:00.418,000] <inf> disp_pm: Display: resuming after PM wake
   [00:01:00.730,000] <dbg> disp_pm: display_streaming_thread: Display: reinit complete, resuming streaming
   [00:01:00.730,000] <inf> disp_pm: FB0 - 0x02000000, size - 1152000
   [00:01:10.861,000] <inf> disp_pm: Display: streaming cycle complete, waiting for next PM cycle
   [00:01:10.861,000] <inf> disp_pm: Display streaming cycle OK (STOP)
   [00:01:10.861,000] <inf> disp_pm: Main thread running - iteration 0 - tick: 70861
   [00:01:10.861,000] <inf> disp_pm: Display: suspending for PM cycle
   [00:01:12.862,000] <inf> disp_pm: Main thread running - iteration 1 - tick: 72862
   [00:01:14.863,000] <inf> disp_pm: Main thread running - iteration 2 - tick: 74863
   [00:01:16.864,000] <inf> disp_pm: === DISPLAY PM SEQUENCE COMPLETED ===

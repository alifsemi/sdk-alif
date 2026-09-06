.. _i2s-dw-pm-sample:

I2S DW Power Management Demo
#############################

Overview
********

This sample demonstrates Zephyr power management states combined with I2S DW
echo streaming on Alif RTSS cores. The application cycles through PM states
and restarts I2S RX/TX echo after each wake, verifying that the I2S peripheral
resumes correctly.

PM states exercised (determined at runtime by capability predicates):

- **S2RAM path** (HE TCM retention): RUNTIME_IDLE → SUSPEND_TO_IDLE →
  S2RAM STANDBY → S2RAM STOP → idle loop
- **SOFT_OFF path** (MRAM boot, no retention): RUNTIME_IDLE → SUSPEND_TO_IDLE →
  SOFT_OFF (system resets on wakeup)

I2S routing (microphone in, speaker out):

- Ensemble HE (E7/E8): ``i2s4`` (LPI2S RX) + ``i2s3`` (TX)
- Ensemble HP (E7/E8): ``i2s3`` (RX) + ``i2s1`` (TX)
- Balletto B1 / Ensemble E1C HE: ``i2s4`` (LPI2S RX) + ``i2s0`` (TX)

.. note::

   Use LPUART port for console logs on E1C and B1 DevKits.

Building and Running
********************

HE Core — TCM boot S2RAM (E7/E8/E1C/B1)
=========================================

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/i2s_dw
   :board: alif_e7_dk/ae722f80f55d5xx/rtss_he
   :goals: build
   :west-args: -p auto
   :snippets: i2s-dw-pm-s2ram-tcm
   :gen-args: -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

HE Core — MRAM boot SOFT_OFF (E7/E8/E1C/B1)
=============================================

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/i2s_dw
   :board: alif_e7_dk/ae722f80f55d5xx/rtss_he
   :goals: build
   :west-args: -p auto
   :snippets: i2s-dw-pm-mram

HP Core — MRAM boot SOFT_OFF (E7/E8)
======================================

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/i2s_dw
   :board: alif_e7_dk/ae722f80f55d5xx/rtss_hp
   :goals: build
   :west-args: -p auto
   :snippets: i2s-dw-pm-mram

Sample Output (S2RAM path)
**************************

The output below is from an E8 DK HE core TCM-boot run (``APP_PM_WAKEUP_DEBUG 0``,
the default). Setting ``APP_PM_WAKEUP_DEBUG 1`` in ``main.c`` additionally prints
``PM wakeup: NVIC ISPR[x] = 0x...`` lines on each resume.

.. code-block:: console

   *** Booting Zephyr OS build 2134310f0be8 ***
   [00:00:00.000,000] <inf> pm_i2s_dw: alif_e8_dk (S2RAM): I2S DW PM demo (RUNTIME_IDLE, SUSPEND_TO_IDLE, )
   [00:00:00.000,000] <inf> pm_i2s_dw: POWER STATE SEQUENCE:
   [00:00:00.000,000] <inf> pm_i2s_dw:   1. PM_STATE_RUNTIME_IDLE
   [00:00:00.000,000] <inf> pm_i2s_dw:   2. PM_STATE_SUSPEND_TO_IDLE
   [00:00:00.000,000] <inf> pm_i2s_dw:   3. PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY)
   [00:00:00.000,000] <inf> pm_i2s_dw:   4. PM_STATE_SUSPEND_TO_RAM (substate 1: STOP)
   [00:00:00.001,000] <inf> pm_i2s_dw: [I2S Demo] before RUNTIME_IDLE
   [00:00:01.989,000] <inf> pm_i2s_dw: stream loop exited
   [00:00:01.989,000] <inf> pm_i2s_dw: i2s_stop: RX drop returned 0
   [00:00:01.989,000] <inf> pm_i2s_dw: i2s_stop: TX drop returned 0
   [00:00:02.090,000] <inf> pm_i2s_dw: i2s_stop: done
   [00:00:02.090,000] <inf> pm_i2s_dw: Enter RUNTIME_IDLE sleep for (18000000 microseconds)
   [00:00:20.091,000] <inf> pm_i2s_dw: Exited from RUNTIME_IDLE sleep
   [00:00:20.091,000] <inf> pm_i2s_dw: Request SUSPEND_TO_IDLE for 10000 us
   [00:00:20.093,000] <inf> pm_i2s_dw: PM enter: SUSPEND_TO_IDLE (substate 0)
   [00:00:20.093,000] <inf> pm_i2s_dw: PM exit:  SUSPEND_TO_IDLE (substate 0)
   [00:00:20.102,000] <inf> pm_i2s_dw: [I2S Demo] after SUSPEND_TO_IDLE
   [00:00:22.090,000] <inf> pm_i2s_dw: stream loop exited
   [00:00:22.090,000] <inf> pm_i2s_dw: i2s_stop: RX drop returned 0
   [00:00:22.090,000] <inf> pm_i2s_dw: i2s_stop: TX drop returned 0
   [00:00:22.191,000] <inf> pm_i2s_dw: i2s_stop: done
   [00:00:22.191,000] <inf> pm_i2s_dw: Request S2RAM STANDBY for 6000000 us
   [00:00:22.193,000] <inf> pm_i2s_dw: PM enter: SUSPEND_TO_IDLE (substate 0)
   [00:00:22.193,000] <inf> pm_i2s_dw: PM exit:  SUSPEND_TO_IDLE (substate 0)
   [00:00:22.253,000] <inf> pm_i2s_dw: PM enter: SUSPEND_TO_RAM (substate 0)
   [00:00:22.253,000] <inf> pm_i2s_dw: PM wakeup: SUSPEND_TO_RAM (substate 0)
   [00:00:22.253,000] <inf> pm_i2s_dw: PM exit:  SUSPEND_TO_RAM (substate 0)
   [00:00:28.211,000] <inf> pm_i2s_dw: [I2S Demo] after S2RAM STANDBY
   [00:00:30.199,000] <inf> pm_i2s_dw: stream loop exited
   [00:00:30.199,000] <inf> pm_i2s_dw: i2s_stop: RX drop returned 0
   [00:00:30.200,000] <inf> pm_i2s_dw: i2s_stop: TX drop returned 0
   [00:00:30.301,000] <inf> pm_i2s_dw: i2s_stop: done
   [00:00:30.301,000] <inf> pm_i2s_dw: Request S2RAM STOP for 9000000 us
   [00:00:30.303,000] <inf> pm_i2s_dw: PM enter: SUSPEND_TO_IDLE (substate 0)
   [00:00:30.303,000] <inf> pm_i2s_dw: PM exit:  SUSPEND_TO_IDLE (substate 0)
   [00:00:30.363,000] <inf> pm_i2s_dw: PM enter: SUSPEND_TO_RAM (substate 1)
   [00:00:30.363,000] <inf> pm_i2s_dw: PM wakeup: SUSPEND_TO_RAM (substate 1)
   [00:00:30.363,000] <inf> pm_i2s_dw: PM exit:  SUSPEND_TO_RAM (substate 1)
   [00:00:39.316,000] <inf> pm_i2s_dw: [I2S Demo] after S2RAM STOP
   [00:00:41.305,000] <inf> pm_i2s_dw: stream loop exited
   [00:00:41.305,000] <inf> pm_i2s_dw: i2s_stop: RX drop returned 0
   [00:00:41.305,000] <inf> pm_i2s_dw: i2s_stop: TX drop returned 0
   [00:00:41.406,000] <inf> pm_i2s_dw: i2s_stop: done
   [00:00:41.406,000] <inf> pm_i2s_dw: === I2S DW PM SEQUENCE COMPLETED ===

Sample Output (SOFT_OFF path)
*****************************

The output below is from an E8 DK HE core MRAM-boot run. After SOFT_OFF the
system resets and restarts from ``main()``.

.. code-block:: console

   *** Booting Zephyr OS build 2134310f0be8 ***
   [00:00:00.001,000] <inf> pm_i2s_dw: alif_e8_dk (SOFT_OFF): I2S DW PM demo (RUNTIME_IDLE, SUSPEND_TO_IDL)
   [00:00:00.001,000] <inf> pm_i2s_dw: POWER STATE SEQUENCE:
   [00:00:00.001,000] <inf> pm_i2s_dw:   1. PM_STATE_RUNTIME_IDLE
   [00:00:00.001,000] <inf> pm_i2s_dw:   2. PM_STATE_SUSPEND_TO_IDLE
   [00:00:00.001,000] <inf> pm_i2s_dw:   3. PM_STATE_SOFT_OFF
   [00:00:00.001,000] <inf> pm_i2s_dw: [I2S Demo] before RUNTIME_IDLE
   [00:00:01.990,000] <inf> pm_i2s_dw: stream loop exited
   [00:00:01.990,000] <inf> pm_i2s_dw: i2s_stop: RX drop returned 0
   [00:00:01.990,000] <inf> pm_i2s_dw: i2s_stop: TX drop returned 0
   [00:00:02.091,000] <inf> pm_i2s_dw: i2s_stop: done
   [00:00:02.091,000] <inf> pm_i2s_dw: Enter RUNTIME_IDLE sleep for (18000000 microseconds)
   [00:00:20.092,000] <inf> pm_i2s_dw: Exited from RUNTIME_IDLE sleep
   [00:00:20.092,000] <inf> pm_i2s_dw: Request SUSPEND_TO_IDLE for 10000 us
   [00:00:20.103,000] <inf> pm_i2s_dw: [I2S Demo] after SUSPEND_TO_IDLE
   [00:00:22.091,000] <inf> pm_i2s_dw: stream loop exited
   [00:00:22.091,000] <inf> pm_i2s_dw: i2s_stop: RX drop returned 0
   [00:00:22.091,000] <inf> pm_i2s_dw: i2s_stop: TX drop returned 0
   [00:00:22.192,000] <inf> pm_i2s_dw: i2s_stop: done
   [00:00:22.192,000] <inf> pm_i2s_dw: Request SOFT_OFF for 10000000 us (no retention - system resets on w)

   *** Booting Zephyr OS build 2134310f0be8 ***

Notes
*****

* Disconnect the debugger before testing — it prevents cores from entering OFF states.
* I2S is stopped before each PM state and restarted after wakeup (44.1 kHz, 16-bit stereo).
* Deep-sleep durations stay below the 10.7 s PM driver tick ceiling (except RUNTIME_IDLE).

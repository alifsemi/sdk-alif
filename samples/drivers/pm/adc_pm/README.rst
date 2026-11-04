.. _adc-pm-sample:

ADC Power Management Demo
#########################

Overview
********

This sample demonstrates Zephyr power management states combined with Alif
ADC12 conversions on RTSS cores. The application cycles through PM states
and re-reads ADC channel 6 (internal temperature sensor) after each wake,
verifying that the ADC peripheral resumes correctly.

PM states exercised (determined at runtime by capability predicates):

- **S2RAM path** (HE TCM retention): RUNTIME_IDLE → SUSPEND_TO_IDLE →
  S2RAM STANDBY → S2RAM STOP → idle loop
- **SOFT_OFF path** (MRAM boot, no retention): RUNTIME_IDLE → SUSPEND_TO_IDLE →
  SOFT_OFF (system resets on wakeup)

ADC channel 6 is the on-chip temperature sensor, so no external analog
input is required.

.. note::

   Use LPUART port for console logs on E1C DevKits.

Building and Running
********************

HE Core — TCM boot S2RAM (E7/E8/E1C/B1)
=======================================

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/adc_pm
   :board: alif_e7_dk/ae722f80f55d5xx/rtss_he
   :goals: build
   :west-args: -p auto
   :snippets: adc-pm-s2ram-tcm
   :gen-args: -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

HE Core — MRAM boot SOFT_OFF (E7/E8/E1C/B1)
===========================================

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/adc_pm
   :board: alif_e7_dk/ae722f80f55d5xx/rtss_he
   :goals: build
   :west-args: -p auto
   :snippets: adc-pm-mram

HP Core — MRAM boot SOFT_OFF (E7/E8)
====================================

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/adc_pm
   :board: alif_e7_dk/ae722f80f55d5xx/rtss_hp
   :goals: build
   :west-args: -p auto
   :snippets: adc-pm-mram

Sample Output (S2RAM path)
**************************

The output below is from an E7 DK HE core TCM-boot run (``APP_PM_WAKEUP_DEBUG 0``,
the default). Setting ``APP_PM_WAKEUP_DEBUG 1`` in ``main.c`` additionally prints
``PM wakeup: NVIC ISPR[x] = 0x...`` lines on each resume.

.. code-block:: console

   *** Booting Zephyr OS build v4.1.0-607-g7fafce9cf3ee ***
   [00:00:00.000,000] <inf> pm_adc: alif_e7_dk (S2RAM): ADC PM demo (RUNTIME_IDLE, SUSPEND_TO_IDLE, S2RAM STANDBY, S2RAM STOP)
   [00:00:00.000,000] <inf> pm_adc: POWER STATE SEQUENCE:
   [00:00:00.000,000] <inf> pm_adc:   1. PM_STATE_RUNTIME_IDLE
   [00:00:00.000,000] <inf> pm_adc:   2. PM_STATE_SUSPEND_TO_IDLE
   [00:00:00.000,000] <inf> pm_adc:   3. PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY)
   [00:00:00.000,000] <inf> pm_adc:   4. PM_STATE_SUSPEND_TO_RAM (substate 1: STOP)
   [00:00:00.000,000] <inf> pm_adc: [ADC Demo] before RUNTIME_IDLE
   [00:00:00.001,000] <inf> pm_adc: ADC read iter=1 raw=0x43D temp=21.2 C
   [00:00:00.102,000] <inf> pm_adc: ADC read iter=2 raw=0x43D temp=21.2 C
   [00:00:00.203,000] <inf> pm_adc: ADC read iter=3 raw=0x43D temp=21.2 C
   [00:00:00.304,000] <inf> pm_adc: ADC conversion Successfully Completed
   [00:00:00.304,000] <inf> pm_adc: Enter RUNTIME_IDLE sleep for (18000000 microseconds)
   [00:00:18.305,000] <inf> pm_adc: Exited from RUNTIME_IDLE sleep
   [00:00:18.305,000] <inf> pm_adc: Request SUSPEND_TO_IDLE for 10000 us
   [00:00:18.306,000] <inf> pm_adc: PM enter: SUSPEND_TO_IDLE (substate 0)
   [00:00:18.306,000] <inf> pm_adc: PM wakeup: SUSPEND_TO_IDLE (substate 0)
   [00:00:18.306,000] <inf> pm_adc: PM exit:  SUSPEND_TO_IDLE (substate 0)
   [00:00:18.316,000] <inf> pm_adc: [ADC Demo] after SUSPEND_TO_IDLE
   ...
   [00:00:18.620,000] <inf> pm_adc: Request S2RAM STANDBY for 6000000 us
   [00:00:18.721,000] <inf> pm_adc: PM enter: SUSPEND_TO_RAM (substate 0)
   [00:00:18.721,000] <inf> pm_adc: PM wakeup: SUSPEND_TO_RAM (substate 0)
   [00:00:18.721,000] <inf> pm_adc: PM exit:  SUSPEND_TO_RAM (substate 0)
   [00:00:24.722,000] <inf> pm_adc: [ADC Demo] after S2RAM STANDBY
   ...
   [00:00:25.030,000] <inf> pm_adc: Request S2RAM STOP for 9000000 us
   [00:00:25.131,000] <inf> pm_adc: PM enter: SUSPEND_TO_RAM (substate 1)
   [00:00:25.131,000] <inf> pm_adc: PM wakeup: SUSPEND_TO_RAM (substate 1)
   [00:00:25.131,000] <inf> pm_adc: PM exit:  SUSPEND_TO_RAM (substate 1)
   [00:00:34.132,000] <inf> pm_adc: [ADC Demo] after S2RAM STOP
   ...
   [00:00:34.440,000] <inf> pm_adc: === ADC PM SEQUENCE COMPLETED ===

Sample Output (SOFT_OFF path)
*****************************

The output below is from an E7 DK HE core MRAM-boot run. After SOFT_OFF the
system resets and restarts from ``main()``.

.. code-block:: console

   *** Booting Zephyr OS build v4.1.0-607-g7fafce9cf3ee ***
   [00:00:00.000,000] <inf> pm_adc: alif_e7_dk (SOFT_OFF): ADC PM demo (RUNTIME_IDLE, SUSPEND_TO_IDLE, SOFT_OFF)
   [00:00:00.000,000] <inf> pm_adc: POWER STATE SEQUENCE:
   [00:00:00.000,000] <inf> pm_adc:   1. PM_STATE_RUNTIME_IDLE
   [00:00:00.000,000] <inf> pm_adc:   2. PM_STATE_SUSPEND_TO_IDLE
   [00:00:00.000,000] <inf> pm_adc:   3. PM_STATE_SOFT_OFF
   [00:00:00.000,000] <inf> pm_adc: [ADC Demo] before RUNTIME_IDLE
   ...
   [00:00:18.620,000] <inf> pm_adc: Request SOFT_OFF for 10000000 us (no retention - system resets on wakeup)

   *** Booting Zephyr OS build v4.1.0-607-g7fafce9cf3ee ***

Notes
*****

* Disconnect the debugger before testing — it prevents cores from entering OFF states.
* ADC is suspended (conversion stopped, pins in sleep state) before each PM state
  and restored after wakeup.
* Deep-sleep durations stay below the 10.7 s PM driver tick ceiling (except RUNTIME_IDLE).
* Temperature is displayed only when the converted 12-bit value is inside the
  lookup-table range.

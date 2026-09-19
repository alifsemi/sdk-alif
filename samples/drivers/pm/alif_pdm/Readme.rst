. _pdm-pm-sample:

PDM Power Management Demo
#########################

Overview
********

This sample demonstrates Zephyr power management states combined with PDM
microphone capture on Alif RTSS cores. The application records PCM from the
PDM/DMIC interface, cycles through PM states, then records again after each
wake to verify that the PDM block and DMA path resume correctly.

HE cores use LPPDM and PDM. HP cores use PDM.

PM states exercised (determined at runtime by capability predicates):

- **S2RAM path** (HE TCM retention): RUNTIME_IDLE → SUSPEND_TO_IDLE →
  S2RAM STANDBY → S2RAM STOP → idle loop
- **SOFT_OFF path** (MRAM boot, no retention): RUNTIME_IDLE → SUSPEND_TO_IDLE →
  SOFT_OFF (system resets on wakeup)

Building and Running
********************

HE Core — TCM boot S2RAM (E7/E8/E1C/B1)
=========================================

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/alif_pdm
   :board: alif_e7_dk/ae722f80f55d5xx/rtss_he
   :goals: build
   :west-args: -p auto
   :snippets: pdm-pm-s2ram-tcm
   :gen-args: -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

HE Core — MRAM boot SOFT_OFF (E7/E8/E1C/B1)
=============================================

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/alif_pdm
   :board: alif_e7_dk/ae722f80f55d5xx/rtss_he
   :goals: build
   :west-args: -p auto
   :snippets: pdm-pm-mram

HP Core — MRAM boot SOFT_OFF (E7/E8)
======================================

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/alif_pdm
   :board: alif_e7_dk/ae722f80f55d5xx/rtss_hp
   :goals: build
   :west-args: -p auto
   :snippets: pdm-pm-mram

Sample Output (S2RAM path)
The output below is from an E8 DK HE core TCM-boot run

*** Booting Zephyr OS build f002a4d8499c ***
[00:00:00.000,000] <inf> pdm_pm: alif_e8_dk (S2RAM): PDM PM demo (RUNTIME_IDLE, SUSPEND_TO_IDLE, S2RAM STANDBY, S2RAM STOP)
[00:00:00.000,000] <inf> pdm_pm: POWER STATE SEQUENCE:
[00:00:00.000,000] <inf> pdm_pm:   1. PM_STATE_RUNTIME_IDLE
[00:00:00.000,000] <inf> pdm_pm:   2. PM_STATE_SUSPEND_TO_IDLE
[00:00:00.000,000] <inf> pdm_pm:   3. PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY)
[00:00:00.000,000] <inf> pdm_pm:   4. PM_STATE_SUSPEND_TO_RAM (substate 1: STOP)
[00:00:00.000,000] <inf> pdm_pm: === before RUNTIME_IDLE: PDM Audio Recording ===
[00:00:00.003,000] <inf> pdm_pm: Start recording audio...
[00:00:00.003,000] <inf> alif_pdm: PDM path: DMA
[00:00:01.879,000] <inf> pdm_pm: PDM recording completed: 60000 bytes captured
[00:00:01.879,000] <inf> pdm_pm: First 40 bytes of PCM data:
[00:00:01.879,000] <inf> pdm_pm:   00 00 00 00 00 00 00 00
[00:00:01.879,000] <inf> pdm_pm:   00 00 00 00 00 00 00 00
[00:00:01.879,000] <inf> pdm_pm:   00 00 00 00 00 00 00 00
[00:00:01.879,000] <inf> pdm_pm:   ff ff 00 00 00 00 00 00
[00:00:01.879,000] <inf> pdm_pm:   08 00 fb ff 1a 00 fb ff
[00:00:01.879,000] <inf> pdm_pm: Enter RUNTIME_IDLE sleep for (18000000 microseconds)
[00:00:19.880,000] <inf> pdm_pm: Exited from RUNTIME_IDLE sleep
[00:00:19.880,000] <inf> pdm_pm: Enter PM_STATE_SUSPEND_TO_IDLE for (10000 microseconds)
[00:00:19.881,000] <inf> pdm_pm: PM enter: SUSPEND_TO_IDLE (substate 0)
[00:00:19.881,000] <inf> pdm_pm: PM wakeup: SUSPEND_TO_IDLE (substate 0)
[00:00:19.881,000] <inf> pdm_pm: PM exit:  SUSPEND_TO_IDLE (substate 0)
[00:00:19.891,000] <inf> pdm_pm: Exited from PM_STATE_SUSPEND_TO_IDLE
[00:00:19.891,000] <inf> pdm_pm: === after SUSPEND_TO_IDLE: PDM Audio Recording ===
[00:00:19.893,000] <inf> pdm_pm: Start recording audio...
[00:00:19.893,000] <inf> alif_pdm: PDM path: DMA
[00:00:21.769,000] <inf> pdm_pm: PDM recording completed: 60000 bytes captured
[00:00:21.769,000] <inf> pdm_pm: First 40 bytes of PCM data:
[00:00:21.769,000] <inf> pdm_pm:   00 00 00 00 00 00 00 00
[00:00:21.769,000] <inf> pdm_pm:   00 00 00 00 00 00 00 00
[00:00:21.769,000] <inf> pdm_pm:   00 00 00 00 00 00 00 00
[00:00:21.769,000] <inf> pdm_pm:   00 00 00 00 ff ff 00 00
[00:00:21.769,000] <inf> pdm_pm:   00 00 00 00 08 00 fb ff
[00:00:21.769,000] <inf> pdm_pm: Enter PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) for (6000000 microseconds)
[00:00:21.771,000] <inf> pdm_pm: PM enter: SUSPEND_TO_IDLE (substate 0)
[00:00:21.771,000] <inf> pdm_pm: PM wakeup: SUSPEND_TO_IDLE (substate 0)
[00:00:21.771,000] <inf> pdm_pm: PM exit:  SUSPEND_TO_IDLE (substate 0)
[00:00:21.926,000] <inf> pdm_pm: PM enter: SUSPEND_TO_RAM (substate 0)
[00:00:21.926,000] <inf> pdm_pm: PM wakeup: SUSPEND_TO_RAM (substate 0)
[00:00:21.926,000] <inf> pdm_pm: PM exit:  SUSPEND_TO_RAM (substate 0)
[00:00:27.837,000] <inf> pdm_pm: === Resumed from PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) ===
[00:00:27.837,000] <inf> pdm_pm: === after S2RAM STANDBY: PDM Audio Recording ===
[00:00:27.840,000] <inf> pdm_pm: Start recording audio...
[00:00:27.840,000] <inf> alif_pdm: PDM path: DMA
[00:00:29.716,000] <inf> pdm_pm: PDM recording completed: 60000 bytes captured
[00:00:29.716,000] <inf> pdm_pm: First 40 bytes of PCM data:
[00:00:29.716,000] <inf> pdm_pm:   00 00 00 00 00 00 00 00
[00:00:29.716,000] <inf> pdm_pm:   00 00 00 00 00 00 00 00
[00:00:29.716,000] <inf> pdm_pm:   00 00 00 00 00 00 00 00
[00:00:29.716,000] <inf> pdm_pm:   00 00 00 00 ff ff 00 00
[00:00:29.716,000] <inf> pdm_pm:   00 00 00 00 08 00 fb ff
[00:00:29.716,000] <inf> pdm_pm: Enter PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) for (9000000 microseconds)
[00:00:29.718,000] <inf> pdm_pm: PM enter: SUSPEND_TO_IDLE (substate 0)
[00:00:29.718,000] <inf> pdm_pm: PM wakeup: SUSPEND_TO_IDLE (substate 0)
[00:00:29.718,000] <inf> pdm_pm: PM exit:  SUSPEND_TO_IDLE (substate 0)
[00:00:29.872,000] <inf> pdm_pm: PM enter: SUSPEND_TO_RAM (substate 1)
[00:00:29.872,000] <inf> pdm_pm: PM wakeup: SUSPEND_TO_RAM (substate 1)
[00:00:29.872,000] <inf> pdm_pm: PM exit:  SUSPEND_TO_RAM (substate 1)
[00:00:38.777,000] <inf> pdm_pm: === Resumed from PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) ===
[00:00:38.777,000] <inf> pdm_pm: === after S2RAM STOP: PDM Audio Recording ===
[00:00:38.780,000] <inf> pdm_pm: Start recording audio...
[00:00:38.780,000] <inf> alif_pdm: PDM path: DMA
[00:00:40.656,000] <inf> pdm_pm: PDM recording completed: 60000 bytes captured
[00:00:40.656,000] <inf> pdm_pm: First 40 bytes of PCM data:
[00:00:40.656,000] <inf> pdm_pm:   00 00 00 00 00 00 00 00
[00:00:40.656,000] <inf> pdm_pm:   00 00 00 00 00 00 00 00
[00:00:40.656,000] <inf> pdm_pm:   00 00 00 00 00 00 00 00
[00:00:40.656,000] <inf> pdm_pm:   00 00 00 00 ff ff 00 00
[00:00:40.656,000] <inf> pdm_pm:   00 00 00 00 08 00 fb ff
[00:00:40.656,000] <inf> pdm_pm: === PDM PM SEQUENCE COMPLETED ===

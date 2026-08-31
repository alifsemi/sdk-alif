. _qdec-pm-sample:

UTimer QDEC Power Management Demo
#################################

Overview
********.

This sample demonstrates Zephyr power management states combined with
Alif uTimer QDEC on RTSS cores. GPIO pins emulate a quadrature encoder;
after each wake the application steps the emulator and reads rotation to
verify that the QDEC driver resumed correctly.

PM states exercised (determined at runtime by capability predicates):

- **S2RAM path** (HE TCM retention): RUNTIME_IDLE → SUSPEND_TO_IDLE →
  S2RAM STANDBY → S2RAM STOP → idle loop
- **SOFT_OFF path** (MRAM boot, no retention): RUNTIME_IDLE → SUSPEND_TO_IDLE →
  SOFT_OFF (system resets on wakeup)

A snippet is required. SoC DTS leaves cpu-power-states and AIPM off
profiles disabled. The snippet enables the states, off profiles, wakeup
source, and retained memory for that boot path.

QDEC and GPIO emulator
======================

The application uses **UTIMER1** in QDEC mode. Two GPIO outputs bit-bang
a quadrature Gray code (phase A leads phase B). Jumper wires connect the
GPIO outputs to the UTIMER QDEC input pins.

+------------------+---------------------------+---------------------------+
| Role             | Ensemble E7/E8            | E1C / B1                  |
+==================+===========================+===========================+
| Emulator phase A | ``P0_0`` (GPIO0 pin 0)    | ``P2_4`` (GPIO2 pin 4)    |
| Emulator phase B | ``P0_1`` (GPIO0 pin 1)    | ``P2_5`` (GPIO2 pin 5)    |
| QDEC input T0/A  | ``P0_2`` (UT1_T0_A)       | ``P6_2`` (UT1_T0_B)       |
| QDEC input T1/B  | ``P0_3`` (UT1_T1_A)       | ``P6_3`` (UT1_T1_B)       |
+------------------+---------------------------+---------------------------+

**Jumper loopback (required):**

* Ensemble E7/E8: ``P0_0`` → ``P0_2`` (phase A), ``P0_1`` → ``P0_3`` (phase B)
* E1C / B1: ``P2_4`` → ``P6_2`` (phase A), ``P2_5`` → ``P6_3`` (phase B)

Without jumpers the QDEC count stays at zero.

Building and Running
********************

HE Core — TCM boot S2RAM (E8)
==============================

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/qdec_alif
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_he
   :goals: build
   :west-args: -p auto
   :snippets: qdec-pm-s2ram-tcm
   :gen-args: -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

HE Core — MRAM boot SOFT_OFF (E8)
==================================

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/qdec_alif
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_he
   :goals: build
   :west-args: -p auto
   :snippets: qdec-pm-mram

HP Core — MRAM boot SOFT_OFF (E8)
==================================

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/qdec_alif
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_hp
   :goals: build
   :west-args: -p auto
   :snippets: qdec-pm-mram

Other boards
============

Use the same snippets and ``-DCONFIG_FLASH_*`` arguments as above; replace the
board target only:

* **E7 HE TCM / MRAM:** ``alif_e7_dk/ae722f80f55d5xx/rtss_he``
* **E7 HP MRAM:** ``alif_e7_dk/ae722f80f55d5xx/rtss_hp``
* **E1C / B1 HE TCM / MRAM:** ``alif_e1c_dk/.../rtss_he`` or ``alif_b1_dk/.../rtss_he``
  (use the part-specific board string from your DK)

Sample Output (S2RAM path)
**************************

The output below is from an E8 DK HE core TCM-boot run (``APP_PM_WAKEUP_DEBUG 0``,
the default). Setting ``APP_PM_WAKEUP_DEBUG 1`` in ``main.c`` additionally prints
``PM wakeup: NVIC ISPR[x] = 0x...`` lines on each resume.

.. code-block:: console

   *** Booting Zephyr OS build 2fa5235f4bf3 ***
   [00:00:00.001,000] <inf> qdec_pm: GPIO encoder emulator ready
   [00:00:00.001,000] <inf> qdec_pm: alif_e8_dk (S2RAM): QDEC PM demo (RUNTIME_IDLE, SUSPEND_TO_IDLE, S2RAM STANDBY, S2RAM STOP)
   [00:00:00.001,000] <inf> qdec_pm: POWER STATE SEQUENCE:
   [00:00:00.001,000] <inf> qdec_pm:   1. PM_STATE_RUNTIME_IDLE
   [00:00:00.001,000] <inf> qdec_pm:   2. PM_STATE_SUSPEND_TO_IDLE
   [00:00:00.001,000] <inf> qdec_pm:   3. PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY)
   [00:00:00.001,000] <inf> qdec_pm:   4. PM_STATE_SUSPEND_TO_RAM (substate 1: STOP)
   [00:00:00.001,000] <inf> qdec_pm: === before RUNTIME_IDLE: QDEC read ===
   [00:00:00.011,000] <inf> qdec_pm: sample 1/5: position 36 deg
   [00:00:00.021,000] <inf> qdec_pm: sample 2/5: position 72 deg
   [00:00:00.031,000] <inf> qdec_pm: sample 3/5: position 108 deg
   [00:00:00.042,000] <inf> qdec_pm: sample 4/5: position 144 deg
   [00:00:00.052,000] <inf> qdec_pm: sample 5/5: position 180 deg
   [00:00:00.052,000] <inf> qdec_pm: Enter RUNTIME_IDLE sleep for (18000000 microseconds)
   [00:00:18.053,000] <inf> qdec_pm: Exited from RUNTIME_IDLE sleep
   [00:00:18.053,000] <inf> qdec_pm: Enter PM_STATE_SUSPEND_TO_IDLE for (10000 microseconds)
   [00:00:18.054,000] <inf> qdec_pm: PM enter: SUSPEND_TO_IDLE (substate 0)
   [00:00:18.054,000] <inf> qdec_pm: PM exit:  SUSPEND_TO_IDLE (substate 0)
   [00:00:18.064,000] <inf> qdec_pm: Exited from PM_STATE_SUSPEND_TO_IDLE
   [00:00:18.064,000] <inf> qdec_pm: === after SUSPEND_TO_IDLE: QDEC read ===
   [00:00:18.074,000] <inf> qdec_pm: sample 1/5: position 216 deg
   [00:00:18.084,000] <inf> qdec_pm: sample 2/5: position 252 deg
   [00:00:18.094,000] <inf> qdec_pm: sample 3/5: position 288 deg
   [00:00:18.104,000] <inf> qdec_pm: sample 4/5: position 324 deg
   [00:00:18.114,000] <inf> qdec_pm: sample 5/5: position 0 deg
   [00:00:18.114,000] <inf> qdec_pm: Enter PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) for (6000000 microseconds)
   [00:00:18.276,000] <inf> qdec_pm: PM enter: SUSPEND_TO_RAM (substate 0)
   [00:00:18.276,000] <inf> qdec_pm: PM wakeup: SUSPEND_TO_RAM (substate 0)
   [00:00:18.276,000] <inf> qdec_pm: PM exit:  SUSPEND_TO_RAM (substate 0)
   [00:00:24.188,000] <inf> qdec_pm: === Resumed from PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) ===
   [00:00:24.188,000] <inf> qdec_pm: === after S2RAM STANDBY: QDEC read ===
   [00:00:24.198,000] <inf> qdec_pm: sample 1/5: position 36 deg
   [00:00:24.208,000] <inf> qdec_pm: sample 2/5: position 72 deg
   [00:00:24.218,000] <inf> qdec_pm: sample 3/5: position 108 deg
   [00:00:24.229,000] <inf> qdec_pm: sample 4/5: position 144 deg
   [00:00:24.239,000] <inf> qdec_pm: sample 5/5: position 180 deg
   [00:00:24.239,000] <inf> qdec_pm: Enter PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) for (9000000 microseconds)
   [00:00:24.400,000] <inf> qdec_pm: PM enter: SUSPEND_TO_RAM (substate 1)
   [00:00:24.400,000] <inf> qdec_pm: PM wakeup: SUSPEND_TO_RAM (substate 1)
   [00:00:24.400,000] <inf> qdec_pm: PM exit:  SUSPEND_TO_RAM (substate 1)
   [00:00:33.321,000] <inf> qdec_pm: === Resumed from PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) ===
   [00:00:33.321,000] <inf> qdec_pm: GPIO encoder emulator ready
   [00:00:33.321,000] <inf> qdec_pm: === after S2RAM STOP: QDEC read ===
   [00:00:33.331,000] <inf> qdec_pm: sample 1/5: position 36 deg
   [00:00:33.341,000] <inf> qdec_pm: sample 2/5: position 72 deg
   [00:00:33.352,000] <inf> qdec_pm: sample 3/5: position 108 deg
   [00:00:33.362,000] <inf> qdec_pm: sample 4/5: position 144 deg
   [00:00:33.372,000] <inf> qdec_pm: sample 5/5: position 180 deg
   [00:00:33.372,000] <inf> qdec_pm: === QDEC PM SEQUENCE COMPLETED ===

Sample Output (SOFT_OFF path)
*****************************

On MRAM boot the application exercises RUNTIME_IDLE, SUSPEND_TO_IDLE (when the
LPM idle timer is configured), and SOFT_OFF. After SOFT_OFF the system resets
and restarts from ``main()``. The final ``LOG_ERR`` / ``__ASSERT`` after
SOFT_OFF should not be reached on a successful run.

Notes
*****

* Disconnect the debugger before testing — it can block OFF states.
* QDEC pinmux and encoder-emulate GPIOs come from the snippet overlays.
* ``SUSPEND_TO_IDLE`` needs the LPM idle timer (RTC on HE TCM/MRAM snippets).
* Position is reported in integer degrees (``counts * 360 / counts-per-revolution``);
  values wrap at 360° (for example, 324° + 36° displays as 0°).
* Deep-sleep durations stay below the 10.7 s PM driver tick ceiling (except
  RUNTIME_IDLE).

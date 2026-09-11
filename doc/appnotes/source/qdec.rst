.. _appnote-zas-qdec:

====
QDEC
====

Introduction
============

The Alif UTIMER IP on the Alif DevKit supports Quadrature Decoder (QDEC) mode, enabling precise position tracking of a mechanical rotary encoder. This mode is ideal for applications requiring angular position feedback, such as motor control, robotics, or user interface dials. This application note guides developers through configuring, building, and running a Zephyr-based QDEC application (``samples/sensor/qdec/``) using the UTIMER peripheral on the Alif DevKit.

.. include:: prerequisites.rst

.. include:: note.rst

Build a QDEC Application with Zephyr
=====================================

Follow these steps to build the QDEC application using the Alif Zephyr SDK:

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, refer to the `ZAS User Guide`_.

.. note::
   The build commands shown here are specifically for the Alif E7 DevKit.
   To build the application for other boards, modify the board name in the build command accordingly. For more information, refer to the `ZAS User Guide`_, under the section ``Setting Up and Building Zephyr Applications``.

2. Build command for application on the M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     samples/sensor/qdec/ \
     -S alif-qdec

3. Build command for application on the M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     samples/sensor/qdec/ \
     -S alif-qdec

Once the build command completes successfully, executable images will be generated and placed in the ``build/zephyr`` directory. Both ``.bin`` (binary) and ``.elf`` (Executable and Linkable Format) files will be available.

Executing Binary on the DevKit
=================================

To execute binaries on the DevKit, follow the command:

.. code-block:: console

   west flash

Expected Result
===============

Once the application is loaded and the mechanical encoder is connected:

- The DevKit runs the QDEC sample, reading the UTIMER counter in quadrature decoder mode.
- The angular position is printed every second to the console via UART4 (M55 HE core).

Console Output
===============

.. code-block:: text

   Quadrature decoder sensor test
   Quadrature encoder emulator enabled with 100 ms period
   Position = 0 degrees
   Position = 7 degrees
   Position = 14 degrees
   Position = 21 degrees
   Position = 28 degrees
   Position = 36 degrees
   Position = 43 degrees
   Position = 50 degrees
   Position = 57 degrees
   Position = 64 degrees
   Position = 72 degrees
   Position = 79 degrees
   Position = 86 degrees
   Position = 93 degrees
   …

PM Support
==========

The QDEC sample supports Zephyr Power Management (PM) states on Alif
Ensemble and Balletto RTSS cores. GPIO pins emulate a quadrature encoder.
After each wake, the application steps the emulator and reads rotation to
verify that the QDEC driver resumed correctly.

The following PM states are supported:

* **PM_STATE_RUNTIME_IDLE**: Light sleep. The CPU clock is gated using WFI
  for quick wakeup.
* **PM_STATE_SUSPEND_TO_IDLE**: CPU sleep with IWIC. Devices remain active
  without device-PM overhead. An LPM idle timer is required.
* **PM_STATE_SUSPEND_TO_RAM (S2RAM)**: Deep sleep with retention on the HE
  core when booting from TCM.

  * Substate 0: STANDBY
  * Substate 1: STOP

* **PM_STATE_SOFT_OFF**: Deepest sleep without retention. The system performs
  a full reset on wakeup.

PM states exercised (determined at runtime by capability predicates):

- **S2RAM path** (HE TCM retention): ``RUNTIME_IDLE`` → ``SUSPEND_TO_IDLE`` →
  S2RAM STANDBY → S2RAM STOP → idle loop
- **SOFT_OFF path** (MRAM boot, no retention): ``RUNTIME_IDLE`` → ``SUSPEND_TO_IDLE`` →
  ``SOFT_OFF`` (system resets on wakeup)

A snippet is required because SoC DTS leaves cpu-power-states and AIPM off
profiles disabled. The snippet enables the states, off profiles, wakeup
source, and retained memory for that boot path.

QDEC and GPIO emulator
----------------------

The application uses **UTIMER1** in QDEC mode. Two GPIO outputs bit-bang
a quadrature Gray code (phase A leads phase B). Jumper wires connect the
GPIO outputs to the UTIMER QDEC input pins.

.. list-table::
   :header-rows: 1
   :widths: 20 40 40
   :align: left

   * - Role
     - Ensemble E7/E8
     - E1C / B1
   * - Emulator phase A
     - ``P0_0`` (GPIO0 pin 0)
     - ``P2_4`` (GPIO2 pin 4)
   * - Emulator phase B
     - ``P0_1`` (GPIO0 pin 1)
     - ``P2_5`` (GPIO2 pin 5)
   * - QDEC input T0/A
     - ``P0_2`` (UT1_T0_A)
     - ``P6_2`` (UT1_T0_B)
   * - QDEC input T1/B
     - ``P0_3`` (UT1_T1_A)
     - ``P6_3`` (UT1_T1_B)

**Jumper loopback (required):**

- Ensemble E7/E8: ``P0_0`` → ``P0_2`` (phase A), ``P0_1`` → ``P0_3`` (phase B)
- E1C / B1: ``P2_4`` → ``P6_2`` (phase A), ``P2_5`` → ``P6_3`` (phase B)

Without jumpers, the QDEC count stays at zero.

Building and Running the PM Sample
-----------------------------------

Follow these steps to build the PM sample application using the Alif Zephyr SDK:

For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, refer to the `ZAS User Guide`_.

.. note::
   The build commands shown here are specifically for the Alif E8 DevKit.
   To build the application for other boards, modify the board name in the build command accordingly. For more information, refer to the `ZAS User Guide`_, under the section ``Setting Up and Building Zephyr Applications``.

HE Core — TCM Boot S2RAM (E8)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Build the QDEC PM sample for the HE core using the following command:

.. code-block:: console

   west build -p auto \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/qdec_alif \
     -S qdec-pm-s2ram-tcm \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

HE Core — MRAM Boot SOFT_OFF (E8)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Build the QDEC PM sample for the HE core using the following command:

.. code-block:: console

   west build -p auto \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/qdec_alif \
     -S qdec-pm-mram

HP Core — MRAM Boot SOFT_OFF (E8)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Build the QDEC PM sample for the HP core using the following command:

.. code-block:: console

   west build -p auto \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/pm/qdec_alif \
     -S qdec-pm-mram

Use the same snippets and ``-DCONFIG_FLASH_*`` arguments as above, and replace only the board target. For E7 HE TCM or MRAM boot, use ``alif_e7_dk/ae722f80f55d5xx/rtss_he``. For E7 HP MRAM boot, use ``alif_e7_dk/ae722f80f55d5xx/rtss_hp``. For E1C or B1 HE TCM or MRAM boot, use ``alif_e1c_dk/.../rtss_he`` or ``alif_b1_dk/.../rtss_he``, with the part-specific board string from your DevKit.

PM Test Sequence
----------------

The S2RAM PM test performs the following sequence:

1. Read the QDEC position before entering the PM states.
2. Enter ``PM_STATE_RUNTIME_IDLE`` and wake up.
3. Enter ``PM_STATE_SUSPEND_TO_IDLE`` and wake up.
4. Read the QDEC position after ``SUSPEND_TO_IDLE``.
5. Enter ``PM_STATE_SUSPEND_TO_RAM`` with substate 0 (STANDBY) and wake up.
6. Read the QDEC position after S2RAM STANDBY.
7. Enter ``PM_STATE_SUSPEND_TO_RAM`` with substate 1 (STOP) and wake up.
8. Read the QDEC position after S2RAM STOP.
9. Complete the PM sequence.

The SOFT_OFF configuration follows the ``RUNTIME_IDLE`` and ``SUSPEND_TO_IDLE`` states before entering ``PM_STATE_SOFT_OFF``. On an MRAM boot, the application exercises ``RUNTIME_IDLE``, ``SUSPEND_TO_IDLE`` (when the LPM idle timer is configured), and ``SOFT_OFF``. After ``SOFT_OFF``, the system resets and restarts from ``main()``. The final ``LOG_ERR`` / ``__ASSERT`` after ``SOFT_OFF`` should not be reached on a successful run.

PM Support Verification
-----------------------

After each supported PM state, the application steps the encoder emulator and reads the QDEC position to check that the QDEC driver resumed correctly.

A successful S2RAM test displays messages similar to the following. This log is from an E8 DevKit HE core TCM-boot run with ``APP_PM_WAKEUP_DEBUG`` set to 0, which is the default. Setting ``APP_PM_WAKEUP_DEBUG`` to 1 in ``main.c`` also prints ``PM wakeup: NVIC ISPR[x] = 0x...`` on each resume.

.. code-block:: console

   *** Booting Zephyr OS build ***
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

Notes
-----

* Disconnect the debugger before testing. It can block OFF states.
* QDEC pinmux and encoder-emulate GPIOs come from the snippet overlays.
* ``SUSPEND_TO_IDLE`` needs the LPM idle timer (RTC on HE TCM/MRAM snippets).
* Position is reported in integer degrees
  (``counts * 360 / counts-per-revolution``). Values wrap at 360°
  (for example, 324° + 36° displays as 0°).
* Deep-sleep durations stay below the 10.7 s PM driver tick ceiling
  (except ``RUNTIME_IDLE``).

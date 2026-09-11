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

The ``samples/drivers/pm/system_off_qdec`` sample demonstrates Zephyr
power management states combined with Alif UTIMER QDEC on RTSS cores.
GPIO pins emulate a quadrature encoder. After each wake, the application
steps the emulator and reads rotation to verify that the QDEC driver
resumed correctly.

PM states exercised (determined at runtime by capability predicates):

- **S2RAM path** (HE TCM retention): RUNTIME_IDLE → SUSPEND_TO_IDLE →
  S2RAM STANDBY → S2RAM STOP → idle loop
- **SOFT_OFF path** (MRAM boot, no retention): RUNTIME_IDLE → SUSPEND_TO_IDLE →
  SOFT_OFF (system resets on wakeup)

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

Disconnect the debugger before testing, because it can block OFF states.

Building and Running the PM Sample
----------------------------------

Follow these steps to build the PM sample application using the Alif Zephyr SDK:

For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, refer to the `ZAS User Guide`_.

.. note::
   The build commands shown here are specifically for the Alif E8 DevKit.
   To build the application for other boards, modify the board name in the build command accordingly. For more information, refer to the `ZAS User Guide`_, under the section ``Setting Up and Building Zephyr Applications``.

HE Core — TCM boot S2RAM (E8)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/system_off_qdec \
     -S qdec-pm-s2ram-tcm \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

HE Core — MRAM boot SOFT_OFF (E8)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/system_off_qdec \
     -S qdec-pm-mram

HP Core — MRAM boot SOFT_OFF (E8)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/pm/system_off_qdec \
     -S qdec-pm-mram

Use the same snippets and ``-DCONFIG_FLASH_*`` arguments as above; replace the
board target only:

- **E7 HE TCM / MRAM:** ``alif_e7_dk/ae722f80f55d5xx/rtss_he``
- **E7 HP MRAM:** ``alif_e7_dk/ae722f80f55d5xx/rtss_hp``
- **E1C HE TCM / MRAM:** ``alif_e1c_dk/ae1c1f4051920hh/rtss_he``
- **B1 HE TCM / MRAM:** ``alif_b1_dk/ab1c1f4m51820hh0/rtss_he``

PM Support Verification
-----------------------

The output below is from an E8 DK HE core TCM-boot run
(``APP_PM_WAKEUP_DEBUG 0``, the default). Setting
``APP_PM_WAKEUP_DEBUG 1`` in ``main.c`` additionally prints
``PM wakeup: NVIC ISPR[x] = 0x...`` lines on each resume.

A successful S2RAM test displays messages similar to the following:

.. code-block:: console

   *** Booting Zephyr OS build ***
   [00:00:00.001,000] <inf> qdec_pm: GPIO encoder emulator ready
   [00:00:00.001,000] <inf> qdec_pm: alif_e8_dk (S2RAM): QDEC PM demo (RUNTIME_IDLE, SUSPEND_TO_IDLE, S2RAM STANDBY, S2RAM STOP)
   [00:00:00.001,000] <inf> qdec_pm: POWER STATE SEQUENCE:
   [00:00:00.001,000] <inf> qdec_pm:   1. PM_STATE_RUNTIME_IDLE
   [00:00:00.001,000] <inf> qdec_pm:   2. PM_STATE_SUSPEND_TO_IDLE
   [00:00:00.001,000] <inf> qdec_pm:   3. PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY)
   [00:00:00.001,000] <inf> qdec_pm:   4. PM_STATE_SUSPEND_TO_RAM (substate 1: STOP)
   [00:00:24.188,000] <inf> qdec_pm: === Resumed from PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) ===
   [00:00:33.321,000] <inf> qdec_pm: === Resumed from PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) ===
   [00:00:33.372,000] <inf> qdec_pm: === QDEC PM SEQUENCE COMPLETED ===

On MRAM boot the application exercises RUNTIME_IDLE, SUSPEND_TO_IDLE (when the
LPM idle timer is configured), and SOFT_OFF. After SOFT_OFF the system resets
and restarts from ``main()``.

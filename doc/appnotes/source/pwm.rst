.. _appnote-zephyr-pwm:

===
PWM
===

Introduction
============

The Alif UTIMER IP serves to generate PWM (Pulse Width Modulation) signals on the Alif DevKit. It allows configuration of the first 12 UTIMER channels to produce 2 PWM signals each, resulting in a total of 24 signals simultaneously. Each UTIMER instance incorporates 2 compare blocks dedicated to PWM signal generation.

Driver Description
==================

The PWM driver is functional within the Zephyr framework, but the PWM Capture mode feature has not yet been implemented and will be added in a future release. Sample applications, such as `fade_led` and `blinky_pwm`, have been integrated with the PWM driver and tested successfully.

Currently, LED0 (Green) is used for PWM output on the HP core, and LED1 (Red) is used for PWM output on the HE core in these applications. For debugging and output, UART2 is used for the M55 HP core, while UART4 is used for the M55 HE core.

.. include:: prerequisites.rst

.. include:: note.rst

Build a PWM Application with Zephyr
===================================

Follow these steps to build the `fade_led` and `blinky_pwm` applications using the PWM driver and the west tool. The following commands are used to build the image with the GCC compiler on ITCM memory:

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, refer to the `ZAS User Guide`_.

.. note::
   The build commands shown here are specifically for the Alif E7 DevKit.
   To build the application for other boards, modify the board name in the build command accordingly. For more information, refer to the `ZAS User Guide`_, under the section ``Setting Up and Building Zephyr Applications``.

2. Build command for fade_led application on the M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     samples/basic/fade_led/ \
     -S alif-fade-led

3. Build command for blinky_pwm application on the M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     samples/basic/blinky_pwm/ \
     -S alif-blinky-pwm

4. Build command for fade_led application on the M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     samples/basic/fade_led/ \
     -S alif-fade-led

5. Build command for blinky_pwm application on the M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     samples/basic/blinky_pwm/ \
     -S alif-blinky-pwm

Once the build command completes successfully, executable images will be generated and placed in the ``build/zephyr`` directory. Both ``.bin`` (binary) and ``.elf`` (Executable and Linkable Format) files will be available.

Executing Binary on the DevKit
================================

To execute binaries on the DevKit, follow the command:

.. code-block:: console

   west flash

Console Output
===============

.. list-table::
   :header-rows: 1
   :widths: 50 50

   * - **fade_led**
     - **blinky_pwm**

   * - .. code-block:: text

          PWM-based LED fade. Found 1 LEDs
          LED 0: Using pulse width 0%
          LED 0: Using pulse width 2%
          LED 0: Using pulse width 4%
          LED 0: Using pulse width 6%
          LED 0: Using pulse width 8%
          LED 0: Using pulse width 10%
          LED 0: Using pulse width 12%
          LED 0: Using pulse width 14%
          LED 0: Using pulse width 16%
          LED 0: Using pulse width 18%
          LED 0: Using pulse width 20%
          LED 0: Using pulse width 22%
          LED 0: Using pulse width 24%
          LED 0: Using pulse width 26%
          LED 0: Using pulse width 28%
          LED 0: Using pulse width 30%
          LED 0: Using pulse width 32%
          LED 0: Using pulse width 34%
          LED 0: Using pulse width 36%
          LED 0: Using pulse width 38%
          LED 0: Using pulse width 40%
          LED 0: Using pulse width 42%
          LED 0: Using pulse width 44%
          LED 0: Using pulse width 46%
          LED 0: Using pulse width 48%
          LED 0: Using pulse width 50%
          LED 0: Using pulse width 52%
          LED 0: Using pulse width 54%
          LED 0: Using pulse width 56%
          LED 0: Using pulse width 58%
          LED 0: Using pulse width 60%
          LED 0: Using pulse width 62%

     - .. code-block:: text

          PWM-based blinky
          Calibrating for channel 1...
          Done calibrating; maximum/minimum periods 1000000000/7812500 nsec
          Using period 1000000000
          Using period 500000000
          Using period 250000000
          Using period 125000000
          Using period 62500000
          Using period 31250000
          Using period 15625000
          Using period 7812500
          Using period 15625000
          Using period 31250000
          Using period 62500000
          Using period 125000000
          Using period 250000000
          Using period 500000000
          Using period 1000000000
          Using period 500000000
          Using period 250000000
          Using period 125000000
          Using period 62500000
          Using period 31250000
          Using period 15625000
          Using period 7812500
          Using period 15625000
          Using period 31250000
          Using period 62500000
          Using period 125000000
          Using period 250000000
          Using period 500000000
          Using period 1000000000
          Using period 500000000
          Using period 250000000
          Using period 1250000

PM Support
==========

The PWM sample supports Zephyr Power Management (PM) states on Alif
Ensemble and Balletto RTSS cores. The application verifies that the
``pwm_alif_utimer`` driver continues to generate the PWM waveform after
each supported PM transition.

The following PM states are supported:

* **PM_STATE_RUNTIME_IDLE**: Light sleep. The CPU clock is gated using WFI
  for quick wakeup.
* **PM_STATE_SUSPEND_TO_IDLE**: CPU sleep with IWIC. Devices remain active
  without device-PM overhead.
* **PM_STATE_SUSPEND_TO_RAM (S2RAM)**: Deep sleep with retention on the HE
  core when booting from TCM.

  * Substate 0: STANDBY
  * Substate 1: STOP

* **PM_STATE_SOFT_OFF**: Deepest sleep without retention. The system performs
  a full reset on wakeup.

The PWM fade runs before entering the PM states and again after each
wakeup. This verifies that the PWM driver suspends and resumes correctly
with the device.

HE cores support the S2RAM path when booting from TCM. The HE core can also
use the SOFT_OFF path when booting from MRAM. HP cores use the SOFT_OFF path.

Build the PM Sample Application
---------------------------------

Follow these steps to build the PM Sample application using the Alif Zephyr SDK:

For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, refer to the `ZAS User Guide`_.

.. note::
   The build commands shown here are specifically for the Alif E8 DevKit.
   To build the application for other boards, modify the board name in the build command accordingly. For more information, refer to the `ZAS User Guide`_, under the section ``Setting Up and Building Zephyr Applications``.

HE Core — TCM Boot S2RAM (E8)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Build the PWM PM sample for the HE core using the following command:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/pwm-alif \
     -S pwm-pm-s2ram-tcm \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

HE Core — MRAM Boot SOFT_OFF (E8)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Build the PWM PM sample for the HE core using the following command:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/pwm-alif \
     -S pwm-pm-mram

HP Core — MRAM Boot SOFT_OFF (E8)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Build the PWM PM sample for the HP core using the following command:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/pm/pwm-alif \
     -S pwm-pm-mram

PM Test Sequence
----------------

The S2RAM PM test performs the following sequence:

1. Run the PWM fade before entering the PM states.
2. Enter ``PM_STATE_RUNTIME_IDLE`` and wake up.
3. Enter ``PM_STATE_SUSPEND_TO_IDLE`` and wake up.
4. Run the PWM fade after ``SUSPEND_TO_IDLE``.
5. Enter ``PM_STATE_SUSPEND_TO_RAM`` with substate 0 (STANDBY) and wake up.
6. Run the PWM fade after S2RAM STANDBY.
7. Enter ``PM_STATE_SUSPEND_TO_RAM`` with substate 1 (STOP) and wake up.
8. Run the PWM fade after S2RAM STOP.
9. Complete the PM sequence.

The SOFT_OFF configuration follows the supported PM sequence before
entering ``PM_STATE_SOFT_OFF``. The system resets when it wakes from
``SOFT_OFF``.

PM Support Verification
-----------------------

After each supported PM state, the application runs the PWM fade and checks
that the PWM driver continues to operate correctly.

A successful S2RAM test displays messages similar to the following:

.. code-block:: console

   *** Booting Zephyr OS build ***
   [00:00:00.000,000] <inf> pwm_pm: alif_e8_dk (S2RAM): PWM PM demo (RUNTIME_IDLE, SUSPEND_TO_IDLE, S2RAM STANDBY, S2RAM STOP)
   [00:00:00.000,000] <inf> pwm_pm: POWER STATE SEQUENCE:
   [00:00:00.000,000] <inf> pwm_pm:   1. PM_STATE_RUNTIME_IDLE
   [00:00:00.000,000] <inf> pwm_pm:   2. PM_STATE_SUSPEND_TO_IDLE
   [00:00:00.000,000] <inf> pwm_pm:   3. PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY)
   [00:00:00.000,000] <inf> pwm_pm:   4. PM_STATE_SUSPEND_TO_RAM (substate 1: STOP)
   [00:00:00.000,000] <inf> pwm_pm: === before RUNTIME_IDLE: PWM fade ===
   [00:00:03.016,000] <inf> pwm_pm: fade done, 1 LED(s) idle
   [00:00:03.016,000] <inf> pwm_pm: Enter RUNTIME_IDLE sleep for (18000000 microseconds)
   [00:00:21.017,000] <inf> pwm_pm: Exited from RUNTIME_IDLE sleep
   [00:00:21.017,000] <inf> pwm_pm: Enter PM_STATE_SUSPEND_TO_IDLE for (10000 microseconds)
   [00:00:21.018,000] <inf> pwm_pm: PM enter: SUSPEND_TO_IDLE (substate 0)
   [00:00:21.018,000] <inf> pwm_pm: PM exit:  SUSPEND_TO_IDLE (substate 0)
   [00:00:21.028,000] <inf> pwm_pm: Exited from PM_STATE_SUSPEND_TO_IDLE
   [00:00:21.028,000] <inf> pwm_pm: === after SUSPEND_TO_IDLE: PWM fade ===
   [00:00:24.044,000] <inf> pwm_pm: fade done, 1 LED(s) idle
   [00:00:24.044,000] <inf> pwm_pm: Enter PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) for (6000000 microseconds)
   [00:00:24.045,000] <inf> pwm_pm: PM enter: SUSPEND_TO_IDLE (substate 0)
   [00:00:24.045,000] <inf> pwm_pm: PM exit:  SUSPEND_TO_IDLE (substate 0)
   [00:00:24.111,000] <inf> pwm_pm: PM enter: SUSPEND_TO_RAM (substate 0)
   [00:00:24.111,000] <inf> pwm_pm: PM wakeup: SUSPEND_TO_RAM (substate 0)
   [00:00:24.111,000] <inf> pwm_pm: PM exit:  SUSPEND_TO_RAM (substate 0)
   [00:00:30.068,000] <inf> pwm_pm: === Resumed from PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) ===
   [00:00:30.068,000] <inf> pwm_pm: === after S2RAM STANDBY: PWM fade ===
   [00:00:33.084,000] <inf> pwm_pm: fade done, 1 LED(s) idle
   [00:00:33.084,000] <inf> pwm_pm: Enter PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) for (9000000 microseconds)
   [00:00:33.085,000] <inf> pwm_pm: PM enter: SUSPEND_TO_IDLE (substate 0)
   [00:00:33.085,000] <inf> pwm_pm: PM exit:  SUSPEND_TO_IDLE (substate 0)
   [00:00:33.151,000] <inf> pwm_pm: PM enter: SUSPEND_TO_RAM (substate 1)
   [00:00:33.151,000] <inf> pwm_pm: PM wakeup: SUSPEND_TO_RAM (substate 1)
   [00:00:33.151,000] <inf> pwm_pm: PM exit:  SUSPEND_TO_RAM (substate 1)
   [00:00:42.101,000] <inf> pwm_pm: === Resumed from PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) ===
   [00:00:42.101,000] <inf> pwm_pm: === after S2RAM STOP: PWM fade ===
   [00:00:45.117,000] <inf> pwm_pm: fade done, 1 LED(s) idle
   [00:00:45.117,000] <inf> pwm_pm: === PWM PM SEQUENCE COMPLETED ===

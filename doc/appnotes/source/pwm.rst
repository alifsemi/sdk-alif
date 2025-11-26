.. _appnote-zephyr-pwm:

===
PWM
===

Introduction
============

The Alif UTIMER IP serves to generate PWM (Pulse Width Modulation) signals on the Alif devkit. It allows configuration of the first 12 UTIMER channels to produce 2 PWM signals each, resulting in a total of 24 signals simultaneously. Each UTIMER instance incorporates 2 compare blocks dedicated to PWM signal generation.

Driver Description
==================

The PWM driver is functional within the Zephyr framework, but the PWM Capture mode feature has not yet been implemented and will be added in a future release. Sample applications, such as `fade_led` and `blinky_pwm`, have been integrated with the PWM driver and tested successfully.

Currently, LED0 (Green) is used for PWM output on the HP core, and LED1 (Red) is used for PWM output on the HE core in these applications. For debugging and output, UART2 is used for the M55 HP core, while UART4 is used for the M55 HE core.

.. include:: prerequisites.rst

.. include:: note.rst

Building an PWM Application with Zephyr
========================================

Follow these steps to build the `fade_led` and `blinky_pwm` applications using the PWM driver and the west tool. The following commands are used to build the image with the GCC compiler on ITCM memory:

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, please refer to the `ZAS User Guide`_

.. note::
   The build commands shown here are specifically for the Alif E7 DevKit.
   To build the application for other boards, modify the board name in the build command accordingly. For more information, refer to the `ZAS User Guide`_, under the section Setting Up and Building Zephyr Applications.

2. Build commands for fade_led applications on the M55 HP core:

.. code-block:: bash

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp samples/basic/fade_led

3. Build commands for blinky_pwm applications on the M55 HP core:

.. code-block:: bash

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp samples/basic/blinky_pwm

4. Build commands for fade_led applications on the M55 HE core:

.. code-block:: bash

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he samples/basic/fade_led

5. Build commands for blinky_pwm applications on the M55 HE core:

.. code-block:: bash

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he samples/basic/blinky_pwm


Once the build command completes successfully, executable images will be generated and placed in the `build/zephyr` directory. Both `.bin` (binary) and `.elf` (Executable and Linkable Format) files will be available.

Executing Binary on the DevKit
==============================================

To execute binaries on the DevKit follow the command

.. code-block:: bash

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


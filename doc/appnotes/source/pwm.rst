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

Building PWM Application in Zephyr
==================================

Follow these steps to build the `fade_led` and `blinky_pwm` applications using the PWM driver and the west tool. The following commands are used to build the image with the GCC compiler on ITCM memory:

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, please refer to the `ZAS User Guide`_

.. note::
   The build commands shown here are specifically for the Alif E7 DevKit.
   To build the application for other boards, modify the board name in the build command accordingly. For more information, refer to the `ZAS User Guide`_, under the section Setting Up and Building Zephyr Applications.

2. Build commands for both applications on the M55 HP core using the Ninja build command:

.. code-block:: bash

   rm -rf build
   west build -b alif_e7_dk/ae722f80f55d5xx/rtss_hp samples/basic/fade_led

   rm -rf build
   west build -b alif_e7_dk/ae722f80f55d5xx/rtss_hp samples/basic/blinky_pwm

3. Build commands for both applications on the M55 HE core using the Ninja build command:

.. code-block:: bash

   rm -rf build
   west build -b alif_e7_dk/ae722f80f55d5xx/rtss_he samples/basic/fade_led

   rm -rf build
   west build -b alif_e7_dk/ae722f80f55d5xx/rtss_he samples/basic/blinky_pwm


Once the build command completes successfully, executable images will be generated and placed in the `build/zephyr` directory. Both `.bin` (binary) and `.elf` (Executable and Linkable Format) files will be available.

Executing Binary on the DevKit
==============================================

To execute binaries on the DevKit follow the command

.. code-block:: bash

   west flash

.. include:: west_debug.rst
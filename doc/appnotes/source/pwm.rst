.. _appnote-zephyr-pwm:

===
PWM
===

Introduction
============

The Alif UTIMER IP serves to generate PWM (Pulse Width Modulation) signals on the Alif devkit. It allows configuration of the first 12 UTIMER channels to produce 2 PWM signals each, resulting in a total of 24 signals simultaneously. Each UTIMER instance incorporates 2 compare blocks dedicated to PWM signal generation.

Software Requirement
====================

To work with the Alif Ensemble development kit and run Zephyr applications, you'll need the following software:

- **Alif SDK**: Clone from `https://github.com/alifsemi/sdk-alif <https://github.com/alifsemi/sdk-alif>`_

Driver Description
==================

The PWM driver is functional within the Zephyr framework, but the PWM Capture mode feature has not yet been implemented and will be added in a future release. Sample applications, such as `fade_led` and `blinky_pwm`, have been integrated with the PWM driver and tested successfully.

Currently, LED0 (Green) is used for PWM output on the HP core, and LED1 (Red) is used for PWM output on the HE core in these applications. For debugging and output, UART2 is used for the M55 HP core, while UART4 is used for the M55 HE core.

Building PWM Application in Zephyr
==================================

Follow these steps to build the `fade_led` and `blinky_pwm` applications using the PWM driver and the west tool. The following commands are used to build the image with the GCC compiler on ITCM memory:

.. note::
   The application is designed for the Alif Ensemble E7 DevKit. Modify the sample code as needed for other DevKits.


1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, please refer to the `ZAS User Guide`_

2. Build commands for both applications on the M55 HP core using the Ninja build command:

.. code-block:: bash

   rm -rf build
   west build -b alif_e7_dk_rtss_hp samples/basic/fade_led

   rm -rf build
   west build -b alif_e7_dk_rtss_hp samples/basic/blinky_pwm

3. Build commands for both applications on the M55 HE core using the Ninja build command:

.. code-block:: bash

   rm -rf build
   west build -b alif_e7_dk_rtss_he samples/basic/fade_led

   rm -rf build
   west build -b alif_e7_dk_rtss_he samples/basic/blinky_pwm

.. note::
   For detailed instructions on handling various scenarios, including different memory types (MRAM, flash) and alternative compilers (LLVM, ARMCLANG), refer to the document AUGD0008_Getting-Started-with-ZAS-for-Ensemble-v0.5.0-Beta.

After the build commands execute successfully, the generated executable images will be located in the `build/zephyr` directory. Both a `.bin` file (raw binary format) and a `.elf` file (Executable and Linkable Format) will be produced.

Hardware Requirements
=====================

To work with the Alif Ensemble development kit and run Zephyr applications, you'll need the following hardware:

- **Alif Ensemble DevKit**
- **Debugger (ULinkpro or JLink)**

Loading the Binary on the Alif Ensemble Devkit
==============================================

To execute binaries on the DevKit, follow these steps:

1. Open the **Debug Configuration** window with *Create, manage, and run configurations*.

   .. figure:: _static/debug_config_window.png
      :alt: Debug Configuration Window
      :align: center

      Debug Configuration Window

2. In the **Connection** tab, ensure the correct Core and ULINKpro selections are made. In the **Select Target** section, choose:

   - ``Cortex-M55_0`` for M55-HP core
   - ``Cortex-M55_1`` for M55-HE core

   .. figure:: _static/connections_tab.png
      :alt: Connection Tab Settings
      :align: center

      Connection Tab Settings

3. In the **Debugger** tab:

   - Select **Connect Only**.
   - Use the ``loadfile`` command to specify the path to the application’s ``.elf`` file.
   - Click the **Debug** symbol to load debugging information.
   - Click **Apply** and then **Debug** to start the debugging process.

   .. figure:: _static/debugger_tab.png
      :alt: Debugger Tab Settings
      :align: center

      Debugger Tab Settings
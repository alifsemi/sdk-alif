.. _spi:

===
SPI
===

Introduction
============

The Serial Peripheral Interface (SPI) module is a programmable low pin count, full-duplex master or slave synchronous serial interface. The device includes up to four SPI modules in Shared Peripherals and one Low-Power SPI module (LPSPI) in the RTSS-HE. SPI instances can be configured as both master and slave devices, but LPSPI only works in master mode. Programmable data item size (4 to 32 bits) is supported for each data transfer. SPI is connected to the AHB interface, and LPSPI is connected to the APB interface.

.. figure:: _static/spi_block_diagram.png
   :alt: SPI Block Diagram
   :align: center

   SPI Block Diagram (Contains Synopsys proprietary information. Used with permission)

Required Software
=================

The following software and drivers are required to run the Zephyr SPI application:

- Arm DS IDE version 2021.0 or newer
- Alif Zephyr SDK (v0.5.0)
  - SPI driver
  - Sample application

Application Description
=======================

This document describes two demo applications available on the Alif DevKit:

**LPSPI (Master) to SPI0 (Slave) Data Transfer**: This demo application demonstrates data transfer between the LPSPI peripheral as master and SPI0 peripheral as slave. It is specifically designed to run on the M55-HE core, which is the only core with access to the LPSPI instance. This application is DMA enabled. DMA can be disabled by configuring ``CONFIG_SPI_DW_USE_DMA=n`` in the ``prj.conf`` file.

**SPI0 (Master) to SPI1 (Slave) Data Transfer**: This demo application showcases data transfer between the SPI0 peripheral as master and the SPI1 peripheral as slave. This application can be executed on either the M55-HE or M55-HP cores. By default, this application has DMA enabled. DMA can be disabled by configuring ``CONFIG_SPI_DW_USE_DMA=n`` in the ``prj.conf`` file.

Hardware Details
================

Hardware Requirements
---------------------

**SPI IP**: The SoC utilizes Synopsys SPI controllers, with four SPI instances based on AHB SPI controllers and the LPSPI instance using an APB SPI controller.



Building the SPI Application
============================

Follow these steps to build your Zephyr-based SPI application using the GCC compiler and the Alif Zephyr SDK:

.. note::
   The application is designed for the Alif Ensemble E7 DevKit. Modify the sample code as needed for other DevKits.

1. Set up the environment by adding the necessary tools to your PATH:

   .. code-block:: bash

      export PATH=$PATH:$HOME/.local/bin

2. F1. For instructions on fetching the Alif Zephyr SDK, please refer to the `ZAS User Guide`_

3. Remove the existing build directory and build the SPI application:

   If using the M55-HP core, the application will fetch SPI0 and SPI1 instances:

   .. code-block:: bash

      rm -rf build
      west build -b alif_e7_dk_rtss_hp ../alif/samples/drivers/spi_dw -p

   If using the M55-HE core, the application will fetch SPI0 and LPSPI instances:

   .. code-block:: bash

      rm -rf build
      west build -b alif_e7_dk_rtss_he ../alif/samples/drivers/spi_dw -p

4. Access the output and binary files: The binary file is available in the ``./build/zephyr/`` directory.

**DMA Configuration**

By default, the Alif Zephyr SDK v0.5.0 enables DMA (Direct Memory Access) support for SPI transactions. To disable Tx/Rx with DMA on SPI, set the following in ``../alif/samples/drivers/spi_dw/prj.conf``:

.. code-block:: bash

   CONFIG_SPI_DW_USE_DMA=n

.. figure:: _static/spi_proj_conf.png
      :alt: Proj.conf Settings
      :align: center

      Proj.conf Settings

Executing Binary on the DevKit
==============================

To execute binaries on the DevKit , follow these steps:

1. Open the **Debug Configuration** window using the *Create, manage, and run configurations* option.

   .. note::
      The configuration name "M55_HE_I2S_Zephyr" in the screenshot is not fixed and can be changed to something appropriate for your project.

   .. figure:: _static/debug_config_window.png
      :alt: Debug Configuration Window
      :align: center

      Debug Configuration Window

2. In the **Connection** tab, ensure the correct Core and selections are made, as shown in the reference image.

   .. note::
      Ignore the configuration name "M55_HE_I2S_Zephyr" in the screenshot; it can be renamed.

   .. figure:: _static/connections_tab.png
      :alt: Connection Tab Settings
      :align: center

      Connection Tab Settings

3. In the **Debugger** tab:

   - Select **Debug from entry point** or **Debug from symbol** based on the type of debugging.
   - Use the ``loadfile`` command to specify the path to the application’s ``.elf`` file.
   - Click the **Debug** symbol to load debugging information.
   - Click **Apply** and then **Debug** to start the debugging process.

   .. note::
      Ignore the configuration name "M55_HE_I2S_Zephyr" in the screenshot; it can be renamed.

   .. figure:: _static/debugger_tab.png
      :alt: Debugger Tab Settings
      :align: center

      Debugger Tab Settings

Validating SPI
==============

Output Logs
-----------

.. figure:: _static/spi_validation_screenshot.png
   :alt: SPI Validation
   :align: center

   Validation of SPI Functionality on DevKit-E7 Board
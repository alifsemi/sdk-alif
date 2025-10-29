.. _appnote-zas-PSRAM:

========
PSRAM
========

Introduction
=============

Currently, only **AP Memory RAM** support has been added, which is available exclusively on customized **E8 DevKits**.
The default DevKits are populated with **ISSI HyperRAM**, while a few units feature the **AP Memory APS512XXN HyperRAM (Octal)**.
The **OSPI0 SS0** instance is connected to the APS512XXN device, which operates using the **HyperBus protocol**.

Driver Description
-------------------

The **APS512XXN** driver is fully functional within the **Zephyr** framework.
The **spi_psram** component from the **ALIF sdk-alif** repository has been integrated with the APS512XXN MEMC driver and verified successfully.

The APS512XXN device is connected to the **OSPI0 SS0** instance of the **DW SPI** peripheral.
It currently supports operation up to **100 MHz**. If a higher or different frequency is required, the corresponding **OSPI and RAM parameters** must be tuned accordingly to ensure stable operation.

For debugging and console output:
- **UART2** is used on the **M55 HP** core.
- **UART4** is used on the **M55 HE** core.

Prerequisites
==============

Hardware Requirements
----------------------
- Alif Devkit
- Debugger: JLink

Software Requirements
----------------------
- **Alif SDK**: Clone from `https://github.com/alifsemi/sdk-alif_2.0.git <https://github.com/alifsemi/sdk-alif_2.0.git>`_
- **West Tool**: For building Zephyr applications (installed via ``pip install west``)
- **Arm GCC Compiler**: For compiling the application (part of the Zephyr SDK)
- **SE Tools (optional)**: For loading binaries (refer to Alif documentation)

Building the PSRAM Application in Zephyr
==========================================

Run the following commands to build the PSRAM application:

.. note::
   The PSRAM feature is supported **only** on the **Alif E8 DevKit**.
   It is not applicable to other boards.

1. Build commands for applications on the M55 HE core using the Ninja build command:

.. code-block:: console

   rm -rf build; west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
      ../alif/samples/drivers/spi_psram/ \
      -DDTC_OVERLAY_FILE=$PWD/../alif/samples/drivers/spi_psram/boards/alif_aps512xxn_psram.overlay

3. Build commands for applications on the M55 HP core using the Ninja build command:


.. code-block:: bash

   rm -rf build; west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
      ../alif/samples/drivers/spi_psram/ \
      -DDTC_OVERLAY_FILE=$PWD/../alif/samples/drivers/spi_psram/boards/alif_aps512xxn_psram.overlay

Executing Binary on the DevKit
===============================

To execute binaries on the DevKit follow the command

.. code-block:: bash

   west flash

Expected Logs
===============

Below is an example of the expected console output:

.. code-block:: console

   PSRAM XIP mode demo app started
   Writing data to the XIP region:
   Reading back: Done,
   total errors = 0

Debug Binary on the DevKit
============================

To debug binaries on the DevKit follow the command

.. note::
   Current Alif supports only the J-Link debugger.
   Ensure your DevKit is connected via a J-Link interface before running this command.

.. code-block:: bash

   west debug
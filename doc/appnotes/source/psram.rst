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

.. include:: prerequisites.rst

Building the PSRAM Application in Zephyr
==========================================

Run the following commands to build the PSRAM application:

.. note::
   The PSRAM feature is supported **only** on the **Alif E8 DevKit**.
   It is not applicable to other boards.

1. Build commands for applications on the M55 HE core:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
   ../alif/samples/drivers/spi_psram/ \
   -DDTC_OVERLAY_FILE=$PWD/../alif/samples/drivers/spi_psram/boards/alif_aps512xxn_psram.overlay

2. Build commands for applications on the M55 HP core:


.. code-block:: bash

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
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

.. include:: west_debug.rst
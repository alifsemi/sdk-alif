.. _spi:

===
SPI
===

Introduction
============

The Serial Peripheral Interface (SPI) module is a programmable low pin count, full-duplex master or slave synchronous serial interface. The device includes up to four SPI modules in Shared Peripherals and one Low-Power SPI module (LP SPI) in the RTSS-HE. SPI instances can be configured as both master and slave devices, but LP SPI only works in master mode. Programmable data item size (4 to 32 bits) is supported for each data transfer. SPI is connected to the AHB interface, and LP SPI is connected to the APB interface.

.. figure:: _static/spi_block_diagram.png
   :alt: SPI Block Diagram
   :align: center

   SPI Block Diagram (Contains Synopsys proprietary information. Used with permission)

Application Description
=======================

This document describes two demo applications available on the Alif DevKit:

**LP SPI (Master) to SPI0 (Slave) Data Transfer**: This demo application demonstrates data transfer between the LP SPI peripheral as master and SPI0 peripheral as slave. It is specifically designed to run on the M55-HE core, which is the only core with access to the LP SPI instance. This application is DMA enabled. DMA can be disabled by configuring ``CONFIG_SPI_DW_USE_DMA=n`` in the ``prj.conf`` file.

**SPI0 (Master) to SPI1 (Slave) Data Transfer**: This demo application showcases data transfer between the SPI0 peripheral as master and the SPI1 peripheral as slave. This application can be executed on either the M55-HE or M55-HP cores. By default, this application has DMA enabled. DMA can be disabled by configuring ``CONFIG_SPI_DW_USE_DMA=n`` in the ``prj.conf`` file.

.. include:: prerequisites.rst

.. include:: note.rst

Building an SPI Application with Zephyr
========================================

Follow these steps to build the SPI application using the Alif Zephyr SDK:

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, please refer to the `ZAS User Guide`_

.. note::
   The build commands shown here are specifically for the Alif E7 DevKit.
   To build the application for other boards, modify the board name in the build command accordingly. For more information, refer to the `ZAS User Guide`_, under the section Setting Up and Building Zephyr Applications

2. Build commands for applications on the M55 HP core, application will fetch SPI0 and SPI1 instances:

   .. code-block:: bash

      west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp ../alif/samples/drivers/spi_dw

3. Build commands for applications on the M55 HE core, application will fetch SPI0 and LP SPI instances:

   .. code-block:: bash

      west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he ../alif/samples/drivers/spi_dw

Once the build command completes successfully, executable images will be generated and placed in the `build/zephyr` directory. Both `.bin` (binary) and `.elf` (Executable and Linkable Format) files will be available.

**DMA Configuration**

By default, the Alif Zephyr SDK enables DMA (Direct Memory Access) support for SPI transactions. To disable Tx/Rx with DMA on SPI, set the following in ``../alif/samples/drivers/spi_dw/prj.conf``:

.. code-block:: bash

   CONFIG_SPI_DW_USE_DMA=n


Proj.conf Settings
---------------------

.. code-block:: text

    # Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
    # Use, distribution and modification of this code is permitted under the
    # terms stated in the Alif Semiconductor Software License Agreement
    #
    # You should have received a copy of the Alif Semiconductor Software
    # License Agreement with this file. If not, please write to:
    # contact@alifsemi.com, or visit: https://alifsemi.com/license

    CONFIG_STDOUT_CONSOLE=y
    CONFIG_SPI=y
    CONFIG_SPI_DW=y
    CONFIG_SPI_SLAVE=y
    CONFIG_SPI_LOG_LEVEL_INF=n
    CONFIG_SPI_LOG_LEVEL_DBG=n
    CONFIG_LOG=n
    CONFIG_DMA=y
    CONFIG_DMA_PL330=y
    CONFIG_SPI_DW_USE_DMA=y
    CONFIG_PRINTK=y
    CONFIG_DMA_LOG_LEVEL_INF=n


Executing Binary on the DevKit
===============================

To execute binaries on the DevKit follow the command

.. code-block:: bash

   west flash


Console Output
===============

SPI Output Logs for HP
-----------------------

.. code-block:: text

    configure spi0 for dma0

    configure spi1 for dma0
    Slave Transceive Iter= 10
    Master Transceive Iter= 10
    Master wrote: a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
    Master receive: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
    SUCCESS: SPI Master RX & Slave TX DATA IS MATCHING: 0
    slave wrote: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
    slave read: a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
    SUCCESS: SPI Master TX & Slave RX DATA IS MATCHING: 0
    Slave Transceive Iter= 9
    Master Transceive Iter= 9
    Master wrote: a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
    Master receive: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
    SUCCESS: SPI Master RX & Slave TX DATA IS MATCHING: 0
    slave wrote: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
    slave read: a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
    SUCCESS: SPI Master TX & Slave RX DATA IS MATCHING: 0
    Slave Transceive Iter= 8
    Master Transceive Iter= 8
    Master wrote: a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
    Master receive: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
    SUCCESS: SPI Master RX & Slave TX DATA IS MATCHING: 0
    slave wrote: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
    slave read: a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
    SUCCESS: SPI Master TX & Slave RX DATA IS MATCHING: 0
    Slave Transceive Iter= 7
    Master Transceive Iter= 7
    Master wrote: a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
    Master receive: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
    SUCCESS: SPI Master RX & Slave TX DATA IS MATCHING: 0
    slave wrote: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
    slave read: a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
    SUCCESS: SPI Master TX & Slave RX DATA IS MATCHING: 0
    Slave Transceive Iter= 6
    Master Transceive Iter= 6
    Master wrote: a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
    Master receive: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
    SUCCESS: SPI Master RX & Slave TX DATA IS MATCHING: 0
    slave wrote: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
    slave read: a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
    SUCCESS: SPI Master TX & Slave RX DATA IS MATCHING: 0
    Slave Transceive Iter= 5
    Master Transceive Iter= 5
    Master wrote: a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
    Master receive: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
    SUCCESS: SPI Master RX & Slave TX DATA IS MATCHING: 0
    slave wrote: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
    slave read: a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
    SUCCESS: SPI Master TX & Slave RX DATA IS MATCHING: 0
    Slave Transceive Iter= 4
    Master Transceive Iter= 4
    Master wrote: a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
    Master receive: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
    SUCCESS: SPI Master RX & Slave TX DATA IS MATCHING: 0
    slave wrote: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
    slave read: a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
    SUCCESS: SPI Master TX & Slave RX DATA IS MATCHING: 0
    Slave Transceive Iter= 3
    Master Transceive Iter= 3
    Master wrote: a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
    Master receive: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
    SUCCESS: SPI Master RX & Slave TX DATA IS MATCHING: 0
    slave wrote: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
    slave read: a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
    SUCCESS: SPI Master TX & Slave RX DATA IS MATCHING: 0
    Slave Transceive Iter= 2
    Master Transceive Iter= 2
    Master wrote: a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
    Master receive: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
    SUCCESS: SPI Master RX & Slave TX DATA IS MATCHING: 0
    slave wrote: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
    slave read: a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
    SUCCESS: SPI Master TX & Slave RX DATA IS MATCHING: 0
    Slave Transceive Iter= 1
    Master Transceive Iter= 1
    Master wrote: a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
    Master receive: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
    SUCCESS: SPI Master RX & Slave TX DATA IS MATCHING: 0
    slave wrote: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
    slave read: a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
    SUCCESS: SPI Master TX & Slave RX DATA IS MATCHING: 0
    Slave Transfer Successfully Completed
    Master Transfer Successfully Completed

SPI Output Logs for HE
-----------------------

.. code-block:: text

     configure spi0 for dma0

     configure lpspi for dma2
    Slave Transceive Iter= 10
    Master Transceive Iter= 10
    Master wrote: a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
    Master receive: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
    SUCCESS: SPI Master RX & Slave TX DATA IS MATCHING: 0
    slave wrote: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
    slave read: a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
    SUCCESS: SPI Master TX & Slave RX DATA IS MATCHING: 0
    Slave Transceive Iter= 9
    Master Transceive Iter= 9
    Master wrote: a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
    Master receive: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
    SUCCESS: SPI Master RX & Slave TX DATA IS MATCHING: 0
    slave wrote: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
    slave read: a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
    SUCCESS: SPI Master TX & Slave RX DATA IS MATCHING: 0
    Slave Transceive Iter= 8
    Master Transceive Iter= 8
    Master wrote: a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
    Master receive: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
    SUCCESS: SPI Master RX & Slave TX DATA IS MATCHING: 0
    slave wrote: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
    slave read: a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
    SUCCESS: SPI Master TX & Slave RX DATA IS MATCHING: 0
    Slave Transceive Iter= 7
    Master Transceive Iter= 7
    Master wrote: a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
    Master receive: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
    SUCCESS: SPI Master RX & Slave TX DATA IS MATCHING: 0
    slave wrote: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
    slave read: a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
    SUCCESS: SPI Master TX & Slave RX DATA IS MATCHING: 0
    Slave Transceive Iter= 6
    Master Transceive Iter= 6
    Master wrote: a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
    Master receive: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
    SUCCESS: SPI Master RX & Slave TX DATA IS MATCHING: 0
    slave wrote: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
    slave read: a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
    SUCCESS: SPI Master TX & Slave RX DATA IS MATCHING: 0
    Slave Transceive Iter= 5
    Master Transceive Iter= 5
    Master wrote: a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
    Master receive: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
    SUCCESS: SPI Master RX & Slave TX DATA IS MATCHING: 0
    slave wrote: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
    slave read: a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
    SUCCESS: SPI Master TX & Slave RX DATA IS MATCHING: 0
    Slave Transceive Iter= 4
    Master Transceive Iter= 4
    Master wrote: a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
    Master receive: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
    SUCCESS: SPI Master RX & Slave TX DATA IS MATCHING: 0
    slave wrote: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
    slave read: a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
    SUCCESS: SPI Master TX & Slave RX DATA IS MATCHING: 0
    Slave Transceive Iter= 3
    Master Transceive Iter= 3
    Master wrote: a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
    Master receive: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
    SUCCESS: SPI Master RX & Slave TX DATA IS MATCHING: 0
    slave wrote: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
    slave read: a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
    SUCCESS: SPI Master TX & Slave RX DATA IS MATCHING: 0
    Slave Transceive Iter= 2
    Master Transceive Iter= 2
    Master wrote: a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
    Master receive: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
    SUCCESS: SPI Master RX & Slave TX DATA IS MATCHING: 0
    slave wrote: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
    slave read: a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
    SUCCESS: SPI Master TX & Slave RX DATA IS MATCHING: 0
    Slave Transceive Iter= 1
    Master Transceive Iter= 1
    Master wrote: a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
    Master receive: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
    SUCCESS: SPI Master RX & Slave TX DATA IS MATCHING: 0
    slave wrote: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
    slave read: a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
    SUCCESS: SPI Master TX & Slave RX DATA IS MATCHING: 0
    Slave Transfer Successfully Completed
    Master Transfer Successfully Completed


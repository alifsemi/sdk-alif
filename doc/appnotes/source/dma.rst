.. _appnote-zephyr-dma:

====
DMA
====

Introduction
============

Direct Memory Access (DMA) enhances system performance by offloading the CPUs from data transfers, enabling efficient data movement between memory and peripherals. The Alif platform features three DMA controllers:

- **DMA0**: A general-purpose DMA controller accessible by any core.
- **DMA1**: A dedicated DMA controller for the RTSS-HP core.
- **DMA2**: A dedicated DMA controller for the RTSS-HE core.

Since this core has numerous peripherals, we have a MUX for DMA0, which allows mapping peripherals to the DMA. This application note covers a sample test application for DMA with SPI (HE core).

.. include:: prerequisites.rst

.. include:: note.rst

Hardware Design
---------------

The block diagram illustrates the hardware design of Zephyr DMA, showing the integration of DMA controllers with peripherals and memory.


.. figure:: _static/dma_diagram.png
   :alt: Zephyr DMA Block Diagram
   :align: center

   Block Diagram of Zephyr DMA


Building an DMA Application with Zephyr
=========================================

Follow these steps to build the DMA application using the Alif Zephyr SDK:

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, please refer to the `ZAS User Guide`_

.. note::
   The build commands shown here are specifically for the Alif E7 DevKit.
   To build the application for other boards, modify the board name in the build command accordingly. For more information, refer to the `ZAS User Guide`_, under the section Setting Up and Building Zephyr Applications.

2. Build commands for applications on the M55 HE core:

.. code-block:: bash

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/spi_dw/


3. Build commands for applications on the M55 HP core:

.. code-block:: bash

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/spi_dw/


Ensure that the DMA-related configurations are enabled in ``../alif/samples/drivers/spi_dw/prj.conf``:

DMA Configuration in prj.conf
-----------------------------

.. code-block:: none

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


Select the DMA instance from the overlay file ``../alif/samples/drivers/spi_dw/boards/aalif_e7_dk/ae722f80f55d5xx/rtss_he.overlay``:

DMA Instance Selection in Overlay File
--------------------------------------

.. code-block:: dts

    /*
     * License Agreement with this file. If not, please write to:
     * contact@alifsemi.com, or visit: https://alifsemi.com/license
     */

    /* setting SPI4 as master and SPI0 instance as slave. */

    / {
        aliases {
            master-spi = &spi4;
            slave-spi  = &spi0;
        };
    };

    &dma2 {
        status = "okay";
    };

    &spi4 {
        status = "okay";
        dmas = <&dma2 0 13>, <&dma2 1 12>;
        // dmas = <&dma0 0 25>, <&dma2 1 24>;
        dma-names = "txdma", "rxdma";
    };

    &spi0 {
        status = "okay";
        serial-target;
        // dmas = <&dma2 0 20>, <&dma0 3 16>;
        dma-names = "txdma", "rxdma";
    };

Executing Binary on the DevKit
==============================

To execute binaries on the DevKit follow the command

.. code-block:: bash

   west flash

Console Output
===============

SPI data transfer occurs using the selected DMA. The following output is displayed on the serial terminal, showing the results of the SPI data transfer:

Console Output 1
----------------

.. code-block:: console


    Slave Transceive Iter= 10
    Master Transceive Iter= 10
    slave wrote: ef120000 ef120001 ef120002 ef120003 ef120004
    Slave read: abcd0000 abcd0001 abcd0002 abcd0003 abcd0004
    SUCCESS: SPI Master TX & Slave RX DATA IS MATCHING: 0

    Slave Transceive Iter= 9
    Master wrote: abcd0000 abcd0001 abcd0002 abcd0003 abcd0004
    Master receive: ef120000 ef120001 ef120002 ef120003 ef120004
    SUCCESS: SPI Master RX & Slave TX DATA IS MATCHING: 0

    Slave Transceive Iter= 8
    slave wrote: ef120000 ef120001 ef120002 ef120003 ef120004
    Slave read: abcd0000 abcd0001 abcd0002 abcd0003 abcd0004
    SUCCESS: SPI Master TX & Slave RX DATA IS MATCHING: 0

    Slave Transceive Iter= 8
    Master wrote: abcd0000 abcd0001 abcd0002 abcd0003 abcd0004
    Master receive: ef120000 ef120001 ef120002 ef120003 ef120004
    SUCCESS: SPI Master RX & Slave TX DATA IS MATCHING: 0

    Slave Transceive Iter= 7
    slave wrote: ef120000 ef120001 ef120002 ef120003 ef120004
    Slave read: abcd0000 abcd0001 abcd0002 abcd0003 abcd0004
    SUCCESS: SPI Master TX & Slave RX DATA IS MATCHING: 0


Console Output 2
----------------

.. code-block:: console

    Master Transceive Iter= 7
    slave wrote: ef120000 ef120001 ef120002 ef120003 ef120004
    Slave read: abcd0000 abcd0001 abcd0002 abcd0003 abcd0004
    SUCCESS: SPI Master TX & Slave RX DATA IS MATCHING: 0

    Slave Transceive Iter= 6
    Master wrote: abcd0000 abcd0001 abcd0002 abcd0003 abcd0004
    Master receive: ef120000 ef120001 ef120002 ef120003 ef120004
    SUCCESS: SPI Master RX & Slave TX DATA IS MATCHING: 0

    Master Transceive Iter= 6
    slave wrote: ef120000 ef120001 ef120002 ef120003 ef120004
    Slave read: abcd0000 abcd0001 abcd0002 abcd0003 abcd0004
    SUCCESS: SPI Master TX & Slave RX DATA IS MATCHING: 0

    Slave Transceive Iter= 5
    Master wrote: abcd0000 abcd0001 abcd0002 abcd0003 abcd0004
    Master receive: ef120000 ef120001 ef120002 ef120003 ef120004
    SUCCESS: SPI Master RX & Slave TX DATA IS MATCHING: 0

    Master Transceive Iter= 5
    slave wrote: ef120000 ef120001 ef120002 ef120003 ef120004
    Slave read: abcd0000 abcd0001 abcd0002 abcd0003 abcd0004
    SUCCESS: SPI Master TX & Slave RX DATA IS MATCHING: 0

    Slave Transceive Iter= 4
    Master wrote: abcd0000 abcd0001 abcd0002 abcd0003 abcd0004
    Master receive: ef120000 ef120001 ef120002 ef120003 ef120004
    SUCCESS: SPI Master RX & Slave TX DATA IS MATCHING: 0

    Master Transceive Iter= 4
    slave wrote: ef120000 ef120001 ef120002 ef120003 ef120004
    Slave read: abcd0000 abcd0001 abcd0002 abcd0003 abcd0004
    SUCCESS: SPI Master TX & Slave RX DATA IS MATCHING: 0

    Slave Transceive Iter= 3
    Master wrote: abcd0000 abcd0001 abcd0002 abcd0003 abcd0004
    Master receive: ef120000 ef120001 ef120002 ef120003 ef120004
    SUCCESS: SPI Master RX & Slave TX DATA IS MATCHING: 0

    Master Transceive Iter= 3
    slave wrote: ef120000 ef120001 ef120002 ef120003 ef120004
    Slave read: abcd0000 abcd0001 abcd0002 abcd0003 abcd0004
    SUCCESS: SPI Master TX & Slave RX DATA IS MATCHING: 0

Console Output 3
----------------

.. code-block:: console

    Slave Transceive Iter= 2
    Master wrote: abcd0000 abcd0001 abcd0002 abcd0003 abcd0004
    Master receive: ef120000 ef120001 ef120002 ef120003 ef120004
    SUCCESS: SPI Master RX & Slave TX DATA IS MATCHING: 0

    Master Transceive Iter= 2
    slave wrote: ef120000 ef120001 ef120002 ef120003 ef120004
    Slave read: abcd0000 abcd0001 abcd0002 abcd0003 abcd0004
    SUCCESS: SPI Master TX & Slave RX DATA IS MATCHING: 0

    Slave Transceive Iter= 1
    Master wrote: abcd0000 abcd0001 abcd0002 abcd0003 abcd0004
    Master receive: ef120000 ef120001 ef120002 ef120003 ef120004
    SUCCESS: SPI Master RX & Slave TX DATA IS MATCHING: 0

    Master Transceive Iter= 1
    slave wrote: ef120000 ef120001 ef120002 ef120003 ef120004
    Slave read: abcd0000 abcd0001 abcd0002 abcd0003 abcd0004
    SUCCESS: SPI Master TX & Slave RX DATA IS MATCHING: 0

    Slave Transfer Successfully Completed
    Master wrote: abcd0000 abcd0001 abcd0002 abcd0003 abcd0004
    Master receive: ef120000 ef120001 ef120002 ef120003 ef120004
    SUCCESS: SPI Master RX & Slave TX DATA IS MATCHING: 0

    Master Transfer Successfully Completed

.. note::
   The console output displays the results of SPI data transfer using DMA, including transferred data and status messages. Refer to the SPI_dw sample application (``../alif/samples/drivers/spi_dw/``) for specific output details.


.. _ospi_flash:

==========
OSPI Flash
==========

Introduction
============

The Alif DevKit features a 32MB ISSI Flash (IS25WX256) connected to the Octal SPI 1 (OSPI1) controller. This application note describes how to read from and write to the flash using the Alif Semiconductor Zephyr SDK. The flash driver implements Zephyr's standard flash APIs for erasing, reading, and writing to the flash.

.. figure:: _static/block_diagram_ospi1_flash.png
   :alt: Block Diagram of OSPI1 Connected to Flash
   :align: center

   Block Diagram of OSPI1 Connected to Flash


Application Description
=======================

This document covers the demo application for the Alif DevKit:

**Flash Demo Application**: Demonstrates the Zephyr Standard Flash API implementation on the Alif DevKit.

.. note::
   For more details, refer to the `Zephyr Flash API Reference <https://docs.zephyrproject.org/latest/reference/peripherals/flash.html>`_.

.. include:: prerequisites.rst

Hardware Connections
--------------------

The ISSI Flash is connected to the DevKit via the OSPI1 interface. No additional connections are required.

Building OSPI Flash Application in Zephyr
============================================

Follow these steps to build the Zephyr-based OSPI Flash application using the GCC compiler and the Alif Zephyr SDK:

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, please refer to the `ZAS User Guide`_

.. note::
   The build commands shown here are specifically for the Alif E7 DevKit.
   To build the application for other boards, modify the board name in the build command accordingly. For more information, refer to the `ZAS User Guide`_, under the section Setting Up and Building Zephyr Applications.


2. Build commands for applications on the M55 HE core using the Ninja build command:

.. code-block:: bash

      rm -rf build
      west build -b alif_e7_dk/ae722f80f55d5xx/rtss_he ../alif/samples/drivers/ospi_flash -p

3. Build commands for applications on the M55 HP core using the Ninja build command:

.. code-block:: bash

      rm -rf build
      west build -b alif_e7_dk/ae722f80f55d5xx/rtss_hp ../alif/samples/drivers/ospi_flash -p


Once the build command completes successfully, executable images will be generated and placed in the `build/zephyr` directory. Both `.bin` (binary) and `.elf` (Executable and Linkable Format) files will be available.

Executing Binary on the DevKit
===============================

To execute binaries on the DevKit follow the command

.. code-block:: bash

   west flash

Validating OSPI Flash
=====================

Output Logs
-----------

The following logs demonstrate the OSPI Flash functionality:

.. code-block:: bash

   **** Flash Configured Parameters ****
   * Num Of Sectors: 16384
   * Sector Size: 4096
   * Page Size: 4096
   * Erase Value: 255
   * Write Block Size: 1
   * Total Size in MB: 32

   Test 1: Flash Erase
   Flash erase succeeded!

   Test 1: Flash Write
   Attempting to write 4 bytes
   Data written successfully.

   Test 1: Flash Read
   Data read matches data written. Good!

   Test 2: Flash Full Erase
   Successfully erased entire flash memory.
   Total errors after reading erased chip: 0

   Test 3: Flash Erase
   Flash erase succeeded!

   Test 3: Flash Write
   Attempting to write 1024 bytes
   Data written successfully.

   Test 3: Flash Read
   Data read matches data written. Good!

   Test 4: Write Sector 16384
   Data written successfully.

   Test 4: Write Sector 20480
   Data written successfully.

   Test 4: Read and Verify Sector 16384
   Data read matches data written. Good!

   Test 4: Read and Verify Sector 20480
   Data read matches data written. Good!

   Test 4: Erase Sectors 16384 and 20480
   Flash erase from sector 16384, size 8192 bytes.
   Multi-sector erase succeeded!

   Test 4: Read Sector 16384
   Total errors after reading erased sector: 0

   Test 4: Read Sector 20480
   Total errors after reading erased sector: 0

   Multi-Sector Erase Test Succeeded!

   Test 5: XiP Read
   Content read from OSPI Flash in XiP mode successfully.
   Read from flash command while XiP mode enabled.
   XiP Read Test Succeeded!

.. include:: west_debug.rst
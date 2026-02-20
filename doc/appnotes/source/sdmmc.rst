.. _appnote-zephyr-alif-sdmmc:

=====
SDMMC
=====

Overview
========

This application note describes the process of creating, compiling, and running a simple FatFS demo application using the Alif SDMMC host stack on the DevKit.

.. figure:: _static/sdmmc_diagram.png
   :alt: SDMMC Configuration Diagram
   :align: center

   Diagram of the SDMMC Configuration

Introduction
============

The Alif SDMMC driver, integrated within the Ensemble ROM code, leverages Intel's eMMC host stack as an alternative to the Zephyr SDMMC host stack. This approach optimizes flash memory usage while maintaining robust SDMMC functionality.

Alif SDMMC Features
===================

The Alif SDMMC driver supports the following features:

- **SDMMC v4.1 Compliance**: Ensures compatibility with modern SD card standards.
- **Bus Width**: Supports 1-bit and 4-bit configurations.
- **Voltage**: Operates at 3.3V.
- **ADMA2**: Enables efficient data transfers with Advanced DMA.

.. include:: prerequisites.rst

.. include:: note.rst

Building an SDMMC Application with Zephyr
===========================================

Follow these steps to build the SDMMC application using the Alif Zephyr SDK:

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository,      please refer to the `ZAS User Guide`_

.. note::
   The build commands shown here are specifically for the Alif E7 DevKit with MRAM Memory.
   To build the application for other boards, modify the board name in the build command accordingly. For more information, refer to the `ZAS User Guide`_, under the section Setting Up and Building Zephyr Applications.

2. Build commands for applications on the M55 HE core:

.. code-block:: bash

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     samples/subsys/fs/fs_sample/


3 .Build commands for applications on the M55 HP core:

.. code-block:: bash

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     samples/subsys/fs/fs_sample/


Once the build command completes successfully, executable images will be generated and placed in the `build/zephyr` directory. Both `.bin` (binary) and `.elf` (Executable and Linkable Format) files will be available.

Configuring and Flashing Binary on DevKit
=========================================

Creating a JSON Configuration
-----------------------------

Create a JSON configuration file (e.g., ``sdmmc_config.json``) for the SE tool:

.. code-block:: json

   {
       "DEVICE": {
           "disabled": false,
           "binary": "app-device-config.json",
           "version": "0.5.00",
           "signed": true
       },
       "SDMMC-HP": {
           "binary": "zephyr.bin",
           "version": "1.0.0",
           "signed": false,
           "cpu_id": "M55_HP",
           "mramAddress": "0x80200000",
           "loadAddress": "0x58000000",
           "flags": ["load", "boot"]
       }
   }

Flashing the Application
------------------------

Copy files to the SE tool directory:

- ``zephyr.bin`` → ``<SE tool folder>/build/images``
- ``sdmmc_config.json`` → ``<SE tool folder>/build/config``

Execute the flashing commands:

.. code-block:: bash

   cd <SE tool folder>
   python3 app-gen-toc.py --filename build/config/sdmmc_config.json
   python3 app-write-mram.py

Executing Binary on the DevKit
===============================

To execute binaries on the DevKit follow the command

.. code-block:: bash

   west flash


Sample output
===============

The application is expected to list all directories and files available on the SD card.


.. code-block:: text

   [00:00:01.141,000] <err> emmc_hc: adma err:60
   [00:00:01.141,000] <err> emmc_hc: adma err:60
   [00:00:01.145,000] <err> emmc_hc: adma err:60
   [00:00:01.151,000] <err> emmc_hc: adma err:60
   [00:00:01.156,000] <inf> main: Block count 62333952
   Sector size 512
   Memory Size(MB) 30436
   [00:00:01.164,000] <err> emmc_hc: adma err:60
   [00:00:01.169,000] <err> emmc_hc: adma err:60
   [00:00:01.174,000] <err> emmc_hc: adma err:60
   Disk mounted.
   Listing dir /SD: ...
   [00:00:01.183,000] <err> emmc_hc: adma err:60
   [FILE] Ztest1.txt (size = 5757)
   [FILE] TestFile34.txt (size = 5757)
   [FILE] some.dat (size = 5757)
   [FILE] some9.txt (size = 5757)



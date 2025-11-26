.. _appnote-zas-mram:

========
MRAM
========

Introduction
=============

Embedded Magnetoresistive Random-Access Memory (MRAM) is a type of non-volatile random-access memory that stores data using magnetic elements.

The MRAM module operates at a clock frequency of 26.67 MHz over a 128-bit (16-byte) data bus. The 128-bit word represents the smallest addressable sector size in MRAM. This fine granularity significantly improves efficiency compared to legacy flash memory technologies, which require much larger sectors and consequently suffer from longer program and erase times.

A built-in state machine manages the erase and program sequences for 16-byte memory blocks. These operations are fully transparent to the CPU core and do not require software intervention or a dedicated driver.

To enhance performance, the MRAM controller incorporates a read cache and a write buffer, enabling concurrent read and write operations. This architecture is especially beneficial in multi-core systems, where multiple bus masters can access the MRAM simultaneously without explicit synchronization. Specifically, up to four bus masters can initiate concurrent write operations, while the number of concurrent read operations is unrestricted.

**Key Features**
------------------
- **Capacity**: 5.5 MB
- **High endurance**: Supports more than 100,000 erase/program cycles
- **Long data retention**: Greater than 10 years at a junction temperature of 125°C
- **Error correction**: 16 ECC bits per 128-bit data word
- **Read caching**: 2x 16 bytes read cache lines to accelerate access to frequently used data and   support non-16-byte-aligned read requests
- **Autonomous operation**: Built-in state machine handles 16-byte program/erase cycles—no driver required
- **Concurrent access**: Supports simultaneous read and write operations; up to four bus masters can write concurrently without coordination
- **DMA support**: Direct Memory Access (DMA) write operations can transfer up to 128 bytes per cycle


.. include:: prerequisites.rst

.. include:: note.rst

Building an MRAM Application with Zephyr
=========================================

Follow these steps to build the MRAM application using the Alif Zephyr SDK:

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, please refer to the `ZAS User Guide`_

.. note::
   The build commands shown here are specifically for the Alif E8 DevKit.
   To build the application for other boards, modify the board name in the build command accordingly. For more information, refer to the `ZAS User Guide`_, under the section Setting Up and Building Zephyr Applications.

2. Build commands for applications on the M55 HE core:

.. code-block:: console

   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_he samples/subsys/fs/littlefs/ -p -- -DSNIPPET=alif-lfs-mram

3. Build commands for applications on the M55 HP core:


.. code-block:: bash

   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp samples/subsys/fs/littlefs/ -p -- -DSNIPPET=alif-lfs-mram

Executing Binary on the DevKit
===============================

To execute binaries on the DevKit follow the command

.. code-block:: bash

   west flash

Console Output
================


.. code-block:: console

   Sample program to r/w files on littlefs
   Area 4 at 0x1a0000 on mram_flash@80000000 for 16384 bytes
   [00:00:00.012,000] <inf> littlefs: LittleFS version 2.10, disk version 2.1
   [00:00:00.019,000] <inf> littlefs: FS at mram_flash@80000000:0x1a0000 is 4 0x1000-byte blocks with 512 cycle
   [00:00:00.030,000] <inf> littlefs: partition sizes: rd 16 ; pr 16 ; ca 64 ; la 32
   [00:00:00.037,000] <err> littlefs: WEST_TOPDIR/modules/fs/littlefs/lfs.c:1389: Corrupted dir pair at {0x0, 0x1}
   [00:00:00.048,000] <wrn> littlefs: can't mount (LFS -84); formatting
   [00:00:00.074,000] <inf> littlefs: /lfs mounted
   /lfs mount: 0
   /lfs: bsize = 16 ; frsize = 4096 ; blocks = 4 ; bfree = 2

   Listing dir /lfs ...
   /lfs/boot_count read count:0 (bytes: 0)
   /lfs/boot_count write new boot count 1: [wr:1]
   [00:00:00.096,000] <inf> main: Test file: /lfs/pattern.bin not found, create one!
   ------ FILE: /lfs/pattern.bin ------
   01 55 55 55 55 55 55 55 02 55 55 55 55 55 55 55
   03 55 55 55 55 55 55 55 04 55 55 55 55 55 55 55
   05 55 55 55 55 55 55 55 06 55 55 55 55 55 55 55
   07 55 55 55 55 55 55 55 08 55 55 55 55 55 55 55
   09 55 55 55 55 55 55 55 0a 55 55 55 55 55 55 55
   0b 55 55 55 55 55 55 55 0c 55 55 55 55 55 55 55
   0d 55 55 55 55 55 55 55 0e 55 55 55 55 55 55 55
   0f 55 55 55 55 55 55 55 10 55 55 55 55 55 55 55
   11 55 55 55 55 55 55 55 12 55 55 55 55 55 55 55
   13 55 55 55 55 55 55 55 14 55 55 55 55 55 55 55
   15 55 55 55 55 55 55 55 16 55 55 55 55 55 55 55
   17 55 55 55 55 55 55 55 18 55 55 55 55 55 55 55
   19 55 55 55 55 55 55 55 1a 55 55 55 55 55 55 55
   1b 55 55 55 55 55 55 55 1c 55 55 55 55 55 55 55
   1d 55 55 55 55 55 55 55 1e 55 55 55 55 55 55 55
   1f 55 55 55 55 55 55 55 20 55 55 55 55 55 55 55
   21 55 55 55 55 55 55 55 22 55 55 55 55 55 55 55
   23 55 55 55 55 55 55 55 24 55 55 55 55 55 55 55
   25 55 55 55 55 55 55 55 26 55 55 55 55 55 55 55
   27 55 55 55 55 55 55 55 28 55 55 55 55 55 55 55
   29 55 55 55 55 55 55 55 2a 55 55 55 55 55 55 55
   2b 55 55 55 55 55 55 55 2c 55 55 55 55 55 55 55
   2d 55 55 55 55 55 55 55 2e 55 55 55 55 55 55 55
   2f 55 55 55 55 55 55 55 30 55 55 55 55 55 55 55
   31 55 55 55 55 55 55 55 32 55 55 55 55 55 55 55
   33 55 55 55 55 55 55 55 34 55 55 55 55 55 55 55
   35 55 55 55 55 55 55 55 36 55 55 55 55 55 55 55
   37 55 55 55 55 55 55 55 38 55 55 55 55 55 55 55
   39 55 55 55 55 55 55 55 3a 55 55 55 55 55 55 55
   3b 55 55 55 55 55 55 55 3c 55 55 55 55 55 55 55
   3d 55 55 55 55 55 55 55 3e 55 55 55 55 55 55 55
   3f 55 55 55 55 55 55 55 40 55 55 55 55 55 55 55

   41 55 55 55 55 55 55 55 42 55 55 55 55 55 55 55
   43 55 55 55 55 55 55 55 44 55 55 55 55 55 55 55
   45 55 aa
   [00:00:00.264,000] <inf> littlefs: /lfs unmounted
   /lfs unmount: 0

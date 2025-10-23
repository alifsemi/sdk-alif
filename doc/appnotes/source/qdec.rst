.. _appnote-zas-qdec:

====
QDEC
====

Introduction
============

The Alif UTIMER IP on the Alif Devkit supports Quadrature Decoder (QDEC) mode, enabling precise position tracking of a mechanical rotary encoder. This mode is ideal for applications requiring angular position feedback, such as motor control, robotics, or user interface dials. This application note guides developers through configuring, building, and running a Zephyr-based QDEC application (``samples/sensor/qdec/``) using the UTIMER peripheral on the Alif Devkit.

.. include:: Prerequisites.rst

Building QDEC Application in Zephyr
=====================================

Navigate to the SDK directory and build the application with TCM Memory. Refer to the User Guide for all other build options.

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, please refer to the `ZAS User Guide`_

.. note::
   The build commands shown here are specifically for the Alif E7 DevKit.
   To build the application for other boards, modify the board name in the build command accordingly. For more information, refer to the `ZAS User Guide`_, under the section Setting Up and Building Zephyr Applications.


2. Build commands for applications on the M55 HE core using the Ninja build command:


.. code-block:: bash

   rm -rf build
   west build -b alif_e7_dk/ae722f80f55d5xx/rtss_he samples/sensor/qdec/ \
       -DCONFIG_FLASH_BASE_ADDRESS=0 -DCONFIG_FLASH_LOAD_OFFSET=0 -DCONFIG_FLASH_SIZE=256

3. Build commands for applications on the M55 HP core using the Ninja build command:


.. code-block:: bash

   rm -rf build
   west build -b alif_e7_dk/ae722f80f55d5xx/rtss_hp samples/sensor/qdec/ \
       -DCONFIG_FLASH_BASE_ADDRESS=0 -DCONFIG_FLASH_LOAD_OFFSET=0 -DCONFIG_FLASH_SIZE=256


Once the build command completes successfully, executable images will be generated and placed in the `build/zephyr` directory. Both `.bin` (binary) and `.elf` (Executable and Linkable Format) files will be available.


Executing Binary on the DevKit
=============================================

To execute binaries on the DevKit follow the command

.. code-block:: bash

   west flash

Expected Result
===============

Once the application is loaded and the mechanical encoder is connected:

- The devkit runs the QDEC sample, reading the UTIMER counter in quadrature decoder mode.
- The angular position is printed every second to the console via UART4 (M55 HE core).

Sample Output:

.. code-block:: text

   *** Booting Zephyr OS build zas-v1.2-109-g81dbfb8f3841 ***
   Quadrature decoder sensor test
   Quadrature encoder emulator enabled with 100 ms period
   Position = 0 degrees
   Position = 7 degrees
   Position = 14 degrees
   Position = 21 degrees
   Position = 28 degrees
   Position = 36 degrees
   Position = 43 degrees
   Position = 50 degrees
   Position = 57 degrees
   Position = 64 degrees
   Position = 72 degrees
   Position = 79 degrees
   Position = 86 degrees
   Position = 93 degrees
   …


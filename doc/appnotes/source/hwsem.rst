.. _appnote-HWSEM:

=======
HWSEM
=======

Introduction
=============

This document provides detailed instructions on how to create, compile, and run a demo application for the Hardware Semaphore (HWSEM). The HWSEM is a mechanism used to coordinate concurrency between processor cores when accessing shared resources such as memory regions or peripherals.

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

Building HWSEM Application in Zephyr
======================================

Follow these steps to build the HWSEM application using the Alif Zephyr SDK:

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, please refer to the `ZAS User Guide`_.

.. note::
   The build commands shown here are specifically for the Alif E7 DevKit.
   To build the application for other boards, modify the board name in the build command accordingly. For more information, refer to the `ZAS User Guide`_, under the section Setting Up and Building Zephyr Applications.

2. Build commands for applications on the M55 HE core using the Ninja build command:

.. code-block:: bash

    rm -rf build
    west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he ../alif/samples/drivers/ipm/ipm_alif_hwsem/ -DHWSEM_ALL=ON

3. Build commands for applications on the M55 HP core using the Ninja build command:

.. code-block:: bash

    rm -rf build
    west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp ../alif/samples/drivers/ipm/ipm_alif_hwsem/ -DHWSEM_ALL=ON

Once the build command completes successfully, executable images will be generated and placed in the `build/zephyr` directory. Both `.bin` (binary) and `.elf` (Executable and Linkable Format) files will be available.

Executing Binary on the DevKit
===============================

To execute binaries on the DevKit, follow the command:

.. code-block:: bash

    west flash

Console Outputs
================

This section provides the console outputs for both single HWSEM and all HWSEM test runs on the **alif_e7_dk** board.

Single HWSEM Test Output
--------------------------

.. code-block:: console

    Welcome to minicom 2.8
    OPTIONS: I18n
    Port /dev/ttyUSB0, 15:43:20
    Press CTRL-A Z for help on special keys
    *** Booting Zephyr OS 4.1 build ZAS 2.0  4c97d8baffec ***
    I: Hardware Semaphore (HWSEM) example on alif_e7_dk
    I: hwsem_lock: HWSEM locked!
    I: Locked HWSEM0!
    I: Perform critical work here 1 !!!!
    I: hwsem_lock: Already locked HWSEM is locked again
    I: Locked HWSEM0!
    I: Perform critical work here 2 !!!!
    I: hwsem_unlock: HWSEM unlocked!
    I: Unlocked HWSEM0!
    I: hwsem_unlock: HWSEM unlocked!
    I: Unlocked HWSEM0!

All HWSEM Output Log
---------------------

.. code-block:: console

    Welcome to minicom 2.8
    OPTIONS: I18n
    Port /dev/ttyUSB0, 12:26:52
    Press CTRL-A Z for help on special keys
    *** Booting Zephyr OS build 4c97d8baffec ***
    I: Test all 16 Hardware Semaphores(HWSEM) on alif_e7_dk
    I: hwsem_trylock: HWSEM locked!
    I: Locked HWSEM0!
    I: Perform critical work here 1 !!!!
    I: hwsem_trylock: Already locked HWSEM is locked again
    I: Locked HWSEM0!
    I: Perform critical work here 2 !!!!
    I: hwsem_unlock: HWSEM unlocked!
    I: Unlocked HWSEM0!
    ...
    (repeat for HWSEM1 to HWSEM15 outputs)

Debug Binary on the DevKit
============================

To debug binaries on the DevKit follow the command

.. note::
   Current Alif supports only the J-Link debugger.
   Ensure your DevKit is connected via a J-Link interface before running this command.

.. code-block:: bash

   west debug
.. _appnote-zephyr-alif-ble:

===
BLE
===

Overview
========

This document outlines the process to create, compile, and run a demo application using the Alif BLE (Bluetooth Low Energy) host stack.

Introduction
============

The Alif BLE host stack, integrated within the Balletto B1 ROM code, offers an alternative to the Zephyr BLE host stack, helping to conserve flash space.


Alif BLE Features
=================

The following are important features of Alif BLE:

- **BLE v5.3 Compliance**
- **ISO Data Transfer**: Facilitates data transfer over shared memory between the Link Layer (LL) and Host Stack (HS).

.. include:: prerequisites.rst

.. include:: note.rst

Building BLE Application in Zephyr
===================================

Follow these steps to build the BLE application using the Alif Zephyr SDK:

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, please refer to the `ZAS User Guide`_

.. note::
   The build commands shown here are specifically for the Alif E7 DevKit.
   To build the application for other boards, modify the board name in the build command accordingly. For more information, refer to the `ZAS User Guide`_, under the section Setting Up and Building Zephyr Applications.

2. Build commands for applications on the M55 HE core:

.. code-block:: bash

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he samples/bluetooth/le_periph_hr

3. Build commands for applications on the M55 HP core:

.. code-block:: bash

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp samples/bluetooth/le_periph_hr

Once the build command completes successfully, executable images will be generated and placed in the `build/zephyr` directory. Both `.bin` (binary) and `.elf` (Executable and Linkable Format) files will be available.

Executing Binary on the DevKit-B1
=================================

To execute binaries on the DevKit-B1 board, follow these steps:

1. Create a JSON configuration for the SE tool:

.. code-block:: json

   {
       "DEVICE": {
           "disabled": false,
           "binary": "app-device-config.json",
           "version": "0.5.00",
           "signed": true
       },
       "BLE-HR": {
           "binary": "zephyr.bin",
           "version": "1.0.0",
           "signed": false,
           "cpu_id": "M55_HE",
           "mramAddress": "0x80000000",
           "loadAddress": "0x58000000",
           "flags": ["load", "boot"]
       }
   }

2. Flash the Application:

   a. Copy ``zephyr.bin`` into ``<SE tool folder>/build/images``.
   b. Copy the JSON file into ``<SE tool folder>/build/config``.
   c. Run the following commands in ``<SE tool folder>``:

.. code-block:: bash

   python3 app-gen-toc.py --filename build/config/<your_config_name>.json
   python3 app-write-mram.py

Upon device reset, you should see an ``ALIF_HR`` device listed among nearby Bluetooth devices when using a mobile application.

Testing the BLE Application
===========================

To test the BLE application, you'll need a mobile app that can scan BLE devices. Alif will also provide an app in the future, but it is currently in closed testing. If you need access, please contact your Alif representative.


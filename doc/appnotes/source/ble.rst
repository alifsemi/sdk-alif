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

Software Requirements
=====================

- **Alif Zephyr SDK**: Version 1.1.0, which includes BLE stack APIs and a sample application.

Building the BLE Application
============================

Follow these steps to prepare your Zephyr-based BLE application using the GCC compiler and the Alif Zephyr SDK:

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, please refer to the `ZAS User Guide`_

2. Build the BLE Heart Rate Application:

.. code-block:: bash

   cd alif
   west build -p always -b alif_b1_dk_rtss_he samples/bluetooth/le_periph_hr

3. Output and Binary Files:

   - The binary file will be located in the ``./build/zephyr/`` directory.

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
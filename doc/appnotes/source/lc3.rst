.. _appnote-zephyr-alif-lc3:

===
LC3
===

Overview
========

This document describes how to create, compile, and run a demo application using the Alif LC3 Codec.

Introduction
============

The Alif LC3 Codec, integrated into the Balletto B1 ROM code, is designed for encoding and decoding BLE isochronous audio data.


Alif LC3 Features
=================

The following are important features of the Alif LC3 codec:

- **BLE Standard Compliance**: Provides encoding/decoding of audio packets as required by the BLE standard.
- **Helium Optimisation**: Enhances performance through optimized calculations tailored for the Codec.

Software Requirements
=====================

The software required for the Zephyr LC3 Codec Application is listed below:

- **Alif Zephyr SDK**: Version 1.1.0, which includes BLE stack APIs and a sample application.

Building LC3 Application in Zephyr
=======================================

Follow these steps to build the LC3 application in Zephyr using the Alif Zephyr SDK:

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, please refer to the `ZAS User Guide`_

.. note::
   The build commands shown here are specifically for the Alif E7 DevKit.
   To build the application for other boards, please modify the board name in the build command accordingly. For more information, refer to the ZAS User Guide.

2. Build commands for applications on the M55 HE core using the Ninja build command:

.. code-block:: bash

   cd alif
   west build -p always -b alif_b1_dk_rtss_he samples/lc3/lc3_codec

3. Build commands for applications on the M55 HP core using the Ninja build command:

.. code-block:: bash

   cd alif
   west build -p always -b alif_b1_dk_rtss_hp samples/lc3/lc3_codec

.. note::
   To address various scenarios, such as utilizing MRAM or flash addresses and employing alternative compilers like LLVM or ARMCLANG, refer to the document AUGD0008_Getting-Started-with-ZAS-for-Ensemble-v0.5.0-Beta.

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
       "LC3-APP": {
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
   b. Copy the JSON config into ``<SE tool folder>/build/config``.
   c. Run the following commands in ``<SE tool folder>``:

.. code-block:: bash

   python3 app-gen-toc.py --filename build/config/<your_config_name>.json
   python3 app-write-mram.py

After resetting the device, you will see the details of the codec's encoding and decoding operations on the console.
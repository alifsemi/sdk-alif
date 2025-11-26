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

Prerequisites
=============

Hardware Requirements
---------------------
- Alif Devkit
- Debugger: JLink

Software Requirements
---------------------
- **Alif SDK**: Clone from `https://github.com/alifsemi/sdk-alif.git <https://github.com/alifsemi/sdk-alif.git>`_
- **West Tool**: For building Zephyr applications (refer to the `ZAS User Guide`_)
- **Arm GCC Compiler**: For compiling the application (part of the Zephyr SDK)
- **SE Tools**: For loading binaries (refer to the `ZAS User Guide`_)
- **Mobile App**: Scan BLE devices

.. include:: note.rst

Building an BLE Application with Zephyr
========================================

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

Executing Binary on the DevKit
===============================

To execute binaries on the DevKit follow the command

.. code-block:: bash

   west flash

Testing the BLE Application
===========================

To test the BLE application, you'll need a mobile app that can scan BLE devices. Alif will also provide an app in the future, but it is currently in closed testing. If you need access, please contact your Alif representative.

Console Output
===============

::

  [00:00:00.000,000] <inf> dma_pl330: Device dma2@400c0000 initialized

  [00:00:00.409,000] <dbg> main: main: Waiting for init...

  [00:00:00.418,000] <dbg> main: on_gapm_process_complete: gapm process completed successfully
  [00:00:00.419,000] <dbg> main: main: Init complete!

  [00:00:00.420,000] <dbg> main: on_adv_created: Advertising activity created, index 0, selected tx power 0
  [00:00:00.420,000] <dbg> main: on_adv_actv_proc_cmp: Advertising activity is created
  [00:00:00.420,000] <dbg> main: on_adv_actv_proc_cmp: Advertising data is set
  [00:00:00.421,000] <dbg> main: on_adv_actv_proc_cmp: Scan data is set
  [00:00:00.422,000] <inf> address: Device Identity Address: C9:43:C9:55:A4:D1
  [00:00:00.422,000] <inf> address: Advertising has been started, address: C9:43:C9:55:A4:D1
  [00:00:01.419,000] <dbg> main: service_process: Waiting for peer connection...
.. _can:

===
CAN
===

Overview
========

This document explains how to create, compile, and run the demo application for Controller Area Network (CAN) validation.

Introduction
============

The Controller Area Network (CAN) is a communication protocol designed for efficient and reliable data exchange between electronic control units (ECUs) in automotive and industrial systems. It operates on a bus topology, allowing multiple devices to share a common communication line while avoiding collisions using message prioritization. The protocol uses a differential signal over a twisted-pair cable, ensuring noise immunity in harsh environments, making it ideal for safety-critical applications. It supports features like error detection, retransmission, and fault isolation, ensuring data integrity. Advanced versions like CAN FD offer higher data rates and larger payloads, addressing the growing demands of modern applications.

CAN Features
============

The following CAN features are currently supported by the Alif driver:

- CAN 2.0B (up to 8 bytes payload, verified by Bosch reference model)
- CAN FD (up to 64 bytes payload, ISO 11898-1:2015)
- Programmable data rates up to 10 Mbps
- Programmable baud rate prescaler (1 to 1/256)
- Programmable internal 29-bit acceptance filters
- Extended features:
  - Single Shot Transmission mode
  - Listen-Only mode
  - Loop Back mode (Internal)
- Extended status and error report:
  - Capturing of last occurred Kind of Error (KOER) and arbitration lost position
  - Programmable Error Warning Limit
- CiA 603 timestamping

Hardware Requirements and Setup
===============================

.. figure:: _static/can_internal_connections.png
   :alt: CAN Internal Connections
   :align: center

   CAN Internal Connections

Hardware Connection & Setup
---------------------------

Utilize the Alif DevKit

Software Requirements
=====================

The software required for the CAN application includes:

- **Arm DS IDE**: Version 2021.0 or newer
- **Zephyr Alif SDK**: v1.2.0 (includes CAN driver and sample application) or newer

Building the CAN Application
============================

Follow these steps to build your Zephyr-based CAN application using the GCC compiler and the Alif Zephyr SDK:

.. note::
   The application is designed for the Alif Ensemble E7 DevKit. Modify the sample code as needed for other DevKits.

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, please refer to the `ZAS User Guide`_

2. Remove the existing build directory and build the application:

For the loopback application:

.. code-block:: bash

   rm -rf build
   west build -b alif_e7_dk_rtss_hp ../alif/samples/drivers/can/loopback -p


For the CAN counter application:

.. code-block:: bash

   rm -rf build
   west build -b alif_e7_dk_rtss_hp samples/drivers/can/counter/ -p

3. Access the output and binary files: The binary file is available in the `./build/zephyr/` directory.

Executing Binary on the DevKit
==============================

To execute binaries on the DevKit, follow these steps:

1. Open the **Debug Configuration** window using the *Create, manage, and run configurations* option.

   .. note::
      The configuration may be labeled as `M55_HE_I2S`, but you can rename it to suit your preferences.

   .. figure:: _static/debug_config_window.png
      :alt: Debug Configuration Window
      :align: center

      Debug Configuration Window

2. In the **Connection** tab, ensure the correct Core and ULINKpro selections are made. In the **Select Target** section, choose:

   - ``Cortex-M55_0`` for M55-HP core

   .. figure:: _static/connections_tab.png
      :alt: Connection Tab Settings
      :align: center

      Connection Tab Settings

3. In the **Debugger** tab:

   - Select **Debug from entry point** or **Debug from symbol** based on the type of debugging.
   - Use the ``loadfile`` command to specify the path to the application’s ``.elf`` file.
   - Click the **Debug** symbol to load debugging information.
   - Click **Apply** and then **Debug** to start the debugging process.

   .. figure:: _static/debugger_tab.png
      :alt: Debugger Tab Settings
      :align: center

      Debugger Tab Settings

Validating CAN
==============

Console Output
--------------

Upon reviewing the output logs, the CAN functionality has been successfully validated for Loopback mode.

.. figure:: _static/can_output_logs.png
   :alt: CAN Output Logs
   :align: center

   CAN Output Logs

.. note::
   The console output depends on the CAN application configuration (e.g., loopback mode, message transmission). Refer to the CAN sample application (``../alif/samples/drivers/can/loopback/``) for specific output details. Typically, the output includes status messages indicating successful message transmission and reception in loopback mode.
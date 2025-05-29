.. _i3c:

===
I3C
===

Overview
========

This document provides a comprehensive guide on creating, compiling, and running the demo application for Improved Inter-Integrated Circuit (I3C) validation.

Introduction
============

The I3C (Improved Inter-Integrated Circuit) is a cutting-edge communication interface designed to overcome the limitations of the traditional I2C protocol, enhancing both performance and efficiency. I3C supports advanced features such as dynamic address assignment, in-band interrupts, and multi-master capabilities, making it ideal for sensor-based applications in mobile, automotive, and IoT systems. With support for multiple data rates, including SDR (Standard Data Rate) and HDR (High Data Rate) modes, I3C offers greater flexibility for modern high-speed applications. It is a key enabler for reducing pin count and improving scalability in devices with diverse peripheral requirements.

I3C Features
============

The following I3C features are currently supported by the Alif driver:

- Dynamic Addressing
- Broadcast and directed Common Command Code (CCC) transfers
- In-Band Interrupts
  - Hot-Join
  - Slave Interrupt Request
  - Master-Request
- Data Rates:
  - Fast Speed (FS) mode
  - Fast Mode Plus (FM+) mode
  - SDR (Standard Data Rate)
  - HDR (High Data Rate)
- Support for legacy I2C devices
- CRC/parity generation and validation
- DMA support through hardware handshake interface
- Autonomous clock stalling
- Device address table for addressing multiple slaves
- Programmable Serial Data (SDA) transmit hold
- Programmable retry count for transfers that are addressed by slaves
- Byte support for vendor-specific Broadcast and Directed CCC Transfers

Hardware Requirements and Setup
===============================

.. figure:: _static/i3c_internal_connections.png
    :alt: I3C Internal Connections
    :align: center

    I3C Internal Connections

Hardware Connection & Setup
---------------------------

Select a board equipped with the ICM42670P (IMU sensor) I3C slave, such as the Alif Ensemble DevKit (E7, Appkit configuration) or Spark E1C DevKit.

.. note::
    The SCL and SDA lines are internally connected, so no external connection is required.

Pin Connections I3C
-------------------

- **SDA**: I3C0 (P7_6)
- **SCL**: I3C0 (P7_7)

.. list-table:: I3C Pin Connections
    :widths: 20 20 20
    :header-rows: 1

    * - Instance
      - SDA
      - SCL
    * - I3C-0
      - J15-8
      - J15-10

Software Requirements
=====================

The software required for the I3C application includes:

- **Arm DS IDE**: Version 2021.0 or newer
- **Alif Zephyr SDK**: v1.2.0 or later (includes I3C driver and sample application)

Building the I3C Application
============================

Follow these steps to build your Zephyr-based I3C application using the GCC compiler and the Alif Zephyr SDK:

.. note::
   The application is designed for the Alif Ensemble E7 DevKit. Modify the sample code as needed for other DevKits.

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, please refer to the `ZAS User Guide`_

2. Remove the existing build directory and build the I3C ICM42670P sensor application:

    .. code-block:: bash

        rm -rf build
        west build -b alif_e7_dk_rtss_hp samples/sensor/icm42670 -p

3. Access the output and binary files: The binary file is available in the `./build/zephyr/` directory.

Executing Binary on the DevKit
==============================

To execute binaries on the DevKit, follow these steps:

1. Open the **Debug Configuration** window using the *Create, manage, and run configurations* option.

    .. note::
        The provided configuration is labeled as `M55HEI2S`. However, you can rename it according to your preferences.

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

Validating I3C
==============

Output Logs
-----------

.. figure:: _static/i3c_output_logs.png
    :alt: I3C Output Logs
    :align: center

    I3C Output Logs

Observation
-----------

Upon reviewing the output logs, it can be concluded that the I3C functionality has been successfully validated with the ICM42670P IMU sensor.
.. _i2c:

===
I2C
===

Overview
========

This document explains how to create, compile, and run the demo application for Inter-Integrated Circuit (I2C) master and slave validation.

Introduction
============

The Inter-Integrated Circuit (I2C) module, functioning as a synchronized master/slave serial communication bus, is highly relevant in various system control applications. Its hierarchical structure facilitates efficient data exchange, with a master device controlling communication and slaves responding.

I2C Features
============

The following are important features of I2C:

- Select the appropriate I2C speed mode according to the I2C or I3C slave device (currently, SDR mode is supported; HDR-DDR mode is not yet supported):
  - Standard Speed (SS) mode (up to 100 kbps)
  - Fast Speed (FS) mode (up to 400 kbps)
  - Fast Speed Plus (FS+) mode (up to 1 Mbps)
- Master or slave operation (LPI2C is slave-only)
- 7- or 10-bit addressing
- 7- or 10-bit combined format transfers
- Bulk transmit mode
- CBUS addresses are ignored
- Interrupt or polled-mode operation
- Bit and byte waiting at all bus speeds
- DMA handshaking interface
- Programmable SDA hold time
- Bus clear feature

Hardware Requirements and Setup
===============================

.. figure:: _static/i2c_internal_connections.png
    :alt: I2C Internal Connections
    :align: center

    I2C Internal Connections

Software Requirements
=====================

The software required for the I2C application includes:

- **Alif Zephyr SDK**: v0.5.0 (includes I2C driver and sample application) or newer

Building I2C Application in Zephyr
====================================

Follow these steps to build your Zephyr-based I2C application using the GCC compiler and the Alif Zephyr SDK:

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, please refer to the `ZAS User Guide`_

2. Remove the existing build directory and build the application


.. note::
   The build commands shown here are specifically for the Alif E7 DevKit.
   To build the application for other boards, please modify the board name in the build command accordingly. For more information, refer to the ZAS User Guide.

3. Build commands for applications on the M55 HP core using the Ninja build command:

    .. code-block:: bash

        rm -rf build
        west build -b alif_e7_dk_rtss_hp ../alif/samples/drivers/i2c_dw -p

4. Build commands for applications on the M55 HE core using the Ninja build command:

.. code-block:: bash

        rm -rf build
        west build -b alif_e7_dk_rtss_he ../alif/samples/drivers/i2c_dw -p

.. note::
   To address various scenarios, such as utilizing MRAM or flash addresses and employing alternative compilers like LLVM or ARMCLANG, refer to the document AUGD0008_Getting-Started-with-ZAS-for-Ensemble-v0.5.0-Beta.

Once the build command completes successfully, executable images will be generated and placed in the `build/zephyr` directory. Both `.bin` (binary) and `.elf` (Executable and Linkable Format) files will be available.

Executing Binary on the DevKit
===============================

To execute binaries on the DevKit follow the command

.. code-block:: bash

   west flash

Validating I2C
==============

Console Output
--------------

Upon reviewing the output logs, the I2C functionality has been successfully validated.

.. figure:: _static/i2c_output_logs.png
    :alt: I2C Output Logs
    :align: center

    I2C Output Logs
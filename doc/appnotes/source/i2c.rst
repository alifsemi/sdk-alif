.. _i2c:

===
I2C
===

Overview
========

This document explains how to create, compile, and run the demo application for Inter-Integrated Circuit (I2C) master and slave validation.

.. figure:: _static/i2c_internal_connections.png
    :alt: I2C Internal Connections
    :align: center

    I2C Internal Connections

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

.. include:: prerequisites.rst


Pin Setup
---------

.. list-table:: I2C Pin Setup
   :widths: 20 20 20
   :header-rows: 1

   * - Function
     - I2C0 Pin
     - I2C1 Pin
   * - SDA
     - P3_5
     - P7_2
   * - SCL
     - P3_4
     - P7_3

Connection
~~~~~~~~~~

- **SDA**: Connect I2C0 instance P3_5 (J11-29) to I2C1 pin P7_2 (J11_3).
- **SCL**: Connect I2C0 instance P3_4 (J11-27) to I2C1 pin P7_3 (J11_5).

.. include:: note.rst

Building an I2C Application with Zephyr
========================================

Follow these steps to build the I2C application using the Alif Zephyr SDK:

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, please refer to the `ZAS User Guide`_


.. note::
   The build commands shown here are specifically for the Alif E7 DevKit.
   To build the application for other boards, modify the board name in the build command accordingly. For more information, refer to the `ZAS User Guide`_, under the section Setting Up and Building Zephyr Applications.

2. Build commands for applications on the M55 HP core:

.. code-block:: bash

        west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S alif-dk ../alif/samples/drivers/i2c_dw

> If using an external **Environment 3 Click Board**, use the build commands for applications on the  M55 HP core:

.. code-block:: bash

        west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S alif-dk ../alif/samples/sensor/bme680/

3. Build commands for applications on the M55 HE core:

.. code-block:: bash

        west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S alif-dk ../alif/samples/drivers/i2c_dw

> If using an external **Environment 3 Click Board**, use the build commands for applications on the  M55 HE core:

.. code-block:: bash

        west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S alif-dk ../alif/samples/sensor/bme680/

Once the build command completes successfully, executable images will be generated and placed in the `build/zephyr` directory. Both `.bin` (binary) and `.elf` (Executable and Linkable Format) files will be available.

Executing Binary on the DevKit
===============================

To execute binaries on the DevKit follow the command

.. code-block:: bash

   west flash

Console Output
===============

Upon reviewing the output logs, the I2C functionality has been successfully validated.

.. code-block:: none

   Received a byte in slave : 0xaa
   Received a byte in slave : 0xab
   Received a byte in slave : 0xac
   Received a byte in slave : 0xad
   Master wrote 0xaa 0xab 0xac 0xad to slave
   Read requested from Master and send 0x60 from slave
   Read processed_cb called
   Master received Data 0x60 From Slave

   Received a byte in slave : 0xaa
   Received a byte in slave : 0xab
   Received a byte in slave : 0xac
   Received a byte in slave : 0xad
   Master wrote 0xaa 0xab 0xac 0xad to slave
   Read requested from Master and send 0x60 from slave
   Read processed_cb called
   Master received Data 0x60 From Slave

   Received a byte in slave : 0xaa
   Received a byte in slave : 0xab
   Received a byte in slave : 0xac
   Received a byte in slave : 0xad
   Master wrote 0xaa 0xab 0xac 0xad to slave
   Read requested from Master and send 0x60 from slave
   Read processed_cb called
   Master received Data 0x60 From Slave


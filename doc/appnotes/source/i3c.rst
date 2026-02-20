.. _i3c:

===
I3C
===

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

.. include:: prerequisites.rst


Hardware Requirements and Setup
----------------------------------

.. figure:: _static/i3c_internal_connections.png
    :alt: I3C Internal Connections
    :align: center

    I3C Internal Connections

Hardware Connection & Setup
---------------------------

Select a board equipped with the BMI323 (IMU sensor) I3C slave, such as the Alif Ensemble DevKit (E7, Appkit configuration) or Spark E1C DevKit.

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

.. include:: note.rst

Building an I3C Application with Zephyr
========================================

Follow these steps to build the I3C application using the Alif Zephyr SDK:

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, please refer to the `ZAS User Guide`_

.. note::
   The build commands shown here are specifically for the Alif E7 DevKit.
   To build the application for other boards, modify the board name in the build command accordingly. For more information, refer to the `ZAS User Guide`_, under the section Setting Up and Building Zephyr Applications.


2. Build commands for applications on the M55 HP core:

.. code-block:: bash

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     samples/sensor/bmi323 \
     -S alif-dk-ak


3. Build commands for applications on the M55 HE core:

.. code-block:: bash

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     samples/sensor/bmi323 \
     -S alif-dk-ak


Once the build command completes successfully, executable images will be generated and placed in the `build/zephyr` directory. Both `.bin` (binary) and `.elf` (Executable and Linkable Format) files will be available.

Executing Binary on the DevKit
==============================

To execute binaries on the DevKit follow the command

.. code-block:: bash

   west flash

Console Output
===============

.. code-block:: bash

   Device 0xb35c name is bmi323@69000003b810431000
   Accel AX: -0.004209; AY: -0.008052; AZ: 1.022177 g       Gyro GX: 0.076295; GY: -0.289921; GZ: -0.015259 deg/s
   Accel AX: -0.003294; AY: -0.009760; AZ: 1.019310 g       Gyro GX: 0.106813; GY: -0.183108; GZ: -0.015259 deg/s
   Accel AX: -0.005551; AY: -0.008906; AZ: 1.020164 g       Gyro GX: 0.061036; GY: -0.183108; GZ: 0.000000 deg/s
   Accel AX: -0.005856; AY: -0.007869; AZ: 1.021750 g       Gyro GX: 0.045777; GY: -0.183108; GZ: 0.015259 deg/s
   Accel AX: -0.005612; AY: -0.008052; AZ: 1.021384 g       Gyro GX: 0.091554; GY: -0.228885; GZ: 0.015259 deg/s
   Accel AX: -0.006161; AY: -0.008601; AZ: 1.021506 g       Gyro GX: 0.061036; GY: -0.167849; GZ: -0.076295 deg/s
   Accel AX: -0.003172; AY: -0.006588; AZ: 1.020042 g       Gyro GX: 0.045777; GY: -0.244144; GZ: 0.000000 deg/s
   Accel AX: -0.004575; AY: -0.008174; AZ: 1.019981 g       Gyro GX: 0.061036; GY: -0.213626; GZ: 0.000000 deg/s
   Accel AX: -0.004087; AY: -0.007503; AZ: 1.020469 g       Gyro GX: 0.076295; GY: -0.183108; GZ: -0.045777 deg/s
   *** Booting Zephyr OS build 3ba659300a80 ***
   Accel AX: -0.004880; AY: -0.006405; AZ: 1.020042 g       Gyro GX: 0.045777; GY: -0.183108; GZ: -0.061036 deg/s
   Accel AX: -0.007015; AY: -0.008967; AZ: 1.020042 g       Gyro GX: 0.015259; GY: -0.198367; GZ: -0.015259 deg/s
   Accel AX: -0.005368; AY: -0.007930; AZ: 1.021140 g       Gyro GX: 0.076295; GY: -0.198367; GZ: 0.030518 deg/s
   Accel AX: -0.006100; AY: -0.008113; AZ: 1.021384 g       Gyro GX: 0.030518; GY: -0.183108; GZ: -0.045777 deg/s
   Accel AX: -0.004392; AY: -0.007381; AZ: 1.020408 g       Gyro GX: 0.015259; GY: -0.183108; GZ: -0.015259 deg/s
   Accel AX: -0.004697; AY: -0.009455; AZ: 1.020103 g       Gyro GX: 0.061036; GY: -0.183108; GZ: -0.015259 deg/s
   Accel AX: -0.005063; AY: -0.007198; AZ: 1.020164 g       Gyro GX: 0.076295; GY: -0.198367; GZ: -0.030518 deg/s
   Accel AX: -0.004880; AY: -0.009211; AZ: 1.022604 g       Gyro GX: 0.030518; GY: -0.213626; GZ: 0.045777 deg/s

Observation
-----------

Upon reviewing the output logs, it can be concluded that the I3C functionality has been successfully validated with the ICM42670P IMU sensor.
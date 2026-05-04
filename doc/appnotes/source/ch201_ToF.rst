.. _ch201_tof_sensor:

==================================
CH201 Time-of-Flight (ToF) Sensor
==================================

Overview
=========

This document explains how to create, compile, and run the demo application for
the CH201 Time-of-Flight sensor validation.

Introduction
=============

The CH201 is a MEMS-based ultrasonic Time-of-Flight (ToF) sensor designed for
proximity sensing and range measurement. Ultrasonic ToF technology is widely used
for reliable distance and presence detection in embedded systems, especially in
environments where optical sensors are affected by lighting conditions, surface color,
or transparency.

Distance is measured by transmitting ultrasonic pulses and computing the time taken
for the reflected echo to return from nearby objects. The CH201 communicates with
the host processor using an **I²C interface** and supports **interrupt-based signaling**
for event-driven operation.

CH201 Features
===============

Key features include:

- Fast, accurate range finding
- Operating range: **20 cm to 5 m**
- Programmable modes for long-range or short-range sensing
- Customizable field of view (FoV) up to **180°**
- Multi-object detection
- Works reliably in **all lighting conditions**
- Detects optically transparent objects (glass, clear plastic, etc.)
- I2C Fast Mode compatible (up to 400 kHz)

Prerequisites
=============

Hardware Requirements
---------------------
- Alif Appkit
- Debugger: JLink

Software Requirements
---------------------
- **Alif SDK**: Clone from `https://github.com/alifsemi/sdk-alif.git <https://github.com/alifsemi/sdk-alif.git>`_
- **West Tool**: For building Zephyr applications (refer to the `ZAS User Guide`_)
- **Arm GCC Compiler**: For compiling the application (part of the Zephyr SDK)
- **SE Tools**: For loading binaries (refer to the `ZAS User Guide`_)


Pin Setup
---------

+---------------+--------+
| Function      | Pin    |
+===============+========+
| I2C1 SCL      | P7_3   |
+---------------+--------+
| I2C1 SDA      | P7_2   |
+---------------+--------+
| INT           | P6_3   |
+---------------+--------+
| PROG          | P6_2   |
+---------------+--------+
| RESETn        | P6_4   |
+---------------+--------+

**Connection:**
The CH201 sensor is available only on the **E8 App-Kit** and is connected onboard to **I2C1**.
No external wiring is required.

.. include:: note.rst

Building a CH201 Application with Zephyr
=========================================

Follow these steps to build the CH201 ToF Application using the Alif Zephyr SDK:

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, please refer to the `ZAS User Guide`_

.. note::
   The CH201 ToF feature is supported **only** on the **Alif E8 AppKit**.
   It is not applicable to other boards.

2. Build the example application for the M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     -S alif-ak ../alif/samples/sensor/ch201

3. Build the example application for the M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
     -S alif-ak ../alif/samples/sensor/ch201

Once the build command completes successfully, executable images will be generated and placed in the `build/zephyr` directory. Both `.bin` (binary) and `.elf` (Executable and Linkable Format) files will be available.

Executing Binary on the Appkit
================================

To execute binaries on the AppKit follow the command

.. code-block:: console

   west flash

Console Output
==============

Example runtime output:

.. code-block:: text

   [00:00:00.417,000] <inf> CH201_TOF: Device 0x9a98 name is ch201@29
   [00:00:00.455,000] <inf> CH201_TOF:     Object Range --> 0.223593 m, Amplitude --> 13981
   [00:00:01.459,000] <inf> CH201_TOF:     Object Range --> 0.221031 m, Amplitude --> 12765
   [00:00:02.463,000] <inf> CH201_TOF:     Object Range --> 0.221250 m, Amplitude --> 12678
   [00:00:03.468,000] <inf> CH201_TOF:     Object Range --> 0.221718 m, Amplitude --> 12918
   [00:00:04.472,000] <inf> CH201_TOF:     Object Range --> 0.221375 m, Amplitude --> 12182
   [00:00:05.476,000] <inf> CH201_TOF:     Object Range --> 0.220937 m, Amplitude --> 12983
   [00:00:06.480,000] <inf> CH201_TOF:     Object Range --> 0.223718 m, Amplitude --> 13207
   <repeats endlessly>

.. note::
   The measured range values may differ based on object type, distance, and environment.
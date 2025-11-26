.. _lpi2c:

=====
LPI2C
=====

Introduction
============

This document explains how to create, compile, and run a demo application for the LPI2C (Low Power Inter-Integrated Circuit) controller IP provided by Alif Semiconductor™ and integrated into Ensemble™ devices.

.. figure:: _static/lpi2c_block_diagram.png
   :alt: LPI2C Block Diagram
   :align: center

   LPI2C Block Diagram

.. include:: prerequisites.rst


LPI2C Interface
---------------

The LPI2C block diagram illustrates the integration of the LPI2C controller with other system components.

Pin Setup
---------

.. list-table:: LPI2C0 Pin Setup
   :widths: 20 20 20
   :header-rows: 1

   * - Function
     - I2C0 Pin
     - LPI2C0 Pin
   * - SDA
     - P3_5
     - P7_5
   * - SCL
     - P3_4
     - P7_4

Hardware Connections and Setup
------------------------------

.. figure:: _static/lpi2c_hardware_setup.png
   :alt: LPI2C Hardware Setup
   :align: center

   LPI2C Hardware Setup

Connection
~~~~~~~~~~

- **SDA**: Connect I2C0 instance P3_5 to LPI2C pin P7_5.
- **SCL**: Connect I2C0 instance P3_4 to LPI2C0 pin P7_4.

.. note::
   In the Balletto A5 SoC, the SCL and SDA lines of I2C0 (configured as Master) are not pulled up. Therefore, it is recommended to use I2C1 as the bus master in such cases.

.. include:: note.rst

Building an LPI2C Application with Zephyr
==========================================

Follow these steps to build the LPI2C application using the Alif Zephyr SDK:


1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, please refer to the `ZAS User Guide`_

.. note::
   The build commands shown here are specifically for the Alif E7 DevKit.
   To build the application for other boards, modify the board name in the build command accordingly. For more information, refer to the `ZAS User Guide`_, under the section Setting Up and Building Zephyr Applications.


2. Build command for applications on the M55 HE core:

.. code-block:: bash

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he ../alif/samples/drivers/lpi2c


Once the build command completes successfully, executable images will be generated and placed in the `build/zephyr` directory. Both `.bin` (binary) and `.elf` (Executable and Linkable Format) files will be available.


Executing Binary on the DevKit
==============================

To execute binaries on the DevKit follow the command

.. code-block:: bash

   west flash

Console Output
================


.. code-block:: console

   [00:00:00.000,000] <inf> ALIF_LPI2C: Start Master transmit and Slave receive
   [00:00:00.001,000] <inf> ALIF_LPI2C: Master transmit and slave receive successful
   [00:00:00.002,000] <inf> ALIF_LPI2C: Start Slave transmit and Master receive
   [00:00:00.006,000] <inf> ALIF_LPI2C: Slave transmit and Master receive successful
   [00:00:00.006,000] <inf> ALIF_LPI2C: Transfer completed


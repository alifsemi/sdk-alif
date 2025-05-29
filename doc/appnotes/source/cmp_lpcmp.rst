.. _appnote-zephyr-cmp-lpcmp:

=========
CMP/LPCMP
=========

Introduction
============

This application note provides a comprehensive guide on how to create, compile, and run a demo application for the CMP (Analog Comparator) controller IP and LPCMP (Low Power Comparator), which is provided by Alif Semiconductor™ and integrated into the Ensemble™ devices using the Zephyr OS.

.. figure:: _static/cmp_lpcmp_diagram.png
   :alt: CMP/LPCMP Interface Diagram
   :align: center

   CMP/LPCMP Interface Diagram

CMP/LPCMP Overview
==================

The High-Speed Comparator (CMP) and Low-Power Comparator (LPCMP) modules are analog comparators integrated into the Ensemble™ devices, designed for high-performance and low-power applications, respectively. The device includes up to two CMP modules and one LPCMP module, each with distinct features tailored to specific use cases.

- **CMP Features**:

  - Rail-to-rail, multi-input analog comparator with programmable reference voltage and hysteresis

  - Reference voltage selectable from DAC6, internal Vref, or external pins

  - Programmable hysteresis: 0 mV to 45 mV

  - Windowing (gating) driven by one of four events from UTIMER

  - Comparator result inverter

  - Configurable number of taps for filtering

  - Interrupt generation after filtering

  - Maximum current consumption: 100 microampere

  - Response time: < 5 ns

  - Input offset: under 8 mV

  - Power supply: internal 1.8-V LDO (LDO-5)

- **LPCMP Features**:
  - Low-power, rail-to-rail analog comparator with selectable reference voltage and hysteresis, located in the PD0 power domain

  - Up to four external pins for voltage monitoring

  - Voltage reference from:


    - Internal 0.6-V voltage reference

    - External VREF pins

  - Programmable hysteresis: 5 mV to 30 mV (measured with input voltage range limited from 0.7 V to VDD – 0.7 V)

  - Power supply: VDD_IO_1V8 pin

  - Maximum current consumption: 20 microAmpere

  - Response time: < 10 microseconds

  - Input offset: 20 mV / 1 sigma

- **Supported Channels**:
  - CMP: Up to four channels (CMP0, CMP1, CMP2, CMP3)
  - LPCMP: Single channel with up to four input pins (P2_4, P2_5, P2_6, P2_7)

.. figure:: _static/cmp_lpcmp_diagram.png
   :alt: CMP Implementation Overview
   :align: center

   CMP Implementation Overview

Building CMP/LPCMP Application in Zephyr using GCC Compiler
===========================================================

Follow these steps to build the CMP/LPCMP application in Zephyr using the Alif Zephyr SDK:

.. note::
   The application is designed for the Alif Ensemble E7 DevKit. Modify the sample code as needed for other DevKits.

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, please refer to the `ZAS User Guide`_

2. Remove any existing build and build the CMP/LPCMP sample:

   **For M55-HE core:**

.. code-block:: bash

   rm -rf build
   west build -b alif_e7_dk_rtss_he ../alif/samples/drivers/cmp

   **For M55-HP core:**

.. code-block:: bash

   rm -rf build
   west build -b alif_e7_dk_rtss_hp ../alif/samples/drivers/cmp

Hardware Requirements and Setup
===============================

Hardware Requirements
---------------------

- **Alif Ensemble DevKit (Flat Board)**
- **Debugger (ULinkpro or JLink)**

CMP/LPCMP Interface
-------------------

The CMP/LPCMP module interfaces with the external environment through designated pins on the Ensemble™ devices.

Pin Setup
---------

The CMP pin setup is as follows:

.. list-table:: CMP Pin Setup
   :widths: 20 20 20 20 20
   :header-rows: 1

   * - CMP
     - CMP0
     - CMP1
     - CMP2
     - CMP3
   * -
     - P0_0
     - P0_2
     - P0_1
     - P0_3
   * -
     - P0_6
     - P0_8
     - P0_7
     - P0_9
   * -
     - P1_4
     - P0_14
     - P0_13
     - P0_15
   * -
     - P0_4
     - P0_10
     - P0_5
     - P0_11
   * - Output Pin
     - P14_7
     - P14_6
     - P14_5
     - P14_4

The LPCMP pin setup is as follows:

.. list-table:: LPCMP Pin Setup
   :widths: 20 80
   :header-rows: 1

   * - LPCMP
     - Pins
   * -
     - P2_4, P2_5, P2_6, P2_7

.. note:: LPCMP has no output pins.

Hardware Connections
---------------------

.. figure:: _static/hardware_connections_cmp.png
   :alt: Hardware Connections
   :align: center

   Hardware Connections

Software Requirements
=====================

Below is a list of required software and drivers needed to run the CMP/LPCMP application:

- **Alif Zephyr CMP driver**

Executing Binary on the DevKit
==============================

To execute binaries on the DevKit board, follow these steps:

1. Open the **Debug Configuration** window by selecting *Create, manage, and run configurations* from the menu.

   .. figure:: _static/debug_config_window.png
      :alt: Debug Configuration Window
      :align: center

      Debug Configuration Window

2. Ensure that the **Connection** tab has the correct Core and ULINKpro selections:

   - Choose ``Cortex-M55_0`` for M55-HP core or ``Cortex-M55_1`` for M55-HE core.

   .. figure:: _static/connections_tab.png
      :alt: Connection Tab Settings
      :align: center

      Connection Tab Settings

3. Move to the **Debugger** tab:

   - Select **Connect only**.
   - Add the application executable to be loaded using the ``loadfile`` command.
   - Click the **Debug** symbol to start debugging.
   - Click **Apply** and then **Debug** to proceed with code debugging.

   .. figure:: _static/debugger_tab.png
      :alt: Debugger Tab Settings
      :align: center

      Debugger Tab Settings

Sample Output
=============

**Console Output for CMP**

.. code-block:: text

   Welcome to minicom 2.7.1
   OPTIONS: I18n
   Compiled on Dec 23 2019, 02:06:26.
   Port /dev/ttyACM1, 12:41:44
   Press CTRL-A Z for help on special keys
   *** Booting Zephyr OS build Zephyr-Alif-SDK-v0.5.0-17-g17b360353343 ***
   [00:00:02.000,000] <inf> ALIF_CMP: start comparing
   [00:00:02.050,000] <inf> ALIF_CMP: positive input voltage is greater than negative input voltage
   [00:00:02.101,000] <inf> ALIF_CMP: negative input voltage is greater than the positive input voltage
   [00:00:02.151,000] <inf> ALIF_CMP: positive input voltage is greater than negative input voltage
   [00:00:02.201,000] <inf> ALIF_CMP: negative input voltage is greater than the positive input voltage
   [00:00:02.251,000] <inf> ALIF_CMP: positive input voltage is greater than negative input voltage
   [00:00:02.301,000] <inf> ALIF_CMP: negative input voltage is greater than the positive input voltage
   [00:00:02.351,000] <inf> ALIF_CMP: positive input voltage is greater than negative input voltage
   [00:00:02.401,000] <inf> ALIF_CMP: negative input voltage is greater than the positive input voltage
   [00:00:02.451,000] <inf> ALIF_CMP: positive input voltage is greater than negative input voltage
   [00:00:02.501,000] <inf> ALIF_CMP: negative input voltage is greater than the positive input voltage
   [00:00:02.501,000] <inf> ALIF_CMP: Comparison Completed

**Console Output for LPCMP**

.. code-block:: text

   Welcome to minicom 2.8
   OPTIONS: I18n
   Port /dev/ttyUSBO, 14:31:40
   Press CTRL-A Z for help on special keys
   *** Booting Zephyr OS build zas-vl.l-main-2-g12049f5595b5 ***
   [00:00:16.838,000] <inf> ALIF_CMP: Comparison completed
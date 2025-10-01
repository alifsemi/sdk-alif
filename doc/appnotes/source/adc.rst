.. _appnote-zephyr-adc12-24:

========
ADC12/24
========

Introduction
============

This document explains how to create, compile, and run a demo application for the Analog-to-Digital Conversion (ADC) 12- and 24-bit controller IP provided by Alif Semiconductor™ and integrated into Devkit devices.

The ADC12 supports 8 channels (6 external and 2 internal inputs). One temperature sensor is connected to all ADC12 instances at channel no 6. ADC24 supports 4 channels (differential only). The ADC12 works with both single-ended input and differential input (ADC24 works only with differential input).

- **Single-Ended Input**:

  - Single-shot conversion
  - Single-channel scan
  - Continuous conversion
  - Single-channel scan
  - Multiple-channel scan
- **Differential Input**:

  - Single-shot conversion
  - Single-channel scan
  - Continuous conversion

.. figure:: _static/adc12_diagram.png
   :alt: 12-Bit ADC Block Diagram
   :align: center

   12-Bit ADC Block Diagram

.. figure:: _static/adc24_diagram.png
   :alt: 24-Bit ADC Block Diagram
   :align: center

   24-Bit ADC Block Diagram

Building the ADC Application
============================

Follow these steps to build the ADC application in Zephyr using the Alif Zephyr SDK:

.. note::
   The application is designed for the Alif Ensemble E7 DevKit. Modify the sample code as needed for other DevKits.

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, please refer to the `ZAS User Guide`_

2. Build the project for the Alif DevKit using the sample code found in ``../alif/samples/drivers/adc``.

   **For M55-HP core:**

.. code-block:: bash

   rm -rf build
   west build -b alif_ensemble_e7_devkit_rtss_hp ../alif/samples/drivers/adc

   **For M55-HE core:**

.. code-block:: bash

   rm -rf build
   west build -b alif_ensemble_e7_devkit_rtss_he ../alif/samples/drivers/adc

Hardware Requirements
=====================

- **Alif DevKit**
- **Debugger (ULinkpro or JLink)**

ADC Interface
=============

The ADC12 and ADC24 modules interface with the external environment through designated pins on the DevKit devices.

Hardware Connections
====================

**ADC12**

No hardware connection is required to test the temperature sensor, which is internally connected to all instances of ADC12 (0, 1, and 2).

**Setup for Checking Single-Ended Conversion from an External Input Source**

(0–7 channels are available)

.. figure:: _static/single_ended_connections_for_ADC_12.png
   :alt: Single-Ended Conversion Setup for ADC 12
   :align: center

   Setup for Single-Ended Conversion for ADC 12

**Setup for Checking Differential Input Conversion from an External Input Source**

(0, 1, and 2 channels are available)

Enable differential mode from the ADC sample application for operating ADC in differential mode.

.. figure:: _static/screen_capture_of_ADC12_differential_connections.png
   :alt: ADC 12 Differential Conversion Setup Screen Capture
   :align: center

   Screen capture of ADC 12 Differential Conversion for ADC 12

.. figure:: _static/differential_connections_for_ADC_12.png
   :alt: Differential Conversion Setup
   :align: center

   Setup for Differential Conversion for ADC 12

.. figure:: _static/differential_connections_for_ADC_24.png
   :alt: Differential Conversion Setup for ADC 24
   :align: center

   Setup for Differential Conversion for ADC 24

Executing Binary on the DevKit
==============================

To execute binaries on the DevKit board, follow these steps:

1. Open the **Debug Configuration** window with *Create, manage, and run configurations*.

   .. figure:: _static/debug_config_window.png
      :alt: Debug Configuration Window
      :align: center

      Debug Configuration Window

2. In the **Connection** tab, ensure the correct Core and ULINKpro selections are made. In the **Select Target** section, choose:

   - ``Cortex-M55_0`` for M55-HP core
   - ``Cortex-M55_1`` for M55-HE core

   .. figure:: _static/connections_tab.png
      :alt: Connection Tab Settings
      :align: center

      Connection Tab Settings

3. In the **Debugger** tab:

   - Select **Connect Only**.
   - Use the ``loadfile`` command to specify the path to the application’s ``.elf`` file.
   - Click the **Debug** symbol to load debugging information.
   - Click **Apply** and then **Debug** to start the debugging process.

   .. figure:: _static/debugger_tab.png
      :alt: Debugger Tab Settings
      :align: center

      Debugger Tab Settings

Sample Output
=============

.. note::
   The console output depends on the ADC configuration (e.g., single-ended or differential mode, channel selection, single-shot or continuous conversion). Refer to the ADC sample application (``../alif/samples/drivers/adc``) for specific output details. Typically, the output includes voltage readings or temperature sensor data in a format defined by the application.
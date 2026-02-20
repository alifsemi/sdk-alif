.. _appnote-zephyr-adc12/24:

=========
ADC12/24
=========

Introduction
============

This document explains how to create, compile, and run a demo application for the Analog-to-Digital Conversion (ADC) 12-bit controller IP provided by Alif Semiconductor™ and integrated into Devkit devices.

The ADC12 supports 8 channels (6 external and 2 internal inputs). One temperature sensor is connected to all ADC12 instances at channel no. 6. The ADC12 works with both single-ended and differential inputs.

- **Single-Ended Input**:

  - Single-shot conversion
  - Single-channel scan
  - Continuous conversion
  - Multiple-channel scan

- **Differential Input**:

  - Single-shot conversion
  - Single-channel scan
  - Continuous conversion

**Note: In Balletto, Channel 7 (Vref) has been removed from the ADC12.**

.. figure:: _static/adc12_diagram.png
   :alt: 12-Bit ADC Block Diagram
   :align: center

   12-Bit ADC Block Diagram

.. figure:: _static/adc24_diagram.png
   :alt: 24-Bit ADC Block Diagram
   :align: center

   24-Bit ADC Block Diagram

.. include:: prerequisites.rst

.. include:: note.rst

Building an ADC Application with Zephyr
========================================

Follow these steps to build the ADC application using the Alif Zephyr SDK:

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, please refer to the `ZAS User Guide`_


.. note::
   The build commands shown here are specifically for the Alif E7 DevKit.
   To build the application for other boards, modify the board name in the build command accordingly. For more information, refer to the `ZAS User Guide`_, under the section Setting Up and Building Zephyr Applications.

2. Build commands for applications on the M55 HP core:

.. code-block:: bash

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/adc \
     -S alif-adc


3. Build commands for applications on the M55 HE core:

.. code-block:: bash

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/adc \
     -S alif-adc


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

**Screen capture of ADC 12 Differential Conversion for ADC 12**

.. code-block:: c

   struct adc_channel_cfg channel_cfg = {
       .differential = 0,
       .channel_id   = ADC_CHANNEL_6,
   };


.. figure:: _static/differential_connections_for_ADC_12.png
   :alt: Differential Conversion Setup
   :align: center

   Setup for Differential Conversion for ADC 12

.. figure:: _static/differential_connections_for_ADC_24.png
   :alt: Differential Conversion Setup for ADC 24
   :align: center

   Setup for Differential Conversion for ADC 24

Executing Binary on the DevKit
===============================

To execute binaries on the DevKit follow the command

.. code-block:: bash

   west flash

Console Output
===============

.. note::
   The console output depends on the ADC configuration (e.g., single-ended or differential mode, channel selection, single-shot or continuous conversion). Refer to the ADC sample application (``../alif/samples/drivers/adc -S alif-adc``) for specific output details. Typically, the output includes voltage readings or temperature sensor data in a format defined by the application.


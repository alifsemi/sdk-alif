.. _appnote-zephyr-adc12/24:

=========
ADC12/24
=========

Introduction
============

This application note explains how to create, build, and run a demo
application for the Analog-to-Digital Converter (ADC) on Alif DevKit devices,
with a focus on ADC12.

ADC12 supports eight channels: six external inputs and two internal inputs. A
temperature sensor is connected to channel 6 on all ADC12 instances. ADC12
supports both single-ended and differential inputs.

- **Single-Ended Input**:

  - Single-shot conversion
  - Single-channel scan
  - Continuous conversion
  - Multiple-channel scan

- **Differential Input**:

  - Single-shot conversion
  - Single-channel scan
  - Continuous conversion

**Note:** On Balletto, channel 7 (Vref) is not available on ADC12.

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

ADC Interface
=============

The ADC12 and ADC24 modules connect to external pins on the DevKit devices.

Hardware Connections
====================

**ADC12**

No external hardware connection is required to test the temperature sensor
because it is internally connected to ADC12 instances 0, 1, and 2.

**Setup for Checking Single-Ended Conversion Using an External Input Source**

Channels 0 through 7 are available for single-ended conversion.

.. figure:: _static/single_ended_connections_for_ADC_12.png
   :alt: Single-ended conversion setup for ADC12
   :align: center

   Single-ended conversion setup for ADC12

**Setup for Checking Differential Conversion Using an External Input Source**

Channels 0, 1, and 2 are available for differential conversion.

The default ADC sample application is configured for single-ended
temperature-sensor measurements. To evaluate differential mode, update the ADC
sample application to enable differential input and select a supported channel
pair.

.. figure:: _static/differential_connections_for_ADC_12.png
   :alt: Differential conversion setup for ADC12
   :align: center

   Differential conversion setup for ADC12

.. figure:: _static/differential_connections_for_ADC_24.png
   :alt: Differential conversion setup for ADC24
   :align: center

   Differential conversion setup for ADC24

Build an ADC Application with Zephyr
=====================================

Follow these steps to build the ADC sample application using the Alif Zephyr
SDK:

For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr
repository, refer to the `ZAS User Guide`_, under the section ``Setting Up and Building Zephyr Applications``.

Alif E7 DevKit
---------------

Build for SoC variant ``ae722f80f55d5xx``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/adc \
     -S alif-adc

Build for SoC variant ``ae722f80f55d5xx``, M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/adc \
     -S alif-adc

Build for SoC variant ``ae302f80f55d5xx``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/adc \
     -S alif-adc

Build for SoC variant ``ae302f80f55d5xx``, M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/adc \
     -S alif-adc

Alif E7 AppKit
----------------

Build for SoC variant ``ae722f80f55d5xx``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/adc \
     -S alif-adc

Build for SoC variant ``ae722f80f55d5xx``, M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/adc \
     -S alif-adc

Alif E8 DevKit
---------------

Build for SoC variant ``ae822fa0e5597xx0``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/adc \
     -S alif-adc

Build for SoC variant ``ae822fa0e5597xx0``, M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/adc \
     -S alif-adc

Build for SoC variant ``ae402fa0e5597xx0``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/adc \
     -S alif-adc

Build for SoC variant ``ae402fa0e5597xx0``, M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/adc \
     -S alif-adc

Alif E8 AppKit
----------------

Build for SoC variant ``ae822fa0e5597xx0``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/adc \
     -S alif-adc

Build for SoC variant ``ae822fa0e5597xx0``, M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/adc \
     -S alif-adc

Alif E1C DevKit
----------------

Build for SoC variant ``ae1c1f4051920hh``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     ../alif/samples/drivers/adc \
     -S alif-adc

Alif B1 DevKit
---------------

Build for SoC variant ``ab1c1f4m51820ph0``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     ../alif/samples/drivers/adc \
     -S alif-adc

Build for SoC variant ``ab1c1f4m51820hh0``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820hh0/rtss_he \
     ../alif/samples/drivers/adc \
     -S alif-adc

Build for SoC variant ``ab1c1f1m41820hh0``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     ../alif/samples/drivers/adc \
     -S alif-adc

Build for SoC variant ``ab1c1f1m41820ph0``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     ../alif/samples/drivers/adc \
     -S alif-adc

Once the build command completes successfully, executable images will be generated and placed in the ``build/zephyr`` directory. Both ``.bin`` (binary) and ``.elf`` (Executable and Linkable Format) files will be available.

Executing Binary on the DevKit
===============================

To execute the binary on the DevKit, follow the command:

.. code-block:: console

   west flash

Console Output
===============

.. note::
   The console output depends on the ADC configuration, such as single-ended or
   differential mode, channel selection, and single-shot or continuous
   conversion. Refer to the ADC sample application in
   ``../alif/samples/drivers/adc`` and build it with the ``-S alif-adc`` option
   for the expected output. Typical output includes voltage readings or
   temperature-sensor data in the format defined by the application.

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

The ``alif-adc`` sample builds only when a ``alif,adc`` compatible node is
``okay``. The snippet enables ``adc0``. Temperature is printed only for
12-bit ADC12 results. Channel 6 is the internal temperature sensor.

.. note::

   On Balletto (B1, including A5) and E1C:

   - **ADC12 instance 0 (ADC120), channels 0 and 1** are multiplexed with
     ``SE_UART_RX`` and ``SE_UART_TX``. Flash with the SE Tool first, then
     apply analog input.
   - **ADC120 channel 2** is connected to OSPI (``OSPI0_SS0``) and **cannot**
     be used as an ADC input.
   - **ADC12 instances 0 and 1, channel 7** (internal VREF) is not available.

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

Build an ADC Application with Zephyr
========================================

Follow these steps to build the ADC application using the Alif Zephyr SDK:

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, refer to the `ZAS User Guide`_.

.. note::
   The first commands are for the Alif E7 DevKit. B1 (Balletto A5) and E1C
   examples follow. Change the SoC string if your kit uses a different
   variant. Refer to the `ZAS User Guide`_, under
   ``Setting Up and Building Zephyr Applications``.

2. Build command for application on the M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/adc \
     -S alif-adc

3. Build command for application on the M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/adc \
     -S alif-adc

4. Build command for the Alif B1 / Balletto A5 DevKit (M55 HE):

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820hh0/rtss_he \
     ../alif/samples/drivers/adc \
     -S alif-adc

5. Build command for the Alif E1C DevKit (M55 HE):

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     ../alif/samples/drivers/adc \
     -S alif-adc

Once the build command completes successfully, executable images will be generated and placed in the ``build/zephyr`` directory. Both ``.bin`` (binary) and ``.elf`` (Executable and Linkable Format) files will be available.

ADC Interface
=============

The ADC12 and ADC24 modules interface with the external environment through designated pins on the DevKit devices.

Hardware Connections
====================

**ADC12**

No hardware connection is required to test the temperature sensor, which is
internally connected to all instances of ADC12 (0, 1, and 2). The default
``alif-adc`` sample uses **channel 6** (temperature).

**Balletto A5: ADC120 channel 0 on P0_0**

On Balletto A5, ADC12 instance 0 (ADC120) channel 0 is pin **P0_0**.

1. Program the binary with the **SE Tool**. Do not connect the analog source
   yet.
2. After flashing, connect the analog input to **P0_0**.
3. Reset the board. ADC120 channel 0 then reads the input consistently.

ADC120 **channel 2** is connected to OSPI and cannot be used.

To run this external-input case, set the sample to channel 0:

.. code-block:: c

   struct adc_channel_cfg channel_cfg = {
       .differential = 0,
       .channel_id   = ADC_CHANNEL_0,
   };

**Setup for Checking Single-Ended Conversion from an External Input Source**

On Ensemble E7/E8, external channels 0–5 may be used as shown below.
Channels 6 and 7 are internal (temperature and VREF) and are not external
inputs. On Balletto and E1C, ADC120 channel 2 is not available, and
channel 7 is not available on ADC12 instances 0 and 1.

.. figure:: _static/single_ended_connections_for_ADC_12.png
   :alt: Single-Ended Conversion Setup for ADC 12
   :align: center

   Setup for Single-Ended Conversion for ADC 12

**Setup for Checking Differential Input Conversion from an External Input Source**

On Ensemble E7/E8, differential channels 0, 1, and 2 may be used. On Balletto
and E1C, ADC120 channel 2 is connected to OSPI and cannot be used.

Enable differential mode from the ADC sample application:

.. code-block:: c

   struct adc_channel_cfg channel_cfg = {
       .differential = 1,
       .channel_id   = ADC_CHANNEL_0,
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

To execute the binary with J-Link:

.. code-block:: console

   west flash

On Balletto A5, program the binary with the **SE Tool** (ATOC into MRAM).
Refer to **SE Tool Flashing for Alif DevKits**. Set ``cpu_id`` to
``M55_HE``. Connect the analog source to **P0_0** only after flashing, then
reset the board.

Console Output
===============

Default sample (ADC12 channel 6, internal temperature; no external wiring):

.. code-block:: text

   [00:00:00.000,000] <inf> ALIF_ADC: Allocated memory buffer Address is 0x20002050
   [00:00:00.000,000] <inf> ALIF_ADC: Current temp 21.2 C
   [00:00:00.000,000] <inf> ALIF_ADC: ADC sampling Done

The buffer address varies at runtime. Temperature is printed only for 12-bit
ADC12 results. For Balletto A5 channel 0 on P0_0, the log shows a conversion
result instead of temperature.

PM Support
==========

The ADC sample supports Zephyr Power Management (PM) states on Alif
Ensemble and Balletto RTSS cores. The application reads ADC channel 6
(the on-chip temperature sensor) and checks that the ADC resumes after
the supported PM transitions. No external analog input is required.

The following PM states are supported:

* **PM_STATE_RUNTIME_IDLE**: Light sleep. The CPU clock is gated using WFI
  for quick wakeup.
* **PM_STATE_SUSPEND_TO_IDLE**: CPU sleep with IWIC. Devices remain active
  without device-PM overhead.
* **PM_STATE_SUSPEND_TO_RAM (S2RAM)**: Deep sleep with retention on the HE
  core when booting from TCM.

  * Substate 0: STANDBY
  * Substate 1: STOP

* **PM_STATE_SOFT_OFF**: Deepest sleep without retention. The system performs
  a full reset on wakeup.

The ADC conversion runs before ``RUNTIME_IDLE`` and again after
``SUSPEND_TO_IDLE``, S2RAM STANDBY, and S2RAM STOP. It is not repeated
after the ``RUNTIME_IDLE`` wakeup.

HE cores support the S2RAM path when booting from TCM and the SOFT_OFF path
when booting from MRAM. HP cores on E7 and E8 use the SOFT_OFF path.

On E1C, console logs use LPUART. Disconnect the debugger before testing,
because it can block OFF states.

PM Test Sequence
----------------

The S2RAM PM test performs the following sequence:

1. Read the ADC temperature before entering the PM states.
2. Enter ``PM_STATE_RUNTIME_IDLE`` and wake up.
3. Enter ``PM_STATE_SUSPEND_TO_IDLE`` and wake up.
4. Read the ADC temperature after ``SUSPEND_TO_IDLE``.
5. Enter ``PM_STATE_SUSPEND_TO_RAM`` with substate 0 (STANDBY) and wake up.
6. Read the ADC temperature after S2RAM STANDBY.
7. Enter ``PM_STATE_SUSPEND_TO_RAM`` with substate 1 (STOP) and wake up.
8. Read the ADC temperature after S2RAM STOP.
9. Complete the PM sequence.

The SOFT_OFF configuration follows the ``RUNTIME_IDLE`` and
``SUSPEND_TO_IDLE`` states before entering ``PM_STATE_SOFT_OFF``. The
system resets when it wakes from ``SOFT_OFF`` and restarts ``main()``.


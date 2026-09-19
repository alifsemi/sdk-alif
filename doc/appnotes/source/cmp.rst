.. _appnote-zephyr-alif-cmp:

===
CMP
===

Introduction
============

This application note describes digital control to process data from the Analog Comparator.

- The **High-Speed Comparator (CMP)** module is a rail-to-rail, multi-input, analog comparator with programmable reference voltage and hysteresis.
- Reference voltage can be sourced from:
  - DAC6,
  - Internal VREF, or
  - External pins.
- **Programmable hysteresis**: from 0 mV to 45 mV.
- **Comparator result inverter** (polarity control).
- **Configurable number of taps** for digital filtering.
- **Interrupt generation** after filtering is applied.

High-Speed Comparator
-----------------------

.. figure:: _static/cmp.png
    :alt: CMP Block Diagram
    :align: center

    CMP Block Diagram

- The HSCMP compares the voltages at its two inputs (i.e., the positive and negative terminal voltages).
  If the **positive input voltage** is greater than the **negative input voltage**, the comparator outputs a **logic 1 (high)**.
  Otherwise, it outputs a **logic 0 (low)**.

- When the two input voltages are very close, the comparator output may oscillate rapidly due to noise or minor fluctuations.
  To prevent this, the HSCMP includes **programmable hysteresis** (from 0 mV to 45 mV), which introduces a voltage threshold gap between rising and falling transitions.

- **Polarity control** allows the comparator output to be inverted as needed.

- **Filter control** provides digital filtering with a configurable number of taps to suppress high-frequency glitches and ensure stable output transitions.

Hardware Requirements and Setup
===============================

Comparator Register:
--------------------

.. list-table::
   :header-rows: 1
   :widths: 25 10 10 55

   * - Register
     - Offset
     - Access
     - Description
   * - ``CMP_COMP_REG1``
     - 0x0
     - R/W
     - Comparator Register 1
   * - ``CMP_COMP_REG2``
     - 0x4
     - R/W
     - Comparator Register 2
   * - ``CMP_POLARITY_CTRL``
     - 0x8
     - R/W
     - CMP Polarity Control Register
   * - ``CMP_WINDOW_CTRL``
     - 0xC
     - R/W
     - CMP Window Control Register
   * - ``CMP_FILTER_CTRL``
     - 0x10
     - R/W
     - CMP Filter Control Register
   * - ``CMP_PRESCALER_CTRL``
     - 0x14
     - R/W
     - CMP Prescaler Control Register
   * - ``CMP_STATUS``
     - 0x18
     - R
     - CMP Status Register
   * - ``CMP_INTERRUPT_STATUS``
     - 0x20
     - W1C
     - CMP Interrupt Status and Clear Register
   * - ``CMP_INTERRUPT_MASK``
     - 0x24
     - R/W
     - CMP Interrupt Mask Register

Analog Comparator Inputs
--------------------------

.. figure:: _static/analog_cmp.png
    :alt: Analog Comparator Inputs
    :align: center

    Analog Comparator Inputs

Hardware Setup
---------------

- **For the CMP0 instance**:
  The user can select one of the following pins as the **positive input terminal**:
  ``P0_0``, ``P0_6``, ``P1_4``, or ``P0_4``.

  The **negative input terminal** can be selected from:
  ``P2_0``, ``P2_1``, **Internal VREF**, or **DAC6**.

  The same pin options apply to other CMP instances (CMP1–CMP3).

- **Wiring for testing**:
  Use a wire to connect **P0_0** (configured as the positive terminal) to **P12_3** (LED output, labeled on header **J14**).

- **Negative terminal configuration**:
  Configure the negative input as **DAC6**, which is internally set to **0.8 V**.
  No external hardware connection is required for this reference voltage.

- **Prerequisite**:
  The **LED Blinky application** must be running on the board, with **P12_3 toggling every 1 second** (i.e., 1 Hz square wave).

- **Comparator output observation**:
  Monitor the following pins for comparator outputs:

  - **CMP0 output**: ``P14_7``
  - **CMP1 output**: ``P14_6``
  - **CMP2 output**: ``P14_5``
  - **CMP3 output**: ``P14_4``

Analog Comparator Operation
=============================

Comparator Configuration Steps
--------------------------------

1. **Configure ``CMP_COMP_REG1``**:
   - Select the **positive input terminal**, **negative input terminal**, and set **hysteresis to 45 mV**.

2. **Enable High-Speed Comparators** in ``COMP_REG1``:
   - **Bit 28**: Enable for ``COMPHS0``
   - **Bit 29**: Enable for ``COMPHS1``
   - **Bit 30**: Enable for ``COMPHS2``
   - **Bit 31**: Enable for ``COMPHS3``

3. **Polarity Control** (register ``CMP_POLARITY_CTRL`` at offset ``0x08``):
   - Write ``0x1`` to **invert** the ``Comp_in`` signal.
   - If set to ``0x0``, ``Comp_in`` passes **directly** (via synchronizer) without inversion before sampling.

4. **Filter Control** (register ``CMP_FILTER_CTRL`` at offset ``0x10``):

   Write value ``0x0501``:

   - **Bit [0] = 1**: Enables digital filtering.
   - **Bits [11:8] = 0x5**: Requires ``Comp_in`` to be **stable for 5 consecutive samples** with a different value than ``Comp_out`` before updating ``Comp_out``.

5. **Prescaler Control** (register ``CMP_PRESCALER_CTRL`` at offset ``0x14``):
   - Write ``0x8`` → Comparator input is sampled **every 8 system clocks** (i.e., at clocks 0, 8, 16, 24, 32, …).
   - Samples at intermediate clocks are **ignored**.

Example: Using CMP0 Instance
------------------------------

1. In ``devicetree``, configure input muxes for **CMP0** node.
2. Set **positive input** via:
   ``CMP_POS_IN0`` → selects analog pin **P0_0**.
3. Set **negative input** via:
   ``CMP_NEG_IN3`` → uses **DAC6** as internal 0.8 V reference.
4. **Hardware setup**: Connect **P0_0** to **P12_3** (labeled LED output on header **J14**).
5. **DAC6** is internally configured to **0.8 V**—no external wiring needed for the negative input.
6. **Prerequisite**: The **LED Blinky application** must be running, causing **P12_3 to toggle every 1 second** (1 Hz square wave).

This setup allows the comparator to detect the toggling signal on P0_0 (driven by the LED output) against a stable 0.8 V threshold, with filtering and hysteresis ensuring clean output transitions.

.. include:: prerequisites.rst

.. include:: note.rst

Build a CMP Application with Zephyr
====================================

Follow these steps to build the CMP application using the Alif Zephyr SDK:

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, refer to the `ZAS User Guide`_.

.. note::
   The build commands shown here are specifically for the Alif E7 DevKit.
   To build the application for other boards, modify the board name in the
   build command accordingly. For more information, refer to the
   `ZAS User Guide`_, under the section
   ``Setting Up and Building Zephyr Applications``.

   The ``alif-cmp`` snippet enables **cmp0** and disables **lpcmp**.
   The application builds only when a ``alif,cmp`` compatible node is
   ``okay`` in the devicetree.

   To use **LPCMP**, set ``status = "okay"`` on the ``lpcmp`` node and
   ``status = "disabled"`` on ``cmp0`` in an application overlay.

   On boards such as the Alif E7 DevKit RTSS-HP, the **CMP2** and **CMP3**
   pins are shared with **UART2**. Before building a CMP2/CMP3 configuration,
   move the console to **UART4** so those pins are free.

   LPCMP is supported on the Alif E7 DevKit RTSS-HE and RTSS-HP, but the
   board has no LPCMP output pinouts.

2. Build command for application on the M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/cmp \
     -S alif-cmp

3. Build command for application on the M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/cmp \
     -S alif-cmp

LPCMP Build
-----------

The snippet overlay leaves ``lpcmp`` disabled. To run LPCMP, add an
application overlay that contains:

.. code-block:: dts

   &cmp0 {
           status = "disabled";
   };

   &lpcmp {
           status = "okay";
   };

Then build with the same ``-S alif-cmp`` command used for CMP0.

Once the build command completes successfully, executable images will be generated and placed in the ``build/zephyr`` directory. Both ``.bin`` (binary) and ``.elf`` (Executable and Linkable Format) files will be available.

Supported Boards
================

The ``alif-cmp`` snippet applies a board overlay for these targets:

.. list-table::
   :header-rows: 1
   :widths: 28 18 54

   * - Board
     - Cores
     - West board target (example)
   * - Alif E7 DevKit / AppKit
     - RTSS-HE, RTSS-HP
     - ``alif_e7_dk/ae722f80f55d5xx/rtss_he``
   * - Alif E8 DevKit / AppKit
     - RTSS-HE, RTSS-HP
     - ``alif_e8_dk/ae822fa0e5597xx0/rtss_he``
   * - Alif E1C DevKit
     - RTSS-HE
     - ``alif_e1c_dk/ae1c1f4051920hh/rtss_he``
   * - Alif B1 DevKit
     - RTSS-HE
     - ``alif_b1_dk/ab1c1f4m51820hh0/rtss_he``

E7 and E8 use ``alif_e4_e7_e8_dk.overlay`` (LED on GPIO12 pin 3).
B1 and E1C use ``alif_b1_e1c_dk.overlay`` (LED on GPIO4 pin 5).

Executing Binary on the DevKit
===============================

To execute the binary on the DevKit, run:

.. code-block:: console

   west flash

CMP Console Output
====================

The following log is observed during execution of the Analog Comparator (CMP) application:

.. code-block:: text

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

LPCMP Console Output
======================

When LPCMP is enabled (and ``cmp0`` is disabled), the following log is observed.
The toggling comparison messages are not printed for LPCMP.

.. code-block:: text

    [00:00:02.000,000] <inf> ALIF_CMP: start comparing
    [00:00:02.501,000] <inf> ALIF_CMP: Comparison Completed
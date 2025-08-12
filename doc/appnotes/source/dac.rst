.. _appnote-zephyr-alif-dac:

===
DAC
===

Introduction
============

The Digital-to-Analog Converter (DAC12) module converts 12-bit digital values into analog voltage signals, with an analog output range of 0 V to 1.8 V in Low-Power (LP) mode. The device includes two DAC12 modules, each supporting the following features:

- **Conversion rate**: Up to 1 kHz at 12-bit resolution
- **Input data**: Unsigned binary or two’s complement signed digital data
- **Programmable output current**: Up to 1.5 mA
- **Programmable load capacitance compensation**
- **Internal 1.8 V voltage reference**
- **High-frequency Power Supply Rejection Ratio (PSRR)**
- **Maximum current output**: 1.5 mA
- **Modes**:
  - **High-Performance (HP)**: Supports larger resistive loads at higher power consumption
  - **Low-Power (LP)**: Supports slower sample rates and light resistive loads for power savings

  .. figure:: _static/dac_diagram.png
   :alt: DAC Configuration Diagram
   :align: center

   Diagram of the DAC Configuration


Hardware Requirements
=====================

- Alif Ensemble Devkit
- Debugger (ULINKpro or JLink, optional)

Software Requirements
=====================

- **Alif Zephyr SDK**: Clone from `https://github.com/AlifSemiDev/sdk-alif <https://github.com/AlifSemiDev/sdk-alif>`_

Hardware Connection and Setup


Output Calculation
==================

**For Positive Input**

Formula:

.. math::

   DAC_{out} = \frac{DAC_{Input}}{2^{12}} \times V_{ref}

Example:

- Given: \( DAC_{Input} = 4000 \), \( V_{ref} = 1.8 \, \text{V} \)
- Calculation: \( DAC_{out} = \frac{4000}{4096} \times 1.8 \, \text{V} = 1.757 \, \text{V} \)

**For Negative Input**

Formula:

- If \( DAC_{Input} > 2047 \):

.. math::

   DAC_{out} = \frac{DAC_{Input} - 2047}{2^{12}} \times V_{ref}

- If \( DAC_{Input} < 2047 \):

.. math::

   DAC_{out} = \frac{DAC_{Input} + 2047}{2^{12}} \times V_{ref}

Note: If DAC input is ``0xFFFFFFFFFFFFFFFF`` (-1 in decimal), the lower 12 bits (``0xFFF``, or 4095 in decimal) are considered.

Device Tree Specification
=========================

Users can modify parameters to convert two's complement to unsigned binary data and select the DAC input data source.

**Input MUX:**

- ``input_mux_val = 1``: DAC input through DAC_REG1 (bypass mode).
- ``input_mux_val = 0``: DAC input through DAC_IN register.

**Two’s Complement:**

- ``dac_twoscomp_in = 0``: Positive input.
- ``dac_twoscomp_in = 1``: Negative input.

.. figure:: _static/device_tree_code_dac.png
   :alt: Device Tree Code Snippet
   :align: center

   Device Tree Code Snippet

Building DAC Application in Zephyr
==================================

Follow these steps to build the DAC application using the Alif Zephyr SDK:

.. note::
   The application is designed for the Alif Ensemble E7 DevKit. Modify the sample code as needed for other DevKits.

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, please refer to the `ZAS User Guide`_

2. Build for M55 HP core:

.. code-block:: bash

   rm -rf build/
   west build -b alif_e7_dk_rtss_hp ../alif/samples/drivers/dac -p -- -G"Unix Makefiles"

3. Build for M55 HE core:

.. code-block:: bash

   rm -rf build/
   west build -b alif_e7_dk_rtss_he ../alif/samples/drivers/dac -p -- -G"Unix Makefiles"

Upon successful build, executable images (``.bin`` and ``.elf``) will be generated in the ``build/zephyr`` directory.

Loading the Binary on the Alif Ensemble Devkit
==============================================

To execute binaries on the DevKit-E7 board using ULINKpro Debugger in Arm DS:

1. Open the **Debug Configuration** window in Arm DS.

   .. figure:: _static/debug_config_window.png
      :alt: Debug Configuration Window
      :align: center

      Debug Configuration Window

2. In the **Connection** tab:

   - Ensure the correct Core and ULINKpro selections are made.
   - In the **Select Target** section, choose:
     - ``Cortex-M55_0`` for M55-HP core.
     - ``Cortex-M55_1`` for M55-HE core.

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

Expected Result
===============

The sample Output will be as follows

   .. figure:: _static/dac_sample_output.png
      :alt: DAC Sample Output
      :align: center

      DAC Sample Output
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

.. include:: prerequisites.rst

.. include:: note.rst

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

Device Tree Code Snippet
------------------------

.. code-block:: dts

   dac0: dac0@49028000 {
		compatible = "alif,dac";
		reg = <0x49028000 0x1000>,
			<0x49023000 0x100>,
			<0x1A60A000 0x100>,
			<0x4902B000 0x100>;
		reg-names = "dac_reg","cmp_reg","vbat_reg","adc_vref";
		pinctrl-names = "default";
		pinctrl-0 = <&pinctrl_dac0>;
		output_current = "DAC_1200UA_OUT_CUR";
		capacitance = "DAC_8PF_CAPACITANCE";
		status = "disabled";
	};

   dac1: dac1@49029000 {
	   compatible = "alif,dac";
	   reg = <0x49029000 0x1000>,
		  <0x49023000 0x100>,
		  <0x1A60A000 0x100>,
		  <0x4902B000 0x100>;
	   reg-names = "dac_reg","cmp_reg","vbat_reg","adc_vref";
	   pinctrl-names = "default";
	   pinctrl-0 = <&pinctrl_dac1>;
	   output_current = "DAC_1200UA_OUT_CUR";
	   capacitance = "DAC_8PF_CAPACITANCE";
	   status = "disabled";
   };


Building an DAC Application with Zephyr
=========================================

Follow these steps to build the DAC application using the Alif Zephyr SDK:

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, please refer to the `ZAS User Guide`_


.. note::
   The build commands shown here are specifically for the Alif E7 DevKit.
   To build the application for other boards, modify the board name in the build command accordingly. For more information, refer to the `ZAS User Guide`_, under the section Setting Up and Building Zephyr Applications.


2. Build commands for applications on the M55 HE core:

.. code-block:: bash

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/dac


3. Build commands for applications on the M55 HP core:

.. code-block:: bash

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/dac


Once the build command completes successfully, executable images will be generated and placed in the `build/zephyr` directory. Both `.bin` (binary) and `.elf` (Executable and Linkable Format) files will be available.


Executing Binary on the DevKit
==============================================

To execute binaries on the DevKit follow the command

.. code-block:: bash

   west flash

Sample Output
===============

.. code-block:: bash

   >>>Starting up the Zephyr DAC demo!!! <<<

The sample Output will be as follows

   .. figure:: _static/dac_sample_output.png
      :alt: DAC Sample Output
      :align: center

      DAC Sample Output


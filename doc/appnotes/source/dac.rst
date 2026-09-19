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


Build a DAC Application with Zephyr
====================================

Follow these steps to build the DAC application using the Alif Zephyr SDK:

For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr
repository, refer to the `ZAS User Guide`_.

Alif E7 DevKit
----------------

Build for SoC variant ``ae722f80f55d5xx``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/dac

Build for SoC variant ``ae722f80f55d5xx``, M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/dac

Build for SoC variant ``ae302f80f55d5xx``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/dac


Build for SoC variant ``ae302f80f55d5xx``, M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/dac

Alif E7 AppKit
----------------

Build for SoC variant ``ae722f80f55d5xx``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/dac


Build for SoC variant ``ae722f80f55d5xx``, M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/dac

Alif E8 DevKit
---------------

Build for SoC variant ``ae822fa0e5597xx0``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/dac

Build for SoC variant ``ae822fa0e5597xx0``, M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/dac

Build for SoC variant ``ae402fa0e5597xx0``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/dac

Build for SoC variant ``ae402fa0e5597xx0``, M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/dac

Alif E8 AppKit
----------------

Build for SoC variant ``ae822fa0e5597xx0``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/dac

Build for SoC variant ``ae822fa0e5597xx0``, M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/dac

Alif E1C DevKit
----------------

Build for SoC variant ``ae1c1f4051920hh``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     ../alif/samples/drivers/dac

Alif B1 DevKit
---------------

Build for SoC variant ``ab1c1f4m51820ph0``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     ../alif/samples/drivers/dac

Build for SoC variant ``ab1c1f4m51820hh0``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820hh0/rtss_he \
     ../alif/samples/drivers/dac

Build for SoC variant ``ab1c1f1m41820hh0``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     ../alif/samples/drivers/dac

Build for SoC variant ``ab1c1f1m41820ph0``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     ../alif/samples/drivers/dac

Once the build command completes successfully, executable images will be generated and placed in the ``build/zephyr`` directory. Both ``.bin`` (binary) and ``.elf`` (Executable and Linkable Format) files will be available.

Executing Binary on the DevKit
===============================

To execute binaries on the DevKit, follow the command:

.. code-block:: console

   west flash

Sample Output
===============

.. code-block:: text

   >>>Starting up the Zephyr DAC demo!!! <<<

The sample Output will be as follows:

   .. figure:: _static/dac_sample_output.png
      :alt: DAC Sample Output
      :align: center

      DAC Sample Output


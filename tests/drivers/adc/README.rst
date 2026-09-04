.. zephyr:code-sample:: alif-adc
	name: Analog-to-digital converter (ADC12 and ADC24)

	Read analog input from ADC12 module 1 (SAR) and the ADC24 module (Sigma-Delta).
###########

Overview
********
This test suite validates the Analog-to-Digital Converter (ADC) drivers for Alif Semiconductor's ADC12 instance 1 (SAR) and ADC24 (Sigma-Delta) modules.

The tests verify:
* **ADC12 (adc1)**: 12-bit Successive Approximation Register (SAR) ADC supporting 8 channels (6 external, 2 internal). It includes tests for single-ended inputs and differential pairs.
* **ADC24 (adc24)**: 24-bit Sigma-Delta ADC supporting 4 differential input channels.
* **Temperature Sensor**: Internal sensor connected to ADC12 Channel 6, ensuring correct thermal-to-digital conversion.

Support for ADC Testcases:
* **Input Modes**: Single-ended (ADC12) and Differential (ADC12/ADC24).
* **Sampling**: Configurable oversampling/averaging (1 to 256 samples).
* **Gain & Reference**: Validation of gain settings and reference voltage selection via devicetree overlays.
* **Conversion Modes**: Implementation of both One-shot and Continuous conversion modes.
* **Scanning**: Single-channel and Multi-channel (scan) modes using unmasked channel sequences.
* **Error Handling**: Verification of invalid channel configurations and out-of-range sensor values.
*************************************************************************
				NOTE
*************************************************************************
To build the ADC tests, pass Kconfig flags with ``-DCONFIG_...=y`` and DT overlays
with ``DTC_OVERLAY_FILE``. There are no extra ``.conf`` fragments; ``prj.conf`` is
the only config file. Twister variants in ``testcase.yaml`` pass the same flags
via ``extra_configs``.

**Basic build (ADC12 instance 1, channel 0):**

.. code-block:: console

   west build -p always -b < board > tests/drivers/adc \
   -DCONFIG_TEST_ADC_CH0=y \
   -DDTC_OVERLAY_FILE="boards/alif_adc1.overlay"

Pass additional ``-DCONFIG_TEST_ADC_CH<N>=y`` flags to include more channel tests
(0-7). Channel tests are off unless you enable them on the command line.

**Example: ADC12 differential**

.. code-block:: console

   west build -p always -b < board > tests/drivers/adc \
   -DCONFIG_TEST_ADC_DIFFERENTIAL=y \
   -DCONFIG_TEST_ADC_CH0=y -DCONFIG_TEST_ADC_CH1=y \
   -DDTC_OVERLAY_FILE="boards/alif_adc1.overlay"

**Example: ADC12 continuous**

.. code-block:: console

   west build -p always -b < board > tests/drivers/adc \
   -DCONFIG_TEST_ADC_CONTINUOUS=y \
   -DDTC_OVERLAY_FILE="boards/alif_adc1.overlay;boards/alif_adc1_continuous.overlay"

**Example: ADC12 multi-channel scan**

.. code-block:: console

   west build -p always -b < board > tests/drivers/adc \
   -DCONFIG_TEST_ADC_MultiCH=y \
   -DDTC_OVERLAY_FILE="boards/alif_adc1.overlay;boards/alif_adc1_multich.overlay"

**Example: Continuous mode with differential input on ADC24:**

.. code-block:: console

   west build -p always -b < board > tests/drivers/adc \
   -DCONFIG_TEST_ADC24=y -DCONFIG_TEST_ADC_CONTINUOUS=y -DCONFIG_TEST_ADC_DIFFERENTIAL=y \
   -DDTC_OVERLAY_FILE="boards/alif_adc24.overlay;boards/alif_adc24_continuous.overlay"

**Available Configuration Flags (Kconfig):**

Specify these using ``-DCONFIG_<FLAG>=y``:

* ``CONFIG_TEST_ADC_CH<N>=y``: Enables channel test N (0-7). Default is off.
* ``CONFIG_TEST_ADC_CONTINUOUS=y``: Enables Continuous conversion mode.
* ``CONFIG_TEST_ADC_DIFFERENTIAL=y``: Enables Differential input mode.
* ``CONFIG_TEST_ADC_MultiCH=y``: Enables multi-channel scan. This also enables continuous conversion (single-shot + scan is not supported).
* ``CONFIG_TEST_ADC24=y``: Builds ADC24 testcases instead of ADC12.


**Overlay files:**

All overlay files are located in the ``boards/`` directory. Append them to ``DTC_OVERLAY_FILE``.
Kconfig flags are passed on the west command line with ``-DCONFIG_...=y`` (not ``OVERLAY_CONFIG``).

*   **Base Module Overlays** (Primary selection):
    *   ``alif_adc1.overlay``: ADC12 Module 1.
    *   ``alif_adc24.overlay``: ADC24 Module.

*   **Feature-Specific Overlays** (Append to the matching base module):
    *   Continuous Mode: ``alif_adc1_continuous.overlay`` or ``alif_adc24_continuous.overlay``
    *   Multi-Channel scan: ``alif_adc1_multich.overlay`` or ``alif_adc24_multich.overlay`` (also sets continuous conversion)
    *   Gain configuration: ``alif_adc1_gain.overlay`` or ``alif_adc24_gain.overlay``
    *   Oversampling (Samples): ``alif_adc1_samples<X>.overlay`` or ``alif_adc24_samples<X>.overlay`` (X=1, 2, 4, 8, 16, 32, 64, 128)
    *   ADC24 Maximum Connections: ``alif_adc24_max_con.overlay``

Building and Running
********************

The testcode will build only for a target that has a devicetree entry with
:dt compatible:`alif,adc` as a compatible.

Twister
=======

.. code-block:: console

   twister -T tests/drivers/adc -p < board >

Scenarios include ADC12 on adc1 (single-ended, continuous, differential,
multi-channel, gain, samples8) and ADC24 (differential, continuous, multi-channel).

console Output
==============

Example output for ADC12 single-ended test:

.. code-block:: console

## Hardware Requirements
- Alif development board with ADC support
- ADC input signal for testing (can be internal VREF for basic functionality)
- Proper devicetree configuration for ADC channels

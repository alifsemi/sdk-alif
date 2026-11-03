.. zephyr:code-testcase:: alif-adc
	name: Analog-to-digital converter (ADC12 and ADC24)

	Read analog input from ADC12 module 1 (SAR) and the ADC24 module (Sigma-Delta).

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
* **Threshold / Window**: Comparator threshold A/B and above, below, or window comparison via overlays.
* **ADC & DAC**: Simultaneous enable of an ADC instance with DAC0 via combination overlays.
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
Balletto has no ADC12 channel 7, so leave ``CONFIG_TEST_ADC_CH7`` off on B1.

Building differential mode without any ``-DCONFIG_TEST_ADC_CH<N>=y`` compiles the
invalid-channel error-scenario tests instead, which check that
``adc_channel_setup()`` rejects channels outside the differential range. The
``drivers.adc.adc12.differential_invalid_channel`` and
``drivers.adc.adc24.differential_invalid_channel`` Twister scenarios cover this.

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
   -DCONFIG_TEST_ADC_MULTICH=y \
   -DDTC_OVERLAY_FILE="boards/alif_adc1.overlay;boards/alif_adc1_multich.overlay"

**Example: PGA gain**

Enable with ``-DCONFIG_TEST_ADC_PGA=y`` (separate from DAC and threshold).
Each gain overlay enables ``pga_enable`` and sets ``pga_gain``. The ztest
name is ``test_adc12_pga_gain`` (or ``test_adc24_pga_gain``). Configurable
values:

* ``ADC_PGA_GAIN_0_DB``  — ``alif_adc1_gain0.overlay`` / ``alif_adc24_gain0.overlay``
* ``ADC_PGA_GAIN_12_DB`` — ``alif_adc1_gain12.overlay`` / ``alif_adc24_gain12.overlay``
* ``ADC_PGA_GAIN_24_DB`` — ``alif_adc1_gain24.overlay`` / ``alif_adc24_gain24.overlay``
* ``ADC_PGA_GAIN_36_DB`` — ``alif_adc1_gain36.overlay`` / ``alif_adc24_gain36.overlay``
* ``ADC_PGA_GAIN_42_DB`` — ``alif_adc1_gain42.overlay`` / ``alif_adc24_gain42.overlay``
  (also ``alif_adc1_gain.overlay`` / ``alif_adc24_gain.overlay``)

Append one gain overlay to the matching base module. Optionally pass
``-DCONFIG_TEST_ADC_CHn=y`` to select the channel (default channel 1).
Example (ADC12, 0 dB, channel 1):

.. code-block:: console

   west build -p always -b < board > tests/drivers/adc \
   -DCONFIG_TEST_ADC_PGA=y -DCONFIG_TEST_ADC_CH1=y \
   -DDTC_OVERLAY_FILE="boards/alif_adc1.overlay;boards/alif_adc1_gain0.overlay"

**Example: Threshold and window detection**

Enable with ``-DCONFIG_TEST_ADC_THRESHOLD=y``. That builds
``test_adc_threshold.c`` and skips the plain channel ztests so the UART log
shows a dedicated threshold name (``test_adc12_threshold_above`` /
``_below`` / ``_window``, or the ADC24 equivalents) instead of
``test_adc_single_ended_input_channelN``. Optionally add
``-DCONFIG_TEST_ADC_CHn=y`` to pick the channel (default CH0).

Comparison mode is set only by a feature overlay. Only one mode can be
programmed at a time. Each overlay sets ``comparator_threshold_a``,
``comparator_threshold_b``, ``comparator_threshold_comparison``, and
``interrupt_en``:

* ``ABOVE_THRESHOLD_A_AND_B`` — ``alif_adc1_thresh_above.overlay`` /
  ``alif_adc24_thresh_above.overlay``
* ``BELOW_THRESHOLD_A_AND_B`` — ``alif_adc1_thresh_below.overlay`` /
  ``alif_adc24_thresh_below.overlay``
* ``BETWEEN_AND_OUTSIDE_THRESHOLD_A_AND_B`` —
  ``alif_adc1_thresh_window.overlay`` / ``alif_adc24_thresh_window.overlay``

Append one of these to the matching base module. Do not combine with
``CONFIG_TEST_ADC_DAC``. Example (ADC12, window, CH0):

.. code-block:: console

   west build -p always -b < board > tests/drivers/adc \
   -DCONFIG_TEST_ADC_THRESHOLD=y -DCONFIG_TEST_ADC_CH0=y \
   -DDTC_OVERLAY_FILE="boards/alif_adc1.overlay;boards/alif_adc1_thresh_window.overlay"

**Example: ADC and DAC combination**

DAC0 drives a DC voltage and the ADC reads it back (``test_adc_dac.c``).
Enable with ``-DCONFIG_TEST_ADC_DAC=y`` (selects ``CONFIG_DAC``). Use an
ADC+DAC base overlay only — do not combine with threshold overlays or
``CONFIG_TEST_ADC_THRESHOLD``. Wire **DAC0 P2_3** to the ADC pin below.
Configurable combinations:

* ADC12 instance 1 + DAC0 — ``alif_adc1_dac0.overlay`` (``test-adc`` =
  ``&adc1``, CH0 = **P0_6**). Prefer this on Balletto / B1; ADC0 CH0
  (**P0_0**) shares UART0 RX and often sits near 1.8 V.
* ADC12 instance 0 + DAC0 — ``alif_adc12_dac0.overlay`` (``&adc0``, CH0 =
  **P0_0**)
* ADC24 + DAC0 — ``alif_adc24_dac0.overlay`` (``&adc24``; also pass
  ``-DCONFIG_TEST_ADC24=y``; positive **P0_0**, negative **P0_1** to GND)

Example (ADC12 instance 1 + DAC0, B1):

.. code-block:: console

   west build -p always -b < board > tests/drivers/adc \
   -DCONFIG_TEST_ADC_DAC=y \
   -DDTC_OVERLAY_FILE="boards/alif_adc1_dac0.overlay"

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
* ``CONFIG_TEST_ADC_MULTICH=y``: Enables multi-channel scan. This also enables continuous conversion (single-shot + scan is not supported). Differential multi-channel is ADC24 only.
* ``CONFIG_TEST_ADC24=y``: Builds ADC24 testcases instead of ADC12.
* ``CONFIG_TEST_ADC_DAC=y``: ADC reads DAC0 loopback (separate from threshold/PGA).
* ``CONFIG_TEST_ADC_THRESHOLD=y``: Comparator threshold / window (separate from DAC/PGA).
* ``CONFIG_TEST_ADC_PGA=y``: PGA gain test (separate from DAC/threshold).


**Overlay files:**

All overlay files are located in the ``boards/`` directory. Append them to ``DTC_OVERLAY_FILE``.
Kconfig flags are passed on the west command line with ``-DCONFIG_...=y`` (not ``OVERLAY_CONFIG``).

*   **Base Module Overlays** (Primary selection):
    *   ``alif_adc1.overlay``: ADC12 Module 1.
    *   ``alif_adc24.overlay``: ADC24 Module.
    *   ``alif_adc1_dac0.overlay``: ADC12 Module 1 with DAC0 (B1 preferred; P2_3→P0_6).
    *   ``alif_adc12_dac0.overlay``: ADC12 Module 0 with DAC0 (P2_3→P0_0).
    *   ``alif_adc24_dac0.overlay``: ADC24 with DAC0 (replaces the ADC-only base).

*   **Feature-Specific Overlays** (Append to the matching base module):
    *   Continuous Mode: ``alif_adc1_continuous.overlay`` or ``alif_adc24_continuous.overlay``
    *   Multi-Channel scan: ``alif_adc1_multich.overlay`` or ``alif_adc24_multich.overlay`` (also sets continuous conversion)
    *   PGA gain: ``alif_adc1_gain<X>.overlay`` or ``alif_adc24_gain<X>.overlay``
      for ``ADC_PGA_GAIN_<X>_DB`` where X is 0, 12, 24, 36, or 42.
      ``alif_adc1_gain.overlay`` / ``alif_adc24_gain.overlay`` are 42 dB.
    *   Threshold / window: ``alif_adc1_thresh_<mode>.overlay`` or
      ``alif_adc24_thresh_<mode>.overlay`` where ``mode`` is ``above``,
      ``below``, or ``window``.
    *   Oversampling (Samples): ``alif_adc1_samples<X>.overlay`` or ``alif_adc24_samples<X>.overlay`` (X=1, 2, 4, 8, 16, 32, 64, 128)
    *   ADC24 Maximum Connections: ``alif_adc24_max_con.overlay``

Building and Running
********************

The testcode will build only for a target that has a devicetree entry with
:dt compatible:`alif,adc` as a compatible.

## Hardware Requirements
- Alif development board with ADC support
- ADC input signal for testing (can be internal VREF for basic functionality)
- Proper devicetree configuration for ADC channels

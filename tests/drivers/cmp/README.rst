.. zephyr:code-sample:: alif-cmp
	name: Analog Comparator (CMP)

	Verify the Analog Comparator (CMP) functionality including hysteresis, polarity, and filter tap settings.

#########
Overview
#########

This test suite validates the Analog Comparator (CMP) driver for Alif Semiconductor devices. It covers basic comparison, polarity inversion, and digital filter configurations.

The tests verify:
* **Basic Comparison**: Validates the output state based on positive and negative input levels.
* **Hysteresis**: Verification of different hysteresis levels (6mV to 36mV).
* **Digital Filter**: Testing of various filter tap configurations (1 to 4 taps).
* **Polarity**: Verification of output polarity inversion.
* **Reference Source**: Verification of negative reference selection for DAC6, internal Vref, and external pin.
* **Trigger Modes**: Verification of all trigger configurations (rising edge, falling edge, both edges, none).
* **Output & Pending**: Verification of ``comparator_get_output`` and ``comparator_trigger_is_pending`` APIs.
* **Negative Tests**: Robustness tests including callback overwrite, trigger without callback, and repeated reads.
* **Response-Time Feature**: Manual hardware validation testcase for comparator propagation delay requirement (``< 5 ns``).

Building and Running
********************

The application will build only for a target that has a devicetree entry with the compatible ``alif,cmp``.

Manual Build Instructions
*************************

To build the CMP tests for specific configurations, use the following manual build approach.

**Basic build (Example for E7 HE):**

.. code-block:: console

   west build -p always -b alif_e7_dk/ae711ca0e5597xx0/rtss_he Zephyr_tests/tests/drivers/alif_cmp -DDTC_OVERLAY_FILE="boards/alif_e7_dk_rtss_he.overlay"

**Overlay and Config Files:**

*   **Hysteresis Overlays**:
    Apply the overlay for the desired hysteresis level and pass the corresponding
    Kconfig flag on the build command line (no separate ``.conf`` file needed):

    *   6mV: ``-DDTC_OVERLAY_FILE="boards/alif_hys6.overlay" -DCONFIG_HY6=y``
    *   12mV: ``-DDTC_OVERLAY_FILE="boards/alif_hys12.overlay" -DCONFIG_HY12=y``
    *   18mV: ``-DDTC_OVERLAY_FILE="boards/alif_hys18.overlay" -DCONFIG_HY18=y``
    *   24mV: ``-DDTC_OVERLAY_FILE="boards/alif_hys24.overlay" -DCONFIG_HY24=y``
    *   30mV: ``-DDTC_OVERLAY_FILE="boards/alif_hys30.overlay" -DCONFIG_HY30=y``
    *   36mV: ``-DDTC_OVERLAY_FILE="boards/alif_hys36.overlay" -DCONFIG_HY36=y``

    **Example** (E7 HE with 6mV hysteresis):

    .. code-block:: console

       west build -p always -b alif_e7_dk/ae711ca0e5597xx0/rtss_he Zephyr_tests/tests/drivers/alif_cmp \
           -DDTC_OVERLAY_FILE="boards/alif_e7_dk_rtss_he.overlay;boards/alif_hys6.overlay" \
           -DCONFIG_HY6=y

*   **Reference Source Overlays**:
    Apply one of the following overlays and pass the corresponding Kconfig flag
    on the build command line (no separate ``.conf`` file needed):

    * DAC6 reference:
      ``-DDTC_OVERLAY_FILE=boards/alif_ref_dac6.overlay -DCONFIG_REF_DAC6=y``

    * Internal Vref reference:
      ``-DDTC_OVERLAY_FILE=boards/alif_ref_vref.overlay -DCONFIG_REF_VREF=y``

    * External pin reference:
      ``-DDTC_OVERLAY_FILE=boards/alif_ref_ext.overlay -DCONFIG_REF_EXT=y``

    **DAC6 reference test build:**

    This test selects ``CMP_NEG_IN3`` as the comparator negative input and
    runs ``test_Comp_ref_dac6``.

    .. code-block:: console

       west build -p always -b alif_e7_dk/ae711ca0e5597xx0/rtss_he Zephyr_tests/tests/drivers/alif_cmp \
           -DDTC_OVERLAY_FILE="boards/alif_e7_dk_rtss_he.overlay;boards/alif_ref_dac6.overlay" \
           -DCONFIG_REF_DAC6=y

    **Internal Vref reference test build:**

    This test selects ``CMP_NEG_IN2`` as the comparator negative input and
    runs ``test_Comp_int_vref``.

    .. code-block:: console

       west build -p always -b alif_e7_dk/ae711ca0e5597xx0/rtss_he Zephyr_tests/tests/drivers/alif_cmp \
           -DDTC_OVERLAY_FILE="boards/alif_e7_dk_rtss_he.overlay;boards/alif_ref_vref.overlay" \
           -DCONFIG_REF_VREF=y

    **External pin reference test:**

    The external reference test selects ``CMP_NEG_IN0`` and runs
    ``test_Comp_ref_external_pin``. This test requires board support for the
    comparator negative external input pin and an external voltage source
    connected to that pin. If the current board setup does not expose/support
    the external CMP negative input, do not run this testcase as an automated
    test; use DAC6 or internal Vref instead.

    .. code-block:: console

       west build -p always -b alif_e7_dk/ae711ca0e5597xx0/rtss_he Zephyr_tests/tests/drivers/alif_cmp \
           -DDTC_OVERLAY_FILE="boards/alif_e7_dk_rtss_he.overlay;boards/alif_ref_ext.overlay" \
           -DCONFIG_REF_EXT=y

Response-Time Validation
************************

The testcase ``test_Comp_response_time_manual_hw_validation`` is added as a
manual hardware-validation hook. It configures CMP trigger/callback path and
marks the test as skipped in ztest, because absolute propagation delay in
nanoseconds cannot be validated from software timing.

To validate CMP response time against ``< 5 ns``, use external lab equipment
(signal generator + oscilloscope) and measure input crossing to comparator
output propagation delay.

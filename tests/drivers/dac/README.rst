.. _DAC-test:
##########################################################################################

.. zephyr:code-tests:: DAC
		name: Digital-to-Analog Converter (DAC12) Test Suite

#########################

Overview
********
Comprehensive test suite for the Alif DAC12 driver covering all HRM and
requirement scenarios. The DAC12 converts 12-bit digital values into analog
voltage signals with output range 0V to 1.8V (LP mode).

Requirement Coverage
********************
- **R-32_1**: Dual channel DAC control independently (test_dac_basic.c)
- **R-32_2**: Unsigned binary / two's complement data (test_dac_data_format.c)
- **R-32_3**: Continuous digital data conversion (test_dac_continuous.c)
- **R-32_4**: Programmable load capacitance compensation (test_dac_capacitance.c)
- **Boundary**: Edge case and boundary value tests (test_dac_boundary.c)

Bug Regression Test Coverage
****************************
- **R-BUG-1**: Invalid channel_id rejection (test_dac_basic.c)
  * Verifies driver returns ``-EINVAL`` for ``channel_id != 0``
  * Tests channel_id = 1 and channel_id = 255
  * Documents compliance gap with Zephyr DAC API channel validation

- **R-BUG-2**: Two's complement input range validation (test_dac_basic.c)
  * Verifies driver behavior when ``twoscomp_enabled`` with input values > 12-bit
  * Documents current permissive behavior (accepts any 32-bit value)
  * Should reject or mask values exceeding 0xFFF in twoscomp mode

- **R-BUG-3**: Rapid write stress test (test_dac_basic.c)
  * Stress test with 100 iterations of alternating min/max writes
    (200 total writes)
  * Verifies driver stability under high-frequency operation

- **R-BUG-4**: Repeated channel setup (test_dac_basic.c)
  * Verifies driver handles multiple consecutive ``dac_channel_setup()`` calls
  * Tests re-configuration without device reset

- **R-BUG-5**: Write max+1 edge case (test_dac_basic.c)
  * Verifies exact boundary rejection (DAC_MAX_INPUT + 1 = 0x1000)
  * Ensures unsigned mode properly rejects out-of-range values

Test Suites
***********
- **test_dac_basic** (13 tests): Device ready, channel setup, invalid resolution,
  basic write, DAC0/DAC1 independent init, dual channel write, dual channel
  simultaneous operation, **invalid channel_id rejection (R-BUG-1)**,
  **two's complement input range validation (R-BUG-2)**, **rapid write stress (R-BUG-3)**,
  **repeated setup (R-BUG-4)**, **max+1 edge case (R-BUG-5)**.
- **test_dac_data_format** (12 tests): Unsigned binary min/max/mid/quarter/
  three-quarter, step values, bit patterns, over-range error, two's complement
  zero/positive/negative, monotonic sweep.
- **test_dac_continuous** (9 tests): Ramp-up, ramp-down, sawtooth, triangle,
  square wave, rapid writes, 1 kHz rate, variable step, repeated value.
- **test_dac_capacitance** (7 tests): Init with capacitance, write with
  capacitance, stability, step response, ramp with settling, output current
  config, LP mode operation.
- **test_dac_boundary** (11 tests): Min/max boundaries, above-max, large value,
  min non-zero, max-1, alternating, power-of-two, repeated setup, zero
  resolution, sequential transitions.

Board Overlays
**************
- ``alif_dac0.overlay`` - Enable DAC0 only (unsigned binary, 4pF capacitance, 800uA current)
- ``alif_dac0_twoscomp.overlay`` - Enable DAC0 only with ``twoscomp_enabled`` set
- ``alif_dac1.overlay`` - Enable DAC1 only (unsigned binary, 4pF capacitance, 800uA current)
- ``alif_dac_dual.overlay`` - Enable both DAC0 and DAC1 for R-32_1 dual tests:
  * DAC0: unsigned binary, 4pF capacitance, 800uA current
  * DAC1: two's complement enabled (for R-32_2), 6pF capacitance, 1000uA current
- ``alif_dac0_max_current.overlay`` - DAC0 with **maximum** output current (1500uA / 1.5mA)
- ``alif_dac0_min_current.overlay`` - DAC0 with **minimum** output current (0uA)

Mode Selection (Unsigned vs Two's Complement)
*********************************************

The test suite uses ``twoscomp_enabled`` as a boolean property on the
active DAC node and handles mode-specific cases as follows:

- Unsigned-only over-range tests run only when ``twoscomp_enabled`` is absent.
- Two's-complement positive/negative tests run only when ``twoscomp_enabled`` is present.

This keeps unsigned overlays clean (property omitted) and enables two's
complement only when required (property present).

Build examples:

.. code-block:: console

    # Unsigned mode (default): over-range rejection tests enabled
    west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_he ../alif/tests/drivers/dac/ \
    -- -DDTC_OVERLAY_FILE=boards/alif_dac0.overlay

    # Two's-complement mode: twos-complement tests enabled
    west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_he ../alif/tests/drivers/dac/ \
    -- -DDTC_OVERLAY_FILE=boards/alif_dac0_twoscomp.overlay

    # Maximum output current (1.5 mA) - HRM feature test
    west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_he ../alif/tests/drivers/dac/ \
    -- -DDTC_OVERLAY_FILE=boards/alif_dac0_max_current.overlay

    # Minimum output current (0 uA) - light load testing
    west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_he ../alif/tests/drivers/dac/ \
    -- -DDTC_OVERLAY_FILE=boards/alif_dac0_min_current.overlay

DT Properties Tested
********************
- ``twoscomp_enabled`` - Two's complement mode (boolean: absent=unsigned,
  present=twoscomp)
- ``capacitance`` - Load capacitance trim (DAC_2PF_CAPACITANCE / DAC_4PF_CAPACITANCE /
  DAC_6PF_CAPACITANCE / DAC_8PF_CAPACITANCE)
- ``output_current`` - Output drive current (DAC_0UA_OUT_CUR to DAC_1500UA_OUT_CUR)

Known Limitations and Current Behavior
**************************************

This section documents known gaps and currently expected outcomes.

- **R-BUG-1 (channel_id validation)**
  - Current behavior: driver accepts any ``channel_id`` value silently.
  - Expected behavior: return ``-EINVAL`` for ``channel_id != 0``.
  - Location to fix: ``dac_alif.c:dac_enable()``.

- **R-BUG-2 (twoscomp range validation)**
  - Current behavior: with twoscomp enabled, driver accepts any 32-bit value.
  - Expected behavior: either mask to 12-bit or return ``-EINVAL`` for values
    greater than ``0xFFF``.
  - Test note: assertions currently document existing permissive behavior and
    should be updated after driver fix.

- **R-BUG-3 (rapid write stress)**
  - Current test coverage: 100 alternating min/max iterations
    (200 total writes).
  - Expected behavior: stable high-frequency writes with no DAC API errors.

- **R-BUG-4 (repeated setup)**
  - Current test coverage: 10 consecutive ``dac_channel_setup()`` calls.
  - Expected behavior: reliable re-configuration without failures.

- **R-BUG-5 (max+1 edge case)**
  - Current behavior: in unsigned mode, ``DAC_MAX_INPUT + 1 (0x1000)`` is
    rejected with ``-EINVAL``.
  - Test note: this check is skipped in twoscomp mode.

Building and Running
********************

The testcases will build only for a target that has a devicetree entry with
:dt compatible:`alif,dac` as a compatible.

Use the dual overlay for full dual-channel testing:

.. code-block:: console

    west build -b <board> -- -DDTC_OVERLAY_FILE=boards/alif_dac_dual.overlay

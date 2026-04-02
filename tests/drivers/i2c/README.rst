.. _i2c_ztest:

ALIF I2C validation suite
=========================

Overview
--------

This suite validates the ALIF I2C controller and Zephyr integration,
with focus on real bug detection rather than API compliance.

Test sources are organized as:

- Functional and target-mode tests: ``src/test_i2c.c``
- Stress and callback validation: ``src/test_i2c_stress.c``
- Boundary validation: ``src/test_i2c_boundary.c``
- Fault injection and error paths: ``src/test_i2c_fault_injection.c``
- Hardware-aware validation: ``src/test_i2c_hardware_aware.c``
- Aggregated validation: ``src/test_i2c_comprehensive.c``
- Throughput tests: ``src/test_i2c_performance.c``
- Sensor integration: ``src/test_i2c_bme680.c``

Key improvements:

- Consolidated duplicate negative tests into fault injection suite
- Centralized error handling validation
- Unified callback accounting model
- Reduced redundant debug paths
- Added restart-condition validation for DW driver issues

Validation strategy
-------------------

Tests target failure modes observed in silicon bring-up and stress:

- Repeated START corruption (DW RESTART bug)
- Callback ordering and byte-count drift
- FIFO boundary and oversize writes
- Buffer-mode truncation and state-reset issues
- NACK recovery failures leaving controller wedged
- Suspend/resume recovery failures
- Timing-sensitive regressions under stress

Restart condition validation
----------------------------

Dedicated tests validate DesignWare RESTART handling:

- ``i2c_stress.test_wr_restart_integrity``
- Executed via:
  - ``drivers.i2c.stress.restart``
  - ``drivers.i2c.stress.restart.buffer``

Focus:

- Data integrity across write-read restart sequences
- Correct transaction sequencing (single write + stop)
- Clear separation of capture failure vs corruption


Coverage improvements
---------------------

- Shared payload validation via ``i2c_validate_data_match()``
- Single callback state model across all suites
- Reduced duplication in ``testcase.yaml``
- Stress configuration via Kconfig (no duplicated sources)
- Centralized callback validation in stress tests

Key Kconfig options
-------------------

Top-level:

- ``CONFIG_I2C_ALL_TESTS``        Enable all suites (except performance)
- ``CONFIG_I2C_TARGET_BUFFER_MODE`` Enable target buffer mode
- ``CONFIG_I2C_STRESS_TESTS``     Enable stress tests
- ``CONFIG_I2C_ENHANCED_STRESS``  Extended stress with fault injection
- ``CONFIG_I2C_COMPREHENSIVE_VALIDATION`` Legacy aggregation

Frequency selection (multi-frequency execution):

- ``CONFIG_I2C_TEST_FREQ_STANDARD``   Include Standard mode (100 kHz) in test loops (default: y)
- ``CONFIG_I2C_TEST_FREQ_FAST``       Include Fast mode (400 kHz) in test loops (default: y)
- ``CONFIG_I2C_TEST_FREQ_FAST_PLUS``  Include Fast Plus mode (1 MHz) in test loops (default: y)
- ``CONFIG_I2C_TEST_FREQ_HIGH``       Include High Speed mode (3.4 MHz) in test loops (default: y)

Transfer-bearing tests (7bit, 10bit, tx, xcv, mrx_stx, nack_recovery) now loop through
all enabled frequencies in deterministic order: Standard -> Fast -> Fast+ -> High.
Readiness and config-only tests remain single-run.

To narrow testing to specific speeds, disable unwanted frequencies::

    west build ... -DCONFIG_I2C_TEST_FREQ_FAST_PLUS=n -DCONFIG_I2C_TEST_FREQ_HIGH=n

Granular:

- ``CONFIG_I2C_PERFORMANCE_TESTS`` (non-buffer only)
- ``CONFIG_I2C_BOUNDARY_TESTS``
- ``CONFIG_I2C_FAULT_INJECTION_TESTS``
- ``CONFIG_I2C_HARDWARE_AWARE_TESTS``

Note: 10-bit addressing is always enabled.

Buffer mode vs performance
--------------------------

Performance tests require non-buffer mode.

Performance tests:

- Validate full payload (``BUFF_PERF``)
- Track per-byte callbacks
- Require precise timing
- Depend on full data visibility
- Execute at all enabled frequencies via unified frequency registry

Buffer mode limitations:

- Payload truncation possible
- Uses ``buf_write_received``
- Batch processing affects timing

Recommended usage:

Performance:

    west build ... -DCONFIG_I2C_PERFORMANCE_TESTS=y

All tests with default frequencies (Standard, Fast, Fast+, High):

    west build ... -DCONFIG_I2C_ALL_TESTS=y

Narrowed frequency selection (Standard and Fast only):

    west build ... -DCONFIG_I2C_ALL_TESTS=y \
      -DCONFIG_I2C_TEST_FREQ_FAST_PLUS=n \
      -DCONFIG_I2C_TEST_FREQ_HIGH=n

Buffer-mode validation:

    west build ... \
      -DCONFIG_I2C_TARGET_BUFFER_MODE=y \
      -DCONFIG_I2C_STRESS_TESTS=y \
      -DCONFIG_I2C_FAULT_INJECTION_TESTS=y

Test compatibility
------------------

| Suite              | Buffer mode | Status |
|--------------------|-------------|--------|
| All tests          | Yes         | Full (no performance) |
| Performance        | No          | Non-buffer only       |
| Stress             | Yes         | Supported             |
| Enhanced stress    | Yes         | Supported             |
| Fault injection    | Yes         | Supported             |
| Hardware-aware     | Yes         | Supported             |
| Boundary           | Yes         | Supported             |

Running tests
-------------

Build:

    west build -p always -b <board> \
      zephyr/Zephyr_tests/tests/drivers/alif_i2c

Twister:

    west twister -T zephyr/Zephyr_tests/tests/drivers/alif_i2c \
      --platform <board>

Examples:

- Performance:
  ``-s drivers.i2c.performance``

- Stress:
  ``-s drivers.i2c.stress.basic``

- Restart validation:
  ``-s drivers.i2c.stress.restart``

- Fault injection:
  ``-s drivers.i2c.fault``

- Hardware-aware:
  ``-s drivers.i2c.hardware``

- BME680:
  ``-s drivers.i2c.bme680``

Configuration
-------------

- Default: basic validation
- Selective: enable suites via Kconfig
- Stress depth configurable
- Comprehensive mode intended for manual runs

Hardware requirements
---------------------

- Two I2C controllers (master + slave)
- Bus connection between controllers
- Optional: BME680 sensor

Platform support
----------------

Supported:

- alif_e7_dk (RTSS-HE / RTSS-HP)
- Other ALIF E7 platforms with dual I2C

Requirements:

- Dual I2C controllers
- ``master_i2c`` and ``slave_i2c`` DT aliases
- Compatible Zephyr driver

Test coverage
-------------

Functional:

- Read/write operations (at all enabled frequencies)
- 7-bit and 10-bit addressing (at all enabled frequencies)
- Callback sequencing
- Data integrity
- Runtime frequency switching with ENOTSUP skip handling

Performance:

- Unified frequency registry (Standard, Fast, Fast+, High)
- Configurable frequency selection via Kconfig
- Latency and throughput at each enabled speed
- Speed reconfiguration scenarios

Robustness:

- Boundary and edge cases
- Fault injection and recovery
- Stress iterations
- Restart validation

Integration:

- Sensor integration (BME680)
- Cross-suite validation
- Multi-frequency test execution in single binary

Failure detection
-----------------

- Payload mismatch via ``zassert_*``
- Callback drift via shared model (stress only)
- Timing failures logged with context
- Stress regressions via iteration checks

Known issues
------------

- NACK recovery test:
  General call (0x000) treated as valid
  → Test expectation issue

- Interrupt stress failures:
  Likely timing/race condition

- DMA boundary failures:
  Possible alignment/config issue

Platform notes:

- Hardware-aware tests require dual I2C
- BME680 tests are mutually exclusive
- High-frequency operation must match HW capability

Assumptions
-----------

- DT aliases: ``master_i2c`` and ``slave_i2c``
- Target mode available
- Buffer mode enabled only when configured
- Performance tests assume full payload capture

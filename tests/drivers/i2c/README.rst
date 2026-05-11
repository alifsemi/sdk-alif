.. _i2c_ztest:

I2C validation suite
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
- Throughput tests: ``src/test_i2c_performance.c``
- Sensor integration: ``src/test_i2c_sensor.c``

Requirements
------------

Hardware
~~~~~~~~

- Two I2C instances wired for loopback (controller + target on same bus).
- Default overlay: ``boards/i2c.overlay``.

Software
~~~~~~~~

Default ``prj.conf`` enables synchronous loopback tests at Standard
frequency only.  Async transfers and additional frequencies are opt-in.

Granular test suites:

- ``CONFIG_I2C_PERFORMANCE_TESTS``     Throughput/latency tests (non-buffer)
- ``CONFIG_I2C_BOUNDARY_TESTS``        Edge cases and limits
- ``CONFIG_I2C_FAULT_INJECTION_TESTS`` Error handling validation
- ``CONFIG_I2C_HARDWARE_AWARE_TESTS``  Platform-specific tests
- ``CONFIG_I2C_SENSOR_TESTS``         Sensor integration (auto-detects DT sensors)

Note: 10-bit addressing is always enabled.

Test compatibility
------------------

+------------------+-------------+-----------------------+
| Suite            | Buffer mode | Status                |
+==================+=============+=======================+
| All tests        | Yes         | Full                  |
+------------------+-------------+-----------------------+
| Performance      | No          | Non-buffer only       |
+------------------+-------------+-----------------------+

Building and Running
--------------------

Standard loopback (sync only, Standard frequency)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: console

   west build -- \
        -DDTC_OVERLAY_FILE=boards/i2c.overlay

Enable async transfers
~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: console

   west build -- \
        -DDTC_OVERLAY_FILE=boards/i2c.overlay \
        -DCONFIG_I2C_CALLBACK=y

Enable all frequencies (Fast, Fast+, High)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: console

   west build -- \
        -DDTC_OVERLAY_FILE=boards/i2c.overlay \
        -DCONFIG_I2C_TEST_FREQ_FAST=y \
        -DCONFIG_I2C_TEST_FREQ_FAST_PLUS=y \
        -DCONFIG_I2C_TEST_FREQ_HIGH=y

Enable all test suites
~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: console

   west build -- \
        -DDTC_OVERLAY_FILE=boards/i2c.overlay \
        -DCONFIG_I2C_ALL_TESTS=y

Sensor integration (e.g. BME680)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: console

   west build -- \
        -DDTC_OVERLAY_FILE=boards/i2c_sensor.overlay \
        -DCONFIG_I2C_SENSOR_TESTS=y -DCONFIG_SENSOR=y

Sensors are auto-detected from the devicetree.
Kconfig or CMake changes are not required for new sensors.


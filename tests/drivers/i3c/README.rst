.. _i3c_ztest:

I3C Driver Tests
################

Overview
========

This test suite validates I3C controller and target communication for MIPI I3C
Basic v1.0 specification compliance. The tests use Zephyr ZTest framework with
per-test setup/teardown hooks to ensure clean bus state.

Test Framework
==============

The test suite uses ZTest's ``test_before`` and ``test_after`` hooks:

- **suite_setup**: Initializes I3C controller, configures SDR mode, validates bus ready
- **test_before**: Verifies devices have dynamic addresses from prior DAA
- **test_after**: Empty teardown hook (reserved for future use)

.. important::

   **DAA (Dynamic Address Assignment) is performed ONCE externally** before any
   test suites run - typically by system initialization or application setup.
   Tests do NOT perform RSTDAA+DAA because:

   1. Repeated RSTDAA causes address increments when devices are on bus
      but not registered in device tree (hot-join scenario)
   2. Driver may not handle repeated controller resets properly
   3. Zephyr does not support hot-join - all devices must be in DT

   If you see "PID not in registered device list" warnings with incrementing
   addresses (0x0f, 0x10, 0x11...), ensure ALL physical devices are declared
   in the device tree overlay, or perform a single RSTDAA+DAA before tests.

   To perform DAA before tests, ensure your application or test main calls::

      i3c_test_do_rstdaa_daa(ctrl);  /* Once, before ztest_run() */

Test Suites
===========

- **discovery**: Bus discovery, DAA, target enumeration, address assignment
- **ccc**: Common Command Codes (GETPID, GETBCR, GETDCR, SETNEWDA, etc.)
- **ibi**: In-Band Interrupt capability and event handling
- **negative**: Error conditions and negative scenarios
- **stress**: Endurance tests under repeated operations
- **sensor**: Sensor integration tests for BMI323

Key Files
=========

- ``src/i3c_discovery.c``: Discovery and DAA test suite
- ``src/i3c_ccc.c``: CCC command test suite
- ``src/i3c_ibi.c``: IBI test suite
- ``src/i3c_negative.c``: Negative test suite
- ``src/i3c_stress.c``: Stress test suite
- ``src/i3c_sensor.c``: Sensor integration test suite
- ``src/i3c_common.c``: Common test utilities
- ``src/i3c_common.h``: Common definitions
- ``boards/i3c_test.overlay``: Device tree overlay

Building and Running
====================

Build and run all tests:

.. code-block:: console

   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp tests/drivers/i3c \
     -DDTC_OVERLAY_FILE=boards/i3c_test.overlay \
     -DCONFIG_I3C_ALL_TESTS=y
   west flash

Build with specific test suite:

.. code-block:: console

   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp tests/drivers/i3c \
     -DDTC_OVERLAY_FILE=boards/i3c_test.overlay \
     -DCONFIG_I3C_DISCOVERY_TESTS=y

Test Suite Selection
====================

Enable test suites via Kconfig:

- ``CONFIG_I3C_DISCOVERY_TESTS=y``: Discovery tests
- ``CONFIG_I3C_CCC_TESTS=y``: CCC tests
- ``CONFIG_I3C_IBI_TESTS=y``: IBI tests
- ``CONFIG_I3C_NEGATIVE_TESTS=y``: Negative tests
- ``CONFIG_I3C_STRESS_TESTS=y``: Stress tests
- ``CONFIG_I3C_SENSOR_TESTS=y``: Sensor tests
- ``CONFIG_I3C_ALL_TESTS=y``: All test suites

Configuration Options
=====================

Key options in ``prj.conf``:

- ``CONFIG_I3C=y``: Enable I3C driver
- ``CONFIG_SENSOR=y``: Enable sensor subsystem
- ``CONFIG_BMI323=y``: Enable BMI323 sensor driver
- ``CONFIG_ZTEST_STACK_SIZE=4096``: Test thread stack size


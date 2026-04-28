.. _ospi-flash-test:
.. _snippet-ospi-flash:

OSPI Flash Test Suite
#####################

Overview
********

This test application verifies OSPI Flash operations including Read, Write, and Erase.
The OSPI driver is located under ``modules/hal/alif/drivers``.
The test suite uses 16-bit Data Frame Size for R/W operations as supported by current hardware.

Test Structure
**************

The test suite is organized into multiple test files:

*   ``test_ospi_main.c`` - Basic flash operations (setup, erase, write, read, patterns)
*   ``test_ospi_perf_tests.c`` - Performance benchmarking tests
*   ``test_ospi_xip_tests.c`` - Execute-In-Place (XIP) mode tests
*   ``test_ospi_boundary_tests.c`` - Boundary and cross-page tests
*   ``test_ospi_negative_tests.c`` - Negative scenario tests (invalid params, unaligned access)
*   ``test_ospi_flash_test.h`` - Common header with shared macros and declarations

Key Features
************

*   **SPI_FLASH_DT_NODE macro**: Centralized device tree node alias for flash device
*   **Shared test configuration**: Common offsets, sizes, and buffer definitions
*   **ZTEST framework**: Proper test suite lifecycle with setup/teardown
*   **Zephyr coding standards**: SPDX headers, proper logging, consistent formatting

Supported Boards
****************

*   ``alif_b1_dk/ab1c1f4m51820ph0/rtss_he``
*   ``alif_e1c_dk/ae1c1f4051920hh/rtss_he``
*   ``alif_e8_dk/ae822fa0e5597xx0/rtss_he``
*   ``alif_e8_dk/ae822fa0e5597xx0/rtss_hp``
*   ``alif_e7_dk/ae722f80f55d5xx/rtss_he``
*   ``alif_e7_dk/ae722f80f55d5xx/rtss_hp``

Building and Running
********************

The application builds only for targets with devicetree entry compatible with ``snps,designware-ospi``.

Manual Build Instructions
*************************

**Basic build using snippet system (recommended):**

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he \\
     tests/drivers/spi_flash/ -S ospi-flash

**Optional: Override or extend overlays (advanced usage):**

In addition to snippet-based configuration, specific overlays (for example,
frequency settings) can be applied manually using ``EXTRA_DTC_OVERLAY_FILE``.

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     tests/drivers/spi_flash/ -- \
     -DEXTRA_DTC_OVERLAY_FILE="snippets/e7_ospi1.overlay"

Configuration Flags
*******************

*   ``-DCONFIG_TEST_PERF=y``: Enable performance logging/metrics
*   ``-DCONFIG_ALIF_OSPI_FLASH_XIP=y``: Enable XIP mode configuration

XIP Mode Testcases
******************

To run OSPI flash tests in XIP mode, enable:

.. code-block:: none

   CONFIG_ALIF_OSPI_FLASH_XIP=y

Test Configuration
******************

Common test parameters (defined in ``test_ospi_flash_test.h``):

*   ``SPI_FLASH_DT_NODE`` - Device tree alias for flash device (DT_ALIAS(spi_flash0))
*   ``SPI_FLASH_TEST_REGION_OFFSET`` - 0x8000 (32KB, safe test region)
*   ``SPI_FLASH_SECTOR_SIZE`` - 4096 bytes
*   ``BUFF_SIZE`` - 1024 bytes

Running Tests
*************

The test application uses the Zephyr ``ztest`` framework, which organizes tests
into multiple test suites.

**Run all tests:**

.. code-block:: console

   west build -t run

This runs the complete test application and executes all test suites.

**Run specific test suite:**

.. code-block:: console

   west build -t run -- -s test_ospi_flash
   west build -t run -- -s test_ospi_perf
   west build -t run -- -s test_ospi_xip
   west build -t run -- -s test_ospi_boundary
   west build -t run -- -s test_ospi_negative

The ``-s`` option runs only the specified test suite, which is useful for
faster debugging and targeted validation.

*   ``test_ospi_flash``: Basic functional tests (read, write, erase, patterns)
*   ``test_ospi_perf``: Performance benchmarking tests
*   ``test_ospi_xip``: Execute-In-Place (XIP) mode tests
*   ``test_ospi_boundary``: Boundary and cross-page tests
*   ``test_ospi_negative``: Negative tests (invalid parameters, unaligned access)

**Note:**

The suite name passed to ``-s`` must match the name defined in the code
using ``ZTEST_SUITE(...)``.


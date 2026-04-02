.. _crc_ztest:

Alif CRC Driver Test Suite
**************************

This comprehensive test suite validates the Alif CRC hardware driver implementation
against a software reference model, ensuring correctness and robustness across
all supported CRC algorithms and configurations.

Overview
========

The Alif CRC driver provides hardware acceleration for common CRC algorithms.
This test suite validates functional correctness, edge cases, and stress scenarios
to guarantee reliable operation in production environments.

Supported Algorithms
====================

- **CRC-8**: Polynomial ``0x07`` (standard 8-bit CRC)
- **CRC-16-CCITT**: Polynomial ``0x1021`` (X.25, Bluetooth, etc.)
- **CRC-16**: Polynomial ``0x8005`` (IBM/modem protocol)
- **CRC-32**: Polynomial ``0x04C11DB7`` (ISO 3309, PKZIP, etc.)
- **CRC-32C**: Polynomial ``0x1EDC6F41`` (iSCSI, Btrfs, etc.)

Features Tested
===============

Functional Tests
----------------

- Standard CRC computation with known test vectors
- Output reflection (bit reversal)
- Output inversion (bitwise NOT)
- Byte swapping (endianness conversion)
- Bit swapping (MSB/LSB reversal)
- Single-byte payload handling
- Unaligned memory access
- Custom seed values

Stress Tests
------------

- High-volume data processing
- Rapid configuration changes
- Multi-algorithm concurrent testing
- Resource exhaustion scenarios
- Boundary condition testing
- Hardware register stress

Test Architecture
=================

Source Organization
-------------------

- ``src/main.c``: Shared utilities, device validation, and test framework
- ``src/software_crc.c`` & ``src/software_crc.h``: Reference software implementation
- ``src/test_crc_8.c``: CRC-8 algorithm test suite
- ``src/test_crc_16.c``: CRC-16-CCITT algorithm test suite
- ``src/test_crc_16_alt.c``: CRC-16 algorithm test suite
- ``src/test_crc_32.c``: CRC-32 algorithm test suite
- ``src/test_crc_32c.c``: CRC-32C algorithm test suite
- ``src/test_crc_stress.c``: Stress and robustness test suite

Device Mapping
--------------

- ``boards/crc0.overlay``: Maps CRC0 hardware instance to ``crc`` alias
- ``boards/crc1.overlay``: Maps CRC1 hardware instance to ``crc`` alias

Configuration
=============

Enable CRC algorithms using Kconfig options:

Core Algorithm Tests
--------------------

- ``CONFIG_TEST_CRC_8``: Enable CRC-8 test suite
- ``CONFIG_TEST_CRC_16``: Enable CRC-16-CCITT test suite
- ``CONFIG_TEST_CRC_16_ALT``: Enable CRC-16 test suite
- ``CONFIG_TEST_CRC_32``: Enable CRC-32 test suite
- ``CONFIG_TEST_CRC_32C``: Enable CRC-32C test suite

Stress Testing
--------------

- ``CONFIG_TEST_CRC_STRESS``: Enable stress testing for all active algorithms

Multi-Algorithm Support
-----------------------

The test framework supports building multiple CRC algorithms in a single image.
Each algorithm runs in an isolated ztest suite with proper configuration switching.

Building
========

Automated Testing (Twister)
---------------------------

Execute the complete test matrix:

::

   west twister -p alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     -T tests/drivers/alif_crc

For E8 and A5 platforms:

::

   west twister -p alif_e8_dk/ae822f80f55d5xx/rtss_hp \
     -T tests/drivers/alif_crc

   west twister -p alif_a5_dk/aa522f80f55d5xx/rtss_he \
     -T tests/drivers/alif_crc

Manual Build Examples
---------------------

Single Algorithm - CRC0 with CRC-32:

::

   west build -p -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     tests/drivers/alif_crc -- \
     -DDTC_OVERLAY_FILE=boards/crc0.overlay \
     -DCONFIG_TEST_CRC_32=y

E8 Platform - CRC0 with CRC-32:

::

   west build -p -b alif_e8_dk/ae822f80f55d5xx/rtss_hp \
     tests/drivers/alif_crc -- \
     -DDTC_OVERLAY_FILE=boards/crc0.overlay \
     -DCONFIG_TEST_CRC_32=y

A5 Platform - CRC0 with CRC-32:

::

   west build -p -b alif_a5_dk/aa522f80f55d5xx/rtss_he \
     tests/drivers/alif_crc -- \
     -DDTC_OVERLAY_FILE=boards/crc0.overlay \
     -DCONFIG_TEST_CRC_32=y

Single Algorithm - CRC1 with CRC-32C:

::

   west build -p -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     tests/drivers/alif_crc -- \
     -DDTC_OVERLAY_FILE=boards/crc1.overlay \
     -DCONFIG_TEST_CRC_32C=y

Comprehensive Coverage - All Algorithms with Stress:

E7 Platform:

::

   west build -p -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     tests/drivers/alif_crc -- \
     -DDTC_OVERLAY_FILE=boards/crc0.overlay \
     -DCONFIG_TEST_CRC_8=y \
     -DCONFIG_TEST_CRC_16=y \
     -DCONFIG_TEST_CRC_16_ALT=y \
     -DCONFIG_TEST_CRC_32=y \
     -DCONFIG_TEST_CRC_32C=y \
     -DCONFIG_TEST_CRC_STRESS=y

E8 Platform:

::

   west build -p -b alif_e8_dk/ae822f80f55d5xx/rtss_hp \
     tests/drivers/alif_crc -- \
     -DDTC_OVERLAY_FILE=boards/crc0.overlay \
     -DCONFIG_TEST_CRC_8=y \
     -DCONFIG_TEST_CRC_16=y \
     -DCONFIG_TEST_CRC_16_ALT=y \
     -DCONFIG_TEST_CRC_32=y \
     -DCONFIG_TEST_CRC_32C=y \
     -DCONFIG_TEST_CRC_STRESS=y

A5 Platform:

::

   west build -p -b alif_a5_dk/aa522f80f55d5xx/rtss_hp \
     tests/drivers/alif_crc -- \
     -DDTC_OVERLAY_FILE=boards/crc0.overlay \
     -DCONFIG_TEST_CRC_8=y \
     -DCONFIG_TEST_CRC_16=y \
     -DCONFIG_TEST_CRC_16_ALT=y \
     -DCONFIG_TEST_CRC_32=y \
     -DCONFIG_TEST_CRC_32C=y \
     -DCONFIG_TEST_CRC_STRESS=y

Test Matrix
===========

The ``testcase.yaml`` defines comprehensive test coverage:

Instance Coverage
-----------------

- **CRC0**: Tests on CRC0 hardware instance (alif_e7_dk/rtss_hp, alif_e8_dk/rtss_hp, alif_a5_dk/rtss_hp)
- **CRC1**: Tests on CRC1 hardware instance (alif_e7_dk/rtss_he, alif_e8_dk/rtss_he, alif_a5_dk/rtss_he)

Algorithm Coverage
------------------

- Individual algorithm testing for each CRC variant
- Stress testing for each algorithm
- Combined multi-algorithm testing

Test Categories
----------------

1. **Functional Tests**: Validate correctness against known vectors
2. **Stress Tests**: High-load and boundary condition testing
3. **Integration Tests**: Multi-algorithm coexistence

Platform Support
===============

Supported Hardware
------------------

- **Alif E7 DK**: Development Kit with AE722F80F55D5XX SoC
- **Alif E8 DK**: Development Kit with AE822F80F55D5XX SoC
- **Alif A5 DK**: Development Kit with AA522F80F55D5XX SoC
- **RTSS_HP**: High-performance real-time subsystem
- **RTSS_HE**: High-efficiency real-time subsystem

Hardware Instances
------------------

- **CRC0**: Primary CRC hardware accelerator
- **CRC1**: Secondary CRC hardware accelerator

Troubleshooting
===============

Common Issues
-------------

Build Failures
~~~~~~~~~~~~~~

- Ensure correct platform selection in build commands
- Verify overlay file paths are correct
- Check Kconfig conflicts between algorithms

Runtime Failures
~~~~~~~~~~~~~~~~

- Hardware initialization failures: Check device tree bindings
- CRC mismatches: Verify polynomial and configuration parameters
- Stress test timeouts: May indicate hardware resource contention

Debug Tips
----------

1. Enable general system logging: ``-DCONFIG_LOG=y``
2. Set global log level to debug: ``-DCONFIG_LOG_DEFAULT_LEVEL=4``
3. Use single-algorithm builds for isolated testing
4. Verify hardware clock configuration
5. Check memory alignment for large data buffers
6. Enable driver-specific logging if available: ``-DCONFIG_DRV_LOG=y``

Contributing
============

Adding New CRC Algorithms
--------------------------

1. Implement software reference in ``src/software_crc.c``
2. Create test suite file ``src/test_crc_<algorithm>.c``
3. Add Kconfig option in ``Kconfig``
4. Update ``testcase.yaml`` with new test scenarios
5. Update this README with algorithm details

Stress Test Enhancement
-----------------------

Stress tests are located in ``src/test_crc_stress.c``. Follow existing patterns
when adding new stress scenarios:

- Use deterministic seed values for reproducibility
- Validate against software reference implementation
- Include proper error handling and cleanup
- Add comprehensive assertions

Code Style
----------

This codebase follows Zephyr coding standards. Run checkpatch before submitting:

::

   find . -name "*.c" -exec scripts/checkpatch.pl --no-tree --file {} \;

License
=======

Copyright Alif Semiconductor - All Rights Reserved.
Use, distribution and modification of this code is permitted under the
terms stated in the Alif Semiconductor Software License Agreement.

You should have received a copy of the Alif Semiconductor Software
License Agreement with this file. If not, please write to:
contact@alifsemi.com, or visit: https://alifsemi.com/license


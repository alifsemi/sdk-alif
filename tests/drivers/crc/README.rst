.. SPDX-License-Identifier: Apache-2.0

Alif CRC Driver Test Suite
**************************

Overview
========

Hardware CRC driver tests for Alif Semiconductor platforms.
Each test computes CRC using the hardware peripheral and compares against
a software reference implementation.

Supported Algorithms
--------------------

- **CRC 8-bit CCITT** — polynomial 0x07
- **CRC 16-bit CCITT** — polynomial 0x1021
- **CRC 32-bit standard** — polynomial 0x04C11DB7
- **CRC 32-bit custom polynomial** — polynomial 0x2CEEA6C8
- **CRC 32-bit Castagnoli (CRC-32C)** — polynomial 0x1EDC6F41

Each algorithm tests the following modes where applicable:
basic, reflect output, bit/byte swap, invert output, single-byte input,
non-zero seed, and reflect+invert combined.

Stress & Robustness Tests
-------------------------

Algorithm-agnostic tests (``CONFIG_TEST_CRC_STRESS``) that exercise:

- **Repeatability** — 50 identical computations
- **Back-to-back** — compute without re-seeding
- **All-zeros** — various lengths (1–64 bytes)
- **Rapid 100 iterations** — register state leakage detection
- **Alternating params** — reflect/invert toggling, stale-bit detection
- **Output width** — asserts upper bits are clear for 8/16-bit algos
- **Known vector "123456789"** — canonical CRC check string
- **Large data (256 bytes)** — determinism on large buffers
- **Bit-flip sensitivity** — single bit flip changes CRC
- **Reflect+Invert orthogonality** — all 4 combos distinct
- **Swap sensitivity** — bit_swap / byte_swap produce distinct CRCs

For ``CONFIG_TEST_CRC_32_CUSTOM_POLY`` and ``CONFIG_TEST_CRC_32C`` combined
with ``CONFIG_TEST_CRC_STRESS``, dedicated CRC-32-family stress tests are
enabled for known-vector, repeatability, and unaligned-tail behavior with
custom polynomial handling.

Project Structure
=================

::

  tests/drivers/crc/
  ├── CMakeLists.txt
  ├── Kconfig
  ├── prj.conf
  ├── testcase.yaml              ← twister test definitions
  ├── boards/
  │   ├── crc0_8_bit.overlay
  │   ├── crc0_16_bit.overlay
  │   ├── crc0_32_bit.overlay
  │   ├── crc1_8_bit.overlay
  │   ├── crc1_16_bit.overlay
  │   └── crc1_32_bit.overlay
  └── src/
      ├── main.c                 ← suite registration + common helpers
      ├── test_crc_common.h      ← shared declarations
      ├── test_crc_8.c           ← CRC-8 test cases (7 tests)
      ├── test_crc_16.c          ← CRC-16 test cases (7 tests)
      ├── test_crc_32.c          ← CRC-32 test cases (7 tests)
      ├── test_crc_32_custom.c   ← CRC-32 custom poly test cases (2 tests)
      ├── test_crc_32c.c         ← CRC-32C Castagnoli test cases (6 tests)
      ├── test_crc_stress.c      ← stress tests (11 generic + 3 CRC32-family extended)
      ├── software_crc.c         ← software reference CRC
      └── software_crc.h

Prerequisites
=============

- Zephyr SDK and ``west`` tool installed.
- ``ZEPHYR_BASE`` environment variable set.
- Board with CRC peripheral support (e.g. ``alif_e7_dk/ae722f80f55d5xx/rtss_hp``).

Building (using twister — recommended)
=======================================

Twister automatically picks up the per-scenario ``extra_configs`` and
``extra_dtc_overlay_files`` defined in ``testcase.yaml``.

Run **all** CRC test scenarios on the target board::

  west twister -p alif_e7_dk/ae722f80f55d5xx/rtss_hp -T .

Run from the Zephyr test tree (if this sample is placed under
``tests/drivers/alif_crc/``)::

  west twister -p alif_e7_dk/ae722f80f55d5xx/rtss_hp -T tests/drivers/alif_crc/

Run a **single** test scenario::

  west twister -p alif_e7_dk/ae722f80f55d5xx/rtss_hp -s drivers.crc.crc0_8_bit
  west twister -p alif_e7_dk/ae722f80f55d5xx/rtss_hp -s drivers.crc.crc0_16_bit
  west twister -p alif_e7_dk/ae722f80f55d5xx/rtss_hp -s drivers.crc.crc0_32_bit
  west twister -p alif_e7_dk/ae722f80f55d5xx/rtss_hp -s drivers.crc.crc0_32_bit_custom_poly
  west twister -p alif_e7_dk/ae722f80f55d5xx/rtss_hp -s drivers.crc.crc0_32c

Building (using west build — manual)
=====================================

Each algorithm requires its own DTC overlay (to select the CRC bit-size in
devicetree) **and** its own Kconfig fragment (to compile the matching test
source file).  Pass both via ``-DDTC_OVERLAY_FILE`` and
``-DOVERLAY_CONFIG``.

.. note::

   Always use ``-p`` (pristine build) or delete the ``build/`` directory when
   switching between CRC algorithms, because the devicetree and Kconfig
   change between variants.

CRC 8-bit::

  west build -p -b alif_e7_dk/ae722f80f55d5xx/rtss_hp . -- \
    -DCONFIG_TEST_CRC_8=y \
    -DDTC_OVERLAY_FILE="boards/crc0_8_bit.overlay"

CRC 16-bit::

  west build -p -b alif_e7_dk/ae722f80f55d5xx/rtss_hp . -- \
    -DCONFIG_TEST_CRC_16=y \
    -DDTC_OVERLAY_FILE="boards/crc0_16_bit.overlay"

CRC 32-bit::

  west build -p -b alif_e7_dk/ae722f80f55d5xx/rtss_hp . -- \
    -DCONFIG_TEST_CRC_32=y \
    -DDTC_OVERLAY_FILE="boards/crc0_32_bit.overlay"

CRC 32-bit custom polynomial::

  west build -p -b alif_e7_dk/ae722f80f55d5xx/rtss_hp . -- \
    -DCONFIG_TEST_CRC_32_CUSTOM_POLY=y \
    -DDTC_OVERLAY_FILE="boards/crc0_32_bit.overlay"

CRC 8-bit::

  west build -p -b alif_e7_dk/ae722f80f55d5xx/rtss_he . -- \
    -DCONFIG_TEST_CRC_8=y \
    -DDTC_OVERLAY_FILE="boards/crc1_8_bit.overlay"

CRC 16-bit::

  west build -p -b alif_e7_dk/ae722f80f55d5xx/rtss_he . -- \
    -DCONFIG_TEST_CRC_16=y \
    -DDTC_OVERLAY_FILE="boards/crc1_16_bit.overlay"

CRC 32-bit::

  west build -p -b alif_e7_dk/ae722f80f55d5xx/rtss_he . -- \
    -DCONFIG_TEST_CRC_32=y \
    -DDTC_OVERLAY_FILE="boards/crc1_32_bit.overlay"

 CRC 32-bit custom polynomial::

  west build -p -b alif_e7_dk/ae722f80f55d5xx/rtss_he . -- \
    -DCONFIG_TEST_CRC_32_CUSTOM_POLY=y \
    -DDTC_OVERLAY_FILE="boards/crc1_32_bit.overlay"

Stress tests (CRC-8 with stress on crc0)::

  west build -p -b alif_e7_dk/ae722f80f55d5xx/rtss_hp . -- \
    -DCONFIG_TEST_CRC_8=y \
    -DCONFIG_TEST_CRC_STRESS=y \
    -DDTC_OVERLAY_FILE="boards/crc0_8_bit.overlay"

Stress tests (CRC-32 with stress on crc0)::

  west build -p -b alif_e7_dk/ae722f80f55d5xx/rtss_hp . -- \
    -DCONFIG_TEST_CRC_32=y \
    -DCONFIG_TEST_CRC_STRESS=y \
    -DDTC_OVERLAY_FILE="boards/crc0_32_bit.overlay"

Stress tests (CRC-32 custom polynomial with stress on crc0)::

  west build -p -b alif_e7_dk/ae722f80f55d5xx/rtss_hp . -- \
    -DCONFIG_TEST_CRC_32_CUSTOM_POLY=y \
    -DCONFIG_TEST_CRC_STRESS=y \
    -DDTC_OVERLAY_FILE="boards/crc0_32_bit.overlay"

Stress tests (CRC-32C with stress on crc0)::

  west build -p -b alif_e7_dk/ae722f80f55d5xx/rtss_hp . -- \
    -DCONFIG_TEST_CRC_32C=y \
    -DCONFIG_TEST_CRC_STRESS=y \
    -DDTC_OVERLAY_FILE="boards/crc0_32_bit.overlay"

Run stress tests via twister::

  west twister -p alif_e7_dk/ae722f80f55d5xx/rtss_hp -s drivers.crc.crc0_8_bit_stress
  west twister -p alif_e7_dk/ae722f80f55d5xx/rtss_hp -s drivers.crc.crc0_16_bit_stress
  west twister -p alif_e7_dk/ae722f80f55d5xx/rtss_hp -s drivers.crc.crc0_32_bit_stress
  west twister -p alif_e7_dk/ae722f80f55d5xx/rtss_hp -s drivers.crc.crc0_32_bit_custom_poly_stress
  west twister -p alif_e7_dk/ae722f80f55d5xx/rtss_hp -s drivers.crc.crc0_32c_stress

Flashing & running (after west build)::

  west flash

Alif UTIMER Counter Tests
=========================

Overview
********

This test suite validates the Alif UTIMER counter driver using the Zephyr
Ztest framework. It exercises the Zephyr Counter API against the
``alif_utimer_counter`` and ``alif_lputimer_counter`` hardware instances.

Test suite: ``alif_utimer`` - 11 tests

The suite currently contains 11 implemented ``ZTEST()`` cases in
``src/test_utimer.c``. These tests exercise the Alif UTIMER counter driver
across the main Zephyr Counter API behaviors, including basic counter and
alarm handling, top value configuration, invalid-parameter/error paths, and
multi-channel operation where supported.

To avoid the documentation drifting from the code, treat
``src/test_utimer.c`` as the authoritative source for the exact test case
names and coverage.

Supported Boards
****************

- ``alif_e8_dk/ae822fa0e5597xx0/rtss_he`` (via ``alif_utimer.overlay``)
- ``alif_e8_dk/ae822fa0e5597xx0/rtss_hp`` (via ``alif_utimer.overlay``)
- ``alif_e7_dk/ae722f80f55d5xx/rtss_he``  (via ``alif_utimer.overlay``)
- ``alif_e7_dk/ae722f80f55d5xx/rtss_hp``  (via ``alif_utimer.overlay``)
- ``alif_b1_dk/ab1c1f4m51820ph0/rtss_he`` (via ``alif_lputimer.overlay``)

Board-specific overlays are located under ``boards/``.

Configuration
*************

Key Kconfig options set in ``prj.conf``:

- ``CONFIG_COUNTER=y`` - Zephyr counter subsystem
- ``CONFIG_COUNTER_ALIF_UTIMER=y`` - Alif uTimer counter driver
- ``CONFIG_ZTEST=y`` - Zephyr test framework

Building and Running
********************

Build for alif_e7_dk/rtss_hp:

.. code-block:: console

   west build -b alif_e7_dk/ae722f80f55d5xx/rtss_hp tests/drivers/utimer/ \
       -DDTC_OVERLAY_FILE=boards/alif_utimer.overlay

Build for alif_e7_dk/rtss_he:

.. code-block:: console

   west build -b alif_e7_dk/ae722f80f55d5xx/rtss_he tests/drivers/utimer/ \
       -DDTC_OVERLAY_FILE=boards/alif_utimer.overlay

Build for alif_e8_dk/rtss_hp:

.. code-block:: console

   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp tests/drivers/utimer/ \
       -DDTC_OVERLAY_FILE=boards/alif_utimer.overlay

Build for alif_e8_dk/rtss_he:

.. code-block:: console

   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_he tests/drivers/utimer/ \
       -DDTC_OVERLAY_FILE=boards/alif_utimer.overlay

Build for alif_b1_dk/rtss_he:

.. code-block:: console

   west build -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he tests/drivers/utimer/ \
       -DDTC_OVERLAY_FILE=boards/alif_lputimer.overlay

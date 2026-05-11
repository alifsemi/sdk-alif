Alif UTIMER QDEC (Quadrature Decoder) Tests
============================================

Overview
********

This test suite validates the Alif UTIMER quadrature decoder (QDEC) sensor
driver using the Zephyr Ztest framework. A GPIO-based encoder emulator
generates quadrature Phase A/B signals; the tests verify that the QDEC
sensor driver correctly reports position via the Zephyr sensor API
(``sensor_sample_fetch`` / ``sensor_channel_get(SENSOR_CHAN_ROTATION)``).

Test suite: ``qec_tests`` — 31 tests covering 8 hardware features:

**Feature 1: 32-bit counters** (TC-QEC-001 to TC-QEC-004)

- Device initialization and readiness
- 32-bit counter range — position stays 0-359 after large rotation
- Counter reload wrap across multiple revolutions
- Output range always 0-359 degrees with ``val2 == 0``

**Feature 2: Quadrature decoding** (TC-QEC-005 to TC-QEC-008)

- CW quadrature signal pattern decoded correctly
- CCW quadrature signal pattern decoded correctly
- Position changes after CW and CCW rotations
- Position stable with no encoder input

**Feature 3: Increment/decrement counting** (TC-QEC-009 to TC-QEC-012)

- Up-counter increments on CW rotation
- Down-counter decrements on CCW rotation
- Multiple CW batches accumulate correctly
- Equal CW then CCW round-trip returns to start (tolerance: 2 degrees)

**Feature 4: Pulse/period measurement** (TC-QEC-013 to TC-QEC-015)

- Position proportional to number of encoder steps
- Minimum step resolution (1 step and 10 steps)
- ``counts-per-revolution`` DT property verified via quarter-revolution test

**Feature 5: Digital input filter** (TC-QEC-016 to TC-QEC-018)

- Device initializes with filter enabled
- Filter rejects rapid glitch pulses (< 10 us per half-cycle)
- Filter passes valid quadrature signals at normal speed

**Feature 6: Interrupts/events** (TC-QEC-019 to TC-QEC-020)

- Counter overflow wraps correctly across multiple revolutions
- No spurious position change when encoder is idle

**Feature 7: Internal clock** (TC-QEC-021 to TC-QEC-022)

- uTimer clock enabled — counter responds to encoder input
- Pin control applied — encoder signals reach the QDEC counter

**Feature 8: External signals** (TC-QEC-023 to TC-QEC-025)

- GPIO encoder emulator init and re-init
- ``sensor_sample_fetch_chan(SENSOR_CHAN_ALL)``
- ``sensor_sample_fetch_chan(SENSOR_CHAN_ROTATION)``

**Negative tests** (TC-QEC-026 to TC-QEC-028)

- ``SENSOR_CHAN_ACCEL_X`` returns ``-ENOTSUP``
- ``SENSOR_CHAN_GYRO_X`` returns ``-ENOTSUP``
- Unsupported fetch channel returns ``-ENOTSUP``

**Stress tests** (TC-QEC-029 to TC-QEC-032)

- Rapid CW/CCW direction changes (10 iterations)
- Continuous CW over 5 full revolutions
- Continuous CCW over 5 full revolutions
- 50 consecutive fetch+get calls without motion

Supported Boards
****************

- ``alif_e8_dk/ae822fa0e5597xx0/rtss_he`` (via ``alif_ensemble_qdec.overlay``)
- ``alif_e8_dk/ae822fa0e5597xx0/rtss_hp`` (via ``alif_ensemble_qdec.overlay``)
- ``alif_e7_dk/ae722f80f55d5xx/rtss_he``  (via ``alif_ensemble_qdec.overlay``)
- ``alif_e7_dk/ae722f80f55d5xx/rtss_hp``  (via ``alif_ensemble_qdec.overlay``)
- ``alif_b1_dk/ab1c1f4m51820ph0/rtss_he`` (via ``alif_b1_dk_rtss_he.overlay``)
- ``alif_e1c_dk/ae1c1f4051920hh/rtss_he`` (via ``alif_e1c_dk_rtss_he.overlay``)

Board-specific overlays are located under ``boards/``.

Prerequisites
*************

Hardware loopback
  The GPIO encoder emulator output pins (``qenca``, ``qencb``) must be
  physically wired to the QDEC input pins configured via ``pinctrl`` in the
  overlay. Without this wiring, motion-dependent tests skip automatically
  via ``SKIP_IF_NO_LOOPBACK()``.

Device tree
  Each overlay enables ``utimer1`` with a QDEC child node and defines two
  GPIO pins (``gpio_a``, ``gpio_b``) under an ``encoder-emulate`` node used
  by the emulator.

Configuration
*************

Key Kconfig options set in ``prj.conf``:

- ``CONFIG_ZTEST=y`` — Zephyr test framework
- ``CONFIG_GPIO=y`` — GPIO driver (encoder emulator)
- ``CONFIG_SENSOR=y`` — sensor subsystem (QDEC uses sensor API)
- ``CONFIG_PRINTK=y`` — diagnostic output

Building and Running
********************

alif_e8_dk (rtss_he):
  .. code-block:: console

     west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
       tests/drivers/utimer_qdec/ \
       -DDTC_OVERLAY_FILE=$PWD/tests/drivers/utimer_qdec/boards/ \
       alif_ensemble_qdec.overlay

alif_e8_dk (rtss_hp):
  .. code-block:: console

     west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
       tests/drivers/utimer_qdec/ \
       -DDTC_OVERLAY_FILE=$PWD/tests/drivers/utimer_qdec/boards/ \
       alif_ensemble_qdec.overlay

alif_e7_dk (rtss_he):
  .. code-block:: console

     west build -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
       tests/drivers/utimer_qdec/ \
       -DDTC_OVERLAY_FILE=$PWD/tests/drivers/utimer_qdec/boards/ \
       alif_ensemble_qdec.overlay

alif_b1_dk (rtss_he):
  .. code-block:: console

     west build -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
       tests/drivers/utimer_qdec/ \
       -DDTC_OVERLAY_FILE=$PWD/tests/drivers/utimer_qdec/boards/ \
       alif_b1_dk_rtss_he.overlay

alif_e1c_dk (rtss_he):
  .. code-block:: console

     west build -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
       tests/drivers/utimer_qdec/ \
       -DDTC_OVERLAY_FILE=$PWD/tests/drivers/utimer_qdec/boards/ \
       alif_e1c_dk_rtss_he.overlay

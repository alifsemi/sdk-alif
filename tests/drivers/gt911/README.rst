Alif GT911 Touchscreen Driver Tests
===================================

Overview
********
This test suite validates the Goodix GT911 capacitive touch controller on
Alif boards using the Zephyr input subsystem and Ztest.

Board wiring is applied with the ``alif-gt911`` snippet (same I2C1 / reset /
IRQ mapping as ``samples/subsys/input/input_dump``).

The tests are split into two ZTEST suites:

**gt911 suite — no operator (CI-safe):**

- device ready
- I2C product ID (``0x8140`` == ``"911"``)
- I2C address ``0x5D``
- config blob checksum at ``0x8047``
- programmed max touch points
- no input events while idle
- status register buffer-ready bit cleared while idle
- IRQ GPIO is an input after init

**gt911_interactive suite — needs a finger:**

Skipped unless ``CONFIG_TEST_GT911_INTERACTIVE=y``.

- press and release, coordinates in range, no ghost events after lift
- tap anywhere (range check)
- four corners (axis span)
- hold for report rate (~100 Hz)
- drag with stable track ID
- ten rapid taps (dropped-event check)
- two-finger press and partial lift
- complete X/Y/BTN frames (sync)

Supported Boards
****************

- ``alif_b1_dk/ab1c1f4m51820ph0/rtss_he``
- ``alif_e1c_dk/ae1c1f4051920hh/rtss_he``
- ``alif_e7_dk/ae722f80f55d5xx/rtss_he``
- ``alif_e7_dk/ae722f80f55d5xx/rtss_hp``
- ``alif_e8_dk/ae822fa0e5597xx0/rtss_he``
- ``alif_e8_dk/ae822fa0e5597xx0/rtss_hp``

Prerequisites
*************

- GT911 panel connected (second thinner cable to J23 on the E7 DevKit).
- Interactive tests require an operator at the console.

Building and Running
********************

Default build waits for you to touch the panel (interactive tests on):

.. code-block:: console

   rm -rf build; west build -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
       ../alif/tests/drivers/gt911 -S alif-gt911

Automated-only (no operator; interactive cases skip):

.. code-block:: console

   rm -rf build; west build -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
       ../alif/tests/drivers/gt911 -S alif-gt911 \
       -- -DCONFIG_TEST_GT911_INTERACTIVE=n

Polling mode (no GT911 interrupt):

.. code-block:: console

   rm -rf build; west build -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
       ../alif/tests/drivers/gt911 -S alif-gt911 \
       -- -DCONFIG_INPUT_GT911_INTERRUPT=n

Flash with:

.. code-block:: console

   west flash

Configuration
*************

Key Kconfig options in ``prj.conf``:

- ``CONFIG_INPUT=y`` — Zephyr input subsystem
- ``CONFIG_INPUT_GT911_INTERRUPT=y`` — INT-driven reports (default)
- ``CONFIG_INPUT_GT911_MAX_TOUCH_POINTS=5`` — GT911 5-point max
- ``CONFIG_INPUT_MODE_SYNCHRONOUS=y`` — avoid input-queue drops
- ``CONFIG_I2C_DW_CLOCK_SPEED=100`` — 100 kHz I2C
- ``CONFIG_TEST_GT911_INTERACTIVE=y`` — wait for tap/drag prompts (set ``n`` for CI)

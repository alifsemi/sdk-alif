Alif USB CDC-ACM Driver Tests
=============================

Overview
********
This test suite validates the USB Communication Device Class — Abstract Control
Model (CDC-ACM) interface using the Ztest framework. It targets Alif boards
with USB device controller support enabled via board-specific overlays.

The tests are split into two ZTEST suites:

**cdc_acm_qa suite (19 tests) — requires USB hardware:**

All shared state lives in a ``cdc_acm_qa_fixture`` struct allocated by
``cdc_suite_setup()``. Device lookup, USB stack initialization and
``usbd_enable()`` are validated once in setup; if any step fails the
entire suite aborts. Individual tests access state via ``fixture->``.

- USB disable and re-enable
- USB speed capability query (Full-Speed / High-Speed)
- line control: DTR, RTS, baudrate read
- line control: DCD and DSR set (host-dependent)
- UART configuration get and set (115200/8N1)
- IRQ callback registration and verification
- IRQ RX and TX enable/disable
- poll-based single byte output
- bulk write via poll_out loop
- stress: repeated poll output (50 iterations)
- stress: repeated DTR reads (50 iterations)
- stress: USB disable/enable cycling (5 iterations)
- VBUS detection capability query
- stress: IRQ enable/disable cycling (50 iterations)

**cdc_acm_basic suite (3 tests) — software-only, no USB needed:**

- ring buffer echo logic
- ring buffer full capacity
- ring buffer wraparound

Supported Boards
****************

- ``alif_b1_dk/ab1c1f4m51820ph0/rtss_he``
- ``alif_e7_dk/ae722f80f55d5xx/rtss_hp``
- ``alif_e7_dk/ae722f80f55d5xx/rtss_he``
- ``alif_e8_dk/ae822fa0e5597xx0/rtss_hp``
- ``alif_e8_dk/ae822fa0e5597xx0/rtss_he``

Board-specific overlays are located under:

- ``tests/drivers/usb/cdc-acm/boards/``

Prerequisites
*************

- USB cable connected between the board's USB device port and a host PC.
- Some tests (DCD/DSR set, poll out, FIFO fill) require an active USB host
  connection with a terminal application opening the ``/dev/ttyACMx`` device.
- Tests that depend on host connectivity will skip gracefully if DTR=0
  (no host terminal open).

Configuration
*************

Key Kconfig options in ``prj.conf``:

- ``CONFIG_USB_DEVICE_STACK_NEXT=y`` — enable next-gen USB device stack
- ``CONFIG_SERIAL=y`` — enable serial driver subsystem
- ``CONFIG_UART_LINE_CTRL=y`` — enable UART line control API
- ``CONFIG_UART_INTERRUPT_DRIVEN=y`` — enable UART IRQ-driven API
- ``CONFIG_USBD_CDC_ACM_CLASS=y`` — enable CDC-ACM USB class
- ``CONFIG_ZTEST=y`` — enable Zephyr test framework

Building and Running
********************

 .. code-block:: console

    west build -b alif_e7_dk/ae722f80f55d5xx/rtss_hp tests/drivers/usb/cdc-acm/ -DDTC_OVERLAY_FILE=$PWD/tests/drivers/usb/cdc-acm/boards/alif_usb.overlay
    west build -b alif_e7_dk/ae722f80f55d5xx/rtss_he tests/drivers/usb/cdc-acm/ -DDTC_OVERLAY_FILE=$PWD/tests/drivers/usb/cdc-acm/boards/alif_usb.overlay
    west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp tests/drivers/usb/cdc-acm/ -DDTC_OVERLAY_FILE=$PWD/tests/drivers/usb/cdc-acm/boards/alif_usb.overlay
    west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_he tests/drivers/usb/cdc-acm/ -DDTC_OVERLAY_FILE=$PWD/tests/drivers/usb/cdc-acm/boards/alif_usb.overlay
    west build -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he tests/drivers/usb/cdc-acm/ -DDTC_OVERLAY_FILE=$PWD/tests/drivers/usb/cdc-acm/boards/alif_usb.overlay

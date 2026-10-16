Alif USB HID QA (single tree)
=============================

One directory: ``alif/tests/drivers/usb/hid``.
Compile **one device at a time**. Mouse, keyboard, and joystick each
need their own report and HID interface, so they are not combined in
one image.

Test-only. No driver changes. Same image on E8, E7, and B1.

App path
********

::

   ../alif/tests/drivers/usb/hid

Kconfig
*******

Run from ``~/zas-v2.4.0-rc3/zephyr``. Default suite is mouse.

Mouse (4-byte report: button, X, Y, wheel)::

   west build -p always -d build-hid-mouse \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/tests/drivers/usb/hid \
     -- -DCONFIG_HID_TEST_MOUSE=y

   west build -p always -d build-hid-mouse \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/tests/drivers/usb/hid \
     -- -DCONFIG_HID_TEST_MOUSE=y

   west build -p always -d build-hid-mouse \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     ../alif/tests/drivers/usb/hid \
     -- -DCONFIG_HID_TEST_MOUSE=y

   west flash -d build-hid-mouse

Keyboard (8-byte boot report)::

   west build -p always -d build-hid-kb \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/tests/drivers/usb/hid \
     -- -DCONFIG_HID_TEST_KEYBOARD=y

   west flash -d build-hid-kb

Joystick (3-byte report: button, X, Y)::

   west build -p always -d build-hid-joy \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/tests/drivers/usb/hid \
     -- -DCONFIG_HID_TEST_JOYSTICK=y

   west flash -d build-hid-joy

Use a separate build directory when switching device. Reboot is not
required between flashes; flash replaces the image.

UART
****

- Mouse: ``Running TESTSUITE hid_mouse_qa``
- Keyboard: ``Running TESTSUITE hid_keyboard_qa``
- Joystick: ``Running TESTSUITE hid_joystick_qa``

Pass: ``PROJECT EXECUTION SUCCESSFUL``.

Host
****

USB **device** cable (not the debug UART). After the suite the product
string is one of:

- ``Alif HID Mouse``
- ``Alif HID Keyboard``
- ``Alif HID Joystick``

::

   dmesg | tail
   sudo evtest

Pick the Alif device. ``[UFW BLOCK]`` lines are the PC firewall.

``usbd_disable`` is not a fail. On DWC3 it can return ``-EIO`` after an
IN burst; joystick disable cases skip in that case. Mouse and keyboard
do not call ``usbd_disable``.

Interrupt IN completion is a DWC3 driver path. A quiet ``evtest`` after
enumeration means the host bound the HID device and the IN transfer did
not deliver a report. Device-side PASS still stands.

Test specifications
*******************

``testlink/TESTLINK_SPECS.rst`` lists every ztest, the UART pass line,
and the host check.

TestLink import (mouse, keyboard, and joystick in one file):
``testlink/testlink_hid.xml``.
Import that XML. The ``.rst`` file is not a TestLink import.

Sources
*******

- ``src/test_hid_mouse.c``
- ``src/test_hid_keyboard.c``
- ``src/test_hid_joystick.c``

CMake compiles **only** the suite selected with ``-DCONFIG_HID_TEST_*``.

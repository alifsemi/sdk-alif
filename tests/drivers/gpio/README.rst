# Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
# Use, distribution and modification of this code is permitted under the
# terms stated in the Alif Semiconductor Software License Agreement
#
# You should have received a copy of the Alif Semiconductor Software
# License Agreement with this file. If not, please write to:
# contact@alifsemi.com, or visit: https://alifsemi.com/license

Connect P6_4 (J12_22) to P1_7 (J11_20)

Supported Boards:
	``alif_b1_dk/ab1c1f4m51820hh/rtss_he``
	``alif_e8_dk/ae822fa0e5597xx0/rtss_he``
	``alif_e8_dk/ae822fa0e5597xx0/rtss_hp``
	``alif_e7_dk/ae722f80f55d5xx/rtss_he``
	``alif_e7_dk/ae722f80f55d5xx/rtss_hp	``


Requirements
************

Your board must:

#. Have an LED connected via a GPIO pin (these are called "User LEDs" on many of
   Zephyr's :ref:`boards`).
#. Have the LED configured using the ``led0`` devicetree alias.

Build Steps:
***********
if we need to run gpio on b1 board we need to use below command
rm -rf build;west build -b alif_b1_dk/ab1c1f4m51820hh/rtss_he ../alif/tests/drivers/gpio/ -DDTC_OVERLAY_FILE=$PWD/../alif/tests/drivers/gpio/boards/alif_b1_dk_rtss_he_gpio.overlay

if we need to run lpgpio on b1 board we need to use below command
rm -rf build;west build -b alif_b1_dk/ab1c1f4m51820hh/rtss_he ../alif/tests/drivers/gpio/ -DDTC_OVERLAY_FILE=$PWD/../alif/tests/drivers/gpio/boards/alif_b1_dk_rtss_he_lpgpio.overlay

Note: if user want's to run on other boards we need to change board name along with partnumber and overlay file also we need to change

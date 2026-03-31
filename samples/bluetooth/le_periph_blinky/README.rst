.. _bluetooth-periph-blinky-sample:

BLE Blinky Sample
#################

Overview
********

This sample application demonstrates the use of the Proprietary Blinky BLE profile. It allows a BLE central device to control an LED and receive button state notifications from the peripheral.

When Power Management (PM) is enabled, the sample uses LPGPIO pins instead of standard LED/Button interface in devkit:

* **LED output**: LPGPIO pin 0
* **Button input**: LPGPIO pin 1

On the Alif Balletto Development Kit, these correspond to J11 pin 28 (LED) and pin 30 (button).

.. note::
   Power Management (PM) is enabled by default in this sample. This is configured in ``CMakeLists.txt`` with the following line::

      set(SNIPPET pm-ble ${SNIPPET} CACHE STRING "" FORCE)

Requirements
************

* Alif Balletto Development Kit
* BLE central device, e.g., smartphone with a Alif BLE app (available on Android and iOS)

Building and Running
********************

This sample can be found under :zephyr_file:`samples/bluetooth/le_periph_blinky` in the
sdk-alif tree.

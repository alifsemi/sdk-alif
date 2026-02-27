.. _bluetooth-periph-beacon-sample:

BLE Beacon Sample
#################

Overview
********

Application to demonstrate the use of the BLE beacon.

When not connected, the application blinks an LED to indicate its status.
When Power Management (PM) is enabled, it toggles LPGPIO pin 1. On the Alif Balletto Development Kit, this corresponds to J11 pin 30.

.. note::
   Power Management (PM) is enabled by default in this sample. This is set in ``CMakeLists.txt`` by the following line::

      set(SNIPPET pm-ble ${SNIPPET} CACHE STRING "" FORCE)

Requirements
************

* Alif Balletto Development Kit

Building and Running
********************

This sample can be found under :zephyr_file:`samples/bluetooth/le_periph` in the
sdk-alif tree.
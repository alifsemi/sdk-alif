.. _bluetooth-periph-multi-profile:

BLE Multi-Profile Integration Sample
#####################

Overview
********

Application to demonstrate simultaneous integration of multiple BLE profiles, including standard health profiles and a custom Blinky service, within a single peripheral device.

This is a demo-only bundle intended for integration testing and reference.

Requirements
************

* Alif Balletto Development Kit
* BLE-capable central (phone / BLE app)

Building and Running
********************

When running, the device starts advertising and waits for a central to connect.
After connection, individual profiles begin operation once the corresponding CCCDs are enabled by the central.
Dummy measurements are sent for supported profiles.
The custom Blinky service allows LED control from the central device.

Limitations
********************

* GLPS / CGMS advertise correctly, but measurement values are not visible in some BLE apps due to incomplete RACP handling (demo limitation).
* Measurements are dummy values and not sensor-backed.

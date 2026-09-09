.. _bluetooth-periph-multi-profile:

BLE Multi-Profile Integration Sample
#####################

Overview
********

Application to demonstrate simultaneous integration of multiple BLE profiles,
including standard health profiles and a custom Blinky service, within a single peripheral device.
This is a demo-only bundle intended for integration testing and reference.

The following services are included:

* Heart Rate (HRPS)
* Blood Pressure (BLPS)
* Health Thermometer (HTPT)
* Glucose (GLPS)
* Continuous Glucose Monitoring (CGMS)
* Cycling Speed & Cadence (CSCPS)
* Running Speed & Cadence (RSCPS)
* Proximity (PRXP)
* Battery (BASS)
* Blinky LED control (custom 128-bit GATT service)

Requirements
************

* Alif Balletto Development Kit or Starter Kit
* BLE-capable central (phone / BLE app)

Building and Running
********************

When running, the device starts advertising and waits for a central to connect.
After connection, individual profiles begin operation once the corresponding CCCDs are enabled by the central.
Dummy measurements are sent for supported profiles.
The custom Blinky service allows LED control from the central device.

Pairing
*******

The Glucose (GLPS) and Continuous Glucose Monitoring (CGMS) profiles require a one-time
Bluetooth pairing when first accessed. A pairing request will appear on the central device
prompting for a passcode. Enter **000000** to complete pairing.

This pairing only happens once — on all subsequent connections the link is automatically
re-encrypted using the stored bond, so no passcode will be requested again.


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
* Cycling Speed & Cadence (CSCPS)
* Running Speed & Cadence (RSCPS)
* Proximity (PRXP)
* Battery (BASS)
* Blinky LED control (custom 128-bit GATT service)

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

* Glucose (GLPS) is advertised and the service is visible, but runtime
  measurement values are not accessible due to incomplete RACP (Record Access
  Control Point) handling.
* For the same reason as above, Continuous Glucose Monitor Sample (CGMS) was not added as well.
* The Proximity Profile (PRXP) includes Link Loss Service (LLS) only.
  Immediate Alert (IAS) and TX Power (TPS) are excluded to stay within the profile task limit.
* Measurements are dummy values and not sensor-backed.

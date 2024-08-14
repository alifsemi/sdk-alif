# Peripheral Proximity profile sample application

   Application to demonstrate the use of the Proximity profile BLE profile.

## Overview

An application that sends an alert on link loss with the alert level set by the central.
Battery service is supported.

## Requirements

* Alif Balletto Development Kit

## Building and Running

This sample application can be built using the following command:

```
west build -b alif_b1_fpga_rtss_he_ble alif/samples/bluetooth/le_periph_prxp
```

When running, the sample application starts advertising and waits for a central to connect.
After connection, the central should set the level of Link Loss alert.
If the link is lost, the peripheral will notify the application with the selected alert level.
Notifications for battery service are handled independently.

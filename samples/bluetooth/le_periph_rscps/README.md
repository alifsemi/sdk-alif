# Peripheral Running Speed and Cadence sample application

   Application to demonstrate the use of the Running Speed and Cadence BLE profile.

## Overview

An application that sends running data notifications to the first device that connects to it.
Battery service is supported.

## Requirements

* Alif Balletto Development Kit

## Building and Running

This sample application can be built using the following command:

```
west build -b alif_b1_fpga_rtss_he_ble alif/samples/bluetooth/le_periph_rscps
```

When running, the sample application starts advertising and waits for a central to connect.
After connection, the central should start notifications and the peripheral will send measurements every 2 seconds.
Notifications for battery service are handled independently.

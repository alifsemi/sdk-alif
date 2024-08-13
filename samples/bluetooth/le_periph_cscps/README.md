# Cycling Power sample application

   Application to demonstrate the use of the Cycling Speed and Cadence BLE profile.

## Overview

An application that sends Cycling Speed and Cadence notifications to the first device that connects to it.

## Requirements

* Alif Balletto Development Kit

## Building and Running

This sample application can be built using the following command:

```
west build -b alif_b1_fpga_rtss_he_ble alif/samples/bluetooth/le_periph_cscps
```

When running, the sample application starts advertising and waits for a central to connect.
After connection, the central should start notifications and the peripheral will send measurements every 1 second.

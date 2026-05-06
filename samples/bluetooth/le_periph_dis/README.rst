.. _bluetooth-periph-dis-sample:

BLE Device Information Service Sample
######################################

Overview
********

Peripheral sample demonstrating the Device Information Service (DIS) server.
Responds to read requests for the following characteristics:

* Manufacturer Name String
* Model Number String
* Firmware Revision String
* System ID

Requirements
************

* Alif Balletto Development Kit
* BLE-capable central (phone / BLE app)

Building and Running
********************

This sample is located under
``alif/samples/bluetooth/le_periph_dis`` in the sdk-alif tree.

When running, the device starts general undirected advertising and waits for a
central to connect.  After connection, all DIS characteristics are readable
without any subscription.

Limitations
***********

* DIS characteristics are static values; no runtime updates are generated.

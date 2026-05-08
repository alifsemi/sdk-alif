.. _bluetooth-periph-has-sample:

BLE Hearing Access Service Sample
##################################

Overview
********

Peripheral sample demonstrating the Hearing Access Service (HAS) server role
via the Hearing Access Profile (HAP).

Exposes a monaural hearing aid with one preset ("Default") and enables feature
notifications so a client can track hearing aid type changes. HAS bond data
(notification/indication subscription bits) is stored in NVS via
``ble_storage`` so subscriptions survive reconnects.

Requirements
************

* Alif Balletto Development Kit
* BLE-capable central (phone / BLE app) that supports HAP

Building and Running
********************

This sample is located under
``alif/samples/bluetooth/le_periph_has`` in the sdk-alif tree.

When running, the device starts general undirected advertising and waits for a
central to connect.  After connection, the central can enable HAS
notifications/indications to monitor Hearing Aid Features and Active Preset
Index changes.

Limitations
***********

* Preset records are read-only; write-preset-name requests are rejected.
* Hearing Aid Features value is static; no runtime feature updates are
  generated.

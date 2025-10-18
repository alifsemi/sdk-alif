.. _bluetooth-central-battery-sample:
BLE Battery Central Client Sample
##################################
Overview
********
This sample demonstrates a Bluetooth LE Central device that:

* Scans for BLE peripherals
* Connects to a peripheral device
* Discovers the Battery Service on the connected device
* Reads battery level values
* Enables notifications for battery level changes
The sample acts as a central/client and should be paired with a peripheral device
running the Battery Service (such as the le_periph_batt sample).

Requirements
************
* Alif Balletto Development Kit
* A BLE peripheral device advertising Battery Service
  (e.g., another board running the le_periph_batt sample)

Building and Running
********************
This sample can be found under :zephyr_file:`samples/bluetooth/le_central_batt` in the
sdk-alif tree.
Build and flash the sample as follows:

.. code-block:: console

   west build -b <board> samples/bluetooth/le_central_batt

The resulting binary then need to be flashed to MRAM.
After flashing, the device will:

1. Start scanning for BLE peripherals
2. Connect to any device advertising with the name given
3. Discover and read the battery level
4. Subscribe to battery level notifications

Expected Output
***************
On the serial console, you should see output similar to:

.. code-block:: console

	[00:00:00.447,000] <dbg> central_itf: central_itf_gapm_cfg: Waiting for init...

	[00:00:01.860,000] <inf> central_itf: Initiating direct connection
	[00:00:03.873,000] <inf> central_itf: Connection request on index 0
	[00:00:03.873,000] <dbg> central_itf: on_le_connection_req: Connection parameters: interval 40, latency 5, supervision timeout 100
	[00:00:03.873,000] <inf> central_itf: Peer device address DC:D3:D7:51:27:23 (conidx: 0)
	[00:00:06.022,000] <inf> batt_cli: Battery service discovered
	[00:00:06.022,000] <inf> batt_cli: Read Battery Level
	[00:00:07.071,000] <dbg> batt_cli: on_cb_value: Battery level: 66
	[00:00:07.171,000] <dbg> batt_cli: on_cb_enable_cmp: Notifications enabled
	[00:00:07.571,000] <dbg> batt_cli: on_cb_value: Battery level: 65
	[00:00:09.671,000] <dbg> batt_cli: on_cb_value: Battery level: 64
	[00:00:11.571,000] <dbg> batt_cli: on_cb_value: Battery level: 63
	[00:00:13.621,000] <dbg> batt_cli: on_cb_value: Battery level: 62
	[00:00:15.671,000] <dbg> batt_cli: on_cb_value: Battery level: 61
	[00:00:17.571,000] <dbg> batt_cli: on_cb_value: Battery level: 60


Notice
***************
Depending on the success of the connection, the following messages could be seen:

.. code-block:: console

	[00:00:04.471,000] <inf> central_itf: Connection index 0 disconnected for reason 0xCE
	[00:00:04.472,000] <err> batt_cli: Failed to discover Battery Service: 0x46

The application will re-attempt connecting and continue the process.


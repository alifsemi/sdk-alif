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

	*** Booting Zephyr OS build v4.1.0-316-g252c1c636b31 ***
	D: Waiting for init...

	I: Scanning...

	D: Init complete.

	I: Initiating direct connection
	I: Connection request on index 0
	D: Connection parameters: interval 40, latency 5, supervision timeout 100
	I: Peer device address C6:17:0B:1F:B7:10 (conidx: 0)
	I: Battery service discovered
	I: Read Battery Level
	D: Battery level: 93
	D: Notifications enabled
	D: Battery level: 90
	D: Battery level: 89
	D: Battery level: 88
	D: Battery level: 87

Notice
***************
Depending on the success of the connection, the following messages could be seen:

.. code-block:: console

	[00:00:04.471,000] <inf> central_itf: Connection index 0 disconnected for reason 0xCE
	[00:00:04.472,000] <err> batt_cli: Failed to discover Battery Service: 0x46

The application will re-attempt connecting and continue the process.


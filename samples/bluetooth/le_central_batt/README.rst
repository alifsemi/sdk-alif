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
running the Battery Service, such as the le_periph_batt sample (see :zephyr_file:`samples/bluetooth/le_periph_batt`).

Requirements
************
* Alif Balletto Development Kit
* A BLE peripheral device advertising Battery Service
  (e.g., another board running the le_periph_batt sample)

Building and Running
********************
This sample can be found under :zephyr_file:`samples/bluetooth/le_central_batt` in the
sdk-alif tree.

Building for an alif_b1_dk
--------------------------

.. zephyr-app-commands::
   :zephyr-app: samples/bluetooth/le_central_batt/
   :board: alif_b1_dk/ab1c1f4m51820ph0/rtss_he
   :goals: build

Expected Output
***************
On the serial console, you should see output similar to:

.. code-block:: console

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

	I: central_itf: Connection index 0 disconnected for reason 0xCE
	E: batt_cli: Failed to discover Battery Service: 0x46

The application will re-attempt connecting and continue the process.


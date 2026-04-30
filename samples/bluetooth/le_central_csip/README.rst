LE Central CSIP Sample
======================

This sample demonstrates a Bluetooth Low Energy (LE) Central device implementing the Coordinated Set Identification Profile (CSIP) Initiator role using the Alif Semiconductor SDK and Zephyr RTOS.

Features
--------
- Acts as a Bluetooth LE Central (client) to discover and connect to CSIP Set Members (peripherals).
- Handles pairing, bonding, and secure connections.
- Manages multiple peer devices and their bond data.
- Uses Zephyr's logging and settings subsystems.
- Supports device name and peripheral name configuration.

File Structure
--------------
- **main.c**: Main application logic, peer management, connection handling, and BLE stack configuration.
- **app_csisc.c/h**: CSIP client-specific logic.
- **app_gaf_scan.c/h**: Scanning and discovery logic for Generic Audio Framework (GAF).
- **storage.c/h**: Persistent storage for bond and key data.
- **prj.conf**: Zephyr configuration options for Bluetooth, logging, flash, and stack sizes.
- **Kconfig**: Custom configuration options for device and peripheral names.
- **app.overlay**: Device tree overlay for board-specific LEDs and buttons.

Configuration
-------------
- Device name: Set via `CONFIG_BLE_DEVICE_NAME` (default: "ALIF_CSIP_CLI").
- Peripheral name: Set via `CONFIG_PERIPHERAL_NAME` (default: "ALIF_CSIP-CSIP").
- Logging and stack sizes are configurable in `prj.conf`.

How It Works
------------
1. Initializes the BLE stack and loads bond data from storage.
2. Configures the device as a LE Central and sets up security.
3. Scans for CSIP Set Members and attempts to connect.
4. Handles pairing, bonding, and secure reconnections.
5. Manages peer contexts and bond data for multiple devices.

Requirements
------------
- Alif Semiconductor hardware with Zephyr RTOS support.
- Proper board overlay for LEDs and buttons (see `app.overlay`).
- Zephyr toolchain and build environment.

Building and Running
--------------------
1. Configure the project using `prj.conf` and `Kconfig` as needed.
2. Build the sample with your Zephyr toolchain.
3. Flash the binary to your board.
4. Monitor logs via RTT or UART as configured.


Usage Instructions
------------------

1. **Build the Sample**
	- Make sure your Zephyr environment is set up and your board is supported.
	- From the project root, run:
	  ::

		 west build -b <your_board> alif/samples/bluetooth/le_central_csip

2. **Flash the Firmware**
	- Connect your board and flash the binary:
	  ::

		 west flash

3. **Connect to the Device**
	- Power on the CSIP Set Member (peripheral) devices.
	- The central will scan and attempt to connect automatically.
	- Monitor the logs via RTT or UART for connection and bonding status.

4. **Resetting Bonds**
	- To clear bond data, erase the board's flash or use a dedicated erase command if available.

5. **Customizing**
	- Change the device name or peripheral name in `prj.conf` or via menuconfig:
	  ::

		 west build -t menuconfig

	- Adjust stack sizes, logging, or other options as needed.

6. **Troubleshooting**
	- Ensure the board overlay matches your hardware (see `app.overlay`).
	- Check Zephyr and SDK documentation for additional help.

For more advanced usage, refer to the source code and comments in each file.


Joystick Button Usage
----------------------

This application supports three joystick buttons for user interaction. The joystick buttons are mapped as follows (see `app.overlay`):

- **TOGGLE_LOCK** (button0): Toggles the CSIP lock state for the first device in the list.
- **DISCONNECT** (button_up): Disconnects all connected peers.
- **READ_SIRK** (button_down): Reads the SIRK (Set Identity Resolving Key) of the first device in the list.

Button GPIO mapping:

- TOGGLE_LOCK:  GPIO5, Pin 7
- DISCONNECT:   GPIO5, Pin 0
- READ_SIRK:    GPIO5, Pin 4

Typical usage:

* **Press TOGGLE_LOCK** to toggle the lock state of the first discovered device.
* **Press DISCONNECT** to disconnect all currently connected peers.
* **Press READ_SIRK** to read and display the SIRK of the first discovered device.

The buttons are debounced and handled via Zephyr's GPIO and workqueue mechanisms. Ensure your hardware matches the overlay configuration for correct operation.

For custom actions or additional buttons, modify the `app.overlay` and update the application logic as needed.

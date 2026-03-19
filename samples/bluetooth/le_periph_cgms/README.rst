
.. _bluetooth-periph-cgms-sample:

BLE Continuous Glucose Monitor (CGMS) Sample
############################################

Overview
--------
This sample demonstrates the use of the Bluetooth Low Energy (BLE) Continuous Glucose Monitor Service (CGMS) profile on the Alif Balletto Development Kit. It implements a peripheral device that advertises, connects to a central, and provides CGMS measurements, battery service notifications, and advanced control point operations.

Features
--------
- Implements the BLE CGMS profile for continuous glucose monitoring
- Periodic measurement notifications to a connected central
- Battery Service notifications
- Full support for Record Access Control Point (RACP) operations:
  - Report number of stored records
  - Report stored records (with filtering)
  - Abort operation
  - Error handling for invalid/unsupported operations
- Sensor annotations (optional, for testing/debugging)
- Configurable record store size for compliance testing
- Power management (PM) support via snippet

Requirements
------------
- Alif Balletto Development Kit
- Zephyr SDK and toolchain
- A BLE central device (e.g., smartphone app or test tool)

Building and Running
--------------------
1. Locate the sample in the SDK tree:
        :zephyr_file:`samples/bluetooth/le_periph_cgms`

2. Build the sample:
        .. code-block:: console

                west build -b <your_board> samples/bluetooth/le_periph_cgms

        To enable power management features, add the following to your build command:

                west build -b <your_board> samples/bluetooth/le_periph_cgms --snippets pm_ble

3. Flash the image to your board and reset.

Usage
-----
- On startup, the device advertises and waits for a BLE central to connect.
- After connection, enable notifications on the central to receive CGMS and battery measurements every second.
- RACP commands can be sent from the central to request stored records, report counts, or abort operations. The device will respond and send records as appropriate.
- If sensor annotations are enabled (see below), additional information will be included in measurement notifications.

Configuration Options
---------------------
- **Sensor Annotations**: Enable by adding the following to your prj.conf:

  .. code-block:: console

          CONFIG_IUT_TESTER_ENABLED=y

  When enabled, sensor annotations are included in CGMS measurements. This is useful for protocol testing and debugging.

- **Record Store Size**: When CONFIG_IUT_TESTER_ENABLED is enabled, the record store size toggles between 3 and 250. Toggle record size is done by pressing the SW0 button. A larger store is required for certain Bluetooth SIG test cases, such as:
  - CGMS/SEN/RAA/BV-01-C (Abort Operation – ‘Report Stored Records’)
  - CGMS/SEN/RAE/BI-02-C (RACP specific Errors – ‘Procedure Already In Progress’)

- **Power Management**: To enable PM features, include the 'pm_ble' snippet as shown above. This ensures the necessary power management functionality is included in your build.

Testing
-------
- Use a BLE central (e.g., nRF Connect, Android/iOS app, or test tool) to connect and interact with the device.
- Enable notifications for CGMS and battery services.
- Send RACP commands to test record reporting, abort, and error handling.
- If testing with CONFIG_IUT_TESTER_ENABLED, verify that sensor annotations and record store size behave as expected.

References
----------
- Bluetooth SIG CGMS Profile: https://www.bluetooth.com/specifications/specs/glucose-profile-1-0/
- Zephyr Project Documentation: https://docs.zephyrproject.org/

For further details, see the source code and comments in this directory.

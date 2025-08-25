=================================
SE Tool Flashing for Alif DevKits
=================================

Introduction
============

This application note provides a concise guide on using the Secure Execution (SE) Tool to flash application binaries onto Alif Semiconductor’s Ensemble or Balletto DevKits for secure execution. The SE Tool programs binaries into the DevKit’s memory (e.g., MRAM) by generating an Application Table of Contents (ATOC). This method supports applications running on the Real-Time Subsystem High-Performance (RTSS-HP, Cortex-M55_0) and Real-Time Subsystem High-Efficiency (RTSS-HE, Cortex-M55_1) cores, with optional AES encryption for secure booting.

The note focuses on the flashing process, JSON configuration, and memory considerations, using the Bluetooth Low Energy (BLE) application as an example.

Overview
========

The SE Tool is a Python-based utility from Alif Semiconductor for programming application binaries into Alif DevKits. Key features include:

- ATOC Generation: Creates an ATOC defining the binary’s metadata (e.g., version, core, memory addresses).
- Memory Programming: Writes the binary and ATOC to MRAM, enabling secure boot.
- AES Encryption Support: Optionally encrypts binaries using a 16-byte AES key.
- Core Targeting: Supports RTSS-HP (Cortex-M55_0) and RTSS-HE (Cortex-M55_1).
- Memory Flexibility: Supports execution from SRAM, TCM, or OSPI NOR flash.

Hardware Requirements
=====================

- Alif Ensemble or Balletto DevKit: Provides RTSS-HP (Cortex-M55_0) and RTSS-HE (Cortex-M55_1) cores, with MRAM for storage.
- USB Connection: For communication between the DevKit and the host PC running the SE Tool.

Software Requirements
=====================

- Alif Zephyr SDK: Clone from https://github.com/alifsemi/sdk-alif, version 1.1.0 or later.
- West Tool: For managing Alif Zephyr SDK builds.
- GCC Compiler: Included in the Alif Zephyr SDK.
- SE Tool: Python scripts (app-gen-toc.py, app-write-mram.py) in /home/$USER/<SE tool folder>.
- Python 3: Required to run SE Tool scripts.
- AES Encryption Tool (Optional): CSPI_AES128_ECB in /home/$USER/ZAS-v1.2.0-Beta/prebuilt-images/CSPI_AES128_ECB.

.. note::
   Replace ZAS-v1.2.0-Beta with the appropriate Alif Zephyr SDK or SE Tool version.

Building the Application
========================

Compile the application to generate the binary (zephyr.bin) using the Alif Zephyr SDK.

.. note::

   The application is designed for the Alif Ensemble E7 DevKit. Modify the sample code as needed for other DevKits.

1. For instructions on fetching the Alif Zephyr SDK, please refer to the `ZAS User Guide`_

2. Build the Application:
   - Example for BLE (RTSS-HE):

   .. code-block:: bash

      west build -p always -b alif_b1_dk_rtss_he samples/bluetooth/le_periph_hr

For other applications, use the appropriate board target (e.g., alif_e1c_dk_rtss_hp) and sample path.

3. Locate the Binary:
   - The binary (zephyr.bin) is in ./build/zephyr/.

SE Tool Flashing Process
========================

The SE Tool flashing process involves creating a JSON configuration, copying files to the SE Tool’s directories, and running scripts to program the DevKit’s memory.

Step 1: Create a JSON Configuration File
----------------------------------------

Create a JSON file (e.g., ble_hr.json) defining the application’s metadata. Store it in the SE Tool’s config directory.

Template JSON::

   ```json
   {
       "DEVICE": {
           "disabled": false,
           "binary": "app-device-config.json",
           "version": "0.5.00",
           "signed": true
       },
       "APP_NAME": {
           "binary": "zephyr.bin",
           "version": "1.0.0",
           "signed": false,
           "cpu_id": "M55_HP or M55_HE",
           "mramAddress": "MRAM_ADDRESS",
           "loadAddress": "EXECUTION_ADDRESS",
           "flags": ["load", "boot"]
       }
   }
   ```

BLE Example::

   ```json
   {
       "DEVICE": {
           "disabled": false,
           "binary": "app-device-config.json",
           "version": "0.5.00",
           "signed": true
       },
       "BLE-HR": {
           "binary": "zephyr.bin",
           "version": "1.0.0",
           "signed": false,
           "cpu_id": "M55_HE",
           "mramAddress": "0x80000000",
           "flags": ["boot"]
       }
   }
   ```

Customization:

- APP_NAME: Application identifier (e.g., BLE-HR).
- cpu_id: M55_HP for RTSS-HP, M55_HE for RTSS-HE.
- mramAddress: Typically 0x80000000 for MRAM storage.
- signed: Set to true if encrypted, false otherwise.

Step 2: Encrypt the Binary (Optional)
-------------------------------------

Encrypt the binary for secure applications using a 16-byte AES key.

.. code-block:: bash

   /home/$USER/ZAS-v1.2.0-Beta/prebuilt-images/CSPI_AES128_ECB -i build/zephyr/zephyr.bin -o build/zephyr/zephyr_en.bin -k 0123456789ABCDEF -d 1

- Replace the key with your 16-byte AES key.
- Update the JSON to use "binary": "zephyr_en.bin" and "signed": true.

Step 3: Copy Files to SE Tool Directories
-----------------------------------------

1. Copy the Binary:

   .. code-block:: bash

      cp ./build/zephyr/zephyr.bin <SE tool folder>/build/images/

2. Copy the JSON Configuration:

   .. code-block:: bash

      cp ble_hr.json <SE tool folder>/build/config/

Step 4: Run SE Tool Scripts
---------------------------

1. Generate ATOC:

   .. code-block:: bash

      cd <SE tool folder>
      python3 app-gen-toc.py --filename build/config/ble_hr.json

2. Write to MRAM:

   .. code-block:: bash

      python3 app-write-mram.py

Step 5: Reset the DevKit
------------------------

- Reset the DevKit using the reset button or a software command.
- Verify execution:
  - For BLE, use a mobile app to detect the ALIF_HR device.
  - Monitor UART output via a terminal emulator (e.g., minicom).

Hardware Connections and Setup
===============================

- USB Connection: Connect the DevKit to the host PC for flashing and UART.
- UART for Debugging: Configure a terminal emulator to the DevKit’s UART port (consult DevKit documentation for port and baud rate, typically 115200).
- Application-Specific Setup: For BLE, no additional connections are needed; other applications may require specific pin connections.

Troubleshooting
===============

- Binary Fails to Boot:
  - Ensure cpu_id matches the build target (e.g., M55_HE for alif_b1_dk_rtss_he).
  - Verify mramAddress aligns with the DevKit’s memory map.

- SE Tool Script Errors:
  - Confirm Python 3 is installed and the SE Tool path is correct.
  - Check JSON syntax and file paths in build/images and build/config.

- No Console Output:
  - Verify UART settings in the terminal emulator.
  - Ensure the DevKit is powered and reset correctly.

- Encryption Issues:
  - If "signed": true, ensure the binary is encrypted with the correct AES key.

Sample Output
=============

For the BLE application

   .. code-block:: bash

         Device ALIF_HR appears in BLE scanner apps.

Conclusion
==========

The SE Tool flashing method enables secure programming of Alif DevKit applications. By configuring a JSON file, copying files to the SE Tool’s directories, and running the provided scripts, users can flash binaries to MRAM for execution on RTSS-HP or RTSS-HE cores. The BLE example demonstrates the process, which can be adapted for other applications by adjusting core and memory settings. For further assistance, contact Alif Semiconductor support or refer to the Alif Zephyr SDK documentation.
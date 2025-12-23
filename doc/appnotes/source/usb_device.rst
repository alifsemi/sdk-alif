
============
USB Device
============

Introduction
============

This application note outlines the process of creating, compiling, and running a sample CDC-ACM demo application using the Alif UDC driver (dwc3) in USB device mode on the DevKit.
The implementation enables the target board to function as a USB virtual COM port, allowing communication with a host PC over the USB interface.

USB Features
------------

- The Alif UDC dwc3 driver supports **USB 2.0 High Speed**.
- Supports CDC-ACM.

.. include:: prerequisites.rst

.. include:: note.rst

Building an USB Application with Zephyr
========================================

Follow these steps to build the USB application using the Alif Zephyr SDK:

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, please refer to the `ZAS User Guide`_


.. note::
   The build commands shown here are specifically for the Alif E7 DevKit.
   To build the application for other boards, modify the board name in the build command accordingly. For more information, refer to the `ZAS User Guide`_, under the section Setting Up and Building Zephyr Applications.

2. Build commands for applications on the M55 HP core:

.. code-block:: bash

   west build -b alif_e7_dk/ae722f80f55d5xx/rtss_hp samples/subsys/usb/cdc_acm/ -DCONF_FILE=usbd_next_prj.conf -DDTC_OVERLAY_FILE=boards/alif_usb.overlay


3. Build commands for applications on the M55 HE core:

.. code-block:: bash

   west build -b alif_e7_dk/ae722f80f55d5xx/rtss_he samples/subsys/usb/cdc_acm/ -DCONF_FILE=usbd_next_prj.conf -DDTC_OVERLAY_FILE=boards/alif_usb.overlay

Once the build command completes successfully, executable images will be generated and placed in the `build/zephyr` directory. Both `.bin` (binary) and `.elf` (Executable and Linkable Format) files will be available.

Executing Binary on the DevKit
===============================

To execute binaries on the DevKit, follow the command:

.. code-block:: bash

    west flash


Validation
============

To validate that the USB device has been correctly enumerated and is functioning as a virtual COM port, follow these steps:

**On Windows Host:**

1. Ensure the device is detected by checking **Device Manager** under **Ports (COM & LPT)**.
   The device should appear as a COM port (e.g., `COMx`).
2. Open the COM port using **Tera Term**.
3. Set the baud rate to **115200** and establish a connection.
4. Type characters in Tera Term—the device should echo them back, confirming bidirectional communication.

**On Linux Host:**

1. Verify device detection:

   .. code-block:: console

      ls /dev/ttyACM*

2. Open the serial port using **minicom**:

   .. code-block:: console

      sudo minicom -D /dev/ttyACM* -b 115200

3. In another terminal, send data to the device:

   .. code-block:: console

      echo "Test Data" > /dev/ttyACM*

4. The sent data should appear in the **minicom** window, confirming successful communication.

Console Output
===============


.. code-block:: text

   [00:00:00.105,000] <err> usbd_cdc_acm: Failed to add interface string descriptor
   [00:00:00.105,000] <inf> cdc_acm_echo: USB device support enabled
   [00:00:00.105,000] <inf> cdc_acm_echo: Wait for DTR
   [00:00:00.206,000] <inf> cdc_acm_echo: USBD message: Bus reset
   [00:00:00.261,000] <inf> cdc_acm_echo: USBD message: VBUS ready
   [00:00:00.329,000] <inf> cdc_acm_echo: USBD message: New device configuration
   [00:00:00.335,000] <inf> cdc_acm_echo: USBD message: CDC ACM control line state
   [00:00:00.335,000] <inf> cdc_acm_echo: USBD message: CDC ACM line coding
   [00:00:00.335,000] <inf> cdc_acm_echo: Baudrate 115200
   [00:00:00.352,000] <inf> cdc_acm_echo: USBD message: CDC ACM line coding
   [00:00:00.352,000] <inf> cdc_acm_echo: Baudrate 115200
   [00:00:00.352,000] <inf> cdc_acm_echo: USBD message: CDC ACM control line state
   [00:00:00.352,000] <inf> cdc_acm_echo: DTR set
   [00:00:00.352,000] <inf> cdc_acm_echo: USBD message: CDC ACM line coding
   [00:00:00.352,000] <inf> cdc_acm_echo: Baudrate 115200


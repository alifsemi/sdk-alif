
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
- CDC-ACM.
- MSC

.. include:: prerequisites.rst

.. include:: note.rst

Build an USB CDC_ACM sample Application with Zephyr
===================================================

Follow these steps to build the USB cdc-acm sample application using the Alif Zephyr SDK:

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, please refer to the `ZAS User Guide`_


.. note::
   The build commands shown here are specifically for the Alif Boards.
   To build the application for other boards, modify the board name in the build command accordingly. For more information, refer to the `ZAS User Guide`_, under the section Setting Up and Building Zephyr Applications.

2. Build command for cdc-acm sample application on Alif E7 Board(M55 HP):

.. code-block:: console

   west build \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     samples/subsys/usb/cdc_acm/ \
     -- \
     -DCONF_FILE=usbd_next_prj.conf \
     -DDTC_OVERLAY_FILE=boards/alif_usb.overlay


3. Build command for cdc-acm sample application on Alif E7 Board(M55 HE):

.. code-block:: console

   west build \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     samples/subsys/usb/cdc_acm/ \
     -- \
     -DCONF_FILE=usbd_next_prj.conf \
     -DDTC_OVERLAY_FILE=boards/alif_usb.overlay

4. Build command for cdc-acm sample application on Alif E8 Board(M55 HP)

.. code-block:: console

   west build \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     samples/subsys/usb/cdc_acm/ \
     -- \
     -DCONF_FILE=usbd_next_prj.conf \
     -DDTC_OVERLAY_FILE=boards/alif_usb.overlay

5. Build command for cdc-acm sample application on Alif E8 Board(M55 HE)

.. code-block:: console

   west build \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     samples/subsys/usb/cdc_acm/ \
     -- \
     -DCONF_FILE=usbd_next_prj.conf \
     -DDTC_OVERLAY_FILE=boards/alif_usb.overlay

6. Build command for cdc-acm sample application on Alif B1 Board(M55 HE):

.. code-block:: console

   west build \
     -b alif_b1_dk/ab1c1f4m51820hh0/rtss_he \
     samples/subsys/usb/cdc_acm/ \
     -- \
     -DCONF_FILE=usbd_next_prj.conf \
     -DDTC_OVERLAY_FILE=boards/alif_usb.overlay

7. Build command for cdc-acm sample application on Alif E1C Board(M55 HE):

.. code-block:: console

   west build \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     samples/subsys/usb/cdc_acm/ \
     -- \
     -DCONF_FILE=usbd_next_prj.conf \
     -DDTC_OVERLAY_FILE=boards/alif_usb.overlay

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


Build an USB MSC sample Application with Zephyr
================================================

This application note describes the implementation of the USB Mass Storage Class (MSC) device using Zephyr RTOS on the Alif platform. The solution leverages the DWC3 USB device controller to enumerate the Alif board as a USB mass storage device on the host system.

The application supports multiple storage backends, including RAM disk, SD card, and OSPI flash. Each storage medium can be configured independently, allowing the device to operate with any one of them or with all three simultaneously. When configured together, each storage type is exposed as a separate Logical Unit Number (LUN).

Upon successful enumeration, the host system detects and populates the configured storage media as individual mass storage devices. This allows the host to access each medium independently using standard file system operations such as read and write, similar to conventional USB storage devices.

Follow these steps to build the USB MSC sample application using the Alif Zephyr SDK:

For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, please refer to the `ZAS User Guide`_


.. note::
   The build commands shown here are specifically for the Alif Boards.
   To build the application for other boards, modify the board name in the build command accordingly. For more information, refer to the `ZAS User Guide`_, under the section Setting Up and Building Zephyr Applications.

MSC with SD Support
-------------------

1. Build command for MSC sample application with SD support on Alif E7 Board(M55 HP):

.. code-block:: console

   west build \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     samples/subsys/usb/mass/ \
     "-DCONF_FILE=usbd_next_prj.conf;boards/alif_msc_sd.conf" \
     -S alif-msc-sd

2. Build command for MSC sample application with SD support on Alif E7 Board(M55 HE):

.. code-block:: console

   west build \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     samples/subsys/usb/mass/ \
     "-DCONF_FILE=usbd_next_prj.conf;boards/alif_msc_sd.conf" \
     -S alif-msc-sd

3. Build command for MSC sample application with SD support on Alif E8 Board(M55 HP):

.. code-block:: console

   west build \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     samples/subsys/usb/mass/ \
     "-DCONF_FILE=usbd_next_prj.conf;boards/alif_msc_sd.conf" \
     -S alif-msc-sd

4. Build command for MSC sample application with SD support on Alif E8 Board(M55 HE):

.. code-block:: console

   west build \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     samples/subsys/usb/mass/ \
     "-DCONF_FILE=usbd_next_prj.conf;boards/alif_msc_sd.conf" \
     -S alif-msc-sd

5. Build command for MSC sample application with SD support on Alif B1 Board(M55 HE):

.. code-block:: console

   west build \
     -b alif_b1_dk/ab1c1f4m51820hh0/rtss_he \
     samples/subsys/usb/mass/ \
     "-DCONF_FILE=usbd_next_prj.conf;boards/alif_msc_sd.conf" \
     -S alif-msc-sd

6. Build command for MSC sample application with SD support on Alif E1C Board(M55 HE):

.. code-block:: console

   west build \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     samples/subsys/usb/mass/ \
     "-DCONF_FILE=usbd_next_prj.conf;boards/alif_msc_sd.conf" \
     -S alif-msc-sd


Once the build command completes successfully, executable images will be generated and placed in the `build/zephyr` directory. Both `.bin` (binary) and `.elf` (Executable and Linkable Format) files will be available.

Executing Binary on the DevKit
===============================

To execute binaries on the DevKit, follow the command:

.. code-block:: bash

    west flash


Validation of USB MSC Application
==================================

To validate the functionality of the USB Mass Storage (MSC) application on the Alif board, follow these steps:

1. Connect the Alif board to the host machine (Windows/Linux) via USB.
2. Ensure the board enumerates as a mass storage device (either RAMDISK or SD card or OSPI) on the host system (e.g., separate drives).
3. Test the file read and write operations by copying files to the board's storage.


Console Output with SD Support
==============================

.. code-block:: text

   *** Booting Zephyr OS build 783f9469451a ***
   Mount /SD:: 0
   /SD:: bsize = 512 ; frsize = 8192 ; blocks = 1942592 ; bfree = 1942588
   /SD: opendir: 0
     D 0 System Volume Information
   End of files
   [00:00:05.878,000] <inf> main: The device is put in USB mass storage mode.
   [00:00:06.100,000] <inf> usbd_msc: Enable
   [00:00:06.100,000] <inf> usbd_msc: Bulk-Only Mass Storage Reset


MSC with RAMDISK Support
-------------------------

1. Build command for MSC sample application with RAMDISK support on Alif E7 Board(M55 HP):

.. code-block:: console

   west build \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     samples/subsys/usb/mass/ \
     -DCONF_FILE=usbd_next_prj.conf \
     "-DDTC_OVERLAY_FILE=boards/alif_usb.overlay;ramdisk.overlay"

2. Build command for MSC sample application with RAMDISK support on Alif E7 Board(M55 HE):

.. code-block:: console

   west build \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     samples/subsys/usb/mass/ \
     -DCONF_FILE=usbd_next_prj.conf \
     "-DDTC_OVERLAY_FILE=boards/alif_usb.overlay;ramdisk.overlay"

3. Build command for MSC sample application with RAMDISK support on Alif E8 Board(M55 HP):

.. code-block:: console

   west build \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     samples/subsys/usb/mass/ \
     -DCONF_FILE=usbd_next_prj.conf \
     "-DDTC_OVERLAY_FILE=boards/alif_usb.overlay;ramdisk.overlay"

4. Build command for MSC sample application with RAMDISK support on Alif E8 Board(M55 HE):

.. code-block:: console

   west build \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     samples/subsys/usb/mass/ \
     -DCONF_FILE=usbd_next_prj.conf \
     "-DDTC_OVERLAY_FILE=boards/alif_usb.overlay;ramdisk.overlay"

5. Build command for MSC sample application with RAMDISK support on Alif B1 Board(M55 HE):

.. code-block:: console

   west build \
     -b alif_b1_dk/ab1c1f4m51820hh0/rtss_he \
     samples/subsys/usb/mass/ \
     -DCONF_FILE=usbd_next_prj.conf \
     "-DDTC_OVERLAY_FILE=boards/alif_usb.overlay;ramdisk.overlay"

6. Build command for MSC sample application with RAMDISK support on Alif E1C Board(M55 HE):

.. code-block:: console

   west build \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     samples/subsys/usb/mass/ \
     -DCONF_FILE=usbd_next_prj.conf \
     "-DDTC_OVERLAY_FILE=boards/alif_usb.overlay;ramdisk.overlay"


Once the build command completes successfully, executable images will be generated and placed in the `build/zephyr` directory. Both `.bin` (binary) and `.elf` (Executable and Linkable Format) files will be available.

Executing Binary on the DevKit
===============================

To execute binaries on the DevKit, follow the command:

.. code-block:: bash

    west flash


Console Output with RAMDISK Support
===================================

.. code-block:: text

   *** Booting Zephyr OS build 783f9469451a ***
   [00:00:00.000,000] <inf> main: No file system selected
   [00:00:00.105,000] <inf> main: The device is put in USB mass storage mode.
   [00:00:00.328,000] <inf> usbd_msc: Enable
   [00:00:00.328,000] <inf> usbd_msc: Bulk-Only Mass Storage Reset


MSC with RAMDISK, SD Card and OSPI Support
------------------------------------------

1. Build command for MSC sample application with RAMDISK, SD card and OSPI support on Alif E7 Board(M55 HP):

.. code-block:: console

   west build \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/subsys/usb/mass/ \
     -S alif-msc-ramdisk-ospi-sd

2. Build command for MSC sample application with RAMDISK, SD card and OSPI support on Alif E7 Board(M55 HE):

.. code-block:: console

   west build \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/subsys/usb/mass/ \
     -S alif-msc-ramdisk-ospi-sd

3. Build command for MSC sample application with RAMDISK, SD card and OSPI support on Alif E8 Board(M55 HP):

.. code-block:: console

   west build \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/subsys/usb/mass/ \
     -S alif-msc-ramdisk-ospi-sd

4. Build command for MSC sample application with RAMDISK, SD card and OSPI support on Alif E8 Board(M55 HE):

.. code-block:: console

   west build \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/subsys/usb/mass/ \
     -S alif-msc-ramdisk-ospi-sd

5. Build command for MSC sample application with RAMDISK, SD card and OSPI support on Alif B1 Board(M55 HE):

.. code-block:: console

   west build \
     -b alif_b1_dk/ab1c1f4m51820hh0/rtss_he \
     ../alif/samples/subsys/usb/mass/ \
     -S alif-msc-ramdisk-ospi-sd

6. Build command for MSC sample application with RAMDISK, SD card and OSPI support on Alif E1C Board(M55 HE):

.. code-block:: console

   west build \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     ../alif/samples/subsys/usb/mass/ \
     -S alif-msc-ramdisk-ospi-sd


Once the build command completes successfully, executable images will be generated and placed in the `build/zephyr` directory. Both `.bin` (binary) and `.elf` (Executable and Linkable Format) files will be available.

Executing Binary on the DevKit
===============================

To execute binaries on the DevKit, follow the command:

.. code-block:: bash

    west flash


Console Output with RAMDISK, SD Card and OSPI Support
=====================================================

.. code-block:: text

   *** Booting Zephyr OS build 783f9469451a ***
   Area 0 at 0x0 on ospi_flash@0 for 67108864 bytes
   Mount /RAM:: 0
   /RAM:: bsize = 512 ; frsize = 512 ; blocks = 158 ; bfree = 158
   /RAM: opendir: 0
   End of files
   [00:00:01.100,000] <inf> flashdisk: Initialize device NAND
   [00:00:01.100,000] <inf> flashdisk: offset 0, sector size 512, page size 4096, volume size 67108864
   Mount /NAND:: 0
   /NAND:: bsize = 512 ; frsize = 4096 ; blocks = 16371 ; bfree = 16312
   /NAND: opendir: 0
     D 0 System Volume Information
     F 79027 CMSIS_driver_MSC.tdc
     F 45804 SDSIO_server_8kb.tdc
     F 47553 SDSIO_server_9kb.tdc
     F 47040 SDSIO_server_camera_with_usb.tdc
   End of files
   Mount /SD:: 0
   /SD:: bsize = 512 ; frsize = 4096 ; blocks = 1946112 ; bfree = 1946050
   /SD: opendir: 0
     D 0 System Volume Information
     F 5966 camera_with_usb.txt
     F 45804 SDSIO_server_8kb.tdc
     F 47553 SDSIO_server_9kb.tdc
     F 47040 SDSIO_server_camera_with_usb.tdc
     F 79027 CMSIS_driver_MSC.tdc
   End of files
   [00:00:20.445,000] <inf> main: The device is put in USB mass storage mode.
   [00:00:20.669,000] <inf> usbd_msc: Enable
   [00:00:20.669,000] <inf> usbd_msc: Bulk-Only Mass Storage Reset



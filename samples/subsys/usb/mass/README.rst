.. sdk-alif:code-sample:: usb-mass
   :name: USB Mass Storage
   :relevant-api: usbd_api usbd_msc_device _usb_device_core_api file_system_api

   Expose an Alif board's RAM, OSPI FLASH, or SD card as a USB disk.

Overview
********

This sample demonstrates USB Mass Storage Class (MSC) on Alif Semiconductor
SoCs. It can expose one or more storage devices (RAM disk, OSPI flash, SD card)
as USB LUNs that appear as removable drives on the host PC. Both FAT and
LittleFS file systems are supported.

This sample can be found under :sdk_alif:`alif/samples/subsys/usb/mass` in the
sdk-alif tree.

Supported Boards
****************

The following Alif development kits are supported:

- **Alif B1 DK** (``alif_b1_dk``)
- **Alif E1C DK** (``alif_e1c_dk``)
- **Alif E7 DK** (``alif_e7_dk``)
- **Alif E8 DK** (``alif_e8_dk``)

Requirements
************

- A USB device controller (DWC3) supported by the board.
- For OSPI flash: the board must have an OSPI flash device.
- For SD card: the board must have an SD card slot with a FAT-formatted card.
- For RAM disk: at least 96 KiB of available RAM.

Storage Configurations
**********************

The storage back-end and file system are selected via Kconfig options in
``prj.conf``. The available configurations are:

- ``CONFIG_APP_MSC_STORAGE_RAM`` — RAM disk with FAT file system
- ``CONFIG_APP_MSC_STORAGE_FLASH_FATFS`` — OSPI flash with FAT file system
- ``CONFIG_APP_MSC_STORAGE_SDCARD`` — SD card with FAT file system

Building and Running
********************

Single Partition (One LUN)
==========================

To expose only **one** storage device, set
``CONFIG_USBD_MSC_LUNS_PER_INSTANCE=1`` in ``prj.conf`` and enable only the
desired storage back-end.

RAM-disk Example without any file system
-----------------------------------------

The default configuration selects a RAM-based disk without any file system.
This example only needs additional 96 KiB RAM for the RAM-disk and is intended
for testing the USB mass storage class implementation.

For RAM disk only, use the board-specific overlay file from the
``alif-msc-ramdisk-ospi-sd`` snippet. The overlay must contain the following
nodes (remaining nodes can be commented out):

.. code-block:: dts

   &ns {
       status = "okay";
   };

   zephyr_udc0: &usb {
       status = "okay";
   };

   / {
       ramdisk0 {
           compatible = "zephyr,ram-disk";
           disk-name = "RAM";
           sector-size = <512>;
           sector-count = <192>;
       };
   };

Modify ``prj.conf`` accordingly by disabling the SD card and OSPI support
configs.

FAT FS Example with OSPI
-------------------------

In this example we build the sample with a FLASH-based disk and FAT file
system. The board configures to use the external 64 MiB OSPI flash chip
with a 64 MiB FAT partition.

.. code-block:: cfg

   CONFIG_APP_MSC_STORAGE_FLASH_FATFS=y

The ``alif-msc-ramdisk-ospi-sd`` snippet overlays already contain the required
OSPI configuration for each board. Modify ``prj.conf`` accordingly by disabling
the SD card support configs.

SD Card Example
----------------

This example requires SD card support, see :ref:`disk_access_api`, and
an SD card formatted with FAT file system.

.. code-block:: cfg

   CONFIG_APP_MSC_STORAGE_SDCARD=y

The ``alif-msc-ramdisk-ospi-sd`` snippet overlays already contain the required
SD configuration for each board. Modify ``prj.conf`` accordingly by disabling
the OSPI support configs.

Multi-Partition
===============

To expose **multiple** storage devices simultaneously as separate USB LUNs,
use the ``alif-msc-ramdisk-ospi-sd`` snippet which provides board-specific
overlays that enable all three storage devices (RAM disk, OSPI flash, SD card).

**RAM + OSPI flash + SD card:**

The default ``prj.conf`` shipped with this sample enables all three storage
back-ends.

Build command (B1 DK):

.. code-block:: shell

   rm -rf build/
   west build -b alif_b1_dk/ab1c1f4m51820hh0/rtss_he \
       ../alif/samples/subsys/usb/mass/ \
       -DCONFIG_FLASH_BASE_ADDRESS=0 \
       -DCONFIG_FLASH_LOAD_OFFSET=0 \
       -DCONFIG_FLASH_SIZE=256 \
       -S alif-msc-ramdisk-ospi-sd

Build command (E7 DK):

.. code-block:: shell

   rm -rf build/
   west build -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
       ../alif/samples/subsys/usb/mass/ \
       -DCONFIG_FLASH_BASE_ADDRESS=0 \
       -DCONFIG_FLASH_LOAD_OFFSET=0 \
       -DCONFIG_FLASH_SIZE=256 \
       -S alif-msc-ramdisk-ospi-sd

Build command (E8 DK):

.. code-block:: shell

   rm -rf build/
   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
       ../alif/samples/subsys/usb/mass/ \
       -DCONFIG_FLASH_BASE_ADDRESS=0 \
       -DCONFIG_FLASH_LOAD_OFFSET=0 \
       -DCONFIG_FLASH_SIZE=256 \
       -S alif-msc-ramdisk-ospi-sd

This will expose three drives on the host:

- **/RAM:** — 96 KiB RAM disk
- **/NAND:** — OSPI flash partition
- **/SD:** — SD card

Snippet: alif-msc-ramdisk-ospi-sd
==================================

The ``alif-msc-ramdisk-ospi-sd`` snippet applies board-specific device tree
overlays that enable all three storage devices for multi-partition testing:

- RAM disk (``ramdisk0``)
- OSPI flash partition (``ospi_storage_partition``)
- SD card (``sdhc`` / ``mmc``)

The snippet automatically selects the correct overlay for each board:

- ``alif_b1_dk`` → ``alif_b1_dk.overlay``
- ``alif_e1c_dk`` → ``alif_e1c_dk.overlay``
- ``alif_e7_dk`` → ``alif_e7_dk.overlay``
- ``alif_e8_dk`` → ``alif_e8_dk.overlay``


Sample Output
*************

After building and flashing, connect the board's USB port to a host PC.
The console output will look like this (multi-LUN example with FAT):

.. code-block:: none

   *** Booting Zephyr OS ***
   [00:00:01.100,000] <inf> main: Area 0 at 0x0 on ospi_flash@0 for 67108864 bytes
   [00:00:01.100,000] <inf> main: Mount /RAM:: 0
   [00:00:01.100,000] <inf> main: /RAM:: bsize = 512 ; frsize = 512 ; blocks = 158 ; bfree = 158
   [00:00:01.100,000] <inf> main: /RAM: opendir: 0
   [00:00:01.101,000] <inf> main: End of files
   [00:00:01.101,000] <inf> flashdisk: Initialize device NAND
   [00:00:01.101,000] <inf> flashdisk: offset 0, sector size 512, page size 4096, volume size 67108864
   [00:00:01.110,000] <inf> main: Mount /NAND:: 0
   [00:00:01.117,000] <inf> main: /NAND:: bsize = 512 ; frsize = 4096 ; blocks = 16371 ; bfree = 16312
   [00:00:01.117,000] <inf> main: /NAND: opendir: 0
   [00:00:01.117,000] <inf> main:   D 0 System Volume Information
   [00:00:01.117,000] <inf> main:   F 79027 CMSIS_driver_MSC.tdc
   [00:00:01.117,000] <inf> main:   F 45804 SDSIO_server_8kb.tdc
   [00:00:01.117,000] <inf> main:   F 47553 SDSIO_server_9kb.tdc
   [00:00:01.117,000] <inf> main:   F 47040 SDSIO_server_camera_with_usb.tdc
   [00:00:01.118,000] <inf> main: End of files
   [00:00:03.348,000] <inf> main: Mount /SD:: 0
   [00:00:22.646,000] <inf> main: /SD:: bsize = 512 ; frsize = 4096 ; blocks = 1946112 ; bfree = 1946024
   [00:00:22.646,000] <inf> main: /SD: opendir: 0
   [00:00:22.646,000] <inf> main:   D 0 System Volume Information
   [00:00:22.646,000] <inf> main:   F 5966 camera_with_usb.txt
   [00:00:22.646,000] <inf> main:   F 45804 SDSIO_server_8kb.tdc
   [00:00:22.646,000] <inf> main:   F 47553 SDSIO_server_9kb.tdc
   [00:00:22.647,000] <inf> main:   F 47040 SDSIO_server_camera_with_usb.tdc
   [00:00:22.647,000] <inf> main:   F 79027 CMSIS_driver_MSC.tdc
   [00:00:22.647,000] <inf> main:   F 104869 Chapter 9 Tests - USB 2 - 2025-09-11 10-48-57.html
   [00:00:22.647,000] <inf> main: End of files
   [00:00:22.753,000] <inf> main: The device is put in USB mass storage mode.
   [00:00:22.977,000] <inf> usbd_msc: Enable
   [00:00:22.977,000] <inf> usbd_msc: Bulk-Only Mass Storage Reset

On the host, the board will appear as three removable USB drives on
Linux/Windows, and you can perform write operations to each partition.

Wiping Flash Storage
====================

To erase the flash area before mounting (useful when the file system is
corrupted), enable:

.. code-block:: cfg

   CONFIG_APP_WIPE_STORAGE=y

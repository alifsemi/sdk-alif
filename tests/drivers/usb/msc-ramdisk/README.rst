Alif USB MSC RAMDisk Driver Tests
=================================

Overview
********

This test suite validates USB Mass Storage Class (MSC) with a RAM disk backend
using the Ztest framework. Application-specific hardware is enabled by
``app.overlay``, which the Zephyr build system applies for any supported board.

The RAM disk is 192 sectors of 512 bytes (96 KB) and is volatile: contents are
lost on reset or power cycle. No SD card or other external storage is required.

The tests live in a single ``msc_ramdisk_qa`` suite (6 tests). Shared setup
mounts the FAT filesystem at ``/RAM:`` and initializes the USB device stack.
``usbd_enable()`` is deferred until the USB tests so the host does not contend
for the disk during filesystem checks. After the suite finishes, USB MSC stays
enabled so the host can mount the drive.

**msc_ramdisk_qa suite (6 tests):**

- RAM disk mount and disk access: ``fs_statvfs``, sector count/size (192 x 512),
  ``disk_access`` init/status, raw read of sector 0
- USB init and descriptors: ``usbd_enable``, Full-Speed / High-Speed capability
  and bus speed, FS/HS configuration lists, PID ``0x0008``
- File CRUD: write, read-back, verify, delete
- Directory and LFN: mkdir, nested file, listing, long file name (when
  ``CONFIG_FS_FATFS_LFN`` is enabled)
- Data integrity: 2 KB file, 5 multi-file writes, 512-byte alignment
  boundaries (1, 511, 512, 513 bytes)
- Reconnect, overwrite, volatile, stress: storage still accessible with USB
  left enabled; overwrite; unmount/remount persistence within the session;
  5 file I/O stress cycles

``usbd_disable()`` is not exercised. DWC3 halt can time out after
``SET_CONFIGURATION(0)`` and drop the host MSC drive.

``app.overlay`` enables USB, ungates the USB PHY, and defines ``ramdisk0``
(``zephyr,ram-disk``, disk name ``RAM``, 192 x 512-byte sectors).

Prerequisites
*************

- USB cable between the board USB device port and a host PC.
- At least 96 KiB of available RAM for the RAM disk.
- No SD card or formatted media is required.
- After the automated suite, the board remains in USB MSC mode. The host
  should enumerate a removable drive (VID ``0x2fe3``, PID ``0x0008``).

Configuration
*************

Key Kconfig options in ``prj.conf``:

- ``CONFIG_USB_DEVICE_STACK_NEXT=y`` — next-gen USB device stack
- ``CONFIG_USBD_MSC_CLASS=y`` — USB Mass Storage class
- ``CONFIG_USBD_MSC_LUNS_PER_INSTANCE=1`` — single LUN (RAM disk)
- ``CONFIG_DISK_DRIVER_RAM=y`` — RAM disk backend
- ``CONFIG_FAT_FILESYSTEM_ELM=y`` — FAT filesystem
- ``CONFIG_FS_FATFS_LFN=y`` — long file names
- ``CONFIG_FS_FATFS_MOUNT_MKFS=y`` — format the RAM disk on first mount
- ``CONFIG_SAMPLE_USBD_PID=0x0008`` — USB product ID
- ``CONFIG_ZTEST=y`` — Zephyr test framework

The LUN disk name comes from ``USBD_DEFINE_MSC_LUN()`` and the ``disk-name``
property in ``app.overlay``, not from a Kconfig string.

Building and Running
********************

Board names are listed in the user guide. ``app.overlay`` is picked up
automatically.

.. code-block:: console

   west build -b <board> tests/drivers/usb/msc-ramdisk/

Flash and reset the board. Ztest runs on the serial console. After the suite,
connect the USB device port if it is not already connected; the host should
see a ~96 KB FAT volume.

Manual Host Checks
******************

These are not part of the automated suite:

1. Host file operations — confirm the drive appears, format as FAT if needed,
   copy files, and safe-eject.
2. USB cable hot-plug — disconnect and reconnect; the host should re-enumerate
   the MSC device.
3. Power-cycle volatile check — write files, reset the board, and confirm the
   data is gone (expected for RAM disk).

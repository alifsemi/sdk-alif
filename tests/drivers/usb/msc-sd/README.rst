Alif USB MSC SD Card Driver Tests
=================================

Overview
********

This test suite validates USB Mass Storage Class (MSC) with an SD card backend
using the Ztest framework. Application-specific hardware is enabled by
``app.overlay``, which the Zephyr build system applies for any supported board.

The SD card is non-volatile. Device-side tests mount FAT at ``/SD:`` and
exercise disk access, filesystem I/O, USB descriptors, overwrite, remount
persistence, regulator/SDHC init order, and file I/O stress.

The tests live in a single ``msc_sd_qa`` suite (8 tests). Shared setup mounts
the card and initializes the USB device stack. ``usbd_enable()`` is deferred
until the USB tests so the host MSC driver does not hit the same SD card while
filesystem tests run (stale DEPEVT / cancelled endpoints). Suite setup also
runs an SDHC write warm-up to absorb post-boot CMD24 error bursts. After the
suite finishes, USB MSC stays enabled so the host can mount the drive.

**msc_sd_qa suite (8 tests):**

- SD mount and disk info: ``fs_statvfs``, ``disk_access`` init/status,
  sector count/size, SDSC/SDHC/SDXC size class, raw read of sector 0
  (never writes sector 0)
- USB init and descriptors: ``usbd_enable``, Full-Speed / High-Speed
  capability and bus speed, PID ``0x0008``, write-protect check
- File CRUD: write, read-back, verify, delete
- Directory and LFN: mkdir, nested file, listing, long file name (when
  ``CONFIG_FS_FATFS_LFN`` is enabled)
- Data integrity and ADMA alignment: 4 KB file, 5 multi-file writes,
  512-byte alignment boundaries (1, 511, 512, 513, 1024 bytes)
- USB disconnect/reconnect: USB left enabled; storage I/O still works
- Overwrite and persistence: overwrite replaces content; unmount/remount
  keeps data (non-volatile)
- Regulator and stress: SDHC device ready, ``CONFIG_SDHC_INIT_PRIORITY >= 80``,
  5 file I/O stress cycles

``usbd_disable()`` is not exercised. DWC3 halt can time out after
``SET_CONFIGURATION(0)`` and drop the host MSC drive.

``app.overlay`` enables USB, ungates the USB PHY, enables ``gpio14`` and
``sdhc``, and sets the MMC disk name to ``SD``.

A complementary host-side pytest script under ``scripts/`` verifies the
enumerated USB drive from a Linux PC.

Prerequisites
*************

- USB cable between the board USB device port and a host PC.
- SD card in the board SD slot, formatted as **FAT32**, not write-protected.
- SDHC regulator/power sequencing must come up before the SDHC driver
  (``CONFIG_SDHC_INIT_PRIORITY=80``).
- After the automated suite, the board remains in USB MSC mode. The host
  should enumerate a removable drive (VID ``0x2fe3``, PID ``0x0008``).

Do not remove the SD card while automated tests are running.

Configuration
*************

Key Kconfig options in ``prj.conf``:

- ``CONFIG_USB_DEVICE_STACK_NEXT=y`` — next-gen USB device stack
- ``CONFIG_USBD_MSC_CLASS=y`` — USB Mass Storage class
- ``CONFIG_USBD_MSC_LUNS_PER_INSTANCE=1`` — single LUN (SD card)
- ``CONFIG_APP_MSC_STORAGE_SDCARD=y`` — select the SD card as the MSC storage
  backend (default in ``Kconfig``). Implies ``CONFIG_DISK_DRIVER_SDMMC``,
  ``CONFIG_FILE_SYSTEM``, and ``CONFIG_FAT_FILESYSTEM_ELM`` so the card is
  exposed as the single MSC LUN. The LUN disk name itself comes from
  ``USBD_DEFINE_MSC_LUN()`` and the ``disk-name`` property in ``app.overlay``.
- ``CONFIG_DISK_DRIVER_SDMMC=y`` — SDMMC disk driver
- ``CONFIG_FAT_FILESYSTEM_ELM=y`` — FAT filesystem
- ``CONFIG_FS_FATFS_LFN=y`` — long file names
- ``CONFIG_FS_FATFS_WINDOW_ALIGNMENT=512`` — FAT window alignment
- ``CONFIG_SDHC=y`` / ``CONFIG_SDHC_DWC=y`` / ``CONFIG_SDHC_DWC_ADMA=y``
- ``CONFIG_REGULATOR=y`` — SD card power
- ``CONFIG_SDHC_INIT_PRIORITY=80`` — SDHC after regulator, before USB
- ``CONFIG_SAMPLE_USBD_PID=0x0008`` — USB product ID
- ``CONFIG_ZTEST=y`` — Zephyr test framework

Building and Running
********************

Board names are listed in the user guide. ``app.overlay`` is picked up
automatically.

.. code-block:: console

   west build -b <board> tests/drivers/usb/msc-sd/

Flash and reset the board. Ztest runs on the serial console. After the suite,
connect the USB device port if it is not already connected; the host should
see the SD card as a removable FAT volume.

Host-Side Tests
***************

``scripts/test_usb_msc_sd_host.py`` is a pytest suite that runs on the Linux
host after the firmware is flashed and the MSC drive is enumerated.

Prerequisites on the host:

- ``pytest`` (``pip install pytest``)
- ``lsusb``, ``dmesg``, ``mount``, ``lsblk``
- Optional: ``sg_inq`` for SCSI inquiry

Identify the block device (for example ``/dev/sdb``), then:

.. code-block:: console

   sudo mkdir -p /mnt/usb_msc
   sudo pytest tests/drivers/usb/msc-sd/scripts/test_usb_msc_sd_host.py -v \
       --mount-point /mnt/usb_msc --block-dev /dev/sdb

Host coverage includes:

- VID:PID ``2fe3:0008`` detection and MSC interface class
- Mount / ``df`` / block device presence
- File create, read, delete, directories, LFN
- 10 MB file MD5 integrity and 20 small files
- Overwrite, unmount/remount persistence
- Write-protect off
- Stress: 50 x 100 KB files and concurrent copy + listing
- ADMA alignment sizes 1, 511, 512, 513, 1024 bytes
- ``lsblk`` card size

USB cable disconnect/reconnect and physical SD-card scenarios are skipped on
the host; they remain manual.

Manual Host Checks
******************

These are not part of the automated suite:

1. Boot without SD card — remove the card, reset, confirm no crash and that
   the USB stack still initializes.
2. SD card hot-removal while idle — remove the card; the host may report I/O
   errors; the board should not crash. Re-insert and reset.
3. Host file operations — confirm the drive appears, copy files, safe-eject.
4. Unformatted SD card — boot with an unformatted card without crash; format
   from the host if needed (``sudo mkfs.vfat /dev/sdX``).

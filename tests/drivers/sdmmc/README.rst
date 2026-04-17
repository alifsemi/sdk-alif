Alif SDMMC Driver Tests
=======================

Overview
********
This test suite validates the Alif SD/MMC storage interface using the Ztest
framework. It covers three layers of the storage stack and targets Alif boards
with SDHC controller support enabled via the ``sdmmc-test`` snippet.

Covered behaviors (SD card suite — 34 tests):

- SDHC card presence detection
- SDHC host controller properties (clock frequencies, bus width)
- disk initialization, status, sync, deinit/reinit
- sector count, sector size, erase block size queries
- raw single-sector and multi-sector read/write
- FAT filesystem mount/unmount
- file create, write, read, append, delete
- directory create and listing
- multifile integrity and remount persistence
- single-byte write, empty file read, seek beyond EOF
- open non-existent file, double mount, double init
- sector 0 (boot sector) and last sector boundary read/write
- large file (2 KB) write/read
- stress: repeated file write/read (25 iterations)
- stress: mount/unmount cycling (10 iterations)
- stress: health monitor append + seek + read (50 iterations)
- stress: raw sector write/read (100 iterations)
- stress: file create/delete cycling (20 iterations)
- stress: disk deinit/reinit cycling (5 iterations)

Supported Boards
****************

- ``alif_b1_dk/ab1c1f4m51820ph0/rtss_he``
- ``alif_e7_dk/ae722f80f55d5xx/rtss_hp``
- ``alif_e7_dk/ae722f80f55d5xx/rtss_he``
- ``alif_e8_dk/ae822fa0e5597xx0/rtss_hp``
- ``alif_e8_dk/ae822fa0e5597xx0/rtss_he``

Board-specific overlays are managed via the ``sdmmc-test`` snippet under:

- ``tests/drivers/sdmmc/snippets/sdmmc-test/``

Prerequisites
*************

- An SD card formatted as **FAT32** inserted in the board's SD slot.

Configuration
*************

Key Kconfig options in ``prj.conf``:

- ``CONFIG_SDHC=y`` — enable SDHC driver (low-level tests)
- ``CONFIG_SDMMC_TEST_DESTRUCTIVE_SECTOR_ZERO=y`` — enable the destructive
  raw sector 0 test (default n). WARNING: if the test aborts or power is lost,
  the SD card's FAT boot sector may be corrupted and require reformatting.

Building and Running
********************

 .. code-block:: console

    west build -b alif_e7_dk/ae722f80f55d5xx/rtss_hp tests/drivers/sdmmc/ -DSNIPPET=sdmmc-test
    west build -b alif_e7_dk/ae722f80f55d5xx/rtss_he tests/drivers/sdmmc/ -DSNIPPET=sdmmc-test
    west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp tests/drivers/sdmmc/ -DSNIPPET=sdmmc-test
    west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_he tests/drivers/sdmmc/ -DSNIPPET=sdmmc-test
    west build -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he tests/drivers/sdmmc/ -DSNIPPET=sdmmc-test

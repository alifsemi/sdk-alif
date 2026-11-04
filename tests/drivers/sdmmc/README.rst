Alif SDMMC Driver Tests
=======================

Overview
********
This test suite validates the Alif SD/MMC storage interface using the Ztest
framework. It covers three layers of the storage stack and targets Alif boards
with SDHC controller support enabled via the ``sdmmc-test`` snippet.

Covered behaviors (SD card suite — 41 tests):

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
- speed modes: host f_max class (Default/SDR12, High Speed/SDR25, UHS)
  and HS/SDR50/SDR104/DDR50/HS200/HS400 capability bits
- transfer types: 4-bit bus, ADMA2/SDMA, single-block (1 x 512 B),
  multi-block (8 x 512 B)
- performance: raw and FAT write/read of CONFIG_SDMMC_TEST_PERF_KIB
  (default 256 KiB) in 16 KiB chunks, plus single-block vs multi-block
  timing for 4 KiB

Do not switch the live card into a different UHS mode from the test
(that would drop the FAT mount). Caps and f_max report what the host
negotiated. Throughput numbers are informational; FAIL only on I/O error
or zero kbps.

Supported Boards
****************

Same testcode on every board. Only the snippet overlay changes.

+-----------------------------------------------+--------+--------+
| Board                                         | SD     | eMMC   |
+===============================================+========+========+
| ``alif_e8_dk/ae822fa0e5597xx0/rtss_he``       | yes    | yes    |
+-----------------------------------------------+--------+--------+
| ``alif_e8_dk/ae822fa0e5597xx0/rtss_hp``       | yes    | yes    |
+-----------------------------------------------+--------+--------+
| ``alif_e7_dk/ae722f80f55d5xx/rtss_he``        | yes    | yes    |
+-----------------------------------------------+--------+--------+
| ``alif_e7_dk/ae722f80f55d5xx/rtss_hp``        | yes    | yes    |
+-----------------------------------------------+--------+--------+
| ``alif_b1_dk/ab1c1f4m51820ph0/rtss_he``       | yes    | yes    |
+-----------------------------------------------+--------+--------+

Test Coverage by Layer
**********************

+-------------------+---------------------------------------------------+
| Test Layer        | Tests                                             |
+===================+===================================================+
| SDHC Driver       | Card presence detection, host properties          |
+-------------------+---------------------------------------------------+
| Disk Access       | Init, status, sync, deinit/reinit, sector queries |
+-------------------+---------------------------------------------------+
| Raw I/O           | Single/multi-sector read/write, sector boundaries |
+-------------------+---------------------------------------------------+
| Filesystem        | Mount/unmount, file ops, directory ops            |
+-------------------+---------------------------------------------------+
| Stress            | File I/O, mount cycling, health monitor,          |
|                   | raw sector cycling, file create/delete,           |
|                   | deinit/reinit cycling                             |
+-------------------+---------------------------------------------------+
| Speed modes       | Host f_max class, HS/UHS capability bits          |
+-------------------+---------------------------------------------------+
| Transfer types    | 4-bit bus, ADMA2/SDMA, single- vs multi-block     |
+-------------------+---------------------------------------------------+
| Performance       | Raw and FAT write/read kbps, single vs multi time |
+-------------------+---------------------------------------------------+

Board-specific overlays are managed via the ``sdmmc-test`` snippet under:

- ``tests/drivers/sdmmc/snippets/sdmmc-test/``

Prerequisites
*************

- An SD card formatted as **FAT32** inserted in the board's SD slot.

Configuration
*************

Key Kconfig options in ``prj.conf``:

- ``CONFIG_SDHC=y`` — enable SDHC driver (low-level tests)
- ``CONFIG_SDMMC_TEST_PERF_SECTORS=32`` — I/O chunk (8–64 sectors,
  default 32 = 16 KiB)
- ``CONFIG_SDMMC_TEST_PERF_KIB=256`` — timed transfer size (16–1024 KiB)
- ``CONFIG_SDMMC_TEST_DESTRUCTIVE_SECTOR_ZERO=y`` — enable the destructive
  raw sector 0 test (default n).

.. warning::
   ⚠️ WARNING: CONFIG_SDMMC_TEST_DESTRUCTIVE_SECTOR_ZERO writes to sector 0
   (FAT boot sector). If the test aborts or power is lost, the SD card may be
   left corrupted and require reformatting. Only enable this option for
   isolated destructive testing.

Building and Running
********************

Same test code, two snippets:

- ``-DSNIPPET=sdmmc-test`` — SD slot, disk ``SD``, mount ``/SD:``
- ``-DSNIPPET=emmc-test`` — on-board eMMC, disk ``SD2``, mount ``/SD2:``

eMMC needs a FAT partition (same as ``fs_sample -S alif-emmc``). CMD8
errors then legacy MMC are expected. UART should print
``Storage disk=SD2 mount=/SD2:``.

From ``zephyr/``. Replace ``-DSNIPPET=sdmmc-test`` with ``emmc-test`` for eMMC.

.. code-block:: console

   # E8 HE
   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
       ../alif/tests/drivers/sdmmc/ -DSNIPPET=sdmmc-test
   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
       ../alif/tests/drivers/sdmmc/ -DSNIPPET=emmc-test

   # E8 HP
   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
       ../alif/tests/drivers/sdmmc/ -DSNIPPET=sdmmc-test

   # E7 HE
   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
       ../alif/tests/drivers/sdmmc/ -DSNIPPET=sdmmc-test
   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
       ../alif/tests/drivers/sdmmc/ -DSNIPPET=emmc-test

   # E7 HP
   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
       ../alif/tests/drivers/sdmmc/ -DSNIPPET=sdmmc-test

   # B1 HE
   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
       ../alif/tests/drivers/sdmmc/ -DSNIPPET=sdmmc-test
   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
       ../alif/tests/drivers/sdmmc/ -DSNIPPET=emmc-test

The dedicated eMMC suite (``../alif/tests/drivers/emmc -S alif-emmc``)
is still valid on the same boards.

TestLink
********

Import into TestLink (keep SD and eMMC as separate suites — same case names):

- SD (``-DSNIPPET=sdmmc-test``): ``testlink/testlink_sdmmc.xml``
  (full 42-case ``sd_card`` suite on disk SD). Specs: ``testlink/TESTLINK_SPECS.rst``
- eMMC (``-DSNIPPET=emmc-test``): ``testlink/testlink_emmc.xml``
  (full 42-case ``sd_card`` suite on disk SD2). Specs: ``testlink/TESTLINK_SPECS_emmc.rst``

Older dedicated eMMC suite (``test_emmc_*`` names):
``alif/tests/drivers/emmc/testlink_emmc.xml``.

XIP HyperRAM Test Suite
###############################

Overview
********

One ztest for OSPI XIP memory. The shared cases in ``src/main.c``
(suite ``xip_tests``) are the common driver tests for APS512XXN PSRAM,
ISSI HyperRAM, and S80KS2564. All three use the ``spi-psram`` alias and
the memory-mapped window. Device-specific cases are selected in Kconfig:

* ``CONFIG_TEST_HYPERRAM=y`` — ``src/test_s80ks2564.c`` (S80KS2564). Default.
* ``CONFIG_TEST_PSRAM=y`` — ``src/test_hyperram.c`` (APS512XXN).
* ``CONFIG_TEST_ISSI_HYPERRAM=y`` — shared suite on ISSI IS66 (B1 dev kit).
* ``CONFIG_TEST_PSRAM_FLASH_COPY=y`` — ``src/test_flash_to_psram.c``.

There is no Zephyr ``flash_read`` / ``flash_write`` API on this MEMC driver.

The suite lives at::

    tests/drivers/flash/hyperram/

Test Structure
**************

* ``src/main.c`` — shared XIP access, patterns, boundaries, bursts, DDR,
  retention, and the optional full-array walk
* ``src/test_hyperram.c`` — APS512XXN identity and latency-code vs bus-speed
* ``src/test_s80ks2564.c`` — S80KS2564 datasheet cases (tCSM, RWDS, refresh).
  Frequency builds (40 / 100 / 200 MHz) are not separate test cases.
* ``src/test_flash_to_psram.c`` — 1 KiB OSPI flash to PSRAM window copy
* ``boards/alif_psram.overlay`` — APS512XXN on OSPI0
* ``boards/psram_*Mhz.overlay`` — PSRAM bus-speed and ``latency-code`` only
* ``boards/alif_hex_s80ks.overlay`` — S80KS2564 on OSPI1 (40 MHz, latency 7)
* ``boards/alif_b1_issi_hyperram.overlay`` — ISSI IS66 on B1 OSPI0 (80 MHz)
* ``boards/hyperram_*mhz.overlay`` — HyperRAM bus-speed and ``latency`` only

AppKit x16 pinctrl is in ``boards/alif_e8_ak_*.overlay`` and is picked up
automatically. Do not add an ``alif_e8_dk_*.overlay`` here: that name is
also automatic and would apply the S80KS pinmux to PSRAM builds.

Building and Running
********************

PSRAM (``CONFIG_TEST_PSRAM=y``; HyperRAM is the Kconfig default)::

   west build -p always -b <board> tests/drivers/flash/hyperram \
     -- -DDTC_OVERLAY_FILE="boards/alif_psram.overlay" \
        -DCONFIG_TEST_PSRAM=y

PSRAM clock overlay (appended, does not replace the base overlay)::

   west build -p always -b <board> tests/drivers/flash/hyperram \
     -- -DEXTRA_DTC_OVERLAY_FILE="boards/alif_psram.overlay;boards/psram_100Mhz.overlay" \
        -DCONFIG_TEST_PSRAM=y

* ``psram_50Mhz.overlay`` — 50 MHz, ``latency-code = 3``
* ``psram_100Mhz.overlay`` — 100 MHz, ``latency-code = 4``
* ``psram_200Mhz.overlay`` — 200 MHz, ``latency-code = 7``

HyperRAM is the default (``CONFIG_TEST_HYPERRAM=y``, E8 DevKit)::

   west build --pristine -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     tests/drivers/flash/hyperram \
     -- -DDTC_OVERLAY_FILE="boards/alif_hex_s80ks.overlay" \
        -DCONFIG_TEST_HYPERRAM=y

ISSI IS66 HyperRAM (B1 dev kit, shared suite only)::

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     tests/drivers/flash/hyperram \
     -- -DDTC_OVERLAY_FILE="boards/alif_b1_issi_hyperram.overlay" \
        -DCONFIG_TEST_ISSI_HYPERRAM=y

The S80KS functional cases run once, on ``boards/alif_hex_s80ks.overlay``.
``hyperram_100mhz.overlay`` and ``hyperram_200mhz.overlay`` are not
twister scenarios.

S80KS2564 CR0[7:4] vs clock (tACC 35 ns): latency 3 max 85 MHz, 4 max
104 MHz, 5 max 133 MHz, 6 max 166 MHz, 7 max 200 MHz. Do not use 200 MHz
with latency 3.

Full-array walk (long runtime). Either option registers only
``test_full_memory_rw_test``; the other XIP cases are left out::

   -DCONFIG_TEST_PSRAM_FULL_CHIP=y
   -DCONFIG_TEST_HYPERRAM_FULL_CHIP=y

Flash to PSRAM copy (E8 dev kit, separate controllers)::

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     tests/drivers/flash/hyperram -- \
     -DSNIPPET=ospi-flash \
     -DEXTRA_DTC_OVERLAY_FILE="boards/alif_psram.overlay" \
     -DCONFIG_TEST_PSRAM=y \
     -DCONFIG_TEST_PSRAM_FLASH_COPY=y \
     -DCONFIG_FLASH=y -DCONFIG_CRC=y

Twister scenarios are ``drivers.hyperram.xip*``, ``drivers.hyperram.s80ks``,
``drivers.hyperram.issi``, and ``drivers.hyperram.flash_copy``.

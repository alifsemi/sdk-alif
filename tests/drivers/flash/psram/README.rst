XIP PSRAM / HyperRAM Test Suite
###############################

Overview
********

One ztest for OSPI XIP memory. Shared cases are in ``src/main.c``
(suite ``xip_tests``). Device-specific cases are selected in Kconfig:

* ``CONFIG_TEST_PSRAM=y`` — ``src/test_psram.c`` (APS512XXN). Default.
* ``CONFIG_TEST_HYPERRAM=y`` — ``src/test_s80ks2564.c`` (S80KS2564).

There is no Zephyr ``flash_read`` / ``flash_write`` API on this MEMC driver.

The suite lives at::

    tests/drivers/flash/psram/

Test Structure
**************

* ``src/main.c`` — shared XIP access, patterns, boundaries, bursts, DDR,
  retention, and the optional full-array walk
* ``src/test_psram.c`` — APS512XXN identity and latency-code vs bus-speed
* ``src/test_s80ks2564.c`` — S80KS2564 datasheet cases (tCSM, RWDS, refresh)
* ``boards/alif_psram.overlay`` — APS512XXN on OSPI0
* ``boards/psram_*Mhz.overlay`` — PSRAM bus-speed and ``latency-code`` only
* ``boards/alif_hex_s80ks.overlay`` — S80KS2564 on OSPI1 (40 MHz, latency 7)
* ``boards/hyperram_*mhz.overlay`` — HyperRAM bus-speed and ``latency`` only

AppKit x16 pinctrl is in ``boards/alif_e8_ak_*.overlay`` and is picked up
automatically. Do not add an ``alif_e8_dk_*.overlay`` here: that name is
also automatic and would apply the S80KS pinmux to PSRAM builds.

Building and Running
********************

PSRAM (default ``CONFIG_TEST_PSRAM=y``)::

   west build -p always -b <board> tests/drivers/flash/psram \
     -- -DDTC_OVERLAY_FILE="boards/alif_psram.overlay"

PSRAM clock overlay (appended, does not replace the base overlay)::

   west build -p always -b <board> tests/drivers/flash/psram \
     -- -DEXTRA_DTC_OVERLAY_FILE="boards/alif_psram.overlay;boards/psram_100Mhz.overlay"

* ``psram_50Mhz.overlay`` — 50 MHz, ``latency-code = 3``
* ``psram_100Mhz.overlay`` — 100 MHz, ``latency-code = 4``
* ``psram_200Mhz.overlay`` — 200 MHz, ``latency-code = 7``

HyperRAM (E8 DevKit). ``CONFIG_TEST_HYPERRAM=y`` selects the S80KS cases
and clears the PSRAM choice::

   west build --pristine -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     tests/drivers/flash/psram \
     -- -DDTC_OVERLAY_FILE="boards/alif_hex_s80ks.overlay" \
        -DCONFIG_TEST_HYPERRAM=y

100 MHz / LC4 and 200 MHz / LC7 append the clock overlay::

   west build --pristine -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     tests/drivers/flash/psram \
     -- -DDTC_OVERLAY_FILE="boards/alif_hex_s80ks.overlay;boards/hyperram_100mhz.overlay" \
        -DCONFIG_TEST_HYPERRAM=y

S80KS2564 CR0[7:4] vs clock (tACC 35 ns): latency 3 max 85 MHz, 4 max
104 MHz, 5 max 133 MHz, 6 max 166 MHz, 7 max 200 MHz. Do not use 200 MHz
with latency 3.

Full-array walk (long runtime)::

   -DCONFIG_TEST_PSRAM_FULL_CHIP=y
   -DCONFIG_TEST_HYPERRAM_FULL_CHIP=y

Twister scenarios are ``drivers.psram.xip*`` and ``drivers.hyperram.xip*``.

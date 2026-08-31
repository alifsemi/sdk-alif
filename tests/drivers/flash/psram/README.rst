PSRAM XIP Test Suite
####################

Overview
********

This ztest verifies APS512XXN PSRAM in XIP mode over OSPI. The reference
sample is ``samples/drivers/spi_psram``: ``device_is_ready()`` plus
word read/write through the memory-mapped window. There is no Zephyr
``flash_read`` / ``flash_write`` API on this MEMC driver.

The suite lives at::

    tests/drivers/flash/psram/

Test Structure
**************

* ``src/main.c`` — XIP pointer access (byte / halfword / word), patterns,
  boundaries, wrap-order page walk, idle and active-load refresh,
  optional full-array walk matching the sample
* ``boards/alif_psram.overlay`` — same alias, pinctrl, and XIP window as
  the sample board overlays
* ``boards/psram_*Mhz.overlay`` — bus-speed and ``latency-code`` only;
  apply **in addition to** ``alif_psram.overlay``

Devicetree
**********

Required:

* Alias ``spi-psram``
* Parent ``xip-base-address`` (base, window length) and ``bus-speed``
* Child ``size``, ``latency-code``, optional ``x16-data-transfer-mode``

Building and Running
********************

The application builds when the overlay provides ``spi-psram``.

.. code-block:: console

   west build -p always -b <board> tests/drivers/flash/psram \
     -- -DDTC_OVERLAY_FILE="boards/alif_psram.overlay"

Clock overlay (appended, does not replace the base overlay)::

   west build -p always -b <board> tests/drivers/flash/psram \
     -- -DEXTRA_DTC_OVERLAY_FILE="boards/alif_psram.overlay;boards/psram_100Mhz.overlay"

Available clock overlays:

* ``psram_50Mhz.overlay`` — 50 MHz, ``latency-code = 3``
* ``psram_100Mhz.overlay`` — 100 MHz, ``latency-code = 4``
* ``psram_200Mhz.overlay`` — 200 MHz, ``latency-code = 7``

Full-array walk (same coverage as the sample; long runtime)::

   west build -p always -b <board> tests/drivers/flash/psram \
     -- -DDTC_OVERLAY_FILE="boards/alif_psram.overlay" \
        -DCONFIG_TEST_PSRAM_FULL_CHIP=y

Twister picks up ``testcase.yaml`` (base overlay, clock scenarios, and
an optional ``drivers.psram.xip.full_chip`` case).

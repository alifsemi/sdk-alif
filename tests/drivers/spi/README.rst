SPI Driver Tests
#####################

Overview
********

This test suite validates the SPI/LPSPI controller and Zephyr integration,
focusing on real bug detection rather than API compliance.

Test suites:

- **Loopback** -- internal/external loopback functional tests
- **Performance** -- frequency sweep and throughput measurement
- **Negative** -- driver error path validation
- **Boundary** -- FIFO limits, scatter-gather, edge cases
- **Stress** -- repeated transfer stress with repeated iterations

Building and Running
********************

All commands are run from the ``zephyr/`` directory (the repo's Zephyr base),
so the test path is ``tests/drivers/spi``.

Twister (recommended)
=====================

``ALIF_BOARD`` is auto-detected from the board name by ``CMakeLists.txt``,
so no extra flag is required for twister.  To override it manually:

.. code-block:: console

   west twister -T tests/drivers/spi --platform <board> \
               -- -DALIF_BOARD=ALIF_BOARD_E7_HP

Test scenarios (``-s`` filter):

- ``drivers.spi.alif.loopback``
- ``drivers.spi.alif.loopback.explicit``
- ``drivers.spi.alif.loopback.ext``
- ``drivers.spi.alif.perf``
- ``drivers.spi.alif.perf.dma``
- ``drivers.spi.alif.perf.dma.evtrtr``
- ``drivers.spi.alif.negative``
- ``drivers.spi.alif.boundary``
- ``drivers.spi.alif.boundary.ext``
- ``drivers.spi.alif.stress``
- ``drivers.spi.alif.stress.dma``
- ``drivers.spi.alif.stress.dma.evtrtr``
- ``drivers.spi.alif.lpspi.loopback``
- ``drivers.spi.alif.lpspi.perf``
- ``drivers.spi.alif.lpspi.perf.dma``
- ``drivers.spi.alif.lpspi.perf.dma.evtrtr``
- ``drivers.spi.alif.lpspi.stress``
- ``drivers.spi.alif.lpspi.stress.dma``
- ``drivers.spi.alif.lpspi.stress.dma.evtrtr``

Manual west build
=================

The default overlay (``boards/spi.overlay``) enables SPI1/SPI0 loopback tests.
Other topologies and options are selected with ``-DDTC_OVERLAY_FILE`` and
``-DCONFIG_*`` flags.

Loopback tests (SPI1/SPI0)
--------------------------

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
              tests/drivers/spi \
              -- -DDTC_OVERLAY_FILE=boards/spi.overlay \
                 -DCONFIG_TEST_SPI_LOOPBACK=y

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
              tests/drivers/spi \
              -- -DDTC_OVERLAY_FILE=boards/spi.overlay \
                 -DCONFIG_TEST_SPI_LOOPBACK=y

Performance tests
-----------------

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
              tests/drivers/spi \
              -- -DDTC_OVERLAY_FILE=boards/spi.overlay \
                 -DCONFIG_TEST_SPI_PERFORMANCE=y

Performance with DMA
--------------------

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
              tests/drivers/spi \
              -- "-DDTC_OVERLAY_FILE=boards/spi.overlay;boards/spi_DMA.overlay" \
                 -DCONFIG_TEST_SPI_PERFORMANCE=y \
                 -DCONFIG_TEST_SPI_USE_DMA=y

Negative tests
--------------

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
              tests/drivers/spi \
              -- -DDTC_OVERLAY_FILE=boards/spi.overlay \
                 -DCONFIG_TEST_SPI_NEGATIVE=y

Boundary tests
--------------

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
              tests/drivers/spi \
              -- -DDTC_OVERLAY_FILE=boards/spi.overlay \
                 -DCONFIG_TEST_SPI_BOUNDARY=y

Stress tests
------------

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
              tests/drivers/spi \
              -- -DDTC_OVERLAY_FILE=boards/spi.overlay \
                 -DCONFIG_TEST_SPI_STRESS=y

LPSPI tests
-----------

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
              tests/drivers/spi \
              -- -DDTC_OVERLAY_FILE=boards/lpspi.overlay

LPSPI with DMA
--------------

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
              tests/drivers/spi \
              -- "-DDTC_OVERLAY_FILE=boards/lpspi.overlay;boards/lpspi_DMA.overlay" \
                 -DCONFIG_TEST_SPI_PERFORMANCE=y \
                 -DCONFIG_TEST_SPI_USE_DMA=y

Console Output
==============

After flashing, open the console to view test progress:

.. code-block:: console

   west flash

Kconfig Options
***************

Top-level suite selection:

- ``CONFIG_TEST_SPI_LOOPBACK``       Loopback functional tests
- ``CONFIG_TEST_SPI_NEGATIVE``       Driver error path tests
- ``CONFIG_TEST_SPI_BOUNDARY``       Boundary condition tests
- ``CONFIG_TEST_SPI_STRESS``         Stress tests
- ``CONFIG_TEST_SPI_PERFORMANCE``    Performance tests
- ``CONFIG_TEST_SPI_USE_DMA``        Enable DMA for SPI transfers

Test tuning:

- ``CONFIG_TEST_SPI_FREQUENCY``      Base SPI frequency in Hz (default 1 MHz)
- ``CONFIG_TEST_SPI_WORD_SIZE``      Pinned word size for all suites (0 = sweep 8/16/32)
- ``CONFIG_TEST_SPI_INTERNAL_LOOPBACK`` Use internal loopback mode

Setting Kconfig options
=====================

**Via ``prj.conf`` (project default):**

.. code-block:: kconfig

   CONFIG_TEST_SPI_FREQUENCY=2500000
   CONFIG_TEST_SPI_WORD_SIZE=16

**Via west build command line (one-shot):**

.. code-block:: console

   west build -- -DCONFIG_TEST_SPI_FREQUENCY=2500000 -DCONFIG_TEST_SPI_WORD_SIZE=16

**Via Twister ``extra_configs`` in ``testcase.yaml``:**

.. code-block:: yaml

   extra_configs:
     - CONFIG_TEST_SPI_FREQUENCY=2500000
     - CONFIG_TEST_SPI_WORD_SIZE=16

The frequency value is the raw Hz value (e.g. ``1000000`` = 1 MHz).
Word size values: ``0`` sweeps 8/16/32 bits, ``8`` / ``16`` / ``32`` pins to that width.

Devicetree Overlays
*******************

- ``boards/spi.overlay``              SPI1 master / SPI0 slave
- ``boards/spi2.overlay``             SPI2 master / SPI0 slave
- ``boards/spi3.overlay``             SPI3 master / SPI0 slave
- ``boards/lpspi.overlay``            LPSPI0 master / SPI0 slave
- ``boards/spi_ss_sw.overlay``        Software chip-select (SPI)
- ``boards/lpspi_ss_sw.overlay``      Software chip-select (LPSPI)
- ``boards/spi_DMA.overlay``          DMA option (SPI)
- ``boards/spi_DMA_evtrtr.overlay``   DMA with event router (SPI)
- ``boards/lpspi_DMA.overlay``        DMA option (LPSPI)
- ``boards/lpspi_DMA_evtrtr.overlay``   DMA with event router (LPSPI)

Best practice: stack a base overlay with DMA/GPIO overlays via
``-DDTC_OVERLAY_FILE``.

Hardware Requirements
*********************

Supported boards
================

- ``alif_e7_dk/ae722f80f55d5xx/rtss_hp``
- ``alif_e7_dk/ae722f80f55d5xx/rtss_he``
- ``alif_e8_dk/ae822fa0e5597xx0/rtss_hp``
- ``alif_e8_dk/ae822fa0e5597xx0/rtss_he``
- ``alif_e1c_dk/ae1c1f4051920hh/rtss_he``
- ``alif_b1_dk/ab1c1f4m51820hh0/rtss_he``

Loopback / Boundary tests
=========================

These suites support two modes, selectable via Kconfig.

Internal loopback (default):

- ``CONFIG_TEST_SPI_INTERNAL_LOOPBACK=y``
- Single controller, ``SPI_MODE_LOOP`` enabled inside the IP.
- No external wiring required.

External loopback:

- ``CONFIG_TEST_SPI_INTERNAL_LOOPBACK=n``
- Two controllers (SPI1 as controller, SPI0 as target) wired together.
- Same scenarios run without ``SPI_MODE_LOOP``, validating both MOSI
  and MISO directions across two independent transceives.

Pinned word size:

- ``CONFIG_TEST_SPI_WORD_SIZE=N`` (range 4..32) pins the loopback /
  boundary scenarios to a single data width for targeted debugging.
- ``CONFIG_TEST_SPI_WORD_SIZE=0`` (default) sweeps 8/16/32-bit.

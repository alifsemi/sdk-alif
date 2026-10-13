.. _i2s_ztest:

I2S Driver Tests
################

Overview
========

This test suite validates the I2S controller driver on Alif Semiconductor
platforms. The tests use the Zephyr ZTest framework and cover
configuration, state machine, negative paths, stress, performance,
golden-vector TX, audio playback, and a full loopback matrix across bit
depths and sample rates.

Test Suites
===========

- **i2s_alif_functional** (``src/functional/test_i2s_loopback.c``):
  loopback matrix, one case per bit depth (12, 16, 20, 24, 32), each
  walking 8, 16, 32, 44.1, 48, 88.2, 96, and 192 kHz.
- **i2s_features** (``src/functional/test_i2s_features.c``): sample-rate,
  word-select, resolution, FIFO, DMA handshake, full-duplex, and
  clock-slave forwarding cases. The clock-slave cases run only when
  ``CONFIG_I2S_SLAVE_MODE_SUPPORTED=y``.
- **i2s_functional** (``src/functional/test_i2s_playback.c``):
  ``test_PlayHelloSamples48k_TxOnly_10s`` when
  ``CONFIG_I2S_PLAY_HELLO=y``, and
  ``test_PlayHelloSamples48k_Codec_10s`` when
  ``CONFIG_I2S_PLAY_HELLO_CODEC=y``. The other case skips.
- **i2s_golden_tx** (``src/golden/test_i2s_golden_tx.c``): TX-master
  golden vectors at the same five bit depths and eight rates. RX is
  checked only when ``CONFIG_I2S_LOOPBACK_VERIFY=y``.
- **i2s_negative** (``src/negative/test_i2s_negative.c``): state machine,
  underrun, overrun, queue limits, and full-duplex mismatch. The TX half
  of ``neg_stop_and_drain`` runs only when the loopback wire is declared.
- **i2s_stress** (``src/stress/test_i2s_stress.c``): repeated start/drop,
  start/stop, and start/drain, underrun and overrun recovery, a 30 s
  192 kHz transmit, queue fill/drain, and a 10 s full-duplex run.
  Off unless ``CONFIG_I2S_STRESS_TESTS=y``. Full duplex skips without
  the loopback wire.
- **i2s_performance** (``src/performance/test_i2s_performance.c``): block
  period, throughput, datasheet rates at 16-bit and 32-bit, startup
  latency, and underrun margin. Off unless ``CONFIG_I2S_PERF_TESTS=y``.
  Startup latency skips without the loopback wire.
- **i2s_config** (``src/config/test_i2s_config.c``): configure matrix for
  sample rate and word size, word-select cycles, FIFO threshold, and
  DMA handshake bits.

Key Files
=========

- ``src/functional/test_i2s_loopback.c``: ``i2s_alif_functional``.
- ``src/functional/test_i2s_features.c``: ``i2s_features``.
- ``src/functional/test_i2s_playback.c``: ``i2s_functional``.
- ``src/golden/test_i2s_golden_tx.c``: ``i2s_golden_tx``.
- ``src/negative/test_i2s_negative.c``: ``i2s_negative``.
- ``src/stress/test_i2s_stress.c``: ``i2s_stress``.
- ``src/performance/test_i2s_performance.c``: ``i2s_performance``.
- ``src/config/test_i2s_config.c``: ``i2s_config``.
- ``src/common/i2s_test_common.c``: Shared helpers (golden run, slabs,
  stream reset, blocked-write feed).
- ``src/common/i2s_golden_vectors.c``: Pre-computed reference vectors.
  Linked for loopback, golden, and negative builds.
- ``snippets/i2s/``: FIFO interrupt path, applied via ``-S i2s``
  (or ``SNIPPET=i2s`` under twister). ``alif_lpi2s_rtss_he.overlay``
  aliases ``i2s-node0`` to ``i2s4`` on RTSS-HE. ``alif_i2s2_rtss_hp.overlay``
  selects I2S2 on E7 RTSS-HP. ``alif_i2s3_rtss_hp.overlay`` selects I2S3
  on E8 RTSS-HP.
- ``snippets/i2s-dma/``: same controller as ``snippets/i2s``, plus
  ``rxdma`` / ``txdma``. Applied via ``-S i2s-dma``. Use it for the
  loopback and golden suites. Negative, stress, and performance stay on
  ``-S i2s``. Overlays: ``alif_e7_e8_rtss_he_dma.overlay`` (group 0 on
  ``evtrtr2``), ``alif_b1_rtss_he_dma.overlay`` (B1 and E1C),
  ``alif_e7_rtss_hp_dma.overlay``, and ``alif_e8_rtss_hp_dma.overlay``.
  ``i2s_dma.conf`` sets ``CONFIG_DMA``, ``CONFIG_I2S_DW_USE_DMA``, and
  ``CONFIG_NOCACHE_MEMORY``. Pick ``i2s`` or ``i2s-dma``. There is no
  ``boards/`` directory.
- ``snippets/i2s-codec/``: WM8904 playback. ``e1c_b1_rtss.overlay`` uses
  I2S0, ``e4_e8_rtss_ak.overlay`` uses I2S2, and
  ``e4_e8_rtss_dk.overlay`` uses I2S3. Each enables ``i2c1`` and
  ``wm8904@1a``, and aliases ``i2s-codec-tx``. Applied via ``-S i2s-codec``.
- ``testcase.yaml``: Twister test definitions (all entries use
  ``harness: ztest``).

Building and Running
====================

The ``i2s`` snippet selects the FIFO interrupt overlay for each board.
The ``i2s-dma`` snippet selects the same controller and attaches DMA.

Build and run the default test set (interrupt). Functional, golden,
negative, and config are on by default, so this image contains all four.
Stress and performance stay out until their switches are set:

.. code-block:: console

   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S i2s \
       tests/drivers/i2s
   west flash

Build the loopback matrix on the DMA path (requires the wire below):

.. code-block:: console

   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S i2s-dma \
       tests/drivers/i2s \
       -- -DCONFIG_I2S_FUNCTIONAL_TESTS=y -DCONFIG_I2S_LOOPBACK=y \
          -DCONFIG_I2S_GPIO_LOOPBACK=y
   west flash

Build the golden-vector suite on the DMA path (TX only, no wire):

.. code-block:: console

   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S i2s-dma \
       tests/drivers/i2s \
       -- -DCONFIG_I2S_FUNCTIONAL_TESTS=n -DCONFIG_I2S_GOLDEN_TESTS=y \
          -DCONFIG_I2S_NEGATIVE_TESTS=n -DCONFIG_I2S_CONFIG_TESTS=n
   west flash

Build with the loopback matrix only (requires the hardware wire below):

.. code-block:: console

   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S i2s \
       tests/drivers/i2s \
       -- -DCONFIG_I2S_GPIO_LOOPBACK=y -DCONFIG_I2S_LOOPBACK=y

Build the golden-vector suite with RX verification over the loopback
wire. ``CONFIG_I2S_FUNCTIONAL_TESTS=n`` is required here: with functional
left at its default and no functional selector set, CMake also compiles
the loopback, feature, playback, negative, and config sources.

.. code-block:: console

   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S i2s \
       tests/drivers/i2s \
       -- -DCONFIG_I2S_FUNCTIONAL_TESTS=n \
          -DCONFIG_I2S_GOLDEN_TESTS=y \
          -DCONFIG_I2S_NEGATIVE_TESTS=n \
          -DCONFIG_I2S_CONFIG_TESTS=n \
          -DCONFIG_I2S_GPIO_LOOPBACK=y \
          -DCONFIG_I2S_LOOPBACK_VERIFY=y

Supported boards (overlay supplied by ``snippets/i2s/snippet.yml``
and ``snippets/i2s-dma/snippet.yml``):

- ``alif_e7_dk/ae722f80f55d5xx/rtss_he``
- ``alif_e7_dk/ae722f80f55d5xx/rtss_hp``
- ``alif_e8_dk/ae822fa0e5597xx0/rtss_he``
- ``alif_e8_dk/ae822fa0e5597xx0/rtss_hp``
- ``alif_b1_dk/ab1c1f4m51820ph0/rtss_he``

Negative, stress, and performance together, on the FIFO path, with the
loopback wire:

.. code-block:: console

   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S i2s \
       tests/drivers/i2s \
       -- -DCONFIG_I2S_FUNCTIONAL_TESTS=n \
          -DCONFIG_I2S_GOLDEN_TESTS=n \
          -DCONFIG_I2S_CONFIG_TESTS=n \
          -DCONFIG_I2S_NEGATIVE_TESTS=y \
          -DCONFIG_I2S_STRESS_TESTS=y \
          -DCONFIG_I2S_PERF_TESTS=y \
          -DCONFIG_I2S_GPIO_LOOPBACK=y

Codec playback (WM8904 on I2C1; the ``i2s`` snippet is not used):

.. code-block:: console

   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S i2s-codec \
       tests/drivers/i2s \
       -- -DCONFIG_I2S_FUNCTIONAL_TESTS=y \
          -DCONFIG_I2S_PLAY_HELLO_CODEC=y

Test Suite Selection
====================

``CONFIG_I2S_FUNCTIONAL_TESTS``, ``CONFIG_I2S_GOLDEN_TESTS``,
``CONFIG_I2S_NEGATIVE_TESTS``, and ``CONFIG_I2S_CONFIG_TESTS`` default to
``y``. ``CONFIG_I2S_STRESS_TESTS`` and ``CONFIG_I2S_PERF_TESTS`` default
to ``n``.

A functional selector below sets the image to that one functional file
and drops golden, negative, stress, performance, and config:

- ``CONFIG_I2S_LOOPBACK=y``: only ``test_i2s_loopback.c``.
- ``CONFIG_I2S_FEATURES=y``: only ``test_i2s_features.c``.
- ``CONFIG_I2S_PLAY_HELLO=y``: only ``test_i2s_playback.c`` (TX-only case).
- ``CONFIG_I2S_PLAY_HELLO_CODEC=y``: only ``test_i2s_playback.c`` (codec
  case). Selects ``I2C`` and ``WM8904``. Build with ``-S i2s-codec``.
- ``CONFIG_I2S_FULL_DUPLEX=y``: only ``test_full_duplex_same_device`` from
  ``test_i2s_features.c``.

With functional left on and none of those selectors set, CMake compiles
``test_i2s_loopback.c``, ``test_i2s_features.c``, and
``test_i2s_playback.c`` together, and still compiles golden, negative,
and config when their switches are ``y``.

Other flags:

- ``CONFIG_I2S_GPIO_LOOPBACK=y``: the SDO-to-SDI wire is fitted. The
  loopback matrix, golden RX verify, startup latency, and the 10 s
  stress full-duplex run execute. Without it those cases skip, except
  golden RX verify, which is left out of the build because
  ``CONFIG_I2S_LOOPBACK_VERIFY`` depends on this flag.
- ``CONFIG_I2S_LOOPBACK_VERIFY=y``: ``i2s_golden_run()`` also configures
  RX and compares received blocks with the transmitted golden vectors.
  Depends on ``CONFIG_I2S_GOLDEN_TESTS`` and ``CONFIG_I2S_GPIO_LOOPBACK``.
  The driver does not implement ``I2S_OPT_LOOPBACK``, so the wire is
  required.
- ``CONFIG_I2S_SLAVE_MODE_SUPPORTED=y``: the controller can take BCLK and
  WS from an external master. ``test_mic_rx_to_tx_bit_exact_forwarding``
  and ``test_mic_rx_to_tx_bit_exact_matrix`` skip when this is ``n``.

Hardware Requirements
=====================

The loopback tests require a physical wire from SDO to SDI. ``-S i2s``
selects the controller, and the same pins are used for ``-S i2s-dma``:

- E7/E8 DK RTSS-HE (LPI2S): ``P13_5`` (LPI2S_SDO) -> ``P13_4`` (LPI2S_SDI)
- E1C and B1 DK RTSS-HE (LPI2S): ``P0_1`` (LPI2S_SDO) -> ``P0_0`` (LPI2S_SDI)
- E7 DK RTSS-HP (I2S2): ``P8_2`` (I2S2_SDO) -> ``P8_1`` (I2S2_SDI)
- E8 DK RTSS-HP (I2S3): ``P9_3`` (I2S3_SDO) -> ``P9_0`` (I2S3_SDI)

Without the wire, leave ``CONFIG_I2S_GPIO_LOOPBACK`` off. The loopback
matrix, startup latency, and the 10 s full-duplex stress test then skip.
``neg_stop_and_drain`` still runs its RX STOP/DRAIN half and returns
before the TX half. On RTSS-HE that case is
``test_lpi2s_neg_stop_and_drain``; on RTSS-HP it is
``test_i2s_neg_stop_and_drain``.

Configuration Options
=====================

Key options in ``prj.conf``:

- ``CONFIG_I2S=y``: Enable the I2S driver.
- ``CONFIG_ZTEST=y``: Enable the ZTest framework.
- ``CONFIG_LOG=y``: Enable logging.
- ``CONFIG_HEAP_MEM_POOL_SIZE=81920``: Heap used by the test slabs.

``-S i2s`` keeps the FIFO interrupt path, so ``CONFIG_DMA`` stays off.
``-S i2s-dma`` sets ``CONFIG_DMA``, ``CONFIG_I2S_DW_USE_DMA``, and
``CONFIG_NOCACHE_MEMORY``, and adds ``dmas`` / ``dma-names``
(``rxdma``, ``txdma``) on that board's I2S node. Use it for the
loopback and golden-vector suites. Negative, stress, and performance
stay on ``-S i2s``. The loopback buffer and the golden slabs are placed
in the nocache section because the driver does not
maintain the cache around DMA.

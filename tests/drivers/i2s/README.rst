.. _i2s_ztest:

I2S Driver Tests
################

Overview
========

This test suite validates the I2S controller driver on Alif Semiconductor
platforms. The tests use the Zephyr ZTest framework and cover
configuration, state machine, negative paths, golden-vector TX, audio
playback, and a full loopback matrix across bit depths and sample rates.

Test Suites
===========

- **functional**: Loopback matrix, full-duplex, feature validation, and
  audio playback on the same device.
- **golden**: TX-master golden vector patterns across multiple rates and
  bit depths; optionally verified against RX when a loopback wire is
  connected.
- **negative**: State-machine negative cases and invalid-parameter
  handling (also contains the state-machine helper sources).
- **config**: Register / configuration matrix validation (sampling
  frequency, word size, word-select cycles, FIFO depth, DMA handshake
  bits, etc.).

Key Files
=========

- ``src/functional/test_i2s_loopback.c``: Loopback matrix suite
  (``test_lb_matrix_*``).
- ``src/functional/test_i2s_features.c``: Feature suite (full-duplex,
  TX/RX, sample-rate sweeps).
- ``src/functional/test_i2s_playback.c``: ``hello_samples`` TX-only
  playback test.
- ``src/golden/test_i2s_golden_tx.c``: Golden-vector TX suite.
- ``src/negative/test_i2s_negative.c``: Negative state-machine suite.
- ``src/config/test_i2s_config.c``: Configuration matrix suite.
- ``src/common/i2s_test_common.c``: Shared helpers (golden run, slabs).
- ``src/common/i2s_golden_vectors.c``: Pre-computed reference vectors.
- ``snippets/i2s/``: FIFO interrupt path, applied via ``-S i2s``
  (or ``SNIPPET=i2s`` under twister).
- ``snippets/i2s-dma/``: DMA path for the loopback and golden-vector
  suites, applied via ``-S i2s-dma``. ``snippet.yml`` picks the overlay:
  ``alif_e7_e8_rtss_he_dma.overlay`` (group 0 on ``evtrtr2``),
  ``alif_b1_rtss_he_dma.overlay`` (B1 and E1C),
  ``alif_e7_rtss_hp_dma.overlay``, and ``alif_e8_rtss_hp_dma.overlay``.
  Pick ``i2s`` or ``i2s-dma``. There is no ``boards/`` directory.
- ``testcase.yaml``: Twister test definitions (all entries use
  ``harness: ztest``).

Building and Running
====================

The ``i2s`` snippet selects the FIFO interrupt overlay for each board.
The ``i2s-dma`` snippet selects the same controller and attaches DMA.

Build and run the default test set (interrupt):

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
wire:

.. code-block:: console

   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S i2s \
       tests/drivers/i2s \
       -- -DCONFIG_I2S_GOLDEN_TESTS=y \
          -DCONFIG_I2S_GPIO_LOOPBACK=y \
          -DCONFIG_I2S_LOOPBACK_VERIFY=y

Supported boards (overlay supplied by ``snippets/i2s/snippet.yml``
and ``snippets/i2s-dma/snippet.yml``):

- ``alif_e7_dk/ae722f80f55d5xx/rtss_he``
- ``alif_e7_dk/ae722f80f55d5xx/rtss_hp``
- ``alif_e8_dk/ae822fa0e5597xx0/rtss_he``
- ``alif_e8_dk/ae822fa0e5597xx0/rtss_hp``
- ``alif_b1_dk/ab1c1f4m51820ph0/rtss_he``

Test Suite Selection
====================

Enable suites via Kconfig:

- ``CONFIG_I2S_FUNCTIONAL_TESTS=y``: Functional suite.
- ``CONFIG_I2S_GOLDEN_TESTS=y``: Golden-vector TX suite.
- ``CONFIG_I2S_NEGATIVE_TESTS=y``: Negative and state-machine suite.
- ``CONFIG_I2S_CONFIG_TESTS=y``: Configuration matrix suite.

Functional sub-suite selectors (mutually exclusive; each compiles only the named suite):

- ``CONFIG_I2S_LOOPBACK=y``: Compile only the loopback matrix.
- ``CONFIG_I2S_FEATURES=y``: Compile only the feature suite.
- ``CONFIG_I2S_PLAY_HELLO=y``: Compile only the TX-only hello playback test.
- ``CONFIG_I2S_PLAY_HELLO_CODEC=y``: Compile only the codec hello playback test.
- ``CONFIG_I2S_FULL_DUPLEX=y``: Compile only the full-duplex test.

Loopback per-bit-depth switches (for single-rate golden loopback tests):

- ``CONFIG_I2S_LB_12BIT=y``
- ``CONFIG_I2S_LB_16BIT=y``
- ``CONFIG_I2S_LB_20BIT=y``
- ``CONFIG_I2S_LB_24BIT=y``
- ``CONFIG_I2S_LB_32BIT=y``

Other feature flags:

- ``CONFIG_I2S_GPIO_LOOPBACK=y``: Declare that the board has the
  loopback wire installed (required for all loopback tests to actually
  execute).
- ``CONFIG_I2S_LOOPBACK_VERIFY=y``: Enable RX-side verification inside
  ``i2s_golden_run()`` (depends on the loopback wire).
- ``CONFIG_I2S_PLAY_HELLO=y``: Enable the TX-only playback test.
- ``CONFIG_I2S_SLAVE_MODE_SUPPORTED=y``: Declare that the I2S hardware
  supports clock-slave mode (controls skip behaviour for slave-mode
  tests).

Hardware Requirements
=====================

The loopback tests require a physical wire between SDO and SDI. The
exact pins depend on the board; see the header of each overlay in
``snippets/i2s/`` for the pin pair used by that target. Examples:

- E8 DK RTSS-HP: ``P9_3`` (I2S3_SDO) -> ``P9_0`` (I2S3_SDI)
- E7/E8 DK RTSS-HE (LPI2S): ``P13_5`` (LPI2S_SDO) -> ``P13_4`` (LPI2S_SDI)
- E7 DK RTSS-HP: ``P8_2`` (I2S2_SDO) -> ``P8_1`` (I2S2_SDI)
- B1 DK RTSS-HE: ``P0_1`` (I2S0_SDO) -> ``P0_0`` (I2S0_SDI)

Without the wire, loopback and RX-verify tests self-skip at runtime.

Configuration Options
=====================

Key options in ``prj.conf``:

- ``CONFIG_I2S=y``: Enable the I2S driver.
- ``CONFIG_ZTEST=y``: Enable the ZTest framework.

``-S i2s`` keeps the FIFO interrupt path, so ``CONFIG_DMA`` stays off.
``-S i2s-dma`` sets ``CONFIG_DMA``, ``CONFIG_I2S_DW_USE_DMA``, and
``CONFIG_NOCACHE_MEMORY``, and adds ``dmas`` / ``dma-names``
(``rxdma``, ``txdma``) on that board's I2S node. Use it for the
loopback and golden-vector suites. The loopback buffer and the golden
slabs are placed in the nocache section because the driver does not
maintain the cache around DMA.

Running under Twister
=====================

The application ships a ``testcase.yaml`` so it can be discovered by
Zephyr's ``twister`` runner. Every scenario uses ``harness: ztest``
-- twister flashes the device and parses the on-target ZTest output
directly (PASS/FAIL/PROJECT EXECUTION lines). No external pytest
scripts are involved.

Defined scenarios:

- ``drivers.i2s.alif.smoke`` -- default suite mix on the FIFO path; no HW required.
- ``drivers.i2s.alif.features`` -- features suite only.
- ``drivers.i2s.alif.loopback`` -- loopback matrix. Gated on ``gpio_loopback``.
- ``drivers.i2s.alif.loopback_dma`` -- same matrix with ``SNIPPET=i2s-dma``.
  Gated on ``gpio_loopback``.
- ``drivers.i2s.alif.golden_verify`` -- golden-vector TX with RX verification.
  Gated on ``gpio_loopback``.
- ``drivers.i2s.alif.golden_dma`` -- golden-vector TX on the DMA path.
  No wire required.
- ``drivers.i2s.alif.play_hello`` -- TX-only playback. Gated on ``audio_codec``.
- ``drivers.i2s.alif.play_hello_codec`` -- codec playback. Gated on ``audio_codec``.

Fixture-gated scenarios are auto-skipped by twister when the
corresponding ``--fixture <name>`` is not declared on the command line.

List all defined tests:

.. code-block:: console

   twister -T tests/drivers/i2s --list-tests

CI smoke build (no HW wire required, all five supported boards):

.. code-block:: console

   twister -T tests/drivers/i2s -s drivers.i2s.alif.smoke --build-only

Loopback matrix on E8 RTSS-HP with the SDO->SDI wire installed:

.. code-block:: console

   twister -T tests/drivers/i2s \
       -p alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
       -s drivers.i2s.alif.loopback \
       --device-testing --device-serial /dev/ttyACM0 \
       --fixture gpio_loopback

Golden-vector RX verification (also requires the loopback wire):

.. code-block:: console

   twister -T tests/drivers/i2s \
       -p alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
       -s drivers.i2s.alif.golden_verify \
       --device-testing --device-serial /dev/ttyACM0 \
       --fixture gpio_loopback

DMA loopback on E8 RTSS-HE with the SDO->SDI wire installed:

.. code-block:: console

   twister -T tests/drivers/i2s \
       -p alif_e8_dk/ae822fa0e5597xx0/rtss_he \
       -s drivers.i2s.alif.loopback_dma \
       --device-testing --device-serial /dev/ttyACM0 \
       --fixture gpio_loopback

DMA golden-vector TX (no wire):

.. code-block:: console

   twister -T tests/drivers/i2s \
       -p alif_e8_dk/ae822fa0e5597xx0/rtss_he \
       -s drivers.i2s.alif.golden_dma \
       --device-testing --device-serial /dev/ttyACM0

The ``i2s`` snippet is applied automatically via ``extra_args:
SNIPPET=i2s`` in ``testcase.yaml`` -- you do not need to pass
``-S i2s`` again on the twister command line. The DMA scenarios pass
``SNIPPET=i2s-dma`` themselves.

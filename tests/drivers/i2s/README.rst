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
- ``snippets/i2s/``: Devicetree overlays applied via ``-S i2s``.
- ``boards/``: Per-board devicetree overlays.

Building and Running
====================

The ``i2s`` snippet selects the correct overlay for each board
automatically.

Build and run the default test set:

.. code-block:: console

   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S i2s \
       tests/drivers/i2s
   west flash

Build with the loopback matrix only (requires the hardware wire below):

.. code-block:: console

   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S i2s \
       tests/drivers/i2s \
       -- -DCONFIG_I2S_GPIO_LOOPBACK=y -DCONFIG_I2S_LOOPBACK_ONLY=y

Build the golden-vector suite with RX verification over the loopback
wire:

.. code-block:: console

   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S i2s \
       tests/drivers/i2s \
       -- -DCONFIG_I2S_GOLDEN_TESTS=y \
          -DCONFIG_I2S_GPIO_LOOPBACK=y \
          -DCONFIG_I2S_LOOPBACK_VERIFY=y

Supported boards (overlays provided):

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

Functional sub-suite modifiers (mutually exclusive):

- ``CONFIG_I2S_LOOPBACK_ONLY=y``: Compile only the loopback matrix.
- ``CONFIG_I2S_FEATURES_ONLY=y``: Compile only the feature suite.
- ``CONFIG_I2S_FULL_DUPLEX_ONLY=y``: Compile only the full-duplex test.
- ``CONFIG_I2S_PLAY_HELLO_10S_ONLY=y``: Compile only the audio
  playback test.

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
- ``CONFIG_I2S_FULL_DUPLEX_TEST=y``: Enable the full-duplex test.
- ``CONFIG_I2S_SAMPLE44_TEST=y``: Enable the 44.1 kHz TX/RX test.
- ``CONFIG_I2S_SLAVE_MODE_SUPPORTED=y``: Declare that the I2S hardware
  supports clock-slave mode (controls skip behaviour for slave-mode
  tests).

Hardware Requirements
=====================

The loopback tests require a physical wire between SDO and SDI. The
exact pins depend on the board; see the header of each overlay in
``boards/`` for the pin pair used by that target. Examples:

- E8 DK RTSS-HP: ``P9_3`` (I2S3_SDO) -> ``P9_0`` (I2S3_SDI)
- E7/E8 DK RTSS-HE (LPI2S): ``P13_5`` (LPI2S_SDO) -> ``P13_4`` (LPI2S_SDI)
- E7 DK RTSS-HP: ``P8_2`` (I2S2_SDO) -> ``P8_1`` (I2S2_SDI)
- B1 DK RTSS-HE: ``P2_5`` (I2S0_SDO) -> ``P2_4`` (I2S0_SDI)

Without the wire, loopback and RX-verify tests self-skip at runtime.

Configuration Options
=====================

Key options in ``prj.conf``:

- ``CONFIG_I2S=y``: Enable the I2S driver.
- ``CONFIG_ZTEST=y``: Enable the ZTest framework.

The driver uses FIFO + ISR, so ``CONFIG_DMA`` is not required.

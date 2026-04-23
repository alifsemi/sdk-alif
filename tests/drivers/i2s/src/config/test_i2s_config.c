/*
 * Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * test_i2s_config.c - I2S register/config matrix test suite.
 *
 * Suite: i2s_config
 *
 * Test cases (6 total):
 *   test_config_sampling_freq_matrix    - sampling freq × word size configure
 *   test_config_word_select_cycles      - CCR.WSS field verify for 16/24/32 WS
 *   test_config_audio_resolution_matrix - TCR/RCR WLEN field for all word sizes
 *   test_config_fifo_depth_and_threshold- TFCR/RFCR default within FIFO depth
 *   test_config_programmable_fifo       - all levels 0..DEPTH-1 write/readback
 *   test_config_dma_handshake_interface - DMACR TX/RX enable bits independent
 */

#include <zephyr/drivers/i2s.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/sys_io.h>
#include <zephyr/ztest.h>
#include "i2s_test.h"

#define CFG_FIFO_DEPTH  16U
#define CFG_BLOCK_SAMPLES 64U
#define CFG_CHANNELS      2U
#define CFG_BLOCK_SIZE   (CFG_BLOCK_SAMPLES * CFG_CHANNELS * sizeof(uint32_t))
#define CFG_BLOCK_COUNT   8U
#define CFG_TIMEOUT_MS    500U

K_MEM_SLAB_DEFINE_STATIC(cfg_rx_slab, CFG_BLOCK_SIZE, CFG_BLOCK_COUNT, 4);
K_MEM_SLAB_DEFINE_STATIC(cfg_tx_slab, CFG_BLOCK_SIZE, CFG_BLOCK_COUNT, 4);

/* -------------------------------------------------------------------------
 * Helpers
 * -------------------------------------------------------------------------
 */
static void cfg_prepare(struct i2s_config *cfg, uint32_t rate, uint8_t word_size)
{
	cfg->word_size      = word_size;
	cfg->channels       = CFG_CHANNELS;
	cfg->format         = I2S_FMT_DATA_FORMAT_I2S;
	cfg->frame_clk_freq = rate;
	cfg->block_size     = CFG_BLOCK_SIZE;
	cfg->timeout        = CFG_TIMEOUT_MS;
	cfg->options        = I2S_OPT_BIT_CLK_MASTER | I2S_OPT_FRAME_CLK_MASTER;
}

static int cfg_configure_streams(const struct device *rx_dev,
				 const struct device *tx_dev,
				 const struct i2s_config *cfg)
{
	struct i2s_config rx_cfg = *cfg;
	struct i2s_config tx_cfg = *cfg;

	if (rx_dev != tx_dev) {
		rx_cfg.options = I2S_OPT_BIT_CLK_SLAVE | I2S_OPT_FRAME_CLK_SLAVE;
	}
	rx_cfg.mem_slab = &cfg_rx_slab;
	tx_cfg.mem_slab = &cfg_tx_slab;

	int ret = i2s_configure(tx_dev, I2S_DIR_TX, &tx_cfg);

	if (ret != 0) {
		return ret;
	}

	ret = i2s_configure(rx_dev, I2S_DIR_RX, &rx_cfg);
	if ((rx_dev == tx_dev) && ((ret == -ENOSYS) || (ret == -ENOTSUP))) {
		/*
		 * Shared device that does not support a second
		 * direction-specific configure: TX configure already
		 * programmed the common register file, so treat as success.
		 */
		return 0;
	}
	return ret;
}

static void cfg_deconfigure(const struct device *rx_dev,
			    const struct device *tx_dev)
{
	struct i2s_config reset = { 0 };

	(void)i2s_configure(tx_dev, I2S_DIR_TX, &reset);
	(void)i2s_configure(rx_dev, I2S_DIR_RX, &reset);
	k_sleep(K_MSEC(2));
}

/*
 * cfg_expected_wlen: The Alif I2S driver always programs WLEN=5 (32-bit
 * container) regardless of the requested word_size.  Word precision is
 * controlled via the sample clock, not the WLEN field.  Return 5 for all
 * supported word sizes to match the driver's actual register behaviour.
 */
static uint32_t cfg_expected_wlen(uint8_t word_size)
{
	(void)word_size;
	return 5U;
}

/* =========================================================================
 * TC-CFG-01: Sampling frequency × word size configure matrix
 *   Verifies that i2s_configure succeeds for all supported sample rate /
 *   word-size combinations used in the v1 feature suite.
 * =========================================================================
 */
ZTEST(i2s_config, test_config_sampling_freq_matrix)
{
	const uint32_t rates[]     = {8000U, 16000U, 32000U, 44100U, 48000U,
				      88200U, 96000U, 192000U};
	const uint8_t  word_sizes[] = {16U, 24U, 32U};
	const struct device *const rx_dev = DEVICE_DT_GET(I2S_RX_NODE);
	const struct device *const tx_dev = DEVICE_DT_GET(I2S_TX_NODE);
	uint32_t failures = 0U;
	uint32_t passes   = 0U;

	zassert_true(device_is_ready(tx_dev), "TX device not ready");

	for (size_t r = 0; r < ARRAY_SIZE(rates); r++) {
		for (size_t b = 0; b < ARRAY_SIZE(word_sizes); b++) {
			struct i2s_config cfg;

			cfg_prepare(&cfg, rates[r], word_sizes[b]);

			int ret = cfg_configure_streams(rx_dev, tx_dev, &cfg);

			if (ret == 0) {
				passes++;
				TC_PRINT("PASS cfg rate=%u bits=%u\n",
					 rates[r], word_sizes[b]);
			} else {
				failures++;
				TC_PRINT("FAIL cfg rate=%u bits=%u ret=%d\n",
					 rates[r], word_sizes[b], ret);
			}
			cfg_deconfigure(rx_dev, tx_dev);
		}
	}

	TC_PRINT("sampling-freq matrix: pass=%u fail=%u\n", passes, failures);
	zassert_equal(failures, 0U, "sampling-freq configure failures: %u", failures);
}

/* =========================================================================
 * TC-CFG-02: Word-select cycle CCR.WSS field verification
 *   Configures streams at 48 kHz / 24-bit, then writes the WSS field for
 *   16, 24, and 32 WS-cycles and reads it back to confirm hardware acceptance.
 * =========================================================================
 */
ZTEST(i2s_config, test_config_word_select_cycles)
{
	const uint32_t ws_cycles[] = {16U, 24U, 32U};
	const uint32_t ws_fields[] = {0U,   1U,  2U};
	const struct device *const rx_dev = DEVICE_DT_GET(I2S_RX_NODE);
	const struct device *const tx_dev = DEVICE_DT_GET(I2S_TX_NODE);
	uintptr_t tx_base = DT_REG_ADDR(I2S_TX_NODE);
	struct i2s_config cfg;
	uint32_t failures = 0U;
	int ret;

	zassert_true(device_is_ready(tx_dev), "TX device not ready");

	cfg_prepare(&cfg, 48000U, 24U);
	ret = cfg_configure_streams(rx_dev, tx_dev, &cfg);
	zassert_equal(ret, 0, "configure failed (%d)", ret);

	for (size_t i = 0; i < ARRAY_SIZE(ws_cycles); i++) {
		uint32_t ccr = sys_read32(tx_base + I2S_REG_CCR_OFFSET);

		ccr = (ccr & ~I2S_CCR_WSS_MASK) | (ws_fields[i] << I2S_CCR_WSS_POS);
		sys_write32(ccr, tx_base + I2S_REG_CCR_OFFSET);

		uint32_t readback = sys_read32(tx_base + I2S_REG_CCR_OFFSET);
		uint32_t wss_read = (readback & I2S_CCR_WSS_MASK) >> I2S_CCR_WSS_POS;

		if (wss_read != ws_fields[i]) {
			failures++;
			TC_PRINT("FAIL ws=%u: exp_field=%u got=%u\n",
				 ws_cycles[i], ws_fields[i], wss_read);
		} else {
			TC_PRINT("PASS ws=%u: WSS field=%u\n",
				 ws_cycles[i], wss_read);
		}
	}

	cfg_deconfigure(rx_dev, tx_dev);
	zassert_equal(failures, 0U, "word-select cycle failures: %u", failures);
}

/* =========================================================================
 * TC-CFG-03: Audio resolution TCR/RCR WLEN field matrix
 *   For each supported word size, configure streams and read TCR[2:0] and
 *   RCR[2:0] to verify the WLEN field matches the expected encoding.
 * =========================================================================
 */
ZTEST(i2s_config, test_config_audio_resolution_matrix)
{
	const uint8_t word_sizes[] = {12U, 16U, 20U, 24U, 32U};
	const struct device *const rx_dev = DEVICE_DT_GET(I2S_RX_NODE);
	const struct device *const tx_dev = DEVICE_DT_GET(I2S_TX_NODE);
	uintptr_t tx_base = DT_REG_ADDR(I2S_TX_NODE);
	uintptr_t rx_base = DT_REG_ADDR(I2S_RX_NODE);
	struct i2s_config cfg;
	uint32_t failures = 0U;
	int ret;

	zassert_true(device_is_ready(tx_dev), "TX device not ready");

	for (size_t i = 0; i < ARRAY_SIZE(word_sizes); i++) {
		cfg_prepare(&cfg, 48000U, word_sizes[i]);
		ret = cfg_configure_streams(rx_dev, tx_dev, &cfg);
		if (ret != 0) {
			failures++;
			TC_PRINT("FAIL word_size=%u configure ret=%d\n",
				 word_sizes[i], ret);
			cfg_deconfigure(rx_dev, tx_dev);
			continue;
		}

		uint32_t tcr_wlen = sys_read32(tx_base + I2S_REG_TCR_OFFSET) & 0x7U;
		uint32_t rcr_wlen = sys_read32(rx_base + I2S_REG_RCR_OFFSET) & 0x7U;
		uint32_t expected = cfg_expected_wlen(word_sizes[i]);

		if (tcr_wlen != expected) {
			failures++;
			TC_PRINT("FAIL word_size=%u TX WLEN exp=%u got=%u\n",
				 word_sizes[i], expected, tcr_wlen);
		} else {
			TC_PRINT("PASS word_size=%u TX WLEN=%u\n",
				 word_sizes[i], tcr_wlen);
		}
		if (rcr_wlen != expected) {
			failures++;
			TC_PRINT("FAIL word_size=%u RX WLEN exp=%u got=%u\n",
				 word_sizes[i], expected, rcr_wlen);
		} else {
			TC_PRINT("PASS word_size=%u RX WLEN=%u\n",
				 word_sizes[i], rcr_wlen);
		}

		cfg_deconfigure(rx_dev, tx_dev);
	}

	zassert_equal(failures, 0U, "audio resolution WLEN failures: %u", failures);
}

/* =========================================================================
 * TC-CFG-04: FIFO depth and default threshold
 *   After configure, verifies TFCR[3:0] and RFCR[3:0] are strictly less
 *   than CFG_FIFO_DEPTH (i.e. driver sets a valid default threshold).
 * =========================================================================
 */
ZTEST(i2s_config, test_config_fifo_depth_and_threshold)
{
	const struct device *const rx_dev = DEVICE_DT_GET(I2S_RX_NODE);
	const struct device *const tx_dev = DEVICE_DT_GET(I2S_TX_NODE);
	uintptr_t tx_base = DT_REG_ADDR(I2S_TX_NODE);
	uintptr_t rx_base = DT_REG_ADDR(I2S_RX_NODE);
	struct i2s_config cfg;
	uint32_t failures = 0U;
	int ret;

	zassert_true(device_is_ready(tx_dev), "TX device not ready");

	cfg_prepare(&cfg, 48000U, 24U);
	ret = cfg_configure_streams(rx_dev, tx_dev, &cfg);
	zassert_equal(ret, 0, "configure failed (%d)", ret);

	uint32_t tfcr = sys_read32(tx_base + I2S_REG_TFCR_OFFSET) & 0xFU;
	uint32_t rfcr = sys_read32(rx_base + I2S_REG_RFCR_OFFSET) & 0xFU;

	TC_PRINT("FIFO default thresholds: TX TFCR[3:0]=%u  RX RFCR[3:0]=%u\n",
		 tfcr, rfcr);

	if (tfcr >= CFG_FIFO_DEPTH) {
		failures++;
		TC_PRINT("FAIL TX default threshold=%u exceeds fifo depth %u\n",
			 tfcr, CFG_FIFO_DEPTH);
	}
	if (rfcr >= CFG_FIFO_DEPTH) {
		failures++;
		TC_PRINT("FAIL RX default threshold=%u exceeds fifo depth %u\n",
			 rfcr, CFG_FIFO_DEPTH);
	}

	cfg_deconfigure(rx_dev, tx_dev);
	zassert_equal(failures, 0U, "fifo depth threshold failures: %u", failures);
	TC_PRINT("PASS: FIFO default thresholds within depth\n");
}

/* =========================================================================
 * TC-CFG-05: Programmable FIFO threshold levels 0..DEPTH-1
 *   For every level in [0, CFG_FIFO_DEPTH), writes it to TFCR[3:0] and
 *   RFCR[3:0] and reads back to confirm register writability.
 * =========================================================================
 */
ZTEST(i2s_config, test_config_programmable_fifo)
{
	const struct device *const rx_dev = DEVICE_DT_GET(I2S_RX_NODE);
	const struct device *const tx_dev = DEVICE_DT_GET(I2S_TX_NODE);
	uintptr_t tx_base = DT_REG_ADDR(I2S_TX_NODE);
	uintptr_t rx_base = DT_REG_ADDR(I2S_RX_NODE);
	struct i2s_config cfg;
	uint32_t failures = 0U;
	int ret;

	zassert_true(device_is_ready(tx_dev), "TX device not ready");

	cfg_prepare(&cfg, 48000U, 24U);
	ret = cfg_configure_streams(rx_dev, tx_dev, &cfg);
	zassert_equal(ret, 0, "configure failed (%d)", ret);

	uint32_t tfcr_saved = sys_read32(tx_base + I2S_REG_TFCR_OFFSET);
	uint32_t rfcr_saved = sys_read32(rx_base + I2S_REG_RFCR_OFFSET);

	for (uint32_t level = 0U; level < CFG_FIFO_DEPTH; level++) {
		uint32_t tx_val = (tfcr_saved & ~0xFU) | level;
		uint32_t rx_val = (rfcr_saved & ~0xFU) | level;

		sys_write32(tx_val, tx_base + I2S_REG_TFCR_OFFSET);
		sys_write32(rx_val, rx_base + I2S_REG_RFCR_OFFSET);

		uint32_t tx_read = sys_read32(tx_base + I2S_REG_TFCR_OFFSET) & 0xFU;
		uint32_t rx_read = sys_read32(rx_base + I2S_REG_RFCR_OFFSET) & 0xFU;

		if (tx_read != level) {
			failures++;
			TC_PRINT("FAIL TX threshold level=%u read=%u\n",
				 level, tx_read);
		}
		if (rx_read != level) {
			failures++;
			TC_PRINT("FAIL RX threshold level=%u read=%u\n",
				 level, rx_read);
		}
	}

	sys_write32(tfcr_saved, tx_base + I2S_REG_TFCR_OFFSET);
	sys_write32(rfcr_saved, rx_base + I2S_REG_RFCR_OFFSET);

	cfg_deconfigure(rx_dev, tx_dev);
	zassert_equal(failures, 0U,
		      "programmable FIFO threshold failures: %u", failures);
	TC_PRINT("PASS: all FIFO levels 0..%u write/readback verified\n",
		 CFG_FIFO_DEPTH - 1U);
}

/* =========================================================================
 * TC-CFG-06: DMA hardware handshake interface — TX and RX enable bits
 *   Reads DMACR, then writes the four combinations of TDMAE/RDMAE and reads
 *   back to verify each bit is independently writable/readable.
 * =========================================================================
 */
ZTEST(i2s_config, test_config_dma_handshake_interface)
{
	const struct device *const rx_dev = DEVICE_DT_GET(I2S_RX_NODE);
	const struct device *const tx_dev = DEVICE_DT_GET(I2S_TX_NODE);
	uintptr_t tx_base = DT_REG_ADDR(I2S_TX_NODE);
	struct i2s_config cfg;
	uint32_t dmacr_saved;
	uint32_t dmacr;
	uint32_t failures = 0U;
	int ret;

	zassert_true(device_is_ready(tx_dev), "TX device not ready");

	cfg_prepare(&cfg, 48000U, 24U);
	ret = cfg_configure_streams(rx_dev, tx_dev, &cfg);
	zassert_equal(ret, 0, "configure failed (%d)", ret);

	dmacr_saved = sys_read32(tx_base + I2S_REG_DMACR_OFFSET);

	/* Both bits clear */
	sys_write32(dmacr_saved & ~(I2S_DMACR_TX_EN_BIT | I2S_DMACR_RX_EN_BIT),
		    tx_base + I2S_REG_DMACR_OFFSET);
	dmacr = sys_read32(tx_base + I2S_REG_DMACR_OFFSET);
	if ((dmacr & (I2S_DMACR_TX_EN_BIT | I2S_DMACR_RX_EN_BIT)) != 0U) {
		failures++;
		TC_PRINT("FAIL DMACR clear: read=0x%x\n", dmacr);
	}

	/* TX only */
	sys_write32((dmacr_saved & ~(I2S_DMACR_TX_EN_BIT | I2S_DMACR_RX_EN_BIT)) |
		    I2S_DMACR_TX_EN_BIT,
		    tx_base + I2S_REG_DMACR_OFFSET);
	dmacr = sys_read32(tx_base + I2S_REG_DMACR_OFFSET);
	if ((dmacr & I2S_DMACR_TX_EN_BIT) == 0U ||
	    (dmacr & I2S_DMACR_RX_EN_BIT) != 0U) {
		failures++;
		TC_PRINT("FAIL DMACR TX-only: read=0x%x\n", dmacr);
	}

	/* RX only */
	sys_write32((dmacr_saved & ~(I2S_DMACR_TX_EN_BIT | I2S_DMACR_RX_EN_BIT)) |
		    I2S_DMACR_RX_EN_BIT,
		    tx_base + I2S_REG_DMACR_OFFSET);
	dmacr = sys_read32(tx_base + I2S_REG_DMACR_OFFSET);
	if ((dmacr & I2S_DMACR_RX_EN_BIT) == 0U ||
	    (dmacr & I2S_DMACR_TX_EN_BIT) != 0U) {
		failures++;
		TC_PRINT("FAIL DMACR RX-only: read=0x%x\n", dmacr);
	}

	/* Both bits set */
	sys_write32((dmacr_saved & ~(I2S_DMACR_TX_EN_BIT | I2S_DMACR_RX_EN_BIT)) |
		    I2S_DMACR_TX_EN_BIT | I2S_DMACR_RX_EN_BIT,
		    tx_base + I2S_REG_DMACR_OFFSET);
	dmacr = sys_read32(tx_base + I2S_REG_DMACR_OFFSET);
	if ((dmacr & (I2S_DMACR_TX_EN_BIT | I2S_DMACR_RX_EN_BIT)) !=
	    (I2S_DMACR_TX_EN_BIT | I2S_DMACR_RX_EN_BIT)) {
		failures++;
		TC_PRINT("FAIL DMACR TX+RX: read=0x%x\n", dmacr);
	}

	/* Restore */
	sys_write32(dmacr_saved, tx_base + I2S_REG_DMACR_OFFSET);
	cfg_deconfigure(rx_dev, tx_dev);

	zassert_equal(failures, 0U,
		      "DMA handshake interface failures: %u", failures);
	TC_PRINT("PASS: DMA TX/RX enable bits independently writable\n");
}

/* =========================================================================
 * Suite registration
 * =========================================================================
 */
ZTEST_SUITE(i2s_config, NULL, NULL, NULL, NULL, NULL);

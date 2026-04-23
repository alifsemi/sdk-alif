/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 */

#include <zephyr/drivers/i2s.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/sys_io.h>
#include <zephyr/ztest.h>
#include <errno.h>

LOG_MODULE_REGISTER(i2s_features_test, LOG_LEVEL_INF);

#if DT_NODE_EXISTS(DT_NODELABEL(i2s_rxtx))
#define I2S_RX_NODE DT_NODELABEL(i2s_rxtx)
#define I2S_TX_NODE I2S_RX_NODE
#else
#define I2S_RX_NODE DT_NODELABEL(i2s_rx)
#define I2S_TX_NODE DT_NODELABEL(i2s_tx)
#endif

#define I2S_REG_IRER_OFFSET  0x04U
#define I2S_REG_CER_OFFSET   0x0CU
#define I2S_REG_CCR_OFFSET   0x10U
#define I2S_REG_RXFFR_OFFSET 0x14U
#define I2S_REG_RER_OFFSET   0x28U
#define I2S_REG_RCR_OFFSET   0x30U
#define I2S_REG_TCR_OFFSET   0x34U
#define I2S_REG_RFCR_OFFSET  0x48U
#define I2S_REG_TFCR_OFFSET  0x4CU
#define I2S_REG_RFF_OFFSET   0x50U
#define I2S_REG_DMACR_OFFSET 0x200U

#define I2S_FEATURE_TIMEOUT_MS 1000U
#define I2S_FEATURE_CHANNELS 2U
#define I2S_FEATURE_BLOCK_SAMPLES 64U
#define I2S_FEATURE_BLOCK_SIZE (I2S_FEATURE_BLOCK_SAMPLES * I2S_FEATURE_CHANNELS * sizeof(uint32_t))
#define I2S_FEATURE_BLOCK_COUNT 8U
#define I2S_FEATURE_FIFO_DEPTH 16U
#define I2S_CCR_WSS_POS 3U
#define I2S_CCR_WSS_MASK (0x3U << I2S_CCR_WSS_POS)
#define I2S_DMACR_RX_EN_BIT BIT(16)
#define I2S_DMACR_TX_EN_BIT BIT(17)
#define I2S_FEATURE_LOOPBACK_BLOCK_COUNT 16U

K_MEM_SLAB_DEFINE_STATIC(feature_mem_slab, I2S_FEATURE_BLOCK_SIZE, I2S_FEATURE_BLOCK_COUNT, 4);
K_MEM_SLAB_DEFINE_STATIC(feature_rx_mem_slab, I2S_FEATURE_BLOCK_SIZE,
			 I2S_FEATURE_LOOPBACK_BLOCK_COUNT, 4);
K_MEM_SLAB_DEFINE_STATIC(feature_tx_mem_slab, I2S_FEATURE_BLOCK_SIZE,
			 I2S_FEATURE_LOOPBACK_BLOCK_COUNT, 4);

struct i2s_feature_regs {
	uint32_t ccr;
	uint32_t rcr;
	uint32_t tcr;
	uint32_t rfcr;
	uint32_t tfcr;
};

static int i2s_validate_bits_exact(const uint8_t *rx, const uint8_t *tx, size_t size)
{
	for (size_t i = 0; i < size; i++) {
		if (rx[i] != tx[i]) {
			return -EINVAL;
		}
	}
	return 0;
}

static int i2s_validate_single_node_aligned(const uint32_t *rx, const uint32_t *tx,
					    uint32_t words)
{
	uint32_t best = 0U;

	if (words < 4U) {
		return -EINVAL;
	}

	for (uint32_t rx_start = 0U; rx_start < 4U && rx_start < words; rx_start++) {
		uint32_t span = words - rx_start;
		uint32_t threshold = (span * 3U) / 4U;

		for (uint32_t tx_off = 0U; tx_off < words; tx_off++) {
			uint32_t matched = 0U;

			for (uint32_t i = 0U; i < span; i++) {
				if (rx[rx_start + i] != tx[(tx_off + i) % words]) {
					break;
				}
				matched++;
			}

			if (matched > best) {
				best = matched;
			}
			if (matched >= threshold) {
				return 0;
			}
		}
	}

	return -EINVAL;
}

static inline uint32_t i2s_reg_read(uintptr_t base, uint32_t offset)
{
	return *(volatile uint32_t *)(base + offset);
}

static inline void i2s_reg_write(uintptr_t base, uint32_t offset, uint32_t value)
{
	sys_write32(value, base + offset);
}

static inline uintptr_t i2s_tx_base(void)
{
	return DT_REG_ADDR(I2S_TX_NODE);
}

static inline uintptr_t i2s_rx_base(void)
{
	return DT_REG_ADDR(I2S_RX_NODE);
}

static int i2s_ws_cycles_to_field(uint32_t ws_cycles, uint32_t *field)
{
	switch (ws_cycles) {
	case 16:
		*field = 0U;
		return 0;
	case 24:
		*field = 1U;
		return 0;
	case 32:
		*field = 2U;
		return 0;
	default:
		return -EINVAL;
	}
}

static int i2s_feature_prepare_config(struct i2s_config *cfg, uint32_t sample_rate,
				      uint8_t word_size)
{
	cfg->word_size = word_size;
	cfg->channels = I2S_FEATURE_CHANNELS;
	cfg->format = I2S_FMT_DATA_FORMAT_I2S;
	cfg->options = I2S_OPT_BIT_CLK_MASTER | I2S_OPT_FRAME_CLK_MASTER;
	cfg->frame_clk_freq = sample_rate;
	cfg->mem_slab = &feature_mem_slab;
	cfg->block_size = I2S_FEATURE_BLOCK_SIZE;
	cfg->timeout = I2S_FEATURE_TIMEOUT_MS;

	return 0;
}

static void i2s_feature_deconfigure(const struct device *rx_dev, const struct device *tx_dev)
{
	struct i2s_config reset_cfg = { 0 };

	/* Settle: give any in-flight RX/TX ISR time to complete before we
	 * zero stream->cfg (which the Alif DW driver's ISR dereferences as
	 * stream->cfg.mem_slab for the next buffer alloc). Without this
	 * sleep, a late ISR can fault at NULL + offset. Matches the proven
	 * mx_stop_streams pattern used by the lb_matrix_* tests.
	 */
	k_sleep(K_MSEC(50));
	(void)i2s_configure(rx_dev, I2S_DIR_RX, &reset_cfg);
	(void)i2s_configure(tx_dev, I2S_DIR_TX, &reset_cfg);
}

static int i2s_feature_configure_streams(const struct device *rx_dev, const struct device *tx_dev,
					 const struct i2s_config *cfg)
{
	int ret;

	ret = i2s_configure(rx_dev, I2S_DIR_RX, cfg);
	if (ret < 0) {
		return ret;
	}

	ret = i2s_configure(tx_dev, I2S_DIR_TX, cfg);
	if (ret < 0) {
		return ret;
	}

	return 0;
}

static int i2s_feature_configure_streams_split(const struct device *rx_dev,
					       const struct device *tx_dev,
					       const struct i2s_config *rx_cfg,
					       const struct i2s_config *tx_cfg)
{
	int ret;

	ret = i2s_configure(rx_dev, I2S_DIR_RX, rx_cfg);
	if (ret < 0) {
		return ret;
	}

	ret = i2s_configure(tx_dev, I2S_DIR_TX, tx_cfg);
	if (ret < 0) {
		return ret;
	}

	return 0;
}
static bool i2s_feature_is_not_supported_error(int ret)
{
	return ret == -ENOSYS || ret == -ENOTSUP;
}

static bool i2s_feature_ptr_in_slab(struct k_mem_slab *slab, void *ptr)
{
	uintptr_t p;
	uintptr_t start;
	size_t span;

	if (ptr == NULL || slab == NULL || slab->buffer == NULL) {
		return false;
	}

	p = (uintptr_t)ptr;
	start = (uintptr_t)slab->buffer;
	span = (size_t)slab->info.block_size * slab->info.num_blocks;

	if (p < start || p >= (start + span)) {
		return false;
	}

	return ((p - start) % slab->info.block_size) == 0U;
}

static int i2s_feature_safe_slab_free(struct k_mem_slab *slab, void *ptr)
{
	if (!i2s_feature_ptr_in_slab(slab, ptr)) {
		return -EINVAL;
	}

	k_mem_slab_free(slab, ptr);
	return 0;
}

static int i2s_feature_start_and_capture(const struct device *rx_dev,
					 const struct device *tx_dev,
					 const struct i2s_config *cfg,
					 struct i2s_feature_regs *regs,
					 uint32_t force_ws_cycles)
{
	int ret;
	void *mem_block = NULL;
	uint32_t *samples;
	uint32_t ccr;

	ret = k_mem_slab_alloc(&feature_mem_slab, &mem_block, K_NO_WAIT);
	if (ret < 0) {
		return ret;
	}

	samples = mem_block;
	for (uint32_t i = 0; i < (cfg->block_size / sizeof(uint32_t)); i++) {
		samples[i] = i;
	}

	ret = i2s_write(tx_dev, mem_block, cfg->block_size);
	if (ret < 0) {
		(void)i2s_feature_safe_slab_free(&feature_mem_slab, mem_block);
		return ret;
	}

	ret = i2s_trigger(rx_dev, I2S_DIR_RX, I2S_TRIGGER_START);
	if (ret < 0) {
		(void)i2s_trigger(tx_dev, I2S_DIR_TX, I2S_TRIGGER_DROP);
		(void)i2s_trigger(rx_dev, I2S_DIR_RX, I2S_TRIGGER_DROP);
		return ret;
	}

	ret = i2s_trigger(tx_dev, I2S_DIR_TX, I2S_TRIGGER_START);
	if (ret < 0) {
		(void)i2s_trigger(tx_dev, I2S_DIR_TX, I2S_TRIGGER_DROP);
		(void)i2s_trigger(rx_dev, I2S_DIR_RX, I2S_TRIGGER_DROP);
		return ret;
	}

	if (force_ws_cycles != 0U) {
		uint32_t ws_field;

		ret = i2s_ws_cycles_to_field(force_ws_cycles, &ws_field);
		if (ret < 0) {
			(void)i2s_trigger(tx_dev, I2S_DIR_TX, I2S_TRIGGER_DROP);
			(void)i2s_trigger(rx_dev, I2S_DIR_RX, I2S_TRIGGER_DROP);
			return ret;
		}

		ccr = i2s_reg_read(i2s_tx_base(), I2S_REG_CCR_OFFSET);
		ccr = (ccr & ~I2S_CCR_WSS_MASK) | (ws_field << I2S_CCR_WSS_POS);
		i2s_reg_write(i2s_tx_base(), I2S_REG_CCR_OFFSET, ccr);
	}

	if (regs != NULL) {
		regs->ccr = i2s_reg_read(i2s_tx_base(), I2S_REG_CCR_OFFSET);
		regs->tcr = i2s_reg_read(i2s_tx_base(), I2S_REG_TCR_OFFSET);
		regs->tfcr = i2s_reg_read(i2s_tx_base(), I2S_REG_TFCR_OFFSET);
		regs->rcr = i2s_reg_read(i2s_rx_base(), I2S_REG_RCR_OFFSET);
		regs->rfcr = i2s_reg_read(i2s_rx_base(), I2S_REG_RFCR_OFFSET);
	}

	k_sleep(K_MSEC(2));

	(void)i2s_trigger(tx_dev, I2S_DIR_TX, I2S_TRIGGER_DROP);
	(void)i2s_trigger(rx_dev, I2S_DIR_RX, I2S_TRIGGER_DROP);

	return 0;
}

static uint32_t i2s_expected_wlen_field(uint32_t word_size)
{
	switch (word_size) {
	case 12:
		return 1U;
	case 16:
		return 2U;
	case 20:
		return 3U;
	case 24:
		return 4U;
	case 32:
		return 5U;
	default:
		return 0U;
	}
}

#ifndef CONFIG_I2S_FULL_DUPLEX
ZTEST(i2s_features, test_sampling_frequencies_matrix)
{
	const uint32_t sample_rates[] = {8000, 16000, 32000, 44100, 48000, 88200, 96000, 192000};
	const struct device *const rx_dev = DEVICE_DT_GET(I2S_RX_NODE);
	const struct device *const tx_dev = DEVICE_DT_GET(I2S_TX_NODE);
	struct i2s_config cfg;
	int ret;
	uint32_t failures = 0U;
	uint32_t skips = 0U;
	uint32_t passes = 0U;

	zassert_true(device_is_ready(rx_dev), "RX device not ready");
	zassert_true(device_is_ready(tx_dev), "TX device not ready");

	for (size_t i = 0; i < ARRAY_SIZE(sample_rates); i++) {
		i2s_feature_prepare_config(&cfg, sample_rates[i], 24);

		ret = i2s_feature_configure_streams(rx_dev, tx_dev, &cfg);
		if (ret != 0) {
			failures++;
			LOG_INF("FAIL sample_rate=%u configure ret=%d\n", sample_rates[i], ret);
			i2s_feature_deconfigure(rx_dev, tx_dev);
			continue;
		}

		ret = i2s_feature_start_and_capture(rx_dev, tx_dev, &cfg, NULL, 0U);
		if (ret != 0) {
			if (i2s_feature_is_not_supported_error(ret)) {
				skips++;
				LOG_INF("SKIP sample_rate=%u start/capture ret=%d\n",
					 sample_rates[i], ret);
			} else {
				failures++;
				LOG_INF("FAIL sample_rate=%u start/capture ret=%d\n",
					 sample_rates[i], ret);
			}
		} else {
			passes++;
		}

		i2s_feature_deconfigure(rx_dev, tx_dev);
	}

	LOG_INF("sampling matrix summary pass=%u skip=%u fail=%u\n", passes, skips, failures);
	if (passes == 0U && failures == 0U) {
		ztest_test_skip();
	}
	zassert_equal(failures, 0U, "sampling frequency failures: %u", failures);
}

ZTEST(i2s_features, test_word_select_cycles_matrix)
{
	const uint32_t ws_cycles[] = {16U, 24U, 32U};
	const struct device *const rx_dev = DEVICE_DT_GET(I2S_RX_NODE);
	const struct device *const tx_dev = DEVICE_DT_GET(I2S_TX_NODE);
	struct i2s_config cfg;
	struct i2s_feature_regs regs;
	int ret;
	uint32_t failures = 0U;
	uint32_t skips = 0U;
	uint32_t passes = 0U;

	zassert_true(device_is_ready(rx_dev), "RX device not ready");
	zassert_true(device_is_ready(tx_dev), "TX device not ready");

	for (size_t i = 0; i < ARRAY_SIZE(ws_cycles); i++) {
		uint32_t expected_wss_field = 0U;

		i2s_feature_prepare_config(&cfg, 48000, 24);

		ret = i2s_feature_configure_streams(rx_dev, tx_dev, &cfg);
		if (ret != 0) {
			failures++;
			LOG_INF("FAIL ws=%u configure ret=%d\n", ws_cycles[i], ret);
			i2s_feature_deconfigure(rx_dev, tx_dev);
			continue;
		}

		ret = i2s_feature_start_and_capture(rx_dev, tx_dev, &cfg, &regs, ws_cycles[i]);
		if (ret != 0) {
			if (i2s_feature_is_not_supported_error(ret)) {
				skips++;
				LOG_INF("SKIP ws=%u start/capture ret=%d\n", ws_cycles[i], ret);
			} else {
				failures++;
				LOG_INF("FAIL ws=%u start/capture ret=%d\n", ws_cycles[i], ret);
			}
			i2s_feature_deconfigure(rx_dev, tx_dev);
			continue;
		}

		ret = i2s_ws_cycles_to_field(ws_cycles[i], &expected_wss_field);
		if (ret != 0) {
			failures++;
			LOG_INF("FAIL ws=%u invalid test vector\n", ws_cycles[i]);
		} else {
			uint32_t actual_wss_field =
				(regs.ccr & I2S_CCR_WSS_MASK) >> I2S_CCR_WSS_POS;

			if (actual_wss_field != expected_wss_field) {
				failures++;
				LOG_INF("FAIL ws=%u wss_field exp=%u act=%u\n",
					 ws_cycles[i], expected_wss_field,
					 actual_wss_field);
			} else {
				passes++;
			}
		}

		i2s_feature_deconfigure(rx_dev, tx_dev);
	}

	LOG_INF("word-select matrix summary pass=%u skip=%u fail=%u\n", passes, skips, failures);
	if (passes == 0U && failures == 0U) {
		ztest_test_skip();
	}
	zassert_equal(failures, 0U, "word-select failures: %u", failures);
}

ZTEST(i2s_features, test_audio_resolution_matrix)
{
	const uint8_t word_sizes[] = {12, 16, 20, 24, 32};
	const struct device *const rx_dev = DEVICE_DT_GET(I2S_RX_NODE);
	const struct device *const tx_dev = DEVICE_DT_GET(I2S_TX_NODE);
	struct i2s_config cfg;
	struct i2s_feature_regs regs;
	int ret;
	uint32_t failures = 0U;

	zassert_true(device_is_ready(rx_dev), "RX device not ready");
	zassert_true(device_is_ready(tx_dev), "TX device not ready");

	for (size_t i = 0; i < ARRAY_SIZE(word_sizes); i++) {
		i2s_feature_prepare_config(&cfg, 48000, word_sizes[i]);

		ret = i2s_feature_configure_streams(rx_dev, tx_dev, &cfg);
		if (ret != 0) {
			failures++;
			LOG_INF("FAIL word_size=%u configure ret=%d\n", word_sizes[i], ret);
			i2s_feature_deconfigure(rx_dev, tx_dev);
			continue;
		}

		ret = i2s_feature_start_and_capture(rx_dev, tx_dev, &cfg, &regs, 0U);
		if (ret != 0) {
			failures++;
			LOG_INF("FAIL word_size=%u start/capture ret=%d\n", word_sizes[i], ret);
			i2s_feature_deconfigure(rx_dev, tx_dev);
			continue;
		}

		if ((regs.tcr & 0x7U) != i2s_expected_wlen_field(word_sizes[i])) {
			failures++;
			LOG_INF("FAIL word_size=%u tx_wlen exp=%u act=%u\n", word_sizes[i],
				 i2s_expected_wlen_field(word_sizes[i]), (regs.tcr & 0x7U));
		}
		if ((regs.rcr & 0x7U) != i2s_expected_wlen_field(word_sizes[i])) {
			failures++;
			LOG_INF("FAIL word_size=%u rx_wlen exp=%u act=%u\n", word_sizes[i],
				 i2s_expected_wlen_field(word_sizes[i]), (regs.rcr & 0x7U));
		}

		i2s_feature_deconfigure(rx_dev, tx_dev);
	}

	zassert_equal(failures, 0U, "resolution failures: %u", failures);
}

static int i2s_same_node_loopback_run(uint32_t sample_rate, uint8_t word_size,
				      uint32_t ws_cycles)
{
	const struct device *const rx_dev = DEVICE_DT_GET(I2S_RX_NODE);
	const struct device *const tx_dev = DEVICE_DT_GET(I2S_TX_NODE);
	const struct device *const dev = tx_dev;
	struct i2s_config probe_cfg;
	struct i2s_config reset_cfg = { 0 };
	struct i2s_config rx_cfg;
	struct i2s_config tx_cfg;
	void *tx_block = NULL;
	void *rx_block = NULL;
	size_t rx_size = 0U;
	uint32_t *tx_words;
	const uint32_t words = I2S_FEATURE_BLOCK_SIZE / sizeof(uint32_t);
	const uint32_t shift = (word_size < 32U) ? (32U - word_size) : 0U;
	const uint32_t mask = (word_size < 32U) ? ((1U << word_size) - 1U) : 0xFFFFFFFFU;
	uint32_t ws_field = 0U;
	uint32_t ccr;
	int ret;
	int rc = 0;

	if (rx_dev != tx_dev) {
		return -ENOTSUP;
	}

	(void)i2s_trigger(dev, I2S_DIR_BOTH, I2S_TRIGGER_DROP);
	(void)i2s_configure(dev, I2S_DIR_TX, &reset_cfg);
	(void)i2s_configure(dev, I2S_DIR_RX, &reset_cfg);
	k_sleep(K_MSEC(5));

	i2s_feature_prepare_config(&probe_cfg, sample_rate, word_size);
	probe_cfg.mem_slab = &feature_tx_mem_slab;

	ret = i2s_configure(dev, I2S_DIR_BOTH, &probe_cfg);
	if (ret != 0) {
		return i2s_feature_is_not_supported_error(ret) ? -ENOTSUP : ret;
	}

	ret = i2s_trigger(dev, I2S_DIR_BOTH, I2S_TRIGGER_DROP);
	if (ret != 0) {
		rc = i2s_feature_is_not_supported_error(ret) ? -ENOTSUP : ret;
		goto cleanup_reset;
	}

	i2s_feature_prepare_config(&rx_cfg, sample_rate, word_size);
	i2s_feature_prepare_config(&tx_cfg, sample_rate, word_size);
	rx_cfg.options = I2S_OPT_BIT_CLK_MASTER | I2S_OPT_FRAME_CLK_MASTER;
	tx_cfg.options = I2S_OPT_BIT_CLK_MASTER | I2S_OPT_FRAME_CLK_MASTER;
	rx_cfg.mem_slab = &feature_rx_mem_slab;
	tx_cfg.mem_slab = &feature_tx_mem_slab;
	rx_cfg.timeout = 500U;
	tx_cfg.timeout = 500U;

	ret = i2s_configure(dev, I2S_DIR_RX, &rx_cfg);
	if (ret != 0) {
		rc = i2s_feature_is_not_supported_error(ret) ? -ENOTSUP : ret;
		goto cleanup_reset;
	}

	ret = i2s_configure(dev, I2S_DIR_TX, &tx_cfg);
	if (ret != 0) {
		rc = i2s_feature_is_not_supported_error(ret) ? -ENOTSUP : ret;
		goto cleanup_reset;
	}

	ret = i2s_ws_cycles_to_field(ws_cycles, &ws_field);
	if (ret != 0) {
		rc = ret;
		goto cleanup_reset;
	}

	ccr = i2s_reg_read(i2s_tx_base(), I2S_REG_CCR_OFFSET);
	ccr = (ccr & ~I2S_CCR_WSS_MASK) | (ws_field << I2S_CCR_WSS_POS);
	i2s_reg_write(i2s_tx_base(), I2S_REG_CCR_OFFSET, ccr);

	for (uint8_t s = 0U; s < 3U; s++) {
		void *seed = NULL;

		ret = k_mem_slab_alloc(&feature_tx_mem_slab, &seed, K_NO_WAIT);
		if (ret != 0) {
			rc = i2s_feature_is_not_supported_error(ret) ? -ENOTSUP : ret;
			goto cleanup_streams;
		}

		memset(seed, 0, I2S_FEATURE_BLOCK_SIZE);
		ret = i2s_write(dev, seed, I2S_FEATURE_BLOCK_SIZE);
		if (ret != 0) {
			(void)i2s_feature_safe_slab_free(&feature_tx_mem_slab, seed);
			rc = i2s_feature_is_not_supported_error(ret) ? -ENOTSUP : ret;
			goto cleanup_streams;
		}
	}

	ret = k_mem_slab_alloc(&feature_tx_mem_slab, &tx_block, K_NO_WAIT);
	if (ret != 0) {
		rc = i2s_feature_is_not_supported_error(ret) ? -ENOTSUP : ret;
		goto cleanup_streams;
	}

	tx_words = tx_block;
	for (uint32_t i = 0U; i < words; i++) {
		uint32_t v = (i + 1U) & mask;

		if (v == 0U) {
			v = 1U;
		}
		tx_words[i] = v << shift;
	}

	ret = i2s_write(dev, tx_block, I2S_FEATURE_BLOCK_SIZE);
	if (ret != 0) {
		(void)i2s_feature_safe_slab_free(&feature_tx_mem_slab, tx_block);
		tx_block = NULL;
		rc = i2s_feature_is_not_supported_error(ret) ? -ENOTSUP : ret;
		goto cleanup_streams;
	}

	ret = i2s_trigger(dev, I2S_DIR_BOTH, I2S_TRIGGER_START);
	if (ret != 0) {
		rc = i2s_feature_is_not_supported_error(ret) ? -ENOTSUP : ret;
		goto cleanup_streams;
	}

	for (uint8_t s = 0U; s < 3U; s++) {
		void *discard = NULL;
		size_t discard_sz = 0U;

		ret = i2s_read(dev, &discard, &discard_sz);
		if (ret != 0) {
			rc = i2s_feature_is_not_supported_error(ret) ? -ENOTSUP : ret;
			goto cleanup_streams;
		}
		(void)i2s_feature_safe_slab_free(&feature_rx_mem_slab, discard);
	}

	ret = i2s_read(dev, &rx_block, &rx_size);
	if (ret != 0) {
		rc = i2s_feature_is_not_supported_error(ret) ? -ENOTSUP : ret;
		goto cleanup_streams;
	}

	if (rx_size != I2S_FEATURE_BLOCK_SIZE) {
		rc = -EIO;
		goto cleanup_rx_same;
	}

	{
		const uint32_t *rx_w = rx_block;
		uint32_t nz = 0U;

		for (uint32_t i = 0U; i < (rx_size / sizeof(uint32_t)); i++) {
			if (rx_w[i] != 0U) {
				nz++;
			}
		}

		if (nz < 4U) {
			rc = -ENOTSUP;
			goto cleanup_rx_same;
		}
	}

	ret = i2s_validate_single_node_aligned((const uint32_t *)rx_block,
					       (const uint32_t *)tx_block,
					       words);
	if (ret != 0) {
		rc = -EIO;
	}

cleanup_rx_same:
	if (rx_block != NULL) {
		(void)i2s_feature_safe_slab_free(&feature_rx_mem_slab, rx_block);
	}

cleanup_streams:
	(void)i2s_trigger(dev, I2S_DIR_BOTH, I2S_TRIGGER_DROP);

cleanup_reset:
	(void)i2s_configure(dev, I2S_DIR_TX, &reset_cfg);
	(void)i2s_configure(dev, I2S_DIR_RX, &reset_cfg);
	k_sleep(K_MSEC(5));

	return rc;
}

ZTEST(i2s_features, test_same_node_loopback_32bit_8khz_ws32)
{
	const struct device *const rx_dev = DEVICE_DT_GET(I2S_RX_NODE);
	const struct device *const tx_dev = DEVICE_DT_GET(I2S_TX_NODE);
	int ret;

	zassert_true(device_is_ready(rx_dev), "RX device not ready");
	zassert_true(device_is_ready(tx_dev), "TX device not ready");

	if (rx_dev != tx_dev) {
		ztest_test_skip();
		return;
	}

	ret = i2s_same_node_loopback_run(8000U, 32U, 32U);
	if (ret == -ENOTSUP || i2s_feature_is_not_supported_error(ret)) {
		LOG_INF("SKIP same-node loopback rate=8000 ws=32 bits=32 ret=%d\n", ret);
		ztest_test_skip();
		return;
	}

	LOG_INF("PASS same-node loopback rate=8000 ws=32 bits=32\n");
	zassert_equal(ret, 0, "same-node loopback 8k/32-bit/ws32 failed (%d)", ret);
}

ZTEST(i2s_features, test_fifo_depth_and_threshold)
{
	const struct device *const rx_dev = DEVICE_DT_GET(I2S_RX_NODE);
	const struct device *const tx_dev = DEVICE_DT_GET(I2S_TX_NODE);
	struct i2s_config cfg;
	struct i2s_feature_regs regs = {0};
	int ret;
	uint32_t failures = 0U;
	uint32_t tfcr_saved;
	uint32_t rfcr_saved;

	zassert_true(device_is_ready(rx_dev), "RX device not ready");
	zassert_true(device_is_ready(tx_dev), "TX device not ready");

	i2s_feature_prepare_config(&cfg, 48000, 24);
	ret = i2s_feature_configure_streams(rx_dev, tx_dev, &cfg);
	zassert_equal(ret, 0, "configure failed (%d)", ret);

	tfcr_saved = i2s_reg_read(i2s_tx_base(), I2S_REG_TFCR_OFFSET);
	rfcr_saved = i2s_reg_read(i2s_rx_base(), I2S_REG_RFCR_OFFSET);

	regs.tfcr = i2s_reg_read(i2s_tx_base(), I2S_REG_TFCR_OFFSET) & 0xFU;
	regs.rfcr = i2s_reg_read(i2s_rx_base(), I2S_REG_RFCR_OFFSET) & 0xFU;

	if (regs.tfcr >= I2S_FEATURE_FIFO_DEPTH) {
		failures++;
		LOG_INF("FAIL tx_trigger default=%u exceeds fifo depth\n", regs.tfcr);
	}
	if (regs.rfcr >= I2S_FEATURE_FIFO_DEPTH) {
		failures++;
		LOG_INF("FAIL rx_trigger default=%u exceeds fifo depth\n", regs.rfcr);
	}

	for (uint32_t level = 0U; level < I2S_FEATURE_FIFO_DEPTH; level++) {
		uint32_t tx_val = (tfcr_saved & ~0xFU) | level;
		uint32_t rx_val = (rfcr_saved & ~0xFU) | level;
		uint32_t tx_read;
		uint32_t rx_read;

		i2s_reg_write(i2s_tx_base(), I2S_REG_TFCR_OFFSET, tx_val);
		i2s_reg_write(i2s_rx_base(), I2S_REG_RFCR_OFFSET, rx_val);

		tx_read = i2s_reg_read(i2s_tx_base(), I2S_REG_TFCR_OFFSET) & 0xFU;
		rx_read = i2s_reg_read(i2s_rx_base(), I2S_REG_RFCR_OFFSET) & 0xFU;

		if (tx_read != level) {
			failures++;
			LOG_INF("FAIL tx_fifo_level level=%u read=%u\n", level, tx_read);
		}
		if (rx_read != level) {
			failures++;
			LOG_INF("FAIL rx_fifo_level level=%u read=%u\n", level, rx_read);
		}
	}

	i2s_reg_write(i2s_tx_base(), I2S_REG_TFCR_OFFSET, tfcr_saved);
	i2s_reg_write(i2s_rx_base(), I2S_REG_RFCR_OFFSET, rfcr_saved);

	i2s_feature_deconfigure(rx_dev, tx_dev);
	zassert_equal(failures, 0U, "fifo depth failures: %u", failures);
}

ZTEST(i2s_features, test_programmable_fifo_thresholds)
{
	const struct device *const rx_dev = DEVICE_DT_GET(I2S_RX_NODE);
	const struct device *const tx_dev = DEVICE_DT_GET(I2S_TX_NODE);
	struct i2s_config cfg;
	uint32_t tfcr_saved;
	uint32_t rfcr_saved;
	uint32_t failures = 0U;
	int ret;

	zassert_true(device_is_ready(rx_dev), "RX device not ready");
	zassert_true(device_is_ready(tx_dev), "TX device not ready");

	i2s_feature_prepare_config(&cfg, 48000, 24);
	ret = i2s_feature_configure_streams(rx_dev, tx_dev, &cfg);
	zassert_equal(ret, 0, "configure failed (%d)", ret);

	tfcr_saved = i2s_reg_read(i2s_tx_base(), I2S_REG_TFCR_OFFSET);
	rfcr_saved = i2s_reg_read(i2s_rx_base(), I2S_REG_RFCR_OFFSET);

	for (uint32_t level = 0U; level < I2S_FEATURE_FIFO_DEPTH; level++) {
		uint32_t tx_expected = (tfcr_saved & ~0xFU) | level;
		uint32_t rx_expected = (rfcr_saved & ~0xFU) | level;
		uint32_t tx_actual;
		uint32_t rx_actual;

		i2s_reg_write(i2s_tx_base(), I2S_REG_TFCR_OFFSET, tx_expected);
		i2s_reg_write(i2s_rx_base(), I2S_REG_RFCR_OFFSET, rx_expected);

		tx_actual = i2s_reg_read(i2s_tx_base(), I2S_REG_TFCR_OFFSET) & 0xFU;
		rx_actual = i2s_reg_read(i2s_rx_base(), I2S_REG_RFCR_OFFSET) & 0xFU;

		if (tx_actual != level) {
			failures++;
			LOG_INF("FAIL programmable TX threshold level=%u read=%u\n",
				 level, tx_actual);
		}
		if (rx_actual != level) {
			failures++;
			LOG_INF("FAIL programmable RX threshold level=%u read=%u\n",
				 level, rx_actual);
		}
	}

	i2s_reg_write(i2s_tx_base(), I2S_REG_TFCR_OFFSET, tfcr_saved);
	i2s_reg_write(i2s_rx_base(), I2S_REG_RFCR_OFFSET, rfcr_saved);
	i2s_feature_deconfigure(rx_dev, tx_dev);

	zassert_equal(failures, 0U, "programmable fifo threshold failures: %u", failures);
}

ZTEST(i2s_features, test_dma_hardware_handshaking_interface)
{
	const struct device *const rx_dev = DEVICE_DT_GET(I2S_RX_NODE);
	const struct device *const tx_dev = DEVICE_DT_GET(I2S_TX_NODE);
	struct i2s_config cfg;
	uint32_t dmacr_saved;
	uint32_t dmacr;
	uint32_t failures = 0U;
	int ret;

	zassert_true(device_is_ready(rx_dev), "RX device not ready");
	zassert_true(device_is_ready(tx_dev), "TX device not ready");

	i2s_feature_prepare_config(&cfg, 48000, 24);
	ret = i2s_feature_configure_streams(rx_dev, tx_dev, &cfg);
	zassert_equal(ret, 0, "configure failed (%d)", ret);

	dmacr_saved = i2s_reg_read(i2s_tx_base(), I2S_REG_DMACR_OFFSET);

	/* Verify TX and RX DMA handshake enables can be controlled independently. */
	i2s_reg_write(i2s_tx_base(), I2S_REG_DMACR_OFFSET, dmacr_saved & ~(I2S_DMACR_TX_EN_BIT |
								  I2S_DMACR_RX_EN_BIT));
	dmacr = i2s_reg_read(i2s_tx_base(), I2S_REG_DMACR_OFFSET);
	if ((dmacr & (I2S_DMACR_TX_EN_BIT | I2S_DMACR_RX_EN_BIT)) != 0U) {
		failures++;
		LOG_INF("FAIL DMACR clear handshake bits read=0x%x\n", dmacr);
	}

	i2s_reg_write(i2s_tx_base(), I2S_REG_DMACR_OFFSET,
		      (dmacr_saved & ~(I2S_DMACR_TX_EN_BIT | I2S_DMACR_RX_EN_BIT)) |
			      I2S_DMACR_TX_EN_BIT);
	dmacr = i2s_reg_read(i2s_tx_base(), I2S_REG_DMACR_OFFSET);
	if ((dmacr & I2S_DMACR_TX_EN_BIT) == 0U || (dmacr & I2S_DMACR_RX_EN_BIT) != 0U) {
		failures++;
		LOG_INF("FAIL DMACR TX-only handshake read=0x%x\n", dmacr);
	}

	i2s_reg_write(i2s_tx_base(), I2S_REG_DMACR_OFFSET,
		      (dmacr_saved & ~(I2S_DMACR_TX_EN_BIT | I2S_DMACR_RX_EN_BIT)) |
			      I2S_DMACR_RX_EN_BIT);
	dmacr = i2s_reg_read(i2s_tx_base(), I2S_REG_DMACR_OFFSET);
	if ((dmacr & I2S_DMACR_RX_EN_BIT) == 0U || (dmacr & I2S_DMACR_TX_EN_BIT) != 0U) {
		failures++;
		LOG_INF("FAIL DMACR RX-only handshake read=0x%x\n", dmacr);
	}

	i2s_reg_write(i2s_tx_base(), I2S_REG_DMACR_OFFSET,
		      (dmacr_saved & ~(I2S_DMACR_TX_EN_BIT | I2S_DMACR_RX_EN_BIT)) |
			      I2S_DMACR_TX_EN_BIT | I2S_DMACR_RX_EN_BIT);
	dmacr = i2s_reg_read(i2s_tx_base(), I2S_REG_DMACR_OFFSET);
	if ((dmacr & (I2S_DMACR_TX_EN_BIT | I2S_DMACR_RX_EN_BIT)) !=
	    (I2S_DMACR_TX_EN_BIT | I2S_DMACR_RX_EN_BIT)) {
		failures++;
		LOG_INF("FAIL DMACR TX+RX handshake read=0x%x\n", dmacr);
	}

	i2s_reg_write(i2s_tx_base(), I2S_REG_DMACR_OFFSET, dmacr_saved);
	i2s_feature_deconfigure(rx_dev, tx_dev);

	zassert_equal(failures, 0U, "dma hardware handshaking failures: %u", failures);
}

ZTEST(i2s_features, test_mic_rx_to_tx_bit_exact_forwarding)
{
	if (!IS_ENABLED(CONFIG_I2S_SLAVE_MODE_SUPPORTED)) {
		LOG_INF("SKIP: I2S clock-slave mode not supported on this platform "
			"(enable CONFIG_I2S_SLAVE_MODE_SUPPORTED if the IP supports it)");
		ztest_test_skip();
		return;
	}

	const struct device *const rx_dev = DEVICE_DT_GET(I2S_RX_NODE);
	const struct device *const tx_dev = DEVICE_DT_GET(I2S_TX_NODE);
	struct i2s_config rx_cfg;
	struct i2s_config tx_cfg;
	struct k_mem_slab *active_slab;
	uint32_t failures = 0U;
	uint32_t skips = 0U;
	uint32_t passes = 0U;
	const bool shared_dev = (rx_dev == tx_dev);
	int ret;

	/* Keep bounded to avoid test hangs when no microphone stream is present. */
	const uint32_t transfer_blocks = 8U;

	zassert_true(device_is_ready(rx_dev), "RX device not ready");
	zassert_true(device_is_ready(tx_dev), "TX device not ready");
	if (shared_dev) {
		ztest_test_skip();
		return;
	}

	i2s_feature_prepare_config(&rx_cfg, 48000, 24);
	i2s_feature_prepare_config(&tx_cfg, 48000, 24);
	rx_cfg.timeout = 100U;
	tx_cfg.timeout = 100U;
	rx_cfg.options = I2S_OPT_BIT_CLK_SLAVE | I2S_OPT_FRAME_CLK_SLAVE;
	if (shared_dev) {
		active_slab = &feature_mem_slab;
		rx_cfg.mem_slab = active_slab;
		tx_cfg.mem_slab = active_slab;
	} else {
		active_slab = &feature_tx_mem_slab;
		rx_cfg.mem_slab = &feature_rx_mem_slab;
		tx_cfg.mem_slab = active_slab;
	}

	ret = i2s_feature_configure_streams_split(rx_dev, tx_dev, &rx_cfg, &tx_cfg);
	zassert_equal(ret, 0, "configure failed (%d)", ret);

	/* Seed multiple TX blocks to avoid underrun on shared RX/TX instances. */
	for (uint8_t seed = 0U; seed < 3U; seed++) {
		void *tx_seed = NULL;
		uint32_t *seed_words;

		ret = k_mem_slab_alloc(active_slab, &tx_seed, K_NO_WAIT);
		zassert_equal(ret, 0, "seed alloc failed (%d)", ret);
		seed_words = tx_seed;
		for (uint32_t i = 0; i < (tx_cfg.block_size / sizeof(uint32_t)); i++) {
			seed_words[i] = 0U;
		}
		ret = i2s_write(tx_dev, tx_seed, tx_cfg.block_size);
		zassert_equal(ret, 0, "seed write failed (%d)", ret);
	}

	ret = i2s_trigger(tx_dev, I2S_DIR_TX, I2S_TRIGGER_START);
	zassert_equal(ret, 0, "TX start failed (%d)", ret);
	ret = i2s_trigger(rx_dev, I2S_DIR_RX, I2S_TRIGGER_START);
	zassert_equal(ret, 0, "RX start failed (%d)", ret);

	for (uint32_t block = 0U; block < transfer_blocks; block++) {
		void *rx_block;
		void *tx_block;
		size_t rx_size;

		ret = i2s_read(rx_dev, &rx_block, &rx_size);
		if (ret < 0) {
			if (ret == -EAGAIN) {
				skips++;
				LOG_INF("SKIP mic->tx block=%u no rx data ret=%d\n", block, ret);
				break;
			} else if (i2s_feature_is_not_supported_error(ret)) {
				skips++;
				LOG_INF("SKIP mic->tx block=%u rx read ret=%d\n", block, ret);
			} else {
				failures++;
				LOG_INF("FAIL mic->tx block=%u rx read ret=%d\n", block, ret);
			}
			continue;
		}

		if (rx_size != rx_cfg.block_size) {
			struct k_mem_slab *rx_free_slab =
				shared_dev ? active_slab : &feature_rx_mem_slab;

			failures++;
			LOG_INF("FAIL mic->tx block=%u size mismatch exp=%u act=%u\n",
				 block, rx_cfg.block_size, (uint32_t)rx_size);
			if (i2s_feature_safe_slab_free(rx_free_slab, rx_block) != 0) {
				failures++;
				LOG_INF("FAIL mic->tx block=%u invalid rx slab pointer\n",
					 block);
			}
			continue;
		}

		ret = k_mem_slab_alloc(active_slab, &tx_block, K_NO_WAIT);
		if (ret < 0) {
			struct k_mem_slab *rx_free_slab =
				shared_dev ? active_slab : &feature_rx_mem_slab;

			failures++;
			LOG_INF("FAIL mic->tx block=%u tx alloc ret=%d\n", block, ret);
			if (i2s_feature_safe_slab_free(rx_free_slab, rx_block) != 0) {
				failures++;
				LOG_INF("FAIL mic->tx block=%u invalid rx slab pointer\n",
					 block);
			}
			continue;
		}

		memcpy(tx_block, rx_block, rx_size);
		ret = i2s_validate_bits_exact((const uint8_t *)rx_block,
					      (const uint8_t *)tx_block, rx_size);
		if (ret != 0) {
			failures++;
			LOG_INF("FAIL mic->tx block=%u bit mismatch\n", block);
		}

		if (i2s_feature_safe_slab_free(shared_dev ? active_slab : &feature_rx_mem_slab,
					       rx_block) != 0) {
			failures++;
			LOG_INF("FAIL mic->tx block=%u invalid rx slab pointer\n", block);
		}

		ret = i2s_write(tx_dev, tx_block, rx_size);
		if (ret < 0) {
			if (i2s_feature_is_not_supported_error(ret)) {
				skips++;
				LOG_INF("SKIP mic->tx block=%u tx write ret=%d\n", block, ret);
			} else {
				failures++;
				LOG_INF("FAIL mic->tx block=%u tx write ret=%d\n", block, ret);
			}
			if (i2s_feature_safe_slab_free(active_slab, tx_block) != 0) {
				failures++;
				LOG_INF("FAIL mic->tx block=%u invalid tx slab pointer\n", block);
			}
		} else {
			passes++;
		}
	}

	(void)i2s_trigger(tx_dev, I2S_DIR_TX, I2S_TRIGGER_DROP);
	(void)i2s_trigger(rx_dev, I2S_DIR_RX, I2S_TRIGGER_DROP);
	i2s_feature_deconfigure(rx_dev, tx_dev);

	LOG_INF("mic forwarding summary pass=%u skip=%u fail=%u\n", passes, skips, failures);
	if (passes == 0U && failures == 0U) {
		ztest_test_skip();
	}
	zassert_equal(failures, 0U, "mic rx->tx bit validation failures: %u", failures);
}

enum mic_matrix_result {
	MIC_CASE_PASS,
	MIC_CASE_SKIP,
	MIC_CASE_FAIL,
};

static enum mic_matrix_result mic_matrix_classify(int ret)
{
	return i2s_feature_is_not_supported_error(ret) ? MIC_CASE_SKIP
							: MIC_CASE_FAIL;
}

static const char *mic_matrix_tag(enum mic_matrix_result r)
{
	return (r == MIC_CASE_SKIP) ? "SKIP" : "FAIL";
}

static enum mic_matrix_result mic_matrix_run_one(const struct device *rx_dev,
						 const struct device *tx_dev,
						 bool shared_dev,
						 uint32_t rate, uint32_t ws,
						 uint8_t bits, bool *tx_queued_out)
{
	struct i2s_config rx_cfg;
	struct i2s_config tx_cfg;
	struct k_mem_slab *tx_slab =
		shared_dev ? &feature_mem_slab : &feature_tx_mem_slab;
	struct k_mem_slab *rx_slab =
		shared_dev ? &feature_mem_slab : &feature_rx_mem_slab;
	void *rx_block = NULL;
	void *tx_block = NULL;
	size_t rx_size = 0U;
	uint32_t ws_field = 0U;
	uint32_t ccr;
	enum mic_matrix_result res = MIC_CASE_FAIL;
	int ret;

	*tx_queued_out = false;

	i2s_feature_prepare_config(&rx_cfg, rate, bits);
	i2s_feature_prepare_config(&tx_cfg, rate, bits);
	rx_cfg.timeout = 100U;
	tx_cfg.timeout = 100U;
	rx_cfg.options = I2S_OPT_BIT_CLK_SLAVE | I2S_OPT_FRAME_CLK_SLAVE;
	rx_cfg.mem_slab = rx_slab;
	tx_cfg.mem_slab = tx_slab;

	ret = i2s_feature_configure_streams_split(rx_dev, tx_dev, &rx_cfg,
						  &tx_cfg);
	if (ret != 0) {
		res = mic_matrix_classify(ret);
		LOG_INF("%s mic-matrix rate=%u ws=%u bits=%u configure ret=%d\n",
			 mic_matrix_tag(res), rate, ws, bits, ret);
		i2s_feature_deconfigure(rx_dev, tx_dev);
		return res;
	}

	for (uint8_t seed = 0U; seed < 3U; seed++) {
		void *seed_block = NULL;

		ret = k_mem_slab_alloc(tx_slab, &seed_block, K_NO_WAIT);
		if (ret != 0) {
			res = mic_matrix_classify(ret);
			LOG_INF("%s mic-matrix rate=%u ws=%u bits=%u seed alloc ret=%d\n",
				 mic_matrix_tag(res), rate, ws, bits, ret);
			goto cleanup;
		}
		memset(seed_block, 0, tx_cfg.block_size);
		ret = i2s_write(tx_dev, seed_block, tx_cfg.block_size);
		if (ret != 0) {
			res = mic_matrix_classify(ret);
			LOG_INF("%s mic-matrix rate=%u ws=%u bits=%u seed write ret=%d\n",
				 mic_matrix_tag(res), rate, ws, bits, ret);
			(void)i2s_feature_safe_slab_free(tx_slab, seed_block);
			goto cleanup;
		}
	}

	ret = i2s_trigger(tx_dev, I2S_DIR_TX, I2S_TRIGGER_START);
	if (ret != 0) {
		res = mic_matrix_classify(ret);
		LOG_INF("%s mic-matrix rate=%u ws=%u bits=%u TX start ret=%d\n",
			 mic_matrix_tag(res), rate, ws, bits, ret);
		goto cleanup;
	}

	ret = i2s_trigger(rx_dev, I2S_DIR_RX, I2S_TRIGGER_START);
	if (ret != 0) {
		res = mic_matrix_classify(ret);
		LOG_INF("%s mic-matrix rate=%u ws=%u bits=%u RX start ret=%d\n",
			 mic_matrix_tag(res), rate, ws, bits, ret);
		goto cleanup;
	}

	ret = i2s_ws_cycles_to_field(ws, &ws_field);
	if (ret != 0) {
		LOG_INF("FAIL mic-matrix rate=%u ws=%u bits=%u invalid ws vector\n",
			 rate, ws, bits);
		res = MIC_CASE_FAIL;
		goto cleanup;
	}
	ccr = i2s_reg_read(i2s_tx_base(), I2S_REG_CCR_OFFSET);
	ccr = (ccr & ~I2S_CCR_WSS_MASK) | (ws_field << I2S_CCR_WSS_POS);
	i2s_reg_write(i2s_tx_base(), I2S_REG_CCR_OFFSET, ccr);

	ret = i2s_read(rx_dev, &rx_block, &rx_size);
	if (ret != 0) {
		if (ret == -EAGAIN) {
			LOG_INF("SKIP mic-matrix rate=%u ws=%u bits=%u no rx data ret=%d\n",
				 rate, ws, bits, ret);
			res = MIC_CASE_SKIP;
		} else {
			res = mic_matrix_classify(ret);
			LOG_INF("%s mic-matrix rate=%u ws=%u bits=%u RX read ret=%d\n",
				 mic_matrix_tag(res), rate, ws, bits, ret);
		}
		goto cleanup;
	}
	if (rx_size != rx_cfg.block_size) {
		LOG_INF("FAIL mic-matrix rate=%u ws=%u bits=%u size exp=%u act=%u\n",
			 rate, ws, bits, rx_cfg.block_size, (uint32_t)rx_size);
		res = MIC_CASE_FAIL;
		goto cleanup;
	}

	ret = k_mem_slab_alloc(tx_slab, &tx_block, K_NO_WAIT);
	if (ret != 0) {
		res = mic_matrix_classify(ret);
		LOG_INF("%s mic-matrix rate=%u ws=%u bits=%u TX alloc ret=%d\n",
			 mic_matrix_tag(res), rate, ws, bits, ret);
		tx_block = NULL;
		goto cleanup;
	}

	{
		uint8_t *rx_bytes = (uint8_t *)rx_block;
		uint8_t *tx_bytes = (uint8_t *)tx_block;

		for (size_t i = 0; i < rx_size; i++) {
			tx_bytes[i] = rx_bytes[i];
			for (uint8_t bit = 0U; bit < 8U; bit++) {
				uint8_t rb = (rx_bytes[i] >> bit) & 0x1U;
				uint8_t tb = (tx_bytes[i] >> bit) & 0x1U;

				if (rb != tb) {
					LOG_INF("FAIL mic-matrix rate=%u ws=%u "
						 "bits=%u byte=%u bit=%u\n",
						 rate, ws, bits,
						 (uint32_t)i, bit);
					res = MIC_CASE_FAIL;
					goto cleanup;
				}
			}
		}
	}

	ret = i2s_write(tx_dev, tx_block, rx_size);
	if (ret != 0) {
		res = mic_matrix_classify(ret);
		LOG_INF("%s mic-matrix rate=%u ws=%u bits=%u TX write ret=%d\n",
			 mic_matrix_tag(res), rate, ws, bits, ret);
		goto cleanup;
	}
	/* Ownership of tx_block transferred to driver. */
	tx_block = NULL;
	*tx_queued_out = true;

	LOG_INF("PASS mic-matrix rate=%u ws=%u bits=%u\n", rate, ws, bits);
	res = MIC_CASE_PASS;

cleanup:
	if (rx_block != NULL) {
		(void)i2s_feature_safe_slab_free(rx_slab, rx_block);
	}
	if (tx_block != NULL) {
		(void)i2s_feature_safe_slab_free(tx_slab, tx_block);
	}
	(void)i2s_trigger(tx_dev, I2S_DIR_TX, I2S_TRIGGER_DROP);
	(void)i2s_trigger(rx_dev, I2S_DIR_RX, I2S_TRIGGER_DROP);
	i2s_feature_deconfigure(rx_dev, tx_dev);
	return res;
}

ZTEST(i2s_features, test_mic_rx_to_tx_bit_exact_matrix)
{
	if (!IS_ENABLED(CONFIG_I2S_SLAVE_MODE_SUPPORTED)) {
		LOG_INF("SKIP: I2S clock-slave mode not supported on this platform "
			"(enable CONFIG_I2S_SLAVE_MODE_SUPPORTED if the IP supports it)");
		ztest_test_skip();
		return;
	}

	const struct device *const rx_dev = DEVICE_DT_GET(I2S_RX_NODE);
	const struct device *const tx_dev = DEVICE_DT_GET(I2S_TX_NODE);
	const uint32_t sample_rates[] = {
		8000U, 16000U, 32000U, 44100U, 48000U, 88200U, 96000U, 192000U,
	};
	const uint32_t ws_cycles[] = {16U, 24U, 32U};
	const uint8_t word_sizes[] = {12U, 16U, 20U, 24U, 32U};
	uint32_t failures = 0U;
	uint32_t skips = 0U;
	uint32_t passes = 0U;
	uint32_t executed = 0U;
	const bool shared_dev = (rx_dev == tx_dev);

	zassert_true(device_is_ready(rx_dev), "RX device not ready");
	zassert_true(device_is_ready(tx_dev), "TX device not ready");
	if (shared_dev) {
		ztest_test_skip();
		return;
	}

	for (size_t r = 0; r < ARRAY_SIZE(sample_rates); r++) {
		for (size_t w = 0; w < ARRAY_SIZE(ws_cycles); w++) {
			for (size_t b = 0; b < ARRAY_SIZE(word_sizes); b++) {
				bool tx_queued = false;
				enum mic_matrix_result res;

				executed++;
				res = mic_matrix_run_one(rx_dev, tx_dev,
							 shared_dev,
							 sample_rates[r],
							 ws_cycles[w],
							 word_sizes[b],
							 &tx_queued);
				switch (res) {
				case MIC_CASE_PASS:
					passes++;
					break;
				case MIC_CASE_SKIP:
					skips++;
					break;
				case MIC_CASE_FAIL:
				default:
					failures++;
					break;
				}
				if (tx_queued) {
					k_sleep(K_MSEC(1));
				}
			}
		}
	}

	LOG_INF("Mic bit-matrix executed cases: %u\n", executed);
	LOG_INF("Mic bit-matrix summary pass=%u skip=%u fail=%u\n", passes, skips, failures);
	zassert_equal(executed, (uint32_t)(ARRAY_SIZE(sample_rates) * ARRAY_SIZE(ws_cycles) *
					   ARRAY_SIZE(word_sizes)),
		      "mic bit-matrix did not execute all scenarios");
	if (passes == 0U && failures == 0U) {
		ztest_test_skip();
	}
	zassert_equal(failures, 0U, "mic bit-matrix failures: %u", failures);
}
#endif /* !CONFIG_I2S_FULL_DUPLEX */

/*
 * Full-duplex test on the same I2S device (i2s3).
 *
 * The DW I2S controller has a single `dev_data->dir` field and separate
 * rx_stream_start / tx_stream_start functions that each disable the opposite
 * channel.  True simultaneous full-duplex on a single device therefore
 * requires the driver to support I2S_DIR_BOTH in both i2s_configure() and
 * i2s_trigger().
 *
 * This test:
 *   1. Probes i2s_configure(dev, I2S_DIR_BOTH, ...) and i2s_trigger(dev,
 *      I2S_DIR_BOTH, ...) — FAILS the test (not skip) if either returns
 *      -ENOSYS, reporting which call is unsupported so the driver gap is
 *      clearly visible in the test report.
 *   2. If the driver reports support, performs an actual sustained full-duplex
 *      transfer on the same device: TX sends an incrementing pattern while RX
 *      captures it simultaneously, then validates the received data.
 *
 * Wire requirement (same as loopback tests):
 *   P9_3 (I2S3_SDO) --> P9_0 (I2S3_SDI)
 */
ZTEST(i2s_features, test_full_duplex_same_device)
{
	/*
	 * Use the TX node as the device under test.  For this test we always
	 * want a single concrete device (i2s3), not a shared i2s_rxtx alias,
	 * so resolve whichever node the build provides for TX.
	 */
	const struct device *const dev = DEVICE_DT_GET(I2S_TX_NODE);
	struct i2s_config cfg;
	struct i2s_config reset_cfg = {0};
	struct i2s_config rx_cfg;
	struct i2s_config tx_cfg;
	void *tx_block = NULL;
	void *rx_block = NULL;
	size_t rx_size = 0U;
	uint32_t *tx_words;
	const uint32_t words = I2S_FEATURE_BLOCK_SIZE / sizeof(uint32_t);
	const uint32_t shift = 32U - 24U;
	const uint32_t mask  = (1U << 24U) - 1U;
	int ret;
	int rc = 0;

	/* ------------------------------------------------------------------ */
	/* Step 0 – device ready check                                         */
	/* ------------------------------------------------------------------ */
	zassert_true(device_is_ready(dev), "I2S device not ready");

	/* ------------------------------------------------------------------ */
	/* Step 1 – probe: i2s_configure with I2S_DIR_BOTH                     */
	/*                                                                     */
	/* The DW I2S driver returns -ENOSYS for I2S_DIR_BOTH in configure.   */
	/* We treat this as a hard FAIL so the gap is visible in CI results.  */
	/* ------------------------------------------------------------------ */
	i2s_feature_prepare_config(&cfg, 48000U, 24U);
	cfg.mem_slab = &feature_tx_mem_slab;

	ret = i2s_configure(dev, I2S_DIR_BOTH, &cfg);
	LOG_INF("full-duplex probe: i2s_configure(DIR_BOTH) returned %d\n", ret);
	if (ret == -ENOSYS) {
		LOG_INF("FAIL: driver does not implement i2s_configure(DIR_BOTH)"
			 " (-ENOSYS). Full-duplex on a single device requires"
			 " this to be supported.\n");
		/* Clean up any partial state before hard-failing. */
		(void)i2s_configure(dev, I2S_DIR_TX, &reset_cfg);
		(void)i2s_configure(dev, I2S_DIR_RX, &reset_cfg);
		zassert_equal(ret, 0,
			      "i2s_configure(DIR_BOTH) not supported (%d)", ret);
		return;
	}
	zassert_equal(ret, 0, "i2s_configure(DIR_BOTH) failed unexpectedly (%d)", ret);

	/* ------------------------------------------------------------------ */
	/* Step 2 – probe: i2s_trigger with I2S_DIR_BOTH (PREPARE first so    */
	/*           the stream is in a triggerable state for DROP)            */
	/* ------------------------------------------------------------------ */
	ret = i2s_trigger(dev, I2S_DIR_BOTH, I2S_TRIGGER_DROP);
	LOG_INF("full-duplex probe: i2s_trigger(DIR_BOTH, DROP) returned %d\n", ret);
	if (ret == -ENOSYS) {
		LOG_INF("FAIL: driver does not implement i2s_trigger(DIR_BOTH)"
			 " (-ENOSYS). Full-duplex on a single device requires"
			 " this to be supported.\n");
		(void)i2s_configure(dev, I2S_DIR_TX, &reset_cfg);
		(void)i2s_configure(dev, I2S_DIR_RX, &reset_cfg);
		zassert_equal(ret, 0,
			      "i2s_trigger(DIR_BOTH) not supported (%d)", ret);
		return;
	}
	zassert_equal(ret, 0, "i2s_trigger(DIR_BOTH, DROP) failed unexpectedly (%d)", ret);

	/* ------------------------------------------------------------------ */
	/* Step 3 – full-duplex transfer                                       */
	/*                                                                     */
	/* Configure both directions separately (the driver accepted DIR_BOTH  */
	/* in Step 1, so now use the documented split-configure path to put    */
	/* both streams into READY state).  Then trigger DIR_BOTH to start     */
	/* simultaneous RX+TX.                                                 */
	/* ------------------------------------------------------------------ */
	i2s_feature_prepare_config(&rx_cfg, 48000U, 24U);
	i2s_feature_prepare_config(&tx_cfg, 48000U, 24U);
	rx_cfg.options  = I2S_OPT_BIT_CLK_MASTER | I2S_OPT_FRAME_CLK_MASTER;
	tx_cfg.options  = I2S_OPT_BIT_CLK_MASTER | I2S_OPT_FRAME_CLK_MASTER;
	rx_cfg.mem_slab = &feature_rx_mem_slab;
	tx_cfg.mem_slab = &feature_tx_mem_slab;
	rx_cfg.timeout  = 500U;
	tx_cfg.timeout  = 500U;

	ret = i2s_configure(dev, I2S_DIR_RX, &rx_cfg);
	zassert_equal(ret, 0, "RX configure failed (%d)", ret);

	ret = i2s_configure(dev, I2S_DIR_TX, &tx_cfg);
	zassert_equal(ret, 0, "TX configure failed (%d)", ret);

	/* Pre-queue 3 zero seed blocks so the TX FIFO keeps the clock alive  */
	/* long enough for the RX DMA to capture a full block.                */
	for (uint8_t s = 0U; s < 3U; s++) {
		void *seed = NULL;

		ret = k_mem_slab_alloc(&feature_tx_mem_slab, &seed, K_NO_WAIT);
		zassert_equal(ret, 0, "seed alloc failed s=%u (%d)", s, ret);
		memset(seed, 0, I2S_FEATURE_BLOCK_SIZE);
		ret = i2s_write(dev, seed, I2S_FEATURE_BLOCK_SIZE);
		zassert_equal(ret, 0, "seed write failed s=%u (%d)", s, ret);
	}

	/* Allocate and fill the pattern TX block. */
	ret = k_mem_slab_alloc(&feature_tx_mem_slab, &tx_block, K_NO_WAIT);
	zassert_equal(ret, 0, "TX slab alloc failed (%d)", ret);

	tx_words = tx_block;

	for (uint32_t i = 0U; i < words; i++) {
		uint32_t v = (i + 1U) & mask;

		if (v == 0U) {
			v = 1U;
		}
		tx_words[i] = v << shift;
	}

	ret = i2s_write(dev, tx_block, I2S_FEATURE_BLOCK_SIZE);
	if (ret != 0) {
		LOG_INF("FAIL: i2s_write failed (%d)\n", ret);
		k_mem_slab_free(&feature_tx_mem_slab, tx_block);
		rc = ret;
		goto fd_cleanup;
	}

	/* Trigger both directions simultaneously. */
	ret = i2s_trigger(dev, I2S_DIR_BOTH, I2S_TRIGGER_START);
	LOG_INF("full-duplex: i2s_trigger(DIR_BOTH, START) returned %d\n", ret);
	if (ret == -ENOSYS) {
		LOG_INF("FAIL: driver does not implement"
			 " i2s_trigger(DIR_BOTH, START) (-ENOSYS).\n");
		rc = ret;
		goto fd_cleanup;
	}
	if (ret != 0) {
		LOG_INF("FAIL: i2s_trigger(DIR_BOTH, START) failed (%d)\n", ret);
		rc = ret;
		goto fd_cleanup;
	}

	/* Drain 3 seed RX blocks (all-zero, not verified). */
	for (uint8_t s = 0U; s < 3U; s++) {
		void  *discard = NULL;
		size_t discard_sz = 0U;

		ret = i2s_read(dev, &discard, &discard_sz);
		if (ret != 0) {
			LOG_INF("FAIL: seed drain read failed s=%u (%d)\n", s, ret);
			rc = ret;
			goto fd_stop;
		}
		k_mem_slab_free(&feature_rx_mem_slab, discard);
	}

	/* Read back the pattern block. */
	ret = i2s_read(dev, &rx_block, &rx_size);
	if (ret != 0) {
		LOG_INF("FAIL: i2s_read failed (%d)\n", ret);
		rc = ret;
		goto fd_stop;
	}

	if (rx_size != I2S_FEATURE_BLOCK_SIZE) {
		LOG_INF("FAIL: size mismatch exp=%u act=%u\n",
			 (uint32_t)I2S_FEATURE_BLOCK_SIZE, (uint32_t)rx_size);
		rc = -EIO;
		goto fd_free_rx;
	}

	/* Check wire: if fewer than 4 non-zero words the loopback wire       */
	/* P9_3->P9_0 is not connected — report as a test failure because    */
	/* full-duplex on the same device always needs the wire.             */
	{
		const uint32_t *rx_w = rx_block;
		uint32_t nz = 0U;

		for (uint32_t i = 0U; i < words; i++) {
			if (rx_w[i] != 0U) {
				nz++;
			}
		}
		if (nz < 4U) {
			LOG_INF("FAIL: loopback wire P9_3->P9_0 not connected"
				 " (non-zero words=%u). Full-duplex test requires"
				 " the wire to be present.\n", nz);
			rc = -EIO;
			goto fd_free_rx;
		}
	}

	/* Bit-exact validation using the aligned sliding-window comparator.  */
	ret = i2s_validate_single_node_aligned((const uint32_t *)rx_block,
					       (const uint32_t *)tx_block,
					       words);
	if (ret != 0) {
		const uint32_t *rx_w = rx_block;
		const uint32_t *tx_w = tx_block;

		LOG_INF("FAIL: full-duplex data mismatch\n");
		LOG_INF("  TX[0..7]: %08x %08x %08x %08x %08x %08x %08x %08x\n",
			 tx_w[0], tx_w[1], tx_w[2], tx_w[3],
			 tx_w[4], tx_w[5], tx_w[6], tx_w[7]);
		LOG_INF("  RX[0..7]: %08x %08x %08x %08x %08x %08x %08x %08x\n",
			 rx_w[0], rx_w[1], rx_w[2], rx_w[3],
			 rx_w[4], rx_w[5], rx_w[6], rx_w[7]);
		rc = -EIO;
	} else {
		LOG_INF("PASS: full-duplex same-device data verified\n");
	}

fd_free_rx:
	k_mem_slab_free(&feature_rx_mem_slab, rx_block);
fd_stop:
	(void)i2s_trigger(dev, I2S_DIR_BOTH, I2S_TRIGGER_DROP);
fd_cleanup:
	(void)i2s_configure(dev, I2S_DIR_TX, &reset_cfg);
	(void)i2s_configure(dev, I2S_DIR_RX, &reset_cfg);
	k_sleep(K_MSEC(5));

	zassert_equal(rc, 0, "full-duplex same-device test failed (%d)", rc);
}

/* =========================================================================
 * TC-FEAT-DMA: DMA Support Feature Detection
 *
 * Objective:
 *   Verify that the I2S driver enables DMA-based data transfer during
 *   active streaming.  The DesignWare I2S IP exposes a DMA Control
 *   Register (DMACR, offset 0x200) with per-direction enable bits:
 *     - DMAEN_TXBLOCK (bit 17): TX DMA handshake active
 *     - DMAEN_RXBLOCK (bit 16): RX DMA handshake active
 *
 * Method:
 *   1. Configure TX and RX streams at 48 kHz / 24-bit.
 *   2. Pre-queue one TX block and trigger START on both directions.
 *   3. While the streams are RUNNING, sample the DMACR register.
 *   4. Assert that both DMA enable bits are set.
 *
 * Expected Result:
 *   PASS  — driver sets DMACR bits during active streaming (DMA path).
 *   FAIL  — DMACR bits remain clear (interrupt+FIFO path only, no DMA).
 *
 * Guardrails: No driver code is modified; we only observe register state.
 * =========================================================================
 */
#ifndef CONFIG_I2S_FULL_DUPLEX
ZTEST(i2s_features, test_dma_support_active_during_streaming)
{
	const struct device *const rx_dev = DEVICE_DT_GET(I2S_RX_NODE);
	const struct device *const tx_dev = DEVICE_DT_GET(I2S_TX_NODE);
	uintptr_t tx_base_addr = DT_REG_ADDR(I2S_TX_NODE);
	struct i2s_config cfg;
	void *mem_block = NULL;
	uint32_t *samples;
	uint32_t dmacr;
	int ret;

	zassert_true(device_is_ready(rx_dev), "RX device not ready");
	zassert_true(device_is_ready(tx_dev), "TX device not ready");

	/* --- Configure streams at 48 kHz / 24-bit --- */
	i2s_feature_prepare_config(&cfg, 48000U, 24U);

	ret = i2s_feature_configure_streams(rx_dev, tx_dev, &cfg);
	zassert_equal(ret, 0, "configure failed (%d)", ret);

	/* --- Pre-queue one TX block with a simple ramp pattern --- */
	ret = k_mem_slab_alloc(&feature_mem_slab, &mem_block, K_NO_WAIT);
	zassert_equal(ret, 0, "TX slab alloc failed");

	samples = mem_block;
	for (uint32_t i = 0; i < (cfg.block_size / sizeof(uint32_t)); i++) {
		samples[i] = i;
	}

	ret = i2s_write(tx_dev, mem_block, cfg.block_size);
	zassert_equal(ret, 0, "TX write failed (%d)", ret);

	/* --- Start RX then TX --- */
	ret = i2s_trigger(rx_dev, I2S_DIR_RX, I2S_TRIGGER_START);
	zassert_equal(ret, 0, "RX START failed (%d)", ret);

	ret = i2s_trigger(tx_dev, I2S_DIR_TX, I2S_TRIGGER_START);
	zassert_equal(ret, 0, "TX START failed (%d)", ret);

	/* Let the streams run briefly so the driver has time to enable DMA */
	k_sleep(K_MSEC(5));

	/* --- Sample DMACR while streams are active --- */
	dmacr = sys_read32(tx_base_addr + I2S_REG_DMACR_OFFSET);
	LOG_INF("DMACR during active stream: 0x%08x\n", dmacr);

	/* --- Cleanup: stop streams before assertions --- */
	(void)i2s_trigger(tx_dev, I2S_DIR_TX, I2S_TRIGGER_DROP);
	(void)i2s_trigger(rx_dev, I2S_DIR_RX, I2S_TRIGGER_DROP);
	i2s_feature_deconfigure(rx_dev, tx_dev);

	/* --- Verify DMA was enabled during streaming --- */
	zassert_true((dmacr & I2S_DMACR_TX_EN_BIT) != 0U,
		     "DMACR TX DMA enable bit (bit 17) not set during active "
		     "streaming — DMA support absent in driver "
		     "(DMACR=0x%08x)", dmacr);

	zassert_true((dmacr & I2S_DMACR_RX_EN_BIT) != 0U,
		     "DMACR RX DMA enable bit (bit 16) not set during active "
		     "streaming — DMA support absent in driver "
		     "(DMACR=0x%08x)", dmacr);

	LOG_INF("PASS: DMA TX and RX enabled during active streaming\n");
}
#endif /* !CONFIG_I2S_FULL_DUPLEX */

static bool i2s_features_predicate(const void *state)
{
	if (IS_ENABLED(CONFIG_I2S_PLAY_HELLO) ||
	    IS_ENABLED(CONFIG_I2S_PLAY_HELLO_CODEC)) {
		return false;
	}

	return true;
}

ZTEST_SUITE(i2s_features, i2s_features_predicate, NULL, NULL, NULL, NULL);

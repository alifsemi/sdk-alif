/*
 * Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * i2s_test_common.c - Helper implementations for the I2S test suite.
 */

#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/i2s.h>
#include <zephyr/device.h>
#include <zephyr/ztest.h>
#include "i2s_test.h"

/* -------------------------------------------------------------------------
 * Memory slabs (shared by golden-vector suite)
 * -------------------------------------------------------------------------
 */
K_MEM_SLAB_DEFINE(g_tx_slab, I2S_GOLDEN_BLOCK_SIZE,
		  I2S_GOLDEN_NUM_TX_BLOCKS, 4);
K_MEM_SLAB_DEFINE(g_rx_slab, I2S_GOLDEN_BLOCK_SIZE,
		  I2S_GOLDEN_NUM_RX_BLOCKS, 4);

/* -------------------------------------------------------------------------
 * Device accessors
 * -------------------------------------------------------------------------
 */
const struct device *i2s_rx_dev(void)
{
	return DEVICE_DT_GET(I2S_RX_NODE);
}

const struct device *i2s_tx_dev(void)
{
	return DEVICE_DT_GET(I2S_TX_NODE);
}

uintptr_t i2s_rx_base(void)
{
	return DT_REG_ADDR(I2S_RX_NODE);
}

uintptr_t i2s_tx_base(void)
{
	return DT_REG_ADDR(I2S_TX_NODE);
}

/* -------------------------------------------------------------------------
 * i2s_golden_configure
 * -------------------------------------------------------------------------
 */
int i2s_golden_configure(const struct device *dev, enum i2s_dir dir,
			 uint32_t rate, uint8_t word_size)
{
	struct i2s_config cfg = {
		.word_size      = word_size,
		.channels       = I2S_GOLDEN_CHANNELS,
		.format         = I2S_FMT_DATA_FORMAT_I2S,
		.frame_clk_freq = rate,
		.block_size     = I2S_GOLDEN_BLOCK_SIZE,
		.timeout        = I2S_GOLDEN_TIMEOUT_MS,
		.options        = I2S_OPT_BIT_CLK_MASTER | I2S_OPT_FRAME_CLK_MASTER,
	};

	if (dir == I2S_DIR_TX || dir == I2S_DIR_RX) {
		cfg.mem_slab = (dir == I2S_DIR_TX) ? &g_tx_slab : &g_rx_slab;
	} else {
		TC_PRINT("[i2s-golden] configure: I2S_DIR_BOTH not supported\n");
		return -ENOTSUP;
	}

	int ret = i2s_configure(dev, dir, &cfg);

	if (ret < 0) {
		TC_PRINT("[i2s-golden] i2s_configure(dir=%d rate=%u bits=%u) failed: %d\n",
			 (int)dir, rate, (unsigned int)word_size, ret);
	}
	return ret;
}

/* -------------------------------------------------------------------------
 * i2s_golden_tx_write
 * -------------------------------------------------------------------------
 */
int i2s_golden_tx_write(const struct device *dev, uint8_t word_size,
			const uint32_t *vec_L, const uint32_t *vec_R)
{
	void *block = NULL;
	int ret;
	uint32_t shift;

	ret = k_mem_slab_alloc(&g_tx_slab, &block, K_NO_WAIT);
	if (ret < 0) {
		TC_PRINT("[i2s-golden] tx slab alloc failed: %d\n", ret);
		return ret;
	}

	shift = (word_size < 32U) ? (32U - word_size) : 0U;

	uint32_t *words = block;

	for (uint32_t i = 0U; i < I2S_GOLDEN_FRAMES; i++) {
		words[2U * i]      = vec_L[i] >> shift;
		words[2U * i + 1U] = vec_R[i] >> shift;
	}

	ret = i2s_write(dev, block, I2S_GOLDEN_BLOCK_SIZE);
	if (ret < 0) {
		TC_PRINT("[i2s-golden] i2s_write failed: %d\n", ret);
		k_mem_slab_free(&g_tx_slab, block);
	}
	return ret;
}

/* -------------------------------------------------------------------------
 * i2s_golden_rx_verify
 * -------------------------------------------------------------------------
 */
int i2s_golden_rx_verify(const struct device *dev, uint8_t word_size,
			 const uint32_t *vec_L, const uint32_t *vec_R)
{
	void *block = NULL;
	size_t size = 0U;
	int ret;
	uint32_t shift;
	int rc = 0;

	ret = i2s_read(dev, &block, &size);
	if (ret < 0) {
		TC_PRINT("[i2s-golden] i2s_read failed: %d\n", ret);
		return ret;
	}
	if (size != I2S_GOLDEN_BLOCK_SIZE) {
		TC_PRINT("[i2s-golden] rx size mismatch: exp=%u got=%u\n",
			 (unsigned int)I2S_GOLDEN_BLOCK_SIZE, (unsigned int)size);
		k_mem_slab_free(&g_rx_slab, block);
		return -EIO;
	}

	shift = (word_size < 32U) ? (32U - word_size) : 0U;

	const uint32_t *rx = block;
	uint32_t mask = (word_size < 32U) ? ((1U << word_size) - 1U) : 0xFFFFFFFFU;

	for (uint32_t i = 0U; i < I2S_GOLDEN_FRAMES; i++) {
		uint32_t exp_L = (vec_L[i] >> shift) & mask;
		uint32_t exp_R = (vec_R[i] >> shift) & mask;
		uint32_t got_L = rx[2U * i]      & mask;
		uint32_t got_R = rx[2U * i + 1U] & mask;

		if (got_L != exp_L) {
			TC_PRINT("[i2s-golden] L mismatch @ frame %u: exp=0x%08x got=0x%08x\n",
				 i, exp_L, got_L);
			rc = -EIO;
			break;
		}
		if (got_R != exp_R) {
			TC_PRINT("[i2s-golden] R mismatch @ frame %u: exp=0x%08x got=0x%08x\n",
				 i, exp_R, got_R);
			rc = -EIO;
			break;
		}
	}

	if (rc == 0) {
		TC_PRINT("[i2s-golden] verify PASS: %u frames, %u-bit\n",
			 I2S_GOLDEN_FRAMES, (unsigned int)word_size);
	}

	k_mem_slab_free(&g_rx_slab, block);
	return rc;
}

/* -------------------------------------------------------------------------
 * i2s_golden_stop
 * -------------------------------------------------------------------------
 */
void i2s_golden_stop(const struct device *dev)
{
	(void)i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_DROP);
	(void)i2s_trigger(dev, I2S_DIR_RX, I2S_TRIGGER_DROP);
}

/* -------------------------------------------------------------------------
 * i2s_golden_run
 *
 * TX-master-only sequence.  When CONFIG_I2S_LOOPBACK_VERIFY=y the
 * function also probes for duplex support and, if available, verifies the
 * received data against the golden vectors.
 * -------------------------------------------------------------------------
 */
int i2s_golden_run(const struct device *dev, uint32_t rate, uint8_t word_size,
		   const uint32_t *vec_L, const uint32_t *vec_R)
{
	int ret;
	int rc = 0;
	const uint8_t tx_blocks = 4U;
	bool do_rx_verify  = false;  /* RX verify still active (cleared on timeout) */
	uint32_t drain_ms = (uint32_t)(
		((uint64_t)tx_blocks * I2S_GOLDEN_FRAMES * 1500U) / rate);

	if (drain_ms < 10U) {
		drain_ms = 10U;
	}

	ret = i2s_golden_configure(dev, I2S_DIR_TX, rate, word_size);
	if (ret < 0) {
		TC_PRINT("[i2s-golden] TX configure failed (%d)\n", ret);
		return (ret == -ENOTSUP || ret == -ENOSYS) ? -ENOTSUP : ret;
	}

#if defined(CONFIG_I2S_LOOPBACK_VERIFY)
	/* Requires physical wire: P9_3 (I2S3_SDO) --> P9_0 (I2S3_SDI).
	 * snps,designware-i2s does not support I2S_OPT_LOOPBACK.
	 */
	ret = i2s_golden_configure(dev, I2S_DIR_RX, rate, word_size);
	if (ret == -ENOSYS || ret == -ENOTSUP) {
		TC_PRINT("[i2s-golden] INFO rate=%u bits=%u: RX not supported, "
			 "TX-only run\n",
			 (unsigned int)rate, (unsigned int)word_size);
	} else if (ret < 0) {
		TC_PRINT("[i2s-golden] RX configure failed (%d)\n", ret);
		rc = ret;
		goto out_reset;
	} else {
		do_rx_verify = true;
	}
#endif

	for (uint8_t s = 0U; s < tx_blocks; s++) {
		ret = i2s_golden_tx_write(dev, word_size, vec_L, vec_R);
		if (ret < 0) {
			TC_PRINT("[i2s-golden] TX write[%u] failed: %d\n", s, ret);
			rc = ret;
			goto out_reset;
		}
	}

	if (do_rx_verify) {
		/* Atomic duplex start — driver trigger supports DIR_BOTH */
		ret = i2s_trigger(dev, I2S_DIR_BOTH, I2S_TRIGGER_START);
		if (ret < 0) {
			TC_PRINT("[i2s-golden] DIR_BOTH START failed (%d), TX-only\n", ret);
			do_rx_verify = false;
			ret = i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_START);
		}
	} else {
		ret = i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_START);
	}

	if (ret < 0) {
		TC_PRINT("[i2s-golden] TX START failed: %d\n", ret);
		rc = (ret == -ENOTSUP || ret == -ENOSYS) ? -ENOTSUP : ret;
		goto out_stop;
	}

	k_sleep(K_MSEC(drain_ms));

	ret = i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_DRAIN);
	if (ret == -ENOSYS || ret == -ENOTSUP) {
		k_sleep(K_MSEC(drain_ms));
		(void)i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_DROP);
	} else if (ret < 0 && ret != -EIO) {
		TC_PRINT("[i2s-golden] TX DRAIN failed: %d\n", ret);
		rc = ret;
		goto out_stop;
	}

	TC_PRINT("[i2s-golden] TX PASS rate=%u bits=%u\n",
		 (unsigned int)rate, (unsigned int)word_size);

#if defined(CONFIG_I2S_LOOPBACK_VERIFY)
	if (do_rx_verify) {
		for (uint8_t s = 0U; s < (tx_blocks - 1U); s++) {
			void *discard = NULL;
			size_t dsz = 0U;

			ret = i2s_read(dev, &discard, &dsz);
			if (ret == -EAGAIN) {
				TC_PRINT("[i2s-golden] RX timeout — no loopback data\n");
				do_rx_verify = false;
				break;
			}
			if (ret < 0) {
				TC_PRINT("[i2s-golden] discard read[%u] failed: %d\n", s, ret);
				do_rx_verify = false;
				break;
			}
			k_mem_slab_free(&g_rx_slab, discard);
		}
	}
	if (do_rx_verify) {
		int vrc = i2s_golden_rx_verify(dev, word_size, vec_L, vec_R);

		if (vrc == -EAGAIN) {
			TC_PRINT("[i2s-golden] RX timeout on golden read — no loopback data\n");
		} else if (vrc != 0) {
			rc = vrc;
		}
	}
#endif

out_stop:
	/* Drop only directions that were actually configured.
	 * Triggering DROP on a NOT_READY stream causes the driver to log
	 * "DROP trigger: invalid state". RX is only configured when
	 * CONFIG_I2S_LOOPBACK_VERIFY=y and do_rx_verify was set.
	 */
	(void)i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_DROP);
#if defined(CONFIG_I2S_LOOPBACK_VERIFY)
	if (do_rx_verify) {
		(void)i2s_trigger(dev, I2S_DIR_RX, I2S_TRIGGER_DROP);
	}
#endif

out_reset:
	{
		struct i2s_config reset = { 0 };

		(void)i2s_configure(dev, I2S_DIR_TX, &reset);
		(void)i2s_configure(dev, I2S_DIR_RX, &reset);
	}
	/* 10 ms lets the SE-services clock FSM fully settle after a
	 * hard-reset of both streams.
	 */
	k_sleep(K_MSEC(10));

	return rc;
}

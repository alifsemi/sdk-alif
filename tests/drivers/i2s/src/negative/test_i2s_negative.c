/*
 * Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 */

#include <string.h>
#include <zephyr/drivers/i2s.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/ztest.h>
#include "i2s_test.h"

LOG_MODULE_REGISTER(i2s_negative_test, LOG_LEVEL_INF);

K_THREAD_STACK_DEFINE(i2s_write_stack, 2048);
K_THREAD_STACK_DEFINE(i2s_read_stack, 2048);

#define I2S_LB_BLOCKS 8U
#define I2S_LB_RATE   8000U

/* -------------------------------------------------------------------------
 * Helper: reset TX (and RX) to NOT_READY state
 * -------------------------------------------------------------------------
 */
static void neg_reset_streams(const struct device *dev)
{
	struct i2s_config zero = { 0 };

	(void)i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_DROP);
	(void)i2s_trigger(dev, I2S_DIR_RX, I2S_TRIGGER_DROP);
	(void)i2s_configure(dev, I2S_DIR_TX, &zero);
	(void)i2s_configure(dev, I2S_DIR_RX, &zero);
	k_sleep(K_MSEC(2));
}

/* -------------------------------------------------------------------------
 * All triggers rejected in NOT_READY state
 * Tests both TX and RX directions.
 * -------------------------------------------------------------------------
 */
ZTEST(i2s_negative, test_neg_not_ready_triggers)
{
	const struct device *const dev = DEVICE_DT_GET(I2S_GOLDEN_NODE);
	int ret;

	zassert_true(device_is_ready(dev), "I2S device not ready");
	neg_reset_streams(dev);

	/* RX in NOT_READY */
	zassert_equal(i2s_trigger(dev, I2S_DIR_RX, I2S_TRIGGER_START),  -EIO,
		      "RX START in NOT_READY should return -EIO");
	zassert_equal(i2s_trigger(dev, I2S_DIR_RX, I2S_TRIGGER_DRAIN),  -EIO,
		      "RX DRAIN in NOT_READY should return -EIO");
	zassert_equal(i2s_trigger(dev, I2S_DIR_RX, I2S_TRIGGER_STOP),   -EIO,
		      "RX STOP in NOT_READY should return -EIO");
	zassert_equal(i2s_trigger(dev, I2S_DIR_RX, I2S_TRIGGER_DROP),   -EIO,
		      "RX DROP in NOT_READY should return -EIO");
	zassert_equal(i2s_trigger(dev, I2S_DIR_RX, I2S_TRIGGER_PREPARE), -EIO,
		      "RX PREPARE in NOT_READY should return -EIO");

	/* TX in NOT_READY */
	zassert_equal(i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_START),  -EIO,
		      "TX START in NOT_READY should return -EIO");
	zassert_equal(i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_DRAIN),  -EIO,
		      "TX DRAIN in NOT_READY should return -EIO");
	zassert_equal(i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_STOP),   -EIO,
		      "TX STOP in NOT_READY should return -EIO");
	zassert_equal(i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_DROP),   -EIO,
		      "TX DROP in NOT_READY should return -EIO");
	zassert_equal(i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_PREPARE), -EIO,
		      "TX PREPARE in NOT_READY should return -EIO");

	ret = i2s_golden_configure(dev, I2S_DIR_TX, 48000U, 16U);
	if (ret == -ENOTSUP || ret == -ENOSYS) {
		ztest_test_skip();
		return;
	}
	zassert_equal(ret, 0, "TX configure failed: %d", ret);

	struct i2s_config zero_freq_cfg = {
		.word_size      = 16U,
		.channels       = I2S_GOLDEN_CHANNELS,
		.format         = I2S_FMT_DATA_FORMAT_I2S,
		.frame_clk_freq = 0U,
		.block_size     = I2S_GOLDEN_BLOCK_SIZE,
		.timeout        = I2S_GOLDEN_TIMEOUT_MS,
		.options        = I2S_OPT_BIT_CLK_MASTER | I2S_OPT_FRAME_CLK_MASTER,
		.mem_slab       = &g_tx_slab,
	};

	ret = i2s_configure(dev, I2S_DIR_TX, &zero_freq_cfg);
	zassert_equal(ret, 0, "zero-freq configure failed: %d", ret);
	zassert_equal(i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_DROP), -EIO,
		      "DROP after zero-freq configure should fail");
}

/* -------------------------------------------------------------------------
 * DRAIN, STOP, PREPARE rejected in READY state
 * -------------------------------------------------------------------------
 */
ZTEST(i2s_negative, test_neg_ready_invalid_triggers)
{
	const struct device *const dev = DEVICE_DT_GET(I2S_GOLDEN_NODE);
	int ret;

	zassert_true(device_is_ready(dev), "I2S device not ready");
	neg_reset_streams(dev);

	/* Configure RX → READY */
	ret = i2s_golden_configure(dev, I2S_DIR_RX, 48000U, 16U);
	if (ret == -ENOTSUP || ret == -ENOSYS) {
		ztest_test_skip();
		return;
	}
	zassert_equal(ret, 0, "RX configure failed: %d", ret);

	zassert_equal(i2s_trigger(dev, I2S_DIR_RX, I2S_TRIGGER_DRAIN),   -EIO,
		      "RX DRAIN in READY should return -EIO");
	zassert_equal(i2s_trigger(dev, I2S_DIR_RX, I2S_TRIGGER_STOP),    -EIO,
		      "RX STOP in READY should return -EIO");
	zassert_equal(i2s_trigger(dev, I2S_DIR_RX, I2S_TRIGGER_PREPARE), -EIO,
		      "RX PREPARE in READY should return -EIO");

	/* Configure TX → READY */
	ret = i2s_golden_configure(dev, I2S_DIR_TX, 48000U, 16U);
	zassert_equal(ret, 0, "TX configure failed: %d", ret);

	zassert_equal(i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_DRAIN),   -EIO,
		      "TX DRAIN in READY should return -EIO");
	zassert_equal(i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_STOP),    -EIO,
		      "TX STOP in READY should return -EIO");
	zassert_equal(i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_PREPARE), -EIO,
		      "TX PREPARE in READY should return -EIO");

	zassert_equal(i2s_trigger(dev, I2S_DIR_RX, I2S_TRIGGER_DROP), 0,
		      "RX DROP in READY failed");
	zassert_equal(i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_DROP), 0,
		      "TX DROP in READY failed");
	neg_reset_streams(dev);
}

static bool i2s_nonzero(const void *block, size_t size)
{
	const uint8_t *p = block;
	uint32_t nz = 0U;

	for (size_t i = 0U; i < size; i++) {
		if (p[i] != 0U) {
			nz++;
		}
	}
	return nz >= 4U;
}

static uint32_t i2s_rx_count(const struct device *dev, uint32_t max_blocks)
{
	uint32_t count = 0U;

	for (uint32_t i = 0U; i < max_blocks; i++) {
		void *block = NULL;
		size_t size = 0U;
		int ret = i2s_read(dev, &block, &size);

		if (ret != 0) {
			break;
		}
		if (i2s_nonzero(block, size)) {
			count++;
		}
		k_mem_slab_free(&i2s_rx_slab, block);
	}
	return count;
}

static int i2s_arm_loopback(const struct device *dev, struct i2s_config *tx,
			   struct i2s_config *rx)
{
	size_t block = i2s_block_bytes(I2S_FRAMES, 16U);
	int ret;

	i2s_setup_cfg(tx, &i2s_tx_slab, I2S_LB_RATE, 16U, I2S_CHANNELS, block,
		     300, I2S_FMT_DATA_FORMAT_I2S,
		     I2S_OPT_BIT_CLK_MASTER | I2S_OPT_FRAME_CLK_MASTER);
	*rx = *tx;
	rx->mem_slab = &i2s_rx_slab;

	ret = i2s_configure(dev, I2S_DIR_TX, tx);
	if (ret != 0) {
		return ret;
	}
	return i2s_configure(dev, I2S_DIR_RX, rx);
}

ZTEST(i2s_negative, test_neg_tx_underrun_prepare)
{
	const struct device *dev = i2s_dev();
	struct i2s_config cfg;
	int ret;

	ret = i2s_force_tx_error(dev);
	zassert_equal(ret, -EIO, "write after underrun returned %d", ret);

	i2s_setup_default(&cfg, &i2s_tx_slab);
	ret = i2s_configure(dev, I2S_DIR_TX, &cfg);
	zassert_equal(ret, -EINVAL, "configure while ERROR returned %d", ret);

	ret = i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_PREPARE);
	zassert_equal(ret, 0, "PREPARE after underrun returned %d", ret);
	zassert_true(i2s_slabs_full(), "slab leak after PREPARE tx=%u rx=%u",
		     i2s_tx_used(), i2s_rx_used());

	i2s_setup_default(&cfg, &i2s_tx_slab);
	ret = i2s_configure(dev, I2S_DIR_TX, &cfg);
	zassert_equal(ret, 0, "reconfigure after PREPARE returned %d", ret);
	ret = i2s_block_write(dev, cfg.block_size, 0x33);
	zassert_equal(ret, 0, "write after recovery returned %d", ret);
	ret = i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_START);
	zassert_equal(ret, 0, "START after recovery returned %d", ret);

	k_msleep(5);
	(void)i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_DROP);
	zassert_true(i2s_slabs_full(), "slab leak after restart tx=%u",
		     i2s_tx_used());
}

ZTEST(i2s_negative, test_neg_rx_overrun_prepare)
{
	const struct device *dev = i2s_dev();
	struct i2s_config cfg;
	uint32_t got = 0U;
	int ret;

	i2s_setup_default(&cfg, &i2s_rx_slab);
	cfg.timeout = 80;
	ret = i2s_configure(dev, I2S_DIR_RX, &cfg);
	zassert_equal(ret, 0, "RX configure returned %d", ret);

	ret = i2s_trigger(dev, I2S_DIR_RX, I2S_TRIGGER_START);
	zassert_equal(ret, 0, "RX START returned %d", ret);

	/* Queue depth plus the in-flight block, with margin. */
	k_msleep(100);

	for (uint32_t i = 0U; i < I2S_RX_QUEUE + 2U; i++) {
		void *block = NULL;
		size_t size = 0U;

		ret = i2s_read(dev, &block, &size);
		if (ret != 0) {
			break;
		}
		got++;
		k_mem_slab_free(&i2s_rx_slab, block);
	}

	zassert_equal(ret, -ENOMSG,
		      "RX read after overrun returned %d (got %u blocks)",
		      ret, got);
	zassert_true(got > 0U && got <= I2S_RX_QUEUE,
		     "overrun left %u blocks, queue is %u", got, I2S_RX_QUEUE);

	ret = i2s_trigger(dev, I2S_DIR_RX, I2S_TRIGGER_PREPARE);
	zassert_equal(ret, 0, "PREPARE after overrun returned %d", ret);
	zassert_true(i2s_slabs_full(), "slab leak after RX PREPARE tx=%u rx=%u",
		     i2s_tx_used(), i2s_rx_used());
}

struct i2s_write_ctx {
	const struct device *dev;
	size_t size;
	int ret;
	int64_t t0;
	int64_t t1;
};

static void i2s_blocked_write(void *p1, void *p2, void *p3)
{
	struct i2s_write_ctx *ctx = p1;

	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	ctx->t0 = k_uptime_get();
	ctx->ret = i2s_block_write(ctx->dev, ctx->size, 0x44);
	ctx->t1 = k_uptime_get();
}

ZTEST(i2s_negative, test_neg_tx_queue_full)
{
	const struct device *dev = i2s_dev();
	struct i2s_config cfg;
	struct i2s_write_ctx ctx = { 0 };
	struct k_thread thread;
	int ret;

	i2s_setup_default(&cfg, &i2s_tx_slab);
	cfg.timeout = 0;
	ret = i2s_configure(dev, I2S_DIR_TX, &cfg);
	zassert_equal(ret, 0, "configure returned %d", ret);

	for (uint32_t i = 0U; i < I2S_TX_QUEUE; i++) {
		ret = i2s_block_write(dev, cfg.block_size, 0x10);
		zassert_equal(ret, 0, "fill write %u returned %d", i, ret);
	}

	ret = i2s_block_write(dev, cfg.block_size, 0x11);
	zassert_equal(ret, -EBUSY, "timeout 0 on a full queue returned %d", ret);

	cfg.timeout = 200;
	ret = i2s_configure(dev, I2S_DIR_TX, &cfg);
	zassert_equal(ret, 0, "timeout reconfigure returned %d", ret);

	{
		int64_t t0 = k_uptime_get();

		ret = i2s_block_write(dev, cfg.block_size, 0x12);
		zassert_equal(ret, -EAGAIN,
			      "short timeout on a full queue returned %d", ret);
		zassert_true((k_uptime_get() - t0) >= 150,
			     "full-queue write returned before the timeout");
	}

	cfg.timeout = 2000;
	ret = i2s_configure(dev, I2S_DIR_TX, &cfg);
	zassert_equal(ret, 0, "long-timeout reconfigure returned %d", ret);

	ctx.dev = dev;
	ctx.size = cfg.block_size;
	ctx.ret = 1;
	k_thread_create(&thread, i2s_write_stack,
			K_THREAD_STACK_SIZEOF(i2s_write_stack),
			i2s_blocked_write, &ctx, NULL, NULL,
			K_PRIO_PREEMPT(5), 0, K_NO_WAIT);

	k_msleep(50);
	zassert_equal(ctx.t1, 0, "write returned before DROP");

	ret = i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_DROP);
	zassert_equal(ret, 0, "DROP returned %d", ret);

	ret = k_thread_join(&thread, K_MSEC(500));
	zassert_equal(ret, 0, "blocked write did not return after DROP");
	zassert_equal(ctx.ret, 0, "write woken by DROP returned %d", ctx.ret);
	zassert_true((ctx.t1 - ctx.t0) >= 40,
		     "write was not blocked (%lld ms)",
		     (long long)(ctx.t1 - ctx.t0));

	(void)i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_DROP);
	zassert_true(i2s_slabs_full(), "slab leak after queue-full tx=%u",
		     i2s_tx_used());
}

struct i2s_read_ctx {
	const struct device *dev;
	volatile bool waiting;
	int ret;
	int64_t t0;
	int64_t t1;
};

static void i2s_blocked_read(void *p1, void *p2, void *p3)
{
	struct i2s_read_ctx *ctx = p1;
	void *block = NULL;
	size_t size = 0U;

	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	ctx->waiting = true;
	ctx->t0 = k_uptime_get();
	ctx->ret = i2s_read(ctx->dev, &block, &size);
	ctx->t1 = k_uptime_get();
	if (ctx->ret == 0 && block != NULL) {
		k_mem_slab_free(&i2s_rx_slab, block);
	}
}

ZTEST(i2s_negative, test_neg_read_timeout)
{
	const struct device *dev = i2s_dev();
	struct i2s_config cfg;
	struct i2s_read_ctx ctx = { 0 };
	struct k_thread thread;
	void *block = NULL;
	size_t size = 0U;
	int64_t t0;
	int ret;

	i2s_setup_default(&cfg, &i2s_rx_slab);
	cfg.timeout = 100;
	ret = i2s_configure(dev, I2S_DIR_RX, &cfg);
	zassert_equal(ret, 0, "RX configure returned %d", ret);

	t0 = k_uptime_get();
	ret = i2s_read(dev, &block, &size);
	zassert_equal(ret, -EAGAIN, "read before START returned %d", ret);
	zassert_true((k_uptime_get() - t0) >= 80,
		     "read returned before the timeout");

	cfg.timeout = 400;
	ret = i2s_configure(dev, I2S_DIR_RX, &cfg);
	zassert_equal(ret, 0, "RX reconfigure returned %d", ret);

	ctx.dev = dev;
	ctx.ret = 1;
	k_thread_create(&thread, i2s_read_stack,
			K_THREAD_STACK_SIZEOF(i2s_read_stack),
			i2s_blocked_read, &ctx, NULL, NULL,
			K_PRIO_COOP(7), 0, K_NO_WAIT);

	/* DROP resets the RX semaphore. The read must already be waiting. */
	while (!ctx.waiting) {
		k_yield();
	}
	k_msleep(5);
	t0 = k_uptime_get();
	ret = i2s_trigger(dev, I2S_DIR_RX, I2S_TRIGGER_DROP);
	zassert_equal(ret, 0, "DROP returned %d", ret);

	ret = k_thread_join(&thread, K_MSEC(800));
	zassert_equal(ret, 0, "blocked read did not return");
	zassert_equal(ctx.ret, -EAGAIN, "read during DROP returned %d", ctx.ret);
	zassert_true((ctx.t1 - t0) < 50,
		     "DROP did not wake the read (%lld ms after DROP)",
		     (long long)(ctx.t1 - t0));
}

ZTEST(i2s_negative, test_neg_configure_while_running)
{
	const struct device *dev = i2s_dev();
	struct i2s_config cfg;
	int ret;

	i2s_setup_default(&cfg, &i2s_tx_slab);
	ret = i2s_configure(dev, I2S_DIR_TX, &cfg);
	zassert_equal(ret, 0, "configure returned %d", ret);

	for (uint32_t i = 0U; i < 4U; i++) {
		ret = i2s_block_write(dev, cfg.block_size, 0x55);
		zassert_equal(ret, 0, "write %u returned %d", i, ret);
	}

	ret = i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_START);
	zassert_equal(ret, 0, "START returned %d", ret);
	ret = i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_START);
	zassert_true(ret < 0, "second START while RUNNING returned 0");

	ret = i2s_configure(dev, I2S_DIR_TX, &cfg);
	zassert_equal(ret, -EINVAL, "configure while RUNNING returned %d", ret);

	(void)i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_DROP);
	zassert_true(i2s_slabs_full(), "slab leak tx=%u", i2s_tx_used());
}

ZTEST(i2s_negative, test_neg_stop_and_drain)
{
	const struct device *dev = i2s_dev();
	struct i2s_config cfg;
	struct i2s_config tx;
	struct i2s_config rx;
	uint32_t queued_ms;
	uint32_t got;
	int64_t t0;
	int64_t elapsed;
	int ret;

	i2s_setup_default(&cfg, &i2s_rx_slab);
	cfg.frame_clk_freq = I2S_LB_RATE;
	ret = i2s_configure(dev, I2S_DIR_RX, &cfg);
	zassert_equal(ret, 0, "RX configure returned %d", ret);

	ret = i2s_trigger(dev, I2S_DIR_RX, I2S_TRIGGER_START);
	zassert_equal(ret, 0, "RX START returned %d", ret);
	ret = i2s_trigger(dev, I2S_DIR_RX, I2S_TRIGGER_STOP);
	zassert_equal(ret, 0, "RX STOP returned %d", ret);
	ret = i2s_wait_ready(dev, I2S_DIR_RX, &cfg, 500);
	zassert_equal(ret, 0, "RX did not return to READY after STOP");

	ret = i2s_trigger(dev, I2S_DIR_RX, I2S_TRIGGER_START);
	zassert_equal(ret, 0, "second RX START returned %d", ret);
	ret = i2s_trigger(dev, I2S_DIR_RX, I2S_TRIGGER_DRAIN);
	zassert_equal(ret, 0, "RX DRAIN returned %d", ret);
	ret = i2s_wait_ready(dev, I2S_DIR_RX, &cfg, 500);
	zassert_equal(ret, 0, "RX did not return to READY after DRAIN");
	(void)i2s_trigger(dev, I2S_DIR_RX, I2S_TRIGGER_DROP);
	zassert_true(i2s_slabs_full(), "slab leak rx=%u", i2s_rx_used());

	if (!IS_ENABLED(CONFIG_I2S_GPIO_LOOPBACK)) {
		return;
	}

	i2s_stream_reset(dev);
	ret = i2s_arm_loopback(dev, &tx, &rx);
	zassert_equal(ret, 0, "loopback configure returned %d", ret);

	for (uint32_t i = 0U; i < I2S_LB_BLOCKS; i++) {
		ret = i2s_block_write(dev, tx.block_size, 0xA5);
		zassert_equal(ret, 0, "write %u returned %d", i, ret);
	}

	ret = i2s_trigger(dev, I2S_DIR_BOTH, I2S_TRIGGER_START);
	zassert_equal(ret, 0, "BOTH START returned %d", ret);

	ret = i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_STOP);
	zassert_equal(ret, 0, "STOP returned %d", ret);

	ret = i2s_block_write(dev, tx.block_size, 0xA6);
	zassert_equal(ret, -EIO, "write while STOPPING returned %d", ret);

	ret = i2s_wait_ready(dev, I2S_DIR_TX, &tx, 200);
	zassert_equal(ret, 0, "TX did not return to READY after STOP");

	got = i2s_rx_count(dev, I2S_LB_BLOCKS + 2U);
	LOG_INF("STOP delivered %u of %u blocks", got, I2S_LB_BLOCKS);
	zassert_true(got <= 3U, "STOP delivered %u blocks, expected at most 3",
		     got);

	(void)i2s_trigger(dev, I2S_DIR_BOTH, I2S_TRIGGER_DROP);
	zassert_true(i2s_slabs_full(), "slab leak after STOP tx=%u rx=%u",
		     i2s_tx_used(), i2s_rx_used());

	i2s_stream_reset(dev);
	ret = i2s_arm_loopback(dev, &tx, &rx);
	zassert_equal(ret, 0, "loopback configure returned %d", ret);

	for (uint32_t i = 0U; i < I2S_LB_BLOCKS; i++) {
		ret = i2s_block_write(dev, tx.block_size, 0xA5);
		zassert_equal(ret, 0, "write %u returned %d", i, ret);
	}

	ret = i2s_trigger(dev, I2S_DIR_BOTH, I2S_TRIGGER_START);
	zassert_equal(ret, 0, "BOTH START returned %d", ret);

	t0 = k_uptime_get();
	ret = i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_DRAIN);
	zassert_equal(ret, 0, "DRAIN returned %d", ret);

	ret = i2s_wait_ready(dev, I2S_DIR_TX, &tx, 500);
	zassert_equal(ret, 0, "TX did not return to READY after DRAIN");
	elapsed = k_uptime_get() - t0;

	got = i2s_rx_count(dev, I2S_LB_BLOCKS + 2U);
	queued_ms = (I2S_LB_BLOCKS * I2S_FRAMES * 1000U) / I2S_LB_RATE;
	LOG_INF("DRAIN delivered %u of %u blocks in %lld ms (queued %u ms)",
		got, I2S_LB_BLOCKS, (long long)elapsed, queued_ms);

	zassert_true(got >= 5U, "DRAIN delivered only %u blocks", got);
	zassert_true(elapsed >= (int64_t)queued_ms / 2,
		     "DRAIN finished too quickly (%lld ms)", (long long)elapsed);

	(void)i2s_trigger(dev, I2S_DIR_BOTH, I2S_TRIGGER_DROP);
	zassert_true(i2s_slabs_full(), "slab leak tx=%u rx=%u",
		     i2s_tx_used(), i2s_rx_used());
}

ZTEST(i2s_negative, test_neg_fd_mismatch)
{
	const struct device *dev = i2s_dev();
	struct i2s_config tx;
	struct i2s_config rx;
	int ret;

	struct {
		const char *name;
		uint32_t rate;
		uint8_t word_size;
		uint8_t channels;
		i2s_fmt_t format;
		i2s_opt_t options;
	} cases[] = {
		{ "rate", 16000U, 16U, 2U, I2S_FMT_DATA_FORMAT_I2S,
		  I2S_OPT_BIT_CLK_MASTER | I2S_OPT_FRAME_CLK_MASTER },
		{ "word_size", 48000U, 32U, 2U, I2S_FMT_DATA_FORMAT_I2S,
		  I2S_OPT_BIT_CLK_MASTER | I2S_OPT_FRAME_CLK_MASTER },
		{ "channels", 48000U, 16U, 4U, I2S_FMT_DATA_FORMAT_I2S,
		  I2S_OPT_BIT_CLK_MASTER | I2S_OPT_FRAME_CLK_MASTER },
		{ "format", 48000U, 16U, 2U, I2S_FMT_DATA_FORMAT_PCM_SHORT,
		  I2S_OPT_BIT_CLK_MASTER | I2S_OPT_FRAME_CLK_MASTER },
		{ "options", 48000U, 16U, 2U, I2S_FMT_DATA_FORMAT_I2S,
		  I2S_OPT_BIT_CLK_SLAVE | I2S_OPT_FRAME_CLK_SLAVE },
	};

	i2s_setup_default(&tx, &i2s_tx_slab);

	for (size_t i = 0U; i < ARRAY_SIZE(cases); i++) {
		i2s_stream_reset(dev);
		rx = tx;
		rx.mem_slab = &i2s_rx_slab;
		rx.frame_clk_freq = cases[i].rate;
		rx.word_size = cases[i].word_size;
		rx.channels = cases[i].channels;
		rx.format = cases[i].format;
		rx.options = cases[i].options;

		ret = i2s_configure(dev, I2S_DIR_TX, &tx);
		zassert_equal(ret, 0, "%s TX configure returned %d",
			      cases[i].name, ret);
		ret = i2s_configure(dev, I2S_DIR_RX, &rx);
		zassert_equal(ret, 0, "%s RX configure returned %d",
			      cases[i].name, ret);

		ret = i2s_trigger(dev, I2S_DIR_BOTH, I2S_TRIGGER_START);
		zassert_equal(ret, -EINVAL, "%s BOTH START returned %d",
			      cases[i].name, ret);
	}

	i2s_stream_reset(dev);
	i2s_setup_default(&tx, &i2s_tx_slab);
	rx = tx;
	rx.mem_slab = &i2s_rx_slab;
	rx.block_size = tx.block_size / 2U;

	ret = i2s_configure(dev, I2S_DIR_TX, &tx);
	zassert_equal(ret, 0, "block_size TX configure returned %d", ret);
	ret = i2s_configure(dev, I2S_DIR_RX, &rx);
	zassert_equal(ret, 0, "block_size RX configure returned %d", ret);
	ret = i2s_block_write(dev, tx.block_size, 0x66);
	zassert_equal(ret, 0, "block_size write returned %d", ret);

	ret = i2s_trigger(dev, I2S_DIR_BOTH, I2S_TRIGGER_START);
	zassert_equal(ret, 0, "block_size mismatch BOTH START returned %d", ret);

	(void)i2s_trigger(dev, I2S_DIR_BOTH, I2S_TRIGGER_DROP);
	zassert_true(i2s_slabs_full(), "slab leak tx=%u rx=%u",
		     i2s_tx_used(), i2s_rx_used());
}

ZTEST(i2s_negative, test_neg_both_start_rollback)
{
	const struct device *dev = i2s_dev();
	struct i2s_config tx;
	struct i2s_config rx;
	int ret;

	i2s_setup_default(&tx, &i2s_tx_slab);
	rx = tx;
	rx.mem_slab = &i2s_rx_slab;

	ret = i2s_configure(dev, I2S_DIR_TX, &tx);
	zassert_equal(ret, 0, "TX configure returned %d", ret);
	ret = i2s_configure(dev, I2S_DIR_RX, &rx);
	zassert_equal(ret, 0, "RX configure returned %d", ret);

	ret = i2s_trigger(dev, I2S_DIR_BOTH, I2S_TRIGGER_START);
	zassert_true(ret < 0, "BOTH START with an empty TX queue returned 0");

	ret = i2s_configure(dev, I2S_DIR_RX, &rx);
	zassert_equal(ret, 0, "RX was left running (configure %d)", ret);
	ret = i2s_configure(dev, I2S_DIR_TX, &tx);
	zassert_equal(ret, 0, "TX was left running (configure %d)", ret);
	zassert_true(i2s_slabs_full(), "slab leak tx=%u rx=%u",
		     i2s_tx_used(), i2s_rx_used());

	i2s_stream_reset(dev);

	void *held[I2S_SLAB_COUNT];
	uint32_t held_n = 0U;

	while (held_n < I2S_SLAB_COUNT &&
	       k_mem_slab_alloc(&i2s_rx_slab, &held[held_n], K_NO_WAIT) == 0) {
		held_n++;
	}
	zassert_equal(held_n, I2S_SLAB_COUNT, "held %u of %u RX blocks",
		      held_n, I2S_SLAB_COUNT);

	i2s_setup_default(&tx, &i2s_tx_slab);
	rx = tx;
	rx.mem_slab = &i2s_rx_slab;

	ret = i2s_configure(dev, I2S_DIR_TX, &tx);
	zassert_equal(ret, 0, "TX configure returned %d", ret);
	ret = i2s_configure(dev, I2S_DIR_RX, &rx);
	zassert_equal(ret, 0, "RX configure returned %d", ret);
	ret = i2s_block_write(dev, tx.block_size, 0x77);
	zassert_equal(ret, 0, "TX write returned %d", ret);

	ret = i2s_trigger(dev, I2S_DIR_BOTH, I2S_TRIGGER_START);
	zassert_true(ret < 0, "BOTH START with an empty RX slab returned 0");

	ret = i2s_configure(dev, I2S_DIR_TX, &tx);
	zassert_equal(ret, 0, "TX was not rolled back to READY (%d)", ret);
	ret = i2s_configure(dev, I2S_DIR_RX, &rx);
	zassert_equal(ret, 0, "RX configure after failed START returned %d", ret);

	for (uint32_t i = 0U; i < held_n; i++) {
		k_mem_slab_free(&i2s_rx_slab, held[i]);
	}
	zassert_equal(i2s_tx_used(), 0U, "TX slab leak %u", i2s_tx_used());
	zassert_equal(i2s_rx_used(), 0U, "RX slab leak %u", i2s_rx_used());
}

/*
 * Word sizes other than 12, 16, 20, 24, and 32 must be rejected.
 * The driver currently accepts them and programs WLEN 0.
 */
ZTEST(i2s_negative, test_neg_invalid_word_size)
{
	const struct device *dev = i2s_dev();
	struct i2s_config cfg;
	int ret;

	i2s_setup_default(&cfg, &i2s_tx_slab);
	cfg.word_size = 8U;
	ret = i2s_configure(dev, I2S_DIR_TX, &cfg);
	zassert_equal(ret, -EINVAL, "word size 8 configure returned %d", ret);

	i2s_stream_reset(dev);
	i2s_setup_default(&cfg, &i2s_tx_slab);
	cfg.word_size = 18U;
	ret = i2s_configure(dev, I2S_DIR_TX, &cfg);
	zassert_equal(ret, -EINVAL, "word size 18 configure returned %d", ret);

	i2s_stream_reset(dev);
	i2s_setup_default(&cfg, &i2s_tx_slab);
	ret = i2s_configure(dev, I2S_DIR_TX, &cfg);
	zassert_equal(ret, 0, "word size 16 configure returned %d", ret);
}

/*
 * STOP must finish the current memory block. A later START, with no new
 * write, resumes the blocks still in the queue. The TX interrupt currently
 * disables the stream on the next FIFO interrupt, so READY arrives in
 * well under half a block.
 */
ZTEST(i2s_negative, test_neg_tx_stop_resumes)
{
	const struct device *dev = i2s_dev();
	struct i2s_config cfg;
	const uint32_t block_ms = (I2S_MAX_FRAMES * 1000U) / I2S_LB_RATE;
	int64_t t0;
	int64_t elapsed;
	int ret;

	i2s_setup_cfg(&cfg, &i2s_tx_slab, I2S_LB_RATE, 16U, I2S_CHANNELS,
		      i2s_block_bytes(I2S_MAX_FRAMES, 16U), 300,
		      I2S_FMT_DATA_FORMAT_I2S,
		      I2S_OPT_BIT_CLK_MASTER | I2S_OPT_FRAME_CLK_MASTER);
	ret = i2s_configure(dev, I2S_DIR_TX, &cfg);
	zassert_equal(ret, 0, "configure returned %d", ret);

	for (uint32_t i = 0U; i < 4U; i++) {
		ret = i2s_block_write(dev, cfg.block_size, 0x5A);
		zassert_equal(ret, 0, "write %u returned %d", i, ret);
	}

	ret = i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_START);
	zassert_equal(ret, 0, "START returned %d", ret);

	t0 = k_uptime_get();
	ret = i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_STOP);
	zassert_equal(ret, 0, "STOP returned %d", ret);

	ret = i2s_wait_ready(dev, I2S_DIR_TX, &cfg, 500);
	zassert_equal(ret, 0, "TX did not return to READY after STOP");
	elapsed = k_uptime_get() - t0;
	zassert_true(elapsed >= (int64_t)block_ms / 2,
		     "STOP returned to READY in %lld ms, block is %u ms",
		     (long long)elapsed, block_ms);

	ret = i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_START);
	zassert_equal(ret, 0, "START did not resume the queued blocks (%d)",
		      ret);

	(void)i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_DROP);
	zassert_true(i2s_slabs_full(), "slab leak tx=%u", i2s_tx_used());
}

/* =========================================================================
 * Suite registration
 * =========================================================================
 */
static void *neg_setup(void)
{
	const struct device *dev = i2s_dev();

	zassert_true(device_is_ready(dev), "I2S device not ready");
	return NULL;
}

static void neg_before(void *fixture)
{
	ARG_UNUSED(fixture);
	i2s_stream_reset(i2s_dev());
}

ZTEST_SUITE(i2s_negative, NULL, neg_setup, neg_before, NULL, NULL);

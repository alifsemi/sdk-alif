/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/time_units.h>
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>
#include "i2s_test.h"

LOG_MODULE_REGISTER(i2s_performance, LOG_LEVEL_INF);

K_THREAD_STACK_DEFINE(perf_feed_stack, 2048);
K_THREAD_STACK_DEFINE(perf_spin_stack, 512);

/* Thread control blocks are large. The ztest stack is 1024 bytes, so these
 * stay at file scope. Two of them inside perf_stream_ms overflow that stack
 * as soon as the first stream starts, which resets the board.
 */
static struct k_thread perf_feed_thread;
static struct k_thread perf_spin_thread;

#define PERF_BLOCKS 1000U
#define PERF_RATE   48000U
/* One ISR fills FIFO depth minus the TX watermark (16 - 8). A shorter block
 * leaves the FIFO interrupt asserted, so the ISR drains the whole queue and
 * the stream is already in ERROR when the test writes again.
 */
#define PERF_MIN_FRAMES 8U

static void *perf_setup(void)
{
	const struct device *dev = i2s_dev();

	zassert_true(device_is_ready(dev), "I2S device not ready");
	return NULL;
}

static void perf_before(void *fixture)
{
	ARG_UNUSED(fixture);
	i2s_stream_reset(i2s_dev());
}

static uint32_t perf_tol(uint32_t expected_us)
{
	uint32_t tol = expected_us / 20U;

	/* Short block periods are under a millisecond. Keep a floor so the
	 * cycle-counter conversion cannot gfail a correct stream.
	 */
	if (tol < 15U) {
		tol = 15U;
	}
	return tol;
}

static void perf_one_geometry(const struct device *dev, uint32_t frames,
			      uint8_t word_size)
{
	struct i2s_config cfg;
	size_t block = i2s_block_bytes(frames, word_size);
	uint32_t expected_us = (frames * 1000000U) / PERF_RATE;
	uint32_t tol = perf_tol(expected_us);
	uint64_t gap_sum = 0U;
	uint32_t gap_n = 0U;
	uint32_t worst = 0U;
	uint32_t prev = 0U;
	uint32_t t_start;
	uint32_t elapsed_us;
	uint32_t avg;
	uint64_t bytes;
	uint32_t throughput;
	uint32_t expect_bps;
	int ret;

	zassert_true(block <= I2S_MAX_BLOCK, "block %u exceeds the slab",
		     (unsigned int)block);

	i2s_setup_cfg(&cfg, &i2s_tx_slab, PERF_RATE, word_size, I2S_CHANNELS,
		     block, 200, I2S_FMT_DATA_FORMAT_I2S,
		     I2S_OPT_BIT_CLK_MASTER | I2S_OPT_FRAME_CLK_MASTER);
	ret = i2s_configure(dev, I2S_DIR_TX, &cfg);
	zassert_equal(ret, 0, "%u-frame configure returned %d", frames, ret);

	for (uint32_t i = 0U; i < I2S_TX_QUEUE; i++) {
		ret = i2s_block_write(dev, block, 0x31);
		zassert_equal(ret, 0, "%u-frame prime %u returned %d",
			      frames, i, ret);
	}

	t_start = k_cycle_get_32();
	ret = i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_START);
	zassert_equal(ret, 0, "%u-frame START returned %d", frames, ret);
	prev = k_cycle_get_32();

	for (uint32_t i = 0U; i < PERF_BLOCKS; i++) {
		uint32_t now;
		uint32_t gap;

		ret = i2s_block_write(dev, block, 0x32);
		now = k_cycle_get_32();
		zassert_equal(ret, 0, "%u-frame write %u returned %d",
			      frames, i, ret);
		gap = k_cyc_to_us_near32(now - prev);
		prev = now;
		if (i >= 5U) {
			gap_sum += gap;
			gap_n++;
			if (gap > worst) {
				worst = gap;
			}
		}
	}

	elapsed_us = k_cyc_to_us_near32(k_cycle_get_32() - t_start);
	ret = i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_PREPARE);
	(void)i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_DROP);
	zassert_equal(ret, -EIO, "%u-frame stream entered ERROR", frames);
	zassert_equal(i2s_tx_used(), 0U, "%u-frame leaked %u blocks",
		      frames, i2s_tx_used());

	avg = (uint32_t)(gap_sum / gap_n);
	bytes = (uint64_t)PERF_BLOCKS * block;
	throughput = (elapsed_us == 0U) ? 0U :
		     (uint32_t)((bytes * 1000000ULL) / elapsed_us);
	expect_bps = PERF_RATE * I2S_CHANNELS * (uint32_t)i2s_sample_bytes(word_size);

	LOG_INF("%u frames: period %u us (expect %u), worst %u us, "
		"throughput %u B/s (expect %u)",
		frames, avg, expected_us, worst, throughput, expect_bps);

	zassert_true(avg + tol >= expected_us && avg <= expected_us + tol,
		     "%u-frame period %u us, expect %u +/- %u",
		     frames, avg, expected_us, tol);
	zassert_true(worst <= (expected_us * 2U) + tol,
		     "%u-frame gap %u us exceeds two periods", frames, worst);
	zassert_true(throughput + (expect_bps / 20U) >= expect_bps &&
		     throughput <= expect_bps + (expect_bps / 20U),
		     "%u-frame throughput %u B/s, expect %u",
		     frames, throughput, expect_bps);
}

ZTEST(i2s_performance, test_performance_transfer)
{
	const struct device *dev = i2s_dev();

	perf_one_geometry(dev, PERF_MIN_FRAMES, 16U);
	i2s_stream_reset(dev);
	perf_one_geometry(dev, I2S_FRAMES, 16U);
	i2s_stream_reset(dev);
	perf_one_geometry(dev, I2S_MAX_FRAMES, 16U);
}

static volatile bool perf_spin_run;
static volatile uint32_t perf_spins;

static void perf_spin_fn(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	while (perf_spin_run) {
		perf_spins++;
	}
}

static int perf_stream_ms(const struct device *dev, uint32_t rate,
			  uint8_t word_size, uint32_t ms, bool count_idle)
{
	struct i2s_config cfg;
	struct i2s_tx_feed feed = { 0 };
	size_t block = i2s_block_bytes(I2S_FRAMES, word_size);
	int ret;

	i2s_setup_cfg(&cfg, &i2s_tx_slab, rate, word_size, I2S_CHANNELS, block,
		     50, I2S_FMT_DATA_FORMAT_I2S,
		     I2S_OPT_BIT_CLK_MASTER | I2S_OPT_FRAME_CLK_MASTER);
	ret = i2s_configure(dev, I2S_DIR_TX, &cfg);
	if (ret != 0) {
		return ret;
	}

	for (uint32_t i = 0U; i < 4U; i++) {
		ret = i2s_block_write(dev, block, 0x41);
		if (ret != 0) {
			return ret;
		}
	}

	feed.dev = dev;
	feed.size = block;
	feed.run = true;
	k_thread_create(&perf_feed_thread, perf_feed_stack,
			K_THREAD_STACK_SIZEOF(perf_feed_stack),
			i2s_tx_feed_fn, &feed, NULL, NULL,
			K_PRIO_PREEMPT(2), 0, K_NO_WAIT);

	if (count_idle) {
		perf_spins = 0U;
		perf_spin_run = true;
		k_thread_create(&perf_spin_thread, perf_spin_stack,
				K_THREAD_STACK_SIZEOF(perf_spin_stack),
				perf_spin_fn, NULL, NULL, NULL,
				K_PRIO_PREEMPT(14), 0, K_NO_WAIT);
	}

	ret = i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_START);
	if (ret != 0) {
		feed.run = false;
		perf_spin_run = false;
		(void)k_thread_join(&perf_feed_thread, K_SECONDS(1));
		if (count_idle) {
			(void)k_thread_join(&perf_spin_thread, K_MSEC(200));
		}
		(void)i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_DROP);
		return ret;
	}

	k_msleep(ms);

	ret = i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_PREPARE);
	feed.run = false;
	perf_spin_run = false;
	(void)i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_DROP);
	(void)k_thread_join(&perf_feed_thread, K_SECONDS(2));
	(void)i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_DROP);
	if (count_idle) {
		(void)k_thread_join(&perf_spin_thread, K_MSEC(200));
		LOG_INF("idle spin count at %u Hz / %u-bit: %u "
			"(higher means more CPU left; compare IRQ and DMA builds)",
			rate, word_size, perf_spins);
	}

	if (feed.fail != 0) {
		return feed.fail;
	}
	if (ret == 0) {
		return -EIO;
	}
	return 0;
}

/*
 * Every datasheet rate, at the two FIFO slot widths. 12-bit uses the same
 * 16-bit slot as 16-bit, and 20/24-bit use the same 32-bit slot as 32-bit.
 * The golden and config suites check those resolutions.
 */
ZTEST(i2s_performance, test_performance_sample_rates)
{
	const struct device *dev = i2s_dev();
	const uint32_t rates[] = {
		8000U, 16000U, 32000U, 44100U,
		48000U, 88200U, 96000U, 192000U,
	};
	const uint8_t words[] = { 16U, 32U };
	int ret;

	for (size_t r = 0U; r < ARRAY_SIZE(rates); r++) {
		for (size_t w = 0U; w < ARRAY_SIZE(words); w++) {
			bool idle = rates[r] == 192000U && words[w] == 32U;

			i2s_stream_reset(dev);
			ret = perf_stream_ms(dev, rates[r], words[w], 1000U, idle);
			zassert_equal(ret, 0, "%u Hz / %u-bit returned %d",
				      rates[r], words[w], ret);
			LOG_INF("clean %u Hz / %u-bit", rates[r], words[w]);
		}
	}
}

ZTEST(i2s_performance, test_performance_startup_latency)
{
	const struct device *dev = i2s_dev();
	struct i2s_config tx;
	struct i2s_config rx;
	uint32_t t0;
	uint32_t latency_us = 0U;
	bool got = false;
	int ret;

	if (!IS_ENABLED(CONFIG_I2S_GPIO_LOOPBACK)) {
		ztest_test_skip();
		return;
	}

	i2s_setup_default(&tx, &i2s_tx_slab);
	rx = tx;
	rx.mem_slab = &i2s_rx_slab;
	rx.timeout = 500;

	ret = i2s_configure(dev, I2S_DIR_TX, &tx);
	zassert_equal(ret, 0, "TX configure returned %d", ret);
	ret = i2s_configure(dev, I2S_DIR_RX, &rx);
	zassert_equal(ret, 0, "RX configure returned %d", ret);

	for (uint32_t i = 0U; i < 4U; i++) {
		ret = i2s_block_write(dev, tx.block_size, 0xA5);
		zassert_equal(ret, 0, "prime %u returned %d", i, ret);
	}

	t0 = k_cycle_get_32();
	ret = i2s_trigger(dev, I2S_DIR_BOTH, I2S_TRIGGER_START);
	zassert_equal(ret, 0, "BOTH START returned %d", ret);

	for (uint32_t i = 0U; i < 8U; i++) {
		void *block = NULL;
		size_t size = 0U;

		ret = i2s_read(dev, &block, &size);
		if (ret != 0) {
			break;
		}
		if (!got) {
			latency_us = k_cyc_to_us_near32(k_cycle_get_32() - t0);
			got = true;
		}
		k_mem_slab_free(&i2s_rx_slab, block);
	}

	(void)i2s_trigger(dev, I2S_DIR_BOTH, I2S_TRIGGER_DROP);
	LOG_INF("startup latency %u us", latency_us);
	zassert_true(got, "no RX block arrived");
}

ZTEST(i2s_performance, test_performance_underrun_margin)
{
	const struct device *dev = i2s_dev();
	const uint32_t depths[] = { 1U, 2U, 4U };
	const uint32_t rate = 192000U;
	uint32_t prev_us = 0U;

	/* No refill thread. With nothing topping up the queue, time to
	 * ERROR has to grow with the number of blocks that were primed.
	 */
	for (size_t d = 0U; d < ARRAY_SIZE(depths); d++) {
		struct i2s_config cfg;
		size_t block = i2s_block_bytes(I2S_FRAMES, 32U);
		uint32_t expect_us = (depths[d] * I2S_FRAMES * 1000000U) / rate;
		uint32_t t0;
		uint32_t elapsed_us = 0U;
		int ret;

		i2s_stream_reset(dev);
		i2s_setup_cfg(&cfg, &i2s_tx_slab, rate, 32U, I2S_CHANNELS,
			     block, 20, I2S_FMT_DATA_FORMAT_I2S,
			     I2S_OPT_BIT_CLK_MASTER | I2S_OPT_FRAME_CLK_MASTER);
		ret = i2s_configure(dev, I2S_DIR_TX, &cfg);
		zassert_equal(ret, 0, "depth %u configure returned %d",
			      depths[d], ret);

		for (uint32_t i = 0U; i < depths[d]; i++) {
			ret = i2s_block_write(dev, block, 0x51);
			zassert_equal(ret, 0, "depth %u prime returned %d",
				      depths[d], ret);
		}

		t0 = k_cycle_get_32();
		ret = i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_START);
		zassert_equal(ret, 0, "depth %u START returned %d",
			      depths[d], ret);

		do {
			ret = i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_PREPARE);
			elapsed_us = k_cyc_to_us_near32(k_cycle_get_32() - t0);
		} while (ret != 0 && elapsed_us < 5000U);

		(void)i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_DROP);
		LOG_INF("depth %u underran after %u us (expect %u us)",
			depths[d], elapsed_us, expect_us);

		zassert_equal(ret, 0, "depth %u did not reach ERROR", depths[d]);
		zassert_true(elapsed_us + 50U >= expect_us / 2U &&
			     elapsed_us <= expect_us * 3U + 300U,
			     "depth %u took %u us, expect %u us",
			     depths[d], elapsed_us, expect_us);
		zassert_true(elapsed_us > prev_us,
			     "depth %u (%u us) was not longer than depth below (%u us)",
			     depths[d], elapsed_us, prev_us);
		prev_us = elapsed_us;
		zassert_equal(i2s_tx_used(), 0U, "depth %u leaked %u blocks",
			      depths[d], i2s_tx_used());
	}
}

ZTEST_SUITE(i2s_performance, NULL, perf_setup, perf_before, NULL, NULL);

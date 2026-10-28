/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>
#include "i2s_test.h"

LOG_MODULE_REGISTER(i2s_stress, LOG_LEVEL_INF);

K_THREAD_STACK_DEFINE(stress_feed_stack, 2048);
K_THREAD_STACK_DEFINE(stress_fuzz_stack, 2048);

/* The ztest stack is 1024 bytes. Thread control blocks stay here. */
static struct k_thread stress_fd_thread;
static struct k_thread stress_mt_write_thread;
static struct k_thread stress_mt_trig_thread;

#define STRESS_192K_MS 30000
#define STRESS_FUZZ_MS 10000
#define STRESS_FD_MS   10000
#define STRESS_MT_MS   5000
#define STRESS_FD_RATE 16000U
#define STRESS_FD_MARK 0x5AU

static void *stress_setup(void)
{
	const struct device *dev = i2s_dev();

	zassert_true(device_is_ready(dev), "I2S device not ready");
	return NULL;
}

static void stress_before(void *fixture)
{
	ARG_UNUSED(fixture);
	i2s_stream_reset(i2s_dev());
}

static void stress_check_slabs(const char *what)
{
	zassert_true(i2s_slabs_full(), "%s leaked tx=%u rx=%u",
		     what, i2s_tx_used(), i2s_rx_used());
}

ZTEST(i2s_stress, test_stress_start_drop)
{
	const struct device *dev = i2s_dev();
	struct i2s_config cfg;
	int ret;

	i2s_setup_default(&cfg, &i2s_tx_slab);

	for (uint32_t i = 0U; i < 500U; i++) {
		ret = i2s_configure(dev, I2S_DIR_TX, &cfg);
		zassert_equal(ret, 0, "configure %u returned %d", i, ret);
		ret = i2s_block_write(dev, cfg.block_size, 0x21);
		zassert_equal(ret, 0, "write %u returned %d", i, ret);
		ret = i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_START);
		zassert_equal(ret, 0, "START %u returned %d", i, ret);
		k_msleep(1);
		ret = i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_DROP);
		zassert_equal(ret, 0, "DROP %u returned %d", i, ret);
	}

	stress_check_slabs("start/drop");
}

ZTEST(i2s_stress, test_stress_start_stop)
{
	const struct device *dev = i2s_dev();
	struct i2s_config cfg;
	int ret;

	i2s_setup_default(&cfg, &i2s_tx_slab);

	for (uint32_t i = 0U; i < 200U; i++) {
		ret = i2s_configure(dev, I2S_DIR_TX, &cfg);
		zassert_equal(ret, 0, "configure %u returned %d", i, ret);

		for (uint32_t n = 0U; n < 2U; n++) {
			ret = i2s_block_write(dev, cfg.block_size, 0x22);
			zassert_equal(ret, 0, "write %u.%u returned %d", i, n, ret);
		}

		ret = i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_START);
		zassert_equal(ret, 0, "START %u returned %d", i, ret);
		ret = i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_STOP);
		zassert_equal(ret, 0, "STOP %u returned %d", i, ret);
		ret = i2s_wait_ready(dev, I2S_DIR_TX, &cfg, 200);
		zassert_equal(ret, 0, "READY after STOP %u", i);
		ret = i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_DROP);
		zassert_equal(ret, 0, "DROP %u returned %d", i, ret);
	}

	stress_check_slabs("start/stop");
}

ZTEST(i2s_stress, test_stress_start_drain)
{
	const struct device *dev = i2s_dev();
	struct i2s_config cfg;
	int ret;

	i2s_setup_default(&cfg, &i2s_tx_slab);

	for (uint32_t i = 0U; i < 200U; i++) {
		ret = i2s_configure(dev, I2S_DIR_TX, &cfg);
		zassert_equal(ret, 0, "configure %u returned %d", i, ret);

		for (uint32_t n = 0U; n < 2U; n++) {
			ret = i2s_block_write(dev, cfg.block_size, 0x23);
			zassert_equal(ret, 0, "write %u.%u returned %d", i, n, ret);
		}

		ret = i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_START);
		zassert_equal(ret, 0, "START %u returned %d", i, ret);
		ret = i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_DRAIN);
		zassert_equal(ret, 0, "DRAIN %u returned %d", i, ret);
		ret = i2s_wait_ready(dev, I2S_DIR_TX, &cfg, 200);
		zassert_equal(ret, 0, "READY after DRAIN %u", i);
		zassert_equal(i2s_tx_used(), 0U, "DRAIN %u left %u blocks",
			      i, i2s_tx_used());
	}
}

ZTEST(i2s_stress, test_stress_underrun_recover)
{
	const struct device *dev = i2s_dev();

	for (uint32_t i = 0U; i < 100U; i++) {
		int ret = i2s_force_tx_error(dev);

		zassert_equal(ret, -EIO, "underrun %u write returned %d", i, ret);
		ret = i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_PREPARE);
		zassert_equal(ret, 0, "PREPARE %u returned %d", i, ret);
		zassert_equal(i2s_tx_used(), 0U, "underrun %u leaked %u",
			      i, i2s_tx_used());
	}
}

ZTEST(i2s_stress, test_stress_rx_overrun_recover)
{
	const struct device *dev = i2s_dev();
	struct i2s_config cfg;

	i2s_setup_default(&cfg, &i2s_rx_slab);

	for (uint32_t i = 0U; i < 50U; i++) {
		int64_t end;
		int ret = i2s_configure(dev, I2S_DIR_RX, &cfg);

		zassert_equal(ret, 0, "RX configure %u returned %d", i, ret);
		ret = i2s_trigger(dev, I2S_DIR_RX, I2S_TRIGGER_START);
		zassert_equal(ret, 0, "RX START %u returned %d", i, ret);

		end = k_uptime_get() + 200;
		do {
			ret = i2s_trigger(dev, I2S_DIR_RX, I2S_TRIGGER_PREPARE);
			if (ret == 0) {
				break;
			}
			k_msleep(5);
		} while (k_uptime_get() < end);

		zassert_equal(ret, 0, "RX overrun %u did not reach ERROR", i);
		zassert_equal(i2s_rx_used(), 0U, "overrun %u leaked %u",
			      i, i2s_rx_used());
	}
}

ZTEST(i2s_stress, test_stress_tx_192k_30s)
{
	const struct device *dev = i2s_dev();
	struct i2s_config cfg;
	struct i2s_tx_feed feed = { 0 };
	struct k_thread thread;
	size_t block;
	int64_t t0;
	int64_t elapsed_ms;
	uint64_t audio_ms;
	int ret;

	block = i2s_block_bytes(I2S_FRAMES, 32U);
	i2s_setup_cfg(&cfg, &i2s_tx_slab, 192000U, 32U, I2S_CHANNELS, block,
		     50, I2S_FMT_DATA_FORMAT_I2S,
		     I2S_OPT_BIT_CLK_MASTER | I2S_OPT_FRAME_CLK_MASTER);

	ret = i2s_configure(dev, I2S_DIR_TX, &cfg);
	zassert_equal(ret, 0, "configure returned %d", ret);

	for (uint32_t i = 0U; i < 4U; i++) {
		ret = i2s_block_write(dev, block, 0x24);
		zassert_equal(ret, 0, "prime write %u returned %d", i, ret);
	}

	feed.dev = dev;
	feed.size = block;
	feed.run = true;
	k_thread_create(&thread, stress_feed_stack,
			K_THREAD_STACK_SIZEOF(stress_feed_stack),
			i2s_tx_feed_fn, &feed, NULL, NULL,
			K_PRIO_PREEMPT(2), 0, K_NO_WAIT);

	ret = i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_START);
	if (ret != 0) {
		feed.run = false;
		(void)i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_DROP);
		(void)k_thread_join(&thread, K_SECONDS(2));
		(void)i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_DROP);
	}
	zassert_equal(ret, 0, "START returned %d", ret);

	t0 = k_uptime_get();
	k_msleep(STRESS_192K_MS);
	elapsed_ms = k_uptime_get() - t0;

	ret = i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_PREPARE);
	feed.run = false;
	(void)i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_DROP);
	zassert_equal(k_thread_join(&thread, K_SECONDS(2)), 0,
		      "feed thread did not stop");
	(void)i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_DROP);
	zassert_equal(feed.fail, 0, "TX feed hit %d after %u writes",
		      feed.fail, feed.writes);
	zassert_equal(ret, -EIO, "stream entered ERROR during the 30 s run");

	/* k_uptime, not the ztest line. That line converts a 32-bit cycle
	 * delta and wraps after about 28 s on this CPU, so a 30 s run prints
	 * as about 3 s.
	 */
	audio_ms = ((uint64_t)feed.writes * I2S_FRAMES * 1000ULL) / 192000ULL;
	LOG_INF("192 kHz TX held for %lld ms, %u blocks (%llu ms of audio)",
		(long long)elapsed_ms, feed.writes, (unsigned long long)audio_ms);
	zassert_true(elapsed_ms >= (STRESS_192K_MS * 9) / 10,
		     "sleep returned after %lld ms", (long long)elapsed_ms);
	zassert_true(audio_ms + audio_ms / 5U >= (uint64_t)elapsed_ms &&
		     (uint64_t)elapsed_ms + (uint64_t)elapsed_ms / 5U >= audio_ms,
		     "audio %llu ms disagrees with sleep %lld ms",
		     (unsigned long long)audio_ms, (long long)elapsed_ms);
	stress_check_slabs("192k/30s");
}

ZTEST(i2s_stress, test_stress_queue_fill_drain)
{
	const struct device *dev = i2s_dev();
	struct i2s_config cfg;
	int ret;

	i2s_setup_default(&cfg, &i2s_tx_slab);
	cfg.timeout = 0;
	ret = i2s_configure(dev, I2S_DIR_TX, &cfg);
	zassert_equal(ret, 0, "configure returned %d", ret);

	for (uint32_t i = 0U; i < 1000U; i++) {
		for (uint32_t n = 0U; n < I2S_TX_QUEUE; n++) {
			ret = i2s_block_write(dev, cfg.block_size, 0x25);
			zassert_equal(ret, 0, "fill %u.%u returned %d", i, n, ret);
		}
		ret = i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_DROP);
		zassert_equal(ret, 0, "DROP %u returned %d", i, ret);
		zassert_equal(i2s_tx_used(), 0U, "fill %u left %u blocks",
			      i, i2s_tx_used());
	}
}

static volatile bool fuzz_run;
static volatile int fuzz_bad;

static bool fuzz_ret_ok(int ret)
{
	return ret == 0 || ret == -EIO || ret == -EINVAL || ret == -ENOMSG ||
	       ret == -EAGAIN || ret == -EBUSY || ret == -ENOMEM;
}

static void fuzz_fn(void *p1, void *p2, void *p3)
{
	const struct device *dev = p1;
	static const enum i2s_dir dirs[] = {
		I2S_DIR_TX, I2S_DIR_RX, I2S_DIR_BOTH
	};
	static const enum i2s_trigger_cmd cmds[] = {
		I2S_TRIGGER_START, I2S_TRIGGER_STOP, I2S_TRIGGER_DRAIN,
		I2S_TRIGGER_DROP, I2S_TRIGGER_PREPARE
	};

	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	while (fuzz_run) {
		uint32_t pick = k_cycle_get_32();
		int ret = i2s_trigger(dev, dirs[pick % ARRAY_SIZE(dirs)],
				      cmds[(pick / 3U) % ARRAY_SIZE(cmds)]);

		if (!fuzz_ret_ok(ret) && fuzz_bad == 0) {
			fuzz_bad = ret;
		}
		k_msleep(1);
	}
}

ZTEST(i2s_stress, test_stress_trigger_fuzz)
{
	const struct device *dev = i2s_dev();
	struct i2s_config tx;
	struct i2s_config rx;
	struct k_thread thread;
	int64_t end;
	int ret;

	i2s_setup_default(&tx, &i2s_tx_slab);
	rx = tx;
	rx.mem_slab = &i2s_rx_slab;
	tx.timeout = 20;
	rx.timeout = 20;

	ret = i2s_configure(dev, I2S_DIR_TX, &tx);
	zassert_equal(ret, 0, "TX configure returned %d", ret);
	ret = i2s_configure(dev, I2S_DIR_RX, &rx);
	zassert_equal(ret, 0, "RX configure returned %d", ret);

	fuzz_bad = 0;
	fuzz_run = true;
	k_thread_create(&thread, stress_fuzz_stack,
			K_THREAD_STACK_SIZEOF(stress_fuzz_stack),
			fuzz_fn, (void *)dev, NULL, NULL,
			K_PRIO_PREEMPT(6), 0, K_NO_WAIT);

	end = k_uptime_get() + STRESS_FUZZ_MS;
	while (k_uptime_get() < end && fuzz_bad == 0) {
		ret = i2s_block_write(dev, tx.block_size, 0x26);
		if (!fuzz_ret_ok(ret)) {
			fuzz_bad = ret;
			break;
		}
		k_msleep(2);
	}

	fuzz_run = false;
	(void)i2s_trigger(dev, I2S_DIR_BOTH, I2S_TRIGGER_DROP);
	ret = k_thread_join(&thread, K_SECONDS(2));
	zassert_equal(ret, 0, "fuzz thread did not stop");
	zassert_equal(fuzz_bad, 0, "unexpected errno %d", fuzz_bad);

	i2s_stream_reset(dev);
	i2s_setup_default(&tx, &i2s_tx_slab);
	ret = i2s_configure(dev, I2S_DIR_TX, &tx);
	zassert_equal(ret, 0, "configure after fuzz returned %d", ret);
	stress_check_slabs("fuzz");
}

static bool stress_has_byte(const void *block, size_t size, uint8_t byte)
{
	const uint8_t *p = block;

	for (size_t i = 0U; i < size; i++) {
		if (p[i] == byte) {
			return true;
		}
	}
	return false;
}

ZTEST(i2s_stress, test_stress_continuous_full_duplex)
{
	const struct device *dev = i2s_dev();
	struct i2s_config tx;
	struct i2s_config rx;
	struct i2s_tx_feed feed = { 0 };
	size_t block = i2s_block_bytes(I2S_FRAMES, 16U);
	int64_t end;
	uint32_t got = 0U;
	uint32_t marked = 0U;
	int tx_prep;
	int rx_prep;
	int ret;

	if (!IS_ENABLED(CONFIG_I2S_GPIO_LOOPBACK)) {
		ztest_test_skip();
		return;
	}

	i2s_setup_cfg(&tx, &i2s_tx_slab, STRESS_FD_RATE, 16U, I2S_CHANNELS,
		     block, 50, I2S_FMT_DATA_FORMAT_I2S,
		     I2S_OPT_BIT_CLK_MASTER | I2S_OPT_FRAME_CLK_MASTER);
	rx = tx;
	rx.mem_slab = &i2s_rx_slab;

	ret = i2s_configure(dev, I2S_DIR_TX, &tx);
	zassert_equal(ret, 0, "TX configure returned %d", ret);
	ret = i2s_configure(dev, I2S_DIR_RX, &rx);
	zassert_equal(ret, 0, "RX configure returned %d", ret);

	for (uint32_t i = 0U; i < 4U; i++) {
		ret = i2s_block_write(dev, block, STRESS_FD_MARK);
		zassert_equal(ret, 0, "prime %u returned %d", i, ret);
	}

	feed.dev = dev;
	feed.size = block;
	feed.run = true;
	k_thread_create(&stress_fd_thread, stress_feed_stack,
			K_THREAD_STACK_SIZEOF(stress_feed_stack),
			i2s_tx_feed_fn, &feed, NULL, NULL,
			K_PRIO_PREEMPT(2), 0, K_NO_WAIT);

	ret = i2s_trigger(dev, I2S_DIR_BOTH, I2S_TRIGGER_START);
	if (ret != 0) {
		feed.run = false;
		(void)i2s_trigger(dev, I2S_DIR_BOTH, I2S_TRIGGER_DROP);
		(void)k_thread_join(&stress_fd_thread, K_SECONDS(2));
		(void)i2s_trigger(dev, I2S_DIR_BOTH, I2S_TRIGGER_DROP);
	}
	zassert_equal(ret, 0, "BOTH START returned %d", ret);

	end = k_uptime_get() + STRESS_FD_MS;
	while (k_uptime_get() < end) {
		void *rx_block = NULL;
		size_t rx_size = 0U;

		ret = i2s_read(dev, &rx_block, &rx_size);
		if (ret != 0) {
			k_msleep(1);
			continue;
		}
		got++;
		if (stress_has_byte(rx_block, rx_size, STRESS_FD_MARK)) {
			marked++;
		}
		k_mem_slab_free(&i2s_rx_slab, rx_block);
		if ((got & 3U) == 0U) {
			k_msleep(1);
		}
	}

	/* PREPARE succeeds only from ERROR. Sample it while the feed is
	 * still running, then stop.
	 */
	tx_prep = i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_PREPARE);
	rx_prep = i2s_trigger(dev, I2S_DIR_RX, I2S_TRIGGER_PREPARE);
	feed.run = false;
	(void)i2s_trigger(dev, I2S_DIR_BOTH, I2S_TRIGGER_DROP);
	zassert_equal(k_thread_join(&stress_fd_thread, K_SECONDS(2)), 0,
		      "feed thread did not stop");
	(void)i2s_trigger(dev, I2S_DIR_BOTH, I2S_TRIGGER_DROP);

	LOG_INF("full duplex %u ms: rx %u blocks, %u carried 0x%02x, feed %d",
		STRESS_FD_MS, got, marked, STRESS_FD_MARK, feed.fail);
	zassert_equal(feed.fail, 0, "TX feed hit %d", feed.fail);
	zassert_equal(tx_prep, -EIO, "TX entered ERROR (%d)", tx_prep);
	zassert_equal(rx_prep, -EIO, "RX entered ERROR (%d)", rx_prep);
	zassert_true(got >= 1000U, "RX delivered only %u blocks", got);
	zassert_true(marked >= got / 2U,
		     "only %u of %u RX blocks carried the TX pattern",
		     marked, got);
	stress_check_slabs("full duplex");
}

static volatile bool mt_run;
static volatile int mt_bad;

static bool stress_ret_ok(int ret)
{
	return ret == 0 || ret == -EIO || ret == -EINVAL || ret == -ENOMSG ||
	       ret == -EAGAIN || ret == -EBUSY || ret == -ENOMEM;
}

static void mt_write_fn(void *p1, void *p2, void *p3)
{
	const struct device *dev = p1;
	size_t size = (size_t)(uintptr_t)p2;

	ARG_UNUSED(p3);

	while (mt_run) {
		int ret = i2s_block_write(dev, size, 0x3C);

		if (!stress_ret_ok(ret) && mt_bad == 0) {
			mt_bad = ret;
		}
		if (ret != 0) {
			k_msleep(1);
		}
	}
}

static void mt_trig_fn(void *p1, void *p2, void *p3)
{
	const struct device *dev = p1;

	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	while (mt_run) {
		int ret = i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_START);

		if (!stress_ret_ok(ret) && mt_bad == 0) {
			mt_bad = ret;
		}
		k_msleep(2);
		ret = i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_DROP);
		if (!stress_ret_ok(ret) && mt_bad == 0) {
			mt_bad = ret;
		}
		k_msleep(2);
	}
}

ZTEST(i2s_stress, test_stress_multi_thread_access)
{
	const struct device *dev = i2s_dev();
	struct i2s_config cfg;
	size_t block;
	int ret;

	i2s_setup_default(&cfg, &i2s_tx_slab);
	cfg.timeout = 20;
	block = cfg.block_size;
	ret = i2s_configure(dev, I2S_DIR_TX, &cfg);
	zassert_equal(ret, 0, "configure returned %d", ret);

	mt_bad = 0;
	mt_run = true;
	k_thread_create(&stress_mt_write_thread, stress_feed_stack,
			K_THREAD_STACK_SIZEOF(stress_feed_stack),
			mt_write_fn, (void *)dev, (void *)(uintptr_t)block,
			NULL, K_PRIO_PREEMPT(3), 0, K_NO_WAIT);
	k_thread_create(&stress_mt_trig_thread, stress_fuzz_stack,
			K_THREAD_STACK_SIZEOF(stress_fuzz_stack),
			mt_trig_fn, (void *)dev, NULL, NULL,
			K_PRIO_PREEMPT(5), 0, K_NO_WAIT);

	k_msleep(STRESS_MT_MS);

	mt_run = false;
	(void)i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_DROP);
	zassert_equal(k_thread_join(&stress_mt_write_thread, K_SECONDS(2)), 0,
		      "writer thread did not stop");
	zassert_equal(k_thread_join(&stress_mt_trig_thread, K_SECONDS(2)), 0,
		      "trigger thread did not stop");
	(void)i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_DROP);
	zassert_equal(mt_bad, 0, "unexpected errno %d", mt_bad);

	i2s_stream_reset(dev);
	i2s_setup_default(&cfg, &i2s_tx_slab);
	ret = i2s_configure(dev, I2S_DIR_TX, &cfg);
	zassert_equal(ret, 0, "configure after concurrent access returned %d",
		      ret);
	stress_check_slabs("multi-thread");
	LOG_INF("writer and trigger threads shared TX for %u ms", STRESS_MT_MS);
}

ZTEST_SUITE(i2s_stress, NULL, stress_setup, stress_before, NULL, NULL);

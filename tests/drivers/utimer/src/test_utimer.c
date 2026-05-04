/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/drivers/counter.h>
#include <zephyr/devicetree.h>
#include <zephyr/ztest.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/atomic.h>

#include "utimer.h"
#if defined(CONFIG_SOC_AB1C1F4M51820HH0_RTSS_HE) || \
		defined(CONFIG_SOC_AB1C1F4M51820PH0_RTSS_HE) || \
		defined(CONFIG_SOC_AB1C1F1M41820HH0_RTSS_HE) || \
		defined(CONFIG_SOC_AB1C1F1M41820PH0_RTSS_HE) || \
		defined(CONFIG_SOC_AE722F80F55D5XX_RTSS_HE) || \
		defined(CONFIG_SOC_AE722F80F55D5XX_RTSS_HP) || \
		defined(CONFIG_SOC_AE822FA0E5597XX0_RTSS_HE) || \
		defined(CONFIG_SOC_AE822FA0E5597XX0_RTSS_HP)
#define UTIMER_SKIP_UNIT_ERRORS 0
#else
#define UTIMER_SKIP_UNIT_ERRORS 1
#endif
#define TEST_WAIT_MS 1000
#define MAX_TEST_CHANNELS 2
#define UTIMER_NUM_CHANNELS 2U
#define STRESS_ITERATIONS 100
#define PERF_ITERATIONS 1000
#define STRESS_ALARM_US 50000
#define BACK_TO_BACK_COUNT 20

struct test_ctx {
	struct k_sem alarm_sem[MAX_TEST_CHANNELS];
	atomic_t alarm_count[MAX_TEST_CHANNELS];
	volatile uint32_t alarm_ticks[MAX_TEST_CHANNELS];
	struct k_sem top_sem;
	atomic_t top_count;
	volatile uint32_t top_ticks;
};

static const struct device *const counter_dev =
	DEVICE_DT_GET(DT_INST(0, alif_utimer_counter));

#if DT_NODE_EXISTS(DT_NODELABEL(utimer0))
#define UTIMER_NODE DT_NODELABEL(utimer0)
#elif DT_NODE_EXISTS(DT_NODELABEL(lputimer0))
#define UTIMER_NODE DT_NODELABEL(lputimer0)
#else
#error "No UTIMER node label found"
#endif

static const uint32_t utimer_timer_base =
	DT_REG_ADDR_BY_NAME(UTIMER_NODE, timer);

static void test_alarm_cb(const struct device *dev, uint8_t chan_id,
		uint32_t ticks, void *user_data)
{
	struct test_ctx *ctx = user_data;

	ARG_UNUSED(dev);
	if (chan_id >= MAX_TEST_CHANNELS) {
		return;
	}

	ctx->alarm_ticks[chan_id] = ticks;
	atomic_inc(&ctx->alarm_count[chan_id]);
	k_sem_give(&ctx->alarm_sem[chan_id]);
}

static void reset_alarm_state(struct test_ctx *ctx, uint8_t chan)
{
	if (chan >= MAX_TEST_CHANNELS) {
		return;
	}

	while (k_sem_take(&ctx->alarm_sem[chan], K_NO_WAIT) == 0) {
	}
	atomic_set(&ctx->alarm_count[chan], 0);
}

static void ensure_alarm_disabled(const struct device *dev, uint8_t chan)
{
	/* Best-effort cleanup to avoid driver assert on enabled CC interrupt. */
	(void)counter_cancel_channel_alarm(dev, chan);
	alif_utimer_disable_compare_match(utimer_timer_base, chan);
	alif_utimer_disable_interrupt(utimer_timer_base, chan);
	alif_utimer_clear_interrupt(utimer_timer_base, chan);
}

static void *utimer_setup(void)
{
	int err;

	zassert_true(device_is_ready(counter_dev), "counter device not ready");
	for (uint8_t ch = 0; ch < MIN(UTIMER_NUM_CHANNELS, MAX_TEST_CHANNELS); ch++) {
		ensure_alarm_disabled(counter_dev, ch);
	}
	err = counter_start(counter_dev);
	zassert_true(err == 0 || err == -EALREADY,
			"counter start failed in setup: %d", err);
	return NULL;
}

/* Reset counter state before each test: disable all alarms, ensure running. */
static void utimer_before_each(void *unused)
{
	ARG_UNUSED(unused);
	for (uint8_t ch = 0; ch < MIN(UTIMER_NUM_CHANNELS, MAX_TEST_CHANNELS); ch++) {
		ensure_alarm_disabled(counter_dev, ch);
	}
	(void)counter_start(counter_dev);
}

static void init_test_ctx(struct test_ctx *ctx, uint8_t channels)
{
	for (uint8_t i = 0; i < MIN(channels, MAX_TEST_CHANNELS); i++) {
		k_sem_init(&ctx->alarm_sem[i], 0, 1);
		atomic_set(&ctx->alarm_count[i], 0);
	}
	k_sem_init(&ctx->top_sem, 0, 1);
	atomic_set(&ctx->top_count, 0);
}

ZTEST(alif_utimer, test_utimer_unit_errors)
{
#if UTIMER_SKIP_UNIT_ERRORS
	ztest_test_skip();
	return;
#else
	uint8_t primary_chan = (UTIMER_NUM_CHANNELS > 1U) ? 1U : 0U;
	uint32_t top = counter_get_top_value(counter_dev);
	struct counter_alarm_cfg cfg = { 0 };
	struct counter_top_cfg top_cfg = { 0 };
	int err;

	/* Invalid channel for cancel alarm */
	{
		uint8_t invalid_chan = UTIMER_NUM_CHANNELS + 1U;

		err = counter_cancel_channel_alarm(counter_dev, invalid_chan);
		zassert_true(err == -ENOTSUP || err == -EINVAL || err == 0,
				"cancel invalid channel unexpected return: %d", err);
	}

	/* Invalid ticks (greater than top) */
#if !defined(CONFIG_SOC_FAMILY_ALIF_BALLETO)
	if (top != UINT32_MAX) {
		cfg.flags = 0;
		cfg.ticks = top + 1U;
		cfg.callback = test_alarm_cb;
		cfg.user_data = NULL;
		err = counter_set_channel_alarm(counter_dev, primary_chan, &cfg);
		zassert_equal(err, -EINVAL, "alarm ticks > top should fail");
	}
#else
	ARG_UNUSED(top);
#endif

	/* Guard period greater than top */
#if !defined(CONFIG_SOC_FAMILY_ALIF_BALLETO)
	if (top != UINT32_MAX) {
		err = counter_set_guard_period(counter_dev, top + 1U,
				COUNTER_GUARD_PERIOD_LATE_TO_SET);
		zassert_equal(err, -EINVAL, "guard period > top should fail");
	}
#endif

	/* Busy alarm check - use top value so alarm never fires during test */
	cfg.flags = 0;
	cfg.ticks = top;
	cfg.callback = test_alarm_cb;
	cfg.user_data = NULL;
	ensure_alarm_disabled(counter_dev, primary_chan);
	err = counter_set_channel_alarm(counter_dev, primary_chan, &cfg);
	zassert_equal(err, 0, "alarm set failed");
	err = counter_set_channel_alarm(counter_dev, primary_chan, &cfg);
	zassert_equal(err, -EBUSY, "second alarm set should be busy");
	ensure_alarm_disabled(counter_dev, primary_chan);

	/* Top set while alarm active should return -EBUSY */
	cfg.flags = 0;
	cfg.ticks = top;
	cfg.callback = test_alarm_cb;
	cfg.user_data = NULL;
	err = counter_set_channel_alarm(counter_dev, primary_chan, &cfg);
	zassert_equal(err, 0, "alarm set failed");
	top_cfg.ticks = (top == UINT32_MAX) ? 0xFFFFFFFEU : (top - 1U);
	top_cfg.flags = 0;
	top_cfg.callback = NULL;
	top_cfg.user_data = NULL;
	err = counter_set_top_value(counter_dev, &top_cfg);
	zassert_equal(err, -EBUSY, "top set with active alarm should be busy");
	ensure_alarm_disabled(counter_dev, primary_chan);
#endif
}

ZTEST(alif_utimer, test_utimer_get_top_value)
{
	uint32_t top;

	top = counter_get_top_value(counter_dev);
	/* Default top value configured in driver is UINT32_MAX */
	zassert_true(top > 0U, "top value should be greater than zero");
	zassert_equal(top, UINT32_MAX,
			"default top value should be UINT32_MAX");
}

ZTEST(alif_utimer, test_utimer_get_pending_int)
{
	uint32_t pending;

	/* Read pending interrupt status - should not crash and returns a value */
	pending = counter_get_pending_int(counter_dev);
	/* At idle, no overflow or compare interrupts should be pending */
	zassert_equal(pending, 0U,
			"pending interrupt should be 0 at idle, got %u",
			pending);
}

ZTEST(alif_utimer, test_utimer_set_alarm_invalid_channel)
{
#if UTIMER_SKIP_UNIT_ERRORS
	ztest_test_skip();
	return;
#else
	uint8_t invalid_chan = UTIMER_NUM_CHANNELS + 1U;
	struct counter_alarm_cfg cfg = { 0 };
	int err;

	cfg.flags = 0;
	cfg.ticks = 1U;
	cfg.callback = test_alarm_cb;
	cfg.user_data = NULL;

	err = counter_set_channel_alarm(counter_dev, invalid_chan, &cfg);
	zassert_true(err != 0,
			"set_alarm with invalid channel should fail, got %d", err);
#endif
}

ZTEST(alif_utimer, test_utimer_guard_period_boundary)
{
#if UTIMER_SKIP_UNIT_ERRORS
	ztest_test_skip();
	return;
#else
	uint32_t top = counter_get_top_value(counter_dev);
	int err;

	/* Guard period equal to top should succeed */
	err = counter_set_guard_period(counter_dev, top,
			COUNTER_GUARD_PERIOD_LATE_TO_SET);
	zassert_equal(err, 0,
			"guard period == top should succeed, got %d", err);

	/* Guard period greater than top should fail */
	if (top != UINT32_MAX) {
		err = counter_set_guard_period(counter_dev, top + 1U,
				COUNTER_GUARD_PERIOD_LATE_TO_SET);
		zassert_equal(err, -EINVAL,
				"guard period > top should return -EINVAL");
	}

	/* Reset guard period to 0 */
	err = counter_set_guard_period(counter_dev, 0U,
			COUNTER_GUARD_PERIOD_LATE_TO_SET);
	zassert_equal(err, 0, "reset guard period to 0 failed");
	zassert_equal(counter_get_guard_period(counter_dev,
			COUNTER_GUARD_PERIOD_LATE_TO_SET),
			0U, "guard period readback should be 0");
#endif
}

ZTEST(alif_utimer, test_utimer_stress_alarm_set_cancel)
{
	struct counter_alarm_cfg cfg = { 0 };
	uint8_t chan = 0;
	int err;
	uint32_t top = counter_get_top_value(counter_dev);
	int fail_count = 0;

	cfg.flags = 0;
	cfg.ticks = top;
	cfg.callback = test_alarm_cb;
	cfg.user_data = NULL;

	for (int i = 0; i < STRESS_ITERATIONS; i++) {
		ensure_alarm_disabled(counter_dev, chan);
		err = counter_set_channel_alarm(counter_dev, chan, &cfg);
		if (err != 0) {
			fail_count++;
			continue;
		}
		err = counter_cancel_channel_alarm(counter_dev, chan);
		if (err != 0) {
			fail_count++;
		}
	}
	ensure_alarm_disabled(counter_dev, chan);
	zassert_equal(fail_count, 0,
			"stress set/cancel: %d failures in %d iterations",
			fail_count, STRESS_ITERATIONS);
	TC_PRINT("stress set/cancel: %d iterations OK\n", STRESS_ITERATIONS);
}

ZTEST(alif_utimer, test_utimer_stress_back_to_back_alarms)
{
	struct test_ctx ctx = { 0 };
	struct counter_alarm_cfg cfg = { 0 };
	uint8_t chan = 0;
	uint32_t ticks;
	int err;
	int cb_count = 0;

	init_test_ctx(&ctx, UTIMER_NUM_CHANNELS);

	ticks = counter_us_to_ticks(counter_dev, STRESS_ALARM_US);
	if (ticks == 0U) {
		ticks = 1U;
	}

	for (int i = 0; i < BACK_TO_BACK_COUNT; i++) {
		ensure_alarm_disabled(counter_dev, chan);
		reset_alarm_state(&ctx, chan);
		cfg.flags = 0;
		cfg.ticks = ticks;
		cfg.callback = test_alarm_cb;
		cfg.user_data = &ctx;

		err = counter_set_channel_alarm(counter_dev, chan, &cfg);
		zassert_equal(err, 0, "back-to-back alarm set failed at iter %d", i);

		err = k_sem_take(&ctx.alarm_sem[chan], K_MSEC(TEST_WAIT_MS));
		if (err == 0) {
			cb_count++;
		}
	}
	ensure_alarm_disabled(counter_dev, chan);
	zassert_equal(cb_count, BACK_TO_BACK_COUNT,
			"back-to-back: only %d/%d callbacks received",
			cb_count, BACK_TO_BACK_COUNT);
	TC_PRINT("back-to-back alarms: %d/%d callbacks OK\n",
			cb_count, BACK_TO_BACK_COUNT);
}

ZTEST(alif_utimer, test_utimer_stress_top_value_changes)
{
	uint32_t top = counter_get_top_value(counter_dev);
	uint32_t new_top;
	struct counter_top_cfg cfg = { 0 };
	int err;
	int fail_count = 0;

	for (int i = 0; i < STRESS_ITERATIONS; i++) {
		new_top = (top == UINT32_MAX) ? (UINT32_MAX - (uint32_t)i) : (top - (uint32_t)i);
		if (new_top == 0U) {
			new_top = 1U;
		}
		cfg.flags = 0;
		cfg.ticks = new_top;
		cfg.callback = NULL;
		cfg.user_data = NULL;
		err = counter_set_top_value(counter_dev, &cfg);
		if (err != 0) {
			fail_count++;
		}
	}

	/* Restore original top */
	cfg.flags = 0;
	cfg.ticks = top;
	cfg.callback = NULL;
	cfg.user_data = NULL;
	(void)counter_set_top_value(counter_dev, &cfg);

	zassert_equal(fail_count, 0,
			"stress top changes: %d failures in %d iterations",
			fail_count, STRESS_ITERATIONS);
	TC_PRINT("stress top value changes: %d iterations OK\n", STRESS_ITERATIONS);
}

ZTEST(alif_utimer, test_utimer_stress_multichannel_concurrent)
{
	struct test_ctx ctx = { 0 };
	struct counter_alarm_cfg cfg0 = { 0 };
	struct counter_alarm_cfg cfg1 = { 0 };
	uint32_t ticks;
	int err;
	int success_count = 0;

	if (UTIMER_NUM_CHANNELS < 2U) {
		TC_PRINT("fewer than 2 channels, skipping concurrent test\n");
		ztest_test_skip();
		return;
	}

	init_test_ctx(&ctx, UTIMER_NUM_CHANNELS);

	ticks = counter_us_to_ticks(counter_dev, STRESS_ALARM_US);
	if (ticks == 0U) {
		ticks = 1U;
	}

	for (int i = 0; i < BACK_TO_BACK_COUNT; i++) {
		ensure_alarm_disabled(counter_dev, 0);
		ensure_alarm_disabled(counter_dev, 1);
		reset_alarm_state(&ctx, 0);
		reset_alarm_state(&ctx, 1);

		cfg0.flags = 0;
		cfg0.ticks = ticks;
		cfg0.callback = test_alarm_cb;
		cfg0.user_data = &ctx;

		cfg1.flags = 0;
		cfg1.ticks = ticks;
		cfg1.callback = test_alarm_cb;
		cfg1.user_data = &ctx;

		err = counter_set_channel_alarm(counter_dev, 0, &cfg0);
		zassert_equal(err, 0, "concurrent ch0 alarm set failed iter %d", i);
		err = counter_set_channel_alarm(counter_dev, 1, &cfg1);
		zassert_equal(err, 0, "concurrent ch1 alarm set failed iter %d", i);

		err = k_sem_take(&ctx.alarm_sem[0], K_MSEC(TEST_WAIT_MS));
		zassert_equal(err, 0, "concurrent ch0 callback timeout iter %d", i);
		err = k_sem_take(&ctx.alarm_sem[1], K_MSEC(TEST_WAIT_MS));
		zassert_equal(err, 0, "concurrent ch1 callback timeout iter %d", i);

		success_count++;
	}
	ensure_alarm_disabled(counter_dev, 0);
	ensure_alarm_disabled(counter_dev, 1);
	zassert_equal(success_count, BACK_TO_BACK_COUNT,
			"concurrent: %d/%d iterations succeeded",
			success_count, BACK_TO_BACK_COUNT);
	TC_PRINT("concurrent multi-channel: %d iterations OK\n", BACK_TO_BACK_COUNT);
}

ZTEST(alif_utimer, test_utimer_perf_counter_read)
{
	uint32_t val;
	uint32_t start_cycles, end_cycles;
	uint32_t total_ns;
	int err;
	int fail_count = 0;

	start_cycles = k_cycle_get_32();
	for (int i = 0; i < PERF_ITERATIONS; i++) {
		err = counter_get_value(counter_dev, &val);
		if (err != 0) {
			fail_count++;
		}
	}
	end_cycles = k_cycle_get_32();

	total_ns = k_cyc_to_ns_ceil32(end_cycles - start_cycles);

	zassert_equal(fail_count, 0,
			"counter read: %d failures in %d reads", fail_count, PERF_ITERATIONS);
	TC_PRINT("perf counter_get_value: %u reads in %u ns, avg %u ns/read\n",
			PERF_ITERATIONS, total_ns, total_ns / PERF_ITERATIONS);
}

ZTEST(alif_utimer, test_utimer_perf_alarm_latency)
{
	struct test_ctx ctx = { 0 };
	struct counter_alarm_cfg cfg = { 0 };
	uint8_t chan = 0;
	uint32_t ticks;
	uint32_t start_cycles, end_cycles;
	uint32_t latency_ns, total_ns = 0;
	int err;
	int measured = 0;

	init_test_ctx(&ctx, UTIMER_NUM_CHANNELS);

	ticks = counter_us_to_ticks(counter_dev, STRESS_ALARM_US);
	if (ticks == 0U) {
		ticks = 1U;
	}

	for (int i = 0; i < BACK_TO_BACK_COUNT; i++) {
		ensure_alarm_disabled(counter_dev, chan);
		reset_alarm_state(&ctx, chan);
		cfg.flags = 0;
		cfg.ticks = ticks;
		cfg.callback = test_alarm_cb;
		cfg.user_data = &ctx;

		start_cycles = k_cycle_get_32();
		err = counter_set_channel_alarm(counter_dev, chan, &cfg);
		if (err != 0) {
			continue;
		}
		err = k_sem_take(&ctx.alarm_sem[chan], K_MSEC(TEST_WAIT_MS));
		end_cycles = k_cycle_get_32();

		if (err == 0) {
			latency_ns = k_cyc_to_ns_ceil32(end_cycles - start_cycles);
			total_ns += latency_ns;
			measured++;
		}
	}
	ensure_alarm_disabled(counter_dev, chan);
	zassert_true(measured > 0, "no alarm latency measurements obtained");
	TC_PRINT("perf alarm latency: avg %u ns over %d measurements (ticks=%u)\n",
		 total_ns / measured, measured, ticks);
}

ZTEST_SUITE(alif_utimer, NULL, utimer_setup, utimer_before_each, NULL, NULL);

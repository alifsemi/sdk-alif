/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include "test_dac.h"

LOG_MODULE_DECLARE(dac_test, LOG_LEVEL_INF);

/* -------- Test: Continuous ramp-up from 0 to MAX -------- */
static void dac_continuous_ramp_up_test(void)
{
	int ret;
	uint32_t errors = 0;

	LOG_INF("=== DAC Continuous Ramp-Up Test ===");

	dac_dev = init_dac_device();

	for (uint32_t val = DAC_MIN_INPUT; val <= DAC_MAX_INPUT;
	     val += DAC_RAMP_STEP) {
		ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, val);
		if (ret != 0) {
			errors++;
			LOG_ERR("Ramp-up write %u failed [%d]", val, ret);
		}
		k_sleep(K_MSEC(50));
	}

	/* Write final max value */
	ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, DAC_MAX_INPUT);
	zassert_equal(ret, 0, "Final max write failed [%d]", ret);
	k_sleep(K_MSEC(50));

	zassert_equal(errors, 0, "Ramp-up had %u write errors", errors);

	LOG_INF("DAC continuous ramp-up test PASSED");
}

/* -------- Test: Continuous ramp-down from MAX to 0 -------- */
static void dac_continuous_ramp_down_test(void)
{
	int ret;
	uint32_t errors = 0;

	LOG_INF("=== DAC Continuous Ramp-Down Test ===");

	dac_dev = init_dac_device();

	for (int32_t val = DAC_MAX_INPUT; val >= (int32_t)DAC_MIN_INPUT;
	     val -= DAC_RAMP_STEP) {
		ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, (uint32_t)val);
		if (ret != 0) {
			errors++;
			LOG_ERR("Ramp-down write %d failed [%d]", val, ret);
		}
		k_sleep(K_MSEC(50));
	}

	/* Write final min value */
	ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, DAC_MIN_INPUT);
	zassert_equal(ret, 0, "Final min write failed [%d]", ret);
	k_sleep(K_MSEC(50));

	zassert_equal(errors, 0, "Ramp-down had %u write errors", errors);

	LOG_INF("DAC continuous ramp-down test PASSED");
}

/* -------- Test: Sawtooth waveform pattern -------- */
static void dac_continuous_sawtooth_test(void)
{
	int ret;
	uint32_t errors = 0;
	int cycles = 5;

	LOG_INF("=== DAC Continuous Sawtooth Test (%d cycles) ===", cycles);

	dac_dev = init_dac_device();

	for (int c = 0; c < cycles; c++) {
		/* Ramp up */
		for (uint32_t val = DAC_MIN_INPUT; val <= DAC_MAX_INPUT;
		     val += DAC_RAMP_STEP) {
			ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, val);
			if (ret != 0) {
				errors++;
			}
			k_sleep(K_MSEC(50));
		}
		/* Reset to min (sawtooth drop) */
		ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, DAC_MIN_INPUT);
		if (ret != 0) {
			errors++;
		}
		k_sleep(K_MSEC(100));
	}

	zassert_equal(errors, 0, "Sawtooth had %u write errors", errors);

	LOG_INF("DAC continuous sawtooth test PASSED");
}

/* -------- Test: Triangle waveform pattern -------- */
static void dac_continuous_triangle_test(void)
{
	int ret;
	uint32_t errors = 0;
	int cycles = 5;

	LOG_INF("=== DAC Continuous Triangle Test (%d cycles) ===", cycles);

	dac_dev = init_dac_device();

	for (int c = 0; c < cycles; c++) {
		/* Ramp up */
		for (uint32_t val = DAC_MIN_INPUT; val <= DAC_MAX_INPUT;
		     val += DAC_RAMP_STEP) {
			ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, val);
			if (ret != 0) {
				errors++;
			}
			k_sleep(K_MSEC(50));
		}
		/* Ramp down */
		for (int32_t val = DAC_MAX_INPUT; val >= (int32_t)DAC_MIN_INPUT;
		     val -= DAC_RAMP_STEP) {
			ret = dac_write_value(dac_dev, DAC_CHANNEL_ID,
					      (uint32_t)val);
			if (ret != 0) {
				errors++;
			}
			k_sleep(K_MSEC(50));
		}
	}

	zassert_equal(errors, 0, "Triangle had %u write errors", errors);

	LOG_INF("DAC continuous triangle test PASSED");
}

/* -------- Test: Square wave pattern (toggle min/max) -------- */
static void dac_continuous_square_wave_test(void)
{
	int ret;
	uint32_t errors = 0;

	LOG_INF("=== DAC Continuous Square Wave Test ===");

	dac_dev = init_dac_device();

	for (int i = 0; i < 10; i++) {  /* Reduced for LA visibility */
		/* High */
		ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, DAC_MAX_INPUT);
		if (ret != 0) {
			errors++;
		}
		k_sleep(K_MSEC(100));

		/* Low */
		ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, DAC_MIN_INPUT);
		if (ret != 0) {
			errors++;
		}
		k_sleep(K_MSEC(100));
	}

	zassert_equal(errors, 0, "Square wave had %u write errors", errors);

	LOG_INF("DAC continuous square wave test PASSED (10 toggles with 100ms delay)");
}

/* -------- Test: Rapid consecutive writes (stress) -------- */
static void dac_continuous_rapid_writes_test(void)
{
	int ret;
	uint32_t errors = 0;

	LOG_INF("=== DAC Continuous Rapid Writes Test ===");

	dac_dev = init_dac_device();

	for (int i = 0; i < 100; i++) {  /* Reduced for LA visibility */
		uint32_t val = (i * 7) & DAC_MAX_INPUT; /* pseudo-random */

		ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, val);
		if (ret != 0) {
			errors++;
		}
		k_sleep(K_MSEC(50));
	}

	zassert_equal(errors, 0, "Rapid writes had %u errors out of %d",
		      errors, 100);

	LOG_INF("DAC continuous rapid writes test PASSED (100 writes with 50ms delay)");
}

/* -------- Test: 1 kHz conversion rate timing -------- */
static void dac_continuous_1khz_rate_test(void)
{
	int ret;
	uint32_t errors = 0;
	int num_samples = 100;
	uint32_t start_time, end_time, elapsed_ms;

	LOG_INF("=== DAC 1 kHz Conversion Rate Test ===");

	dac_dev = init_dac_device();

	/* Write num_samples values at 1ms intervals (1 kHz rate) */
	start_time = k_uptime_get_32();

	for (int i = 0; i < num_samples; i++) {
		uint32_t val = (i * DAC_MAX_INPUT) / num_samples;

		ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, val);
		if (ret != 0) {
			errors++;
		}
		/* 1ms delay for 1 kHz rate */
		k_msleep(1);
	}

	end_time = k_uptime_get_32();
	elapsed_ms = end_time - start_time;

	zassert_equal(errors, 0, "1 kHz rate test had %u write errors", errors);

	LOG_INF("1 kHz test: %d samples in %u ms (expected ~%d ms)",
		num_samples, elapsed_ms, num_samples);

	/* Verify timing is reasonable (allow 20% tolerance) */
	zassert_true(elapsed_ms >= (uint32_t)(num_samples * 0.8),
		     "Timing too fast: %u ms", elapsed_ms);

	LOG_INF("DAC 1 kHz conversion rate test PASSED");
}

/* -------- Test: Continuous conversion with varying step sizes -------- */
static void dac_continuous_variable_step_test(void)
{
	int ret;
	uint32_t errors = 0;

	LOG_INF("=== DAC Continuous Variable Step Test ===");

	dac_dev = init_dac_device();

	/* Test with different step sizes to ensure all transitions work */
	uint32_t steps[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024};

	for (int s = 0; s < ARRAY_SIZE(steps); s++) {
		uint32_t step = steps[s];
		uint32_t write_count = 0;
		uint32_t delay_ms = (step >= 64U) ? 20U :
				    (step >= 16U) ? 5U : 0U;
		uint32_t limited_sweep = (step <= 8U);

		for (uint32_t val = 0; val <= DAC_MAX_INPUT; val += step) {
			if (limited_sweep && write_count >= 256U) {
				break;
			}

			ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, val);
			if (ret != 0) {
				errors++;
			}
			write_count++;

			if (delay_ms > 0U) {
				k_sleep(K_MSEC(delay_ms));
			}
		}

		if (limited_sweep) {
			ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, DAC_MAX_INPUT);
			if (ret != 0) {
				errors++;
			}
			write_count++;
		}

		LOG_INF("  Step %u: %u writes OK%s", step, write_count,
			limited_sweep ? " (limited sweep)" : "");
		k_sleep(K_MSEC(100));
	}

	zassert_equal(errors, 0, "Variable step had %u errors", errors);

	LOG_INF("DAC continuous variable step test PASSED");
}

/* -------- Test: Repeated same-value writes (stability) -------- */
static void dac_continuous_repeated_value_test(void)
{
	int ret;
	uint32_t errors = 0;
	uint32_t test_value = DAC_MID_INPUT;

	LOG_INF("=== DAC Continuous Repeated Value Test ===");

	dac_dev = init_dac_device();

	/* Write the same value repeatedly to ensure stability */
	for (int i = 0; i < 50; i++) {  /* Reduced for LA visibility */
		ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, test_value);
		if (ret != 0) {
			errors++;
		}
		k_sleep(K_MSEC(100));
	}

	zassert_equal(errors, 0, "Repeated value had %u errors out of %d",
		      errors, 50);

	LOG_INF("DAC continuous repeated value test PASSED (50 writes of 0x%x with 100ms delay)",
		test_value);
}

/* ======== Continuous Test Suite ======== */

/* R-32_3: Continuous ramp-up */
ZTEST(test_dac_continuous, test_dac_continuous_ramp_up)
{
	dac_continuous_ramp_up_test();
}

/* R-32_3: Continuous ramp-down */
ZTEST(test_dac_continuous, test_dac_continuous_ramp_down)
{
	dac_continuous_ramp_down_test();
}

/* R-32_3: Sawtooth waveform */
ZTEST(test_dac_continuous, test_dac_continuous_sawtooth)
{
	dac_continuous_sawtooth_test();
}

/* R-32_3: Triangle waveform */
ZTEST(test_dac_continuous, test_dac_continuous_triangle)
{
	dac_continuous_triangle_test();
}

/* R-32_3: Square wave */
ZTEST(test_dac_continuous, test_dac_continuous_square_wave)
{
	dac_continuous_square_wave_test();
}

/* R-32_3: Rapid consecutive writes */
ZTEST(test_dac_continuous, test_dac_continuous_rapid_writes)
{
	dac_continuous_rapid_writes_test();
}

/* R-32_3: 1 kHz conversion rate */
ZTEST(test_dac_continuous, test_dac_continuous_1khz_rate)
{
	dac_continuous_1khz_rate_test();
}

/* R-32_3: Variable step sizes */
ZTEST(test_dac_continuous, test_dac_continuous_variable_step)
{
	dac_continuous_variable_step_test();
}

/* R-32_3: Repeated same value (stability) */
ZTEST(test_dac_continuous, test_dac_continuous_repeated_value)
{
	dac_continuous_repeated_value_test();
}

/* ======== Test Suite Lifecycle ======== */

static void *test_dac_continuous_setup(void)
{
	LOG_INF("=== DAC Continuous Test Suite Setup ===");
	dac_dev = init_dac_device();
	zassert_not_null(dac_dev, "init_dac_device() returned NULL");
	return NULL;
}

static void test_dac_continuous_before(void *fixture)
{
	ARG_UNUSED(fixture);
	LOG_INF("--- Before continuous test ---");
}

static void test_dac_continuous_after(void *fixture)
{
	ARG_UNUSED(fixture);
	LOG_INF("--- After continuous test ---");
	/* Reset DAC output to 0 after each test */
	dac_write_value(dac_dev, DAC_CHANNEL_ID, DAC_MIN_INPUT);
}

static void test_dac_continuous_teardown(void *fixture)
{
	ARG_UNUSED(fixture);
	LOG_INF("=== DAC Continuous Test Suite Teardown ===");
}


ZTEST_SUITE(test_dac_continuous, NULL, test_dac_continuous_setup,
	    test_dac_continuous_before, test_dac_continuous_after,
	    test_dac_continuous_teardown);


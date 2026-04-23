/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/**
 * @file test_dac_boundary.c
 * @brief DAC Boundary and Edge Case Tests
 *
 * HRM Coverage:
 *   - 12-bit resolution boundary values
 *   - Analog output range 0V to 1.8V
 *   - Error handling for out-of-range inputs
 */

#include "test_dac.h"

LOG_MODULE_DECLARE(dac_test, LOG_LEVEL_INF);

/* -------- Test: Write exact minimum boundary (0x000) -------- */
static void dac_boundary_min_test(void)
{
	int ret;

	LOG_INF("=== DAC Boundary Min (0x000) Test ===");

	dac_dev = init_dac_device();

	ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, 0x000);
	zassert_equal(ret, 0, "DAC write 0x000 failed [%d]", ret);
	k_sleep(K_MSEC(100));

	LOG_INF("DAC boundary min test PASSED");
}

/* -------- Test: Write exact maximum boundary (0xFFF) -------- */
static void dac_boundary_max_test(void)
{
	int ret;

	LOG_INF("=== DAC Boundary Max (0xFFF) Test ===");

	dac_dev = init_dac_device();

	ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, 0xFFF);
	zassert_equal(ret, 0, "DAC write 0xFFF failed [%d]", ret);
	k_sleep(K_MSEC(100));

	LOG_INF("DAC boundary max test PASSED");
}

/* -------- Test: Write value just above maximum (0x1000) -------- */
static void dac_boundary_above_max_test(void)
{
	int ret;

	LOG_INF("=== DAC Boundary Above Max (0x1000) Test ===");

	dac_dev = init_dac_device();

	/* Value 0x1000 exceeds 12-bit range; driver should return -EINVAL
	 * when twoscomp is not enabled
	 */
	ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, 0x1000);
	zassert_equal(ret, -EINVAL,
		      "Expected -EINVAL for 0x1000, got [%d]", ret);

	LOG_INF("DAC boundary above max test PASSED");
}

/* -------- Test: Write large out-of-range value (0xFFFF) -------- */
static void dac_boundary_large_value_test(void)
{
	int ret;

	LOG_INF("=== DAC Boundary Large Value (0xFFFF) Test ===");

	dac_dev = init_dac_device();

	ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, 0xFFFF);
	zassert_equal(ret, -EINVAL,
		      "Expected -EINVAL for 0xFFFF, got [%d]", ret);

	ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, 0xFFFFFFFF);
	zassert_equal(ret, -EINVAL,
		      "Expected -EINVAL for 0xFFFFFFFF, got [%d]", ret);

	LOG_INF("DAC boundary large value test PASSED");
}

/* -------- Test: Write value 1 (minimum non-zero) -------- */
static void dac_boundary_min_nonzero_test(void)
{
	int ret;

	LOG_INF("=== DAC Boundary Min Non-Zero (0x001) Test ===");

	dac_dev = init_dac_device();

	ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, 0x001);
	zassert_equal(ret, 0, "DAC write 0x001 failed [%d]", ret);
	k_sleep(K_MSEC(100));

	LOG_INF("DAC boundary min non-zero test PASSED");
}

/* -------- Test: Write value 0xFFE (max minus one) -------- */
static void dac_boundary_max_minus_one_test(void)
{
	int ret;

	LOG_INF("=== DAC Boundary Max-1 (0xFFE) Test ===");

	dac_dev = init_dac_device();

	ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, 0xFFE);
	zassert_equal(ret, 0, "DAC write 0xFFE failed [%d]", ret);
	k_sleep(K_MSEC(100));

	LOG_INF("DAC boundary max-1 test PASSED");
}

/* -------- Test: Alternating min/max writes -------- */
static void dac_boundary_alternating_test(void)
{
	int ret;
	uint32_t errors = 0;

	LOG_INF("=== DAC Boundary Alternating Min/Max Test ===");

	dac_dev = init_dac_device();

	for (int i = 0; i < 10; i++) {  /* Reduced for LA visibility */
		ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, DAC_MIN_INPUT);
		if (ret != 0) {
			errors++;
		}
		k_sleep(K_MSEC(100));

		ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, DAC_MAX_INPUT);
		if (ret != 0) {
			errors++;
		}
		k_sleep(K_MSEC(100));
	}

	zassert_equal(errors, 0,
		      "Alternating min/max had %u errors out of %d writes",
		      errors, 20);

	LOG_INF("DAC boundary alternating min/max test PASSED (10 cycles with 100ms delay)");
}

/* -------- Test: Power-of-two boundary values -------- */
static void dac_boundary_power_of_two_test(void)
{
	int ret;
	uint32_t errors = 0;

	LOG_INF("=== DAC Boundary Power-of-Two Test ===");

	dac_dev = init_dac_device();

	/* Test all power-of-2 values within 12-bit range */
	uint32_t pow2_values[] = {
		1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048
	};

	for (int i = 0; i < ARRAY_SIZE(pow2_values); i++) {
		ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, pow2_values[i]);
		if (ret != 0) {
			errors++;
			LOG_ERR("Write %u failed [%d]", pow2_values[i], ret);
		}
		k_sleep(K_MSEC(100));

		/* Also test (power_of_2 - 1) */
		uint32_t val_minus_1 = pow2_values[i] - 1;

		ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, val_minus_1);
		if (ret != 0) {
			errors++;
			LOG_ERR("Write %u failed [%d]", val_minus_1, ret);
		}
		k_sleep(K_MSEC(100));
	}

	zassert_equal(errors, 0, "Power-of-two test had %u errors", errors);

	LOG_INF("DAC boundary power-of-two test PASSED");
}

/* -------- Test: Repeated setup/write cycles -------- */
static void dac_boundary_repeated_setup_test(void)
{
	int ret;
	int cycles = 10;

	LOG_INF("=== DAC Boundary Repeated Setup/Write Test ===");

	const struct device *dev = DEVICE_DT_GET(DAC_NODE);

	zassert_true(device_is_ready(dev), "DAC device not ready");

	for (int i = 0; i < cycles; i++) {
		const struct dac_channel_cfg cfg = {
			.channel_id = DAC_CHANNEL_ID,
			.resolution = DAC_RESOLUTION,
			.buffered = 0
		};

		ret = dac_channel_setup(dev, &cfg);
		zassert_equal(ret, 0, "Setup cycle %d failed [%d]", i, ret);

		ret = dac_write_value(dev, DAC_CHANNEL_ID,
				      (i * 400) & DAC_MAX_INPUT);
		zassert_equal(ret, 0, "Write cycle %d failed [%d]", i, ret);
		k_sleep(K_MSEC(100));
	}

	LOG_INF("DAC boundary repeated setup/write test PASSED");
}

/* -------- Test: Zero resolution setup (invalid) -------- */
static void dac_boundary_zero_resolution_test(void)
{
	int ret;

	LOG_INF("=== DAC Boundary Zero Resolution Test ===");

	const struct device *dev = DEVICE_DT_GET(DAC_NODE);

	zassert_true(device_is_ready(dev), "DAC device not ready");

	struct dac_channel_cfg cfg = {
		.channel_id = DAC_CHANNEL_ID,
		.resolution = 0,
		.buffered = 0
	};

	ret = dac_channel_setup(dev, &cfg);
	zassert_equal(ret, -ENOTSUP,
		      "Expected -ENOTSUP for resolution 0, got [%d]", ret);

	LOG_INF("DAC boundary zero resolution test PASSED");
}

/* -------- Test: Sequential boundary transitions -------- */
static void dac_boundary_sequential_transitions_test(void)
{
	int ret;

	LOG_INF("=== DAC Boundary Sequential Transitions Test ===");

	dac_dev = init_dac_device();

	/* Test transitions between boundary values */
	uint32_t boundary_values[] = {
		0x000, 0x001, 0x002,           /* near minimum */
		0x7FE, 0x7FF, 0x800, 0x801,   /* around mid-point */
		0xFFD, 0xFFE, 0xFFF            /* near maximum */
	};

	for (int i = 0; i < ARRAY_SIZE(boundary_values); i++) {
		ret = dac_write_value(dac_dev, DAC_CHANNEL_ID,
				      boundary_values[i]);
		zassert_equal(ret, 0, "Write 0x%03x failed [%d]",
			      boundary_values[i], ret);
		LOG_INF("  Written 0x%03x OK", boundary_values[i]);
		k_sleep(K_MSEC(100));
	}

	LOG_INF("DAC boundary sequential transitions test PASSED");
}

/* ======== Boundary Test Suite ======== */

ZTEST(test_dac_boundary, test_dac_boundary_min)
{
	dac_boundary_min_test();
}

ZTEST(test_dac_boundary, test_dac_boundary_max)
{
	dac_boundary_max_test();
}

ZTEST(test_dac_boundary, test_dac_boundary_above_max)
{
	if (DAC_TWOSCOMP_ENABLED) {
		ztest_test_skip();
	}

	dac_boundary_above_max_test();
}

ZTEST(test_dac_boundary, test_dac_boundary_large_value)
{
	if (DAC_TWOSCOMP_ENABLED) {
		ztest_test_skip();
	}

	dac_boundary_large_value_test();
}

ZTEST(test_dac_boundary, test_dac_boundary_min_nonzero)
{
	dac_boundary_min_nonzero_test();
}

ZTEST(test_dac_boundary, test_dac_boundary_max_minus_one)
{
	dac_boundary_max_minus_one_test();
}

ZTEST(test_dac_boundary, test_dac_boundary_alternating)
{
	dac_boundary_alternating_test();
}

ZTEST(test_dac_boundary, test_dac_boundary_power_of_two)
{
	dac_boundary_power_of_two_test();
}

ZTEST(test_dac_boundary, test_dac_boundary_repeated_setup)
{
	dac_boundary_repeated_setup_test();
}

ZTEST(test_dac_boundary, test_dac_boundary_zero_resolution)
{
	dac_boundary_zero_resolution_test();
}

ZTEST(test_dac_boundary, test_dac_boundary_sequential_transitions)
{
	dac_boundary_sequential_transitions_test();
}

/* ======== Test Suite Lifecycle ======== */

static void *test_dac_boundary_setup(void)
{
	LOG_INF("=== DAC Boundary Test Suite Setup ===");
	dac_dev = init_dac_device();
	zassert_not_null(dac_dev, "init_dac_device() returned NULL");
	return NULL;
}

static void test_dac_boundary_before(void *fixture)
{
	ARG_UNUSED(fixture);
	LOG_INF("--- Before boundary test ---");
}

static void test_dac_boundary_after(void *fixture)
{
	ARG_UNUSED(fixture);
	LOG_INF("--- After boundary test ---");
}

static void test_dac_boundary_teardown(void *fixture)
{
	ARG_UNUSED(fixture);
	LOG_INF("=== DAC Boundary Test Suite Teardown ===");
}

ZTEST_SUITE(test_dac_boundary, NULL, test_dac_boundary_setup,
	    test_dac_boundary_before, test_dac_boundary_after,
	    test_dac_boundary_teardown);

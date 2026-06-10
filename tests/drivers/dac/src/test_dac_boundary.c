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

/* -------- Test: Valid boundary values (0x000, 0x001, 0xFFE, 0xFFF) -------- */
static void dac_boundary_valid_values_test(void)
{
	int ret;
	uint32_t valid_boundaries[] = {0x000, 0x001, 0xFFE, 0xFFF};

	TC_PRINT("=== DAC Boundary Valid Values Test ===\n");

	for (int i = 0; i < ARRAY_SIZE(valid_boundaries); i++) {
		ret = dac_write_value(dac_dev, DAC_CHANNEL_ID,
				      valid_boundaries[i]);
		zassert_equal(ret, 0, "DAC write 0x%03x failed [%d]",
			      valid_boundaries[i], ret);
		k_sleep(K_MSEC(100));
	}

	TC_PRINT("DAC boundary valid values test PASSED\n");
}

/* -------- Test: Out-of-range values (0x1000, 0xFFFF, 0xFFFFFFFF) -------- */
static void dac_boundary_out_of_range_test(void)
{
	int ret;
	uint32_t invalid_values[] = {0x1000, 0xFFFF, 0xFFFFFFFF};

	TC_PRINT("=== DAC Boundary Out-of-Range Test ===\n");

	for (int i = 0; i < ARRAY_SIZE(invalid_values); i++) {
		ret = dac_write_value(dac_dev, DAC_CHANNEL_ID,
				      invalid_values[i]);
		zassert_equal(ret, -EINVAL,
			      "Expected -EINVAL for 0x%x, got [%d]",
			      invalid_values[i], ret);
	}

	TC_PRINT("DAC boundary out-of-range test PASSED\n");
}

/* -------- Test: Alternating min/max writes -------- */
static void dac_boundary_alternating_test(void)
{
	int ret;
	uint32_t errors = 0;

	TC_PRINT("=== DAC Boundary Alternating Min/Max Test ===\n");

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

	TC_PRINT("DAC boundary alternating min/max test PASSED (10 cycles with 100ms delay)\n");
}

/* -------- Test: Power-of-two boundary values -------- */
static void dac_boundary_power_of_two_test(void)
{
	int ret;
	uint32_t errors = 0;

	TC_PRINT("=== DAC Boundary Power-of-Two Test ===\n");

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

	TC_PRINT("DAC boundary power-of-two test PASSED\n");
}

/* -------- Test: Repeated setup/write cycles -------- */
static void dac_boundary_repeated_setup_test(void)
{
	int ret;
	int cycles = 10;

	TC_PRINT("=== DAC Boundary Repeated Setup/Write Test ===\n");

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

	TC_PRINT("DAC boundary repeated setup/write test PASSED\n");
}

/* -------- Test: Zero resolution setup (invalid) -------- */
static void dac_boundary_zero_resolution_test(void)
{
	int ret;

	TC_PRINT("=== DAC Boundary Zero Resolution Test ===\n");

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

	TC_PRINT("DAC boundary zero resolution test PASSED\n");
}

/* -------- Test: Sequential boundary transitions -------- */
static void dac_boundary_sequential_transitions_test(void)
{
	int ret;

	TC_PRINT("=== DAC Boundary Sequential Transitions Test ===\n");

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

	TC_PRINT("DAC boundary sequential transitions test PASSED\n");
}

/* ======== Boundary Test Suite ======== */

ZTEST(test_dac_boundary, test_dac_boundary_valid_values)
{
	dac_boundary_valid_values_test();
}

ZTEST(test_dac_boundary, test_dac_boundary_out_of_range)
{
	if (DAC_TWOSCOMP_ENABLED) {
		ztest_test_skip();
	}

	dac_boundary_out_of_range_test();
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
	TC_PRINT("=== DAC Boundary Test Suite Setup ===\n");
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
	TC_PRINT("=== DAC Boundary Test Suite Teardown ===\n");
}

ZTEST_SUITE(test_dac_boundary, NULL, test_dac_boundary_setup,
	    test_dac_boundary_before, test_dac_boundary_after,
	    test_dac_boundary_teardown);

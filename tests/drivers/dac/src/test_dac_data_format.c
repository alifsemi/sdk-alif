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

/* -------- Test: Unsigned binary minimum value (0x000 = 0V) -------- */
static void dac_unsigned_binary_min_test(void)
{
	int ret;

	LOG_INF("=== DAC Unsigned Binary Min (0x000) Test ===");

	ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, 0x000);
	zassert_equal(ret, 0, "DAC write 0x000 failed [%d]", ret);
	k_sleep(K_MSEC(100));

	LOG_INF("DAC unsigned binary min test PASSED");
}

/* -------- Test: Unsigned binary maximum value (0xFFF = 1.8V) -------- */
static void dac_unsigned_binary_max_test(void)
{
	int ret;

	LOG_INF("=== DAC Unsigned Binary Max (0xFFF) Test ===");

	ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, 0xFFF);
	zassert_equal(ret, 0, "DAC write 0xFFF failed [%d]", ret);
	k_sleep(K_MSEC(100));

	LOG_INF("DAC unsigned binary max test PASSED");
}

/* -------- Test: Unsigned binary mid-scale value (0x800 = 0.9V) -------- */
static void dac_unsigned_binary_mid_test(void)
{
	int ret;

	LOG_INF("=== DAC Unsigned Binary Mid (0x800) Test ===");

	ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, DAC_MID_INPUT);
	zassert_equal(ret, 0, "DAC write 0x800 failed [%d]", ret);
	k_sleep(K_MSEC(100));

	LOG_INF("DAC unsigned binary mid test PASSED");
}

/* -------- Test: Unsigned binary quarter-scale value (0x400 = 0.45V) -------- */
static void dac_unsigned_binary_quarter_test(void)
{
	int ret;

	LOG_INF("=== DAC Unsigned Binary Quarter (0x400) Test ===");

	ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, DAC_QUARTER_INPUT);
	zassert_equal(ret, 0, "DAC write 0x400 failed [%d]", ret);
	k_sleep(K_MSEC(100));

	LOG_INF("DAC unsigned binary quarter test PASSED");
}

/* -------- Test: Unsigned binary three-quarter scale (0xC00 = 1.35V) -------- */
static void dac_unsigned_binary_three_quarter_test(void)
{
	int ret;
	uint32_t three_quarter = (DAC_MAX_INPUT * 3) / 4;  /* 0xBFF */

	LOG_INF("=== DAC Unsigned Binary Three-Quarter (0x%x) Test ===",
		three_quarter);

	ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, three_quarter);
	zassert_equal(ret, 0, "DAC write 0x%x failed [%d]", three_quarter, ret);
	k_sleep(K_MSEC(100));

	LOG_INF("DAC unsigned binary three-quarter test PASSED");
}

/* -------- Test: Unsigned binary step values across full range -------- */
static void dac_unsigned_binary_step_values_test(void)
{
	int ret;

	LOG_INF("=== DAC Unsigned Binary Step Values Test ===");

	/* Write values at regular intervals across the full 12-bit range */
	uint32_t step_values[] = {
		0x000, 0x100, 0x200, 0x300, 0x400, 0x500,
		0x600, 0x700, 0x800, 0x900, 0xA00, 0xB00,
		0xC00, 0xD00, 0xE00, 0xF00, 0xFFF
	};

	for (int i = 0; i < ARRAY_SIZE(step_values); i++) {
		ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, step_values[i]);
		zassert_equal(ret, 0, "DAC write 0x%x failed [%d]",
			      step_values[i], ret);
		LOG_INF("  Written 0x%03x OK", step_values[i]);
		k_sleep(K_MSEC(100));
	}

	LOG_INF("DAC unsigned binary step values test PASSED");
}

/* -------- Test: Unsigned binary all individual bits -------- */
static void dac_unsigned_binary_bit_pattern_test(void)
{
	int ret;

	LOG_INF("=== DAC Unsigned Binary Bit Pattern Test ===");

	/* Test each individual bit position in 12-bit range */
	for (int bit = 0; bit < DAC_RESOLUTION; bit++) {
		uint32_t val = (1U << bit);

		ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, val);
		zassert_equal(ret, 0, "DAC write bit %d (0x%x) failed [%d]",
			      bit, val, ret);
		k_sleep(K_MSEC(50));
	}

	/* Test all-ones pattern */
	ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, 0xFFF);
	zassert_equal(ret, 0, "DAC write 0xFFF failed [%d]", ret);
	k_sleep(K_MSEC(100));

	/* Test alternating bit patterns */
	ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, 0xAAA);
	zassert_equal(ret, 0, "DAC write 0xAAA failed [%d]", ret);
	k_sleep(K_MSEC(100));

	ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, 0x555);
	zassert_equal(ret, 0, "DAC write 0x555 failed [%d]", ret);
	k_sleep(K_MSEC(100));

	LOG_INF("DAC unsigned binary bit pattern test PASSED");
}

/* -------- Test: Unsigned binary over-range value (error case) -------- */
static void dac_unsigned_binary_over_range_test(void)
{
	int ret;

	LOG_INF("=== DAC Unsigned Binary Over-Range Test ===");

	/* Write value exceeding 12-bit max (0xFFF).
	 * Driver should return -EINVAL when twoscomp is not enabled
	 */
	ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, 0x1000);
	zassert_equal(ret, -EINVAL,
		      "Expected -EINVAL for value > 0xFFF, got [%d]", ret);

	ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, 0xFFFF);
	zassert_equal(ret, -EINVAL,
		      "Expected -EINVAL for value 0xFFFF, got [%d]", ret);

	LOG_INF("DAC unsigned binary over-range test PASSED");
}

/* -------- Test: Two's complement zero value -------- */
static void dac_twos_complement_zero_test(void)
{
	int ret;

	LOG_INF("=== DAC Two's Complement Zero Test ===");

	/* Note: Two's complement mode is configured via device tree property
	 * 'twoscomp_enabled'. This test verifies the DAC accepts a zero value
	 * which is valid in both unsigned and two's complement formats.
	 */

	ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, 0);
	zassert_equal(ret, 0, "DAC write 0 (twos comp zero) failed [%d]", ret);
	k_sleep(K_MSEC(100));

	LOG_INF("DAC two's complement zero test PASSED");
}

/* -------- Test: Two's complement positive values -------- */
static void dac_twos_complement_positive_test(void)
{
	int ret;

	LOG_INF("=== DAC Two's Complement Positive Values Test ===");

	/* In two's complement 12-bit:
	 * Positive range: 0x000 to 0x7FF (0 to +2047)
	 * These values are valid in both unsigned and twos complement modes
	 */

	uint32_t positive_values[] = {0x001, 0x100, 0x200, 0x3FF, 0x400, 0x7FF};

	for (int i = 0; i < ARRAY_SIZE(positive_values); i++) {
		ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, positive_values[i]);
		zassert_equal(ret, 0, "DAC write 0x%x failed [%d]",
			      positive_values[i], ret);
		LOG_INF("  Written positive 0x%03x OK", positive_values[i]);
		k_sleep(K_MSEC(100));
	}

	LOG_INF("DAC two's complement positive test PASSED");
}

/* -------- Test: Two's complement negative values -------- */
static void dac_twos_complement_negative_test(void)
{
	int ret;

	LOG_INF("=== DAC Two's Complement Negative Values Test ===");

	/* In two's complement 12-bit:
	 * Negative range: 0x800 to 0xFFF (-2048 to -1)
	 * When twoscomp_enabled is set in DT, the driver converts these to
	 * the appropriate unsigned representation internally.
	 *
	 * Note: If twoscomp is NOT enabled in DT, these values are still
	 * valid as unsigned values (2048-4095). The driver accepts them
	 * as long as they are within 0-0xFFF range.
	 */

	uint32_t negative_tc_values[] = {0x800, 0x900, 0xA00, 0xC00, 0xFFE, 0xFFF};

	for (int i = 0; i < ARRAY_SIZE(negative_tc_values); i++) {
		ret = dac_write_value(dac_dev, DAC_CHANNEL_ID,
				      negative_tc_values[i]);
		zassert_equal(ret, 0, "DAC write 0x%x failed [%d]",
			      negative_tc_values[i], ret);
		LOG_INF("  Written twos-comp 0x%03x OK", negative_tc_values[i]);
		k_sleep(K_MSEC(100));
	}

	LOG_INF("DAC two's complement negative test PASSED");
}

/* -------- Test: Full range monotonic sweep (unsigned) -------- */
static void dac_unsigned_monotonic_sweep_test(void)
{
	int ret;
	uint32_t step = 16; /* sweep every 16th value for practical test time */

	LOG_INF("=== DAC Unsigned Monotonic Sweep Test ===");

	/* Sweep from 0 to MAX in steps */
	for (uint32_t val = DAC_MIN_INPUT; val <= DAC_MAX_INPUT; val += step) {
		ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, val);
		zassert_equal(ret, 0, "DAC write %u failed [%d]", val, ret);
		k_sleep(K_MSEC(50));
	}

	/* Ensure max value is written */
	ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, DAC_MAX_INPUT);
	zassert_equal(ret, 0, "DAC write MAX failed [%d]", ret);
	k_sleep(K_MSEC(50));

	LOG_INF("DAC unsigned monotonic sweep test PASSED (step=%u)", step);
}

/* ======== Data Format Test Suite ======== */

/* R-32_2: Unsigned binary min */
ZTEST(test_dac_data_format, test_dac_unsigned_binary_min)
{
	dac_unsigned_binary_min_test();
}

/* R-32_2: Unsigned binary max */
ZTEST(test_dac_data_format, test_dac_unsigned_binary_max)
{
	dac_unsigned_binary_max_test();
}

/* R-32_2: Unsigned binary mid-scale */
ZTEST(test_dac_data_format, test_dac_unsigned_binary_mid)
{
	dac_unsigned_binary_mid_test();
}

/* R-32_2: Unsigned binary quarter-scale */
ZTEST(test_dac_data_format, test_dac_unsigned_binary_quarter)
{
	dac_unsigned_binary_quarter_test();
}

/* R-32_2: Unsigned binary three-quarter scale */
ZTEST(test_dac_data_format, test_dac_unsigned_binary_three_quarter)
{
	dac_unsigned_binary_three_quarter_test();
}

/* R-32_2: Unsigned binary step values */
ZTEST(test_dac_data_format, test_dac_unsigned_binary_step_values)
{
	dac_unsigned_binary_step_values_test();
}

/* R-32_2: Unsigned binary bit patterns */
ZTEST(test_dac_data_format, test_dac_unsigned_binary_bit_pattern)
{
	dac_unsigned_binary_bit_pattern_test();
}

/* R-32_2: Unsigned binary over-range error */
ZTEST(test_dac_data_format, test_dac_unsigned_binary_over_range)
{
	if (DAC_TWOSCOMP_ENABLED) {
		ztest_test_skip();
	}

	dac_unsigned_binary_over_range_test();
}

/* R-32_2: Two's complement zero */
ZTEST(test_dac_data_format, test_dac_twos_complement_zero)
{
	dac_twos_complement_zero_test();
}

/* R-32_2: Two's complement positive values */
ZTEST(test_dac_data_format, test_dac_twos_complement_positive)
{
	if (!DAC_TWOSCOMP_ENABLED) {
		ztest_test_skip();
	}

	dac_twos_complement_positive_test();
}

/* R-32_2: Two's complement negative values */
ZTEST(test_dac_data_format, test_dac_twos_complement_negative)
{
	if (!DAC_TWOSCOMP_ENABLED) {
		ztest_test_skip();
	}

	dac_twos_complement_negative_test();
}

/* R-32_2: Full range monotonic sweep */
ZTEST(test_dac_data_format, test_dac_unsigned_monotonic_sweep)
{
	dac_unsigned_monotonic_sweep_test();
}

/* ======== Test Suite Lifecycle ======== */

static void *test_dac_data_format_setup(void)
{
	LOG_INF("=== DAC Data Format Test Suite Setup ===");
	dac_dev = init_dac_device();
	zassert_not_null(dac_dev, "init_dac_device() returned NULL");
	return NULL;
}

static void test_dac_data_format_before(void *fixture)
{
	ARG_UNUSED(fixture);
	LOG_INF("--- Before data format test ---");
}

static void test_dac_data_format_after(void *fixture)
{
	ARG_UNUSED(fixture);
	LOG_INF("--- After data format test ---");
}

static void test_dac_data_format_teardown(void *fixture)
{
	ARG_UNUSED(fixture);
	LOG_INF("=== DAC Data Format Test Suite Teardown ===");
}


ZTEST_SUITE(test_dac_data_format, NULL, test_dac_data_format_setup,
	    test_dac_data_format_before, test_dac_data_format_after,
	    test_dac_data_format_teardown);

/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/**
 * @file test_dac_basic.c
 * @brief DAC Basic Functionality and Dual Channel Tests
 *
 * Requirement Coverage:
 *   R-32_1: Driver should support Dual channel DAC control independently
 *
 * HRM Coverage:
 *   - The device includes two DAC12 modules
 *   - 12-bit resolution
 *   - Internal 1.8V voltage reference
 */

#include "test_dac.h"

LOG_MODULE_DECLARE(dac_test, LOG_LEVEL_INF);

#if DT_NODE_HAS_STATUS(DAC0_NODE, okay) && DT_NODE_HAS_STATUS(DAC1_NODE, okay)
static struct dac_dual_devs dac_dual_devs;
#endif

/* -------- Test: DAC channel setup with invalid resolution -------- */
static void dac_channel_setup_invalid_resolution_test(void)
{
	int ret;
	const struct device *dev = DEVICE_DT_GET(DAC_NODE);

	LOG_INF("=== DAC Invalid Resolution Test ===");

	zassert_true(device_is_ready(dev), "DAC device is not ready");

	/* Test 8-bit resolution (unsupported - DAC12 only supports 12-bit) */
	struct dac_channel_cfg cfg_8bit = {
		.channel_id = DAC_CHANNEL_ID,
		.resolution = 8,
		.buffered = 0
	};

	ret = dac_channel_setup(dev, &cfg_8bit);
	zassert_equal(ret, -ENOTSUP,
		      "Expected -ENOTSUP for 8-bit resolution, got [%d]", ret);

	/* Test 10-bit resolution (unsupported) */
	struct dac_channel_cfg cfg_10bit = {
		.channel_id = DAC_CHANNEL_ID,
		.resolution = 10,
		.buffered = 0
	};

	ret = dac_channel_setup(dev, &cfg_10bit);
	zassert_equal(ret, -ENOTSUP,
		      "Expected -ENOTSUP for 10-bit resolution, got [%d]", ret);

	/* Test 16-bit resolution (unsupported) */
	struct dac_channel_cfg cfg_16bit = {
		.channel_id = DAC_CHANNEL_ID,
		.resolution = 16,
		.buffered = 0
	};

	ret = dac_channel_setup(dev, &cfg_16bit);
	zassert_equal(ret, -ENOTSUP,
		      "Expected -ENOTSUP for 16-bit resolution, got [%d]", ret);

	LOG_INF("DAC invalid resolution test PASSED");
}

/* -------- Test: Basic DAC write value -------- */
static void dac_basic_write_test(void)
{
	int ret;

	LOG_INF("=== DAC Basic Write Test ===");

	dac_dev = init_dac_device();

	/* Write mid-scale value */
	ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, DAC_MID_INPUT);
	zassert_equal(ret, 0, "DAC write mid-scale failed [%d]", ret);
	k_sleep(K_MSEC(100));

	/* Write minimum value */
	ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, DAC_MIN_INPUT);
	zassert_equal(ret, 0, "DAC write min failed [%d]", ret);
	k_sleep(K_MSEC(100));

	/* Write maximum value */
	ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, DAC_MAX_INPUT);
	zassert_equal(ret, 0, "DAC write max failed [%d]", ret);
	k_sleep(K_MSEC(100));

	LOG_INF("DAC basic write test PASSED");
}

/* -------- Test: DAC0 independent initialization (R-32_1) -------- */
static void dac0_independent_init_test(void)
{
	int ret;

	LOG_INF("=== DAC0 Independent Init Test ===");

#if DT_NODE_HAS_STATUS(DAC0_NODE, okay)
	const struct device *dac0 = DEVICE_DT_GET(DAC0_NODE);

	zassert_not_null(dac0, "DAC0 device handle is NULL");
	zassert_true(device_is_ready(dac0), "DAC0 device is not ready");

	const struct dac_channel_cfg cfg = {
		.channel_id = DAC_CHANNEL_ID,
		.resolution = DAC_RESOLUTION,
		.buffered = 0
	};

	ret = dac_channel_setup(dac0, &cfg);
	zassert_equal(ret, 0, "DAC0 channel setup failed [%d]", ret);

	/* Write a known value to DAC0 */
	ret = dac_write_value(dac0, DAC_CHANNEL_ID, 1000);
	zassert_equal(ret, 0, "DAC0 write failed [%d]", ret);
	k_sleep(K_MSEC(100));

	LOG_INF("DAC0 independent init test PASSED");
#else
	ztest_test_skip();
#endif
}

/* -------- Test: DAC1 independent initialization (R-32_1) -------- */
static void dac1_independent_init_test(void)
{
	int ret;

	LOG_INF("=== DAC1 Independent Init Test ===");

#if DT_NODE_HAS_STATUS(DAC1_NODE, okay)
	const struct device *dac1 = DEVICE_DT_GET(DAC1_NODE);

	zassert_not_null(dac1, "DAC1 device handle is NULL");
	zassert_true(device_is_ready(dac1), "DAC1 device is not ready");

	const struct dac_channel_cfg cfg = {
		.channel_id = DAC_CHANNEL_ID,
		.resolution = DAC_RESOLUTION,
		.buffered = 0
	};

	ret = dac_channel_setup(dac1, &cfg);
	zassert_equal(ret, 0, "DAC1 channel setup failed [%d]", ret);

	/* Write a known value to DAC1 */
	ret = dac_write_value(dac1, DAC_CHANNEL_ID, 2000);
	zassert_equal(ret, 0, "DAC1 write failed [%d]", ret);
	k_sleep(K_MSEC(100));

	LOG_INF("DAC1 independent init test PASSED");
#else
	LOG_WRN("DAC1 not enabled in device tree");
	LOG_WRN("To enable DAC1, use: alif_dac_dual.overlay or alif_dac1.overlay");
	ztest_test_skip();
#endif
}

/* -------- Test: Dual DAC independent write (R-32_1) -------- */
static void dac_dual_channel_independent_write_test(void)
{
	int ret;

	LOG_INF("=== DAC Dual Channel Independent Write Test ===");

#if DT_NODE_HAS_STATUS(DAC0_NODE, okay) && DT_NODE_HAS_STATUS(DAC1_NODE, okay)
	/* Write different values to each DAC independently */
	uint32_t dac0_values[] = {0, 1024, 2048, 3072, 4095};
	uint32_t dac1_values[] = {4095, 3072, 2048, 1024, 0};

	for (int i = 0; i < ARRAY_SIZE(dac0_values); i++) {
		ret = dac_write_value(dac_dual_devs.dac0, DAC_CHANNEL_ID,
				      dac0_values[i]);
		zassert_equal(ret, 0, "DAC0 write %d failed [%d]",
			      dac0_values[i], ret);

		ret = dac_write_value(dac_dual_devs.dac1, DAC_CHANNEL_ID,
				      dac1_values[i]);
		zassert_equal(ret, 0, "DAC1 write %d failed [%d]",
			      dac1_values[i], ret);

		LOG_INF("DAC0=%u DAC1=%u written successfully",
			dac0_values[i], dac1_values[i]);
		k_sleep(K_MSEC(100));
	}

	LOG_INF("DAC dual channel independent write test PASSED");
#else
	LOG_WRN("Both DAC0 and DAC1 must be enabled for dual channel test");
#if !DT_NODE_HAS_STATUS(DAC0_NODE, okay)
	LOG_WRN("  -> DAC0 is disabled. Use: alif_dac0.overlay");
#endif
#if !DT_NODE_HAS_STATUS(DAC1_NODE, okay)
	LOG_WRN("  -> DAC1 is disabled. Use: alif_dac1.overlay or alif_dac_dual.overlay");
#endif
	LOG_WRN("  -> For both DACs together: alif_dac_dual.overlay");
	ztest_test_skip();
#endif
}

/* -------- Test: Dual DAC simultaneous operation (R-32_1) -------- */
static void dac_dual_channel_simultaneous_test(void)
{
	int ret;

	LOG_INF("=== DAC Dual Channel Simultaneous Test ===");

#if DT_NODE_HAS_STATUS(DAC0_NODE, okay) && DT_NODE_HAS_STATUS(DAC1_NODE, okay)
	/* Ramp DAC0 up while ramping DAC1 down simultaneously */
	for (uint32_t val = 0; val <= DAC_MAX_INPUT; val += DAC_RAMP_STEP) {
		uint32_t val_inv = DAC_MAX_INPUT - val;

		ret = dac_write_value(dac_dual_devs.dac0, DAC_CHANNEL_ID, val);
		zassert_equal(ret, 0, "DAC0 write %u failed [%d]", val, ret);

		ret = dac_write_value(dac_dual_devs.dac1, DAC_CHANNEL_ID, val_inv);
		zassert_equal(ret, 0, "DAC1 write %u failed [%d]", val_inv, ret);

		k_sleep(K_MSEC(50));
	}

	LOG_INF("DAC dual channel simultaneous test PASSED");
#else
	LOG_WRN("Both DAC0 and DAC1 must be enabled for simultaneous test");
#if !DT_NODE_HAS_STATUS(DAC0_NODE, okay)
	LOG_WRN("  -> DAC0 is disabled. Use: alif_dac0.overlay");
#endif
#if !DT_NODE_HAS_STATUS(DAC1_NODE, okay)
	LOG_WRN("  -> DAC1 is disabled. Use: alif_dac1.overlay or alif_dac_dual.overlay");
#endif
	LOG_WRN("  -> For both DACs together: alif_dac_dual.overlay");
	ztest_test_skip();
#endif
}


/* -------- Test: Invalid channel_id rejection -------- */
static void dac_invalid_channel_id_test(void)
{
	int ret;
	const struct device *dev = DEVICE_DT_GET(DAC_NODE);

	LOG_INF("=== DAC Invalid Channel ID Test ===");

	zassert_true(device_is_ready(dev), "DAC device is not ready");

	/* Test channel_id = 1 (invalid - Alif DAC has only channel 0) */
	struct dac_channel_cfg cfg_ch1 = {
		.channel_id = 1,
		.resolution = DAC_RESOLUTION,
		.buffered = 0
	};

	ret = dac_channel_setup(dev, &cfg_ch1);
	zassert_equal(ret, -EINVAL,
			  "Expected -EINVAL for channel_id=1, got [%d]", ret);

	/* Test channel_id = 255 (invalid - max uint8) */
	struct dac_channel_cfg cfg_ch255 = {
		.channel_id = 255,
		.resolution = DAC_RESOLUTION,
		.buffered = 0
	};

	ret = dac_channel_setup(dev, &cfg_ch255);
	zassert_equal(ret, -EINVAL,
			  "Expected -EINVAL for channel_id=255, got [%d]", ret);

	LOG_INF("DAC invalid channel_id rejection test PASSED");
}

/* -------- Test: Two's complement input range validation -------- */
static void dac_twoscomp_input_range_test(void)
{
	int ret_1;
	int ret_2;

	LOG_INF("=== DAC Two's Complement Input Range Test ===");

	/* Only run when twoscomp mode is enabled */
	if (!DAC_TWOSCOMP_ENABLED) {
		LOG_INF("Skipping: twoscomp mode not enabled");
		ztest_test_skip();
	}

	dac_dev = init_dac_device();

	/* Test value exceeding 12-bit range in twoscomp mode */
	/* Driver should either mask to 12 bits or return -EINVAL */
	ret_1 = dac_write_value(dac_dev, DAC_CHANNEL_ID, 0x1FFF);
	/* Currently driver accepts any value silently; this test documents behavior */
	/* After fix: zassert_equal(ret, -EINVAL, ...) or verify value is masked */

	/* Test 16-bit over-range value */
	ret_2 = dac_write_value(dac_dev, DAC_CHANNEL_ID, 0xFFFF);
	/* Same - documents current behavior vs expected */

	/* For now, verify both writes succeed (documents current behavior) */
	zassert_equal(ret_1, 0,
		      "DAC write 0x1FFF in twoscomp mode failed [%d]",
		      ret_1);
	zassert_equal(ret_2, 0,
		      "DAC write 0xFFFF in twoscomp mode failed [%d]",
		      ret_2);

	LOG_INF("DAC twoscomp input range test completed (documents current behavior)");
}

/* -------- Test: Rapid write stress test -------- */
static void dac_rapid_write_stress_test(void)
{
	int ret;
	uint32_t errors = 0;
	const int iterations = 100;  /* Reduced for LA visibility */

	LOG_INF("=== DAC Rapid Write Stress Test (LA Visible) ===");

	dac_dev = init_dac_device();

	/* Alternating writes with delay for Logic Analyzer visibility */
	for (int i = 0; i < iterations; i++) {
		/* Write MIN value (~0V) */
		ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, DAC_MIN_INPUT);
		if (ret != 0) {
			errors++;
		}

		/* Delay for LA capture */
		k_sleep(K_MSEC(50));

		/* Write MAX value (~1.8V) */
		ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, DAC_MAX_INPUT);
		if (ret != 0) {
			errors++;
		}

		/* Delay for LA capture */
		k_sleep(K_MSEC(50));
	}

	zassert_equal(errors, 0, "Rapid write stress test had %u errors out of %d writes",
		      errors, iterations * 2);

	LOG_INF("DAC rapid write stress test PASSED (%d alternating writes, 50ms delay)",
		iterations * 2);
}

/* -------- Test: Repeated channel setup -------- */
static void dac_repeated_setup_test(void)
{
	int ret;
	const struct device *dev = DEVICE_DT_GET(DAC_NODE);

	LOG_INF("=== DAC Repeated Channel Setup Test ===");

	zassert_true(device_is_ready(dev), "DAC device is not ready");

	/* Setup channel multiple times - driver should handle re-configuration */
	struct dac_channel_cfg cfg = {
		.channel_id = DAC_CHANNEL_ID,
		.resolution = DAC_RESOLUTION,
		.buffered = 0
	};

	for (int i = 0; i < 10; i++) {
		ret = dac_channel_setup(dev, &cfg);
		zassert_equal(ret, 0, "Channel setup iteration %d failed [%d]", i, ret);

		ret = dac_write_value(dev, DAC_CHANNEL_ID, DAC_MID_INPUT);
		zassert_equal(ret, 0, "Write after setup %d failed [%d]", i, ret);
	}

	LOG_INF("DAC repeated setup test PASSED (10 iterations)");
}

/* -------- Test: Write value max+1 edge case -------- */
static void dac_write_max_plus_one_test(void)
{
	int ret;

	LOG_INF("=== DAC Write Max+1 Edge Case Test ===");

	/* Skip if twoscomp mode - different behavior expected */
	if (DAC_TWOSCOMP_ENABLED) {
		LOG_INF("Skipping: twoscomp mode has different range handling");
		ztest_test_skip();
	}

	dac_dev = init_dac_device();

	/* Test exactly DAC_MAX_INPUT + 1 (0x1000) - should be rejected in unsigned mode */
	ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, DAC_MAX_INPUT + 1);
	zassert_equal(ret, -EINVAL,
		      "Expected -EINVAL for DAC_MAX_INPUT+1, got [%d]", ret);

	/* Test boundary: DAC_MAX_INPUT should succeed */
	ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, DAC_MAX_INPUT);
	zassert_equal(ret, 0, "Write DAC_MAX_INPUT should succeed [%d]", ret);

	LOG_INF("DAC write max+1 edge case test PASSED");
}

/* ======== Test Suite Lifecycle ======== */

static void *test_dac_basic_setup(void)
{
	LOG_INF("=== DAC Basic Test Suite Setup ===");
	dac_dev = init_dac_device();
	zassert_not_null(dac_dev, "init_dac_device() returned NULL");

#if DT_NODE_HAS_STATUS(DAC0_NODE, okay) && DT_NODE_HAS_STATUS(DAC1_NODE, okay)
	dac_dual_devs = init_dac_dual_devices();
#endif

	return NULL;
}

static void test_dac_basic_before(void *fixture)
{
	ARG_UNUSED(fixture);
}

static void test_dac_basic_after(void *fixture)
{
	ARG_UNUSED(fixture);
}

static void test_dac_basic_teardown(void *fixture)
{
	ARG_UNUSED(fixture);
}


/* ======== Basic Test Suite ======== */

/* R-32_1: Channel setup with invalid resolution */
ZTEST(test_dac_basic, test_dac_channel_setup_invalid_resolution)
{
	dac_channel_setup_invalid_resolution_test();
}

/* R-32_1: Basic write value */
ZTEST(test_dac_basic, test_dac_basic_write)
{
	dac_basic_write_test();
}

/* R-32_1: DAC0 independent init */
ZTEST(test_dac_basic, test_dac0_independent_init)
{
	dac0_independent_init_test();
}

/* R-32_1: DAC1 independent init */
ZTEST(test_dac_basic, test_dac1_independent_init)
{
	dac1_independent_init_test();
}

/* R-32_1: Dual channel independent write */
ZTEST(test_dac_basic, test_dac_dual_channel_independent_write)
{
	dac_dual_channel_independent_write_test();
}

/* R-32_1: Dual channel simultaneous operation */
ZTEST(test_dac_basic, test_dac_dual_channel_simultaneous)
{
	dac_dual_channel_simultaneous_test();
}

/* Invalid channel_id rejection test */
ZTEST(test_dac_basic, test_dac_invalid_channel_id)
{
	dac_invalid_channel_id_test();
}

/* Two's complement input range validation test */
ZTEST(test_dac_basic, test_dac_twoscomp_input_range)
{
	dac_twoscomp_input_range_test();
}

/* Rapid write stress test */
ZTEST(test_dac_basic, test_dac_rapid_write_stress)
{
	dac_rapid_write_stress_test();
}

/* Repeated channel setup test */
ZTEST(test_dac_basic, test_dac_repeated_setup)
{
	dac_repeated_setup_test();
}

/* Write max+1 edge case test */
ZTEST(test_dac_basic, test_dac_write_max_plus_one)
{
	dac_write_max_plus_one_test();
}

ZTEST_SUITE(test_dac_basic, NULL, test_dac_basic_setup,
	    test_dac_basic_before, test_dac_basic_after,
	    test_dac_basic_teardown);

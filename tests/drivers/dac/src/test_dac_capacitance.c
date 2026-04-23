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

/* -------- Test: DAC init with capacitance config -------- */
static void dac_init_with_capacitance_test(void)
{
	int ret;

	LOG_INF("=== DAC Init With Capacitance Config Test ===");

	/* DAC initializes with capacitance from device tree.
	 * Verify that initialization succeeds with the configured
	 * capacitance compensation setting.
	 */
	const struct device *dev = DEVICE_DT_GET(DAC_NODE);

	zassert_not_null(dev, "DAC device handle is NULL");
	zassert_true(device_is_ready(dev),
		     "DAC device not ready (capacitance config may be invalid)");

	const struct dac_channel_cfg cfg = {
		.channel_id = DAC_CHANNEL_ID,
		.resolution = DAC_RESOLUTION,
		.buffered = 0
	};

	ret = dac_channel_setup(dev, &cfg);
	zassert_equal(ret, 0, "Channel setup failed with capacitance config [%d]",
		      ret);

	LOG_INF("DAC init with capacitance config test PASSED");
}

/* -------- Test: Write values with capacitance compensation -------- */
static void dac_write_with_capacitance_test(void)
{
	int ret;
	uint32_t errors = 0;

	LOG_INF("=== DAC Write With Capacitance Compensation Test ===");

	dac_dev = init_dac_device();

	/* Write various values across the full range with configured
	 * capacitance compensation active
	 */
	uint32_t test_values[] = {
		DAC_MIN_INPUT,     /* 0V */
		DAC_QUARTER_INPUT, /* ~0.45V */
		DAC_MID_INPUT,     /* ~0.9V */
		(DAC_MAX_INPUT * 3) / 4, /* ~1.35V */
		DAC_MAX_INPUT      /* ~1.8V */
	};

	for (int i = 0; i < ARRAY_SIZE(test_values); i++) {
		ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, test_values[i]);
		if (ret != 0) {
			errors++;
			LOG_ERR("Write 0x%x with capacitance failed [%d]",
				test_values[i], ret);
		} else {
			LOG_INF("  Written 0x%03x with capacitance OK",
				test_values[i]);
		}
		/* Allow settling time with load capacitance - LA visibility delay */
		k_sleep(K_MSEC(100));
	}

	zassert_equal(errors, 0, "Capacitance write had %u errors", errors);

	LOG_INF("DAC write with capacitance compensation test PASSED");
}

/* -------- Test: Output stability with capacitance compensation -------- */
static void dac_capacitance_stability_test(void)
{
	int ret;
	uint32_t errors = 0;

	LOG_INF("=== DAC Capacitance Stability Test ===");

	dac_dev = init_dac_device();

	/* Write mid-scale value repeatedly to verify stable output
	 * with capacitance compensation active
	 */
	for (int i = 0; i < 20; i++) {  /* Reduced for LA visibility */
		ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, DAC_MID_INPUT);
		if (ret != 0) {
			errors++;
		}
		k_sleep(K_MSEC(100));
	}

	zassert_equal(errors, 0, "Stability test had %u errors out of %d",
		      errors, 20);

	LOG_INF("DAC capacitance stability test PASSED (20 writes with 100ms delay)");
}

/* -------- Test: Step response with capacitance compensation -------- */
static void dac_capacitance_step_response_test(void)
{
	int ret;

	LOG_INF("=== DAC Capacitance Step Response Test ===");

	dac_dev = init_dac_device();

	/* Large step from min to max - tests settling with capacitance comp */
	ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, DAC_MIN_INPUT);
	zassert_equal(ret, 0, "Write min failed [%d]", ret);
	k_sleep(K_MSEC(100));

	ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, DAC_MAX_INPUT);
	zassert_equal(ret, 0, "Write max (step up) failed [%d]", ret);
	k_sleep(K_MSEC(100));

	/* Large step from max to min */
	ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, DAC_MIN_INPUT);
	zassert_equal(ret, 0, "Write min (step down) failed [%d]", ret);
	k_sleep(K_MSEC(100));

	/* Medium steps */
	ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, DAC_QUARTER_INPUT);
	zassert_equal(ret, 0, "Write quarter failed [%d]", ret);
	k_sleep(K_MSEC(100));

	ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, DAC_MID_INPUT);
	zassert_equal(ret, 0, "Write mid failed [%d]", ret);
	k_sleep(K_MSEC(100));

	ret = dac_write_value(dac_dev, DAC_CHANNEL_ID,
			      (DAC_MAX_INPUT * 3) / 4);
	zassert_equal(ret, 0, "Write three-quarter failed [%d]", ret);
	k_sleep(K_MSEC(100));

	ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, DAC_MAX_INPUT);
	zassert_equal(ret, 0, "Write max failed [%d]", ret);

	LOG_INF("DAC capacitance step response test PASSED");
}

/* -------- Test: Ramp with capacitance at settling intervals -------- */
static void dac_capacitance_ramp_with_settling_test(void)
{
	int ret;
	uint32_t errors = 0;
	uint32_t step = 256; /* coarse steps to allow settling */

	LOG_INF("=== DAC Capacitance Ramp With Settling Test ===");

	dac_dev = init_dac_device();

	/* Ramp with settling delay between steps */
	for (uint32_t val = DAC_MIN_INPUT; val <= DAC_MAX_INPUT; val += step) {
		ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, val);
		if (ret != 0) {
			errors++;
			LOG_ERR("Ramp write %u failed [%d]", val, ret);
		}
		/* Allow output to settle with load capacitance - LA visibility */
		k_sleep(K_MSEC(50));
	}

	/* Final max value */
	ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, DAC_MAX_INPUT);
	zassert_equal(ret, 0, "Final max write failed [%d]", ret);

	zassert_equal(errors, 0, "Ramp with settling had %u errors", errors);

	LOG_INF("DAC capacitance ramp with settling test PASSED");
}

/* -------- Test: DAC output current configuration -------- */
static void dac_output_current_config_test(void)
{
	int ret;

	LOG_INF("=== DAC Output Current Configuration Test ===");

	/* Output current is configured via DT property 'output_current'.
	 * This test verifies the DAC operates correctly with the
	 * configured output current (0-1500 uA).
	 *
	 * HRM: Programmable output current up to 1.5 mA
	 * Supported values: 0, 100, 200, 300, 400, 500, 600, 700, 800,
	 *                   900, 1000, 1100, 1200, 1300, 1400, 1500 uA
	 */

	/* Log the configured output current from device tree for verification */
#if DT_NODE_HAS_PROP(DAC_NODE, output_current)
	const char *output_current_str = DT_PROP(DAC_NODE, output_current);

	LOG_INF("Configured output_current: %s", output_current_str);
#else
	LOG_WRN("No output_current property in device tree");
#endif

	dac_dev = init_dac_device();

	/* Write values at key points to verify operation with
	 * the configured output current. Even at maximum current (1.5mA),
	 * the DAC should handle min/mid/max digital values correctly.
	 */
	ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, DAC_MIN_INPUT);
	zassert_equal(ret, 0, "Write min with output current config failed [%d]",
		      ret);
	k_sleep(K_MSEC(100));

	ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, DAC_MID_INPUT);
	zassert_equal(ret, 0, "Write mid with output current config failed [%d]",
		      ret);
	k_sleep(K_MSEC(100));

	ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, DAC_MAX_INPUT);
	zassert_equal(ret, 0, "Write max with output current config failed [%d]",
		      ret);
	k_sleep(K_MSEC(100));

	LOG_INF("DAC output current configuration test PASSED");
}

/* -------- Test: LP mode operation -------- */
static void dac_lp_mode_operation_test(void)
{
	int ret;
	uint32_t errors = 0;

	LOG_INF("=== DAC LP Mode Operation Test ===");

	/* LP mode is the default mode. Verify DAC operates correctly
	 * with slow sample rates suitable for LP mode.
	 */
	dac_dev = init_dac_device();

	/* Write at slow rate (suitable for LP mode) */
	for (uint32_t val = 0; val <= DAC_MAX_INPUT; val += 512) {
		ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, val);
		if (ret != 0) {
			errors++;
		}
		/* LP mode: slow sample rate - LA visibility delay */
		k_sleep(K_MSEC(100));
	}

	zassert_equal(errors, 0, "LP mode operation had %u errors", errors);

	LOG_INF("DAC LP mode operation test PASSED");
}

/* ======== Capacitance Test Suite ======== */

/* R-32_4: Init with capacitance config */
ZTEST(test_dac_capacitance, test_dac_init_with_capacitance)
{
	dac_init_with_capacitance_test();
}

/* R-32_4: Write values with capacitance compensation */
ZTEST(test_dac_capacitance, test_dac_write_with_capacitance)
{
	dac_write_with_capacitance_test();
}

/* R-32_4: Output stability with capacitance */
ZTEST(test_dac_capacitance, test_dac_capacitance_stability)
{
	dac_capacitance_stability_test();
}

/* R-32_4: Step response with capacitance */
ZTEST(test_dac_capacitance, test_dac_capacitance_step_response)
{
	dac_capacitance_step_response_test();
}

/* R-32_4: Ramp with settling intervals */
ZTEST(test_dac_capacitance, test_dac_capacitance_ramp_with_settling)
{
	dac_capacitance_ramp_with_settling_test();
}

/* HRM: Programmable output current */
ZTEST(test_dac_capacitance, test_dac_output_current_config)
{
	dac_output_current_config_test();
}

/* HRM: LP mode operation */
ZTEST(test_dac_capacitance, test_dac_lp_mode_operation)
{
	dac_lp_mode_operation_test();
}

/* ======== Test Suite Lifecycle ======== */

static void *test_dac_capacitance_setup(void)
{
	LOG_INF("=== DAC Capacitance Test Suite Setup ===");
	dac_dev = init_dac_device();
	zassert_not_null(dac_dev, "init_dac_device() returned NULL");
	return NULL;
}

static void test_dac_capacitance_before(void *fixture)
{
	ARG_UNUSED(fixture);
	LOG_INF("--- Before capacitance test ---");
}

static void test_dac_capacitance_after(void *fixture)
{
	ARG_UNUSED(fixture);
	LOG_INF("--- After capacitance test ---");
	/* Reset DAC to 0 */
	dac_write_value(dac_dev, DAC_CHANNEL_ID, DAC_MIN_INPUT);
}

static void test_dac_capacitance_teardown(void *fixture)
{
	ARG_UNUSED(fixture);
	LOG_INF("=== DAC Capacitance Test Suite Teardown ===");
}


ZTEST_SUITE(test_dac_capacitance, NULL, test_dac_capacitance_setup,
	    test_dac_capacitance_before, test_dac_capacitance_after,
	    test_dac_capacitance_teardown);

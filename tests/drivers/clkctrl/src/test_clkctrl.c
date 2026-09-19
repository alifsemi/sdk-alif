/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/ztest.h>
#include <zephyr/device.h>
#include <zephyr/drivers/clock_control.h>
#include <zephyr/logging/log.h>

#include <errno.h>

#if defined(CONFIG_SOC_FAMILY_BALLETTO)
#include <zephyr/dt-bindings/clock/alif_balletto_clocks.h>
#elif defined(CONFIG_SOC_FAMILY_ENSEMBLE)
#include <zephyr/dt-bindings/clock/alif_ensemble_clocks.h>
#else
#error "Unsupported Alif SoC family for clkctrl test"
#endif

LOG_MODULE_REGISTER(clock_test, LOG_LEVEL_DBG);

#define CLOCK_CTRL_NODE DT_NODELABEL(clockctrl)

static const struct device *clock_dev;

/*
 * Wrapper functions to pass clock ID by value (cast to uintptr_t) to the
 * clock control driver API, which expects clock_control_subsys_t.
 */
static int clock_on_id(uint32_t clk_id)
{
	return clock_control_on(clock_dev, (clock_control_subsys_t)(uintptr_t)clk_id);
}

static int clock_off_id(uint32_t clk_id)
{
	return clock_control_off(clock_dev, (clock_control_subsys_t)(uintptr_t)clk_id);
}

static enum clock_control_status clock_status_id(uint32_t clk_id)
{
	return clock_control_get_status(clock_dev, (clock_control_subsys_t)(uintptr_t)clk_id);
}

static int clock_get_rate_id(uint32_t clk_id, uint32_t *rate)
{
	return clock_control_get_rate(clock_dev, (clock_control_subsys_t)(uintptr_t)clk_id,
				      rate);
}

static int clock_set_rate_id(uint32_t clk_id, uint32_t rate)
{
	return clock_control_set_rate(clock_dev, (clock_control_subsys_t)(uintptr_t)clk_id,
				     (clock_control_subsys_rate_t)(uintptr_t)rate);
}

static int verify_clock_rate(uint32_t clk_id, const char *name)
{
	uint32_t rate = 0U;
	int ret = clock_get_rate_id(clk_id, &rate);

	if (ret == -ENOTSUP) {
		LOG_INF("Clock get_rate not supported for %s", name);
		return -ENOTSUP;
	}

	zassert_equal(ret, 0, "clock_control_get_rate(%s) failed: %d", name, ret);
	zassert_true(rate > 0U, "clock_control_get_rate(%s) returned zero", name);
	LOG_INF("Clock %s rate: %u Hz", name, rate);
	return 0;
}

/* Clocks selected for broad coverage on Alif platforms. */
static const uint32_t clk_uart0_pclk = ALIF_UART0_SYST_PCLK;
static const uint32_t clk_utimer = ALIF_UTIMER_CLK;
static const uint32_t clk_lptimer0 = ALIF_LPTIMER0_S32K_CLK;
static const uint32_t clk_adc0 = ALIF_ADC0_CLK;
static const uint32_t clk_gpio0_db = ALIF_GPIO0_DB_CLK;

/* Setup: Run before all test cases */
static void *clock_test_setup(void)
{
	clock_dev = DEVICE_DT_GET(CLOCK_CTRL_NODE);
	zassert_true(device_is_ready(clock_dev), "Clock device not ready");
	return NULL;
}

ZTEST_SUITE(clock_control, NULL, clock_test_setup, NULL, NULL, NULL);

/* Basic on/off/status is covered by upstream clock_control_api. */

ZTEST(clock_control, test_clock_get_rate)
{
	if (verify_clock_rate(clk_uart0_pclk, "UART0_SYST_PCLK") == -ENOTSUP) {
		ztest_test_skip();
	}
}

ZTEST(clock_control, test_clock_get_rate_multiple_clocks)
{
	int verified = 0;

	if (verify_clock_rate(clk_utimer, "UTIMER") == 0) {
		verified++;
	}
	if (verify_clock_rate(clk_lptimer0, "LPTIMER0_S32K") == 0) {
		verified++;
	}
	if (verified == 0) {
		ztest_test_skip();
	}
}

ZTEST(clock_control, test_invalid_subsystem_get_rate)
{
	uint32_t rate = 0U;
	uint32_t invalid_clk_id = 0xDEADBEEFU;
	int ret;

	ret = clock_get_rate_id(invalid_clk_id, &rate);
	zassert_not_equal(ret, 0, "clock_control_get_rate() with invalid clk should fail");
}

ZTEST(clock_control, test_clock_set_rate)
{
	uint32_t original_rate = 0U;
	uint32_t new_rate = 0U;
	int ret;

	ret = clock_get_rate_id(clk_gpio0_db, &original_rate);
	if (ret == -ENOTSUP) {
		ztest_test_skip();
	}
	zassert_equal(ret, 0, "get_rate(GPIO0_DB) failed: %d", ret);
	zassert_true(original_rate > 0U, "GPIO0_DB rate is zero");
	LOG_INF("GPIO0_DB original rate: %u Hz", original_rate);

	/* set_rate to the same frequency should be a no-op success */
	ret = clock_set_rate_id(clk_gpio0_db, original_rate);
	zassert_equal(ret, 0, "set_rate(same freq) failed: %d", ret);

	/* verify rate unchanged */
	ret = clock_get_rate_id(clk_gpio0_db, &new_rate);
	zassert_equal(ret, 0, "get_rate after set_rate failed: %d", ret);
	zassert_equal(new_rate, original_rate, "Rate changed after set_rate(same)");
}

ZTEST(clock_control, test_clock_set_rate_unsupported)
{
	int ret;

	/*
	 * UART0_SYST_PCLK has no divisor (div_mask == 0).
	 * set_rate should return -ENOTSUP or 0 (no divisor available).
	 */
	ret = clock_set_rate_id(clk_uart0_pclk, 1U);
	zassert_true(ret == -ENOTSUP || ret == 0,
		     "set_rate on non-divisor clock returned unexpected: %d", ret);
	LOG_INF("set_rate(UART0_SYST_PCLK, 1) returned: %d", ret);
}

ZTEST(clock_control, test_clock_on_already_enabled)
{
	int ret;
	bool enabled_by_test = false;

	/* Enable ADC0 clock; it may already be on from boot */
	ret = clock_on_id(clk_adc0);
	zassert_true(ret == 0 || ret == -EALREADY,
		     "First clock_on(ADC0) unexpected: %d", ret);
	enabled_by_test = (ret == 0);

	/* Second enable returns -EALREADY on Ensemble, 0 on Balletto */
	ret = clock_on_id(clk_adc0);
	zassert_true(ret == -EALREADY || ret == 0,
		     "Double clock_on(ADC0) unexpected: %d", ret);
	LOG_INF("Double clock_on(ADC0) returned: %d", ret);

	/* Cleanup: only turn off if we turned it on */
	if (enabled_by_test) {
		ret = clock_off_id(clk_adc0);
		zassert_equal(ret, 0, "clock_off(ADC0) failed: %d", ret);
	}
}

ZTEST(clock_control, test_clock_get_rate_consistency)
{
	uint32_t rate1 = 0U;
	uint32_t rate2 = 0U;
	int ret;
	bool was_already_on = false;

	/* Use ADC0 so console UART clock is not gated */
	ret = clock_get_rate_id(clk_adc0, &rate1);
	if (ret == -ENOTSUP) {
		ztest_test_skip();
	}
	zassert_equal(ret, 0, "get_rate(ADC0) pre-cycle failed: %d", ret);

	ret = clock_on_id(clk_adc0);
	zassert_true(ret == 0 || ret == -EALREADY,
		     "clock_on(ADC0) failed: %d", ret);
	was_already_on = (ret == -EALREADY);

	ret = clock_off_id(clk_adc0);
	zassert_equal(ret, 0, "clock_off(ADC0) failed: %d", ret);

	ret = clock_get_rate_id(clk_adc0, &rate2);
	zassert_equal(ret, 0, "get_rate(ADC0) post-cycle failed: %d", ret);

	if (was_already_on) {
		ret = clock_on_id(clk_adc0);
		zassert_true(ret == 0 || ret == -EALREADY,
			     "Restore clock_on(ADC0) failed: %d", ret);
	}

	zassert_equal(rate1, rate2,
		      "Rate changed after on/off cycle: %u -> %u", rate1, rate2);
	LOG_INF("ADC0 rate stable at %u Hz across on/off", rate1);
}

ZTEST(clock_control, test_clock_status_unsupported_clock)
{
	enum clock_control_status status;

	/*
	 * ALIF_UTIMER_CLK is a dummy clock (en_mask == 0).
	 * get_status should return CLOCK_CONTROL_STATUS_UNKNOWN.
	 */
	status = clock_status_id(clk_utimer);
	zassert_equal(status, CLOCK_CONTROL_STATUS_UNKNOWN,
		      "Status of dummy clock should be UNKNOWN, got: %d", status);
}

ZTEST(clock_control, test_clock_on_off_unsupported_clock)
{
	int ret;

	/*
	 * ALIF_UTIMER_CLK is a dummy clock (en_mask == 0).
	 * on/off should return 0 (silently succeed with a warning log).
	 */
	ret = clock_on_id(clk_utimer);
	zassert_equal(ret, 0,
		      "clock_on(dummy UTIMER) should return 0, got: %d", ret);

	ret = clock_off_id(clk_utimer);
	zassert_equal(ret, 0,
		      "clock_off(dummy UTIMER) should return 0, got: %d", ret);
}

ZTEST(clock_control, test_clock_get_rate_all_domains)
{
	int verified = 0;

	if (verify_clock_rate(clk_uart0_pclk, "UART0_SYST_PCLK") == 0) {
		verified++;
	}
	if (verify_clock_rate(clk_utimer, "UTIMER") == 0) {
		verified++;
	}
	if (verify_clock_rate(clk_lptimer0, "LPTIMER0_S32K") == 0) {
		verified++;
	}
	if (verify_clock_rate(clk_gpio0_db, "GPIO0_DB") == 0) {
		verified++;
	}
	if (verified == 0) {
		ztest_test_skip();
	}
}

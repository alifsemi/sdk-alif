/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include "test_cmp.h"

LOG_MODULE_REGISTER(ALIF_CMP);

/* Macros for call back */
volatile uint8_t call_back_event;
volatile uint8_t cmp_status;
uint8_t value;

void cmp_callback(const struct device *dev, void *user_data)
{
	call_back_event = 1;

	int out = comparator_get_output(dev);

	if (out >= 0) {
		cmp_status = (uint8_t)out;
		if (user_data != NULL) {
			*(uint8_t *)user_data = (uint8_t)out;
		}
	}
}

void Comp_func(uint32_t loop)
{

#if defined(CONFIG_REF_EXT)
	ztest_test_skip();
	return;
#endif

	static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_NODELABEL(aled0), gpios);
	uint8_t cmp = DT_ENUM_IDX(NODE_LABEL, driver_instance);
	const struct device *const cmp_dev = DEVICE_DT_GET(NODE_LABEL);

	zassert_true(device_is_ready(led.port), "LED device is not ready");
	zassert_true(device_is_ready(cmp_dev), "device is not ready");

	if (cmp) {
		int ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_LOW);

		zassert_equal(ret, 0, "Error %d: failed to configure LED pin\n", ret);

		k_msleep(2000);
	}
	int ret = comparator_set_trigger_callback(cmp_dev, cmp_callback, &value);

	zassert_equal(ret, 0, "set_trigger_callback failed (%d)", ret);
	ret = comparator_set_trigger(cmp_dev, COMPARATOR_TRIGGER_BOTH_EDGES);
	zassert_equal(ret, 0, "set_trigger failed (%d)", ret);
	LOG_INF("start comparing");
	while (loop--) {

		if (cmp) {
			gpio_pin_toggle_dt(&led);
		}

		/* wait for comparator callback */
		int64_t deadline = k_uptime_get() + CMP_CALLBACK_TIMEOUT_MS;

		while (call_back_event == 0U && k_uptime_get() < deadline) {
			k_yield();
		}
		zassert_true(call_back_event != 0U,
			"Timed out waiting for comparator callback");

		call_back_event = 0U;

		cmp_status = comparator_get_output(cmp_dev);
		/* Introducing a delay to stabilize input
		 * voltage for comparator measurement
		 */
		k_msleep(50);

		/* If the user gives a positive input voltage greater than the
		 * negative input voltage, status will be set to 1
		 */
		if (cmp_status == 1) {
			LOG_INF("[+] positive input voltage is greater than "
				"negative input voltage");
		}
		/* If the user gives a negative input voltage greater than the
		 * positive input voltage, status will be set to 0
		 */
		else if (cmp_status == 0) {
			LOG_INF("[-] negative input voltage is greater than the positive input "
				"voltage");
		} else {
			LOG_INF("ERROR: Status detection failed");
		}
	}

	LOG_INF("Comparison Completed");
}

void Comp_func_ref_ext(uint32_t loop)
{
	const struct device *const cmp_dev = DEVICE_DT_GET(NODE_LABEL);

	zassert_true(device_is_ready(cmp_dev), "device is not ready");

	int ret = comparator_set_trigger_callback(cmp_dev, cmp_callback, &value);

	zassert_equal(ret, 0, "set_trigger_callback failed (%d)", ret);
	ret = comparator_set_trigger(cmp_dev, COMPARATOR_TRIGGER_BOTH_EDGES);
	zassert_equal(ret, 0, "set_trigger failed (%d)", ret);
	LOG_INF("start comparing (external reference)");

	while (loop--) {
		int64_t deadline = k_uptime_get() + CMP_CALLBACK_TIMEOUT_MS;

		while (call_back_event == 0U && k_uptime_get() < deadline) {
			k_yield();
		}
		zassert_true(call_back_event != 0U,
			"Timed out waiting for comparator callback");

		call_back_event = 0U;
		cmp_status = comparator_get_output(cmp_dev);
		k_msleep(50);

		if (cmp_status == 1) {
			LOG_INF("[+] positive input voltage is greater than "
				"negative input voltage");
		} else if (cmp_status == 0) {
			LOG_INF("[-] negative input voltage is greater than "
				"the positive input voltage");
		} else {
			LOG_INF("ERROR: Status detection failed");
		}
	}

	LOG_INF("Comparison Completed (external reference)");
}

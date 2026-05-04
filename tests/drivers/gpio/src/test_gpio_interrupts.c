/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/*
 * Alif-unique GPIO interrupt coverage.
 *
 * Edge, level, add/remove, enable/disable and self-remove callback
 * flows are already covered by Zephyr's upstream
 * tests/drivers/gpio/gpio_basic_api conformance test
 * (gpio_port_cb_vari and gpio_port_cb_mgmt suites) and are therefore
 * NOT duplicated here.
 *
 * The only scenario retained in this file is multi-input / single-
 * output: two independent input pins wired to the same output
 * source, each with its own gpio_callback registered on a different
 * controller/pin, exercised together. That wiring topology is not
 * expressible in the upstream single-IN/single-OUT test harness and
 * so remains valuable as Alif-specific coverage.
 */

#include "test_gpio.h"
#include <zephyr/drivers/gpio.h>

/* Shared helpers also used from test_gpio.c */
int check_pin_conf(int ret)
{
	if (ret == 0) {
		return 0;
	}
	if (ret == -ENOTSUP) {
		TC_PRINT("Pin configuration option not supported\n");
		ztest_test_skip();
	}
	return -1;
}

static int check_cb_res(int ret)
{
	return (ret == 0) ? 0 : -1;
}

static int check_int_conf(int ret)
{
	if (ret == 0) {
		return 0;
	}
	if (ret == -ENOTSUP) {
		TC_PRINT("Interrupt configuration not supported\n");
		ztest_test_skip();
	}
	return -1;
}

static int check_pin_toggle(int ret)
{
	return (ret == 0) ? 0 : -1;
}

#define LED0_NODE  DT_ALIAS(led0)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

#if DT_HAS_ALIAS(led2)

static const struct gpio_dt_spec gpio_in1 = GPIO_DT_SPEC_GET(GPIO_NODE, gpios);
static const struct gpio_dt_spec gpio_in2 = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);

static int cb_multi_in1_count;
static int cb_multi_in2_count;

static void callback_multi_in1(const struct device *port, struct gpio_callback *cb,
			       gpio_port_pins_t pins)
{
	ARG_UNUSED(port);
	ARG_UNUSED(cb);

	if (pins & BIT(gpio_in1.pin)) {
		cb_multi_in1_count++;
	}
}

static void callback_multi_in2(const struct device *port, struct gpio_callback *cb,
			       gpio_port_pins_t pins)
{
	ARG_UNUSED(port);
	ARG_UNUSED(cb);

	if (pins & BIT(gpio_in2.pin)) {
		cb_multi_in2_count++;
	}
}

static void drive_led_edges(int count)
{
	for (int i = 0; i < count; i++) {
		zassert_equal(check_pin_toggle(gpio_pin_toggle_dt(&led)), 0,
			      "Failed to toggle LED pin");
		k_msleep(100);
	}
}

/*
 * Alif-unique: two inputs driven from a single output trigger
 * independent callbacks. Upstream gpio_basic_api only supports one
 * IN/OUT pair, so this wiring topology is not otherwise exercised.
 */
ZTEST(gpio_test_interrupts, test_gpio_multi_input_single_output_interrupt)
{
	struct gpio_callback gpio_cb_in1;
	struct gpio_callback gpio_cb_in2;
	int in1_initial;
	int in2_initial;
	bool in1_toggled = false;
	bool in2_toggled = false;
	int ret;

	zassert_true(gpio_is_ready_dt(&gpio_in1), "GPIO input1 dev is not ready");
	zassert_true(gpio_is_ready_dt(&gpio_in2), "GPIO input2 dev is not ready");
	zassert_true(gpio_is_ready_dt(&led), "GPIO output dev is not ready");

	zassert_equal(check_int_conf(gpio_pin_interrupt_configure_dt(&gpio_in1,
								     GPIO_INT_DISABLE)),
		      0, "Disable input1 interrupt failed");
	zassert_equal(check_int_conf(gpio_pin_interrupt_configure_dt(&gpio_in2,
								     GPIO_INT_DISABLE)),
		      0, "Disable input2 interrupt failed");

	zassert_equal(check_pin_conf(gpio_pin_configure_dt(&gpio_in1, GPIO_INPUT)), 0,
		      "Configure input1 pin failed");
	zassert_equal(check_pin_conf(gpio_pin_configure_dt(&gpio_in2, GPIO_INPUT)), 0,
		      "Configure input2 pin failed");
	zassert_equal(check_pin_conf(gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE)), 0,
		      "Configure output pin failed");

	gpio_init_callback(&gpio_cb_in1, callback_multi_in1, BIT(gpio_in1.pin));
	zassert_equal(check_cb_res(gpio_add_callback(gpio_in1.port, &gpio_cb_in1)), 0,
		      "Add callback for input1 failed");

	gpio_init_callback(&gpio_cb_in2, callback_multi_in2, BIT(gpio_in2.pin));
	zassert_equal(check_cb_res(gpio_add_callback(gpio_in2.port, &gpio_cb_in2)), 0,
		      "Add callback for input2 failed");

	ret = gpio_pin_interrupt_configure_dt(&gpio_in1, GPIO_INT_EDGE_BOTH);
	zassert_equal(check_int_conf(ret), 0, "Enable input1 interrupt failed");

	ret = gpio_pin_interrupt_configure_dt(&gpio_in2, GPIO_INT_EDGE_BOTH);
	zassert_equal(check_int_conf(ret), 0, "Enable input2 interrupt failed");

	in1_initial = gpio_pin_get_dt(&gpio_in1);
	in2_initial = gpio_pin_get_dt(&gpio_in2);

	cb_multi_in1_count = 0;
	cb_multi_in2_count = 0;
	for (int i = 0; i < 10; i++) {
		drive_led_edges(1);
		if (gpio_pin_get_dt(&gpio_in1) != in1_initial) {
			in1_toggled = true;
		}
		if (gpio_pin_get_dt(&gpio_in2) != in2_initial) {
			in2_toggled = true;
		}
	}

	if (!in2_toggled) {
		TC_PRINT("Input2 (led2) did not toggle; check wiring led0->led2.\n");
		ztest_test_skip();
	}

	zassert_true(in1_toggled, "Input1 did not toggle; check wiring");
	zassert_true(cb_multi_in1_count > 0, "No interrupt seen on input1");
	zassert_true(cb_multi_in2_count > 0, "No interrupt seen on input2");

	zassert_equal(check_int_conf(gpio_pin_interrupt_configure_dt(&gpio_in1,
								     GPIO_INT_DISABLE)),
		      0, "Disable input1 interrupt failed");
	zassert_equal(check_int_conf(gpio_pin_interrupt_configure_dt(&gpio_in2,
								     GPIO_INT_DISABLE)),
		      0, "Disable input2 interrupt failed");

	zassert_equal(check_cb_res(gpio_remove_callback(gpio_in1.port, &gpio_cb_in1)), 0,
		      "Remove callback for input1 failed");
	zassert_equal(check_cb_res(gpio_remove_callback(gpio_in2.port, &gpio_cb_in2)), 0,
		      "Remove callback for input2 failed");
}

#else /* !DT_HAS_ALIAS(led2) */

ZTEST(gpio_test_interrupts, test_gpio_multi_input_single_output_interrupt)
{
	TC_PRINT("led2 alias not present in devicetree; skipping multi-input test.\n");
	ztest_test_skip();
}

#endif /* DT_HAS_ALIAS(led2) */

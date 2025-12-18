/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include "test_gpio.h"
#include <limits.h>
#include <zephyr/sys/util.h>
#include <zephyr/drivers/gpio.h>

#define SLEEP_TIME_MS   1000

int cb_count, int_level_type = 0;

static const struct gpio_dt_spec test_int = GPIO_DT_SPEC_GET(GPIO_NODE, gpios);

/** gpio_port = P1_7 input */
static const struct gpio_dt_spec gpio_port = GPIO_DT_SPEC_GET(GPIO_NODE, gpios);

/** led = P6_4 output */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

extern int Check_result(int ret);
extern int check_pin_set_conf(int ret);

int check_cb_res(int ret)
{
switch (ret) {
case 0:
TC_PRINT("Callback set\n");
return 0;
case -ENOSYS:
	TC_PRINT("Operation is not implemented by the driver.\n");
	return -1;
default:
	if (ret < 0) {
		TC_PRINT("Other negative errno code on failure\n");
		return -1;
	}
	break;
}
return -1;
}

/**
 * Function to check gpio_pin_interrupt_configure_dt() return status
 */
int check_int_conf(int ret)
{
switch (ret) {
case 0:
TC_PRINT("Configured interrupt\n");
return 0;
default:
	if (ret == -ENOSYS) {
		TC_PRINT("Operation is not implemented by the driver.\n");
		ztest_test_skip();
		return -1;
	} else if (ret == -ENOTSUP) {
		TC_PRINT("The configuration option is not supported\n");
		ztest_test_skip();
		return -1;
	} else if (ret == -EINVAL) {
		TC_PRINT("Invalid argument.\n");
		return -1;
	} else if (ret == -EBUSY) {
		TC_PRINT("Interrupt line required to configure pin interrupt "
				"is already in use.\n");
		return -1;
	} else if (ret == -EIO) {
		TC_PRINT("I/O error when accessing\n");
		return -1;
	} else if (ret == -EWOULDBLOCK) {
		TC_PRINT("Operation would block\n");
		return -1;
	}
	break;
}
return -1;
}

/**
 * Function to check gpio_pin_configure_dt() return status
 */
int check_pin_conf(int ret)
{
switch (ret) {
case 0:
TC_PRINT("Configured the pin\n");
return 0;
default:
	if (ret == -ENOTSUP) {
		TC_PRINT("Simultaneous pin in/out mode is not supported.\n");
		ztest_test_skip();
		return -1;
	} else if (ret == -EINVAL) {
		TC_PRINT("Invalid argument.\n");
		return -1;
	} else if (ret == -EIO) {
		TC_PRINT("Error when accessing\n");
		return -1;
	} else if (ret == -EWOULDBLOCK) {
		TC_PRINT("Operation would block\n");
		return -1;
	}
	break;
}
return -1;
}

/**
 * Function to check gpio_pin_toggle_dt() return status
 */
int check_pin_toggle(int ret)
{
switch (ret) {
case 0:
	TC_PRINT("LED pin toggled\n");
	return 0;
case -EIO:
	TC_PRINT("I/O error when toggling LED pin\n");
	return -1;
case -EWOULDBLOCK:
	TC_PRINT("Operation would block\n");
	return -1;
default:
	TC_PRINT("Unexpected return value: %d\n", ret);
	return -1;
	}
}

static void callback_edge(const struct device *port, struct gpio_callback *cb,
			gpio_port_pins_t pins)
{
int ret, ret_int3;

if (pins & BIT(test_int.pin)) {
	printk("Zephyr GPIO Interrupt triggered\n");
	cb_count++;
	printk("cb_count value %d\n", cb_count);

	if (int_level_type != 0) {
		ret = gpio_pin_interrupt_configure_dt(&gpio_port, GPIO_INT_DISABLE);
		ret_int3 = check_int_conf(ret);
		zassert_equal(ret_int3, 0, "Pin interrupt disabling failed\n");
	}
} else {
	printk("Zephyr GPIO Interrupt triggered on an unexpected pin\n");
}
}

void test_gpio_pin_interrupt(unsigned int int_flags)
{
int ou_ret, in_ret, ret, pin_value = 0;
struct gpio_callback gpio_cb_edge;

	zassert_true(gpio_is_ready_dt(&gpio_port), "GPIO dev is not ready");
	zassert_true(gpio_is_ready_dt(&led), "LED dev is not ready");

	TC_PRINT("Running test on GPIO dev and pin\n");

	ret = gpio_pin_interrupt_configure_dt(&gpio_port, GPIO_INT_DISABLE);
	int ret_int = check_int_conf(ret);

	zassert_equal(ret_int, 0, "Pin interrupt disable failed\n");

	/* Configure GPIO PIN as an input */
	in_ret = gpio_pin_configure_dt(&gpio_port, GPIO_INPUT);
	int ret_PCI = check_pin_conf(in_ret);

	zassert_equal(ret_PCI, 0, "Failed to configure the input pin");

	/* Configure LED pin as an output */
	ou_ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	int ret_PCO = check_pin_conf(ou_ret);

	zassert_equal(ret_PCO, 0, "Failed to configure the output pin");

	cb_count = 0;
	gpio_init_callback(&gpio_cb_edge, callback_edge, BIT(gpio_port.pin));

	TC_PRINT("Callback initialized\n");
	int cb_ret = gpio_add_callback(gpio_port.port, &gpio_cb_edge);

	TC_PRINT("Callback added\n");

	int cb_ret_status = check_cb_res(cb_ret);

	zassert_equal(cb_ret_status, 0, "GPIO add callback failed\n");

	ret = gpio_pin_interrupt_configure_dt(&gpio_port, int_flags);
	int ret_int2 = check_int_conf(ret);

	zassert_equal(ret_int2, 0, "Interrupt pin configuring failed\n");

	TC_PRINT("LED pin starts toggling now...\n");
	for (int i = 0; i < 10; i++) {
		/* Toggle LED pin */
		int ret1 = gpio_pin_toggle_dt(&led);
		int ret_pin_tog = check_pin_toggle(ret1);

		zassert_equal(ret_pin_tog, 0, "Failed to toggle LED pin");

		pin_value = gpio_pin_get_dt(&gpio_port);
		TC_PRINT("Input pin value is %d\n", pin_value);

		k_msleep(SLEEP_TIME_MS);
	}

	ret = gpio_pin_interrupt_configure_dt(&gpio_port, GPIO_INT_DISABLE);
	int ret_int3 = check_int_conf(ret);

	zassert_equal(ret_int3, 0, "Pin interrupt disabling failed\n");

	int cb_rm_ret = gpio_remove_callback(gpio_port.port, &gpio_cb_edge);
	int cb_rm_status = check_cb_res(cb_rm_ret);

	zassert_equal(cb_rm_status, 0, "GPIO remove callback failed\n");
}

/** @brief Verify GPIO_INT_EDGE_FALLING flag. */
ZTEST(gpio_test_interrupts, test_gpio_int_edge_falling)
{
	test_gpio_pin_interrupt(GPIO_INT_EDGE_FALLING);
}

/** @brief Verify GPIO_INT_EDGE_RISING flag. */
ZTEST(gpio_test_interrupts, test_gpio_int_edge_rising)
{
	test_gpio_pin_interrupt(GPIO_INT_EDGE_RISING);
}

/** @brief Verify GPIO_INT_LEVEL_HIGH flag with 1 interrupt call */
ZTEST(gpio_test_interrupts, test_gpio_2int_level_high)
{
	int_level_type++;
	test_gpio_pin_interrupt(GPIO_INT_LEVEL_HIGH);
	int_level_type = 0;
}

/** @brief Verify GPIO_INT_LEVEL_LOW flag with 1 interrupt call */
ZTEST(gpio_test_interrupts, test_gpio_2int_level_low)
{
	int_level_type++;
	test_gpio_pin_interrupt(GPIO_INT_LEVEL_LOW);
	int_level_type = 0;
}

/** @brief Verify GPIO_INT_EDGE_BOTH flag with 1 interrupt call */
ZTEST(gpio_test_interrupts, test_gpio_int_edge_both)
{
	test_gpio_pin_interrupt(GPIO_INT_EDGE_BOTH);
}

ZTEST(gpio_test_interrupts, test_gpio_VO_FLEX_Pin_test)
{
	ztest_test_skip();
}

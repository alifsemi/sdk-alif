/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include "test_gpio.h"

extern int check_pin_conf(int ret);
extern int check_pin_toggle(int ret);

/*
 * Function to check gpio_pin_get_dt() return status
 */
int check_pin_get_conf(int ret)
{
	switch (ret) {
	case 1:
		TC_PRINT("Pin logical value is 1/active\n");
		return 0;

	case 0:
		TC_PRINT("Pin logical value is 0/inactive\n");
		return 0;

	case -EIO:
		TC_PRINT("Error when accessing\n");
		return -1;

	case -EWOULDBLOCK:
		TC_PRINT("Operation would block\n");
		return -1;

	default:
		TC_PRINT("Unexpected return value: %d\n", ret);
		return -1;
	}
}

/*
 * Function to check gpio_pin_set_dt() return status
 */
int check_pin_set_conf(int ret)
{
	switch (ret) {
	case 0:
		TC_PRINT("Logical level of output pin set successfully\n");
		return 0;

	case -EIO:
		TC_PRINT("Error when accessing\n");
		return -1;

	case -EWOULDBLOCK:
		TC_PRINT("Operation would block\n");
		return -1;

	default:
		TC_PRINT("Unexpected return value: %d\n", ret);
		return -1;
	}
}

/*
 * Function to check gpio_pin_is_input_dt() return status
 */
int check_pin_is_input(int ret)
{
	switch (ret) {
	case 1:
		TC_PRINT("Pin is input pin\n");
		return 0;

	case 0:
		TC_PRINT("Pin is not input pin\n");
		return -1;

	case -ENOSYS:
		TC_PRINT("Operation not implemented by the driver\n");
		ztest_test_skip();
		return -1;

	case -EIO:
		TC_PRINT("Error when accessing\n");
		return -1;

	case -EWOULDBLOCK:
		TC_PRINT("Operation would block\n");
		return -1;

	default:
		TC_PRINT("Unexpected return value: %d\n", ret);
		return -1;
	}
}

/*
 * Function to check gpio_pin_is_output_dt() return status
 */
int check_pin_is_output(int ret)
{
	switch (ret) {
	case 1:
		TC_PRINT("Pin is output pin\n");
		return 0;

	case 0:
		TC_PRINT("Pin is not output pin\n");
		return -1;

	case -ENOSYS:
		TC_PRINT("Operation not implemented by the driver\n");
		ztest_test_skip();
		return -1;

	case -EIO:
		TC_PRINT("Error when accessing\n");
		return -1;

	case -EWOULDBLOCK:
		TC_PRINT("Operation would block\n");
		return -1;

	default:
		TC_PRINT("Unexpected return value: %d\n", ret);
		return -1;
	}
}

/*
 * gpio test_disconnect test
 */
ZTEST(gpio_check, test_disconnect)
{
	int rv;
	static const struct gpio_dt_spec gpio_port = GPIO_DT_SPEC_GET(GPIO_NODE, gpios);

	rv = gpio_pin_is_input_dt(&gpio_port);
	int ret_in = check_pin_is_input(rv);

	zassert_not_equal(ret_in, 0, "Pin is not an input pin\n");

	rv = gpio_pin_is_output_dt(&gpio_port);
	int ret_ou = check_pin_is_output(rv);

	zassert_not_equal(ret_ou, 0, "Pin is not an output pin\n");
}

/*
 * gpio test_input test
 */
ZTEST(gpio_check, test_input)
{
	int rv, ret;
	static const struct gpio_dt_spec gpio_port = GPIO_DT_SPEC_GET(GPIO_NODE, gpios);

	/* Configure pin as input */
	ret = gpio_pin_configure_dt(&gpio_port, GPIO_INPUT);
	int ret_PCI = check_pin_conf(ret);

	zassert_equal(ret_PCI, 0, "Failed to configure input pin");

	rv = gpio_pin_is_input_dt(&gpio_port);
	int ret_in = check_pin_is_input(rv);

	zassert_equal(ret_in, 0, "Pin is not an input pin\n");

	rv = gpio_pin_is_output_dt(&gpio_port);
	int ret_ou = check_pin_is_output(rv);

	zassert_not_equal(ret_ou, 0, "Pin is not an output pin\n");
}

/*
 * gpio test_output test
 */
ZTEST(gpio_check, test_output)
{
	int ret, rv, ret_in, ret_ou, ret_PCI;
	static const struct gpio_dt_spec gpio_port = GPIO_DT_SPEC_GET(GPIO_NODE, gpios);

	/* Configure pin as GPIO_OUTPUT_LOW */
	ret = gpio_pin_configure_dt(&gpio_port, GPIO_OUTPUT_LOW);
	ret_PCI = check_pin_conf(ret);
	zassert_equal(ret_PCI, 0, "Failed to configure output pin");

	rv = gpio_pin_is_input_dt(&gpio_port);
	ret_in = check_pin_is_input(rv);
	zassert_not_equal(ret_in, 0, "Pin is not an input pin\n");

	rv = gpio_pin_is_output_dt(&gpio_port);
	ret_ou = check_pin_is_output(rv);
	zassert_equal(ret_ou, 0, "Pin is not an output pin\n");

	/* Configure pin as GPIO_OUTPUT_ACTIVE */
	ret = gpio_pin_configure_dt(&gpio_port, GPIO_OUTPUT_ACTIVE);
	ret_PCI = check_pin_conf(ret);
	zassert_equal(ret_PCI, 0, "Failed to configure output pin");

	rv = gpio_pin_is_input_dt(&gpio_port);
	ret_in = check_pin_is_input(rv);
	zassert_not_equal(ret_in, 0, "Pin is not an input pin\n");

	/* Toggle the LED */
	ret = gpio_pin_toggle_dt(&gpio_port);
	int ret_pin_tog = check_pin_toggle(ret);

	zassert_equal(ret_pin_tog, 0, "Failed to toggle LED pin");

	rv = gpio_pin_is_output_dt(&gpio_port);
	ret_ou = check_pin_is_output(rv);
	zassert_equal(ret_ou, 0, "Pin is not an output pin\n");

	/* Configure pin as GPIO_OUTPUT_HIGH */
	ret = gpio_pin_configure_dt(&gpio_port, GPIO_OUTPUT_HIGH);
	ret_PCI = check_pin_conf(ret);
	zassert_equal(ret_PCI, 0, "Failed to configure output pin");

	rv = gpio_pin_is_input_dt(&gpio_port);
	ret_in = check_pin_is_input(rv);
	zassert_not_equal(ret_in, 0, "Pin is not an input pin\n");

	rv = gpio_pin_is_output_dt(&gpio_port);
	ret_ou = check_pin_is_output(rv);
	zassert_equal(ret_ou, 0, "Pin is not an output pin\n");
}

/*
 * gpio_pin_get_dt - logical level of input pin
 * gpio_pin_set_dt - logical level of output pin
 */
ZTEST(gpio_check, test_gpio_input_output)
{
	int ret, i = 0, ret_res;
	static const struct gpio_dt_spec gpio_port = GPIO_DT_SPEC_GET(GPIO_NODE, gpios);

	ret = gpio_pin_configure_dt(&gpio_port, GPIO_OUTPUT_ACTIVE);
	int ret_PIO = check_pin_conf(ret);

	zassert_equal(ret_PIO, 0, "Failed to configure output pin");

	int res = gpio_pin_get_dt(&gpio_port);

	ret_res = check_pin_get_conf(res);
	zassert_equal(ret_res, 0, "Pin get configuration failed\n");
	TC_PRINT("GPIO Pin Value: %d\n", res);

	while (i < 10) {
		res = gpio_pin_get_dt(&gpio_port);
		ret_res = check_pin_get_conf(res);
		zassert_equal(ret_res, 0, "Failed to get pin configuration");
		TC_PRINT("GPIO Pin Value: %d\n", res);

		ret = gpio_pin_set_dt(&gpio_port, GPIO_OUTPUT_INIT_HIGH);
		ret_res = check_pin_set_conf(ret);
		zassert_equal(ret_res, 0, "Failed to set pin configuration");
		zassert_equal(ret, 0, "Failed to set pin configuration");

		res = gpio_pin_get_dt(&gpio_port);
		ret_res = check_pin_get_conf(res);
		zassert_equal(ret_res, 0, "Failed to get pin configuration");
		TC_PRINT("GPIO Pin Value after setting 1: %d\n", res);

		ret = gpio_pin_set_dt(&gpio_port, GPIO_OUTPUT_INIT_LOW);
		ret_res = check_pin_set_conf(ret);
		zassert_equal(ret_res, 0, "Failed to set pin configuration");

		res = gpio_pin_get_dt(&gpio_port);
		ret_res = check_pin_get_conf(res);
		zassert_equal(ret_res, 0, "Pin get configuration failed\n");
		TC_PRINT("GPIO Pin Value after setting 0: %d\n", res);

		k_msleep(SLEEP_TIME_MS);
		i++;
	}
}


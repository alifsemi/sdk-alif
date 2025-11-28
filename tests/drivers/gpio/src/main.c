/* Copyright (c) 2025 Alif Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "test_gpio.h"

int check_res(int ret)
{
	zassert_equal(ret, 0, "Failed to configure interrupt");
	zassert_not_equal(ret, -ENOSYS, "Operation is not implemented"
			" by the driver.\n");
	zassert_not_equal(ret, -ENOTSUP, "The configuration option"
			" is not supported\n");
	zassert_not_equal(ret, -EINVAL, "Invalid argument\n");
	zassert_not_equal(ret, -EBUSY, "Interrupt line required to configure pin
			"interrupt is already in use\n");
	zassert_not_equal(ret, -EIO, "I/O error when accessing\n");
	zassert_not_equal(ret, -EWOULDBLOCK, "Operation would block\n");

	return ret;
}

int check_conf(int ret)
{
	zassert_equal(ret, 0, "Failed to configure interrupt");
	zassert_not_equal(ret, -ENOTSUP, "The configuration option"
			" is not supported\n");
	zassert_not_equal(ret, -EINVAL, "Invalid argument\n");
	zassert_not_equal(ret, -EIO, "I/O error when accessing\n");
	zassert_not_equal(ret, -EWOULDBLOCK, "Operation would block\n");

	return ret;
}

ZTEST_SUITE(gpio_LED, NULL, NULL, NULL, NULL, NULL);
ZTEST_SUITE(gpio_check, NULL, NULL, NULL, NULL, NULL);
ZTEST_SUITE(gpio_test_interrupts, NULL, NULL, NULL, NULL, NULL);


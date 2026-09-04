/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */
#include <zephyr/kernel.h>
#include <zephyr/ztest.h>

#include "test_pwm_loopback.h"

static void *pwm_loopback_setup(void)
{
	struct test_pwm in;
	struct test_pwm out;

	get_test_pwms(&out, &in);

	k_object_access_grant(out.dev, k_current_get());
	k_object_access_grant(in.dev, k_current_get());

	return NULL;
}

ZTEST_SUITE(pwm_loopback, NULL, pwm_loopback_setup, NULL, NULL, NULL);

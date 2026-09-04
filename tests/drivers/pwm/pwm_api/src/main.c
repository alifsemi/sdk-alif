#include <zephyr/kernel.h>
#include <zephyr/ztest.h>
#include <zephyr/device.h>
#include "test_pwm_cycles.h"

void get_test_pwm(struct test_pwm *led);

static void *pwm_basic_setup(void)
{
	struct test_pwm led;

	get_test_pwm(&led);
	k_object_access_grant(led.dev, k_current_get());

	return NULL;
}

ZTEST_SUITE(pwm_basic, NULL, pwm_basic_setup, NULL, NULL, NULL);

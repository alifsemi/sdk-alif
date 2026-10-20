/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/ztest.h>

static const struct pwm_dt_spec pwm_led0 = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led0));

#define NUM_STEPS		50U
#define SLEEP_MSEC		25U

/*
 *	test_pwm_fade_led_app()
 */
ZTEST(fade_led, test_pwm_fade_led_app)
{
	uint32_t pulse_width = 0U;
	uint32_t step = pwm_led0.period / NUM_STEPS;
	uint8_t dir = 1U;
	int ret, i = 0;

	TC_PRINT("step = %u\n", step);
	TC_PRINT("PWM-based LED fade\n");

	if (!pwm_is_ready_dt(&pwm_led0)) {
		TC_PRINT("Error: PWM device %s is not ready\n",
				pwm_led0.dev->name);
		ztest_test_fail();
	}

	while (i++ <= 100) {
		ret = pwm_set_pulse_dt(&pwm_led0, pulse_width);
			if (ret) {
				TC_PRINT("Error %d: failed to set pulse width\n", ret);
				ztest_test_fail();
			}

			if (dir) {
				pulse_width += step;
					if (pulse_width >= pwm_led0.period) {
						pulse_width = pwm_led0.period - step;
						dir = 0U;
					}
			} else {
				if (pulse_width >= step) {
					pulse_width -= step;
				} else {
					pulse_width = step;
					dir = 1U;
				}
			}

			k_sleep(K_MSEC(SLEEP_MSEC));
	}

}

ZTEST_SUITE(fade_led, NULL, NULL, NULL, NULL, NULL);

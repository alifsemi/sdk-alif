/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/ztest.h>

static const struct pwm_dt_spec pwm_led0 = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led0));

#define MIN_PERIOD PWM_SEC(1U) / 128U
#define MAX_PERIOD PWM_SEC(1U)

/*
 *test_blinky_pwm_app()
 */
ZTEST(pwm_blinky, test_blinky_pwm_app)
{
	uint32_t max_period, i = 0;
	uint32_t period;
	uint8_t dir = 0U;
	int ret;

	printk("PWM-based blinky\n");

	if (!pwm_is_ready_dt(&pwm_led0)) {
		TC_PRINT("Error: PWM device %s is not ready\n",
				pwm_led0.dev->name);
		ztest_test_fail();
	}
	/*
	 * In case the default MAX_PERIOD value cannot be set for
	 * some PWM hardware, decrease its value until it can.
	 *
	 * Keep its value at least MIN_PERIOD * 4 to make sure
	 * the sample changes frequency at least once.
	 */
	TC_PRINT("Calibrating for channel %d...\n", pwm_led0.channel);
	max_period = MAX_PERIOD;
	while (pwm_set_dt(&pwm_led0, max_period, max_period / 2U)) {
		max_period /= 2U;
			if (max_period < (4U * MIN_PERIOD)) {
				TC_PRINT("Error: PWM device "
						"does not support a period at least %lu\n",
						4U * MIN_PERIOD);
				ztest_test_fail();
			}
	}
	TC_PRINT("Done calibrating; maximum/minimum periods %u/%lu nsec\n",
			max_period, MIN_PERIOD);

	period = max_period;
	while (i++ <= 20) {
		ret = pwm_set_dt(&pwm_led0, period, period / 2U);
			if (ret) {
				TC_PRINT("Error %d: failed to set pulse width\n", ret);
					ztest_test_fail();
			}

			period = dir ? (period * 2U) : (period / 2U);
			if (period > max_period) {
				period = max_period / 2U;
				dir = 0U;
			} else if (period < MIN_PERIOD) {
				period = MIN_PERIOD * 2U;
				dir = 1U;
			}

		k_sleep(K_MSEC(200));
	}
}

ZTEST_SUITE(pwm_blinky, NULL, NULL, NULL, NULL, NULL);


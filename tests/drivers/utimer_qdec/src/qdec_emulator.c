/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include "qdec_emulator.h"

#define PHASE_DELAY_US 1000

static const struct gpio_dt_spec phase_a = GPIO_DT_SPEC_GET(DT_ALIAS(qenca), gpios);
static const struct gpio_dt_spec phase_b = GPIO_DT_SPEC_GET(DT_ALIAS(qencb), gpios);

int qenc_emulate_init(void)
{
	int ret;

	if (!gpio_is_ready_dt(&phase_a) || !gpio_is_ready_dt(&phase_b)) {
		return -ENODEV;
	}

	ret = gpio_pin_configure_dt(&phase_a, GPIO_OUTPUT_INACTIVE);
	if (ret != 0) {
		return ret;
	}

	ret = gpio_pin_configure_dt(&phase_b, GPIO_OUTPUT_INACTIVE);
	if (ret != 0) {
		return ret;
	}

	return 0;
}

void simulate_cw_rotation(int steps)
{
	/*
	 * GPIO return values are ignored here since this is an emulation
	 * function for testing purposes. GPIO configuration is validated
	 * during qenc_emulate_init().
	 */
	for (int i = 0; i < steps; i++) {
		gpio_pin_set_dt(&phase_a, 1);
		k_busy_wait(PHASE_DELAY_US);
		gpio_pin_set_dt(&phase_b, 1);
		k_busy_wait(PHASE_DELAY_US);
		gpio_pin_set_dt(&phase_a, 0);
		k_busy_wait(PHASE_DELAY_US);
		gpio_pin_set_dt(&phase_b, 0);
		k_busy_wait(PHASE_DELAY_US);
	}
}

void simulate_ccw_rotation(int steps)
{
	for (int i = 0; i < steps; i++) {
		gpio_pin_set_dt(&phase_b, 1);
		k_busy_wait(PHASE_DELAY_US);
		gpio_pin_set_dt(&phase_a, 1);
		k_busy_wait(PHASE_DELAY_US);
		gpio_pin_set_dt(&phase_b, 0);
		k_busy_wait(PHASE_DELAY_US);
		gpio_pin_set_dt(&phase_a, 0);
		k_busy_wait(PHASE_DELAY_US);
	}
}

void simulate_glitch_pulses(int pulses)
{
	for (int i = 0; i < pulses; i++) {
		gpio_pin_set_dt(&phase_a, 1);
		k_busy_wait(5);
		gpio_pin_set_dt(&phase_a, 0);
		k_busy_wait(5);
	}
}

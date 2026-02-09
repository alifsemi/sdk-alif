/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */


#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include "ble_gpio.h"

LOG_MODULE_REGISTER(ui, LOG_LEVEL_DBG);

#define BUTTONS_NODE DT_PATH(buttons)
#define LEDS_NODE DT_PATH(leds)

#define GPIO_SPEC_AND_COMMA(button_or_led) GPIO_DT_SPEC_GET(button_or_led, gpios),

static const struct gpio_dt_spec buttons[] = {
#if DT_NODE_EXISTS(BUTTONS_NODE)
	DT_FOREACH_CHILD(BUTTONS_NODE, GPIO_SPEC_AND_COMMA)
#endif
};

static const struct gpio_dt_spec leds[] = {
#if DT_NODE_EXISTS(LEDS_NODE)
	DT_FOREACH_CHILD(LEDS_NODE, GPIO_SPEC_AND_COMMA)
#endif
};

static void ButtonWorkerHandler(struct k_work *work);

static K_WORK_DELAYABLE_DEFINE(buttonWork, ButtonWorkerHandler);
static button_handler_cb mButtonHandler;

static int ButtonInterruptCtrl(bool enable)
{
	int err = 0;
	gpio_flags_t flags;

	for (size_t i = 0; (i < ARRAY_SIZE(buttons)) && !err; i++) {
		if (enable) {
			flags = GPIO_INT_EDGE_BOTH;
		} else {
			flags = GPIO_INT_DISABLE;
		}

		err = gpio_pin_interrupt_configure_dt(&buttons[i], flags);
		if (err) {
			LOG_ERR("GPIO IRQ config set failed: %d", err);
			return err;
		}
	}

	return err;
}

static int ButtonStateRead(void)
{
	int mask = 0;

	for (size_t i = 0; (i < ARRAY_SIZE(buttons)); i++) {
		int pin_state;

		pin_state = gpio_pin_get_dt(&buttons[i]);
		if (pin_state < 0) {
			return 0;
		}
		if (pin_state) {
			/* Mark Active Button state */
			mask |= 1U << i;
		}
	}

	return mask;
}

static void ButtonWorkerHandler(struct k_work *work)
{
	static uint32_t last_button_scan;
	static bool first_run = true;
	uint32_t button_mask;

	button_mask = ButtonStateRead();

	if (!first_run) {
		if (button_mask != last_button_scan) {
			uint32_t has_changed = (button_mask ^ last_button_scan);

			if (mButtonHandler) {
				mButtonHandler(button_mask, has_changed);
			}
		}
	} else {
		first_run = false;
	}

	last_button_scan = button_mask;

	if (button_mask != 0) {
		/* Button still pressed schedule new poll round */
		k_work_reschedule(&buttonWork, K_MSEC(25));
	} else {
		/* All buttons released enable interrupts again */
		ButtonInterruptCtrl(true);
	}
}

static void ButtonEventHandler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	/* Disable Interrupts */
	ButtonInterruptCtrl(false);
	/* Button scan process trigger */
	k_work_reschedule(&buttonWork, K_MSEC(1));
}

int ble_gpio_buttons_init(button_handler_cb buttonHandler)
{
	static struct gpio_callback button_cb_data;
	uint32_t callback_pin_mask = 0;
	int err;

	for (size_t i = 0; i < ARRAY_SIZE(buttons); i++) {

		if (!gpio_is_ready_dt(&buttons[i])) {
			LOG_ERR("Button %zu not ready", i);
			return -1;
		}
		err = gpio_pin_configure_dt(&buttons[i], GPIO_INPUT);
		if (err) {
			LOG_ERR("Button configure failed %zu", i);
			return err;
		}
		/* Disable Interrupt by default */
		err = gpio_pin_interrupt_configure_dt(&buttons[i], GPIO_INT_DISABLE);
		if (err) {
			LOG_ERR("Button %zu interrupt config failed: %d", i, err);
			return err;
		}
		callback_pin_mask |= BIT(buttons[i].pin);
	}

	/* Init callback handlers */
	gpio_init_callback(&button_cb_data, ButtonEventHandler, callback_pin_mask);
	for (size_t i = 0; i < ARRAY_SIZE(buttons); i++) {
		err = gpio_add_callback(buttons[i].port, &button_cb_data);
		if (err) {
			LOG_ERR("Callback add failed for button %zu: %d", i, err);
			return err;
		}
	}

	/* Set Button user callback */
	mButtonHandler = buttonHandler;

	k_work_reschedule(&buttonWork, K_MSEC(1));

	return 0;
}

int ble_gpio_led_init(void)
{
	int err;

	for (size_t i = 0; i < ARRAY_SIZE(leds); i++) {

		if (!gpio_is_ready_dt(&leds[i])) {
			LOG_ERR("LED %zu not ready", i);
			return -1;
		}
		err = gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT_ACTIVE);
		if (err) {
			LOG_ERR("LED %zu configure failed: %d", i, err);
			return err;
		}
		/* Disable led */
		gpio_pin_set_dt(&leds[i], 0);
	}

	return 0;
}

void ble_gpio_led_set(const struct gpio_dt_spec *led_dev, bool enable)
{
	gpio_pin_set_dt(led_dev, enable);
}

void ble_gpio_led_toggle(const struct gpio_dt_spec *led_dev)
{
	gpio_pin_toggle_dt(led_dev);
}

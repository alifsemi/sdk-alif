/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */


#include <errno.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include "ble_gpio.h"

LOG_MODULE_REGISTER(ui, LOG_LEVEL_DBG);

#define BUTTONS_NODE DT_PATH(buttons)
#define LEDS_NODE DT_PATH(leds)

#define GPIO_SPEC_AND_COMMA(button_or_led) GPIO_DT_SPEC_GET(button_or_led, gpios),

/* Child index in the full /buttons list. A disabled earlier sibling stays in
 * this list, so the callback bit does not move.
 */
#define BUTTON_IDX_AND_COMMA(node) DT_NODE_CHILD_IDX(node),

static const struct gpio_dt_spec buttons[] = {
#if DT_NODE_HAS_STATUS(BUTTONS_NODE, okay)
	DT_FOREACH_CHILD_STATUS_OKAY(BUTTONS_NODE, GPIO_SPEC_AND_COMMA)
#endif
};

static const uint8_t button_idx[] = {
#if DT_NODE_HAS_STATUS(BUTTONS_NODE, okay)
	DT_FOREACH_CHILD_STATUS_OKAY(BUTTONS_NODE, BUTTON_IDX_AND_COMMA)
#endif
};

static const struct gpio_dt_spec leds[] = {
#if DT_NODE_HAS_STATUS(LEDS_NODE, okay)
	DT_FOREACH_CHILD_STATUS_OKAY(LEDS_NODE, GPIO_SPEC_AND_COMMA)
#endif
};

/*
 * gpio-keys / gpio-leds use GPIO_ACTIVE_LOW, and the gpio-keys example also
 * sets GPIO_PULL_UP. This GPIO controller has no pin bias and rejects those
 * flags with -ENOTSUP. Keep the polarity and drop only the bias flags.
 */
static int gpio_configure(const struct gpio_dt_spec *spec, gpio_flags_t extra_flags)
{
	int err = gpio_pin_configure_dt(spec, extra_flags);

	if (err == -ENOTSUP && (spec->dt_flags & (GPIO_PULL_UP | GPIO_PULL_DOWN)) != 0) {
		struct gpio_dt_spec nopull = *spec;

		nopull.dt_flags &= ~(gpio_flags_t)(GPIO_PULL_UP | GPIO_PULL_DOWN);
		err = gpio_pin_configure_dt(&nopull, extra_flags);
	}

	return err;
}

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
		if (pin_state && button_idx[i] < 32) {
			/* Bit is the child's index, not its place in the enabled list */
			mask |= 1U << button_idx[i];
		}
	}

	return mask;
}

uint32_t ble_gpio_button_cb_mask_dt(struct gpio_dt_spec button)
{
	if (button.port == NULL) {
		return 0U;
	}

	for (size_t i = 0; i < ARRAY_SIZE(buttons); i++) {
		if (buttons[i].port == button.port && buttons[i].pin == button.pin) {
			if (button_idx[i] < 32) {
				return 1U << button_idx[i];
			}
			return 0U;
		}
	}

	return 0U;
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
	static struct gpio_callback button_cb[ARRAY_SIZE(buttons) > 0 ? ARRAY_SIZE(buttons) : 1];
	int err;

	for (size_t i = 0; i < ARRAY_SIZE(buttons); i++) {

		if (!gpio_is_ready_dt(&buttons[i])) {
			LOG_ERR("Button %zu not ready", i);
			return -1;
		}
		err = gpio_configure(&buttons[i], GPIO_INPUT);
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
	}

	/* One callback per port. A gpio_callback can only be linked to one port. */
	for (size_t i = 0; i < ARRAY_SIZE(buttons); i++) {
		bool seen = false;
		uint32_t pin_mask = 0;

		for (size_t j = 0; j < i; j++) {
			if (buttons[j].port == buttons[i].port) {
				seen = true;
				break;
			}
		}
		if (seen) {
			continue;
		}

		for (size_t j = i; j < ARRAY_SIZE(buttons); j++) {
			if (buttons[j].port == buttons[i].port) {
				pin_mask |= BIT(buttons[j].pin);
			}
		}

		gpio_init_callback(&button_cb[i], ButtonEventHandler, pin_mask);
		err = gpio_add_callback(buttons[i].port, &button_cb[i]);
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
		/* Logical off, so GPIO_ACTIVE_LOW LEDs start dark */
		err = gpio_configure(&leds[i], GPIO_OUTPUT_INACTIVE);
		if (err) {
			LOG_ERR("LED %zu configure failed: %d", i, err);
			return err;
		}
	}

	return 0;
}

/* Only pins configured by ble_gpio_led_init() may be touched. A disabled LED
 * stays out of leds[] and may be reserved for another use.
 */
static bool led_is_configured(const struct gpio_dt_spec *led_dev)
{
	if (led_dev == NULL || led_dev->port == NULL) {
		return false;
	}

	for (size_t i = 0; i < ARRAY_SIZE(leds); i++) {
		if (leds[i].port == led_dev->port && leds[i].pin == led_dev->pin) {
			return true;
		}
	}

	return false;
}

void ble_gpio_led_set(const struct gpio_dt_spec *led_dev, bool enable)
{
	if (!led_is_configured(led_dev)) {
		return;
	}

	gpio_pin_set_dt(led_dev, enable);
}

void ble_gpio_led_toggle(const struct gpio_dt_spec *led_dev)
{
	if (!led_is_configured(led_dev)) {
		return;
	}

	gpio_pin_toggle_dt(led_dev);
}

int ble_gpio_led_get(const struct gpio_dt_spec *led_dev)
{
	if (!led_is_configured(led_dev)) {
		return 0;
	}

	return gpio_pin_get_dt(led_dev);
}

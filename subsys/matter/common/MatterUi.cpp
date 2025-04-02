/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include "MatterUi.h"
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

#define BUTTONS_NODE DT_PATH(buttons)

#define GPIO_SPEC_AND_COMMA(button_or_led) GPIO_DT_SPEC_GET(button_or_led, gpios),

static const struct gpio_dt_spec buttons[] = {
#if DT_NODE_EXISTS(BUTTONS_NODE)
	DT_FOREACH_CHILD(BUTTONS_NODE, GPIO_SPEC_AND_COMMA)
#endif
};


static const struct gpio_dt_spec statusLed = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

int MatterUi::ButtonInterruptCtrl(bool enable)
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

int MatterUi::ButtonStateRead(void)
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

void MatterUi::ButtonWorkerHandler(struct k_work *work)
{
	static uint32_t last_button_scan;
	static bool first_run = true;
	uint32_t button_mask;

	button_mask = ButtonStateRead();

	if (!first_run) {
		if (button_mask != last_button_scan) {
			uint32_t has_changed = (button_mask ^ last_button_scan);

			if (Instance().mButtonHandler) {
				Instance().mButtonHandler(button_mask, has_changed);
			}
		}
	} else {
		first_run = false;
	}

	last_button_scan = button_mask;

	if (button_mask != 0) {
                /* Button still pressed shedule new poll round */
		k_work_reschedule(&Instance().buttonWork, K_MSEC(25));
	} else {
                 /* All buttons released enable interrupts again */
                 ButtonInterruptCtrl(true);
	}
}

void MatterUi::ButtonEventHandler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
        /* Disable Interrupts */
        ButtonInterruptCtrl(false);
	/* Button scan process trigger  */
	k_work_reschedule(&Instance().buttonWork, K_MSEC(1));
}

int MatterUi::ButtonInit(void)
{
	static struct gpio_callback button_cb_data;
	uint32_t callback_pin_mask = 0;
	int err;

	for (size_t i = 0; i < ARRAY_SIZE(buttons); i++) {

		if (!gpio_is_ready_dt(&buttons[i])) {
			LOG_ERR("Button is %d not ready", i);
			return -1;
		}
		err = gpio_pin_configure_dt(&buttons[i], GPIO_INPUT);
		if (err) {
			LOG_ERR("Button configure failed %d", i);
			return err;
		}
		/* Disable Interrupt by default */
		err = gpio_pin_interrupt_configure_dt(&buttons[i], GPIO_INT_DISABLE);
		if (err) {
			LOG_ERR("button int conf failed %d", i);
			return err;
		}
		callback_pin_mask |= BIT(buttons[i].pin);
	}

        /* Init callback handlers */
	gpio_init_callback(&button_cb_data, ButtonEventHandler, callback_pin_mask);
	for (size_t i = 0; i < ARRAY_SIZE(buttons); i++) {
		err = gpio_add_callback(buttons[i].port, &button_cb_data);
		if (err) {
			LOG_ERR("cb add failed to %d", i);
			return err;
		}
	}

	return 0;
}

int MatterUi::StatusLedInit(void)
{
	/* Configure LED */
	int err;

	if (!gpio_is_ready_dt(&statusLed)) {
		LOG_ERR("led is not ready!");
		return -1;
	}

	err = gpio_pin_configure_dt(&statusLed, GPIO_OUTPUT_ACTIVE);
	if (err < 0) {
		LOG_ERR("led configure failed");
		return err;
	}

	k_timer_init(&Instance().ledTimer, MatterUi::StatusLedTimer, nullptr);
	/* LED initial state */
	gpio_pin_set_dt(&statusLed, 0);

	return 0;
}

void MatterUi::StatusLedTimer(k_timer * timer)
{
	if (Instance().statusTimerPeriodMs) {
		gpio_pin_toggle_dt(&statusLed);
		k_timer_start(&Instance().ledTimer, K_MSEC(Instance().statusTimerPeriodMs), K_NO_WAIT);
	} else {
		/* Disable led for short blinky */
		gpio_pin_set_dt(&statusLed, 0);
	}
	
}

bool MatterUi::Init(ButtonHandler buttonHandler)
{
	/* Init button worker */
	memset(&Instance().buttonWork, 0, sizeof(struct k_work_delayable));
	Instance().buttonWork = Z_WORK_DELAYABLE_INITIALIZER(ButtonWorkerHandler);

        /* Set Button user calback */
	mButtonHandler = buttonHandler;
	statusTimerPeriodMs = 0;
        /* Init button */
	if (ButtonInit()) {
		return false;
	}

	if (StatusLedInit()) {
		return false;
	}
	k_work_reschedule(&buttonWork, K_MSEC(1));
	return true;
}

void MatterUi::StatusLedTimerSet(int period_ms)
{
	if (statusTimerPeriodMs == 0) {
		int set_led_blink_period;

		if (period_ms == 0) {
			/* Set short Blink */
			set_led_blink_period = 100;
		} else {
			statusTimerPeriodMs = period_ms;
			set_led_blink_period = period_ms;
		}
		/* Enable led state */
		gpio_pin_set_dt(&statusLed, 1);
		k_timer_start(&ledTimer, K_MSEC(set_led_blink_period), K_NO_WAIT);

	} else if (statusTimerPeriodMs != period_ms) {
		/* Update Period */
		statusTimerPeriodMs = period_ms;
	}
}
/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#pragma once

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <platform/CHIPDeviceEvent.h>

class MatterUi
{
	using ButtonHandler = void (*)(uint32_t button_state, uint32_t has_changed);

      private:
	MatterUi() = default;

	static int ButtonInit(void);
	static int ButtonInterruptCtrl(bool enable);
	static int ButtonStateRead(void);
	static void ButtonWorkerHandler(struct k_work *work);
	static void ButtonEventHandler(const struct device *dev, struct gpio_callback *cb,
				   uint32_t pins);
	static int StatusLedInit(void);
	static void StatusLedTimer(k_timer * timer);

	struct k_work_delayable buttonWork;
	struct k_timer ledTimer;
	ButtonHandler mButtonHandler = nullptr;
	int statusTimerPeriodMs = 0;
	

      public:
	bool Init(ButtonHandler buttonHandler = nullptr);
	void StatusLedTimerSet(int period_ms);
	static MatterUi &Instance()
	{
		static MatterUi sMatterUi;
		return sMatterUi;
	};
};

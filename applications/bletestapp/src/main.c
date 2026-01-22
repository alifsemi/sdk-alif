/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/init.h>
#include <zephyr/pm/pm.h>
#include <zephyr/pm/policy.h>
#include <zephyr/sys/poweroff.h>
#include <zephyr/drivers/counter.h>
#include <zephyr/drivers/uart.h>
#include <cmsis_core.h>
#include <soc_common.h>
#include <se_service.h>
#include <es0_power_manager.h>

#include "ke_mem.h"

#include "appl_shell.h"
#include "power_mgr.h"
#include "ble_handler.h"

#if !defined(CONFIG_SOC_SERIES_B1)
#error "Application works only with B1 devices"
#endif

/**
 * Use the HFOSC clock for the UART console
 */
#if DT_SAME_NODE(DT_NODELABEL(uart4), DT_CHOSEN(zephyr_console))
#define CONSOLE_UART_NUM        4
#define EARLY_BOOT_CONSOLE_INIT 1
#elif DT_SAME_NODE(DT_NODELABEL(uart3), DT_CHOSEN(zephyr_console))
#define CONSOLE_UART_NUM        3
#define EARLY_BOOT_CONSOLE_INIT 1
#elif DT_SAME_NODE(DT_NODELABEL(uart2), DT_CHOSEN(zephyr_console))
#define CONSOLE_UART_NUM        2
#define EARLY_BOOT_CONSOLE_INIT 1
#elif DT_SAME_NODE(DT_NODELABEL(uart1), DT_CHOSEN(zephyr_console))
#define CONSOLE_UART_NUM        1
#define EARLY_BOOT_CONSOLE_INIT 1
#else
#define EARLY_BOOT_CONSOLE_INIT 0
#endif

#if EARLY_BOOT_CONSOLE_INIT
#define UART_CTRL_CLK_SEL_POS 8
static int app_pre_console_init(void)
{
	/* Enable HFOSC in CGU */
	sys_set_bits(CGU_CLK_ENA, BIT(23));

	/* Enable HFOSC for the UART console */
	sys_clear_bits(EXPSLV_UART_CTRL, BIT((CONSOLE_UART_NUM + UART_CTRL_CLK_SEL_POS)));
	return 0;
}

/**
 * Console UART init is needed to adjust clocks properly.
 * This must be fixed into UART dirver and this code can be
 * removed when ready.
 */
SYS_INIT(app_pre_console_init, PRE_KERNEL_1, 50);
#endif

static volatile bool wakeup_status;
static volatile int run_profile_error;

/* Macros */
LOG_MODULE_REGISTER(main, CONFIG_MAIN_LOG_LEVEL);

/*
 * CRITICAL: Must run at PRE_KERNEL_1 to restore SYSTOP before peripherals initialize.
 *
 * On cold boot: SYSTOP is already ON by default, safe to call.
 * On SOFT_OFF wakeup: SYSTOP is OFF, must restore BEFORE peripherals access registers.
 */
SYS_INIT(app_set_run_params, PRE_KERNEL_1, 3);

/**
 * PM Notifier callback for power state entry
 */
static void pm_notify_state_entry(enum pm_state const state)
{
	/* TODO: enable when this is needed */
	/*
	 * const struct pm_state_info *next_state = pm_state_next_get(0);
	 * uint8_t substate_id = next_state ? next_state->substate_id : 0;
	 */

	switch (state) {
	case PM_STATE_SUSPEND_TO_RAM:
	case PM_STATE_SOFT_OFF:
		break;
	default:
		__ASSERT(false, "Entering unknown power state %d", state);
		LOG_ERR("Entering unknown power state %d", state);
		break;
	}
}

/**
 * PM Notifier callback called BEFORE devices are resumed
 *
 * This restores SE run configuration when resuming from S2RAM states.
 * Note: For SOFT_OFF, the system resets completely and app_set_run_params()
 * runs during normal PRE_KERNEL_1 initialization, so this callback is not needed.
 */
static void pm_notify_pre_device_resume(enum pm_state const state)
{
	wakeup_status = get_wakeup_irq_status();

	switch (state) {
	case PM_STATE_SUSPEND_TO_RAM: {
		run_profile_error = app_set_run_params();
		break;
	}
	case PM_STATE_SOFT_OFF: {
		/* No action needed - SOFT_OFF causes reset, not resume */
		break;
	}
	default: {
		__ASSERT(false, "Pre-resume for unknown power state %d", state);
		LOG_ERR("Pre-resume for unknown power state %d", state);
		break;
	}
	}
}

/**
 * PM Notifier structure
 */
static struct pm_notifier app_pm_notifier = {
	.state_entry = pm_notify_state_entry,
	.pre_device_resume = pm_notify_pre_device_resume,
};

/*
 * This function will be invoked in the PRE_KERNEL_1 phase of the init
 * routine to prevent sleep during startup.
 */
static int app_pre_kernel_init(void)
{
	/* Register PM notifier callbacks */
	pm_notifier_register(&app_pm_notifier);

	pm_policy_state_lock_get(PM_STATE_SOFT_OFF, PM_ALL_SUBSTATES);
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_RAM, PM_ALL_SUBSTATES);

	return 0;
}
SYS_INIT(app_pre_kernel_init, PRE_KERNEL_1, 39);

#if defined(CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_HOOKS)

static uint32_t idle_timer_pre_idle;

/* Idle timer used for timer while entering the idle state */
static const struct device *idle_timer = DEVICE_DT_GET(DT_CHOSEN(zephyr_cortex_m_idle_timer));
/**
 * To simplify the driver, implement the callout to Counter API
 * as hooks that would be provided by platform drivers if
 * CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_HOOKS was selected instead.
 */
void z_cms_lptim_hook_on_lpm_entry(uint64_t max_lpm_time_us)
{

	/* Store current value of the selected timer to calculate a
	 * difference in measurements after exiting the idle state.
	 */
	counter_get_value(idle_timer, &idle_timer_pre_idle);
	/**
	 * Disable the counter alarm in case it was already running.
	 */
	/* counter_cancel_channel_alarm(idle_timer, 0); */

	/* Set the alarm using timer that runs the idle.
	 * Needed rump-up/setting time, lower accurency etc. should be
	 * included in the exit-latency in the power state definition.
	 */

	struct counter_alarm_cfg cfg = {
		.callback = NULL,
		.ticks = counter_us_to_ticks(idle_timer, max_lpm_time_us) + idle_timer_pre_idle,
		.user_data = NULL,
		.flags = COUNTER_ALARM_CFG_ABSOLUTE,
	};
	counter_set_channel_alarm(idle_timer, 0, &cfg);
}

uint64_t z_cms_lptim_hook_on_lpm_exit(void)
{
	/**
	 * Calculate how much time elapsed according to counter.
	 */
	uint32_t idle_timer_post, idle_timer_diff;

	counter_get_value(idle_timer, &idle_timer_post);

	/**
	 * Check for counter timer overflow
	 * (TODO: this doesn't work for downcounting timers!)
	 */
	if (idle_timer_pre_idle > idle_timer_post) {
		idle_timer_diff = (counter_get_top_value(idle_timer) - idle_timer_pre_idle) +
				  idle_timer_post + 1;
	} else {
		idle_timer_diff = idle_timer_post - idle_timer_pre_idle;
	}

	return (uint64_t)counter_ticks_to_us(idle_timer, idle_timer_diff);
}
#endif /* CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER */

int main(void)
{
	if (is_cold_boot()) {
		int ret = pwm_init();

		if (ret) {
			LOG_ERR("pwm_init failed: %d", ret);
			return ret;
		}

		ret = set_off_profile(PM_STATE_MODE_STOP);

		if (ret) {
			LOG_ERR("off profile set failed. error: %d", ret);
			return ret;
		}
		printk("BLE testapp cold start\n");
	}

	appl_wait_to_continue();

	int ret = ble_init();

	if (ret) {
		return ret;
	}

	if (appl_allow_sleep()) {
		app_ready_for_sleep();
	}

	while (1) {
		/* checks for whether sleep period is done */
		appl_allow_sleep();

		if (!ble_is_connected()) {
			k_sleep(K_MSEC(ble_rtc_connected_wakeup));
		} else {
			k_sleep(K_MSEC(ble_rtc_wakeup));
		}
	}
	return 0;
}

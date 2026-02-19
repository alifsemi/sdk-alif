/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/pm/pm.h>
#include <zephyr/pm/policy.h>
#include <zephyr/drivers/counter.h>
#include <zephyr/shell/shell.h>
#include <zephyr/shell/shell_uart.h>
#include <cmsis_core.h>
#include <se_service.h>
#include "power_mgr.h"
#include "power_defines.h"
#include "ble_handler.h"

struct k_timer sleep_end_timer;
void sleep_period_end(struct k_timer *timer_id);
void sleep_period_done(struct k_work *work);

K_WORK_DEFINE(sleep_end_worker, sleep_period_done);

LOG_MODULE_DECLARE(main, CONFIG_MAIN_LOG_LEVEL);

extern enum lp_counter_source counter_source;
static const struct device *wakeup_dev;
static bool counter_started;

run_profile_t current_runp __attribute__((noinit));
off_profile_t current_offp __attribute__((noinit));
int16_t power_profile __attribute__((noinit));

static int power_mgr_init(void)
{
	counter_started = false;
	wakeup_dev = 0;
	return select_timer_source(LPRTC);
}

int select_timer_source(enum lp_counter_source source)
{
	int ret = 0;

	if (counter_source == source) {
		return 0;
	}

	if (counter_started) {
		ret = counter_stop(wakeup_dev);
		if (ret && ret != -EALREADY) {
			printk("failed to stop counter: %d", ret);
			return -1;
		}
		counter_started = false;
	}

	wakeup_dev = 0;

	if (source == LPRTC) {
#ifndef WAKEUP_SOURCE_RTC
		printk("Wakeup Device RTC not enabled in the dts");
		return -1;
#else
		wakeup_dev = DEVICE_DT_GET(WAKEUP_SOURCE_RTC);
#endif
	} else if (source == LPTIMER) {
#ifndef WAKEUP_SOURCE_LPTIMER
		printk("Wakeup Device LPTIMER not enabled in the dts");
		return -1;
#else
		wakeup_dev = DEVICE_DT_GET(WAKEUP_SOURCE_LPTIMER);
#endif
	} else {
		LOG_ERR("Invalid timer source selected");
		return -1;
	}

	if (!device_is_ready(wakeup_dev)) {
		printk("%s: device not ready", wakeup_dev->name);
		return -1;
	}

	ret = counter_start(wakeup_dev);

	if (ret && ret != -EALREADY) {
		printk("failed to start counter: %d", ret);
		return -1;
	}

	counter_source = source;
	counter_started = true;
	get_default_off_cfg(&current_offp);
	ret = set_off_profile(PM_STATE_MODE_STOP_1);

	get_default_run_cfg(&current_runp);

	if (ret) {
		LOG_ERR("off profile set failed. error: %d", ret);
	}
	k_timer_init(&sleep_end_timer, sleep_period_end, NULL);

	return 0;
}
SYS_INIT(power_mgr_init, POST_KERNEL, 99);

void app_ready_for_sleep(void)
{
	pm_policy_state_lock_put(PM_STATE_SOFT_OFF, PM_ALL_SUBSTATES);
	pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_RAM, PM_ALL_SUBSTATES);
}

void app_prevent_off(void)
{
	pm_policy_state_lock_get(PM_STATE_SOFT_OFF, PM_ALL_SUBSTATES);
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_RAM, PM_ALL_SUBSTATES);
}

void sleep_period_done(struct k_work *work)
{
	const struct shell *shell = shell_backend_uart_get_ptr();

	shell_print(shell, "sleep period done.");
}

void sleep_period_end(struct k_timer *timer_id)
{
	app_prevent_off();
	k_work_submit(&sleep_end_worker);
}

void app_sleep_start(uint32_t time_s)
{
	k_timer_start(&sleep_end_timer, K_SECONDS(time_s), K_NO_WAIT);
	app_ready_for_sleep();
}

int set_current_off_profile(void)
{
	int ret = se_service_set_off_cfg(&current_offp);

	if (ret) {
		LOG_ERR("SE: set_off_cfg failed = %d", ret);
	}
	return ret;
}

int set_off_profile(const enum pm_state_mode_type pm_mode)
{
	/* Set default for stop mode with RTC wakeup support */
	current_offp.power_domains = PD_VBAT_AON_MASK;
/* If CONFIG_FLASH_BASE_ADDRESS is zero application run from itcm and no MRAM needed */
#if (CONFIG_FLASH_BASE_ADDRESS == 0)
	current_offp.memory_blocks = 0;
#else
	current_offp.memory_blocks = MRAM_MASK;
#endif
	current_offp.memory_blocks |= SERAM_MEMORY_BLOCKS_IN_USE;
	current_offp.memory_blocks |= APP_RET_MEM_BLOCKS;
	current_offp.dcdc_voltage = 775;
	current_offp.vdd_ioflex_3V3 = IOFLEX_LEVEL_1V8;

	switch (pm_mode) {
	case PM_STATE_MODE_IDLE_1:
	case PM_STATE_MODE_STANDBY_1:
		current_offp.power_domains |= PD_SSE700_AON_MASK;
		current_offp.ip_clock_gating = 0;
		current_offp.phy_pwr_gating = 0;
		current_offp.dcdc_mode = DCDC_MODE_PFM_AUTO;
		break;
	case PM_STATE_MODE_STOP_1:
		current_offp.ip_clock_gating = 0;
		current_offp.phy_pwr_gating = 0;
		current_offp.dcdc_mode = DCDC_MODE_OFF;
		break;
	case PM_STATE_MODE_GO_1:
	case PM_STATE_MODE_GO_2:
	case PM_STATE_MODE_GO_3:
	case PM_STATE_MODE_GO_4:
	case PM_STATE_MODE_GO_5:
	case PM_STATE_MODE_READY_1:
	case PM_STATE_MODE_READY_2:
	case PM_STATE_MODE_IDLE_2:
	case PM_STATE_MODE_STOP_2:
	case PM_STATE_MODE_STOP_3:
	case PM_STATE_MODE_STOP_4:
	case PM_STATE_MODE_STOP_5:
		break;
	}

	current_offp.aon_clk_src = CLK_SRC_LFXO;
	current_offp.stby_clk_src = CLK_SRC_HFRC;
	current_offp.stby_clk_freq = SCALED_FREQ_RC_STDBY_0_075_MHZ;
	if (counter_source == LPRTC) {
		current_offp.ewic_cfg = EWIC_RTC_A;
		current_offp.wakeup_events = WE_LPRTC;
	} else if (counter_source == LPTIMER) {
		current_offp.ewic_cfg = EWIC_VBAT_TIMER;
		current_offp.wakeup_events = WE_LPTIMER;
	} else {
		LOG_ERR("Invalid timer source selected");
		return -1;
	}
	current_offp.vtor_address = SCB->VTOR;
	current_offp.vtor_address_ns = SCB->VTOR;

	return set_current_off_profile();
}

void get_default_run_cfg(run_profile_t *runp)
{
	runp->power_domains = PD_VBAT_AON_MASK | PD_SYST_MASK | PD_SSE700_AON_MASK;
	runp->power_domains |= PD_SESS_MASK | PD_DBSS_MASK;
	runp->dcdc_voltage = 775;
	runp->dcdc_mode = DCDC_MODE_PFM_FORCED;
	runp->aon_clk_src = CLK_SRC_LFXO;
	runp->run_clk_src = CLK_SRC_PLL;
	runp->cpu_clk_freq = CLOCK_FREQUENCY_160MHZ;
	runp->phy_pwr_gating = 0;
	runp->ip_clock_gating = 0;
	runp->vdd_ioflex_3V3 = IOFLEX_LEVEL_1V8;
	runp->scaled_clk_freq = SCALED_FREQ_RC_ACTIVE_76_8_MHZ;

	runp->memory_blocks = MRAM_MASK;
	runp->memory_blocks |= SERAM_MEMORY_BLOCKS_IN_USE;
	runp->memory_blocks |= APP_RET_MEM_BLOCKS;

	if (IS_ENABLED(CONFIG_MIPI_DSI)) {
		runp->phy_pwr_gating |= MIPI_TX_DPHY_MASK | MIPI_RX_DPHY_MASK;
		runp->phy_pwr_gating |= MIPI_PLL_DPHY_MASK;
		runp->ip_clock_gating |= CDC200_MASK | MIPI_DSI_MASK | GPU_MASK;
	}
}

void get_default_off_cfg(off_profile_t *offp)
{
	/* Default is STOP mode */
	offp->power_domains = PD_VBAT_AON_MASK;
/* If CONFIG_FLASH_BASE_ADDRESS is zero application run from itcm and no MRAM needed */
#if (CONFIG_FLASH_BASE_ADDRESS == 0)
	offp->memory_blocks = 0;
#else
	offp->memory_blocks = MRAM_MASK;
#endif
	offp->memory_blocks |= SERAM_MEMORY_BLOCKS_IN_USE;
	offp->memory_blocks |= APP_RET_MEM_BLOCKS;
	offp->dcdc_voltage = 775;
	offp->ip_clock_gating = 0;
	offp->phy_pwr_gating = 0;
	offp->vdd_ioflex_3V3 = IOFLEX_LEVEL_1V8;
	offp->dcdc_mode = DCDC_MODE_OFF;
	offp->aon_clk_src = CLK_SRC_LFXO;
	offp->stby_clk_src = CLK_SRC_HFRC;
	offp->stby_clk_freq = SCALED_FREQ_RC_STDBY_0_075_MHZ;
	if (counter_source == LPRTC) {
		offp->ewic_cfg = EWIC_RTC_A;
		offp->wakeup_events = WE_LPRTC;
	} else if (counter_source == LPTIMER) {
		offp->ewic_cfg = EWIC_VBAT_TIMER;
		offp->wakeup_events = WE_LPTIMER;
	} else {
		LOG_ERR("Invalid timer source selected");
	}
	offp->vtor_address = SCB->VTOR;
	offp->vtor_address_ns = SCB->VTOR;
}

/**
 * Set the RUN profile parameters for this application.
 */
int app_set_run_params(void)
{
	int ret = se_service_set_run_cfg(&current_runp);

	return ret;
}

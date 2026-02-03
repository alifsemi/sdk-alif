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
#include <cmsis_core.h>
#include <se_service.h>
#include <aipm.h>
#include "power_mgr.h"

#if DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(rtc0), snps_dw_apb_rtc, okay)
#define WAKEUP_SOURCE         DT_NODELABEL(rtc0)
#define SE_OFFP_EWIC_CFG      EWIC_RTC_A
#define SE_OFFP_WAKEUP_EVENTS WE_LPRTC
#elif DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(timer0), snps_dw_timers, okay)
#define WAKEUP_SOURCE         DT_NODELABEL(timer0)
#define SE_OFFP_EWIC_CFG      EWIC_VBAT_TIMER
#define SE_OFFP_WAKEUP_EVENTS WE_LPTIMER0
#else
#error "Wakeup Device not enabled in the dts"
#endif

#define WAKEUP_SOURCE_IRQ DT_IRQ_BY_IDX(WAKEUP_SOURCE, 0, irq)

LOG_MODULE_DECLARE(main, CONFIG_MAIN_LOG_LEVEL);

static const struct device *const wakeup_dev = DEVICE_DT_GET(WAKEUP_SOURCE);

/**
 * As per the application requirements, it can remove the memory blocks which are not in use.
 */
#define APP_RET_MEM_BLOCKS                                                                         \
	SRAM4_1_MASK | SRAM4_2_MASK | SRAM4_3_MASK | SRAM4_4_MASK | SRAM5_1_MASK | SRAM5_2_MASK |  \
		SRAM5_3_MASK | SRAM5_4_MASK | SRAM5_5_MASK
#define SERAM_MEMORY_BLOCKS_IN_USE SERAM_1_MASK | SERAM_2_MASK | SERAM_3_MASK | SERAM_4_MASK

run_profile_t current_runp __attribute__((noinit));
off_profile_t current_offp __attribute__((noinit));
int16_t power_profile __attribute__((noinit));

static bool cold_boot;
#define VBAT_RESUME_ENABLED 0xcafecafe

uint32_t vbat_resume __attribute__((noinit));

static void balletto_vbat_resume_enable(void)
{
	vbat_resume = VBAT_RESUME_ENABLED;
}

static bool balletto_vbat_resume_enabled(void)
{
	if (vbat_resume == VBAT_RESUME_ENABLED) {
		return true;
	}
	return false;
}

static int pm_application_init(void)
{
	if (!balletto_vbat_resume_enabled()) {
		/* Mark a cold boot */
		cold_boot = true;
	}

	return 0;
}
SYS_INIT(pm_application_init, PRE_KERNEL_1, 3);

int pwm_init(void)
{
	balletto_vbat_resume_enable();
	if (!device_is_ready(wakeup_dev)) {
		printk("%s: device not ready", wakeup_dev->name);
		return -1;
	}

	int ret = counter_start(wakeup_dev);

	if (ret && ret != -EALREADY) {
		printk("failed to start counter: %d", ret);
		return -1;
	}

	return 0;
}

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
	current_offp.ewic_cfg = SE_OFFP_EWIC_CFG;
	current_offp.wakeup_events = SE_OFFP_WAKEUP_EVENTS;
	current_offp.vtor_address = SCB->VTOR;
	current_offp.vtor_address_ns = SCB->VTOR;

	return set_current_off_profile();
}

/**
 * Set the RUN profile parameters for this application.
 */
int app_set_run_params(void)
{
	/**
	 * In case of the cold boot, set default values.
	 * Otherwise, current_runp should contain the correct values.
	 */
	if (is_cold_boot()) {
		current_runp.power_domains = PD_VBAT_AON_MASK | PD_SYST_MASK | PD_SSE700_AON_MASK;
		current_runp.power_domains |= PD_SESS_MASK | PD_DBSS_MASK;
		current_runp.dcdc_voltage = 775;
		current_runp.dcdc_mode = DCDC_MODE_PFM_FORCED;
		current_runp.aon_clk_src = CLK_SRC_LFXO;
		current_runp.run_clk_src = CLK_SRC_PLL;
		current_runp.cpu_clk_freq = CLOCK_FREQUENCY_160MHZ;
		current_runp.phy_pwr_gating = 0;
		current_runp.ip_clock_gating = 0;
		current_runp.vdd_ioflex_3V3 = IOFLEX_LEVEL_1V8;
		current_runp.scaled_clk_freq = SCALED_FREQ_RC_ACTIVE_76_8_MHZ;

		current_runp.memory_blocks = MRAM_MASK;
		current_runp.memory_blocks |= SERAM_MEMORY_BLOCKS_IN_USE;
		current_runp.memory_blocks |= APP_RET_MEM_BLOCKS;

		if (IS_ENABLED(CONFIG_MIPI_DSI)) {
			current_runp.phy_pwr_gating |= MIPI_TX_DPHY_MASK | MIPI_RX_DPHY_MASK;
			current_runp.phy_pwr_gating |= MIPI_PLL_DPHY_MASK;
			current_runp.ip_clock_gating |= CDC200_MASK | MIPI_DSI_MASK | GPU_MASK;
		}
	}

	int ret = se_service_set_run_cfg(&current_runp);

	return ret;
}

uint32_t get_wakeup_irq_status(void)
{
	return NVIC_GetPendingIRQ(WAKEUP_SOURCE_IRQ);
}

bool is_cold_boot(void)
{
	return cold_boot;
}

uint32_t get_current_ticks(void)
{
	uint32_t curr_ticks;
	int ret = counter_get_value(wakeup_dev, &curr_ticks);

	if (ret) {
		LOG_ERR("ERROR: %s ret = %d!\n", __func__, ret);
	}

	return curr_ticks;
}

uint32_t s_to_ticks(const uint32_t s)
{
	/* this might saturate the ticks (return UINT32T_MAX) if value would exceed uint32_t */
	return counter_us_to_ticks(wakeup_dev, s * 1000 * 1000);
}

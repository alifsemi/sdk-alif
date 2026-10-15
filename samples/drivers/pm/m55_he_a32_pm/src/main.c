/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https: //alifsemi.com/license
 *
 */

#include "aipm.h"
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/pm/pm.h>
#include <zephyr/pm/policy.h>
#include <zephyr/drivers/counter.h>
#include <se_service.h>
#include <zephyr/sys/util.h>

#include "a32_mhu.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(m55_he_a32_pm, LOG_LEVEL_INF);

#if !defined(CONFIG_ALIF_SE_DTS_RUN_PROFILE) || !defined(CONFIG_ALIF_SE_DTS_OFF_PROFILE)
/**
 * As per the application requirements, it can remove the memory blocks which are not in use.
 */
#if defined(CONFIG_SOC_SERIES_E1C) || defined(CONFIG_SOC_SERIES_B1)
	#define APP_RET_MEM_BLOCKS SRAM4_1_MASK | SRAM4_2_MASK | SRAM4_3_MASK | SRAM4_4_MASK | \
					SRAM5_1_MASK | SRAM5_2_MASK | SRAM5_3_MASK | SRAM5_4_MASK |\
					SRAM5_5_MASK
	#define SERAM_MEMORY_BLOCKS_IN_USE SERAM_1_MASK | SERAM_2_MASK | SERAM_3_MASK | SERAM_4_MASK
#else
	#define APP_RET_MEM_BLOCKS SRAM4_1_MASK | SRAM4_2_MASK | SRAM5_1_MASK | SRAM5_2_MASK
	#define SERAM_MEMORY_BLOCKS_IN_USE SERAM_MASK
#endif
#endif /* CONFIG_ALIF_SE_DTS_RUN_PROFILE || CONFIG_ALIF_SE_DTS_OFF_PROFILE */

#if !defined(CONFIG_ALIF_SE_DTS_OFF_PROFILE)
#if DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(rtc0), snps_dw_apb_rtc, okay)
	#define SE_OFFP_EWIC_CFG EWIC_RTC_A
	#define SE_OFFP_WAKEUP_EVENTS WE_LPRTC
#elif DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(timer0), snps_dw_timers, okay)
	#define SE_OFFP_EWIC_CFG EWIC_VBAT_TIMER
	#define SE_OFFP_WAKEUP_EVENTS WE_LPTIMER0
#endif
#endif /* CONFIG_ALIF_SE_DTS_OFF_PROFILE */

#if DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(rtc0), snps_dw_apb_rtc, okay)
	#define WAKEUP_SOURCE DT_NODELABEL(rtc0)
#elif DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(timer0), snps_dw_timers, okay)
	#define WAKEUP_SOURCE DT_NODELABEL(timer0)
#else
#error "Wakeup Device not enabled in the dts"
#endif

/* Sleep duration for PM_STATE_SUSPEND_TO_RAM substate 0 (STANDBY) */
#define S2RAM_STANDBY_SLEEP_USEC (20 * 1000 * 1000)

/*
 * MRAM base address - used to determine boot location
 * TCM boot: VTOR = 0x0
 * MRAM boot: VTOR >= 0x80000000
 */
#define MRAM_BASE_ADDRESS 0x80000000

#define IS_BOOTING_FROM_MRAM() (SCB->VTOR >= MRAM_BASE_ADDRESS)

/*
 * S2RAM is required after A32/PD9 is off so the HE off-profile can drop
 * es1_ppu. That path needs TCM retention (HE + TCM boot).
 */
#if defined(CONFIG_RTSS_HE)
#define S2RAM_SUPPORTED (!IS_BOOTING_FROM_MRAM())
#else
#define S2RAM_SUPPORTED 0
#endif

#if !defined(CONFIG_ALIF_SE_DTS_RUN_PROFILE)
/**
 * Set the RUN profile parameters for this application.
 */
static int app_set_run_params(void)
{
	run_profile_t runp;
	int ret;

	runp.power_domains = PD_SYST_MASK | PD_SSE700_AON_MASK;
	runp.dcdc_voltage  = 825;
	runp.dcdc_mode     = DCDC_MODE_PWM;
	runp.aon_clk_src   = CLK_SRC_LFXO;
	runp.run_clk_src   = CLK_SRC_PLL;
	runp.vdd_ioflex_3V3 = IOFLEX_LEVEL_1V8;
	runp.ip_clock_gating = 0;
	runp.phy_pwr_gating = 0;
#if defined(CONFIG_RTSS_HP)
	runp.cpu_clk_freq  = CLOCK_FREQUENCY_400MHZ;
#else
	runp.cpu_clk_freq  = CLOCK_FREQUENCY_160MHZ;
#endif

	runp.memory_blocks = MRAM_MASK;

	ret = se_service_set_run_cfg(&runp);
	__ASSERT(ret == 0, "SE: set_run_cfg failed = %d", ret);

	return ret;
}
/*
 * CRITICAL: Must run at PRE_KERNEL_1 to restore SYSTOP before peripherals initialize.
 *
 * Priority 46 ensures this runs:
 *   - AFTER SE Services (priority 45) - SE must be ready for set_run_cfg()
 *   - BEFORE Power Domain (priority 47) - Power domain needs SYSTOP enabled
 *   - BEFORE UART and peripherals (priority 50+) - Peripherals need SYSTOP ON
 *
 * On cold boot: SYSTOP is already ON by default, safe to call.
 * On SOFT_OFF wakeup: SYSTOP is OFF, must restore BEFORE peripherals access registers.
 */
SYS_INIT(app_set_run_params, PRE_KERNEL_1, 46);
#endif /* CONFIG_ALIF_SE_DTS_RUN_PROFILE */

#if !defined(CONFIG_ALIF_SE_DTS_OFF_PROFILE)
static int app_set_off_params(enum pm_state state, uint8_t substate_id)
{
	int ret;
	off_profile_t offp;

	offp.dcdc_voltage  = 825;
	offp.dcdc_mode     = DCDC_MODE_OFF;
	offp.stby_clk_freq = SCALED_FREQ_RC_STDBY_76_8_MHZ;
	offp.aon_clk_src   = CLK_SRC_LFXO;
	offp.stby_clk_src  = CLK_SRC_HFRC;
	offp.vtor_address  = SCB->VTOR;
	offp.ip_clock_gating = 0;
	offp.phy_pwr_gating = 0;
	offp.vdd_ioflex_3V3 = IOFLEX_LEVEL_1V8;
	offp.ewic_cfg      = SE_OFFP_EWIC_CFG;
	offp.wakeup_events = SE_OFFP_WAKEUP_EVENTS;
	offp.memory_blocks = MRAM_MASK;

#if defined(CONFIG_RTSS_HE)
	/*
	 * HE core retention configuration:
	 * - TCM boot (VTOR = 0): Enable TCM retention (SERAM + APP_RET_MEM_BLOCKS)
	 * - MRAM boot (VTOR >= 0x80000000): Only SERAM retention needed
	 */
	if (!IS_BOOTING_FROM_MRAM()) {
		offp.memory_blocks |= APP_RET_MEM_BLOCKS | SERAM_MEMORY_BLOCKS_IN_USE;
	} else {
		offp.memory_blocks |= SERAM_MEMORY_BLOCKS_IN_USE;
	}
#else
	__ASSERT(IS_BOOTING_FROM_MRAM(), "HP TCM Retention is not possible - VTOR is set to TCM");
#endif

	switch (state) {
	case PM_STATE_SUSPEND_TO_RAM:
		if (substate_id == 0) {
			offp.power_domains = PD_SSE700_AON_MASK;
		} else if (substate_id == 1) {
			offp.power_domains = PD_VBAT_AON_MASK;
		}
		break;
	case PM_STATE_SOFT_OFF:
		offp.memory_blocks = MRAM_MASK | SERAM_MEMORY_BLOCKS_IN_USE;
		offp.power_domains = PD_VBAT_AON_MASK;
		break;
	default:
		break;
	}

	ret = se_service_set_off_cfg(&offp);
	__ASSERT(ret == 0, "SE: set_off_cfg failed = %d", ret);

	return ret;
}

static void pm_notify_state_entry(enum pm_state state)
{
	const struct pm_state_info *next_state = pm_state_next_get(0);
	uint8_t substate_id = next_state ? next_state->substate_id : 0;
	int ret;

	switch (state) {
	case PM_STATE_SUSPEND_TO_IDLE:
		break;
	case PM_STATE_SUSPEND_TO_RAM:
	case PM_STATE_SOFT_OFF:
		ret = app_set_off_params(state, substate_id);
		__ASSERT(ret == 0, "app_set_off_params failed = %d", ret);
		break;
	default:
		__ASSERT(false, "Entering unknown power state %d", state);
		break;
	}
}
#endif /* CONFIG_ALIF_SE_DTS_OFF_PROFILE */

#if !defined(CONFIG_ALIF_SE_DTS_RUN_PROFILE)
static void pm_notify_pre_device_resume(enum pm_state state)
{
	int ret;

	switch (state) {
	case PM_STATE_SUSPEND_TO_RAM:
		ret = app_set_run_params();
		__ASSERT(ret == 0, "app_set_run_params failed = %d", ret);
		break;
	case PM_STATE_SUSPEND_TO_IDLE:
	case PM_STATE_SOFT_OFF:
		break;
	default:
		__ASSERT(false, "Pre-resume for unknown power state %d", state);
		break;
	}
}
#endif

#if !defined(CONFIG_ALIF_SE_DTS_RUN_PROFILE) || !defined(CONFIG_ALIF_SE_DTS_OFF_PROFILE)
static struct pm_notifier app_pm_notifier = {
#if !defined(CONFIG_ALIF_SE_DTS_OFF_PROFILE)
	.state_entry = pm_notify_state_entry,
#endif
#if !defined(CONFIG_ALIF_SE_DTS_RUN_PROFILE)
	.pre_device_resume = pm_notify_pre_device_resume,
#endif
};
#endif

static void app_pm_lock_deeper_states(bool lock)
{
#if defined(CONFIG_RTSS_HE)
	if (lock) {
		pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_RAM, PM_ALL_SUBSTATES);
		pm_policy_state_lock_get(PM_STATE_SOFT_OFF, PM_ALL_SUBSTATES);
	} else {
		/* Keep SOFT_OFF locked so idle selects S2RAM STANDBY. */
		pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_RAM, PM_ALL_SUBSTATES);
	}
#else
	#error "m55_he_a32_pm is an RTSS-HE sample"
#endif

	LOG_DBG("%s deeper power state(s)", lock ? "Locked" : "Unlocked");
}

static int app_pre_kernel_init(void)
{
	app_pm_lock_deeper_states(true);

#if !defined(CONFIG_ALIF_SE_DTS_RUN_PROFILE) || !defined(CONFIG_ALIF_SE_DTS_OFF_PROFILE)
	pm_notifier_register(&app_pm_notifier);
#endif

	return 0;
}
SYS_INIT(app_pre_kernel_init, PRE_KERNEL_2, 0);

static int app_enter_deep_sleep(uint32_t sleep_usec)
{
#if defined(CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER)
	k_sleep(K_USEC(sleep_usec));
#else
	const struct device *const wakeup_dev = DEVICE_DT_GET(WAKEUP_SOURCE);
	struct counter_alarm_cfg alarm_cfg;
	int ret;

	alarm_cfg.ticks = counter_us_to_ticks(wakeup_dev, sleep_usec);
	ret = counter_set_channel_alarm(wakeup_dev, 0, &alarm_cfg);
	if (ret) {
		LOG_ERR("Failed to set the alarm (err %d)", ret);
		return ret;
	}

	LOG_DBG("Set alarm for %u microseconds", sleep_usec);
	k_sleep(K_USEC(sleep_usec));
#endif

	return 0;
}

int main(void)
{
	const struct device *const cons = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
	const struct device *const wakeup_dev = DEVICE_DT_GET(WAKEUP_SOURCE);
	int ret;

	__ASSERT(device_is_ready(cons), "%s: device not ready", cons->name);
	__ASSERT(device_is_ready(wakeup_dev), "%s: device not ready", wakeup_dev->name);

	if (IS_BOOTING_FROM_MRAM()) {
		LOG_INF("%s RTSS_HE (MRAM boot): A32 off then HE S2RAM", CONFIG_BOARD);
	} else {
		LOG_INF("%s RTSS_HE (TCM boot): A32 off then HE S2RAM", CONFIG_BOARD);
	}

	ret = counter_start(wakeup_dev);
	__ASSERT(!ret || ret == -EALREADY, "Failed to start counter (err %d)", ret);

	if (a32_mhu_shutdown() != 0) {
		LOG_ERR("A32/PD9 shutdown failed");
		return -1;
	}

	if (!S2RAM_SUPPORTED) {
		LOG_ERR("S2RAM not supported (need HE TCM boot)");
		return -1;
	}

	/*
	 * Keep IWIC locked so the idle thread selects S2RAM STANDBY.
	 * sys_poweroff() skips PM notifiers, so SE would never apply the
	 * standby off-profile and es1_ppu would stay ON.
	 */
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
	app_pm_lock_deeper_states(false);

	LOG_INF("POWER STATE SEQUENCE:");
	LOG_INF("  1. A32/PD9 OFF");
	LOG_INF("  2. PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY)");
	LOG_INF("Enter PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) for (%d microseconds)",
		S2RAM_STANDBY_SLEEP_USEC);

	ret = app_enter_deep_sleep(S2RAM_STANDBY_SLEEP_USEC);
	__ASSERT(ret == 0, "Could not enter PM_STATE_SUSPEND_TO_RAM (err %d)", ret);

	LOG_INF("=== Resumed from PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) ===");

	app_pm_lock_deeper_states(true);

	while (true) {
		k_sleep(K_SECONDS(1));
	}

	return 0;
}

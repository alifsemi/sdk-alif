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
#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/pm/pm.h>
#include <zephyr/pm/policy.h>
#if defined(CONFIG_POWEROFF)
#include <zephyr/sys/poweroff.h>
#endif
#include <zephyr/drivers/counter.h>
#include <se_service.h>
#include <zephyr/sys/util.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(pm_system_off, LOG_LEVEL_INF);

/**
 * As per the application requirements, it can remove the memory blocks which are not in use.
 */
#if defined(CONFIG_SOC_SERIES_ENSEMBLE_E1C) || defined(CONFIG_SOC_SERIES_BALLETTO_B1)
	#define APP_RET_MEM_BLOCKS SRAM4_1_MASK | SRAM4_2_MASK | SRAM4_3_MASK | SRAM4_4_MASK | \
					SRAM5_1_MASK | SRAM5_2_MASK | SRAM5_3_MASK | SRAM5_4_MASK |\
					SRAM5_5_MASK
	#define SERAM_MEMORY_BLOCKS_IN_USE SERAM_1_MASK | SERAM_2_MASK | SERAM_3_MASK | SERAM_4_MASK
#else
	#define APP_RET_MEM_BLOCKS SRAM4_1_MASK | SRAM4_2_MASK | SRAM5_1_MASK | SRAM5_2_MASK
	#define SERAM_MEMORY_BLOCKS_IN_USE SERAM_MASK
#endif

#if DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(rtc0), snps_dw_apb_rtc, okay)
	#define WAKEUP_SOURCE DT_NODELABEL(rtc0)
	#define SE_OFFP_EWIC_CFG EWIC_RTC_A
	#define SE_OFFP_WAKEUP_EVENTS WE_LPRTC
#elif DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(timer0), snps_dw_timers, okay)
	#define WAKEUP_SOURCE DT_NODELABEL(timer0)
	#define SE_OFFP_EWIC_CFG EWIC_VBAT_TIMER
	#define SE_OFFP_WAKEUP_EVENTS WE_LPTIMER0
#else
#error "Wakeup Device not enabled in the dts"
#endif

/* Sleep duration for PM_STATE_RUNTIME_IDLE */
#define RUNTIME_IDLE_SLEEP_USEC (18 * 1000 * 1000)
/* Sleep duration for PM_STATE_SUSPEND_TO_RAM substate 0 (STANDBY) */
#define S2RAM_STANDBY_SLEEP_USEC (20 * 1000 * 1000)
/* Sleep duration for PM_STATE_SUSPEND_TO_RAM substate 1 (STOP) */
#define S2RAM_STOP_SLEEP_USEC (22 * 1000 * 1000)
/* Sleep duration for PM_STATE_SOFT_OFF */
#define SOFT_OFF_SLEEP_USEC (24 * 1000 * 1000)
/* Wakeup duration for sys_poweroff (permanent power off) */
#define POWEROFF_WAKEUP_USEC (30 * 1000 * 1000)

/*
 * MRAM base address - used to determine boot location
 * TCM boot: VTOR = 0x0
 * MRAM boot: VTOR >= 0x80000000
 */
#define MRAM_BASE_ADDRESS 0x80000000

/*
 * Helper macro to check if booting from MRAM
 */
#define IS_BOOTING_FROM_MRAM() (SCB->VTOR >= MRAM_BASE_ADDRESS)

/*
 * PM_STATE_SUSPEND_TO_RAM (S2RAM) support:
 * - HP core: NOT supported (no retention capability)
 * - HE core + TCM boot: SUPPORTED (TCM retention keeps code and context)
 */
#if defined(CONFIG_RTSS_HE)
#define S2RAM_SUPPORTED (!IS_BOOTING_FROM_MRAM())
#else
#define S2RAM_SUPPORTED 0
#endif

/*
 * PM_STATE_SOFT_OFF support:
 * - HP core: Always supported (no retention, must use SOFT_OFF)
 * - HE core + MRAM boot: Supported (MRAM preserved, wakeup possible)
 * - HE core + TCM boot: Skip (use S2RAM with retention instead)
 */
#if defined(CONFIG_RTSS_HP)
#define SOFT_OFF_SUPPORTED 1
#elif defined(CONFIG_RTSS_HE)
#define SOFT_OFF_SUPPORTED IS_BOOTING_FROM_MRAM()
#else
#define SOFT_OFF_SUPPORTED 0
#endif

#define OFF_STATE_NODE_ID DT_PHANDLE_BY_IDX(DT_NODELABEL(cpu0), cpu_power_states, 0)

BUILD_ASSERT((RUNTIME_IDLE_SLEEP_USEC < DT_PROP_OR(OFF_STATE_NODE_ID, min_residency_us, 0)),
	"RUNTIME_IDLE sleep should be less than min-residency-us");

#if defined(CONFIG_RTSS_HE)
/* Additional validation for power state sleep durations */
BUILD_ASSERT((S2RAM_STOP_SLEEP_USEC > S2RAM_STANDBY_SLEEP_USEC),
	"STOP sleep duration should be greater than STANDBY sleep duration");
BUILD_ASSERT((SOFT_OFF_SLEEP_USEC > S2RAM_STOP_SLEEP_USEC),
	"SOFT_OFF sleep duration should be greater than STOP sleep duration");
#endif

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
	if (ret) {
		__ASSERT(false, "SE: set_run_cfg failed = %d", ret);
	}

	return 0;
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
		/* TCM boot: enable full retention including TCM memory blocks */
		offp.memory_blocks |= APP_RET_MEM_BLOCKS | SERAM_MEMORY_BLOCKS_IN_USE;
	} else {
		/* MRAM boot */
		offp.memory_blocks |= SERAM_MEMORY_BLOCKS_IN_USE;
	}
#else
	/*
	 * HP core: Retention is not possible with HP-TCM
	 */
	if (!IS_BOOTING_FROM_MRAM()) {
		__ASSERT(false, "HP TCM Retention is not possible - VTOR is set to TCM");
	}
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
	if (ret) {
		__ASSERT(false, "SE: set_off_cfg failed = %d", ret);
		return ret;
	}

	return 0;
}

/**
 * PM Notifier callback for power state entry
 */
static void pm_notify_state_entry(enum pm_state state)
{
	const struct pm_state_info *next_state = pm_state_next_get(0);
	uint8_t substate_id = next_state ? next_state->substate_id : 0;
	int ret;

	switch (state) {
	case PM_STATE_SUSPEND_TO_RAM:
	case PM_STATE_SOFT_OFF:
		ret = app_set_off_params(state, substate_id);
		if (ret) {
			__ASSERT(false, "app_set_off_params failed = %d", ret);
		}
		break;
	default:
		__ASSERT(false, "Entering unknown power state %d", state);
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
static void pm_notify_pre_device_resume(enum pm_state state)
{
	int ret;

	switch (state) {
	case PM_STATE_SUSPEND_TO_RAM:
		ret = app_set_run_params();
		if (ret) {
			__ASSERT(false, "app_set_run_params failed = %d", ret);
		}
		break;
	case PM_STATE_SOFT_OFF:
		/* No action needed - SOFT_OFF causes reset, not resume */
		break;
	default:
		__ASSERT(false, "Pre-resume for unknown power state %d", state);
		break;
	}
}

/**
 * PM Notifier structure
 */
static struct pm_notifier app_pm_notifier = {
	.state_entry = pm_notify_state_entry,
	.pre_device_resume = pm_notify_pre_device_resume,
};

/**
 * Helper function to lock/unlock deeper power states
 * @param lock true to lock deeper states (allow only RUNTIME_IDLE), false to unlock all
 */
static void app_pm_lock_deeper_states(bool lock)
{
	const char *state_desc;

#if defined(CONFIG_RTSS_HP)
	/* HP core: only SOFT_OFF (no S2RAM support) */
	enum pm_state deep_states[] = {
		PM_STATE_SOFT_OFF
	};
	state_desc = "SOFT_OFF";

	for (int i = 0; i < ARRAY_SIZE(deep_states); i++) {
		if (lock) {
			pm_policy_state_lock_get(deep_states[i], PM_ALL_SUBSTATES);
		} else {
			pm_policy_state_lock_put(deep_states[i], PM_ALL_SUBSTATES);
		}
	}

#elif defined(CONFIG_RTSS_HE)
	/*
	 * HE core: States depend on boot location
	 * - TCM boot: S2RAM only (SOFT_OFF not needed with retention)
	 * - MRAM boot: SOFT_OFF only
	 */
	enum pm_state deep_states[2];
	int num_states = 0;

	if (S2RAM_SUPPORTED) {
		/* TCM boot: S2RAM works with retention */
		deep_states[num_states++] = PM_STATE_SUSPEND_TO_RAM;
		state_desc = "S2RAM";
	}

	if (SOFT_OFF_SUPPORTED) {
		/* MRAM boot: SOFT_OFF is the only deep sleep option for now */
		deep_states[num_states++] = PM_STATE_SOFT_OFF;
		state_desc = "SOFT_OFF";
	}

	for (int i = 0; i < num_states; i++) {
		if (lock) {
			pm_policy_state_lock_get(deep_states[i], PM_ALL_SUBSTATES);
		} else {
			pm_policy_state_lock_put(deep_states[i], PM_ALL_SUBSTATES);
		}
	}

#else
	#error "Unknown core type"
#endif

	LOG_DBG("%s deeper power state(s) (%s)",
	       lock ? "Locked" : "Unlocked", state_desc);
}

/*
 * This function will be invoked in the PRE_KERNEL_2 phase of the init routine.
 */
static int app_pre_kernel_init(void)
{
	/* Lock deeper power states to allow only RUNTIME_IDLE */
	app_pm_lock_deeper_states(true);

	/* Register PM notifier callbacks */
	pm_notifier_register(&app_pm_notifier);

	return 0;
}
SYS_INIT(app_pre_kernel_init, PRE_KERNEL_2, 0);

#if !defined(CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER)
static volatile uint32_t alarm_cb_status;
static void alarm_callback_fn(const struct device *wakeup_dev,
				uint8_t chan_id, uint32_t ticks,
				void *user_data)
{
	LOG_DBG("%s: Alarm triggered", wakeup_dev->name);
	alarm_cb_status = 1;
}
#endif

static int app_enter_normal_sleep(uint32_t sleep_usec)
{
#if defined(CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER)
	k_sleep(K_USEC(sleep_usec));
#else
	const struct device *const wakeup_dev = DEVICE_DT_GET(WAKEUP_SOURCE);
	struct counter_alarm_cfg alarm_cfg;
	int ret;

	alarm_cfg.flags = 0;
	alarm_cfg.ticks = counter_us_to_ticks(wakeup_dev, sleep_usec);
	alarm_cfg.callback = alarm_callback_fn;
	alarm_cfg.user_data = &alarm_cfg;

	ret = counter_set_channel_alarm(wakeup_dev, 0, &alarm_cfg);
	if (ret) {
		LOG_ERR("Could not set the alarm");
		return ret;
	}
	LOG_DBG("Set alarm for %u microseconds", sleep_usec);

	k_sleep(K_USEC(sleep_usec));

	if (!alarm_cb_status) {
		return -1;
	}
	alarm_cb_status = 0;


#endif
	return 0;
}

#if !defined(CONFIG_POWEROFF)
static int app_enter_deep_sleep(uint32_t sleep_usec)
{
#if defined(CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER)
	/**
	 * Set a delay more than the min-residency-us configured so that
	 * the sub-system will go to OFF state.
	 */
	k_sleep(K_USEC(sleep_usec));
#else
	const struct device *const wakeup_dev = DEVICE_DT_GET(WAKEUP_SOURCE);
	struct counter_alarm_cfg alarm_cfg;
	int ret;
	/*
	 * Set the alarm and delay so that idle thread can run
	 */
	alarm_cfg.ticks = counter_us_to_ticks(wakeup_dev, sleep_usec);
	ret = counter_set_channel_alarm(wakeup_dev, 0, &alarm_cfg);
	if (ret) {
		LOG_ERR("Failed to set the alarm (err %d)", ret);
		return ret;
	}

	LOG_DBG("Set alarm for %u microseconds", sleep_usec);
	/*
	 * Wait for the alarm to trigger. The idle thread will
	 * take care of entering the deep sleep state via PM framework.
	 */
	k_sleep(K_USEC(sleep_usec));
#endif

	return 0;
}
#endif /* !CONFIG_POWEROFF */

int main(void)
{
	const struct device *const cons = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
	const struct device *const wakeup_dev = DEVICE_DT_GET(WAKEUP_SOURCE);
	int ret;

	if (!device_is_ready(cons)) {
		__ASSERT(false, "%s: device not ready", cons->name);
	}

	if (!device_is_ready(wakeup_dev)) {
		__ASSERT(false, "%s: device not ready", wakeup_dev->name);
	}

#if defined(CONFIG_RTSS_HE)
	/* Boot location determines which PM states are available */
	bool is_mram_boot = IS_BOOTING_FROM_MRAM();

	if (is_mram_boot) {
		LOG_INF("\n%s RTSS_HE (MRAM boot): PM states demo (RUNTIME_IDLE, SOFT_OFF)",
			CONFIG_BOARD);
	} else {
		LOG_INF("\n%s RTSS_HE (TCM boot): PM states demo (RUNTIME_IDLE, S2RAM)",
			CONFIG_BOARD);
	}
#else
	LOG_INF("\n%s RTSS_HP: PM states demo (RUNTIME_IDLE, SOFT_OFF)", CONFIG_BOARD);
#endif

	ret = counter_start(wakeup_dev);
	if (ret && ret != -EALREADY) {
		__ASSERT(false, "Failed to start counter (err %d)", ret);
	}

	LOG_INF("POWER STATE SEQUENCE:");
#if defined(CONFIG_POWEROFF)
	LOG_INF("  1. PM_STATE_RUNTIME_IDLE");
	LOG_INF("  2. Power off (sys_poweroff)");
#elif defined(CONFIG_RTSS_HE)
	/* HE core: sequence depends on boot location */
	LOG_INF("  1. PM_STATE_RUNTIME_IDLE");
	if (!is_mram_boot) {
		/* TCM boot: S2RAM works (TCM retention) */
		LOG_INF("  2. PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY)");
		LOG_INF("  3. PM_STATE_SUSPEND_TO_RAM (substate 1: STOP)");
		LOG_INF("  4. (SOFT_OFF skipped - TCM boot, using retention)");
	} else {
		/* MRAM boot: Only SOFT_OFF (S2RAM disabled - TODO: investigate) */
		LOG_INF("  2. (S2RAM skipped - MRAM boot, not supported yet)");
		LOG_INF("  3. PM_STATE_SOFT_OFF");
	}
#else
	/* HP core: no retention, only SOFT_OFF supported */
	LOG_INF("  1. PM_STATE_RUNTIME_IDLE");
	LOG_INF("  2. PM_STATE_SOFT_OFF");
#endif

	LOG_INF("Enter RUNTIME_IDLE sleep for (%d microseconds)", RUNTIME_IDLE_SLEEP_USEC);
	ret = app_enter_normal_sleep(RUNTIME_IDLE_SLEEP_USEC);
	if (ret) {
		__ASSERT(false, "Could not enter RUNTIME_IDLE sleep (err %d)", ret);
	}

	LOG_INF("Exited from RUNTIME_IDLE sleep");

#if defined(CONFIG_POWEROFF)
	/* Configure wakeup source for permanent power off */
	struct counter_alarm_cfg alarm_cfg;

	LOG_INF("=== Enter (sys_poweroff) ===");
	LOG_INF("System will power off and can only wake via external event (RTC/Timer)");
	k_sleep(K_SECONDS(2));

	/* Set alarm for wakeup from power off */
	alarm_cfg.ticks = counter_us_to_ticks(wakeup_dev, POWEROFF_WAKEUP_USEC);
	ret = counter_set_channel_alarm(wakeup_dev, 0, &alarm_cfg);
	if (ret) {
		LOG_ERR("Failed to set wakeup alarm (err %d)", ret);
	} else {
		LOG_INF("Wakeup alarm set for %u seconds", POWEROFF_WAKEUP_USEC / 1000000);
	}

	/* Configure OFF profile for wakeup capability */
	app_set_off_params(PM_STATE_SOFT_OFF, 0);

	LOG_INF("Calling sys_poweroff() - system will power off permanently");
	sys_poweroff();

	/* Should never reach here */
	LOG_ERR("Failed to execute sys_poweroff()");

	return -1;
#else
	/* Unlock deeper power states to allow S2RAM and/or SOFT_OFF */
	app_pm_lock_deeper_states(false);

#if defined(CONFIG_RTSS_HE)
	/* HE core: S2RAM only if booting from TCM */
	if (S2RAM_SUPPORTED) {
		LOG_INF("Enter PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) for (%d microseconds)",
			S2RAM_STANDBY_SLEEP_USEC);
		ret = app_enter_deep_sleep(S2RAM_STANDBY_SLEEP_USEC);
		if (ret) {
			__ASSERT(false, "Could not enter PM_STATE_SUSPEND_TO_RAM (err %d)", ret);
		}

		LOG_INF("=== Resumed from PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) ===");

		/* Verify main thread is running properly */
		for (int i = 0; i < 3; i++) {
			LOG_INF("Main thread running - iteration %d - tick: %llu",
				i, k_uptime_ticks());
			k_sleep(K_SECONDS(2));
		}

		LOG_INF("Enter PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) for (%d microseconds)",
			S2RAM_STOP_SLEEP_USEC);
		ret = app_enter_deep_sleep(S2RAM_STOP_SLEEP_USEC);
		if (ret) {
			__ASSERT(false, "Could not enter PM_STATE_SUSPEND_TO_RAM (err %d)", ret);
		}

		LOG_INF("=== Resumed from PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) ===");

		/* Verify main thread is running properly */
		for (int i = 0; i < 3; i++) {
			LOG_INF("Main thread running - iteration %d - tick: %llu",
				i, k_uptime_ticks());
			k_sleep(K_SECONDS(2));
		}
	} else {
		LOG_INF("Skipping PM_STATE_SUSPEND_TO_RAM (MRAM boot - not supported yet)");
	}
#endif /* CONFIG_RTSS_HE */

	/* PM_STATE_SOFT_OFF (deepest sleep with wake capability) */
#if defined(CONFIG_RTSS_HP)
	/* HP core: always SOFT_OFF */
	LOG_INF("Enter PM_STATE_SOFT_OFF for (%d microseconds)", SOFT_OFF_SLEEP_USEC);
	LOG_INF("Note: SOFT_OFF has no retention - system will reset on wakeup");
	ret = app_enter_deep_sleep(SOFT_OFF_SLEEP_USEC);
	if (ret) {
		__ASSERT(false, "Could not enter PM_STATE_SOFT_OFF (err %d)", ret);
	}

	/* Should never reach here - SOFT_OFF causes full reset on wakeup */
	LOG_ERR("ERROR: Resumed after PM_STATE_SOFT_OFF - this should not happen!");
	__ASSERT(false, "PM_STATE_SOFT_OFF should have caused a reset");

#elif defined(CONFIG_RTSS_HE)
	/* HE core: only SOFT_OFF when booting from MRAM */
	if (SOFT_OFF_SUPPORTED) {
		LOG_INF("Enter PM_STATE_SOFT_OFF for (%d microseconds)", SOFT_OFF_SLEEP_USEC);
		LOG_INF("Note: SOFT_OFF has no retention - system will reset on wakeup");
		ret = app_enter_deep_sleep(SOFT_OFF_SLEEP_USEC);
		if (ret) {
			__ASSERT(false, "Could not enter PM_STATE_SOFT_OFF (err %d)", ret);
		}

		/* Should never reach here - SOFT_OFF causes full reset on wakeup */
		LOG_ERR("ERROR: Resumed after PM_STATE_SOFT_OFF - this should not happen!");
		__ASSERT(false, "PM_STATE_SOFT_OFF should have caused a reset");
	} else {
		LOG_INF("Skipping PM_STATE_SOFT_OFF (TCM boot, using retention instead)");
	}
#endif

	LOG_INF("=== POWER STATE SEQUENCE COMPLETED ===");

	app_pm_lock_deeper_states(true);

	while (true) {
		/* spin here */
		k_sleep(K_SECONDS(1));
	}
#endif

	return 0;
}

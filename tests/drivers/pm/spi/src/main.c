/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/*
 * Power Management (PM) state transition tests for Alif RTSS cores.
 *
 * This file uses Zephyr's ztest framework to exercise RUNTIME_IDLE,
 * SUSPEND_TO_IDLE, SUSPEND_TO_RAM (STANDBY/STOP), and SOFT_OFF states.
 * When SPI_PM_TEST is defined, SPI transactions run in a background thread
 * and are suspended/resumed around each power state entry/exit.
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

#include <zephyr/ztest.h>

LOG_MODULE_REGISTER(spi_pm, LOG_LEVEL_DBG);

#define SPI_PM_TEST

#ifdef SPI_PM_TEST
#include "spi_test.h"
#endif

/**
 * As per the application requirements, it can remove the memory blocks which are not in use.
 */
#if defined(CONFIG_SOC_SERIES_E1C) || defined(CONFIG_SOC_SERIES_B1)
#define APP_RET_MEM_BLOCKS                                                                         \
	(SRAM4_1_MASK | SRAM4_2_MASK | SRAM4_3_MASK | SRAM4_4_MASK | SRAM5_1_MASK | SRAM5_2_MASK | \
	 SRAM5_3_MASK | SRAM5_4_MASK | SRAM5_5_MASK)
#define SERAM_MEMORY_BLOCKS_IN_USE (SERAM_1_MASK | SERAM_2_MASK | SERAM_3_MASK | SERAM_4_MASK)
#else
#define APP_RET_MEM_BLOCKS         (SRAM4_1_MASK | SRAM4_2_MASK | SRAM5_1_MASK | SRAM5_2_MASK)
#define SERAM_MEMORY_BLOCKS_IN_USE SERAM_MASK
#endif

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

/* Sleep duration for PM_STATE_RUNTIME_IDLE */
#define RUNTIME_IDLE_SLEEP_USEC  (10 * 1000 * 1000)
/* Sleep duration for PM_STATE_SUSPEND_TO_IDLE */
#define SUSPEND_IDLE_SLEEP_USEC  (4 * 1000)
/* Sleep duration for PM_STATE_SUSPEND_TO_RAM substate 0 (STANDBY) */
#define S2RAM_STANDBY_SLEEP_USEC (20 * 1000 * 1000)
/* Sleep duration for PM_STATE_SUSPEND_TO_RAM substate 1 (STOP) */
#define S2RAM_STOP_SLEEP_USEC    (22 * 1000 * 1000)
/* Sleep duration for PM_STATE_SOFT_OFF */
#define SOFT_OFF_SLEEP_USEC      (26 * 1000 * 1000)
/* Wakeup duration for sys_poweroff (permanent power off) */
#define POWEROFF_WAKEUP_USEC     (30 * 1000 * 1000)

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
	runp.dcdc_voltage = 825;
	runp.dcdc_mode = DCDC_MODE_PWM;
	runp.aon_clk_src = CLK_SRC_LFXO;
	runp.run_clk_src = CLK_SRC_PLL;
	runp.vdd_ioflex_3V3 = IOFLEX_LEVEL_1V8;
	runp.ip_clock_gating = 0;
	runp.phy_pwr_gating = 0;
#if defined(CONFIG_RTSS_HP)
	runp.cpu_clk_freq = CLOCK_FREQUENCY_400MHZ;
#else
	runp.cpu_clk_freq = CLOCK_FREQUENCY_160MHZ;
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

static int app_set_off_params(enum pm_state state, uint8_t substate_id)
{
	int ret;
	off_profile_t offp;

	offp.dcdc_voltage = 825;
	offp.dcdc_mode = DCDC_MODE_OFF;
	offp.stby_clk_freq = SCALED_FREQ_RC_STDBY_76_8_MHZ;
	offp.aon_clk_src = CLK_SRC_LFXO;
	offp.stby_clk_src = CLK_SRC_HFRC;
	offp.vtor_address = SCB->VTOR;
	offp.ip_clock_gating = 0;
	offp.phy_pwr_gating = 0;
	offp.vdd_ioflex_3V3 = IOFLEX_LEVEL_1V8;
	offp.ewic_cfg = SE_OFFP_EWIC_CFG;
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

/**
 * PM Notifier callback for power state entry
 */
static void pm_notify_state_entry(enum pm_state state)
{
	const struct pm_state_info *next_state = pm_state_next_get(0);
	uint8_t substate_id = next_state ? next_state->substate_id : 0;
	int ret;

	switch (state) {
	case PM_STATE_SUSPEND_TO_IDLE:
		/* No action needed */
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
		__ASSERT(ret == 0, "app_set_run_params failed = %d", ret);
		break;
	case PM_STATE_SUSPEND_TO_IDLE:
		/* No action needed - IWIC keeps power, no restoration required */
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
	enum pm_state deep_states[] = {PM_STATE_SOFT_OFF};

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
	 * - MRAM boot: SOFT_OFF only (Keep S2RAM locked)
	 */
	enum pm_state deep_states[2];
	int num_states = 0;

	if (S2RAM_SUPPORTED) {
		/* TCM boot: S2RAM works with retention */
		deep_states[num_states++] = PM_STATE_SUSPEND_TO_RAM;
		state_desc = "S2RAM";
	} else {
		/* MRAM boot: Keep S2RAM locked so SOFT_OFF is selected */
		if (!lock) {
			/* Ensure S2RAM stays locked when unlocking SOFT_OFF */
			pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_RAM, PM_ALL_SUBSTATES);
		}
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

	LOG_DBG("%s deeper power state(s) (%s)", lock ? "Locked" : "Unlocked", state_desc);
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
static void alarm_callback_fn(const struct device *wakeup_dev, uint8_t chan_id, uint32_t ticks,
			      void *user_data)
{
	LOG_DBG("%s: Alarm triggered", wakeup_dev->name);
	alarm_cb_status = 1;
}
#endif

static int app_enter_normal_sleep(uint32_t sleep_usec)
{
#ifdef SPI_PM_TEST
	spi_pm_thread_suspend();
	LOG_INF("====== SPI Transactions are Suspended");
#endif

#if defined(CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER)
	k_sleep(K_USEC(sleep_usec));
#else
	const struct device *const wakeup_dev = DEVICE_DT_GET(WAKEUP_SOURCE);
	struct counter_alarm_cfg alarm_cfg = {0};
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
#ifdef SPI_PM_TEST
	spi_pm_thread_suspend();
	LOG_INF("==SPI Transactions are Suspended: for Deep Sleep");
#endif

#if defined(CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER)
	/**
	 * Set a delay more than the min-residency-us configured so that
	 * the sub-system will go to OFF state.
	 */

	/* Wait for UART to finish transmitting logs */
	k_msleep(100);

	k_sleep(K_USEC(sleep_usec));
#else

	const struct device *const wakeup_dev = DEVICE_DT_GET(WAKEUP_SOURCE);
	struct counter_alarm_cfg alarm_cfg = {0};
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

#ifdef SPI_PM_TEST
	spi_pm_thread_resume();
	LOG_INF("==SPI Transactions are Resumed: for Deep Sleep");
#endif

	return 0;
}
#endif /* !CONFIG_POWEROFF */

/*===============================================================================================*/

/*
 * Suite-level setup: called once before all test suites run.
 * Logs the active core/boot mode and starts the SPI background thread
 * when SPI_PM_TEST is enabled.
 */
void *setup(void)
{
#if defined(CONFIG_RTSS_HE)
	/* Boot location determines which PM states are available */
	bool is_mram_boot = IS_BOOTING_FROM_MRAM();

	if (is_mram_boot) {
		LOG_INF("\n%s RTSS_HE (MRAM boot): PM states demo "
			"(RUNTIME_IDLE, SUSPEND_TO_IDLE, SOFT_OFF)",
			CONFIG_BOARD);
	} else {
		LOG_INF("\n%s RTSS_HE (TCM boot): PM states demo "
			"(RUNTIME_IDLE, SUSPEND_TO_IDLE, S2RAM)",
			CONFIG_BOARD);
	}

#ifdef SPI_PM_TEST
	LOG_INF("RTSS_HE: PM states demo WITH SPI Transaction");
	spi_pm_thread_init();
	spi_pm_thread_start();
#endif

#else

	LOG_INF("%s RTSS_HP: PM states demo "
		"(RUNTIME_IDLE, SUSPEND_TO_IDLE, SOFT_OFF)",
		CONFIG_BOARD);

#ifdef SPI_PM_TEST
	LOG_INF("RTSS_HP: PM states demo WITH SPI Transaction");
	spi_pm_thread_init();
	spi_pm_thread_start();
#endif

#endif

	return NULL;
}

/*
 * Per-test setup: called before each ztest case.
 * Verifies console and wakeup counter devices are ready, starts the
 * counter, and resumes the SPI thread if it was suspended.
 */
void before(void *fixture)
{

	ARG_UNUSED(fixture);
	const struct device *const cons = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
	const struct device *const wakeup_dev = DEVICE_DT_GET(WAKEUP_SOURCE);

	__ASSERT(device_is_ready(cons), "%s: device not ready", cons->name);
	__ASSERT(device_is_ready(wakeup_dev), "%s: device not ready", wakeup_dev->name);

	int ret = counter_start(wakeup_dev);

	__ASSERT(!ret || ret == -EALREADY, "Failed to start counter (err %d)", ret);

	if (spi_pm_thread_is_started() && spi_pm_thread_is_suspended()) {
		spi_pm_thread_resume();
		k_sleep(K_MSEC(50));
	}
}

/*
 * Per-test teardown: called after each ztest case.
 * Suspends the SPI background thread while SPI_PM_TEST is enabled.
 */
static void after(void *fixture)
{
	ARG_UNUSED(fixture);

#ifdef SPI_PM_TEST
	if (spi_pm_thread_is_started() && !spi_pm_thread_is_suspended()) {
		spi_pm_thread_suspend();
		LOG_INF("====== SPI Threads Suspended in %s()", __func__);
	}
#endif
}

/*
 * Suite-level teardown: called once after all test suites finish.
 * Ensures the SPI background thread is left suspended.
 */
static void teardown(void *fixture)
{
	ARG_UNUSED(fixture);

#ifdef SPI_PM_TEST
	if (spi_pm_thread_is_started() && !spi_pm_thread_is_suspended()) {
		spi_pm_thread_suspend();
	}
	LOG_INF("SPI thread suspending after test suite");
#endif
}

/* FIXME: Make this local to each test function instead of a shared global. */
int ret;

ZTEST(pm_POS_01_Runtime_idle_testing_for_SPI, test_SPI_POS_01_runtime_idle_basic_test)
{

	LOG_INF("=== TEST 1: Basic RUNTIME_IDLE ===");

	/* Lock SUSPEND_IDLE to force PM policy to select RUNTIME_IDLE only */
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);

#ifdef SPI_PM_TEST
	/* let SPI do some transactions. */
	k_sleep(K_SECONDS(5));
#endif

	LOG_INF("Enter RUNTIME_IDLE sleep for (%d microseconds)", RUNTIME_IDLE_SLEEP_USEC);
	ret = app_enter_normal_sleep(RUNTIME_IDLE_SLEEP_USEC);
	__ASSERT(ret == 0, "Could not enter RUNTIME_IDLE sleep (err %d)", ret);

	LOG_INF("Exited from RUNTIME_IDLE sleep");

#ifdef SPI_PM_TEST
	spi_pm_thread_resume();
	LOG_INF(" ====== SPI Transactions are starting...");
	k_sleep(K_SECONDS(2));
#endif

	pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
}

ZTEST(pm_POS_01_Runtime_idle_testing_for_SPI, test_SPI_POS_02_runtime_idle_multiple_cycles_test)
{

#define NUM_CYCLES 3

	LOG_INF("=== TEST 2: Multiple RUNTIME_IDLE Cycles (%d) ===", NUM_CYCLES);

	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);

	for (int cycle = 0; cycle < NUM_CYCLES; cycle++) {

		LOG_INF("--- Cycle %d of %d ---", cycle + 1, NUM_CYCLES);

#ifdef SPI_PM_TEST
		/* let SPI do some transactions. */
		k_sleep(K_SECONDS(5));
#endif

		LOG_INF("Enter RUNTIME_IDLE sleep for (%d microseconds)", RUNTIME_IDLE_SLEEP_USEC);
		ret = app_enter_normal_sleep(RUNTIME_IDLE_SLEEP_USEC);
		__ASSERT(ret == 0, "Could not enter RUNTIME_IDLE sleep (err %d)", ret);

		LOG_INF("Exited from RUNTIME_IDLE sleep");

#ifdef SPI_PM_TEST
		spi_pm_thread_resume();
		LOG_INF(" ====== SPI Transactions are starting...");
		k_sleep(K_SECONDS(2));
#endif
	}

	pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
}

ZTEST(pm_POS_01_Runtime_idle_testing_for_SPI, test_SPI_POS_03_runtime_idle_min_sleep)
{

#define MIN_SLEEP_USEC (1 * 1000 * 1000)

	LOG_INF("=== TEST 3: RUNTIME_IDLE Minimum Sleep (%d microseconds) ===", MIN_SLEEP_USEC);

	/* Lock SUSPEND_IDLE to force PM policy to select RUNTIME_IDLE only */
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);

#ifdef SPI_PM_TEST
	/* let SPI do some transactions. */
	k_sleep(K_SECONDS(5));
#endif

	LOG_INF("Enter RUNTIME_IDLE sleep for (%d microseconds)", MIN_SLEEP_USEC);
	ret = app_enter_normal_sleep(MIN_SLEEP_USEC);
	__ASSERT(ret == 0, "Could not enter RUNTIME_IDLE sleep (err %d)", ret);

	LOG_INF("Exited from RUNTIME_IDLE sleep");

#ifdef SPI_PM_TEST
	spi_pm_thread_resume();
	LOG_INF(" ====== SPI Transactions are starting...");
	k_sleep(K_SECONDS(2));
#endif

	pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
}

/* ============================================================
 * TEST 4: RUNTIME_IDLE maximum sleep duration
 * Sleep = 18 seconds (just below S2RAM min-residency 19s)
 * From DTS: standby min-residency-us = <19000000>
 * ============================================================
 */

ZTEST(pm_POS_01_Runtime_idle_testing_for_SPI, test_SPI_POS_04_runtime_idle_max_sleep)
{

#define MAX_SLEEP_USEC (18 * 1000 * 1000)

	LOG_INF("=== TEST 4: RUNTIME_IDLE Maximum Sleep ((%d microseconds)) ===", MAX_SLEEP_USEC);

	/* Lock SUSPEND_IDLE to force PM policy to select RUNTIME_IDLE only */
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);

#ifdef SPI_PM_TEST
	/* let SPI do some transactions. */
	k_sleep(K_SECONDS(5));
#endif

	LOG_INF("Enter RUNTIME_IDLE sleep for (%d microseconds)", MAX_SLEEP_USEC);
	ret = app_enter_normal_sleep(MAX_SLEEP_USEC);
	__ASSERT(ret == 0, "Could not enter RUNTIME_IDLE sleep (err %d)", ret);

	LOG_INF("Exited from RUNTIME_IDLE sleep");

#ifdef SPI_PM_TEST
	spi_pm_thread_resume();
	LOG_INF(" ====== SPI Transactions are starting...");
	k_sleep(K_SECONDS(2));
#endif

	pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
}

ZTEST(pm_POS_02_Suspend_to_idle_testing_for_SPI, test_SPI_POS_01_suspend_to_idle_basic)
{
	LOG_INF("=== TEST 1: SUSPEND_TO_IDLE Basic ===");

#ifdef SPI_PM_TEST
	k_sleep(K_SECONDS(2));
#endif

	/* 3. Enter SUSPEND_TO_IDLE sleep */
	LOG_INF("Enter SUSPEND_TO_IDLE for (%d microseconds)", SUSPEND_IDLE_SLEEP_USEC);

	int ret = app_enter_normal_sleep(SUSPEND_IDLE_SLEEP_USEC);

	zassert_equal(ret, 0, "app_enter_normal_sleep failed (%d)", ret);

	LOG_INF("Woke up from SUSPEND_TO_IDLE");

	/* 4. Verify SPI can resume */
#ifdef SPI_PM_TEST
	spi_pm_thread_resume();
	k_sleep(K_SECONDS(2));
	LOG_INF("SPI resumed successfully after SUSPEND_TO_IDLE");
#endif
}

ZTEST(pm_POS_02_Suspend_to_idle_testing_for_SPI, test_SPI_POS_02_suspend_to_idle_multiple_cycles)
{
#define NUM_CYCLES 3
	LOG_INF("=== TEST 2: SUSPEND_TO_IDLE for (%d cycles) ===", NUM_CYCLES);

	for (int cycle = 0; cycle < NUM_CYCLES; cycle++) {

		LOG_INF("--- Cycle %d of %d ---", cycle + 1, NUM_CYCLES);

#ifdef SPI_PM_TEST
		k_sleep(K_SECONDS(2));
#endif

		/* 3. Enter SUSPEND_TO_IDLE sleep */
		LOG_INF("Enter SUSPEND_TO_IDLE for (%d microseconds)", SUSPEND_IDLE_SLEEP_USEC);

		int ret = app_enter_normal_sleep(SUSPEND_IDLE_SLEEP_USEC);

		zassert_equal(ret, 0, "app_enter_normal_sleep failed (%d)", ret);

		LOG_INF("Woke up from SUSPEND_TO_IDLE");

		/* 4. Verify SPI can resume */
#ifdef SPI_PM_TEST
		spi_pm_thread_resume();
		k_sleep(K_SECONDS(2));
		LOG_INF("SPI resumed successfully after SUSPEND_TO_IDLE");
#endif
	}
}

ZTEST(pm_POS_02_Suspend_to_idle_testing_for_SPI, test_SPI_POS_03_suspend_to_idle_min_duration)
{
#define MIN_SLEEP_USEC (1 * 1000 * 1000)

	LOG_INF("=== TEST 3: SUSPEND TO IDLE Minimum Sleep ((%d microseconds)) ===",
		MIN_SLEEP_USEC);

#ifdef SPI_PM_TEST
	k_sleep(K_SECONDS(2));
#endif

	/* 3. Enter SUSPEND_TO_IDLE sleep */
	LOG_INF("Enter SUSPEND_TO_IDLE for (%d microseconds)", MIN_SLEEP_USEC);

	int ret = app_enter_normal_sleep(MIN_SLEEP_USEC);

	zassert_equal(ret, 0, "app_enter_normal_sleep failed (%d)", ret);

	LOG_INF("Woke up from SUSPEND_TO_IDLE");

	/* 4. Verify SPI can resume */
#ifdef SPI_PM_TEST
	spi_pm_thread_resume();
	k_sleep(K_SECONDS(2));
	LOG_INF("SPI resumed successfully after SUSPEND_TO_IDLE");
#endif
}

ZTEST(pm_POS_02_Suspend_to_idle_testing_for_SPI, test_SPI_POS_04_suspend_to_idle_max_duration)
{
#define MAX_SLEEP_USEC (18 * 1000 * 1000)

	LOG_INF("=== TEST 4: SUSPEND TO IDLE Maximum Sleep ((%d microseconds)) ===",
		MAX_SLEEP_USEC);

#ifdef SPI_PM_TEST
	k_sleep(K_SECONDS(2));
#endif

	/* 3. Enter SUSPEND_TO_IDLE sleep */
	LOG_INF("Enter SUSPEND_TO_IDLE for (%d microseconds)", MAX_SLEEP_USEC);

	int ret = app_enter_normal_sleep(MAX_SLEEP_USEC);

	zassert_equal(ret, 0, "app_enter_normal_sleep failed (%d)", ret);

	LOG_INF("Woke up from SUSPEND_TO_IDLE");

	/* 4. Verify SPI can resume */
#ifdef SPI_PM_TEST
	spi_pm_thread_resume();
	k_sleep(K_SECONDS(2));
	LOG_INF("SPI resumed successfully after SUSPEND_TO_IDLE");
#endif
}

ZTEST(pm_POS_02_Suspend_to_idle_testing_for_SPI,
test_SPI_POS_05_Runtime_Then_suspend_to_idle_sequence_test)
{
	LOG_INF("=== TEST 1: Basic RUNTIME_IDLE ===");

	/* Lock SUSPEND_IDLE to force PM policy to select RUNTIME_IDLE only */
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);

#ifdef SPI_PM_TEST
	/* let SPI do some transactions. */
	k_sleep(K_SECONDS(5));
#endif

	LOG_INF("Enter RUNTIME_IDLE sleep for (%d microseconds)", RUNTIME_IDLE_SLEEP_USEC);
	ret = app_enter_normal_sleep(RUNTIME_IDLE_SLEEP_USEC);
	__ASSERT(ret == 0, "Could not enter RUNTIME_IDLE sleep (err %d)", ret);

	LOG_INF("Exited from RUNTIME_IDLE sleep");

#ifdef SPI_PM_TEST
	spi_pm_thread_resume();
	LOG_INF(" ====== SPI Transactions are starting...");
	k_sleep(K_SECONDS(2));
#endif

	pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);

	k_sleep(K_MSEC(50));

	LOG_INF("=== TEST 2: SUSPEND_TO_IDLE Basic ===");

#ifdef SPI_PM_TEST
	k_sleep(K_SECONDS(2));
#endif

	/* 3. Enter SUSPEND_TO_IDLE sleep */
	LOG_INF("Enter SUSPEND_TO_IDLE for (%d microseconds)", SUSPEND_IDLE_SLEEP_USEC);

	int ret = app_enter_normal_sleep(SUSPEND_IDLE_SLEEP_USEC);

	zassert_equal(ret, 0, "app_enter_normal_sleep failed (%d)", ret);

	LOG_INF("Woke up from SUSPEND_TO_IDLE");

	/* 4. Verify SPI can resume */
#ifdef SPI_PM_TEST
	spi_pm_thread_resume();
	k_sleep(K_SECONDS(2));
	LOG_INF("SPI resumed successfully after SUSPEND_TO_IDLE");
#endif
}

ZTEST(pm_POS_04_STANDBY_testing_for_SPI, test_SPI_POS_01_s2ram_standby_basic)
{

	LOG_INF("=== TEST 1: Basic S2RAM STANDBY ===");

#if defined(CONFIG_RTSS_HE)

	if (S2RAM_SUPPORTED) {

		app_pm_lock_deeper_states(false);

#ifdef SPI_PM_TEST
		k_sleep(K_SECONDS(5));
#endif

		LOG_INF("Enter PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) for (%d microseconds)",
			S2RAM_STANDBY_SLEEP_USEC);
		ret = app_enter_deep_sleep(S2RAM_STANDBY_SLEEP_USEC);
		__ASSERT(ret == 0, "Could not enter PM_STATE_SUSPEND_TO_RAM (err %d)", ret);

		LOG_INF("=== Resumed from PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) ===");

		/* Verify main thread is running properly */
		for (int i = 0; i < 3; i++) {
			LOG_INF("Main thread running - iteration %d - tick: %llu", i,
				k_uptime_ticks());
			k_sleep(K_SECONDS(2));
		}

		app_pm_lock_deeper_states(true);

	} else {

		LOG_INF("Skipping PM_STATE_SUSPEND_TO_RAM (MRAM boot)");
		ztest_test_skip();
	}

#elif defined(CONFIG_RTSS_HP)

	LOG_INF("Skipping PM_STATE_SUSPEND_TO_RAM Not supported for HP");
	ztest_test_skip();

#endif
}

ZTEST(pm_POS_04_STANDBY_testing_for_SPI, test_SPI_POS_02_s2ram_standby_multiple_cycles)
{

#define NUM_CYCLES 3

	LOG_INF("=== TEST 2: Multiple S2RAM STANDBY Cycles (%d) ===", NUM_CYCLES);

	for (int cycle = 0; cycle < NUM_CYCLES; cycle++) {

#if defined(CONFIG_RTSS_HE)

		if (S2RAM_SUPPORTED) {

			app_pm_lock_deeper_states(false);

#ifdef SPI_PM_TEST
			k_sleep(K_SECONDS(5));
#endif

			LOG_INF("Enter PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) for (%d "
				"microseconds)",
				S2RAM_STANDBY_SLEEP_USEC);
			ret = app_enter_deep_sleep(S2RAM_STANDBY_SLEEP_USEC);
			__ASSERT(ret == 0, "Could not enter PM_STATE_SUSPEND_TO_RAM (err %d)", ret);

			LOG_INF("=== Resumed from PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) "
				"===");

			/* Verify main thread is running properly */
			for (int i = 0; i < 3; i++) {
				LOG_INF("Main thread running - iteration %d - tick: %llu", i,
					k_uptime_ticks());
				k_sleep(K_SECONDS(2));
			}

			app_pm_lock_deeper_states(true);

		} else {

			LOG_INF("Skipping PM_STATE_SUSPEND_TO_RAM (MRAM boot)");
			ztest_test_skip();
		}

#elif defined(CONFIG_RTSS_HP)

		LOG_INF("Skipping PM_STATE_SUSPEND_TO_RAM Not supported for HP");
		ztest_test_skip();

#endif
	}
}

ZTEST(pm_POS_04_STANDBY_testing_for_SPI, test_SPI_POS_03_runtime_idle_then_s2ram_standby)
{

#if defined(CONFIG_RTSS_HE)

	if (S2RAM_SUPPORTED) {

		app_pm_lock_deeper_states(false);

		LOG_INF("=== TEST 3: RUNTIME_IDLE then S2RAM STANDBY ===");

		pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);

#ifdef SPI_PM_TEST
		k_sleep(K_SECONDS(5));
#endif

		LOG_INF("Enter RUNTIME_IDLE for (%d microseconds)", RUNTIME_IDLE_SLEEP_USEC);

		ret = app_enter_normal_sleep(RUNTIME_IDLE_SLEEP_USEC);

		__ASSERT(ret == 0, "Colud not enter RUNTIME IDLE (%d err)", ret);

		LOG_INF("Exited from RUNTIME_IDLE");

#ifdef SPI_PM_TEST

		spi_pm_thread_resume();
		LOG_INF("====== SPI Transactions starting after RUNTIME_IDLE...");
		k_sleep(K_SECONDS(2));

#endif

		pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);

		k_sleep(K_MSEC(50));

#ifdef SPI_PM_TEST
		k_sleep(K_SECONDS(5));
#endif

		LOG_INF("Enter PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) for (%d "
			"microseconds)",
			S2RAM_STANDBY_SLEEP_USEC);
		ret = app_enter_deep_sleep(S2RAM_STANDBY_SLEEP_USEC);
		__ASSERT(ret == 0, "Could not enter PM_STATE_SUSPEND_TO_RAM (err %d)", ret);

		LOG_INF("=== Resumed from PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) "
			"===");

		/* Verify main thread is running properly */
		for (int i = 0; i < 3; i++) {
			LOG_INF("Main thread running - iteration %d - tick: %llu", i,
				k_uptime_ticks());
			k_sleep(K_SECONDS(2));
		}

		app_pm_lock_deeper_states(true);

#ifdef SPI_PM_TEST
		k_sleep(K_SECONDS(5));
#endif
	} else {

		LOG_INF("Skipping PM_STATE_SUSPEND_TO_RAM (MRAM boot)");
		ztest_test_skip();
	}

#elif defined(CONFIG_RTSS_HP)

	LOG_INF("Skipping PM_STATE_SUSPEND_TO_RAM Not supported for HP");
	ztest_test_skip();

#endif
}

ZTEST(pm_POS_04_STANDBY_testing_for_SPI, test_SPI_POS_04_s2ram_standby_min_sleep)
{

#define STANDBY_MIN_SLEEP_USEC (20 * 1000 * 1000)

	LOG_INF("=== TEST 4: S2RAM STANDBY Min Sleep (%d us) ===", STANDBY_MIN_SLEEP_USEC);

#if defined(CONFIG_RTSS_HE)

	if (S2RAM_SUPPORTED) {

		app_pm_lock_deeper_states(false);

#ifdef SPI_PM_TEST
		k_sleep(K_SECONDS(5));
#endif

		LOG_INF("Enter PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) for (%d microseconds)",
			STANDBY_MIN_SLEEP_USEC);
		ret = app_enter_deep_sleep(STANDBY_MIN_SLEEP_USEC);
		__ASSERT(ret == 0, "Could not enter PM_STATE_SUSPEND_TO_RAM (err %d)", ret);

		LOG_INF("=== Resumed from PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) ===");

		/* Verify main thread is running properly */
		for (int i = 0; i < 3; i++) {
			LOG_INF("Main thread running - iteration %d - tick: %llu", i,
				k_uptime_ticks());
			k_sleep(K_SECONDS(2));
		}

		app_pm_lock_deeper_states(true);

	} else {

		LOG_INF("Skipping PM_STATE_SUSPEND_TO_RAM (MRAM boot)");
		ztest_test_skip();
	}

#elif defined(CONFIG_RTSS_HP)

	LOG_INF("Skipping PM_STATE_SUSPEND_TO_RAM Not supported for HP");
	ztest_test_skip();

#endif
}

ZTEST(pm_POS_04_STANDBY_testing_for_SPI, test_SPI_POS_05_s2ram_standby_max_sleep)
{

#define STANDBY_MAX_SLEEP_USEC (21 * 1000 * 1000)

	LOG_INF("=== TEST 4: S2RAM STANDBY Max Sleep (%d us) ===", STANDBY_MAX_SLEEP_USEC);

#if defined(CONFIG_RTSS_HE)

	if (S2RAM_SUPPORTED) {

		app_pm_lock_deeper_states(false);

#ifdef SPI_PM_TEST
		k_sleep(K_SECONDS(5));
#endif

		LOG_INF("Enter PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) for (%d microseconds)",
			STANDBY_MAX_SLEEP_USEC);
		ret = app_enter_deep_sleep(STANDBY_MAX_SLEEP_USEC);
		__ASSERT(ret == 0, "Could not enter PM_STATE_SUSPEND_TO_RAM (err %d)", ret);

		LOG_INF("=== Resumed from PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) ===");

		/* Verify main thread is running properly */
		for (int i = 0; i < 3; i++) {
			LOG_INF("Main thread running - iteration %d - tick: %llu", i,
				k_uptime_ticks());
			k_sleep(K_SECONDS(2));
		}

		app_pm_lock_deeper_states(true);

	} else {

		LOG_INF("Skipping PM_STATE_SUSPEND_TO_RAM (MRAM boot)");
		ztest_test_skip();
	}

#elif defined(CONFIG_RTSS_HP)

	LOG_INF("Skipping PM_STATE_SUSPEND_TO_RAM Not supported for HP");
	ztest_test_skip();

#endif
}

ZTEST(pm_POS_04_STANDBY_testing_for_SPI, test_SPI_POS_06_s2ram_standby_no_soft_off)
{

	LOG_INF("=== TEST 6: Verify SOFT_OFF Blocked During S2RAM STANDBY ===");

#if defined(CONFIG_RTSS_HE)

	if (S2RAM_SUPPORTED) {

		zassert_equal(SOFT_OFF_SUPPORTED, 0,
			      "SOFT_OFF should not be supported on TCM boot");

		app_pm_lock_deeper_states(false);

		/* Verify SOFT_OFF is locked - TCM boot should never use SOFT_OFF */
		zassert_false(
			pm_policy_state_lock_is_active(PM_STATE_SUSPEND_TO_RAM, PM_ALL_SUBSTATES),
			"SOFT_OFF should be locked during S2RAM STANDBY test");

#ifdef SPI_PM_TEST
		k_sleep(K_SECONDS(5));
#endif

		LOG_INF("Enter PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) for (%d microseconds)",
			S2RAM_STANDBY_SLEEP_USEC);
		ret = app_enter_deep_sleep(S2RAM_STANDBY_SLEEP_USEC);
		__ASSERT(ret == 0, "Could not enter PM_STATE_SUSPEND_TO_RAM (err %d)", ret);

		LOG_INF("=== Resumed from PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) ===");

		/* Verify SOFT_OFF still locked after wakeup */
		zassert_false(
			pm_policy_state_lock_is_active(PM_STATE_SUSPEND_TO_RAM, PM_ALL_SUBSTATES),
			"SOFT_OFF should be locked during S2RAM STANDBY test");

		/* Verify main thread is running properly */
		for (int i = 0; i < 3; i++) {
			LOG_INF("Main thread running - iteration %d - tick: %llu", i,
				k_uptime_ticks());
			k_sleep(K_SECONDS(2));
		}

		LOG_INF("PASS: SOFT_OFF correctly blocked during S2RAM STANDBY");

		app_pm_lock_deeper_states(true);
	} else {

		LOG_INF("Skipping PM_STATE_SUSPEND_TO_RAM (MRAM BOOT)");
		ztest_test_skip();
	}

#elif defined(CONFIG_RTSS_HP)

	LOG_INF("Skipping PM_STATE_SUSPEND_TO_RAM Not supported for HP");
	ztest_test_skip();

#endif
}

ZTEST(pm_POS_05_STOP_testing_for_SPI, test_SPI_POS_01_s2ram_stop_basic)
{

	LOG_INF("=== TEST 1: Basic S2RAM STOP ===");

#if defined(CONFIG_RTSS_HE)

	if (S2RAM_SUPPORTED) {

		app_pm_lock_deeper_states(false);

#ifdef SPI_PM_TEST
		k_sleep(K_SECONDS(5));
#endif

		LOG_INF("Enter PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) for (%d "
			"microseconds)",
			S2RAM_STOP_SLEEP_USEC);

		ret = app_enter_deep_sleep(S2RAM_STOP_SLEEP_USEC);

		__ASSERT(ret == 0, "Could not enter in STOP mode");

		LOG_INF("=== Resumed from PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) "
			"===");

		/* Verify main thread is running properly */
		for (int i = 0; i < 3; i++) {
			LOG_INF("Main thread running - iteration %d - tick: %llu", i,
				k_uptime_ticks());
			k_sleep(K_SECONDS(2));
		}

		app_pm_lock_deeper_states(true);

	} else {
		LOG_INF("Skipping PM_STATE_SUSPEND_TO_RAM (MRAM BOOT)");
		ztest_test_skip();
	}

#elif defined(CONFIG_RTSS_HP)

	LOG_INF("Skipping PM_STATE_SUSPEND_TO_RAM Not supported for HP");
	ztest_test_skip();

#endif

	LOG_INF("=== POWER STATE SEQUENCE COMPLETED ===");
}

ZTEST(pm_POS_03_SOFT_OFF__testing_for_SPI, test_SPI_POS_01_soft_off_basic)
{

	app_pm_lock_deeper_states(false);

	/* PM_STATE_SOFT_OFF (deepest sleep with wake capability) */
#if defined(CONFIG_RTSS_HP)

	if (!IS_BOOTING_FROM_MRAM()) {
		LOG_ERR("HP core does not support PM_STATE_SOFT_OFF when booting from TCM");
		ztest_test_skip();
	}

	/* HP core: always SOFT_OFF */
	LOG_INF("Enter PM_STATE_SOFT_OFF for (%d microseconds)", SOFT_OFF_SLEEP_USEC);
	LOG_INF("Note: SOFT_OFF has no retention - system will reset on wakeup");
	ret = app_enter_deep_sleep(SOFT_OFF_SLEEP_USEC);
	__ASSERT(ret == 0, "Could not enter PM_STATE_SOFT_OFF (err %d)", ret);

	/* Should never reach here - SOFT_OFF causes full reset on wakeup */
	LOG_ERR("ERROR: Resumed after PM_STATE_SOFT_OFF - this should not happen!");
	__ASSERT(false, "PM_STATE_SOFT_OFF should have caused a reset");

#elif defined(CONFIG_RTSS_HE)
	/* HE core: only SOFT_OFF when booting from MRAM */
	if (SOFT_OFF_SUPPORTED) {
		LOG_INF("Enter PM_STATE_SOFT_OFF for (%d microseconds)", SOFT_OFF_SLEEP_USEC);
		LOG_INF("Note: SOFT_OFF has no retention - system will reset on wakeup");
		ret = app_enter_deep_sleep(SOFT_OFF_SLEEP_USEC);
		__ASSERT(ret == 0, "Could not enter PM_STATE_SOFT_OFF (err %d)", ret);

		/* Should never reach here - SOFT_OFF causes full reset on wakeup */
		LOG_ERR("ERROR: Resumed after PM_STATE_SOFT_OFF - this should not happen!");
		__ASSERT(false, "PM_STATE_SOFT_OFF should have caused a reset");
	} else {
		LOG_INF("Skipping PM_STATE_SOFT_OFF (TCM boot, using retention instead)");
	}
#endif

	app_pm_lock_deeper_states(true);
}

ZTEST_SUITE(pm_POS_01_Runtime_idle_testing_for_SPI, NULL, setup, before, after, teardown);
ZTEST_SUITE(pm_POS_02_Suspend_to_idle_testing_for_SPI, NULL, NULL, before, after, teardown);
ZTEST_SUITE(pm_POS_04_STANDBY_testing_for_SPI, NULL, NULL, before, after, teardown);
ZTEST_SUITE(pm_POS_05_STOP_testing_for_SPI, NULL, NULL, before, after, teardown);
ZTEST_SUITE(pm_POS_03_SOFT_OFF__testing_for_SPI, NULL, NULL, before, after, teardown);

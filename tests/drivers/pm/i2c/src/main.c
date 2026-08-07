/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#include "aipm.h"
#include <stdio.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/pm/pm.h>
#include <zephyr/pm/device.h>
#include <zephyr/pm/policy.h>
#if defined(CONFIG_POWEROFF)
#include <zephyr/sys/poweroff.h>
#endif
#include <zephyr/drivers/counter.h>
#include <zephyr/drivers/i2c.h>
#include <se_service.h>
#include <zephyr/sys/util.h>

#include <zephyr/ztest.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(i2c_pm, LOG_LEVEL_INF);

#define I2C_DEV_NODE DT_ALIAS(i2c_instance)

/* I2C target address (e.g., BME280 sensor) */
#define I2C_TARGET_ADDR 0x76
/* Register to read (chip ID register) */
#define I2C_REG_CHIP_ID 0xD0

#if !defined(CONFIG_ALIF_SE_DTS_RUN_PROFILE) || !defined(CONFIG_ALIF_SE_DTS_OFF_PROFILE)
/**
 * As per the application requirements, it can remove the memory blocks which are not in use.
 */
#if defined(CONFIG_SOC_SERIES_E1C) || defined(CONFIG_SOC_SERIES_B1)
#define APP_RET_MEM_BLOCKS                                                                         \
	(SRAM4_1_MASK | SRAM4_2_MASK | SRAM4_3_MASK | SRAM4_4_MASK |     \
	 SRAM5_1_MASK | SRAM5_2_MASK | SRAM5_3_MASK | SRAM5_4_MASK |     \
	 SRAM5_5_MASK)
#define SERAM_MEMORY_BLOCKS_IN_USE (SERAM_1_MASK | SERAM_2_MASK | SERAM_3_MASK | SERAM_4_MASK)
#else
#define APP_RET_MEM_BLOCKS         (SRAM4_1_MASK | SRAM4_2_MASK | SRAM5_1_MASK | SRAM5_2_MASK)
#define SERAM_MEMORY_BLOCKS_IN_USE SERAM_MASK
#endif
#endif /* CONFIG_ALIF_SE_DTS_RUN_PROFILE || CONFIG_ALIF_SE_DTS_OFF_PROFILE */

#if !defined(CONFIG_ALIF_SE_DTS_OFF_PROFILE)
#if DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(rtc0), snps_dw_apb_rtc, okay)
#define SE_OFFP_EWIC_CFG      EWIC_RTC_A
#define SE_OFFP_WAKEUP_EVENTS WE_LPRTC
#elif DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(timer0), snps_dw_timers, okay)
#define SE_OFFP_EWIC_CFG      EWIC_VBAT_TIMER
#define SE_OFFP_WAKEUP_EVENTS WE_LPTIMER0
#elif DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(lputimer0), alif_utimer, okay)
#define SE_OFFP_EWIC_CFG      EWIC_VBAT_TIMER
#define SE_OFFP_WAKEUP_EVENTS WE_LPTIMER0
#endif
#endif /* CONFIG_ALIF_SE_DTS_OFF_PROFILE */

#if DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(rtc0), snps_dw_apb_rtc, okay)
#define WAKEUP_SOURCE DT_NODELABEL(rtc0)
#elif DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(timer0), snps_dw_timers, okay)
#define WAKEUP_SOURCE DT_NODELABEL(timer0)
#elif DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(lputimer0), alif_utimer, okay)
#define WAKEUP_SOURCE DT_NODELABEL(lputimer0)
#else
#error "Wakeup Device not enabled in the dts"
#endif

/* Sleep duration for PM_STATE_RUNTIME_IDLE */
#define RUNTIME_IDLE_SLEEP_USEC  (18 * 1000 * 1000)
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

#if !defined(CONFIG_ALIF_SE_DTS_RUN_PROFILE)
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
#endif /* CONFIG_ALIF_SE_DTS_RUN_PROFILE */

#if !defined(CONFIG_ALIF_SE_DTS_OFF_PROFILE)
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
#endif /* CONFIG_ALIF_SE_DTS_OFF_PROFILE */

#if !defined(CONFIG_ALIF_SE_DTS_RUN_PROFILE)
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
#endif

#if !defined(CONFIG_ALIF_SE_DTS_RUN_PROFILE) || !defined(CONFIG_ALIF_SE_DTS_OFF_PROFILE)
/**
 * PM Notifier structure
 */
static struct pm_notifier app_pm_notifier = {
#if !defined(CONFIG_ALIF_SE_DTS_OFF_PROFILE)
	.state_entry = pm_notify_state_entry,
#endif
#if !defined(CONFIG_ALIF_SE_DTS_RUN_PROFILE)
	.pre_device_resume = pm_notify_pre_device_resume,
#endif
};
#endif

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

#if defined(CONFIG_RTSS_HE)
	if (S2RAM_SUPPORTED) {
		/* TCM boot: SOFT_OFF is not supported, keep it permanently locked
		 * so the system never accidentally enters SOFT_OFF.
		 */
		pm_policy_state_lock_get(PM_STATE_SOFT_OFF, PM_ALL_SUBSTATES);
	}
#endif

	/* Lock SUSPEND_TO_IDLE so only RUNTIME_IDLE is available by default.
	 * Tests that need SUSPEND_TO_IDLE will explicitly unlock it.
	 */
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);

#if !defined(CONFIG_ALIF_SE_DTS_RUN_PROFILE) || !defined(CONFIG_ALIF_SE_DTS_OFF_PROFILE)
	/* Register PM notifier callbacks */
	pm_notifier_register(&app_pm_notifier);
#endif

	return 0;
}
SYS_INIT(app_pre_kernel_init, PRE_KERNEL_2, 0);

/**
 * Perform an I2C register read to verify the bus is functional.
 * Writes register address 0xD0 and reads back 1 byte (chip ID).
 * Returns 0 on success, negative errno on failure.
 */
static int i2c_test_transfer(const struct device *i2c_dev)
{
	uint8_t tx_data = I2C_REG_CHIP_ID;
	uint8_t rx_data = 0;
	int ret;

	ret = i2c_write_read(i2c_dev, I2C_TARGET_ADDR, &tx_data, 1, &rx_data, 1);
	if (ret) {
		LOG_ERR("I2C write_read failed: %d", ret);
		return ret;
	}
	LOG_INF("I2C read reg 0x%02X = 0x%02X", tx_data, rx_data);

	return 0;
}

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
	struct counter_alarm_cfg alarm_cfg = {0};
	int ret;
	/*
	 * Set the alarm and delay so that idle thread can run
	 */
	alarm_cfg.ticks = counter_us_to_ticks(wakeup_dev, sleep_usec);
	alarm_cfg.callback = alarm_callback_fn;
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

/* ======================================================== */

const struct device *cons;
const struct device *wakeup_dev;
const struct device *i2c_dev;

void *setup(void)
{
	cons = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
	wakeup_dev = DEVICE_DT_GET(WAKEUP_SOURCE);
	i2c_dev = DEVICE_DT_GET(I2C_DEV_NODE);

	__ASSERT(device_is_ready(cons), "%s: device not ready", cons->name);
	__ASSERT(device_is_ready(wakeup_dev), "%s: device not ready", wakeup_dev->name);

	if (!device_is_ready(i2c_dev)) {
		LOG_ERR("I2C device not ready");
		return 0;
	}
	printk("Device %p name is %s\n", i2c_dev, i2c_dev->name);

#if defined(CONFIG_RTSS_HE)
	/* Boot location determines which PM states are available */
	bool is_mram_boot = IS_BOOTING_FROM_MRAM();

	if (is_mram_boot) {
		LOG_INF("%s RTSS_HE (MRAM boot): I2C PM states demo "
			"(RUNTIME_IDLE, SUSPEND_TO_IDLE, SOFT_OFF)",
			CONFIG_BOARD);
	} else {
		LOG_INF("%s RTSS_HE (TCM boot): I2C PM states demo "
			"(RUNTIME_IDLE, SUSPEND_TO_IDLE, S2RAM)",
			CONFIG_BOARD);
	}
#else
	LOG_INF("%s RTSS_HP: I2C PM states demo "
		"(RUNTIME_IDLE, SUSPEND_TO_IDLE, SOFT_OFF)",
		CONFIG_BOARD);
#endif

	int ret = counter_start(wakeup_dev);

	__ASSERT(!ret || ret == -EALREADY, "Failed to start counter (err %d)", ret);

	return NULL;
}

void before(void *fixture)
{
	/* Test I2C transfer before sleep */
	LOG_INF("--- I2C transfer prior to sleep ---");

	int ret = i2c_test_transfer(i2c_dev);

	if (ret) {
		LOG_WRN("I2C transfer failed prior to sleep (err %d), continuing PM test", ret);
	}
}

void after(void *fixture)
{
	/* Test I2C transfer after sleep */
	LOG_INF("--- I2C transfer following sleep ---");

	int ret = i2c_test_transfer(i2c_dev);

	if (ret) {
		LOG_ERR("I2C transfer failed following sleep (err %d)", ret);
	}
}

void teardown(void *fixture)
{
	/* Nothing to do here.  Each test body re-locks whatever PM
	 * states it unlocked, and pre_kernel_init's initial locks
	 * keep the system in RUNTIME_IDLE between suites.
	 * Do NOT add lock_get calls here — teardown runs once per
	 * suite, and extra locks would accumulate, preventing later
	 * suites from entering deep sleep states.
	 */
}

ZTEST_SUITE(i2c_test_POS_01_RUNTIME_IDLE, NULL, setup, before, after, teardown);
ZTEST_SUITE(i2c_test_POS_02_SUSPEND_TO_IDLE, NULL, setup, before, after, teardown);
ZTEST_SUITE(i2c_test_POS_03_SOFT_OFF, NULL, setup, before, after, teardown);
ZTEST_SUITE(i2c_test_POS_04_S2RAM_STANDBY, NULL, setup, before, after, teardown);
ZTEST_SUITE(i2c_test_POS_05_S2RAM_STOP, NULL, setup, before, after, teardown);

/*========================RUNTIME_IDLE TESTS========================*/

ZTEST(i2c_test_POS_01_RUNTIME_IDLE, test_POS_01_RUNTIME_IDLE_basic)
{

	LOG_INF("=== TEST 1: Basic RUNTIME_IDLE ===");

	/* SUSPEND_TO_IDLE and deeper states are locked from app_pre_kernel_init,
	 * so PM can only select RUNTIME_IDLE.  No need to touch the lock.
	 */
	LOG_INF("Enter RUNTIME_IDLE sleep for (%d microseconds)", RUNTIME_IDLE_SLEEP_USEC);
	int ret = app_enter_normal_sleep(RUNTIME_IDLE_SLEEP_USEC);

	__ASSERT(ret == 0, "Could not enter RUNTIME_IDLE sleep (err %d)", ret);

	LOG_INF("Exited from RUNTIME_IDLE sleep");
}

ZTEST(i2c_test_POS_01_RUNTIME_IDLE, test_POS_02_RUNTIME_IDLE_multiple_cycles)
{

#define RUNTIME_IDLE_NUM_CYCLES 3

	LOG_INF("=== TEST 2: Multiple RUNTIME_IDLE Cycles (%d) ===", RUNTIME_IDLE_NUM_CYCLES);

	for (int cycle = 0; cycle < RUNTIME_IDLE_NUM_CYCLES; cycle++) {

		LOG_INF("--- Cycle %d of %d ---", cycle + 1, RUNTIME_IDLE_NUM_CYCLES);

		pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
		LOG_INF("Enter RUNTIME_IDLE sleep for (%d microseconds)", RUNTIME_IDLE_SLEEP_USEC);
		int ret = app_enter_normal_sleep(RUNTIME_IDLE_SLEEP_USEC);

		__ASSERT(ret == 0, "Could not enter RUNTIME_IDLE sleep (err %d)", ret);

		LOG_INF("Exited from RUNTIME_IDLE sleep");
		pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);

		if (cycle < RUNTIME_IDLE_NUM_CYCLES - 1) {
			after(NULL);
		}
	}
}

ZTEST(i2c_test_POS_01_RUNTIME_IDLE, test_POS_03_RUNTIME_IDLE_maximum_duration)
{

#define RUNTIME_IDLE_MAX_DURATION_USEC (18 * 1000 * 1000)

	LOG_INF("=== TEST 3: RUNTIME_IDLE Maximum Duration (%d microseconds) ===",
		RUNTIME_IDLE_MAX_DURATION_USEC);

	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
	LOG_INF("Enter RUNTIME_IDLE sleep for (%d microseconds)", RUNTIME_IDLE_MAX_DURATION_USEC);
	int ret = app_enter_normal_sleep(RUNTIME_IDLE_MAX_DURATION_USEC);

	__ASSERT(ret == 0, "Could not enter RUNTIME_IDLE sleep (err %d)", ret);

	LOG_INF("Exited from RUNTIME_IDLE sleep");
	pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
}

ZTEST(i2c_test_POS_01_RUNTIME_IDLE, test_POS_04_RUNTIME_IDLE_minimum_duration)
{

#define RUNTIME_IDLE_MIN_DURATION_USEC (2 * 1000 * 1000)

	LOG_INF("=== TEST 4: RUNTIME_IDLE Minimum Duration (%d microseconds) ===",
		RUNTIME_IDLE_MIN_DURATION_USEC);

	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
	LOG_INF("Enter RUNTIME_IDLE sleep for (%d microseconds)", RUNTIME_IDLE_MIN_DURATION_USEC);
	int ret = app_enter_normal_sleep(RUNTIME_IDLE_MIN_DURATION_USEC);

	__ASSERT(ret == 0, "Could not enter RUNTIME_IDLE sleep (err %d)", ret);

	LOG_INF("Exited from RUNTIME_IDLE sleep");
	pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
}

/*========================END RUNTIME_IDLE TESTS========================*/

/*========================SUSPEND_TO_IDLE TESTS========================*/

ZTEST(i2c_test_POS_02_SUSPEND_TO_IDLE, test_POS_01_SUSPEND_TO_IDLE_basic)
{

#if defined(CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER)
	LOG_INF("=== TEST 1: Basic SUSPEND_TO_IDLE ===");

	/* Unlock SUSPEND_TO_IDLE so PM framework can select it */
	pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);

	LOG_INF("Enter PM_STATE_SUSPEND_TO_IDLE for (%d microseconds)", SUSPEND_IDLE_SLEEP_USEC);
	k_sleep(K_USEC(SUSPEND_IDLE_SLEEP_USEC));
	LOG_INF("Exited from PM_STATE_SUSPEND_TO_IDLE");

	/* Re-lock to prevent accidental entry during cleanup */
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
#else
	LOG_INF("=== TEST 1: SUSPEND_TO_IDLE SKIPPED (LPM timer not enabled) ===");
	ztest_test_skip();
#endif
}

ZTEST(i2c_test_POS_02_SUSPEND_TO_IDLE, test_POS_02_SUSPEND_TO_IDLE_multiple_cycles)
{

#define SUSPEND_TO_IDLE_NUM_CYCLES 3

	LOG_INF("=== TEST 2: SUSPEND_TO_IDLE Multiple Cycles (%d cycles) ===",
		SUSPEND_TO_IDLE_NUM_CYCLES);

	for (int cycle = 0; cycle < SUSPEND_TO_IDLE_NUM_CYCLES; cycle++) {

		LOG_INF("--- Cycle %d of %d ---", cycle + 1, SUSPEND_TO_IDLE_NUM_CYCLES);

#if defined(CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER)
		LOG_INF("=== TEST 2: Basic SUSPEND_TO_IDLE ===");

		/* Unlock SUSPEND_TO_IDLE so PM framework can select it */
		pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);

		LOG_INF("Enter PM_STATE_SUSPEND_TO_IDLE for (%d microseconds)",
			SUSPEND_IDLE_SLEEP_USEC);
		k_sleep(K_USEC(SUSPEND_IDLE_SLEEP_USEC));
		LOG_INF("Exited from PM_STATE_SUSPEND_TO_IDLE");

		/* Re-lock to prevent accidental entry during cleanup */
		pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
#else
		LOG_INF("=== TEST 2: SUSPEND_TO_IDLE SKIPPED (LPM timer not enabled) ===");
		ztest_test_skip();
#endif

		if (cycle < SUSPEND_TO_IDLE_NUM_CYCLES - 1) {
			after(NULL);
		}
	}
}

ZTEST(i2c_test_POS_02_SUSPEND_TO_IDLE, test_POS_03_SUSPEND_TO_IDLE_maximum_duration)
{

#define SUSPEND_TO_IDLE_MAX_DURATION_USEC (18 * 1000 * 1000)

	LOG_INF("=== TEST 3: SUSPEND_TO_IDLE Maximum Duration (%d microseconds) ===",
		SUSPEND_TO_IDLE_MAX_DURATION_USEC);

#if defined(CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER)
	LOG_INF("=== TEST 3: Basic SUSPEND_TO_IDLE ===");

	/* Unlock SUSPEND_TO_IDLE so PM framework can select it */
	pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);

	LOG_INF("Enter PM_STATE_SUSPEND_TO_IDLE for (%d microseconds)",
		SUSPEND_TO_IDLE_MAX_DURATION_USEC);
	k_sleep(K_USEC(SUSPEND_TO_IDLE_MAX_DURATION_USEC));
	LOG_INF("Exited from PM_STATE_SUSPEND_TO_IDLE");

	/* Re-lock to prevent accidental entry during cleanup */
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
#else
	LOG_INF("=== TEST 3: SUSPEND_TO_IDLE SKIPPED (LPM timer not enabled) ===");
	ztest_test_skip();
#endif
}

ZTEST(i2c_test_POS_02_SUSPEND_TO_IDLE, test_POS_04_SUSPEND_TO_IDLE_minimum_duration)
{

#define SUSPEND_TO_IDLE_MIN_DURATION_USEC (1 * 1000 * 1000)

	LOG_INF("=== TEST 4: SUSPEND_TO_IDLE Minimum Duration (%d microseconds) ===",
		SUSPEND_TO_IDLE_MIN_DURATION_USEC);

#if defined(CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER)
	LOG_INF("=== TEST 4: Basic SUSPEND_TO_IDLE ===");

	/* Unlock SUSPEND_TO_IDLE so PM framework can select it */
	pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);

	LOG_INF("Enter PM_STATE_SUSPEND_TO_IDLE for (%d microseconds)",
		SUSPEND_TO_IDLE_MIN_DURATION_USEC);
	k_sleep(K_USEC(SUSPEND_TO_IDLE_MIN_DURATION_USEC));
	LOG_INF("Exited from PM_STATE_SUSPEND_TO_IDLE");

	/* Re-lock to prevent accidental entry during cleanup */
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
#else
	LOG_INF("=== TEST 4: SUSPEND_TO_IDLE SKIPPED (LPM timer not enabled) ===");
	ztest_test_skip();
#endif
}

ZTEST(i2c_test_POS_02_SUSPEND_TO_IDLE, test_POS_05_RUNTIME_IDLE_THEN_SUSPEND_TO_IDLE)
{

	LOG_INF("=== TEST 5.1: Basic RUNTIME_IDLE ===");

	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
	LOG_INF("Enter RUNTIME_IDLE sleep for (%d microseconds)", RUNTIME_IDLE_SLEEP_USEC);
	int ret = app_enter_normal_sleep(RUNTIME_IDLE_SLEEP_USEC);

	__ASSERT(ret == 0, "Could not enter RUNTIME_IDLE sleep (err %d)", ret);

	LOG_INF("Exited from RUNTIME_IDLE sleep");
	pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);

#if defined(CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER)
	LOG_INF("=== TEST 5.2: Basic SUSPEND_TO_IDLE ===");

	/* Unlock SUSPEND_TO_IDLE so PM framework can select it */
	pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);

	LOG_INF("Enter PM_STATE_SUSPEND_TO_IDLE for (%d microseconds)", SUSPEND_IDLE_SLEEP_USEC);
	k_sleep(K_USEC(SUSPEND_IDLE_SLEEP_USEC));
	LOG_INF("Exited from PM_STATE_SUSPEND_TO_IDLE");

	/* Re-lock to prevent accidental entry during cleanup */
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
#else
	LOG_INF("=== TEST 5.2: SUSPEND_TO_IDLE SKIPPED (LPM timer not enabled) ===");
	ztest_test_skip();
#endif
}

/*========================End of SUSPEND_TO_IDLE tests========================*/

/*========================SOFT OFF TESTS========================*/

ZTEST(i2c_test_POS_03_SOFT_OFF, test_POS_01_SOFT_OFF_basic)
{

	LOG_INF("=== TESTING SOFT OFF ===");

	app_pm_lock_deeper_states(false);

#if defined(CONFIG_RTSS_HP)

	if (!IS_BOOTING_FROM_MRAM()) {

		__ASSERT(IS_BOOTING_FROM_MRAM(),
			 "HP TCM Retention is not possible - VTOR is set to TCM");
	}

	LOG_INF("Enter PM_STATE_SOFT_OFF for (%d microseconds)", SOFT_OFF_SLEEP_USEC);

	int ret = app_enter_deep_sleep(SOFT_OFF_SLEEP_USEC);

	__ASSERT(ret == 0, "Could not enter SOFT_OFF sleep (err %d)", ret);

	LOG_ERR("ERROR: Resumed after SOFT OFF - This should not happen");
	__ASSERT(false, "PM_STATE_SOFT_OFF should have caused a reset");

#elif defined(CONFIG_RTSS_HE)

	if (SOFT_OFF_SUPPORTED) {

		LOG_INF("Enter PM_STATE_SOFT_OFF for (%d microseconds)", SOFT_OFF_SLEEP_USEC);

		int ret = app_enter_deep_sleep(SOFT_OFF_SLEEP_USEC);

		__ASSERT(ret == 0, "Could not enter SOFT_OFF sleep (err %d)", ret);

		LOG_ERR("ERROR: Resumed after SOFT OFF - This should not happen");
		__ASSERT(false, "PM_STATE_SOFT_OFF should have caused a reset");
	} else {

		LOG_INF("Skipping PM_STATE_SOFT_OFF (TCM boot, using retention instead)");
		/* Re-lock before skip — ztest_test_skip() uses longjmp
		 * and never returns, so code after #endif won't execute.
		 */
		app_pm_lock_deeper_states(true);
		ztest_test_skip();
	}

#endif

	app_pm_lock_deeper_states(true);
}

/*========================END OF SOFT OFF TESTS========================*/

/*========================S2RAM_STANDBY TESTS========================*/

ZTEST(i2c_test_POS_04_S2RAM_STANDBY, test_POS_01_S2RAM_STANDBY_basic)
{

	LOG_INF("=== TEST 1: Basic S2RAM_STANDBY ===");

#if defined(CONFIG_RTSS_HE)

	if (S2RAM_SUPPORTED) {

		app_pm_lock_deeper_states(false);

		LOG_INF("Enter PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) for (%d microseconds)",
			S2RAM_STANDBY_SLEEP_USEC);

		int ret = app_enter_deep_sleep(S2RAM_STANDBY_SLEEP_USEC);

		__ASSERT(ret == 0, "Could not enter S2RAM_STANDBY sleep (err %d)", ret);

		LOG_INF("Resumed from PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY)");

		/* Re-lock deeper states immediately to prevent k_sleep from
		 * re-entering S2RAM during post-resume verification.
		 */
		app_pm_lock_deeper_states(true);

	} else {
		LOG_INF("=== TEST 1: S2RAM_STANDBY SKIPPED (S2RAM not supported) ===");
		ztest_test_skip();
	}

#elif defined(CONFIG_RTSS_HP)

	LOG_INF("Skipping PM_STATE_SUSPEND_TO_RAM (MRAM boot)");
	ztest_test_skip();

#endif
}

ZTEST(i2c_test_POS_04_S2RAM_STANDBY, test_POS_02_S2RAM_STANDBY_multiple_cycles)
{

#define STANDBY_NUM_CYCLES 3
	LOG_INF("=== TEST 2: Multiple S2RAM_STANDBY cycles (%d) ===", STANDBY_NUM_CYCLES);

	for (int cycle = 0; cycle < STANDBY_NUM_CYCLES; cycle++) {
		LOG_INF("Cycle %d of %d", cycle + 1, STANDBY_NUM_CYCLES);

#if defined(CONFIG_RTSS_HE)

		if (S2RAM_SUPPORTED) {

			app_pm_lock_deeper_states(false);

			LOG_INF("Enter PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) for (%d "
				"microseconds)",
				S2RAM_STANDBY_SLEEP_USEC);

			int ret = app_enter_deep_sleep(S2RAM_STANDBY_SLEEP_USEC);

			__ASSERT(ret == 0, "Could not enter S2RAM_STANDBY sleep (err %d)", ret);

			LOG_INF("Resumed from PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY)");

			/* Re-lock deeper states immediately to prevent k_sleep from
			 * re-entering S2RAM during post-resume verification.
			 */
			app_pm_lock_deeper_states(true);

			if (cycle < STANDBY_NUM_CYCLES - 1) {
				after(NULL);
			}

		} else {
			LOG_INF("=== TEST 2: S2RAM_STANDBY SKIPPED (S2RAM not supported) ===");
			ztest_test_skip();
		}

#elif defined(CONFIG_RTSS_HP)

		LOG_INF("Skipping PM_STATE_SUSPEND_TO_RAM (MRAM boot)");
		ztest_test_skip();

#endif
	}
}

ZTEST(i2c_test_POS_04_S2RAM_STANDBY, test_POS_03_S2RAM_STANDBY_maximum_duration)
{

#define STANDBY_MAXIMUM_DURATION_USEC (21 * 1000 * 1000)

	LOG_INF("=== TEST 3: S2RAM_STANDBY maximum duration (%d microseconds) ===",
		STANDBY_MAXIMUM_DURATION_USEC);

#if defined(CONFIG_RTSS_HE)

	if (S2RAM_SUPPORTED) {

		app_pm_lock_deeper_states(false);

		LOG_INF("Enter PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) for (%d "
			"microseconds)",
			STANDBY_MAXIMUM_DURATION_USEC);

		int ret = app_enter_deep_sleep(STANDBY_MAXIMUM_DURATION_USEC);

		__ASSERT(ret == 0, "Could not enter S2RAM_STANDBY sleep (err %d)", ret);

		LOG_INF("Resumed from PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY)");

		/* Re-lock deeper states immediately to prevent k_sleep from
		 * re-entering S2RAM during post-resume verification.
		 */
		app_pm_lock_deeper_states(true);

	} else {
		LOG_INF("=== TEST 3: S2RAM_STANDBY SKIPPED (S2RAM not supported) ===");
		ztest_test_skip();
	}

#elif defined(CONFIG_RTSS_HP)

	LOG_INF("Skipping PM_STATE_SUSPEND_TO_RAM (MRAM boot)");
	ztest_test_skip();

#endif
}

ZTEST(i2c_test_POS_04_S2RAM_STANDBY, test_POS_04_S2RAM_STANDBY_minimum_duration)
{

#define STANDBY_MINIMUM_DURATION_USEC (20 * 1000 * 1000)

	LOG_INF("=== TEST 4: S2RAM_STANDBY minimum duration (%d microseconds) ===",
		STANDBY_MINIMUM_DURATION_USEC);

#if defined(CONFIG_RTSS_HE)

	if (S2RAM_SUPPORTED) {

		app_pm_lock_deeper_states(false);

		LOG_INF("Enter PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) for (%d "
			"microseconds)",
			STANDBY_MINIMUM_DURATION_USEC);

		int ret = app_enter_deep_sleep(STANDBY_MINIMUM_DURATION_USEC);

		__ASSERT(ret == 0, "Could not enter S2RAM_STANDBY sleep (err %d)", ret);

		LOG_INF("Resumed from PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY)");

		/* Re-lock deeper states immediately to prevent k_sleep from
		 * re-entering S2RAM during post-resume verification.
		 */
		app_pm_lock_deeper_states(true);

	} else {
		LOG_INF("=== TEST 4: S2RAM_STANDBY SKIPPED (S2RAM not supported) ===");
		ztest_test_skip();
	}

#elif defined(CONFIG_RTSS_HP)

	LOG_INF("Skipping PM_STATE_SUSPEND_TO_RAM (MRAM boot)");
	ztest_test_skip();

#endif
}

ZTEST(i2c_test_POS_04_S2RAM_STANDBY, test_POS_05_RUNTIME_IDLE_THEN_S2RAM_STANDBY)
{

	LOG_INF("=== TEST 5.1: RUNTIME_IDLE===");

	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
	LOG_INF("Enter RUNTIME_IDLE sleep for (%d microseconds)", RUNTIME_IDLE_SLEEP_USEC);
	int ret = app_enter_normal_sleep(RUNTIME_IDLE_SLEEP_USEC);

	__ASSERT(ret == 0, "Could not enter RUNTIME_IDLE sleep (err %d)", ret);

	LOG_INF("Exited from RUNTIME_IDLE sleep");
	pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);

#if defined(CONFIG_RTSS_HE)

	LOG_INF("=== TEST 5.2: S2RAM_STANDBY ===");

	if (S2RAM_SUPPORTED) {

		app_pm_lock_deeper_states(false);

		LOG_INF("Enter PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) for (%d "
			"microseconds)",
			STANDBY_MINIMUM_DURATION_USEC);

		ret = app_enter_deep_sleep(STANDBY_MINIMUM_DURATION_USEC);
		__ASSERT(ret == 0, "Could not enter S2RAM_STANDBY sleep (err %d)", ret);

		LOG_INF("Resumed from PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY)");

		/* Re-lock deeper states immediately to prevent k_sleep from
		 * re-entering S2RAM during post-resume verification.
		 */
		app_pm_lock_deeper_states(true);

	} else {
		LOG_INF("=== TEST 5: S2RAM_STANDBY SKIPPED (S2RAM not supported) ===");
		ztest_test_skip();
	}

#elif defined(CONFIG_RTSS_HP)

	LOG_INF("Skipping PM_STATE_SUSPEND_TO_RAM (MRAM boot)");
	ztest_test_skip();

#endif
}

ZTEST(i2c_test_POS_04_S2RAM_STANDBY, test_POS_06_S2RAM_STANDBY_NO_SOFT_OFF)
{
#if defined(CONFIG_RTSS_HE)

	if (S2RAM_SUPPORTED) {

		zassert_equal(SOFT_OFF_SUPPORTED, 0,
			      "SOFT OFF should not be supported in this test");

		app_pm_lock_deeper_states(false);

		zassert_true(
			pm_policy_state_lock_is_active(PM_STATE_SOFT_OFF, PM_ALL_SUBSTATES),
			"SOFT OFF should be locked during STANDBY test");

		LOG_INF("Enter PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) for (%d "
			"microseconds)",
			STANDBY_MINIMUM_DURATION_USEC);

		int ret = app_enter_deep_sleep(STANDBY_MINIMUM_DURATION_USEC);

		__ASSERT(ret == 0, "Could not enter S2RAM_STANDBY sleep (err %d)", ret);

		LOG_INF("Resumed from PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY)");

		zassert_true(
			pm_policy_state_lock_is_active(PM_STATE_SOFT_OFF, PM_ALL_SUBSTATES),
			"SOFT OFF should be locked during STANDBY test");

		/* Re-lock deeper states immediately to prevent k_sleep from
		 * re-entering S2RAM during post-resume verification.
		 */
		app_pm_lock_deeper_states(true);

	} else {
		LOG_INF("=== TEST 4: S2RAM_STANDBY SKIPPED (S2RAM not supported) ===");
		ztest_test_skip();
	}

#elif defined(CONFIG_RTSS_HP)

	LOG_INF("Skipping PM_STATE_SUSPEND_TO_RAM (MRAM boot)");
	ztest_test_skip();

#endif
}

/*========================END OF S2RAM_STANDBY TESTS========================*/

/*========================S2RAM_STOP TESTS========================*/

ZTEST(i2c_test_POS_05_S2RAM_STOP, test_POS_01_S2RAM_STOP_basic)
{

	LOG_INF("=== TEST 1: Basic S2RAM_STOP ===");

#if defined(CONFIG_RTSS_HE)

	if (S2RAM_SUPPORTED) {

		app_pm_lock_deeper_states(false);

		LOG_INF("Enter PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) for (%d microseconds)",
			S2RAM_STOP_SLEEP_USEC);

		int ret = app_enter_deep_sleep(S2RAM_STOP_SLEEP_USEC);

		__ASSERT(ret == 0, "Could not enter S2RAM_STOP sleep (err %d)", ret);

		LOG_INF("Resumed from PM_STATE_SUSPEND_TO_RAM (substate 1: STOP)");

		/* Re-lock deeper states immediately to prevent k_sleep from
		 * re-entering S2RAM during post-resume verification.
		 */
		app_pm_lock_deeper_states(true);

	} else {
		LOG_INF("=== TEST 1: S2RAM_STOP SKIPPED (S2RAM not supported) ===");
		ztest_test_skip();
	}

#elif defined(CONFIG_RTSS_HP)

	LOG_INF("Skipping PM_STATE_SUSPEND_TO_RAM (MRAM boot)");
	ztest_test_skip();

#endif
}

ZTEST(i2c_test_POS_05_S2RAM_STOP, test_POS_02_S2RAM_STOP_multiple_cycles)
{

#define STOP_NUM_CYCLES 3
	LOG_INF("=== TEST 2: Multiple S2RAM_STOP cycles (%d) ===", STOP_NUM_CYCLES);

	for (int cycle = 0; cycle < STOP_NUM_CYCLES; cycle++) {
		LOG_INF("Cycle %d of %d", cycle + 1, STOP_NUM_CYCLES);

#if defined(CONFIG_RTSS_HE)

		if (S2RAM_SUPPORTED) {

			app_pm_lock_deeper_states(false);

			LOG_INF("Enter PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) for (%d "
				"microseconds)",
				S2RAM_STOP_SLEEP_USEC);

			int ret = app_enter_deep_sleep(S2RAM_STOP_SLEEP_USEC);

			__ASSERT(ret == 0, "Could not enter S2RAM_STOP sleep (err %d)", ret);

			LOG_INF("Resumed from PM_STATE_SUSPEND_TO_RAM (substate 1: STOP)");

			/* Re-lock deeper states immediately to prevent k_sleep from
			 * re-entering S2RAM during post-resume verification.
			 */
			app_pm_lock_deeper_states(true);

			if (cycle < STOP_NUM_CYCLES - 1) {
				after(NULL);
			}

		} else {
			LOG_INF("=== TEST 2: S2RAM_STOP SKIPPED (S2RAM not supported) ===");
			ztest_test_skip();
		}

#elif defined(CONFIG_RTSS_HP)

		LOG_INF("Skipping PM_STATE_SUSPEND_TO_RAM (MRAM boot)");
		ztest_test_skip();

#endif
	}
}

ZTEST(i2c_test_POS_05_S2RAM_STOP, test_POS_03_S2RAM_STOP_maximum_duration)
{

#define STOP_MAXIMUM_DURATION_USEC (24 * 1000 * 1000)

	LOG_INF("=== TEST 3: S2RAM_STOP maximum duration (%d microseconds) ===",
		STOP_MAXIMUM_DURATION_USEC);

#if defined(CONFIG_RTSS_HE)

	if (S2RAM_SUPPORTED) {

		app_pm_lock_deeper_states(false);

		LOG_INF("Enter PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) for (%d "
			"microseconds)",
			STOP_MAXIMUM_DURATION_USEC);

		int ret = app_enter_deep_sleep(STOP_MAXIMUM_DURATION_USEC);

		__ASSERT(ret == 0, "Could not enter S2RAM_STOP sleep (err %d)", ret);

		LOG_INF("Resumed from PM_STATE_SUSPEND_TO_RAM (substate 1: STOP)");

		/* Re-lock deeper states immediately to prevent k_sleep from
		 * re-entering S2RAM during post-resume verification.
		 */
		app_pm_lock_deeper_states(true);

	} else {
		LOG_INF("=== TEST 3: S2RAM_STOP SKIPPED (S2RAM not supported) ===");
		ztest_test_skip();
	}

#elif defined(CONFIG_RTSS_HP)

	LOG_INF("Skipping PM_STATE_SUSPEND_TO_RAM (MRAM boot)");
	ztest_test_skip();

#endif
}

ZTEST(i2c_test_POS_05_S2RAM_STOP, test_POS_04_S2RAM_STOP_minimum_duration)
{

#define STOP_MINIMUM_DURATION_USEC (22 * 1000 * 1000)

	LOG_INF("=== TEST 4: S2RAM_STOP minimum duration (%d microseconds) ===",
		STOP_MINIMUM_DURATION_USEC);

#if defined(CONFIG_RTSS_HE)

	if (S2RAM_SUPPORTED) {

		app_pm_lock_deeper_states(false);

		LOG_INF("Enter PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) for (%d "
			"microseconds)",
			STOP_MINIMUM_DURATION_USEC);

		int ret = app_enter_deep_sleep(STOP_MINIMUM_DURATION_USEC);

		__ASSERT(ret == 0, "Could not enter S2RAM_STOP sleep (err %d)", ret);

		LOG_INF("Resumed from PM_STATE_SUSPEND_TO_RAM (substate 1: STOP)");

		/* Re-lock deeper states immediately to prevent k_sleep from
		 * re-entering S2RAM during post-resume verification.
		 */
		app_pm_lock_deeper_states(true);

	} else {
		LOG_INF("=== TEST 4: S2RAM_STOP SKIPPED (S2RAM not supported) ===");
		ztest_test_skip();
	}

#elif defined(CONFIG_RTSS_HP)

	LOG_INF("Skipping PM_STATE_SUSPEND_TO_RAM (MRAM boot)");
	ztest_test_skip();

#endif
}

ZTEST(i2c_test_POS_05_S2RAM_STOP, test_POS_05_S2RAM_STANDBY_THEN_S2RAM_STOP)
{

#if defined(CONFIG_RTSS_HE)

	if (S2RAM_SUPPORTED) {

		app_pm_lock_deeper_states(false);

		LOG_INF("=== TEST 5.1: S2RAM_STANDBY ===");

		LOG_INF("Enter PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) for (%d "
			"microseconds)",
			STANDBY_MINIMUM_DURATION_USEC);

		int ret = app_enter_deep_sleep(STANDBY_MINIMUM_DURATION_USEC);

		__ASSERT(ret == 0, "Could not enter S2RAM_STANDBY sleep (err %d)", ret);

		LOG_INF("Resumed from PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY)");

		k_sleep(K_USEC(5));

		LOG_INF("=== TEST 5.2: S2RAM_STOP ===");

		LOG_INF("Enter PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) for (%d "
			"microseconds)",
			STOP_MINIMUM_DURATION_USEC);

		ret = app_enter_deep_sleep(STOP_MINIMUM_DURATION_USEC);
		__ASSERT(ret == 0, "Could not enter S2RAM_STOP sleep (err %d)", ret);

		LOG_INF("Resumed from PM_STATE_SUSPEND_TO_RAM (substate 1: STOP)");

		/* Re-lock deeper states immediately to prevent k_sleep from
		 * re-entering S2RAM during post-resume verification.
		 */
		app_pm_lock_deeper_states(true);

	} else {
		LOG_INF("=== TEST 5.2: S2RAM_STOP SKIPPED (S2RAM not supported) ===");
		ztest_test_skip();
	}

#elif defined(CONFIG_RTSS_HP)

	LOG_INF("Skipping PM_STATE_SUSPEND_TO_RAM (MRAM boot)");
	ztest_test_skip();

#endif
}

/*========================END OF S2RAM_STOP TESTS========================*/

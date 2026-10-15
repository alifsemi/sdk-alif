/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */


#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/init.h>
#include <zephyr/pm/pm.h>
#include <zephyr/pm/policy.h>
#include <zephyr/pm/state.h>
#include <zephyr/drivers/counter.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/sys_io.h>
#include <zephyr/ztest.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/logging/log.h>
#include <zephyr/dt-bindings/power-domain/alif_power_domain.h>
#include <se_service.h>
#include <soc_common.h>

#include "spi_test.h"

LOG_MODULE_REGISTER(spi_pm, LOG_LEVEL_INF);

/* Set to 1 to dump full NVIC ISPR state on wakeup */
#define APP_PM_WAKEUP_DEBUG 0

#if DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(rtc0), snps_dw_apb_rtc, okay)
	#define WAKEUP_SOURCE DT_NODELABEL(rtc0)
#elif DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(timer0), snps_dw_timers, okay)
	#define WAKEUP_SOURCE DT_NODELABEL(timer0)
#else
#error "Wakeup Device not enabled in the dts"
#endif

#define SUSPEND_IDLE_IN_DTS DT_NODE_HAS_STATUS(DT_NODELABEL(suspend_idle), okay)
#define SOFT_OFF_IN_DTS     DT_NODE_HAS_STATUS(DT_NODELABEL(subsys_off), okay)

#define S2RAM_STANDBY_SUBSTATE \
	DT_PROP_OR(DT_NODELABEL(standby_s2ram), substate_id, 0)
#define S2RAM_STOP_SUBSTATE \
	DT_PROP_OR(DT_NODELABEL(stop_s2ram), substate_id, 1)

/*
 * Sleep durations.
 *
 * Upper bound: at 400 MHz, ticks-to-cycles calculation in the PM driver
 * overflows uint32 for values > 10.7 s (2^32 / 400). Every sleep that may
 * enter SUSPEND_TO_IDLE / S2RAM / SOFT_OFF must stay below this ceiling.
 * RUNTIME_IDLE may exceed it because deeper states are locked out.
 *
 * Lower bounds come from the snippet overlays (min-residency-us):
 *   suspend_idle 2.5 ms, standby_s2ram 5 s, stop_s2ram 8 s, subsys_off 9 s.
 */
#define RUNTIME_IDLE_SLEEP_USEC     (10 * 1000 * 1000)
#define RUNTIME_IDLE_MIN_SLEEP_USEC (1 * 1000 * 1000)
#define RUNTIME_IDLE_MAX_SLEEP_USEC (18 * 1000 * 1000)

#define SUSPEND_IDLE_SLEEP_USEC     (10 * 1000)
#define SUSPEND_IDLE_MIN_SLEEP_USEC (5 * 1000)
#define SUSPEND_IDLE_MAX_SLEEP_USEC (10 * 1000 * 1000)

#define S2RAM_STANDBY_SLEEP_USEC     (6 * 1000 * 1000)
#define S2RAM_STANDBY_MIN_SLEEP_USEC (5500 * 1000)
/* STOP is locked in STANDBY cases, so this may exceed stop residency. */
#define S2RAM_STANDBY_MAX_SLEEP_USEC (10 * 1000 * 1000)

#define S2RAM_STOP_SLEEP_USEC     (9 * 1000 * 1000)
#define S2RAM_STOP_MIN_SLEEP_USEC (8500 * 1000)
#define S2RAM_STOP_MAX_SLEEP_USEC (10500 * 1000)
/* Below stop_s2ram min-residency (8 s) but above standby (5 s). */
#define S2RAM_STOP_BELOW_RESIDENCY_USEC (7 * 1000 * 1000)
#define SOFT_OFF_SLEEP_USEC       (10 * 1000 * 1000)

#define NUM_CYCLES 3

#define PM_DEEP_SLEEP_MAX_USEC 10700000U

BUILD_ASSERT(S2RAM_STOP_SLEEP_USEC > S2RAM_STANDBY_SLEEP_USEC,
	"STOP sleep duration must be greater than STANDBY sleep duration");
BUILD_ASSERT(SOFT_OFF_SLEEP_USEC > S2RAM_STOP_SLEEP_USEC,
	"SOFT_OFF sleep duration must be greater than STOP sleep duration");
BUILD_ASSERT(SUSPEND_IDLE_MAX_SLEEP_USEC < PM_DEEP_SLEEP_MAX_USEC &&
	     S2RAM_STANDBY_MAX_SLEEP_USEC < PM_DEEP_SLEEP_MAX_USEC &&
	     S2RAM_STOP_MAX_SLEEP_USEC < PM_DEEP_SLEEP_MAX_USEC &&
	     SOFT_OFF_SLEEP_USEC < PM_DEEP_SLEEP_MAX_USEC,
	"Deep sleep durations must stay below 10.7 s");

/*
 * MRAM base address - used to determine boot location
 * TCM boot: VTOR = 0x0
 * MRAM boot: VTOR >= 0x80000000
 */
#define MRAM_BASE_ADDRESS 0x80000000

#define IS_BOOTING_FROM_MRAM() (SCB->VTOR >= MRAM_BASE_ADDRESS)

/*
 * S2RAM_SUPPORTED — retention-capable sleep is possible on HE TCM boot
 * (TCM has hardware retention).
 *
 * SOFT_OFF_SUPPORTED — mutually exclusive with S2RAM: used when no
 * retained RAM is available (HP, or HE MRAM boot).
 */
#define S2RAM_SUPPORTED \
	(IS_ENABLED(CONFIG_RTSS_HE) && !IS_BOOTING_FROM_MRAM())

#define SOFT_OFF_SUPPORTED (!S2RAM_SUPPORTED)

/* ---------------------------------------------------------------------------
 * PM policy locks
 *
 * Baseline (outside a test case): S2RAM and SOFT_OFF locked, SUSPEND_TO_IDLE
 * allowed. Each helper is idempotent so after() can always restore baseline,
 * even when a test aborts on a failed assertion.
 * ---------------------------------------------------------------------------
 */
static bool deep_unlocked;
static bool s2i_locked;
static bool s2ram_stop_locked;

static int app_pre_kernel_init(void)
{
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_RAM, PM_ALL_SUBSTATES);
	pm_policy_state_lock_get(PM_STATE_SOFT_OFF,       PM_ALL_SUBSTATES);

	return 0;
}
SYS_INIT(app_pre_kernel_init, PRE_KERNEL_2, 0);

/*
 * lock = false unlocks only the deep state that this boot mode supports;
 * the other one stays locked for the whole test run.
 */
static void app_pm_lock_deeper_states(bool lock)
{
	if (lock != deep_unlocked) {
		return;
	}

	if (S2RAM_SUPPORTED) {
		if (lock) {
			pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_RAM,
						 PM_ALL_SUBSTATES);
		} else {
			pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_RAM,
						 PM_ALL_SUBSTATES);
		}
	} else {
		if (lock) {
			pm_policy_state_lock_get(PM_STATE_SOFT_OFF,
						 PM_ALL_SUBSTATES);
		} else {
			pm_policy_state_lock_put(PM_STATE_SOFT_OFF,
						 PM_ALL_SUBSTATES);
		}
	}

	deep_unlocked = !lock;
}

static void app_pm_lock_suspend_idle(bool lock)
{
	if (lock == s2i_locked) {
		return;
	}

	if (lock) {
		pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE,
					 PM_ALL_SUBSTATES);
	} else {
		pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_IDLE,
					 PM_ALL_SUBSTATES);
	}

	s2i_locked = lock;
}

static void app_pm_lock_s2ram_stop(bool lock)
{
	if (lock == s2ram_stop_locked) {
		return;
	}

	if (lock) {
		pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_RAM,
					 S2RAM_STOP_SUBSTATE);
	} else {
		pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_RAM,
					 S2RAM_STOP_SUBSTATE);
	}

	s2ram_stop_locked = lock;
}

static void app_pm_restore_baseline(void)
{
	app_pm_lock_deeper_states(true);
	app_pm_lock_s2ram_stop(false);
	app_pm_lock_suspend_idle(false);
}

/* ---------------------------------------------------------------------------
 * Sleep helpers (from samples/drivers/pm/spi_dw)
 * ---------------------------------------------------------------------------
 */
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
	struct counter_alarm_cfg alarm_cfg = {0};
	int ret;

	alarm_cb_status = 0;
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

/*
 * The SE service drops its run-profile cache on every S2RAM entry and
 * re-applies it from the idle thread in pre_device_resume. After STOP the SE
 * itself is powered off (no SSE700 AON in the off-profile); if that restore
 * fails, the cache stays empty, the Alif power-domain driver can no longer
 * suspend (se_service_get_last_set_run_cfg() -> -ENODATA), and
 * pm_suspend_devices() silently aborts every later S2RAM attempt.
 * Re-seed the cache from thread context, where blocking SE calls are allowed.
 */
static int app_enter_deep_sleep_raw(uint32_t sleep_usec);

static int app_se_run_profile_ensure(const char *where)
{
	run_profile_t runp;
	int ret;

	ret = se_service_get_last_set_run_cfg(&runp);
	if (ret == 0) {
		return 0;
	}

	LOG_WRN("SE run-profile cache empty %s (%d) - restoring", where, ret);

	ret = se_service_get_run_cfg(&runp);
	if (ret) {
		LOG_ERR("se_service_get_run_cfg failed (%d)", ret);
		return ret;
	}

	ret = se_service_set_run_cfg(&runp);
	if (ret) {
		LOG_ERR("se_service_set_run_cfg failed (%d)", ret);
		return ret;
	}

	LOG_INF("SE run profile restored (power_domains 0x%08x)",
		runp.power_domains);
	return 0;
}

static int app_enter_deep_sleep(uint32_t sleep_usec)
{
	int ret = app_se_run_profile_ensure("before deep sleep");

	if (ret) {
		return ret;
	}

	ret = app_enter_deep_sleep_raw(sleep_usec);

	(void)app_se_run_profile_ensure("after deep sleep");
	return ret;
}

static int app_enter_deep_sleep_raw(uint32_t sleep_usec)
{
#if defined(CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER)
	/*
	 * Set a delay more than the min-residency-us configured so that
	 * the sub-system will go to OFF state.
	 */
	k_sleep(K_USEC(sleep_usec));
#else
	const struct device *const wakeup_dev = DEVICE_DT_GET(WAKEUP_SOURCE);
	struct counter_alarm_cfg alarm_cfg = {0};
	int ret;

	alarm_cfg.ticks = counter_us_to_ticks(wakeup_dev, sleep_usec);
	alarm_cfg.callback = alarm_callback_fn;
	ret = counter_set_channel_alarm(wakeup_dev, 0, &alarm_cfg);
	if (ret) {
		LOG_ERR("Failed to set the alarm (err %d)", ret);
		return ret;
	}

	LOG_DBG("Set alarm for %u microseconds", sleep_usec);
	/* The idle thread enters the deep sleep state via the PM framework. */
	k_sleep(K_USEC(sleep_usec));
#endif

	return 0;
}

/* ---------------------------------------------------------------------------
 * PM notifier: logs transitions and counts entries so each test can verify
 * which state was really entered.
 * ---------------------------------------------------------------------------
 */
static const char * const pm_state_names[] = {
	[PM_STATE_ACTIVE]          = "ACTIVE",
	[PM_STATE_RUNTIME_IDLE]    = "RUNTIME_IDLE",
	[PM_STATE_SUSPEND_TO_IDLE] = "SUSPEND_TO_IDLE",
	[PM_STATE_STANDBY]         = "STANDBY",
	[PM_STATE_SUSPEND_TO_RAM]  = "SUSPEND_TO_RAM",
	[PM_STATE_SUSPEND_TO_DISK] = "SUSPEND_TO_DISK",
	[PM_STATE_SOFT_OFF]        = "SOFT_OFF",
};

#define PM_STATE_STR(s) \
	((s) < ARRAY_SIZE(pm_state_names) ? pm_state_names[(s)] : "UNKNOWN")

/*
 * SUSPEND_TO_IDLE is entered on every idle period of the baseline policy.
 * Logging it would keep the deferred log thread busy and wake the core again
 * (endless enter/exit loop), so it is only counted.
 */
#define PM_STATE_IS_LOGGED(s) ((s) != PM_STATE_SUSPEND_TO_IDLE)

static atomic_t pm_enter_cnt[PM_STATE_COUNT];
static atomic_t s2ram_standby_cnt;
static atomic_t s2ram_stop_cnt;

static int32_t pm_cnt(enum pm_state state)
{
	return atomic_get(&pm_enter_cnt[state]);
}

static void pm_counters_reset(void)
{
	for (size_t i = 0; i < ARRAY_SIZE(pm_enter_cnt); i++) {
		atomic_set(&pm_enter_cnt[i], 0);
	}
	atomic_set(&s2ram_standby_cnt, 0);
	atomic_set(&s2ram_stop_cnt, 0);
}

static void pm_notify_entry(enum pm_state state)
{
	const struct pm_state_info *info = pm_state_next_get(0);
	uint8_t substate_id = info ? info->substate_id : 0U;

	if (state < PM_STATE_COUNT) {
		atomic_inc(&pm_enter_cnt[state]);
	}
	if (state == PM_STATE_SUSPEND_TO_RAM) {
		if (substate_id == S2RAM_STANDBY_SUBSTATE) {
			atomic_inc(&s2ram_standby_cnt);
		} else if (substate_id == S2RAM_STOP_SUBSTATE) {
			atomic_inc(&s2ram_stop_cnt);
		}
	}

	if (PM_STATE_IS_LOGGED(state)) {
		LOG_INF("PM enter: %s (substate %u)",
			PM_STATE_STR(state), substate_id);
	}
}

/*
 * With CONFIG_LOG_MODE_DEFERRED, LOG_INF here only writes to the ring buffer
 * — no UART access — so this is safe even before devices are resumed.
 */
static void pm_notify_pre_resume(enum pm_state state)
{
	const struct pm_state_info *info = pm_state_next_get(0);
	uint8_t substate_id = info ? info->substate_id : 0U;
	uint32_t active_exc = SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk;

	if (!PM_STATE_IS_LOGGED(state)) {
		return;
	}

	if (active_exc >= 16U) {
		LOG_INF("PM wakeup: %s (substate %u) IRQ %u",
			PM_STATE_STR(state), substate_id, active_exc - 16U);
	} else if (active_exc != 0U) {
		LOG_INF("PM wakeup: %s (substate %u) exception %u",
			PM_STATE_STR(state), substate_id, active_exc);
	} else {
		LOG_INF("PM wakeup: %s (substate %u)",
			PM_STATE_STR(state), substate_id);
	}
#if APP_PM_WAKEUP_DEBUG
	for (int i = 0; i < 16; i++) {
		if (NVIC->ISPR[i]) {
			LOG_INF("PM wakeup: NVIC ISPR[%d] = 0x%08x",
				i, NVIC->ISPR[i]);
		}
	}
#endif
}

static void pm_notify_exit(enum pm_state state)
{
	const struct pm_state_info *info = pm_state_next_get(0);
	uint8_t substate_id = info ? info->substate_id : 0U;

	if (PM_STATE_IS_LOGGED(state)) {
		LOG_INF("PM exit:  %s (substate %u)",
			PM_STATE_STR(state), substate_id);
	}
}

static struct pm_notifier app_pm_notifier = {
	.state_entry       = pm_notify_entry,
	.pre_device_resume = pm_notify_pre_resume,
	.state_exit        = pm_notify_exit,
};

/* ---------------------------------------------------------------------------
 * Test building blocks
 * ---------------------------------------------------------------------------
 */
static void spi_round(const char *label)
{
	int ret = spi_loopback_run(label);

	zassert_equal(ret, 0, "SPI loopback '%s' failed (%d)", label, ret);
}

static void skip_if_no_suspend_idle(void)
{
#if !defined(CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER)
	LOG_INF("Skipping SUSPEND_TO_IDLE (LPM idle timer off)");
	ztest_test_skip();
#elif !SUSPEND_IDLE_IN_DTS
	LOG_INF("Skipping SUSPEND_TO_IDLE (disabled in DTS)");
	ztest_test_skip();
#endif
}

static void skip_if_no_s2ram(void)
{
	if (!S2RAM_SUPPORTED) {
		LOG_INF("Skipping SUSPEND_TO_RAM (no retained RAM: %s)",
			IS_BOOTING_FROM_MRAM() ? "MRAM" : "TCM");
		ztest_test_skip();
	}
}

static void do_runtime_idle(uint32_t sleep_usec)
{
	int ret;

	spi_round("before RUNTIME_IDLE");

	app_pm_lock_suspend_idle(true);
	pm_counters_reset();

	LOG_INF("Enter RUNTIME_IDLE sleep for (%u microseconds)", sleep_usec);
	ret = app_enter_normal_sleep(sleep_usec);
	app_pm_lock_suspend_idle(false);

	zassert_equal(ret, 0,
		      "Could not enter RUNTIME_IDLE sleep (err %d)", ret);
	zassert_equal(pm_cnt(PM_STATE_SUSPEND_TO_IDLE) +
		      pm_cnt(PM_STATE_SUSPEND_TO_RAM) +
		      pm_cnt(PM_STATE_SOFT_OFF), 0,
		      "A state deeper than RUNTIME_IDLE was entered");
	LOG_INF("Exited from RUNTIME_IDLE sleep");

	spi_round("after RUNTIME_IDLE");
}

static void do_suspend_to_idle(uint32_t sleep_usec)
{
	spi_round("before SUSPEND_TO_IDLE");

	pm_counters_reset();

	LOG_INF("Enter SUSPEND_TO_IDLE for (%u microseconds)", sleep_usec);
	k_sleep(K_USEC(sleep_usec));

	LOG_INF("SUSPEND_TO_IDLE entered %d time(s)",
		pm_cnt(PM_STATE_SUSPEND_TO_IDLE));
	zassert_true(pm_cnt(PM_STATE_SUSPEND_TO_IDLE) > 0,
		     "SUSPEND_TO_IDLE was not entered");
	zassert_equal(pm_cnt(PM_STATE_SUSPEND_TO_RAM) +
		      pm_cnt(PM_STATE_SOFT_OFF), 0,
		      "A state deeper than SUSPEND_TO_IDLE was entered");
	LOG_INF("Woke up from SUSPEND_TO_IDLE");

	spi_round("after SUSPEND_TO_IDLE");
}

static void verify_soft_off_blocked(void)
{
#if SOFT_OFF_IN_DTS
	zassert_true(pm_policy_state_lock_is_active(PM_STATE_SOFT_OFF,
						    PM_ALL_SUBSTATES),
		     "SOFT_OFF must stay locked during S2RAM");
#else
	LOG_INF("SOFT_OFF state disabled in DTS - cannot be entered");
#endif
}

static void do_s2ram_standby(uint32_t sleep_usec, bool check_soft_off)
{
	int ret;

	spi_round("before S2RAM STANDBY");

	app_pm_lock_s2ram_stop(true);
	app_pm_lock_deeper_states(false);

	zassert_false(pm_policy_state_lock_is_active(PM_STATE_SUSPEND_TO_RAM,
						     S2RAM_STANDBY_SUBSTATE),
		      "S2RAM STANDBY is still locked");
	if (check_soft_off) {
		verify_soft_off_blocked();
	}

	pm_counters_reset();

	LOG_INF("Enter SUSPEND_TO_RAM STANDBY for (%u microseconds)",
		sleep_usec);
	ret = app_enter_deep_sleep(sleep_usec);

	app_pm_lock_deeper_states(true);
	app_pm_lock_s2ram_stop(false);

	zassert_equal(ret, 0, "Could not enter SUSPEND_TO_RAM (err %d)", ret);
	LOG_INF("=== Resumed from SUSPEND_TO_RAM STANDBY ===");

	zassert_true(atomic_get(&s2ram_standby_cnt) > 0,
		     "S2RAM STANDBY was not entered");
	zassert_equal(atomic_get(&s2ram_stop_cnt), 0,
		      "STOP entered instead of STANDBY");
	zassert_equal(pm_cnt(PM_STATE_SOFT_OFF), 0, "SOFT_OFF entered");

	if (check_soft_off) {
		verify_soft_off_blocked();
		LOG_INF("PASS: SOFT_OFF blocked during S2RAM STANDBY");
	}

	spi_round("after S2RAM STANDBY");
}

static void do_s2ram_stop(uint32_t sleep_usec, bool check_soft_off)
{
	int ret;

	spi_round("before S2RAM STOP");

	app_pm_lock_deeper_states(false);

	zassert_false(pm_policy_state_lock_is_active(PM_STATE_SUSPEND_TO_RAM,
						     S2RAM_STOP_SUBSTATE),
		      "S2RAM STOP is still locked");
	if (check_soft_off) {
		verify_soft_off_blocked();
	}

	pm_counters_reset();

	LOG_INF("Enter SUSPEND_TO_RAM STOP for (%u microseconds)",
		sleep_usec);
	ret = app_enter_deep_sleep(sleep_usec);

	app_pm_lock_deeper_states(true);

	zassert_equal(ret, 0, "Could not enter SUSPEND_TO_RAM (err %d)", ret);
	LOG_INF("=== Resumed from SUSPEND_TO_RAM STOP ===");

	zassert_true(atomic_get(&s2ram_stop_cnt) > 0,
		     "S2RAM STOP was not entered");
	zassert_equal(atomic_get(&s2ram_standby_cnt), 0,
		      "STANDBY entered instead of STOP");
	zassert_equal(pm_cnt(PM_STATE_SOFT_OFF), 0, "SOFT_OFF entered");

	if (check_soft_off) {
		verify_soft_off_blocked();
		LOG_INF("PASS: SOFT_OFF blocked during S2RAM STOP");
	}

	spi_round("after S2RAM STOP");
}

/* ---------------------------------------------------------------------------
 * ztest fixtures
 * ---------------------------------------------------------------------------
 */
static int spi_init_status = -ENODEV;

static void *suite_setup(void)
{
	static bool banner_done;

	if (!banner_done) {
		banner_done = true;
		pm_notifier_register(&app_pm_notifier);

		if (S2RAM_SUPPORTED) {
			LOG_INF("%s (S2RAM): SPI DW PM test", CONFIG_BOARD);
			LOG_INF("RUNTIME_IDLE, S2I, S2RAM STANDBY, S2RAM STOP");
		} else {
			LOG_INF("%s (SOFT_OFF): SPI DW PM test", CONFIG_BOARD);
			LOG_INF("RUNTIME_IDLE, SUSPEND_TO_IDLE, SOFT_OFF");
		}
		LOG_INF("Boot: %s, VTOR 0x%08x",
			IS_BOOTING_FROM_MRAM() ? "MRAM" : "TCM",
			(uint32_t)SCB->VTOR);
	}

	spi_init_status = spi_test_init();

	return NULL;
}

static void before(void *fixture)
{
	const struct device *const cons =
		DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
	const struct device *const wakeup_dev = DEVICE_DT_GET(WAKEUP_SOURCE);
	int ret;

	ARG_UNUSED(fixture);

	zassert_true(device_is_ready(cons), "%s: device not ready",
		     cons->name);
	zassert_true(device_is_ready(wakeup_dev), "%s: device not ready",
		     wakeup_dev->name);
	zassert_equal(spi_init_status, 0, "SPI loopback init failed (%d)",
		      spi_init_status);

	ret = counter_start(wakeup_dev);
	zassert_true(ret == 0 || ret == -EALREADY,
		     "Failed to start counter (err %d)", ret);

	(void)app_se_run_profile_ensure("pre-test");

	app_pm_restore_baseline();
}

static void after(void *fixture)
{
	ARG_UNUSED(fixture);

	app_pm_restore_baseline();
}

/* ===========================================================================
 * Suite 1: RUNTIME_IDLE
 * ===========================================================================
 */
ZTEST(pm_POS_01_Runtime_idle_testing_for_SPI,
	test_SPI_POS_01_runtime_idle_basic_test)
{
	LOG_INF("=== TEST 1: Basic RUNTIME_IDLE ===");
	do_runtime_idle(RUNTIME_IDLE_SLEEP_USEC);
}

ZTEST(pm_POS_01_Runtime_idle_testing_for_SPI,
	test_SPI_POS_02_runtime_idle_multiple_cycles_test)
{
	LOG_INF("=== TEST 2: Multiple RUNTIME_IDLE Cycles (%d) ===",
		NUM_CYCLES);

	for (int cycle = 0; cycle < NUM_CYCLES; cycle++) {
		LOG_INF("--- Cycle %d of %d ---", cycle + 1, NUM_CYCLES);
		do_runtime_idle(RUNTIME_IDLE_SLEEP_USEC);
	}
}

ZTEST(pm_POS_01_Runtime_idle_testing_for_SPI,
	test_SPI_POS_03_runtime_idle_min_sleep)
{
	LOG_INF("=== TEST 3: RUNTIME_IDLE Minimum Sleep (%d microseconds) ===",
		RUNTIME_IDLE_MIN_SLEEP_USEC);
	do_runtime_idle(RUNTIME_IDLE_MIN_SLEEP_USEC);
}

ZTEST(pm_POS_01_Runtime_idle_testing_for_SPI,
	test_SPI_POS_04_runtime_idle_max_sleep)
{
	LOG_INF("=== TEST 4: RUNTIME_IDLE Maximum Sleep (%d microseconds) ===",
		RUNTIME_IDLE_MAX_SLEEP_USEC);
	do_runtime_idle(RUNTIME_IDLE_MAX_SLEEP_USEC);
}

/* ===========================================================================
 * Suite 2: SUSPEND_TO_IDLE
 * ===========================================================================
 */
ZTEST(pm_POS_02_Suspend_to_idle_testing_for_SPI,
	test_SPI_POS_01_suspend_to_idle_basic)
{
	LOG_INF("=== TEST 1: SUSPEND_TO_IDLE Basic ===");
	skip_if_no_suspend_idle();
	do_suspend_to_idle(SUSPEND_IDLE_SLEEP_USEC);
}

ZTEST(pm_POS_02_Suspend_to_idle_testing_for_SPI,
	test_SPI_POS_02_suspend_to_idle_multiple_cycles)
{
	LOG_INF("=== TEST 2: SUSPEND_TO_IDLE for (%d cycles) ===", NUM_CYCLES);
	skip_if_no_suspend_idle();

	for (int cycle = 0; cycle < NUM_CYCLES; cycle++) {
		LOG_INF("--- Cycle %d of %d ---", cycle + 1, NUM_CYCLES);
		do_suspend_to_idle(SUSPEND_IDLE_SLEEP_USEC);
	}
}

ZTEST(pm_POS_02_Suspend_to_idle_testing_for_SPI,
	test_SPI_POS_03_suspend_to_idle_min_duration)
{
	LOG_INF("=== TEST 3: SUSPEND_TO_IDLE min sleep (%d us) ===",
		SUSPEND_IDLE_MIN_SLEEP_USEC);
	skip_if_no_suspend_idle();
	do_suspend_to_idle(SUSPEND_IDLE_MIN_SLEEP_USEC);
}

ZTEST(pm_POS_02_Suspend_to_idle_testing_for_SPI,
	test_SPI_POS_04_suspend_to_idle_max_duration)
{
	LOG_INF("=== TEST 4: SUSPEND_TO_IDLE max sleep (%d us) ===",
		SUSPEND_IDLE_MAX_SLEEP_USEC);
	skip_if_no_suspend_idle();
	do_suspend_to_idle(SUSPEND_IDLE_MAX_SLEEP_USEC);
}

ZTEST(pm_POS_02_Suspend_to_idle_testing_for_SPI,
	test_SPI_POS_05_Runtime_Then_suspend_to_idle_sequence_test)
{
	LOG_INF("=== TEST 5: RUNTIME_IDLE then SUSPEND_TO_IDLE ===");
	skip_if_no_suspend_idle();

	do_runtime_idle(RUNTIME_IDLE_SLEEP_USEC);
	k_sleep(K_MSEC(50));
	do_suspend_to_idle(SUSPEND_IDLE_SLEEP_USEC);
}

/* ===========================================================================
 * Suite 4: S2RAM STANDBY (retained RAM builds: HE TCM)
 * ===========================================================================
 */
ZTEST(pm_POS_04_STANDBY_testing_for_SPI, test_SPI_POS_01_s2ram_standby_basic)
{
	LOG_INF("=== TEST 1: Basic S2RAM STANDBY ===");
	skip_if_no_s2ram();
	do_s2ram_standby(S2RAM_STANDBY_SLEEP_USEC, false);
}

ZTEST(pm_POS_04_STANDBY_testing_for_SPI,
	test_SPI_POS_02_s2ram_standby_multiple_cycles)
{
	LOG_INF("=== TEST 2: Multiple S2RAM STANDBY Cycles (%d) ===",
		NUM_CYCLES);
	skip_if_no_s2ram();

	for (int cycle = 0; cycle < NUM_CYCLES; cycle++) {
		LOG_INF("--- Cycle %d of %d ---", cycle + 1, NUM_CYCLES);
		do_s2ram_standby(S2RAM_STANDBY_SLEEP_USEC, false);
	}
}

ZTEST(pm_POS_04_STANDBY_testing_for_SPI,
	test_SPI_POS_03_runtime_idle_then_s2ram_standby)
{
	LOG_INF("=== TEST 3: RUNTIME_IDLE then S2RAM STANDBY ===");
	skip_if_no_s2ram();

	do_runtime_idle(RUNTIME_IDLE_SLEEP_USEC);
	k_sleep(K_MSEC(50));
	do_s2ram_standby(S2RAM_STANDBY_SLEEP_USEC, false);
}

ZTEST(pm_POS_04_STANDBY_testing_for_SPI,
	test_SPI_POS_04_s2ram_standby_min_sleep)
{
	LOG_INF("=== TEST 4: S2RAM STANDBY min sleep (%d us) ===",
		S2RAM_STANDBY_MIN_SLEEP_USEC);
	skip_if_no_s2ram();
	do_s2ram_standby(S2RAM_STANDBY_MIN_SLEEP_USEC, false);
}

ZTEST(pm_POS_04_STANDBY_testing_for_SPI,
	test_SPI_POS_05_s2ram_standby_max_sleep)
{
	LOG_INF("=== TEST 5: S2RAM STANDBY max sleep (%d us) ===",
		S2RAM_STANDBY_MAX_SLEEP_USEC);
	skip_if_no_s2ram();
	do_s2ram_standby(S2RAM_STANDBY_MAX_SLEEP_USEC, false);
}

ZTEST(pm_POS_04_STANDBY_testing_for_SPI,
	test_SPI_POS_06_s2ram_standby_no_soft_off)
{
	LOG_INF("=== TEST 6: SOFT_OFF blocked during S2RAM STANDBY ===");
	skip_if_no_s2ram();

	zassert_false(SOFT_OFF_SUPPORTED,
		      "SOFT_OFF must not be used with retained RAM");
	do_s2ram_standby(S2RAM_STANDBY_SLEEP_USEC, true);
}

/* ===========================================================================
 * Suite 5: S2RAM STOP
 * ===========================================================================
 */
ZTEST(pm_POS_05_STOP_testing_for_SPI, test_SPI_POS_01_s2ram_stop_basic)
{
	LOG_INF("=== TEST 1: Basic S2RAM STOP ===");
	skip_if_no_s2ram();
	do_s2ram_stop(S2RAM_STOP_SLEEP_USEC, false);
}

ZTEST(pm_POS_05_STOP_testing_for_SPI,
	test_SPI_POS_02_s2ram_stop_multiple_cycles)
{
	LOG_INF("=== TEST 2: Multiple S2RAM STOP Cycles (%d) ===", NUM_CYCLES);
	skip_if_no_s2ram();

	for (int cycle = 0; cycle < NUM_CYCLES; cycle++) {
		LOG_INF("--- Cycle %d of %d ---", cycle + 1, NUM_CYCLES);
		do_s2ram_stop(S2RAM_STOP_SLEEP_USEC, false);
	}
}

ZTEST(pm_POS_05_STOP_testing_for_SPI,
	test_SPI_POS_03_runtime_idle_then_s2ram_stop)
{
	LOG_INF("=== TEST 3: RUNTIME_IDLE then S2RAM STOP ===");
	skip_if_no_s2ram();

	do_runtime_idle(RUNTIME_IDLE_SLEEP_USEC);
	k_sleep(K_MSEC(50));
	do_s2ram_stop(S2RAM_STOP_SLEEP_USEC, false);
}

ZTEST(pm_POS_05_STOP_testing_for_SPI, test_SPI_POS_04_s2ram_stop_min_sleep)
{
	LOG_INF("=== TEST 4: S2RAM STOP min sleep (%d us) ===",
		S2RAM_STOP_MIN_SLEEP_USEC);
	skip_if_no_s2ram();
	do_s2ram_stop(S2RAM_STOP_MIN_SLEEP_USEC, false);
}

ZTEST(pm_POS_05_STOP_testing_for_SPI, test_SPI_POS_05_s2ram_stop_max_sleep)
{
	LOG_INF("=== TEST 5: S2RAM STOP max sleep (%d us) ===",
		S2RAM_STOP_MAX_SLEEP_USEC);
	skip_if_no_s2ram();
	do_s2ram_stop(S2RAM_STOP_MAX_SLEEP_USEC, false);
}

ZTEST(pm_POS_05_STOP_testing_for_SPI, test_SPI_POS_06_s2ram_stop_no_soft_off)
{
	LOG_INF("=== TEST 6: Verify SOFT_OFF Blocked During S2RAM STOP ===");
	skip_if_no_s2ram();

	zassert_false(SOFT_OFF_SUPPORTED,
		      "SOFT_OFF must not be used with retained RAM");
	do_s2ram_stop(S2RAM_STOP_SLEEP_USEC, true);
}

ZTEST(pm_POS_05_STOP_testing_for_SPI, test_SPI_POS_07_s2ram_standby_then_stop)
{
	LOG_INF("=== TEST 7: S2RAM STANDBY then S2RAM STOP ===");
	skip_if_no_s2ram();

	do_s2ram_standby(S2RAM_STANDBY_SLEEP_USEC, false);
	k_sleep(K_MSEC(50));
	do_s2ram_stop(S2RAM_STOP_SLEEP_USEC, false);
}

/*
 * Both substates unlocked, sleep shorter than stop_s2ram min-residency:
 * the policy must pick STANDBY, not STOP.
 */
ZTEST(pm_POS_05_STOP_testing_for_SPI,
	test_SPI_POS_08_s2ram_stop_below_min_residency)
{
	int ret;

	LOG_INF("=== TEST 8: below STOP residency (%d us) => STANDBY ===",
		S2RAM_STOP_BELOW_RESIDENCY_USEC);
	skip_if_no_s2ram();

	spi_round("before S2RAM (below STOP residency)");

	app_pm_lock_deeper_states(false);
	zassert_false(pm_policy_state_lock_is_active(PM_STATE_SUSPEND_TO_RAM,
						     S2RAM_STOP_SUBSTATE),
		      "S2RAM STOP is still locked");
	pm_counters_reset();

	LOG_INF("Request PM_STATE_SUSPEND_TO_RAM for (%d microseconds)",
		S2RAM_STOP_BELOW_RESIDENCY_USEC);
	ret = app_enter_deep_sleep(S2RAM_STOP_BELOW_RESIDENCY_USEC);
	app_pm_lock_deeper_states(true);

	zassert_equal(ret, 0, "Could not enter SUSPEND_TO_RAM (err %d)", ret);
	zassert_equal(atomic_get(&s2ram_stop_cnt), 0,
		      "STOP entered below min-residency");
	zassert_true(atomic_get(&s2ram_standby_cnt) > 0,
		     "STANDBY was not entered");
	LOG_INF("PASS: policy selected STANDBY below STOP min-residency");

	spi_round("after S2RAM (below STOP residency)");

	LOG_INF("=== POWER STATE SEQUENCE COMPLETED ===");
}

/* ===========================================================================
 * Suite 3: SOFT_OFF (no retained RAM: HE MRAM, HP MRAM)
 *
 * Wakeup from SOFT_OFF is a full reset, so the whole test run restarts.
 * ===========================================================================
 */
ZTEST(pm_POS_03_SOFT_OFF__testing_for_SPI, test_SPI_POS_01_soft_off_basic)
{
	int ret;

	LOG_INF("=== TEST 1: Basic SOFT_OFF ===");

	if (!SOFT_OFF_SUPPORTED) {
		LOG_INF("Skipping SOFT_OFF (retained RAM, using S2RAM)");
		ztest_test_skip();
	}

	spi_round("before SOFT_OFF");

	app_pm_lock_deeper_states(false);

	LOG_INF("Enter PM_STATE_SOFT_OFF for (%d us)", SOFT_OFF_SLEEP_USEC);
	LOG_INF("Note: SOFT_OFF has no retention; reset on wakeup");
	ret = app_enter_deep_sleep(SOFT_OFF_SLEEP_USEC);
	app_pm_lock_deeper_states(true);

	zassert_equal(ret, 0, "Could not enter SOFT_OFF (err %d)", ret);
	LOG_ERR("ERROR: Resumed after SOFT_OFF - should not happen");
	zassert_unreachable("PM_STATE_SOFT_OFF should have caused a reset");
}

ZTEST_SUITE(pm_POS_01_Runtime_idle_testing_for_SPI, NULL,
	    suite_setup, before, after, NULL);
ZTEST_SUITE(pm_POS_02_Suspend_to_idle_testing_for_SPI, NULL,
	    suite_setup, before, after, NULL);
ZTEST_SUITE(pm_POS_04_STANDBY_testing_for_SPI, NULL,
	    suite_setup, before, after, NULL);
ZTEST_SUITE(pm_POS_05_STOP_testing_for_SPI, NULL,
	    suite_setup, before, after, NULL);
ZTEST_SUITE(pm_POS_03_SOFT_OFF__testing_for_SPI, NULL,
	    suite_setup, before, after, NULL);

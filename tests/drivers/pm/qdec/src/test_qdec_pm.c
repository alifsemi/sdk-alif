/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/*
 * Ztest for the QDEC power-management sample.
 *
 * HE TCM (snippet qdec-pm-s2ram-tcm, flash at 0x0):
 *   RUNTIME_IDLE, SUSPEND_TO_IDLE, S2RAM STANDBY, S2RAM STOP.
 *   The core resumes in place. STANDBY retains the UTIMER count.
 *   STOP powers that domain down, so the count starts again at 0.
 *
 * HE or HP MRAM (snippet qdec-pm-mram):
 *   RUNTIME_IDLE, SUSPEND_TO_IDLE when the idle timer exists, then SOFT_OFF.
 *   SOFT_OFF resets the core. A marker in the MRAM storage partition survives
 *   that reset, so the next boot stays up instead of entering SOFT_OFF again.
 *   HP skips SUSPEND_TO_IDLE (no idle timer).
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/pm/pm.h>
#include <zephyr/pm/policy.h>
#include <zephyr/drivers/counter.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sys/util.h>
#include <zephyr/logging/log.h>
#include <zephyr/ztest.h>

LOG_MODULE_REGISTER(qdec_pm, LOG_LEVEL_INF);

#define QDEC_NODE DT_ALIAS(qdec0)
#define QDEC_SAMPLES 5
#define QENC_STEPS_PER_SAMPLE 25
#define QENC_PHASE_DELAY_US 100
#define QDEC_STEP_DEG 36

#define RUNTIME_IDLE_SLEEP_USEC (18 * 1000 * 1000)
#define SUSPEND_IDLE_SLEEP_USEC (10 * 1000)
#define S2RAM_STANDBY_SLEEP_USEC (6 * 1000 * 1000)
#define S2RAM_STOP_SLEEP_USEC (9 * 1000 * 1000)
#define SOFT_OFF_SLEEP_USEC (10 * 1000 * 1000)
#define STANDBY_REPEAT 3

#define MRAM_BASE_ADDRESS 0x80000000
#define IS_BOOTING_FROM_MRAM() (SCB->VTOR >= MRAM_BASE_ADDRESS)
#define S2RAM_SUPPORTED (IS_ENABLED(CONFIG_RTSS_HE) && !IS_BOOTING_FROM_MRAM())
#define SOFT_OFF_SUPPORTED (!S2RAM_SUPPORTED)

BUILD_ASSERT(S2RAM_STOP_SLEEP_USEC > S2RAM_STANDBY_SLEEP_USEC,
	     "STOP sleep must be longer than STANDBY sleep");
BUILD_ASSERT(SOFT_OFF_SLEEP_USEC > S2RAM_STOP_SLEEP_USEC,
	     "SOFT_OFF sleep must be longer than STOP sleep");

#if DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(rtc0), snps_dw_apb_rtc, okay)
#define WAKEUP_SOURCE DT_NODELABEL(rtc0)
#elif DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(timer0), snps_dw_timers, okay)
#define WAKEUP_SOURCE DT_NODELABEL(timer0)
#else
#error "Wakeup device not enabled in the devicetree"
#endif

static const struct device *qdec_dev = DEVICE_DT_GET(QDEC_NODE);
static const struct device *wakeup_dev = DEVICE_DT_GET(WAKEUP_SOURCE);

static const struct gpio_dt_spec phase_a = GPIO_DT_SPEC_GET(DT_ALIAS(qenca), gpios);
static const struct gpio_dt_spec phase_b = GPIO_DT_SPEC_GET(DT_ALIAS(qencb), gpios);

static uint8_t pm_s2ram_substate;
static bool hw_ready;
static bool soft_off_wakeup;

/*
 * SOFT_OFF drops RAM. The storage partition sits in MRAM and is mapped
 * writable, so one 16-byte marker survives the reset.
 */
#define SOFT_OFF_MAGIC 0x510F00F1U

static volatile uint32_t *soft_off_slot(void)
{
	return (volatile uint32_t *)DT_FIXED_PARTITION_ADDR(DT_NODELABEL(storage_partition));
}

static void soft_off_slot_write(uint32_t value)
{
	volatile uint32_t *dst = soft_off_slot();

	__disable_irq();
	dst[0] = value;
	dst[1] = value;
	dst[2] = value;
	dst[3] = value;
	__DSB();
	__ISB();
	__enable_irq();
}

static bool soft_off_slot_is_set(void)
{
	volatile uint32_t *dst = soft_off_slot();

	return dst[0] == SOFT_OFF_MAGIC && dst[1] == SOFT_OFF_MAGIC;
}

static int qenc_emulate_init(void)
{
	int ret;

	if (!gpio_is_ready_dt(&phase_a) || !gpio_is_ready_dt(&phase_b)) {
		return -ENODEV;
	}

	ret = gpio_pin_configure_dt(&phase_a, GPIO_OUTPUT_INACTIVE);
	if (ret != 0) {
		return ret;
	}

	ret = gpio_pin_configure_dt(&phase_b, GPIO_OUTPUT_INACTIVE);
	if (ret != 0) {
		return ret;
	}

	LOG_INF("GPIO encoder emulator ready");
	return 0;
}

static void qenc_emulate_step(void)
{
	gpio_pin_set_dt(&phase_a, 1);
	k_busy_wait(QENC_PHASE_DELAY_US);
	gpio_pin_set_dt(&phase_b, 1);
	k_busy_wait(QENC_PHASE_DELAY_US);
	gpio_pin_set_dt(&phase_a, 0);
	k_busy_wait(QENC_PHASE_DELAY_US);
	gpio_pin_set_dt(&phase_b, 0);
	k_busy_wait(QENC_PHASE_DELAY_US);
}

static void qenc_emulate_burst(void)
{
	for (int i = 0; i < QENC_STEPS_PER_SAMPLE; i++) {
		qenc_emulate_step();
	}
}

static int qdec_read_deg(int *deg)
{
	struct sensor_value val;
	int ret;

	ret = sensor_sample_fetch(qdec_dev);
	if (ret != 0) {
		return ret;
	}

	ret = sensor_channel_get(qdec_dev, SENSOR_CHAN_ROTATION, &val);
	if (ret != 0) {
		return ret;
	}

	*deg = val.val1;
	return 0;
}

/* 25 quadrature steps are 36 degrees at 1000 counts per revolution. */
static void expect_series(int first_deg, const char *label)
{
	int expect = first_deg;

	LOG_INF("=== %s: QDEC read ===", label);

	for (int i = 0; i < QDEC_SAMPLES; i++) {
		int pos;
		int ret;

		qenc_emulate_burst();
		ret = qdec_read_deg(&pos);
		zassert_equal(ret, 0, "QDEC read failed (%d)", ret);
		LOG_INF("sample %d/%d: position %d deg", i + 1, QDEC_SAMPLES, pos);
		zassert_equal(pos, expect,
			      "%s sample %d: got %d deg, expected %d. "
			      "Fit the QDEC jumpers (E7/E8 P0_0-P0_2 and P0_1-P0_3, "
			      "E1C/B1 P2_4-P6_2 and P2_5-P6_3)",
			      label, i + 1, pos, expect);
		expect = (expect + QDEC_STEP_DEG) % 360;
	}
}

static void app_pm_lock_deeper_states(bool lock)
{
	if (lock) {
		pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_RAM, PM_ALL_SUBSTATES);
		pm_policy_state_lock_get(PM_STATE_SOFT_OFF, PM_ALL_SUBSTATES);
	} else if (S2RAM_SUPPORTED) {
		pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_RAM, PM_ALL_SUBSTATES);
	} else {
		pm_policy_state_lock_put(PM_STATE_SOFT_OFF, PM_ALL_SUBSTATES);
	}
}

static int app_pre_kernel_init(void)
{
	app_pm_lock_deeper_states(true);
	return 0;
}
SYS_INIT(app_pre_kernel_init, PRE_KERNEL_2, 0);

#if !defined(CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER)
static volatile uint32_t alarm_cb_status;

static void alarm_callback_fn(const struct device *dev, uint8_t chan_id, uint32_t ticks,
			      void *user_data)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(chan_id);
	ARG_UNUSED(ticks);
	ARG_UNUSED(user_data);
	alarm_cb_status = 1;
}
#endif

static int app_enter_normal_sleep(uint32_t sleep_usec)
{
#if defined(CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER)
	k_sleep(K_USEC(sleep_usec));
#else
	struct counter_alarm_cfg alarm_cfg;
	int ret;

	alarm_cfg.flags = 0;
	alarm_cfg.ticks = counter_us_to_ticks(wakeup_dev, sleep_usec);
	alarm_cfg.callback = alarm_callback_fn;
	alarm_cfg.user_data = &alarm_cfg;

	ret = counter_set_channel_alarm(wakeup_dev, 0, &alarm_cfg);
	if (ret != 0) {
		return ret;
	}

	k_sleep(K_USEC(sleep_usec));

	if (alarm_cb_status == 0U) {
		return -ETIMEDOUT;
	}
	alarm_cb_status = 0;
#endif
	return 0;
}

static int app_enter_deep_sleep(uint32_t sleep_usec)
{
#if defined(CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER)
	k_sleep(K_USEC(sleep_usec));
#else
	struct counter_alarm_cfg alarm_cfg = { 0 };
	int ret;

	alarm_cfg.ticks = counter_us_to_ticks(wakeup_dev, sleep_usec);
	ret = counter_set_channel_alarm(wakeup_dev, 0, &alarm_cfg);
	if (ret != 0) {
		return ret;
	}

	k_sleep(K_USEC(sleep_usec));
#endif
	return 0;
}

static void pm_notify_entry(enum pm_state state)
{
	const struct pm_state_info *info = pm_state_next_get(0);

	/*
	 * Capture only the S2RAM entry. A later idle exit must not overwrite it,
	 * and GPIO is not usable in this callback (configure returns -EAGAIN).
	 */
	if (state == PM_STATE_SUSPEND_TO_RAM && info != NULL) {
		pm_s2ram_substate = info->substate_id;
		LOG_INF("PM enter: SUSPEND_TO_RAM (substate %u)", info->substate_id);
	}
}

static struct pm_notifier app_pm_notifier = {
	.state_entry = pm_notify_entry,
};

static void *suite_setup(void)
{
	int ret;

	if (hw_ready) {
		return NULL;
	}

	soft_off_wakeup = SOFT_OFF_SUPPORTED && soft_off_slot_is_set();
	if (soft_off_wakeup) {
		LOG_INF("SOFT_OFF wakeup seen. Staying up");
		pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
		hw_ready = true;
		return NULL;
	}

	zassert_true(device_is_ready(qdec_dev), "QDEC device not ready");
	zassert_true(device_is_ready(wakeup_dev), "wakeup counter not ready");

	ret = qenc_emulate_init();
	zassert_equal(ret, 0, "GPIO encoder emulator init failed (%d)", ret);

	pm_notifier_register(&app_pm_notifier);

	ret = counter_start(wakeup_dev);
	zassert_true(ret == 0 || ret == -EALREADY, "counter_start failed (%d)", ret);

	hw_ready = true;
	return NULL;
}

static void enter_s2ram(uint32_t sleep_usec, uint8_t substate, const char *name)
{
	int64_t t0;
	int64_t dt;
	int ret;

	pm_s2ram_substate = 0xff;
	/* Unlock only this substate. The other S2RAM substate and SOFT_OFF stay locked. */
	pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_RAM, substate);

	LOG_INF("Enter PM_STATE_SUSPEND_TO_RAM (substate %u: %s) for (%u microseconds)",
		substate, name, sleep_usec);
	t0 = k_uptime_get();
	ret = app_enter_deep_sleep(sleep_usec);
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_RAM, substate);
	dt = k_uptime_get() - t0;
	zassert_equal(ret, 0, "deep sleep failed (%d)", ret);
	zassert_true(dt > (int64_t)sleep_usec / 1000 - 1500 &&
			     dt < (int64_t)sleep_usec / 1000 + 1500,
		     "%s sleep was %lld ms", name, dt);

	LOG_INF("=== Resumed from PM_STATE_SUSPEND_TO_RAM (substate %u: %s) ===",
		substate, name);
	zassert_equal(pm_s2ram_substate, substate, "%s entered substate %u", name,
		      pm_s2ram_substate);

	/* gpio_dw has no PM callbacks. Reconfigure the emulator once the CPU is back. */
	ret = qenc_emulate_init();
	zassert_equal(ret, 0, "GPIO emulator re-init failed after %s (%d)", name, ret);
}

ZTEST(qdec_pm_a_common, test_qdec_pm_boot)
{
	if (soft_off_wakeup) {
		LOG_INF("SOFT_OFF wakeup: previous boot reset the core. Staying up");
		return;
	}

	LOG_INF("VTOR 0x%08x", (uint32_t)SCB->VTOR);

	if (S2RAM_SUPPORTED) {
		LOG_INF("%s (S2RAM): QDEC PM demo "
			"(RUNTIME_IDLE, SUSPEND_TO_IDLE, S2RAM STANDBY, S2RAM STOP)",
			CONFIG_BOARD);
		LOG_INF("  3. PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY)");
		LOG_INF("  4. PM_STATE_SUSPEND_TO_RAM (substate 1: STOP)");
		zassert_false(IS_BOOTING_FROM_MRAM(), "TCM image booted with an MRAM vector table");
	} else {
		LOG_INF("%s (SOFT_OFF): QDEC PM demo "
			"(RUNTIME_IDLE, SUSPEND_TO_IDLE, SOFT_OFF)",
			CONFIG_BOARD);
		LOG_INF("  3. PM_STATE_SOFT_OFF");
		zassert_true(IS_BOOTING_FROM_MRAM() || IS_ENABLED(CONFIG_RTSS_HP),
			     "SOFT_OFF path on a TCM HE boot");
	}
}

ZTEST(qdec_pm_a_common, test_qdec_pm_position_before_idle)
{
	if (soft_off_wakeup) {
		ztest_test_skip();
	}

	expect_series(QDEC_STEP_DEG, "before RUNTIME_IDLE");
}

ZTEST(qdec_pm_a_common, test_qdec_pm_runtime_idle)
{
	int64_t t0;

	if (soft_off_wakeup) {
		ztest_test_skip();
	}
	int64_t dt;
	int pos;
	int ret;

	ret = qdec_read_deg(&pos);
	zassert_equal(ret, 0, "QDEC read failed (%d)", ret);
	zassert_equal(pos, 180, "position changed before RUNTIME_IDLE (%d)", pos);

	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);

	LOG_INF("Enter RUNTIME_IDLE sleep for (%d microseconds)", RUNTIME_IDLE_SLEEP_USEC);
	t0 = k_uptime_get();
	ret = app_enter_normal_sleep(RUNTIME_IDLE_SLEEP_USEC);
	dt = k_uptime_get() - t0;
	zassert_equal(ret, 0, "RUNTIME_IDLE failed (%d)", ret);
	zassert_true(dt >= 17000 && dt <= 20000, "RUNTIME_IDLE was %lld ms", dt);
	LOG_INF("Exited from RUNTIME_IDLE sleep");

	pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);

	ret = qdec_read_deg(&pos);
	zassert_equal(ret, 0, "QDEC read failed (%d)", ret);
	zassert_equal(pos, 180, "QDEC count changed across RUNTIME_IDLE (%d)", pos);
}

ZTEST(qdec_pm_a_common, test_qdec_pm_suspend_to_idle)
{
	if (soft_off_wakeup) {
		ztest_test_skip();
	}

#if !defined(CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER)
	LOG_INF("PM_STATE_SUSPEND_TO_IDLE (skipped - LPM timer not enabled)");
	ztest_test_skip();
#else
	int64_t t0;
	int64_t dt;

	LOG_INF("Enter PM_STATE_SUSPEND_TO_IDLE for (%d microseconds)",
		SUSPEND_IDLE_SLEEP_USEC);
	t0 = k_uptime_get();
	k_sleep(K_USEC(SUSPEND_IDLE_SLEEP_USEC));
	dt = k_uptime_get() - t0;
	LOG_INF("Exited from PM_STATE_SUSPEND_TO_IDLE");
	zassert_true(dt < 1000, "SUSPEND_TO_IDLE took %lld ms", dt);

	/* UTIMER stays powered, so the count continues from 180 and wraps. */
	expect_series(216, "after SUSPEND_TO_IDLE");
#endif
}

ZTEST(qdec_pm_b_standby, test_qdec_pm_s2ram_standby)
{
	if (!S2RAM_SUPPORTED) {
		ztest_test_skip();
	}

	enter_s2ram(S2RAM_STANDBY_SLEEP_USEC, 0, "STANDBY");
	/*
	 * SUSPEND_TO_IDLE left the counter at 0, and STANDBY keeps UTIMER
	 * powered, so the next sample is 36. A cleared counter looks the same.
	 */
	expect_series(QDEC_STEP_DEG, "after S2RAM STANDBY");
}

ZTEST(qdec_pm_c_stop, test_qdec_pm_s2ram_stop)
{
	if (!S2RAM_SUPPORTED) {
		ztest_test_skip();
	}

	enter_s2ram(S2RAM_STOP_SLEEP_USEC, 1, "STOP");
	/* STOP drops the UTIMER domain, so the count starts over. */
	expect_series(QDEC_STEP_DEG, "after S2RAM STOP");
}

ZTEST(qdec_pm_d_repeat, test_qdec_pm_s2ram_repeat)
{
	/* STOP's last sample is 180. STANDBY does not clear that count. */
	int next = 180 + QDEC_STEP_DEG;

	if (!S2RAM_SUPPORTED) {
		ztest_test_skip();
	}

	for (int cycle = 0; cycle < STANDBY_REPEAT; cycle++) {
		LOG_INF("STANDBY repeat %d/%d", cycle + 1, STANDBY_REPEAT);
		enter_s2ram(S2RAM_STANDBY_SLEEP_USEC, 0, "STANDBY");
		expect_series(next, "after S2RAM STANDBY");
		next = (next + QDEC_SAMPLES * QDEC_STEP_DEG) % 360;
	}
}

ZTEST(qdec_pm_e_stays_up, test_qdec_pm_stays_up)
{
	int64_t t0;

	if (!S2RAM_SUPPORTED) {
		ztest_test_skip();
	}
	int64_t dt;

	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
	t0 = k_uptime_get();
	k_sleep(K_SECONDS(2));
	dt = k_uptime_get() - t0;
	pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);

	zassert_true(dt >= 1500 && dt <= 4000, "idle after the sequence was %lld ms", dt);
	LOG_INF("=== QDEC PM SEQUENCE COMPLETED ===");
}

ZTEST(qdec_pm_z_soft_off, test_qdec_pm_soft_off)
{
	int ret;

	if (!SOFT_OFF_SUPPORTED) {
		ztest_test_skip();
	}

	if (soft_off_wakeup) {
		soft_off_slot_write(0);
		LOG_INF("=== QDEC PM SEQUENCE COMPLETED ===");
		return;
	}

	soft_off_slot_write(SOFT_OFF_MAGIC);
	LOG_INF("Enter PM_STATE_SOFT_OFF for (%d microseconds)", SOFT_OFF_SLEEP_USEC);
	LOG_INF("Note: SOFT_OFF has no retention - system will reset on wakeup");

	app_pm_lock_deeper_states(false);
	ret = app_enter_deep_sleep(SOFT_OFF_SLEEP_USEC);
	zassert_equal(ret, 0, "SOFT_OFF entry failed (%d)", ret);

	LOG_ERR("ERROR: Resumed after PM_STATE_SOFT_OFF - this should not happen!");
	zassert_unreachable("PM_STATE_SOFT_OFF should have caused a reset");
}

ZTEST_SUITE(qdec_pm_a_common, NULL, suite_setup, NULL, NULL, NULL);
ZTEST_SUITE(qdec_pm_b_standby, NULL, suite_setup, NULL, NULL, NULL);
ZTEST_SUITE(qdec_pm_c_stop, NULL, suite_setup, NULL, NULL, NULL);
ZTEST_SUITE(qdec_pm_d_repeat, NULL, suite_setup, NULL, NULL, NULL);
ZTEST_SUITE(qdec_pm_e_stays_up, NULL, suite_setup, NULL, NULL, NULL);
ZTEST_SUITE(qdec_pm_z_soft_off, NULL, suite_setup, NULL, NULL, NULL);

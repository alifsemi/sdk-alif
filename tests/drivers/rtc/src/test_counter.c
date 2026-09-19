/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */
#include <zephyr/ztest.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/drivers/counter.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/hwinfo.h>
#include <zephyr/sys/sys_io.h>
#include <zephyr/devicetree.h>
#if CONFIG_TEST_RTC_SUSPEND_WAKE
#include <zephyr/pm/pm.h>
#include <zephyr/pm/policy.h>
#include <zephyr/irq.h>
#endif
#if CONFIG_TEST_RTC_SUSPEND_WAKE || CONFIG_TEST_RTC_CLK_SOURCE
#include <se_service.h>
#endif

LOG_MODULE_REGISTER(test);

#if IS_ENABLED(CONFIG_TEST_RTC_WRAP_MODE) || \
	IS_ENABLED(CONFIG_TEST_RTC_PRESCALER) || \
	IS_ENABLED(CONFIG_TEST_RTC_CLK_SOURCE) || \
	IS_ENABLED(CONFIG_TEST_RTC_SUSPEND_WAKE)
#define RTC_SNIPPET_BUILD 1
#else
#define RTC_SNIPPET_BUILD 0
#endif

#define DOWN_COUNTER 0
#define DELAY   2000000
#define DELAY_2 3000000

#define PRESCALER_TEST_PERIOD_US   5000000   /* 5 sec */
#define PRESCALER_SAMPLE_COUNT 3
#define PRESCALER_TOLERANCE_TICKS 2

#define TICKS_PER_SEC 32768
#define ALARM_CHANNEL_ID 0

#if !IS_ENABLED(CONFIG_TEST_RTC_SUSPEND_WAKE)
static int check_reset_cause(void)
{
	hwinfo_clear_reset_cause();

	return 0;
}
SYS_INIT(check_reset_cause, APPLICATION, 0);
#endif

#if CONFIG_TEST_RTC_SUSPEND_WAKE
#define SUSPEND_WAKE_ALARM_USEC         3000000
#define SUSPEND_WAKE_POST_WAKE_DELAY_MS 10000

#if defined(CONFIG_SOC_SERIES_E1C) || defined(CONFIG_SOC_SERIES_B1)
#define SERAM_MEMORY_BLOCKS_IN_USE \
	(SERAM_1_MASK | SERAM_2_MASK | SERAM_3_MASK | SERAM_4_MASK)
#define APP_RET_MEM_BLOCKS \
	(SRAM4_1_MASK | SRAM4_2_MASK | SRAM4_3_MASK | SRAM4_4_MASK | \
	 SRAM5_1_MASK | SRAM5_2_MASK | SRAM5_3_MASK | SRAM5_4_MASK | \
	 SRAM5_5_MASK)
#else
#define SERAM_MEMORY_BLOCKS_IN_USE SERAM_MASK
#define APP_RET_MEM_BLOCKS \
	(SRAM4_1_MASK | SRAM4_2_MASK | SRAM5_1_MASK | SRAM5_2_MASK)
#endif

#define MRAM_BASE_ADDRESS 0x80000000
#define IS_BOOTING_FROM_MRAM() (SCB->VTOR >= MRAM_BASE_ADDRESS)

#if defined(CONFIG_RTSS_HE)
#define RTC_S2RAM_SUPPORTED (!IS_BOOTING_FROM_MRAM())
#else
#define RTC_S2RAM_SUPPORTED 0
#endif

#if defined(CONFIG_RTSS_HP)
#define RTC_SOFT_OFF_SUPPORTED 1
#elif defined(CONFIG_RTSS_HE)
#define RTC_SOFT_OFF_SUPPORTED IS_BOOTING_FROM_MRAM()
#else
#define RTC_SOFT_OFF_SUPPORTED 0
#endif

#define RTC0_BASE DT_REG_ADDR(DT_NODELABEL(rtc0))
#define RTC0_IRQ  DT_IRQN(DT_NODELABEL(rtc0))

K_SEM_DEFINE(suspend_wake_sem, 0, 1);

static struct counter_alarm_cfg rtc_suspend_alarm_cfg;
static uint32_t rtc_suspend_saved_iser[2];


static void suspend_wake_alarm_cb(const struct device *dev, uint8_t chan_id,
		uint32_t ticks, void *user_data)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(chan_id);
	ARG_UNUSED(ticks);
	ARG_UNUSED(user_data);

	k_sem_give(&suspend_wake_sem);
}

static int rtc_suspend_set_off_profile(enum pm_state state)
{
	off_profile_t offp = {0};
	int ret;

	offp.dcdc_voltage    = 825;
	offp.dcdc_mode       = DCDC_MODE_OFF;
	offp.stby_clk_freq   = SCALED_FREQ_RC_STDBY_76_8_MHZ;
	offp.aon_clk_src     = CLK_SRC_LFXO;
	offp.stby_clk_src    = CLK_SRC_HFRC;
	offp.vtor_address    = SCB->VTOR;
	offp.ip_clock_gating = 0;
	offp.phy_pwr_gating  = 0;
	offp.vdd_ioflex_3V3  = IOFLEX_LEVEL_1V8;
	offp.ewic_cfg        = EWIC_RTC_A;
	offp.wakeup_events   = WE_LPRTC;
	offp.memory_blocks   = MRAM_MASK | SERAM_MEMORY_BLOCKS_IN_USE;
	offp.power_domains   = PD_VBAT_AON_MASK;

	if (state == PM_STATE_SUSPEND_TO_RAM) {
		offp.memory_blocks |= APP_RET_MEM_BLOCKS;
	}

	ret = se_service_set_off_cfg(&offp);
	if (ret) {
		TC_PRINT("SE set_off_cfg failed (err %d)\n", ret);
	}

	return ret;
}

static int rtc_suspend_restore_run_profile(void)
{
	run_profile_t runp = {0};
	int ret;

	runp.power_domains  = PD_SYST_MASK | PD_SSE700_AON_MASK;
	runp.dcdc_voltage   = 825;
	runp.dcdc_mode      = DCDC_MODE_PWM;
	runp.aon_clk_src    = CLK_SRC_LFXO;
	runp.run_clk_src    = CLK_SRC_PLL;
	runp.vdd_ioflex_3V3 = IOFLEX_LEVEL_1V8;
#if defined(CONFIG_RTSS_HP)
	runp.cpu_clk_freq   = CLOCK_FREQUENCY_400MHZ;
#else
	runp.cpu_clk_freq   = CLOCK_FREQUENCY_160MHZ;
#endif
	runp.memory_blocks  = MRAM_MASK;

	ret = se_service_set_run_cfg(&runp);
	if (ret) {
		TC_PRINT("SE set_run_cfg failed (err %d)\n", ret);
	}

	return 0;
}
SYS_INIT(rtc_suspend_restore_run_profile, PRE_KERNEL_1, 46);

static void rtc_suspend_mask_non_rtc_ewic_irqs(void)
{
	unsigned int rtc_irq = DT_IRQN(DT_NODELABEL(rtc0));

	rtc_suspend_saved_iser[0] = NVIC->ISER[0];
	rtc_suspend_saved_iser[1] = NVIC->ISER[1];

	for (unsigned int irq = 0; irq < 64; irq++) {
		if (irq == rtc_irq) {
			continue;
		}
		if (NVIC_GetEnableIRQ(irq)) {
			irq_disable(irq);
			NVIC_ClearPendingIRQ(irq);
		}
	}
	NVIC_ClearPendingIRQ(rtc_irq);
}

static void rtc_suspend_restore_ewic_irqs(void)
{
	unsigned int rtc_irq = DT_IRQN(DT_NODELABEL(rtc0));

	for (unsigned int irq = 0; irq < 64; irq++) {
		if (irq == rtc_irq) {
			continue;
		}
		if (rtc_suspend_saved_iser[irq / 32U] & BIT(irq % 32U)) {
			irq_enable(irq);
		}
	}
}


static int rtc_suspend_arm_lprtc(const struct device *dev)
{
	int err;
	uint32_t alarm_ticks = counter_us_to_ticks(dev,
			SUSPEND_WAKE_ALARM_USEC);

	(void)counter_cancel_channel_alarm(dev, ALARM_CHANNEL_ID);
	(void)sys_read32(RTC0_BASE + 0x18);
	k_sem_reset(&suspend_wake_sem);

	rtc_suspend_alarm_cfg.flags = 0;
	rtc_suspend_alarm_cfg.ticks = alarm_ticks;
	rtc_suspend_alarm_cfg.callback = suspend_wake_alarm_cb;
	rtc_suspend_alarm_cfg.user_data = NULL;

	err = counter_set_channel_alarm(dev, ALARM_CHANNEL_ID,
			&rtc_suspend_alarm_cfg);
	return err;
}

static int rtc_suspend_start_rtc(const struct device *dev)
{
	int err;
	uint32_t now_ticks;

	err = counter_start(dev);
	zassert_true(err == 0 || err == -EALREADY,
			"%s: Counter failed to start (err %d)", dev->name, err);

	err = counter_get_value(dev, &now_ticks);
	zassert_equal(0, err, "%s: Failed to read RTC", dev->name);
	return err;
}

static void rtc_suspend_enter_soft_off(const struct device *dev)
{
	int err, ret;
	int64_t start_ms, end_ms, elapsed_ms;

	pm_policy_state_lock_get(PM_STATE_RUNTIME_IDLE, PM_ALL_SUBSTATES);
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_RAM, PM_ALL_SUBSTATES);
	pm_policy_state_lock_get(PM_STATE_SOFT_OFF, PM_ALL_SUBSTATES);


	ret = rtc_suspend_arm_lprtc(dev);
	zassert_equal(0, ret, "%s: setting alarm failed", dev->name);

	ret = rtc_suspend_set_off_profile(PM_STATE_SOFT_OFF);
	zassert_equal(0, ret, "%s: SE set_off_cfg failed (err %d)",
			dev->name, ret);

	rtc_suspend_mask_non_rtc_ewic_irqs();
	k_busy_wait(50000);

	pm_policy_state_lock_put(PM_STATE_SOFT_OFF, PM_ALL_SUBSTATES);

	start_ms = k_uptime_get();
	k_sleep(K_FOREVER);
	end_ms = k_uptime_get();
	elapsed_ms = end_ms - start_ms;

	pm_policy_state_lock_put(PM_STATE_RUNTIME_IDLE, PM_ALL_SUBSTATES);
	pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
	pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_RAM, PM_ALL_SUBSTATES);

	ret = k_sem_take(&suspend_wake_sem, K_NO_WAIT);
	{
		uint32_t ccvr = sys_read32(RTC0_BASE + 0x00);
		uint32_t cmr  = sys_read32(RTC0_BASE + 0x04);
		uint32_t stat = sys_read32(RTC0_BASE + 0x10);

		TC_PRINT("SOFT_OFF returned after %lld ms\n", elapsed_ms);
		TC_PRINT("sem=%d ccvr=%u cmr=%u stat=0x%x %s\n",
				ret, ccvr, cmr, stat,
				(ret == 0 || (stat & BIT(0)) ||
				 (cmr && ccvr >= cmr)) ?
				"RECEIVED" : "NOT RECEIVED");
	}

	err = counter_cancel_channel_alarm(dev, ALARM_CHANNEL_ID);
	ARG_UNUSED(err);
	err = counter_stop(dev);
	zassert_equal(err, 0, "Failed to stop counter (err %d)\n", err);

	zassert_true(false,
			"%s: SOFT_OFF did not power off the core (slept %lld ms)",
			dev->name, elapsed_ms);
}

static void rtc_suspend_enter_s2ram(const struct device *dev)
{
	int ret;

	pm_policy_state_lock_get(PM_STATE_RUNTIME_IDLE, PM_ALL_SUBSTATES);
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
	pm_policy_state_lock_get(PM_STATE_SOFT_OFF, PM_ALL_SUBSTATES);
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_RAM, PM_ALL_SUBSTATES);

	ret = rtc_suspend_arm_lprtc(dev);
	zassert_equal(0, ret, "%s: setting alarm failed", dev->name);

	rtc_suspend_warn_debugger();

	ret = rtc_suspend_set_off_profile(PM_STATE_SUSPEND_TO_RAM);
	zassert_equal(0, ret, "%s: SE set_off_cfg failed (err %d)",
			dev->name, ret);

	rtc_suspend_mask_non_rtc_ewic_irqs();
	k_busy_wait(50000);

	pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_RAM, PM_ALL_SUBSTATES);

	ret = k_sem_take(&suspend_wake_sem,
			K_MSEC((SUSPEND_WAKE_ALARM_USEC / 1000U) + 5000U));

	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_RAM, PM_ALL_SUBSTATES);
	rtc_suspend_restore_ewic_irqs();
	(void)rtc_suspend_restore_run_profile();

	zassert_equal(0, ret,
			"%s: did not wake from SUSPEND_TO_RAM via LPRTC",
			dev->name);

	TC_PRINT("Woke from SUSPEND_TO_RAM via LPRTC, uptime=%lld ms\n",
			k_uptime_get());

	k_sleep(K_MSEC(SUSPEND_WAKE_POST_WAKE_DELAY_MS));
}

static void *rtc_suspend_setup(void)
{
	const struct device *dev = DEVICE_DT_GET(DT_NODELABEL(rtc0));

	k_busy_wait(USEC_PER_MSEC * 300);
	zassert_true(device_is_ready(dev), "Device %s is not ready", dev->name);
	k_object_access_grant(dev, k_current_get());

	return NULL;
}

ZTEST(test_rtc_suspend_wake, test_rtc_suspend_wake)
{
	const struct device *dev = DEVICE_DT_GET(DT_NODELABEL(rtc0));

	if (RTC_SOFT_OFF_SUPPORTED) {
		TC_PRINT("Testing %s SOFT_OFF (MRAM)\n", dev->name);
		(void)rtc_suspend_start_rtc(dev);
		rtc_suspend_enter_soft_off(dev);
	}
}

ZTEST(test_rtc_suspend_wake, test_rtc_suspend_to_ram)
{
	const struct device *dev = DEVICE_DT_GET(DT_NODELABEL(rtc0));

	if (RTC_S2RAM_SUPPORTED) {
		TC_PRINT("Testing %s S2RAM (TCM)\n", dev->name);
		(void)rtc_suspend_start_rtc(dev);
		rtc_suspend_enter_s2ram(dev);
	}
}

ZTEST_SUITE(test_rtc_suspend_wake, NULL, rtc_suspend_setup, NULL, NULL, NULL);
#endif

#define STRESS_TEST_COUNT 3
uint32_t wrap_mode;
struct counter_alarm_cfg alarm_cfg;
struct counter_alarm_cfg alarm_cfg2;

#if CONFIG_TEST_RTC_PRESCALER
K_SEM_DEFINE(prescaler_sem, 0, 1);

struct prescaler_test_data {
	const struct device *dev;
	uint32_t expected_ticks;
	uint32_t prev_ticks;
	uint32_t last_delta;
	int samples_done;
};

static struct prescaler_test_data prescaler_data;
#endif

#define DEVICE_DT_GET_AND_COMMA(node_id) DEVICE_DT_GET(node_id),
/* Generate a list of devices for all instances of the "compat" */
#define DEVS_FOR_DT_COMPAT(compat) \
	DT_FOREACH_STATUS_OKAY(compat, DEVICE_DT_GET_AND_COMMA)

static const struct device *const devices[] = {
#ifdef CONFIG_COUNTER_RTC_SNPS_DW
	DEVS_FOR_DT_COMPAT(snps_dw_apb_rtc)
#endif
};

static const struct device *const period_devs[] = {

};

typedef void (*counter_test_func_t)(const struct device *dev);
typedef bool (*counter_capability_func_t)(const struct device *dev);

int check_set_channel_alarm_return_status(int ret)
{

	if (ret == -EINVAL) {
		TC_PRINT("Alarm settings invalid\n");
		return -1;
	}
	if (ret == -ENOTSUP) {
		TC_PRINT("\nAlarm request not supported or "
				"the counter was not started yet.\n");
		return -1;
	}
	if (ret == -ETIME) {
		TC_PRINT("\nabsolute alarm was set too late\n");
		return -1;
	}
	if (ret == -EBUSY) {
		TC_PRINT("\nalarm is already active\n");
		return -1;
	}
	if (ret != 0) {
		TC_PRINT("Error\n");
		TC_PRINT("API is not enabled\n");
		return -1;
	}
	return 0;
}

int check_cancel_channel_alarm_return_status(int ret)
{

	if (ret == -ENOTSUP) {
		TC_PRINT("\nAlarm request not supported or "
				"the counter was not started yet.\n");
		return -1;
	}
	if (ret != 0) {
		TC_PRINT("Error\n");
		TC_PRINT("API is not enabled\n");
		return -1;
	}
	return 0;
}
/*
 * Function to get counter period in microseconds
 */
static inline uint32_t get_counter_period_us(const struct device *dev)
{
	for (int i = 0; i < ARRAY_SIZE(period_devs); i++) {
		if (period_devs[i] == dev) {
			return (USEC_PER_SEC * 2U);
		}
	}
	/* if more counter drivers exist other than RTC,
	 * the test value set to 20000 by default
	 */
	return 20000;
}

static void counter_setup_instance(const struct device *dev)
{
}

static void counter_tear_down_instance(const struct device *dev)
{
	int err;

#if DOWN_COUNTER
	struct counter_top_cfg top_cfg = {
		.callback = NULL,
		.user_data = NULL,
		.flags = 0
	};
	top_cfg.ticks = counter_get_max_top_value(dev);
	err = counter_set_top_value(dev, &top_cfg);

	if (err == -ENOTSUP) {
		/* If resetting is not support, attempt without reset. */
		top_cfg.flags = COUNTER_TOP_CFG_DONT_RESET;
		err = counter_set_top_value(dev, &top_cfg);

	}
	zassert_true((err == 0) || (err == -ENOTSUP),
			"%s: Setting top value to default failed", dev->name);
#endif
	err = counter_stop(dev);
	zassert_equal(0, err, "%s: Counter failed to stop", dev->name);

}

static void test_all_instances(counter_test_func_t func,
		counter_capability_func_t capability_check)
{
	zassert_true(ARRAY_SIZE(devices) > 0, "No device found");

	for (int i = 0; i < ARRAY_SIZE(devices); i++) {
		counter_setup_instance(devices[i]);
		if ((capability_check == NULL) ||
				capability_check(devices[i])) {
			TC_PRINT("Testing %s\n", devices[i]->name);
			func(devices[i]);
		} else {
			TC_PRINT("Skipped for %s\n", devices[i]->name);
		}
		counter_tear_down_instance(devices[i]);

		/* Allow logs to be printed. */
		k_sleep(K_MSEC(100));
	}
}

static bool reliable_cancel_capable(const struct device *dev)
{
#ifdef CONFIG_COUNTER_RTC_SNPS_DW
	if (dev == DEVICE_DT_GET(DT_NODELABEL(rtc0))) {
		return true;
	}
#endif
	return false;
}

static void *counter_setup(void)
{
	int i;

	/* Give required clocks some time to stabilize. In particular, nRF SoCs
	 * need such delay for the Xtal LF clock source to start and for this
	 * test to use the correct timing.
	 */
	k_busy_wait(USEC_PER_MSEC * 300);

	for (i = 0; i < ARRAY_SIZE(devices); i++) {
		zassert_true(device_is_ready(devices[i]),
				"Device %s is not ready", devices[i]->name);
		k_object_access_grant(devices[i], k_current_get());
	}

	return NULL;
}

static void test_counter_interrupt_fn(const struct device *counter_dev,
		uint8_t chan_id, uint32_t ticks,
		void *user_data)
{
	struct counter_alarm_cfg *config = user_data;
	uint32_t now_ticks;
	uint64_t now_usec;
	int now_sec;
	int err;

	err = counter_get_value(counter_dev, &now_ticks);
	if (err) {
		printk("Failed to read counter value (err %d)", err);
		return;
	}

	now_usec = counter_ticks_to_us(counter_dev, now_ticks);
	now_sec = (int)(now_usec / USEC_PER_SEC);

	printk("!!! Alarm !!!\n");
	printk("Now: %u\n", now_sec);
#if CONFIG_TEST_RTC_WRAP_MODE
	if (now_sec != 0) {
		printk("Wrap Mode failed as counter "
			"initial value is not zero\n");
		wrap_mode = 1;

		return;
	}
#endif
	/* Set a new alarm with a double length duration */
	config->ticks = config->ticks * 2U;
	printk("Set alarm in %u sec (%u ticks)\n",
			(uint32_t)(counter_ticks_to_us(counter_dev,
					config->ticks) / USEC_PER_SEC),
			config->ticks);

	err = counter_set_channel_alarm(counter_dev, ALARM_CHANNEL_ID,
			user_data);
	if (err != 0) {
		printk("Alarm could not be set\n");
	}
}

static void test_counter_multiple_alarm(const struct device *counter_dev,
		uint8_t chan_id, uint32_t ticks,
		void *user_data)
{
	uint32_t now_ticks;
	uint64_t now_usec;
	int now_sec;
	int err, ret;

	err = counter_get_value(counter_dev, &now_ticks);
	if (err) {
		printk("Failed to read counter value (err %d)", err);
		return;
	}

	now_usec = counter_ticks_to_us(counter_dev, now_ticks);
	now_sec = (int)(now_usec / USEC_PER_SEC);

	if (chan_id == ALARM_CHANNEL_ID) {
		printk("!!! Alarm %u triggered !!!\n", chan_id);
		printk("Now: %u sec\n", now_sec);

		/* Further actions for each alarm can be added here. */
	}
	/* Cancel alarms on channel 0. */
	err = counter_cancel_channel_alarm(counter_dev, 0);
	ret = check_cancel_channel_alarm_return_status(err);
}


#if CONFIG_TEST_RTC_CLK_SOURCE

#define RTC_CLK_SRC_ALARM_USEC DELAY
#define RTC_CLK_SRC_EXPECTED_MS \
	(RTC_CLK_SRC_ALARM_USEC / 1000U)
#define RTC_CLK_SRC_LFXO_TOL_PCT 5   /* 1900..2100 ms */
#define RTC_CLK_SRC_LFRC_TOL_PCT 15  /* 1700..2300 ms */
#define RTC_CLK_SRC_ALARM_TIMEOUT_MS 5000
#define RTC_CLK_SRC_COUNT_WAIT_MS 200
#define RTC_CLK_SRC_LFXO_SETTLE_MS 1000
#define RTC_CLK_SRC_LFRC_SETTLE_MS 50

K_SEM_DEFINE(rtc_clk_src_sem, 0, 1);

static const char *rtc_clk_src_name(int aon_clk_src)
{
	return (aon_clk_src == CLK_SRC_LFXO) ? "LFXO" : "LFRC";
}

static int rtc_clk_src_tol_pct(int aon_clk_src)
{
	return (aon_clk_src == CLK_SRC_LFXO) ?
		RTC_CLK_SRC_LFXO_TOL_PCT : RTC_CLK_SRC_LFRC_TOL_PCT;
}

static int rtc_clk_src_read_sel(void)
{
	run_profile_t runp = {0};
	int err;

	err = se_service_get_run_cfg(&runp);
	zassert_equal(0, err, "se_service_get_run_cfg failed (err %d)", err);

	return (int)runp.aon_clk_src;
}

static void rtc_clk_src_alarm_cb(const struct device *dev, uint8_t chan_id,
		uint32_t ticks, void *user_data)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(chan_id);
	ARG_UNUSED(ticks);
	ARG_UNUSED(user_data);

	k_sem_give(&rtc_clk_src_sem);
}

static void rtc_clk_src_select(const struct device *dev, int aon_clk_src)
{
	run_profile_t runp = {0};
	int err;

	(void)counter_cancel_channel_alarm(dev, ALARM_CHANNEL_ID);
	err = counter_stop(dev);
	if (err != 0 && err != -EALREADY) {
		TC_PRINT("counter_stop before clock switch returned %d\n",
				err);
	}

	err = se_service_get_run_cfg(&runp);
	zassert_equal(0, err, "se_service_get_run_cfg failed (err %d)", err);

	runp.aon_clk_src = aon_clk_src;
	err = se_service_set_run_cfg(&runp);
	zassert_equal(0, err, "se_service_set_run_cfg failed (err %d)", err);

	k_sleep(K_MSEC((aon_clk_src == CLK_SRC_LFXO) ?
			RTC_CLK_SRC_LFXO_SETTLE_MS :
			RTC_CLK_SRC_LFRC_SETTLE_MS));
}

static void rtc_clk_src_verify_and_alarm(const struct device *dev,
		int aon_clk_src)
{
	int err, ret;
	uint32_t t0, t1;
	int64_t start_ms, elapsed_ms;

	TC_PRINT("LPRTC clock source: %s (aon_clk_src=%d)\n",
			rtc_clk_src_name(aon_clk_src), rtc_clk_src_read_sel());

	zassert_equal(rtc_clk_src_read_sel(), aon_clk_src,
			"aon_clk_src did not select %s",
			rtc_clk_src_name(aon_clk_src));

	err = counter_cancel_channel_alarm(dev, ALARM_CHANNEL_ID);
	(void)check_cancel_channel_alarm_return_status(err);

	err = counter_start(dev);
	zassert_true(err == 0 || err == -EALREADY,
			"%s: Counter failed to start (err %d)", dev->name, err);

	err = counter_get_value(dev, &t0);
	zassert_equal(0, err, "%s: Failed to read RTC", dev->name);
	k_sleep(K_MSEC(RTC_CLK_SRC_COUNT_WAIT_MS));
	err = counter_get_value(dev, &t1);
	zassert_equal(0, err, "%s: Failed to read RTC", dev->name);
	zassert_true(t1 != t0,
			"%s: RTC is not counting on %s (t0=%u t1=%u)",
			dev->name, rtc_clk_src_name(aon_clk_src), t0, t1);

	k_sem_reset(&rtc_clk_src_sem);

	alarm_cfg.flags = 0;
	alarm_cfg.ticks = counter_us_to_ticks(dev, RTC_CLK_SRC_ALARM_USEC);
	alarm_cfg.callback = rtc_clk_src_alarm_cb;
	alarm_cfg.user_data = NULL;

	err = counter_set_channel_alarm(dev, ALARM_CHANNEL_ID, &alarm_cfg);
	ret = check_set_channel_alarm_return_status(err);
	zassert_equal(0, ret, "%s: setting alarm failed on %s",
			dev->name, rtc_clk_src_name(aon_clk_src));

	TC_PRINT("Set %s alarm in %u ticks (~%u us)\n",
			rtc_clk_src_name(aon_clk_src), alarm_cfg.ticks,
			RTC_CLK_SRC_ALARM_USEC);

	start_ms = k_uptime_get();
	ret = k_sem_take(&rtc_clk_src_sem,
			K_MSEC(RTC_CLK_SRC_ALARM_TIMEOUT_MS));
	elapsed_ms = k_uptime_get() - start_ms;

	zassert_equal(0, ret,
			"%s: RTC alarm did not fire on %s within %d ms",
			dev->name, rtc_clk_src_name(aon_clk_src),
			RTC_CLK_SRC_ALARM_TIMEOUT_MS);

	{
		int tol_pct = rtc_clk_src_tol_pct(aon_clk_src);
		uint32_t min_ms = RTC_CLK_SRC_EXPECTED_MS *
				(100U - tol_pct) / 100U;
		uint32_t max_ms = RTC_CLK_SRC_EXPECTED_MS *
				(100U + tol_pct) / 100U;
		int64_t pct = (elapsed_ms * 100) /
				RTC_CLK_SRC_EXPECTED_MS;

		TC_PRINT("%s alarm after %lld ms (%lld%% of %u)\n",
				rtc_clk_src_name(aon_clk_src), elapsed_ms,
				pct, RTC_CLK_SRC_EXPECTED_MS);
		TC_PRINT("tol +/-%d%% window %u..%u ms\n",
				tol_pct, min_ms, max_ms);

		zassert_true(elapsed_ms >= min_ms && elapsed_ms <= max_ms,
				"%s: %s alarm %lld ms out of range",
				dev->name, rtc_clk_src_name(aon_clk_src),
				elapsed_ms);
	}

	err = counter_cancel_channel_alarm(dev, ALARM_CHANNEL_ID);
	(void)check_cancel_channel_alarm_return_status(err);
	err = counter_stop(dev);
	zassert_equal(0, err, "%s: Failed to stop counter (err %d)",
			dev->name, err);
}

static void test_rtc_clk_source_instance(const struct device *dev)
{
	int first = rtc_clk_src_read_sel();
	int second = (first == CLK_SRC_LFXO) ? CLK_SRC_LFRC : CLK_SRC_LFXO;

	TC_PRINT("First AON clock source is %s; then %s\n",
			rtc_clk_src_name(first), rtc_clk_src_name(second));

	rtc_clk_src_verify_and_alarm(dev, first);

	rtc_clk_src_select(dev, second);
	rtc_clk_src_verify_and_alarm(dev, second);

	/* Restore the source that was in use at the start of the test. */
	rtc_clk_src_select(dev, first);
	zassert_equal(rtc_clk_src_read_sel(), first,
			"Failed to restore original aon_clk_src");
}

ZTEST(test_RTC, test_rtc_clk_source)
{
	test_all_instances(test_rtc_clk_source_instance,
			reliable_cancel_capable);
}
#endif


#if CONFIG_TEST_RTC_PRESCALER
static void prescaler_alarm_cb(const struct device *dev, uint8_t chan_id,
		uint32_t ticks, void *user_data)
{
	struct prescaler_test_data *data = user_data;
	uint32_t now_ticks;

	counter_get_value(dev, &now_ticks);
	data->last_delta = now_ticks - data->prev_ticks;
	data->prev_ticks = now_ticks;
	data->samples_done++;

	if (data->samples_done < PRESCALER_SAMPLE_COUNT) {
		alarm_cfg.ticks = data->expected_ticks;
		alarm_cfg.callback = prescaler_alarm_cb;
		alarm_cfg.user_data = data;
		counter_set_channel_alarm(dev, ALARM_CHANNEL_ID, &alarm_cfg);
	}

	k_sem_give(&prescaler_sem);
}

static void test_prescaler_instance(const struct device *dev)
{
	int err, ret;
	uint32_t now_ticks;

	err = counter_cancel_channel_alarm(dev, ALARM_CHANNEL_ID);
	check_cancel_channel_alarm_return_status(err);

	counter_start(dev);

	counter_get_value(dev, &now_ticks);

	prescaler_data.dev = dev;
	prescaler_data.expected_ticks =
			counter_us_to_ticks(dev, PRESCALER_TEST_PERIOD_US);
	prescaler_data.prev_ticks = now_ticks;
	prescaler_data.samples_done = 0;

	TC_PRINT("Prescaler test: expecting %u ticks per %d us\n",
			prescaler_data.expected_ticks,
			PRESCALER_TEST_PERIOD_US);

	alarm_cfg.flags = 0;
	alarm_cfg.ticks = prescaler_data.expected_ticks;
	alarm_cfg.callback = prescaler_alarm_cb;
	alarm_cfg.user_data = &prescaler_data;

	err = counter_set_channel_alarm(dev, ALARM_CHANNEL_ID, &alarm_cfg);
	ret = check_set_channel_alarm_return_status(err);
	zassert_equal(0, ret, "%s: setting alarm failed", dev->name);

	for (int i = 0; i < PRESCALER_SAMPLE_COUNT; i++) {
		ret = k_sem_take(&prescaler_sem, K_MSEC(7000));
		zassert_equal(0, ret,
				"%s: alarm did not fire in time",
				dev->name);

		TC_PRINT("Sample %d: delta=%u expected=%u\n", i,
				prescaler_data.last_delta,
				prescaler_data.expected_ticks);

		zassert_within(prescaler_data.last_delta,
				prescaler_data.expected_ticks,
				PRESCALER_TOLERANCE_TICKS,
				"%s: tick delta %u out of tolerance (expected %u)",
				dev->name, prescaler_data.last_delta,
				prescaler_data.expected_ticks);
	}

	counter_cancel_channel_alarm(dev, ALARM_CHANNEL_ID);
	err = counter_stop(dev);
	zassert_equal(err, 0, "Failed to stop counter (err %d)\n", err);
}

ZTEST(test_RTC, test_prescaler_alarm)
{
	test_all_instances(test_prescaler_instance,
			reliable_cancel_capable);
}
#endif

#if CONFIG_TEST_RTC_WRAP_MODE
static void test_counter_interrupt_fn2sec(const struct device *counter_dev,
		uint8_t chan_id, uint32_t ticks,
		void *user_data)
{
	struct counter_alarm_cfg *config = user_data;
	uint32_t now_ticks, Val;
	uint64_t now_usec;
	int now_sec;
	int err;

	err = counter_get_value(counter_dev, &now_ticks);
	if (err) {
		printk("Failed to read counter value (err %d)", err);
		return;
	}

	now_usec = counter_ticks_to_us(counter_dev, now_ticks);
	now_sec = (int)(now_usec / USEC_PER_SEC);

	printk("!!! Alarm !!!\n");
	printk("Now: %u\n", now_sec);

	/* Set alarm for 2sec every time  */
	config->ticks = 2U;

	printk("Set alarm in %u sec (%u ticks)\n",
			(uint32_t)(counter_ticks_to_us(counter_dev,
					config->ticks) / USEC_PER_SEC),
			config->ticks);
	err = counter_get_value(counter_dev, &Val);
	if (err) {
		printk("Failed to read counter value (err %d)", err);
		return;
	}
	err = counter_set_channel_alarm(counter_dev, ALARM_CHANNEL_ID,
			user_data);
	if (err != 0) {
		printk("Alarm could not be set\n");
	}
}

static void test_counter_value_before_start(const struct device *dev)
{
	int ret;
	uint32_t now_ticks = -1;

	TC_PRINT("Counter to test initial value of counter\n\n");
	zassert_true(device_is_ready(dev), "device not ready.\n");

	ret = counter_get_value(dev, &now_ticks);

	if (ret < 0) {
		TC_PRINT("error while reading value");
		ztest_test_fail();
	}
	zassert_equal(now_ticks, 0, "Initial counter value is not 0 -> %d.",
			now_ticks);
	TC_PRINT("Initial counter value is -> %d.\n", now_ticks);
}

ZTEST(test_RTC, test_counter_value_before_start)
{
	test_all_instances(test_counter_value_before_start,
			reliable_cancel_capable);
}

static void test_alarm_instance2sec(const struct device *dev)
{
	int err, ret;
	uint32_t Val;

	err = counter_cancel_channel_alarm(dev, ALARM_CHANNEL_ID);
	ret = check_cancel_channel_alarm_return_status(err);
	zassert_equal(0, ret, "%s: Counter disabling alarm failed", dev->name);

	err = counter_get_value(dev, &Val);
	if (err) {
		printk("Failed to read counter value (err %d)", err);
		return;
	}
	zassert_equal(0, err, "Initial value is not Zero -> %d", err);
	counter_start(dev);

	alarm_cfg.flags = 0;
	alarm_cfg.ticks = counter_us_to_ticks(dev, DELAY);
	alarm_cfg.callback = test_counter_interrupt_fn2sec;
	alarm_cfg.user_data = &alarm_cfg;

	err = counter_set_channel_alarm(dev, ALARM_CHANNEL_ID,
			&alarm_cfg);

	ret = check_set_channel_alarm_return_status(err);
	zassert_equal(0, ret, "%s: Counter setting alarm failed", dev->name);

	err = counter_get_value(dev, &Val);
	if (err) {
		printk("Failed to read counter value (err %d)", err);
		return;
	}
	zassert_equal(0, err, "Initial value is not Zero -> %d", err);

	k_sleep(K_MSEC(1000));

	err = counter_cancel_channel_alarm(dev, ALARM_CHANNEL_ID);
	ret = check_cancel_channel_alarm_return_status(err);
	zassert_equal(0, ret, "%s: Counter disabling alarm failed", dev->name);

	err = counter_stop(dev);
	zassert_equal(err, 0, "Failed to stop counter (err %d)\n", err);
}

ZTEST(test_RTC, test_alarm_2sec)
{
	test_all_instances(test_alarm_instance2sec,
			reliable_cancel_capable);
}

static void test_RTC_wrap_mode_instance(const struct device *counter_dev)
{
	int err, ret;
	uint32_t now_ticks;

	TC_PRINT("Counter to Test RTC wrap mode\n\n");

	err = counter_cancel_channel_alarm(counter_dev, ALARM_CHANNEL_ID);
	ret = check_cancel_channel_alarm_return_status(err);
	zassert_equal(0, ret,
			"%s: Counter disabling alarm failed",
			counter_dev->name);

	zassert_true(device_is_ready(counter_dev), "device not ready.\n");
	counter_start(counter_dev);

	alarm_cfg.flags = 0;
	alarm_cfg.ticks = counter_us_to_ticks(counter_dev, 400000);
	alarm_cfg.callback = test_counter_interrupt_fn;
	alarm_cfg.user_data = &alarm_cfg;

	err = counter_get_value(counter_dev, &now_ticks);
	zassert_equal(err, 0, "Failed to read counter value (err %d)", err);

	err = counter_set_channel_alarm(counter_dev, ALARM_CHANNEL_ID,
			&alarm_cfg);

	ret = check_set_channel_alarm_return_status(err);
	zassert_equal(0, ret,
			"%s: Counter setting alarm failed", counter_dev->name);

	TC_PRINT("Set alarm in %u sec (%u ticks)\n",
			(uint32_t)(counter_ticks_to_us(counter_dev,
					alarm_cfg.ticks) / USEC_PER_SEC),
			alarm_cfg.ticks);

	k_sleep(K_MSEC(1000));

	err = counter_cancel_channel_alarm(counter_dev, ALARM_CHANNEL_ID);
	ret = check_cancel_channel_alarm_return_status(err);
	zassert_equal(0, ret,
			"%s: Counter disabling alarm failed",
			counter_dev->name);
	zassert_equal(wrap_mode, 0,
			"Wrap Mode failed as counter initial value is not zero\n");

	err = counter_stop(counter_dev);
	zassert_equal(err, 0, "Failed to stop counter (err %d)\n", err);
}

ZTEST(test_RTC, test_RTC_wrap_mode)
{
	test_all_instances(test_RTC_wrap_mode_instance,
			reliable_cancel_capable);
}
#endif

#if !RTC_SNIPPET_BUILD
static void test_Compare_alarm_instance(const struct device *dev)
{
	int err, ret;

	err = counter_cancel_channel_alarm(dev, ALARM_CHANNEL_ID);
	zassert_equal(0, err, "%s: Counter disabling alarm failed", dev->name);
	printk("Compare Triggered alarm with expected Test alarm\n");
	counter_start(dev);

	alarm_cfg.flags = 0;
	alarm_cfg.ticks = counter_us_to_ticks(dev, DELAY);
	alarm_cfg.callback = test_counter_interrupt_fn;
	alarm_cfg.user_data = &alarm_cfg;

	err = counter_set_channel_alarm(dev, ALARM_CHANNEL_ID,
			&alarm_cfg);

	ret = check_set_channel_alarm_return_status(err);
	zassert_equal(0, ret, "%s: Counter setting alarm failed", dev->name);

	printk("Set alarm in %u sec (%u ticks)\n",
			(uint32_t)(counter_ticks_to_us(dev,
					alarm_cfg.ticks) / USEC_PER_SEC),
			alarm_cfg.ticks);

	/* Calculate expected duration in seconds divide by ticksPer sec */
	uint32_t expected_seconds = alarm_cfg.ticks / TICKS_PER_SEC;

	zassert_equal(expected_seconds, (uint32_t)(counter_ticks_to_us(dev,
			alarm_cfg.ticks) / USEC_PER_SEC),
			"%s: Counter Triggered alarm at non expected duration",
			dev->name);

	printk("%s: Counter Triggered alarm at %u sec (%u ticks )\n and "
"expected duration is %d\n", dev->name,
	(uint32_t)(counter_ticks_to_us(dev, alarm_cfg.ticks)/USEC_PER_SEC),
	alarm_cfg.ticks, expected_seconds);

	k_sleep(K_MSEC(2000));

	err = counter_cancel_channel_alarm(dev, ALARM_CHANNEL_ID);
	zassert_equal(0, err, "%s: Counter disabling alarm failed", dev->name);

	/* Stop the counter */
	err = counter_stop(dev);
	zassert_equal(err, 0, "Failed to stop counter (err %d)\n", err);
}

ZTEST(test_RTC, test_Compare_alarm_instance)
{
	test_all_instances(test_Compare_alarm_instance,
			reliable_cancel_capable);
}

/*
 * Verify that the counter can be read before and after start.
 */
static void test_counter_value_instance(const struct device *dev)
{
	int err, ret;
	uint32_t now_ticks = -1;

	printk("Counter to test initial value of counter\n");
	zassert_true(device_is_ready(dev), "device not ready.\n");

	ret = counter_get_value(dev, &now_ticks);
	if (ret < 0) {
		printk("error while reading value");
		ztest_test_fail();
	}
	printk("Initial value of the counter before Starting the "
"counter is -> %d.\n", now_ticks);
	counter_start(dev);
	ret = counter_get_value(dev, &now_ticks);
	if (ret < 0) {
		printk("error while reading value");
		ztest_test_fail();
	}
	printk("Initial counter value is %d", now_ticks);
	/* Stop the counter */
	err = counter_stop(dev);
	zassert_equal(err, 0, "Failed to stop counter (err %d)\n", err);
}

ZTEST(test_RTC, test_counter_value)
{
	test_all_instances(test_counter_value_instance,
			reliable_cancel_capable);
}

static void test_counter_value_after_stop_counter(const struct device *dev)
{
	int err, ret;
	uint32_t now_ticks = -1, now_ticks_stop;

	err = counter_cancel_channel_alarm(dev, ALARM_CHANNEL_ID);
	ret = check_cancel_channel_alarm_return_status(err);
	zassert_equal(0, ret, "%s: Counter disabling alarm failed", dev->name);

	printk("Counter to test initial value of counter\n");
	zassert_true(device_is_ready(dev), "device not ready.\n");

	counter_start(dev);
	ret = counter_get_value(dev, &now_ticks);
	printk("Counter value is %d ", now_ticks);

	zassert_true(ret == 0, "error while reading value");
	counter_stop(dev);
	ret = counter_get_value(dev, &now_ticks_stop);
	printk("After stopping the counter ,the value is %d\n",
		now_ticks_stop);
	counter_start(dev);
	ret = counter_get_value(dev, &now_ticks);
	printk("The value of counter after starting counter again is %d\n",
		now_ticks);
	/* Stop the counter */
	err = counter_stop(dev);
	zassert_equal(err, 0, "Failed to stop counter (err %d)\n", err);

}

ZTEST(test_RTC, test_counter_value_Stop_counter)
{
	test_all_instances(test_counter_value_after_stop_counter,
			reliable_cancel_capable);
}
#endif

#if !RTC_SNIPPET_BUILD
static void test_counter_counting_up(const struct device *dev)
{
	int ret;

	printk("Counter to test if Real time Counter is counting up\n");
	zassert_true(device_is_ready(dev), "device not ready.\n");

	ret = counter_is_counting_up(dev);
	zassert_true(ret, "failed as Counter is counting down ret is %d", ret);

}

ZTEST(test_RTC, test_counter_is_counting_up)
{
	test_all_instances(test_counter_counting_up,
			reliable_cancel_capable);
}
/*
 * Test that the counter increments by one with each tick
 */
static void test_counter_increment_instance(const struct device *counter_dev)
{
	int ret_count1, ret_count2, ret, err;
	uint32_t now_ticks = -1, prev_ticks;

	printk("Counter initial value\n");

	zassert_true(device_is_ready(counter_dev), "device not ready.\n");

	ret = counter_start(counter_dev);
	zassert_equal(0, ret, "%s: Counter failed to start", counter_dev->name);

	ret_count1 = counter_get_value(counter_dev, &prev_ticks);
	if (ret_count1 < 0) {
		printk("error while reading value");
	}
	TC_PRINT("counter value is %d\n", prev_ticks);
	k_sleep(K_MSEC(1000));
	ret_count2 = counter_get_value(counter_dev, &now_ticks);
	zassert_true(ret_count2 == 0, "%s: Counter read failed (err: %d)",
			counter_dev->name, ret_count2);
	TC_PRINT("counter value is %d\n", now_ticks);
	zassert_false(prev_ticks == now_ticks,
			"%s: Counter incrementing failed",
			counter_dev->name);

	/* Stop the counter */
	err = counter_stop(counter_dev);
	zassert_equal(err, 0, "Failed to stop counter (err %d)\n", err);
}

ZTEST(test_RTC, test_counter_increment)
{
	test_all_instances(test_counter_increment_instance,
			reliable_cancel_capable);
}

/*
 * RTC - Error Scenario set alarm and pass NULL as callback function
 */
static void test_error_check_instance(const struct device *counter_dev)
{

	int err, ret;

	printk("Counter alarm check for NULL Callback "
"So Alarm should not be set\n");

	err = counter_cancel_channel_alarm(counter_dev, ALARM_CHANNEL_ID);
	ret = check_cancel_channel_alarm_return_status(err);
	zassert_equal(0, ret,
			"%s: Counter disabling alarm failed", counter_dev->name);

	zassert_true(device_is_ready(counter_dev), "device not ready.\n");

	counter_start(counter_dev);

	alarm_cfg.flags = 0;
	alarm_cfg.ticks = counter_us_to_ticks(counter_dev, DELAY);
	alarm_cfg.callback = NULL;
	alarm_cfg.user_data = &alarm_cfg;

	err = counter_set_channel_alarm(counter_dev, ALARM_CHANNEL_ID,
			&alarm_cfg);
	ret = check_set_channel_alarm_return_status(err);
	zassert_equal(0, ret,
			"%s: Counter setting alarm failed", counter_dev->name);

	if (ret  >= 0) {
		printk("Set alarm in %u sec (%u ticks)\n",
				(uint32_t)(counter_ticks_to_us(counter_dev,
						alarm_cfg.ticks) / USEC_PER_SEC),
				alarm_cfg.ticks);
		err = counter_cancel_channel_alarm(counter_dev, ALARM_CHANNEL_ID);
		ret = check_cancel_channel_alarm_return_status(err);
		zassert_equal(0, ret,
				"%s: Counter disabling alarm failed",
				counter_dev->name);
	}

	/* Stop the counter */
	err = counter_stop(counter_dev);
	zassert_equal(err, 0, "Failed to stop counter (err %d)\n", err);

}

ZTEST(test_RTC, test_error_check)
{
	test_all_instances(test_error_check_instance,
			reliable_cancel_capable);
}
#endif

#if !RTC_SNIPPET_BUILD
static void test_cancel_alarm_instance(const struct device *dev)
{

	int err, ret;

	zassert_true(device_is_ready(dev), "device not ready.\n");
	printk("Testcase to Cancel Alarm\n");
	counter_start(dev);

	alarm_cfg.flags = 0;
	alarm_cfg.ticks = counter_us_to_ticks(dev, DELAY);
	alarm_cfg.callback = test_counter_interrupt_fn;
	alarm_cfg.user_data = &alarm_cfg;

	err = counter_set_channel_alarm(dev, ALARM_CHANNEL_ID,
			&alarm_cfg);
	printk("Set alarm in %u sec (%u ticks)\n",
			(uint32_t)(counter_ticks_to_us(dev,
					alarm_cfg.ticks) / USEC_PER_SEC),
			alarm_cfg.ticks);

	ret = check_set_channel_alarm_return_status(err);
	zassert_equal(0, ret, "%s: Counter setting alarm failed", dev->name);

	err = counter_cancel_channel_alarm(dev, ALARM_CHANNEL_ID);
	ret = check_cancel_channel_alarm_return_status(err);
	zassert_equal(0, ret, "%s: Counter disabling alarm failed", dev->name);
	printk("wait for 100000 ms -> 101 sec\n");
	k_sleep(K_MSEC(100001));

	/* Stop the counter */
	err = counter_stop(dev);
	zassert_equal(err, 0, "Failed to stop counter (err %d)\n", err);
}

ZTEST(test_RTC, test_Cancel_alarm)
{
	test_all_instances(test_cancel_alarm_instance,
			reliable_cancel_capable);
}

/*
 * Two alarms set. First alarm is absolute, second relative. Because
 * setting of both alarms is delayed it is expected that second alarm
 * will expire first (relative to the time called) while first alarm
 * will expire after next wrap around.
 */
static void test_multiple_instance(const struct device *dev)
{
	int err, ret;
	struct counter_alarm_cfg alarm_cfg_1;
	struct counter_alarm_cfg alarm_cfg_2;

	/* Cancel alarms on channel 0 */
	err = counter_cancel_channel_alarm(dev, 0);
	ret = check_cancel_channel_alarm_return_status(err);
	printk("Counter Test multiple alarms\n");

	err = counter_stop(dev);
	zassert_equal(err, 0, "Failed to stop counter (err %d)\n", err);
	counter_start(dev);

	alarm_cfg_1.flags = 0;
	alarm_cfg_1.ticks = counter_us_to_ticks(dev, DELAY);
	alarm_cfg_1.callback = test_counter_multiple_alarm;
	alarm_cfg_1.user_data = &alarm_cfg_1;

	/* Use channel 0 */
	err = counter_set_channel_alarm(dev, 0, &alarm_cfg_1);
	ret = check_set_channel_alarm_return_status(err);
	zassert_equal(0, ret, "%s: Counter setting alarm 1 failed", dev->name);

	printk("Set alarm 1 in %u sec (%u ticks)\n",
			(uint32_t)(counter_ticks_to_us(dev,
			alarm_cfg_1.ticks) / USEC_PER_SEC),
			alarm_cfg_1.ticks);
	/* Sleep to ensure first alarm triggers first */
	k_sleep(K_MSEC(200));

	alarm_cfg_2.flags = 0;
	alarm_cfg_2.ticks = counter_us_to_ticks(dev, DELAY_2);
	alarm_cfg_2.callback = test_counter_multiple_alarm;
	alarm_cfg_2.user_data = &alarm_cfg_2;

	/* Use channel 0 again */
	err = counter_set_channel_alarm(dev, 0, &alarm_cfg_2);
	ret = check_set_channel_alarm_return_status(err);
	zassert_not_equal(0, ret, "%s: Counter setting alarm 2 Passed "
				"but should fail because alarm is still running",
				dev->name);

	printk("Set alarm 2 in %u sec (%u ticks)\n",
			(uint32_t)(counter_ticks_to_us(dev,
			alarm_cfg_2.ticks) / USEC_PER_SEC),
			alarm_cfg_2.ticks);
	/* Sleep long enough for both alarms to trigger */
	k_sleep(K_MSEC(2000));
	/* Cancel alarms on channel 0 */
	err = counter_cancel_channel_alarm(dev, 0);
	ret = check_cancel_channel_alarm_return_status(err);

	/* Stop the counter */
	err = counter_stop(dev);
	zassert_equal(err, 0, "Failed to stop counter (err %d)\n", err);
}

ZTEST(counter_basic, test_multiple_alarms)
{
	test_all_instances(test_multiple_instance,
			reliable_cancel_capable);
}

static void Test_alarm_stress(const struct device *dev)
{
	int err, ret;

	err = counter_stop(dev);
	zassert_equal(err, 0, "Failed to stop counter (err %d)\n", err);
	for (int i = 0; i  < STRESS_TEST_COUNT; i++) {
		counter_start(dev);
		alarm_cfg.flags = 0;
		alarm_cfg.ticks = counter_us_to_ticks(dev, DELAY);
		alarm_cfg.callback = test_counter_interrupt_fn;
		alarm_cfg.user_data = &alarm_cfg;

		err = counter_set_channel_alarm(dev, ALARM_CHANNEL_ID,
				&alarm_cfg);

		ret = check_set_channel_alarm_return_status(err);
		zassert_equal(0, ret,
				"%s: Counter setting alarm failed", dev->name);

		printk("Set alarm in %u sec (%u ticks)\n",
			(uint32_t)(counter_ticks_to_us(dev,
			alarm_cfg.ticks) / USEC_PER_SEC),
			alarm_cfg.ticks);

		k_sleep(K_MSEC(2000));

		err = counter_cancel_channel_alarm(dev, ALARM_CHANNEL_ID);
		ret = check_cancel_channel_alarm_return_status(err);
		zassert_equal(0, ret,
				"%s: Counter disabling alarm failed",
				dev->name);
		printk("Alarm was set for %d\n", i);
		counter_stop(dev);
	}

}

ZTEST(Stress_test, alarm_stress_test)
{
	test_all_instances(Test_alarm_stress,
			reliable_cancel_capable);
}
/*
 * pending alarm testcase
 */
void test_RTC_pending_alarm_instance(const struct device *dev)
{
	int err, ret, pen_ret;

	zassert_true(device_is_ready(dev), "device not ready.\n");
	printk("Testcase to Check Pending alarm\n");
	counter_start(dev);

	alarm_cfg.flags = 0;
	alarm_cfg.ticks = counter_us_to_ticks(dev, DELAY);
	alarm_cfg.callback = test_counter_interrupt_fn;
	alarm_cfg.user_data = &alarm_cfg;

	err = counter_set_channel_alarm(dev, ALARM_CHANNEL_ID, &alarm_cfg);
	printk("Set alarm in %u sec (%u ticks)\n",
		(uint32_t)(counter_ticks_to_us(dev,
		alarm_cfg.ticks) / USEC_PER_SEC),
		alarm_cfg.ticks);

	ret = check_set_channel_alarm_return_status(err);
	zassert_equal(0, ret, "%s: Counter setting alarm failed", dev->name);

	/* check for pending */
	pen_ret = counter_get_pending_int(dev);
	zassert_equal(0, pen_ret,
			"%s: There is a pending interrupt ", dev->name);
}

ZTEST(test_RTC, test_RTC_pending_alarm)
{
	test_all_instances(test_RTC_pending_alarm_instance,
			reliable_cancel_capable);
}
#endif

#if !RTC_SNIPPET_BUILD
ZTEST_SUITE(counter_basic, NULL, counter_setup, NULL, NULL, NULL);
ZTEST_SUITE(Stress_test, NULL, counter_setup, NULL, NULL, NULL);
#endif
ZTEST_SUITE(test_RTC, NULL, counter_setup, NULL, NULL, NULL);

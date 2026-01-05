/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
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

LOG_MODULE_REGISTER(test);

struct counter_alarm_cfg cntr_alarm_cfg;
struct counter_alarm_cfg cntr_alarm_cfg2;
struct counter_config_info counter_info;
static struct k_sem top_cnt_sem;
static struct k_sem alarm_cnt_sem;
static uint32_t alarm_cnt;

void *exp_user_data = (void *)199;

#define DOWN_COUNTER 0
#define DELAY   2000000
#define DELAY_2 3000000

/*1 minute */
#define CANCEL 100000000
/*20000000 */
#define TICKS_PER_SEC 32768
#define ALARM_CHANNEL_ID 0

#define STRESS_TEST_COUNT 3
uint32_t wrap_mode;
struct counter_alarm_cfg alarm_cfg;
struct counter_alarm_cfg alarm_cfg2;

#define DEVICE_DT_GET_AND_COMMA(node_id) DEVICE_DT_GET(node_id),
/* Generate a list of devices for all instances of the "compat" */
#define DEVS_FOR_DT_COMPAT(compat) \
	DT_FOREACH_STATUS_OKAY(compat, DEVICE_DT_GET_AND_COMMA)

static const struct device *const devices[] = {
#ifdef CONFIG_COUNTER_RTC_SNPS_DW /* CONFIG_COUNTER_DW_RTC */
	DEVS_FOR_DT_COMPAT(snps_dw_apb_rtc)
#endif
};

static const struct device *const period_devs[] = {
#ifdef CONFIG_COUNTER_MCUX_RTC
	DEVS_FOR_DT_COMPAT(nxp_kinetis_rtc)
#endif
#ifdef CONFIG_COUNTER_MCUX_LPC_RTC
		DEVS_FOR_DT_COMPAT(nxp_lpc_rtc)
#endif
		DEVS_FOR_DT_COMPAT(st_stm32_rtc)
};

typedef void (*counter_test_func_t)(const struct device *dev);
typedef bool (*counter_capability_func_t)(const struct device *dev);
/*
 * check number of channels available
 * static bool single_channel_alarm_capable(const struct device *dev)
 * {
 *	return (counter_get_num_of_channels(dev) > 0);
 * }
 */
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
	TC_PRINT("Successfully set channel alarm\n");
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
	TC_PRINT("Successfully cancelled alarm\n");
	return 0;
}

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
	k_sem_reset(&alarm_cnt_sem);
	if (!k_is_user_context()) {
		alarm_cnt = 0;
	}
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

	k_sem_init(&top_cnt_sem, 0, UINT_MAX);
	k_object_access_grant(&top_cnt_sem, k_current_get());

	k_sem_init(&alarm_cnt_sem, 0, UINT_MAX);
	k_object_access_grant(&alarm_cnt_sem, k_current_get());

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
#endif

#if CONFIG_TEST_RTC_WRAP_MODE
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

	k_sleep(K_MSEC(10000));

	err = counter_cancel_channel_alarm(dev, ALARM_CHANNEL_ID);
	ret = check_cancel_channel_alarm_return_status(err);
	zassert_equal(0, ret, "%s: Counter disabling alarm failed", dev->name);

	/* Stop the counter */
	err = counter_stop(dev);
	zassert_equal(err, 0, "Failed to stop counter (err %d)\n", err);
}
#endif

ZTEST(test_RTC, test_alarm_2sec)
{
#if CONFIG_TEST_RTC_WRAP_MODE
	test_all_instances(test_alarm_instance2sec,
			reliable_cancel_capable);
#else
	printk("RTC is in non-wrap mode enable wrap "
"mode to test this testcase\n");
	ztest_test_skip();
#endif
}

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

	k_sleep(K_MSEC(20000));

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
 * Verify that the counter initializes to zero
 * when testing only in Wrap Mode
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
		/* ztest_test_fail(); */
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
/*
 * Verify that the counter initializes to zero
 * when testing only in Wrap Mode
 */
#ifdef CONFIG_TEST_RTC_WRAP_MODE
static void test_counter_value_before_start(const struct device *dev)
{
	int ret;
	uint32_t now_ticks = -1;

	printk("Counter to test initial value of counter\n\n");
	zassert_true(device_is_ready(dev), "device not ready.\n");

	ret = counter_get_value(dev, &now_ticks);

	if (ret < 0) {
		printk("error while reading value");
		ztest_test_fail();
	}
	zassert_equal(now_ticks, 0, "Initial counter value is not 0 -> %d.",
			now_ticks);
	printk("Initial counter value is -> %d.\n", now_ticks);
}
#endif

ZTEST(test_RTC, test_counter_value_before_start)
{
#ifdef CONFIG_TEST_RTC_WRAP_MODE
	test_all_instances(test_counter_value_before_start,
			reliable_cancel_capable);
#else
	printk("RTC is in non-wrap mode enable wrap "
"mode to test this testcase\n");
	ztest_test_skip();
#endif
}
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
	/* k_busy_wait(500); */
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
		ret = check_set_channel_alarm_return_status(err);
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

/*
 * RTC - Wrap mode enabled
 */
#if CONFIG_TEST_RTC_WRAP_MODE
static void test_RTC_wrap_mode_instance(const struct device *counter_dev)
{

	int err, ret;
	uint32_t now_ticks;

	printk("Counter to Test RTC wrap mode\n\n");

	err = counter_cancel_channel_alarm(counter_dev, ALARM_CHANNEL_ID);
	ret = check_cancel_channel_alarm_return_status(err);
	zassert_equal(0, ret,
			"%s: Counter disabling alarm failed", counter_dev->name);

	zassert_true(device_is_ready(counter_dev), "device not ready.\n");
	counter_start(counter_dev);

	alarm_cfg.flags = 0;
	/* 1 min 40 sec 60000000 */

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

	printk("Set alarm in %u sec (%u ticks)\n",
			(uint32_t)(counter_ticks_to_us(counter_dev,
					alarm_cfg.ticks) / USEC_PER_SEC),
			alarm_cfg.ticks);

	k_sleep(K_MSEC(10000));

	err = counter_cancel_channel_alarm(counter_dev, ALARM_CHANNEL_ID);
	ret = check_set_channel_alarm_return_status(err);
	zassert_equal(0, ret,
			"%s: Counter disabling alarm failed", counter_dev->name);
	zassert_equal(wrap_mode, 0,
			"Wrap Mode failed as counter initial value is not zero\n");

	/* Stop the counter */
	err = counter_stop(counter_dev);
	zassert_equal(err, 0, "Failed to stop counter (err %d)\n", err);

}
#endif

ZTEST(test_RTC, test_RTC_wrap_mode)
{
#if CONFIG_TEST_RTC_WRAP_MODE
	test_all_instances(test_RTC_wrap_mode_instance,
			reliable_cancel_capable);
#else
	printk("Skipping testcase as wrap mode needs to be enabled in DTS\n");
	ztest_test_skip();
#endif
}

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
	k_sleep(K_MSEC(20000));
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

		k_sleep(K_MSEC(20000));

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
	/* k_sleep(K_MSEC(4000)) */

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

ZTEST_SUITE(counter_basic, NULL, counter_setup, NULL, NULL, NULL);
ZTEST_SUITE(test_RTC, NULL, counter_setup, NULL, NULL, NULL);
ZTEST_SUITE(Stress_test, NULL, counter_setup, NULL, NULL, NULL);

#include "test_lptimer.h"

uint32_t cb_count;
uint32_t lp_cnt;

struct counter_alarm_cfg cntr_alarm_cfg;
struct counter_alarm_cfg cntr_alarm_cfg2;

static const struct device *const devices[] = {
#ifdef CONFIG_COUNTER_SNPS_DW
    #if DT_NODE_HAS_STATUS(TIMER1, okay)
	DEVICE_DT_GET(TIMER1),
    #elif DT_NODE_HAS_STATUS(TIMER2, okay)
	DEVICE_DT_GET(TIMER2),
    #elif DT_NODE_HAS_STATUS(TIMER3, okay)
	DEVICE_DT_GET(TIMER3),
    #else
	DEVICE_DT_GET(TIMER0),
    #endif
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
	#ifdef CONFIG_COUNTER_SNPS_DW
	#if DT_NODE_HAS_STATUS(TIMER1, okay)
	DEVICE_DT_GET(TIMER1),
    #elif DT_NODE_HAS_STATUS(TIMER2, okay)
	DEVICE_DT_GET(TIMER2),
    #elif DT_NODE_HAS_STATUS(TIMER3, okay)
	DEVICE_DT_GET(TIMER3),
    #else
	DEVICE_DT_GET(TIMER0),
    #endif
#endif
#ifdef CONFIG_COUNTER_RTC_SNPS_DW
	DEVICE_DT_GET(DT_NODELABEL(rtc0)),
#endif
};

typedef void (*counter_test_func_t)(const struct device *dev);

typedef bool (*counter_capability_func_t)(const struct device *dev);
/*
 *
 * Function to check gpio_pin_toggle_dt() return status
 *
 *
 */
int check_pin_toggle(int ret)
{
	switch (ret) {
	case 0:
		LOG_INF("LED pin toggled\n");
		return 0;
	default:
		if (ret == -EIO) {
			LOG_INF("I/O error when toggling LED pin\n");
			return -1;
		} else if (ret == -EWOULDBLOCK) {
			LOG_INF("Operation would block\n");
			return -1;
		}
		break;
	}
	return -1;
}

/*
 *
 * Function to check gpio_pin_set_dt() return status
 *
 *
 */
int check_pin_set_conf(int ret)
{
	switch (ret) {
	case 0:
		LOG_INF("logical level of an output pin is successful\n");
		return 0;
	default:
		if (ret == -EIO) {
			LOG_INF("error when accessing\n");
			return -1;
		} else if (ret == -EWOULDBLOCK) {
			LOG_INF("Operation would block\n");
			return -1;
		}
		break;
	}
	return -1;
}

int check_cb_res(int ret)
{
	switch (ret) {
	case 0:
		LOG_INF("Callback set\n");
		return 0;
	default:
		if (ret == -ENOSYS) {
			LOG_INF("operation is not implemented by the driver.\n");
			ztest_test_skip();
			return -1;
		} else if (ret < 0) {
			LOG_INF("Other negative errno code on failure\n");
			return -1;
		}
		break;
	}
	return -1;
}

/*
 *
 * Function to check gpio_pin_interrupt_configure_dt() return status
 *
 *
 */
int check_int_conf(int ret)
{
	switch (ret) {
	case 0:
		LOG_INF("configured interrupt\n");
		return 0;
	default:
		if (ret == -ENOSYS) {
			LOG_INF("operation is not implemented by the driver.\n");
			ztest_test_skip();
			return -1;
		} else if (ret == -ENOTSUP) {
			LOG_INF("the configuration options is not supported\n");
			ztest_test_skip();
			return -1;
		} else if (ret == -EINVAL) {
			LOG_INF("Invalid argument.\n");
			return -1;
		} else if (ret == -EBUSY) {
			LOG_INF("Interrupt line required to configure pin interrupt "
				"is already in use.\n");
			return -1;
		} else if (ret == -EIO) {
			LOG_INF("I/O error when accessing\n");
			return -1;
		} else if (ret == -EWOULDBLOCK) {
			LOG_INF("Operation would block\n");
			return -1;
		}
		break;
	}
	return -1;
}

/*
 *
 * Function to check gpio_pin_configure_dt() return status
 *
 *
 */
int check_pin_conf(int ret)
{
	switch (ret) {
	case 0:
		LOG_INF("configured the pin\n");
		return 0;
	default:
		if (ret == -ENOTSUP) {
			LOG_INF("Simultaneous pin in/out mode is not supported.\n");
			return -1;
		} else if (ret == -EINVAL) {
			LOG_INF("Invalid argument.\n");
			return -1;
		} else if (ret == -EIO) {
			LOG_INF("error when accessing\n");
			return -1;
		} else if (ret == -EWOULDBLOCK) {
			LOG_INF("Operation would block\n");
			return -1;
		}
		break;
	}
	return -1;
}

int check_set_channel_alarm_return_status(int ret)
{
	switch (ret) {
	case 0:
		LOG_INF("Successfully set channel alarm\n");
		return 0;
	default:
		if (ret == -EINVAL) {
			LOG_INF("Alarm settings invalid\n");
			return -1;
		} else if (ret == -ENOTSUP) {
			LOG_INF("\nAlarm request not supported or "
				"the counter was not started yet.\n");
			return -1;
		} else if (ret == -ETIME) {
			LOG_INF("\nabsolute alarm was set too late\n");
			return -1;
		} else if (ret == -EBUSY) {
			LOG_INF("\nalarm is already active\n");
			return -1;
		} else if (ret != 0) {
			LOG_INF("Error\n");
			LOG_INF("API is not enabled\n");
			return -1;
		}
	}
	return 0;
}

int check_cancel_channel_alarm_return_status(int ret)
{
	switch (ret) {
	case 0:
		LOG_INF("Successfully cancelled alarm\n");
		return 0;
	default:
		if (ret == -ENOTSUP) {
			LOG_INF("\nAlarm request not supported or "
				"the counter was not started yet.\n");
			return -1;
		} else if (ret != 0) {
			LOG_INF("Error\n");
			LOG_INF("API is not enabled\n");
			return -1;
		}
	}
	return 0;
}

static inline uint32_t get_counter_period_us(const struct device *dev)
{
	for (int i = 0; i < ARRAY_SIZE(period_devs); i++) {
		LOG_INF("Inside get counter period_us %s\n",
			period_devs[i]->name);
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
			"%s:Setting top value to default failed", dev->name);
	err = counter_stop(dev);
	zassert_equal(0, err, "%s:Counter failed to stop", dev->name);
}

static void test_all_instances(counter_test_func_t func,
				counter_capability_func_t capability_check)
{
	zassert_true(ARRAY_SIZE(devices) > 0, "No device found");
	for (int i = 0; i < ARRAY_SIZE(devices); i++) {
		counter_setup_instance(devices[i]);
		if ((capability_check == NULL) ||
		     capability_check(devices[i])) {
			LOG_INF("Testing %s\n", devices[i]->name);
			func(devices[i]);
		} else {
			LOG_INF("Skipped for %s\n", devices[i]->name);
			ztest_test_skip();
		}
		counter_tear_down_instance(devices[i]);
		/* Allow logs to be printed. */
		k_sleep(K_MSEC(100));
	}
}

static bool set_top_value_capable(const struct device *dev)
{
	struct counter_top_cfg cfg = {
		.ticks = counter_get_top_value(dev) - 1
	};
	int err;

	err = counter_set_top_value(dev, &cfg);
	if (err == -ENOTSUP) {
		return false;
	}

	cfg.ticks++;
	err = counter_set_top_value(dev, &cfg);
	if (err == -ENOTSUP) {
		return false;
	}

	return true;
}

static void top_handler(const struct device *dev, void *user_data)
{
	zassert_true(user_data == exp_user_data,
			"%s:Unexpected callback", dev->name);
	if (IS_ENABLED(CONFIG_ZERO_LATENCY_IRQS)) {
		top_cnt++;

		return;
	}

	k_sem_give(&top_cnt_sem);
}

static void test_set_top_value_with_alarm_instance(const struct device *dev)
{
	int err;
	uint32_t cnt;
	uint32_t top_value;
	uint32_t counter_period_us;
	uint32_t top_handler_cnt;

	struct counter_top_cfg top_cfg = {
		.callback = top_handler,
		.user_data = exp_user_data,
		.flags = 0
	};

	k_sem_reset(&top_cnt_sem);
	top_cnt = 0;

	err = counter_cancel_channel_alarm(dev, ALARM_CHANNEL_ID);

	counter_period_us = get_counter_period_us(dev);
	top_cfg.ticks = counter_us_to_ticks(dev, counter_period_us);
	err = counter_start(dev);
	zassert_equal(0, err, "%s:Counter failed to start", dev->name);

	k_busy_wait(5000);

	err = counter_get_value(dev, &cnt);
	zassert_true(err == 0, "%s:Counter read failed (err:%d)", dev->name,
		     err);
	if (counter_is_counting_up(dev)) {
		err = (cnt > 0) ? 0 : 1;
	} else {
		top_value = counter_get_top_value(dev);
		err = (cnt < top_value) ? 0 : 1;
	}
	zassert_true(err == 0, "%s: Counter should progress", dev->name);

	err = counter_set_top_value(dev, &top_cfg);
	zassert_equal(0, err, "%s: Counter failed to set top value (err: %d)",
			dev->name, err);

	k_busy_wait((uint32_t)(5.2 * counter_period_us));

	top_handler_cnt = IS_ENABLED(CONFIG_ZERO_LATENCY_IRQS) ?
		top_cnt : k_sem_count_get(&top_cnt_sem);
	zassert_true(top_handler_cnt == 5U,
			"%s:Unexpected number of turnarounds (%d).",
			dev->name, top_handler_cnt);
}

/** @brief Validate setting a custom top value with callback enabled. */
ZTEST(counter_basic, test_set_top_value_with_alarm)
{
	test_all_instances(test_set_top_value_with_alarm_instance,
			   set_top_value_capable);
}

static void test_set_top_value_without_alarm_instance(const struct device *dev)
{
	int err;
	uint32_t cnt;
	uint32_t top_value;
	uint32_t counter_period_us;
	struct counter_top_cfg top_cfg = {
		.callback = NULL,
		.user_data = NULL,
		.flags = 0
	};

	err = counter_cancel_channel_alarm(dev, ALARM_CHANNEL_ID);

	counter_period_us = get_counter_period_us(dev);
	top_cfg.ticks = counter_us_to_ticks(dev, counter_period_us);
	err = counter_start(dev);
	zassert_equal(0, err, "%s:Counter failed to start", dev->name);

	k_busy_wait(5000);

	err = counter_get_value(dev, &cnt);
	zassert_true(err == 0, "%s:Counter read failed (err:%d)", dev->name,
		     err);
	if (counter_is_counting_up(dev)) {
		err = (cnt > 0) ? 0 : 1;
	} else {
		top_value = counter_get_top_value(dev);
		err = (cnt < top_value) ? 0 : 1;
	}
	zassert_true(err == 0, "%s: Counter should progress", dev->name);

	err = counter_set_top_value(dev, &top_cfg);
	zassert_equal(0, err, "%s: Counter failed to set top value (err: %d)",
			dev->name, err);

	zassert_true(counter_get_top_value(dev) == top_cfg.ticks,
			"%s: new top value not in use.",
			dev->name);
}

/** @brief Validate setting a custom top value without callback. */
ZTEST_USER(counter_no_callback, test_set_top_value_without_alarm)
{
	test_all_instances(test_set_top_value_without_alarm_instance,
			   set_top_value_capable);
}

static void alarm_handler(const struct device *dev, uint8_t chan_id,
			  uint32_t counter,
			  void *user_data)
{
	/* Arbitrary limit for alarm processing - time between hw expiration
	 * and read-out from counter in the handler.
	 */
	static const uint64_t processing_limit_us = 1000;
	uint32_t now;
	int err;
	uint32_t top;
	uint32_t diff;

	err = counter_get_value(dev, &now);
	zassert_true(err == 0, "%s:Counter read failed (err:%d)",
		     dev->name, err);

	top = counter_get_top_value(dev);
	if (counter_is_counting_up(dev)) {
		diff = (now < counter) ?
			(now + top - counter) : (now - counter);
	} else {
		diff = (now > counter) ?
			(counter + top - now) : (counter - now);
	}

	zassert_true(diff <= counter_us_to_ticks(dev, processing_limit_us),
			"Unexpected distance between reported alarm value(%u) "
			"and actual counter value (%u), top:%d (processing "
			"time limit (%d us) might be exceeded?",
			counter, now, top, processing_limit_us);

	if (user_data) {
		zassert_true(&cntr_alarm_cfg == user_data,
			"%s:Unexpected callback", dev->name);
	}

	if (IS_ENABLED(CONFIG_ZERO_LATENCY_IRQS)) {
		alarm_cnt++;
		return;
	}
	zassert_true(k_is_in_isr(), "%s:Expected interrupt context",
			dev->name);
	k_sem_give(&alarm_cnt_sem);
}

static void test_single_shot_alarm_instance(const struct device *dev, bool set_top)
{
	int err;
	uint32_t ticks;
	uint32_t cnt;
	uint32_t counter_period_us;
	struct counter_top_cfg top_cfg = {
		.callback = top_handler,
		.user_data = exp_user_data,
		.flags = 0
	};

	err = counter_cancel_channel_alarm(dev, ALARM_CHANNEL_ID);
	check_cancel_channel_alarm_return_status(err);
	counter_period_us = get_counter_period_us(dev);
	ticks = counter_us_to_ticks(dev, counter_period_us);
	top_cfg.ticks = ticks;

	cntr_alarm_cfg.flags = 0;
	cntr_alarm_cfg.callback = alarm_handler;
	cntr_alarm_cfg.user_data = &cntr_alarm_cfg;

	k_sem_reset(&alarm_cnt_sem);
	alarm_cnt = 0;

	if (counter_get_num_of_channels(dev) < 1U) {
		/* Counter does not support any alarm, skip the test */
		ztest_test_skip();  /*  Skip the test if no alarms are supported */
		return;
	}

	err = counter_start(dev);
	zassert_equal(0, err, "%s:Counter failed to start", dev->name);
	uint32_t top = counter_get_top_value(dev);

	TC_PRINT("top value is %d and ticks value is %d",
			top, cntr_alarm_cfg.ticks);
	if (set_top) {
		err = counter_set_top_value(dev, &top_cfg);

		zassert_equal(0, err,
			     "%s:Counter failed to set top value", dev->name);
		uint32_t top_c = counter_get_top_value(dev);

		TC_PRINT("top value is %d and ticks value is %d\n",
				top_c, cntr_alarm_cfg.ticks);

		cntr_alarm_cfg.ticks = ticks + 1;
		TC_PRINT("ticks value is %d\n", cntr_alarm_cfg.ticks);
		err = counter_set_channel_alarm(dev, 0, &cntr_alarm_cfg);
		/* int ret1 = check_set_channel_alarm_return_status(err); */
		cntr_alarm_cfg.ticks = ticks - 1;
	}

	k_sleep(K_MSEC(20000));

	cntr_alarm_cfg.ticks = ticks;
	err = counter_set_channel_alarm(dev, 0, &cntr_alarm_cfg);
	zassert_equal(0, err, "%s:Counter set alarm failed (err:%d)",
			dev->name, err);

	k_busy_wait(2 * (uint32_t)counter_ticks_to_us(dev, ticks));

	cnt = IS_ENABLED(CONFIG_ZERO_LATENCY_IRQS) ?
		alarm_cnt : k_sem_count_get(&alarm_cnt_sem);
	TC_PRINT("count value is %d\n", cnt);
	zassert_equal(1, cnt, "%s:Expecting alarm callback", dev->name);
	k_busy_wait((uint32_t)(1.5 * counter_ticks_to_us(dev, ticks)));
	cnt = IS_ENABLED(CONFIG_ZERO_LATENCY_IRQS) ?
		alarm_cnt : k_sem_count_get(&alarm_cnt_sem);
	zassert_equal(1, cnt, "%s:Expecting alarm callback", dev->name);

	err = counter_cancel_channel_alarm(dev, 0);
	zassert_equal(0, err, "%s:Counter disabling alarm failed", dev->name);

	top_cfg.ticks = counter_get_max_top_value(dev);
	top_cfg.callback = NULL;
	top_cfg.user_data = NULL;
	err = counter_set_top_value(dev, &top_cfg);
	if (err == -ENOTSUP) {
		/* If resetting is not support, attempt without reset. */
		top_cfg.flags = COUNTER_TOP_CFG_DONT_RESET;
		err = counter_set_top_value(dev, &top_cfg);

	}
	zassert_true((err == 0) || (err == -ENOTSUP),
			"%s:Setting top value to default failed", dev->name);

	err = counter_stop(dev);
	zassert_equal(0, err, "%s:Counter failed to stop", dev->name);
}

void test_single_shot_alarm_notop_instance(const struct device *dev)
{
	test_single_shot_alarm_instance(dev, false);
}

void test_single_shot_alarm_top_instance(const struct device *dev)
{
	test_single_shot_alarm_instance(dev, true);
}

static bool single_channel_alarm_capable(const struct device *dev)
{
	return (counter_get_num_of_channels(dev) > 0);
}

static bool single_channel_alarm_and_custom_top_capable(const struct device *dev)
{
	return single_channel_alarm_capable(dev) &&
		set_top_value_capable(dev);
}

/** @brief Validate single-shot alarm without changing top value. */
ZTEST(counter_basic, test_single_shot_alarm_notop)
{
	test_all_instances(test_single_shot_alarm_notop_instance,
			   single_channel_alarm_capable);
}

/** @brief Validate single-shot alarm with custom top value. */
ZTEST(counter_basic, test_single_shot_alarm_top)
{
	/* test_single_shot_alarm_top_instance currently disabled */
	ztest_test_skip();
}

static void *clbk_data[10];

static void alarm_handler2(const struct device *dev, uint8_t chan_id,
			   uint32_t counter,
			   void *user_data)
{
	if (IS_ENABLED(CONFIG_ZERO_LATENCY_IRQS)) {
		clbk_data[alarm_cnt] = user_data;
		alarm_cnt++;

		return;
	}

	clbk_data[k_sem_count_get(&alarm_cnt_sem)] = user_data;
	k_sem_give(&alarm_cnt_sem);
}

/*
 * Two alarms set. First alarm is absolute, second relative. Because
 * setting of both alarms is delayed it is expected that second alarm
 * will expire first (relative to the time called) while first alarm
 * will expire after next wrap around.
 */
static void test_multiple_alarms_instance(const struct device *dev)
{
	int err;
	uint32_t ticks;
	uint32_t cnt;
	uint32_t counter_period_us;
	struct counter_top_cfg top_cfg = {
		.callback = top_handler,
		.user_data = exp_user_data,
		.flags = 0
	};

	err = counter_cancel_channel_alarm(dev, ALARM_CHANNEL_ID);
	counter_period_us = get_counter_period_us(dev);
	ticks = counter_us_to_ticks(dev, counter_period_us);

	err = counter_get_value(dev, &(top_cfg.ticks));
	zassert_equal(0, err, "%s:Counter get value failed", dev->name);
	top_cfg.ticks += ticks;

	cntr_alarm_cfg.flags = COUNTER_ALARM_CFG_ABSOLUTE;
	cntr_alarm_cfg.ticks = counter_us_to_ticks(dev, 2000);
	cntr_alarm_cfg.callback = alarm_handler2;
	cntr_alarm_cfg.user_data = &cntr_alarm_cfg;

	cntr_alarm_cfg2.flags = 0;
	cntr_alarm_cfg2.ticks = counter_us_to_ticks(dev, 2000);
	cntr_alarm_cfg2.callback = alarm_handler2;
	cntr_alarm_cfg2.user_data = &cntr_alarm_cfg2;

	k_sem_reset(&alarm_cnt_sem);
	alarm_cnt = 0;

	if (counter_get_num_of_channels(dev) < 2U) {
		/* Counter does not support two alarms */
		return;
	}

	err = counter_start(dev);
	zassert_equal(0, err, "%s:Counter failed to start", dev->name);

	if (set_top_value_capable(dev)) {
		err = counter_set_top_value(dev, &top_cfg);
		zassert_equal(0, err, "%s:Counter failed to set top value", dev->name);
	} else {
		/* Counter does not support top value, do not run this test
		 * as it might take a long time to wrap and trigger the alarm
		 * resulting in test failures.
		 */
		return;
	}

	k_busy_wait(3*(uint32_t)counter_ticks_to_us(dev, cntr_alarm_cfg.ticks));

	err = counter_set_channel_alarm(dev, 0, &cntr_alarm_cfg);
	zassert_equal(0, err, "%s:Counter set alarm failed", dev->name);

	err = counter_set_channel_alarm(dev, 1, &cntr_alarm_cfg2);
	zassert_equal(0, err, "%s:Counter set alarm failed", dev->name);

	k_busy_wait((uint32_t)(1.2 * counter_ticks_to_us(dev, ticks * 2U)));

	cnt = IS_ENABLED(CONFIG_ZERO_LATENCY_IRQS) ?
		alarm_cnt : k_sem_count_get(&alarm_cnt_sem);
	zassert_equal(2, cnt,
			"%s:Invalid number of callbacks %d (expected:%d)",
			dev->name, cnt, 2);

	zassert_equal(&cntr_alarm_cfg2, clbk_data[0],
			"%s:Expected different order or callbacks",
			dev->name);
	zassert_equal(&cntr_alarm_cfg, clbk_data[1],
			"%s:Expected different order or callbacks",
			dev->name);

	/* tear down */
	err = counter_cancel_channel_alarm(dev, 0);
	zassert_equal(0, err, "%s:Counter disabling alarm failed", dev->name);

	err = counter_cancel_channel_alarm(dev, 1);
	zassert_equal(0, err, "%s:Counter disabling alarm failed", dev->name);
}

static bool multiple_channel_alarm_capable(const struct device *dev)
{
	return (counter_get_num_of_channels(dev) > 1);
}

/** @brief Validate ordering of absolute and relative alarms on two channels.*/
ZTEST(counter_basic, test_multiple_alarms)
{
	test_all_instances(test_multiple_alarms_instance,
			   multiple_channel_alarm_capable);
}

static void test_all_channels_instance(const struct device *dev)
{
	int err;
	const int n = 10;
	int nchan = 0;
	bool limit_reached = false;
	struct counter_alarm_cfg alarm_cfgs;
	uint32_t ticks;
	uint32_t cnt;
	uint32_t counter_period_us;

	counter_period_us = get_counter_period_us(dev);
	ticks = counter_us_to_ticks(dev, counter_period_us);

	alarm_cfgs.flags = 0;
	alarm_cfgs.ticks = ticks;
	alarm_cfgs.callback = alarm_handler2;
	alarm_cfgs.user_data = NULL;

	err = counter_start(dev);
	zassert_equal(0, err, "%s:Counter failed to start", dev->name);

	for (int i = 0; i < n; i++) {
		err = counter_set_channel_alarm(dev, i, &alarm_cfgs);
		if ((err == 0) && !limit_reached) {
			nchan++;
		} else if (err == -ENOTSUP) {
			limit_reached = true;
		} else {
			zassert_equal(0, 1,
			   "%s:Unexpected error on setting alarm", dev->name);
		}
	}

	k_busy_wait((uint32_t)(1.5 * counter_ticks_to_us(dev, ticks)));
	cnt = IS_ENABLED(CONFIG_ZERO_LATENCY_IRQS) ?
		alarm_cnt : k_sem_count_get(&alarm_cnt_sem);
	zassert_equal(nchan, cnt,
			"%s:Expecting alarm callback", dev->name);

	for (int i = 0; i < nchan; i++) {
		err = counter_cancel_channel_alarm(dev, i);
		zassert_equal(0, err,
			"%s:Unexpected error on disabling alarm", dev->name);
	}

	for (int i = nchan; i < n; i++) {
		err = counter_cancel_channel_alarm(dev, i);
		zassert_equal(-ENOTSUP, err,
			"%s:Unexpected error on disabling alarm", dev->name);
	}
}
/** @brief Validate alarm configuration across all supported channels. */
ZTEST(counter_basic, test_all_channels)
{
	test_all_instances(test_all_channels_instance,
			   single_channel_alarm_capable);
}

/**
 * Test validates if alarm set too late (current tick or
 * current tick + 1)
 * results in callback being called.
 */
static void test_late_alarm_instance(const struct device *dev)
{
	int err;
	uint32_t cnt;
	uint32_t tick_us = (uint32_t)counter_ticks_to_us(dev, 1);

	err = counter_cancel_channel_alarm(dev, ALARM_CHANNEL_ID);

	uint32_t guard = counter_us_to_ticks(dev, 200);
	struct counter_alarm_cfg alarm_cfg = {
		.callback = alarm_handler,
		.flags = COUNTER_ALARM_CFG_ABSOLUTE |
			 COUNTER_ALARM_CFG_EXPIRE_WHEN_LATE,
		.user_data = NULL
	};

	err = counter_set_guard_period(dev, guard,
					COUNTER_GUARD_PERIOD_LATE_TO_SET);
	zassert_equal(0, err, "%s:Unexpected error", dev->name);

	err = counter_start(dev);
	zassert_equal(0, err, "%s:Unexpected error", dev->name);

	k_busy_wait(2 * tick_us);

	alarm_cfg.ticks = 0;
	err = counter_set_channel_alarm(dev, 0, &alarm_cfg);
	zassert_equal(-ETIME, err, "%s:Unexpected error (%d)", dev->name, err);

	/* wait couple of ticks */
	k_busy_wait(5 * tick_us);

	cnt = IS_ENABLED(CONFIG_ZERO_LATENCY_IRQS) ?
		alarm_cnt : k_sem_count_get(&alarm_cnt_sem);
	zassert_equal(1, cnt,
			"%s:Expected %d callbacks, got %d\n",
			dev->name, 1, cnt);

	err = counter_get_value(dev, &(alarm_cfg.ticks));
	zassert_true(err == 0, "%s:Counter read failed (err:%d)", dev->name,
		     err);

	err = counter_set_channel_alarm(dev, 0, &alarm_cfg);
	zassert_equal(-ETIME, err, "%s:Failed to set an alarm (err:%d)",
			dev->name, err);

	/* wait to ensure that tick+1 timeout will expire. */
	k_busy_wait(3 * tick_us);

	cnt = IS_ENABLED(CONFIG_ZERO_LATENCY_IRQS) ?
		alarm_cnt : k_sem_count_get(&alarm_cnt_sem);
	zassert_equal(2, cnt,
			"%s:Expected %d callbacks, got %d\n",
			dev->name, 2, cnt);
}

static void test_late_alarm_error_instance(const struct device *dev)
{
	int err;
	uint32_t tick_us = (uint32_t)counter_ticks_to_us(dev, 1);
	uint32_t guard = counter_us_to_ticks(dev, 200);

	err = counter_cancel_channel_alarm(dev, ALARM_CHANNEL_ID);

	struct counter_alarm_cfg alarm_cfg = {
		.callback = alarm_handler,
		.flags = COUNTER_ALARM_CFG_ABSOLUTE,
		.user_data = NULL
	};

	err = counter_set_guard_period(dev, guard,
					COUNTER_GUARD_PERIOD_LATE_TO_SET);
	zassert_equal(0, err, "%s:Unexpected error", dev->name);

	err = counter_start(dev);
	zassert_equal(0, err, "%s:Unexpected error", dev->name);

	k_busy_wait(2 * tick_us);

	alarm_cfg.ticks = 0;
	err = counter_set_channel_alarm(dev, 0, &alarm_cfg);
	zassert_equal(-ETIME, err,
			"%s:Failed to detect late setting (err:%d)",
			dev->name, err);

	err = counter_get_value(dev, &(alarm_cfg.ticks));
	zassert_true(err == 0, "%s:Counter read failed (err:%d)", dev->name,
		     err);

	err = counter_set_channel_alarm(dev, 0, &alarm_cfg);
	zassert_equal(-ETIME, err,
			"%s:Counter failed to detect late setting (err:%d)",
			dev->name, err);
}

static bool late_detection_capable(const struct device *dev)
{
	uint32_t guard = counter_get_guard_period(dev,
					COUNTER_GUARD_PERIOD_LATE_TO_SET);
	int err = counter_set_guard_period(dev, guard,
					COUNTER_GUARD_PERIOD_LATE_TO_SET);

	if (err == -ENOTSUP) {
		return false;
	}

	if (single_channel_alarm_capable(dev) == false) {
		return false;
	}

	return true;
}

/** @brief Validate late alarm handling with EXPIRE_WHEN_LATE behavior. */
ZTEST(counter_basic, test_late_alarm)
{
	test_all_instances(test_late_alarm_instance, late_detection_capable);
}

/**@brief Validate late alarm returns ETIME when expire-when-late is not set.*/
ZTEST(counter_basic, test_late_alarm_error)
{
	test_all_instances(test_late_alarm_error_instance,
			   late_detection_capable);
}

/*
 * Test checks if cancelled alarm does not get triggered
 * when new alarm is
 * configured at the point where previous alarm was about to expire.
 */
static void test_cancelled_alarm_does_not_expire_instance(const struct device *dev)
{
	int err;
	uint32_t cnt;
	uint32_t us = 1000;
	uint32_t ticks = counter_us_to_ticks(dev, us);
	uint32_t top = counter_get_top_value(dev);

	us = (uint32_t)counter_ticks_to_us(dev, ticks);

	err = counter_cancel_channel_alarm(dev, ALARM_CHANNEL_ID);

	struct counter_alarm_cfg alarm_cfg = {
		.callback = alarm_handler,
		.flags = COUNTER_ALARM_CFG_ABSOLUTE,
		.user_data = NULL
	};

	err = counter_start(dev);
	zassert_equal(0, err, "%s:Unexpected error", dev->name);

	for (int i = 0; i < us / 2; ++i) {
		err = counter_get_value(dev, &(alarm_cfg.ticks));
		zassert_true(err == 0, "%s:Counter read failed (err:%d)",
			     dev->name, err);

		alarm_cfg.ticks	+= ticks;
		alarm_cfg.ticks = alarm_cfg.ticks % top;
		err = counter_set_channel_alarm(dev, 0, &alarm_cfg);
		zassert_equal(0, err, "%s:Failed to set an alarm (err:%d)",
				dev->name, err);

		err = counter_cancel_channel_alarm(dev, 0);
		zassert_equal(0, err, "%s:Failed to cancel an alarm (err:%d)",
				dev->name, err);

		k_busy_wait(us / 2 + i);

		alarm_cfg.ticks = alarm_cfg.ticks + 2 * ticks;
		alarm_cfg.ticks = alarm_cfg.ticks % top;
		err = counter_set_channel_alarm(dev, 0, &alarm_cfg);
		zassert_equal(0, err, "%s:Failed to set an alarm (err:%d)",
				dev->name, err);

		/* wait to ensure that tick+1 timeout will expire. */
		k_busy_wait(us);

		err = counter_cancel_channel_alarm(dev, 0);
		zassert_equal(0, err, "%s:Failed to cancel an alarm (err:%d)",
					dev->name, err);

		cnt = IS_ENABLED(CONFIG_ZERO_LATENCY_IRQS) ?
			alarm_cnt : k_sem_count_get(&alarm_cnt_sem);
		zassert_equal(0, cnt,
				"%s:Expected %d callbacks, got %d (i:%d)\n",
				dev->name, 0, cnt, i);
	}
}

static bool reliable_cancel_capable(const struct device *dev)
{
/* Alif lptimer */
#ifdef CONFIG_COUNTER_SNPS_DW
	if (single_channel_alarm_capable(dev)) {
		return true;
	}
#endif
	return false;
}

/** @brief Validate cancelling alarms near expiry does not trigger callbacks. */
ZTEST(counter_basic, test_cancelled_alarm_does_not_expire)
{
	if (0) {
		test_all_instances(test_cancelled_alarm_does_not_expire_instance,
				   reliable_cancel_capable);
	} else {
		ztest_test_skip();
	}
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
		LOG_INF("Failed to read counter value (err %d)", err);
		return;
	}

	now_usec = counter_ticks_to_us(counter_dev, now_ticks);
	now_sec = (int)(now_usec / USEC_PER_SEC);

	LOG_INF("!!! Alarm !!!\n");
	LOG_INF("Now:%u\n", now_sec);

	/* Set a new alarm with a double length duration */
	config->ticks = config->ticks * 2U;

	LOG_INF("Set alarm in %u sec (%u ticks)\n",
	       (uint32_t)(counter_ticks_to_us(counter_dev,
					   config->ticks) / USEC_PER_SEC),
	       config->ticks);
	LOG_INF("About to set alarm for 2nd time\n");
	err = counter_set_channel_alarm(counter_dev, ALARM_CHANNEL_ID,
					user_data);
	if (err != 0) {
		LOG_INF("Alarm could not be set\n");
	}
}

static void test_alarm_32MhzClockInput_instance(const struct device *dev)
{
	int err, ret;

	err = counter_cancel_channel_alarm(dev, ALARM_CHANNEL_ID);

	LOG_INF("Counter Test alarm\n");

	err = counter_start(dev);
	zassert_equal(0, err, "%s:Counter failed to start", dev->name);

	alarm_cfg.flags = 0;
	alarm_cfg.ticks = counter_us_to_ticks(dev, DELAY);
	alarm_cfg.callback = test_counter_interrupt_fn;
	alarm_cfg.user_data = &alarm_cfg;

	err = counter_set_channel_alarm(dev, ALARM_CHANNEL_ID,
					&alarm_cfg);

	ret = check_set_channel_alarm_return_status(err);
	zassert_equal(0, ret, "%s:Counter setting alarm failed", dev->name);

	LOG_INF("Set alarm in %u sec (%u ticks)\n",
	       (uint32_t)(counter_ticks_to_us(dev,
					   alarm_cfg.ticks) / USEC_PER_SEC),
	       alarm_cfg.ticks);

	/* wait for 15sec */
	k_sleep(K_MSEC(15000));

	err = counter_cancel_channel_alarm(dev, ALARM_CHANNEL_ID);
	ret = check_cancel_channel_alarm_return_status(err);
	zassert_equal(0, ret, "%s:Counter disabling alarm failed", dev->name);
}

/** @brief Validate alarm behavior when using 32 MHz clock input mode. */
ZTEST(test_LPTimer, test_alarm_32MhzClockInput)
{
	test_all_instances(test_alarm_32MhzClockInput_instance,
			reliable_cancel_capable);
}

static void test_counter_interrupt_cmfn(const struct device *counter_dev,
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
		LOG_INF("Failed to read counter value (err %d)", err);
		return;
	}

	now_usec = counter_ticks_to_us(counter_dev, now_ticks);
	now_sec = (int)(now_usec / USEC_PER_SEC);

	LOG_INF("!!! Alarm !!!\n");
	LOG_INF("Now:%u\n", now_sec);

	/* Set a new alarm with a double length duration */
	config->ticks = config->ticks * 2U;

	LOG_INF("Set alarm in %u sec (%u ticks)\n",
	       (uint32_t)(counter_ticks_to_us(counter_dev,
					   config->ticks) / USEC_PER_SEC),
	       config->ticks);

	err = counter_set_channel_alarm(counter_dev, ALARM_CHANNEL_ID,
					user_data);
	if (err != 0) {
		LOG_INF("Alarm could not be set\n");
	}
}

static void test_Compare_alarm_instance(const struct device *dev)
{
	int err, ret;
	uint32_t frequency;

	err = counter_cancel_channel_alarm(dev, ALARM_CHANNEL_ID);
	LOG_INF("Counter Test alarm\n");

	err = counter_start(dev);
	zassert_equal(0, err, "%s:Counter failed to start", dev->name);

	alarm_cfg.flags = 0;
	alarm_cfg.ticks = counter_us_to_ticks(dev, DELAY);
	alarm_cfg.callback = test_counter_interrupt_cmfn;
	alarm_cfg.user_data = &alarm_cfg;

	err = counter_set_channel_alarm(dev, ALARM_CHANNEL_ID,
					&alarm_cfg);

	ret = check_set_channel_alarm_return_status(err);
	zassert_equal(0, ret, "%s:Counter setting alarm failed", dev->name);

	LOG_INF("Set alarm in %u sec (%u ticks)\n",
	       (uint32_t)(counter_ticks_to_us(dev,
					   alarm_cfg.ticks) / USEC_PER_SEC),
	       alarm_cfg.ticks);

	/* Use actual timer frequency instead of hardcoded TICKS_PER_SEC */
	frequency = counter_get_frequency(dev);
	if (frequency == 0) {
		/* Fallback if frequency not available */
		frequency = TICKS_PER_SEC;
	}

	/* Calculate expected duration in seconds using actual frequency */
	uint32_t expected_seconds = alarm_cfg.ticks / frequency;

	zassert_equal(expected_seconds, (uint32_t)(counter_ticks_to_us(dev,
					   alarm_cfg.ticks) / USEC_PER_SEC),
	"%s:Counter Triggered alarm at non expected duration", dev->name);

	k_sleep(K_MSEC(5000));

	err = counter_cancel_channel_alarm(dev, ALARM_CHANNEL_ID);
	zassert_equal(0, err, "%s:Counter disabling alarm failed", dev->name);
}

/** @brief Validate compare alarm timing conversion from ticks to seconds. */
ZTEST(test_LPTimer, test_Compare_alarm_instance)
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
	int ret, err;
	uint32_t now_ticks = 0;

	LOG_INF("Counter to test initial value of counter\n");
	zassert_true(device_is_ready(dev), "device not ready.\n");

	ret = counter_get_value(dev, &now_ticks);
	if (ret < 0) {
		printk("error while reading value");
	}
	printk("Initial value of the counter before Starting the counter is -> %d.\n",
	      now_ticks);
	err = counter_start(dev);
	zassert_equal(0, err, "%s:Counter failed to start", dev->name);

	ret = counter_get_value(dev, &now_ticks);
	if (ret < 0) {
		printk("error while reading value");
		ztest_test_fail();
	}
	printk("Initial counter value is %d", now_ticks);
	/* zassert_equal(now_ticks, 0, "Initial counter value is not 0 %d.",now_ticks); */
	/*  Stop the counter */
	err = counter_stop(dev);
	zassert_equal(err, 0, "Failed to stop counter (err %d)\n", err);
}

/** @brief Validate counter value can be read before/after start. */
ZTEST(test_LPTimer, test_counter_value)
{
	test_all_instances(test_counter_value_instance,
			reliable_cancel_capable);
}

/*
 * Test that the counter increments by one with each tick
 */
static void test_counter_increment_instance(const struct device *counter_dev)
{
	int ret_count1, ret_count2, ret;
	uint32_t now_ticks = 0, prev_ticks;

	LOG_INF("Counter initial value\n");

	zassert_true(device_is_ready(counter_dev), "device not ready.\n");

	ret = counter_start(counter_dev);
	zassert_equal(0, ret, "%s:Counter failed to start", counter_dev->name);

	ret_count1 = counter_get_value(counter_dev, &prev_ticks);
	zassert_true(ret_count1 == 0, "%s:Counter read failed (err:%d)", counter_dev->name,
		     ret_count1);
	LOG_INF("counter value is %d\n", prev_ticks);
	k_busy_wait(500);

	ret_count2 = counter_get_value(counter_dev, &now_ticks);
	zassert_true(ret_count2 == 0, "%s:Counter read failed (err:%d)", counter_dev->name,
		     ret_count2);
	LOG_INF("counter value is %d\n", now_ticks);
	zassert_false(prev_ticks == now_ticks, "%s:Counter incrementing failed", counter_dev->name);
}

/** @brief Validate counter value increments over time after start. */
ZTEST(test_LPTimer, test_counter_increment)
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

	LOG_INF("Counter alarm check for NULL Callback\nSo Alarm should not be set\n");

	zassert_true(device_is_ready(counter_dev), "device not ready.\n");

	err = counter_cancel_channel_alarm(counter_dev, ALARM_CHANNEL_ID);
	err = counter_start(counter_dev);
	zassert_equal(0, err, "%s:Counter failed to start", counter_dev->name);

	alarm_cfg.flags = 0;
	alarm_cfg.ticks = counter_us_to_ticks(counter_dev, DELAY);
	alarm_cfg.callback = NULL;
	alarm_cfg.user_data = &alarm_cfg;

	err = counter_set_channel_alarm(counter_dev, ALARM_CHANNEL_ID,
					&alarm_cfg);
	ret = check_set_channel_alarm_return_status(err);
	zassert_not_equal(0, ret, "%s:Counter setting alarm failed", counter_dev->name);

	if (ret >= 0) {
		LOG_INF("Set alarm in %u sec (%u ticks)\n",
		       (uint32_t)(counter_ticks_to_us(counter_dev,
						   alarm_cfg.ticks) / USEC_PER_SEC),
		       alarm_cfg.ticks);
		err = counter_cancel_channel_alarm(counter_dev, ALARM_CHANNEL_ID);
		ret = check_set_channel_alarm_return_status(err);
		zassert_equal(0, ret, "%s:Counter disabling alarm failed", counter_dev->name);
	}
}

/** @brief Validate API behavior when alarm callback is NULL. */
ZTEST(test_LPTimer, test_error_check)
{
	test_all_instances(test_error_check_instance,
			reliable_cancel_capable);
}

static void test_cancel_alarm_instance(const struct device *dev)
{
	int err, ret;

	zassert_true(device_is_ready(dev), "device not ready.\n");
	err = counter_cancel_channel_alarm(dev, ALARM_CHANNEL_ID);

	LOG_INF("testcase to cancel alarm\n");
	err = counter_start(dev);
	zassert_equal(0, err, "%s:Counter failed to start", dev->name);

	alarm_cfg.flags = 0;
	alarm_cfg.ticks = counter_us_to_ticks(dev, DELAY);
	alarm_cfg.callback = test_counter_interrupt_fn;
	alarm_cfg.user_data = &alarm_cfg;

	err = counter_set_channel_alarm(dev, ALARM_CHANNEL_ID,
					&alarm_cfg);
	LOG_INF("Set alarm in %u sec (%u ticks)\n",
	       (uint32_t)(counter_ticks_to_us(dev,
					   alarm_cfg.ticks) / USEC_PER_SEC),
	       alarm_cfg.ticks);

	ret = check_set_channel_alarm_return_status(err);
	zassert_equal(0, ret, "%s:Counter setting alarm failed", dev->name);

	err = counter_cancel_channel_alarm(dev, ALARM_CHANNEL_ID);
	ret = check_cancel_channel_alarm_return_status(err);
	zassert_equal(0, ret, "%s:Counter disabling alarm failed", dev->name);

	k_sleep(K_MSEC(3000));
}

/** @brief Validate cancelling an active alarm succeeds. */
ZTEST(test_LPTimer, test_cancel_alarm)
{
	test_all_instances(test_cancel_alarm_instance,
			reliable_cancel_capable);
}

static void callback_lptimer_External(const struct device *counter_dev,
				      uint8_t chan_id, uint32_t ticks,
				      void *user_data)
{
	uint32_t now_ticks;
	uint64_t now_usec;
	uint64_t now_sec;
	int err;

	err = counter_get_value(counter_dev, &now_ticks);
	if (err) {
		LOG_INF("Failed to read counter value (err %d)", err);
		return;
	}
	now_usec = counter_ticks_to_us(counter_dev, now_ticks);
	now_sec = now_usec / USEC_PER_SEC;
	LOG_INF("!!! Alarm triggered !!!\n");
	LOG_INF("Now:%llu sec (ticks=%u)\n", (unsigned long long)now_sec, now_ticks);

	lp_cnt++;
	LOG_INF("lpcnt value is %d\n", lp_cnt);
}

/*
 *	Configuring the lptimer Timer0 for 500 micro second
 *
 *	Selected clock frequency (F)= 32Khz
 *
 *	USEC_PER_SEC=1000U*1000U
 *
 *	ticks = (us * z_impl_counter_get_frequency(dev)) / USEC_PER_SEC;
 *	(500 * 32.768)
 *	16384/1000*1000
 *	ticks = (500 * 32.768) / 1000*1000;
 *	ticks = 16
 *
 *	counter_us_to_ticks -> function to convert microsecs to ticks
 *
 *	Here Lptimer pin is already configured as input pin
 *	soc/arm/alif_ensemble/e7/soc_e7_dk_rtss_he.c and  soc_e7_dk_rtss_hp.c
 *
 *	Assuming for LPTIMER0
 *	Short p15_0 with p1_0 and check if interrupt triggered on edge rising
 */

static void test_External_Clock_Source_instance(void)
{
	int in_ret, ret;
	int err, ret_res;
	int i;
	uint32_t Exter_count;
	uint64_t now_usec;
	uint64_t now_sec;

	const struct device *dev;
	#ifdef CONFIG_COUNTER_SNPS_DW
	    #if DT_NODE_HAS_STATUS(TIMER1, okay)
		dev = DEVICE_DT_GET(TIMER1);
	    #elif DT_NODE_HAS_STATUS(TIMER2, okay)
		dev = DEVICE_DT_GET(TIMER2);
	    #elif DT_NODE_HAS_STATUS(TIMER3, okay)
		dev = DEVICE_DT_GET(TIMER3);
	    #else
		dev = DEVICE_DT_GET(TIMER0);
	    #endif
	#endif
	static const struct gpio_dt_spec gpio_port = GPIO_DT_SPEC_GET(GPIO_NODE, gpios);

	zassert_true(device_is_ready(dev), "device not ready.%s\n", dev->name);
	zassert_true(gpio_is_ready_dt(&gpio_port), "GPIO dev is not ready");

	LOG_INF("testcase to test Alarm with External Clock Source\n");
	LOG_INF("Running test on gpio port and lptimer pin "
		"and taking external input clock source\n");

	/* GPIO pin configuration used for lptimer channel 0 */
	in_ret = gpio_pin_configure_dt(&gpio_port, GPIO_OUTPUT_ACTIVE);
	int ret_PCI = check_pin_conf(in_ret);

	zassert_equal(ret_PCI, 0, "Failed to configure the input pin");

	ret = gpio_pin_set_dt(&gpio_port, GPIO_OUTPUT_INIT_LOW);
	ret_res = check_pin_set_conf(ret);
	zassert_equal(ret_res, 0, "Failed to set pin configuration\n");

	cb_count = 0;

	alarm_cfg.flags = 0;
	alarm_cfg.ticks = counter_us_to_ticks(dev, DELAY_EXTERNAL);
	alarm_cfg.callback = callback_lptimer_External;
	alarm_cfg.user_data = NULL;

	err = counter_start(dev);
	zassert_equal(0, err, "%s:Counter failed to start", dev->name);
	err = counter_get_value(dev, &Exter_count);
	if (err) {
		LOG_INF("Failed to read counter value (err %d)", err);
		return;
	}
	now_usec = counter_ticks_to_us(dev, Exter_count);
	now_sec = now_usec / USEC_PER_SEC;
	LOG_INF("Counter value:ticks=%u, seconds=%llu\n",
		Exter_count, (unsigned long long)now_sec);
	err = counter_set_channel_alarm(dev, ALARM_CHANNEL_ID,
					&alarm_cfg);
	ret = check_set_channel_alarm_return_status(err);
	zassert_equal(0, ret, "%s:Counter setting alarm failed", dev->name);

	LOG_INF("Set alarm in %u sec (%u ticks)\n",
	       (uint32_t)(counter_ticks_to_us(dev,
					   alarm_cfg.ticks) / USEC_PER_SEC),
	       alarm_cfg.ticks);

	for (i = 0; i < 10; i++) {
		int ret1 = gpio_pin_toggle_dt(&gpio_port);
		int ret_pin_tog = check_pin_toggle(ret1);

		zassert_equal(ret_pin_tog, 0, "Failed to toggle LED pin");
		k_sleep(K_MSEC(100));
	}
	LOG_INF("i value is %d\n", i);
	err = counter_get_value(dev, &Exter_count);
	if (err) {
		LOG_INF("Failed to read counter value (err %d)", err);
		return;
	}
	now_usec = counter_ticks_to_us(dev, Exter_count);
	now_sec = now_usec / USEC_PER_SEC;
	LOG_INF("Counter value:ticks=%u, seconds=%llu\n",
		Exter_count, (unsigned long long)now_sec);
	err = counter_cancel_channel_alarm(dev, ALARM_CHANNEL_ID);
	ret = check_cancel_channel_alarm_return_status(err);
	zassert_equal(0, ret, "%s:Counter disabling alarm failed", dev->name);
}

/** @brief Validate alarm trigger path with external clock source. */
#if (CONFIG_LPTIMER0_EXT_CLK_FREQ | \
	CONFIG_LPTIMER1_EXT_CLK_FREQ | \
	CONFIG_LPTIMER2_EXT_CLK_FREQ | \
	CONFIG_LPTIMER3_EXT_CLK_FREQ)
ZTEST(lptimer_external_clock_source, test_External_Clock_Source)
{
	test_External_Clock_Source_instance();
}
#endif

/** @brief Validate LPTIMER operation with 128kHz clock source. */
#if CONFIG_LP_128K
ZTEST(lptimer_128k_clock_source, test_lptimer_128k_clock_source)
{
	/* Test basic counter functionality with 128kHz clock */
	test_all_instances(test_counter_value_instance, reliable_cancel_capable);
	test_all_instances(test_counter_increment_instance, reliable_cancel_capable);
	test_all_instances(test_alarm_32MhzClockInput_instance, reliable_cancel_capable);
}
#endif

static volatile uint32_t toggle_alarm_cnt;

static void toggle_alarm_handler(const struct device *counter_dev,
				 uint8_t chan_id, uint32_t ticks,
				 void *user_data)
{
	toggle_alarm_cnt++;
	LOG_INF("Output toggle alarm fired (count=%u, ticks=%u)\n",
		toggle_alarm_cnt, ticks);
}

static void test_lptimer_output_toggle_instance(const struct device *dev)
{
	int err, ret;
	uint32_t initial_ticks, later_ticks;

	LOG_INF("Testing LPTIMER output toggle mode on %s\n", dev->name);

	zassert_true(device_is_ready(dev), "device not ready.\n");

	err = counter_cancel_channel_alarm(dev, ALARM_CHANNEL_ID);

	toggle_alarm_cnt = 0;

	/* Start the counter in output toggle mode (configured via overlay) */
	err = counter_start(dev);
	zassert_equal(0, err, "%s:Counter failed to start", dev->name);

	/* Verify counter is running by reading value twice */
	err = counter_get_value(dev, &initial_ticks);
	zassert_equal(0, err, "%s:Counter read failed (err:%d)", dev->name, err);

	k_busy_wait(1000);

	err = counter_get_value(dev, &later_ticks);
	zassert_equal(0, err, "%s:Counter read failed (err:%d)", dev->name, err);

	zassert_true(initial_ticks != later_ticks,
		     "%s:Counter not progressing in output toggle mode", dev->name);

	/* Set an alarm to verify interrupt works in toggle mode */
	alarm_cfg.flags = 0;
	alarm_cfg.ticks = counter_us_to_ticks(dev, DELAY);
	alarm_cfg.callback = toggle_alarm_handler;
	alarm_cfg.user_data = &alarm_cfg;

	err = counter_set_channel_alarm(dev, ALARM_CHANNEL_ID, &alarm_cfg);
	ret = check_set_channel_alarm_return_status(err);
	zassert_equal(0, ret, "%s:Counter setting alarm failed", dev->name);

	LOG_INF("Set toggle alarm in %u sec (%u ticks)\n",
		(uint32_t)(counter_ticks_to_us(dev,
					       alarm_cfg.ticks) / USEC_PER_SEC),
		alarm_cfg.ticks);

	/* Wait for alarm to fire */
	k_sleep(K_MSEC(5000));

	zassert_true(toggle_alarm_cnt > 0,
		     "%s:Alarm callback not triggered in output toggle mode",
		     dev->name);

	LOG_INF("Output toggle alarm triggered %u times\n", toggle_alarm_cnt);

	err = counter_cancel_channel_alarm(dev, ALARM_CHANNEL_ID);
	ret = check_cancel_channel_alarm_return_status(err);
	zassert_equal(0, ret, "%s:Counter disabling alarm failed", dev->name);

	err = counter_stop(dev);
	zassert_equal(0, err, "%s:Counter failed to stop", dev->name);
}

/** @brief Validate alarm and counter operation in output toggle mode. */
#if (CONFIG_LPTIMER0_OUTPUT_TOGGLE | \
	CONFIG_LPTIMER1_OUTPUT_TOGGLE | \
	CONFIG_LPTIMER2_OUTPUT_TOGGLE | \
	CONFIG_LPTIMER3_OUTPUT_TOGGLE)
ZTEST(lptimer_Output_toggle, test_lptimer_toggle_Clock_Source)
{
	test_all_instances(test_lptimer_output_toggle_instance,
			reliable_cancel_capable);
}
#endif

static void Test_alarm_stress(const struct device *dev)
{
	int err, ret;

	err = counter_stop(dev);
	zassert_equal(err, 0, "Failed to stop counter (err %d)\n", err);
	for (int i = 0; i < STRESS_TEST_COUNT; i++) {
		counter_start(dev);
		alarm_cfg.flags = 0;
		alarm_cfg.ticks = counter_us_to_ticks(dev, DELAY);
		alarm_cfg.callback = test_counter_interrupt_fn;
		alarm_cfg.user_data = &alarm_cfg;

		err = counter_set_channel_alarm(dev, ALARM_CHANNEL_ID,
						&alarm_cfg);

		ret = check_set_channel_alarm_return_status(err);
		zassert_equal(0, ret, "%s:Counter setting alarm failed", dev->name);

		printk("Set alarm in %u sec (%u ticks)\n",
		       (uint32_t)(counter_ticks_to_us(dev,
						   alarm_cfg.ticks) / USEC_PER_SEC),
		       alarm_cfg.ticks);

		k_sleep(K_MSEC(20000));

		err = counter_cancel_channel_alarm(dev, ALARM_CHANNEL_ID);
		ret = check_cancel_channel_alarm_return_status(err);
		zassert_equal(0, ret, "%s:Counter disabling alarm failed", dev->name);
		printk("Alarm was set for %d\n", i);
		counter_stop(dev);
	}
}

/** @brief Stress test repeated alarm set/cancel cycles. */
ZTEST(Stress_test, alarm_stress_test)
{
	test_all_instances(Test_alarm_stress,
			reliable_cancel_capable);
}

/*
 * Feature 7:Cascaded 64-bit timer test
 *
 * Each odd-numbered LPTIMER can be concatenated with the previous
 * even-numbered LPTIMER to form a 64-bit timer. When cascade mode is
 * enabled (CONFIG_LPTIMERx_CLOCK_SOURCE_CASCADE), the counter should
 * provide a 64-bit value via counter_get_value_64().
 *
 * This test validates:
 *   - counter_get_value_64() returns successfully
 *   - The 64-bit counter value progresses over time
 *   - The 64-bit value is consistent (upper 32 bits are stable or
 *     incrementing when lower 32 bits wrap)
 */
#if (defined(CONFIG_LPTIMER0_CLOCK_SOURCE_CASCADE) || \
	defined(CONFIG_LPTIMER1_CLOCK_SOURCE_CASCADE) || \
	defined(CONFIG_LPTIMER3_CLOCK_SOURCE_CASCADE))
static void test_cascade_64bit_counter_instance(const struct device *dev)
{
	int err;
	uint64_t val1 = 0, val2 = 0;

	LOG_INF("Testing 64-bit cascaded counter on %s\n", dev->name);

	zassert_true(device_is_ready(dev), "device not ready.\n");

	err = counter_start(dev);
	zassert_equal(0, err, "%s:Counter failed to start", dev->name);

	/* First 64-bit read */
	err = counter_get_value_64(dev, &val1);
	if (err == -ENOTSUP) {
		LOG_INF("%s:counter_get_value_64 not supported, skipping\n",
			dev->name);
		counter_stop(dev);
		ztest_test_skip();
		return;
	}
	zassert_equal(0, err,
		      "%s:counter_get_value_64 failed (err:%d)",
		      dev->name, err);

	LOG_INF("First 64-bit value:0x%08x%08x\n",
		(uint32_t)(val1 >> 32), (uint32_t)(val1 & 0xFFFFFFFF));

	/* Wait to allow counter to progress */
	k_busy_wait(5000);

	/* Second 64-bit read */
	err = counter_get_value_64(dev, &val2);
	zassert_equal(0, err,
		      "%s:counter_get_value_64 failed (err:%d)",
		      dev->name, err);

	LOG_INF("Second 64-bit value:0x%08x%08x\n",
		(uint32_t)(val2 >> 32), (uint32_t)(val2 & 0xFFFFFFFF));

	/* For a down counter, val2 < val1; for up counter, val2 > val1 */
	zassert_true(val1 != val2,
		     "%s:64-bit cascaded counter did not progress "
		     "(val1=0x%llx, val2=0x%llx)",
		     dev->name, val1, val2);

	LOG_INF("64-bit cascaded counter verified successfully\n");

	err = counter_stop(dev);
	zassert_equal(0, err, "%s:Counter failed to stop", dev->name);
}

/** @brief Validate 64-bit counter read in cascaded (concatenated) mode. */
ZTEST(lptimer_cascade, test_cascade_64bit_counter)
{
	test_all_instances(test_cascade_64bit_counter_instance,
			   reliable_cancel_capable);
}

static void test_cascade_64bit_alarm_instance(const struct device *dev)
{
	int err, ret;
	uint64_t val_before = 0, val_after = 0;

	LOG_INF("Testing 64-bit cascaded alarm on %s\n", dev->name);

	zassert_true(device_is_ready(dev), "device not ready.\n");

	err = counter_cancel_channel_alarm(dev, ALARM_CHANNEL_ID);

	k_sem_reset(&alarm_cnt_sem);
	alarm_cnt = 0;

	err = counter_start(dev);
	zassert_equal(0, err, "%s:Counter failed to start", dev->name);

	/* Read 64-bit value before alarm */
	err = counter_get_value_64(dev, &val_before);
	if (err == -ENOTSUP) {
		counter_stop(dev);
		ztest_test_skip();
		return;
	}
	zassert_equal(0, err, "%s:64-bit read failed (err:%d)",
		      dev->name, err);

	/* Set alarm */
	alarm_cfg.flags = 0;
	alarm_cfg.ticks = counter_us_to_ticks(dev, DELAY);
	alarm_cfg.callback = alarm_handler;
	alarm_cfg.user_data = NULL;

	err = counter_set_channel_alarm(dev, ALARM_CHANNEL_ID, &alarm_cfg);
	ret = check_set_channel_alarm_return_status(err);
	zassert_equal(0, ret, "%s:Counter setting alarm failed", dev->name);

	/* Wait for alarm */
	k_sleep(K_MSEC(5000));

	uint32_t cnt = IS_ENABLED(CONFIG_ZERO_LATENCY_IRQS) ?
		alarm_cnt : k_sem_count_get(&alarm_cnt_sem);
	zassert_true(cnt > 0,
		     "%s:Alarm did not fire in cascade mode", dev->name);

	/* Read 64-bit value after alarm */
	err = counter_get_value_64(dev, &val_after);
	zassert_equal(0, err, "%s:64-bit read failed (err:%d)",
		      dev->name, err);

	LOG_INF("Before:0x%08x%08x, After:0x%08x%08x\n",
		(uint32_t)(val_before >> 32), (uint32_t)(val_before & 0xFFFFFFFF),
		(uint32_t)(val_after >> 32), (uint32_t)(val_after & 0xFFFFFFFF));

	zassert_true(val_before != val_after,
		     "%s:64-bit counter unchanged across alarm",
		     dev->name);

	err = counter_cancel_channel_alarm(dev, ALARM_CHANNEL_ID);
	ret = check_cancel_channel_alarm_return_status(err);
	zassert_equal(0, ret, "%s:Counter disabling alarm failed", dev->name);

	err = counter_stop(dev);
	zassert_equal(0, err, "%s:Counter failed to stop", dev->name);
}

/** @brief Validate alarm fires correctly when timer is in 64-bit cascade mode. */
ZTEST(lptimer_cascade, test_cascade_64bit_alarm)
{
	test_all_instances(test_cascade_64bit_alarm_instance,
			   reliable_cancel_capable);
}
#endif /* CASCADE CONFIGS */

/*
 * Feature 8:Wake-up from low-power mode test
 *
 * The LPTIMER interrupt can be used as a wake-up source from STANDBY
 * and STOP low-power modes. This test validates:
 *   - Set an LPTIMER alarm
 *   - Force system into a low-power state (STANDBY)
 *   - Verify the LPTIMER interrupt wakes the system
 *   - Verify the alarm callback fires after wake-up
 */
#ifdef CONFIG_PM
static volatile uint32_t wakeup_alarm_cnt;

static void wakeup_alarm_handler(const struct device *counter_dev,
				 uint8_t chan_id, uint32_t ticks,
				 void *user_data)
{
	wakeup_alarm_cnt++;
	LOG_INF("Wake-up alarm fired (count=%u, ticks=%u)\n",
		wakeup_alarm_cnt, ticks);
}

static void test_lptimer_wakeup_instance(const struct device *dev)
{
	int err, ret;
	struct pm_state_info pm_info;

	LOG_INF("Testing LPTIMER wake-up from low-power on %s\n", dev->name);

	zassert_true(device_is_ready(dev), "device not ready.\n");

	err = counter_cancel_channel_alarm(dev, ALARM_CHANNEL_ID);

	wakeup_alarm_cnt = 0;

	err = counter_start(dev);
	zassert_equal(0, err, "%s:Counter failed to start", dev->name);

	/* Set alarm for 2 seconds */
	alarm_cfg.flags = 0;
	alarm_cfg.ticks = counter_us_to_ticks(dev, DELAY);
	alarm_cfg.callback = wakeup_alarm_handler;
	alarm_cfg.user_data = &alarm_cfg;

	err = counter_set_channel_alarm(dev, ALARM_CHANNEL_ID, &alarm_cfg);
	ret = check_set_channel_alarm_return_status(err);
	zassert_equal(0, ret, "%s:Counter setting alarm failed", dev->name);

	LOG_INF("Set wake-up alarm in %u sec (%u ticks)\n",
		(uint32_t)(counter_ticks_to_us(dev,
					       alarm_cfg.ticks) / USEC_PER_SEC),
		alarm_cfg.ticks);

	/*
	 * Force system into STANDBY low-power state.
	 * The LPTIMER interrupt should wake the system.
	 */
	pm_info.state = PM_STATE_STANDBY;
	pm_info.substate_id = 0;
	pm_info.min_residency_us = 0;
	pm_info.exit_latency_us = 0;

	LOG_INF("Forcing system into STANDBY mode...\n");

	bool forced = pm_state_force(0U, &pm_info);

	if (!forced) {
		LOG_INF("pm_state_force(STANDBY) not supported, "
			"trying SUSPEND_TO_IDLE\n");
		pm_info.state = PM_STATE_SUSPEND_TO_IDLE;
		forced = pm_state_force(0U, &pm_info);
		if (!forced) {
			LOG_INF("Low-power state not available, skipping\n");
			counter_cancel_channel_alarm(dev, ALARM_CHANNEL_ID);
			counter_stop(dev);
			ztest_test_skip();
			return;
		}
	}

	/*
	 * Idle the thread to allow PM subsystem to enter low-power.
	 * The LPTIMER alarm interrupt should wake the system.
	 */
	k_sleep(K_MSEC(5000));

	LOG_INF("System woke up, checking alarm callback\n");

	zassert_true(wakeup_alarm_cnt > 0,
		     "%s:LPTIMER alarm did not wake system from low-power",
		     dev->name);

	LOG_INF("Wake-up alarm triggered %u times\n", wakeup_alarm_cnt);

	err = counter_cancel_channel_alarm(dev, ALARM_CHANNEL_ID);
	ret = check_cancel_channel_alarm_return_status(err);
	zassert_equal(0, ret, "%s:Counter disabling alarm failed", dev->name);

	err = counter_stop(dev);
	zassert_equal(0, err, "%s:Counter failed to stop", dev->name);
}

/** @brief Validate LPTIMER alarm wakes system from STANDBY low-power mode. */
ZTEST(lptimer_wakeup, test_lptimer_wakeup_from_standby)
{
	test_all_instances(test_lptimer_wakeup_instance,
			   reliable_cancel_capable);
}
#endif /* CONFIG_PM */

/* Test suite registration - mutually exclusive based on configuration */
#if (CONFIG_LPTIMER0_EXT_CLK_FREQ | \
	CONFIG_LPTIMER1_EXT_CLK_FREQ | \
	CONFIG_LPTIMER2_EXT_CLK_FREQ | \
	CONFIG_LPTIMER3_EXT_CLK_FREQ)
/* External clock configuration - only run external clock tests */
ZTEST_SUITE(lptimer_external_clock_source, NULL, NULL, NULL, NULL, NULL);
#elif (CONFIG_LPTIMER0_OUTPUT_TOGGLE | \
	CONFIG_LPTIMER1_OUTPUT_TOGGLE | \
	CONFIG_LPTIMER2_OUTPUT_TOGGLE | \
	CONFIG_LPTIMER3_OUTPUT_TOGGLE)
/* Output toggle configuration - only run output toggle tests */
ZTEST_SUITE(lptimer_Output_toggle, NULL, NULL, NULL, NULL, NULL);
#elif CONFIG_LP_128K
/* 128kHz clock configuration - only run 128kHz clock tests */
ZTEST_SUITE(lptimer_128k_clock_source, NULL,
	    counter_setup, NULL, NULL, NULL);
#elif (defined(CONFIG_LPTIMER0_CLOCK_SOURCE_CASCADE) || \
	defined(CONFIG_LPTIMER1_CLOCK_SOURCE_CASCADE) || \
	defined(CONFIG_LPTIMER3_CLOCK_SOURCE_CASCADE))
/* Cascade configuration - run base + cascade tests */
ZTEST_SUITE(counter_basic, NULL, counter_setup, NULL, NULL, NULL);
ZTEST_SUITE(test_LPTimer, NULL, NULL, NULL, NULL, NULL);
ZTEST_SUITE(counter_no_callback, NULL, counter_setup, NULL, NULL, NULL);
ZTEST_SUITE(Stress_test, NULL, NULL, NULL, NULL, NULL);
ZTEST_SUITE(lptimer_cascade, NULL, counter_setup, NULL, NULL, NULL);
#elif defined(CONFIG_PM)
/* Wake-up configuration - run base + wakeup tests */
ZTEST_SUITE(counter_basic, NULL, counter_setup, NULL, NULL, NULL);
ZTEST_SUITE(test_LPTimer, NULL, NULL, NULL, NULL, NULL);
ZTEST_SUITE(counter_no_callback, NULL, counter_setup, NULL, NULL, NULL);
ZTEST_SUITE(Stress_test, NULL, NULL, NULL, NULL, NULL);
ZTEST_SUITE(lptimer_wakeup, NULL, counter_setup, NULL, NULL, NULL);
#else
/* Default configuration - run base test suites */
ZTEST_SUITE(counter_basic, NULL, counter_setup, NULL, NULL, NULL);
ZTEST_SUITE(test_LPTimer, NULL, NULL, NULL, NULL, NULL);
ZTEST_SUITE(counter_no_callback, NULL, counter_setup, NULL, NULL, NULL);
ZTEST_SUITE(Stress_test, NULL, NULL, NULL, NULL, NULL);
#endif

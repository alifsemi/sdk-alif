/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#include <errno.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/pm/pm.h>
#include <zephyr/pm/policy.h>
#include <zephyr/drivers/counter.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(pm_i2c_dw, LOG_LEVEL_INF);

/* Set to 1 to dump full NVIC ISPR state on wakeup */
#define APP_PM_WAKEUP_DEBUG 0

#if DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(rtc0), snps_dw_apb_rtc, okay)
	#define WAKEUP_SOURCE DT_NODELABEL(rtc0)
#elif DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(timer0), snps_dw_timers, okay)
	#define WAKEUP_SOURCE DT_NODELABEL(timer0)
#else
#error "Wakeup Device not enabled in the dts"
#endif

#if !DT_NODE_EXISTS(DT_ALIAS(master_i2c)) || !DT_NODE_EXISTS(DT_ALIAS(slave_i2c))
#error "aliases master-i2c (I2C0) and slave-i2c (I2C1) must be set in the overlay"
#endif

#define I2C_MASTER DT_ALIAS(master_i2c)
#define I2C_SLAVE  DT_ALIAS(slave_i2c)

#define SLV_I2C_ADDR 0x50
#define I2C_LB_LEN   4

/*
 * Sleep duration constants for each PM state.
 *
 * Upper bound: at 400 MHz, ticks-to-cycles calculation in the PM driver
 * overflows uint32 max for values > 10.7s (2^32 / 400). Deep-sleep durations
 * (S2RAM, SOFT_OFF) and overlay min-residency-us values must stay below this
 * ceiling. RUNTIME_IDLE may exceed it because deeper states are locked out
 * during that sleep (see app_pm_lock_sleep_states()).
 */
#define RUNTIME_IDLE_SLEEP_USEC  (18 * 1000 * 1000)
#define SUSPEND_IDLE_SLEEP_USEC  (10 * 1000)
#define S2RAM_STANDBY_SLEEP_USEC (6 * 1000 * 1000)
#define S2RAM_STOP_SLEEP_USEC    (9 * 1000 * 1000)
#define SOFT_OFF_SLEEP_USEC      (10 * 1000 * 1000)

#define MRAM_BASE_ADDRESS 0x80000000
#define IS_BOOTING_FROM_MRAM() (SCB->VTOR >= MRAM_BASE_ADDRESS)

#if DT_NODE_EXISTS(DT_NODELABEL(sram0)) && DT_HAS_CHOSEN(zephyr_sram)
#define IS_SRAM0_CONFIGURED_AS_RAM() \
	DT_SAME_NODE(DT_CHOSEN(zephyr_sram), DT_NODELABEL(sram0))
#else
#define IS_SRAM0_CONFIGURED_AS_RAM() 0
#endif

#define S2RAM_SUPPORTED \
	(IS_SRAM0_CONFIGURED_AS_RAM() || \
	 (IS_ENABLED(CONFIG_RTSS_HE) && !IS_BOOTING_FROM_MRAM()))

#define SOFT_OFF_SUPPORTED (!S2RAM_SUPPORTED)

BUILD_ASSERT(S2RAM_STOP_SLEEP_USEC > S2RAM_STANDBY_SLEEP_USEC,
	"STOP sleep duration must be greater than STANDBY sleep duration");
BUILD_ASSERT(SOFT_OFF_SLEEP_USEC > S2RAM_STOP_SLEEP_USEC,
	"SOFT_OFF sleep duration must be greater than STOP sleep duration");

/**
 * Helper function to lock/unlock deeper power states.
 * @param lock true  → lock both deep states (S2RAM and SOFT_OFF)
 *             false → unlock both deep states
 */
static void app_pm_lock_deeper_states(bool lock)
{
	if (lock) {
		pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_RAM, PM_ALL_SUBSTATES);
		pm_policy_state_lock_get(PM_STATE_SOFT_OFF,       PM_ALL_SUBSTATES);
	} else {
		pm_policy_state_lock_put(PM_STATE_SOFT_OFF,       PM_ALL_SUBSTATES);
		pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_RAM, PM_ALL_SUBSTATES);
	}
}

/*
 * Lock every PM sleep state deeper than RUNTIME_IDLE. Used around I2C
 * loopback and around any sleep the app must keep in RUNTIME_IDLE.
 */
static void app_pm_lock_sleep_states(bool lock)
{
	if (lock) {
		pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
		pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_RAM,  PM_ALL_SUBSTATES);
		pm_policy_state_lock_get(PM_STATE_SOFT_OFF,        PM_ALL_SUBSTATES);
	} else {
		pm_policy_state_lock_put(PM_STATE_SOFT_OFF,        PM_ALL_SUBSTATES);
		pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_RAM,  PM_ALL_SUBSTATES);
		pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
	}
}

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

static int app_enter_deep_sleep(uint32_t sleep_usec)
{
#if defined(CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER)
	k_sleep(K_USEC(sleep_usec));
#else
	const struct device *const wakeup_dev = DEVICE_DT_GET(WAKEUP_SOURCE);
	struct counter_alarm_cfg alarm_cfg = {0};
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

static uint8_t master_tx[I2C_LB_LEN] = { 0xA5, 0xA6, 0xA7, 0xA8 };
static uint8_t master_rx[I2C_LB_LEN];
static uint8_t slave_rx[I2C_LB_LEN];
static uint8_t slave_tx[I2C_LB_LEN];
static uint32_t slave_rx_len;
static struct i2c_target_config slave_cfg;

static int i2c_target_write_requested_cb(struct i2c_target_config *config)
{
	ARG_UNUSED(config);
	return 0;
}

static int i2c_target_read_requested_cb(struct i2c_target_config *config, uint8_t *val)
{
	ARG_UNUSED(config);
	*val = slave_tx[0];
	return 0;
}

static int i2c_target_write_received_cb(struct i2c_target_config *config, uint8_t val)
{
	ARG_UNUSED(config);
#ifndef CONFIG_I2C_TARGET_BUFFER_MODE
	if (slv_rx_idx < I2C_XFER_LEN) {
		slv_rx_buf[slv_rx_idx++] = val;
	}
#endif
	//LOG_INF("Received a byte in slave : 0x%x", val);
	return 0;
}

static int i2c_target_read_processed_cb(struct i2c_target_config *config, uint8_t *val)
{
	ARG_UNUSED(config);
	*val = slave_tx[0];
	return 0;
}

#ifdef CONFIG_I2C_TARGET_BUFFER_MODE
static int i2c_target_buf_read_requested_cb(struct i2c_target_config *config,
					    uint8_t **val, uint32_t *len)
{
	ARG_UNUSED(config);

	*val = slave_tx;
	*len = I2C_LB_LEN;
	LOG_INF("Read requested from Master, send 0x%x 0x%x 0x%x 0x%x",
		slave_tx[0], slave_tx[1], slave_tx[2], slave_tx[3]);
	return 0;
}

static void i2c_target_buf_write_received_cb(struct i2c_target_config *config,
					     uint8_t *data_buf, uint32_t len)
{
	uint32_t n = MIN(len, I2C_LB_LEN);

	ARG_UNUSED(config);
	memcpy(slave_rx, data_buf, n);
	slave_rx_len = n;
	LOG_INF("Received %u bytes from master: 0x%x 0x%x 0x%x 0x%x",
		n,
		n > 0 ? slave_rx[0] : 0,
		n > 1 ? slave_rx[1] : 0,
		n > 2 ? slave_rx[2] : 0,
		n > 3 ? slave_rx[3] : 0);
}
#endif

static const struct i2c_target_callbacks slave_cbs = {
	.write_requested = i2c_target_write_requested_cb,
	.read_requested = i2c_target_read_requested_cb,
	.write_received = i2c_target_write_received_cb,
	.read_processed = i2c_target_read_processed_cb,
#ifdef CONFIG_I2C_TARGET_BUFFER_MODE
	.buf_write_received = i2c_target_buf_write_received_cb,
	.buf_read_requested = i2c_target_buf_read_requested_cb,
#endif
};

static int register_slave_i2c(void)
{
	const struct device *const slave_dev = DEVICE_DT_GET(I2C_SLAVE);
	int ret;

	if (!device_is_ready(slave_dev)) {
		LOG_ERR("I2C slave device is not ready");
		return -ENODEV;
	}

	slave_cfg.flags = 0;
	slave_cfg.address = SLV_I2C_ADDR;
	slave_cfg.callbacks = &slave_cbs;

	ret = i2c_target_register(slave_dev, &slave_cfg);
	if (ret) {
		LOG_ERR("I2C slave register failed: %d", ret);
	}
	return ret;
}

/*
 * One write + one read on I2C0 (master) / I2C1 (slave). Slave echoes the
 * written payload so both directions are checked after each PM wake.
 */
static int i2c_loopback_run(const char *phase_label)
{
	const struct device *const master_dev = DEVICE_DT_GET(I2C_MASTER);
	struct i2c_msg msgs[2];
	int ret;

	app_pm_lock_sleep_states(true);
	ARG_UNUSED(phase_label);

	if (!device_is_ready(master_dev)) {
		LOG_ERR("I2C master device is not ready");
		app_pm_lock_sleep_states(false);
		return -ENODEV;
	}

	memcpy(slave_tx, master_tx, I2C_LB_LEN);
	memset(master_rx, 0, sizeof(master_rx));
	memset(slave_rx, 0, sizeof(slave_rx));
	slave_rx_len = 0;

	msgs[0].buf = master_tx;
	msgs[0].len = I2C_LB_LEN;
	msgs[0].flags = I2C_MSG_WRITE | I2C_MSG_STOP;

	msgs[1].buf = master_rx;
	msgs[1].len = I2C_LB_LEN;
	msgs[1].flags = I2C_MSG_READ | I2C_MSG_STOP;

	ret = i2c_transfer(master_dev, &msgs[0], 1, SLV_I2C_ADDR);
	if (ret) {
		LOG_ERR("I2C master write failed: %d", ret);
		app_pm_lock_sleep_states(false);
		return ret;
	}
	LOG_INF("Master wrote:  %x %x %x %x",
		master_tx[0], master_tx[1], master_tx[2], master_tx[3]);

	ret = i2c_transfer(master_dev, &msgs[1], 1, SLV_I2C_ADDR);
	if (ret) {
		LOG_ERR("I2C master read failed: %d", ret);
		app_pm_lock_sleep_states(false);
		return ret;
	}
	LOG_INF("Master read:   %x %x %x %x",
		master_rx[0], master_rx[1], master_rx[2], master_rx[3]);

	if (slave_rx_len != I2C_LB_LEN || memcmp(slave_rx, master_tx, I2C_LB_LEN) != 0) {
		LOG_ERR("I2C master TX & slave RX data mismatch");
		app_pm_lock_sleep_states(false);
		return -EIO;
	}

	if (memcmp(master_rx, slave_tx, I2C_LB_LEN) != 0) {
		LOG_ERR("I2C master RX & slave TX data mismatch");
		app_pm_lock_sleep_states(false);
		return -EIO;
	}

	LOG_INF("SUCCESS: I2C loopback data is matching");
	LOG_INF("Master Transfer Successfully Completed");

	app_pm_lock_sleep_states(false);
	return 0;
}

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

static void pm_notify_entry(enum pm_state state)
{
	const struct pm_state_info *info = pm_state_next_get(0);
	uint8_t substate_id = info ? info->substate_id : 0U;

	LOG_INF("PM enter: %s (substate %u)", PM_STATE_STR(state), substate_id);
}

static void pm_notify_pre_resume(enum pm_state state)
{
	const struct pm_state_info *info = pm_state_next_get(0);
	uint8_t substate_id = info ? info->substate_id : 0U;
	uint32_t active_exc = SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk;

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
			LOG_INF("PM wakeup: NVIC ISPR[%d] = 0x%08x", i, NVIC->ISPR[i]);
		}
	}
#endif
}

static void pm_notify_exit(enum pm_state state)
{
	const struct pm_state_info *info = pm_state_next_get(0);
	uint8_t substate_id = info ? info->substate_id : 0U;

	LOG_INF("PM exit:  %s (substate %u)", PM_STATE_STR(state), substate_id);
}

static struct pm_notifier app_pm_notifier = {
	.state_entry       = pm_notify_entry,
	.pre_device_resume = pm_notify_pre_resume,
	.state_exit        = pm_notify_exit,
};

int main(void)
{
	const struct device *const cons = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
	const struct device *const wakeup_dev = DEVICE_DT_GET(WAKEUP_SOURCE);
	int ret;

	__ASSERT(device_is_ready(cons), "%s: device not ready", cons->name);
	__ASSERT(device_is_ready(wakeup_dev), "%s: device not ready", wakeup_dev->name);

	pm_notifier_register(&app_pm_notifier);

	if (S2RAM_SUPPORTED) {
		LOG_INF("%s (S2RAM): I2C DW PM demo "
			"(RUNTIME_IDLE, SUSPEND_TO_IDLE, S2RAM STANDBY, S2RAM STOP)",
			CONFIG_BOARD);
	} else {
		LOG_INF("%s (SOFT_OFF): I2C DW PM demo "
			"(RUNTIME_IDLE, SUSPEND_TO_IDLE, SOFT_OFF)",
			CONFIG_BOARD);
	}

	ret = counter_start(wakeup_dev);
	__ASSERT(!ret || ret == -EALREADY, "Failed to start counter (err %d)", ret);

	ret = register_slave_i2c();
	__ASSERT(ret == 0, "Failed to register I2C slave (err %d)", ret);

	LOG_INF("POWER STATE SEQUENCE:");
	LOG_INF("  1. PM_STATE_RUNTIME_IDLE");
	LOG_INF("  2. PM_STATE_SUSPEND_TO_IDLE");
	if (S2RAM_SUPPORTED) {
		LOG_INF("  3. PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY)");
		LOG_INF("  4. PM_STATE_SUSPEND_TO_RAM (substate 1: STOP)");
	} else {
		LOG_INF("  3. PM_STATE_SOFT_OFF");
	}

	ret = i2c_loopback_run("before RUNTIME_IDLE");
	__ASSERT(ret == 0, "I2C loopback failed (err %d)", ret);

	LOG_INF("Enter RUNTIME_IDLE sleep for (%d microseconds)", RUNTIME_IDLE_SLEEP_USEC);
	app_pm_lock_sleep_states(true);
	ret = app_enter_normal_sleep(RUNTIME_IDLE_SLEEP_USEC);
	app_pm_lock_sleep_states(false);
	__ASSERT(ret == 0, "Could not enter RUNTIME_IDLE sleep (err %d)", ret);

	LOG_INF("Exited from RUNTIME_IDLE sleep");

#if defined(CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER)
	LOG_INF("Enter PM_STATE_SUSPEND_TO_IDLE for (%d microseconds)",
		SUSPEND_IDLE_SLEEP_USEC);
	app_pm_lock_deeper_states(true);
	k_sleep(K_USEC(SUSPEND_IDLE_SLEEP_USEC));
	app_pm_lock_deeper_states(false);
	LOG_INF("Exited from PM_STATE_SUSPEND_TO_IDLE");
	ret = i2c_loopback_run("after SUSPEND_TO_IDLE");
	__ASSERT(ret == 0, "I2C loopback failed (err %d)", ret);
#else
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
	LOG_INF("PM_STATE_SUSPEND_TO_IDLE (skipped - LPM timer not enabled)");
#endif

	if (S2RAM_SUPPORTED) {
		LOG_INF("Enter PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) for (%d microseconds)",
			S2RAM_STANDBY_SLEEP_USEC);
		ret = app_enter_deep_sleep(S2RAM_STANDBY_SLEEP_USEC);
		__ASSERT(ret == 0, "Could not enter PM_STATE_SUSPEND_TO_RAM (err %d)", ret);

		LOG_INF("=== Resumed from PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) ===");
		ret = i2c_loopback_run("after S2RAM STANDBY");
		__ASSERT(ret == 0, "I2C loopback failed (err %d)", ret);

		LOG_INF("Enter PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) for (%d microseconds)",
			S2RAM_STOP_SLEEP_USEC);
		ret = app_enter_deep_sleep(S2RAM_STOP_SLEEP_USEC);
		__ASSERT(ret == 0, "Could not enter PM_STATE_SUSPEND_TO_RAM (err %d)", ret);

		LOG_INF("=== Resumed from PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) ===");
		ret = i2c_loopback_run("after S2RAM STOP");
		__ASSERT(ret == 0, "I2C loopback failed (err %d)", ret);
	}

	if (SOFT_OFF_SUPPORTED) {
		LOG_INF("Enter PM_STATE_SOFT_OFF for (%d microseconds)", SOFT_OFF_SLEEP_USEC);
		LOG_INF("Note: SOFT_OFF has no retention - system will reset on wakeup");
		ret = app_enter_deep_sleep(SOFT_OFF_SLEEP_USEC);
		__ASSERT(ret == 0, "Could not enter PM_STATE_SOFT_OFF (err %d)", ret);

		LOG_ERR("ERROR: Resumed after PM_STATE_SOFT_OFF - this should not happen!");
		__ASSERT(false, "PM_STATE_SOFT_OFF should have caused a reset");
	}

	LOG_INF("=== I2C DW PM SEQUENCE COMPLETED ===");

	app_pm_lock_deeper_states(true);
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);

	while (true) {
		k_sleep(K_SECONDS(1));
	}

	return 0;
}

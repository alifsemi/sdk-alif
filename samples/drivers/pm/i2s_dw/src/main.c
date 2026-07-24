/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#include <stdio.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/pm/pm.h>
#include <zephyr/pm/policy.h>
#include <zephyr/drivers/counter.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2s.h>
#include <zephyr/sys/util.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(i2s_pm, LOG_LEVEL_INF);

/* Set to 1 to dump full NVIC ISPR state on wakeup */
#define APP_PM_WAKEUP_DEBUG 0

/* ========================================================================
 * I2S Configuration
 * ========================================================================
 */

#if DT_NODE_EXISTS(DT_NODELABEL(i2s_rxtx))
#define I2S_RX_NODE  DT_NODELABEL(i2s_rxtx)
#define I2S_TX_NODE  I2S_RX_NODE
#else
#define I2S_RX_NODE  DT_NODELABEL(i2s_rx)
#define I2S_TX_NODE  DT_NODELABEL(i2s_tx)
#endif

#define SAMPLE_FREQUENCY    44100
#define SAMPLE_BIT_WIDTH    16
#define BYTES_PER_SAMPLE    sizeof(int16_t)
#define NUMBER_OF_CHANNELS  2
/* Such block length provides an echo with the delay of 100 ms. */
#define SAMPLES_PER_BLOCK   ((SAMPLE_FREQUENCY / 10) * NUMBER_OF_CHANNELS)
#define INITIAL_BLOCKS      4
#define TIMEOUT             1000

#define BLOCK_SIZE  (BYTES_PER_SAMPLE * SAMPLES_PER_BLOCK)
#define BLOCK_COUNT (INITIAL_BLOCKS + 6)
K_MEM_SLAB_DEFINE_STATIC(mem_slab, BLOCK_SIZE, BLOCK_COUNT, 4);

/* I2S streaming duration per phase (number of loop iterations) */
#define I2S_STREAM_ITERATIONS 20

static int16_t echo_block[SAMPLES_PER_BLOCK];
static volatile bool echo_enabled = true;

/* ========================================================================
 * Power Management Configuration
 * ========================================================================
 */

#if DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(rtc0), snps_dw_apb_rtc, okay)
	#define WAKEUP_SOURCE DT_NODELABEL(rtc0)
#elif DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(timer0), snps_dw_timers, okay)
	#define WAKEUP_SOURCE DT_NODELABEL(timer0)
#else
#error "Wakeup Device not enabled in the dts"
#endif

/*
 * Sleep duration constants for each PM state.
 *
 * Upper bound: at 400 MHz, ticks to cycles calculation in PM driver
 * overflows uint32 max for values > 10.7s (2^32 / 400). All deep-sleep
 * durations and overlay min-residency-us values must stay below this ceiling.
 */
/* Sleep duration for PM_STATE_RUNTIME_IDLE */
#define RUNTIME_IDLE_SLEEP_USEC (18 * 1000 * 1000)
/* Sleep duration for PM_STATE_SUSPEND_TO_IDLE */
#define SUSPEND_IDLE_SLEEP_USEC (10 * 1000)
/* Sleep duration for PM_STATE_SUSPEND_TO_RAM substate 0 (STANDBY) */
#define S2RAM_STANDBY_SLEEP_USEC (6 * 1000 * 1000)
/* Sleep duration for PM_STATE_SUSPEND_TO_RAM substate 1 (STOP) */
#define S2RAM_STOP_SLEEP_USEC (9 * 1000 * 1000)
/* Sleep duration for PM_STATE_SOFT_OFF */
#define SOFT_OFF_SLEEP_USEC (10 * 1000 * 1000)

/*
 * MRAM base address - used to determine boot location
 * TCM boot: VTOR = 0x0
 * MRAM boot: VTOR >= 0x80000000
 */
#define MRAM_BASE_ADDRESS 0x80000000

#define IS_BOOTING_FROM_MRAM() (SCB->VTOR >= MRAM_BASE_ADDRESS)

/*
 * S2RAM_SUPPORTED — retention-capable sleep is possible when:
 *   - HE core booting from TCM (TCM has hardware retention)
 */
#define S2RAM_SUPPORTED \
		(IS_ENABLED(CONFIG_RTSS_HE) && !IS_BOOTING_FROM_MRAM())

#define SOFT_OFF_SUPPORTED (!S2RAM_SUPPORTED)

/* Validate ordering of deep-sleep durations at compile time */
BUILD_ASSERT(S2RAM_STOP_SLEEP_USEC > S2RAM_STANDBY_SLEEP_USEC,
	"STOP sleep duration must be greater than STANDBY sleep duration");
BUILD_ASSERT(SOFT_OFF_SLEEP_USEC > S2RAM_STOP_SLEEP_USEC,
	"SOFT_OFF sleep duration must be greater than STOP sleep duration");

/* ========================================================================
 * I2S Helper Functions
 * ========================================================================
 */

static void process_block_data(void *mem_block, uint32_t number_of_samples)
{
	static bool clear_echo_block;

	if (echo_enabled) {
		for (int i = 0; i < number_of_samples; ++i) {
			int16_t *sample = &((int16_t *)mem_block)[i];
			*sample += echo_block[i];
			echo_block[i] = (*sample) / 2;
		}

		clear_echo_block = true;
	} else if (clear_echo_block) {
		clear_echo_block = false;
		memset(echo_block, 0, sizeof(echo_block));
	}
}

static bool i2s_configure_streams(const struct device *i2s_dev_rx,
				  const struct device *i2s_dev_tx,
				  const struct i2s_config *config)
{
	int ret;

	if (i2s_dev_rx == i2s_dev_tx) {
		ret = i2s_configure(i2s_dev_rx, I2S_DIR_BOTH, config);
		if (ret == 0) {
			return true;
		}
		if (ret != -ENOSYS) {
			LOG_ERR("Failed to configure streams: %d", ret);
			return false;
		}
	}

	ret = i2s_configure(i2s_dev_rx, I2S_DIR_RX, config);
	if (ret < 0) {
		LOG_ERR("Failed to configure RX stream: %d", ret);
		return false;
	}

	ret = i2s_configure(i2s_dev_tx, I2S_DIR_TX, config);
	if (ret < 0) {
		LOG_ERR("Failed to configure TX stream: %d", ret);
		return false;
	}

	return true;
}

static bool i2s_prepare_transfer(const struct device *i2s_dev_tx)
{
	int ret;

	for (int i = 0; i < INITIAL_BLOCKS; ++i) {
		void *mem_block;

		ret = k_mem_slab_alloc(&mem_slab, &mem_block, K_NO_WAIT);
		if (ret < 0) {
			LOG_ERR("Failed to allocate TX block %d: %d", i, ret);
			return false;
		}

		memset(mem_block, 0, BLOCK_SIZE);

		ret = i2s_write(i2s_dev_tx, mem_block, BLOCK_SIZE);
		if (ret < 0) {
			LOG_ERR("Failed to write block %d: %d", i, ret);
			k_mem_slab_free(&mem_slab, mem_block);
			return false;
		}
	}

	return true;
}

static bool i2s_trigger_command(const struct device *i2s_dev_rx,
				const struct device *i2s_dev_tx,
				enum i2s_trigger_cmd cmd)
{
	int ret;

	if (i2s_dev_rx == i2s_dev_tx) {
		ret = i2s_trigger(i2s_dev_rx, I2S_DIR_BOTH, cmd);
		if (ret == 0) {
			return true;
		}
		if (ret != -ENOSYS) {
			LOG_ERR("Failed to trigger command %d: %d", cmd, ret);
			return false;
		}
	}

	ret = i2s_trigger(i2s_dev_rx, I2S_DIR_RX, cmd);
	if (ret < 0) {
		LOG_ERR("Failed to trigger command %d on RX: %d", cmd, ret);
		return false;
	}

	ret = i2s_trigger(i2s_dev_tx, I2S_DIR_TX, cmd);
	if (ret < 0) {
		LOG_ERR("Failed to trigger command %d on TX: %d", cmd, ret);
		return false;
	}

	return true;
}

/**
 * Configure and start I2S streaming.
 * Returns true on success, false on failure.
 */
static bool i2s_start(const struct device *i2s_dev_rx,
		      const struct device *i2s_dev_tx)
{
	struct i2s_config config;

	config.word_size = SAMPLE_BIT_WIDTH;
	config.channels = NUMBER_OF_CHANNELS;
	config.format = I2S_FMT_DATA_FORMAT_I2S;
	config.options = I2S_OPT_BIT_CLK_MASTER | I2S_OPT_FRAME_CLK_MASTER;
	config.frame_clk_freq = SAMPLE_FREQUENCY;
	config.mem_slab = &mem_slab;
	config.block_size = BLOCK_SIZE;
	config.timeout = TIMEOUT;

	if (!i2s_configure_streams(i2s_dev_rx, i2s_dev_tx, &config)) {
		return false;
	}

	if (!i2s_prepare_transfer(i2s_dev_tx)) {
		return false;
	}

	if (!i2s_trigger_command(i2s_dev_rx, i2s_dev_tx, I2S_TRIGGER_START)) {
		return false;
	}

	return true;
}

/**
 * Run I2S echo loop for a given number of iterations.
 * Each iteration reads one block from RX, processes it, and writes to TX.
 */
static void i2s_stream_for(const struct device *i2s_dev_rx,
			   const struct device *i2s_dev_tx,
			   int iterations)
{
	for (int i = 0; i < iterations; i++) {
		void *mem_block;
		uint32_t block_size;
		int ret;

		ret = i2s_read(i2s_dev_rx, &mem_block, &block_size);
		if (ret < 0) {
			LOG_ERR("read failed: %d", ret);
			break;
		}

		process_block_data(mem_block, SAMPLES_PER_BLOCK);

		ret = i2s_write(i2s_dev_tx, mem_block, block_size);
		if (ret < 0) {
			LOG_ERR("write failed: %d", ret);
			k_mem_slab_free(&mem_slab, mem_block);
			break;
		}
	}
	LOG_INF("stream loop exited");
}

/**
 * Stop and drop I2S streams cleanly.
 */
static void i2s_stop(const struct device *i2s_dev_rx,
		     const struct device *i2s_dev_tx)
{
	int ret;

	/* DROP handles any state (RUNNING, ERROR, READY, STOPPING)
	 * except NOT_READY, so no need for PREPARE first.
	 */
	ret = i2s_trigger(i2s_dev_rx, I2S_DIR_RX, I2S_TRIGGER_DROP);
	LOG_INF("%s: RX drop returned %d", __func__, ret);

	ret = i2s_trigger(i2s_dev_tx, I2S_DIR_TX, I2S_TRIGGER_DROP);
	LOG_INF("%s: TX drop returned %d", __func__, ret);

	k_sleep(K_MSEC(100));
	LOG_INF("%s: done", __func__);
}

/*
 * start → stream → stop, with a phase label
 */
static void i2s_echo_run(const struct device *i2s_dev_rx,
			 const struct device *i2s_dev_tx,
			 const char *phase_label)
{
	LOG_INF("[I2S Demo] %s", phase_label);
	if (!i2s_start(i2s_dev_rx, i2s_dev_tx)) {
		LOG_ERR("Failed to start I2S (%s) - skipping", phase_label);
		return;
	}
	i2s_stream_for(i2s_dev_rx, i2s_dev_tx, I2S_STREAM_ITERATIONS);
	i2s_stop(i2s_dev_rx, i2s_dev_tx);
}

/* ========================================================================
 * Power Management Functions
 * ========================================================================
 */

/**
 * Helper function to lock/unlock deeper power states.
 * @param lock true → lock all deep states (allow RUNTIME_IDLE only)
 *             false → unlock the applicable state; keep the other locked
 */
static void app_pm_lock_deeper_states(bool lock)
{
	if (lock) {
		/*
		 * Lock both deep states unconditionally so PM policy cannot
		 * accidentally enter them during the RUNTIME_IDLE phase, where
		 * the 18 s sleep exceeds the reduced min-residency values.
		 */
		pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_RAM, PM_ALL_SUBSTATES);
		pm_policy_state_lock_get(PM_STATE_SOFT_OFF,       PM_ALL_SUBSTATES);
	} else if (S2RAM_SUPPORTED) {
		pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_RAM, PM_ALL_SUBSTATES);
		/* SOFT_OFF stays locked for the entire demo */
	} else {
		pm_policy_state_lock_put(PM_STATE_SOFT_OFF, PM_ALL_SUBSTATES);
		/* S2RAM stays locked for the entire demo */
	}
}

/*
 * This function will be invoked in the PRE_KERNEL_2 phase of the init routine.
 */
static int app_pre_kernel_init(void)
{
	/* Lock deeper power states to allow only RUNTIME_IDLE */
	app_pm_lock_deeper_states(true);

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

/* ========================================================================
 * PM Notifier
 * ========================================================================
 */

/* PM state name lookup for notifier logging */
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

	LOG_INF("PM enter: %s (substate %u)", PM_STATE_STR(state), info->substate_id);
}
/*
 * With CONFIG_LOG_MODE_DEFERRED, LOG_INF here only writes to the ring buffer
 * — no UART access — so this is safe even before devices are resumed.
 */
static void pm_notify_pre_resume(enum pm_state state)
{
	const struct pm_state_info *info = pm_state_next_get(0);
	uint32_t active_exc = SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk;

	if (active_exc >= 16U) {
		LOG_INF("PM wakeup: %s (substate %u) IRQ %u",
			PM_STATE_STR(state), info->substate_id, active_exc - 16U);
	} else if (active_exc != 0U) {
		LOG_INF("PM wakeup: %s (substate %u) exception %u",
			PM_STATE_STR(state), info->substate_id, active_exc);
	} else {
		LOG_INF("PM wakeup: %s (substate %u)",
			PM_STATE_STR(state), info->substate_id);
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

	LOG_INF("PM exit:  %s (substate %u)", PM_STATE_STR(state), info->substate_id);
}

static struct pm_notifier app_pm_notifier = {
	.state_entry       = pm_notify_entry,
	.pre_device_resume = pm_notify_pre_resume,
	.state_exit        = pm_notify_exit,
};

/* ========================================================================
 * Main Thread — I2S + PM
 * ========================================================================
 */

int main(void)
{
	const struct device *const cons = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
	const struct device *const wakeup_dev = DEVICE_DT_GET(WAKEUP_SOURCE);
	const struct device *const i2s_dev_rx = DEVICE_DT_GET(I2S_RX_NODE);
	const struct device *const i2s_dev_tx = DEVICE_DT_GET(I2S_TX_NODE);
	int ret;

	__ASSERT(device_is_ready(cons), "%s: device not ready", cons->name);
	__ASSERT(device_is_ready(wakeup_dev), "%s: device not ready", wakeup_dev->name);
	__ASSERT(device_is_ready(i2s_dev_rx), "%s: device not ready", i2s_dev_rx->name);
	if (i2s_dev_rx != i2s_dev_tx) {
		__ASSERT(device_is_ready(i2s_dev_tx), "%s: device not ready",
			 i2s_dev_tx->name);
	}

	pm_notifier_register(&app_pm_notifier);

	if (S2RAM_SUPPORTED) {
		LOG_INF("%s (S2RAM): I2S PM demo "
			"(RUNTIME_IDLE, SUSPEND_TO_IDLE, S2RAM STANDBY, S2RAM STOP)",
			CONFIG_BOARD);
	} else {
		LOG_INF("%s (SOFT_OFF): I2S PM demo "
			"(RUNTIME_IDLE, SUSPEND_TO_IDLE, SOFT_OFF)",
			CONFIG_BOARD);
	}

	ret = counter_start(wakeup_dev);
	__ASSERT(!ret || ret == -EALREADY, "Failed to start counter (err %d)", ret);

	LOG_INF("POWER STATE SEQUENCE:");
	LOG_INF("  1. PM_STATE_RUNTIME_IDLE");
	LOG_INF("  2. PM_STATE_SUSPEND_TO_IDLE");
	if (S2RAM_SUPPORTED) {
		LOG_INF("  3. PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY)");
		LOG_INF("  4. PM_STATE_SUSPEND_TO_RAM (substate 1: STOP)");
	} else {
		LOG_INF("  3. PM_STATE_SOFT_OFF");
	}

	 /* Lock SUSPEND_TO_IDLE to force PM policy to select RUNTIME_IDLE only */
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
	i2s_echo_run(i2s_dev_rx, i2s_dev_tx, "before RUNTIME_IDLE");

	LOG_INF("Enter RUNTIME_IDLE sleep for (%d microseconds)", RUNTIME_IDLE_SLEEP_USEC);
	ret = app_enter_normal_sleep(RUNTIME_IDLE_SLEEP_USEC);
	__ASSERT(ret == 0, "Could not enter RUNTIME_IDLE sleep (err %d)", ret);

	LOG_INF("Exited from RUNTIME_IDLE sleep");
	pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);

	/* --- SUSPEND_TO_IDLE --- */
#if defined(CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER)
	LOG_INF("Enter PM_STATE_SUSPEND_TO_IDLE for (%d microseconds)",
		SUSPEND_IDLE_SLEEP_USEC);
	k_sleep(K_USEC(SUSPEND_IDLE_SLEEP_USEC));
	LOG_INF("Exited from PM_STATE_SUSPEND_TO_IDLE");
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
	i2s_echo_run(i2s_dev_rx, i2s_dev_tx, "after SUSPEND_TO_IDLE");
	pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
#else
	/* Lock SUSPEND_TO_IDLE when LPM timer support is not configured */
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
	LOG_INF("PM_STATE_SUSPEND_TO_IDLE (skipped - LPM timer not enabled)");
#endif

	/* Unlock deeper power states to allow S2RAM and/or SOFT_OFF */
	app_pm_lock_deeper_states(false);

	if (S2RAM_SUPPORTED) {
		/* --- S2RAM STANDBY --- */
		LOG_INF("Enter PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) for (%d microseconds)",
			S2RAM_STANDBY_SLEEP_USEC);
		ret = app_enter_deep_sleep(S2RAM_STANDBY_SLEEP_USEC);
		__ASSERT(ret == 0, "Could not enter PM_STATE_SUSPEND_TO_RAM (err %d)", ret);

		LOG_INF("=== Resumed from PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) ===");
		pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
		i2s_echo_run(i2s_dev_rx, i2s_dev_tx, "after S2RAM STANDBY");
		pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);

		/* --- S2RAM STOP --- */
		LOG_INF("Enter PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) for (%d microseconds)",
			S2RAM_STOP_SLEEP_USEC);
		ret = app_enter_deep_sleep(S2RAM_STOP_SLEEP_USEC);
		__ASSERT(ret == 0, "Could not enter PM_STATE_SUSPEND_TO_RAM (err %d)", ret);

		LOG_INF("=== Resumed from PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) ===");
		pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
		i2s_echo_run(i2s_dev_rx, i2s_dev_tx, "after S2RAM STOP");
		pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
	}

	if (SOFT_OFF_SUPPORTED) {
		LOG_INF("Enter PM_STATE_SOFT_OFF for (%d microseconds)", SOFT_OFF_SLEEP_USEC);
		LOG_INF("Note: SOFT_OFF has no retention - system will reset on wakeup");
		ret = app_enter_deep_sleep(SOFT_OFF_SLEEP_USEC);
		__ASSERT(ret == 0, "Could not enter PM_STATE_SOFT_OFF (err %d)", ret);

		/* Should never reach here - SOFT_OFF causes full reset on wakeup */
		LOG_ERR("ERROR: Resumed after PM_STATE_SOFT_OFF - this should not happen!");
		__ASSERT(false, "PM_STATE_SOFT_OFF should have caused a reset");
	}

	LOG_INF("=== I2S PM SEQUENCE COMPLETED ===");

	app_pm_lock_deeper_states(true);
	/* Demo complete — prevent SUSPEND_TO_IDLE so the idle spin stays in RUNTIME_IDLE */
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);

	while (true) {
		/* spin here */
		k_sleep(K_SECONDS(1));
	}


	return 0;
}

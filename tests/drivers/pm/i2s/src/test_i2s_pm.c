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
 * System-level I2S power-management tests.
 *
 * Device suites call pm_device_action_run() and check the busy flag.
 * Runtime idle, suspend-to-idle, and soft-off run on the MRAM image.
 * Standby and stop are built only with -S s2ram.
 */

#include "aipm.h"
#include <stdio.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/pm/device.h>
#include <zephyr/pm/pm.h>
#include <zephyr/pm/policy.h>
#if defined(CONFIG_POWEROFF)
#include <zephyr/sys/poweroff.h>
#endif
#include <zephyr/drivers/counter.h>
#include <zephyr/drivers/i2s.h>
#include <se_service.h>
#include <zephyr/sys/util.h>

#include <zephyr/ztest.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(i2s_pm, LOG_LEVEL_INF);

/* ========================================================================
 * I2S Configuration
 * ========================================================================
 */

#if DT_NODE_EXISTS(DT_NODELABEL(i2s_rxtx))
#define I2S_RX_NODE DT_NODELABEL(i2s_rxtx)
#define I2S_TX_NODE I2S_RX_NODE
#else
#define I2S_RX_NODE DT_NODELABEL(i2s_rx)
#define I2S_TX_NODE DT_NODELABEL(i2s_tx)
#endif

#define SAMPLE_FREQUENCY   44100
#define SAMPLE_BIT_WIDTH   16
#define BYTES_PER_SAMPLE   sizeof(int16_t)
#define NUMBER_OF_CHANNELS 2
/* Such block length provides an echo with the delay of 100 ms. */
#define SAMPLES_PER_BLOCK  ((SAMPLE_FREQUENCY / 10) * NUMBER_OF_CHANNELS)
#define INITIAL_BLOCKS     4
#define TIMEOUT            1000

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

/**
 * As per the test requirements, unused memory blocks can be removed.
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

#if defined(CONFIG_RTSS_HP) && \
	DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(timer0), snps_dw_timers, okay)
#define WAKEUP_SOURCE         DT_NODELABEL(timer0)
#define SE_OFFP_EWIC_CFG      EWIC_VBAT_TIMER
#define SE_OFFP_WAKEUP_EVENTS WE_LPTIMER0
#elif DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(rtc0), snps_dw_apb_rtc, okay)
#define WAKEUP_SOURCE         DT_NODELABEL(rtc0)
#define SE_OFFP_EWIC_CFG      EWIC_RTC_A
#define SE_OFFP_WAKEUP_EVENTS WE_LPRTC
#elif DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(timer0), snps_dw_timers, okay)
#define WAKEUP_SOURCE         DT_NODELABEL(timer0)
#define SE_OFFP_EWIC_CFG      EWIC_VBAT_TIMER
#define SE_OFFP_WAKEUP_EVENTS WE_LPTIMER0
#endif

/* Sleep duration for PM_STATE_RUNTIME_IDLE */
#define RUNTIME_IDLE_SLEEP_USEC  (18 * 1000 * 1000)
/* Sleep duration for PM_STATE_SUSPEND_TO_IDLE */
#define SUSPEND_IDLE_SLEEP_USEC  (4 * 1000)
/* Sleep duration for PM_STATE_SUSPEND_TO_RAM substate 0 (STANDBY) */
#define S2RAM_STANDBY_SLEEP_USEC (6 * 1000 * 1000)
/* Sleep duration for PM_STATE_SUSPEND_TO_RAM substate 1 (STOP) */
#define S2RAM_STOP_SLEEP_USEC    (9 * 1000 * 1000)
/* Sleep duration for PM_STATE_SOFT_OFF */
#define SOFT_OFF_SLEEP_USEC      (10 * 1000 * 1000)

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
 * PM_STATE_SOFT_OFF:
 * - HP: always
 * - HE MRAM image: yes
 * - HE TCM image (CONFIG_PM_S2RAM): no. That state stays locked.
 */
#if defined(CONFIG_RTSS_HP) || \
	(defined(CONFIG_RTSS_HE) && !IS_ENABLED(CONFIG_PM_S2RAM))
#define SOFT_OFF_SUPPORTED 1
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

static bool i2s_configure_streams(const struct device *i2s_dev_rx, const struct device *i2s_dev_tx,
				  const struct i2s_config *config)
{
	int ret;

	if (i2s_dev_rx == i2s_dev_tx) {
		ret = i2s_configure(i2s_dev_rx, I2S_DIR_BOTH, config);
		if (ret == 0) {
			return true;
		}
		/* DesignWare returns -ENOSYS for I2S_DIR_BOTH. -ENOTSUP
		 * means the same: configure RX and TX separately.
		 */
		if (ret != -ENOTSUP && ret != -ENOSYS) {
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

static bool i2s_trigger_command(const struct device *i2s_dev_rx, const struct device *i2s_dev_tx,
				enum i2s_trigger_cmd cmd)
{
	int ret;

	if (i2s_dev_rx == i2s_dev_tx) {
		ret = i2s_trigger(i2s_dev_rx, I2S_DIR_BOTH, cmd);
		if (ret == 0) {
			return true;
		}
		/* -ENOTSUP means that commands for the RX and TX streams need
		 * to be triggered separately.
		 */
		if (ret != -ENOTSUP) {
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
static bool i2s_start(const struct device *i2s_dev_rx, const struct device *i2s_dev_tx)
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
static bool i2s_stream_for(const struct device *i2s_dev_rx, const struct device *i2s_dev_tx,
			   int iterations)
{
	int moved = 0;

	for (int i = 0; i < iterations; i++) {
		void *mem_block;
		uint32_t block_size;
		int ret;

		ret = i2s_read(i2s_dev_rx, &mem_block, &block_size);
		if (ret < 0) {
			LOG_ERR("read failed: %d", ret);
			return false;
		}

		process_block_data(mem_block, SAMPLES_PER_BLOCK);

		ret = i2s_write(i2s_dev_tx, mem_block, block_size);
		if (ret < 0) {
			LOG_ERR("write failed: %d", ret);
			k_mem_slab_free(&mem_slab, mem_block);
			return false;
		}
		moved++;
	}
	LOG_INF("stream loop exited");
	return moved == iterations;
}

/*
 * NOT_READY returns -EIO. That is an idle stream. Any other error means
 * the direction is still running.
 */
static bool drop_ok_or_idle(const struct device *dev, enum i2s_dir dir)
{
	int ret = i2s_trigger(dev, dir, I2S_TRIGGER_DROP);

	if (ret == 0 || ret == -EIO) {
		return true;
	}

	LOG_ERR("DROP dir %d failed: %d", dir, ret);
	return false;
}

/**
 * Stop and drop I2S streams cleanly.
 */
static bool i2s_stop(const struct device *i2s_dev_rx, const struct device *i2s_dev_tx)
{
	bool rx_dropped;
	bool tx_dropped;

	/* DROP handles any state (RUNNING, ERROR, READY, STOPPING)
	 * except NOT_READY, so no need for PREPARE first.
	 */
	rx_dropped = drop_ok_or_idle(i2s_dev_rx, I2S_DIR_RX);
	tx_dropped = drop_ok_or_idle(i2s_dev_tx, I2S_DIR_TX);
	if (!rx_dropped || !tx_dropped) {
		return false;
	}

	k_sleep(K_MSEC(100));
	LOG_INF("%s: done", __func__);
	return true;
}

/* ========================================================================
 * Power Management Functions
 * ========================================================================
 */

/**
 * Set the RUN profile parameters for this test.
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

#if defined(WAKEUP_SOURCE)
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
#endif /* WAKEUP_SOURCE */

/**
 * PM Notifier callback for power state entry
 */
static volatile enum pm_state seen_state;
static volatile uint8_t seen_sub;
static volatile bool seen_entry;
static volatile enum pm_device_state seen_tx_state;
static volatile enum pm_device_state seen_rx_state;
static volatile bool seen_dev_state;

const struct device *i2s_dev_rx;
const struct device *i2s_dev_tx;

static void clear_seen_state(void)
{
	seen_entry = false;
	seen_dev_state = false;
}

static void expect_pm_state(enum pm_state state, int substate)
{
	zassert_true(seen_entry, "CPU did not enter a PM state");
	zassert_equal(seen_state, state, "entered state %d, expected %d",
		      seen_state, state);
	if (substate >= 0) {
		zassert_equal(seen_sub, (uint8_t)substate,
			      "entered substate %u, expected %d", seen_sub, substate);
	}
}

#if IS_ENABLED(CONFIG_PM_S2RAM)
static void expect_i2s_suspended(void)
{
	zassert_true(seen_dev_state, "I2S state was not captured on entry");
	zassert_equal(seen_tx_state, PM_DEVICE_STATE_SUSPENDED,
		      "TX state %d while the CPU was suspended", seen_tx_state);
	if (i2s_dev_rx != NULL && i2s_dev_rx != i2s_dev_tx) {
		zassert_equal(seen_rx_state, PM_DEVICE_STATE_SUSPENDED,
			      "RX state %d while the CPU was suspended", seen_rx_state);
	}
}
#endif

static void pm_notify_state_entry(enum pm_state state)
{
	const struct pm_state_info *next_state = pm_state_next_get(0);
	uint8_t substate_id = next_state ? next_state->substate_id : 0;
	int ret;

	seen_state = state;
	seen_sub = substate_id;
	seen_entry = true;

	switch (state) {
	case PM_STATE_RUNTIME_IDLE:
	case PM_STATE_SUSPEND_TO_IDLE:
		/* No action needed */
		break;
	case PM_STATE_SUSPEND_TO_RAM:
		if (i2s_dev_tx != NULL) {
			enum pm_device_state tx_state;
			enum pm_device_state rx_state;

			if (pm_device_state_get(i2s_dev_tx, &tx_state) == 0) {
				seen_tx_state = tx_state;
				seen_dev_state = true;
			}
			if (i2s_dev_rx != NULL && i2s_dev_rx != i2s_dev_tx &&
			    pm_device_state_get(i2s_dev_rx, &rx_state) == 0) {
				seen_rx_state = rx_state;
			}
		}
		__fallthrough;
	case PM_STATE_SOFT_OFF:
#if defined(WAKEUP_SOURCE)
		ret = app_set_off_params(state, substate_id);
		__ASSERT(ret == 0, "app_set_off_params failed = %d", ret);
#else
		ARG_UNUSED(ret);
		ARG_UNUSED(substate_id);
#endif
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
	case PM_STATE_RUNTIME_IDLE:
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
	const char *state_desc = "none";

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
	 * HE core:
	 * - TCM image: toggle S2RAM. Soft-off stays locked.
	 * - MRAM image: toggle SOFT_OFF. Keep S2RAM locked.
	 */
	enum pm_state deep_states[2];
	int num_states = 0;

	if (S2RAM_SUPPORTED) {
		/* TCM boot: S2RAM works with retention.
		 * The s2ram overlay also enables soft-off. This image does
		 * not run that state, so leave it locked.
		 */
		static bool soft_off_locked;

		deep_states[num_states++] = PM_STATE_SUSPEND_TO_RAM;
		state_desc = "S2RAM";
		if (lock && !soft_off_locked) {
			pm_policy_state_lock_get(PM_STATE_SOFT_OFF, PM_ALL_SUBSTATES);
			soft_off_locked = true;
		}
	} else {
		/* MRAM boot: Keep S2RAM locked so SOFT_OFF is selected */
		if (!pm_policy_state_lock_is_active(PM_STATE_SUSPEND_TO_RAM,
						    PM_ALL_SUBSTATES)) {
			/* Ensure S2RAM stays locked when unlocking SOFT_OFF */
			pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_RAM, PM_ALL_SUBSTATES);
		}
	}

#if SOFT_OFF_SUPPORTED
	deep_states[num_states++] = PM_STATE_SOFT_OFF;
	state_desc = "SOFT_OFF";
#endif

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

	/* Also lock SUSPEND_TO_IDLE until a proper wakeup source is configured.
	 * Without this, any k_sleep before the PM sequence could enter
	 * SUSPEND_TO_IDLE with no LPM timer to wake up, causing a hang.
	 */
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);

	/* Register PM notifier callbacks */
	pm_notifier_register(&app_pm_notifier);

	return 0;
}
SYS_INIT(app_pre_kernel_init, PRE_KERNEL_2, 0);

#if defined(WAKEUP_SOURCE)
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

#if defined(CONFIG_RTSS_HP)
/*
 * HP soft-off wakes from timer0. The kernel idle timer stays rtc0, so this
 * alarm has to be programmed here. The timer driver rejects an alarm that
 * has no callback.
 */
static void hp_soft_off_alarm_cb(const struct device *dev, uint8_t chan_id,
				 uint32_t ticks, void *user_data)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(chan_id);
	ARG_UNUSED(ticks);
	ARG_UNUSED(user_data);
}

static int arm_hp_soft_off_timer(uint32_t sleep_usec)
{
	const struct device *const timer = DEVICE_DT_GET(WAKEUP_SOURCE);
	struct counter_alarm_cfg alarm_cfg = {
		.callback = hp_soft_off_alarm_cb,
		.ticks = counter_us_to_ticks(timer, sleep_usec),
	};
	int ret;

	ret = counter_start(timer);
	if (ret != 0) {
		LOG_ERR("timer0 start failed (%d)", ret);
		return ret;
	}

	ret = counter_set_channel_alarm(timer, 0, &alarm_cfg);
	if (ret == -EBUSY) {
		counter_cancel_channel_alarm(timer, 0);
		ret = counter_set_channel_alarm(timer, 0, &alarm_cfg);
	}
	if (ret != 0) {
		LOG_ERR("timer0 alarm failed (%d)", ret);
		return ret;
	}

	LOG_INF("timer0 alarm set for %u us", sleep_usec);
	return 0;
}
#endif

#if !defined(CONFIG_POWEROFF)
static int app_enter_deep_sleep(uint32_t sleep_usec)
{
#if defined(CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER)
	/**
	 * Set a delay more than the min-residency-us configured so that
	 * the sub-system will go to OFF state.
	 */
#if defined(CONFIG_RTSS_HP)
	int ret = arm_hp_soft_off_timer(sleep_usec);

	if (ret != 0) {
		return ret;
	}
#endif
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

	return 0;
}
#endif /* !CONFIG_POWEROFF */

/*==============================================================================================*/

const struct device *cons;
const struct device *wakeup_dev;

void *setup(void)
{

	cons = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
	wakeup_dev = DEVICE_DT_GET(WAKEUP_SOURCE);
	i2s_dev_rx = DEVICE_DT_GET(I2S_RX_NODE);
	i2s_dev_tx = DEVICE_DT_GET(I2S_TX_NODE);

	__ASSERT(device_is_ready(cons), "%s: device not ready", cons->name);
	__ASSERT(device_is_ready(wakeup_dev), "%s: device not ready", wakeup_dev->name);
	__ASSERT(device_is_ready(i2s_dev_rx), "%s: device not ready", i2s_dev_rx->name);
	if (i2s_dev_rx != i2s_dev_tx) {
		__ASSERT(device_is_ready(i2s_dev_tx), "%s: device not ready", i2s_dev_tx->name);
	}

#if defined(CONFIG_RTSS_HE)
	/* Boot location determines which PM states are available */
	bool is_mram_boot = IS_BOOTING_FROM_MRAM();

	if (is_mram_boot) {
		LOG_INF("%s RTSS_HE (MRAM boot): I2S PM test "
			"(RUNTIME_IDLE, SUSPEND_TO_IDLE, SOFT_OFF)",
			CONFIG_BOARD);
	} else {
		LOG_INF("%s RTSS_HE (TCM boot): I2S PM test "
			"(RUNTIME_IDLE, SUSPEND_TO_IDLE, S2RAM)",
			CONFIG_BOARD);
	}
#else
	LOG_INF("%s RTSS_HP: I2S PM test "
		"(RUNTIME_IDLE, SUSPEND_TO_IDLE, SOFT_OFF)",
		CONFIG_BOARD);
#endif

	int ret = counter_start(wakeup_dev);

	__ASSERT(!ret || ret == -EALREADY, "Failed to start counter (err %d)", ret);

	return NULL;
}

/* The echo blocks in i2s_read(). Runtime idle must stay locked for that
 * wait: the CPU test enters it only during the sleep in the test body.
 * Drop first so a failed earlier stream cannot leave the driver busy.
 */
static void stream_while_active(void)
{
	bool dropped;
	bool started;
	bool streamed;
	bool stopped;

	pm_policy_state_lock_get(PM_STATE_RUNTIME_IDLE, PM_ALL_SUBSTATES);
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);

	dropped = drop_ok_or_idle(i2s_dev_rx, I2S_DIR_RX) &&
		  drop_ok_or_idle(i2s_dev_tx, I2S_DIR_TX);

	started = dropped && i2s_start(i2s_dev_rx, i2s_dev_tx);
	streamed = started &&
		   i2s_stream_for(i2s_dev_rx, i2s_dev_tx, I2S_STREAM_ITERATIONS);
	stopped = true;
	if (started) {
		stopped = i2s_stop(i2s_dev_rx, i2s_dev_tx);
	}

	pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
	pm_policy_state_lock_put(PM_STATE_RUNTIME_IDLE, PM_ALL_SUBSTATES);

	zassert_true(dropped, "DROP failed before stream");
	zassert_true(started, "Failed to start I2S");
	zassert_true(streamed, "stream failed");
	zassert_true(stopped, "DROP failed after stream");
}

void before(void *fixture)
{
	ARG_UNUSED(fixture);
	stream_while_active();
}

void after(void *fixture)
{
	ARG_UNUSED(fixture);
	stream_while_active();
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

ZTEST_SUITE(i2s_pm_runtime_idle, NULL, setup, before, after, teardown);
ZTEST_SUITE(i2s_pm_suspend_to_idle, NULL, setup, before, after, teardown);
#if !IS_ENABLED(CONFIG_PM_S2RAM)
ZTEST_SUITE(i2s_pm_system_soft_off, NULL, setup, before, after, teardown);
#endif
#if IS_ENABLED(CONFIG_PM_S2RAM)
ZTEST_SUITE(i2s_pm_standby, NULL, setup, before, after, teardown);
ZTEST_SUITE(i2s_pm_stop, NULL, setup, before, after, teardown);
#endif

/*========================RUNTIME_IDLE TESTS========================*/

ZTEST(i2s_pm_runtime_idle, test_stream_across_runtime_idle)
{

	LOG_INF("=== stream across RUNTIME_IDLE ===");

	/* Deeper states stay locked. This SoC does not implement
	 * PM_STATE_RUNTIME_IDLE, so the sleep is kernel idle (WFI).
	 * Adding that power state makes pm_state_set() return without
	 * waiting and the 18 s sleep never completes.
	 */
	LOG_INF("Enter RUNTIME_IDLE sleep for (%d microseconds)", RUNTIME_IDLE_SLEEP_USEC);
	clear_seen_state();
	int ret = app_enter_normal_sleep(RUNTIME_IDLE_SLEEP_USEC);

	__ASSERT(ret == 0, "Could not enter RUNTIME_IDLE sleep (err %d)", ret);
	zassert_false(seen_entry, "entered state %d during runtime idle",
		      seen_state);

	LOG_INF("Exited from RUNTIME_IDLE sleep");
}


/*========================END RUNTIME_IDLE TESTS========================*/

/*========================SUSPEND_TO_IDLE TESTS========================*/

ZTEST(i2s_pm_suspend_to_idle, test_stream_across_suspend_to_idle)
{

#if defined(CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER)
	LOG_INF("=== stream across SUSPEND_TO_IDLE ===");

	/* Unlock SUSPEND_TO_IDLE so PM framework can select it */
	pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);

	LOG_INF("Enter PM_STATE_SUSPEND_TO_IDLE for (%d microseconds)", SUSPEND_IDLE_SLEEP_USEC);
	clear_seen_state();
	k_sleep(K_USEC(SUSPEND_IDLE_SLEEP_USEC));
	expect_pm_state(PM_STATE_SUSPEND_TO_IDLE, 0);
	LOG_INF("Exited from PM_STATE_SUSPEND_TO_IDLE");

	/* Re-lock to prevent accidental entry during cleanup */
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
#else
	LOG_INF("=== SUSPEND_TO_IDLE skipped (LPM timer not enabled) ===");
	ztest_test_skip();
#endif
}


/*========================End of SUSPEND_TO_IDLE tests========================*/

/*========================SOFT OFF TESTS========================*/

#if !IS_ENABLED(CONFIG_PM_S2RAM)
ZTEST(i2s_pm_system_soft_off, test_stream_then_soft_off)
{

	LOG_INF("=== stream then SOFT_OFF ===");

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

#if SOFT_OFF_SUPPORTED
	LOG_INF("Enter PM_STATE_SOFT_OFF for (%d microseconds)", SOFT_OFF_SLEEP_USEC);

	int ret = app_enter_deep_sleep(SOFT_OFF_SLEEP_USEC);

	__ASSERT(ret == 0, "Could not enter SOFT_OFF sleep (err %d)", ret);

	LOG_ERR("ERROR: Resumed after SOFT OFF - This should not happen");
	__ASSERT(false, "PM_STATE_SOFT_OFF should have caused a reset");
#endif
#endif

	app_pm_lock_deeper_states(true);
}
#endif /* !CONFIG_PM_S2RAM */

/*========================END OF SOFT OFF TESTS========================*/

#if IS_ENABLED(CONFIG_PM_S2RAM)
/*========================S2RAM_STANDBY TESTS========================*/

ZTEST(i2s_pm_standby, test_stream_across_standby)
{

	LOG_INF("=== stream across STANDBY ===");

	app_pm_lock_deeper_states(false);

	LOG_INF("Enter PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) for (%d microseconds)",
		S2RAM_STANDBY_SLEEP_USEC);

	clear_seen_state();
	int ret = app_enter_deep_sleep(S2RAM_STANDBY_SLEEP_USEC);

	__ASSERT(ret == 0, "Could not enter S2RAM_STANDBY sleep (err %d)", ret);
	expect_pm_state(PM_STATE_SUSPEND_TO_RAM, 0);
	expect_i2s_suspended();

	LOG_INF("Resumed from PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY)");

	/* Re-lock deeper states immediately to prevent k_sleep from
	 * re-entering S2RAM during post-resume verification.
	 */
	app_pm_lock_deeper_states(true);
}


/*========================END OF S2RAM_STANDBY TESTS========================*/

/*========================S2RAM_STOP TESTS========================*/

ZTEST(i2s_pm_stop, test_stream_across_stop)
{

	LOG_INF("=== stream across STOP ===");

	app_pm_lock_deeper_states(false);

	LOG_INF("Enter PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) for (%d microseconds)",
		S2RAM_STOP_SLEEP_USEC);

	clear_seen_state();
	int ret = app_enter_deep_sleep(S2RAM_STOP_SLEEP_USEC);

	__ASSERT(ret == 0, "Could not enter S2RAM_STOP sleep (err %d)", ret);
	expect_pm_state(PM_STATE_SUSPEND_TO_RAM, 1);
	expect_i2s_suspended();

	LOG_INF("Resumed from PM_STATE_SUSPEND_TO_RAM (substate 1: STOP)");

	/* Re-lock deeper states immediately to prevent k_sleep from
	 * re-entering S2RAM during post-resume verification.
	 */
	app_pm_lock_deeper_states(true);
}

	/*========================END OF S2RAM_STOP TESTS========================*/
#endif /* CONFIG_PM_S2RAM */
#endif /* WAKEUP_SOURCE */

/* ========================================================================
 * Device PM: pm_device_action_run() and the busy flag.
 * Short blocks use their own slab so they do not share the echo buffer.
 * ========================================================================
 */

/* 20 ms block. Four queued blocks outlast a short SUSPEND_TO_IDLE. */
#define DEV_SAMPLES_PER_BLOCK   ((SAMPLE_FREQUENCY / 50) * NUMBER_OF_CHANNELS)
#define DEV_BLOCK_SIZE          (BYTES_PER_SAMPLE * DEV_SAMPLES_PER_BLOCK)
#define DEV_BLOCK_COUNT         8
#define TX_PRIME_BLOCKS     4
#define STREAM_TIMEOUT_MS   300
/* Long enough for TX_PRIME_BLOCKS to leave the FIFO after STOP or underrun. */
#define DRAIN_WAIT_MS       200
#define SUSPEND_IDLE_USEC   (10 * 1000)

K_MEM_SLAB_DEFINE_STATIC(pm_dev_slab, DEV_BLOCK_SIZE, DEV_BLOCK_COUNT, 4);

static bool suspend_idle_unlocked;

static bool same_controller(void)
{
	return i2s_dev_rx == i2s_dev_tx;
}

static void relock_suspend_idle(void)
{
	if (suspend_idle_unlocked) {
		pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
		suspend_idle_unlocked = false;
	}
}

static void ensure_active(const struct device *dev)
{
	enum pm_device_state state;
	int ret;

	ret = pm_device_state_get(dev, &state);
	zassert_ok(ret, "pm_device_state_get(%s) failed: %d", dev->name, ret);

	if (state == PM_DEVICE_STATE_OFF) {
		ret = pm_device_action_run(dev, PM_DEVICE_ACTION_TURN_ON);
		zassert_ok(ret, "TURN_ON %s failed: %d", dev->name, ret);
		state = PM_DEVICE_STATE_SUSPENDED;
	}

	if (state == PM_DEVICE_STATE_SUSPENDED) {
		ret = pm_device_action_run(dev, PM_DEVICE_ACTION_RESUME);
		zassert_ok(ret, "RESUME %s failed: %d", dev->name, ret);
	}
}

static void recover_devices(void)
{
	relock_suspend_idle();
	ensure_active(i2s_dev_tx);
	if (!same_controller()) {
		ensure_active(i2s_dev_rx);
	}

	/* NOT_READY returns -EIO. Every other failure leaves the stream running. */
	zassert_true(drop_ok_or_idle(i2s_dev_tx, I2S_DIR_TX), "TX DROP failed");
	zassert_true(drop_ok_or_idle(i2s_dev_rx, I2S_DIR_RX), "RX DROP failed");
}

static void configure_streams_with(uint8_t word_size, uint32_t rate)
{
	struct i2s_config config = {
		.word_size = word_size,
		.channels = NUMBER_OF_CHANNELS,
		.format = I2S_FMT_DATA_FORMAT_I2S,
		.options = I2S_OPT_BIT_CLK_MASTER | I2S_OPT_FRAME_CLK_MASTER,
		.frame_clk_freq = rate,
		.mem_slab = &pm_dev_slab,
		.block_size = DEV_BLOCK_SIZE,
		.timeout = STREAM_TIMEOUT_MS,
	};
	int ret;

	ret = i2s_configure(i2s_dev_rx, I2S_DIR_RX, &config);
	zassert_ok(ret, "RX configure failed: %d", ret);

	ret = i2s_configure(i2s_dev_tx, I2S_DIR_TX, &config);
	zassert_ok(ret, "TX configure failed: %d", ret);
}

static void configure_streams(void)
{
	configure_streams_with(SAMPLE_BIT_WIDTH, SAMPLE_FREQUENCY);
}

static void assert_config_intact(const struct device *dev, enum i2s_dir dir)
{
	const struct i2s_config *got = i2s_config_get(dev, dir);

	zassert_not_null(got, "config missing on %s", dev->name);
	zassert_equal(got->frame_clk_freq, SAMPLE_FREQUENCY, "rate changed");
	zassert_equal(got->word_size, SAMPLE_BIT_WIDTH, "word size changed");
	zassert_equal(got->channels, NUMBER_OF_CHANNELS, "channels changed");
	zassert_equal(got->block_size, DEV_BLOCK_SIZE, "block size changed");
}

static void prime_tx(int count)
{
	for (int i = 0; i < count; i++) {
		void *mem_block;
		int ret;

		ret = k_mem_slab_alloc(&pm_dev_slab, &mem_block, K_NO_WAIT);
		zassert_ok(ret, "slab alloc %d failed: %d", i, ret);
		memset(mem_block, 0xA5, DEV_BLOCK_SIZE);

		ret = i2s_write(i2s_dev_tx, mem_block, DEV_BLOCK_SIZE);
		zassert_ok(ret, "prime write %d failed: %d", i, ret);
	}
}

static void start_dir(const struct device *dev, enum i2s_dir dir)
{
	int ret = i2s_trigger(dev, dir, I2S_TRIGGER_START);

	zassert_ok(ret, "START %s dir %d failed: %d", dev->name, dir, ret);
}

static void start_both(void)
{
	start_dir(i2s_dev_tx, I2S_DIR_TX);
	start_dir(i2s_dev_rx, I2S_DIR_RX);
}

static void exchange_one_block(void)
{
	void *mem_block;
	size_t size = 0;
	int ret;

	ret = i2s_read(i2s_dev_rx, &mem_block, &size);
	zassert_ok(ret, "read failed: %d", ret);
	zassert_equal(size, DEV_BLOCK_SIZE, "short RX block %u", size);

	ret = i2s_write(i2s_dev_tx, mem_block, size);
	zassert_ok(ret, "write-back failed: %d", ret);
}

static void drop_both(void)
{
	int ret;

	ret = i2s_trigger(i2s_dev_rx, I2S_DIR_RX, I2S_TRIGGER_DROP);
	zassert_ok(ret, "RX DROP failed: %d", ret);
	ret = i2s_trigger(i2s_dev_tx, I2S_DIR_TX, I2S_TRIGGER_DROP);
	zassert_ok(ret, "TX DROP failed: %d", ret);
}

static void assert_state(const struct device *dev, enum pm_device_state expect)
{
	enum pm_device_state state;
	int ret = pm_device_state_get(dev, &state);

	zassert_ok(ret, "state_get %s failed: %d", dev->name, ret);
	zassert_equal(state, expect, "%s state %d, expected %d",
		      dev->name, state, expect);
}

static void enter_suspend_to_idle(void)
{
	if (!IS_ENABLED(CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER)) {
		ztest_test_skip();
	}

	pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
	suspend_idle_unlocked = true;
	clear_seen_state();
	k_sleep(K_USEC(SUSPEND_IDLE_USEC));
	relock_suspend_idle();
	expect_pm_state(PM_STATE_SUSPEND_TO_IDLE, 0);
}

static void *pm_dev_setup(void)
{
	i2s_dev_rx = DEVICE_DT_GET(I2S_RX_NODE);
	i2s_dev_tx = DEVICE_DT_GET(I2S_TX_NODE);

	zassert_true(device_is_ready(i2s_dev_rx), "%s not ready", i2s_dev_rx->name);
	zassert_true(device_is_ready(i2s_dev_tx), "%s not ready", i2s_dev_tx->name);

	const struct device *idle_timer = DEVICE_DT_GET(DT_CHOSEN(zephyr_cortex_m_idle_timer));
	int ret = counter_start(idle_timer);

	zassert_true(ret == 0 || ret == -EALREADY, "idle timer start failed: %d", ret);
	return NULL;
}

static void pm_dev_before(void *fixture)
{
	ARG_UNUSED(fixture);
	recover_devices();
}

static void pm_dev_after(void *fixture)
{
	ARG_UNUSED(fixture);
	recover_devices();
}

ZTEST_SUITE(i2s_pm_device_actions, NULL, pm_dev_setup, pm_dev_before, pm_dev_after, NULL);
ZTEST_SUITE(i2s_pm_device_busy, NULL, pm_dev_setup, pm_dev_before, pm_dev_after, NULL);
ZTEST_SUITE(i2s_pm_device_sleep, NULL, pm_dev_setup, pm_dev_before, pm_dev_after, NULL);

/* Suspend and resume while idle. Config stays in RAM. The next START
 * reprograms word length, FIFO threshold, and the sample clock.
 */
ZTEST(i2s_pm_device_actions, test_idle_suspend_resume_then_stream)
{
	int ret;

	configure_streams();
	zassert_false(pm_device_is_busy(i2s_dev_tx), "idle TX marked busy");
	assert_state(i2s_dev_tx, PM_DEVICE_STATE_ACTIVE);
	assert_config_intact(i2s_dev_tx, I2S_DIR_TX);

	ret = pm_device_action_run(i2s_dev_tx, PM_DEVICE_ACTION_SUSPEND);
	zassert_ok(ret, "SUSPEND failed: %d", ret);
	assert_state(i2s_dev_tx, PM_DEVICE_STATE_SUSPENDED);
	assert_config_intact(i2s_dev_tx, I2S_DIR_TX);

	ret = pm_device_action_run(i2s_dev_tx, PM_DEVICE_ACTION_SUSPEND);
	zassert_equal(ret, -EALREADY, "second SUSPEND returned %d", ret);

	ret = pm_device_action_run(i2s_dev_tx, PM_DEVICE_ACTION_RESUME);
	zassert_ok(ret, "RESUME failed: %d", ret);
	assert_state(i2s_dev_tx, PM_DEVICE_STATE_ACTIVE);

	ret = pm_device_action_run(i2s_dev_tx, PM_DEVICE_ACTION_RESUME);
	zassert_equal(ret, -EALREADY, "second RESUME returned %d", ret);

	if (!same_controller()) {
		ret = pm_device_action_run(i2s_dev_rx, PM_DEVICE_ACTION_SUSPEND);
		zassert_ok(ret, "RX SUSPEND failed: %d", ret);
		ret = pm_device_action_run(i2s_dev_rx, PM_DEVICE_ACTION_RESUME);
		zassert_ok(ret, "RX RESUME failed: %d", ret);
	}

	prime_tx(TX_PRIME_BLOCKS);
	start_both();
	exchange_one_block();
	drop_both();
	zassert_false(pm_device_is_busy(i2s_dev_tx), "busy after DROP");
}

/* TURN_OFF is legal only from SUSPENDED. The driver accepts it and does
 * not touch the hardware. TURN_ON returns the device to SUSPENDED, and
 * RESUME is what restores the clock and controller.
 */
ZTEST(i2s_pm_device_actions, test_turn_off_on_then_resume)
{
	int ret;

	configure_streams();

	ret = pm_device_action_run(i2s_dev_tx, PM_DEVICE_ACTION_TURN_OFF);
	zassert_equal(ret, -ENOTSUP, "TURN_OFF from ACTIVE returned %d", ret);
	assert_state(i2s_dev_tx, PM_DEVICE_STATE_ACTIVE);

	ret = pm_device_action_run(i2s_dev_tx, PM_DEVICE_ACTION_SUSPEND);
	zassert_ok(ret, "SUSPEND failed: %d", ret);

	ret = pm_device_action_run(i2s_dev_tx, PM_DEVICE_ACTION_TURN_OFF);
	zassert_ok(ret, "TURN_OFF failed: %d", ret);
	assert_state(i2s_dev_tx, PM_DEVICE_STATE_OFF);

	ret = pm_device_action_run(i2s_dev_tx, PM_DEVICE_ACTION_TURN_ON);
	zassert_ok(ret, "TURN_ON failed: %d", ret);
	assert_state(i2s_dev_tx, PM_DEVICE_STATE_SUSPENDED);

	ret = pm_device_action_run(i2s_dev_tx, PM_DEVICE_ACTION_TURN_ON);
	zassert_equal(ret, -EALREADY, "second TURN_ON returned %d", ret);

	ret = pm_device_action_run(i2s_dev_tx, PM_DEVICE_ACTION_RESUME);
	zassert_ok(ret, "RESUME failed: %d", ret);

	prime_tx(TX_PRIME_BLOCKS);
	start_both();
	exchange_one_block();
	drop_both();
}

/* Blocks queued in READY survive clock-off. START after resume sends them
 * without another configure, which is what rewrites TFCR/RFCR today.
 */
ZTEST(i2s_pm_device_actions, test_queued_blocks_survive_suspend)
{
	int ret;

	configure_streams();
	prime_tx(TX_PRIME_BLOCKS);

	ret = pm_device_action_run(i2s_dev_tx, PM_DEVICE_ACTION_SUSPEND);
	zassert_ok(ret, "SUSPEND failed: %d", ret);
	if (!same_controller()) {
		ret = pm_device_action_run(i2s_dev_rx, PM_DEVICE_ACTION_SUSPEND);
		zassert_ok(ret, "RX SUSPEND failed: %d", ret);
	}

	ret = pm_device_action_run(i2s_dev_tx, PM_DEVICE_ACTION_RESUME);
	zassert_ok(ret, "RESUME failed: %d", ret);
	if (!same_controller()) {
		ret = pm_device_action_run(i2s_dev_rx, PM_DEVICE_ACTION_RESUME);
		zassert_ok(ret, "RX RESUME failed: %d", ret);
	}

	assert_config_intact(i2s_dev_tx, I2S_DIR_TX);
	start_both();
	exchange_one_block();
	drop_both();
}

ZTEST(i2s_pm_device_busy, test_tx_busy_until_drop)
{
	configure_streams();
	prime_tx(TX_PRIME_BLOCKS);
	start_dir(i2s_dev_tx, I2S_DIR_TX);

	zassert_true(pm_device_is_busy(i2s_dev_tx), "TX not busy while RUNNING");
	if (!same_controller()) {
		zassert_false(pm_device_is_busy(i2s_dev_rx),
			      "idle RX marked busy");
	}

	zassert_ok(i2s_trigger(i2s_dev_tx, I2S_DIR_TX, I2S_TRIGGER_DROP),
		   "TX DROP failed");
	zassert_false(pm_device_is_busy(i2s_dev_tx), "busy after TX DROP");
}

/* One running direction keeps that controller busy.
 * A shared controller stays busy after TX DROP while RX is RUNNING.
 * Separate TX and RX devices each carry their own busy flag.
 */
ZTEST(i2s_pm_device_busy, test_one_direction_keeps_controller_busy)
{
	bool tx_busy;
	bool rx_busy;

	configure_streams();
	prime_tx(TX_PRIME_BLOCKS);
	start_both();

	zassert_ok(i2s_trigger(i2s_dev_tx, I2S_DIR_TX, I2S_TRIGGER_DROP),
		   "TX DROP failed");
	tx_busy = pm_device_is_busy(i2s_dev_tx);
	rx_busy = pm_device_is_busy(i2s_dev_rx);

	zassert_ok(i2s_trigger(i2s_dev_rx, I2S_DIR_RX, I2S_TRIGGER_DROP),
		   "RX DROP failed");

	if (same_controller()) {
		zassert_true(tx_busy, "controller idle while RX still RUNNING");
	} else {
		zassert_false(tx_busy, "TX busy after its own DROP");
		zassert_true(rx_busy, "RX idle while still RUNNING");
		zassert_false(pm_device_is_busy(i2s_dev_rx),
			      "RX busy after DROP");
	}
	zassert_false(pm_device_is_busy(i2s_dev_tx),
		      "TX busy after both directions dropped");
}

/* DRAIN stays STOPPING until the queued blocks clock out. */
ZTEST(i2s_pm_device_busy, test_drain_clears_busy_after_queue_empty)
{
	int ret;

	configure_streams();
	prime_tx(TX_PRIME_BLOCKS);
	start_dir(i2s_dev_tx, I2S_DIR_TX);
	zassert_true(pm_device_is_busy(i2s_dev_tx), "TX not busy after START");

	ret = i2s_trigger(i2s_dev_tx, I2S_DIR_TX, I2S_TRIGGER_DRAIN);
	zassert_ok(ret, "DRAIN failed: %d", ret);
	zassert_true(pm_device_is_busy(i2s_dev_tx), "busy cleared while TX is STOPPING");

	k_sleep(K_MSEC(DRAIN_WAIT_MS));
	zassert_false(pm_device_is_busy(i2s_dev_tx), "busy still set after TX drained");
}

/* System suspend skips a busy device. A short SUSPEND_TO_IDLE must leave
 * the stream running and the following block transfer must succeed.
 */
ZTEST(i2s_pm_device_sleep, test_sleep_while_streaming_keeps_device_active)
{
	configure_streams();
	prime_tx(TX_PRIME_BLOCKS);
	start_both();
	zassert_true(pm_device_is_busy(i2s_dev_tx), "TX not busy before sleep");

	enter_suspend_to_idle();

	zassert_true(pm_device_is_busy(i2s_dev_tx), "busy lost across sleep");
	assert_state(i2s_dev_tx, PM_DEVICE_STATE_ACTIVE);
	if (!same_controller()) {
		zassert_true(pm_device_is_busy(i2s_dev_rx), "RX busy lost");
		assert_state(i2s_dev_rx, PM_DEVICE_STATE_ACTIVE);
	}

	exchange_one_block();
	drop_both();
}

/* Suspend-to-idle has zephyr,pm-device-disabled, so an idle I2S stays
 * ACTIVE. Standby and stop are the states that suspend it.
 */
ZTEST(i2s_pm_device_sleep, test_sleep_while_idle_then_restart)
{
	configure_streams();
	zassert_false(pm_device_is_busy(i2s_dev_tx), "busy before idle sleep");

	enter_suspend_to_idle();

	assert_state(i2s_dev_tx, PM_DEVICE_STATE_ACTIVE);
	assert_config_intact(i2s_dev_tx, I2S_DIR_TX);
	assert_config_intact(i2s_dev_rx, I2S_DIR_RX);

	prime_tx(TX_PRIME_BLOCKS);
	start_both();
	exchange_one_block();
	drop_both();
}


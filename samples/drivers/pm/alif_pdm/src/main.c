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
#include <zephyr/audio/dmic.h>
#include <zephyr/drivers/pdm/pdm_alif.h>
#include <zephyr/sys/util.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(pdm_pm, LOG_LEVEL_INF);

/* Set to 1 to dump full NVIC ISPR state on wakeup */
#define APP_PM_WAKEUP_DEBUG 0

/* ========================================================================
 * PDM Configuration
 * ========================================================================
 */

#define PDM_NODE	DT_ALIAS(pdm_audio)

/* PDM Channel configurations */
#define CHANNEL_4  4
#define CHANNEL_5  5

#define CH4_PHASE			0x0000001F
#define CH4_GAIN			0x0000000D
#define CH4_PEAK_DETECT_TH		0x00060002
#define CH4_PEAK_DETECT_ITV		0x0004002D

#define CH5_PHASE			0x00000003
#define CH5_GAIN			0x00000013
#define CH5_PEAK_DETECT_TH		0x00060002
#define CH5_PEAK_DETECT_ITV		0x00020027

#define SAMPLE_BIT_WIDTH		16
#define TIMEOUT				5000
#define NUM_CHANNELS			2

#define PCMJ_BLOCK_SIZE			30000
#define MEM_SLAB_NUM_BLOCKS		2
#define DATA_SIZE			(PCMJ_BLOCK_SIZE * MEM_SLAB_NUM_BLOCKS)

static uint8_t pcmj_data[DATA_SIZE];

K_MEM_SLAB_DEFINE(mem_slab, PCMJ_BLOCK_SIZE, MEM_SLAB_NUM_BLOCKS, 4);

struct pdm_ch_config pdm_coef_reg;

uint32_t ch4_fir[18] = {0x00000001, 0x00000003, 0x00000003, 0x000007F4,
		0x00000004, 0x000007ED, 0x000007F5, 0x000007F4, 0x000007D3,
		0x000007FE, 0x000007BC, 0x000007E5, 0x000007D9, 0x00000793,
		0x00000029, 0x0000072C, 0x00000072, 0x000002FD};

uint32_t ch5_fir[18] = {0x00000000, 0x000007FF, 0x00000000, 0x00000004,
		0x00000004, 0x000007FC, 0x00000000, 0x000007FB, 0x000007E4,
		0x00000000, 0x0000002B, 0x00000009, 0x00000016, 0x00000049,
		0x00000793, 0x000006F8, 0x00000045, 0x00000178};

#define PDM_CHANNELS   PDM_MASK_CHANNEL_4|PDM_MASK_CHANNEL_5


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
 * PDM Helper Functions
 * ========================================================================
 */

static void pdm_ch_config(const struct device *pdm_dev)
{
	pdm_set_ch_phase(pdm_dev, CHANNEL_4, CH4_PHASE);
	pdm_set_ch_gain(pdm_dev, CHANNEL_4, CH4_GAIN);
	pdm_set_peak_detect_th(pdm_dev, CHANNEL_4, CH4_PEAK_DETECT_TH);
	pdm_set_peak_detect_itv(pdm_dev, CHANNEL_4, CH4_PEAK_DETECT_ITV);

	pdm_coef_reg.ch_num = 4;
	memcpy(pdm_coef_reg.ch_fir_coef, ch4_fir, sizeof(pdm_coef_reg.ch_fir_coef));
	pdm_coef_reg.ch_iir_coef = 0x00000004;
	pdm_channel_config(pdm_dev, &pdm_coef_reg);

	pdm_set_ch_phase(pdm_dev, CHANNEL_5, CH5_PHASE);
	pdm_set_ch_gain(pdm_dev, CHANNEL_5, CH5_GAIN);
	pdm_set_peak_detect_th(pdm_dev, CHANNEL_5, CH5_PEAK_DETECT_TH);
	pdm_set_peak_detect_itv(pdm_dev, CHANNEL_5, CH5_PEAK_DETECT_ITV);

	pdm_coef_reg.ch_num = 5;
	memcpy(pdm_coef_reg.ch_fir_coef, ch5_fir, sizeof(pdm_coef_reg.ch_fir_coef));
	pdm_coef_reg.ch_iir_coef = 0x00000004;
	pdm_channel_config(pdm_dev, &pdm_coef_reg);

	pdm_mode(pdm_dev, PDM_MODE_STANDARD_VOICE_512_CLK_FRQ);
}

/**
 * Record PDM audio data into buffer
 */
static int pdm_record(const struct device *pdm_dev, uint8_t block_count)
{
	struct dmic_cfg cfg;
	struct pcm_stream_cfg stream;
	int rc;
	int k = 0;
	uint32_t data;
	void *buffer;

	memset(&cfg, 0, sizeof(cfg));
	memset(&stream, 0, sizeof(stream));

	stream.pcm_width = SAMPLE_BIT_WIDTH;
	cfg.streams = &stream;
	cfg.streams[0].mem_slab = &mem_slab;
	cfg.channel.req_num_streams = 1;
	cfg.channel.req_num_chan = NUM_CHANNELS;
	cfg.streams[0].block_size = PCMJ_BLOCK_SIZE;
	cfg.channel.req_chan_map_lo = PDM_CHANNELS;

	rc = dmic_configure(pdm_dev, &cfg);
	if (rc < 0) {
		LOG_ERR("dmic_configure failed: %d", rc);
		return rc;
	}

	pdm_ch_config(pdm_dev);

	LOG_INF("Start recording audio...");

	rc = dmic_trigger(pdm_dev, DMIC_TRIGGER_START);
	if (rc < 0) {
		LOG_ERR("dmic_trigger START failed: %d", rc);
		return rc;
	}

	k = 0;
	for (int i = 0; i < block_count; ++i) {
		rc = dmic_read(pdm_dev, 0, &buffer, &data, TIMEOUT);
		if (rc < 0) {
			LOG_ERR("dmic_read failed: %d", rc);
			dmic_trigger(pdm_dev, DMIC_TRIGGER_STOP);
			return rc;
		}

		if (k + data <= DATA_SIZE) {
			memcpy(pcmj_data + k, buffer, data);
			k += data;
		}

		k_mem_slab_free(&mem_slab, buffer);
	}

	rc = dmic_trigger(pdm_dev, DMIC_TRIGGER_STOP);
	if (rc < 0) {
		LOG_ERR("dmic_trigger STOP failed: %d", rc);
		return rc;
	}

	LOG_INF("PDM recording completed: %d bytes captured", k);

	return 0;
}

static void pdm_record_and_print(const struct device *pdm_dev, const char *phase_label)
{
	int ret;

	LOG_INF("=== %s: PDM Audio Recording ===", phase_label);
	memset(pcmj_data, 0, sizeof(pcmj_data));

	ret = pdm_record(pdm_dev, MEM_SLAB_NUM_BLOCKS);
	if (ret < 0) {
		LOG_ERR("PDM recording failed (err %d)", ret);
		return;
	}

	LOG_INF("First 40 bytes of PCM data:");
	for (int i = 0; i < 40; i += 8) {
		LOG_INF("  %02x %02x %02x %02x %02x %02x %02x %02x",
			pcmj_data[i], pcmj_data[i+1], pcmj_data[i+2], pcmj_data[i+3],
			pcmj_data[i+4], pcmj_data[i+5], pcmj_data[i+6], pcmj_data[i+7]);
	}
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
 * Main Thread — PDM + PM
 * ========================================================================
 */

int main(void)
{
	const struct device *const cons = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
	const struct device *const wakeup_dev = DEVICE_DT_GET(WAKEUP_SOURCE);
	const struct device *const pdm_dev = DEVICE_DT_GET(PDM_NODE);
	int ret;

	__ASSERT(device_is_ready(cons), "%s: device not ready", cons->name);
	__ASSERT(device_is_ready(wakeup_dev), "%s: device not ready", wakeup_dev->name);
	__ASSERT(device_is_ready(pdm_dev), "%s: device not ready", pdm_dev->name);

	pm_notifier_register(&app_pm_notifier);

	if (S2RAM_SUPPORTED) {
		LOG_INF("%s (S2RAM): PDM PM demo "
			"(RUNTIME_IDLE, SUSPEND_TO_IDLE, S2RAM STANDBY, S2RAM STOP)",
			CONFIG_BOARD);
	} else {
		LOG_INF("%s (SOFT_OFF): PDM PM demo "
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
	pdm_record_and_print(pdm_dev, "before RUNTIME_IDLE");

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
	pdm_record_and_print(pdm_dev, "after SUSPEND_TO_IDLE");
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
		pdm_record_and_print(pdm_dev, "after S2RAM STANDBY");
		pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);

		/* --- S2RAM STOP --- */
		LOG_INF("Enter PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) for (%d microseconds)",
			S2RAM_STOP_SLEEP_USEC);
		ret = app_enter_deep_sleep(S2RAM_STOP_SLEEP_USEC);
		__ASSERT(ret == 0, "Could not enter PM_STATE_SUSPEND_TO_RAM (err %d)", ret);

		LOG_INF("=== Resumed from PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) ===");
		pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
		pdm_record_and_print(pdm_dev, "after S2RAM STOP");
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

	LOG_INF("=== PDM PM SEQUENCE COMPLETED ===");

	app_pm_lock_deeper_states(true);
	/* Demo complete — prevent SUSPEND_TO_IDLE so the idle spin stays in RUNTIME_IDLE */
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);

	while (true) {
		/* spin here */
		k_sleep(K_SECONDS(1));
	}


	return 0;
}

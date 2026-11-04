/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include "alif_test_pdm_pm.h"

#include <zephyr/pm/device.h>
#include <zephyr/pm/pm.h>
#include <zephyr/pm/policy.h>
#include <zephyr/drivers/counter.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/init.h>
#include <zephyr/sys/sys_io.h>

LOG_MODULE_REGISTER(alif_test_pdm_pm, LOG_LEVEL_INF);

#define PDM_BASE             DT_REG_ADDR(PDM)
#define PDM_CONFIG_OFF       0x00U
#define PDM_CTL_OFF          0x04U
#define PDM_THRESHOLD_OFF    0x08U
#define PDM_CH_FIR_OFF       0x40U
#define PDM_CH_IIR_OFF       0xC0U
#define PDM_CH_PHASE_OFF     0xC4U
#define PDM_CH_STRIDE        0x100U
#define PDM_CLK_MODE_SHIFT   16U
#define PDM_CLK_MODE_MASK    0xFU
#define PDM_CHAN_ENABLE_MASK 0xFFU
#define PDM_DMA_HANDSHAKE    BIT(24)
#define PDM_IIR_COEF         0x00000004U
#define PDM_FIR0_MARK        0x000000A5U
#define PDM_CH_GAIN_OFF      0xC8U

#define STANDBY_SUBSTATE     0U
#define STOP_SUBSTATE        1U
#define STANDBY_SLEEP_USEC   (6U * 1000U * 1000U)
#define STOP_SLEEP_USEC      (9U * 1000U * 1000U)

#if IS_ENABLED(CONFIG_ALIF_PDM_USE_DMA) && DT_DMAS_HAS_NAME(PDM, rxdma)
#define PDM_DMA_ON 1
#else
#define PDM_DMA_ON 0
#endif

#if DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(rtc0), snps_dw_apb_rtc, okay)
#define WAKEUP_SOURCE DT_NODELABEL(rtc0)
#elif DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(timer0), snps_dw_timers, okay)
#define WAKEUP_SOURCE DT_NODELABEL(timer0)
#endif

#define MRAM_BASE_ADDRESS 0x80000000U
#define IS_BOOTING_FROM_MRAM() (SCB->VTOR >= MRAM_BASE_ADDRESS)
#define S2RAM_SUPPORTED (IS_ENABLED(CONFIG_RTSS_HE) && !IS_BOOTING_FROM_MRAM())

K_MEM_SLAB_DEFINE(pm_slab, PCMJ_BLOCK_SIZE, MEM_SLAB_NUM_BLOCKS, 4);

extern bool pm_suspend_devices(void);
extern void pm_resume_devices(void);

static struct dmic_cfg pm_cfg;
static struct pcm_stream_cfg pm_stream;

#if IS_ENABLED(CONFIG_PM_S2RAM) && IS_ENABLED(CONFIG_RTSS_HE)
static bool wake_started;
static volatile enum pm_state seen_state;
static volatile uint8_t seen_sub;
static volatile bool seen_entry;
#endif

static const uint32_t ch_phase_tbl[8] = {
	CH0_PHASE, CH1_PHASE, CH2_PHASE, CH3_PHASE,
	CH4_PHASE, CH5_PHASE, CH6_PHASE, CH7_PHASE,
};
static const uint32_t ch_gain_tbl[8] = {
	CH0_GAIN, CH1_GAIN, CH2_GAIN, CH3_GAIN,
	CH4_GAIN, CH5_GAIN, CH6_GAIN, CH7_GAIN,
};

static uint32_t expected_ctl(void)
{
	uint32_t ctl = ((uint32_t)DT_PROP(PDM, bypass_iir_filter)) << 2;

	if (PDM_DMA_ON) {
		ctl |= PDM_DMA_HANDSHAKE;
	}
	return ctl;
}

static uint32_t expected_threshold(void)
{
	if (PDM_DMA_ON) {
		return 1U;
	}
	return DT_PROP(PDM, fifo_watermark);
}

static uintptr_t ch_base(uint8_t ch)
{
	return PDM_BASE + ((uintptr_t)ch * PDM_CH_STRIDE);
}

static void assert_resume_regs(void)
{
	zassert_equal(sys_read32(PDM_BASE + PDM_CTL_OFF), expected_ctl(),
		      "PDM_CTL");
	zassert_equal(sys_read32(PDM_BASE + PDM_THRESHOLD_OFF), expected_threshold(),
		      "PDM_THRESHOLD");
}

static void program_enabled_channels(const struct device *dev)
{
	uint32_t fir[PDM_MAX_FIR_COEFFICIENT] = { PDM_FIR0_MARK };

	for (uint8_t ch = 0; ch < 8U; ch++) {
		struct pdm_ch_config coef;

		if ((PDM_CHANNELS & BIT(ch)) == 0U) {
			continue;
		}
		pdm_set_ch_phase(dev, ch, ch_phase_tbl[ch]);
		pdm_set_ch_gain(dev, ch, ch_gain_tbl[ch]);
		coef.ch_num = ch;
		memcpy(coef.ch_fir_coef, fir, sizeof(coef.ch_fir_coef));
		coef.ch_iir_coef = PDM_IIR_COEF;
		pdm_channel_config(dev, &coef);
	}
	pdm_mode(dev, PDM_MODE_HW);
}

static void assert_channels_match_programmed(void)
{
	for (uint8_t ch = 0; ch < 8U; ch++) {
		uintptr_t b;

		if ((PDM_CHANNELS & BIT(ch)) == 0U) {
			continue;
		}
		b = ch_base(ch);
		zassert_equal(sys_read32(b + PDM_CH_PHASE_OFF), ch_phase_tbl[ch],
			      "ch%u phase", ch);
		zassert_equal(sys_read32(b + PDM_CH_GAIN_OFF), ch_gain_tbl[ch],
			      "ch%u gain", ch);
		for (uint32_t i = 0; i < PDM_MAX_FIR_COEFFICIENT; i++) {
			uint32_t expect = (i == 0U) ? PDM_FIR0_MARK : 0U;

			zassert_equal(sys_read32(b + PDM_CH_FIR_OFF + (i * 4U)), expect,
				      "ch%u fir%u", ch, i);
		}
		zassert_equal(sys_read32(b + PDM_CH_IIR_OFF), PDM_IIR_COEF,
			      "ch%u iir", ch);
	}
}

static void force_active(const struct device *dev)
{
	enum pm_device_state st;

	zassert_ok(pm_device_state_get(dev, &st), "state get");
	if (st == PM_DEVICE_STATE_OFF) {
		zassert_ok(pm_device_action_run(dev, PM_DEVICE_ACTION_TURN_ON));
		st = PM_DEVICE_STATE_SUSPENDED;
	}
	if (st == PM_DEVICE_STATE_SUSPENDED) {
		zassert_ok(pm_device_action_run(dev, PM_DEVICE_ACTION_RESUME));
	}
}

static int configure_stream(const struct device *dev)
{
	memset(&pm_cfg, 0, sizeof(pm_cfg));
	memset(&pm_stream, 0, sizeof(pm_stream));
	pm_stream.pcm_width = SAMPLE_BIT_WIDTH;
	pm_cfg.streams = &pm_stream;
	pm_cfg.streams[0].mem_slab = &pm_slab;
	pm_cfg.streams[0].block_size = PCMJ_BLOCK_SIZE;
	pm_cfg.channel.req_num_streams = 1;
	pm_cfg.channel.req_num_chan = NUM_CHANNELS;
	pm_cfg.channel.req_chan_map_lo = PDM_CHANNELS;
	return dmic_configure(dev, &pm_cfg);
}

#if IS_ENABLED(CONFIG_PM_S2RAM) && IS_ENABLED(CONFIG_RTSS_HE)

static bool buffer_has_audio(const void *buffer, size_t size)
{
	const uint8_t *p = buffer;

	for (size_t i = 0; i < size; i++) {
		if (p[i] != 0U) {
			return true;
		}
	}
	return false;
}

/* require_audio: the first block must contain a non-zero sample. */
static int record_blocks(const struct device *dev, uint8_t block_count, bool require_audio)
{
	int rc;

	program_enabled_channels(dev);
	rc = dmic_trigger(dev, DMIC_TRIGGER_START);
	if (rc) {
		return rc;
	}
	for (uint8_t i = 0; i < block_count; i++) {
		void *buffer = NULL;
		size_t size = 0;

		bool audio_ok = true;

		rc = dmic_read(dev, 0, &buffer, &size, TIMEOUT);
		if (rc == 0 && size == PCMJ_BLOCK_SIZE && require_audio && i == 0U) {
			audio_ok = buffer != NULL && buffer_has_audio(buffer, size);
		}
		if (buffer != NULL) {
			k_mem_slab_free(&pm_slab, buffer);
		}
		if (rc || size != PCMJ_BLOCK_SIZE) {
			(void)dmic_trigger(dev, DMIC_TRIGGER_STOP);
			return rc ? rc : -EIO;
		}
		if (!audio_ok) {
			(void)dmic_trigger(dev, DMIC_TRIGGER_STOP);
			zassert_true(false, "capture is all zeros");
		}
	}
	return dmic_trigger(dev, DMIC_TRIGGER_STOP);
}
#endif

static void pm_notify_entry(enum pm_state state)
{
#if IS_ENABLED(CONFIG_PM_S2RAM) && IS_ENABLED(CONFIG_RTSS_HE)
	const struct pm_state_info *info = pm_state_next_get(0);

	seen_state = state;
	seen_sub = info ? info->substate_id : 0U;
	seen_entry = true;
#else
	ARG_UNUSED(state);
#endif
}

static struct pm_notifier pm_note = {
	.state_entry = pm_notify_entry,
};

static int lock_deep_states(void)
{
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_RAM, PM_ALL_SUBSTATES);
	pm_policy_state_lock_get(PM_STATE_SOFT_OFF, PM_ALL_SUBSTATES);
	return 0;
}
SYS_INIT(lock_deep_states, PRE_KERNEL_2, 0);

#if IS_ENABLED(CONFIG_PM_S2RAM) && IS_ENABLED(CONFIG_RTSS_HE)

#if defined(WAKEUP_SOURCE) && !defined(CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER)
static void alarm_cb(const struct device *dev, uint8_t chan, uint32_t ticks, void *user)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(chan);
	ARG_UNUSED(ticks);
	ARG_UNUSED(user);
}
#endif

static int start_wakeup_counter(void)
{
#if defined(WAKEUP_SOURCE)
	const struct device *wake = DEVICE_DT_GET(WAKEUP_SOURCE);
	int rc;

	if (!device_is_ready(wake)) {
		return -ENODEV;
	}
	if (!wake_started) {
		/* rtc0 is zephyr,cortex-m-idle-timer. The systick low-power hook
		 * owns its only alarm channel. counter_start() makes sure the
		 * counter is running; -EALREADY means the hook already started it.
		 */
		rc = counter_start(wake);
		if (rc && rc != -EALREADY) {
			return rc;
		}
		wake_started = true;
	}
	return 0;
#else
	return -ENOTSUP;
#endif
}

/* UART suspend returns -EBUSY while the RX FIFO holds a byte, and that aborts standby. */
static void drain_console_rx(void)
{
#if DT_HAS_CHOSEN(zephyr_console)
	const struct device *uart = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
	unsigned char byte;

	if (!device_is_ready(uart)) {
		return;
	}
	while (uart_poll_in(uart, &byte) == 0) {
	}
#endif
}

static int enter_deep_sleep(uint32_t sleep_usec)
{
#if !defined(CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER)
#if !defined(WAKEUP_SOURCE)
	return -ENOTSUP;
#else
	const struct device *wake = DEVICE_DT_GET(WAKEUP_SOURCE);
	struct counter_alarm_cfg alarm = { 0 };
	int rc;

	alarm.ticks = counter_us_to_ticks(wake, sleep_usec);
	alarm.callback = alarm_cb;
	rc = counter_set_channel_alarm(wake, 0, &alarm);
	if (rc) {
		return rc;
	}
#endif
#endif
	drain_console_rx();
	seen_entry = false;
	k_sleep(K_USEC(sleep_usec));
	if (!seen_entry) {
		return -EAGAIN;
	}
	return 0;
}

/*
 * Sample deep-sleep entry. All S2RAM substates are unlocked and
 * suspend-to-idle stays unlocked across k_sleep. Idle is locked again
 * before the caller touches PDM. S2RAM stays unlocked until
 * sample_sleep_done(), which is how the sample records after wake.
 */
static int sample_sleep(uint32_t sleep_usec)
{
	int rc;

	pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_RAM, PM_ALL_SUBSTATES);
	pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
	rc = enter_deep_sleep(sleep_usec);
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
	return rc;
}

static void sample_sleep_done(void)
{
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_RAM, PM_ALL_SUBSTATES);
}

/* Stop powers the PL330 domain off. Resume it before the next DMA start. */
static int resume_capture_dma(void)
{
#if PDM_DMA_ON && DT_NODE_HAS_PROP(DT_DMAS_CTLR_BY_NAME(PDM, rxdma), dma_controller)
	const struct device *dma = DEVICE_DT_GET(DT_PHANDLE(
		DT_DMAS_CTLR_BY_NAME(PDM, rxdma), dma_controller));
	enum pm_device_state st;
	int rc;

	if (!device_is_ready(dma)) {
		return -ENODEV;
	}
	rc = pm_device_state_get(dma, &st);
	if (rc) {
		return rc;
	}
	if (st == PM_DEVICE_STATE_OFF) {
		rc = pm_device_action_run(dma, PM_DEVICE_ACTION_TURN_ON);
		if (rc) {
			return rc;
		}
		st = PM_DEVICE_STATE_SUSPENDED;
	}
	if (st != PM_DEVICE_STATE_SUSPENDED) {
		rc = pm_device_action_run(dma, PM_DEVICE_ACTION_SUSPEND);
		if (rc) {
			return rc;
		}
	}
	return pm_device_action_run(dma, PM_DEVICE_ACTION_RESUME);
#else
	return 0;
#endif
}

/*
 * Standby, then a capture, then stop. Suspend-to-RAM is locked again
 * during that capture so the idle thread cannot re-enter standby while
 * dmic_read() blocks. It is unlocked only for the stop sleep.
 * sample_sleep_done() takes the suspend-to-RAM lock once on the way out.
 */
static int sample_stop_after_standby(const struct device *dev)
{
	int rc;

	rc = sample_sleep(STANDBY_SLEEP_USEC);
	if (rc) {
		return rc;
	}
	if (seen_state != PM_STATE_SUSPEND_TO_RAM || seen_sub != STANDBY_SUBSTATE) {
		return -EIO;
	}

	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_RAM, PM_ALL_SUBSTATES);
	rc = resume_capture_dma();
	if (rc == 0) {
		rc = record_blocks(dev, MEM_SLAB_NUM_BLOCKS, true);
	}
	pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_RAM, PM_ALL_SUBSTATES);
	if (rc) {
		return rc;
	}

	pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
	rc = enter_deep_sleep(STOP_SLEEP_USEC);
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
	if (rc == 0) {
		rc = resume_capture_dma();
	}
	return rc;
}

static void require_s2ram(void)
{
	zassert_true(S2RAM_SUPPORTED, "s2ram image booted from MRAM");
}

#endif /* CONFIG_PM_S2RAM && CONFIG_RTSS_HE */

static void pm_before(void *fixture)
{
	const struct device *dev = DEVICE_DT_GET(PDM);

	ARG_UNUSED(fixture);
	zassert_true(device_is_ready(dev), "pdm not ready");
	force_active(dev);
	(void)dmic_trigger(dev, DMIC_TRIGGER_STOP);
	zassert_equal(k_mem_slab_num_used_get(&pm_slab), 0U, "slab before");
}

static void pm_after(void *fixture)
{
	const struct device *dev = DEVICE_DT_GET(PDM);

	ARG_UNUSED(fixture);
	force_active(dev);
	(void)dmic_trigger(dev, DMIC_TRIGGER_STOP);
	zassert_equal(k_mem_slab_num_used_get(&pm_slab), 0U, "slab after");
}

static void *pm_setup(void)
{
	static bool registered;

	/* Register the notifier once. Appending &pm_note twice makes the
	 * node point at itself, and the first suspend never returns.
	 */
	if (!registered) {
		pm_notifier_register(&pm_note);
		registered = true;
	}
	return NULL;
}

#define PDMTEST_CAT_(a, b) a##b
#define PDMTEST_CAT(a, b) PDMTEST_CAT_(a, b)

/* Expand PDMTEST before ZTEST token-pastes the suite and case names. */
#define PDMTEST_ZTEST_(suite, name) ZTEST(suite, name)
#define PDMTEST_ZTEST(group, name) \
	PDMTEST_ZTEST_(PDMTEST_CAT(PDMTEST, group), PDMTEST_CAT(PDMTEST, name))

#define PDMTEST_SUITE_(suite) \
	ZTEST_SUITE(suite, NULL, pm_setup, pm_before, pm_after, NULL)
#define PDMTEST_SUITE(group) PDMTEST_SUITE_(PDMTEST_CAT(PDMTEST, group))

PDMTEST_ZTEST(_pm, _suspend_accepted_when_idle)
{
	const struct device *dev = DEVICE_DT_GET(PDM);
	enum pm_device_state st;

	zassert_ok(pm_device_action_run(dev, PM_DEVICE_ACTION_SUSPEND));
	zassert_ok(pm_device_state_get(dev, &st));
	zassert_equal(st, PM_DEVICE_STATE_SUSPENDED, "state");
}

PDMTEST_ZTEST(_pm, _resume_restores_ctl_and_threshold)
{
	const struct device *dev = DEVICE_DT_GET(PDM);

	zassert_ok(pm_device_action_run(dev, PM_DEVICE_ACTION_SUSPEND));
	zassert_ok(pm_device_action_run(dev, PM_DEVICE_ACTION_RESUME));
	assert_resume_regs();
}

PDMTEST_ZTEST(_pm, _resume_keeps_channel_regs)
{
	const struct device *dev = DEVICE_DT_GET(PDM);

	program_enabled_channels(dev);
	zassert_ok(pm_device_action_run(dev, PM_DEVICE_ACTION_SUSPEND));
	zassert_ok(pm_device_action_run(dev, PM_DEVICE_ACTION_RESUME));
	assert_channels_match_programmed();
	assert_resume_regs();
}

PDMTEST_ZTEST(_pm, _busy_skips_suspend)
{
	const struct device *dev = DEVICE_DT_GET(PDM);
	enum pm_device_state st;

	zassert_ok(configure_stream(dev));
	program_enabled_channels(dev);
	bool suspended;

	zassert_ok(dmic_trigger(dev, DMIC_TRIGGER_START));
	zassert_true(pm_device_is_busy(dev), "busy after START");

	/*
	 * PDM is busy, so the PM core skips it. PL330 clears its channel
	 * before the PDM callback starts the next block, so system suspend
	 * may still complete. Resume whatever was suspended before asserting,
	 * or a later case touches a powered-down block.
	 */
	suspended = pm_suspend_devices();
	pm_resume_devices();

	zassert_ok(pm_device_state_get(dev, &st));
	zassert_equal(st, PM_DEVICE_STATE_ACTIVE, "busy PDM was suspended");
	if (!PDM_DMA_ON) {
		zassert_true(suspended, "system suspend");
	} else {
		ARG_UNUSED(suspended);
	}
	zassert_ok(dmic_trigger(dev, DMIC_TRIGGER_STOP));
}

PDMTEST_ZTEST(_pm, _stop_clears_busy_and_allows_suspend)
{
	const struct device *dev = DEVICE_DT_GET(PDM);
	enum pm_device_state st;

	zassert_ok(configure_stream(dev));
	zassert_ok(dmic_trigger(dev, DMIC_TRIGGER_START));
	zassert_ok(dmic_trigger(dev, DMIC_TRIGGER_STOP));
	zassert_false(pm_device_is_busy(dev), "busy after STOP");

	zassert_ok(pm_device_action_run(dev, PM_DEVICE_ACTION_SUSPEND));
	zassert_ok(pm_device_state_get(dev, &st));
	zassert_equal(st, PM_DEVICE_STATE_SUSPENDED, "state");
}

PDMTEST_ZTEST(_pm, _suspend_while_started_holds_slab)
{
	const struct device *dev = DEVICE_DT_GET(PDM);
	unsigned int used;

	zassert_ok(configure_stream(dev));
	program_enabled_channels(dev);
	zassert_ok(dmic_trigger(dev, DMIC_TRIGGER_START));
	used = k_mem_slab_num_used_get(&pm_slab);

	zassert_ok(pm_device_action_run(dev, PM_DEVICE_ACTION_SUSPEND));
	zassert_true(pm_device_is_busy(dev), "suspend cleared busy");
	if (PDM_DMA_ON) {
		zassert_equal(k_mem_slab_num_used_get(&pm_slab), used,
			      "suspend freed the DMA block");
	}

	zassert_ok(pm_device_action_run(dev, PM_DEVICE_ACTION_RESUME));
	zassert_true(pm_device_is_busy(dev), "resume cleared busy");
	zassert_ok(dmic_trigger(dev, DMIC_TRIGGER_STOP));
	zassert_false(pm_device_is_busy(dev), "STOP");
	zassert_equal(k_mem_slab_num_used_get(&pm_slab), 0U, "slab after STOP");
}

/* TURN_ON and TURN_OFF in pdm_pm_action() return 0 and write no
 * registers. This locks the PM core path SUSPENDED → OFF →
 * SUSPENDED → ACTIVE.
 */

PDMTEST_ZTEST(_pm, _turn_on_and_turn_off_return_zero)
{
	const struct device *dev = DEVICE_DT_GET(PDM);
	enum pm_device_state st;

	zassert_ok(pm_device_action_run(dev, PM_DEVICE_ACTION_SUSPEND));
	zassert_ok(pm_device_action_run(dev, PM_DEVICE_ACTION_TURN_OFF));
	zassert_ok(pm_device_state_get(dev, &st));
	zassert_equal(st, PM_DEVICE_STATE_OFF, "off");

	zassert_ok(pm_device_action_run(dev, PM_DEVICE_ACTION_TURN_ON));
	zassert_ok(pm_device_state_get(dev, &st));
	zassert_equal(st, PM_DEVICE_STATE_SUSPENDED, "turn on");
	zassert_ok(pm_device_action_run(dev, PM_DEVICE_ACTION_RESUME));
}

#if IS_ENABLED(CONFIG_PM_S2RAM) && IS_ENABLED(CONFIG_RTSS_HE)

static void assert_reg(uint32_t got, uint32_t expect, const char *what)
{
	zassert_equal(got, expect, "%s got 0x%x", what, got);
}

/* retained: standby kept the stream context. Otherwise stop cleared it. */
static void assert_stream_context(bool retained)
{
	uint32_t cfg = sys_read32(PDM_BASE + PDM_CONFIG_OFF);
	uint32_t mode = (cfg >> PDM_CLK_MODE_SHIFT) & PDM_CLK_MODE_MASK;

	zassert_equal(cfg & PDM_CHAN_ENABLE_MASK, 0U, "channel enable");
	assert_reg(mode, retained ? (uint32_t)PDM_MODE_HW : 0U, "clock mode");

	for (uint8_t ch = 0; ch < 8U; ch++) {
		uintptr_t b;

		if ((PDM_CHANNELS & BIT(ch)) == 0U) {
			continue;
		}
		b = ch_base(ch);
		assert_reg(sys_read32(b + PDM_CH_PHASE_OFF),
			   retained ? ch_phase_tbl[ch] : 0U, "phase");
		assert_reg(sys_read32(b + PDM_CH_GAIN_OFF),
			   retained ? ch_gain_tbl[ch] : 0U, "gain");
		assert_reg(sys_read32(b + PDM_CH_IIR_OFF),
			   retained ? PDM_IIR_COEF : 0U, "iir");
		for (uint32_t i = 0; i < PDM_MAX_FIR_COEFFICIENT; i++) {
			uint32_t expect = (retained && i == 0U) ? PDM_FIR0_MARK : 0U;

			assert_reg(sys_read32(b + PDM_CH_FIR_OFF + (i * 4U)), expect, "fir");
		}
	}
}

PDMTEST_ZTEST(_pm, _standby_resume_writes_ctl_and_threshold)
{
	const struct device *dev = DEVICE_DT_GET(PDM);
	int rc;

	require_s2ram();
	zassert_ok(start_wakeup_counter());
	zassert_ok(configure_stream(dev));
	zassert_ok(record_blocks(dev, MEM_SLAB_NUM_BLOCKS, false), "record before standby");

	rc = sample_sleep(STANDBY_SLEEP_USEC);
	sample_sleep_done();
	zassert_ok(rc, "standby");
	zassert_equal(seen_state, PM_STATE_SUSPEND_TO_RAM, "state");
	zassert_equal(seen_sub, STANDBY_SUBSTATE, "substate");
	assert_resume_regs();
	assert_stream_context(PDM_STANDBY_KEEPS_CONTEXT);
	zassert_ok(record_blocks(dev, 1U, true), "audio after standby");
}

/* Stop clears the stream context. The capture below programs it again. */
PDMTEST_ZTEST(_pm, _capture_after_stop_context_cleared)
{
	const struct device *dev = DEVICE_DT_GET(PDM);
	int rc;

	require_s2ram();
	zassert_ok(start_wakeup_counter());
	zassert_ok(configure_stream(dev));
	zassert_ok(record_blocks(dev, MEM_SLAB_NUM_BLOCKS, false), "record before stop");

	rc = sample_stop_after_standby(dev);
	sample_sleep_done();
	zassert_ok(rc, "stop");
	zassert_equal(seen_state, PM_STATE_SUSPEND_TO_RAM, "state");
	zassert_equal(seen_sub, STOP_SUBSTATE, "substate");
	assert_resume_regs();
	assert_stream_context(false);
	zassert_ok(record_blocks(dev, 1U, true), "audio after stop");
}

#endif

PDMTEST_SUITE(_pm);

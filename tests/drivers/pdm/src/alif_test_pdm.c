/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */
#include "alif_test_pdm.h"
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/ztest.h>
#include <errno.h>
#include <string.h>

LOG_MODULE_REGISTER(alif_test_pdm, LOG_LEVEL_INF);

#define DEFAULT_IIR_COEF   0x00000004U
#define PRINT_BYTES        80U

static struct pdm_ch_config pdm_coef_reg;

/*
 * Even channels (0,2,5,7) share one set; odd (1,3,4,6) share the other.
 */
static const uint32_t fir_even[PDM_MAX_FIR_COEFFICIENT] = {
	0x00000000, 0x000007FF, 0x00000000, 0x00000004,
	0x00000004, 0x000007FC, 0x00000000, 0x000007FB,
	0x000007E4,
	0x00000000, 0x0000002B, 0x00000009, 0x00000016,
	0x00000049, 0x00000793, 0x000006F8, 0x00000045,
	0x00000178,
};
static const uint32_t fir_odd[PDM_MAX_FIR_COEFFICIENT] = {
	0x00000001, 0x00000003, 0x00000003, 0x000007F4,
	0x00000004, 0x000007ED, 0x000007F5, 0x000007F4,
	0x000007D3,
	0x000007FE, 0x000007BC, 0x000007E5, 0x000007D9,
	0x00000793, 0x00000029, 0x0000072C, 0x00000072,
	0x000002FD,
};

static const uint32_t * const fir_table[8] = {
	fir_even, fir_odd, fir_even, fir_odd,
	fir_odd, fir_even, fir_odd, fir_even
};

/* Build enabled channel array via preprocessor */
static const int enabled_channels[] = {
	#if IS_ENABLED(CONFIG_TEST_PDM_CH0)
		0,
	#endif
	#if IS_ENABLED(CONFIG_TEST_PDM_CH1)
		1,
	#endif
	#if IS_ENABLED(CONFIG_TEST_PDM_CH2)
		2,
	#endif
	#if IS_ENABLED(CONFIG_TEST_PDM_CH3)
		3,
	#endif
	#if IS_ENABLED(CONFIG_TEST_PDM_CH4)
		4,
	#endif
	#if IS_ENABLED(CONFIG_TEST_PDM_CH5)
		5,
	#endif
	#if IS_ENABLED(CONFIG_TEST_PDM_CH6)
		6,
	#endif
	#if IS_ENABLED(CONFIG_TEST_PDM_CH7)
		7,
	#endif
};

BUILD_ASSERT(ARRAY_SIZE(enabled_channels) == NUM_CHANNELS,
	     "enabled_channels[] disagrees with NUM_CHANNELS");
BUILD_ASSERT(NUM_CHANNELS > 0, "Enable at least one CONFIG_TEST_PDM_CHx");

/* Enforce that the enabled channel count matches the selected group */
#if IS_ENABLED(CONFIG_TEST_PDM_MONO_CH)
BUILD_ASSERT(NUM_CHANNELS == 1, "Mono group requires exactly 1 CONFIG_TEST_PDM_CHx");
#elif IS_ENABLED(CONFIG_TEST_PDM_STEREO_CH)
BUILD_ASSERT(NUM_CHANNELS == 2, "Stereo group requires exactly 2 CONFIG_TEST_PDM_CHx");
#elif IS_ENABLED(CONFIG_TEST_PDM_MULTI_CH)
BUILD_ASSERT(NUM_CHANNELS >= 3 && NUM_CHANNELS <= 8,
	     "Multi group requires 3..8 CONFIG_TEST_PDM_CHx");
#endif

static const struct pdm_ch_hw_params ch_hw_params[8] = {
	{ CH0_PHASE, CH0_GAIN, CH0_PEAK_DETECT_TH, CH0_PEAK_DETECT_ITV },
	{ CH1_PHASE, CH1_GAIN, CH1_PEAK_DETECT_TH, CH1_PEAK_DETECT_ITV },
	{ CH2_PHASE, CH2_GAIN, CH2_PEAK_DETECT_TH, CH2_PEAK_DETECT_ITV },
	{ CH3_PHASE, CH3_GAIN, CH3_PEAK_DETECT_TH, CH3_PEAK_DETECT_ITV },
	{ CH4_PHASE, CH4_GAIN, CH4_PEAK_DETECT_TH, CH4_PEAK_DETECT_ITV },
	{ CH5_PHASE, CH5_GAIN, CH5_PEAK_DETECT_TH, CH5_PEAK_DETECT_ITV },
	{ CH6_PHASE, CH6_GAIN, CH6_PEAK_DETECT_TH, CH6_PEAK_DETECT_ITV },
	{ CH7_PHASE, CH7_GAIN, CH7_PEAK_DETECT_TH, CH7_PEAK_DETECT_ITV },
};

/* Helper to set channel parameters (phase, gain, peak thresholds).
 */
static int set_channel_hw_params(const struct device *dev, uint8_t ch)
{
	const struct pdm_ch_hw_params *p;

	if (ch >= ARRAY_SIZE(ch_hw_params)) {
		LOG_ERR("Unsupported channel %d in %s", ch, __func__);
		return -EINVAL;
	}
	p = &ch_hw_params[ch];

	pdm_set_ch_phase(dev, ch, p->phase);
	pdm_set_ch_gain(dev, ch, p->gain);
	pdm_set_peak_detect_th(dev, ch, p->peak_th);
	pdm_set_peak_detect_itv(dev, ch, p->peak_itv);

	return 0;
}

/* Configure a single channel: set HW params, copy FIR/IIR, call pdm_channel_config */
static int pdm_configure_channel(const struct device *dev, uint8_t ch,
		const uint32_t *fir, uint32_t iir)
{
	if (!dev) {
		LOG_ERR("%s: device is NULL", __func__);
		return -EINVAL;
	}

	int rc = set_channel_hw_params(dev, ch);

	if (rc) {
		return rc;
	}

	pdm_coef_reg.ch_num = ch;
	memcpy(pdm_coef_reg.ch_fir_coef, fir, sizeof(pdm_coef_reg.ch_fir_coef));
	pdm_coef_reg.ch_iir_coef = iir;
	pdm_channel_config(dev, &pdm_coef_reg);

	LOG_DBG("Configured PDM channel %u", ch);
	return 0;
}

/* Configure channels depending on mono/stereo/multi selection */
static int pdm_configure_selected_channels(const struct device *dev)
{
	size_t idx;
	const size_t num_enabled = ARRAY_SIZE(enabled_channels);

	if (num_enabled == 0) {
		LOG_ERR("No CONFIG_TEST_PDM_CHx enabled; nothing to configure");
		return -EINVAL;
	}

	for (idx = 0; idx < num_enabled; ++idx) {
		int ch = enabled_channels[idx];

		if (ch < 0 || ch > 7) {
			LOG_ERR("Invalid channel entry: %d", ch);
			return -EINVAL;
		}
		int rc = pdm_configure_channel(dev, (uint8_t)ch, fir_table[ch], DEFAULT_IIR_COEF);

		if (rc) {
			LOG_ERR("Failed to configure channel %d: %d", ch, rc);
			return rc;
		}
	}
	return 0;
}

/* PDM mode selection helper -- picks the first enabled mode from compile-time macros */
static void pdm_select_mode(const struct device *dev)
{
#if IS_ENABLED(CONFIG_TEST_PDM_MODE9)
	pdm_mode(dev, PDM_MODE_ULTRASOUND_96_SAMPLING_RATE);
	LOG_INF("Configured MODE9: PDM_MODE_ULTRASOUND_96_SAMPLING_RATE");
#elif IS_ENABLED(CONFIG_TEST_PDM_MODE8)
	pdm_mode(dev, PDM_MODE_ULTRASOUND_4800_CLOCK_FRQ);
	LOG_INF("Configured MODE8: PDM_MODE_ULTRASOUND_4800_CLOCK_FRQ");
#elif IS_ENABLED(CONFIG_TEST_PDM_MODE7)
	pdm_mode(dev, PDM_MODE_FULL_BANDWIDTH_AUDIO_3071_CLK_FRQ);
	LOG_INF("Configured MODE7: PDM_MODE_FULL_BANDWIDTH_AUDIO_3071_CLK_FRQ");
#elif IS_ENABLED(CONFIG_TEST_PDM_MODE6)
	pdm_mode(dev, PDM_MODE_FULL_BANDWIDTH_AUDIO_2400_CLK_FRQ);
	LOG_INF("Configured MODE6: PDM_MODE_FULL_BANDWIDTH_AUDIO_2400_CLK_FRQ");
#elif IS_ENABLED(CONFIG_TEST_PDM_MODE5)
	pdm_mode(dev, PDM_MODE_WIDE_BANDWIDTH_AUDIO_1536_CLK_FRQ);
	LOG_INF("Configured MODE5: PDM_MODE_WIDE_BANDWIDTH_AUDIO_1536_CLK_FRQ");
#elif IS_ENABLED(CONFIG_TEST_PDM_MODE4)
	pdm_mode(dev, PDM_MODE_HIGH_QUALITY_1024_CLK_FRQ);
	LOG_INF("Configured MODE4: PDM_MODE_HIGH_QUALITY_1024_CLK_FRQ");
#elif IS_ENABLED(CONFIG_TEST_PDM_MODE3)
	pdm_mode(dev, PDM_MODE_HIGH_QUALITY_768_CLK_FRQ);
	LOG_INF("Configured MODE3: PDM_MODE_HIGH_QUALITY_768_CLK_FRQ");
#elif IS_ENABLED(CONFIG_TEST_PDM_MODE2)
	pdm_mode(dev, PDM_MODE_HIGH_QUALITY_512_CLK_FRQ);
	LOG_INF("Configured MODE2: PDM_MODE_HIGH_QUALITY_512_CLK_FRQ");
#elif IS_ENABLED(CONFIG_TEST_PDM_MODE1)
	pdm_mode(dev, PDM_MODE_STANDARD_VOICE_512_CLK_FRQ);
	LOG_INF("Configured MODE1: PDM_MODE_STANDARD_VOICE_512_CLK_FRQ");
#endif
}

/* Configure the selected channels and PDM clock mode */
static int pdm_setup_channels_and_mode(void)
{
	const struct device *dev = DEVICE_DT_GET(PDM);
	int rc;

	rc = pdm_configure_selected_channels(dev);
	if (rc) {
		LOG_ERR("%s: channel configuration failed: %d", __func__, rc);
		return rc;
	}

	pdm_select_mode(dev);
	return 0;
}

/* Memory slab for the PDM capture test */
K_MEM_SLAB_DEFINE(mem_slab, PCMJ_BLOCK_SIZE, MEM_SLAB_NUM_BLOCKS, 4);

BUILD_ASSERT(SAMPLE_BIT_WIDTH == 16, "stats assume 16-bit PCM");
BUILD_ASSERT((DATA_SIZE % sizeof(int16_t)) == 0, "DATA_SIZE not 16-bit aligned");

static int16_t pcmj_data[DATA_SIZE / sizeof(int16_t)];

/* Number of valid bytes stored in pcmj_data after the last capture */
static size_t captured_bytes;

static void set_config(struct dmic_cfg *cfg, struct pcm_stream_cfg *stream)
{
	uint32_t channel_map = 0;

	stream->pcm_width = SAMPLE_BIT_WIDTH;
	cfg->streams = stream;
	cfg->streams[0].mem_slab = &mem_slab;
	cfg->channel.req_num_streams = 1;
	cfg->channel.req_num_chan = NUM_CHANNELS;
	cfg->streams[0].block_size = PCMJ_BLOCK_SIZE;

	channel_map = PDM_CHANNELS;

	cfg->channel.req_chan_map_lo = channel_map;

	LOG_INF("memslab: %p", cfg->streams[0].mem_slab);
	LOG_INF("channel_map 0x%x block size: 0x%x", channel_map, PCMJ_BLOCK_SIZE);
}

/* Core capture routine */
static int config_channel(const struct device *dmic_dev,
			  struct dmic_cfg *cfg, uint8_t block_count)
{
	int rc;
	uint8_t i;
	void *buffer = NULL;
	size_t data = 0;
	size_t k = 0;
	const size_t expected = (size_t)block_count * PCMJ_BLOCK_SIZE;

	zassert_not_null(dmic_dev, "%s: dmic_dev is NULL", __func__);
	zassert_not_null(cfg, "%s: cfg is NULL", __func__);
	zassert_true(expected <= DATA_SIZE,
		     "capture %zu bytes exceeds pcmj_data %d", expected, DATA_SIZE);

	rc = dmic_configure(dmic_dev, cfg);
	if (rc) {
		LOG_ERR("dmic_configure failed: %d", rc);
		return rc;
	}

	rc = pdm_setup_channels_and_mode();
	if (rc) {
		return rc;
	}

	LOG_INF("Start Speaking or Play some Audio!");

	rc = dmic_trigger(dmic_dev, DMIC_TRIGGER_START);
	if (rc) {
		LOG_ERR("dmic_trigger START failed: %d", rc);
		return rc;
	}

	/*
	 * Discard the first block: FIR/IIR needs time to settle.
	 * If this read fails, later blocks are not a valid capture.
	 */
	rc = dmic_read(dmic_dev, 0, &buffer, &data, TIMEOUT);
	if (rc || buffer == NULL || data == 0U) {
		LOG_ERR("Settling block read failed: rc=%d data=%zu", rc, data);
		if (buffer != NULL) {
			k_mem_slab_free(&mem_slab, buffer);
		}
		(void)dmic_trigger(dmic_dev, DMIC_TRIGGER_STOP);
		return rc ? rc : -EIO;
	}
	k_mem_slab_free(&mem_slab, buffer);
	LOG_INF("Discarded settling block (%zu bytes)", data);

	for (i = 0; i < block_count; ++i) {
		rc = dmic_read(dmic_dev, 0, &buffer, &data, TIMEOUT);
		if (rc < 0 || buffer == NULL || data == 0U) {
			LOG_ERR("dmic_read block %u failed: rc=%d data=%zu",
				i, rc, data);
			if (buffer != NULL) {
				k_mem_slab_free(&mem_slab, buffer);
			}
			(void)dmic_trigger(dmic_dev, DMIC_TRIGGER_STOP);
			return rc ? rc : -EIO;
		}

		if (data != PCMJ_BLOCK_SIZE) {
			LOG_ERR("block %u size %zu != PCMJ_BLOCK_SIZE %d",
				i, data, PCMJ_BLOCK_SIZE);
			k_mem_slab_free(&mem_slab, buffer);
			(void)dmic_trigger(dmic_dev, DMIC_TRIGGER_STOP);
			return -EIO;
		}

		if ((k + data) > DATA_SIZE) {
			LOG_ERR("PCM buffer overflow: have %zu need %zu cap %d",
				k, data, DATA_SIZE);
			k_mem_slab_free(&mem_slab, buffer);
			(void)dmic_trigger(dmic_dev, DMIC_TRIGGER_STOP);
			return -ENOSPC;
		}

		memcpy((uint8_t *)pcmj_data + k, buffer, data);
		k += data;
		k_mem_slab_free(&mem_slab, buffer);
	}

	captured_bytes = k;

	LOG_INF("Stop recording");
	LOG_INF("PCM samples stored at %p captured_bytes: %zu",
		(void *)pcmj_data, captured_bytes);

	rc = dmic_trigger(dmic_dev, DMIC_TRIGGER_STOP);
	if (rc) {
		LOG_ERR("dmic_trigger STOP failed: %d", rc);
		return rc;
	}

	return 0;
}

/*
 * Informational signal statistics over the captured (post-settling) PCM data.
 * This NEVER fails the test: capture in true silence is valid. It helps verify
 * that real-audio playback was recorded when inspecting the logs.
 */
static void pdm_log_signal_stats(void)
{
	const int16_t *s = pcmj_data;
	size_t n = captured_bytes / sizeof(int16_t);
	int16_t mn = INT16_MAX;
	int16_t mx = INT16_MIN;
	size_t nonzero = 0;
	long first_nz = -1;

	if (n == 0) {
		LOG_INF("Signal stats: no samples captured");
		return;
	}

	for (size_t i = 0; i < n; ++i) {
		int16_t v = s[i];

		if (v < mn) {
			mn = v;
		}
		if (v > mx) {
			mx = v;
		}
		if (v != 0) {
			nonzero++;
			if (first_nz < 0) {
				first_nz = (long)i;
			}
		}
	}

	LOG_INF("Signal stats: samples=%zu nonzero=%zu min=%d max=%d first_nonzero=%ld",
		n, nonzero, mn, mx, first_nz);
}

/* Print first bytes of captured PCM data for debugging */
static void print_data(void)
{
	size_t i;
	size_t lim = MIN((size_t)PRINT_BYTES, captured_bytes);
	const uint8_t *p = (const uint8_t *)pcmj_data;

	LOG_INF("pcm data pointer: %p captured_bytes: %zu",
		(void *)pcmj_data, captured_bytes);

	for (i = 0; i + 7 < lim; i += 8) {
		LOG_INF(" %02x %02x %02x %02x %02x %02x %02x %02x",
			p[i], p[i + 1], p[i + 2], p[i + 3],
			p[i + 4], p[i + 5], p[i + 6], p[i + 7]);
	}
}

static int test_channel(void)
{
	const struct device *pcmj_device;
	struct dmic_cfg cfg = {0};
	struct pcm_stream_cfg stream = {0};
	int rc;

	LOG_INF("PDM test channel");

	pcmj_device = DEVICE_DT_GET(PDM);

	zassert_true(device_is_ready(pcmj_device), "device not ready");

	set_config(&cfg, &stream);

	rc = config_channel(pcmj_device, &cfg, MEM_SLAB_NUM_BLOCKS);
	if (rc) {
		LOG_ERR("config_channel returned %d", rc);
		return rc;
	}

	if (captured_bytes == 0U) {
		LOG_ERR("no PCM captured");
		return -EIO;
	}

	if (captured_bytes != ((size_t)MEM_SLAB_NUM_BLOCKS * PCMJ_BLOCK_SIZE)) {
		LOG_ERR("short capture: got %zu expected %zu",
			captured_bytes,
			(size_t)MEM_SLAB_NUM_BLOCKS * PCMJ_BLOCK_SIZE);
		return -EIO;
	}

	print_data();
	pdm_log_signal_stats();

	return 0;
}

static void pdm_capture_after(void *fixture)
{
	ARG_UNUSED(fixture);
	(void)dmic_trigger(DEVICE_DT_GET(PDM), DMIC_TRIGGER_STOP);
}


/*
 * Single ZTEST whose suite is test_pdm (HP) / test_lppdm (HE) and whose case
 * name is generated at compile time from the active group + mode + channels
 * (see PDM_SUITE_NAME / PDM_TEST_NAME), e.g. test_pdm_multi_mode6_ch_0_1_4_5.
 * The extra indirection forces those macros to expand before ZTEST token-pastes
 * them, yielding a unique, deterministic name per build for TestLink mapping.
 */
#define PDM_ZTEST(suite, name) ZTEST(suite, name)
#define PDM_ZTEST_SUITE(suite) ZTEST_SUITE(suite, NULL, NULL, NULL, pdm_capture_after, NULL)

PDM_ZTEST(PDM_SUITE_NAME, PDM_TEST_NAME)
{
	zassert_ok(test_channel(), "PDM capture failed");

}

PDM_ZTEST_SUITE(PDM_SUITE_NAME);

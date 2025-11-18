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
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>
#include <string.h>

LOG_MODULE_REGISTER(alif_test_pdm, LOG_LEVEL_INF);

/* Fallbacks for test harness */
#ifndef TC_PASS
#define TC_PASS 0
#endif

#ifndef TIMEOUT
#define TIMEOUT K_SECONDS(2)
#endif

#define FIR_COEF_COUNT     18U
#define DEFAULT_IIR_COEF   0x00000004U
#define PRINT_BYTES        80U

struct pdm_ch_config pdm_coef_reg;

/* FIR coefficient arrays (identical to original) */
static const uint32_t ch0_fir[FIR_COEF_COUNT] = {
	0x00000000, 0x000007FF, 0x00000000, 0x00000004,
	0x00000004, 0x000007FC, 0x00000000, 0x000007FB,
	0x000007E4,
	0x00000000, 0x0000002B, 0x00000009, 0x00000016,
	0x00000049, 0x00000793, 0x000006F8, 0x00000045,
	0x00000178,
};
static const uint32_t ch1_fir[FIR_COEF_COUNT] = {
	0x00000001, 0x00000003, 0x00000003, 0x000007F4,
	0x00000004, 0x000007ED, 0x000007F5, 0x000007F4,
	0x000007D3,
	0x000007FE, 0x000007BC, 0x000007E5, 0x000007D9,
	0x00000793, 0x00000029, 0x0000072C, 0x00000072,
	0x000002FD,
};
static const uint32_t ch2_fir[FIR_COEF_COUNT] = {
	0x00000000, 0x000007FF, 0x00000000, 0x00000004,
	0x00000004, 0x000007FC, 0x00000000, 0x000007FB,
	0x000007E4,
	0x00000000, 0x0000002B, 0x00000009, 0x00000016,
	0x00000049, 0x00000793, 0x000006F8, 0x00000045,
	0x00000178,
};
static const uint32_t ch3_fir[FIR_COEF_COUNT] = {
	0x00000001, 0x00000003, 0x00000003, 0x000007F4,
	0x00000004, 0x000007ED, 0x000007F5, 0x000007F4,
	0x000007D3,
	0x000007FE, 0x000007BC, 0x000007E5, 0x000007D9,
	0x00000793, 0x00000029, 0x0000072C, 0x00000072,
	0x000002FD,
};
static const uint32_t ch4_fir[FIR_COEF_COUNT] = {
	0x00000001, 0x00000003, 0x00000003, 0x000007F4,
	0x00000004, 0x000007ED, 0x000007F5, 0x000007F4,
	0x000007D3,
	0x000007FE, 0x000007BC, 0x000007E5, 0x000007D9,
	0x00000793, 0x00000029, 0x0000072C, 0x00000072,
	0x000002FD,
};
static const uint32_t ch5_fir[FIR_COEF_COUNT] = {
	0x00000000, 0x000007FF, 0x00000000, 0x00000004,
	0x00000004, 0x000007FC, 0x00000000, 0x000007FB,
	0x000007E4,
	0x00000000, 0x0000002B, 0x00000009, 0x00000016,
	0x00000049, 0x00000793, 0x000006F8, 0x00000045,
	0x00000178,
};
static const uint32_t ch6_fir[FIR_COEF_COUNT] = {
	0x00000001, 0x00000003, 0x00000003, 0x000007F4,
	0x00000004, 0x000007ED, 0x000007F5, 0x000007F4,
	0x000007D3,
	0x000007FE, 0x000007BC, 0x000007E5, 0x000007D9,
	0x00000793, 0x00000029, 0x0000072C, 0x00000072,
	0x000002FD,
};
static const uint32_t ch7_fir[FIR_COEF_COUNT] = {
	0x00000000, 0x000007FF, 0x00000000, 0x00000004,
	0x00000004, 0x000007FC, 0x00000000, 0x000007FB,
	0x000007E4,
	0x00000000, 0x0000002B, 0x00000009, 0x00000016,
	0x00000049, 0x00000793, 0x000006F8, 0x00000045,
	0x00000178,
};

static const uint32_t * const fir_table[8] = {
	ch0_fir, ch1_fir, ch2_fir, ch3_fir, ch4_fir, ch5_fir, ch6_fir, ch7_fir
};

/* Build enabled channel array via preprocessor */
static const int enabled_channels[] = {
#ifdef CONFIG_TEST_PDM_CH0
	0,
#endif
#ifdef CONFIG_TEST_PDM_CH1
	1,
#endif
#ifdef CONFIG_TEST_PDM_CH2
	2,
#endif
#ifdef CONFIG_TEST_PDM_CH3
	3,
#endif
#ifdef CONFIG_TEST_PDM_CH4
	4,
#endif
#ifdef CONFIG_TEST_PDM_CH5
	5,
#endif
#ifdef CONFIG_TEST_PDM_CH6
	6,
#endif
#ifdef CONFIG_TEST_PDM_CH7
	7,
#endif
};

/* Helper to set channel parameters (phase, gain, peak thresholds).
 */
static void set_channel_hw_params(const struct device *dev, uint8_t ch)
{
	switch (ch) {
	case 0:
		pdm_set_ch_phase(dev, CHANNEL_0, CH0_PHASE);
		pdm_set_ch_gain(dev, CHANNEL_0, CH0_GAIN);
		pdm_set_peak_detect_th(dev, CHANNEL_0, CH0_PEAK_DETECT_TH);
		pdm_set_peak_detect_itv(dev, CHANNEL_0, CH0_PEAK_DETECT_ITV);
		break;
	case 1:
		pdm_set_ch_phase(dev, CHANNEL_1, CH1_PHASE);
		pdm_set_ch_gain(dev, CHANNEL_1, CH1_GAIN);
		pdm_set_peak_detect_th(dev, CHANNEL_1, CH1_PEAK_DETECT_TH);
		pdm_set_peak_detect_itv(dev, CHANNEL_1, CH1_PEAK_DETECT_ITV);
		break;
	case 2:
		pdm_set_ch_phase(dev, CHANNEL_2, CH2_PHASE);
		pdm_set_ch_gain(dev, CHANNEL_2, CH2_GAIN);
		pdm_set_peak_detect_th(dev, CHANNEL_2, CH2_PEAK_DETECT_TH);
		pdm_set_peak_detect_itv(dev, CHANNEL_2, CH2_PEAK_DETECT_ITV);
		break;
	case 3:
		pdm_set_ch_phase(dev, CHANNEL_3, CH3_PHASE);
		pdm_set_ch_gain(dev, CHANNEL_3, CH3_GAIN);
		pdm_set_peak_detect_th(dev, CHANNEL_3, CH3_PEAK_DETECT_TH);
		pdm_set_peak_detect_itv(dev, CHANNEL_3, CH3_PEAK_DETECT_ITV);
		break;
	case 4:
		pdm_set_ch_phase(dev, CHANNEL_4, CH4_PHASE);
		pdm_set_ch_gain(dev, CHANNEL_4, CH4_GAIN);
		pdm_set_peak_detect_th(dev, CHANNEL_4, CH4_PEAK_DETECT_TH);
		pdm_set_peak_detect_itv(dev, CHANNEL_4, CH4_PEAK_DETECT_ITV);
		break;
	case 5:
		pdm_set_ch_phase(dev, CHANNEL_5, CH5_PHASE);
		pdm_set_ch_gain(dev, CHANNEL_5, CH5_GAIN);
		pdm_set_peak_detect_th(dev, CHANNEL_5, CH5_PEAK_DETECT_TH);
		pdm_set_peak_detect_itv(dev, CHANNEL_5, CH5_PEAK_DETECT_ITV);
		break;
	case 6:
		pdm_set_ch_phase(dev, CHANNEL_6, CH6_PHASE);
		pdm_set_ch_gain(dev, CHANNEL_6, CH6_GAIN);
		pdm_set_peak_detect_th(dev, CHANNEL_6, CH6_PEAK_DETECT_TH);
		pdm_set_peak_detect_itv(dev, CHANNEL_6, CH6_PEAK_DETECT_ITV);
		break;
	case 7:
		pdm_set_ch_phase(dev, CHANNEL_7, CH7_PHASE);
		pdm_set_ch_gain(dev, CHANNEL_7, CH7_GAIN);
		pdm_set_peak_detect_th(dev, CHANNEL_7, CH7_PEAK_DETECT_TH);
		pdm_set_peak_detect_itv(dev, CHANNEL_7, CH7_PEAK_DETECT_ITV);
		break;
	default:
		LOG_WRN("Unsupported channel %d in %s", ch, __func__);
		break;
	}
}

/* Configure a single channel: set HW params, copy FIR/IIR, call pdm_channel_config */
static int pdm_configure_channel(const struct device *dev, uint8_t ch,
		const uint32_t *fir, uint32_t iir)
{
	if (!dev) {
		LOG_ERR("%s: device is NULL", __func__);
		return -EINVAL;
	}

	set_channel_hw_params(dev, ch);

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

	if (!device_is_ready(dev)) {
		LOG_ERR("PDM device not ready");
		return -ENODEV;
	}

	/* If no channel macros enabled, enabled_channels[] will be empty.
	 * In that case, default to channel 5 as original code had 'else' branch.
	 */
	if (num_enabled == 0) {
		LOG_INF("No CONFIG_TEST_PDM_CHx macro set; defaulting to channel 5");
		return pdm_configure_channel(dev, 5, fir_table[5], DEFAULT_IIR_COEF);
	}

	for (idx = 0; idx < num_enabled; ++idx) {
		int ch = enabled_channels[idx];

		if (ch < 0 || ch > 7) {
			LOG_WRN("Skipping invalid channel entry: %d", ch);
			continue;
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
#if CONFIG_TEST_PDM_MODE0
	pdm_mode(dev, PDM_MODE_MICROPHONE_SLEEP);
	LOG_INF("Configured MODE0: PDM_MODE_MICROPHONE_SLEEP");
#elif CONFIG_TEST_PDM_MODE2
	pdm_mode(dev, PDM_MODE_HIGH_QUALITY_512_CLK_FRQ);
	LOG_INF("Configured MODE2: PDM_MODE_HIGH_QUALITY_512_CLK_FRQ");
#elif CONFIG_TEST_PDM_MODE3
	pdm_mode(dev, PDM_MODE_HIGH_QUALITY_768_CLK_FRQ);
	LOG_INF("Configured MODE3: PDM_MODE_HIGH_QUALITY_768_CLK_FRQ");
#elif CONFIG_TEST_PDM_MODE4
	pdm_mode(dev, PDM_MODE_HIGH_QUALITY_1024_CLK_FRQ);
	LOG_INF("Configured MODE4: PDM_MODE_HIGH_QUALITY_1024_CLK_FRQ");
#elif CONFIG_TEST_PDM_MODE5
	pdm_mode(dev, PDM_MODE_WIDE_BANDWIDTH_AUDIO_1536_CLK_FRQ);
	LOG_INF("Configured MODE5: PDM_MODE_WIDE_BANDWIDTH_AUDIO_1536_CLK_FRQ");
#elif CONFIG_TEST_PDM_MODE6
	pdm_mode(dev, PDM_MODE_FULL_BANDWIDTH_AUDIO_2400_CLK_FRQ);
	LOG_INF("Configured MODE6: PDM_MODE_FULL_BANDWIDTH_AUDIO_2400_CLK_FRQ");
#elif CONFIG_TEST_PDM_MODE7
	pdm_mode(dev, PDM_MODE_FULL_BANDWIDTH_AUDIO_3071_CLK_FRQ);
	LOG_INF("Configured MODE7: PDM_MODE_FULL_BANDWIDTH_AUDIO_3071_CLK_FRQ");
#elif CONFIG_TEST_PDM_MODE8
	pdm_mode(dev, PDM_MODE_ULTRASOUND_4800_CLOCK_FRQ);
	LOG_INF("Configured MODE8: PDM_MODE_ULTRASOUND_4800_CLOCK_FRQ");
#elif CONFIG_TEST_PDM_MODE9
	pdm_mode(dev, PDM_MODE_ULTRASOUND_96_SAMPLING_RATE);
	LOG_INF("Configured MODE9: PDM_MODE_ULTRASOUND_96_SAMPLING_RATE");

#else
	pdm_mode(dev, PDM_MODE_STANDARD_VOICE_512_CLK_FRQ);
	LOG_INF("Configured MODE1: PDM_MODE_STANDARD_VOICE_512_CLK_FRQ");
#endif
}

/* Configuration functions for compatibility (mono/stereo/multi) */
void pdm_monoch_config(void)
{
	const struct device *dev = DEVICE_DT_GET(PDM);

	if (!device_is_ready(dev)) {
		LOG_ERR("%s: PDM device not ready", __func__);
		return;
	}

	/* In mono mode we want exactly one channel selected.
	 * If multiple CONFIG_TEST_PDM_CHx macros are set, we configure them all,
	 * to match original behavior: build-time selection controls what gets configured.
	 */
	if (pdm_configure_selected_channels(dev) != 0) {
		LOG_ERR("%s: channel configuration failed", __func__);
		return;
	}

	pdm_select_mode(dev);
}

void pdm_multich_config(void)
{
	const struct device *dev = DEVICE_DT_GET(PDM);

	if (!device_is_ready(dev)) {
		LOG_ERR("%s: PDM device not ready", __func__);
		return;
	}

	if (pdm_configure_selected_channels(dev) != 0) {
		LOG_ERR("%s: channel configuration failed", __func__);
		return;
	}

	pdm_select_mode(dev);
}

void pdm_stereoch_config(void)
{
	const struct device *dev = DEVICE_DT_GET(PDM);

	if (!device_is_ready(dev)) {
		LOG_ERR("%s: PDM device not ready", __func__);
		return;
	}

/* In stereo mode original code configured either (0, 1),(2, 3),(6, 7) or defaulted to (4, 5).
 * The enabled_channels[] array will reflect the compile-time set channels.
 */
	if (pdm_configure_selected_channels(dev) != 0) {
		LOG_ERR("%s: channel configuration failed", __func__);
		return;
	}

	pdm_select_mode(dev);
}

/* Memory slab and buffers kept as in original */
K_MEM_SLAB_DEFINE(mem_slab, PCMJ_BLOCK_SIZE, MEM_SLAB_NUM_BLOCKS, 4);
static uint8_t pcmj_data[DATA_SIZE];
uint16_t buffers[PCMJ_BLOCK_SIZE * MEM_SLAB_NUM_BLOCKS];

/* Configure DMIC/PCM stream; kept similar to original but corrected assertions */
void set_config(struct dmic_cfg *cfg, struct pcm_stream_cfg *stream)
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
	int rc = 0;
	int i;
	void *buffer = NULL;
	uint32_t data = 0;
	int k = 0;

	zassert_true(dmic_dev != NULL && cfg != NULL, "%s: input invalid\n", __func__);

	rc = dmic_configure(dmic_dev, cfg);
	if (rc) {
		LOG_ERR("dmic_configure failed: %d", rc);
		return rc;
	}

#if CONFIG_TEST_PDM_MONO_CH
	pdm_monoch_config();
#elif CONFIG_TEST_PDM_MULTI_CH
	pdm_multich_config();
#else
	pdm_stereoch_config();
#endif

	LOG_INF("Start Speaking or Play some Audio!");

	rc = dmic_trigger(dmic_dev, DMIC_TRIGGER_START);
	zassert_equal(rc, 0, "dmic_trigger error\n");

	k = 0;
	for (i = 0; i < block_count; ++i) {
		rc = dmic_read(dmic_dev, 0, &buffer, &data, TIMEOUT);
		if (rc < 0) {
			LOG_ERR("dmic_read failed: %d", rc);
			dmic_trigger(dmic_dev, DMIC_TRIGGER_STOP);
			return rc;
		}

		/* Copy the data from the buffer to the pcmj_data (safely) */
		if ((k + (int)data) <= (int)DATA_SIZE) {
			memcpy(pcmj_data + k, buffer, data);
			k += data;
		} else {
			LOG_WRN("PCM buffer full or data exceeds buffer size; truncating");
		}

		k_mem_slab_free(&mem_slab, buffer);
	}

	LOG_INF("Stop recording");
	LOG_INF("PCM samples will be stored in %p address and size of buffer is %d",
			(void *)pcmj_data, (int)sizeof(pcmj_data));

	LOG_INF("Last block pointer: %p", buffer);

	rc = dmic_trigger(dmic_dev, DMIC_TRIGGER_STOP);
	if (rc) {
		LOG_ERR("dmic_trigger stop failed: %d", rc);
	}

	return rc;
}

/* Print first bytes of captured PCM data for debugging */
void print_data(void)
{
	uint32_t i;
	uint32_t lim = MIN(PRINT_BYTES, DATA_SIZE);

	LOG_INF("pcm data pointer: %p", (void *)pcmj_data);

	for (i = 0; i + 7 < lim; i += 8) {
		LOG_INF(" %02x %02x %02x %02x %02x %02x %02x %02x",
				pcmj_data[i], pcmj_data[i+1], pcmj_data[i+2], pcmj_data[i+3],
				pcmj_data[i+4], pcmj_data[i+5], pcmj_data[i+6], pcmj_data[i+7]);
	}

	k_msleep(500);
}

static int test_channel(void)
{
	const struct device *pcmj_device;
	struct dmic_cfg cfg;
	struct pcm_stream_cfg stream;
	int rc;

	LOG_INF("PDM test channel");

	pcmj_device = DEVICE_DT_GET(PDM);

	zassert_true(pcmj_device, "pcmj_device not ready\n");
	zassert_true(device_is_ready(pcmj_device), "device not ready\n");

	set_config(&cfg, &stream);

	rc = config_channel(pcmj_device, &cfg, MEM_SLAB_NUM_BLOCKS);
	if (rc) {
		LOG_ERR("config_channel returned %d", rc);
		return rc;
	}

	print_data();

	return TC_PASS;
}

#ifdef CONFIG_TEST_PDM_STEREO_CH
	#if (CONFIG_TEST_PDM_CH0 && CONFIG_TEST_PDM_CH1)
		ZTEST(test_PDM, test_pdm_Stereo_channel_0_1)
		{
			zassert_true(test_channel() == TC_PASS);
		}
	#elif (CONFIG_TEST_PDM_CH2 && CONFIG_TEST_PDM_CH3)
		ZTEST(test_PDM, test_pdm_Stereo_channel_2_3)
		{
			zassert_true(test_channel() == TC_PASS);
		}
	#elif (CONFIG_TEST_PDM_CH6 && CONFIG_TEST_PDM_CH7)
		ZTEST(test_PDM, test_pdm_Stereo_channel_6_7)
		{
			zassert_true(test_channel() == TC_PASS);
		}
	#elif (CONFIG_TEST_PDM_CH4 && CONFIG_TEST_PDM_CH5)
		ZTEST(test_PDM, test_pdm_Stereo_channel_4_5)
		{
			zassert_true(test_channel() == TC_PASS);
		}
	#endif
#endif

#ifdef CONFIG_TEST_PDM_MONO_CH
		ZTEST(test_PDM, test_pdm_mono_channel)
		{
			zassert_true(test_channel() == TC_PASS);
		}
#endif

#ifdef CONFIG_TEST_PDM_MULTI_CH
		ZTEST(test_PDM, test_pdm_multi_channel)
		{
			zassert_true(test_channel() == TC_PASS);
		}
#endif

ZTEST_SUITE(test_PDM, NULL, NULL, NULL, NULL, NULL);

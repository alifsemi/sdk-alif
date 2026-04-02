/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/**
 * @file audio_source_pdm.c
 * @brief PDM/DMIC audio source implementation for Bluetooth LE Audio
 *
 * This module provides an audio source implementation using PDM (Pulse Density Modulation)
 * microphones via the DMIC (Digital Microphone) interface. It is designed to work with
 * the Bluetooth LE Audio stack, specifically for unicast audio streaming.
 *
 * Key Features:
 * - Reads audio data from PDM microphones using dmic_read() API
 * - Converts interleaved stereo PCM to separate left/right channels
 * - Integrates with the audio encoder pipeline via audio queues
 * - Supports timing correction for audio/video synchronization
 * - Thread-based continuous audio capture with configurable timeout
 * - Optional GPIO test points for timing analysis and debugging
 *
 * Audio Flow:
 * 1. PDM reader thread continuously calls dmic_read() to get audio data
 * 2. Received interleaved stereo samples are de-interleaved into separate channels
 * 3. Processed audio blocks are placed into the audio queue for encoding
 * 4. Audio encoder consumes blocks from the queue and generates LC3 frames
 *
 * Thread Safety:
 * - Single reader thread accesses DMIC device
 * - Audio queue handles synchronization between reader and encoder
 * - Timing corrections use spinlocks from audio_i2s_common module
 *
 * Configuration:
 * - Frame duration: 7.5ms or 10ms
 * - Sampling rates: 16kHz, 24kHz, 32kHz, 48kHz, 96kHz, 192kHz
 * - Channels: Stereo (interleaved) input, mono or stereo output
 * - Buffer: Based on presentation delay requirements
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/sys/util.h>
#include <zephyr/audio/dmic.h>
#include <zephyr/drivers/pdm/pdm_alif.h>
#include <zephyr/drivers/gpio.h>
#include "gapi_isooshm.h"
#include "audio_i2s_common.h"
#include "audio_source_pdm.h"

LOG_MODULE_REGISTER(audio_source_pdm, CONFIG_BLE_AUDIO_LOG_LEVEL);

/* TODO: Move these configuration values to Kconfig */
#ifndef CONFIG_AUDIO_L_CHANNEL
#define CONFIG_AUDIO_L_CHANNEL 4
#endif

#ifndef CONFIG_AUDIO_R_CHANNEL
#define CONFIG_AUDIO_R_CHANNEL 5
#endif

#ifndef CONFIG_AUDIO_PDM_PHASE
#define CONFIG_AUDIO_PDM_PHASE 0
#endif

#ifndef CONFIG_AUDIO_IIR_COEF
#define CONFIG_AUDIO_IIR_COEF 0
#endif

#ifndef CONFIG_ALIF_BLE_AUDIO_PDM_MICROPHONE_GAIN
#define CONFIG_ALIF_BLE_AUDIO_PDM_MICROPHONE_GAIN 50
#endif

#if CONFIG_ALIF_BLE_AUDIO_USE_RAMFUNC
#define INT_RAMFUNC __ramfunc
#else
#define INT_RAMFUNC
#endif

#if CONFIG_ALIF_BLE_AUDIO_SOURCE_TRANSMISSION_DELAY_MS
#define TRANSMISSION_DELAY_US (CONFIG_ALIF_BLE_AUDIO_SOURCE_TRANSMISSION_DELAY_MS * 1000)
#else
#define TRANSMISSION_DELAY_US 0
#endif

/* Max supported sampling rate is 192kHz.
 * 10ms frame has 1920 bytes and 7.5ms has 1440 bytes.
 */
#define MAX_SAMPLES_PER_BLOCK  1920
/* Left and right audio channels. */
#define NUMBER_OF_CHANNELS     2
/* Number of microphone channels (stereo PDM input) */
#define NUMBER_OF_MIC_CHANNELS 2
/* PDM read timeout in milliseconds. Use 3 times max frame duration (10ms) */
#define PDM_READ_TIMEOUT       (3 * 10)
/* Define a memory slab for PDM sample blocks */
#define AUDIO_BLOCK_SIZE       (NUMBER_OF_MIC_CHANNELS * MAX_SAMPLES_PER_BLOCK)
/* Buffer count for PDM sample blocks */
#define AUDIO_BLOCK_COUNT      (4)

#if MAX_SAMPLES_PER_AUDIO_BLOCK > MAX_SAMPLES_PER_BLOCK
#error "MAX_SAMPLES_PER_AUDIO_BLOCK must be less than MAX_SAMPLES_PER_BLOCK"
#endif

static uint32_t fir[18] = {0x00000001, 0x00000003, 0x00000003, 0x000007F4, 0x00000004, 0x000007ED,
			   0x000007F5, 0x000007F4, 0x000007D3, 0x000007FE, 0x000007BC, 0x000007E5,
			   0x000007D9, 0x00000793, 0x00000029, 0x0000072C, 0x00000072, 0x000002FD};

#define GPIO_TEST0_NODE DT_ALIAS(source_pdm_test0)
#define GPIO_TEST1_NODE DT_ALIAS(source_pdm_test1)

#if DT_NODE_EXISTS(GPIO_TEST0_NODE) || DT_NODE_EXISTS(GPIO_TEST1_NODE)
static const struct gpio_dt_spec test_pin0 = GPIO_DT_SPEC_GET_OR(GPIO_TEST0_NODE, gpios, {0});
static const struct gpio_dt_spec test_pin1 = GPIO_DT_SPEC_GET_OR(GPIO_TEST1_NODE, gpios, {0});

static int init_test_pin(const struct gpio_dt_spec *const p_pin)
{
	if (!p_pin->port) {
		return -ENODEV;
	}
	if (!gpio_is_ready_dt(p_pin)) {
		LOG_WRN("Test pin is not ready");
		return -ENODEV;
	}
	if (gpio_pin_configure_dt(p_pin, GPIO_OUTPUT_ACTIVE)) {
		LOG_ERR("Failed to configure test pin");
		return -EIO;
	}
	gpio_pin_set_dt(p_pin, 0);
	LOG_INF("PDM source test pin %u initialized", p_pin->pin);
	return 0;
}

static inline void toggle_test_pin(const struct gpio_dt_spec *const p_pin)
{
	if (!p_pin->port) {
		return;
	}
	gpio_pin_toggle_dt(p_pin);
}

static inline void set_test_pin(const struct gpio_dt_spec *const p_pin, int const val)
{
	if (!p_pin->port) {
		return;
	}
	gpio_pin_set_dt(p_pin, !!val);
}

/* Initialisation to perform pre-main */
static int audio_source_pdm_init(void)
{
	init_test_pin(&test_pin0);
	init_test_pin(&test_pin1);
	return 0;
}
SYS_INIT(audio_source_pdm_init, APPLICATION, 0);

#endif

struct audio_source_pdm {
	const struct device *dev;
	struct audio_queue *audio_queue;
	struct audio_i2s_timing timing;
	size_t block_samples;
	size_t number_of_channels;
	struct k_thread thread;
	bool drop_next_audio_block;
	bool started;
	bool thread_abort;
};

static struct audio_source_pdm audio_source;

K_MEM_SLAB_DEFINE_STATIC(dmic_mem_slab, AUDIO_BLOCK_SIZE, AUDIO_BLOCK_COUNT, sizeof(uint32_t));

K_KERNEL_STACK_DEFINE(pdm_reader_stack, 2048);

static int16_t fs_to_pdm_mode(uint32_t const sampling_freq_hz)
{
	switch (sampling_freq_hz) {
	case 192000:
		return 9;
	case 96000:
		return 8;
	case 48000:
		return 6;
	case 32000:
		return 5;
	case 16000:
		return 2;
	case 8000:
		return 1;
	default:
		return -EINVAL;
	}
}

int configure_pdm_source(const struct device *pdm_dev, struct audio_queue *audio_queue_mic)
{
	int ret;
	struct dmic_cfg cfg;
	struct pcm_stream_cfg stream;
	struct pdm_ch_config pdm_coef_reg;
	int16_t pdm_mode_val = fs_to_pdm_mode(audio_queue_mic->sampling_freq_hz);

	LOG_DBG("sampling freq=%u Hz, block_samples=%u",
		audio_queue_mic->sampling_freq_hz, audio_queue_mic->audio_block_samples);

	if (pdm_mode_val < 0) {
		LOG_ERR("Unsupported sampling frequency %u", audio_queue_mic->sampling_freq_hz);
		return -EINVAL;
	}

	LOG_DBG("PDM mode value: %d", pdm_mode_val);

	memcpy(pdm_coef_reg.ch_fir_coef, fir, sizeof(pdm_coef_reg.ch_fir_coef));
	pdm_coef_reg.ch_iir_coef = CONFIG_AUDIO_IIR_COEF;

	cfg.streams = &stream;
	cfg.streams[0].mem_slab = &dmic_mem_slab;
	cfg.channel.req_num_streams = 1;
	cfg.channel.req_num_chan = NUMBER_OF_MIC_CHANNELS;
	cfg.streams[0].block_size = audio_queue_mic->audio_block_samples * NUMBER_OF_MIC_CHANNELS *
				    sizeof(pcm_sample_t);
	cfg.channel.req_chan_map_lo =
		((1 << CONFIG_AUDIO_L_CHANNEL) | (1 << CONFIG_AUDIO_R_CHANNEL));
	ret = dmic_configure(pdm_dev, &cfg);
	if (ret != 0) {
		LOG_ERR("Failed to configure DMIC, err %d", ret);
		return ret;
	}

	/* Configure left channel */
	LOG_DBG("Configuring left channel (ch=%d, gain=%d, phase=%d)", CONFIG_AUDIO_L_CHANNEL,
		CONFIG_ALIF_BLE_AUDIO_PDM_MICROPHONE_GAIN, CONFIG_AUDIO_PDM_PHASE);
	pdm_set_ch_phase(pdm_dev, CONFIG_AUDIO_L_CHANNEL, CONFIG_AUDIO_PDM_PHASE);
	pdm_set_ch_gain(pdm_dev, CONFIG_AUDIO_L_CHANNEL,
			(CONFIG_ALIF_BLE_AUDIO_PDM_MICROPHONE_GAIN << 4));
	pdm_coef_reg.ch_num = CONFIG_AUDIO_L_CHANNEL;
	pdm_channel_config(pdm_dev, &pdm_coef_reg);

	/* Cofigure right channel */
	LOG_DBG("Configuring right channel (ch=%d, gain=%d, phase=%d)", CONFIG_AUDIO_R_CHANNEL,
		CONFIG_ALIF_BLE_AUDIO_PDM_MICROPHONE_GAIN, CONFIG_AUDIO_PDM_PHASE);
	pdm_set_ch_gain(pdm_dev, CONFIG_AUDIO_R_CHANNEL,
			(CONFIG_ALIF_BLE_AUDIO_PDM_MICROPHONE_GAIN << 4));
	pdm_set_ch_phase(pdm_dev, CONFIG_AUDIO_R_CHANNEL, CONFIG_AUDIO_PDM_PHASE);
	pdm_coef_reg.ch_num = CONFIG_AUDIO_R_CHANNEL;
	pdm_channel_config(pdm_dev, &pdm_coef_reg);

	pdm_mode(pdm_dev, pdm_mode_val);

	LOG_DBG("DMIC configured successfully");
	return 0;
}

INT_RAMFUNC static void process_audio_block(void const *buffer, size_t const size)
{
#if DT_NODE_EXISTS(GPIO_TEST0_NODE)
	set_test_pin(&test_pin0, 1);
#endif

	struct audio_block *p_audiobuf = NULL;
	int ret =
		k_mem_slab_alloc(&audio_source.audio_queue->slab, (void **)&p_audiobuf, K_NO_WAIT);

	if (ret || !p_audiobuf) {
		/* No buffer available, just drop it */
#if DT_NODE_EXISTS(GPIO_TEST0_NODE)
		set_test_pin(&test_pin0, 0);
#endif
		LOG_WRN("process_audio_block: no buffer available, dropping block");
		return;
	}

	bool const has_right_channel = audio_source.number_of_channels > 1;

	/* Populate the capture timestamp of the block */
#if CONFIG_ALIF_BLE_AUDIO_SOURCE_TRANSMISSION_DELAY_ENABLED
	p_audiobuf->timestamp = gapi_isooshm_dp_get_local_time() + TRANSMISSION_DELAY_US;
#else
	p_audiobuf->timestamp = 0;
#endif
	p_audiobuf->num_channels = 1 + has_right_channel;

	/* Buffer from dmic_read contains interleaved stereo samples */
	pcm_sample_t const *p_input = (pcm_sample_t const *)buffer;
	size_t const input_block_samples = size / sizeof(pcm_sample_t);

	/* Loop input samples and copy sequentially.
	 * Every even sample is for left channel, odd for right.
	 */
	pcm_sample_t *p_out_left = p_audiobuf->channels[0];
#if CONFIG_ALIF_BLE_AUDIO_NMB_CHANNELS > 1
	pcm_sample_t *p_out_right = p_audiobuf->channels[1];
#endif
	for (size_t iter = 0; iter < input_block_samples; iter++) {
		if (likely(has_right_channel) && (iter & 1)) {
#if CONFIG_ALIF_BLE_AUDIO_NMB_CHANNELS > 1
			*p_out_right++ = *p_input++;
#else
			p_input++;
#endif
			continue;
		}
		*p_out_left++ = *p_input++;
	}

	if (k_msgq_put(&audio_source.audio_queue->msgq, &p_audiobuf, K_NO_WAIT)) {
		/* Failed to put into queue */
		k_mem_slab_free(&audio_source.audio_queue->slab, p_audiobuf);
		LOG_ERR("Audio msg queue is full, frame dropped");
	}
#if DT_NODE_EXISTS(GPIO_TEST0_NODE)
	set_test_pin(&test_pin0, 0);
#endif
}

static void pdm_reader_thread(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	LOG_DBG("PDM reader thread started");

	while (!audio_source.thread_abort) {
		if (!audio_source.started) {
			k_sleep(K_MSEC(10));
			continue;
		}

#if DT_NODE_EXISTS(GPIO_TEST1_NODE)
		set_test_pin(&test_pin1, 1);
#endif

		void *buffer = NULL;
		size_t size = 0;

		int ret = dmic_read(audio_source.dev, 0, &buffer, &size, PDM_READ_TIMEOUT);

		if (ret == -EAGAIN) {
			/* Timeout, continue waiting */
			LOG_WRN("dmic_read timeout");
			continue;
		} else if (ret) {
			LOG_ERR("dmic_read failed: %d", ret);
			k_sleep(K_MSEC(10));
			continue;
		}

		if (!buffer || !size) {
			if (buffer) {
				k_mem_slab_free(&dmic_mem_slab, buffer);
			}
			LOG_WRN("Empty buffer from dmic_read");
			continue;
		}

		if (audio_source.drop_next_audio_block) {
			audio_source.drop_next_audio_block = false;
		} else {
			process_audio_block(buffer, size);
		}

		k_mem_slab_free(&dmic_mem_slab, buffer);

#if DT_NODE_EXISTS(GPIO_TEST1_NODE)
		set_test_pin(&test_pin1, 0);
#endif
	}

	LOG_INF("PDM source stopped");
}

int audio_source_pdm_configure(const struct device *dev, struct audio_queue *audio_queue)
{
	if (!dev || !audio_queue) {
		LOG_ERR("Invalid parameters: dev=%p, audio_queue=%p", dev, audio_queue);
		return -EINVAL;
	}

	if (!device_is_ready(dev)) {
		LOG_ERR("DMIC device is not ready");
		return -ENODEV;
	}

	/* Calculate sample count per block depending on the frame duration
	 * - 10ms frame: (sample_rate * 10) / 1000
	 * - 7.5ms frame: (sample_rate * 75) / 10000
	 */
	size_t const block_samples = audio_queue->frame_duration_us == 10000
					     ? (audio_queue->sampling_freq_hz * 10) / 1000
					     : (audio_queue->sampling_freq_hz * 75) / 10000;

	if (block_samples > MAX_SAMPLES_PER_BLOCK) {
		LOG_ERR("Block size exceeds maximum: block_samples=%u, MAX_SAMPLES_PER_BLOCK=%u",
			block_samples, MAX_SAMPLES_PER_BLOCK);
		return -EINVAL;
	}

	/* For stereo interleaved data from PDM */
	size_t const samples_per_full_block = NUMBER_OF_CHANNELS * block_samples;

#if DT_NODE_EXISTS(GPIO_TEST0_NODE)
	set_test_pin(&test_pin0, 0);
#endif

	/* Initialize the PDM audio source context */
	audio_source.dev = dev;
	audio_source.audio_queue = audio_queue;
	audio_source.number_of_channels = NUMBER_OF_CHANNELS;
	audio_source.block_samples = block_samples;
	audio_source.started = false;
	audio_source.thread_abort = false;
	audio_source.timing.correction_us = 0;
	audio_source.timing.us_per_block = audio_queue->frame_duration_us;
	audio_source.timing.samples_per_block = samples_per_full_block;
	audio_source.drop_next_audio_block = false;

	/* Maximum positive correction is slightly less than a full audio block
	 * (cannot receive zero samples in one read)
	 */
	audio_source.timing.max_single_correction = samples_per_full_block - 2;

	/* Minimum negative correction is one full audio block */
	audio_source.timing.min_single_correction = -samples_per_full_block;

	/* Configure the PDM source */
	configure_pdm_source(dev, audio_queue);

	/* Start the PDM reader thread with high priority */
	k_tid_t tid =
		k_thread_create(&audio_source.thread, pdm_reader_stack,
				K_KERNEL_STACK_SIZEOF(pdm_reader_stack), pdm_reader_thread, NULL,
				NULL, NULL, CONFIG_ALIF_BLE_HOST_THREAD_PRIORITY - 1, 0, K_NO_WAIT);

	if (!tid) {
		LOG_ERR("Failed to create PDM reader thread");
		return -ENOMEM;
	}

	k_thread_name_set(tid, "pdm_reader");

	LOG_INF("PDM audio source configured: %uHz, %uus frames, %u samples/block, channels=%u",
		audio_queue->sampling_freq_hz, audio_queue->frame_duration_us, block_samples,
		audio_source.number_of_channels);

	return 0;
}

/**
 * @brief Start the PDM audio source streaming
 *
 * Enables audio capture from the PDM microphone. The reader thread will begin
 * calling dmic_read() and processing audio blocks.
 *
 * Must be called after audio_source_pdm_configure().
 * Can be called multiple times - subsequent calls are ignored if already started.
 *
 * Thread-safe: Can be called from any thread context.
 */
void audio_source_pdm_start(void)
{
	if (!audio_source.dev) {
		LOG_ERR("PDM source not configured");
		return;
	}

	if (audio_source.started) {
		LOG_DBG("PDM source already started, ignoring");
		return;
	}

	int ret = dmic_trigger(audio_source.dev, DMIC_TRIGGER_START);

	if (ret) {
		LOG_ERR("Failed to start! ret=%d", ret);
		return;
	}

	audio_source.started = true;

	LOG_DBG("PDM audio source started");
}

void audio_source_pdm_stop(void)
{
	if (!audio_source.dev) {
		LOG_ERR("PDM source not configured");
		return;
	}

	if (!audio_source.started) {
		LOG_DBG("PDM source already stopped, ignoring");
		return;
	}

	int ret = dmic_trigger(audio_source.dev, DMIC_TRIGGER_STOP);

	if (ret) {
		LOG_ERR("Failed to stop DMIC: ret=%d", ret);
		return;
	}

	LOG_DBG("Terminating PDM reader thread");

	audio_source.thread_abort = true;

	/* Join thread before freeing anything */
	k_thread_join(&audio_source.thread, K_FOREVER);

	audio_source.started = false;
}

INT_RAMFUNC void audio_source_pdm_apply_timing_correction(int32_t const correction_us)
{
	LOG_DBG("Apply correction_us=%d", correction_us);
	audio_i2s_timing_apply_correction(&audio_source.timing, correction_us);
}

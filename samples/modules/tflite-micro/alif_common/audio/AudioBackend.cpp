/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include "AudioBackend.hpp"

#include <zephyr/logging/log.h>
#include <zephyr/devicetree.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/i2s.h>
#include <zephyr/audio/dmic.h>
#include <zephyr/drivers/pdm/pdm_alif.h>

LOG_MODULE_REGISTER(AudioBackend, LOG_LEVEL_DBG);

BUILD_ASSERT(CONFIG_AUDIO_STRIDE % CONFIG_SAMPLE_CNT == 0,
	     "CONFIG_AUDIO_STRIDE must be a multiple of CONFIG_SAMPLE_CNT");
BUILD_ASSERT(CONFIG_SAMPLE_CNT % 4 == 0, "CONFIG_SAMPLE_CNT must be a multiple of 4");

#define I2S_MICS DT_NODE_EXISTS(DT_ALIAS(i2s_mic))

#if I2S_MICS
#define AUDIO_DEVICE DT_ALIAS(i2s_mic)
#else
#define AUDIO_DEVICE DT_ALIAS(pdm_audio)
#endif

#define SAMPLE_SIZE       sizeof(int16_t)
#define WORD_SIZE         (SAMPLE_SIZE * 8)
#define AUDIO_CHANNELS    CONFIG_AUDIO_CHANNELS
#define SAMPLE_CNT        CONFIG_SAMPLE_CNT
#define NUM_BUFFERS       CONFIG_NUM_BUFFERS
#define BUFFER_SIZE       (AUDIO_CHANNELS * SAMPLE_CNT * SAMPLE_SIZE)
#define THREAD_STACK_SIZE CONFIG_THREAD_STACK_SIZE
#define THREAD_PRIORITY   CONFIG_THREAD_PRIORITY
#define I2S_GAIN          CONFIG_I2S_GAIN
#define CHANNEL_4         4
#define CHANNEL_5         5
#define PDM_CHANNELS      PDM_MASK_CHANNEL_4 | PDM_MASK_CHANNEL_5

/* PDM Channel configurations */
#define PDM_PHASE           0x0000001F
#define PDM_GAIN            0x00000F00
#define PDM_PEAK_DETECT_TH  0x00060002
#define PDM_PEAK_DETECT_ITV 0x0004002D
#define PDM_READ_TIMEOUT    500
#define SAMPLE_BIT_WIDTH    16
#define START               true
#define STOP                false

struct pdm_ch_config pdm_coef_reg;

uint32_t pdm_fir[18] = {0x00000001, 0x00000003, 0x00000003, 0x000007F4, 0x00000004, 0x000007ED,
			0x000007F5, 0x000007F4, 0x000007D3, 0x000007FE, 0x000007BC, 0x000007E5,
			0x000007D9, 0x00000793, 0x00000029, 0x0000072C, 0x00000072, 0x000002FD};

K_MEM_SLAB_DEFINE_STATIC(mem_slab, BUFFER_SIZE, NUM_BUFFERS, 4);

K_THREAD_STACK_DEFINE(audio_thread_stack, THREAD_STACK_SIZE);

static const struct device *mic = DEVICE_DT_GET(AUDIO_DEVICE);

static struct k_thread audio_thread;
static struct k_sem rx_start;
static struct k_sem rx_ready;
static int16_t *user_ptr;
static int user_len;
/* Set by audio_uninit() to ask the worker thread to stop. Volatile because it
 * is written from the caller's context and read from the worker thread. */
static volatile bool audio_stop_requested = false;
/* Result of the last capture attempt, published by the worker and returned to
 * the caller via wait_for_audio(). 0 means a chunk is ready; a negative value
 * means the worker stopped (mic start/read error). Volatile because it is
 * written by the worker and read by the caller's thread. */
static volatile int audio_status = 0;

static int mix_mono_output(int16_t *in, size_t in_size, int16_t *out, size_t out_size)
{
	size_t num_samples = in_size / 2;

	if (out_size < num_samples) {
		LOG_ERR("mix_mono_output param failure %d - %d", out_size, num_samples);
		return -EINVAL;
	}

	for (size_t i = 0; i < num_samples; ++i) {
		int16_t left_channel = in[2 * i];
		int16_t right_channel = in[2 * i + 1];

		out[i] = (int16_t)((left_channel + right_channel) / 2);
	}

	return 0;
}

static int trigger_audio(bool start)
{
#if I2S_MICS
	/* Use DROP rather than STOP to stop reception. The DW I2S driver only
	 * accepts STOP from the RUNNING state, but on an RX overrun (the app
	 * falling behind while reception is still active) the driver auto-
	 * transitions the stream to ERROR and disables RX on its own. A
	 * subsequent STOP is then rejected with -EIO ("STOP trigger: invalid
	 * state"). DROP is accepted from any active state (RUNNING or ERROR),
	 * always returns the stream to READY, and so also guarantees the next
	 * i2s_configure() succeeds. */
	return i2s_trigger(mic, I2S_DIR_RX,
			   (start ? I2S_TRIGGER_START : I2S_TRIGGER_DROP));
#else
	return dmic_trigger(mic, (start ? DMIC_TRIGGER_START : DMIC_TRIGGER_STOP));
#endif
}

static int audio_handle_rx(void)
{
	void *buffer = NULL;
	size_t size = 0;
	int offset = 0;

	while (offset < user_len) {
#if I2S_MICS
		int rc = i2s_read(mic, &buffer, &size);
#else
		int rc = dmic_read(mic, 0, &buffer, &size, PDM_READ_TIMEOUT);
#endif
		/* audio_uninit() sets audio_stop_requested while the mic is still
		 * running, so this read returns a valid buffer. Release it and
		 * exit promptly instead of finishing the chunk; otherwise the
		 * buffer is leaked and the slab is eventually exhausted, which
		 * makes a later mic start fail and the next session hang. */
		if (audio_stop_requested) {
			if (rc == 0 && buffer != NULL) {
				k_mem_slab_free(&mem_slab, buffer);
			}
			return 0;
		}
		if (rc != 0) {
			LOG_ERR("mic read failed: %i", rc);
			return rc;
		}

		int stero_samples = size / SAMPLE_SIZE;
		rc = mix_mono_output(static_cast<int16_t *>(buffer), stero_samples,
				     user_ptr + offset, user_len - offset);
		if (rc < 0) {
			LOG_ERR("mix_mono_output failed: %i", rc);
			return rc;
		}

		offset += stero_samples / 2;
		k_mem_slab_free(&mem_slab, buffer);
	}
	return 0;
}

static void audio_worker_thread(void *, void *, void *)
{
	/* Start the mic here, immediately before the first read, to keep the
	 * window between START and the first read as short as possible. The mic
	 * free-runs into a limited set of slab buffers, so starting it too early
	 * (e.g. in audio_init(), before the caller requests the first chunk) can
	 * overrun and push the I2S stream into ERROR before the first read. */
	int rc = trigger_audio(START);
	if (rc < 0) {
		LOG_ERR("mic start failed: %d", rc);
		audio_status = rc;
	} else {
		while (1) {
			k_sem_take(&rx_start, K_FOREVER);

			if (audio_stop_requested) {
				break;
			}

			if (user_ptr == NULL) {
				LOG_ERR("user_ptr is NULL");
				audio_status = -EINVAL;
				break;
			}

			rc = audio_handle_rx();
			if (rc < 0) {
				audio_status = rc;
				break;
			}

			k_sem_give(&rx_ready);
		}
	}

	/* Always wake a thread that may be blocked in wait_for_audio() so it can
	 * observe audio_status instead of blocking forever - a stranded waiter
	 * would leave every thread blocked and idle the whole system. On a clean
	 * stop this extra give is harmless; audio_uninit() resets the semaphore.
	 * The mic is stopped in audio_uninit() after this thread is joined, so
	 * the worker never stops the mic itself. */
	k_sem_give(&rx_ready);
}

#if I2S_MICS
static void i2s_config(int sampling_rate)
{
	const struct i2s_config config = {
		.word_size = WORD_SIZE,
		.channels = AUDIO_CHANNELS,
		.format = I2S_FMT_DATA_FORMAT_I2S,
		.options = I2S_OPT_FRAME_CLK_MASTER | I2S_OPT_BIT_CLK_MASTER,
		.frame_clk_freq = static_cast<uint32_t>(sampling_rate),
		.mem_slab = &mem_slab,
		.block_size = BUFFER_SIZE,
		.timeout = SYS_FOREVER_MS,
	};

	int rc = i2s_configure(mic, I2S_DIR_RX, &config);

	if (rc < 0) {
		LOG_ERR("i2s_configure failed: %i", rc);
	}
}
#else
static int32_t pdm_mode_set(int sampling_rate)
{
	// selects the mode based on requested sampling rate
	switch (sampling_rate) {
	case 8000:
		return PDM_MODE_STANDARD_VOICE_512_CLK_FRQ;
	case 16000:
		return PDM_MODE_HIGH_QUALITY_1024_CLK_FRQ;
	case 32000:
		return PDM_MODE_WIDE_BANDWIDTH_AUDIO_1536_CLK_FRQ;
	case 48000:
		return PDM_MODE_FULL_BANDWIDTH_AUDIO_3071_CLK_FRQ;
	case 96000:
		return PDM_MODE_ULTRASOUND_4800_CLOCK_FRQ;
	default:
		return -1;
	}
}

static void set_pdm_config(struct dmic_cfg *cfg, struct pcm_stream_cfg *stream)
{
	uint32_t channel_map = 0;

	stream->pcm_width = SAMPLE_BIT_WIDTH;
	cfg->streams = stream;
	cfg->streams[0].mem_slab = &mem_slab;
	cfg->channel.req_num_streams = 1;
	cfg->channel.req_num_chan = AUDIO_CHANNELS;
	cfg->streams[0].block_size = BUFFER_SIZE;

	channel_map = PDM_CHANNELS;

	cfg->channel.req_chan_map_lo = channel_map;
}

static int pdm_ch_config(const struct device *pcmj_device, int sampling_rate)
{
	struct dmic_cfg cfg;
	struct pcm_stream_cfg stream;
	int32_t pdm_val = pdm_mode_set(sampling_rate);
	if (pdm_val < 0) {
		LOG_ERR("pdm mode set failed");
		return -1;
	}

	set_pdm_config(&cfg, &stream);

	dmic_configure(mic, &cfg);

	pdm_set_ch_phase(pcmj_device, CHANNEL_4, PDM_PHASE);
	pdm_set_ch_gain(pcmj_device, CHANNEL_4, PDM_GAIN);
	pdm_set_peak_detect_th(pcmj_device, CHANNEL_4, PDM_PEAK_DETECT_TH);
	pdm_set_peak_detect_itv(pcmj_device, CHANNEL_4, PDM_PEAK_DETECT_ITV);
	pdm_coef_reg.ch_num = 4;
	memcpy(pdm_coef_reg.ch_fir_coef, pdm_fir, sizeof(pdm_coef_reg.ch_fir_coef));
	pdm_coef_reg.ch_iir_coef = 0x00000004;
	pdm_channel_config(pcmj_device, &pdm_coef_reg);
	pdm_set_ch_gain(pcmj_device, CHANNEL_5, PDM_GAIN);
	pdm_set_ch_phase(pcmj_device, CHANNEL_5, PDM_PHASE);
	pdm_set_peak_detect_th(pcmj_device, CHANNEL_5, PDM_PEAK_DETECT_TH);
	pdm_set_peak_detect_itv(pcmj_device, CHANNEL_5, PDM_PEAK_DETECT_ITV);
	pdm_coef_reg.ch_num = 5;
	memcpy(pdm_coef_reg.ch_fir_coef, pdm_fir, sizeof(pdm_coef_reg.ch_fir_coef));
	pdm_coef_reg.ch_iir_coef = 0x00000004;
	pdm_channel_config(pcmj_device, &pdm_coef_reg);

	pdm_mode(pcmj_device, pdm_val);

	return 0;
}
#endif

int audio_init(int sampling_rate)
{
	LOG_DBG("Audio init, sampling rate %d", sampling_rate);

	if (!device_is_ready(mic)) {
		LOG_ERR("mic is not ready");
		return -ENODEV;
	}
#if I2S_MICS
	/* Force the stream back to READY before configuring. If a previous
	 * session ended in the I2S ERROR state (e.g. an RX overrun), the stream
	 * is still in ERROR and i2s_configure() - which only accepts READY or
	 * NOT_READY - would otherwise fail and break every subsequent session.
	 * Skip this on the very first init: the device is still NOT_READY then,
	 * the one state from which DROP is rejected ("DROP trigger: invalid
	 * state"). */
	static bool i2s_configured_once = false;

	if (i2s_configured_once) {
		(void)trigger_audio(STOP);
	}
	i2s_configured_once = true;
	i2s_config(sampling_rate);
#else
	int ret = pdm_ch_config(mic, sampling_rate);
	if (ret != 0) {
		return -1;
	}
#endif
	/* Reset session state. audio_stop_requested must be cleared so the
	 * freshly created worker does not see a stale 'stop' from a previous
	 * session and exit immediately. */
	audio_stop_requested = false;
	audio_status = 0;
	user_ptr = NULL;
	user_len = 0;

	k_sem_init(&rx_start, 0, 1);
	k_sem_init(&rx_ready, 0, 1);

	k_thread_create(&audio_thread, audio_thread_stack,
			K_THREAD_STACK_SIZEOF(audio_thread_stack), audio_worker_thread, NULL, NULL,
			NULL, THREAD_PRIORITY, 0, K_NO_WAIT);

	k_thread_name_set(&audio_thread, "input audio");

	return 0;
}

void audio_uninit(void)
{
	/* Ask the worker to stop and wake it if it is waiting for the next
	 * get_audio_data(). The mic is deliberately left running here: a worker
	 * blocked inside a mic read is only unblocked by the next buffer being
	 * delivered, so the read returns promptly, the worker sees the stop
	 * request, releases the buffer and exits. Stopping the mic first would
	 * instead halt the data flow and leave that read blocked forever,
	 * deadlocking the join below. */
	audio_stop_requested = true;
	k_sem_give(&rx_start);
	k_thread_join(&audio_thread, K_FOREVER);

	/* The worker has exited, so it is now safe to stop the mic and clear the
	 * shared state. trigger_audio() uses DROP for I2S, which returns the
	 * stream to READY from any active state (RUNNING or ERROR) and frees any
	 * buffers still queued by the driver. */
	int rc = trigger_audio(STOP);
	if (rc < 0) {
		LOG_ERR("mic stop failed: %d", rc);
	}
	user_ptr = NULL;
	user_len = 0;
	k_sem_reset(&rx_start);
	k_sem_reset(&rx_ready);
}

int get_audio_data(int16_t *data, int len)
{
	user_ptr = data;
	user_len = len;
	k_sem_give(&rx_start);

	return 0;
}

int wait_for_audio(void)
{
	k_sem_take(&rx_ready, K_FOREVER);

	return audio_status;
}

void audio_preprocessing(int16_t *data, int len)
{
#if I2S_MICS
	for (int i = 0; i < len; ++i) {
		data[i] = data[i] * I2S_GAIN;
	}
#endif
}

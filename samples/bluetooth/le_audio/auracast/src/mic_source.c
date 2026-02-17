/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>
#include <zephyr/audio/dmic.h>
#include <zephyr/drivers/pdm/pdm_alif.h>

#include "gapi_isooshm.h"
#include "bluetooth/le_audio/audio_queue.h"
#include "bluetooth/le_audio/audio_encoder.h"
#include "bluetooth/le_audio/audio_source_i2s.h"
#include "mic_source.h"

static const uint32_t fir[18] = {0x00000001, 0x00000003, 0x00000003, 0x000007F4, 0x00000004,
				 0x000007ED, 0x000007F5, 0x000007F4, 0x000007D3, 0x000007FE,
				 0x000007BC, 0x000007E5, 0x000007D9, 0x00000793, 0x00000029,
				 0x0000072C, 0x00000072, 0x000002FD};

struct mic_source_env {
	const struct device *dev;
	struct audio_queue *audio_queue;
	bool capture;
};

static struct mic_source_env mic_source;

#if CONFIG_ALIF_BLE_AUDIO_USE_RAMFUNC
#define INT_RAMFUNC __ramfunc
#else
#define INT_RAMFUNC
#endif

#define INPUT_LEVEL_CALC(_s)   (((int)(_s)*CONFIG_INPUT_VOLUME_LEVEL) / 100)
#define NUMBER_OF_MIC_CHANNELS 2

LOG_MODULE_DECLARE(audio_datapath, CONFIG_BLE_AUDIO_LOG_LEVEL);

INT_RAMFUNC static void audio_encoder_mixer_thread_func(void *p1, void *p2, void *p3)
{
	/* thread:
	 * get audio input jack buffer
	 * if pdm_mic has data available
	 *     lower input audio gain
	 *     mix mic and audio input samples
	 * end
	 * free mic data
	 * push audio samples to LC3 encoder
	 * free audio input buffer
	 */

	struct audio_queue *audio_queue_in1 = p1; /* input I2S codec (WM8904) */
	struct audio_queue *audio_queue_in2 = p2; /* input PDM MIC */
	struct audio_queue *audio_queue_out = p3; /* output (to LC3 encoder) */
	struct audio_block *audio_in1;
	struct audio_block *audio_out;
	int ret;
	size_t size;
	void *buffer = NULL;

	LOG_DBG("Mixer thread started");

	while (1) {

		audio_out = audio_in1 = NULL;
		ret = k_msgq_get(&audio_queue_in1->msgq, &audio_in1, K_FOREVER);
		if (ret || !audio_in1) {
			continue;
		}

		ret = dmic_read(mic_source.dev, 0, &buffer, &size, 0);

		/* Do mixing... */
		if (!ret && mic_source.capture) {
			size_t const samples = size / sizeof(pcm_sample_t); /* stereo interleaved */
			pcm_sample_t *p_mic_data = buffer;
			bool const mic_has_right_channel = !!(audio_in1->num_channels > 1);

			pcm_sample_t *p_input_left = audio_in1->channels[0];
			pcm_sample_t *p_input_right =
				audio_in1->num_channels > 1 ? audio_in1->channels[1] : NULL;
			pcm_sample_t data;

			for (size_t sample = 0; sample < samples; sample++) {
				if ((sample & 1) && mic_has_right_channel) { /* Right channel */
					data = *p_mic_data++;
					if (p_input_right) {
						*p_input_right =
							data + INPUT_LEVEL_CALC(*p_input_right);
						p_input_right++;
					}
				} else { /* Left channel */
					data = *p_mic_data++;
					*p_input_left = data + INPUT_LEVEL_CALC(*p_input_left);
					p_input_left++;
				}
			}
		}
		if (!ret && buffer) {
			k_mem_slab_free(&audio_queue_in2->slab, buffer);
			buffer = NULL;
		}

		ret = k_mem_slab_alloc(&audio_queue_out->slab, (void **)&audio_out, K_NO_WAIT);
		if (ret || !audio_out) {
			LOG_ERR("Failed to allocate audio output block");
			k_mem_slab_free(&audio_queue_in1->slab, audio_in1);
			continue;
		}

		/* Copy in1 to out */
		memcpy(audio_out, audio_in1, sizeof(*audio_out));

		ret = k_msgq_put(&audio_queue_out->msgq, &audio_out, K_NO_WAIT);
		if (ret) {
			k_mem_slab_free(&audio_queue_out->slab, audio_out);
		}
		k_mem_slab_free(&audio_queue_in1->slab, audio_in1);
	}
}

static int16_t fs_to_pdm_mode(uint32_t fs)
{
	switch (fs) {
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

static int configure_pdm_source(const struct device *pdm_dev, struct audio_queue *audio_queue_mic)
{
	int ret;
	struct dmic_cfg cfg = {0};
	struct pcm_stream_cfg stream = {0};
	struct pdm_ch_config pdm_coef_reg = {0};
	int16_t pdm_mode_val = fs_to_pdm_mode(audio_queue_mic->sampling_freq_hz);

	if (pdm_mode_val < 0) {
		LOG_ERR("Unsupported sampling frequency %u", audio_queue_mic->sampling_freq_hz);
		return -EINVAL;
	}

	memcpy(pdm_coef_reg.ch_fir_coef, fir, sizeof(pdm_coef_reg.ch_fir_coef));
	pdm_coef_reg.ch_iir_coef = CONFIG_AUDIO_IIR_COEF;

	cfg.streams = &stream;
	cfg.streams[0].mem_slab = &audio_queue_mic->slab;
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
	pdm_set_ch_phase(pdm_dev, CONFIG_AUDIO_L_CHANNEL, CONFIG_AUDIO_PDM_PHASE);
	pdm_set_ch_gain(pdm_dev, CONFIG_AUDIO_L_CHANNEL, (CONFIG_MICROPHONE_GAIN << 4));
	pdm_coef_reg.ch_num = CONFIG_AUDIO_L_CHANNEL;
	pdm_channel_config(pdm_dev, &pdm_coef_reg);

	/* Configure right channel */
	pdm_set_ch_gain(pdm_dev, CONFIG_AUDIO_R_CHANNEL, (CONFIG_MICROPHONE_GAIN << 4));
	pdm_set_ch_phase(pdm_dev, CONFIG_AUDIO_R_CHANNEL, CONFIG_AUDIO_PDM_PHASE);
	pdm_coef_reg.ch_num = CONFIG_AUDIO_R_CHANNEL;
	pdm_channel_config(pdm_dev, &pdm_coef_reg);

	pdm_mode(pdm_dev, pdm_mode_val);

	mic_source.dev = pdm_dev;
	mic_source.audio_queue = audio_queue_mic;

	ret = dmic_trigger(pdm_dev, DMIC_TRIGGER_START);
	if (ret) {
		LOG_ERR("Failed to start DMIC");
		return ret;
	}

	return 0;
}

int mic_configure(const struct device *mic_dev, const struct device *i2s_dev,
		  struct audio_encoder *audio_encoder)
{
	if (!mic_dev || !i2s_dev || !audio_encoder) {
		return -EINVAL;
	}

	/*
	 * Two input queues are used for audio mixing:
	 *   - audio_queue_i2s: receives audio from the audio jack via I2S
	 *   - audio_queue_mic: receives audio from the PDM microphone
	 *
	 * Configured (audio encoder input) audio I2S input queue will be changed
	 * to audio_queue_i2s and the created thread will mix audio_queue_mic
	 * into audio_queue_i2s. Result will be copied and pushed to
	 * audio_queue_current (audio encoder input queue).
	 */

	struct audio_queue *audio_queue_current = audio_encoder_audio_queue_get(audio_encoder);

	if (!audio_queue_current) {
		LOG_ERR("Failed to get audio queue");
		return -ENODEV;
	}

	struct audio_queue *audio_queue_mic, *audio_queue_i2s;

	audio_queue_mic = audio_queue_create(audio_queue_current->item_count,
					     audio_queue_current->sampling_freq_hz,
					     audio_queue_current->frame_duration_us);

	if (!audio_queue_mic) {
		LOG_ERR("Failed to create audio queue");
		return -ENOMEM;
	}

	audio_queue_i2s = audio_queue_create(audio_queue_current->item_count,
					     audio_queue_current->sampling_freq_hz,
					     audio_queue_current->frame_duration_us);

	if (!audio_queue_i2s) {
		audio_queue_delete(audio_queue_mic);
		LOG_ERR("Failed to create audio queue");
		return -ENOMEM;
	}

	int ret = audio_source_i2s_configure(i2s_dev, audio_queue_i2s);

	if (ret != 0) {
		audio_queue_delete(audio_queue_i2s);
		audio_queue_delete(audio_queue_mic);
		LOG_ERR("Failed to configure audio source I2S, err %d", ret);
		return ret;
	}

	ret = configure_pdm_source(mic_dev, audio_queue_mic);
	if (ret != 0) {
		audio_queue_delete(audio_queue_i2s);
		audio_queue_delete(audio_queue_mic);
		LOG_ERR("Failed to configure mic, err %d", ret);
		return ret;
	}
	static struct k_thread mixer_thread;

	static K_THREAD_STACK_DEFINE(mixer_thread_stack, 2048);

	k_tid_t tid = k_thread_create(
		&mixer_thread, mixer_thread_stack, K_THREAD_STACK_SIZEOF(mixer_thread_stack),
		audio_encoder_mixer_thread_func, audio_queue_i2s, audio_queue_mic,
		audio_queue_current, CONFIG_ALIF_BLE_HOST_THREAD_PRIORITY + 1, 0, K_NO_WAIT);

	if (!tid) {
		audio_queue_delete(audio_queue_i2s);
		audio_queue_delete(audio_queue_mic);
		LOG_ERR("Failed to create mixer thread");
		return -EINVAL;
	}

	k_thread_name_set(tid, "mixer");

	return 0;
}

void mic_start(void)
{
	if (!mic_source.dev) {
		return;
	}

	mic_source.capture = true;
}

void mic_stop(void)
{
	if (!mic_source.dev) {
		return;
	}

	mic_source.capture = false;
}

void mic_control(bool const start)
{
	if (!mic_source.dev) {
		return;
	}

	if (start) {
		LOG_INF("MIC start...");
		mic_start();
		return;
	}

	LOG_INF("MIC stop...");
	mic_stop();
}

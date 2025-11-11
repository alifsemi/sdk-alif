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
#include <zephyr/sys/__assert.h>
#include <zephyr/sys/util.h>
#include "drivers/i2s_sync.h"
#include "gapi_isooshm.h"

#include "bluetooth/le_audio/audio_queue.h"
#include "bluetooth/le_audio/audio_encoder.h"
#include "bluetooth/le_audio/audio_source_i2s.h"
#include "mic_source.h"

struct mic_source_env {
	const struct device *dev;
	struct audio_queue *audio_queue;
	size_t block_size;
	size_t number_of_channels;
	bool started;
	bool capture;
};

static struct mic_source_env mic_source;

#if CONFIG_ALIF_BLE_AUDIO_USE_RAMFUNC
#define INT_RAMFUNC __ramfunc
#else
#define INT_RAMFUNC
#endif

#define MIC_LEVEL_CALC(_s)   (((int)(_s) * CONFIG_MICROPHONE_GAIN) / 100)
#define INPUT_LEVEL_CALC(_s) (((int)(_s) * CONFIG_INPUT_VOLUME_LEVEL) / 100)

LOG_MODULE_DECLARE(audio_datapath, CONFIG_BLE_AUDIO_LOG_LEVEL);

INT_RAMFUNC static void recv_next_block(const struct device *dev)
{
	struct audio_block *p_block = NULL;
	int ret = k_mem_slab_alloc(&mic_source.audio_queue->slab, (void **)&p_block, K_NO_WAIT);

	if (ret || !p_block) {
		return;
	}

	i2s_sync_recv(dev, p_block->channels, mic_source.block_size);

	p_block->timestamp = 0;
	p_block->num_channels = mic_source.number_of_channels;
}

INT_RAMFUNC static void on_data_received(const struct device *dev,
					 const enum i2s_sync_status status, void *block)
{
	ARG_UNUSED(status);

	recv_next_block(dev);

	if (!block) {
		return;
	}

	struct audio_block *p_block = CONTAINER_OF(block, struct audio_block, channels);

	if (!mic_source.capture) {
		/* Just ignore the block */
		k_mem_slab_free(&mic_source.audio_queue->slab, p_block);
		return;
	}

	if (k_msgq_put(&mic_source.audio_queue->msgq, &p_block, K_NO_WAIT)) {
		/* Failed to put into queue */
		k_mem_slab_free(&mic_source.audio_queue->slab, p_block);
	}
}

INT_RAMFUNC static void audio_encoder_mixer_thread_func(void *p1, void *p2, void *p3)
{
	/* thread:
	 * get audio input jack buffer
	 * if audio_queue_mic has data available
	 *     adjust mic gain
	 *     lower input audio gain
	 *     mix mic and audio input samples
	 *     free mic data
	 * end
	 * push audio samples to LC3 encoder
	 * free audio input buffer
	 */

	struct audio_queue *audio_queue_in1 = p1; /* input I2S codec (WM8904) */
	struct audio_queue *audio_queue_in2 = p2; /* input I2S MIC */
	struct audio_queue *audio_queue_out = p3; /* output (to LC3 encoder) */
	struct audio_block *audio_in1;
	struct audio_block *audio_in2;
	struct audio_block *audio_out;
	int ret;

	LOG_DBG("Mixer thread started");

	while (1) {
		audio_out = audio_in2 = audio_in1 = NULL;
		ret = k_msgq_get(&audio_queue_in1->msgq, &audio_in1, K_FOREVER);
		if (ret || !audio_in1) {
			continue;
		}

		ret = k_msgq_get(&audio_queue_in2->msgq, &audio_in2, K_NO_WAIT);
		if (!ret && audio_in2) {
			/* Do mixing... */
			size_t const samples =
				audio_queue_in2->audio_block_samples * audio_in2->num_channels;
			pcm_sample_t *p_mic_data = audio_in2->channels[0];
			bool const mic_has_right_channel = audio_in2->num_channels > 1;

			pcm_sample_t *p_input_left = audio_in1->channels[0];
			pcm_sample_t *p_input_right =
				audio_in1->num_channels > 1 ? audio_in1->channels[1] : NULL;
			pcm_sample_t data;

			for (size_t sample = 0; sample < samples; sample++) {
				data = *p_mic_data++;
				data = MIC_LEVEL_CALC(data);

				if ((sample & 1) && mic_has_right_channel) { /* Right channel */
					if (p_input_right) {
						*p_input_right =
							data + INPUT_LEVEL_CALC(*p_input_right);
						p_input_right++;
					}
				} else { /* Left channel */
					*p_input_left = data + INPUT_LEVEL_CALC(*p_input_left);
					p_input_left++;
				}
			}
			k_mem_slab_free(&audio_queue_in2->slab, audio_in2);
		}

		ret = k_mem_slab_alloc(&audio_queue_out->slab, (void **)&audio_out, K_NO_WAIT);
		if (ret || !audio_out) {
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

static int configure_i2s_sync(const struct device *dev, struct audio_queue *audio_queue)
{
	if (!dev || !audio_queue) {
		return -EINVAL;
	}

	struct i2s_sync_config i2s_cfg;

	if (i2s_sync_get_config(dev, &i2s_cfg)) {
		return -EIO;
	}

	if (i2s_cfg.channel_count > MAX_NUMBER_OF_CHANNELS) {
		return -EINVAL;
	}

	i2s_cfg.sample_rate = audio_queue->sampling_freq_hz;
	if (i2s_sync_configure(dev, &i2s_cfg)) {
		return -EIO;
	}

	/* Shutdown existing stream and wait for start */
	i2s_sync_disable(dev, I2S_DIR_RX);

	mic_source.dev = dev;
	mic_source.audio_queue = audio_queue;
	mic_source.block_size =
		i2s_cfg.channel_count * audio_queue->audio_block_samples * sizeof(pcm_sample_t);
	mic_source.number_of_channels = i2s_cfg.channel_count;
	mic_source.started = false;
	mic_source.capture = false;

	int ret = i2s_sync_register_cb(dev, I2S_DIR_RX, on_data_received);

	if (ret) {
		return ret;
	}

	return 0;
}

int mic_i2s_configure(const struct device *i2s_mic_dev, const struct device *i2s_dev,
		      struct audio_encoder *audio_encoder)
{
	if (!i2s_mic_dev || !i2s_dev || !audio_encoder) {
		return -EINVAL;
	}

	/*
	 * Two I2S input queues are used:
	 *   - audio jack I2S input (audio_queue_i2s)
	 *   - mic I2S input (audio_queue_mic)
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

	ret = configure_i2s_sync(i2s_mic_dev, audio_queue_mic);
	if (ret != 0) {
		audio_queue_delete(audio_queue_i2s);
		audio_queue_delete(audio_queue_mic);
		LOG_ERR("Failed to configure mic I2S, err %d", ret);
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

void mic_i2s_start(void)
{
	if (!mic_source.dev) {
		return;
	}

	/* Kick the I2S receive operation */
	if (!mic_source.started) {
		recv_next_block(mic_source.dev);
		mic_source.started = true;
	}

	mic_source.capture = true;
}

void mic_i2s_stop(void)
{
	if (!mic_source.dev) {
		return;
	}

	if (!mic_source.started) {
		return;
	}

	mic_source.capture = false;
}

void mic_i2s_control(bool const start)
{
	if (!mic_source.dev) {
		return;
	}

	if (start) {
		LOG_INF("MIC start...");
		mic_i2s_start();
		return;
	}

	LOG_INF("MIC stop...");
	mic_i2s_stop();
}

/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
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
#include "drivers/i2s_sync.h"
#include "gapi_isooshm.h"
#include "audio_i2s_common.h"
#include "audio_source_i2s.h"

LOG_MODULE_REGISTER(audio_source_i2s, CONFIG_BLE_AUDIO_LOG_LEVEL);

struct audio_source_i2s {
	const struct device *dev;
	struct audio_queue *audio_queue;
	struct audio_block *current_block;
	bool awaiting_buffer;
	uint32_t i2s_overrun_count;
	uint32_t buffer_not_available_count;
	uint32_t msgq_dropped_count;

	struct audio_i2s_timing timing;
	bool drop_next_audio_block;
};

static struct audio_source_i2s audio_source;
static int16_t temp[960];

static void finish_last_block(void)
{
	if (audio_source.current_block == NULL) {
		return;
	}

	if (audio_source.drop_next_audio_block) {
		audio_source.drop_next_audio_block = false;
		k_mem_slab_free(&audio_source.audio_queue->slab,
				audio_source.current_block);
		audio_source.current_block = NULL;
		return;
	}

	/* We need to split the received audio into left and right channel so
	 * we create a new block where we do this
	 */

	bool mono_mode = IS_ENABLED(CONFIG_BROADCAST_SOURCE_MONO);
	/* const size_t block_samples = audio_source.audio_queue->audio_block_samples */
	const size_t block_samples = mono_mode ? 480 : 960;
	uint8_t div = mono_mode ? 1 : 2;

	for (size_t i = 0; i < block_samples/div; i++) {
		temp[i] = audio_source.current_block->buf[i * 2];
		if (!mono_mode) {
			temp[block_samples/div + i] =
			audio_source.current_block->buf[i * 2 + 1];
		}
	}

	memcpy(audio_source.current_block->buf, temp, block_samples * 2);

	int ret =
		k_msgq_put(&audio_source.audio_queue->msgq, &audio_source.current_block, K_NO_WAIT);
		audio_source.current_block = NULL;

	if (ret) {
		audio_source.msgq_dropped_count++;
	}
}

static void recv_next_block(const struct device *dev, uint32_t timestamp)
{
	int ret = k_mem_slab_alloc(&audio_source.audio_queue->slab,
				   (void **)&audio_source.current_block, K_NO_WAIT);

	if (ret) {
		/* If there is no available buffer to receive into, disable the I2S receiver and
		 * flag that we are awaiting a buffer to become available
		 */
		i2s_sync_disable(dev, I2S_DIR_RX);
		audio_source.awaiting_buffer = true;
		audio_source.buffer_not_available_count++;
		return;
	}

	int32_t correction_samples = audio_i2s_get_sample_correction(NULL);

	size_t rx_count;
	size_t rx_offset;

	if (correction_samples == 0) {
		/* Receive a full block */
		rx_count = audio_source.audio_queue->audio_block_samples;
		rx_offset = 0;
	} else if (correction_samples < 0) {
		/* Drop some samples, by receiving the amount of samples to drop and then dropping
		 * the whole buffer
		 */
		rx_count = -correction_samples;
		rx_offset = 0;
		audio_source.drop_next_audio_block = true;
	} else {
		/* Insert some zeros at the start of the current block, and receive into the rest of
		 * the block
		 */
		rx_count = audio_source.audio_queue->audio_block_samples - correction_samples;
		rx_offset = correction_samples;
	}

	/* Populate the capture timestamp of the block */
	audio_source.current_block->timestamp = timestamp;
	i2s_sync_recv(dev, audio_source.current_block->buf + rx_offset, rx_count * sizeof(int16_t));

	/* LOG_INF("start of i2s buf %04x", (audio_source.current_block->buf)[1]); */

	/* Fill any offset in the buffer with zeros, and adjust timestamp accordingly */
	if (rx_offset) {
		memset(audio_source.current_block->buf, 0, rx_offset * sizeof(int16_t));
		audio_source.current_block->timestamp -= audio_i2s_samples_to_us(
			correction_samples, audio_source.timing.us_per_block,
			audio_source.timing.samples_per_block);
	}
}

static void on_i2s_complete(const struct device *dev, enum i2s_sync_status status)
{
	/* Capture timestamp before doing anything else to reduce jitter */
	uint32_t time_now = gapi_isooshm_dp_get_local_time();

	finish_last_block();
	recv_next_block(dev, time_now);

	if (status != I2S_SYNC_STATUS_OK) {
		audio_source.i2s_overrun_count++;
	}
}

int audio_source_i2s_configure(const struct device *dev, struct audio_queue *audio_queue,
			       uint32_t us_per_block)
{
	if ((dev == NULL) || (audio_queue == NULL)) {
		return -EINVAL;
	}

	audio_source.dev = dev;
	audio_source.audio_queue = audio_queue;
	audio_source.timing.correction_us = 0;
	audio_source.timing.us_per_block = us_per_block;
	audio_source.timing.samples_per_block = audio_queue->audio_block_samples;

	/* Maximum positive correction is slightly less than a full audio block (cannot receive zero
	 * samples)
	 */
	audio_source.timing.max_single_correction = audio_queue->audio_block_samples - 2;

	/* Minimum negative correction is one full audio block */
	audio_source.timing.min_single_correction = -audio_queue->audio_block_samples;

	int ret = i2s_sync_register_cb(dev, I2S_DIR_RX, on_i2s_complete);

	if (ret) {
		LOG_ERR("Failed to register I2S callback");
		return ret;
	}

	/* Flag that audio source has not started yet */
	audio_source.awaiting_buffer = true;

	return 0;
}

void audio_source_i2s_notify_buffer_available(void *param, uint32_t timestamp, uint16_t sdu_seq)
{
	(void)param;
	(void)timestamp;
	(void)sdu_seq;

	if (!audio_source.awaiting_buffer) {
		return;
	}

	/* Kick off the I2S receive operation */
	uint32_t time_now = gapi_isooshm_dp_get_local_time();

	audio_source.awaiting_buffer = false;
	recv_next_block(audio_source.dev, time_now);
}

void audio_source_i2s_apply_timing_correction(int32_t correction_us)
{
	audio_i2s_timing_apply_correction(&audio_source.timing, correction_us);
}

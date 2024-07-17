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
#include "presentation_compensation.h"
#include "audio_i2s_common.h"
#include "audio_sink_i2s.h"

LOG_MODULE_REGISTER(audio_sink_i2s, CONFIG_BLE_AUDIO_LOG_LEVEL);

#define SILENCE_SAMPLES 480

struct audio_sink_i2s {
	const struct device *dev;
	struct audio_queue *audio_queue;
	struct audio_block *current_block;
	uint32_t i2s_underrun_count;
	bool awaiting_buffer;

	struct audio_i2s_timing timing;
};

struct pres_delay_work {
	struct k_work work;
	uint32_t pres_delay_us;
};

static struct audio_sink_i2s audio_sink;
static struct pres_delay_work pd_work;

static int16_t silence[SILENCE_SAMPLES];

static void finish_last_block(void)
{
	if (audio_sink.current_block == NULL) {
		return;
	}

	k_mem_slab_free(&audio_sink.audio_queue->slab, audio_sink.current_block);
	audio_sink.current_block = NULL;
}

static void send_next_block(const struct device *dev, uint32_t time_now)
{
	int32_t correction_samples = audio_i2s_get_sample_correction(&audio_sink.timing);

	/* Send required size of silence and return */
	if (correction_samples > 0) {
		i2s_sync_send(dev, silence, correction_samples * sizeof(int16_t));
		return;
	}

	int ret = k_msgq_get(&audio_sink.audio_queue->msgq, &audio_sink.current_block, K_NO_WAIT);

	if (ret) {
		/* If there is no available buffer, disable I2S transmitter and flag waiting for
		 * data
		 */
		i2s_sync_disable(dev, I2S_DIR_TX);
		audio_sink.i2s_underrun_count++;
		audio_sink.awaiting_buffer = true;
		return;
	}

	size_t rx_count = audio_sink.audio_queue->audio_block_samples;
	size_t rx_offset = 0;
	uint32_t pres_delay_offset = 0;

	/* If necessary drop some samples from the start of the buffer */
	if (correction_samples < 0) {
		rx_count += correction_samples;
		rx_offset = -correction_samples;
		pres_delay_offset =
			audio_i2s_samples_to_us(-correction_samples, audio_sink.timing.us_per_block,
						audio_sink.timing.samples_per_block);
	}

	i2s_sync_send(dev, audio_sink.current_block->buf + rx_offset, rx_count * sizeof(int16_t));

	/* Calculate presentation delay, and then sumbit a work item to perform presentation
	 * compensation calculations so that this is deferred and not performed in ISR context
	 */
	pd_work.pres_delay_us = time_now - audio_sink.current_block->timestamp - pres_delay_offset;
	k_work_submit(&pd_work.work);
}

static void on_i2s_complete(const struct device *dev, enum i2s_sync_status status)
{
	/* Capture timestamp before doing anything else to reduce jitter */
	uint32_t time_now = gapi_isooshm_dp_get_local_time();

	finish_last_block();
	send_next_block(dev, time_now);
}

static void submit_presentation_delay(struct k_work *item)
{
	struct pres_delay_work *w = CONTAINER_OF(item, struct pres_delay_work, work);

	presentation_compensation_notify_timing(w->pres_delay_us);
}

int audio_sink_i2s_configure(const struct device *dev, struct audio_queue *audio_queue,
			     uint32_t us_per_block)
{
	if ((dev == NULL) || (audio_queue == NULL)) {
		return -EINVAL;
	}

	audio_sink.dev = dev;
	audio_sink.audio_queue = audio_queue;
	audio_sink.timing.correction_us = 0;
	audio_sink.timing.us_per_block = us_per_block;
	audio_sink.timing.samples_per_block = audio_queue->audio_block_samples;

	/* Maximum positive correction is the size of the silence buffer */
	audio_sink.timing.max_single_correction = SILENCE_SAMPLES;

	/* Minimum negative correction is slightly less than a full audio block (cannot send zero
	 * samples)
	 */
	audio_sink.timing.min_single_correction = 2 - (int32_t)audio_queue->audio_block_samples;

	k_work_init(&pd_work.work, submit_presentation_delay);

	int ret = i2s_sync_register_cb(dev, I2S_DIR_TX, on_i2s_complete);

	if (ret) {
		LOG_ERR("Failed to register I2S callback");
		return ret;
	}

	/* Flag that audio sink has not started yet */
	audio_sink.awaiting_buffer = true;

	return 0;
}

void audio_sink_i2s_notify_buffer_available(void *param, uint32_t timestamp, uint16_t sdu_seq)
{
	(void)param;
	(void)timestamp;
	(void)sdu_seq;

	if (!audio_sink.awaiting_buffer) {
		return;
	}

	uint32_t time_now = gapi_isooshm_dp_get_local_time();

	audio_sink.awaiting_buffer = false;
	send_next_block(audio_sink.dev, time_now);
}

void audio_sink_i2s_apply_timing_correction(int32_t correction_us)
{
	audio_i2s_timing_apply_correction(&audio_sink.timing, correction_us);
}

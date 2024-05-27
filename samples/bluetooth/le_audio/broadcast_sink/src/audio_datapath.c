/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/logging/log.h>
#include "bluetooth/le_audio/sdu_queue.h"
#include "bluetooth/le_audio/audio_queue.h"
#include "bluetooth/le_audio/iso_datapath_ctoh.h"
#include "bluetooth/le_audio/audio_decoder.h"
#include "bluetooth/le_audio/audio_sink_i2s.h"
#include "bluetooth/le_audio/presentation_compensation.h"
#include "audio_datapath.h"

LOG_MODULE_REGISTER(audio_datapath, CONFIG_BLE_AUDIO_LOG_LEVEL);

#define SDU_QUEUE_LENGTH          4
#define AUDIO_QUEUE_MARGIN_US     20000
#define CHANNEL_COUNT             2
#define AUDIO_BLOCK_SAMPLES       ((AUDIO_SAMPLING_RATE_HZ / AUDIO_FRAMES_PER_SECOND)\
				  * CHANNEL_COUNT)
#define MIN_PRESENTATION_DELAY_US 30000
#define MICROSECONDS_PER_SECOND   1000000

struct audio_datapath {
	struct sdu_queue *sdu_queue_l;
	struct sdu_queue *sdu_queue_r;
	struct iso_datapath_ctoh *iso_dp_l;
	struct iso_datapath_ctoh *iso_dp_r;
	struct audio_queue *audio_queue;
	struct audio_decoder *decoder;
};

K_THREAD_STACK_DEFINE(decoder_stack, CONFIG_LC3_DECODER_STACK_SIZE);

static struct audio_datapath env;

static void print_sdus(void *context, uint32_t timestamp, uint16_t sdu_seq)
{
	if (0 == (sdu_seq % 128)) {
		LOG_INF("SDU sequence number %u", sdu_seq);
	}
}

#ifdef CONFIG_PRESENTATION_COMPENSATION_DEBUG
void on_timing_debug_info_ready(struct presentation_comp_debug_data *dbg_data)
{
	LOG_INF("Presentation compensation debug data is ready");
}
#endif

int audio_datapath_create(struct audio_datapath_config *cfg)
{
	if (cfg == NULL) {
		return -EINVAL;
	}

	/* Presentation delay less than a certain value is impossible due to latency of audio
	 * datapath
	 */
	uint32_t pres_delay_us = MAX(cfg->pres_delay_us, MIN_PRESENTATION_DELAY_US);

	env.sdu_queue_l = sdu_queue_create(SDU_QUEUE_LENGTH, cfg->octets_per_frame);
	if (env.sdu_queue_l == NULL) {
		LOG_ERR("Failed to create SDU queue (left)");
		return -ENOMEM;
	}

	/* Stream LID is zero based on the assumption that the left channel is started first, and no
	 * other datapaths are already running concurrently to this module
	 */
	env.iso_dp_l = iso_datapath_ctoh_create(0, env.sdu_queue_l);
	if (env.iso_dp_l == NULL) {
		LOG_ERR("Failed to create ISO datapath (left)");
		return -ENOMEM;
	}

	if (cfg->stereo) {
		env.sdu_queue_r = sdu_queue_create(SDU_QUEUE_LENGTH, cfg->octets_per_frame);
		if (env.sdu_queue_r == NULL) {
			LOG_ERR("Failed to create SDU queue (right)");
			return -ENOMEM;
		}

		/* Stream LID is one based on the assumption that the right channel is started
		 * second, and no other datapaths are already running concurrently to this module
		 */
		env.iso_dp_r = iso_datapath_ctoh_create(1, env.sdu_queue_r);
		if (env.iso_dp_r == NULL) {
			LOG_ERR("Failed to create ISO datapath (right)");
			return -ENOMEM;
		}
	}

	size_t audio_queue_len_us = pres_delay_us + AUDIO_QUEUE_MARGIN_US;
	size_t audio_queue_len_blocks =
		audio_queue_len_us / (MICROSECONDS_PER_SECOND / AUDIO_FRAMES_PER_SECOND);

	env.audio_queue = audio_queue_create(audio_queue_len_blocks, AUDIO_BLOCK_SAMPLES);
	if (env.audio_queue == NULL) {
		LOG_ERR("Failed to create audio queue");
		return -ENOMEM;
	}

	env.decoder = audio_decoder_create(AUDIO_SAMPLING_RATE_HZ, decoder_stack,
					   CONFIG_LC3_DECODER_STACK_SIZE, env.sdu_queue_l,
					   env.sdu_queue_r, env.audio_queue);

	if (env.decoder == NULL) {
		LOG_ERR("Failed to create audio decoder");
		return -ENOMEM;
	}

	int ret = audio_sink_i2s_configure(cfg->i2s_dev, env.audio_queue,
					   MICROSECONDS_PER_SECOND / AUDIO_FRAMES_PER_SECOND);

	if (ret != 0) {
		LOG_ERR("Failed to configure audio sink I2S, err %d", ret);
		return ret;
	}

	ret = presentation_compensation_configure(cfg->mclk_dev, pres_delay_us);
	if (ret != 0) {
		LOG_ERR("Failed to configure presentation compensation module, err %d", ret);
		return ret;
	}

	/* Add presentation compensation callback to notify audio sink */
	ret = presentation_compensation_register_cb(audio_sink_i2s_apply_timing_correction);
	if (ret != 0) {
		LOG_ERR("Failed to register presentation compensation callback, err %d", ret);
		return ret;
	}

#ifdef CONFIG_PRESENTATION_COMPENSATION_DEBUG
	ret = presentation_compensation_register_debug_cb(on_timing_debug_info_ready);
	if (ret != 0) {
		LOG_ERR("Failed to register presentation compensation debug callback, err %d", ret);
		return ret;
	}
#endif

	ret = audio_decoder_register_cb(env.decoder, print_sdus, NULL);
	if (ret != 0) {
		LOG_ERR("Failed to register decoder cb, err %d", ret);
		return ret;
	}
	/* Add callback(s) to notify ISO datapath(s) */
	ret = audio_decoder_register_cb(env.decoder, iso_datapath_ctoh_notify_sdu_done,
					env.iso_dp_l);
	if (ret != 0) {
		LOG_ERR("Failed to register decoder cb, err %d", ret);
		return ret;
	}

	if (env.iso_dp_r) {
		ret = audio_decoder_register_cb(env.decoder, iso_datapath_ctoh_notify_sdu_done,
						env.iso_dp_r);
		if (ret != 0) {
			LOG_ERR("Failed to register decoder cb, err %d", ret);
			return ret;
		}
	}

	/* Add a callback to notify audio sink */
	ret = audio_decoder_register_cb(env.decoder, audio_sink_i2s_notify_buffer_available, NULL);
	if (ret != 0) {
		LOG_ERR("Failed to register decoder cb, err %d", ret);
		return ret;
	}

	LOG_DBG("Created audio datapath");

	return 0;
}

int audio_datapath_start(void)
{
	/* Notify that first buffer is available to start reception */
	iso_datapath_ctoh_notify_sdu_done(env.iso_dp_l, 0, 0);
	if (env.iso_dp_r) {
		iso_datapath_ctoh_notify_sdu_done(env.iso_dp_r, 0, 0);
	}

	LOG_INF("Audio datapath started");

	return 0;
}

int audio_datapath_cleanup(void)
{
	/* Stop and cleanup decoder thread first as it references other modules */
	if (env.decoder) {
		audio_decoder_delete(env.decoder);
	}

	/* Then delete ISO DP to stop any incoming SDUs */
	if (env.iso_dp_l) {
		iso_datapath_ctoh_delete(env.iso_dp_l);
	}

	if (env.iso_dp_r) {
		iso_datapath_ctoh_delete(env.iso_dp_r);
	}

	/* Then clean up queues */
	if (env.sdu_queue_l) {
		sdu_queue_delete(env.sdu_queue_l);
	}

	if (env.sdu_queue_r) {
		sdu_queue_delete(env.sdu_queue_r);
	}

	if (env.audio_queue) {
		audio_queue_delete(env.audio_queue);
	}

	LOG_DBG("Removed audio datapath");

	return 0;
}

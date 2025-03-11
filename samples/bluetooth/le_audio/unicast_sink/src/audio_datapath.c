/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/types.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include "bluetooth/le_audio/sdu_queue.h"
#include "bluetooth/le_audio/audio_queue.h"
#include "bluetooth/le_audio/iso_datapath_ctoh.h"
#include "bluetooth/le_audio/audio_decoder.h"
#include "bluetooth/le_audio/audio_sink_i2s.h"
#include "bluetooth/le_audio/presentation_compensation.h"
#include "audio_datapath.h"
#include "alif_lc3.h"

LOG_MODULE_REGISTER(audio_datapath, CONFIG_BLE_AUDIO_LOG_LEVEL);

#define SDU_QUEUE_LENGTH          12
#define AUDIO_QUEUE_MARGIN_US     20000
#define CHANNEL_COUNT             2
#define MIN_PRESENTATION_DELAY_US 30000
#define MICROSECONDS_PER_SECOND   1000000
/** Enable debug for SDU sequence number info */
#define DECODER_DEBUG             0

static struct audio_datapath {
	const struct device *i2s_dev;
	const struct device *codec_dev;
	/* const struct device *mclk_dev; */
	struct sdu_queue *sdu_queue[CONFIG_ALIF_BLE_AUDIO_NMB_CHANNELS];
	struct iso_datapath_ctoh *iso_dp[CONFIG_ALIF_BLE_AUDIO_NMB_CHANNELS];
	struct audio_queue *audio_queue;
	struct audio_decoder *decoder;
} env;

K_THREAD_STACK_DEFINE(decoder_stack, CONFIG_LC3_DECODER_STACK_SIZE);

/* Initialisation to perform pre-main */
static int unicast_audio_path_init(void)
{
	__ASSERT(!alif_lc3_init(), "Failed to initialise LC3 codec");

	const struct device *i2s_dev = DEVICE_DT_GET(I2S_NODE);
	const struct device *codec_dev = DEVICE_DT_GET(CODEC_NODE);

	/* Check all devices are ready */
	__ASSERT(device_is_ready(i2s_dev), "I2S is not ready");
	__ASSERT(device_is_ready(codec_dev), "Audio codec is not ready");
	/* __ASSERT(device_is_ready(mclk_gen_dev), "MCLK device is not ready"); */

	env.i2s_dev = i2s_dev;
	env.codec_dev = codec_dev;

	return 0;
}
SYS_INIT(unicast_audio_path_init, APPLICATION, 0);

#if DECODER_DEBUG
static void print_sdus(void *context, uint32_t timestamp, uint16_t sdu_seq)
{
	static uint16_t last_sdu;

	if (last_sdu && (last_sdu + 1 < sdu_seq)) {
		LOG_INF("SDU sequence number jumps, last: %u, now: %u", last_sdu, sdu_seq);
	}
	last_sdu = sdu_seq;

	if (0 == (sdu_seq % 128)) {
		LOG_INF("SDU sequence number %u", sdu_seq);
	}
}
#endif

#ifdef CONFIG_PRESENTATION_COMPENSATION_DEBUG
void on_timing_debug_info_ready(struct presentation_comp_debug_data *dbg_data)
{
	LOG_INF("Presentation compensation debug data is ready");
}
#endif

int audio_datapath_create_channel(size_t const octets_per_frame, uint8_t const ch_index)
{
	if (ARRAY_SIZE(env.sdu_queue) <= ch_index) {
		LOG_ERR("Invalid channel index %u", ch_index);
		return -EINVAL;
	}

	struct sdu_queue *queue = env.sdu_queue[ch_index];

	sdu_queue_delete(queue);
	iso_datapath_ctoh_delete(env.iso_dp[ch_index]);

	env.iso_dp[ch_index] = NULL;
	env.sdu_queue[ch_index] = queue = sdu_queue_create(SDU_QUEUE_LENGTH, octets_per_frame);
	if (queue == NULL) {
		LOG_ERR("Failed to create SDU queue (index %u)", ch_index);
		return -ENOMEM;
	}
	return 0;
}

int audio_datapath_create(struct audio_datapath_config *cfg)
{
	if (cfg == NULL) {
		return -EINVAL;
	}

	uint8_t num_valid_queues = 0;

	for (size_t ch_index = 0; ch_index < ARRAY_SIZE(env.sdu_queue); ch_index++) {
		struct sdu_queue *queue = env.sdu_queue[ch_index];

		if (!queue) {
			continue;
		}

		num_valid_queues++;

		/* Stream LID is zero based on the assumption that the left channel is started
		 * first, and no other datapaths are already running concurrently to this module
		 */
		env.iso_dp[ch_index] = iso_datapath_ctoh_create(ch_index, queue);
		if (env.iso_dp[ch_index] == NULL) {
			LOG_ERR("Failed to create ISO datapath (index %u)", ch_index);
			return -ENOMEM;
		}
	}

	if (!num_valid_queues) {
		LOG_ERR("At least one SDU queue must be provided");
		return -EINVAL;
	}

	/* Presentation delay less than a certain value is impossible due to latency of audio
	 * datapath
	 */
	uint32_t const pres_delay_us = MAX(cfg->pres_delay_us, MIN_PRESENTATION_DELAY_US);
	/* For 10ms: 1000/10 = 100 fps, For 7.5ms: 1000/7.5 = 133.33... fps */
	size_t const frames_per_second = cfg->frame_duration_is_10ms ? 100 : 133;
	size_t const audio_queue_len_us = pres_delay_us + AUDIO_QUEUE_MARGIN_US;
	size_t const audio_queue_len_blocks =
		audio_queue_len_us / (MICROSECONDS_PER_SECOND / frames_per_second);

	/* Calculate block samples with higher precision for different sampling rates
	 * For 10ms frame: samples = sampling_rate * 0.01 * CHANNEL_COUNT
	 * For 7.5ms frame: samples = sampling_rate * 0.0075 * CHANNEL_COUNT
	 */
	size_t const block_samples =
		cfg->frame_duration_is_10ms
			? ((uint32_t)cfg->sampling_rate_hz * 10 * CHANNEL_COUNT) / 1000
			: ((uint32_t)cfg->sampling_rate_hz * 75 * CHANNEL_COUNT) / 10000;

	/* Remove old one and create a new */
	audio_queue_delete(env.audio_queue);
	env.audio_queue = audio_queue_create(audio_queue_len_blocks, block_samples);
	if (env.audio_queue == NULL) {
		LOG_ERR("Failed to create audio queue");
		return -ENOMEM;
	}

	/* Remove old one and create a new */
	audio_decoder_delete(env.decoder);
	env.decoder = audio_decoder_create(
		cfg->sampling_rate_hz, decoder_stack, CONFIG_LC3_DECODER_STACK_SIZE, env.sdu_queue,
		num_valid_queues, env.audio_queue,
		cfg->frame_duration_is_10ms ? AUDIO_DECODER_FRAME_10MS
					    : AUDIO_DECODER_FRAME_7_5_MS);

	if (env.decoder == NULL) {
		LOG_ERR("Failed to create audio decoder");
		return -ENOMEM;
	}

	int ret = audio_sink_i2s_configure(env.i2s_dev, env.audio_queue,
					   MICROSECONDS_PER_SECOND / frames_per_second);

	if (ret != 0) {
		LOG_ERR("Failed to configure audio sink I2S, err %d", ret);
		return ret;
	}

	/* TODO: Change MCLK to use alif clock control */

	/* ret = presentation_compensation_configure(cfg->mclk_dev, pres_delay_us);
	 * if (ret != 0) {
	 *	LOG_ERR("Failed to configure presentation compensation module, err %d", ret);
	 *	return ret;
	 * }
	 */

	/* Add presentation compensation callback to notify audio sink */
	/* ret = presentation_compensation_register_cb(audio_sink_i2s_apply_timing_correction);
	 * if (ret != 0) {
	 *	LOG_ERR("Failed to register presentation compensation callback, err %d", ret);
	 *	return ret;
	 * }
	 */

#ifdef CONFIG_PRESENTATION_COMPENSATION_DEBUG
	/* ret = presentation_compensation_register_debug_cb(on_timing_debug_info_ready);
	 * if (ret != 0) {
	 *	LOG_ERR("Failed to register presentation compensation debug callback, err %d", ret);
	 *	return ret;
	 * }
	 */
#endif

#if DECODER_DEBUG
	ret = audio_decoder_register_cb(env.decoder, print_sdus, NULL);
	if (ret != 0) {
		LOG_ERR("Failed to register decoder cb, err %d", ret);
		return ret;
	}
#endif

	/* Add callback(s) to notify ISO datapath(s) */
	for (uint8_t iter = 0; iter < ARRAY_SIZE(env.iso_dp); iter++) {
		if (!env.iso_dp[iter]) {
			continue;
		}
		ret = audio_decoder_register_cb(env.decoder, iso_datapath_ctoh_notify_sdu_done,
						env.iso_dp[iter]);
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

	LOG_INF("Created audio datapath");

	return 0;
}

int audio_datapath_start(void)
{
	/* Notify that first buffer is available to start reception */
	for (uint8_t iter = 0; iter < ARRAY_SIZE(env.iso_dp); iter++) {
		if (env.iso_dp[iter]) {
			iso_datapath_ctoh_notify_sdu_done(env.iso_dp[iter], 0, 0);
		}
	}
	LOG_INF("Audio datapath started");
	return 0;
}

int audio_datapath_cleanup(void)
{
	/* Stop and cleanup decoder thread first as it references other modules */
	audio_decoder_delete(env.decoder);
	env.decoder = NULL;

	/* Then delete ISO DP to stop any incoming SDUs */
	for (uint8_t iter = 0; iter < ARRAY_SIZE(env.iso_dp); iter++) {
		iso_datapath_ctoh_delete(env.iso_dp[iter]);
		env.iso_dp[iter] = NULL;
	}

	/* Then clean up queues */
	for (uint8_t iter = 0; iter < ARRAY_SIZE(env.sdu_queue); iter++) {
		sdu_queue_delete(env.sdu_queue[iter]);
		env.sdu_queue[iter] = NULL;
	}

	audio_queue_delete(env.audio_queue);
	env.audio_queue = NULL;

	LOG_INF("Removed audio datapath");

	return 0;
}

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
#include "bluetooth/le_audio/audio_queue.h"
#include "bluetooth/le_audio/iso_datapath_ctoh.h"
#include "bluetooth/le_audio/iso_datapath_htoc.h"
#include "bluetooth/le_audio/audio_decoder.h"
#include "bluetooth/le_audio/audio_encoder.h"
#include "bluetooth/le_audio/audio_sink_i2s.h"
#include "bluetooth/le_audio/audio_source_i2s.h"
#include "bluetooth/le_audio/presentation_compensation.h"
#include "bluetooth/le_audio/sdu_queue.h"
#include "alif_lc3.h"
#include "audio_datapath.h"

LOG_MODULE_REGISTER(audio_datapath, CONFIG_BLE_AUDIO_LOG_LEVEL);

#define SDU_QUEUE_LENGTH          12
#define AUDIO_QUEUE_MARGIN_US     20000
#define CHANNEL_COUNT             2
#define MIN_PRESENTATION_DELAY_US 30000
#define MICROSECONDS_PER_SECOND   1000000

#define AUDIO_ENCODER_PATH 1
/* TODO: Enable for bidirectional audio data */
#define AUDIO_DECODER_PATH 0

struct audio_datapath {
	const struct device *i2s_dev;
	const struct device *codec_dev;
	/* const struct device *mclk_dev; */
#if AUDIO_ENCODER_PATH
	struct {
		struct sdu_queue *sdu_queue[CONFIG_ALIF_BLE_AUDIO_NMB_CHANNELS];
		struct iso_datapath_htoc *iso_dp[CONFIG_ALIF_BLE_AUDIO_NMB_CHANNELS];
		struct audio_encoder *p_encoder;
		struct audio_queue *p_audio_queue;
	} encoder;
#endif
#if AUDIO_DECODER_PATH
	struct {
		struct sdu_queue *sdu_queue[CONFIG_ALIF_BLE_AUDIO_NMB_CHANNELS];
		struct iso_datapath_ctoh *iso_dp[CONFIG_ALIF_BLE_AUDIO_NMB_CHANNELS];
		struct audio_decoder *p_decoder;
		struct audio_queue *p_audio_queue;
	} decoder;
#endif
};

static struct audio_datapath env;

#if AUDIO_ENCODER_PATH
K_THREAD_STACK_DEFINE(encoder_stack, CONFIG_LC3_ENCODER_STACK_SIZE);
#endif

#if AUDIO_DECODER_PATH
K_THREAD_STACK_DEFINE(decoder_stack, CONFIG_LC3_DECODER_STACK_SIZE);
#endif

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

#if AUDIO_ENCODER_PATH && ENCODER_DEBUG
static void on_encoder_frame_complete(void *param, uint32_t timestamp, uint16_t sdu_seq)
{
	if ((sdu_seq % 128) == 0) {
		LOG_INF("SDU sequence number: %u", sdu_seq);
	}
}
#endif

#if AUDIO_DECODER_PATH && DECODER_DEBUG
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

static int sdu_queue_init(struct sdu_queue **pp_queue, uint8_t const octets_per_frame)
{
	sdu_queue_delete(*pp_queue);
	*pp_queue = sdu_queue_create(SDU_QUEUE_LENGTH, octets_per_frame);
	if (*pp_queue == NULL) {
		return -ENOMEM;
	}
	return 0;
}

int audio_datapath_channel_create_source(size_t const octets_per_frame, uint8_t const stream_lid)
{
#if AUDIO_ENCODER_PATH
	int ret_val = 0;

	if (ARRAY_SIZE(env.encoder.sdu_queue) <= stream_lid) {
		LOG_ERR("Invalid channel index %u", stream_lid);
		return -EINVAL;
	}
	if (sdu_queue_init(&env.encoder.sdu_queue[stream_lid], octets_per_frame)) {
		LOG_ERR("Failed to create SDU queue (index %u)", stream_lid);
		return -ENOMEM;
	}
	LOG_INF("Created SDU queue (index %u)", stream_lid);
	return ret_val;
#else
	return -ENOTSUP;
#endif
}

int audio_datapath_create_source(struct audio_datapath_config *cfg)
{
#if AUDIO_ENCODER_PATH
	if (cfg == NULL) {
		return -EINVAL;
	}
	int ret;

	/* Presentation delay less than a certain value is impossible due to latency of audio
	 * datapath
	 * TODO: check presentation delay handling!
	 */
#if 1
	uint32_t const pres_delay_us = cfg->pres_delay_us;
#else
	uint32_t const pres_delay_us = MAX(cfg->pres_delay_us, MIN_PRESENTATION_DELAY_US);
#endif
	/* For 10ms: 1000/10 = 100 fps, For 7.5ms: 1000/7.5 = 133.33 fps */
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
	uint8_t num_valid_queues_enc = 0;

	for (ret = 0; ret < ARRAY_SIZE(env.encoder.sdu_queue); ret++) {
		num_valid_queues_enc += !!env.encoder.sdu_queue[ret];
	}

	if (!num_valid_queues_enc) {
		LOG_ERR("At least one SDU queue must be provided");
		return -EINVAL;
	}

	/* Remove old one and create a new */
	audio_queue_delete(env.encoder.p_audio_queue);
	env.encoder.p_audio_queue = audio_queue_create(audio_queue_len_blocks, block_samples);
	if (env.encoder.p_audio_queue == NULL) {
		LOG_ERR("Failed to create audio queue");
		return -ENOMEM;
	}

	/* Remove old one and create a new */
	audio_encoder_delete(env.encoder.p_encoder);
	env.encoder.p_encoder = audio_encoder_create(
		cfg->sampling_rate_hz, encoder_stack, CONFIG_LC3_ENCODER_STACK_SIZE,
		env.encoder.sdu_queue, num_valid_queues_enc, env.encoder.p_audio_queue,
		cfg->frame_duration_is_10ms ? AUDIO_ENCODER_FRAME_10MS
					    : AUDIO_ENCODER_FRAME_7_5_MS);
	if (env.encoder.p_encoder == NULL) {
		LOG_ERR("Failed to create audio encoder");
		return -ENOMEM;
	}

	ret = audio_source_i2s_configure(env.i2s_dev, env.encoder.p_audio_queue,
					 MICROSECONDS_PER_SECOND / frames_per_second);
	if (ret != 0) {
		LOG_ERR("Failed to configure audio source I2S, err %d", ret);
		return ret;
	}

#if ENCODER_DEBUG
	ret = audio_encoder_register_cb(env.encoder.p_encoder, on_encoder_frame_complete, NULL);
	if (ret) {
		LOG_ERR("Failed to register encoder cb for stats, err %d", ret);
		return ret;
	}
#endif

	ret = audio_encoder_register_cb(env.encoder.p_encoder,
					audio_source_i2s_notify_buffer_available, NULL);
	if (ret) {
		LOG_ERR("Failed to register encoder cb for audio source, err %d", ret);
		return ret;
	}
	LOG_INF("Created source audio datapath");

	return 0;
#else
	return -ENOTSUP;
#endif /* AUDIO_ENCODER_PATH */
}

int audio_datapath_start_source(void)
{
#if AUDIO_ENCODER_PATH
	int ret;
	struct sdu_queue *p_queue;
	struct iso_datapath_htoc *p_iso_dp;

	if (!env.encoder.p_encoder) {
		return -EINVAL;
	}

	for (size_t iter = 0; iter < ARRAY_SIZE(env.encoder.sdu_queue); iter++) {
		p_queue = env.encoder.sdu_queue[iter];
		if (!p_queue) {
			continue;
		}

		p_iso_dp = env.encoder.iso_dp[iter];
		iso_datapath_htoc_delete(p_iso_dp);
		p_iso_dp = iso_datapath_htoc_create(iter, p_queue, iter == 0);
		if (p_iso_dp == NULL) {
			LOG_ERR("Failed to create ISO datapath (index %u)", iter);
			return -ENOMEM;
		}

		ret = audio_encoder_register_cb(env.encoder.p_encoder,
						iso_datapath_htoc_notify_sdu_available, p_iso_dp);
		if (ret) {
			LOG_ERR("Failed to register encoder cb for left ISO datapath, err %d", ret);
			return ret;
		}
		env.encoder.iso_dp[iter] = p_iso_dp;
		LOG_INF("Audio encoder datapath created (index %u)", iter);
	}

	/* Start audio stream from I2S */
	audio_source_i2s_notify_buffer_available(NULL, 0, 0);
	LOG_INF("Audio source datapath started");
	return 0;
#else
	return -ENOTSUP;
#endif /* AUDIO_ENCODER_PATH */
}

int audio_datapath_cleanup_source(void)
{
#if AUDIO_ENCODER_PATH
	/* Stop encoder first as it references other modules */
	audio_encoder_delete(env.encoder.p_encoder);
	env.encoder.p_encoder = NULL;

	for (size_t iter = 0; iter < ARRAY_SIZE(env.encoder.iso_dp); iter++) {
		iso_datapath_htoc_delete(env.encoder.iso_dp[iter]);
		env.encoder.iso_dp[iter] = NULL;
	}

	for (size_t iter = 0; iter < ARRAY_SIZE(env.encoder.sdu_queue); iter++) {
		sdu_queue_delete(env.encoder.sdu_queue[iter]);
		env.encoder.sdu_queue[iter] = NULL;
	}

	audio_queue_delete(env.encoder.p_audio_queue);
	env.encoder.p_audio_queue = NULL;

	return 0;
#else
	return -ENOTSUP;
#endif /* AUDIO_ENCODER_PATH */
}

int audio_datapath_channel_create_sink(size_t const octets_per_frame, uint8_t const ch_index)
{
#if AUDIO_DECODER_PATH
	int ret_val = 0;

	if (ARRAY_SIZE(env.decoder.sdu_queue) <= ch_index) {
		LOG_ERR("Invalid channel index %u", ch_index);
		return -EINVAL;
	}

	struct sdu_queue *queue = env.decoder.sdu_queue[ch_index];

	if (sdu_queue_init(&queue, octets_per_frame)) {
		LOG_ERR("Failed to create SDU queue (index %u)", ch_index);
		return -ENOMEM;
	}

	/* Stream LID is zero based on the assumption that the left channel is started first, and no
	 * other datapaths are already running concurrently to this module
	 */
	struct iso_datapath_ctoh *iso_dp = env.decoder.iso_dp[ch_index];

	iso_datapath_ctoh_delete(iso_dp);
	iso_dp = iso_datapath_ctoh_create(ch_index, queue);
	if (iso_dp == NULL) {
		LOG_ERR("Failed to create ISO datapath (index %u)", ch_index);
		sdu_queue_delete(queue);
		queue = NULL;
		ret_val = -ENOMEM;
	}

	env.decoder.sdu_queue[ch_index] = queue;
	env.decoder.iso_dp[ch_index] = iso_dp;
	return ret_val;
#else
	return -ENOTSUP;
#endif /* AUDIO_DECODER_PATH */
}

int audio_datapath_create_sink(struct audio_datapath_config *cfg)
{
#if AUDIO_DECODER_PATH
	if (cfg == NULL) {
		return -EINVAL;
	}
	int ret;

	/* Presentation delay less than a certain value is impossible due to latency of audio
	 * datapath
	 */
	uint32_t const pres_delay_us =
		cfg->pres_delay_us /* MIN(cfg->pres_delay_us, MIN_PRESENTATION_DELAY_US) */;
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
	uint8_t num_valid_queues_dec = 0;

	for (ret = 0; ret < ARRAY_SIZE(env.decoder.sdu_queue); ret++) {
		num_valid_queues_dec += !!env.decoder.sdu_queue[ret];
	}

	if (!num_valid_queues_dec) {
		LOG_ERR("At least one SDU queue must be provided");
		return -EINVAL;
	}

	/* Remove old one and create a new */
	audio_queue_delete(env.decoder.p_audio_queue);
	env.decoder.p_audio_queue = audio_queue_create(audio_queue_len_blocks, block_samples);
	if (env.decoder.p_audio_queue == NULL) {
		LOG_ERR("Failed to create audio queue");
		return -ENOMEM;
	}

	/* Remove old one and create a new */
	audio_decoder_delete(env.decoder.p_decoder);
	env.decoder.p_decoder = audio_decoder_create(
		cfg->sampling_rate_hz, decoder_stack, CONFIG_LC3_DECODER_STACK_SIZE,
		env.decoder.sdu_queue, num_valid_queues_dec, env.decoder.p_audio_queue,
		cfg->frame_duration_is_10ms ? AUDIO_FRAME_DURATION_10MS
					    : AUDIO_FRAME_DURATION_7P5MS);

	if (env.decoder.p_decoder == NULL) {
		LOG_ERR("Failed to create audio decoder");
		return -ENOMEM;
	}

	ret = audio_sink_i2s_configure(env.i2s_dev, env.decoder.p_audio_queue,
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
	ret = audio_decoder_register_cb(env.decoder.p_decoder, print_sdus, NULL);
	if (ret != 0) {
		LOG_ERR("Failed to register decoder cb, err %d", ret);
		return ret;
	}
#endif

	/* Add callback(s) to notify ISO datapath(s) */
	for (uint8_t iter = 0; iter < ARRAY_SIZE(env.decoder.iso_dp); iter++) {
		if (!env.decoder.iso_dp[iter]) {
			continue;
		}
		ret = audio_decoder_register_cb(env.decoder.p_decoder,
						iso_datapath_ctoh_notify_sdu_done,
						env.decoder.iso_dp[iter]);
		if (ret != 0) {
			LOG_ERR("Failed to register decoder cb, err %d", ret);
			return ret;
		}
	}

	/* Add a callback to notify audio sink */
	ret = audio_decoder_register_cb(env.decoder.p_decoder,
					audio_sink_i2s_notify_buffer_available, NULL);
	if (ret != 0) {
		LOG_ERR("Failed to register decoder cb, err %d", ret);
		return ret;
	}

	LOG_INF("Created sink audio datapath");

	return 0;
#else
	return -ENOTSUP;
#endif /* AUDIO_DECODER_PATH */
}

int audio_datapath_start_sink(void)
{
#if AUDIO_DECODER_PATH
	/* Notify that first buffer is available to start reception */
	for (uint8_t iter = 0; iter < ARRAY_SIZE(env.decoder.iso_dp); iter++) {
		if (env.decoder.iso_dp[iter]) {
			iso_datapath_ctoh_notify_sdu_done(env.decoder.iso_dp[iter], 0, 0);
		}
	}
	LOG_INF("Audio sink datapath started");
	return 0;
#else
	return -ENOTSUP;
#endif /* AUDIO_DECODER_PATH */
}

int audio_datapath_cleanup_sink(void)
{
#if AUDIO_DECODER_PATH
	/* Stop and cleanup decoder thread first as it references other modules */
	audio_decoder_delete(env.decoder.p_decoder);
	env.decoder.p_decoder = NULL;

	/* Then delete ISO DP to stop any incoming SDUs */
	for (uint8_t iter = 0; iter < ARRAY_SIZE(env.decoder.iso_dp); iter++) {
		iso_datapath_ctoh_delete(env.decoder.iso_dp[iter]);
		env.decoder.iso_dp[iter] = NULL;
	}

	/* Then clean up queues */
	for (uint8_t iter = 0; iter < ARRAY_SIZE(env.decoder.sdu_queue); iter++) {
		sdu_queue_delete(env.decoder.sdu_queue[iter]);
		env.decoder.sdu_queue[iter] = NULL;
	}

	audio_queue_delete(env.decoder.p_audio_queue);
	env.decoder.p_audio_queue = NULL;

	return 0;
#else
	return -ENOTSUP;
#endif /* AUDIO_DECODER_PATH */
}

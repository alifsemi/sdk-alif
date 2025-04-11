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

#include "bluetooth/le_audio/audio_decoder.h"
#include "bluetooth/le_audio/audio_encoder.h"

#include "audio_datapath.h"

LOG_MODULE_REGISTER(audio_datapath, CONFIG_BLE_AUDIO_LOG_LEVEL);

#define AUDIO_ENCODER_PATH 1
/* TODO: Enable for bidirectional audio data */
#define AUDIO_DECODER_PATH 0

struct audio_datapath {
	const struct device *i2s_dev;
	const struct device *codec_dev;
#if AUDIO_ENCODER_PATH
	struct audio_encoder *encoder;
#endif
#if AUDIO_DECODER_PATH
	struct audio_decoder *decoder;
#endif
};

static struct audio_datapath env;

/* Initialisation to perform pre-main */
static int unicast_audio_path_init(void)
{
	int ret;
	const struct device *i2s_dev = DEVICE_DT_GET(I2S_NODE);
	const struct device *codec_dev = DEVICE_DT_GET(CODEC_NODE);

	/* Check all devices are ready */
	ret = device_is_ready(i2s_dev);
	if (!ret) {
		LOG_ERR("I2S is not ready");
		return -1;
	}

	ret = device_is_ready(codec_dev);
	if (!ret) {
		LOG_ERR("Audio codec is not ready");
		return -1;
	}

	env.i2s_dev = i2s_dev;
	env.codec_dev = codec_dev;

	return 0;
}
SYS_INIT(unicast_audio_path_init, APPLICATION, 0);

#if AUDIO_ENCODER_PATH && ENCODER_DEBUG
__ramfunc static void on_encoder_frame_complete(void *param, uint32_t timestamp, uint16_t sdu_seq)
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

int audio_datapath_create_source(struct audio_datapath_config const *const cfg)
{
#if AUDIO_ENCODER_PATH
	if (cfg == NULL) {
		return -EINVAL;
	}

	if (env.encoder) {
		/* TODO reconfigure decoder? */

		return -EALREADY;
	}

	struct audio_encoder_params const enc_params = {
		.i2s_dev = env.i2s_dev,
		.frame_duration_us = cfg->frame_duration_is_10ms ? 10000 : 7500,
		.sampling_rate_hz = cfg->sampling_rate_hz,
		.audio_buffer_len_us = cfg->pres_delay_us,
	};

	/* Remove old one and create a new */
	env.encoder = audio_encoder_create(&enc_params);
	if (env.encoder == NULL) {
		LOG_ERR("Failed to create audio encoder");
		return -ENOMEM;
	}

#if ENCODER_DEBUG
	ret = audio_encoder_register_cb(env.encoder, on_encoder_frame_complete, NULL);
	if (ret) {
		LOG_ERR("Failed to register encoder cb for stats, err %d", ret);
		return ret;
	}
#endif

	LOG_INF("Source audio datapath created");

	return 0;
#else
	return -ENOTSUP;
#endif /* AUDIO_ENCODER_PATH */
}

int audio_datapath_channel_create_source(size_t const octets_per_frame, uint8_t const stream_lid)
{
#if AUDIO_ENCODER_PATH
	int ret = audio_encoder_add_channel(env.encoder, octets_per_frame, stream_lid);

	if (ret) {
		LOG_ERR("Channel %u creation failed. Err %d", stream_lid, ret);
		return ret;
	}
	return 0;
#else
	return -ENOTSUP;
#endif
}

int audio_datapath_start_channel_source(uint8_t const ch_index)
{
#if AUDIO_ENCODER_PATH
	int ret = audio_encoder_start_channel(env.encoder, ch_index);

	if (ret) {
		LOG_ERR("Channel %u start failed. Err %d", ch_index, ret);
		return ret;
	}
	return 0;
#else
	return -ENOTSUP;
#endif
}

int audio_datapath_stop_channel_source(uint8_t const ch_index)
{
#if AUDIO_ENCODER_PATH
	int ret = audio_encoder_stop_channel(env.encoder, ch_index);

	if (ret) {
		LOG_ERR("Channel %u stop failed. Err %d", ch_index, ret);
		return ret;
	}
	return 0;
#else
	return -ENOTSUP;
#endif
}

int audio_datapath_cleanup_source(void)
{
#if AUDIO_ENCODER_PATH
	/* Stop encoder first as it references other modules */
	audio_encoder_delete(env.encoder);
	env.encoder = NULL;

	return 0;
#else
	return -ENOTSUP;
#endif /* AUDIO_ENCODER_PATH */
}

int audio_datapath_create_sink(struct audio_datapath_config const *const cfg)
{
#if AUDIO_DECODER_PATH
	if (!cfg) {
		return -EINVAL;
	}

	if (env.decoder) {
		/* TODO reconfigure decoder? */

		return -EALREADY;
	}

	struct audio_decoder_params const dec_params = {
		.i2s_dev = env.i2s_dev,
		.frame_duration_us = cfg->frame_duration_is_10ms ? 10000 : 7500,
		.sampling_rate_hz = cfg->sampling_rate_hz,
		.pres_delay_us = cfg->pres_delay_us,
	};

	env.decoder = audio_decoder_create(&dec_params);

	if (!env.decoder) {
		LOG_ERR("Failed to create audio decoder");
		return -ENOMEM;
	}

#if DECODER_DEBUG
	ret = audio_decoder_register_cb(env.decoder, print_sdus, NULL);
	if (ret != 0) {
		LOG_ERR("Failed to register decoder cb, err %d", ret);
		return ret;
	}
#endif

	LOG_INF("Sink audio datapath created");

	return 0;
#else
	return -ENOTSUP;
#endif /* AUDIO_DECODER_PATH */
}

int audio_datapath_channel_create_sink(size_t const octets_per_frame, uint8_t const ch_index)
{
#if AUDIO_DECODER_PATH
	LOG_DBG("Creating channel %u", ch_index);

	int ret = audio_decoder_add_channel(env.decoder, octets_per_frame, ch_index);

	LOG_DBG("Channel %u created", ch_index);

	return ret;
#else
	ARG_UNUSED(octets_per_frame);
	ARG_UNUSED(ch_index);
	return -ENOTSUP;
#endif /* AUDIO_DECODER_PATH */
}
int audio_datapath_channel_start_sink(uint8_t const ch_index)
{
#if AUDIO_DECODER_PATH
	LOG_DBG("Starting channel %u", ch_index);

	int ret = audio_decoder_start_channel(env.decoder, ch_index);

	LOG_DBG("Channel %u started", ch_index);

	return ret;
#else
	ARG_UNUSED(ch_index);
	return -ENOTSUP;
#endif /* AUDIO_DECODER_PATH */
}

int audio_datapath_channel_stop_sink(uint8_t const ch_index)
{
#if AUDIO_DECODER_PATH
	LOG_DBG("Starting channel %u", ch_index);

	int ret = audio_decoder_start_channel(env.decoder, ch_index);

	LOG_DBG("Channel %u started", ch_index);

	return ret;
#else
	ARG_UNUSED(ch_index);
	return -ENOTSUP;
#endif /* AUDIO_DECODER_PATH */
}

int audio_datapath_cleanup_sink(void)
{
#if AUDIO_DECODER_PATH
	audio_decoder_delete(env.decoder);
	env.decoder = NULL;

	LOG_INF("Removed sink audio datapath");

	return 0;
#else
	return -ENOTSUP;
#endif /* AUDIO_DECODER_PATH */
}

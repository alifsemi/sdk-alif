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
#include <zephyr/audio/codec.h>
#include "bluetooth/le_audio/audio_decoder.h"
#include "bluetooth/le_audio/audio_encoder.h"

#include "audio_datapath.h"

LOG_MODULE_REGISTER(audio_datapath, CONFIG_BLE_AUDIO_LOG_LEVEL);

struct audio_datapath {
	const struct device *i2s_dev;
	struct audio_encoder *encoder;
	struct audio_decoder *decoder;
};

static struct audio_datapath env;

/* Initialisation to perform pre-main */
static int unicast_audio_path_init(void)
{
	int ret;

	/* Check all devices are ready */
	ret = device_is_ready(DEVICE_DT_GET(I2S_NODE));
	if (!ret) {
		LOG_ERR("I2S is not ready");
		return -1;
	}

	ret = device_is_ready(DEVICE_DT_GET(CODEC_NODE));
	if (!ret) {
		LOG_ERR("Audio codec is not ready");
		return -1;
	}

	return 0;
}
SYS_INIT(unicast_audio_path_init, APPLICATION, 0);

#if ENCODER_DEBUG
__ramfunc static void on_encoder_frame_complete(void *param, uint32_t timestamp, uint16_t sdu_seq)
{
	if ((sdu_seq % 128) == 0) {
		LOG_INF("SDU sequence number: %u", sdu_seq);
	}
}
#endif

#if DECODER_DEBUG
__ramfunc static void print_sdus(void *context, uint32_t timestamp, uint16_t sdu_seq)
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

int audio_datapath_create_source(struct audio_datapath_config const *const cfg)
{
	if (cfg == NULL) {
		return -EINVAL;
	}

	if (env.encoder) {
		/* TODO reconfigure encoder? */

		return -EALREADY;
	}

	int ret;
	struct audio_encoder_params const enc_params = {
		.i2s_dev = DEVICE_DT_GET(I2S_NODE),
		.frame_duration_us = cfg->frame_duration_is_10ms ? 10000 : 7500,
		.sampling_rate_hz = cfg->sampling_rate_hz,
		.audio_buffer_len_us = cfg->pres_delay_us,
	};
	/* clang-format off */
	struct audio_codec_cfg codec_cfg = {
		.dai_type = AUDIO_DAI_TYPE_I2S,
		.dai_cfg = {
			.i2s = {
				.word_size = AUDIO_PCM_WIDTH_16_BITS,
				.channels = 2,
				.format = I2S_FMT_DATA_FORMAT_I2S,
				.options = 0,
				.frame_clk_freq = cfg->sampling_rate_hz,
				.mem_slab = NULL,
				.block_size = 0,
				.timeout = 0,
			},
		},
	};
	/* clang-format on */

	/* Configure codec */
	ret = audio_codec_configure(DEVICE_DT_GET(CODEC_NODE), &codec_cfg);
	if (ret) {
		LOG_ERR("Failed to configure source codec. err %d", ret);
		return ret;
	}
	audio_codec_start_output(DEVICE_DT_GET(CODEC_NODE));

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
}

int audio_datapath_channel_create_source(size_t const octets_per_frame, uint8_t const stream_lid)
{
	int ret = audio_encoder_add_channel(env.encoder, octets_per_frame, stream_lid);

	if (ret) {
		LOG_ERR("Stream %u creation failed. Err %d", stream_lid, ret);
		return ret;
	}
	return 0;
}

int audio_datapath_channel_start_source(uint8_t const stream_lid)
{
	int ret = audio_encoder_start_channel(env.encoder, stream_lid);

	if (ret) {
		LOG_ERR("Stream %u start failed. Err %d", stream_lid, ret);
		return ret;
	}
	return 0;
}

int audio_datapath_channel_stop_source(uint8_t const stream_lid)
{
	int ret = audio_encoder_stop_channel(env.encoder, stream_lid);

	if (ret) {
		LOG_ERR("Stream %u stop failed. Err %d", stream_lid, ret);
		return ret;
	}
	return 0;
}

int audio_datapath_cleanup_source(void)
{
	/* Stop encoder first as it references other modules */
	audio_encoder_delete(env.encoder);
	env.encoder = NULL;

	return 0;
}

int audio_datapath_create_sink(struct audio_datapath_config const *const cfg)
{
	if (!cfg) {
		return -EINVAL;
	}

	if (env.decoder) {
		/* TODO reconfigure decoder? */

		return -EALREADY;
	}

	struct audio_decoder_params const dec_params = {
		.i2s_dev = DEVICE_DT_GET(I2S_NODE),
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
}

int audio_datapath_channel_create_sink(size_t const octets_per_frame, uint8_t const stream_lid)
{
	int ret = audio_decoder_add_channel(env.decoder, octets_per_frame, stream_lid);

	if (ret) {
		LOG_ERR("Failed to create stream %u, err %d", stream_lid, ret);
		return ret;
	}

	return 0;
}

int audio_datapath_channel_start_sink(uint8_t const stream_lid)
{
	int ret = audio_decoder_start_channel(env.decoder, stream_lid);

	if (ret) {
		LOG_ERR("Failed to start stream %u, err %d", stream_lid, ret);
		return ret;
	}

	return 0;
}

int audio_datapath_channel_stop_sink(uint8_t const stream_lid)
{
	int ret = audio_decoder_stop_channel(env.decoder, stream_lid);

	if (ret) {
		LOG_ERR("Failed to stop stream %u, err %d", stream_lid, ret);
		return ret;
	}

	return 0;
}

int audio_datapath_cleanup_sink(void)
{
	audio_decoder_delete(env.decoder);
	env.decoder = NULL;

	LOG_INF("Removed sink audio datapath");

	return 0;
}

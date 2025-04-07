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
#include "audio_datapath.h"

LOG_MODULE_REGISTER(audio_datapath, CONFIG_BLE_AUDIO_LOG_LEVEL);

/** Enable debug for SDU sequence number info */
#define DECODER_DEBUG             0

static struct audio_datapath {
	const struct device *i2s_dev;
	const struct device *codec_dev;
	struct audio_decoder *decoder;
} env;

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

int audio_datapath_create(struct audio_datapath_config const *const cfg)
{
	if (!cfg) {
		return -EINVAL;
	}

	if (env.decoder) {
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

	LOG_INF("Created audio datapath");

	return 0;
}

int audio_datapath_create_channel(size_t const octets_per_frame, uint8_t const ch_index)
{
	int ret = audio_decoder_add_channel(env.decoder, octets_per_frame, ch_index);

	if (ret) {
		LOG_ERR("Failed to create channel %u, err %d", ch_index, ret);
		return ret;
	}

	return 0;
}

int audio_datapath_start_channel(uint8_t const ch_index)
{
	int ret = audio_decoder_start_channel(env.decoder, ch_index);

	if (ret) {
		LOG_ERR("Failed to start channel %u, err %d", ch_index, ret);
		return ret;
	}

	return 0;
}

int audio_datapath_stop_channel(uint8_t const ch_index)
{
	int ret = audio_decoder_stop_channel(env.decoder, ch_index);

	if (ret) {
		LOG_ERR("Failed to stop channel %u, err %d", ch_index, ret);
		return ret;
	}

	return 0;
}

int audio_datapath_cleanup(void)
{
	audio_decoder_delete(env.decoder);
	env.decoder = NULL;

	LOG_INF("Removed audio datapath");

	return 0;
}

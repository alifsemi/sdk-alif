/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/logging/log.h>
#include "bluetooth/le_audio/audio_decoder.h"
#include "bluetooth/le_audio/presentation_compensation.h"
#include "audio_datapath.h"

LOG_MODULE_REGISTER(audio_datapath, CONFIG_BLE_AUDIO_LOG_LEVEL);

struct audio_datapath {
	struct audio_decoder *decoder;
	size_t octets_per_frame;
};

static struct audio_datapath env;

#if CONFIG_APP_PRINT_STATS
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

int audio_datapath_create(struct audio_datapath_config *cfg)
{
	if (cfg == NULL) {
		__ASSERT(false, "Datapath configuration missing");
		return -EINVAL;
	}

	env.octets_per_frame = cfg->octets_per_frame;

	struct audio_decoder_params const dec_params = {
		.i2s_dev = cfg->i2s_dev,
		.frame_duration_us = cfg->frame_duration_is_10ms ? 10000 : 7500,
		.sampling_rate_hz = cfg->sampling_rate_hz,
		.pres_delay_us = cfg->pres_delay_us,
	};

	audio_decoder_delete(env.decoder);

	env.decoder = audio_decoder_create(&dec_params);
	if (env.decoder == NULL) {
		LOG_ERR("Failed to create audio decoder");
		return -ENOMEM;
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

#if CONFIG_APP_PRINT_STATS
	int ret = audio_decoder_register_cb(env.decoder, print_sdus, NULL);

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

int audio_datapath_start(void)
{
	for (int iter = 0; iter < CONFIG_ALIF_BLE_AUDIO_NMB_CHANNELS; iter++) {
		audio_datapath_create_channel(env.octets_per_frame, iter);
	}

	for (int iter = 0; iter < CONFIG_ALIF_BLE_AUDIO_NMB_CHANNELS; iter++) {
		audio_datapath_start_channel(iter);
	}

	LOG_INF("Audio datapath started");

	return 0;
}

int audio_datapath_cleanup(void)
{
	audio_decoder_delete(env.decoder);
	env.decoder = NULL;

	LOG_INF("Removed audio datapath");

	return 0;
}

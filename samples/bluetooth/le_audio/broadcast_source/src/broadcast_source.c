/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>
#include <zephyr/random/random.h>
#include "bluetooth/le_audio/audio_encoder.h"
#include "bluetooth/le_audio/audio_utils.h"
#include "bap.h"
#include "bap_bc.h"
#include "bap_bc_src.h"
#include "broadcast_source.h"
#include "gapi_isooshm.h"

LOG_MODULE_REGISTER(broadcast_source, CONFIG_BROADCAST_SOURCE_LOG_LEVEL);

#define PRESENTATION_DELAY_US (CONFIG_LE_AUDIO_PRESENTATION_DELAY_MS * 1000)
#define SUBGROUP_ID           0
#define BIS_ID_BASE           0

#define I2S_NODE   DT_ALIAS(i2s_bus)
#define CODEC_NODE DT_ALIAS(audio_codec)

const struct device *i2s_dev = DEVICE_DT_GET(I2S_NODE);
const struct device *codec_dev = DEVICE_DT_GET(CODEC_NODE);

/* Local ID of the broadcast group */
static uint8_t bcast_grp_lid;

/* Audio datapath handles */
static struct audio_encoder *audio_encoder;

#if CONFIG_APP_PRINT_STATS
static void on_frame_complete(void *param, uint32_t timestamp, uint16_t sdu_seq)
{
	if ((sdu_seq % CONFIG_APP_PRINT_STATS_INTERVAL) == 0) {
		LOG_INF("SDU sequence number: %u", sdu_seq);
	}
}
#endif

#ifdef CONFIG_PRESENTATION_COMPENSATION_DEBUG
void on_timing_debug_info_ready(struct presentation_comp_debug_data *dbg_data)
{
	LOG_INF("Presentation compensation debug data is ready");
}
#endif

static int audio_datapath_init(void)
{
	if (!device_is_ready(i2s_dev)) {
		LOG_WRN("I2S device is not ready");
		return -ENODEV;
	}
	if (!device_is_ready(codec_dev)) {
		LOG_WRN("Audio codec device is not ready");
		return -ENODEV;
	}

	struct audio_encoder_params const enc_params = {
		.i2s_dev = i2s_dev,
		.frame_duration_us =
			IS_ENABLED(CONFIG_ALIF_BLE_AUDIO_FRAME_DURATION_10MS) ? 10000 : 7500,
		.sampling_rate_hz = CONFIG_ALIF_BLE_AUDIO_FS_HZ,
		.audio_buffer_len_us = 2 * CONFIG_ALIF_BLE_AUDIO_MAX_TLATENCY,
	};

	audio_encoder_delete(audio_encoder);

	audio_encoder = audio_encoder_create(&enc_params);
	if (!audio_encoder) {
		LOG_ERR("Failed to create audio encoder");
		return -ENODEV;
	}

#if CONFIG_APP_PRINT_STATS
	int ret = audio_encoder_register_cb(audio_encoder, on_frame_complete, NULL);

	if (ret != 0) {
		LOG_ERR("Failed to register encoder cb for stats, err %d", ret);
		return ret;
	}
#endif

	return 0;
}

static void audio_datapath_start(const uint8_t stream_lid)
{
	int retval;

	for (int iter = 0; iter < CONFIG_ALIF_BLE_AUDIO_NMB_CHANNELS; iter++) {
		retval = audio_encoder_start_channel(audio_encoder, stream_lid + iter);
		if (retval != 0) {
			LOG_ERR("Failed to start channel %d, err %d", iter, retval);
			__ASSERT(false, "Failed to start channel");
		}
	}
}

static void on_bap_bc_src_cmp_evt(const uint8_t cmd_type, const uint16_t status,
				  const uint8_t grp_lid, const uint8_t sgrp_lid)
{
	LOG_DBG("BAP BC SRC event complete, type %u status %u grp_lid %u sgrp_lid %u", cmd_type,
		status, grp_lid, sgrp_lid);

	switch (cmd_type) {
	case BAP_BC_SRC_CMD_TYPE_ENABLE_PA: {
		LOG_INF("Periodic advertising enabled");

		uint16_t err = bap_bc_src_enable(bcast_grp_lid);

		if (err) {
			LOG_ERR("Failed to enable broadcast source, err %u", err);
		}
		break;
	}

	case BAP_BC_SRC_CMD_TYPE_ENABLE: {
		LOG_INF("Broadcast group %u enabled", bcast_grp_lid);

		uint16_t err = bap_bc_src_start_streaming(bcast_grp_lid, 0xFFFFFFFF);

		if (err) {
			LOG_ERR("Failed to start streaming, err %u", err);
		}
		break;
	}

	case BAP_BC_SRC_CMD_TYPE_START_STREAMING: {
		LOG_INF("Started streaming");

		audio_datapath_start(BIS_ID_BASE);
		break;
	}

	default: {
		LOG_WRN("Unexpected bap_bc_src command complete event: %u", cmd_type);
		break;
	}
	}
}

static void on_bap_bc_src_info(const uint8_t grp_lid, const gapi_bg_config_t *p_bg_cfg,
			       const uint8_t nb_bis, const uint16_t *p_conhdl)
{
	LOG_DBG("BAP BC SRC info, grp %u, cfg %p, nb_bis %u, p_conhdl %p", grp_lid, p_bg_cfg,
		nb_bis, p_conhdl);
}

static const bap_bc_src_cb_t bap_bc_src_cbs = {.cb_cmp_evt = on_bap_bc_src_cmp_evt,
					       .cb_info = on_bap_bc_src_info};

static int get_adv_param(bap_bc_adv_param_t *p_adv_param)
{
	p_adv_param->adv_intv_min_slot = 160;
	p_adv_param->adv_intv_max_slot = 160;
	p_adv_param->ch_map = ADV_ALL_CHNLS_EN;
	p_adv_param->phy_prim = GAPM_PHY_TYPE_LE_1M;
	p_adv_param->phy_second = GAPM_PHY_TYPE_LE_2M;
	p_adv_param->adv_sid = 1;
#if CONFIG_ALIF_BLE_ROM_IMAGE_V1_0
	p_adv_param->max_tx_pwr = -2;
#else
	p_adv_param->tx_pwr = -2;
	p_adv_param->own_addr_type = GAPM_STATIC_ADDR;
	p_adv_param->max_skip = 0;
	p_adv_param->send_tx_pwr = false;
#endif
	return 0;
}

static int broadcast_source_configure_group(void)
{
	const bap_bc_grp_param_t grp_param = {
		.sdu_intv_us = IS_ENABLED(CONFIG_ALIF_BLE_AUDIO_FRAME_DURATION_10MS) ? 10000 : 7500,
		.max_sdu = CONFIG_ALIF_BLE_AUDIO_OCTETS_PER_CODEC_FRAME,
		.max_tlatency_ms = CONFIG_ALIF_BLE_AUDIO_MAX_TLATENCY,
		.packing = 0,
		.framing = ISO_UNFRAMED_MODE,
		.phy_bf = GAPM_PHY_TYPE_LE_2M,
		.rtn = CONFIG_ALIF_BLE_AUDIO_RTN};

	const gaf_codec_id_t codec_id = GAF_CODEC_ID_LC3;

	bap_bc_adv_param_t adv_param;

	int ret = get_adv_param(&adv_param);

	if (ret) {
		LOG_ERR("Failed to get advertising parameters, err %d", ret);
		return -1;
	}

	const bap_bc_per_adv_param_t per_adv_param = {
		.adv_intv_min_frame = 160,
		.adv_intv_max_frame = 160,
	};

	bap_bcast_id_t bcast_id;

	sys_rand_get(bcast_id.id, sizeof(bcast_id.id));

	uint16_t err = bap_bc_src_add_group(&bcast_id, NULL, CONFIG_ALIF_BLE_AUDIO_NMB_CHANNELS, 1,
					    &grp_param, &adv_param, &per_adv_param,
					    PRESENTATION_DELAY_US, &bcast_grp_lid);

	if (err) {
		LOG_ERR("Failed to add broadcast group, err %u", err);
		return -1;
	}

	LOG_DBG("Broadcast group added, got local ID %u", bcast_grp_lid);

	/* This struct must be accessible to the BLE stack for the lifetime of the BIG, so is
	 * statically allocated
	 */
	static bap_cfg_t sgrp_cfg = {
		.param = {
				.location_bf = 0, /* Location is unspecified at subgroup level */
				.frame_octet = CONFIG_ALIF_BLE_AUDIO_OCTETS_PER_CODEC_FRAME,
				.frame_dur = IS_ENABLED(CONFIG_ALIF_BLE_AUDIO_FRAME_DURATION_10MS)
						     ? BAP_FRAME_DUR_10MS
						     : BAP_FRAME_DUR_7_5MS,
				.frames_sdu =
					0, /* 0 is unspecified, data will not be placed in BASE */
			},
		.add_cfg.len = 0,
	};
	/* Convert to sampling frequency */
	sgrp_cfg.param.sampling_freq = audio_hz_to_bap_sampling_freq(CONFIG_ALIF_BLE_AUDIO_FS_HZ);

	/* Validate sampling frequency conversion */
	if (sgrp_cfg.param.sampling_freq == BAP_SAMPLING_FREQ_UNKNOWN) {
		LOG_ERR("Unsupported sampling frequency: %u Hz", CONFIG_ALIF_BLE_AUDIO_FS_HZ);
		return -1;
	}

	/* This struct must be accessible to the BLE stack for the lifetime of the BIG, so is
	 * statically allocated
	 */
	static const bap_cfg_metadata_t sgrp_meta = {
		.param.context_bf = BAP_CONTEXT_TYPE_UNSPECIFIED_BIT | BAP_CONTEXT_TYPE_MEDIA_BIT,
		.add_metadata.len = 0,
	};

	err = bap_bc_src_set_subgroup(bcast_grp_lid, SUBGROUP_ID, &codec_id, &sgrp_cfg, &sgrp_meta);

	if (err) {
		LOG_ERR("Failed to set subgroup, err %u", err);
		return -1;
	}
	LOG_DBG("Broadcast subgroup added");

	const uint16_t dp_id = GAPI_DP_ISOOSHM;

	/* This struct must be accessible to the BLE stack for the lifetime of the BIG, so is
	 * statically allocated
	 */
	static const bap_cfg_t stream_cfg[CONFIG_ALIF_BLE_AUDIO_NMB_CHANNELS] = {
		{.param = {
				 .sampling_freq =
					 BAP_SAMPLING_FREQ_UNKNOWN,  /* Inherited from subgroup */
				 .frame_dur = BAP_FRAME_DUR_UNKNOWN, /* Inherited from subgroup */
				 .frames_sdu = 0,                    /* Inherited from subgroup */
				 .frame_octet = 0,                   /* Inherited from subgroup */
				 .location_bf = GAF_LOC_FRONT_LEFT_BIT,
			 },
		 .add_cfg.len = 0},
#if CONFIG_ALIF_BLE_AUDIO_NMB_CHANNELS > 1
		{.param = {
				 .sampling_freq =
					 BAP_SAMPLING_FREQ_UNKNOWN,  /* Inherited from subgroup */
				 .frame_dur = BAP_FRAME_DUR_UNKNOWN, /* Inherited from subgroup */
				 .frames_sdu = 0,                    /* Inherited from subgroup */
				 .frame_octet = 0,                   /* Inherited from subgroup */
				 .location_bf = GAF_LOC_FRONT_RIGHT_BIT,
			 },
		 .add_cfg.len = 0},
#endif
	};

	for (size_t iter = 0; iter < ARRAY_SIZE(stream_cfg); iter++) {

		err = bap_bc_src_set_stream(bcast_grp_lid, iter + BIS_ID_BASE, SUBGROUP_ID, dp_id,
					    0, &stream_cfg[iter]);

		if (err) {
			LOG_ERR("Failed to set stream %u, err %u", iter + BIS_ID_BASE, err);
			return -1;
		}
		err = audio_encoder_add_channel(audio_encoder,
						CONFIG_ALIF_BLE_AUDIO_OCTETS_PER_CODEC_FRAME,
						iter + BIS_ID_BASE);
		if (err) {
			LOG_ERR("Failed to add stream channel %u. err %d", iter + BIS_ID_BASE, err);
			return -1;
		}
	}

	LOG_DBG("Broadcast stream added");

	return 0;
}

static int broadcast_source_enable(void)
{
	uint8_t ad_data[1 + sizeof(CONFIG_BLE_DEVICE_NAME)];

	ad_data[0] = sizeof(ad_data) - 1; /* Size of data following the size byte */
	ad_data[1] = 0x09;                /* Complete local name */

	memcpy(&ad_data[2], CONFIG_BLE_DEVICE_NAME, sizeof(ad_data) - 2);

	uint16_t err = bap_bc_src_enable_pa(bcast_grp_lid, sizeof(ad_data), 0, ad_data, NULL,
					    sizeof(CONFIG_BROADCAST_NAME) - 1,
					    CONFIG_BROADCAST_NAME, 0, NULL);

	if (err) {
		LOG_ERR("Failed to enable PA with error %u", err);
		return -1;
	}

	return 0;
}

int broadcast_source_start(void)
{
	int ret;
	uint16_t err = bap_bc_src_configure(&bap_bc_src_cbs);

	if (err) {
		LOG_ERR("Error %u configuring bap_bc_src", err);
		return -1;
	}

	LOG_DBG("bap_bc_src configured");

	ret = audio_datapath_init();
	if (ret) {
		LOG_ERR("Failed to initialise audio datapath, err %d", ret);
		return ret;
	}

	ret = broadcast_source_configure_group();
	if (ret) {
		LOG_ERR("Failed to configure broadcast source group, err %d", ret);
		return ret;
	}

	LOG_DBG("Broadcast group configured");

	return broadcast_source_enable();
}

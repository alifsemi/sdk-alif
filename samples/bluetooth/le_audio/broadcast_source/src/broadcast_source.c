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
#include "bluetooth/le_audio/audio_source_i2s.h"
#include "bluetooth/le_audio/audio_queue.h"
#include "bluetooth/le_audio/audio_encoder.h"
#include "bluetooth/le_audio/sdu_queue.h"
#include "bluetooth/le_audio/iso_datapath_htoc.h"
#include "bluetooth/le_audio/presentation_compensation.h"
#include "alif_lc3.h"
#include "bap.h"
#include "bap_bc.h"
#include "bap_bc_src.h"
#include "broadcast_source.h"
#include "gapi_isooshm.h"

LOG_MODULE_REGISTER(broadcast_source, CONFIG_BROADCAST_SOURCE_LOG_LEVEL);

#define PRESENTATION_DELAY_US (CONFIG_LE_AUDIO_PRESENTATION_DELAY_MS * 1000)

#define SDU_QUEUE_LENGTH        4
#define AUDIO_QUEUE_LENGTH      4
#define MICROSECONDS_PER_SECOND 1000000

#if CONFIG_ALIF_BLE_AUDIO_FRAME_DURATION_10MS
#define FRAMES_PER_SECOND       100
#else
#error "Unsupported configuration"
#endif

#define I2S_NODE      DT_ALIAS(i2s_bus)
#define CODEC_NODE    DT_ALIAS(audio_codec)

const struct device *i2s_dev = DEVICE_DT_GET(I2S_NODE);
const struct device *codec_dev = DEVICE_DT_GET(CODEC_NODE);

K_THREAD_STACK_DEFINE(encoder_stack, CONFIG_LC3_ENCODER_STACK_SIZE);

/* Local ID of the broadcast group */
static uint8_t bcast_grp_lid;

/* Audio datapath handles */
static struct sdu_queue *sdu_queue_l;
static struct sdu_queue *sdu_queue_r;
static struct iso_datapath_htoc *iso_dp_l;
static struct iso_datapath_htoc *iso_dp_r;
static struct audio_queue *audio_queue;
static struct audio_encoder *audio_encoder;

static void on_frame_complete(void *param, uint32_t timestamp, uint16_t sdu_seq)
{
#if CONFIG_APP_PRINT_STATS
	if ((sdu_seq % CONFIG_APP_PRINT_STATS_INTERVAL) == 0) {
		LOG_INF("SDU sequence number: %u", sdu_seq);
	}
#endif
}

#ifdef CONFIG_PRESENTATION_COMPENSATION_DEBUG
void on_timing_debug_info_ready(struct presentation_comp_debug_data *dbg_data)
{
	LOG_INF("Presentation compensation debug data is ready");
}
#endif

static int audio_datapath_init(void)
{
	int ret;
	bool mono_mode = IS_ENABLED(CONFIG_ALIF_BLE_AUDIO_SOURCE_MONO);
	/* uint8_t mult = mono_mode ? 1 : 2; */

	if (!device_is_ready(i2s_dev)) {
		LOG_WRN("I2S device is not ready");
		return -1;
	}
	if (!device_is_ready(codec_dev)) {
		LOG_WRN("Audio codec device is not ready");
		return -1;
	}

	ret = alif_lc3_init();
	__ASSERT(ret == 0, "Failed to initialise LC3 codec");

	sdu_queue_l =
		sdu_queue_create(SDU_QUEUE_LENGTH, CONFIG_ALIF_BLE_AUDIO_OCTETS_PER_CODEC_FRAME);
	__ASSERT(sdu_queue_l, "Failed to create left SDU queue");

	sdu_queue_r =
		sdu_queue_create(SDU_QUEUE_LENGTH, CONFIG_ALIF_BLE_AUDIO_OCTETS_PER_CODEC_FRAME);
	__ASSERT(sdu_queue_r, "Failed to create right SDU queue");

	/* we need to receive both channels even if we are in mono mode */
	audio_queue = audio_queue_create(AUDIO_QUEUE_LENGTH,
					 2 * CONFIG_ALIF_BLE_AUDIO_FS_HZ / FRAMES_PER_SECOND);
	__ASSERT(audio_queue, "Failed to create audio queue");

	audio_encoder = audio_encoder_create(mono_mode, CONFIG_ALIF_BLE_AUDIO_FS_HZ, encoder_stack,
					     CONFIG_LC3_ENCODER_STACK_SIZE, sdu_queue_l,
					     sdu_queue_r, audio_queue);
	__ASSERT(audio_encoder, "Failed to create audio encoder");

	ret = audio_source_i2s_configure(i2s_dev, audio_queue,
					 MICROSECONDS_PER_SECOND / FRAMES_PER_SECOND);
	__ASSERT(ret == 0, "Failed to configure audio source I2S");

	ret = audio_encoder_register_cb(audio_encoder, on_frame_complete, NULL);
	__ASSERT(ret == 0, "Failed to register encoder cb for stats");

	ret = audio_encoder_register_cb(audio_encoder, audio_source_i2s_notify_buffer_available,
					NULL);
	__ASSERT(ret == 0, "Failed to register encoder cb for audio source");

	return 0;
}
SYS_INIT(audio_datapath_init, APPLICATION, 0);

static int bap_sampling_freq_from_hz(uint32_t sampling_freq_hz)
{
	switch (sampling_freq_hz) {
	case 8000:
		return BAP_SAMPLING_FREQ_8000HZ;
	case 16000:
		return BAP_SAMPLING_FREQ_16000HZ;
	case 24000:
		return BAP_SAMPLING_FREQ_24000HZ;
	case 32000:
		return BAP_SAMPLING_FREQ_32000HZ;
	case 48000:
		return BAP_SAMPLING_FREQ_48000HZ;
	default:
		return BAP_SAMPLING_FREQ_UNKNOWN;
	}
}

static void audio_datapath_start(uint8_t sgrp_lid)
{
	iso_dp_l = iso_datapath_htoc_create(sgrp_lid, sdu_queue_l, true);
	if (iso_dp_l == NULL) {
		LOG_ERR("Failed to create left ISO datapath");
		return;
	}

	iso_dp_r = iso_datapath_htoc_create(sgrp_lid + 1, sdu_queue_r, false);
	if (iso_dp_r == NULL) {
		LOG_ERR("Failed to create right ISO datapath");
		return;
	}

	int ret = audio_encoder_register_cb(audio_encoder, iso_datapath_htoc_notify_sdu_available,
					    iso_dp_l);
	if (ret) {
		LOG_ERR("Failed to register encoder cb for left ISO datapath, err %d", ret);
		return;
	}

	ret = audio_encoder_register_cb(audio_encoder, iso_datapath_htoc_notify_sdu_available,
					iso_dp_r);
	if (ret) {
		LOG_ERR("Failed to register encoder cb for right ISO datapath, err %d", ret);
		return;
	}

	/* Start audio stream from I2S */
	audio_source_i2s_notify_buffer_available(NULL, 0, 0);
}

static void on_bap_bc_src_cmp_evt(uint8_t cmd_type, uint16_t status, uint8_t grp_lid,
				  uint8_t sgrp_lid)
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

		audio_datapath_start(sgrp_lid);

		break;
	}

	default: {
		LOG_WRN("Unexpected bap_bc_src command complete event: %u", cmd_type);
		break;
	}
	}
}

static void on_bap_bc_src_info(uint8_t grp_lid, const gapi_bg_config_t *p_bg_cfg, uint8_t nb_bis,
			       const uint16_t *p_conhdl)
{
	LOG_DBG("BAP BC SRC info, grp %u, cfg %p, nb_bis %u, p_conhdl %p", grp_lid, p_bg_cfg,
		nb_bis, p_conhdl);
}

static const bap_bc_src_cb_t bap_bc_src_cbs = {.cb_cmp_evt = on_bap_bc_src_cmp_evt,
					       .cb_info = on_bap_bc_src_info};

static int broadcast_source_configure_group(void)
{
	const bap_bc_grp_param_t grp_param = {.sdu_intv_us = 10000,
					      .max_sdu =
						      CONFIG_ALIF_BLE_AUDIO_OCTETS_PER_CODEC_FRAME,
					      .max_tlatency_ms = CONFIG_ALIF_BLE_AUDIO_MAX_TLATENCY,
					      .packing = 0,
					      .framing = ISO_UNFRAMED_MODE,
					      .phy_bf = GAPM_PHY_TYPE_LE_2M,
					      .rtn = CONFIG_ALIF_BLE_AUDIO_RTN};

	const gaf_codec_id_t codec_id = GAF_CODEC_ID_LC3;

	const bap_bc_adv_param_t adv_param = {
		.adv_intv_min_slot = 160,
		.adv_intv_max_slot = 160,
		.ch_map = ADV_ALL_CHNLS_EN,
		.phy_prim = GAPM_PHY_TYPE_LE_1M,
		.phy_second = GAPM_PHY_TYPE_LE_2M,
		.adv_sid = 1,
		.max_tx_pwr = -2,
	};

	const bap_bc_per_adv_param_t per_adv_param = {
		.adv_intv_min_frame = 160,
		.adv_intv_max_frame = 160,
	};

	bap_bcast_id_t bcast_id;

	sys_rand_get(bcast_id.id, sizeof(bcast_id.id));

	uint16_t err = bap_bc_src_add_group(&bcast_id, NULL, 2, 1, &grp_param, &adv_param,
					    &per_adv_param, PRESENTATION_DELAY_US, &bcast_grp_lid);

	if (err) {
		LOG_ERR("Failed to add broadcast group, err %u", err);
		return -1;
	}

	LOG_DBG("Broadcast group added, got local ID %u", bcast_grp_lid);

	/* This struct must be accessible to the BLE stack for the lifetime of the BIG, so is
	 * statically allocated
	 */
	static bap_cfg_t sgrp_cfg = {
		.param = {.location_bf = 0, /* Location is unspecified at subgroup level */
			  .frame_octet = CONFIG_ALIF_BLE_AUDIO_OCTETS_PER_CODEC_FRAME,
			  .frame_dur = BAP_FRAME_DUR_10MS,
			  .frames_sdu = 0, /* 0 is unspecified, data will not be placed in BASE */
			},
		.add_cfg.len = 0,
	};

	sgrp_cfg.param.sampling_freq =
		bap_sampling_freq_from_hz(CONFIG_ALIF_BLE_AUDIO_FS_HZ);

	/* This struct must be accessible to the BLE stack for the lifetime of the BIG, so is
	 * statically allocated
	 */
	static const bap_cfg_metadata_t sgrp_meta = {
		.param.context_bf = BAP_CONTEXT_TYPE_UNSPECIFIED_BIT | BAP_CONTEXT_TYPE_MEDIA_BIT,
		.add_metadata.len = 0,
	};

	err = bap_bc_src_set_subgroup(bcast_grp_lid, 0, &codec_id, &sgrp_cfg, &sgrp_meta);

	if (err) {
		LOG_ERR("Failed to set subgroup, err %u", err);
		return -1;
	}
	LOG_DBG("Broadcast subgroup added");
	const uint16_t dp_id = GAPI_DP_ISOOSHM;

	/* This struct must be accessible to the BLE stack for the lifetime of the BIG, so is
	 * statically allocated
	 */
	static const bap_cfg_t stream_cfg_l = {
		.param = {
				.sampling_freq =
					BAP_SAMPLING_FREQ_UNKNOWN,  /* Inherited from subgroup */
				.frame_dur = BAP_FRAME_DUR_UNKNOWN, /* Inherited from subgroup */
				.frames_sdu = 0,                    /* Inherited from subgroup */
				.frame_octet = 0,                   /* Inherited from subgroup */
				.location_bf = GAF_LOC_FRONT_LEFT_BIT,
			},
		.add_cfg.len = 0};

	static const bap_cfg_t stream_cfg_r = {
		.param = {
				.sampling_freq =
					BAP_SAMPLING_FREQ_UNKNOWN,  /* Inherited from subgroup */
				.frame_dur = BAP_FRAME_DUR_UNKNOWN, /* Inherited from subgroup */
				.frames_sdu = 0,                    /* Inherited from subgroup */
				.frame_octet = 0,                   /* Inherited from subgroup */
				.location_bf = GAF_LOC_FRONT_RIGHT_BIT,
			},
		.add_cfg.len = 0};

	err = bap_bc_src_set_stream(bcast_grp_lid, 0, 0, dp_id, 0, &stream_cfg_l);

	if (err) {
		LOG_ERR("Failed to set left stream, err %u", err);
		return -1;
	}

	err = bap_bc_src_set_stream(bcast_grp_lid, 1, 0, dp_id, 0, &stream_cfg_r);
	if (err) {
		LOG_ERR("Failed to set right stream, err %u", err);
		return -1;
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
	uint16_t err = bap_bc_src_configure(&bap_bc_src_cbs);

	if (err) {
		LOG_ERR("Error %u configuring bap_bc_src", err);
		return -1;
	}

	LOG_DBG("bap_bc_src configured");

	int ret = broadcast_source_configure_group();

	if (ret) {
		LOG_ERR("Failed to configure broadcast source group, err %d", ret);
		return ret;
	}

	LOG_DBG("Broadcast group configured");

	return broadcast_source_enable();
}

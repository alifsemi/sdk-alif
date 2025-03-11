/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
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
#include "gaf_adv.h"
#include "bap_capa_srv.h"
#include "bap_uc_srv.h"
#include "unicast_sink.h"
#include "co_utils.h"
#include "tmap.h"
#include "audio_datapath.h"

LOG_MODULE_REGISTER(unicast_sink, CONFIG_UNICAST_SINK_LOG_LEVEL);

#define PRESENTATION_DELAY_US (CONFIG_LE_AUDIO_PRESENTATION_DELAY_MS * 1000u)

#define LOCATION_SINK (GAF_LOC_FRONT_LEFT_BIT | GAF_LOC_FRONT_RIGHT_BIT)
#ifndef LOCATION_SINK
#define LOCATION_SINK GAF_LOC_FRONT_LEFT_BIT
#endif
#define LOCATION_SOURCE           0
#define DATA_PATH_CONFIG          DATA_PATH_ISOOSHM
#define MAX_NUMBER_OF_CONNECTIONS 1

/** TODO: This can be removed when the i2s driver is updated to support dynamic sample rate */
#define I2S_BUS_SAMPLE_RATE DT_PROP(I2S_NODE, sample_rate)

#if I2S_BUS_SAMPLE_RATE != 16000 && I2S_BUS_SAMPLE_RATE != 24000 &&                                \
	I2S_BUS_SAMPLE_RATE != 32000 && I2S_BUS_SAMPLE_RATE != 48000
#error "Invalid sample rate"
#endif

/** ASE information structure */
struct ase_info {
	/** Pointer to Codec Configuration */
	bap_cfg_t *p_codec_cfg;
	/** Pointer to Metadata */
	bap_cfg_metadata_t *p_metadata;
	/** ASE state \ref enum bap_uc_ase_state */
	uint8_t ase_state;
	/** ASE connexion index */
	uint8_t conidx;
};

struct unicast_env {
	bool advertising_ongoing;
	/** Bit field allowing to know if a procedure has been completed for an ASE */
	uint8_t ase_done_bf;
	/** Number of simultaneous connections supported */
	uint8_t nb_connection;
	/** Number of Sink ASEs */
	uint8_t nb_ases_sink;
	/** Number of Source ASEs */
	uint8_t nb_ases_src;
	/** Pointer to ASE info
	 * Note: dynamically allocated based on sink and source configurations
	 */
	struct ase_info *p_ase_info;
	/** Codec Capabilities structure (Sink direction) */
	bap_capa_t *p_capa_sink;
	/** Codec Capabilities structure (Source direction) */
	bap_capa_t *p_capa_src;

	struct audio_datapath_config datapath_config;
	bool data_path_configured;
	uint8_t nb_datapaths_sink;
	uint8_t nb_datapaths_src;

	uint8_t ase_lid_cfm;
	uint8_t frame_octet;

	bool server_started;
};

/** Unicast environment */
static struct unicast_env unicast_env = {
	.advertising_ongoing = false,
};

/** Array providing string description of each ASE state */
static const char *ase_state_name[BAP_UC_ASE_STATE_MAX] = {
	[BAP_UC_ASE_STATE_IDLE] = "Idle",
	[BAP_UC_ASE_STATE_CODEC_CONFIGURED] = "Codec Configured",
	[BAP_UC_ASE_STATE_QOS_CONFIGURED] = "QoS Configured",
	[BAP_UC_ASE_STATE_ENABLING] = "Enabling",
	[BAP_UC_ASE_STATE_STREAMING] = "Streaming",
	[BAP_UC_ASE_STATE_DISABLING] = "Disabling",
	[BAP_UC_ASE_STATE_RELEASING] = "Releasing",
};
/** Array providing string description of each Sampling Frequency */
static const char *sampling_freq_name[BAP_SAMPLING_FREQ_MAX + 1] = {
	[BAP_SAMPLING_FREQ_8000HZ] = "8kHz",       [BAP_SAMPLING_FREQ_11025HZ] = "11.025kHz",
	[BAP_SAMPLING_FREQ_16000HZ] = "16kHz",     [BAP_SAMPLING_FREQ_22050HZ] = "22.050kHz",
	[BAP_SAMPLING_FREQ_24000HZ] = "24kHz",     [BAP_SAMPLING_FREQ_32000HZ] = "32kHz",
	[BAP_SAMPLING_FREQ_44100HZ] = "44.1kHz",   [BAP_SAMPLING_FREQ_48000HZ] = "48kHz",
	[BAP_SAMPLING_FREQ_88200HZ] = "88.2kHz",   [BAP_SAMPLING_FREQ_96000HZ] = "96kHz",
	[BAP_SAMPLING_FREQ_176400HZ] = "176.4kHz", [BAP_SAMPLING_FREQ_192000HZ] = "192kHz",
	[BAP_SAMPLING_FREQ_384000HZ] = "384kHz",
};
/** Array providing string description of each Frame Duration */
static const char *frame_dur_name[BAP_FRAME_DUR_MAX + 1] = {
	[BAP_FRAME_DUR_7_5MS] = "7.5ms",
	[BAP_FRAME_DUR_10MS] = "10ms",
};
/** Array providing string description of each location */
static const char *location_name[GAF_LOC_RIGHT_SURROUND_POS + 1] = {
	[GAF_LOC_FRONT_LEFT_POS] = "FRONT LEFT",
	[GAF_LOC_FRONT_RIGHT_POS] = "FRONT RIGHT",
	[GAF_LOC_FRONT_CENTER_POS] = "CENTER",
	[GAF_LOC_LFE1_POS] = "LFE1",
	[GAF_LOC_BACK_LEFT_POS] = "BACK LEFT",
	[GAF_LOC_BACK_RIGHT_POS] = "BACK RIGHT",
	[GAF_LOC_FRONT_LEFT_CENTER_POS] = "FRONT LEFT CENTER",
	[GAF_LOC_FRONT_RIGHT_CENTER_POS] = "FRONT RIGHT CENTER",
	[GAF_LOC_BACK_CENTER_POS] = "BACK CENTER",
	[GAF_LOC_LFE2_POS] = "LFE2",
	[GAF_LOC_SIDE_LEFT_POS] = "SIDE LEFT",
	[GAF_LOC_SIDE_RIGHT_POS] = "SIDE RIGHT",
	[GAF_LOC_TOP_FRONT_LEFT_POS] = "TOP FRONT LEFT",
	[GAF_LOC_TOP_FRONT_RIGHT_POS] = "TOP FRONT RIGHT",
	[GAF_LOC_TOP_FRONT_CENTER_POS] = "TOP FRONT CENTER",
	[GAF_LOC_TOP_CENTER_POS] = "TOP CENTER",
	[GAF_LOC_TOP_BACK_LEFT_POS] = "TOP BACK LEFT",
	[GAF_LOC_TOP_BACK_RIGHT_POS] = "TOP BACK RIGHT",
	[GAF_LOC_TOP_SIDE_LEFT_POS] = "TOP SIDE LEFT",
	[GAF_LOC_TOP_SIDE_RIGHT_POS] = "TOP SIDE RIGHT",
	[GAF_LOC_TOP_BACK_CENTER_POS] = "TOP BACK CENTER",
	[GAF_LOC_BOTTOM_FRONT_CENTER_POS] = "BOTTOM FRONT CENTER",
	[GAF_LOC_BOTTOM_FRONT_LEFT_POS] = " BOTTOM FRONT LEFT",
	[GAF_LOC_BOTTOM_FRONT_RIGHT_POS] = "BOTTOM FRONT RIGHT",
	[GAF_LOC_FRONT_LEFT_WIDE_POS] = "FRONT LEFT WIDE",
	[GAF_LOC_FRONT_RIGHT_WIDE_POS] = "FRONT RIGHT WIDE",
	[GAF_LOC_LEFT_SURROUND_POS] = "LEFT SURROUND",
	[GAF_LOC_RIGHT_SURROUND_POS] = "RIGHT SURROUND",
};

#define WORKER_PRIORITY   2
#define WORKER_STACK_SIZE 2048

K_KERNEL_STACK_DEFINE(worker_task_stack, WORKER_STACK_SIZE);
static struct k_work_q worker_queue;

/* ---------------------------------------------------------------------------------------- */

static uint32_t sampling_freq_index_to_hz(uint32_t const index)
{
	switch (index) {
	case BAP_SAMPLING_FREQ_8000HZ:
		return 8000;
	case BAP_SAMPLING_FREQ_16000HZ:
		return 16000;
	case BAP_SAMPLING_FREQ_24000HZ:
		return 24000;
	case BAP_SAMPLING_FREQ_32000HZ:
		return 32000;
	case BAP_SAMPLING_FREQ_44100HZ:
		return 44100;
	case BAP_SAMPLING_FREQ_48000HZ:
		return 48000;
	default:
		break;
	}
	__ASSERT(0, "Invalid sampling frequency!");
}

/* ---------------------------------------------------------------------------------------- */

#define START_FROM_STREMING 1

static void job_start_audio_datapath(struct k_work *p_job)
{
	ARG_UNUSED(p_job);
#if !START_FROM_STREMING
	k_sleep(K_MSEC(5));
#endif
	if (unicast_env.data_path_configured) {
		return;
	}
	if (audio_datapath_create(&unicast_env.datapath_config) < 0) {
		audio_datapath_cleanup();
		LOG_ERR("Failed to create audio datapath");
		return;
	}

	audio_datapath_start();
	unicast_env.data_path_configured = true;
}

static K_WORK_DEFINE(start_audio_job, job_start_audio_datapath);

/* ---------------------------------------------------------------------------------------- */
/* GAF advertising */

static void on_gaf_advertising_cmp_evt(uint8_t const cmd_type, uint16_t const status,
				       uint8_t const set_lid)
{
	if (GAF_ERR_NO_ERROR != status) {
		LOG_ERR("GAF advertising [%u] error:%u, cmd:%u", set_lid, status, cmd_type);
		assert(GAF_ERR_NO_ERROR == status);
	}

	switch (cmd_type) {
	case GAF_ADV_CMD_TYPE_START: {
		LOG_INF("GAF advertising");
		unicast_env.advertising_ongoing = true;
		break;
	}
	case GAF_ADV_CMD_TYPE_STOP: {
		LOG_INF("GAF advertising stop completed");
		break;
	}
	case GAF_ADV_CMD_TYPE_START_DIRECTED: {
		LOG_INF("GAF directed advertising");
		break;
	}
	case GAF_ADV_CMD_TYPE_START_DIRECTED_FAST: {
		LOG_INF("GAF high-duty cycle directed advertising");
		break;
	}
	default:
		break;
	}
}

static void on_gaf_advertising_stopped(uint8_t const set_lid, uint8_t const reason)
{
	LOG_DBG("GAF advertising [%u] stopped. reason=%u", set_lid, reason);
	unicast_env.advertising_ongoing = false;
}

static const struct gaf_adv_cb gaf_adv_cbs = {
	.cb_cmp_evt = on_gaf_advertising_cmp_evt,
	.cb_stopped = on_gaf_advertising_stopped,
};

/* ---------------------------------------------------------------------------------------- */
/* BAP Unicast Server */

static void on_unicast_server_cb_cmp_evt(uint8_t const cmd_type, uint16_t const status,
					 uint8_t const ase_lid)
{
	switch (cmd_type) {
	case BAP_UC_SRV_CMD_TYPE_DISABLE:
		LOG_INF("Unicast [ASE %u] BAP_UC_SRV_CMD_TYPE_DISABLE (status: %u)", ase_lid,
			status);
		break;
	case BAP_UC_SRV_CMD_TYPE_RELEASE:
		LOG_INF("Unicast [ASE %u] BAP_UC_SRV_CMD_TYPE_RELEASE (status: %u)", ase_lid,
			status);
		break;
	case BAP_UC_SRV_CMD_TYPE_GET_QUALITY:
		LOG_INF("Unicast [ASE %u] BAP_UC_SRV_CMD_TYPE_GET_QUALITY (status: %u)", ase_lid,
			status);
		break;
	default:
		LOG_ERR("Unicast [ASE %u] unknown cmd %u server (error: %u)", ase_lid, cmd_type,
			status);
		break;
	}
}

static void on_unicast_server_cb_quality_cmp_evt(
	uint16_t const status, uint8_t const ase_lid, uint32_t const tx_unacked_packets,
	uint32_t const tx_flushed_packets, uint32_t const tx_last_subevent_packets,
	uint32_t const retransmitted_packets, uint32_t const crc_error_packets,
	uint32_t const rx_unreceived_packets, uint32_t const duplicate_packets)
{
	LOG_DBG("Unicast [ASE %u] quality_cmp_evt (error: %u). TX unack:%u,flush:%u,num:%u"
		" RX retx:%u,crc:%u,unrx:%u,dup:%u",
		ase_lid, status, tx_unacked_packets, tx_flushed_packets, tx_last_subevent_packets,
		retransmitted_packets, crc_error_packets, rx_unreceived_packets, duplicate_packets);
}

static void on_unicast_server_cb_bond_data(uint8_t const conidx, uint8_t const cli_cfg_bf,
					   uint16_t const ase_cli_cfg_bf)
{
	LOG_DBG("ASCS Bond Data updated (conidx: %d, cli_cfg_bf: 0x%02X, ase_cli_cfg_bf: 0x%02X)",
		conidx, cli_cfg_bf, ase_cli_cfg_bf);
}

static void on_unicast_server_cb_ase_state(uint8_t const ase_lid, uint8_t const conidx,
					   uint8_t const state, bap_qos_cfg_t *const p_qos_cfg)
{
	struct ase_info *p_ase_info = &unicast_env.p_ase_info[ase_lid];

	LOG_DBG("ASE %d - %s", ase_lid, ase_state_name[state]);

	p_ase_info->ase_state = state;
	p_ase_info->conidx = conidx;

	switch (state) {
	case BAP_UC_ASE_STATE_IDLE: {
		unicast_env.ase_done_bf |= CO_BIT(ase_lid);
		free(p_ase_info->p_codec_cfg);
		p_ase_info->p_codec_cfg = NULL;
		break;
	}
	case BAP_UC_ASE_STATE_QOS_CONFIGURED: {
		free(p_ase_info->p_metadata);
		p_ase_info->p_metadata = NULL;
		if (p_qos_cfg) {
			p_ase_info->p_codec_cfg->param.frame_octet = p_qos_cfg->max_sdu_size;
		}
		unicast_env.nb_datapaths_sink++;
		break;
	}
	case BAP_UC_ASE_STATE_ENABLING: {
		audio_datapath_create_channel(unicast_env.frame_octet, ase_lid);
		break;
	}
	case BAP_UC_ASE_STATE_RELEASING: {
		bap_uc_srv_release_cfm(ase_lid, BAP_UC_CP_RSP_CODE_SUCCESS, 0, true);
		break;
	}
	case BAP_UC_ASE_STATE_STREAMING: {
#if START_FROM_STREMING
		/* Start audio path when last ASE is ready */
		if ((unicast_env.nb_datapaths_sink - 1) == ase_lid) {
			k_work_submit_to_queue(&worker_queue, &start_audio_job);
		}
#endif
		break;
	}
	default: {
		break;
	}
	}
}

static void on_unicast_server_cb_cis_state(const uint8_t stream_lid, const uint8_t conidx,
					   const uint8_t ase_lid_sink, const uint8_t ase_lid_src,
					   const uint8_t cig_id, const uint8_t cis_id,
					   const uint16_t conhdl, gapi_ug_config_t *const p_cig_cfg,
					   gapi_us_config_t *const p_cis_cfg)
{
	LOG_DBG("CIS %d state - handle:0x%04X, cig_id:%d, stream_lid:%d, ASE lid sink:%u,source:%u",
		cis_id, conhdl, cig_id, stream_lid, ase_lid_sink, ase_lid_src);
	/* Ignore other prints if the handle is undefined */
	if (GAP_INVALID_CONHDL == conhdl) {
		/* state is released, otherwise established */
		return;
	}

	LOG_DBG("  GROUP: sync_delay_us:%u, tlatency_m2s_us:%u, tlatency_s2m_us:%u, "
		"iso_intv_frames:%u",
		p_cig_cfg->sync_delay_us, p_cig_cfg->tlatency_m2s_us, p_cig_cfg->tlatency_s2m_us,
		p_cig_cfg->iso_intv_frames);
	LOG_DBG("  STREAM: sync_delay_us:%u, Max PDU m2s:%u/s2m:%u, PHY m2s:%u/s2m:%u, flush to "
		"m2s:%u/s2m:%u, nse:%u",
		p_cis_cfg->sync_delay_us, p_cis_cfg->max_pdu_m2s, p_cis_cfg->max_pdu_s2m,
		p_cis_cfg->phy_m2s, p_cis_cfg->phy_s2m, p_cis_cfg->ft_m2s, p_cis_cfg->ft_s2m,
		p_cis_cfg->nse);

	unicast_env.datapath_config.pres_delay_us = p_cig_cfg->tlatency_m2s_us;
}

static void on_unicast_server_cb_configure_codec_req(uint8_t conidx, uint8_t ase_instance_idx,
						     uint8_t ase_lid, uint8_t tgt_latency,
						     uint8_t tgt_phy, gaf_codec_id_t *p_codec_id,
						     const bap_cfg_ptr_t *p_cfg)
{
	uint16_t const size =
		sizeof(bap_cfg_t) + ((p_cfg->p_add_cfg != NULL) ? p_cfg->p_add_cfg->len : 0);
	bap_cfg_t *p_ase_codec_cfg = (bap_cfg_t *)malloc(size);
	uint8_t ase_lid_cfm = ase_instance_idx + unicast_env.nb_ases_sink * conidx;
	bap_qos_req_t qos_req = {
		.pres_delay_min_us = 7500,
		.pres_delay_max_us = 40000,
		.pref_pres_delay_min_us = 7500,
		.pref_pres_delay_max_us = 40000,
		.trans_latency_max_ms = 30,
		.framing = 0,
		.phy_bf = (GAP_PHY_LE_1MBPS | GAP_PHY_LE_2MBPS),
		.retx_nb = (tgt_latency == BAP_UC_TGT_LATENCY_LOWER) ? 5 : 13,
	};

	if (ase_instance_idx >= unicast_env.nb_ases_sink) {
		ase_lid_cfm = (unicast_env.nb_ases_sink * unicast_env.nb_connection) +
			      (ase_instance_idx - unicast_env.nb_ases_sink) +
			      unicast_env.nb_ases_src * conidx;
	}

	LOG_DBG("Configure Codec requested (ASE instance %d, ASE lid %d, conidx %u, ASE info "
		"index: %u)",
		ase_instance_idx, ase_lid, conidx, ase_lid_cfm);

	if (p_ase_codec_cfg == NULL) {
		goto bap_codec_config_cfm;
	}

	char const *const p_name = location_name[__builtin_ctz(p_cfg->param.location_bf)];

	LOG_DBG("    Codec %s, Freq: %s, Duration: %s, Length: %dB, Location: %s",
		(GAPI_CODEC_FORMAT_LC3 == p_codec_id->codec_id[0]) ? "LC3" : "not supported!",
		sampling_freq_name[p_cfg->param.sampling_freq],
		frame_dur_name[p_cfg->param.frame_dur], p_cfg->param.frame_octet, p_name);

	struct ase_info *const p_ase_info = &unicast_env.p_ase_info[ase_lid_cfm];

	/* TODO: Check where to capture presentation delay! */
	unicast_env.datapath_config.pres_delay_us = PRESENTATION_DELAY_US;
	unicast_env.datapath_config.sampling_rate_hz =
		sampling_freq_index_to_hz(p_cfg->param.sampling_freq);
	unicast_env.datapath_config.frame_duration_is_10ms = p_cfg->param.frame_dur;
	unicast_env.frame_octet = p_cfg->param.frame_octet;
	unicast_env.ase_lid_cfm = ase_lid_cfm;

	free(p_ase_info->p_codec_cfg);
	p_ase_info->p_codec_cfg = p_ase_codec_cfg;
	memcpy(&(p_ase_codec_cfg->param), &(p_cfg->param), sizeof(bap_cfg_param_t));
	if (p_cfg->p_add_cfg != NULL) {
		p_ase_codec_cfg->add_cfg.len = p_cfg->p_add_cfg->len;
		memcpy(&(p_ase_codec_cfg->add_cfg.data[0]), &(p_cfg->p_add_cfg->data[0]),
		       p_cfg->p_add_cfg->len);
	} else {
		p_ase_codec_cfg->add_cfg.len = 0;
	}

bap_codec_config_cfm:

	bap_uc_srv_configure_codec_cfm(
		conidx,
		(p_ase_codec_cfg) ? BAP_UC_CP_RSP_CODE_SUCCESS
				  : BAP_UC_CP_RSP_CODE_INSUFFICIENT_RESOURCES,
		(p_ase_codec_cfg) ? 0 : BAP_UC_CP_REASON_INVALID_ASE_CIS_MAPPING, ase_lid_cfm,
		&qos_req, p_ase_codec_cfg, 0, DATA_PATH_CONFIG);
}

#if !CONFIG_ALIF_BLE_ROM_IMAGE_V1_0 /* ROM version > 1.0 */
static void on_unicast_server_cb_configure_qos_req(uint8_t const ase_lid, uint8_t const stream_lid,
						   const bap_qos_cfg_t *const p_qos_cfg)
{
	bap_uc_srv_configure_qos_cfm(ase_lid, BAP_UC_CP_RSP_CODE_SUCCESS, 0);
	LOG_DBG("Configure QoS requested (ASE %d)", ase_lid);
}
#endif

static void on_unicast_server_cb_enable_req(uint8_t const ase_lid,
					    bap_cfg_metadata_ptr_t *const p_metadata)
{
	uint16_t const size =
		sizeof(bap_cfg_metadata_t) +
		((p_metadata->p_add_metadata != NULL) ? p_metadata->p_add_metadata->len : 0);
	bap_cfg_metadata_t *p_ase_metadata = (bap_cfg_metadata_t *)calloc(1, size);
	bool const valid_config = !!p_ase_metadata;

	if (p_ase_metadata) {
		struct ase_info *p_ase_info = &(unicast_env.p_ase_info[ase_lid]);

		/* Sanity check for paranaoia */
		assert(!p_ase_info->p_metadata);
		free(p_ase_info->p_metadata);

		p_ase_info->p_metadata = p_ase_metadata;
		p_ase_metadata->param.context_bf = p_metadata->param.context_bf;
		if (p_metadata->p_add_metadata) {
			memcpy(&(p_ase_metadata->add_metadata), p_metadata->p_add_metadata,
			       sizeof(p_metadata->p_add_metadata->len) +
				       p_metadata->p_add_metadata->len);
		}
	}

	bap_uc_srv_enable_cfm(ase_lid,
			      (p_ase_metadata) ? BAP_UC_CP_RSP_CODE_SUCCESS
					       : BAP_UC_CP_RSP_CODE_INSUFFICIENT_RESOURCES,
			      0, p_ase_metadata);

	LOG_DBG("Enable requested (ASE %d) valid:%u", ase_lid, valid_config);
}

static void on_unicast_server_cb_update_metadata_req(uint8_t const ase_lid,
						     bap_cfg_metadata_ptr_t *const p_metadata)
{
	uint16_t size =
		sizeof(bap_cfg_metadata_t) +
		((p_metadata->p_add_metadata != NULL) ? p_metadata->p_add_metadata->len : 0);
	bap_cfg_metadata_t *p_ase_metadata = (bap_cfg_metadata_t *)malloc(size);

	if (p_ase_metadata != NULL) {
		struct ase_info *p_ase_info = &(unicast_env.p_ase_info[ase_lid]);

		/* Sanity check for paranaoia */
		assert(p_ase_info->p_metadata != NULL);
		free(p_ase_info->p_metadata);

		memset(p_ase_metadata, 0, sizeof(bap_cfg_metadata_t));
		p_ase_info->p_metadata = p_ase_metadata;
		p_ase_metadata->param.context_bf = p_metadata->param.context_bf;
		if (p_metadata->p_add_metadata != NULL) {
			memcpy(&(p_ase_metadata->add_metadata), p_metadata->p_add_metadata,
			       sizeof(p_metadata->p_add_metadata->len) +
				       p_metadata->p_add_metadata->len);
		}
	}

	bap_uc_srv_update_metadata_cfm(ase_lid,
				       (p_ase_metadata != NULL)
					       ? BAP_UC_CP_RSP_CODE_SUCCESS
					       : BAP_UC_CP_RSP_CODE_INSUFFICIENT_RESOURCES,
				       0, p_ase_metadata);

	LOG_DBG("Update metadata requested (ASE %d)", ase_lid);
}

static void on_unicast_server_cb_release_req(uint8_t ase_lid)
{
	bap_uc_srv_release_cfm(ase_lid, BAP_UC_CP_RSP_CODE_SUCCESS, 0, true);
	LOG_DBG("Release requested (ASE %d)", ase_lid);
}

static void on_unicast_server_cb_dp_update_req(uint8_t const ase_lid, bool const start)
{
	LOG_DBG("ASE %u data path %s requested", ase_lid, start ? "START" : "STOP");

	bool const valid_config = DATA_PATH_CONFIG == bap_uc_srv_get_dp_id(ase_lid);

	bap_uc_srv_dp_update_cfm(ase_lid, valid_config);

	if (!valid_config) {
		LOG_ERR("Invalid data path configuration for ASE %d", ase_lid);
		return;
	}

	if (!start) {
		audio_datapath_cleanup();
		unicast_env.data_path_configured = false;
		unicast_env.nb_datapaths_sink = 0;
		return;
	}
	if ((unicast_env.nb_datapaths_sink - 1) != ase_lid) {
		return;
	}
#if !START_FROM_STREMING
	k_work_submit_to_queue(&worker_queue, &start_audio_job);
#endif
}

static const struct bap_uc_srv_cb bap_uc_srv_cb = {
	.cb_cmp_evt = on_unicast_server_cb_cmp_evt,
	.cb_quality_cmp_evt = on_unicast_server_cb_quality_cmp_evt,
	.cb_bond_data = on_unicast_server_cb_bond_data,
	.cb_ase_state = on_unicast_server_cb_ase_state,
	.cb_cis_state = on_unicast_server_cb_cis_state,
	.cb_configure_codec_req = on_unicast_server_cb_configure_codec_req,
#if !CONFIG_ALIF_BLE_ROM_IMAGE_V1_0 /* ROM version > 1.0 */
	.cb_configure_qos_req = on_unicast_server_cb_configure_qos_req,
#endif
	.cb_enable_req = on_unicast_server_cb_enable_req,
	.cb_update_metadata_req = on_unicast_server_cb_update_metadata_req,
	.cb_release_req = on_unicast_server_cb_release_req,
	.cb_dp_update_req = on_unicast_server_cb_dp_update_req,
};

/* ---------------------------------------------------------------------------------------- */
/* PAC Capability records and server */

static void on_capabilities_server_cb_bond_data(uint8_t const conidx, uint8_t const cli_cfg_bf,
						uint16_t const pac_cli_cfg_bf)
{
	LOG_DBG("PACS Bond Data (conidx:%d, cli_cfg_bf:0x%02X, pac_cli_cfg_bf:0x%02X)", conidx,
		cli_cfg_bf, pac_cli_cfg_bf);

	/* TODO: Store bond data... */
}

static void on_capabilities_server_cb_location_req(uint8_t const conidx,
#if !CONFIG_ALIF_BLE_ROM_IMAGE_V1_0 /* ROM version > 1.0 */
						   uint16_t const token,
#endif
						   uint8_t const direction,
						   uint32_t const location_bf)
{
	LOG_DBG("BAP CAP server_cb_location_req. conidx:%u, dir:%u, location:%u", conidx, direction,
		location_bf);
}

static struct bap_capa_srv_cb capa_srv_cbs = {
	.cb_bond_data = on_capabilities_server_cb_bond_data,
#if !CONFIG_ALIF_BLE_ROM_IMAGE_V1_0 /* ROM version > 1.0 */
	.cb_location_req = on_capabilities_server_cb_location_req,
#else /* ROM version 1.0 */
	.cb_location = on_capabilities_server_cb_location_req,
#endif
};

/** Maximum number of records per PAC characteristic - Shall be higher than 0 */
#define NB_RECORDS_PER_PAC_MAX (1U)
/** Compute number of PAC characteristics based on number of records */
#define NB_PAC(nb_records)     ROUND_UP((nb_records), NB_RECORDS_PER_PAC_MAX)

#if (NB_RECORDS_PER_PAC_MAX == 0)
#error "Invalid value for NB_RECORDS_PER_PAC_MAX"
#endif

static inline bap_capa_t *get_pointer_to_capa_record(bap_capa_t *const p_base, size_t const index)
{
	return (bap_capa_t *)((uintptr_t)p_base + CO_ALIGN4_HI(sizeof(bap_capa_t)) * index);
}

static size_t capabilities_server_prepare_records_sink(bap_capa_t **pp_capa_out)
{
	/** Following codec capabilities settings are mandatory for Sink direction
	 *     -> 16_2 : LC3 / 16kHz / 10ms / 40 bytes
	 *     -> 24_2 : LC3 / 24kHz / 10ms / 60 bytes
	 * Following codec capabilities are mandatory when Unicast Media Receiver role is
	 * supported
	 *     -> 48_1 : LC3 / 48kHz / 7.5ms / 75 bytes
	 *     -> 48_2 : LC3 / 48kHz / 10ms / 100 bytes
	 *     -> 48_3 : LC3 / 48kHz / 7.5ms / 90 bytes
	 *     -> 48_4 : LC3 / 48kHz / 10ms / 120 bytes
	 *     -> 48_5 : LC3 / 48kHz / 7.5ms / 117 bytes
	 *     -> 48_6 : LC3 / 48kHz / 10ms / 155 bytes
	 * Following codec capabilities are mandatory when Call Terminal role is supported
	 *     -> 32_1 : LC3 / 24kHz / 7.5ms / 60 bytes
	 *     -> 32_2 : LC3 / 24kHz / 10ms / 80 bytes
	 * Following codec capability is mandatory when Call Gateway role is supported
	 *     -> 32_2 : LC3 / 24kHz / 10ms / 80 bytes
	 */
	bap_capa_t *p_capa;

#if 48000 == I2S_BUS_SAMPLE_RATE
	enum tmap_role_bf role_bitmap = TMAP_ROLE_UMR_BIT;
#elif 32000 == I2S_BUS_SAMPLE_RATE
	enum tmap_role_bf role_bitmap = TMAP_ROLE_CT_BIT;
#else
	enum tmap_role_bf role_bitmap = 0;
#endif
	/**
	 * See table 'Unicast Server audio capability support requirements' in BAP 1.0.3
	 * specification
	 * At least 2 record are used, one for 16kHz, one for 24kHz
	 */
	size_t nb_records_sink = 1
#if 24000 <= I2S_BUS_SAMPLE_RATE
				 + 1
#endif
		;

	if (role_bitmap & TMAP_ROLE_UMR_BIT) {
		nb_records_sink++; /* One for all supported 48kHz capabilities */
	}

	if (role_bitmap & (TMAP_ROLE_CT_BIT | TMAP_ROLE_CG_BIT)) {
		nb_records_sink++; /* One for all supported 32kHz capabilities */
	}

	p_capa = (bap_capa_t *)malloc(CO_ALIGN4_HI(sizeof(bap_capa_t)) * nb_records_sink);
	ASSERT_ERR(p_capa != NULL);

	*pp_capa_out = p_capa;

	p_capa->param.sampling_freq_bf = BAP_SAMPLING_FREQ_16000HZ_BIT;
	p_capa->param.frame_dur_bf = BAP_FRAME_DUR_10MS_BIT;
	p_capa->param.chan_cnt_bf = 1;
	p_capa->param.frame_octet_min = 40;
	p_capa->param.frame_octet_max = 40;
	p_capa->param.max_frames_sdu = 1;
	p_capa->add_capa.len = 0;

	if (1 < nb_records_sink) {
		p_capa = (bap_capa_t *)((uintptr_t)p_capa + CO_ALIGN4_HI(sizeof(bap_capa_t)));
		p_capa->param.sampling_freq_bf = BAP_SAMPLING_FREQ_24000HZ_BIT;
		p_capa->param.frame_dur_bf = BAP_FRAME_DUR_10MS_BIT;
		p_capa->param.chan_cnt_bf = 1;
		p_capa->param.frame_octet_min = 60;
		p_capa->param.frame_octet_max = 60;
		p_capa->param.max_frames_sdu = 1;
		p_capa->add_capa.len = 0;
	}

	if (role_bitmap & TMAP_ROLE_UMR_BIT) {
		p_capa = (bap_capa_t *)((uintptr_t)p_capa + CO_ALIGN4_HI(sizeof(bap_capa_t)));
		p_capa->param.sampling_freq_bf = BAP_SAMPLING_FREQ_48000HZ_BIT;
		p_capa->param.frame_dur_bf = /*BAP_FRAME_DUR_7_5MS_BIT |*/ BAP_FRAME_DUR_10MS_BIT;
		p_capa->param.chan_cnt_bf = 1;
		p_capa->param.frame_octet_min = 75;
		p_capa->param.frame_octet_max = 155;
		p_capa->param.max_frames_sdu = 1;
		p_capa->add_capa.len = 0;
	}

	if (role_bitmap & TMAP_ROLE_CT_BIT) {
		p_capa = (bap_capa_t *)((uintptr_t)p_capa + CO_ALIGN4_HI(sizeof(bap_capa_t)));
		p_capa->param.sampling_freq_bf = BAP_SAMPLING_FREQ_32000HZ_BIT;
		p_capa->param.frame_dur_bf = /*BAP_FRAME_DUR_7_5MS_BIT |*/ BAP_FRAME_DUR_10MS_BIT;
		p_capa->param.chan_cnt_bf = 1;
		p_capa->param.frame_octet_min = 60;
		p_capa->param.frame_octet_max = 80;
		p_capa->param.max_frames_sdu = 1;
		p_capa->add_capa.len = 0;
	}

	return nb_records_sink;
}

static size_t capabilities_server_prepare_records_src(bap_capa_t **pp_capa_out)
{
	/**
	 * Following codec capabilities settings are mandatory for Source direction
	 *     -> 16_2 : LC3 / 16kHz / 10ms / 40 bytes
	 * Following codec capabilities are mandatory when Unicast Media Sender role is
	 * supported
	 *     -> 48_2 : LC3 / 48kHz / 10ms / 100 bytes
	 *     At least one of:
	 *         -> 48_4 : LC3 / 48kHz / 10ms / 120 bytes
	 *         -> 48_6 : LC3 / 48kHz / 10ms / 155 bytes
	 * Following codec capabilities are mandatory when Call Terminal role is supported
	 *     -> 32_2 : LC3 / 32kHz / 10ms / 80 bytes
	 * Following codec capability is mandatory when Call Gateway role is supported
	 *     -> 32_2 : LC3 / 32kHz / 10ms / 80 bytes
	 */
	bap_capa_t *p_capa;

#if 48000 == I2S_BUS_SAMPLE_RATE
	enum tmap_role_bf role_bitmap = TMAP_ROLE_UMR_BIT;
#elif 32000 == I2S_BUS_SAMPLE_RATE
	enum tmap_role_bf role_bitmap = TMAP_ROLE_CT_BIT;
#else
	enum tmap_role_bf role_bitmap = 0;
#endif

	/* At least 1 record is used, for 16kHz */
	size_t nb_records_src = 1;

	if (role_bitmap & (TMAP_ROLE_CT_BIT | TMAP_ROLE_CG_BIT)) {
		nb_records_src++; /* One for all supported 32kHz capabilities */
	}

	if (role_bitmap & TMAP_ROLE_UMS_BIT) {
		nb_records_src++; /* One for all supported 48kHz capabilities */
	}

	p_capa = (bap_capa_t *)malloc(CO_ALIGN4_HI(sizeof(bap_capa_t)) * nb_records_src);
	ASSERT_ERR(p_capa != NULL);

	*pp_capa_out = p_capa;

	p_capa->param.sampling_freq_bf = BAP_SAMPLING_FREQ_16000HZ_BIT;
	p_capa->param.frame_dur_bf = BAP_FRAME_DUR_10MS_BIT;
	p_capa->param.chan_cnt_bf = 1;
	p_capa->param.frame_octet_min = 40;
	p_capa->param.frame_octet_max = 40;
	p_capa->param.max_frames_sdu = 1;
	p_capa->add_capa.len = 0;

	if (role_bitmap & TMAP_ROLE_CT_BIT) {
		p_capa = (bap_capa_t *)((uintptr_t)p_capa + CO_ALIGN4_HI(sizeof(bap_capa_t)));
		p_capa->param.sampling_freq_bf = BAP_SAMPLING_FREQ_32000HZ_BIT;
		p_capa->param.frame_dur_bf = BAP_FRAME_DUR_7_5MS_BIT | BAP_FRAME_DUR_10MS_BIT;
		p_capa->param.chan_cnt_bf = 1;
		p_capa->param.frame_octet_min = 60;
		p_capa->param.frame_octet_max = 80;
		p_capa->param.max_frames_sdu = 1;
		p_capa->add_capa.len = 0;
	}

	if (role_bitmap & TMAP_ROLE_UMS_BIT) {
		p_capa = (bap_capa_t *)((uintptr_t)p_capa + CO_ALIGN4_HI(sizeof(bap_capa_t)));
		p_capa->param.sampling_freq_bf = BAP_SAMPLING_FREQ_48000HZ_BIT;
		p_capa->param.frame_dur_bf = BAP_FRAME_DUR_10MS_BIT;
		p_capa->param.chan_cnt_bf = 1;
		p_capa->param.frame_octet_min = 120;
		p_capa->param.frame_octet_max = 155;
		p_capa->param.max_frames_sdu = 1;
		p_capa->add_capa.len = 0;
	}

	return nb_records_src;
}

static int configure_bap_capabilities(uint32_t const location_bf_sink,
				      uint32_t const location_bf_src)
{
	/**
	 * Codec Capabilities structure (Sink direction)
	 */
	bap_capa_t *p_capa_sink = NULL;
	/**
	 * Codec Capabilities structure (Source direction)
	 */
	bap_capa_t *p_capa_src = NULL;

	size_t const nb_records_sink =
		(location_bf_sink) ? capabilities_server_prepare_records_sink(&p_capa_sink) : 0;
	size_t const nb_records_src =
		(location_bf_src) ? capabilities_server_prepare_records_src(&p_capa_src) : 0;

	unicast_env.p_capa_sink = p_capa_sink;
	unicast_env.p_capa_src = p_capa_src;

	/** Number of Sink PAC characteristics */
	uint8_t const nb_pacs_sink = NB_PAC(nb_records_sink);
	/** Number of Source PAC characteristics */
	uint8_t const nb_pacs_src = NB_PAC(nb_records_src);

	bap_capa_srv_cfg_t capa_srv_cfg = {
		.nb_pacs_sink = nb_pacs_sink,
		.nb_pacs_src = nb_pacs_src,
		.cfg_bf = (BAP_CAPA_SRV_CFG_PAC_NTF_BIT | BAP_CAPA_SRV_CFG_LOC_NTF_BIT |
			   BAP_CAPA_SRV_CFG_SUPP_CONTEXT_NTF_BIT | BAP_CAPA_SRV_CFG_LOC_WR_BIT |
			   BAP_CAPA_SRV_CFG_LOC_SUPP_BIT | BAP_CAPA_SRV_CFG_CHECK_LOCK_BIT),
		.pref_mtu = 0,
		.shdl = GATT_INVALID_HDL,
		.location_bf_sink = location_bf_sink,
		.location_bf_src = location_bf_src,
		.supp_context_bf_sink = (nb_pacs_sink) ? (BAP_CONTEXT_TYPE_ALL) : 0,
		.supp_context_bf_src = (nb_pacs_src) ? BAP_CONTEXT_TYPE_UNSPECIFIED_BIT : 0,
	};

	uint16_t err;

	err = bap_capa_srv_configure(&capa_srv_cbs, &capa_srv_cfg);
	if (GAF_ERR_NO_ERROR != err) {
		LOG_ERR("BAP capability server configuration failed! error:%u", err);
		return -1;
	}

	bap_capa_t *p_capa;
	gaf_codec_id_t const codec_id = GAF_CODEC_ID_LC3;
	uint8_t pac_lid = 0;
	uint8_t nb_records = 0;

	if (nb_records_sink && p_capa_sink) {
		p_capa = p_capa_sink;

		for (uint8_t cnt = 0; cnt < nb_records_sink; cnt++) {
			if (NB_RECORDS_PER_PAC_MAX <= nb_records) {
				pac_lid++;
				nb_records = 0;
			}

			err = bap_capa_srv_set_record(pac_lid, cnt + 1, &codec_id, p_capa, NULL);
			if (GAF_ERR_NO_ERROR != err) {
				LOG_ERR("BAP capability record add failed! error:%u", err);
				return -1;
			}

			p_capa = (bap_capa_t *)((uintptr_t)p_capa +
						CO_ALIGN4_HI(sizeof(bap_capa_t)));
			nb_records++;
		}

		pac_lid++;
		nb_records = 0;
	}

	if (nb_records_src && p_capa_src) {
		p_capa = p_capa_src;

		for (uint8_t cnt = 0; cnt < nb_records_src; cnt++) {
			if (NB_RECORDS_PER_PAC_MAX <= nb_records) {
				pac_lid++;
				nb_records = 0;
			}

			err = bap_capa_srv_set_record(pac_lid, nb_records_sink + cnt + 1, &codec_id,
						      p_capa, NULL);
			if (GAF_ERR_NO_ERROR != err) {
				LOG_ERR("BAP capability record add failed! error:%u", err);
				return -1;
			}

			p_capa = (bap_capa_t *)((uintptr_t)p_capa +
						CO_ALIGN4_HI(sizeof(bap_capa_t)));
			nb_records++;
		}
	}

	if (!bap_capa_srv_is_configured()) {
		LOG_ERR("BAP Capa server is not ready!");
		return -1;
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------- */

int unicast_sink_init(void)
{
	k_work_queue_start(&worker_queue, worker_task_stack,
			   K_KERNEL_STACK_SIZEOF(worker_task_stack), WORKER_PRIORITY, NULL);
	k_thread_name_set(&worker_queue.thread, "unicast_srv_workq");

	struct gaf_adv_cfg config = {
		.nb_sets = 2,
	};

	gaf_adv_configure(&config, &gaf_adv_cbs);
	LOG_DBG("GAF advertiser is configured");

	unicast_env.nb_connection = MAX_NUMBER_OF_CONNECTIONS;
	unicast_env.nb_ases_sink = __builtin_popcount(LOCATION_SINK);
	unicast_env.nb_ases_src = __builtin_popcount(LOCATION_SOURCE);
	LOG_INF("NBR ASE sink:%u, source:%u", unicast_env.nb_ases_sink, unicast_env.nb_ases_src);

	bap_uc_srv_cfg_t uc_srv_cfg = {
		.nb_ase_chars_sink = unicast_env.nb_ases_sink,
		.nb_ase_chars_src = unicast_env.nb_ases_src,
		.nb_ases_cfg = (unicast_env.nb_ases_sink + unicast_env.nb_ases_src) *
			       unicast_env.nb_connection,
		.cfg_bf = 0,
		.pref_mtu = 0,
		.shdl = GATT_INVALID_HDL,
	};

	unicast_env.p_ase_info = calloc(uc_srv_cfg.nb_ases_cfg, sizeof(*unicast_env.p_ase_info));
	if (!unicast_env.p_ase_info) {
		LOG_ERR("Unable to alloc memory for ASE configurations!");
		return -1;
	}

	uint16_t const err = bap_uc_srv_configure(&bap_uc_srv_cb, &uc_srv_cfg);

	if (err) {
		LOG_ERR("Error %u (0x%02X) configuring bap_uc_srv_configure", err, err);
		return -1;
	}

	if (!bap_uc_srv_is_configured()) {
		LOG_ERR("BAP unicast server is not configured!");
		return -1;
	}
	LOG_DBG("BAP unicast server is configured");

	return configure_bap_capabilities(LOCATION_SINK, LOCATION_SOURCE);
}

int unicast_sink_adv_start(void)
{
	if (unicast_env.advertising_ongoing) {
		LOG_DBG("...advertising already ongoing...");
		return 0;
	}

	extern const char device_name[];

	size_t const name_len = strlen(device_name);
	uint16_t err;
	char adv_data[32];

	adv_data[0] = name_len + 1;
	adv_data[1] = GAP_AD_TYPE_COMPLETE_NAME;
	strncpy(&adv_data[2], device_name, sizeof(adv_data) - 2);

#define ADV_SET_LOCAL_IDX     0
#define ADV_CONFIG            (GAPM_ADV_MODE_GEN_DISC | GAF_ADV_CFG_GENERAL_ANNOUNCEMENT_BIT)
#define ADV_TIMEOUT           0 /* Infinite (until explicitly stopped) */
#define ADV_SID               1
#define ADV_INTERVAL_QUICK_MS 20
#define ADV_INTERVAL_MS       200
#define ADV_PHY               GAP_PHY_2MBPS
#define ADV_PHY_2nd           GAP_PHY_1MBPS
#define ADV_MAX_TX_PWR        0
#define ADV_MAX_SKIP          2

#if 0
	err = gaf_adv_set_params(ADV_SET_LOCAL_IDX, ADV_INTERVAL_QUICK_MS, ADV_INTERVAL_MS, ADV_PHY,
				 ADV_PHY_2nd, ADV_ALL_CHNLS_EN, ADV_MAX_TX_PWR, ADV_MAX_SKIP);
	if (err) {
		LOG_ERR("Failed to set advertising params, err %u (0x%02X)", err, err);
		return -1;
	}
#endif
	err = gaf_adv_start(ADV_SET_LOCAL_IDX, ADV_CONFIG, ADV_TIMEOUT, ADV_SID, (name_len + 2),
			    (uint8_t *)adv_data, NULL);
	if (err) {
		LOG_ERR("Failed to start advertising, err %u (0x%02X)", err, err);
		return -1;
	}

	return 0;
}

int unicast_sink_adv_stop(void)
{
	if (!unicast_env.advertising_ongoing) {
		LOG_DBG("...advertising not ongoing...");
		return 0;
	}

	uint16_t err = gaf_adv_stop(ADV_SET_LOCAL_IDX);

	if (err) {
		LOG_ERR("Failed to stop advertising, err %u (0x%02X)", err, err);
		return -1;
	}

	return 0;
}

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

#include "alif_ble.h"
#include "gaf_cli.h"
#include "gaf_scan.h"
#include "bap_capa_cli.h"
#include "bap_uc_cli.h"
#include "bap_uc_cfg.h"
#include "co_utils.h"
#include "tmap.h"
#include "gapc.h"

#include "gapi_isooshm.h"

#include "main.h"
#include "unicast_source.h"
#include "storage.h"
#include "audio_datapath.h"

LOG_MODULE_REGISTER(unicast_source, CONFIG_UNICAST_SOURCE_LOG_LEVEL);

#define STREAM_SINK_ASE_MAX_CNT   2
#define STREAM_SOURCE_ASE_MAX_CNT 0
#define STREAM_SINK_PAC_MAX_CNT   5
#define STREAM_SOURCE_PAC_MAX_CNT 2

#define DISCOVER_PACS_BEFORE_ASCS 1
#define BUTTON_ENABLED            0

#define SCAN_DURATION_SEC     5
/** See enum gaf_scan_cfg_bf for more information */
#define SCAN_CONFIG_BITS      (GAF_SCAN_CFG_ASCS_REQ_BIT /*| GAF_SCAN_CFG_TMAS_REQ_BIT*/)
#define PRESENTATION_DELAY_US (CONFIG_LE_AUDIO_PRESENTATION_DELAY_MS * 1000)
#define CONTROL_DELAY_US      10000
#define DATA_PATH_CONFIG      DATA_PATH_ISOOSHM
#define MAX_TIMING_LATENCY_MS 40
#define STREAM_CIS_ID_BASE    (0U)
#define STREAM_PHY_TYPE       BAP_UC_TGT_PHY_2M

/** This can be removed when the i2s driver is updated to support dynamic sample rate */
#define I2S_BUS_SAMPLE_RATE DT_PROP(I2S_NODE, sample_rate)

/** This can be removed when the i2s driver is updated to support dynamic sample rate */
#if I2S_BUS_SAMPLE_RATE != 16000 && I2S_BUS_SAMPLE_RATE != 24000 &&                                \
	I2S_BUS_SAMPLE_RATE != 32000 && I2S_BUS_SAMPLE_RATE != 48000
#error "Invalid sample rate"
#endif

/** Group type values */
enum unicast_client_group_type {
	/** Media */
	UNICAST_CLIENT_GROUP_TYPE_MEDIA = 0U,
	/** Call */
	UNICAST_CLIENT_GROUP_TYPE_CALL,
	/** Ringtone */
	UNICAST_CLIENT_GROUP_TYPE_RINGTONE,
	/** Keep this last */
	UNICAST_CLIENT_GROUP_TYPE_MAX,
};

enum ase_state_bits {
	ASE_STATE_ZERO = 0,
	ASE_STATE_INITIALIZED_POS = 0,
	ASE_STATE_INITIALIZED = BIT(ASE_STATE_INITIALIZED_POS),
	ASE_STATE_CODEC_CONFIGURED_POS = 1,
	ASE_STATE_CODEC_CONFIGURED = BIT(ASE_STATE_CODEC_CONFIGURED_POS),
	ASE_STATE_QOS_CONFIGURED_POS = 2,
	ASE_STATE_QOS_CONFIGURED = BIT(ASE_STATE_QOS_CONFIGURED_POS),
	ASE_STATE_ENABLED_POS = 4,
	ASE_STATE_ENABLED = BIT(ASE_STATE_ENABLED_POS),
	ASE_STATE_STREAMING_POS = 3,
	ASE_STATE_STREAMING = BIT(ASE_STATE_STREAMING_POS),
};

/** ASE structure */
struct unicast_client_ase {

	/** Connection index */
	uint8_t conidx;
	/** ASE local index */
	uint8_t ase_lid;
	/** ASE instance index */
	uint8_t ase_instance_idx;
	/** CIS ID */
	uint8_t cis_id;
	/** Number of octets */
	uint8_t number_of_octets;
	/** Stream index */
	uint8_t stream_lid;
	/** \ref enum ase_state_bits for state bitmask info*/
	uint8_t state_bitmask;
	/** Retransmission number */
	uint8_t retx_number;
};

struct pac_capa {
	uint32_t sampling_freq_hz;
	uint8_t frame_duration_bf;
	uint8_t frame_octet_min;
	uint8_t frame_octet_max;
	uint8_t max_frames_sdu;
};

struct unicast_source_env {
	/** Bonding data */
	struct bap_uc_cli_ascs ascs_bond_data;
	/** Peripheral info */
	struct {
		/** Pointer to device address */
		gap_bdaddr_t addr;
		/** Device name */
		char device_name[32];
		/** Pointer to RSI value */
		atc_csis_rsi_t rsi;
		/** Info flag bits */
		uint8_t info_flags;
	} peripheral_info;

	struct audio_datapath_config datapath_config;
	bool data_path_configured;

	/** ASE information for Sink and Source direction */
	struct unicast_client_ase ase_sink[STREAM_SINK_ASE_MAX_CNT];
	struct unicast_client_ase ase_source[STREAM_SOURCE_ASE_MAX_CNT];
	uint8_t nb_ases_sink;
	uint8_t nb_ases_source;

	/** Group local index */
	uint8_t group_lid[UNICAST_CLIENT_GROUP_TYPE_MAX];
	/** Status bit field */
	uint8_t status_bf;

	/** PACS data from server */
	struct pac_capa pacs_sink[STREAM_SINK_PAC_MAX_CNT];
	struct pac_capa pacs_source[STREAM_SOURCE_PAC_MAX_CNT];
	uint8_t nb_pacs_sink;
	uint8_t nb_pacs_source;

	/** Data Path ID */
	uint8_t dp_id;
	/** Codec ID */
	gaf_codec_id_t codec_id;
};

static struct unicast_source_env unicast_env;

const char peripheral_name[] = CONFIG_PERIPHERAL_NAME;

#define WORKER_PRIORITY   6
#define WORKER_STACK_SIZE 2048

K_KERNEL_STACK_DEFINE(worker_task_stack, WORKER_STACK_SIZE);
static struct k_work_q worker_queue;

K_SEM_DEFINE(bap_complete_sem, 0, 1);

void wait_bap_complete(void)
{
	if (k_sem_take(&bap_complete_sem, K_MSEC(2000))) {
		LOG_ERR("Failed to get BAP complete semaphore!");
	}
}
#define bap_ready_sem_give() k_sem_give(&bap_complete_sem)

/* ---------------------------------------------------------------------------------------- */
const char *bdaddr_str(const gap_bdaddr_t *p_addr)
{
	static char addr_str[18];

	snprintk(addr_str, sizeof(addr_str), "%02X:%02X:%02X:%02X:%02X:%02X", p_addr->addr[0],
		 p_addr->addr[1], p_addr->addr[2], p_addr->addr[3], p_addr->addr[4],
		 p_addr->addr[5]);
	return addr_str;
}

static int bap_sampling_freq_from_hz(uint32_t const sampling_freq_hz)
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

static uint32_t bap_sampling_freq_index_to_hz(uint32_t const index)
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

static uint32_t bap_sampling_freq_bitmask_to_hz(uint32_t const bitmask)
{
	return bap_sampling_freq_index_to_hz((!bitmask) ? 0 : 32 - __builtin_clz(bitmask));
}

/* ---------------------------------------------------------------------------------------- */

static void *get_best_stream(void)
{
	struct pac_capa *p_pac = NULL;
#if !I2S_BUS_SAMPLE_RATE
	uint32_t sampling_freq_hz = 0;
#endif
	for (size_t iter = 0; iter < unicast_env.nb_pacs_sink; iter++) {
		if (
#if !I2S_BUS_SAMPLE_RATE
			sampling_freq_hz < unicast_env.pacs_sink[iter].sampling_freq_hz
#else
			I2S_BUS_SAMPLE_RATE == unicast_env.pacs_sink[iter].sampling_freq_hz
#endif
		) {
#if !I2S_BUS_SAMPLE_RATE
			sampling_freq_hz = unicast_env.pacs_sink[iter].sampling_freq_hz;
#endif
			p_pac = &unicast_env.pacs_sink[iter];
		}
	}
	return p_pac;
}

static void configure_streams(struct k_work *p_job)
{
	/* TODO: use work context to deliver client group type.
	 * struct ooo * p_data = CONTAINER_OF(p_job, struct ooo, work);
	 */
	ARG_UNUSED(p_job);
	uint_fast8_t const type = UNICAST_CLIENT_GROUP_TYPE_MEDIA;

	if (GAF_INVALID_LID != unicast_env.group_lid[type]) {
		LOG_INF("Removing ASE group: %u", unicast_env.group_lid[type]);
		bap_uc_cli_remove_group(unicast_env.group_lid[type]);
		unicast_env.group_lid[type] = GAF_INVALID_LID;
		return;
	}

	struct pac_capa *p_pac = get_best_stream();

	if (!p_pac) {
		LOG_WRN("Unable to find a valid PAC");
		return;
	}

	LOG_INF("Starting streaming with sampling frequency: %uHz", p_pac->sampling_freq_hz);

	uint32_t sdu_intv_us = 0;
	enum bap_frame_dur frame_dur = BAP_FRAME_DUR_10MS;

	if ((p_pac->frame_duration_bf & BAP_FRAME_DUR_7_5MS_BIT) &&
	    !(p_pac->frame_duration_bf & BAP_FRAME_DUR_10MS_PREF_BIT)) {
		sdu_intv_us = 7500;
		frame_dur = BAP_FRAME_DUR_7_5MS;
	} else if (p_pac->frame_duration_bf & BAP_FRAME_DUR_10MS_BIT) {
		sdu_intv_us = 10000;
	}
	__ASSERT(sdu_intv_us, "Invalid frame duration");

	bap_uc_cli_grp_param_t grp_param = {
		.sdu_intv_m2s_us = sdu_intv_us,
		.sdu_intv_s2m_us = sdu_intv_us,
		.packing = 0,
		.framing = ISO_UNFRAMED_MODE,
		.sca = 0,
		.tlatency_m2s_ms = MAX_TIMING_LATENCY_MS,
		.tlatency_s2m_ms = MAX_TIMING_LATENCY_MS,
	};
	uint16_t err;

	err = bap_uc_cli_create_group(1 + UNICAST_CLIENT_GROUP_TYPE_MEDIA, &grp_param,
				      &unicast_env.group_lid[type]);
	if (GAF_ERR_NO_ERROR != err) {
		LOG_ERR("Failed to create group! error %u", err);
		return;
	}

	LOG_INF("ASE group %u created", unicast_env.group_lid[type]);

	struct bap_cfg codec_cfg = {
		/* TODO: SDU size bigger than 84B cause issues with encoding */
		.param.frame_octet = ROUND_UP(84, sizeof(uint32_t)),
		.param.sampling_freq = bap_sampling_freq_from_hz(p_pac->sampling_freq_hz),
		.param.frame_dur = frame_dur,
		.param.frames_sdu = p_pac->max_frames_sdu,
		.param.location_bf = 0,
		.add_cfg.len = 0,
	};
#if 1
	if (p_pac->frame_octet_min == p_pac->frame_octet_max) {
		codec_cfg.param.frame_octet = p_pac->frame_octet_max;
	} else if (p_pac->frame_octet_max < codec_cfg.param.frame_octet) {
		codec_cfg.param.frame_octet = p_pac->frame_octet_max;
	} else if (codec_cfg.param.frame_octet < p_pac->frame_octet_min) {
		codec_cfg.param.frame_octet = p_pac->frame_octet_min;
	}
#endif
	gaf_codec_id_t codec_id = GAF_CODEC_ID_LC3;

	for (size_t iter = 0; iter < ARRAY_SIZE(unicast_env.ase_sink); iter++) {
		struct unicast_client_ase *p_ase = &unicast_env.ase_sink[iter];

		if (GAF_INVALID_LID != p_ase->ase_lid) {
			continue;
		}

		p_ase->number_of_octets = codec_cfg.param.frame_octet;
		p_ase->ase_lid = iter;
		p_ase->cis_id = (UNICAST_CLIENT_GROUP_TYPE_MEDIA << 2) + STREAM_CIS_ID_BASE + iter;

		codec_cfg.param.location_bf = CO_BIT(GAF_LOC_FRONT_LEFT_POS + iter);

		LOG_DBG("Codec setup: conidx %u, ase_instance_idx %u, ase_lid %u, octets %uB",
			p_ase->conidx, p_ase->ase_instance_idx, p_ase->ase_lid,
			codec_cfg.param.frame_octet);

		err = bap_uc_cli_configure_codec(p_ase->conidx, p_ase->ase_instance_idx,
						 p_ase->ase_lid, DATA_PATH_CONFIG, &codec_id,
						 BAP_UC_TGT_LATENCY_RELIABLE, STREAM_PHY_TYPE,
						 CONTROL_DELAY_US, &codec_cfg);
		if (GAF_ERR_NO_ERROR != err) {
			LOG_ERR("Failed to configure codec! error %u", err);
			return;
		}
		wait_bap_complete();
	}
}

static K_WORK_DEFINE(configure_streams_job, configure_streams);

static void configure_qos(struct k_work *p_job)
{
	/* TODO: use work context to deliver client group type.
	 * struct ooo * p_data = CONTAINER_OF(p_job, struct ooo, work);
	 */
	ARG_UNUSED(p_job);
	uint16_t err;
	uint_fast8_t const type = UNICAST_CLIENT_GROUP_TYPE_MEDIA;

	for (size_t iter = 0; iter < ARRAY_SIZE(unicast_env.ase_sink); iter++) {
		struct unicast_client_ase *p_ase = &unicast_env.ase_sink[iter];

		if (GAF_INVALID_LID == p_ase->ase_lid || GAF_INVALID_LID == p_ase->cis_id ||
		    !(p_ase->state_bitmask & ASE_STATE_CODEC_CONFIGURED)) {
			continue;
		}

		struct bap_uc_cli_qos_cfg qos_cfg = {
			.phy = STREAM_PHY_TYPE,
			.retx_nb = p_ase->retx_number,
			.max_sdu_size = p_ase->number_of_octets,
			.pres_delay_us = PRESENTATION_DELAY_US,
		};

		LOG_DBG("ASE %u, CIS %u QoS configing...", p_ase->ase_lid, p_ase->cis_id);
		err = bap_uc_cli_configure_qos(p_ase->ase_lid, unicast_env.group_lid[type],
					       p_ase->cis_id, &qos_cfg);
		if (GAF_ERR_NO_ERROR != err) {
			LOG_ERR("ASE %u, CIS %u Failed to configure qos! error %u", p_ase->ase_lid,
				p_ase->cis_id, err);
			continue;
		}
		wait_bap_complete();
	}
}

static K_WORK_DEFINE(qos_job, configure_qos);

static void enable_streaming(struct k_work *const p_job)
{
	ARG_UNUSED(p_job);

	bap_cfg_metadata_t cfg_metadata = {
		.param.context_bf = BAP_CONTEXT_TYPE_UNSPECIFIED_BIT,
		.add_metadata.len = 0,
	};
	uint16_t err;

	for (size_t iter = 0; iter < ARRAY_SIZE(unicast_env.ase_sink); iter++) {
		struct unicast_client_ase *const p_ase = &unicast_env.ase_sink[iter];
		uint_fast8_t mask = p_ase->state_bitmask;

		if (GAF_INVALID_LID == p_ase->ase_lid || GAF_INVALID_LID == p_ase->cis_id ||
		    !(mask & ASE_STATE_QOS_CONFIGURED) || (mask & ASE_STATE_ENABLED)) {
			continue;
		}

		LOG_INF("ASE %u enabling...!", p_ase->ase_lid);
		err = bap_uc_cli_enable(p_ase->ase_lid, &cfg_metadata);
		if (GAF_ERR_NO_ERROR != err) {
			LOG_ERR("Failed to enable ASE %u! error %u", p_ase->ase_lid, err);
			continue;
		}
		p_ase->state_bitmask = mask | ASE_STATE_ENABLED;
	}
}

static K_WORK_DEFINE(enable_job, enable_streaming);

/** Handle graceful shutdown of the streaming */
#if 0
static void disable_streaming(struct k_work *const p_job)
{
	ARG_UNUSED(p_job);

	uint16_t err;

	for (size_t iter = 0; iter < ARRAY_SIZE(unicast_env.ase_sink); iter++) {
		struct unicast_client_ase *const p_ase = &unicast_env.ase_sink[iter];
		uint_fast8_t mask = p_ase->state_bitmask;

		if (GAF_INVALID_LID == p_ase->ase_lid || GAF_INVALID_LID == p_ase->cis_id ||
		    !(mask & ASE_STATE_ENABLED)) {
			continue;
		}

		LOG_INF("ASE %u disabling...", p_ase->ase_lid);
		err = bap_uc_cli_disable(p_ase->ase_lid);
		if (GAF_ERR_NO_ERROR != err) {
			LOG_ERR("Failed to disable ASE %u! error %u", p_ase->ase_lid, err);
			continue;
		}
		wait_bap_complete();
		p_ase->state_bitmask = mask & ~(ASE_STATE_STREAMING | ASE_STATE_ENABLED);

/* TODO: just stop...??? */
#if 0
		audio_datapath_cleanup_source();
#endif
	}
}

static K_WORK_DEFINE(disable_job, disable_streaming);
#endif

static void configure_datapath(struct k_work *p_job)
{
	ARG_UNUSED(p_job);
	k_sleep(K_MSEC(500));
	int retval;

	if (unicast_env.data_path_configured) {
		LOG_ERR("Data path already configured!");
		return;
	}

	retval = audio_datapath_create_source(&unicast_env.datapath_config);
	__ASSERT(retval == 0, "Failed to create datapath source");
	retval = audio_datapath_start_source();
	__ASSERT(retval == 0, "Failed to start datapath source");

	unicast_env.data_path_configured = true;
}

/* ---------------------------------------------------------------------------------------- */
/** GAF Client callbacks */

static void applet_gaf_scanning_cb_cmp_evt(uint8_t const cmd_type, uint16_t const status)
{
	__ASSERT(GAF_ERR_NO_ERROR == status, "status %u, cmd_type %u", status, cmd_type);
	if (cmd_type == GAF_SCAN_CMD_TYPE_STOP) {
		LOG_INF("GAF scanning stopped");
	} else if (cmd_type == GAF_SCAN_CMD_TYPE_START) {
		unicast_env.peripheral_info.addr.addr_type = 0xff; /* Mark to invalid */
		LOG_INF("GAF scanning started");
	} else {
		LOG_ERR("Unexpected GAF scanning command complete event: %u", cmd_type);
	}
}

static void applet_gaf_scanning_cb_stopped(uint8_t const reason)
{
	static const char *const reason_str[] = {
		"Requested by Upper Layer",
		"Internal error",
		"Timeout",
	};

	LOG_DBG("GAF scanning stopped. Reason: %s", reason_str[reason]);

	if (reason == GAF_SCAN_STOP_REASON_TIMEOUT) {
		gaf_scan_start(SCAN_CONFIG_BITS, SCAN_DURATION_SEC, GAP_PHY_1MBPS);
	} else if (reason == GAF_SCAN_STOP_REASON_UL) {
		connect_to_device(&unicast_env.peripheral_info.addr);
	}
}

static void applet_gaf_scanning_cb_report(const gap_bdaddr_t *p_addr, uint8_t const info_bf,
					  const gaf_adv_report_air_info_t *p_air_info,
					  uint8_t const flags, uint16_t const appearance,
					  uint16_t const tmap_roles, const atc_csis_rsi_t *p_rsi,
					  uint16_t const length, const uint8_t *p_data)
{
	const uint8_t *p_reported_name;
	uint8_t name_length = 0;

	p_reported_name =
		gapm_get_ltv_value(GAP_AD_TYPE_SHORTENED_NAME, length, p_data, &name_length);
	if (p_reported_name == NULL) {
		p_reported_name =
			gapm_get_ltv_value(GAP_AD_TYPE_COMPLETE_NAME, length, p_data, &name_length);
	}

	if (!p_reported_name || !name_length) {
		return;
	}

	/* Check that peripheral name matches */
	if (0 != memcmp(p_reported_name, peripheral_name, sizeof(peripheral_name) - 1)) {
		return;
	}
	memcpy(unicast_env.peripheral_info.device_name, p_reported_name, name_length);
	unicast_env.peripheral_info.device_name[name_length] = 0;
	unicast_env.peripheral_info.addr = *p_addr;
	unicast_env.peripheral_info.info_flags = info_bf;

	if (CHECKB(info_bf, GAF_SCAN_REPORT_INFO_RSI)) {
		memcpy(&unicast_env.peripheral_info.rsi, p_rsi,
		       sizeof(unicast_env.peripheral_info.rsi));
	}

	if (CHECKB(info_bf, GAF_SCAN_REPORT_INFO_ANNOUNCEMENT)) {
		LOG_DBG("Announcement received");
	}
}

static void applet_gaf_scanning_cb_announcement(const gap_bdaddr_t *const p_addr,
						uint8_t const type, uint32_t const context_bf,
						const gaf_ltv_t *const p_metadata)
{
	if (unicast_env.peripheral_info.addr.addr_type <= GAP_ADDR_RAND) {
		LOG_INF("Device found, name: %s, addr: %s", unicast_env.peripheral_info.device_name,
			bdaddr_str(&unicast_env.peripheral_info.addr));
		/* ready. end scanning */
		gaf_scan_stop();
	}
}

/* Set of callback function for communication with GAF Scanning module */
static const struct gaf_scan_cb gaf_scan_callbacks = {
	.cb_cmp_evt = applet_gaf_scanning_cb_cmp_evt,
	.cb_stopped = applet_gaf_scanning_cb_stopped,
	.cb_report = applet_gaf_scanning_cb_report,
	.cb_announcement = applet_gaf_scanning_cb_announcement,
};

/* ---------------------------------------------------------------------------------------- */
/** BAP Unicast Client callbacks */

static void on_bap_uc_cli_cmp_evt(uint8_t const cmd_type, uint16_t const status,
				  uint8_t const con_lid, uint8_t const ase_info,
				  uint8_t const char_type)
{
	static const char *const cmd_type_str[] = {"DISCOVER",  "CONFIGURE_CODEC", "CONFIGURE_QOS",
						   "ENABLE",    "UPDATE META",     "DISABLE",
						   "RELEASE",   "GET_QUALITY",     "SET_CFG",
						   "GET_STATE", "GROUP_REMOVE",    "CIS_CONTROL"};

	LOG_DBG("BAP command '%s' completed status:%u, con_lid:%u, ase_info:%u, char_type:%u",
		cmd_type < ARRAY_SIZE(cmd_type_str) ? cmd_type_str[cmd_type] : "??", status,
		con_lid, ase_info, char_type);

	__ASSERT(GAF_ERR_NO_ERROR == status, "BAC Unicast Client comamnd failed!");

	switch (cmd_type) {
	case BAP_UC_CLI_CMD_TYPE_DISCOVER: {
#if DISCOVER_PACS_BEFORE_ASCS
		k_work_submit_to_queue(&worker_queue, &configure_streams_job);
#else
		/* Discover PACS */
		uint16_t const err =
			bap_capa_cli_discover(con_lid, GATT_INVALID_HDL, GATT_INVALID_HDL);

		if (GAF_ERR_NO_ERROR != err) {
			LOG_ERR("PACS discovery start failed. Error:%u", err);
		}
#endif
		break;
	}
	case BAP_UC_CLI_CMD_TYPE_CONFIGURE_CODEC: {
		bap_ready_sem_give();
		break;
	}
	case BAP_UC_CLI_CMD_TYPE_CONFIGURE_QOS: {
		bap_ready_sem_give();
		break;
	}
	case BAP_UC_CLI_CMD_TYPE_ENABLE: {
		break;
	}
	case BAP_UC_CLI_CMD_TYPE_UPDATE_METADATA: {
		break;
	}
	case BAP_UC_CLI_CMD_TYPE_DISABLE: {
		break;
	}
	case BAP_UC_CLI_CMD_TYPE_RELEASE: {
		break;
	}
	case BAP_UC_CLI_CMD_TYPE_GET_QUALITY: {
		break;
	}
	case BAP_UC_CLI_CMD_TYPE_SET_CFG: {
		break;
	}
	case BAP_UC_CLI_CMD_TYPE_GET_STATE: {
		break;
	}
	case BAP_UC_CLI_CMD_TYPE_REMOVE_GROUP: {
		break;
	}
	case BAP_UC_CLI_CMD_TYPE_CIS_CONTROL: {
		break;
	}
	default: {
		break;
	}
	}
}

static void on_bap_uc_cli_quality_cmp_evt(
	uint16_t const status, uint8_t const ase_lid, uint32_t const tx_unacked_packets,
	uint32_t const tx_flushed_packets, uint32_t const tx_last_subevent_packets,
	uint32_t const retransmitted_packets, uint32_t const crc_error_packets,
	uint32_t const rx_unreceived_packets, uint32_t const duplicate_packets)
{
	LOG_DBG("BAP UC CLI quality event: status %u, ase_lid %u", status, ase_lid);
}

static void on_bap_uc_cli_bond_data(uint8_t const con_lid,
				    struct bap_uc_cli_ascs *const p_ascs_info)
{
	LOG_DBG("BAP UC CLI bond data event: con_lid %u", con_lid);

	uint8_t const nb_chars =
		p_ascs_info->nb_ases_sink + p_ascs_info->nb_ases_src + BAP_UC_CHAR_TYPE_ASE;
	uint16_t const size =
		sizeof(bap_uc_cli_ascs_t) + (nb_chars * sizeof(bap_uc_cli_ascs_char_t));

	LOG_DBG("Bond data: ASCS - char cnt:%u, size:%uB, nb of sink:%u/source:%u", nb_chars, size,
		p_ascs_info->nb_ases_sink, p_ascs_info->nb_ases_src);

	for (uint8_t iter = 0; iter < nb_chars; iter++) {
		bap_uc_cli_ascs_char_t const *const p_char = &p_ascs_info->char_info[iter];

		LOG_DBG("ASE %u, val_hdl:%u, desc_hdl:%u", p_char->ase_id, p_char->val_hdl,
			p_char->desc_hdl);
	}

	/* TODO: save bond data */
#if 0
	storage_store(SETTINGS_NAME_BOND_ASCS, p_ascs_info, size);
#endif

	unicast_env.nb_ases_sink = MIN(p_ascs_info->nb_ases_sink, ARRAY_SIZE(unicast_env.ase_sink));
	unicast_env.nb_ases_source =
		MIN(p_ascs_info->nb_ases_src, ARRAY_SIZE(unicast_env.ase_source));
}

static void on_bap_uc_cli_error(uint8_t const ase_lid, uint8_t const opcode, uint8_t const rsp_code,
				uint8_t const reason)
{
	LOG_ERR("ACSC error! ase_lid %u, opcode %u, rsp_code %u, reason %u", ase_lid, opcode,
		rsp_code, reason);
}

static void on_bap_uc_cli_cis_state(uint8_t const stream_lid, uint8_t const event,
				    uint8_t const con_lid, uint8_t const ase_lid_sink,
				    uint8_t const ase_lid_src, uint8_t const grp_lid,
				    uint8_t const cis_id, uint16_t const conhdl,
				    gapi_ug_config_t *const p_cig_cfg,
				    gapi_us_config_t *const p_cis_cfg)
{
	static const char *const cis_state_str[] = {
		[BAP_UC_CLI_CIS_EVENT_ASE_BOUND] = "ASE_BOUND",
		[BAP_UC_CLI_CIS_EVENT_ASE_UNBOUND] = "ASE_UNBOUND",
		[BAP_UC_CLI_CIS_EVENT_ESTABLISHED] = "ESTABLISHED",
		[BAP_UC_CLI_CIS_EVENT_FAILED] = "FAILED",
		[BAP_UC_CLI_CIS_EVENT_DISCONNECTED] = "DISCONNECTED",
	};

	LOG_DBG("CIS %u state %s - stream %u, ASE sink:%u,source:%u", cis_id, cis_state_str[event],
		stream_lid, ase_lid_sink, ase_lid_src);

	if (event == BAP_UC_CLI_CIS_EVENT_FAILED) {
		__ASSERT(false, "CIS failed to be established");
		/* TODO: implement disconnect handling if connection is lost... if the GAF does not
		 * handle it automatically
		 */
#if 0
#if 0
		if (GAF_INVALID_LID != ase_lid_sink) {
			/* bap_uc_cli_disable(ase_lid_sink); */
			bap_uc_cli_release(ase_lid_sink);
		}
		if (GAF_INVALID_LID != ase_lid_src) {
			/* bap_uc_cli_disable(ase_lid_src); */
			bap_uc_cli_release(ase_lid_src);
		}
#else
		k_work_submit_to_queue(&worker_queue, &disable_job);
#endif
#endif
		return;
	}

	if (GAP_INVALID_CONHDL != conhdl) {
		/* state is established */
		LOG_DBG("  GROUP: sync_delay_us:%u, tlatency_m2s_us:%u, tlatency_s2m_us:%u, "
			"iso_intv_frames:%u",
			p_cig_cfg->sync_delay_us, p_cig_cfg->tlatency_m2s_us,
			p_cig_cfg->tlatency_s2m_us, p_cig_cfg->iso_intv_frames);
		LOG_DBG("  STREAM: sync_delay_us:%u, Max PDU m2s:%u/s2m:%u, PHY m2s:%u/s2m:%u, "
			"flush to "
			"m2s:%u/s2m:%u, nse:%u",
			p_cis_cfg->sync_delay_us, p_cis_cfg->max_pdu_m2s, p_cis_cfg->max_pdu_s2m,
			p_cis_cfg->phy_m2s, p_cis_cfg->phy_s2m, p_cis_cfg->ft_m2s,
			p_cis_cfg->ft_s2m, p_cis_cfg->nse);

		unicast_env.datapath_config.pres_delay_us = p_cig_cfg->tlatency_m2s_us;
	}

	struct unicast_client_ase *p_ase = &unicast_env.ase_sink[ase_lid_sink];

	switch (event) {
	case BAP_UC_CLI_CIS_EVENT_ASE_BOUND: {
		/* An ASE has been bound with the Stream */
		p_ase->stream_lid = stream_lid;
		int retval =
			audio_datapath_channel_create_source(p_ase->number_of_octets, stream_lid);
		__ASSERT(retval == 0, "Failed to create datapath channel");
		break;
	}
	case BAP_UC_CLI_CIS_EVENT_ASE_UNBOUND: {
		/* An ASE has been unbound from the Stream */
		break;
	}
	case BAP_UC_CLI_CIS_EVENT_ESTABLISHED: {
		/* CIS has been successfully established */
		p_ase->state_bitmask |= ASE_STATE_STREAMING;
		break;
	}
	case BAP_UC_CLI_CIS_EVENT_FAILED: {
		/* CIS has failed to be established */
		break;
	}
	case BAP_UC_CLI_CIS_EVENT_DISCONNECTED: {
		/* CIS has been disconnected or has been lost */
		break;
	}
	};
}

static void on_bap_uc_cli_state_empty(uint8_t const con_lid, uint8_t const ase_instance_idx,
				      uint8_t const ase_lid, uint8_t const state)
{
	static const char *const states_str[] = {"IDLE",     "CODEC_CONFIGURED", "QOS_CONFIGURED",
						 "ENABLING", "STREAMING",        "DISABLING",
						 "RELEASING"};

	LOG_DBG("ASE state '%s': con_lid:%u, ase_inst_idx:%u, ase_lid:%u", states_str[state],
		con_lid, ase_instance_idx, ase_lid);

	switch (state) {
	case BAP_UC_ASE_STATE_IDLE: {
		/* IDLE is reported after discovery (connected) */
		if (ase_instance_idx < ARRAY_SIZE(unicast_env.ase_sink)) {
			struct unicast_client_ase *p_ase = &unicast_env.ase_sink[ase_instance_idx];

			if (p_ase->state_bitmask & ASE_STATE_INITIALIZED) {
				/* Already initialized */
				break;
			}
			p_ase->conidx = con_lid;
			p_ase->ase_instance_idx = ase_instance_idx;
			p_ase->ase_lid = ase_lid;
			p_ase->cis_id = GAF_INVALID_LID;
			p_ase->stream_lid = GAF_INVALID_LID;
			p_ase->state_bitmask = ASE_STATE_INITIALIZED;
		}
		break;
	}
	case BAP_UC_ASE_STATE_CODEC_CONFIGURED: {
		break;
	}
	case BAP_UC_ASE_STATE_QOS_CONFIGURED: {
		break;
	}
	case BAP_UC_ASE_STATE_ENABLING: {
		break;
	}
	case BAP_UC_ASE_STATE_STREAMING: {
		break;
	}
	case BAP_UC_ASE_STATE_DISABLING: {
		break;
	}
	case BAP_UC_ASE_STATE_RELEASING:
	default: {
		break;
	}
	}
}

static void on_bap_uc_cli_state_codec(uint8_t const con_lid, uint8_t const ase_instance_idx,
				      uint8_t const ase_lid, gaf_codec_id_t *const p_codec_id,
				      bap_qos_req_t *const p_qos_req,
				      const bap_cfg_ptr_t *const p_cfg)
{
	static const char *sampling_freq_name[BAP_SAMPLING_FREQ_MAX + 1] = {
		[BAP_SAMPLING_FREQ_8000HZ] = "8kHz",
		[BAP_SAMPLING_FREQ_11025HZ] = "11.025kHz",
		[BAP_SAMPLING_FREQ_16000HZ] = "16kHz",
		[BAP_SAMPLING_FREQ_22050HZ] = "22.050kHz",
		[BAP_SAMPLING_FREQ_24000HZ] = "24kHz",
		[BAP_SAMPLING_FREQ_32000HZ] = "32kHz",
		[BAP_SAMPLING_FREQ_44100HZ] = "44.1kHz",
		[BAP_SAMPLING_FREQ_48000HZ] = "48kHz",
		[BAP_SAMPLING_FREQ_88200HZ] = "88.2kHz",
		[BAP_SAMPLING_FREQ_96000HZ] = "96kHz",
		[BAP_SAMPLING_FREQ_176400HZ] = "176.4kHz",
		[BAP_SAMPLING_FREQ_192000HZ] = "192kHz",
		[BAP_SAMPLING_FREQ_384000HZ] = "384kHz",
	};
	static const char *frame_dur_name[BAP_FRAME_DUR_MAX + 1] = {
		[BAP_FRAME_DUR_7_5MS] = "7.5ms",
		[BAP_FRAME_DUR_10MS] = "10ms",
	};

	if (p_codec_id->codec_id[GAF_CODEC_ID_FORMAT_POS] != GAPI_CODEC_FORMAT_LC3) {
		LOG_ERR("ASE codec configured: Not LC3 coded! ignored...");
		return;
	}

	LOG_DBG("ASE codec configured: con_lid %u, ase_instance_idx %u, ase_lid %u", con_lid,
		ase_instance_idx, ase_lid);
	LOG_DBG("    Freq: %s, Duration: %s, Length: %dB, Location: %u",
		sampling_freq_name[p_cfg->param.sampling_freq],
		frame_dur_name[p_cfg->param.frame_dur], p_cfg->param.frame_octet,
		p_cfg->param.location_bf);
	LOG_DBG("    QoS: pres_delay_us %u..%u, pref_pres_delay_us %u..%u, trans_latency_max_ms "
		"%u, retx_nb %u, framing %u, phy_bf %u",
		p_qos_req->pres_delay_min_us, p_qos_req->pres_delay_max_us,
		p_qos_req->pref_pres_delay_min_us, p_qos_req->pref_pres_delay_max_us,
		p_qos_req->trans_latency_max_ms, p_qos_req->retx_nb, p_qos_req->framing,
		p_qos_req->phy_bf);

	unicast_env.datapath_config.sampling_rate_hz =
		bap_sampling_freq_index_to_hz(p_cfg->param.sampling_freq);
	unicast_env.datapath_config.frame_duration_is_10ms = p_cfg->param.frame_dur;

	__ASSERT(ase_lid < ARRAY_SIZE(unicast_env.ase_sink), "Invalid ASE ID");

	struct unicast_client_ase *p_ase = &unicast_env.ase_sink[ase_lid];

	__ASSERT(p_ase->ase_lid == ase_lid, "ASE LID mismatch");

	p_ase->state_bitmask |= ASE_STATE_CODEC_CONFIGURED;
	p_ase->retx_number = p_qos_req->retx_nb;

	/* QoS configuration must be done after all codec configurations... for some reason */
	if (ase_lid == (unicast_env.nb_ases_sink - 1)) {
		/* Cannot call QoS configure from here directly thus trigger a worker thread */
		/* TODO: resolve why direct call from here won't work! */
		k_work_submit_to_queue(&worker_queue, &qos_job);
	}
}

static void on_bap_uc_cli_state_qos(uint8_t const ase_lid, const bap_qos_cfg_t *const p_qos_cfg)
{
	LOG_DBG("ASE %u QoS configured.", ase_lid);

	__ASSERT(ase_lid < ARRAY_SIZE(unicast_env.ase_sink), "Invalid ASE ID");

	struct unicast_client_ase *p_ase = &unicast_env.ase_sink[ase_lid];

	__ASSERT(p_ase->ase_lid == ase_lid, "ASE LID mismatch");

	p_ase->state_bitmask |= ASE_STATE_QOS_CONFIGURED;

	if (ase_lid == (unicast_env.nb_ases_sink - 1)) {
		k_work_submit_to_queue(&worker_queue, &enable_job);
	}
}

static void on_bap_uc_cli_state_metadata(uint8_t const ase_lid, uint8_t const state,
					 const bap_cfg_metadata_ptr_t *const p_metadata)
{
	static const char *const states_str[] = {
		"IDLE",      "CODEC_CONFIGURED", "QOS_CONFIGURED", "ENABLING",
		"STREAMING", "DISABLING",        "RELEASING",
	};

	LOG_DBG("ASE %u metadata state %s", ase_lid, states_str[state]);

	switch (state) {
	case BAP_UC_ASE_STATE_IDLE: {
		break;
	}
	case BAP_UC_ASE_STATE_CODEC_CONFIGURED: {
		break;
	}
	case BAP_UC_ASE_STATE_QOS_CONFIGURED: {
		break;
	}
	case BAP_UC_ASE_STATE_ENABLING: {
		break;
	}
	case BAP_UC_ASE_STATE_STREAMING: {
		break;
	}
	case BAP_UC_ASE_STATE_DISABLING: {
		break;
	}
	case BAP_UC_ASE_STATE_RELEASING: {
		break;
	}
	default: {
		__ASSERT(false, "Invalid metadata state");
	}
	}
}

static void on_bap_uc_cli_svc_changed(uint8_t const con_lid)
{
	LOG_DBG("ASE service changed event: con_lid %u", con_lid);
}

static void on_bap_uc_cli_dp_update_req(uint8_t const ase_lid, bool const start)
{
	LOG_DBG("ASE %u data path %s request", ase_lid, start ? "START" : "STOP");

	bap_uc_cli_dp_update_cfm(ase_lid, true);

	if (start) {
		if (ase_lid == (unicast_env.nb_ases_sink - 1)) {
			static K_WORK_DEFINE(dp_update_job, configure_datapath);
			k_work_submit_to_queue(&worker_queue, &dp_update_job);
		}
	} else {
		audio_datapath_cleanup_source();
	}
}

const struct bap_uc_cli_cb bap_cli_cbs = {
	.cb_cmp_evt = on_bap_uc_cli_cmp_evt,
	.cb_quality_cmp_evt = on_bap_uc_cli_quality_cmp_evt,
	.cb_bond_data = on_bap_uc_cli_bond_data,
	.cb_error = on_bap_uc_cli_error,
	.cb_cis_state = on_bap_uc_cli_cis_state,
	.cb_state_empty = on_bap_uc_cli_state_empty,
	.cb_state_codec = on_bap_uc_cli_state_codec,
	.cb_state_qos = on_bap_uc_cli_state_qos,
	.cb_state_metadata = on_bap_uc_cli_state_metadata,
	.cb_svc_changed = on_bap_uc_cli_svc_changed,
	.cb_dp_update_req = on_bap_uc_cli_dp_update_req,
};

/* ---------------------------------------------------------------------------------------- */

static void on_bap_capa_client_cb_cmp_evt(uint8_t const cmd_type, uint16_t status,
					  uint8_t const con_lid, uint8_t const param_1,
					  uint8_t const pac_lid)
{
	/* cmd_type: enum bap_capa_cli_cmd_type */
	static const char *const cmd_str[] = {"DISCOVER", "GET", "SET_CFG", "SET_LOCATION"};

	LOG_DBG("BAP Capabilities completed: cmd:%s, status:%u, conlid:%u, pac_lid:%u",
		cmd_str[cmd_type], status, con_lid, pac_lid);

	__ASSERT(GAF_ERR_NO_ERROR == status, "BAP Capa Client cmd failed!");

	if (BAP_CAPA_CLI_CMD_TYPE_DISCOVER == cmd_type) {
#if DISCOVER_PACS_BEFORE_ASCS
		status = bap_uc_cli_discover(con_lid, GATT_MIN_HDL, GATT_MAX_HDL);
		if (GAF_ERR_NO_ERROR != status) {
			LOG_ERR("ACSC discovery start failed. Error:%u", status);
		}
#else
		k_work_submit_to_queue(&worker_queue, &configure_streams_job);
#endif
	}
}

static void on_bap_capa_client_cb_bond_data(uint8_t const con_lid,
					    bap_capa_cli_pacs_t *const p_pacs_info)
{
	uint8_t const nb_chars =
		p_pacs_info->nb_pacs_sink + p_pacs_info->nb_pacs_src + BAP_UC_CHAR_TYPE_ASE;
	uint16_t const size =
		sizeof(bap_capa_cli_pacs_char_t) + (nb_chars * sizeof(bap_capa_cli_pacs_char_t));

	LOG_DBG("Bond data: PACS - char cnt:%u, size:%uB, nb of sink:%u/source:%u, opt_feat_bf:%u",
		nb_chars, size, p_pacs_info->nb_pacs_sink, p_pacs_info->nb_pacs_src,
		p_pacs_info->opt_feat_bf);

	unicast_env.nb_pacs_sink = p_pacs_info->nb_pacs_sink;
	unicast_env.nb_pacs_source = p_pacs_info->nb_pacs_src;

/* TODO: handle bond data */
#if 0
	for (uint8_t iter = 0; iter < nb_chars; iter++) {
		bap_capa_cli_pacs_char_t const *const p_char = &p_pacs_info->char_info[iter];

		LOG_DBG("  PAC %u, val_hdl:%u, desc_hdl:%u", iter, p_char->val_hdl,
			p_char->desc_hdl);
	}

	storage_store(SETTINGS_NAME_BOND_PACS, p_pacs_info, size);
#endif
}

static void on_bap_capa_client_cb_record(uint8_t const con_lid, uint8_t const pac_lid,
					 uint8_t const record_lid, uint8_t const nb_records,
					 const gaf_codec_id_t *const p_codec_id,
					 const bap_capa_ptr_t *const p_capa,
					 const bap_capa_metadata_ptr_t *const p_metadata)
{
	if (!nb_records) {
		LOG_WRN("BAP Capabilities record: No records! ignored...");
		return;
	}
	if (p_codec_id->codec_id[GAF_CODEC_ID_FORMAT_POS] != GAPI_CODEC_FORMAT_LC3) {
		LOG_ERR("BAP Capabilities record: Not LC3 coded! ignored...");
		return;
	}

	uint32_t const sampling_freq_hz =
		bap_sampling_freq_bitmask_to_hz(p_capa->param.sampling_freq_bf);

	LOG_DBG("BAP Capa: PAC id:%u, record id:%u, nb_records:%u, sampling_freq:%uHz, "
		"frame_dur_bf:%u, chan_cnt_bf:%u, frame_octet_min:%u, "
		"frame_octet_max:%u, max_frames_sdu:%u, meta.context_bf:%u",
		pac_lid, record_lid, nb_records, sampling_freq_hz, p_capa->param.frame_dur_bf,
		p_capa->param.chan_cnt_bf, p_capa->param.frame_octet_min,
		p_capa->param.frame_octet_max, p_capa->param.max_frames_sdu,
		p_metadata->param.context_bf);

	static size_t index;

	if (!pac_lid && !record_lid) {
		/* Reset the index */
		index = 0;
	}

	if ((ARRAY_SIZE(unicast_env.pacs_sink) + ARRAY_SIZE(unicast_env.pacs_source)) <= index) {
		LOG_WRN("No space for a new record! ignored...");
		return;
	}

	struct pac_capa *const p_pac = (pac_lid < unicast_env.nb_pacs_sink)
					       ? &unicast_env.pacs_sink[index]
					       : &unicast_env.pacs_source[index];

	p_pac->sampling_freq_hz = sampling_freq_hz;
	p_pac->frame_duration_bf = p_capa->param.frame_dur_bf;
	p_pac->frame_octet_min = p_capa->param.frame_octet_min;
	p_pac->frame_octet_max = p_capa->param.frame_octet_max;
	p_pac->max_frames_sdu = p_capa->param.max_frames_sdu;

	/* Increase index for next PAC */
	index++;
}

static void on_bap_capa_client_cb_location(uint8_t const con_lid, uint8_t const direction,
					   uint32_t const location_bf)
{
	LOG_DBG("BAP Capabilities location: conidx %u, dir:%s, bf:%u", con_lid,
		(direction == GAF_DIRECTION_SINK) ? "Sink" : "Source", location_bf);
	if (location_bf & GAF_LOC_FRONT_LEFT_BIT) {
		LOG_DBG("  Front left");
	}
	if (location_bf & GAF_LOC_FRONT_CENTER_BIT) {
		LOG_DBG("  Front center");
	}
	if (location_bf & GAF_LOC_FRONT_RIGHT_BIT) {
		LOG_DBG("  Front right");
	}
}

static void on_bap_capa_client_cb_context(uint8_t const con_lid, uint8_t const context_type,
					  uint16_t const context_bf_sink,
					  uint16_t const context_bf_src)
{
	/* context bitfields see \ref enum bap_context_type_bf */
	if (unicast_env.nb_pacs_sink) {
		LOG_DBG("BAP context(SINK): type:%s, bf:%u",
			(context_type == BAP_CAPA_CONTEXT_TYPE_SUPP) ? "SUPP" : "AVAIL",
			context_bf_sink);
	}
	if (unicast_env.nb_pacs_source) {
		LOG_DBG("BAP context(SOURCE): type:%s, bf:%u",
			(context_type == BAP_CAPA_CONTEXT_TYPE_SUPP) ? "SUPP" : "AVAIL",
			context_bf_src);
	}
}

static void on_bap_capa_client_cb_svc_changed(uint8_t const con_lid)
{
	LOG_DBG("BAP Capabilities service changed event: conidx %u", con_lid);
}

static const struct bap_capa_cli_cb bap_capa_cli_callbacks = {
	.cb_cmp_evt = on_bap_capa_client_cb_cmp_evt,
	.cb_bond_data = on_bap_capa_client_cb_bond_data,
	.cb_record = on_bap_capa_client_cb_record,
	.cb_location = on_bap_capa_client_cb_location,
	.cb_context = on_bap_capa_client_cb_context,
	.cb_svc_changed = on_bap_capa_client_cb_svc_changed,
};

/* ---------------------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------------------- */
#if BUTTON_ENABLED

#include <zephyr/drivers/gpio.h>

#define SW0_NODE DT_ALIAS(sw0)
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios, {0});

void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	static uint32_t last_clicked_ms;

	if ((k_uptime_get_32() - last_clicked_ms) < 1000) {
		return;
	}
	last_clicked_ms = k_uptime_get_32();

	k_work_submit_to_queue(&worker_queue, &enable_job);
}

void configure_button(void)
{
	static struct gpio_callback button_cb_data;

	/* Configure button */
	if (!gpio_is_ready_dt(&button)) {
		LOG_ERR("Button is not ready");
		return;
	}

	if (gpio_pin_configure_dt(&button, GPIO_INPUT)) {
		LOG_ERR("Button configure failed");
		return;
	}

	if (gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_RISING) != 0) {
		LOG_ERR("button int conf failed");
		return;
	}

	gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
	if (gpio_add_callback(button.port, &button_cb_data) != 0) {
		LOG_ERR("cb add failed");
	}
}
#else
#define configure_button()
#endif

/* ---------------------------------------------------------------------------------------- */

int unicast_source_configure(void)
{
	for (size_t iter = 0; iter < ARRAY_SIZE(unicast_env.ase_sink); iter++) {
		unicast_env.ase_sink[iter].conidx = GAP_INVALID_CONIDX;
	}
	for (size_t iter = 0; iter < ARRAY_SIZE(unicast_env.group_lid); iter++) {
		unicast_env.group_lid[iter] = GAF_INVALID_LID;
	}

	k_work_queue_start(&worker_queue, worker_task_stack,
			   K_KERNEL_STACK_SIZEOF(worker_task_stack), WORKER_PRIORITY, NULL);
	k_thread_name_set(&worker_queue.thread, "unicast_cli_workq");

	configure_button();

	struct bap_uc_cli_cfg bap_cli_cfg = {
		/* Configuration bit field */
		.cfg_bf = 0,
		/* Number of ASE configurations that can be maintained
		 * Shall be at larger than 0
		 */
		.nb_ases_cfg = ARRAY_SIZE(unicast_env.ase_sink),
		/* Preferred MTU
		 * Values from 0 to 63 are equivalent to 64
		 */
		.pref_mtu = 0,
		/* Timeout duration in seconds for reception of notification for ASE Control Point
		 * characteristic and for
		 * some notifications of ASE characteristic
		 * From 1s to 5s, 0 means 1s
		 */
		.timeout_s = 2,
	};
	uint16_t err;

	err = bap_uc_cli_configure(&bap_cli_cbs, &bap_cli_cfg);
	if (GAF_ERR_NO_ERROR != err) {
		LOG_ERR("Error %u configuring BAP client", err);
		return -1;
	}
	LOG_DBG("BAP client configured");

	struct bap_capa_cli_cfg capa_cli_cfg = {
		.pref_mtu = 0,
	};

	err = bap_capa_cli_configure(&bap_capa_cli_callbacks, &capa_cli_cfg);
	if (GAF_ERR_NO_ERROR != err) {
		LOG_ERR("Error %u configuring BAP capa client", err);
		return -1;
	}
	LOG_DBG("BAP capa client configured");

	err = gaf_scan_configure(&gaf_scan_callbacks);
	if (GAF_ERR_NO_ERROR != err) {
		LOG_ERR("Error %u configuring GAF scanning", err);
		return -1;
	}
	LOG_DBG("GAF scanning configured");

	return 0;
}

int unicast_source_scan_start(void)
{
	uint16_t err;

	err = gaf_scan_start(SCAN_CONFIG_BITS, SCAN_DURATION_SEC, GAP_PHY_1MBPS);
	if (GAF_ERR_NO_ERROR != err) {
		LOG_ERR("Error %u starting scan", err);
		return -1;
	}
	LOG_DBG("Started scanning");
	return 0;
}

int unicast_source_discover(uint8_t const con_lid)
{
	uint16_t err;

	LOG_DBG("Client connected... conid:%u", con_lid);

#if DISCOVER_PACS_BEFORE_ASCS
	err = bap_capa_cli_discover(con_lid, GATT_INVALID_HDL, GATT_INVALID_HDL);
#else
	err = bap_uc_cli_discover(con_lid, GATT_MIN_HDL, GATT_MAX_HDL);
#endif

	if (GAF_ERR_NO_ERROR != err) {
		LOG_ERR("Unicast client discovery start failed. Error:%u", err);
		return -1;
	}
	return 0;
}

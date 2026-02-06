/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/types.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/init.h>
#include <zephyr/sys/__assert.h>
#include <string.h>

#include "bluetooth/le_audio/audio_utils.h"

#include "bap_bc_sink.h"
#include "bap_bc_scan.h"
#include "audio_datapath.h"
#include "auracast_sink.h"
#include "main.h"
#include "power_mgr.h"

LOG_MODULE_REGISTER(auracast_sink, CONFIG_AURACAST_SINK_LOG_LEVEL);

#define SYNCHRONISATION_TIMEOUT_MS 2000
#define SYNCHRONISATION_TIMEOUT    (SYNCHRONISATION_TIMEOUT_MS / 10)
#define SCAN_TIMEOUT_MS            1000
#define SCAN_TIMEOUT               (SCAN_TIMEOUT_MS / 10)
#define SINK_TIMEOUT_MS            1000
#define SINK_TIMEOUT               (SINK_TIMEOUT_MS / 10)
#define INVALID_CHANNEL_INDEX      0xFF
#define SOURCE_SCAN_TIMEOUT_S      10

#define GAF_LOC_LEFT_OR_CENTRE_MASK                                                                \
	(GAF_LOC_FRONT_LEFT_BIT | GAF_LOC_FRONT_LEFT_BIT | GAF_LOC_BACK_LEFT_BIT |                 \
	 GAF_LOC_FRONT_LEFT_CENTER_BIT | GAF_LOC_BACK_CENTER_BIT | GAF_LOC_SIDE_LEFT_BIT |         \
	 GAF_LOC_TOP_FRONT_LEFT_BIT | GAF_LOC_TOP_FRONT_CENTER_BIT | GAF_LOC_TOP_CENTER_BIT |      \
	 GAF_LOC_TOP_BACK_LEFT_BIT | GAF_LOC_TOP_SIDE_LEFT_BIT | GAF_LOC_TOP_BACK_CENTER_BIT |     \
	 GAF_LOC_BOTTOM_FRONT_CENTER_BIT | GAF_LOC_BOTTOM_FRONT_LEFT_BIT |                         \
	 GAF_LOC_FRONT_LEFT_WIDE_BIT | GAF_LOC_LEFT_SURROUND_BIT)
#define GAF_LOC_RIGHT_MASK                                                                         \
	(GAF_LOC_FRONT_RIGHT_BIT | GAF_LOC_BACK_RIGHT_BIT | GAF_LOC_FRONT_RIGHT_CENTER_BIT |       \
	 GAF_LOC_SIDE_RIGHT_BIT | GAF_LOC_TOP_FRONT_RIGHT_BIT | GAF_LOC_TOP_BACK_RIGHT_BIT |       \
	 GAF_LOC_TOP_SIDE_RIGHT_BIT | GAF_LOC_BOTTOM_FRONT_RIGHT_BIT |                             \
	 GAF_LOC_FRONT_RIGHT_WIDE_BIT)

/* Configuration obtained from advertising reports */
struct auracast_sink_env {
	/* Details of PA, group etc. to enable */
	bap_bcast_id_t bcast_id;
	uint32_t chosen_streams_bf;
	uint32_t started_streams_bf;
	uint8_t left_channel_pos;
	uint8_t right_channel_pos;
	uint8_t pa_lid;
	uint8_t grp_lid;

	/* Audio datapath configuration */
	struct audio_datapath_config datapath_cfg;
	size_t octets_per_frame;
	bool datapath_cfg_valid;
};

static uint8_t expected_streams;
static bool public_broadcast_found;
static struct auracast_sink_env sink_env;

struct found_stream {
	uint32_t features_bf;
	bap_adv_id_t adv_id;
	bap_bcast_id_t bcast_id;
	char name[33];
};

static struct found_stream *found_streams[32];
static size_t found_streams_count;

static void reset_sink_config(void)
{
	/* Assume config is OK to start, and set false if anything incompatible is found */
	memset(&sink_env, 0, sizeof(sink_env));
	sink_env.grp_lid = GAF_INVALID_LID;
	sink_env.datapath_cfg_valid = true;
	/* sink_env.datapath_cfg.mclk_dev = mclk_gen_dev; */
	sink_env.right_channel_pos = INVALID_CHANNEL_INDEX;
	sink_env.left_channel_pos = INVALID_CHANNEL_INDEX;

	found_streams_count = 0;
	for (int i = 0; i < ARRAY_SIZE(found_streams); i++) {
		free(found_streams[i]);
		found_streams[i] = NULL;
	}
}

static void print_found_streams(void)
{
	if (!found_streams_count && !found_streams[0]) {
		printk("No streams found... try to restart with different name prefix\r\n");
		return;
	}

	printk("-- available streams ---\r\n");
	for (int i = 0; i < found_streams_count; i++) {
		printk("%4d: %s\r\n", i, found_streams[i]->name);
	}
	printk("\r\n type 'auracast select <stream index> [password]' to select stream\r\n");
}

static bool stream_exists_already(bap_bcast_id_t const *const bcast_id)
{
	for (int i = 0; i < found_streams_count; i++) {
		if (!memcmp(found_streams[i]->bcast_id.id, bcast_id->id,
			    ARRAY_SIZE(bcast_id->id))) {
			return true;
		}
	}
	return false;
}

static void create_datapaths(void)
{
	size_t stream_bf = sink_env.started_streams_bf;

	while (stream_bf) {
		uint8_t const stream_pos = __builtin_ctz(stream_bf);

		audio_datapath_channel_create_sink(sink_env.octets_per_frame, stream_pos);

		stream_bf &= ~(1U << stream_pos);
	}
}

static void start_datapaths(void)
{
	size_t stream_bf = sink_env.started_streams_bf;

	while (stream_bf) {
		uint8_t const stream_pos = __builtin_ctz(stream_bf);

		audio_datapath_channel_start_sink(stream_pos);

		stream_bf &= ~(1U << stream_pos);
	}
}

static int audio_datapath_start(void)
{
	create_datapaths();
	start_datapaths();

	LOG_INF("Audio datapath started");

	return 0;
}

static int start_scanning(void)
{
	LOG_INF("Start scanning for broadcast sources");

	/* Zero timeout value causes it to scan until explicitly stopped */
	const uint16_t err = bap_bc_scan_start(SOURCE_SCAN_TIMEOUT_S);

	if (err != GAP_ERR_NO_ERROR) {
		LOG_ERR("Failed to start bap_bc_scan, err %u", err);
		return -ENODEV;
	} else if (err == GAP_ERR_COMMAND_DISALLOWED) {
		LOG_INF("Scan already started");
		return 0;
	}

	reset_sink_config();
	public_broadcast_found = false;

	return 0;
}

static int stop_scanning(void)
{
	const uint16_t err = bap_bc_scan_stop();

	if (err == GAF_ERR_COMMAND_DISALLOWED) {
		LOG_INF("Scanning already stopped");
	} else if (err) {
		LOG_ERR("Failed to stop scanning, err %u", err);
		return -EFAULT;
	}
	return 0;
}

static int sink_enable(void)
{
	if (!sink_env.datapath_cfg_valid) {
		LOG_ERR("Cannot enable sink for invalid config");
		return -1;
	}

	if (sink_env.left_channel_pos != INVALID_CHANNEL_INDEX) {
		sink_env.chosen_streams_bf |= (1U << (sink_env.left_channel_pos - 1));
	}

	if (sink_env.right_channel_pos != INVALID_CHANNEL_INDEX) {
		sink_env.chosen_streams_bf |= (1U << (sink_env.right_channel_pos - 1));
	}

	LOG_INF("Chosen streams bitfield: %x", sink_env.chosen_streams_bf);

	gaf_bcast_code_t code;
	const gaf_bcast_code_t *p_code_ptr = 0 < fill_auracast_encryption_key(&code) ? &code : NULL;
	uint16_t const err =
		bap_bc_sink_enable(sink_env.pa_lid, &sink_env.bcast_id, sink_env.chosen_streams_bf,
				   p_code_ptr, 0, SINK_TIMEOUT, &sink_env.grp_lid);

	if (err) {
		LOG_ERR("Failed to enable bap_bc_sink, err %u", err);
		return -1;
	}

	return 0;
}

static void terminate_pa_sync(void)
{
	uint16_t err = bap_bc_scan_pa_terminate(sink_env.pa_lid);

	if (err) {
		LOG_ERR("Failed to terminate sync with PA, err %u", err);
	}
}

static int start_streaming(void)
{
	gaf_codec_id_t codec_id = GAF_CODEC_ID_LC3;

	uint16_t err = bap_bc_sink_start_streaming(sink_env.grp_lid, sink_env.left_channel_pos,
						   &codec_id, GAPI_DP_ISOOSHM, 0, NULL);

	if (err) {
		LOG_ERR("Failed to start streaming with err %u", err);
		start_scanning();
		return -1;
	}

	if (sink_env.right_channel_pos != INVALID_CHANNEL_INDEX) {
		uint16_t err =
			bap_bc_sink_start_streaming(sink_env.grp_lid, sink_env.right_channel_pos,
						    &codec_id, GAPI_DP_ISOOSHM, 0, NULL);

		if (err) {
			LOG_ERR("Failed to start streaming with err %u", err);
			start_scanning();
			return -1;
		}
	}

	return 0;
}

static void on_bap_bc_scan_cmp_evt(uint8_t cmd_type, uint16_t status, uint8_t pa_lid)
{
	switch (cmd_type) {
	case BAP_BC_SCAN_CMD_TYPE_START:
		LOG_DBG("Scan start cmd complete, status %u", status);
		break;
	case BAP_BC_SCAN_CMD_TYPE_STOP:
		LOG_DBG("Scan stop cmd complete, status %u", status);
		break;
	case BAP_BC_SCAN_CMD_TYPE_PA_SYNCHRONIZE:
		LOG_INF("PA synchronise cmd complete, status %u", status);
		break;
	case BAP_BC_SCAN_CMD_TYPE_PA_TERMINATE:
		LOG_INF("PA terminate cmd complete, status %u", status);
		break;
	default:
		LOG_WRN("Unexpected cmd_type %u", cmd_type);
		break;
	}
}

static void on_bap_bc_scan_timeout(void)
{
#if !DT_SAME_NODE(DT_NODELABEL(lpuart), DT_CHOSEN(zephyr_console))
	power_mgr_disable_sleep();
#endif
	LOG_INF("scan timeout");
	print_found_streams();
}

static void on_bap_bc_scan_report(const bap_adv_id_t *p_adv_id, const bap_bcast_id_t *p_bcast_id,
				  uint8_t info_bf, const gaf_adv_report_air_info_t *p_air_info,
				  uint16_t length, const uint8_t *p_data)
{
	LOG_DBG("Broadcast found. ID: %02x %02x %02x, tx_pwr %d, rssi %d", p_bcast_id->id[0],
		p_bcast_id->id[1], p_bcast_id->id[2], p_air_info->tx_pwr, p_air_info->rssi);
	LOG_HEXDUMP_DBG(p_data, length, "adv data: ");
}

static void on_bap_bc_scan_public_bcast(const bap_adv_id_t *p_adv_id,
					const bap_bcast_id_t *p_bcast_id, uint8_t pbp_features_bf,
					uint8_t broadcast_name_len, const uint8_t *p_broadcast_name,
					uint8_t metadata_len, const uint8_t *p_metadata)
{
	const char *const expected_stream_name = get_stream_name();
	const bool correct_stream_found =
		!expected_stream_name || !broadcast_name_len ||
		(p_broadcast_name &&
		 !memcmp(expected_stream_name, p_broadcast_name, strlen(expected_stream_name)));
	const bool exact_stream_match =
		expected_stream_name && p_broadcast_name &&
		!memcmp(expected_stream_name, p_broadcast_name, broadcast_name_len) &&
		!public_broadcast_found;

	bool const bc_stream_is_encrypted = (pbp_features_bf & BAP_BC_PBP_FEATURES_ENCRYPTED_BIT);
	char broadcast_name[broadcast_name_len + 1];

	memcpy(broadcast_name, p_broadcast_name, broadcast_name_len);
	broadcast_name[broadcast_name_len] = '\0';

	LOG_INF("Broadcast '%s': encrypted: %s, standard quality: %s, high quality: %s, bcast id "
		"%02x:%02x:%02x, adv addr: %02x:%02x:%02x:%02x:%02x:%02x",
		broadcast_name, bc_stream_is_encrypted ? "yes" : "no",
		(pbp_features_bf & BAP_BC_PBP_FEATURES_STANDARD_QUALITY_PRESENT_BIT) ? "yes" : "no",
		(pbp_features_bf & BAP_BC_PBP_FEATURES_HIGH_QUALITY_PRESENT_BIT) ? "yes" : "no",
		p_bcast_id->id[0], p_bcast_id->id[1], p_bcast_id->id[2], p_adv_id->addr[0],
		p_adv_id->addr[1], p_adv_id->addr[2], p_adv_id->addr[3], p_adv_id->addr[4],
		p_adv_id->addr[5]);
	LOG_HEXDUMP_DBG(p_metadata, metadata_len, "  metadata: ");

	if (!correct_stream_found && !exact_stream_match) {
		LOG_WRN("missed stream... expected_stream_name: '%s'",
			expected_stream_name ? expected_stream_name : "null");
		return;
	}

	/* If we found a non-encrypted public broadcast, synchronise to this */
	if (exact_stream_match) {
		LOG_INF("Stream found! Synchronising to broadcast");

		/* check that encrypted stream could be connected... */
		if (bc_stream_is_encrypted && !get_auracast_encryption_passwd()) {
			LOG_WRN("Cannot connect to encrypted broadcast without password");
			return;
		}

		stop_scanning();

		public_broadcast_found = true;
		/* Store broadcast ID for later */
		memcpy(&sink_env.bcast_id, p_bcast_id, sizeof(bap_bcast_id_t));

		uint16_t err = bap_bc_scan_pa_synchronize(p_adv_id, 0, BAP_BC_SCAN_REPORT_MASK,
							  SYNCHRONISATION_TIMEOUT, SCAN_TIMEOUT,
							  &sink_env.pa_lid);

		if (err != GAP_ERR_NO_ERROR) {
			LOG_ERR("Failed to start PA synchronise procedure, err %u", err);
		}

		return;
	}

	struct found_stream *p_stream;

	if (broadcast_name_len >= (sizeof(p_stream->name) - 1)) {
		LOG_ERR("Broadcast name too long");
		return;
	}

	if (stream_exists_already(p_bcast_id)) {
		return;
	}

	p_stream = malloc(sizeof(struct found_stream));
	if (!p_stream) {
		LOG_ERR("Failed to allocate memory for found stream");
		return;
	}

	p_stream->features_bf = pbp_features_bf;
	p_stream->adv_id = *p_adv_id;
	p_stream->bcast_id = *p_bcast_id;
	memcpy(p_stream->name, p_broadcast_name, broadcast_name_len);
	p_stream->name[broadcast_name_len] = '\0';

	found_streams[found_streams_count++] = p_stream;
}

static void on_bap_bc_scan_pa_established(uint8_t pa_lid, const bap_adv_id_t *p_adv_id, uint8_t phy,
					  uint16_t interval_frames)
{
	LOG_INF("PA synchronised, pa_lid %u interval %u ms", pa_lid, (interval_frames * 5) / 4);
}

static void on_bap_bc_scan_pa_terminated(uint8_t pa_lid, uint8_t reason)
{
	LOG_INF("PA desynchronised, reason %u", reason);
}

static void on_bap_bc_scan_pa_report(uint8_t pa_lid, const gaf_adv_report_air_info_t *p_air_info,
				     uint16_t length, const uint8_t *p_data)
{
	LOG_INF("PA report");
	LOG_INF("Air info: tx_pwr %u rssi %u", p_air_info->tx_pwr, p_air_info->rssi);
	LOG_HEXDUMP_DBG(p_data, length, "periodic adv data: ");
}

static void on_bap_bc_scan_big_info_report(uint8_t pa_lid, const gapm_le_big_info_t *p_report)
{
	LOG_INF("BIGinfo report");
	LOG_INF("SDU interval %u us, ISO interval %u ms, max_pdu %u max_sdu %u",
		p_report->sdu_interval, p_report->iso_interval, p_report->max_pdu,
		p_report->max_sdu);
	LOG_INF("num_bis %u, NSE %u, BN %u, PTO %u, IRC %u, PHY %u, framing %u, encrypted %u",
		p_report->num_bis, p_report->nse, p_report->bn, p_report->pto, p_report->irc,
		p_report->phy, p_report->framing, p_report->encrypted);
}

static void on_bap_bc_scan_group_report(uint8_t pa_lid, uint8_t nb_subgroups, uint8_t nb_streams,
					uint32_t pres_delay_us)
{
	LOG_INF("Group report: %u subgroups, %u streams, presentation delay %u us", nb_subgroups,
		nb_streams, pres_delay_us);
	expected_streams = nb_streams;

	/* Store presentation delay for later use */
	sink_env.datapath_cfg.pres_delay_us = pres_delay_us;
}

static void on_bap_bc_scan_subgroup_report(uint8_t pa_lid, uint8_t sgrp_id, uint32_t stream_pos_bf,
					   const gaf_codec_id_t *p_codec_id,
					   const bap_cfg_ptr_t *p_cfg,
					   const bap_cfg_metadata_ptr_t *p_metadata)
{
	LOG_INF("Subgroup report");
	LOG_INF("sgrp_id %u, stream_bf %x, codec_id %02x %02x %02x %02x %02x", sgrp_id,
		stream_pos_bf, p_codec_id->codec_id[0], p_codec_id->codec_id[1],
		p_codec_id->codec_id[2], p_codec_id->codec_id[3], p_codec_id->codec_id[4]);
	LOG_INF("BAP cfg: loc_bf %x frame_octet %u sampling_freq %u frame_dur %u "
		"frames_sdu %u",
		p_cfg->param.location_bf, p_cfg->param.frame_octet, p_cfg->param.sampling_freq,
		p_cfg->param.frame_dur, p_cfg->param.frames_sdu);

	/* Validate config is OK and store relevant info for later use */
	if (p_cfg->param.sampling_freq < BAP_SAMPLING_FREQ_MIN ||
	    p_cfg->param.sampling_freq > BAP_SAMPLING_FREQ_MAX) {
		LOG_WRN("Invalid sampling frequency %d(bap_sampling_freq)",
			p_cfg->param.sampling_freq);

		sink_env.datapath_cfg_valid = false;
	}

	if (p_cfg->param.frame_dur != BAP_FRAME_DUR_10MS) {
		LOG_WRN("Frame duration is not compatible, need 10 ms");
		sink_env.datapath_cfg_valid = false;
	}

	sink_env.octets_per_frame = p_cfg->param.frame_octet;
	sink_env.datapath_cfg.frame_duration_is_10ms = p_cfg->param.frame_dur == BAP_FRAME_DUR_10MS;
	sink_env.datapath_cfg.sampling_rate_hz =
		audio_bap_sampling_freq_to_hz(p_cfg->param.sampling_freq);
}

static void assign_audio_channel(uint8_t stream_count, uint8_t stream_pos, uint16_t loc_bf)
{
#ifdef CONFIG_AUDIO_LOCATION_USE_GAF
	if ((loc_bf & GAF_LOC_LEFT_OR_CENTRE_MASK) &&
	    (sink_env.left_channel_pos == INVALID_CHANNEL_INDEX)) {
#else /* CONFIG_AUDIO_LOCATION_IMPLICIT */
	if (stream_count == 0) {
#endif
		LOG_INF("Stream index %u is left or centre channel", stream_pos);
		sink_env.left_channel_pos = stream_pos;
	}

#ifdef CONFIG_AUDIO_LOCATION_USE_GAF
	if ((loc_bf & GAF_LOC_RIGHT_MASK) &&
	    (sink_env.right_channel_pos == INVALID_CHANNEL_INDEX)) {
#else /* CONFIG_AUDIO_LOCATION_IMPLICIT */
	if (stream_count == 1) {
#endif
		LOG_INF("Stream index %u is right channel", stream_pos);
		sink_env.right_channel_pos = stream_pos;
	}
}

static void on_bap_bc_scan_stream_report(uint8_t pa_lid, uint8_t sgrp_id, uint8_t stream_pos,
					 const gaf_codec_id_t *p_codec_id,
					 const bap_cfg_ptr_t *p_cfg)
{
	static uint8_t stream_report_count;

	LOG_INF("Stream report %u", stream_pos);
	LOG_INF("BAP cfg: loc_bf %x frame_octet %u sampling_freq %u frame_dur %u frames_sdu %u",
		p_cfg->param.location_bf, p_cfg->param.frame_octet, p_cfg->param.sampling_freq,
		p_cfg->param.frame_dur, p_cfg->param.frames_sdu);

	assign_audio_channel(stream_report_count, stream_pos, p_cfg->param.location_bf);

	if (++stream_report_count >= expected_streams) {
		expected_streams = 0;
		stream_report_count = 0;

		LOG_INF("Disabling PA reports");
		uint16_t err = bap_bc_scan_pa_report_ctrl(sink_env.pa_lid, 0);

		if (err) {
			LOG_ERR("Failed to disable PA reports");
		}

		if (sink_env.left_channel_pos == INVALID_CHANNEL_INDEX) {
			LOG_INF("A left or centre channel must be present");
			sink_env.datapath_cfg_valid = false;
		}

		if (sink_env.datapath_cfg_valid) {
			LOG_INF("Compatible audio source found");

			/* Enable BC sink for the compatible source */
			sink_enable();
		} else {
			LOG_INF("Audio source is not compatible");

			/* Restart scanning for another source */
			start_scanning();
		}
	}
}

static void on_bap_bc_sink_cmp_evt(uint8_t cmd_type, uint16_t status, uint8_t grp_lid,
				   uint8_t stream_pos)
{
	switch (cmd_type) {
	case BAP_BC_SINK_CMD_TYPE_ENABLE:
		LOG_INF("enable cmd complete, status %u, grp %u, stream %u", status, grp_lid,
			stream_pos);
		break;
	case BAP_BC_SINK_CMD_TYPE_START_STREAMING:
		LOG_INF("start streaming cmd complete, status %u, grp %u, stream %u", status,
			grp_lid, stream_pos);
		sink_env.started_streams_bf |= (1U << (stream_pos - 1));

		/* Start audio datapath when all chosen streams are started */
		if (sink_env.started_streams_bf == sink_env.chosen_streams_bf) {
			int ret = audio_datapath_create_sink(&sink_env.datapath_cfg);

			if (ret) {
				audio_datapath_cleanup_sink();
				LOG_ERR("Failed to create audio datapath");
				start_scanning();
			}

			audio_datapath_start();
		}
		break;
	default:
		LOG_ERR("Unexpected cmd type %u", cmd_type);
		break;
	}
}

static void on_bap_bc_sink_quality_cmp_evt(uint16_t status, uint8_t grp_lid, uint8_t stream_pos,
					   uint32_t crc_error_packets, uint32_t rx_unrx_packets,
					   uint32_t duplicate_packets)
{
	LOG_INF("cb_sink_quality, status %u group %u stream %u crc_err %u missing %u duplicate %u",
		status, grp_lid, stream_pos, crc_error_packets, rx_unrx_packets, duplicate_packets);
}

static void on_bap_bc_sink_status(uint8_t grp_lid, uint8_t state, uint32_t stream_pos_bf,
				  const gapi_bg_sync_config_t *p_bg_cfg, uint8_t nb_bis,
				  const uint16_t *p_conhdl)
{
	switch (state) {
	case BAP_BC_SINK_ESTABLISHED:
		LOG_INF("sync established with group %u", grp_lid);
		terminate_pa_sync();
		start_streaming();
		break;
	case BAP_BC_SINK_FAILED:
	case BAP_BC_SINK_CANCELLED:
	case BAP_BC_SINK_LOST:
	case BAP_BC_SINK_PEER_TERMINATE:
	case BAP_BC_SINK_UPPER_TERMINATE:
	case BAP_BC_SINK_MIC_FAILURE:
		LOG_INF("no sync with group %u, state %u", grp_lid, state);
		audio_datapath_cleanup_sink();
		start_scanning();
		break;
	default:
		LOG_ERR("Unexpected bc_sink state %u", state);
		break;
	}
}

static const bap_bc_scan_cb_t scan_cbs = {
	.cb_cmp_evt = on_bap_bc_scan_cmp_evt,
	.cb_timeout = on_bap_bc_scan_timeout,
	.cb_report = on_bap_bc_scan_report,
	.cb_public_bcast_source = on_bap_bc_scan_public_bcast,
	.cb_pa_established = on_bap_bc_scan_pa_established,
	.cb_pa_terminated = on_bap_bc_scan_pa_terminated,
	.cb_pa_report = on_bap_bc_scan_pa_report,
	.cb_big_info_report = on_bap_bc_scan_big_info_report,
	.cb_group_report = on_bap_bc_scan_group_report,
	.cb_subgroup_report = on_bap_bc_scan_subgroup_report,
	.cb_stream_report = on_bap_bc_scan_stream_report,
};

static const bap_bc_sink_cb_t sink_cbs = {
	.cb_cmp_evt = on_bap_bc_sink_cmp_evt,
	.cb_quality_cmp_evt = on_bap_bc_sink_quality_cmp_evt,
	.cb_status = on_bap_bc_sink_status,
};

int auracast_sink_start(void)
{
	int ret = configure_role(ROLE_AURACAST_SINK);
	uint16_t err;

	if (ret == -EALREADY) {
		LOG_DBG("Auracast sink already configured");

#if !DT_SAME_NODE(DT_NODELABEL(lpuart), DT_CHOSEN(zephyr_console))
		power_mgr_log_flush();
		power_mgr_allow_sleep();
#endif

		start_scanning();
		return 0;
	} else if (ret) {
		return ret;
	}

	err = bap_bc_scan_configure(BAP_ROLE_SUPP_BC_SINK_BIT | BAP_ROLE_SUPP_BC_SCAN_BIT,
				    &scan_cbs);
	if (err != GAP_ERR_NO_ERROR) {
		LOG_ERR("Failed to configure bap_bc_scan, err %u", err);
		return -ENODEV;
	}

	err = bap_bc_sink_configure(BAP_ROLE_SUPP_BC_SINK_BIT | BAP_ROLE_SUPP_BC_SCAN_BIT,
				    &sink_cbs);
	if (err != GAP_ERR_NO_ERROR) {
		LOG_ERR("Failed to configure bap_bc_sink, err %u", err);
		return -ENODEV;
	}

	err = start_scanning();
	if (err) {
		return err;
	}

#if !DT_SAME_NODE(DT_NODELABEL(lpuart), DT_CHOSEN(zephyr_console))
	power_mgr_log_flush();
	power_mgr_allow_sleep();
#endif

	return 0;
}

void auracast_sink_stop(void)
{
	uint16_t err;

	audio_datapath_cleanup_sink();

	if (sink_env.grp_lid != GAF_INVALID_LID) {
		err = bap_bc_sink_disable(sink_env.grp_lid);
		if (err && err != GAF_ERR_INVALID_PARAM) {
			LOG_ERR("Failed to disable bap_bc_sink, err %u", err);
		}
	}

	stop_scanning();

#if !DT_SAME_NODE(DT_NODELABEL(lpuart), DT_CHOSEN(zephyr_console))
	power_mgr_disable_sleep();
#endif
}

int auracast_sink_select_stream(int const stream_index)
{

	if (stream_index < 0 || stream_index >= ARRAY_SIZE(found_streams)) {
		LOG_ERR("Invalid stream index %d", stream_index);
		return -EINVAL;
	}

	struct found_stream *p_stream = found_streams[stream_index];

	if (!p_stream) {
		LOG_ERR("Stream %d not found", stream_index);
		return -EINVAL;
	}

	if (p_stream->features_bf & BAP_BC_PBP_FEATURES_ENCRYPTED_BIT &&
	    !get_auracast_encryption_passwd()) {
		LOG_ERR("Stream %d is encrypted and no password set", stream_index);
		return -EINVAL;
	}

	stop_scanning();

	memcpy(&sink_env.bcast_id, &p_stream->bcast_id, sizeof(sink_env.bcast_id));

	uint16_t err =
		bap_bc_scan_pa_synchronize(&p_stream->adv_id, 0, BAP_BC_SCAN_REPORT_MASK,
					   SYNCHRONISATION_TIMEOUT, SCAN_TIMEOUT, &sink_env.pa_lid);

	if (err != GAP_ERR_NO_ERROR) {
		LOG_ERR("Failed to start stream synchronise procedure, err %u", err);
		return -EIO;
	}

#if !DT_SAME_NODE(DT_NODELABEL(lpuart), DT_CHOSEN(zephyr_console))
	power_mgr_log_flush();
	power_mgr_allow_sleep();
#endif

	return 0;
}

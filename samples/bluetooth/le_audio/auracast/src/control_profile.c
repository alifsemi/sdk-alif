/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/util.h>
#include <stdio.h>
#include <string.h>

#include "gatt_srv.h"
#include "gatt.h"
#include "gatt_msg.h"
#include "gatt_db.h"
#include "prf.h"
#include "gapm.h"
#include "gapm_le.h"
#include "gapm_le_adv.h"
#include "co_buf.h"

#include "control_profile.h"
#include "main.h"

LOG_MODULE_REGISTER(control_profile, CONFIG_CONTROL_PROFILE_LOG_LEVEL);

/*
 * Custom 128-bit UUIDs (Bluetooth Base UUID with 16-bit identifiers):
 *   Service:            0000AC00-0000-1000-8000-00805F9B34FB
 *   Mode (R/W):         0000AC01-...
 *   Mode status (N):    0000AC02-...
 *   Stream name (R/W):  0000AC03-...
 *   Encryption (R/W):   0000AC04-...
 *   Codec (R/W):        0000AC05-...
 *   SDU octets (R/W):   0000AC06-...
 *   Frame duration(R/W):0000AC07-...
 *   Frame rate (R/W):   0000AC08-...
 */
#define ACS_UUID_128(uuid16)                                                                       \
	{                                                                                          \
		0x00, 0x00, ((uuid16) >> 8) & 0xFF, (uuid16) & 0xFF, 0x00, 0x00, 0x10, 0x00, 0x80, \
			0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB                                   \
	}

#define ACS_SERVICE_UUID       0xAC00
#define ACS_MODE_UUID          0xAC01
#define ACS_MODE_STATUS_UUID   0xAC02
#define ACS_STREAM_NAME_UUID   0xAC03
#define ACS_ENCRYPTION_UUID    0xAC04
#define ACS_CODEC_UUID         0xAC05
#define ACS_SDU_UUID           0xAC06
#define ACS_FRAME_DUR_UUID     0xAC07
#define ACS_FRAME_RATE_UUID    0xAC08

#define ACS_METAINFO_MODE_STATUS_NTF_SEND 0xAC02
#define ACS_PREF_MTU                      64

#define ATT_16_TO_128_ARRAY(uuid)                                                                  \
	{                                                                                          \
		(uuid) & 0xFF, ((uuid) >> 8) & 0xFF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0     \
	}

#define ATT_128_PRIMARY_SERVICE ATT_16_TO_128_ARRAY(GATT_DECL_PRIMARY_SERVICE)
#define ATT_128_CHARACTERISTIC  ATT_16_TO_128_ARRAY(GATT_DECL_CHARACTERISTIC)
#define ATT_128_CLIENT_CHAR_CFG ATT_16_TO_128_ARRAY(GATT_DESC_CLIENT_CHAR_CFG)

enum acs_att_list {
	ACS_IDX_SERVICE = 0,
	ACS_IDX_MODE_CHAR,
	ACS_IDX_MODE_VAL,
	ACS_IDX_MODE_STATUS_CHAR,
	ACS_IDX_MODE_STATUS_VAL,
	ACS_IDX_MODE_STATUS_NTF_CFG,
	ACS_IDX_STREAM_NAME_CHAR,
	ACS_IDX_STREAM_NAME_VAL,
	ACS_IDX_ENCRYPTION_CHAR,
	ACS_IDX_ENCRYPTION_VAL,
	ACS_IDX_CODEC_CHAR,
	ACS_IDX_CODEC_VAL,
	ACS_IDX_SDU_CHAR,
	ACS_IDX_SDU_VAL,
	ACS_IDX_FRAME_DUR_CHAR,
	ACS_IDX_FRAME_DUR_VAL,
	ACS_IDX_FRAME_RATE_CHAR,
	ACS_IDX_FRAME_RATE_VAL,
	ACS_IDX_NB,
};

/* GATT value size limits (also used as characteristic max length) */
#define CONTROL_PROFILE_STREAM_NAME_MAX 31
#define CONTROL_PROFILE_ENCRYPTION_MAX  16
#define CONTROL_PROFILE_CODEC_MAX       8
#define CONTROL_PROFILE_FRAME_DUR_MAX   8  /* e.g. "7.5ms" */
#define CONTROL_PROFILE_FRAME_RATE_MAX  8  /* e.g. "48" (kHz) */

/**
 * @brief Auracast control profile mode values (GATT Mode characteristic)
 *
 * Wire values are independent of @ref enum role. Mode 0 selects
 * ROLE_BLE_CONFIG (connectable control idle), not ROLE_NONE.
 *
 * 0 - BLE config (idle / mode selection)
 * 1 - Auracast source
 * 2 - Auracast sink
 * 3 - Auracast scan delegator
 */
enum control_profile_mode {
	CONTROL_PROFILE_MODE_IDLE = 0,
	CONTROL_PROFILE_MODE_SOURCE = 1,
	CONTROL_PROFILE_MODE_SINK = 2,
	CONTROL_PROFILE_MODE_DELEGATOR = 3,
	CONTROL_PROFILE_MODE_MAX,
};

struct control_profile_env {
	uint16_t start_hdl;
	uint8_t user_lid;
	uint16_t ntf_cfg;
	bool ntf_ongoing;
	bool service_ready;
	uint8_t adv_actv_idx;
	bool adv_created;
	bool adv_started;

	/* Source / shared configuration written via GATT */
	char codec[CONTROL_PROFILE_CODEC_MAX + 1];
	uint16_t octets_per_frame;
	uint16_t frame_duration_us;
	uint32_t frame_rate_hz;
};

static struct control_profile_env env = {
	.user_lid = 0,
	.adv_actv_idx = GAP_INVALID_ACTV_IDX,
};

K_SEM_DEFINE(acs_adv_sem, 0, 1);

static const uint8_t acs_service_uuid[] = ACS_UUID_128(ACS_SERVICE_UUID);

static const gatt_att_desc_t acs_att_db[ACS_IDX_NB] = {
	[ACS_IDX_SERVICE] = {ATT_128_PRIMARY_SERVICE, ATT_UUID(16) | PROP(RD), 0},

	[ACS_IDX_MODE_CHAR] = {ATT_128_CHARACTERISTIC, ATT_UUID(16) | PROP(RD), 0},
	[ACS_IDX_MODE_VAL] = {ACS_UUID_128(ACS_MODE_UUID), ATT_UUID(128) | PROP(RD) | PROP(WR),
			      OPT(NO_OFFSET) | sizeof(uint8_t)},

	[ACS_IDX_MODE_STATUS_CHAR] = {ATT_128_CHARACTERISTIC, ATT_UUID(16) | PROP(RD), 0},
	[ACS_IDX_MODE_STATUS_VAL] = {ACS_UUID_128(ACS_MODE_STATUS_UUID),
				     ATT_UUID(128) | PROP(RD) | PROP(N), OPT(NO_OFFSET)},
	[ACS_IDX_MODE_STATUS_NTF_CFG] = {ATT_128_CLIENT_CHAR_CFG,
					 ATT_UUID(16) | PROP(RD) | PROP(WR), 0},

	[ACS_IDX_STREAM_NAME_CHAR] = {ATT_128_CHARACTERISTIC, ATT_UUID(16) | PROP(RD), 0},
	[ACS_IDX_STREAM_NAME_VAL] = {ACS_UUID_128(ACS_STREAM_NAME_UUID),
				     ATT_UUID(128) | PROP(RD) | PROP(WR),
				     OPT(NO_OFFSET) | CONTROL_PROFILE_STREAM_NAME_MAX},

	[ACS_IDX_ENCRYPTION_CHAR] = {ATT_128_CHARACTERISTIC, ATT_UUID(16) | PROP(RD), 0},
	[ACS_IDX_ENCRYPTION_VAL] = {ACS_UUID_128(ACS_ENCRYPTION_UUID),
				    ATT_UUID(128) | PROP(RD) | PROP(WR),
				    OPT(NO_OFFSET) | CONTROL_PROFILE_ENCRYPTION_MAX},

	[ACS_IDX_CODEC_CHAR] = {ATT_128_CHARACTERISTIC, ATT_UUID(16) | PROP(RD), 0},
	[ACS_IDX_CODEC_VAL] = {ACS_UUID_128(ACS_CODEC_UUID), ATT_UUID(128) | PROP(RD) | PROP(WR),
			       OPT(NO_OFFSET) | CONTROL_PROFILE_CODEC_MAX},

	[ACS_IDX_SDU_CHAR] = {ATT_128_CHARACTERISTIC, ATT_UUID(16) | PROP(RD), 0},
	[ACS_IDX_SDU_VAL] = {ACS_UUID_128(ACS_SDU_UUID), ATT_UUID(128) | PROP(RD) | PROP(WR),
			     OPT(NO_OFFSET) | sizeof(uint16_t)},

	[ACS_IDX_FRAME_DUR_CHAR] = {ATT_128_CHARACTERISTIC, ATT_UUID(16) | PROP(RD), 0},
	[ACS_IDX_FRAME_DUR_VAL] = {ACS_UUID_128(ACS_FRAME_DUR_UUID),
				   ATT_UUID(128) | PROP(RD) | PROP(WR),
				   OPT(NO_OFFSET) | CONTROL_PROFILE_FRAME_DUR_MAX},

	[ACS_IDX_FRAME_RATE_CHAR] = {ATT_128_CHARACTERISTIC, ATT_UUID(16) | PROP(RD), 0},
	[ACS_IDX_FRAME_RATE_VAL] = {ACS_UUID_128(ACS_FRAME_RATE_UUID),
				    ATT_UUID(128) | PROP(RD) | PROP(WR),
				    OPT(NO_OFFSET) | CONTROL_PROFILE_FRAME_RATE_MAX},
};

static int parse_codec_config(const char *codec, uint16_t *octets, uint32_t *rate_hz,
			      uint16_t *duration_us)
{
	uint32_t octets32;
	uint32_t rate32;
	uint32_t duration32;

	if (auracast_codec_config_from_name(codec, &octets32, &rate32, &duration32) != 0) {
		return -EINVAL;
	}

	*octets = (uint16_t)octets32;
	*rate_hz = rate32;
	*duration_us = (uint16_t)duration32;
	return 0;
}

/* Known LC3 codec preset names, checked against the configured audio parameters
 * to derive a default codec name when the sample runs without a GATT write.
 */
static const char *const codec_preset_names[] = {
	"8_1",	"8_2",	"16_1", "16_2", "24_1", "24_2", "32_1",
	"32_2", "48_1", "48_2", "48_3", "48_4", "48_5", "48_6",
};

/* Reverse-map configured rate/duration/octets to a preset name, or NULL. */
static const char *codec_name_from_params(uint32_t const rate_hz, uint16_t const duration_us,
					  uint16_t const octets)
{
	for (size_t i = 0; i < ARRAY_SIZE(codec_preset_names); i++) {
		uint16_t p_octets;
		uint32_t p_rate;
		uint16_t p_duration;

		if (parse_codec_config(codec_preset_names[i], &p_octets, &p_rate,
				       &p_duration) != 0) {
			continue;
		}

		if (p_rate == rate_hz && p_duration == duration_us && p_octets == octets) {
			return codec_preset_names[i];
		}
	}

	return NULL;
}

static void source_params_init_defaults(void)
{
	const char *codec;

	env.octets_per_frame = CONFIG_ALIF_BLE_AUDIO_OCTETS_PER_CODEC_FRAME;
	env.frame_rate_hz = CONFIG_ALIF_BLE_AUDIO_FS_HZ;
	env.frame_duration_us =
		IS_ENABLED(CONFIG_ALIF_BLE_AUDIO_FRAME_DURATION_10MS) ? 10000 : 7500;

	codec = codec_name_from_params(env.frame_rate_hz, env.frame_duration_us,
				       env.octets_per_frame);
	if (!codec) {
		codec = "Custom";
	}

	strncpy(env.codec, codec, sizeof(env.codec) - 1);
	env.codec[sizeof(env.codec) - 1] = '\0';
}

static int copy_write_string(char *dst, size_t dst_size, co_buf_t *p_data)
{
	size_t len = co_buf_data_len(p_data);

	if (len >= dst_size) {
		return -EINVAL;
	}

	memcpy(dst, co_buf_data(p_data), len);
	dst[len] = '\0';
	return (int)len;
}

static int frame_duration_to_string(uint16_t duration_us, char *buf, size_t buf_size)
{
	if (duration_us == 7500) {
		return snprintf(buf, buf_size, "7.5ms");
	}
	if (duration_us == 10000) {
		return snprintf(buf, buf_size, "10ms");
	}

	return -EINVAL;
}

static int parse_frame_duration_string(const char *str, uint16_t *duration_us)
{
	if (strcmp(str, "7.5ms") == 0 || strcmp(str, "7.5") == 0) {
		*duration_us = 7500;
		return 0;
	}

	if (strcmp(str, "10ms") == 0 || strcmp(str, "10") == 0) {
		*duration_us = 10000;
		return 0;
	}

	return -EINVAL;
}

static int frame_rate_to_string(uint32_t rate_hz, char *buf, size_t buf_size)
{
	if (rate_hz == 0 || (rate_hz % 1000U) != 0U) {
		return -EINVAL;
	}

	return snprintf(buf, buf_size, "%u", rate_hz / 1000U);
}

static int parse_frame_rate_string(const char *str, uint32_t *rate_hz)
{
	char tmp[CONTROL_PROFILE_FRAME_RATE_MAX + 1];
	size_t len = strlen(str);
	unsigned long khz;
	char *end;

	if (len == 0 || len >= sizeof(tmp)) {
		return -EINVAL;
	}

	strncpy(tmp, str, sizeof(tmp) - 1);
	tmp[sizeof(tmp) - 1] = '\0';

	if (len >= 3 && strncmp(&tmp[len - 3], "kHz", 3) == 0) {
		tmp[len - 3] = '\0';
	} else if (len >= 1 && (tmp[len - 1] == 'k' || tmp[len - 1] == 'K')) {
		tmp[len - 1] = '\0';
	}

	khz = strtoul(tmp, &end, 10);
	if ((end == tmp) || (*end != '\0')) {
		return -EINVAL;
	}

	switch (khz) {
	case 8:
	case 16:
	case 24:
	case 32:
	case 48:
		*rate_hz = (uint32_t)khz * 1000U;
		return 0;
	default:
		return -EINVAL;
	}
}

static uint8_t current_mode_value(void)
{
	switch (get_current_role()) {
	case ROLE_AURACAST_SOURCE:
		return CONTROL_PROFILE_MODE_SOURCE;
	case ROLE_AURACAST_SINK:
		return CONTROL_PROFILE_MODE_SINK;
	case ROLE_AURACAST_SCAN_DELEGATOR:
		return CONTROL_PROFILE_MODE_DELEGATOR;
	case ROLE_BLE_CONFIG:
	case ROLE_NONE:
	default:
		return CONTROL_PROFILE_MODE_IDLE;
	}
}

static int queue_mode_command(enum control_profile_mode mode)
{
	struct startup_params cmd = {0};
	const char *stream_name;

	switch (mode) {
	case CONTROL_PROFILE_MODE_IDLE:
		cmd.cmd = COMMAND_STOP;
		break;
	case CONTROL_PROFILE_MODE_SOURCE:
		cmd.cmd = COMMAND_SOURCE;
		cmd.source.octets_per_frame = env.octets_per_frame;
		cmd.source.frame_rate_hz = env.frame_rate_hz;
		cmd.source.frame_duration_us = env.frame_duration_us;

		stream_name = get_stream_name();
		if (stream_name == NULL) {
			set_stream_name(CONFIG_AURACAST_STREAM_NAME);
			stream_name = get_stream_name();
		}
		if (stream_name != NULL) {
			set_device_name(stream_name);
		}
		break;
	case CONTROL_PROFILE_MODE_SINK:
		cmd.cmd = COMMAND_SINK;
		break;
	case CONTROL_PROFILE_MODE_DELEGATOR:
		cmd.cmd = COMMAND_SCAN_DELEGATOR;
		if (get_device_name() == NULL) {
			set_device_name(DEVICE_NAME_PREFIX_DEFAULT " SD");
		}
		break;
	default:
		return -EINVAL;
	}

	return execute_shell_command(cmd);
}

static uint16_t send_mode_notification(void)
{
	co_buf_t *p_buf;
	uint16_t status;
	uint8_t mode_val;
	uint8_t conidx = 0;

	if (!env.service_ready) {
		return PRF_ERR_REQ_DISALLOWED;
	}

	if (env.ntf_ongoing) {
		return PRF_ERR_REQ_DISALLOWED;
	}

	if (env.ntf_cfg != GATT_CCC_START_NTF) {
		return PRF_ERR_NTF_DISABLED;
	}

	mode_val = current_mode_value();

	status = co_buf_alloc(&p_buf, GATT_BUFFER_HEADER_LEN, sizeof(mode_val),
			      GATT_BUFFER_TAIL_LEN);
	if (status != CO_BUF_ERR_NO_ERROR) {
		return GAP_ERR_INSUFF_RESOURCES;
	}

	memcpy(co_buf_data(p_buf), &mode_val, sizeof(mode_val));
	status = gatt_srv_event_send(conidx, env.user_lid, ACS_METAINFO_MODE_STATUS_NTF_SEND,
				     GATT_NOTIFY, env.start_hdl + ACS_IDX_MODE_STATUS_VAL, p_buf);
	co_buf_release(p_buf);

	if (status == GAP_ERR_NO_ERROR) {
		env.ntf_ongoing = true;
		LOG_INF("Mode notification sent: mode=%u", mode_val);
	}

	return status;
}

static void on_event_sent(uint8_t conidx, uint8_t user_lid, uint16_t metainfo, uint16_t status)
{
	ARG_UNUSED(conidx);
	ARG_UNUSED(user_lid);

	if (metainfo == ACS_METAINFO_MODE_STATUS_NTF_SEND) {
		env.ntf_ongoing = false;
		LOG_DBG("Notification sent, status=%u", status);
	}

	if (status != GAP_ERR_NO_ERROR) {
		LOG_ERR("GATT event send failed, status %u", status);
	}
}

static void on_att_read_get(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t hdl,
			    uint16_t offset, uint16_t max_length)
{
	ARG_UNUSED(max_length);

	uint8_t mode_val;
	uint16_t u16_val;
	char frame_dur_str[CONTROL_PROFILE_FRAME_DUR_MAX + 1];
	char frame_rate_str[CONTROL_PROFILE_FRAME_RATE_MAX + 1];
	co_buf_t *p_buf = NULL;
	uint16_t status = GAP_ERR_NO_ERROR;
	uint16_t att_val_len = 0;
	const void *att_val = NULL;
	uint16_t ccc_value = env.ntf_cfg;
	const char *str_val = NULL;

	LOG_DBG("Read request: conidx=%u, hdl=%u", conidx, hdl);

	do {
		if (offset != 0) {
			status = ATT_ERR_INVALID_OFFSET;
			break;
		}

		switch (hdl - env.start_hdl) {
		case ACS_IDX_MODE_VAL:
		case ACS_IDX_MODE_STATUS_VAL:
			mode_val = current_mode_value();
			att_val_len = sizeof(mode_val);
			att_val = &mode_val;
			break;
		case ACS_IDX_MODE_STATUS_NTF_CFG:
			att_val_len = sizeof(ccc_value);
			att_val = &ccc_value;
			break;
		case ACS_IDX_STREAM_NAME_VAL:
			str_val = get_stream_name();
			att_val = str_val ? str_val : "";
			att_val_len = strlen((const char *)att_val);
			break;
		case ACS_IDX_ENCRYPTION_VAL:
			str_val = get_auracast_encryption_passwd();
			att_val = str_val ? str_val : "";
			att_val_len = strlen((const char *)att_val);
			break;
		case ACS_IDX_CODEC_VAL:
			att_val = env.codec;
			att_val_len = strlen(env.codec);
			break;
		case ACS_IDX_SDU_VAL:
			u16_val = sys_cpu_to_le16(env.octets_per_frame);
			att_val_len = sizeof(u16_val);
			att_val = &u16_val;
			break;
		case ACS_IDX_FRAME_DUR_VAL:
			if (frame_duration_to_string(env.frame_duration_us, frame_dur_str,
						     sizeof(frame_dur_str)) < 0) {
				status = ATT_ERR_REQUEST_NOT_SUPPORTED;
				break;
			}
			att_val = frame_dur_str;
			att_val_len = strlen(frame_dur_str);
			break;
		case ACS_IDX_FRAME_RATE_VAL:
			if (frame_rate_to_string(env.frame_rate_hz, frame_rate_str,
						 sizeof(frame_rate_str)) < 0) {
				status = ATT_ERR_REQUEST_NOT_SUPPORTED;
				break;
			}
			att_val = frame_rate_str;
			att_val_len = strlen(frame_rate_str);
			break;
		default:
			break;
		}

		if (att_val == NULL) {
			status = ATT_ERR_REQUEST_NOT_SUPPORTED;
			break;
		}

		status = co_buf_alloc(&p_buf, GATT_BUFFER_HEADER_LEN, att_val_len,
				      GATT_BUFFER_TAIL_LEN);
		if (status != CO_BUF_ERR_NO_ERROR) {
			status = ATT_ERR_INSUFF_RESOURCE;
			break;
		}

		if (att_val_len > 0) {
			memcpy(co_buf_data(p_buf), att_val, att_val_len);
		}
	} while (0);

	gatt_srv_att_read_get_cfm(conidx, user_lid, token, status, att_val_len, p_buf);
	if (p_buf != NULL) {
		co_buf_release(p_buf);
	}
}

static uint16_t handle_stream_name_write(co_buf_t *p_data)
{
	char name[CONTROL_PROFILE_STREAM_NAME_MAX + 1];
	int len = copy_write_string(name, sizeof(name), p_data);

	if (len < 0) {
		return ATT_ERR_INVALID_ATTRIBUTE_VAL_LEN;
	}

	if (len == 0) {
		set_stream_name(NULL);
		LOG_INF("Stream name cleared");
		return GAP_ERR_NO_ERROR;
	}

	if (set_stream_name(name) != 0) {
		return ATT_ERR_VALUE_NOT_ALLOWED;
	}

	/* Keep device name in sync for advertising / source PA */
	set_device_name(name);
	LOG_INF("Stream name set via GATT: %s", name);
	return GAP_ERR_NO_ERROR;
}

static uint16_t handle_encryption_write(co_buf_t *p_data)
{
	char passwd[CONTROL_PROFILE_ENCRYPTION_MAX + 1];
	int len = copy_write_string(passwd, sizeof(passwd), p_data);

	if (len < 0) {
		return ATT_ERR_INVALID_ATTRIBUTE_VAL_LEN;
	}

	if (len == 0) {
		set_auracast_encryption_passwd(NULL);
		LOG_INF("Encryption key cleared");
		return GAP_ERR_NO_ERROR;
	}

	if (set_auracast_encryption_passwd(passwd) != 0) {
		return ATT_ERR_VALUE_NOT_ALLOWED;
	}

	LOG_INF("Encryption key set via GATT (len=%d)", len);
	return GAP_ERR_NO_ERROR;
}

static uint16_t handle_codec_write(co_buf_t *p_data)
{
	char codec[CONTROL_PROFILE_CODEC_MAX + 1];
	uint16_t octets;
	uint32_t rate_hz;
	uint16_t duration_us;
	int len = copy_write_string(codec, sizeof(codec), p_data);

	if (len < 0) {
		return ATT_ERR_INVALID_ATTRIBUTE_VAL_LEN;
	}

	if (len == 0) {
		env.codec[0] = '\0';
		LOG_INF("Codec preset cleared; using SDU/rate/duration values");
		return GAP_ERR_NO_ERROR;
	}

	if (parse_codec_config(codec, &octets, &rate_hz, &duration_us) != 0) {
		LOG_ERR("Invalid codec '%s'", codec);
		return ATT_ERR_VALUE_NOT_ALLOWED;
	}

	strncpy(env.codec, codec, sizeof(env.codec) - 1);
	env.codec[sizeof(env.codec) - 1] = '\0';
	env.octets_per_frame = octets;
	env.frame_rate_hz = rate_hz;
	env.frame_duration_us = duration_us;

	LOG_INF("Codec '%s' -> sdu=%u rate=%u dur=%u", env.codec, env.octets_per_frame,
		env.frame_rate_hz, env.frame_duration_us);
	return GAP_ERR_NO_ERROR;
}

static uint16_t handle_sdu_write(co_buf_t *p_data)
{
	uint16_t sdu;

	if (co_buf_data_len(p_data) != sizeof(sdu)) {
		return ATT_ERR_INVALID_ATTRIBUTE_VAL_LEN;
	}

	memcpy(&sdu, co_buf_data(p_data), sizeof(sdu));
	sdu = sys_le16_to_cpu(sdu);
	if (sdu == 0 || sdu > 255) {
		return ATT_ERR_VALUE_NOT_ALLOWED;
	}

	env.octets_per_frame = sdu;
	env.codec[0] = '\0';
	LOG_INF("SDU octets set to %u", env.octets_per_frame);
	return GAP_ERR_NO_ERROR;
}

static uint16_t handle_frame_dur_write(co_buf_t *p_data)
{
	char duration_str[CONTROL_PROFILE_FRAME_DUR_MAX + 1];
	uint16_t duration_us;
	int len = copy_write_string(duration_str, sizeof(duration_str), p_data);

	if (len < 0) {
		return ATT_ERR_INVALID_ATTRIBUTE_VAL_LEN;
	}

	if (parse_frame_duration_string(duration_str, &duration_us) != 0) {
		LOG_ERR("Invalid frame duration '%s', use '7.5ms' or '10ms'", duration_str);
		return ATT_ERR_VALUE_NOT_ALLOWED;
	}

	env.frame_duration_us = duration_us;
	env.codec[0] = '\0';
	LOG_INF("Frame duration set to %s", duration_str);
	return GAP_ERR_NO_ERROR;
}

static uint16_t handle_frame_rate_write(co_buf_t *p_data)
{
	char rate_str[CONTROL_PROFILE_FRAME_RATE_MAX + 1];
	uint32_t rate_hz;
	int len = copy_write_string(rate_str, sizeof(rate_str), p_data);

	if (len < 0) {
		return ATT_ERR_INVALID_ATTRIBUTE_VAL_LEN;
	}

	if (parse_frame_rate_string(rate_str, &rate_hz) != 0) {
		LOG_ERR("Invalid frame rate '%s', use kHz value (8/16/24/32/48)", rate_str);
		return ATT_ERR_VALUE_NOT_ALLOWED;
	}

	env.frame_rate_hz = rate_hz;
	env.codec[0] = '\0';
	LOG_INF("Frame rate set to %s kHz", rate_str);
	return GAP_ERR_NO_ERROR;
}

static void on_att_val_set(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t hdl,
			   uint16_t offset, co_buf_t *p_data)
{
	uint16_t status = GAP_ERR_NO_ERROR;

	LOG_INF("Write request: conidx=%u, hdl=%u, len=%u", conidx, hdl, co_buf_data_len(p_data));

	do {
		if (offset != 0) {
			status = ATT_ERR_INVALID_OFFSET;
			break;
		}

		switch (hdl - env.start_hdl) {
		case ACS_IDX_MODE_VAL: {
			uint8_t new_mode;

			if (sizeof(uint8_t) != co_buf_data_len(p_data)) {
				status = ATT_ERR_INVALID_ATTRIBUTE_VAL_LEN;
				break;
			}

			memcpy(&new_mode, co_buf_data(p_data), sizeof(new_mode));
			LOG_INF("Mode switch request: %u", new_mode);

			if (new_mode >= CONTROL_PROFILE_MODE_MAX) {
				status = ATT_ERR_VALUE_NOT_ALLOWED;
				break;
			}

			if (queue_mode_command((enum control_profile_mode)new_mode) != 0) {
				status = ATT_ERR_REQUEST_NOT_SUPPORTED;
			}
			break;
		}

		case ACS_IDX_MODE_STATUS_NTF_CFG: {
			uint16_t cfg;

			if (sizeof(uint16_t) != co_buf_data_len(p_data)) {
				status = ATT_ERR_INVALID_ATTRIBUTE_VAL_LEN;
				break;
			}

			memcpy(&cfg, co_buf_data(p_data), sizeof(cfg));
			if (cfg == GATT_CCC_START_NTF || cfg == GATT_CCC_STOP_NTFIND) {
				env.ntf_cfg = cfg;
				LOG_INF("Mode status CCC %s for connection %u",
					cfg == GATT_CCC_START_NTF ? "enabled" : "disabled", conidx);
				if (cfg == GATT_CCC_START_NTF) {
					send_mode_notification();
				}
			} else {
				status = ATT_ERR_REQUEST_NOT_SUPPORTED;
			}
			break;
		}

		case ACS_IDX_STREAM_NAME_VAL:
			status = handle_stream_name_write(p_data);
			break;
		case ACS_IDX_ENCRYPTION_VAL:
			status = handle_encryption_write(p_data);
			break;
		case ACS_IDX_CODEC_VAL:
			status = handle_codec_write(p_data);
			break;
		case ACS_IDX_SDU_VAL:
			status = handle_sdu_write(p_data);
			break;
		case ACS_IDX_FRAME_DUR_VAL:
			status = handle_frame_dur_write(p_data);
			break;
		case ACS_IDX_FRAME_RATE_VAL:
			status = handle_frame_rate_write(p_data);
			break;

		default:
			status = ATT_ERR_REQUEST_NOT_SUPPORTED;
			break;
		}
	} while (0);

	gatt_srv_att_val_set_cfm(conidx, user_lid, token, status);
}

static const gatt_srv_cb_t gatt_cbs = {
	.cb_att_event_get = NULL,
	.cb_att_info_get = NULL,
	.cb_att_read_get = on_att_read_get,
	.cb_att_val_set = on_att_val_set,
	.cb_event_sent = on_event_sent,
};

static uint16_t service_register(void)
{
	uint16_t status;

	env.service_ready = false;
	env.ntf_ongoing = false;
	env.ntf_cfg = GATT_CCC_STOP_NTFIND;
	env.user_lid = 0;
	env.start_hdl = 0;

	status = gatt_user_srv_register(ACS_PREF_MTU, 0, &gatt_cbs, &env.user_lid);
	if (status != GAP_ERR_NO_ERROR) {
		LOG_ERR("gatt_user_srv_register failed: %u", status);
		return status;
	}

	status = gatt_db_svc_add(env.user_lid, SVC_UUID(128), acs_service_uuid, ACS_IDX_NB, NULL,
				 acs_att_db, ACS_IDX_NB, &env.start_hdl);
	if (status != GAP_ERR_NO_ERROR) {
		LOG_ERR("gatt_db_svc_add failed: %u", status);
		gatt_user_unregister(env.user_lid);
		env.user_lid = 0;
		return status;
	}

	env.service_ready = true;
	LOG_INF("Auracast control service registered, handle=%u", env.start_hdl);
	return GAP_ERR_NO_ERROR;
}

static uint16_t start_le_adv(uint8_t actv_idx)
{
	gapm_le_adv_param_t adv_params = {
		.duration = 0,
	};
	uint16_t err = gapm_le_start_adv(actv_idx, &adv_params);

	if (err) {
		LOG_ERR("Failed to start LE advertising, err %u", err);
	}

	return err;
}

static uint16_t set_advertising_data(uint8_t actv_idx)
{
	const char *name = get_device_name();
	size_t name_len;
	uint16_t adv_len;
	co_buf_t *p_buf;
	uint8_t *p_data;
	uint16_t err;

	if (name == NULL) {
		name = DEVICE_NAME_PREFIX_DEFAULT;
	}

	name_len = strlen(name);
	if (name_len > 29) {
		name_len = 29;
	}

	adv_len = name_len + 2;
	err = co_buf_alloc(&p_buf, 0, adv_len, 0);
	if (err != CO_BUF_ERR_NO_ERROR) {
		return GAP_ERR_INSUFF_RESOURCES;
	}

	p_data = co_buf_data(p_buf);
	p_data[0] = adv_len - 1;
	p_data[1] = 0x09; /* Complete Local Name */
	memcpy(p_data + 2, name, name_len);

	err = gapm_le_set_adv_data(actv_idx, p_buf);
	co_buf_release(p_buf);
	if (err) {
		LOG_ERR("Failed to set advertising data, err %u", err);
	}

	return err;
}

static uint16_t set_scan_data(uint8_t actv_idx)
{
	co_buf_t *p_buf;
	uint16_t err = co_buf_alloc(&p_buf, 0, 0, 0);

	if (err != CO_BUF_ERR_NO_ERROR) {
		return GAP_ERR_INSUFF_RESOURCES;
	}

	err = gapm_le_set_scan_response_data(actv_idx, p_buf);
	co_buf_release(p_buf);
	if (err) {
		LOG_ERR("Failed to set scan response data, err %u", err);
	}

	return err;
}

static void on_adv_actv_stopped(uint32_t metainfo, uint8_t actv_idx, uint16_t reason)
{
	ARG_UNUSED(metainfo);

	LOG_DBG("Control advertising stopped, idx=%u reason=%u", actv_idx, reason);
	env.adv_started = false;
}

static void on_adv_actv_proc_cmp(uint32_t metainfo, uint8_t proc_id, uint8_t actv_idx,
				 uint16_t status)
{
	ARG_UNUSED(metainfo);

	if (status) {
		LOG_ERR("Control advertising proc %u failed, status %u", proc_id, status);
		k_sem_give(&acs_adv_sem);
		return;
	}

	switch (proc_id) {
	case GAPM_ACTV_CREATE_LE_ADV:
		env.adv_actv_idx = actv_idx;
		env.adv_created = true;
		set_advertising_data(actv_idx);
		break;
	case GAPM_ACTV_SET_ADV_DATA:
		set_scan_data(actv_idx);
		break;
	case GAPM_ACTV_SET_SCAN_RSP_DATA:
		start_le_adv(actv_idx);
		break;
	case GAPM_ACTV_START:
		env.adv_started = true;
		LOG_INF("Control profile advertising started (actv %u)", actv_idx);
		k_sem_give(&acs_adv_sem);
		break;
	case GAPM_ACTV_STOP:
		env.adv_started = false;
		k_sem_give(&acs_adv_sem);
		break;
	case GAPM_ACTV_DELETE:
		env.adv_created = false;
		env.adv_actv_idx = GAP_INVALID_ACTV_IDX;
		k_sem_give(&acs_adv_sem);
		break;
	default:
		LOG_WRN("Unexpected control adv proc_id %u", proc_id);
		break;
	}
}

static void on_adv_created(uint32_t metainfo, uint8_t actv_idx, int8_t tx_pwr)
{
	ARG_UNUSED(metainfo);
	LOG_DBG("Control advertising created, idx=%u tx_pwr=%d", actv_idx, tx_pwr);
}

static const gapm_le_adv_cb_actv_t le_adv_cbs = {
	.hdr.actv.stopped = on_adv_actv_stopped,
	.hdr.actv.proc_cmp = on_adv_actv_proc_cmp,
	.created = on_adv_created,
};

static uint16_t create_advertising(void)
{
	gapm_le_adv_create_param_t adv_create_params = {
		.prop = GAPM_ADV_PROP_UNDIR_CONN_MASK,
		.disc_mode = GAPM_ADV_MODE_GEN_DISC,
		.tx_pwr = 0,
		.filter_pol = GAPM_ADV_ALLOW_SCAN_ANY_CON_ANY,
		.prim_cfg = {
			.adv_intv_min = 160,
			.adv_intv_max = 800,
			.ch_map = ADV_ALL_CHNLS_EN,
			.phy = GAPM_PHY_TYPE_LE_1M,
		},
	};

	env.adv_created = false;
	env.adv_started = false;
	env.adv_actv_idx = GAP_INVALID_ACTV_IDX;
	k_sem_reset(&acs_adv_sem);

	uint16_t err =
		gapm_le_create_adv_legacy(0, GAPM_STATIC_ADDR, &adv_create_params, &le_adv_cbs);

	if (err) {
		LOG_ERR("Failed to create control advertising, err %u", err);
	}

	return err;
}

int control_profile_start(void)
{
	uint16_t err;
	static bool params_initialized;

	if (!params_initialized) {
		source_params_init_defaults();
		params_initialized = true;
	}

	err = service_register();
	if (err != GAP_ERR_NO_ERROR) {
		return -EIO;
	}

	err = create_advertising();
	if (err != GAP_ERR_NO_ERROR) {
		return -EIO;
	}

	if (k_sem_take(&acs_adv_sem, K_SECONDS(2)) != 0) {
		LOG_ERR("Control advertising start timeout");
		return -ETIMEDOUT;
	}

	if (!env.adv_started) {
		LOG_ERR("Control advertising did not start");
		return -EIO;
	}

	LOG_INF("Auracast control profile ready");
	return 0;
}

int control_profile_stop_adv(void)
{
	uint16_t err;

	if (!env.adv_created || !env.adv_started) {
		return 0;
	}

	k_sem_reset(&acs_adv_sem);
	err = gapm_stop_activity(env.adv_actv_idx);
	if (err != GAP_ERR_NO_ERROR) {
		LOG_ERR("Failed to stop control advertising, err %u", err);
		return -EIO;
	}

	if (k_sem_take(&acs_adv_sem, K_SECONDS(2)) != 0) {
		return -ETIMEDOUT;
	}

	return 0;
}

int control_profile_restart_adv(void)
{
	uint16_t err;

	if (!env.service_ready) {
		return -ENODEV;
	}

	if (env.adv_started) {
		return 0;
	}

	if (!env.adv_created) {
		err = create_advertising();
		if (err != GAP_ERR_NO_ERROR) {
			return -EIO;
		}

		if (k_sem_take(&acs_adv_sem, K_SECONDS(2)) != 0) {
			return -ETIMEDOUT;
		}

		return env.adv_started ? 0 : -EIO;
	}

	k_sem_reset(&acs_adv_sem);
	err = start_le_adv(env.adv_actv_idx);
	if (err != GAP_ERR_NO_ERROR) {
		return -EIO;
	}

	if (k_sem_take(&acs_adv_sem, K_SECONDS(2)) != 0) {
		return -ETIMEDOUT;
	}

	return env.adv_started ? 0 : -EIO;
}

int control_profile_notify_mode(void)
{
	uint16_t status = send_mode_notification();

	if (status == PRF_ERR_NTF_DISABLED || status == PRF_ERR_REQ_DISALLOWED) {
		return 0;
	}

	return status == GAP_ERR_NO_ERROR ? 0 : -EIO;
}

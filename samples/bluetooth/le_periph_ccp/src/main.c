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
#include <string.h>
#include "alif_ble.h"
#include "gapm.h"
#include "gap_le.h"
#include "gapc_le.h"
#include "gapc_sec.h"
#include "gapm_le.h"
#include "gapm_le_adv.h"
#include "co_buf.h"
#include "address_verification.h"

#include "prf.h"
#include "acc_tbs.h"
#include <alif/bluetooth/bt_adv_data.h>
#include <alif/bluetooth/bt_scan_rsp.h>
#include "gapm_api.h"
#include "rwip_task.h"
#include "ble_storage.h"

/* URI Schemes Supported List value */
static const char call_server_uri_schemes_supported[] = "tel,skype";
/* UCI value */
static const char applet_call_server_uci[] = "E.164";

/* CCID of the Telephone Bearer Service instance */
#define CALL_BEARER_CCID (0x43)
/* Configuration bit field for Telephone Bearer Service instance */
#define CALL_BEARER_CFG_BF                                                                         \
	(ACC_TBS_CFG_SIGNAL_STRENGTH_SUPP_BIT | ACC_TBS_CFG_FRIENDLY_NAME_SUPP_BIT)

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

static struct connection_status app_con_info = {
	.conidx = GAP_INVALID_CONIDX,
	.addr.addr_type = 0xff,
};

struct app_env {
	bool connected;
	const char *p_provider_name;
	const char *p_friendly_name;
	const char *p_uri;

	uint32_t cli_cfg_bf;
	/* Signal strength reporting interval (in seconds) */
	uint8_t signal_strength_intv_s;

	bool start_call;
	bool call_started;
	uint8_t call_id;
};

static struct app_env env = {
	.connected = false,
	.p_friendly_name = NULL,
	.p_provider_name = NULL,
	.p_uri = NULL,
	.cli_cfg_bf = 0,
	.signal_strength_intv_s = 0,
	.start_call = false,
	.call_started = false,
	.call_id = 0,
};

enum app_call_server_event_call {
	/* Incoming call */
	EVENT_CALL_INCOMING = 0U,
	/* Outgoing call */
	EVENT_CALL_OUTGOING,
	/* Incoming call accepted */
	EVENT_CALL_ACCEPTED,
	/* Incoming call rejected */
	EVENT_CALL_REJECTED,
	/* Remote user has answered */
	EVENT_CALL_ANSWERED,
	/* Call has been terminated */
	EVENT_CALL_TERMINATED,
	/* Ringtone stream has been stopped */
	EVENT_CALL_RINGTONE_STOPPED,
};

/**
 * Bluetooth stack configuration
 */
static gapm_config_t gapm_cfg = {
	.role = GAP_ROLE_LE_PERIPHERAL,
	.pairing_mode = GAPM_PAIRING_SEC_CON,
	.privacy_cfg = GAPM_PRIV_CFG_PRIV_ADDR_BIT,
	.renew_dur = 1500,
	.private_identity.addr = {0},
	.irk.key = {0xA1, 0xB2, 0xC3, 0xD4, 0xE5, 0xF6, 0x07, 0x08, 0x11, 0x22, 0x32, 0x44, 0x55,
		    0x66, 0x77, 0x89},
	.gap_start_hdl = 0,
	.gatt_start_hdl = 0,
	.att_cfg = 0,
	.sugg_max_tx_octets = GAP_LE_MIN_OCTETS,
	.sugg_max_tx_time = GAP_LE_MIN_TIME,
	.tx_pref_phy = GAP_PHY_ANY,
	.rx_pref_phy = GAP_PHY_ANY,
	.tx_path_comp = 0,
	.rx_path_comp = 0,
	.class_of_device = 0,  /* BT Classic only */
	.dflt_link_policy = 0, /* BT Classic only */
};

/* Load name from configuration file */
#define DEVICE_NAME CONFIG_BLE_DEVICE_NAME

static void app_call_server_state_call(uint8_t event, uint16_t status)
{
	LOG_DBG("Call event - %d, status - %u", event, status);

	switch (event) {
	case EVENT_CALL_INCOMING: {
		uint16_t ret = acc_tbs_call_incoming(
			0, strlen(env.p_uri), 0, strlen(env.p_friendly_name),
			(const uint8_t *)(env.p_uri), NULL, (const uint8_t *)(env.p_friendly_name),
			&(env.call_id));
		if (ret) {
			LOG_ERR("Failed to process incoming call with error %u", ret);
		}

		/* Start Ringtone stream here */

	} break;

	case EVENT_CALL_OUTGOING: {
		uint16_t ret = acc_tbs_call_outgoing(
			0, strlen(env.p_uri), strlen(env.p_friendly_name),
			(const uint8_t *)(env.p_uri), (const uint8_t *)(env.p_friendly_name),
			&(env.call_id));
		if (ret) {
			LOG_ERR("Failed to process outgoing call with error %u", ret);
			break;
		}
		ret = acc_tbs_call_remote_alert_started(0, env.call_id);
		if (ret) {
			LOG_ERR("Failed to start remote alert with error %u", ret);
		}
	} break;

	case EVENT_CALL_ACCEPTED: {
		env.start_call = 1;
		env.call_started = 1;
	}
		/* no break */

	case EVENT_CALL_REJECTED: {

		/* Stop Ringtone stream here */
	} break;

	case EVENT_CALL_ANSWERED: {
		uint16_t ret = acc_tbs_call_answer(0, env.call_id);

		if (ret) {
			LOG_ERR("Failed to answer call with error %u", ret);
			break;
		}
		env.call_started = 1;

		/* Start call event here */
	} break;

	case EVENT_CALL_TERMINATED: {
		env.call_started = 0;

		/* Stop call event here */
	} break;

	case EVENT_CALL_RINGTONE_STOPPED: {
		if (env.start_call) {
			env.start_call = 0;

			/* Start call event here again? */
		}
	} break;

	default: {
		ASSERT_INFO(0, event, 0);
	} break;
	}
}

/* CCP callbacks */

void app_acc_tbs_cb_bond_data(uint8_t bearer_lid, uint8_t con_lid, uint16_t cli_cfg_bf)
{
	LOG_DBG("Bond data updated for bearer_lid %u, con_lid %u, cli_cfg_bf %u", bearer_lid,
		con_lid, cli_cfg_bf);

	env.cli_cfg_bf = cli_cfg_bf;
	/* TODO: store bond data */
}

void app_acc_tbs_cb_report_intv(uint8_t bearer_lid, uint8_t con_lid, uint8_t sign_strength_intv_s)
{
	LOG_DBG("Report interval updated for bearer_lid %u, con_lid %u, sign_strength_intv_s %u",
		bearer_lid, con_lid, sign_strength_intv_s);

	env.signal_strength_intv_s = sign_strength_intv_s;
	/* TODO: store bond data */
}

void app_acc_tbs_cb_get_req(uint8_t bearer_lid, uint8_t call_id, uint8_t con_lid, uint8_t char_type,
			    uint16_t token, uint16_t offset, uint16_t length)
{
	const char *p_val = NULL;

	LOG_DBG("Get request received for bearer_lid %u, call_id %u, con_lid %u, char_type %u, "
		"token %u, offset %u, length %u",
		bearer_lid, call_id, con_lid, char_type, token, offset, length);

	switch (char_type) {
	case ACC_TB_CHAR_TYPE_PROV_NAME: {
		p_val = env.p_provider_name;
	} break;
	case ACC_TB_CHAR_TYPE_CALL_FRIENDLY_NAME: {
		p_val = env.p_friendly_name;
	} break;
	case ACC_TB_CHAR_TYPE_URI_SCHEMES_LIST: {
		p_val = call_server_uri_schemes_supported;
	} break;
	default: {
	} break;
	}

	if (p_val != NULL) {
		length = strlen(p_val);
	} else {
		length = 0;
	}

	acc_tbs_cfm_get(true, bearer_lid, call_id, con_lid, char_type, token, offset, length,
			(const uint8_t *)p_val);
}

void app_acc_tbs_cb_call_req(uint8_t bearer_lid, uint8_t con_lid, uint8_t opcode, uint8_t call_id,
			     uint8_t len, const uint8_t *p_val)
{
	(void)p_val;
	LOG_DBG("Call request received for bearer_lid %u, con_lid %u, opcode %u, call_id %u, len "
		"%u",
		bearer_lid, con_lid, opcode, call_id, len);

	switch (opcode) {
	case ACC_TB_OPCODE_ACCEPT: {
		LOG_DBG("Call accepted");

		app_call_server_state_call(EVENT_CALL_ACCEPTED, GAF_ERR_NO_ERROR);
	} break;

	case ACC_TB_OPCODE_TERMINATE: {
		LOG_DBG("Terminate call received");

		if (!env.call_started) {
			app_call_server_state_call(EVENT_CALL_REJECTED, GAF_ERR_NO_ERROR);
		} else {
			app_call_server_state_call(EVENT_CALL_TERMINATED, GAF_ERR_NO_ERROR);
		}
	} break;

	default: {
		ASSERT_INFO(0, bearer_lid, opcode);
	} break;
	}

	acc_tbs_cfm_call(ACC_TB_CP_NTF_RESULT_SUCCESS, bearer_lid, 0, NULL);
}

static acc_tbs_cb_t ccp_cbs = {
	.cb_bond_data = app_acc_tbs_cb_bond_data,
	.cb_report_intv = app_acc_tbs_cb_report_intv,
	.cb_get_req = app_acc_tbs_cb_get_req,
	.cb_call_req = app_acc_tbs_cb_call_req,
};

static uint16_t create_advertising(void)
{
	uint16_t err = bt_gaf_create_adv(DEVICE_NAME, strlen(DEVICE_NAME), &app_con_info.addr);

	if (err != GAF_ERR_NO_ERROR) {
		LOG_ERR("Unable to configure GAF advertiser! Error %u (0x%02X)", err, err);
		return err;
	}
	LOG_DBG("GAF advertiser is configured");
	return err;
}

static uint16_t server_configure(void)
{
	uint8_t bearer_lid;
	uint16_t ret = GAF_ERR_NO_ERROR;

	ret = acc_tbs_configure(1, 2, 20, 20, &ccp_cbs, 0);
	if (ret) {
		LOG_ERR("CCP configuration failed %u", ret);
		return ret;
	}

	ret = acc_tbs_add(CALL_BEARER_CFG_BF, 0, CALL_BEARER_CCID, 0,
			  strlen(applet_call_server_uci), (const uint8_t *)applet_call_server_uci,
			  &bearer_lid);
	if (ret) {
		LOG_ERR("CCP add failed %u", ret);
	}

	return ret;
}

void app_connection_status_update(enum gapm_connection_event con_event, uint8_t con_idx,
				  uint16_t status)
{
	switch (con_event) {
	case GAPM_API_SEC_CONNECTED_KNOWN_DEVICE:
		env.connected = true;
		LOG_INF("Connection index %u connected to known device", con_idx);
		LOG_DBG("Please enable notifications on peer device..");
		break;
	case GAPM_API_DEV_CONNECTED:
		env.connected = true;
		LOG_INF("Connection index %u connected to new device", con_idx);
		LOG_DBG("Please enable notifications on peer device..");
		break;
	case GAPM_API_DEV_DISCONNECTED:
		LOG_INF("Connection index %u disconnected for reason %u", con_idx, status);
		env.connected = false;
		break;
	case GAPM_API_PAIRING_FAIL:
		LOG_INF("Connection pairing index %u fail for reason %u", con_idx, status);
		break;
	}
}

static gapm_user_cb_t gapm_user_cb = {
	.connection_status_update = app_connection_status_update,
};

int main(void)
{
	uint16_t err;

	env.connected = false;

	ble_storage_init();

	/* Start up bluetooth host stack */
	alif_ble_enable(NULL);

	/* Define Private identity */
	bt_generate_private_identity(&gapm_cfg);

	/* Configure Bluetooth Stack */
	LOG_INF("Init gapm service");
	err = bt_gapm_init(&gapm_cfg, &gapm_user_cb, DEVICE_NAME, strlen(DEVICE_NAME));
	if (err) {
		LOG_ERR("gapm_configure error %u", err);
		return -1;
	}

	err = server_configure();
	if (err) {
		LOG_ERR("Server configuration failed %u", err);
		return -1;
	}

	err = create_advertising();
	if (err) {
		LOG_ERR("Advertisement create fail %u", err);
		return -1;
	}

	/* Start a Generic audio advertisement */
	err = bt_gaf_adv_start(&app_con_info.addr);
	if (err) {
		LOG_ERR("Advertisement start fail %u", err);
		return -1;
	}

	print_device_identity();

	while (1) {
		k_sleep(K_SECONDS(1));
	}
}

/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "alif_ble.h"

#include "gap_le.h"
#include "gapc_le.h"
#include "gapc_sec.h"
#include "gapm.h"
#include "gapm_le.h"
#include "gapm_le_adv.h"
#include "address_verification.h"

#include "prf.h"
#include "wsc_common.h"
#include "wscs.h"

#include <alif/bluetooth/bt_adv_data.h>
#include <alif/bluetooth/bt_scan_rsp.h>
#include "gapm_api.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

#define APPEARANCE_GENERIC_WEIGHT_SCALE 0x0C80

#define DEVICE_APPEARANCE APPEARANCE_GENERIC_WEIGHT_SCALE

/* Load name from configuration file */
#define DEVICE_NAME CONFIG_BLE_DEVICE_NAME

/* Define advertising address type */
#define SAMPLE_ADDR_TYPE	ALIF_STATIC_RAND_ADDR

/* Store and share advertising address type */
static uint8_t adv_type;

static uint8_t client_conidx;
static uint8_t adv_actv_idx;
static bool ready_to_send;

static void on_cb_bond_data_upd(uint8_t conidx, uint16_t cfg_val)
{
	switch (cfg_val) {
	case PRF_CLI_STOP_NTFIND: {
		LOG_INF("Client requested stop notification/indication (conidx: %u)", conidx);
		ready_to_send = false;
	} break;

	case PRF_CLI_START_NTF:
	case PRF_CLI_START_IND: {
		LOG_INF("Client requested start notification/indication (conidx: %u)", conidx);
		ready_to_send = true;
	}
	}
}

static void on_cb_meas_send_cmp(uint8_t conidx, uint16_t status)
{
	if (status != GAP_ERR_NO_ERROR) {
		LOG_ERR("Measurement sending completion callback failed, error: %u", status);
		return;
	}
		ready_to_send = true;
}

static uint16_t utils_create_adv_data(void)
{
	const uint16_t svc_uuid = GATT_SVC_WEIGHT_SCALE;
	const uint16_t appearance = gapm_le_get_appearance();
	int ret;

	ret = bt_adv_data_set_tlv(GAP_AD_TYPE_COMPLETE_LIST_16_BIT_UUID, &svc_uuid,
				  GATT_UUID_16_LEN);
	if (ret) {
		LOG_ERR("AD profile set fail %d", ret);
		return ATT_ERR_INSUFF_RESOURCE;
	}

	ret = bt_adv_data_set_manufacturer(appearance, NULL, 0);

	if (ret) {
		LOG_ERR("AD appereance data fail %d", ret);
		return ATT_ERR_INSUFF_RESOURCE;
	}

	ret = bt_adv_data_set_name_auto(DEVICE_NAME, strlen(DEVICE_NAME));

	if (ret) {
		LOG_ERR("AD device name data fail %d", ret);
		return ATT_ERR_INSUFF_RESOURCE;
	}

	return bt_gapm_advertiment_data_set(adv_actv_idx);
}

static void on_le_connection_req(uint8_t conidx, uint32_t metainfo, uint8_t actv_idx, uint8_t role,
				 const gap_bdaddr_t *p_peer_addr,
				 const gapc_le_con_param_t *p_con_params, uint8_t clk_accuracy)
{
	uint16_t rc;

	rc = gapc_le_connection_cfm(conidx, 0, NULL);
	if (rc != GAP_ERR_NO_ERROR) {
		LOG_ERR("Failed to accept incoming connection, error: %u", rc);
		return;
	}

	LOG_INF("New client connection from %02X:%02X:%02X:%02X:%02X:%02X (conidx: %u)",
		p_peer_addr->addr[5], p_peer_addr->addr[4], p_peer_addr->addr[3],
		p_peer_addr->addr[2], p_peer_addr->addr[1], p_peer_addr->addr[0], conidx);

	client_conidx = conidx;
}

static void on_key_received(uint8_t conidx, uint32_t metainfo, const gapc_pairing_keys_t *p_keys)
{
	LOG_WRN("Received unexpected pairing key from conidx: %u", conidx);
}

static void on_disconnection(uint8_t conidx, uint32_t metainfo, uint16_t reason)
{
	uint16_t rc;

	ready_to_send = false;
	LOG_INF("Client disconnected (conidx: %u), restating advertising", conidx);

	client_conidx = GAP_INVALID_CONIDX;

	rc = bt_gapm_advertisement_continue(conidx);
	if (rc) {
		LOG_ERR("Failed to restart advertising, error: %u", rc);
		return;
	}
}

static void on_name_get(uint8_t conidx, uint32_t metainfo, uint16_t token, uint16_t offset,
			uint16_t max_len)
{
	LOG_WRN("Received unexpected name get from conidx: %u", conidx);
}

static void on_appearance_get(uint8_t conidx, uint32_t metainfo, uint16_t token)
{
	LOG_WRN("Received unexpected appearance get from conidx: %u", conidx);
}

static void on_gapm_err(uint32_t metainfo, uint8_t code)
{
	LOG_ERR("gapm error %d", code);
}

static uint16_t utils_add_profile(void)
{
	static const struct wscs_db_cfg db_cfg = {
		.feature = 0,
		.bcs_start_hdl = GATT_INVALID_HDL,
	};

	static const wscs_cb_t wscs_cbs = {
		.cb_bond_data_upd = on_cb_bond_data_upd,
		.cb_meas_send_cmp = on_cb_meas_send_cmp,
	};

	uint16_t start_hdl = GATT_INVALID_HDL;

	return prf_add_profile(TASK_ID_WSCS, 0, 0, &db_cfg, &wscs_cbs, &start_hdl);
}

static uint16_t utils_create_adv(void)
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

	return bt_gapm_le_create_advertisement_service(adv_type, &adv_create_params, NULL,
						      &adv_actv_idx);
}

static uint16_t utils_config_gapm(void)
{
	static gapm_config_t gapm_cfg = {
		.role = GAP_ROLE_LE_PERIPHERAL,
		.pairing_mode = GAPM_PAIRING_DISABLE,
		.pairing_min_req_key_size = 0,
		.privacy_cfg = 0,
		.renew_dur = 1500,
		.private_identity.addr = {0, 0, 0, 0, 0, 0},
		.irk.key = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		.gap_start_hdl = 0,
		.gatt_start_hdl = 0,
		.att_cfg = 0,
		.sugg_max_tx_octets = GAP_LE_MIN_OCTETS,
		.sugg_max_tx_time = GAP_LE_MIN_TIME,
		.tx_pref_phy = GAP_PHY_ANY,
		.rx_pref_phy = GAP_PHY_ANY,
		.tx_path_comp = 0,
		.rx_path_comp = 0,
		.class_of_device = 0,
		.dflt_link_policy = 0,
	};

	if (address_verification(SAMPLE_ADDR_TYPE, &adv_type, &gapm_cfg)) {
		LOG_ERR("Address verification failed");
		return -EADV;
	}

	static const gapc_connection_req_cb_t gapc_con_cbs = {
		.le_connection_req = on_le_connection_req,
	};

	static const gapc_security_cb_t gapc_sec_cbs = {
		.key_received = on_key_received,
	};

	static const gapc_connection_info_cb_t gapc_con_inf_cbs = {
		.disconnected = on_disconnection,
		.name_get = on_name_get,
		.appearance_get = on_appearance_get,
	};

	static const gapc_le_config_cb_t gapc_le_cfg_cbs = {};

	static const gapm_cb_t gapm_err_cbs = {
		.cb_hw_error = on_gapm_err,
	};

	static const gapm_callbacks_t gapm_cbs = {
		.p_con_req_cbs = &gapc_con_cbs,
		.p_sec_cbs = &gapc_sec_cbs,
		.p_info_cbs = &gapc_con_inf_cbs,
		.p_le_config_cbs = &gapc_le_cfg_cbs,
		.p_bt_config_cbs = NULL, /* BT classic so not required */
		.p_gapm_cbs = &gapm_err_cbs,
	};

	return bt_gapm_init(&gapm_cfg, &gapm_cbs, DEVICE_NAME, strlen(DEVICE_NAME));
}

static void send_measurement(void)
{
	static uint16_t weight;
	uint16_t rc;

	const wsc_meas_t meas = {
		.flags = 0,
		.weight = weight,
		.time_stamp = {0},
		.user_id = 0,
		.bmi = 0,
		.height = 0,
	};

	rc = wscs_meas_send(client_conidx, &meas);

	ready_to_send = false;

	if (rc != GAP_ERR_NO_ERROR) {
		LOG_ERR("Failed to send wscs measurement (conidx: %u), error: %u", client_conidx,
			rc);
		return;
	}

	LOG_INF("Sent measurement: %u (conidx: %u)", weight, client_conidx);

	weight++;

	if (weight > 200) {
		weight = 0;
	}
}

int main(void)
{
	int rc;
	uint16_t err;

	LOG_INF("Enabling Alif BLE stack");
	rc = alif_ble_enable(NULL);
	if (rc) {
		LOG_ERR("Failed to enable Alif BLE stack, error: %i", rc);
		return -1;
	}

	LOG_INF("Setting device appearance: %u", DEVICE_APPEARANCE);
	err = gapm_le_set_appearance(DEVICE_APPEARANCE);
	if (err != GAP_ERR_NO_ERROR) {
		LOG_ERR("Failed to set device appearance, error: %u", err);
		return -1;
	}

	LOG_INF("Configuring GAP manager");
	err = utils_config_gapm();
	if (err != GAP_ERR_NO_ERROR) {
		LOG_ERR("Failed to configure GAP, error: %u", err);
		return -1;
	}

	LOG_INF("Adding profile");
	err = utils_add_profile();
	if (err != GAP_ERR_NO_ERROR) {
		LOG_ERR("Failed to add WSCS profile, error: %u", err);
		return -1;
	}

	LOG_INF("Creating advertisement");
	err = utils_create_adv();
	if (err != GAP_ERR_NO_ERROR) {
		LOG_ERR("Failed to create advertising activity, error: %u", err);
		return -1;
	}

	err = utils_create_adv_data();

	err = bt_gapm_scan_response_set(adv_actv_idx);
	if (err) {
		LOG_ERR("Scan response set fail %u", err);
		return -1;
	}

	err = bt_gapm_advertisement_start(adv_actv_idx);
	if (err) {
		LOG_ERR("Advertisement start fail %u", err);
		return -1;
	}

	print_device_identity();

	LOG_INF("Waiting for a client");
	while (1) {
		k_sleep(K_SECONDS(2));
		if (ready_to_send) {
			send_measurement();
		}
	}

	return 0;
}

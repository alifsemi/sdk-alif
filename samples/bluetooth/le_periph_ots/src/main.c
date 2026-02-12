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
#include "acc_ots.h"
#include "shared_control.h"
#include <alif/bluetooth/bt_adv_data.h>
#include <alif/bluetooth/bt_scan_rsp.h>
#include "gapm_api.h"
#include "rwip_task.h"


/* Define advertising address type */
#define SAMPLE_ADDR_TYPE	ALIF_STATIC_RAND_ADDR

/* Store and share advertising address type */
static uint8_t adv_type;

K_SEM_DEFINE(conn_sem, 0, 1);

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

struct app_env {
	bool connected;
	uint8_t ots_trf_lid;
	uint16_t ots_start_hdl;
};

static struct app_env env = {
	.connected = false,
};

/**
 * Bluetooth stack configuration
 */
static gapm_config_t gapm_cfg = {
	.role = GAP_ROLE_LE_PERIPHERAL,
	.pairing_mode = GAPM_PAIRING_SEC_CON,
	.privacy_cfg = GAPM_PRIV_CFG_PRIV_ADDR_BIT,
	.renew_dur = 1500,
	.private_identity.addr = {0xCA, 0xFE, 0xFB, 0xDE, 0x15, 0x08},
	.irk.key = {0xA1, 0xB2, 0xC3, 0xD4, 0xE5, 0xF6, 0x07, 0x08, 0x11, 0x22, 0x33, 0x44, 0x55,
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

/* Store advertising activity index for re-starting after disconnection */
static uint8_t adv_actv_idx;

/* OTS callbacks */
void app_ots_cb_bond_data(uint8_t transfer_lid, uint8_t con_lid, uint8_t cli_cfg_bf)
{
	LOG_DBG("Bond data updated for transfer_lid %u, con_lid %u, cli_cfg_bf 0x%02x",
		transfer_lid, con_lid, cli_cfg_bf);
}

void app_ots_cb_coc_connected(uint8_t con_lid, uint16_t peer_max_sdu, uint16_t local_max_sdu)
{
	LOG_DBG("LE CoC connected for con_lid %u, peer_max_sdu %u, local_max_sdu %u", con_lid,
		peer_max_sdu, local_max_sdu);
}

void app_ots_cb_coc_disconnected(uint8_t con_lid, uint16_t reason)
{
	LOG_DBG("LE CoC disconnected for con_lid %u, reason %u", con_lid, reason);
}

void app_ots_cb_coc_data(uint8_t con_lid, uint16_t length, const uint8_t *p_sdu)
{
	(void)p_sdu;
	LOG_DBG("LE CoC data received for con_lid %u, length %u", con_lid, length);
}

void app_ots_cb_get_name(uint8_t con_lid, uint8_t transfer_lid, uint8_t object_lid, uint16_t token,
		     uint16_t offset, uint16_t max_len)
{
	LOG_DBG("Get name requested for con_lid %u, transfer_lid %u, object_lid %u, token %u, "
		"offset %u, max_len %u",
		con_lid, transfer_lid, object_lid, token, offset, max_len);
}

void app_ots_cb_set_name(uint8_t con_lid, uint8_t transfer_lid, uint8_t object_lid, uint16_t token,
		     uint8_t name_len, const uint8_t *p_name)
{
	(void)p_name;
	LOG_DBG("Set name requested for con_lid %u, transfer_lid %u, object_lid %u, token %u, "
		"name_len %u",
		con_lid, transfer_lid, object_lid, token, name_len);
}

void app_ots_cb_object_create(uint8_t con_lid, uint8_t transfer_lid, uint16_t token, uint32_t size,
			  uint8_t uuid_type, const void *p_uuid)
{
	(void)p_uuid;
	LOG_DBG("Object create requested for con_lid %u, transfer_lid %u, token %u, size %u, "
		"uuid_type %u",
		con_lid, transfer_lid, token, size, uuid_type);
}

void app_ots_cb_object_execute(uint8_t con_lid, uint8_t transfer_lid, uint8_t object_lid,
			   uint16_t token, uint16_t param_len, const uint8_t *p_param)
{
	(void)p_param;
	LOG_DBG("Object execute requested for con_lid %u, transfer_lid %u, object_lid %u, token "
		"%u, param_len %u",
		con_lid, transfer_lid, object_lid, token, param_len);
}

void app_ots_cb_object_manipulate(uint8_t con_lid, uint8_t transfer_lid, uint8_t object_lid,
			      uint16_t token, uint8_t opcode, uint32_t offset, uint32_t length,
			      uint8_t mode)
{
	LOG_DBG("Object manipulate requested for con_lid %u, transfer_lid %u, object_lid %u, token "
		"%u, opcode %u, offset %u, length %u, mode %u",
		con_lid, transfer_lid, object_lid, token, opcode, offset, length, mode);
}

void app_ots_cb_object_control(uint8_t con_lid, uint8_t transfer_lid, uint8_t object_lid,
			   uint16_t token, uint8_t opcode)
{
	LOG_DBG("Object control requested for con_lid %u, transfer_lid %u, object_lid %u, token "
		"%u, opcode %u",
		con_lid, transfer_lid, object_lid, token, opcode);
}

void app_ots_cb_filter_get(uint8_t con_lid, uint8_t transfer_lid, uint8_t filter_lid,
		       uint32_t ots_token, uint16_t offset, uint16_t max_len)
{
	LOG_DBG("Filter get requested for con_lid %u, transfer_lid %u, filter_lid %u, ots_token "
		"%u, offset %u, max_len %u",
		con_lid, transfer_lid, filter_lid, ots_token, offset, max_len);
}

void app_ots_cb_list(uint16_t req_ind_code, uint8_t con_lid, uint8_t transfer_lid, uint16_t token,
		 uint8_t opcode, const void *p_value)
{
	(void)p_value;
	LOG_DBG("List requested for req_ind_code %u, con_lid %u, transfer_lid %u, token %u, opcode "
		"%u",
		req_ind_code, con_lid, transfer_lid, token, opcode);
}

void app_ots_cb_filter_set(uint16_t req_ind_code, uint8_t con_lid, uint8_t transfer_lid,
		       uint8_t filter_lid, uint16_t token, uint8_t filter_val, const void *p_value1,
		       const void *p_value2)
{
	(void)p_value1;
	(void)p_value2;
	LOG_DBG("Filter set requested for req_ind_code %u, con_lid %u, transfer_lid %u, filter_lid "
		"%u, token %u, filter_val %u",
		req_ind_code, con_lid, transfer_lid, filter_lid, token, filter_val);
}

void app_ots_cb_coc_connect(uint8_t con_lid, uint16_t token, uint16_t peer_max_sdu)
{
	LOG_DBG("LE CoC connect requested for con_lid %u, token %u, peer_max_sdu %u", con_lid,
		token, peer_max_sdu);
}

void app_ots_cb_cmp_evt(uint16_t cmd_code, uint16_t status, uint8_t con_lid)
{
	LOG_DBG("Command complete for cmd_code %u, status %u, con_lid %u", cmd_code, status,
		con_lid);
}

static const acc_ots_cb_t ots_callbacks = {
	.cb_bond_data = app_ots_cb_bond_data,
	.cb_coc_connected = app_ots_cb_coc_connected,
	.cb_coc_disconnected = app_ots_cb_coc_disconnected,
	.cb_coc_data = app_ots_cb_coc_data,
	.cb_get_name = app_ots_cb_get_name,
	.cb_set_name = app_ots_cb_set_name,
	.cb_object_create = app_ots_cb_object_create,
	.cb_object_execute = app_ots_cb_object_execute,
	.cb_object_manipulate = app_ots_cb_object_manipulate,
	.cb_object_control = app_ots_cb_object_control,
	.cb_filter_get = app_ots_cb_filter_get,
	.cb_list = app_ots_cb_list,
	.cb_filter_set = app_ots_cb_filter_set,
	.cb_coc_connect = app_ots_cb_coc_connect,
	.cb_cmp_evt = app_ots_cb_cmp_evt,
};

static uint16_t set_advertising_data(uint8_t actv_idx)
{
	int ret;
	uint16_t svc[1];
	uint16_t comp_id = CONFIG_BLE_COMPANY_ID;

	/* Define GATT profiles */
	svc[0] = GATT_SVC_OBJECT_TRANSFER;

	ret = bt_adv_data_set_tlv(GAP_AD_TYPE_COMPLETE_LIST_16_BIT_UUID, svc, sizeof(svc));
	if (ret) {
		LOG_ERR("AD profile set fail %d", ret);
		return ATT_ERR_INSUFF_RESOURCE;
	}

	ret = bt_adv_data_set_manufacturer(comp_id, NULL, 0);

	if (ret) {
		LOG_ERR("AD manufacturer data fail %d", ret);
		return ATT_ERR_INSUFF_RESOURCE;
	}

	ret = bt_adv_data_set_name_auto(DEVICE_NAME, strlen(DEVICE_NAME));

	if (ret) {
		LOG_ERR("AD device name data fail %d", ret);
		return ATT_ERR_INSUFF_RESOURCE;
	}

	return bt_gapm_advertiment_data_set(actv_idx);
}

static uint16_t create_advertising(void)
{
	gapm_le_adv_create_param_t adv_create_params = {
		.prop = GAPM_ADV_PROP_UNDIR_CONN_MASK,
		.disc_mode = GAPM_ADV_MODE_GEN_DISC,
		.tx_pwr = 0,
		.filter_pol = GAPM_ADV_ALLOW_SCAN_ANY_CON_ANY,
		.prim_cfg = {
				.adv_intv_min = 160, /* 100 ms */
				.adv_intv_max = 800, /* 500 ms */
				.ch_map = ADV_ALL_CHNLS_EN,
				.phy = GAPM_PHY_TYPE_LE_1M,
			},
	};

	return bt_gapm_le_create_advertisement_service(adv_type, &adv_create_params, NULL,
						      &adv_actv_idx);
}

#define NUMBER_OF_OTS_INSTANCES 1
/* Add Object transfer profile to the stack */
static uint16_t server_configure(void)
{
	uint16_t cfg_flags = OTS_ADD_CFG_ACCESS_CLOCK_BIT;
	uint16_t ret = acc_ots_configure(NUMBER_OF_OTS_INSTANCES, &ots_callbacks);

	if (ret) {
		LOG_ERR("OTS config failed with error %u", ret);
		return ret;
	}

	return acc_ots_add(cfg_flags, GATT_INVALID_HDL, 0, 0, &env.ots_trf_lid, &env.ots_start_hdl);
}

void app_connection_status_update(enum gapm_connection_event con_event, uint8_t con_idx,
				  uint16_t status)
{
	switch (con_event) {
	case GAPM_API_SEC_CONNECTED_KNOWN_DEVICE:
		env.connected = true;
		k_sem_give(&conn_sem);
		LOG_INF("Connection index %u connected to known device", con_idx);
		LOG_DBG("Please enable notifications on peer device..");
		break;
	case GAPM_API_DEV_CONNECTED:
		env.connected = true;
		k_sem_give(&conn_sem);
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
	env.ots_trf_lid = 0;
	env.ots_start_hdl = 0;

	/* Start up bluetooth host stack */
	alif_ble_enable(NULL);

	if (address_verification(SAMPLE_ADDR_TYPE, &adv_type, &gapm_cfg)) {
		LOG_ERR("Address verification failed");
		return -EADV;
	}

	/* Configure Bluetooth Stack */
	LOG_INF("Init gapm service");
	err = bt_gapm_init(&gapm_cfg, &gapm_user_cb, DEVICE_NAME, strlen(DEVICE_NAME));
	if (err) {
		LOG_ERR("gapm_configure error %u", err);
		return -1;
	}

	LOG_INF("Configuring OTS server");
	err = server_configure();

	if (err) {
		LOG_ERR("OTS server configuration failed %u", err);
		return -1;
	}

	err = create_advertising();
	if (err) {
		LOG_ERR("Advertisement create fail %u", err);
		return -1;
	}

	err = set_advertising_data(adv_actv_idx);
	if (err) {
		LOG_ERR("Advertisement data set fail %u", err);
		return -1;
	}

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

	while (1) {
		k_sleep(K_SECONDS(1));
	}
}

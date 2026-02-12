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
#include "tips.h"
#include <alif/bluetooth/bt_adv_data.h>
#include <alif/bluetooth/bt_scan_rsp.h>
#include "gapm_api.h"
#include "rwip_task.h"


/* Define advertising address type */
#define SAMPLE_ADDR_TYPE	ALIF_STATIC_RAND_ADDR

/* Store and share advertising address type */
static uint8_t adv_type;

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

struct app_env {
	bool connected;

	tip_curr_time_t current_time;
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

/* TIPS callbacks */

void app_cb_bond_data_upd(uint8_t conidx, uint16_t cfg_val)
{
	LOG_DBG("Bond data updated for conidx %u, cfg_val 0x%04x", conidx, cfg_val);
}

void app_cb_curr_time_upd_cmp(uint8_t conidx, uint16_t status)
{
	LOG_DBG("Current time update complete for conidx %u, status 0x%04x", conidx, status);
}

void app_cb_rd_info_req(uint8_t conidx, uint32_t token, uint8_t val_id)
{
	LOG_DBG("Read info request for conidx %u, token %u, val_id %u", conidx, token, val_id);

	env.current_time.date_time.year = 2026;
	env.current_time.date_time.month = 1;
	env.current_time.date_time.day = 4;
	env.current_time.date_time.hour = 10;
	env.current_time.date_time.min = 51;
	env.current_time.date_time.sec = 3;

	env.current_time.day_of_week = 0;
	env.current_time.fraction_256 = 0;
	env.current_time.adjust_reason = 0;

	tip_curr_time_t *p_time = (tip_curr_time_t *)(&env.current_time);

	tips_rd_info_cfm(conidx, token, val_id, (const union tip_value *)(p_time));
}

void app_cb_ctnl_pt(uint8_t conidx, tip_time_upd_contr_pt_t value)
{
	LOG_DBG("Control point request for conidx %u, value %u", conidx, value);
}

static tips_cb_t tip_cbs = {
	.cb_bond_data_upd = app_cb_bond_data_upd,
	.cb_curr_time_upd_cmp = app_cb_curr_time_upd_cmp,
	.cb_rd_info_req = app_cb_rd_info_req,
	.cb_ctnl_pt = app_cb_ctnl_pt,
};

static uint16_t set_advertising_data(uint8_t actv_idx)
{
	int ret;
	uint16_t svc[1];
	uint16_t comp_id = CONFIG_BLE_COMPANY_ID;

	/* Define GATT profiles */
	svc[0] = GATT_SVC_CURRENT_TIME;

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

/* Add Time profile to the stack */
static uint16_t server_configure(void)
{
	tips_db_cfg_t cfg_flags = {
		.features = 0,
	};

	return tips_add(&cfg_flags, &tip_cbs, GATT_INVALID_HDL);
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

/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/*
 * BLE peripheral exposing the Hearing Access Service (HAS) server role
 * via the Hearing Access Profile (HAP).
 *
 * Advertises as a monaural hearing aid with a single preset ("Default").
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

/* Hearing Access Profile – Server role */
#include "hap_has.h"
#include "hap.h"

/* GATT service UUID definitions (for GATT_SVC_HEARING_ACCESS) */
#include "gatt.h"

/* Common subsys helpers */
#include <alif/bluetooth/bt_adv_data.h>
#include <alif/bluetooth/bt_scan_rsp.h>
#include "gapm_api.h"
#include "ble_storage.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

/* -------------------------------------------------------------------------
 * Application state
 * -------------------------------------------------------------------------
 */

struct app_env {
	bool    connected;
	uint8_t con_lid;
	/* HAS bonding data – persisted across reconnections */
	uint8_t has_cli_cfg_ntf_bf;
	uint8_t has_cli_cfg_ind_bf;
};

static struct app_env env = {
	.connected          = false,
	.con_lid            = GAP_INVALID_CONIDX,
	.has_cli_cfg_ntf_bf = HAP_HAS_CHAR_TYPE_PRESET_CP | HAP_HAS_CHAR_TYPE_FEATURES,
	.has_cli_cfg_ind_bf = 0,
};

/* Advertising address type, resolved at startup by address_verification() */
static uint8_t adv_type;

/* Activity index used to continue advertising after disconnection */
static uint8_t adv_actv_idx;

/* -------------------------------------------------------------------------
 * Bluetooth stack configuration
 * -------------------------------------------------------------------------
 */

static gapm_config_t gapm_cfg = {
	.role               = GAP_ROLE_LE_PERIPHERAL,
	.pairing_mode       = GAPM_PAIRING_SEC_CON,
	.privacy_cfg        = 0,
	.renew_dur          = 1500,
	.private_identity.addr = {0},
	.irk.key            = {0},
	.gap_start_hdl      = 0,
	.gatt_start_hdl     = 0,
	.att_cfg            = 0,
	.sugg_max_tx_octets = GAP_LE_MIN_OCTETS,
	.sugg_max_tx_time   = GAP_LE_MIN_TIME,
	.tx_pref_phy        = GAP_PHY_ANY,
	.rx_pref_phy        = GAP_PHY_ANY,
	.tx_path_comp       = 0,
	.rx_path_comp       = 0,
	.class_of_device    = 0,
	.dflt_link_policy   = 0,
};

#define DEVICE_NAME      CONFIG_BLE_DEVICE_NAME
#define SAMPLE_ADDR_TYPE ALIF_PUBLIC_ADDR

/* -------------------------------------------------------------------------
 * HAS preset table
 * -------------------------------------------------------------------------
 */

#define PRESET_IDX_DEFAULT 0u

static const char preset_name_default[] = "Default";

/* -------------------------------------------------------------------------
 * HAS callbacks
 * -------------------------------------------------------------------------
 */

static void on_has_bond_data(uint8_t con_lid,
			     uint8_t cli_cfg_ntf_bf,
			     uint8_t cli_cfg_ind_bf)
{
	LOG_DBG("HAS bond data update – con_lid %u ntf_bf 0x%02X ind_bf 0x%02X",
		con_lid, cli_cfg_ntf_bf, cli_cfg_ind_bf);
	env.has_cli_cfg_ntf_bf = cli_cfg_ntf_bf;
	env.has_cli_cfg_ind_bf = cli_cfg_ind_bf;
}

static void on_has_set_active_preset_req(uint8_t con_lid,
					 uint8_t preset_lid,
					 bool relay)
{
	LOG_DBG("HAS set active preset – con_lid %u preset_lid %u relay %u",
		con_lid, preset_lid, relay);
	hap_has_set_active_preset_cfm(true);
}

static void on_has_set_preset_name_req(uint8_t con_lid,
				       uint8_t preset_lid,
				       uint8_t length,
				       const uint8_t *p_name)
{
	LOG_DBG("HAS set preset name – con_lid %u preset_lid %u len %u name \"%.*s\"",
		con_lid, preset_lid, length, (int)length, p_name);
	/* Reject: writable presets are not enabled in this sample. */
	hap_has_set_preset_name_cfm(false, 0, NULL);
}

static const hap_has_cb_t haps_cbs = {
	.cb_bond_data             = on_has_bond_data,
	.cb_set_active_preset_req = on_has_set_active_preset_req,
	.cb_set_preset_name_req   = on_has_set_preset_name_req,
};

/* -------------------------------------------------------------------------
 * GAPM connection status callback
 * -------------------------------------------------------------------------
 */

static void on_connection_status_update(enum gapm_connection_event con_event,
					uint8_t con_idx,
					uint16_t status)
{
	switch (con_event) {
	case GAPM_API_DEV_CONNECTED:
		env.connected = true;
		env.con_lid   = con_idx;
		LOG_INF("Connected – index %u (new device)", con_idx);
		break;

	case GAPM_API_SEC_CONNECTED_KNOWN_DEVICE:
		env.connected = true;
		env.con_lid   = con_idx;
		LOG_INF("Connected – index %u (known device)", con_idx);
		/* Restore HAS bond data for this connection. */
		hap_has_restore_bond_data(con_idx,
					  env.has_cli_cfg_ntf_bf,
					  env.has_cli_cfg_ind_bf,
					  0, 0, NULL);
		break;

	case GAPM_API_DEV_DISCONNECTED:
		env.connected = false;
		env.con_lid   = GAP_INVALID_CONIDX;
		LOG_INF("Disconnected – index %u reason 0x%04X", con_idx, status);
		break;

	case GAPM_API_PAIRING_FAIL:
		LOG_WRN("Pairing failed – index %u status 0x%04X", con_idx, status);
		break;
	}
}

static gapm_user_cb_t gapm_user_cb = {
	.connection_status_update = on_connection_status_update,
};

/* -------------------------------------------------------------------------
 * HAS configuration
 * -------------------------------------------------------------------------
 */

static uint16_t configure_has(void)
{
	hap_has_cfg_param_t cfg = {
		.cfg_bf      = HAP_HAS_CFG_FEATURES_NTF_SUPP_BIT,
		.pref_mtu    = 0,
		.shdl        = 0,
		.nb_presets  = 1,
		.features_bf = HAP_HAS_FEATURES_TYPE_MONAURAL,
	};

	uint16_t err = hap_has_configure(&cfg, &haps_cbs);

	if (err != GAF_ERR_NO_ERROR) {
		LOG_ERR("hap_has_configure error %u", err);
		return err;
	}

	err = hap_has_add_preset(PRESET_IDX_DEFAULT,
				 true, /* writable */
				 true,  /* available */
				 (uint8_t)strlen(preset_name_default),
				 (const uint8_t *)preset_name_default);
	if (err != GAF_ERR_NO_ERROR) {
		LOG_ERR("hap_has_add_preset error %u", err);
		return err;
	}

	err = hap_has_set_active_preset(PRESET_IDX_DEFAULT);
	if (err != GAF_ERR_NO_ERROR) {
		LOG_ERR("hap_has_set_active_preset error %u", err);
	}

	return err;
}

/* -------------------------------------------------------------------------
 * Advertising helpers
 * -------------------------------------------------------------------------
 */

static uint16_t set_advertising_data(uint8_t actv_idx)
{
	int ret;
	uint16_t svc = GATT_SVC_HEARING_ACCESS;

	ret = bt_adv_data_set_tlv(GAP_AD_TYPE_COMPLETE_LIST_16_BIT_UUID, &svc, sizeof(svc));
	if (ret) {
		LOG_ERR("AD service UUID set fail %d", ret);
		return ATT_ERR_INSUFF_RESOURCE;
	}

	ret = bt_adv_data_set_name_auto(DEVICE_NAME, strlen(DEVICE_NAME));
	if (ret) {
		LOG_ERR("AD device name set fail %d", ret);
		return ATT_ERR_INSUFF_RESOURCE;
	}

	return bt_gapm_advertiment_data_set(actv_idx);
}

static uint16_t create_advertising(void)
{
	gapm_le_adv_create_param_t adv_create_params = {
		.prop       = GAPM_ADV_PROP_UNDIR_CONN_MASK,
		.disc_mode  = GAPM_ADV_MODE_GEN_DISC,
		.tx_pwr     = 0,
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

/* -------------------------------------------------------------------------
 * main
 * -------------------------------------------------------------------------
 */

int main(void)
{
	uint16_t err;

	ble_storage_init();

	alif_ble_enable(NULL);

	if (address_verification(SAMPLE_ADDR_TYPE, &adv_type, &gapm_cfg)) {
		LOG_ERR("Address verification failed");
		return -1;
	}

	err = bt_gapm_init(&gapm_cfg, &gapm_user_cb, DEVICE_NAME, strlen(DEVICE_NAME));
	if (err) {
		LOG_ERR("bt_gapm_init error %u", err);
		return -1;
	}

	err = configure_has();
	if (err) {
		LOG_ERR("HAS configuration failed %u", err);
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

	LOG_INF("HAS peripheral running – waiting for connection...");

	return 0;
}

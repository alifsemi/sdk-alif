/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>
#include <zephyr/sys/__assert.h>
#include "alif_ble.h"
#include "gapm.h"
#include "gapm_le_init.h"
#include "gap_le.h"
#include "gapc.h"
#include "gapc_le.h"
#include "gapc_sec.h"
#include "main.h"
#include "unicast_source.h"
#include "storage.h"

/**
 * Enable for bonding support
 * FIXME: issue with stereo streaming if enabled!
 */
#define BONDING_ENABLED 1

#define APPEARANCE_EARBUDS 0x0941
#define APPEARANCE_HEADSET 0x0942

/* TODO: Change appearance to Headset when bidir communication is implemented */
#define APPEARANCE APPEARANCE_EARBUDS

LOG_MODULE_REGISTER(main, CONFIG_MAIN_LOG_LEVEL);

K_SEM_DEFINE(gapm_init_sem, 0, 1);

struct app_env {
	uint8_t actv_idx;
	uint8_t conidx;
};

static struct app_env app_env = {
	.actv_idx = GAP_INVALID_ACTV_IDX,
	.conidx = GAP_INVALID_CONIDX,
};

struct app_con_bond_data {
	gapc_pairing_keys_t keys;
	gapc_bond_data_t bond_data;
};

static struct app_con_bond_data app_con_bond_data = {
	.bond_data = {
		.local_csrk.key = {0xbb, 0x2c, 0xdf, 0x2a, 0x37, 0x3b, 0x6b, 0x65, 0x9, 0xb4, 0x7c,
				   0xcd, 0x28, 0xa2, 0x54, 0xa3},
		.pairing_lvl = GAP_PAIRING_BOND_UNAUTH,
		.cli_info = GAPC_CLI_SVC_CHANGED_IND_EN_BIT,
		.srv_feat = GAPC_SRV_EATT_SUPPORTED_BIT,
	}};

/* Load names from configuration file */
const char device_name[] = CONFIG_BLE_DEVICE_NAME;

static const gap_sec_key_t gapm_irk = {.key = {0xA1, 0xB2, 0xC3, 0xD5, 0xE6, 0xF7, 0x08, 0x09, 0x12,
					       0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89}};

/* ---------------------------------------------------------------------------------------- */
/* Bluetooth GAPM callbacks */

static void request_paring_and_bonding(uint8_t const conidx)
{
	/* Verify bond */
	if (gapc_is_bonded(conidx)) {
		LOG_INF("PARING: Peer device bonded");
	}

	gapc_pairing_t const pairing_info = {
#if BONDING_ENABLED
		.auth = GAP_AUTH_REQ_SEC_CON_BOND,
		.iocap = GAP_IO_CAP_KB_DISPLAY,
#else
		.auth = GAP_AUTH_SEC_CON,
		.iocap = GAP_IO_CAP_NO_INPUT_NO_OUTPUT,
#endif
		.oob = GAP_OOB_AUTH_DATA_NOT_PRESENT,
		.key_size = GAP_KEY_LEN,
		.ikey_dist = GAP_KDIST_ENCKEY | GAP_KDIST_IDKEY,
		.rkey_dist = GAP_KDIST_ENCKEY | GAP_KDIST_IDKEY,
	};

	if (GAP_ERR_NO_ERROR != gapc_le_bond(conidx, &pairing_info, 0)) {
		LOG_ERR("Unable to bond!");
	}

#if BONDING_ENABLED
	LOG_DBG("Pairing and bonding initiated...");
#else
	LOG_DBG("Pairing initiated...");
#endif
}

static void on_get_peer_version_cmp_cb(uint8_t const conidx, uint32_t const metainfo,
				       uint16_t const status, const gapc_version_t *const p_version)
{
	if (GAP_ERR_NO_ERROR != status) {
		LOG_ERR("Peer version fetch failed! err:%u", status);
		return;
	}
	LOG_INF("Peer version (conidx = %u) - company_id:%u, lmp_subversion:%u, lmp_version:%u",
		conidx, p_version->company_id, p_version->lmp_subversion, p_version->lmp_version);

	request_paring_and_bonding(conidx);
}

static void on_peer_features_cmp_cb(uint8_t const conidx, uint32_t const metainfo, uint16_t status,
				    const uint8_t *const p_features)
{
	__ASSERT(status == GAP_ERR_NO_ERROR, "GAPC get peer features failed! status:%u", status);

	LOG_DBG("Peer features (conidx = %d) - %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X", conidx,
		p_features[0], p_features[1], p_features[2], p_features[3], p_features[4],
		p_features[5], p_features[6], p_features[7]);

	status = gapc_get_peer_version(conidx, 0, on_get_peer_version_cmp_cb);
	if (GAP_ERR_NO_ERROR != status) {
		LOG_ERR("Unable to get peer version! err:%u", status);
	}
}

static void connection_confirm_not_bonded(uint_fast8_t const conidx)
{
	uint_fast16_t status;

	status = gapc_le_connection_cfm(conidx, 0, NULL);
	if (GAP_ERR_NO_ERROR != status) {
		LOG_ERR("LE connection confirmation failed! err:%u", status);
	}
	status = gapc_le_get_peer_features(conidx, 0, on_peer_features_cmp_cb);
	if (GAP_ERR_NO_ERROR != status) {
		LOG_ERR("Unable to get peer features! err:%u", status);
	}
}

static void on_address_resolved_cb(uint16_t status, const gap_addr_t *const p_addr,
				   const gap_sec_key_t *const pirk)
{
	uint_fast8_t const conidx = app_env.conidx;
	/* Check whether the peer device is bonded */
	bool const resolved = (status == GAP_ERR_NO_ERROR);

	LOG_INF("Address resolve ready! status:%u, %s peer device", status,
		resolved ? "KNOWN" : "UNKNOWN");

	if (resolved) {
		gapc_le_connection_cfm(conidx, 0, &app_con_bond_data.bond_data);
		request_paring_and_bonding(conidx);
		return;
	}
	connection_confirm_not_bonded(conidx);
}

static void on_le_connection_req(uint8_t const conidx, uint32_t const metainfo,
				 uint8_t const actv_idx, uint8_t const role,
				 const gap_bdaddr_t *const p_peer_addr,
				 const gapc_le_con_param_t *const p_con_params,
				 uint8_t const clk_accuracy)
{
	app_env.conidx = conidx;

	/* Number of IRKs */
	uint8_t nb_irk = 1;
	/* Resolve Address */
	uint16_t const status =
		gapm_le_resolve_address((gap_addr_t *)p_peer_addr->addr, nb_irk,
					&app_con_bond_data.keys.irk.key, on_address_resolved_cb);

	if (GAP_ERR_INVALID_PARAM == status) {
		/* Address not resolvable, just confirm the connection */
		connection_confirm_not_bonded(conidx);
	} else if (GAP_ERR_NO_ERROR != status) {
		LOG_ERR("Unable to start resolve address! err:%u", status);
	}

	LOG_INF("Connection request. conidx:%u (actv_idx:%u), role %s", conidx, actv_idx,
		role ? "PERIPH" : "CENTRAL");

	LOG_DBG("Connection parameters: interval %fms, latency %u, supervision timeout %ums, "
		"clk_accuracy:%u",
		(p_con_params->interval * 1.25), p_con_params->latency,
		(uint32_t)p_con_params->sup_to * 10, clk_accuracy);

	LOG_DBG("LE peer address: %s 0x%02X:%02X:%02X:%02X:%02X:%02X",
		(p_peer_addr->addr_type == GAP_ADDR_PUBLIC) ? "Public" : "Private",
		p_peer_addr->addr[5], p_peer_addr->addr[4], p_peer_addr->addr[3],
		p_peer_addr->addr[2], p_peer_addr->addr[1], p_peer_addr->addr[0]);
}

static const struct gapc_connection_req_cb gapc_con_cbs = {
	.le_connection_req = on_le_connection_req,
};

static void on_gapc_le_encrypt_req(uint8_t conidx, uint32_t metainfo, uint16_t ediv,
				   const gap_le_random_nb_t *p_rand)
{
	if (conidx != app_env.conidx) {
		LOG_ERR("LE encrypt request. Invalid connection id!");
		return;
	}
	gapc_le_encrypt_req_reply(conidx, true, &app_con_bond_data.keys.ltk.key,
				  app_con_bond_data.keys.ltk.key_size);
	LOG_DBG("LE encrypt request. conidx:%d", conidx);
}

static void on_gapc_sec_auth_info(uint8_t const conidx, uint32_t const metainfo,
				  uint8_t const sec_lvl, bool const encrypted,
				  uint8_t const key_size)
{
	LOG_DBG("Security auth info conidx:%d, level:%d, encrypted:%s", conidx, sec_lvl,
		(encrypted ? "TRUE" : "FALSE"));
}

static void on_gapc_pairing_succeed(uint8_t const conidx, uint32_t const metainfo,
				    uint8_t const pairing_level, bool const enc_key_present,
				    uint8_t const key_type)
{
	if (conidx != app_env.conidx) {
		LOG_ERR("Pairing SUCCEED. Invalid connection id!");
		return;
	}
	LOG_DBG("Pairing SUCCEED. conidx:%d, pairing_level:%u", conidx, pairing_level);

	app_con_bond_data.bond_data.pairing_lvl = pairing_level;
	app_con_bond_data.bond_data.enc_key_present = enc_key_present;

	int err = storage_store(SETTINGS_NAME_BOND_DATA, &app_con_bond_data.bond_data,
				sizeof(app_con_bond_data.bond_data));

	if (err) {
		LOG_ERR("Failed to store bond data (err %d)", err);
	}

	/* Verify bond */
	if (gapc_is_bonded(conidx)) {
		LOG_INF("Peer device bonded");
	}

	unicast_source_discover(conidx);
}

static void on_gapc_pairing_failed(uint8_t const conidx, uint32_t const metainfo,
				   uint16_t const reason)
{
	LOG_ERR("Pairing FAILED %d, 0x%04X", conidx, reason);
}

static void on_gapc_info_req(uint8_t const conidx, uint32_t const metainfo, uint8_t const exp_info)
{
	uint16_t err;

	switch (exp_info) {
	case GAPC_INFO_IRK: {
		err = gapc_le_pairing_provide_irk(conidx, &gapm_irk);
		if (GAP_ERR_NO_ERROR != err) {
			LOG_ERR("IRK send failed");
		} else {
			LOG_INF("IRK sent successful");
		}
		break;
	}
	case GAPC_INFO_CSRK: {
		err = gapc_pairing_provide_csrk(conidx, &app_con_bond_data.bond_data.local_csrk);
		if (GAP_ERR_NO_ERROR != err) {
			LOG_ERR("CSRK send failed");
		} else {
			LOG_INF("CSRK sent successful");
		}
		break;
	}
	case GAPC_INFO_PASSKEY_ENTERED:
	case GAPC_INFO_PASSKEY_DISPLAYED: {
		err = gapc_pairing_provide_passkey(conidx, true, 123456);
		if (GAP_ERR_NO_ERROR != err) {
			LOG_ERR("Unable to provide PASSKEY! err: 0x%02x", err);
		} else {
			LOG_INF("PASSKEY 123456 provided");
		}
		break;
	}
	default:
		LOG_WRN("Unsupported info %u requested!", exp_info);
		break;
	}
}

static void encrypt_gapc_proc_cmp_cb(uint8_t const conidx, uint32_t const metainfo,
				     uint16_t const status)
{
	if (GAP_ERR_NO_ERROR != status) {
		LOG_ERR("connection encryp failed!");
		return;
	}
	LOG_INF("Connection Encrypted!");
}

static void on_gapc_auth_req(uint8_t const conidx, uint32_t const metainfo,
			     uint8_t const auth_level)
{
	/* Called when peripheral requests security [gapc_le_request_security()]... */
	LOG_DBG("on_gapc_auth_req on conidx:%u, level:%u", conidx, auth_level);

	uint16_t const status =
		gapc_le_encrypt(conidx, 0, &app_con_bond_data.keys.ltk, encrypt_gapc_proc_cmp_cb);

	if (GAP_ERR_NO_ERROR != status) {
		LOG_ERR("GAPC LE encrypt failed! error: %u", status);
	}
}

static void on_gapc_pairing_req(uint8_t const conidx, uint32_t const metainfo,
				uint8_t const auth_level)
{
	LOG_DBG("Pairing request. conidx:%d, auth_level:%u", conidx, auth_level);

	gapc_pairing_t const pairing_info = {
		.auth = GAP_AUTH_SEC_CON,
		.ikey_dist = GAP_KDIST_ENCKEY | GAP_KDIST_IDKEY,
		.iocap = GAP_IO_CAP_NO_INPUT_NO_OUTPUT,
		.key_size = GAP_KEY_LEN,
		.oob = GAP_OOB_AUTH_DATA_NOT_PRESENT,
		.rkey_dist = GAP_KDIST_ENCKEY | GAP_KDIST_IDKEY,
	};
	uint16_t const status = gapc_le_pairing_accept(conidx, true, &pairing_info, 0);

	if (GAP_ERR_NO_ERROR != status) {
		LOG_ERR("Pairing accept failed! error: %u", status);
	}
}

static void on_gapc_sec_numeric_compare_req(uint8_t const conidx, uint32_t const metainfo,
					    uint32_t const value)
{
	LOG_DBG("Pairing numeric compare. conidx:%u, value:%u", conidx, value);
	/* Automatically confirm */
	gapc_pairing_numeric_compare_rsp(conidx, true);
}

static void generate_ltk(gapc_ltk_t *const p_ltk)
{
	if (!p_ltk) {
		LOG_ERR("Invalid LTK pointer!");
		return;
	}
	if (app_con_bond_data.keys.valid_key_bf & GAP_KDIST_ENCKEY) {
		return;
	}
	p_ltk->key_size = GAP_KEY_LEN;
	p_ltk->ediv = (uint16_t)co_rand_word();

	uint8_t cnt;

	cnt = sizeof(p_ltk->randnb.nb);
	while (cnt--) {
		p_ltk->randnb.nb[cnt] = (uint8_t)co_rand_word();
	}
	cnt = sizeof(p_ltk->key.key);
	while (cnt--) {
		p_ltk->key.key[cnt] = (uint8_t)co_rand_word();
	}

	app_con_bond_data.keys.valid_key_bf |= GAP_KDIST_ENCKEY;
	app_con_bond_data.keys.pairing_lvl = GAP_PAIRING_BOND_UNAUTH;

	LOG_INF("LTK generated");
}

static void on_gapc_sec_ltk_req(uint8_t const conidx, uint32_t const metainfo,
				uint8_t const key_size)
{
	LOG_DBG("LTK request. conidx:%u, key_size:%u", conidx, key_size);

	gapc_ltk_t *const p_ltk = &app_con_bond_data.keys.ltk;
	uint16_t status;

	generate_ltk(p_ltk);

	status = gapc_le_pairing_provide_ltk(conidx, p_ltk);
	if (GAP_ERR_NO_ERROR != status) {
		LOG_ERR("Unable to provide LTK! err %u", status);
		return;
	}
}

static void on_key_received(uint8_t conidx, uint32_t metainfo, const gapc_pairing_keys_t *p_keys)
{
	LOG_DBG("Key received. conidx:%u, key_bf:%u, level:%u", conidx, p_keys->valid_key_bf,
		p_keys->pairing_lvl);

	if (conidx != app_env.conidx) {
		LOG_ERR("Invalid connection id!");
		return;
	}
	int err;
	gapc_pairing_keys_t *const p_appkeys = &app_con_bond_data.keys;
	uint8_t key_bits = GAP_KDIST_NONE;
	uint8_t const valid_key_bf = p_keys->valid_key_bf;

	if (valid_key_bf & GAP_KDIST_ENCKEY) {
		memcpy(&p_appkeys->ltk, &p_keys->ltk, sizeof(p_appkeys->ltk));
		key_bits |= GAP_KDIST_ENCKEY;
		LOG_INF("LTK received");
	}

	if (valid_key_bf & GAP_KDIST_IDKEY) {
		memcpy(&p_appkeys->irk, &p_keys->irk, sizeof(p_appkeys->irk));
		key_bits |= GAP_KDIST_IDKEY;
		LOG_INF("IRK received");
	}

	if (valid_key_bf & GAP_KDIST_SIGNKEY) {
		memcpy(&p_appkeys->csrk, &p_keys->csrk, sizeof(p_appkeys->csrk));
		key_bits |= GAP_KDIST_SIGNKEY;
		LOG_INF("CSRK received");
	}

	p_appkeys->pairing_lvl = p_keys->pairing_lvl;
	p_appkeys->valid_key_bf = key_bits;

	err = storage_store(SETTINGS_NAME_KEYS, &app_con_bond_data.keys,
			    sizeof(app_con_bond_data.keys));
	if (err) {
		LOG_ERR("Failed to store test_data (err %d)", err);
	}
#if 0
	LOG_DBG("Key received. conidx:%u, key_bf:%u, level:%u", conidx,
		app_con_bond_data.keys.valid_key_bf, app_con_bond_data.keys.pairing_lvl);

	LOG_DBG("BOND Address : 0x%02X:%02X:%02X:%02X:%02X:%02X Valid_Key 0x%02X  Pairing_lvl = "
		"0x%02X",
		p_keys->irk.identity.addr.addr[5], p_keys->irk.identity.addr.addr[4],
		p_keys->irk.identity.addr.addr[3], p_keys->irk.identity.addr.addr[2],
		p_keys->irk.identity.addr.addr[1], p_keys->irk.identity.addr.addr[0],
		app_con_bond_data.keys.valid_key_bf, app_con_bond_data.keys.pairing_lvl);
#endif
}

static void on_repeated_attempt(uint8_t const conidx, uint32_t const metainfo)
{
	LOG_DBG("Repeat attempt...");
}

static const gapc_security_cb_t gapc_sec_cbs = {
	.le_encrypt_req = on_gapc_le_encrypt_req,

	.auth_info = on_gapc_sec_auth_info,
	.pairing_succeed = on_gapc_pairing_succeed,
	.pairing_failed = on_gapc_pairing_failed,
	.info_req = on_gapc_info_req,
	.auth_req = on_gapc_auth_req,
	.pairing_req = on_gapc_pairing_req,
	.numeric_compare_req = on_gapc_sec_numeric_compare_req,
	.ltk_req = on_gapc_sec_ltk_req,
	.key_received = on_key_received,
	.repeated_attempt = on_repeated_attempt,
	/* All other callbacks in this struct are optional */
};

static void on_disconnection(uint8_t const conidx, uint32_t const metainfo, uint16_t const reason)
{
	LOG_INF("Client %u disconnected for reason %u", conidx, reason);
}

static void on_bond_data_updated(uint8_t const conidx, uint32_t const metainfo,
				 const gapc_bond_data_updated_t *const p_data)
{
	LOG_INF("Connection %u bond data updated: gatt_start_hdl:%u, gatt_end_hdl:%u, "
		"svc_chg_hdl:%u, cli_info:%u, cli_feat:%u, srv_feat:%u",
		conidx, p_data->gatt_start_hdl, p_data->gatt_end_hdl, p_data->svc_chg_hdl,
		p_data->cli_info, p_data->cli_feat, p_data->srv_feat);
}

static void on_name_get(uint8_t const conidx, uint32_t const metainfo, uint16_t const token,
			uint16_t const offset, uint16_t const max_len)
{
	const size_t device_name_len = sizeof(device_name) - 1;
	const size_t short_len = (device_name_len > max_len ? max_len : device_name_len);

	gapc_le_get_name_cfm(conidx, token, GAP_ERR_NO_ERROR, device_name_len, short_len,
			     (const uint8_t *)device_name);
}

static void on_appearance_get(uint8_t conidx, uint32_t metainfo, uint16_t token)
{
	gapc_le_get_appearance_cfm(conidx, token, GAP_ERR_NO_ERROR, APPEARANCE);
}

static const gapc_connection_info_cb_t gapc_inf_cbs = {
	.disconnected = on_disconnection,
	.bond_data_updated = on_bond_data_updated,
	.name_get = on_name_get,
	.appearance_get = on_appearance_get,
	/* Other callbacks in this struct are optional */
};

static const gapc_le_config_cb_t gapc_le_cfg_cbs = {0};

#if !CONFIG_ALIF_BLE_ROM_IMAGE_V1_0 /* ROM version > 1.0 */
static void on_gapm_err(uint32_t metainfo, uint8_t code)
{
	LOG_ERR("GAPM error %d", code);
}

static const gapm_cb_t gapm_err_cbs = {
	.cb_hw_error = on_gapm_err,
};

#else /* ROM version 1.0 */
static void on_gapm_err(enum co_error err)
{
	LOG_ERR("GAPM error %d", err);
}

static const gapm_err_info_config_cb_t gapm_err_cbs = {
	.ctrl_hw_error = on_gapm_err,
};

#endif

static const gapm_callbacks_t gapm_cbs = {
	.p_con_req_cbs = &gapc_con_cbs,
	.p_sec_cbs = &gapc_sec_cbs,
	.p_info_cbs = &gapc_inf_cbs,
	.p_le_config_cbs = &gapc_le_cfg_cbs,
	.p_bt_config_cbs = NULL,    /* BT classic so not required */
#if !CONFIG_ALIF_BLE_ROM_IMAGE_V1_0 /* ROM version > 1.0 */
	.p_gapm_cbs = &gapm_err_cbs,
#else
	.p_err_info_config_cbs = &gapm_err_cbs,
#endif
};

/* ---------------------------------------------------------------------------------------- */
/**
 * BLE config (GAPM)
 */

static void on_gapm_process_complete(uint32_t const metainfo, uint16_t const status)
{
	if (GAP_ERR_NO_ERROR != status) {
		LOG_ERR("GAPM process completed with error %u", status);
		return;
	}

	switch (metainfo) {
	case 1:
		LOG_INF("GAPM configured successfully");
		break;
	case 2:
		LOG_INF("GAPM name set successfully");
		break;
	default:
		LOG_ERR("GAPM Unknown metadata!");
		return;
	}

	k_sem_give(&gapm_init_sem);
}

static void on_gapm_le_random_addr_cb(uint16_t const status, const gap_addr_t *const p_addr)
{
	if (GAP_ERR_NO_ERROR != status) {
		LOG_ERR("GAPM address generation error %u", status);
		return;
	}

	LOG_DBG("Generated resolvable random address: %02X:%02X:%02X:%02X:%02X:%02X",
		p_addr->addr[5], p_addr->addr[4], p_addr->addr[3], p_addr->addr[2], p_addr->addr[1],
		p_addr->addr[0]);

	k_sem_give(&gapm_init_sem);
}

static int ble_stack_configure(uint8_t const role)
{
	LOG_DBG("Configuring GAPM with BLE role %s",
		GAP_ROLE_LE_CENTRAL == role ? "CENTRAL" : "PERIPHERAL");

	/** Set Private Static Resolvable Address
	 * Note: This is auto generated using gapm_le_generate_random_addr() and must be aligned to
	 * IRK (@ref gapm_irk)
	 */
	gap_addr_t const address = {
		.addr = {0xE8, 0xEB, 0x9D, 0x44, 0x6B, 0x67},
	};
	/* Bluetooth stack configuration*/
	gapm_config_t gapm_cfg = {
		.role = role,
		.pairing_mode = GAPM_PAIRING_MODE_ALL,
		.privacy_cfg = GAPM_PRIV_CFG_PRIV_EN_BIT | GAPM_PRIV_CFG_PRIV_ADDR_BIT,
		.renew_dur = 1500,
		.private_identity = address,
		.irk = gapm_irk,
		.gap_start_hdl = 0,
		.gatt_start_hdl = 0,
		.att_cfg = 0,
		.sugg_max_tx_octets = GAP_LE_MAX_OCTETS,
		.sugg_max_tx_time = GAP_LE_MAX_TIME,
		.tx_pref_phy = GAP_PHY_ANY,
		.rx_pref_phy = GAP_PHY_ANY,
		.tx_path_comp = 0,
		.rx_path_comp = 0,
		/* BT Classic - not used */
		.class_of_device = 0x200408,
		.dflt_link_policy = 0,
	};
	int err;

	err = gapm_configure(1, &gapm_cfg, &gapm_cbs, on_gapm_process_complete);
	if (GAP_ERR_NO_ERROR != err) {
		LOG_ERR("gapm_configure error %u", err);
		return -1;
	}
	if (k_sem_take(&gapm_init_sem, K_MSEC(1000)) != 0) {
		LOG_ERR("  FAIL! GAPM config timeout!");
		return -1;
	}

	/* Generate resolvable random address */
	err = gapm_le_generate_random_addr(GAP_BD_ADDR_RSLV, on_gapm_le_random_addr_cb);
	if (GAP_ERR_NO_ERROR != err) {
		LOG_ERR("gapm_le_generate_random_addr error %u", err);
		return -1;
	}
	if (k_sem_take(&gapm_init_sem, K_MSEC(1000)) != 0) {
		LOG_ERR("  FAIL! GAPM random address timeout!");
		return -1;
	}

	LOG_INF("Set name: %s", device_name);
	err = gapm_set_name(2, strlen(device_name), (const uint8_t *)device_name,
			    on_gapm_process_complete);
	if (GAP_ERR_NO_ERROR != err) {
		LOG_ERR("gapm_set_name error %u", err);
		return -1;
	}
	if (k_sem_take(&gapm_init_sem, K_MSEC(1000)) != 0) {
		LOG_ERR("  FAIL! GAPM name set timeout!");
		return -1;
	}

#if BONDING_ENABLED
	/* Configure security level */
	gapm_le_configure_security_level(GAP_SEC1_AUTH_PAIR_ENC);
#else
	gapm_le_configure_security_level(GAP_SEC1_NOAUTH_PAIR_ENC);
#endif

	gap_bdaddr_t identity;

	gapm_get_identity(&identity);

	LOG_DBG("Device address: %02X:%02X:%02X:%02X:%02X:%02X", identity.addr[5], identity.addr[4],
		identity.addr[3], identity.addr[2], identity.addr[1], identity.addr[0]);

	LOG_DBG("BLE init complete!");

	return 0;
}

/* ---------------------------------------------------------------------------------------- */
/* LE Connection */

static void on_gapm_le_init_proc_cmp(uint32_t const token, uint8_t const proc_id,
				     uint8_t const actv_idx, uint16_t const status)
{
	ASSERT_INFO(status == GAP_ERR_NO_ERROR, status, 0);

	switch (proc_id) {
	case GAPM_ACTV_START: {
		LOG_INF("GAPM activity started. ID:%u", actv_idx);
		break;
	}
	case GAPM_ACTV_DELETE: {
		LOG_INF("GAPM activity deleted. ID:%u", actv_idx);
		break;
	}
	default: {
		LOG_DBG("GAPM unknown activity. Type:%u, ID:%u", proc_id, actv_idx);
		break;
	}
	}
}

static void on_gapm_le_init_stopped(uint32_t const token, uint8_t const actv_idx,
				    uint16_t const reason)
{
	if (reason == GAP_ERR_NO_ERROR) {
		LOG_DBG("GAPM LE connection init ready");
		return;
	}
	LOG_ERR("GAPM LE connection init stopped. Reason: %u", reason);

	/* Activity context is not needed anymore */
	gapm_delete_activity(actv_idx);
	app_env.actv_idx = GAP_INVALID_ACTV_IDX;
}

void on_peer_name_received(uint32_t const metainfo, uint8_t const actv_idx,
			   const gap_bdaddr_t *const p_addr, uint16_t const name_len,
			   const uint8_t *const p_name)
{
	LOG_INF("Peer name received: %s", ((name_len && p_name) ? (char *)p_name : "Unknown"));
}

static const gapm_le_init_cb_actv_t cbs_gapm_le_init = {
	.hdr.actv.stopped = on_gapm_le_init_stopped,
	.hdr.actv.proc_cmp = on_gapm_le_init_proc_cmp,
	.hdr.addr_updated = NULL,
	.peer_name = on_peer_name_received,
};

int connect_to_device(const struct gap_bdaddr *p_peer_addr)
{
	if (!p_peer_addr && GAP_ADDR_RAND < p_peer_addr->addr_type) {
		LOG_ERR("Invalid peer address");
		return -EINVAL;
	}

	/* Scan interval (N * 0.625 ms) = 100ms
	 * Scan window (N * 0.625 ms) = 50ms
	 */
#if BLE_SUBRATING
	/* Use 15ms connection interval */
#define CONN_INTERVAL 12
#define CE_LEN_MIN    1
#define CE_LEN_MAX    1
#else
#define CONN_INTERVAL 48
#define CE_LEN_MIN    5
#define CE_LEN_MAX    10
#endif

	gapm_le_init_param_t const init_params = {
		.prop = (GAPM_INIT_PROP_1M_BIT | GAPM_INIT_PROP_2M_BIT),
		.conn_to = 100, /* Timeout in 10ms units = 1 second */
		.scan_param_1m = {
				.scan_intv = 160,
				.scan_wd = 80,
			},
		.conn_param_1m = {
				.conn_intv_min = CONN_INTERVAL,
				.conn_intv_max = CONN_INTERVAL,
				.conn_latency = 0,
				.supervision_to = 100,
				.ce_len_min = CE_LEN_MIN,
				.ce_len_max = CE_LEN_MAX,
			},
		.conn_param_2m = {
				.conn_intv_min = CONN_INTERVAL,
				.conn_intv_max = CONN_INTERVAL,
				.conn_latency = 0,
				.supervision_to = 100,
				.ce_len_min = CE_LEN_MIN,
				.ce_len_max = CE_LEN_MAX,
			},
		.conn_param_coded = {
				.conn_intv_min = CONN_INTERVAL,
				.conn_intv_max = CONN_INTERVAL,
				.conn_latency = 0,
				.supervision_to = 100,
				.ce_len_min = CE_LEN_MIN,
				.ce_len_max = CE_LEN_MAX,
			},
		.peer_addr = *p_peer_addr,
	};

	uint16_t err;

	if (GAP_INVALID_ACTV_IDX == app_env.actv_idx) {
		err = gapm_le_create_init(0, GAPM_STATIC_ADDR, &cbs_gapm_le_init,
					  &app_env.actv_idx);
		if (GAP_ERR_NO_ERROR != err) {
			LOG_ERR("gapm_le_create_init error %u", err);
			return -1;
		}
	}

	err = gapm_le_start_direct_connection(app_env.actv_idx, &init_params);
	if (GAP_ERR_NO_ERROR != err) {
		LOG_ERR("gapm_le_start_direct_connection error %u", err);
		return -1;
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------- */
/* Application entry point */

int main(void)
{
	if (storage_init() < 0) {
		return -1;
	}
	storage_load(SETTINGS_NAME_KEYS, &app_con_bond_data.keys, sizeof(app_con_bond_data.keys));
	storage_load(SETTINGS_NAME_BOND_DATA, &app_con_bond_data.bond_data,
		     sizeof(app_con_bond_data.bond_data));

	int ret = alif_ble_enable(NULL);

	if (ret) {
		LOG_ERR("Failed to enable bluetooth, err %d", ret);
		return ret;
	}

	LOG_DBG("BLE enabled");

	if (0 != ble_stack_configure(GAP_ROLE_LE_CENTRAL)) {
		return -1;
	}

	/* Check LTK exists */
	generate_ltk(&app_con_bond_data.keys.ltk);

	ret = unicast_source_configure();
	if (ret != 0) {
		return ret;
	}

	ret = unicast_source_scan_start();
	if (ret != 0) {
		return ret;
	}

	while (1) {
		/* Nothing to do here... just sleep to keep app running */
		k_sleep(K_SECONDS(1));
	}

	return 0;
}

/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */
#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>
#include <zephyr/sys/__assert.h>
#include "alif_ble.h"
#include "gapm.h"
#include "gapm_le.h"
#include "gapc.h"
#include "gapc_le.h"
#include "gapc_sec.h"
#include "unicast_sink.h"

/**
 * Enable for bonding support
 * FIXME: issue with stereo streaming if enabled!
 */
#define BONDING_ENABLED 1

#define SETTINGS_BASE           "uc_periph"
#define SETTINGS_NAME_KEYS      "bond_keys_0"
#define SETTINGS_NAME_BOND_DATA "bond_data_0"

LOG_MODULE_REGISTER(main, CONFIG_MAIN_LOG_LEVEL);

K_SEM_DEFINE(gapm_init_sem, 0, 1);

struct connection_status {
	gap_bdaddr_t addr;       /*!< Peer device address */
	uint8_t conidx;          /*!< connection index */
};

static struct connection_status app_con_info = {
	.conidx = GAP_INVALID_CONIDX,
};

struct app_con_bond_data {
	gapc_pairing_keys_t keys;
	gapc_pairing_keys_t generated_keys;
	gapc_bond_data_t bond_data;
};

static struct app_con_bond_data app_con_bond_data;

/* Load names from configuration file */
const char device_name[] = CONFIG_BLE_DEVICE_NAME;

static const gap_sec_key_t gapm_irk = {.key = {0xA1, 0xB2, 0xC3, 0xD4, 0xE5, 0xF6, 0x07, 0x08, 0x11,
					       0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88}};

/* ---------------------------------------------------------------------------------------- */
/* Settings NVM storage handlers */

struct storage_ctx {
	uint8_t *p_output;
	size_t size;
};

static int settings_direct_loader(const char *const key, size_t const len,
				  settings_read_cb const read_cb, void *cb_arg, void *param)
{
	struct storage_ctx *p_ctx = (struct storage_ctx *)param;

	/* Handle the subtree if it is an exact key match. */
	if (settings_name_next(key, NULL) == 0) {
		ssize_t const cb_len = read_cb(cb_arg, p_ctx->p_output, p_ctx->size);

		if (cb_len != p_ctx->size) {
			LOG_ERR("Unable to read bytes_written from storage");
			return cb_len;
		}
	}

	return 0;
}

static int storage_load(const char *key, void *data, size_t const size)
{
	struct storage_ctx ctx = {
		.p_output = data,
		.size = size,
	};

	char key_str[64];

	snprintf(key_str, sizeof(key_str), SETTINGS_BASE "/%s", key);

	int err = settings_load_subtree_direct(key_str, settings_direct_loader, &ctx);

	if (err) {
		LOG_ERR("Failed to load %s (err %d)", key, err);
	}
	return err;
}

static int storage_save(const char *key, void *data, size_t const size)
{
	char key_str[64];

	snprintf(key_str, sizeof(key_str), SETTINGS_BASE "/%s", key);

	int err = settings_save_one(key_str, data, size);

	if (err) {
		LOG_ERR("Failed to store %s (err %d)", key, err);
	}
	return err;
}

static int storage_load_bond_data(void)
{
	int err = settings_subsys_init();

	if (err) {
		LOG_ERR("settings_subsys_init() failed (err %d)", err);
		return err;
	}

	storage_load(SETTINGS_NAME_KEYS, &app_con_bond_data.keys, sizeof(app_con_bond_data.keys));
	storage_load(SETTINGS_NAME_BOND_DATA, &app_con_bond_data.bond_data,
		     sizeof(app_con_bond_data.bond_data));

	LOG_INF("Settings loaded successfully");
	return 0;
}

/* ---------------------------------------------------------------------------------------- */
/* Bluetooth GAPM callbacks */

static void on_get_peer_version_cmp_cb(uint8_t const conidx, uint32_t const metainfo,
				       uint16_t const status, const gapc_version_t *const p_version)
{
	if (GAP_ERR_NO_ERROR != status) {
		LOG_ERR("Peer version fetch failed! err:%u", status);
		return;
	}
	LOG_INF("Peer version (conidx = %u) - company_id:%u, lmp_subversion:%u, lmp_version:%u",
		conidx, p_version->company_id, p_version->lmp_subversion, p_version->lmp_version);
}

static void on_peer_features_cmp_cb(uint8_t const conidx, uint32_t const metainfo, uint16_t status,
				    const uint8_t *const p_features)
{
	if (GAP_ERR_NO_ERROR != status) {
		LOG_ERR("Peer features fetch failed! err:%u", status);
		return;
	}
	LOG_INF("Peer features (conidx = %u) - %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X", conidx,
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
	uint_fast8_t const conidx = app_con_info.conidx;
	/* Check whether the peer device is bonded */
	bool const resolved = (status == GAP_ERR_NO_ERROR);

	LOG_INF("Address resolve ready! status:%u, %s peer device", status,
		resolved ? "KNOWN" : "UNKNOWN");

	if (resolved) {
		gapc_le_connection_cfm(conidx, 0, &app_con_bond_data.bond_data);
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
	app_con_info.conidx = conidx;

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

	LOG_DBG("LE connection created: %s 0x%02X:%02X:%02X:%02X:%02X:%02X",
		(p_peer_addr->addr_type == GAP_ADDR_PUBLIC) ? "Public" : "Private",
		p_peer_addr->addr[5], p_peer_addr->addr[4], p_peer_addr->addr[3],
		p_peer_addr->addr[2], p_peer_addr->addr[1], p_peer_addr->addr[0]);
}

static const struct gapc_connection_req_cb gapc_con_cbs = {
	.le_connection_req = on_le_connection_req,
};

static void on_gapc_le_encrypt_req(uint8_t const conidx, uint32_t const metainfo,
				   uint16_t const ediv, const gap_le_random_nb_t *const p_rand)
{
	bool const accepted = conidx == app_con_info.conidx;

	if (!accepted) {
		LOG_ERR("Invalid LE encrypt request. Invalid connection id!");
	}

	uint16_t const status =
		gapc_le_encrypt_req_reply(conidx, accepted, &app_con_bond_data.keys.ltk.key,
					  app_con_bond_data.keys.ltk.key_size);

	if (GAP_ERR_NO_ERROR != status) {
		LOG_ERR("LE encryption reply failed! err:%u", status);
		return;
	}
	LOG_DBG("LE connection %u encryption reply successful", conidx);
}

static void on_gapc_sec_auth_info(uint8_t const conidx, uint32_t const metainfo,
				  uint8_t const sec_lvl, bool const encrypted,
				  uint8_t const key_size)
{
	LOG_INF("Link security info. conidx:%u, level:%u, encrypted:%s, key_size:%u", conidx,
		sec_lvl, (encrypted ? "TRUE" : "FALSE"), key_size);
}

static void on_gapc_pairing_succeed(uint8_t const conidx, uint32_t const metainfo,
				    uint8_t const pairing_level, bool const enc_key_present,
				    uint8_t const key_type)
{
	if (conidx != app_con_info.conidx) {
		LOG_ERR("Pairing SUCCEED but invalid connection id!");
		return;
	}
	LOG_INF("Pairing SUCCEED. conidx:%d, pairing_level:%u, enc_key_present:%u, key_type:%u",
		conidx, pairing_level, enc_key_present, key_type);

	app_con_bond_data.bond_data.pairing_lvl = pairing_level;
	app_con_bond_data.bond_data.enc_key_present = enc_key_present;

	storage_save(SETTINGS_NAME_BOND_DATA, &app_con_bond_data.bond_data,
		     sizeof(app_con_bond_data.bond_data));

	/* Verify bond */
	if (gapc_is_bonded(conidx)) {
		LOG_INF("  Peer device is bonded");
	}

	if (enc_key_present) {
		/* TODO: is this needed to enable security? */
#if 0
		/* Request encryption from central */
		gapc_le_request_security(conidx, GAP_AUTH_SEC_CON);
#endif
	}
}

static void on_gapc_pairing_failed(uint8_t const conidx, uint32_t const metainfo,
				   uint16_t const reason)
{
	LOG_ERR("Pairing FAILED %d, 0x%04X", conidx, reason);
	app_con_info.conidx = GAP_INVALID_CONIDX;
}

static void on_gapc_info_req(uint8_t const conidx, uint32_t const metainfo, uint8_t const exp_info)
{
	uint16_t err;

	switch (exp_info) {
	case GAPC_INFO_IRK: {
		/* IRK exchange if bonding enabled */
		err = gapc_le_pairing_provide_irk(conidx, &gapm_irk);
		if (err) {
			LOG_ERR("IRK send failed");
		} else {
			LOG_INF("IRK sent successful");
		}
		break;
	}
	case GAPC_INFO_CSRK: {
		/* CSRK exchange if bonding enabled */
		err = gapc_pairing_provide_csrk(conidx, &app_con_bond_data.bond_data.local_csrk);
		if (err) {
			LOG_ERR("CSRK send failed");
		} else {
			LOG_INF("CSRK sent successful");
		}
		break;
	}
	case GAPC_INFO_BT_PASSKEY:
	case GAPC_INFO_PASSKEY_DISPLAYED: {
		err = gapc_pairing_provide_passkey(conidx, true, 123456);
		if (err) {
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

static void on_gapc_auth_req(uint8_t const conidx, uint32_t const metainfo,
			     uint8_t const auth_level)
{
	/* Should not be called for peripheral */
	LOG_ERR("Auth request on conidx:%u, level:%u", conidx, auth_level);
}

static void on_gapc_pairing_req(uint8_t const conidx, uint32_t const metainfo,
				uint8_t const auth_level)
{
	LOG_DBG("Pairing requested. conidx:%d, auth_level:%u", conidx, auth_level);

	gapc_pairing_t pairing_info = {
		.auth = GAP_AUTH_REQ_NO_MITM_NO_BOND,
		.iocap = GAP_IO_CAP_NO_INPUT_NO_OUTPUT,
		.ikey_dist = GAP_KDIST_ENCKEY | GAP_KDIST_IDKEY,
		.key_size = GAP_KEY_LEN,
		.oob = GAP_OOB_AUTH_DATA_NOT_PRESENT,
		.rkey_dist = GAP_KDIST_ENCKEY | GAP_KDIST_IDKEY,
	};

#if BONDING_ENABLED
	if (auth_level & GAP_AUTH_BOND) {
		pairing_info.auth = GAP_AUTH_REQ_SEC_CON_BOND;
		pairing_info.iocap = GAP_IO_CAP_DISPLAY_ONLY;
	} else
#endif
	if (auth_level & GAP_AUTH_SEC_CON) {
		pairing_info.auth = GAP_AUTH_SEC_CON;
	}

	uint16_t const status = gapc_le_pairing_accept(conidx, true, &pairing_info, 0);

	if (GAP_ERR_NO_ERROR != status) {
		LOG_ERR("Pairing accept failed! error: %u", status);
	}
}

static void on_gapc_sec_numeric_compare_req(uint8_t const conidx, uint32_t const metainfo,
					    uint32_t const value)
{
	LOG_DBG("Pairing numeric compare. conidx:%u, value:%u", conidx, value);
	gapc_pairing_numeric_compare_rsp(conidx, true);
}

static void on_gapc_sec_ltk_req(uint8_t const conidx, uint32_t const metainfo,
				uint8_t const key_size)
{
	LOG_DBG("LTK request. conidx:%u, key_size:%u", conidx, key_size);
	if (conidx != app_con_info.conidx) {
		LOG_ERR("Invalid connection id!");
		return;
	}

	gapc_ltk_t *const p_ltk_data = &app_con_bond_data.generated_keys.ltk;

	if (!(app_con_bond_data.generated_keys.valid_key_bf & GAP_KDIST_ENCKEY)) {
		p_ltk_data->key_size = GAP_KEY_LEN;
		p_ltk_data->ediv = (uint16_t)co_rand_word();

		uint8_t cnt;

		cnt = sizeof(p_ltk_data->randnb.nb);
		while (cnt--) {
			p_ltk_data->randnb.nb[cnt] = (uint8_t)co_rand_word();
		}
		cnt = sizeof(p_ltk_data->key.key);
		while (cnt--) {
			p_ltk_data->key.key[cnt] = (uint8_t)co_rand_word();
		}
		LOG_DBG("LTK generated");
	}

	uint16_t const err = gapc_le_pairing_provide_ltk(conidx, p_ltk_data);

	if (GAP_ERR_NO_ERROR != err) {
		LOG_ERR("Providing LTK failed! error: %u", err);
	}

	/* Distributed Encryption key */
	app_con_bond_data.generated_keys.valid_key_bf |= GAP_KDIST_ENCKEY;
	/* Peer device bonded through authenticated pairing */
	app_con_bond_data.generated_keys.pairing_lvl = GAP_PAIRING_BOND_AUTH;
}

static void on_key_received(uint8_t conidx, uint32_t metainfo, const gapc_pairing_keys_t *p_keys)
{
	if (conidx != app_con_info.conidx) {
		LOG_ERR("Invalid connection id!");
		return;
	}

	gapc_pairing_keys_t *const p_appkeys = &app_con_bond_data.keys;
	uint8_t key_bits = GAP_KDIST_NONE;

	if (p_keys->valid_key_bf & GAP_KDIST_ENCKEY) {
		memcpy(&p_appkeys->ltk, &p_keys->ltk, sizeof(p_appkeys->ltk));
		key_bits |= GAP_KDIST_ENCKEY;
	}

	if (p_keys->valid_key_bf & GAP_KDIST_IDKEY) {
		memcpy(&p_appkeys->irk, &p_keys->irk, sizeof(p_appkeys->irk));
		key_bits |= GAP_KDIST_IDKEY;
	}

	if (p_keys->valid_key_bf & GAP_KDIST_SIGNKEY) {
		memcpy(&p_appkeys->csrk, &p_keys->csrk, sizeof(p_appkeys->csrk));
		key_bits |= GAP_KDIST_SIGNKEY;
	}

	p_appkeys->pairing_lvl = p_keys->pairing_lvl;
	p_appkeys->valid_key_bf = key_bits;

	storage_save(SETTINGS_NAME_KEYS, &app_con_bond_data.keys, sizeof(app_con_bond_data.keys));

	LOG_DBG("Key received. conidx:%u, key_bf:%u, level:%u", conidx,
		app_con_bond_data.keys.valid_key_bf, app_con_bond_data.keys.pairing_lvl);
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

	app_con_info.conidx = GAP_INVALID_CONIDX;
	/* Restart advertising... */
	unicast_sink_adv_start();
}

static void on_bond_data_updated(uint8_t const conidx, uint32_t const metainfo,
				 const gapc_bond_data_updated_t *const p_data)
{
	LOG_INF("Connection %u bond data updated: gatt_start_hdl:%u, gatt_end_hdl:%u, "
		"svc_chg_hdl:%u, cli_info:%u, cli_feat:%u, srv_feat:%u",
		conidx, p_data->gatt_start_hdl, p_data->gatt_end_hdl, p_data->svc_chg_hdl,
		p_data->cli_info, p_data->cli_feat, p_data->srv_feat);

	/* Ignore for now since not needed */
#if 0
	app_con_bond_data.bond_data.local_sign_counter = p_data->local_sign_counter;
	app_con_bond_data.bond_data.remote_sign_counter = p_data->peer_sign_counter;
	app_con_bond_data.bond_data.gatt_start_hdl = p_data->gatt_start_hdl;
	app_con_bond_data.bond_data.gatt_end_hdl = p_data->gatt_end_hdl;
	app_con_bond_data.bond_data.svc_chg_hdl = p_data->svc_chg_hdl;
	app_con_bond_data.bond_data.cli_info = p_data->cli_info;
	app_con_bond_data.bond_data.cli_feat = p_data->cli_feat;
	app_con_bond_data.bond_data.srv_feat = p_data->srv_feat;

	storage_save(SETTINGS_NAME_BOND_DATA, &app_con_bond_data.bond_data,
		     sizeof(app_con_bond_data.bond_data));
#endif
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
	/* Send 'Earbuds' appearance */
	gapc_le_get_appearance_cfm(conidx, token, GAP_ERR_NO_ERROR, 0x0941);
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
/* BLE config (GAPM) */

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

static int ble_stack_configure(uint8_t const role)
{
	LOG_DBG("Configuring GAPM with BLE role %s",
		GAP_ROLE_LE_CENTRAL == role ? "CENTRAL" : "PERIPHERAL");

	gap_addr_t private_address = {
		.addr = {0xCF, 'L', 'E', 'U', 'D', 0x0}, /* LE UniDir */
	};

	/* Set Random Static Address */
	if (GAP_ROLE_LE_ALL == role) {
		private_address.addr[5] = 0x06;
	} else if (GAP_ROLE_LE_CENTRAL == role) {
		private_address.addr[5] = 0x07;
	} else {
		private_address.addr[5] = 0x08;
	}

	/* Bluetooth stack configuration*/
	gapm_config_t gapm_cfg = {
		.role = role,
		.pairing_mode = GAPM_PAIRING_MODE_ALL,
		.privacy_cfg = GAPM_PRIV_CFG_PRIV_ADDR_BIT | GAPM_PRIV_CFG_PRIV_EN_BIT,
		.renew_dur = 1500,
		.private_identity = private_address,
		.irk = gapm_irk,
		.gap_start_hdl = 0,
		.gatt_start_hdl = 0,
		.att_cfg = 0,
		.sugg_max_tx_octets = GAP_LE_MAX_OCTETS,
		.sugg_max_tx_time = GAP_LE_MAX_TIME,
		.tx_pref_phy = GAP_PHY_LE_2MBPS,
		.rx_pref_phy = GAP_PHY_LE_2MBPS,
		.tx_path_comp = 0,
		.rx_path_comp = 0,
		/* BT classic - not used */
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
#endif

	gap_bdaddr_t identity;

	gapm_get_identity(&identity);

	LOG_DBG("Device address: %02X:%02X:%02X:%02X:%02X:%02X", identity.addr[5], identity.addr[4],
		identity.addr[3], identity.addr[2], identity.addr[1], identity.addr[0]);

	LOG_DBG("BLE init complete!");

	return 0;
}

/* ---------------------------------------------------------------------------------------- */
/* Application entry point */

int main(void)
{
	if (storage_load_bond_data() < 0) {
		return -1;
	}

	int ret = alif_ble_enable(NULL);

	if (ret) {
		LOG_ERR("Failed to enable bluetooth, err %d", ret);
		return ret;
	}

	LOG_DBG("BLE enabled");

	if (0 != ble_stack_configure(GAP_ROLE_LE_PERIPHERAL)) {
		return -1;
	}

	if (0 != unicast_sink_init()) {
		return -1;
	}

	if (0 != unicast_sink_adv_start()) {
		return -1;
	}

	while (1) {
		/* Nothing to do here... just sleep to keep app running */
		k_sleep(K_SECONDS(5));
	}

	return 0;
}

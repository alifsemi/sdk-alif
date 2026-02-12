/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
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
#include "gapm.h"
#include "gap_le.h"
#include "gapc_le.h"
#include "gapc_sec.h"
#include "gapm_le.h"
#include "gapm_le_adv.h"
#include "co_buf.h"
#include "prf.h"
#include "gatt_db.h"
#include "gatt_srv.h"
#include "ke_mem.h"
#include "gapm_sec.h"
#include "ble_storage.h"

LOG_MODULE_REGISTER(sec, LOG_LEVEL_DBG);

static gapc_pairing_keys_t stored_keys;
static gapc_pairing_keys_t generated_keys;
static gapc_bond_data_t bond_data_saved;
static bool sec_enabled;
static uint8_t temp_conidx;
static uint32_t temp_metainfo;
static pairing_status_cb sec_pairing_status_cb;
static gap_sec_key_t sec_irk;
static gap_bdaddr_t *temp_gap_adr;

/* Security callbacks */
static void on_key_received(uint8_t conidx, uint32_t metainfo, const gapc_pairing_keys_t *p_keys)
{
	int err;

	stored_keys.csrk = p_keys->csrk;
	memcpy(stored_keys.irk.key.key, p_keys->irk.key.key, sizeof(stored_keys.irk.key.key));
	memcpy(stored_keys.irk.identity.addr, p_keys->irk.identity.addr,
	       sizeof(stored_keys.irk.identity.addr));

	stored_keys.irk.identity.addr_type = p_keys->irk.identity.addr_type;
	stored_keys.ltk = p_keys->ltk;
	stored_keys.pairing_lvl = p_keys->pairing_lvl;
	stored_keys.valid_key_bf = p_keys->valid_key_bf;
	if (IS_ENABLED(CONFIG_SETTINGS)) {

		err = ble_storage_save_with_index(BLE_BOND_KEYS_NAME, 0, &stored_keys,
						  sizeof(gapc_pairing_keys_t));
		if (err) {
			LOG_ERR("Failed to store test_data (err %d)", err);
		}
	}
	LOG_INF("Key received");
}

static void on_pairing_req(uint8_t conidx, uint32_t metainfo, uint8_t auth_level)
{
	uint16_t err;

	gapc_pairing_t pairing_info = {
		.auth = GAP_AUTH_REQ_NO_MITM_NO_BOND,
		.iocap = GAP_IO_CAP_NO_INPUT_NO_OUTPUT,
		.ikey_dist = GAP_KDIST_ENCKEY | GAP_KDIST_IDKEY,
		.key_size = GAP_KEY_LEN,
		.oob = GAP_OOB_AUTH_DATA_NOT_PRESENT,
		.rkey_dist = GAP_KDIST_ENCKEY | GAP_KDIST_IDKEY,
	};

	if (auth_level & GAP_AUTH_BOND) {
		pairing_info.auth = GAP_AUTH_REQ_SEC_CON_BOND;
		pairing_info.iocap = GAP_IO_CAP_DISPLAY_ONLY;
	} else if (auth_level & GAP_AUTH_SEC_CON) {
		pairing_info.auth = GAP_AUTH_SEC_CON;
	}

	LOG_INF("pairing req %u , level %u", conidx, auth_level);

	err = gapc_le_pairing_accept(conidx, true, &pairing_info, 0);

	if (err != GAP_ERR_NO_ERROR) {
		LOG_ERR("Pairing error %u", err);
	}
}

static void on_pairing_failed(uint8_t conidx, uint32_t metainfo, uint16_t reason)
{
	LOG_INF("Pairing failed conidx: %u, metainfo: %u, reason: 0x%02x\n", conidx, metainfo,
		reason);
	sec_pairing_status_cb(reason, conidx, false);
}

static void on_le_encrypt_req(uint8_t conidx, uint32_t metainfo, uint16_t ediv,
			      const gap_le_random_nb_t *p_rand)
{
	uint16_t err;

	err = gapc_le_encrypt_req_reply(conidx, true, &stored_keys.ltk.key,
					stored_keys.ltk.key_size);

	if (err) {
		LOG_ERR("Error during encrypt request reply %u", err);
	}
}

static void on_pairing_succeed(uint8_t conidx, uint32_t metainfo, uint8_t pairing_level,
			       bool enc_key_present, uint8_t key_type)
{
	int err;

	LOG_INF("PAIRING SUCCEED %u level, %u key present, %u key type", pairing_level,
		enc_key_present, key_type);

	bond_data_saved.pairing_lvl = pairing_level;
	bond_data_saved.enc_key_present = true;
	if (IS_ENABLED(CONFIG_SETTINGS)) {
		err = ble_storage_save_with_index(BLE_BOND_DATA_NAME, 0, &bond_data_saved,
						  sizeof(gapc_bond_data_t));
		if (err) {
			LOG_ERR("Failed to store test_data (err %d)", err);
		}

		/* Store Peer connection information */
		if (temp_gap_adr) {
			err = ble_storage_save_with_index(BLE_BOND_PEER_NAME, 0, temp_gap_adr,
							  sizeof(gap_bdaddr_t));
			if (err) {
				LOG_ERR("Failed to store test_data (err %d)", err);
			}
		}
	}

	/* Verify bond */
	bool bonded = gapc_is_bonded(conidx);

	if (bonded) {
		LOG_INF("Peer device bonded");
	}
	sec_pairing_status_cb(GAP_ERR_NO_ERROR, conidx, false);
}

void gapm_sec_bondata_update(uint8_t conidx, uint32_t metainfo,
			     const gapc_bond_data_updated_t *p_data)
{

	bond_data_saved.local_sign_counter = p_data->local_sign_counter;
	bond_data_saved.remote_sign_counter = p_data->peer_sign_counter;
	bond_data_saved.gatt_start_hdl = p_data->gatt_start_hdl;
	bond_data_saved.gatt_end_hdl = p_data->gatt_end_hdl;
	bond_data_saved.svc_chg_hdl = p_data->svc_chg_hdl;
	bond_data_saved.cli_info = p_data->cli_info;
	bond_data_saved.cli_feat = p_data->cli_feat;
	bond_data_saved.srv_feat = p_data->srv_feat;

	if (IS_ENABLED(CONFIG_SETTINGS)) {
		int err = ble_storage_save_with_index(BLE_BOND_DATA_NAME, 0, &bond_data_saved,
						      sizeof(gapc_bond_data_t));
		if (err) {
			LOG_ERR("Failed to store test_data (err %d)", err);
		}
	}
}

static void on_info_req(uint8_t conidx, uint32_t metainfo, uint8_t exp_info)
{
	uint16_t err;

	switch (exp_info) {
	case GAPC_INFO_IRK: {
		err = gapc_le_pairing_provide_irk(conidx, &sec_irk);
		if (err) {
			LOG_ERR("IRK send failed");
		} else {
			LOG_INF("IRK sent successful");
		}
	} break;

	case GAPC_INFO_CSRK: {
		/* CSRK exchange if bonding enabled */
		err = gapc_pairing_provide_csrk(conidx, &bond_data_saved.local_csrk);
		if (err) {
			LOG_ERR("Client %u CSRK provide failed. err: %u", conidx, err);
			break;
		}
		LOG_INF("Client %u CSRK sent successful", conidx);
		break;
	}

	case GAPC_INFO_BT_PASSKEY:
	case GAPC_INFO_PASSKEY_DISPLAYED: {
		err = gapc_pairing_provide_passkey(conidx, true, 123456);
		if (err) {
			LOG_ERR("ERROR PROVIDING PASSKEY 0x%02x", err);
		} else {
			LOG_INF("PASSKEY 123456");
		}
	} break;

	default:
		LOG_WRN("Requested info 0x%02x", exp_info);
		break;
	}
}

static void on_ltk_req(uint8_t conidx, uint32_t metainfo, uint8_t key_size)
{
	uint16_t err;
	uint8_t cnt;

	gapc_ltk_t *ltk_data = &(generated_keys.ltk);

	ltk_data->key_size = GAP_KEY_LEN;
	ltk_data->ediv = (uint16_t)co_rand_word();

	for (cnt = 0; cnt < RAND_NB_LEN; cnt++) {
		ltk_data->key.key[cnt] = (uint8_t)co_rand_word();
		ltk_data->randnb.nb[cnt] = (uint8_t)co_rand_word();
	}

	for (cnt = RAND_NB_LEN; cnt < GAP_KEY_LEN; cnt++) {
		ltk_data->key.key[cnt] = (uint8_t)co_rand_word();
	}

	err = gapc_le_pairing_provide_ltk(conidx, &generated_keys.ltk);

	if (err) {
		LOG_ERR("LTK provide error %u\n", err);
	} else {
		LOG_INF("LTK PROVIDED");
	}

	/* Distributed Encryption key */
	generated_keys.valid_key_bf |= GAP_KDIST_ENCKEY;

	/* Peer device bonded through authenticated pairing */
	generated_keys.pairing_lvl = GAP_PAIRING_BOND_AUTH;
}

static void on_numeric_compare_req(uint8_t conidx, uint32_t metainfo, uint32_t numeric_value)
{
	LOG_DBG("PAIRING USER VAL CFM %d %d", conidx, numeric_value);
	/* Automatically confirm */
	gapc_pairing_numeric_compare_rsp(conidx, true);
}


static void on_key_pressed(uint8_t conidx, uint32_t metainfo, uint8_t notification_type)
{
}
static void on_repeated_attempt(uint8_t conidx, uint32_t metainfo)
{
}

static void on_auth_req(uint8_t conidx, uint32_t metainfo, uint8_t auth_level)
{
}

static void on_auth_info(uint8_t conidx, uint32_t metainfo, uint8_t sec_lvl,
				bool encrypted,
				uint8_t key_size)
{
	LOG_DBG("AUTH INFO %d, %d - %s", conidx, sec_lvl, (encrypted ? "TRUE" : "FALSE"));
}

static const gapc_security_cb_t gapc_sec_cbs = {
	.key_received = on_key_received,
	.pairing_req = on_pairing_req,
	.pairing_failed = on_pairing_failed,
	.le_encrypt_req = on_le_encrypt_req,
	.pairing_succeed = on_pairing_succeed,
	.info_req = on_info_req,
	.ltk_req = on_ltk_req,
	.numeric_compare_req = on_numeric_compare_req,
	/* All other callbacks in this struct are optional */
	.auth_req = on_auth_req,
	.auth_info = on_auth_info,
	.key_pressed = on_key_pressed,
	.repeated_attempt = on_repeated_attempt,
};

#ifdef CONFIG_SETTINGS
int gapc_keys_setting_storage_init(void)
{
	int err;

	err = ble_storage_init();
	if (err) {
		LOG_ERR("ble_storage_init() failed (err %d)", err);
		return err;
	}

	ble_storage_load_with_index(BLE_BOND_KEYS_NAME, 0, &stored_keys, sizeof(stored_keys));
	ble_storage_load_with_index(BLE_BOND_DATA_NAME, 0, &bond_data_saved,
				    sizeof(gapc_bond_data_t));

	return 0;
}
#else
int gapc_keys_setting_storage_init(void)
{
	return 0;
}
#endif

const gapc_security_cb_t *gapm_sec_cb_got(void)
{
	return &gapc_sec_cbs;
}

const gapc_security_cb_t *gapm_sec_init(bool security, pairing_status_cb pairing_cb,
					const gap_sec_key_t *irk)
{
	sec_enabled = security;

	sec_pairing_status_cb = pairing_cb;

	sec_irk = *irk;

	if (IS_ENABLED(CONFIG_SETTINGS)) {
		gapc_keys_setting_storage_init();
	}

	return &gapc_sec_cbs;
}

void gapm_sec_load_peer_address(gap_bdaddr_t *p_gap_adr)
{
	temp_gap_adr = p_gap_adr;

	if (IS_ENABLED(CONFIG_SETTINGS)) {
		if (temp_gap_adr) {
			int err = ble_storage_init();

			if (err) {
				LOG_ERR("ble_storage_init() failed (err %d)", err);
				return;
			}
			ble_storage_load(BLE_BOND_PEER_NAME, temp_gap_adr, sizeof(gap_bdaddr_t));
		}
	}
}

static void on_get_peer_version_cmp_cb(uint8_t const conidx, uint32_t const metainfo,
				       uint16_t const status, const gapc_version_t *const p_version)
{
	if (status != GAP_ERR_NO_ERROR) {
		LOG_ERR("Client %u Peer version fetch failed! err:%u", conidx, status);
		return;
	}
	LOG_INF("Client %u company_id:%u, lmp_subversion:%u, lmp_version:%u", conidx,
		p_version->company_id, p_version->lmp_subversion, p_version->lmp_version);
}

static void on_peer_features_cmp_cb(uint8_t const conidx, uint32_t const metainfo, uint16_t status,
				    const uint8_t *const p_features)
{
	if (status == GAP_ERR_NO_ERROR) {
		LOG_INF("Client %u features: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X", conidx,
			p_features[0], p_features[1], p_features[2], p_features[3], p_features[4],
			p_features[5], p_features[6], p_features[7]);
	} else {
		LOG_ERR("Client %u get peer features failed! status:%u", conidx, status);
	}

	status = gapc_get_peer_version(conidx, 0, on_get_peer_version_cmp_cb);
	if (status != GAP_ERR_NO_ERROR) {
		LOG_ERR("Client %u unable to get peer version! err:%u", conidx, status);
	}
}

static void on_address_resolved_cb(uint16_t status, const gap_addr_t *p_addr,
				   const gap_sec_key_t *pirk)
{
	bool resolved = (status != GAP_ERR_NO_ERROR) ? false : true;

	if (temp_gap_adr) {
		temp_gap_adr->addr_type = GAP_ADDR_RAND;
		memcpy(temp_gap_adr->addr, p_addr->addr, sizeof(p_addr->addr));
	}

	if (resolved) {
		LOG_INF("Known peer device");
		gapc_le_connection_cfm(temp_conidx, temp_metainfo, &(bond_data_saved));
		sec_pairing_status_cb(GAP_ERR_NO_ERROR, temp_conidx, true);
	} else {
		LOG_INF("Unknown peer device");
		gapc_le_connection_cfm(temp_conidx, temp_metainfo, NULL);

		status = gapc_le_get_peer_features(temp_conidx, 0, on_peer_features_cmp_cb);
		if (status != GAP_ERR_NO_ERROR) {
			LOG_ERR("Client %u Unable to get peer features! err:%u", temp_conidx,
				status);
		}
		sec_pairing_status_cb(GAP_ERR_NO_ERROR, temp_conidx, false);
	}
}

void gapm_connection_confirm(uint8_t conidx, uint32_t metainfo, const gap_bdaddr_t *p_peer_addr)
{
	uint16_t status;

	temp_conidx = conidx;
	temp_metainfo = metainfo;

	bool const public = (p_peer_addr->addr_type == GAP_ADDR_PUBLIC);

	LOG_DBG("  Peer address: %s %02X:%02X:%02X:%02X:%02X:%02X", public ? "Public" : "Private",
		p_peer_addr->addr[5], p_peer_addr->addr[4], p_peer_addr->addr[3],
		p_peer_addr->addr[2], p_peer_addr->addr[1], p_peer_addr->addr[0]);

	if (sec_enabled || !public) {
		/* Number of IRKs */
		uint8_t nb_irk = 1;

		/* Resolve Address */
		status = gapm_le_resolve_address((gap_addr_t *)p_peer_addr->addr, nb_irk,
						 &(stored_keys.irk.key), on_address_resolved_cb);
		if (status == GAP_ERR_NO_ERROR) {
			return;
		}
	}

	gapc_le_connection_cfm(conidx, metainfo, NULL);
	status = gapc_get_peer_version(conidx, 0, on_get_peer_version_cmp_cb);
	if (status != GAP_ERR_NO_ERROR) {
		LOG_ERR("Client %u unable to get peer version! err:%u", conidx, status);
	}
	sec_pairing_status_cb(GAP_ERR_NO_ERROR, conidx, false);
}

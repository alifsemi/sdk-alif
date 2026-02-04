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

LOG_MODULE_REGISTER(sec, LOG_LEVEL_DBG);

static gapc_pairing_keys_t stored_keys;
static gapc_pairing_keys_t generated_keys;
static gapc_bond_data_t bond_data_saved;
static bool sec_enabled;
static uint8_t temp_conidx;
static uint32_t temp_metainfo;
static pairing_status_cb sec_pairing_status_cb;
static gap_sec_key_t sec_irk;

static gapc_pairing_t p_pairing_info = {
	.auth = GAP_AUTH_BOND | GAP_AUTH_SEC_CON | GAP_AUTH_MITM,
	.ikey_dist = GAP_KDIST_ENCKEY | GAP_KDIST_IDKEY,
	.iocap = GAP_IO_CAP_DISPLAY_ONLY,
	.key_size = GAP_KEY_LEN,
	.oob = GAP_OOB_AUTH_DATA_NOT_PRESENT,
	.rkey_dist = GAP_KDIST_ENCKEY | GAP_KDIST_IDKEY,
};

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

		/* Save under the key "ble/bond_keys_0" */
		err = settings_save_one(BLE_BOND_KEYS_KEY_0, &stored_keys,
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

	LOG_INF("pairing req %u , level %u", conidx, auth_level);

	err = gapc_le_pairing_accept(conidx, true, &p_pairing_info, 0);

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
		err = settings_save_one(BLE_BOND_DATA_KEY_0, &bond_data_saved,
					sizeof(gapc_bond_data_t));
		if (err) {
			LOG_ERR("Failed to store test_data (err %d)", err);
		}
	}

	/* Verify bond */
	bool bonded = gapc_is_bonded(conidx);

	if (bonded) {
		LOG_INF("Peer device bonded");
	}
	sec_pairing_status_cb(GAP_ERR_NO_ERROR, conidx, false);
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

	case GAPC_INFO_PASSKEY_DISPLAYED:
		err = gapc_pairing_provide_passkey(conidx, true, 123456);
		if (err) {
			LOG_ERR("ERROR PROVIDING PASSKEY 0x%02x", err);
		} else {
			LOG_INF("PASSKEY 123456");
		}
		break;

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
static int keys_settings_set(const char *name, size_t len_rd, settings_read_cb read_cb,
			     void *cb_arg)
{
	int err;

	if (strcmp(name, BLE_BOND_KEYS_NAME_0) == 0) {

		if (len_rd != sizeof(stored_keys)) {
			LOG_ERR("Incorrect length for test_data: %zu", len_rd);
			return -EINVAL;
		}

		err = read_cb(cb_arg, &stored_keys, sizeof(gapc_pairing_keys_t));
		if (err < 0) {
			LOG_ERR("Failed to read test_data (err: %d)", err);
			return err;
		}

		return 0;
	} else if (strcmp(name, BLE_BOND_DATA_NAME_0) == 0) {

		if (len_rd != sizeof(bond_data_saved)) {
			LOG_ERR("Incorrect length for test_data: %zu", len_rd);
			return -EINVAL;
		}

		err = read_cb(cb_arg, &bond_data_saved, sizeof(gapc_bond_data_t));
		if (err < 0) {
			LOG_ERR("Failed to read test_data (err: %d)", err);
			return err;
		}

		return 0;
	}

	LOG_ERR("stored data not correct");
	return 0;
}

static struct settings_handler ble_cgms_conf = {
	.name = "ble",
	.h_set = keys_settings_set,
};

int gapc_keys_setting_storage_init(void)
{
	int err;

	err = settings_subsys_init();
	if (err) {
		LOG_ERR("settings_subsys_init() failed (err %d)", err);
		return err;
	}

	err = settings_register(&ble_cgms_conf);
	if (err) {
		LOG_ERR("Failed to register settings handler, err %d", err);
		return err;
	}

	err = settings_load();
	if (err) {
		LOG_ERR("settings_load() failed, err %d", err);
	}

	return err;
}
#else
int gapc_keys_setting_storage_init(void)
{
	return 0;
}
#endif

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

static void on_address_resolved_cb(uint16_t status, const gap_addr_t *p_addr,
				   const gap_sec_key_t *pirk)
{
	bool resolved = (status != GAP_ERR_NO_ERROR) ? false : true;

	if (resolved) {
		LOG_INF("Known peer device");
		gapc_le_connection_cfm(temp_conidx, temp_metainfo, &(bond_data_saved));
		sec_pairing_status_cb(GAP_ERR_NO_ERROR, temp_conidx, true);
	} else {
		LOG_INF("Unknown peer device");
		gapc_le_connection_cfm(temp_conidx, temp_metainfo, NULL);
		sec_pairing_status_cb(GAP_ERR_NO_ERROR, temp_conidx, false);
	}
}

void gapm_connection_confirm(uint8_t conidx, uint32_t metainfo, const gap_bdaddr_t *p_peer_addr)
{
	temp_conidx = conidx;
	temp_metainfo = metainfo;

	if (sec_enabled) {
		/* Number of IRKs */
		uint16_t status;
		uint8_t nb_irk = 1;

		/* Resolve Address */
		status = gapm_le_resolve_address((gap_addr_t *)p_peer_addr->addr, nb_irk,
						 &(stored_keys.irk.key), on_address_resolved_cb);
		if (status == GAP_ERR_NO_ERROR) {
			return;
		}
	}

	gapc_le_connection_cfm(conidx, metainfo, NULL);
	sec_pairing_status_cb(GAP_ERR_NO_ERROR, conidx, false);
}

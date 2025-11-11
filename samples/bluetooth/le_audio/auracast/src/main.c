/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>
#include <zephyr/sys/__assert.h>

#include <stdio.h>

#include <se_service.h>
#include <soc_common.h>

#include "alif_ble.h"
#include "gap_le.h"
#include "gapc_le.h"
#include "gapc_sec.h"
#include "gapm_le.h"

#include "main.h"
#include "auracast_source.h"
#include "auracast_sink.h"
#include "auracast_sd.h"

LOG_MODULE_REGISTER(main, CONFIG_MAIN_LOG_LEVEL);

enum gapm_meta {
	META_CONFIG = 1,
	META_SET_NAME = 2,
	META_RESET = 3,
};

static const gap_sec_key_t g_gapm_irk = {.key = {0xA1, 0xB2, 0xC3, 0xD4, 0xE5, 0xF6, 0x07, 0x08,
						 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88}};
static gap_addr_t g_private_address;

K_SEM_DEFINE(gapm_init_sem, 0, 1);

static enum role g_current_role = ROLE_NONE;
static char g_device_name[32];
static char g_stream_name[32];
static char g_auracast_encryption_passwd[GAP_KEY_LEN + 1];

#define SETTINGS_BASE           "auracast"
#define SETTINGS_NAME_KEYS      "bond_keys_0"
#define SETTINGS_NAME_BOND_DATA "bond_data_0"
#define SETTINGS_NAME_ADDR      "address"

struct connection_status {
	gap_bdaddr_t addr; /*!< Peer device address */
	uint8_t conidx;    /*!< connection index */
};

static struct connection_status app_con_info = {
	.conidx = GAP_INVALID_CONIDX,
	.addr.addr_type = 0xff,
};

#define BLE_THREAD_PRIORITY   1
#define BLE_THREAD_STACK_SIZE 2048

static struct k_thread ble_thread;
static K_THREAD_STACK_DEFINE(ble_stack, BLE_THREAD_STACK_SIZE);

/* ---------------------------------------------------------------------------------------- */
/* Settings NVM storage handlers */

struct app_con_bond_data {
	gapc_pairing_keys_t keys;
	gapc_bond_data_t bond_data;
};

static struct app_con_bond_data app_con_bond_data;

struct storage_ctx {
	uint8_t *p_output;
	size_t size;
};

static int settings_direct_loader(const char *const key, size_t const len,
				  settings_read_cb const read_cb, void *cb_arg, void *param)
{
	struct storage_ctx *p_ctx = (struct storage_ctx *)param;

	if (settings_name_next(key, NULL) == 0) {
		ssize_t const cb_len = read_cb(cb_arg, p_ctx->p_output, p_ctx->size);

		if (cb_len != (ssize_t)p_ctx->size) {
			LOG_ERR("Unable to read bytes_written from storage");
			return (int)cb_len;
		}
	}

	return 0;
}

static int storage_load(const char *key, void *data, size_t const size)
{
	struct storage_ctx ctx = {
		.p_output = (uint8_t *)data,
		.size = size,
	};

	char key_str[64];
	(void)snprintf(key_str, sizeof(key_str), SETTINGS_BASE "/%s", key);

	int err = settings_load_subtree_direct(key_str, settings_direct_loader, &ctx);

	if (err != 0) {
		LOG_ERR("Failed to load %s, err %d", key_str, err);
		return err;
	}

	return 0;
}

static int storage_save(const char *key, void *data, size_t const size)
{
	char key_str[64];
	(void)snprintf(key_str, sizeof(key_str), SETTINGS_BASE "/%s", key);

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

	err = storage_load(SETTINGS_NAME_ADDR, &g_private_address, sizeof(g_private_address));

	if (err != 0) {
		LOG_WRN("No private address found");
	}

	err = storage_load(SETTINGS_NAME_KEYS, &app_con_bond_data.keys,
			   sizeof(app_con_bond_data.keys));

	if (err != 0) {
		LOG_WRN("No bond keys found");
	}

	err = storage_load(SETTINGS_NAME_BOND_DATA, &app_con_bond_data.bond_data,
			   sizeof(app_con_bond_data.bond_data));

	if (err != 0) {
		LOG_WRN("No bond data found");
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------- */
/**
 * Bluetooth stack configuration
 */

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
	__ASSERT(status == GAP_ERR_NO_ERROR, "Client %u get peer features failed! status:%u",
		 conidx, status);

	LOG_INF("Client %u features: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X", conidx,
		p_features[0], p_features[1], p_features[2], p_features[3], p_features[4],
		p_features[5], p_features[6], p_features[7]);

	status = gapc_get_peer_version(conidx, 0, on_get_peer_version_cmp_cb);
	if (status != GAP_ERR_NO_ERROR) {
		LOG_ERR("Client %u unable to get peer version! err:%u", conidx, status);
	}
}

static void connection_confirm_not_bonded(uint_fast8_t const conidx)
{
	uint_fast16_t status;

	status = gapc_le_connection_cfm(conidx, 0, NULL);
	if (status != GAP_ERR_NO_ERROR) {
		LOG_ERR("Client %u connection confirmation failed! err:%u", conidx, status);
	}

	status = gapc_le_get_peer_features(conidx, 0, on_peer_features_cmp_cb);
	if (status != GAP_ERR_NO_ERROR) {
		LOG_ERR("Client %u Unable to get peer features! err:%u", conidx, status);
	}
}

static void on_address_resolved_cb(uint16_t status, const gap_addr_t *const p_addr,
				   const gap_sec_key_t *const pirk)
{
	uint_fast8_t const conidx = app_con_info.conidx;
	bool const resolved = (status == GAP_ERR_NO_ERROR);

	LOG_INF("Client %u address resolve ready! status:%u, %s peer device", conidx, status,
		resolved ? "KNOWN" : "UNKNOWN");

	memcpy(&app_con_info.addr, p_addr, sizeof(app_con_info.addr));

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
	memcpy(&app_con_info.addr, p_peer_addr, sizeof(app_con_info.addr));

	/* Number of IRKs */
	uint8_t nb_irk = 1;
	/* Resolve Address */
	uint16_t const status =
		gapm_le_resolve_address((gap_addr_t *)p_peer_addr->addr, nb_irk,
					&app_con_bond_data.keys.irk.key, on_address_resolved_cb);

	if (status == GAP_ERR_INVALID_PARAM) {
		/* Address not resolvable, just confirm the connection */
		connection_confirm_not_bonded(conidx);
	} else if (status != GAP_ERR_NO_ERROR) {
		LOG_ERR("Client %u Unable to start resolve address! err:%u", conidx, status);
	}

	LOG_INF("Connection request. conidx:%u (actv_idx:%u), role %s", conidx, actv_idx,
		role ? "PERIPH" : "CENTRAL");

	LOG_DBG("  interval %fms, latency %u, supervision timeout %ums, clk_accuracy:%u",
		(p_con_params->interval * 1.25), p_con_params->latency,
		(uint32_t)p_con_params->sup_to * 10, clk_accuracy);

	LOG_DBG("  Peer address: %s %02X:%02X:%02X:%02X:%02X:%02X",
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
	LOG_INF("Client %u LE encryption request received, ediv: 0x%04X", conidx, ediv);

	uint16_t const status = gapc_le_encrypt_req_reply(
		conidx, true, &app_con_bond_data.keys.ltk.key, app_con_bond_data.keys.ltk.key_size);

	if (status != GAP_ERR_NO_ERROR) {
		LOG_ERR("Client %u LE encryption reply failed! err:%u", conidx, status);
		return;
	}

	LOG_INF("Client %u LE encryption reply successful", conidx);
}

static void on_gapc_sec_auth_info(uint8_t const conidx, uint32_t const metainfo,
				  uint8_t const sec_lvl, bool const encrypted,
				  uint8_t const key_size)
{
	LOG_INF("Client %u Link security info. level:%u, encrypted:%s, key_size:%u", conidx,
		sec_lvl, (encrypted ? "TRUE" : "FALSE"), key_size);
}

static void on_gapc_pairing_succeed(uint8_t const conidx, uint32_t const metainfo,
				    uint8_t const pairing_level, bool const enc_key_present,
				    uint8_t const key_type)
{
	bool const bonded = gapc_is_bonded(conidx);

	LOG_INF("Client %u pairing SUCCEED. pairing_level:%u, bonded:%s", conidx, pairing_level,
		bonded ? "TRUE" : "FALSE");

	app_con_bond_data.bond_data.pairing_lvl = pairing_level;
	app_con_bond_data.bond_data.enc_key_present = enc_key_present;

	storage_save(SETTINGS_NAME_BOND_DATA, &app_con_bond_data.bond_data,
		     sizeof(app_con_bond_data.bond_data));
}

static void on_gapc_pairing_failed(uint8_t const conidx, uint32_t const metainfo,
				   uint16_t const reason)
{
	LOG_ERR("Client %u pairing failed, reason: 0x%04X", conidx, reason);
	app_con_info.conidx = GAP_INVALID_CONIDX;
}

static void on_gapc_info_req(uint8_t const conidx, uint32_t const metainfo, uint8_t const exp_info)
{
	uint16_t err;

	switch (exp_info) {
	case GAPC_INFO_IRK: {
		err = gapc_le_pairing_provide_irk(conidx, &g_gapm_irk);
		if (err) {
			LOG_ERR("Client %u IRK provide failed. err: %u", conidx, err);
			break;
		}
		LOG_INF("Client %u IRK sent successful", conidx);
		break;
	}
	case GAPC_INFO_CSRK: {
		err = gapc_pairing_provide_csrk(conidx, &app_con_bond_data.bond_data.local_csrk);
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
			LOG_ERR("Client %u PASSKEY provide failed. err: %u", conidx, err);
			break;
		}
		LOG_INF("Client %u PASSKEY 123456 provided", conidx);
		break;
	}
	default:
		LOG_WRN("Client %u Unsupported info %u requested!", conidx, exp_info);
		break;
	}
}

static void on_gapc_pairing_req(uint8_t const conidx, uint32_t const metainfo,
				uint8_t const auth_level)
{
	LOG_DBG("Client %u pairing requested. auth_level:%u", conidx, auth_level);

	gapc_pairing_t pairing_info = {
		.auth = GAP_AUTH_REQ_SEC_CON_BOND,
		.iocap = GAP_IO_CAP_DISPLAY_ONLY,
		.ikey_dist = GAP_KDIST_ENCKEY | GAP_KDIST_IDKEY,
		.key_size = GAP_KEY_LEN,
		.oob = GAP_OOB_AUTH_DATA_NOT_PRESENT,
		.rkey_dist = GAP_KDIST_ENCKEY | GAP_KDIST_IDKEY,
	};

	if (auth_level & GAP_AUTH_SEC_CON) {
		pairing_info.auth = GAP_AUTH_REQ_SEC_CON_BOND;
	}

	uint16_t const status = gapc_le_pairing_accept(conidx, true, &pairing_info, 0);

	if (status != GAP_ERR_NO_ERROR) {
		LOG_ERR("Pairing accept failed! error: %u", status);
	}
}

static void on_gapc_sec_numeric_compare_req(uint8_t const conidx, uint32_t const metainfo,
					    uint32_t const value)
{
	LOG_INF("Client %u pairing - numeric compare. value:%u", conidx, value);
	gapc_pairing_numeric_compare_rsp(conidx, true);
}

static void on_key_received(uint8_t conidx, uint32_t metainfo, const gapc_pairing_keys_t *p_keys)
{
	LOG_INF("Client %u keys received: key_bf:0x%02X, level:%u", conidx, p_keys->valid_key_bf,
		p_keys->pairing_lvl);

	gapc_pairing_keys_t *const p_appkeys = &app_con_bond_data.keys;
	uint8_t key_bits = GAP_KDIST_NONE;

	if (p_keys->valid_key_bf & GAP_KDIST_ENCKEY) {
		memcpy(&p_appkeys->ltk, &p_keys->ltk, sizeof(p_appkeys->ltk));
		key_bits |= GAP_KDIST_ENCKEY;
		LOG_INF("Client %u LTK received and stored", conidx);
	}

	if (p_keys->valid_key_bf & GAP_KDIST_IDKEY) {
		memcpy(&p_appkeys->irk, &p_keys->irk, sizeof(p_appkeys->irk));
		key_bits |= GAP_KDIST_IDKEY;
		LOG_INF("Client %u IRK received and stored", conidx);
	}

	if (p_keys->valid_key_bf & GAP_KDIST_SIGNKEY) {
		memcpy(&p_appkeys->csrk, &p_keys->csrk, sizeof(p_appkeys->csrk));
		key_bits |= GAP_KDIST_SIGNKEY;
		LOG_INF("Client %u CSRK received and stored", conidx);
	}

	p_appkeys->pairing_lvl = p_keys->pairing_lvl;
	p_appkeys->valid_key_bf = key_bits;

	storage_save(SETTINGS_NAME_KEYS, &app_con_bond_data.keys, sizeof(app_con_bond_data.keys));
	LOG_INF("Client %u keys saved to storage", conidx);
}

static const gapc_security_cb_t gapc_sec_cbs = {
	.le_encrypt_req = on_gapc_le_encrypt_req,
	.auth_info = on_gapc_sec_auth_info,
	.pairing_succeed = on_gapc_pairing_succeed,
	.pairing_failed = on_gapc_pairing_failed,
	.info_req = on_gapc_info_req,
	.pairing_req = on_gapc_pairing_req,
	.numeric_compare_req = on_gapc_sec_numeric_compare_req,
	.key_received = on_key_received,
};

static void on_disconnection(uint8_t const conidx, uint32_t const metainfo, uint16_t const reason)
{
	LOG_INF("Client %u disconnected, reason: 0x%04X", conidx, reason);

	app_con_info.conidx = GAP_INVALID_CONIDX;

	/* Restart BASS solicitation */
	int err = auracast_scan_delegator_start_solicitation();

	if (err != 0) {
		LOG_ERR("Failed to restart BASS solicitation, err %d", err);
		return;
	}
	LOG_INF("BASS solicitation restarted after disconnection");
}

static void on_bond_data_updated(uint8_t const conidx, uint32_t const metainfo,
				 const gapc_bond_data_updated_t *const p_data)
{
	LOG_INF("Client %u bond data updated: gatt_start_hdl:%u, gatt_end_hdl:%u, "
		"svc_chg_hdl:%u, cli_info:%u, cli_feat:%u, srv_feat:%u",
		conidx, p_data->gatt_start_hdl, p_data->gatt_end_hdl, p_data->svc_chg_hdl,
		p_data->cli_info, p_data->cli_feat, p_data->srv_feat);
}

static void on_name_get(uint8_t const conidx, uint32_t const metainfo, uint16_t const token,
			uint16_t const offset, uint16_t const max_len)
{
	const size_t device_name_len = sizeof(g_device_name) - 1;
	const size_t short_len = (device_name_len > max_len ? max_len : device_name_len);

	gapc_le_get_name_cfm(conidx, token, GAP_ERR_NO_ERROR, device_name_len, short_len,
			     (const uint8_t *)g_device_name);
}

static void on_appearance_get(uint8_t const conidx, uint32_t const metainfo, uint16_t const token)
{
	gapc_le_get_appearance_cfm(conidx, token, GAP_ERR_NO_ERROR, APPEARANCE);
}

static const gapc_connection_info_cb_t gapc_inf_cbs = {
	.disconnected = on_disconnection,
	.bond_data_updated = on_bond_data_updated,
	.name_get = on_name_get,
	.appearance_get = on_appearance_get,
};

static const gapc_le_config_cb_t gapc_le_cfg_cbs = {0};

static void on_gapm_err(uint32_t metainfo, uint8_t code)
{
	LOG_ERR("GAPM error %d", code);
}

static const gapm_cb_t gapm_err_cbs = {
	.cb_hw_error = on_gapm_err,
};

/* For the broadcaster role, callbacks are not mandatory */
static const gapm_callbacks_t gapm_cbs = {
	.p_con_req_cbs = &gapc_con_cbs,
	.p_sec_cbs = &gapc_sec_cbs,
	.p_info_cbs = &gapc_inf_cbs,
	.p_le_config_cbs = &gapc_le_cfg_cbs,
	.p_bt_config_cbs = NULL,
	.p_gapm_cbs = &gapm_err_cbs,
};

static void on_gapm_process_complete(uint32_t metainfo, uint16_t status)
{
	if (status) {
		LOG_ERR("gapm process completed with error %u", status);
		return;
	}

	switch (metainfo) {
	case META_CONFIG:
		LOG_INF("GAPM configured successfully");
		break;

	case META_SET_NAME:
		LOG_INF("GAPM name set successfully");
		break;

	case META_RESET:
		LOG_INF("GAPM reset successfully");
		break;

	default:
		LOG_ERR("GAPM Unknown metadata!");
		return;
	}

	k_sem_give(&gapm_init_sem);
}

static void on_gapm_le_random_addr_cb(uint16_t const status, const gap_addr_t *const p_addr)
{
	if (status != GAP_ERR_NO_ERROR) {
		LOG_ERR("RPA generation error %u", status);
		k_sem_give(&gapm_init_sem);
		return;
	}

	LOG_DBG("Generated resolvable random address: %02X:%02X:%02X:%02X:%02X:%02X",
		p_addr->addr[5], p_addr->addr[4], p_addr->addr[3], p_addr->addr[2], p_addr->addr[1],
		p_addr->addr[0]);

	g_private_address = *p_addr;

	storage_save(SETTINGS_NAME_ADDR, &g_private_address, sizeof(g_private_address));

	k_sem_give(&gapm_init_sem);
}

static void update_default_device_name(void)
{
	snprintf((char *)g_stream_name, sizeof(g_stream_name), CONFIG_AURACAST_STREAM_NAME);
	snprintf((char *)g_auracast_encryption_passwd, sizeof(g_auracast_encryption_passwd),
		 CONFIG_AURACAST_ENCRYPTION_PASSWORD);
	snprintf((char *)g_device_name, sizeof(g_device_name), DEVICE_NAME_PREFIX_DEFAULT);
}

static int generate_private_address(void)
{
	int err;

	if (g_private_address.addr[5] != 0) {
		LOG_INF("Using stored private address: %02X:%02X:%02X:%02X:%02X:%02X",
			g_private_address.addr[5], g_private_address.addr[4],
			g_private_address.addr[3], g_private_address.addr[2],
			g_private_address.addr[1], g_private_address.addr[0]);
		return 0;
	}

	gapm_config_t gapm_cfg = {
		.role = GAP_ROLE_LE_PERIPHERAL,
		.pairing_mode = GAPM_PAIRING_DISABLE,
		.privacy_cfg = 0,
		.renew_dur = 1500,
		.private_identity = g_private_address, /* let controller handle RPA */
		.irk = g_gapm_irk,
		.gap_start_hdl = 0,
		.gatt_start_hdl = 0,
		.att_cfg = 0,
		.sugg_max_tx_octets = GAP_LE_MAX_OCTETS,
		.sugg_max_tx_time = GAP_LE_MIN_TIME,
		.tx_pref_phy = GAP_PHY_LE_2MBPS,
		.rx_pref_phy = GAP_PHY_LE_2MBPS,
		.tx_path_comp = 0,
		.rx_path_comp = 0,
	};

	/* Configure GAPM to prepare address generation */
	err = gapm_configure(META_CONFIG, &gapm_cfg, &gapm_cbs, on_gapm_process_complete);
	if (err != GAP_ERR_NO_ERROR) {
		LOG_ERR("gapm_configure error %u", err);
		return -EFAULT;
	}
	if (k_sem_take(&gapm_init_sem, K_SECONDS(1)) != 0) {
		LOG_ERR("  FAIL! GAPM config timeout!");
		return -ETIMEDOUT;
	}

	/* Generate random static address */
	err = gapm_le_generate_random_addr(GAP_BD_ADDR_STATIC, on_gapm_le_random_addr_cb);
	if (err != GAP_ERR_NO_ERROR) {
		LOG_ERR("gapm_le_generate_random_addr error %u", err);
		return -EFAULT;
	}
	if (k_sem_take(&gapm_init_sem, K_SECONDS(1)) != 0) {
		LOG_ERR("  FAIL! GAPM random address timeout!");
		return -ETIMEDOUT;
	}

	/* Reset GAPM to set address */
	err = gapm_reset(META_RESET, on_gapm_process_complete);
	if (err != GAP_ERR_NO_ERROR) {
		LOG_ERR("gapm_reset error %u", err);
		return -EFAULT;
	}
	if (k_sem_take(&gapm_init_sem, K_SECONDS(1)) != 0) {
		LOG_ERR("  FAIL! GAPM reset timeout!");
		return -ETIMEDOUT;
	}

	LOG_INF("Generated new private address: %02X:%02X:%02X:%02X:%02X:%02X",
		g_private_address.addr[5], g_private_address.addr[4], g_private_address.addr[3],
		g_private_address.addr[2], g_private_address.addr[1], g_private_address.addr[0]);

	return 0;
}

int configure_role(const enum role role)
{
	int err;

	if (role >= ROLE_MAX) {
		LOG_ERR("Invalid role %d", role);
		return -EINVAL;
	}

	if (role == g_current_role) {
		return -EALREADY;
	}

	if (role != g_current_role && g_current_role != ROLE_NONE) {

		switch (g_current_role) {
		case ROLE_AURACAST_SOURCE:
			auracast_source_stop();
			break;
		case ROLE_AURACAST_SINK:
			auracast_sink_stop();
			break;
		case ROLE_AURACAST_SCAN_DELEGATOR:
			auracast_scan_delegator_deinit();
			break;
		default:
			break;
		}

		/* Reset GAPM to set address */
		err = gapm_reset(META_RESET, on_gapm_process_complete);
		if (err != GAP_ERR_NO_ERROR) {
			LOG_ERR("gapm_reset error %u", err);
			return -EFAULT;
		}
		if (k_sem_take(&gapm_init_sem, K_MSEC(1000)) != 0) {
			LOG_ERR("  FAIL! GAPM reset timeout!");
			return -ETIMEDOUT;
		}
	}

	g_current_role = role;

	if (role == ROLE_NONE) {
		return 0;
	}

	/* Bluetooth stack configuration*/
	gapm_config_t gapm_cfg = {
		.role = 0,
		.pairing_mode = GAPM_PAIRING_DISABLE,
		.privacy_cfg = GAPM_PRIV_CFG_PRIV_ADDR_BIT,
		.renew_dur = 1500,
		.private_identity = g_private_address, /* let controller handle RPA */
		.irk = g_gapm_irk,
		.gap_start_hdl = 0,
		.gatt_start_hdl = 0,
		.att_cfg = 0,
		.sugg_max_tx_octets = GAP_LE_MAX_OCTETS,
		.sugg_max_tx_time = GAP_LE_MIN_TIME,
		.tx_pref_phy = GAP_PHY_LE_2MBPS,
		.rx_pref_phy = GAP_PHY_LE_2MBPS,
		.tx_path_comp = 0,
		.rx_path_comp = 0,
	};

	switch (role) {
	case ROLE_AURACAST_SOURCE:
		gapm_cfg.role = GAP_ROLE_LE_BROADCASTER;
		break;
	case ROLE_AURACAST_SINK:
		gapm_cfg.role = GAP_ROLE_LE_OBSERVER;
		break;
	case ROLE_AURACAST_SCAN_DELEGATOR:
		gapm_cfg.role = GAP_ROLE_LE_PERIPHERAL | GAP_ROLE_LE_OBSERVER;
		gapm_cfg.pairing_mode = GAPM_PAIRING_SEC_CON;
		break;
	default:
		LOG_ERR("Invalid role %d", role);
		return -1;
	}

	err = gapm_configure(META_CONFIG, &gapm_cfg, &gapm_cbs, on_gapm_process_complete);
	if (err != GAP_ERR_NO_ERROR) {
		LOG_ERR("gapm_configure error %u", err);
		return -EFAULT;
	}
	if (k_sem_take(&gapm_init_sem, K_SECONDS(1)) != 0) {
		LOG_ERR("  FAIL! GAPM config timeout!");
		return -ETIMEDOUT;
	}

	LOG_INF("Set name: %s", g_device_name);
	err = gapm_set_name(META_SET_NAME, strlen(g_device_name), (const uint8_t *)g_device_name,
			    on_gapm_process_complete);
	if (err != GAP_ERR_NO_ERROR) {
		LOG_ERR("gapm_set_name error %u", err);
		return -EFAULT;
	}
	if (k_sem_take(&gapm_init_sem, K_SECONDS(1)) != 0) {
		LOG_ERR("  FAIL! GAPM name set timeout!");
		return -ETIMEDOUT;
	}

	if (role == ROLE_AURACAST_SCAN_DELEGATOR) {
		LOG_INF("Configure security level");
		gapm_le_configure_security_level(GAP_SEC1_SEC_CON_PAIR_ENC);
	}

	return 0;
}

enum role get_current_role(void)
{
	return g_current_role;
}

int set_device_name(const char *name)
{
	if (!name) {
		LOG_ERR("invalid name pointer");
		return -EINVAL;
	}

	size_t const name_len = strlen(name);

	if (name_len == 0 || name_len > sizeof(g_device_name) - 1) {
		LOG_ERR("invalid name length %u", name_len);
		return -EINVAL;
	}

	strncpy(g_device_name, name, sizeof(g_device_name) - 1);
	g_device_name[name_len] = '\0';

	LOG_INF("Device name set to: %s", g_device_name);
	return 0;
}

const char *get_device_name(void)
{
	if (strlen(g_device_name) == 0) {
		return NULL;
	}

	return g_device_name;
}

int set_stream_name(const char *name)
{
	if (name == NULL) {
		/* just clear the name */
		g_stream_name[0] = '\0';
		return 0;
	}

	size_t const name_len = strlen(name);

	if (name_len == 0 || name_len > sizeof(g_stream_name) - 1) {
		return -EINVAL;
	}

	strncpy(g_stream_name, name, sizeof(g_stream_name) - 1);
	g_stream_name[name_len] = '\0';

	LOG_INF("Stream name set to: %s", g_stream_name);
	return 0;
}

const char *get_stream_name(void)
{
	if (strlen(g_stream_name) == 0) {
		return NULL;
	}

	return g_stream_name;
}

int set_auracast_encryption_passwd(const char *passwd)
{
	if (!passwd) {
		memset(g_auracast_encryption_passwd, 0, sizeof(g_auracast_encryption_passwd));
		LOG_INF("Auracast encryption disabled");
		return 0;
	}

	size_t const passwd_len = strlen(passwd);

	if (passwd_len < 4 || passwd_len > GAP_KEY_LEN) {
		LOG_ERR("Password is invalid (len %u), len must be 4..16!", passwd_len);
		return -EINVAL;
	}

	strncpy(g_auracast_encryption_passwd, passwd, GAP_KEY_LEN);
	g_auracast_encryption_passwd[passwd_len] = '\0';

	LOG_INF("Auracast encryption password set");
	return 0;
}

const char *get_auracast_encryption_passwd(void)
{
	if (strlen(g_auracast_encryption_passwd) == 0) {
		return NULL;
	}

	return g_auracast_encryption_passwd;
}

int fill_auracast_encryption_key(gaf_bcast_code_t *const p_code)
{
	if (!p_code) {
		return -EINVAL;
	}

	memset(p_code->bcast_code, 0, sizeof(p_code->bcast_code));

	const char *passwd = get_auracast_encryption_passwd();

	if (!passwd) {
		return 0;
	}

	size_t const passwd_len = strlen(passwd);

	/* Fill key buffer with password and pad with zeros */
	memcpy(p_code->bcast_code, passwd, passwd_len);

	return passwd_len;
}

K_QUEUE_DEFINE(ble_cmd_queue);

int execute_shell_command(struct startup_params msg)
{
	if (msg.cmd >= COMMAND_MAX) {
		LOG_ERR("Invalid command: %d", msg.cmd);
		return -EINVAL;
	}

	struct startup_params *p_msg = malloc(sizeof(*p_msg));

	if (!p_msg) {
		LOG_ERR("Failed to allocate memory for command");
		return -ENOMEM;
	}

	*p_msg = msg;

	k_queue_append(&ble_cmd_queue, p_msg);
	return 0;
}

static void ble_worker(void *p1, void *p2, void *p3)
{
	/* Start up bluetooth host stack */
	int ret = alif_ble_enable(NULL);

	if (ret) {
		LOG_ERR("Failed to enable bluetooth, err %d", ret);
		return;
	}

	LOG_DBG("BLE enabled");

	if (generate_private_address() < 0) {
		return;
	}

	LOG_INF("Type 'auracast help' to get started...");

	while (1) {
		struct startup_params *p_msg = k_queue_get(&ble_cmd_queue, K_FOREVER);

		if (!p_msg) {
			continue;
		}

		switch (p_msg->cmd) {
		case COMMAND_SOURCE:
			auracast_source_start(p_msg->source.octets_per_frame,
					      p_msg->source.frame_rate_hz,
					      p_msg->source.frame_duration_us);
			break;
		case COMMAND_SINK:
			auracast_sink_start();
			break;
		case COMMAND_SINK_SELECT_STREAM:
			auracast_sink_select_stream(p_msg->sink.stream_index);
			break;
		case COMMAND_SCAN_DELEGATOR:
			auracast_scan_delegator_init();
			break;
		case COMMAND_STOP:
			configure_role(ROLE_NONE);
			break;
		default:
			break;
		}

		free(p_msg);
	}
}

static int soc_run_profile(void)
{
#define HOST_SYSTOP_PWR_REQ_LOGIC_ON_MEM_ON 0x20

	const uint32_t host_bsys_pwr_req = sys_read32(HOST_BSYS_PWR_REQ);

	sys_write32(host_bsys_pwr_req | HOST_SYSTOP_PWR_REQ_LOGIC_ON_MEM_ON, HOST_BSYS_PWR_REQ);

	run_profile_t runp = {
		.power_domains = PD_VBAT_AON_MASK | PD_SYST_MASK | PD_SSE700_AON_MASK |
				 PD_DBSS_MASK | PD_SESS_MASK,
		.dcdc_voltage = 825,
		.dcdc_mode = DCDC_MODE_PFM_FORCED,
		.aon_clk_src = CLK_SRC_LFXO,
		.run_clk_src = CLK_SRC_PLL,
		.cpu_clk_freq = CLOCK_FREQUENCY_160MHZ,
		.phy_pwr_gating = LDO_PHY_MASK,
		.ip_clock_gating = LP_PERIPH_MASK,
		.vdd_ioflex_3V3 = IOFLEX_LEVEL_1V8,
		.scaled_clk_freq = SCALED_FREQ_XO_HIGH_DIV_38_4_MHZ,
		.memory_blocks = (MRAM_MASK | (SRAM2_MASK | SRAM3_MASK) |
				  (SERAM_1_MASK | SERAM_2_MASK | SERAM_3_MASK | SERAM_4_MASK) |
				  /* M55-HE ITCM */
				  (SRAM4_1_MASK | SRAM4_2_MASK | SRAM4_3_MASK | SRAM4_4_MASK) |
				  /* M55-HE DTCM */
				  (SRAM5_1_MASK | SRAM5_2_MASK | SRAM5_3_MASK | SRAM5_4_MASK |
				   SRAM5_5_MASK)),
	};

	if (se_service_set_run_cfg(&runp)) {
		LOG_ERR("run profile set failed!");
		return -ENOEXEC;
	}

	sys_write32(host_bsys_pwr_req, HOST_BSYS_PWR_REQ);
	sys_write32(0xFFFFFFFF, 0x1A60900C);

	return 0;
}
SYS_INIT(soc_run_profile, PRE_KERNEL_1, 3);

int main(void)
{
	k_queue_init(&ble_cmd_queue);

	update_default_device_name();

	if (storage_load_bond_data() < 0) {
		return -1;
	}

	k_thread_create(&ble_thread, ble_stack, K_THREAD_STACK_SIZEOF(ble_stack), ble_worker, NULL,
			NULL, NULL, BLE_THREAD_PRIORITY, 0, K_NO_WAIT);

	return 0;
}

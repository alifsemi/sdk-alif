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
#include <stdlib.h>
#include "alif_ble.h"
#include "gapm.h"
#include "gapm_le_init.h"
#include "gap_le.h"
#include "gapc_le.h"
#include "gapc_sec.h"
#include "gapm_le.h"
#include "gapm_le_adv.h"
#include "co_buf.h"

#include "prf.h"
#include "gapm_api.h"
#include "rwip_task.h"
#include "storage.h"
#include "app_csisc.h"
#include "app_gaf_scan.h"

LOG_MODULE_REGISTER(main, CONFIG_MAIN_LOG_LEVEL);

#define APPEARANCE (0x0000)

const char device_name[] = CONFIG_BLE_DEVICE_NAME;

static const gap_sec_key_t gapm_irk = {.key = {0xA1, 0xB2, 0xC3, 0xD5, 0xE6, 0xF7, 0x08, 0x09, 0x12,
					       0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89}};

/**
 * Bluetooth stack configuration
 */

/* Load name from configuration file */
#define DEVICE_NAME CONFIG_BLE_DEVICE_NAME

K_SEM_DEFINE(central_state_sem, 0, 1);
K_SEM_DEFINE(wait_procedure_sem, 0, 1);
K_SEM_DEFINE(wait_connection_sem, 0, 1);
K_SEM_DEFINE(wait_disc_sem, 0, 1);
K_SEM_DEFINE(wait_stop_sem, 0, 1);

static uint16_t conn_status;

static sys_slist_t found_peers_contexts;
static sys_slist_t connected_peers_contexts;
static sys_slist_t bond_data_contexts;

static void *get_peer_by_bdaddr(sys_slist_t *const peers_contexts,
				gap_bdaddr_t const *const p_addr);

static bool is_bdaddr_valid(gap_bdaddr_t const *const p_addr)
{
	return (!!p_addr && p_addr->addr_type <= GAP_ADDR_RAND);
}

static void *get_peer_by_irk(const gap_sec_key_t *const p_irk)
{
	if (!p_irk) {
		return NULL;
	}

	struct peer_data *p_peer;
	sys_snode_t *node = NULL;

	SYS_SLIST_ITERATE_FROM_NODE(&bond_data_contexts, node)
	{
		p_peer = (struct peer_data *)node;
		if (!memcmp(&((gapc_pairing_keys_t *)p_peer->keys.data)->irk.key, p_irk->key,
			    sizeof(p_irk->key))) {
			return p_peer;
		}
	}

	return NULL;
}

static void *get_peer_by_bdaddr(sys_slist_t *const peers_contexts, gap_bdaddr_t const *const p_addr)
{
	if (!is_bdaddr_valid(p_addr)) {
		return NULL;
	}

	struct peer_data *p_peer;
	sys_snode_t *node = NULL;

	SYS_SLIST_ITERATE_FROM_NODE(peers_contexts, node)
	{
		p_peer = (struct peer_data *)node;
		if (!memcmp(((gapc_pairing_keys_t *)p_peer->keys.data)->irk.identity.addr,
			    p_addr->addr, sizeof(p_addr->addr))) {
			return p_peer;
		}
	}

	return NULL;
}

static void *get_peer_by_activity_index(uint32_t const actv_idx)
{
	struct peer_data *p_peer;
	sys_snode_t *node = NULL;

	SYS_SLIST_ITERATE_FROM_NODE(&connected_peers_contexts, node)
	{
		p_peer = (struct peer_data *)node;
		if (p_peer->actv_idx == actv_idx) {
			return p_peer;
		}
	}

	LOG_ERR("Peer not found by activity index %u", actv_idx);
	return NULL;
}

static void *get_peer_by_connection_index(uint32_t const conidx)
{
	struct peer_data *p_peer;
	sys_snode_t *node = NULL;

	SYS_SLIST_ITERATE_FROM_NODE(&connected_peers_contexts, node)
	{
		p_peer = (struct peer_data *)node;
		if (p_peer->conidx == conidx) {
			return p_peer;
		}
	}

	LOG_ERR("Peer not found by connection index %u", conidx);
	return NULL;
}

static int handle_found_peer(gap_bdaddr_t const *const p_addr)
{
	struct peer_data *p_peer = storage_allocate_peer_context();

	if (!p_peer) {
		LOG_ERR("Failed to allocate memory for a new peer data");
		return -ENOMEM;
	}

	gapc_pairing_keys_t *p_keys = p_peer->keys.data;

	memcpy(&p_keys->irk.identity, p_addr, sizeof(p_keys->irk.identity));
	LOG_INF("Found peer index(%u): %02X:%02X:%02X:%02X:%02X:%02X", p_peer->storage_index,
		p_addr->addr[5], p_addr->addr[4], p_addr->addr[3], p_addr->addr[2], p_addr->addr[1],
		p_addr->addr[0]);

	sys_slist_append(&found_peers_contexts, &p_peer->node);

	return 0;
}

void *get_connected_peer_ltk(uint32_t const conidx)
{
	struct peer_data *p_peer = get_peer_by_connection_index(conidx);

	if (!p_peer) {
		return NULL;
	}

	gapc_pairing_keys_t *p_pairing_keys = p_peer->keys.data;

	if (!p_pairing_keys || !(p_pairing_keys->valid_key_bf & GAP_KDIST_ENCKEY)) {
		LOG_ERR("LTK not present for peer with connection index %u", conidx);
		return NULL;
	}

	return p_pairing_keys->ltk.key.key;
}

static void handle_disconnected_peer(struct peer_data *p_peer, bool const cleanup)
{
	if (!p_peer) {
		return;
	}

	csisc_dev_disconnected(p_peer->conidx);

	if (cleanup) {
		gapc_bond_data_t *p_bond_data = p_peer->connection.data;
		gapc_pairing_keys_t *p_appkeys = p_peer->keys.data;

		if (p_bond_data) {
			memset(p_bond_data, 0, sizeof(*p_bond_data));
			storage_delete(SETTINGS_NAME_CONNECTION, p_peer->storage_index);
		}

		if (p_appkeys) {
			memset(p_appkeys, 0, sizeof(*p_appkeys));
			p_appkeys->irk.identity.addr_type = 0xFF;
			storage_delete(SETTINGS_NAME_KEYS, p_peer->storage_index);
		}
	}

	p_peer->conidx = GAP_INVALID_CONIDX;

	sys_slist_find_and_remove(&connected_peers_contexts, &p_peer->node);
	if (!cleanup) {
		sys_slist_append(&bond_data_contexts, &p_peer->node);
	}
}

static size_t num_of_resolves_ongoing;

static void on_address_resolved_cb(const uint16_t status, const gap_addr_t *const p_addr,
				   const gap_sec_key_t *const p_irk)
{
	struct peer_data *p_peer = get_peer_by_irk(p_irk);
	bool const known = (status == GAP_ERR_NO_ERROR && p_peer);
	gap_bdaddr_t address = {
		.addr_type = GAP_ADDR_RAND,
	};
	memcpy(&address.addr, p_addr->addr, sizeof(address.addr));

	LOG_INF("Peer %02X:%02X:%02X:%02X:%02X:%02X is %s device", address.addr[5], address.addr[4],
		address.addr[3], address.addr[2], address.addr[1], address.addr[0],
		known ? "KNOWN" : "UNKNOWN");

	if (!p_peer) {
		LOG_INF("Peer is unknown. Add to found list.");
		handle_found_peer(&address);
		if (num_of_resolves_ongoing) {
			num_of_resolves_ongoing--;
		}

		return;
	}
	LOG_INF("Peer is known. Update address for a connection.");
	/* Update address for a connection */
	((gapc_pairing_keys_t *)p_peer->keys.data)->irk.identity = address;
	sys_slist_find_and_remove(&bond_data_contexts, &p_peer->node);
	sys_slist_append(&found_peers_contexts, &p_peer->node);


	if (num_of_resolves_ongoing) {
		num_of_resolves_ongoing--;
	}
}

static gap_sec_key_t *sec_key_list;
static uint8_t sec_key_list_size;

static int resolve_peer_address(const gap_bdaddr_t *const p_peer_addr)
{
	if (!sec_key_list_size || !sec_key_list) {
		LOG_ERR("Resolving address is not possible. No IRKs in the list!");
		return -ENOEXEC;
	}

	if (!p_peer_addr || p_peer_addr->addr_type == GAP_ADDR_PUBLIC) {
		return -EINVAL;
	}

	uint32_t const add_type =
		(p_peer_addr->addr[GAP_BD_ADDR_LEN - 1] & BD_ADDR_RND_ADDR_TYPE_MSK);

	LOG_INF("Peer address type: %u (%s)", add_type,
		(add_type == BD_ADDR_RSLV) ? "RESOLVABLE" : "NON-RESOLVABLE");

	if (add_type != BD_ADDR_RSLV) {
		LOG_ERR("Peer address is not resolvable. Cannot resolve!");
		return -EINVAL;
	}

	uint16_t const status =
		gapm_le_resolve_address((gap_addr_t *)p_peer_addr->addr, sec_key_list_size,
					sec_key_list, on_address_resolved_cb);

	if (status != GAP_ERR_NO_ERROR) {
		LOG_ERR("Unable to start resolve address! err:%u", status);
		return -status;
	}

	num_of_resolves_ongoing++;

	return 0;
}

static int request_pairing_and_bonding(uint8_t const conidx)
{
	gapc_pairing_t const pairing_info = {
		.auth = GAP_AUTH_REQ_SEC_CON_BOND,
		.iocap = GAP_IO_CAP_KB_DISPLAY,
		.oob = GAP_OOB_AUTH_DATA_NOT_PRESENT,
		.key_size = GAP_KEY_LEN,
		.ikey_dist = GAP_KDIST_ENCKEY | GAP_KDIST_IDKEY,
		.rkey_dist = GAP_KDIST_ENCKEY | GAP_KDIST_IDKEY,
	};

	if (GAP_ERR_NO_ERROR != gapc_le_bond(conidx, &pairing_info, 0)) {
		LOG_ERR("Peer %u unable to start pairing!", conidx);
		return -1;
	}

	LOG_DBG("Peer %u pairing and bonding initiated...", conidx);
	return 0;
}

static void on_get_peer_version_cmp_cb(uint8_t const conidx, uint32_t const metainfo,
				       uint16_t status, const gapc_version_t *const p_version)
{
	struct peer_data *p_peer = get_peer_by_connection_index(conidx);

	if (!p_peer) {
		LOG_ERR("Peer %u not found on the connected list", conidx);
		return;
	}

	if (status != GAP_ERR_NO_ERROR) {
		LOG_ERR("Peer %u get peer version failed, status:%u. Restart pairing", conidx,
			status);
		request_pairing_and_bonding(conidx);
		return;
	}

	LOG_INF("Peer %u company_id:%u, lmp_subversion:%u, lmp_version:%u", conidx,
		p_version->company_id, p_version->lmp_subversion, p_version->lmp_version);

	p_peer->conidx = conidx;
	conn_status = status;

	LOG_INF("Peer %u features and version received, connection complete!", conidx);
	k_sem_give(&wait_connection_sem);
}

static void on_peer_features_cmp_cb(uint8_t const conidx, uint32_t const metainfo, uint16_t status,
				    const uint8_t *const p_features)
{
	struct peer_data *p_peer = get_peer_by_connection_index(conidx);

	if (!p_peer) {
		LOG_ERR("Peer %u not found on the connected list", conidx);
		return;
	}

	if (status != GAP_ERR_NO_ERROR) {
		LOG_ERR("Peer %u get peer features failed, status:%u. Restart pairing", conidx,
			status);
		request_pairing_and_bonding(conidx);
		return;
	}

	LOG_DBG("Peer %u features: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X", conidx, p_features[0],
		p_features[1], p_features[2], p_features[3], p_features[4], p_features[5],
		p_features[6], p_features[7]);

	status = gapc_get_peer_version(conidx, metainfo, on_get_peer_version_cmp_cb);
	if (status != GAP_ERR_NO_ERROR) {
		LOG_ERR("Peer %u unable to get peer version! err:%u", conidx, status);
	}
}

static void on_link_encryption(uint8_t const conidx, uint32_t const metainfo, uint16_t status)
{
	LOG_DBG("Peer %u link encryption! metainfo:%u status:%u", conidx, metainfo, status);

	if (status == SMP_ERR_ENC_KEY_MISSING) {
		LOG_ERR("Peer %u bond data was incorrect! Restart pairing", conidx);
		/* Bond data was incorrect. Request a new pairing */
		if (!request_pairing_and_bonding(conidx)) {
			return;
		}
	}

	if (status != GAP_ERR_NO_ERROR) {
		LOG_ERR("Peer %u link encryption failed! err:%u. Please, reset the app!", conidx,
			status);
		return;
	}

	struct peer_data *p_peer = get_peer_by_connection_index(conidx);

	if (!p_peer) {
		LOG_ERR("Something went wrong. Please reset and try again.");
		return;
	}

	status = gapc_le_get_peer_features(conidx, metainfo, on_peer_features_cmp_cb);
	if (status != GAP_ERR_NO_ERROR) {
		LOG_ERR("Peer %u unable to get peer features! err:%u", conidx, status);
	}
}

static void connection_confirmation(struct peer_data *p_peer)
{
	uint_fast16_t status;
	uint_fast8_t const conidx = p_peer->conidx;
	bool const bonded = (((gapc_bond_data_t *)p_peer->connection.data)->pairing_lvl &
			     GAP_PAIRING_BOND_PRESENT_BIT)
				    ? true
				    : false;

	LOG_INF("Peer %u confirm %s!", conidx, bonded ? "BONDED" : "NOT BONDED");

	status = gapc_le_connection_cfm(conidx, (uint32_t)p_peer,
					bonded ? p_peer->connection.data : NULL);
	if (status != GAP_ERR_NO_ERROR) {
		LOG_ERR("Peer %u connection confirmation failed! err:%u", conidx, status);
	}

	if (bonded) {
		LOG_INF("Peer %u already bonded, try to encrypt link", conidx);

		gapc_pairing_keys_t *p_keys = p_peer->keys.data;

		if (p_keys->valid_key_bf & GAP_KDIST_ENCKEY) {
			status = gapc_le_encrypt(conidx, (uint32_t)p_peer, &p_keys->ltk,
						 on_link_encryption);
			if (status != GAP_ERR_NO_ERROR) {
				LOG_ERR("Peer %u unable to start encryption! err:%u", conidx,
					status);
			}
			return;
		}
		LOG_WRN("Peer %u LTK not found, request pairing", conidx);
	}

	request_pairing_and_bonding(conidx);
}

static void on_le_connection_req(uint8_t const conidx, uint32_t const metainfo,
				 uint8_t const actv_idx, uint8_t const role,
				 const gap_bdaddr_t *const p_peer_addr,
				 const gapc_le_con_param_t *const p_con_params,
				 uint8_t const clk_accuracy)
{
	struct peer_data *p_peer = get_peer_by_activity_index(actv_idx);

	if (!p_peer) {
		LOG_ERR("Peer not found by activity index %u", actv_idx);
		return;
	}

	p_peer->conidx = conidx;

	connection_confirmation(p_peer);

	LOG_INF("Connection request. conidx:%u (actv_idx:%u), role %s", conidx, actv_idx,
		role ? "PERIPH" : "CENTRAL");

	LOG_DBG("  interval %fms, latency %u, supervision timeout %ums, clk_accuracy:%u",
		(p_con_params->interval * 1.25), p_con_params->latency,
		(uint32_t)p_con_params->sup_to * 10, clk_accuracy);

	LOG_DBG("  peer address: %s %02X:%02X:%02X:%02X:%02X:%02X",
		(p_peer_addr->addr_type == GAP_ADDR_PUBLIC) ? "Public" : "Private",
		p_peer_addr->addr[5], p_peer_addr->addr[4], p_peer_addr->addr[3],
		p_peer_addr->addr[2], p_peer_addr->addr[1], p_peer_addr->addr[0]);
}

static const struct gapc_connection_req_cb gapc_con_cbs = {
	.le_connection_req = on_le_connection_req,
};

static void on_gapc_sec_auth_info(uint8_t const conidx, uint32_t const metainfo,
				  uint8_t const sec_lvl, bool const encrypted,
				  uint8_t const key_size)
{
	LOG_DBG("Peer %u security auth info. level:%d, encrypted:%s", conidx, sec_lvl,
		(encrypted ? "TRUE" : "FALSE"));
}

static void on_gapc_pairing_succeed(uint8_t const conidx, uint32_t const metainfo,
				    uint8_t const pairing_level, bool const enc_key_present,
				    uint8_t const key_type)
{
	bool const bonded = gapc_is_bonded(conidx);
	struct peer_data *p_peer = get_peer_by_connection_index(conidx);

	if (!p_peer) {
		return;
	}

	gapc_bond_data_t *p_bond_data = p_peer->connection.data;

	if (!p_bond_data) {
		p_bond_data = malloc(sizeof(*p_bond_data));
		if (!p_bond_data) {
			LOG_ERR("Failed to allocate memory for bond data");
			return;
		}
		p_peer->connection.data = p_bond_data;
		p_peer->connection.size = sizeof(*p_bond_data);
	}

	LOG_INF("Peer %u Pairing SUCCEED. pairing_level:%u, bonded:%s, enc_key_present:%d", conidx,
		pairing_level, bonded ? "TRUE" : "FALSE", enc_key_present);

	p_bond_data->pairing_lvl = pairing_level;
	p_bond_data->enc_key_present = enc_key_present;

	storage_store(SETTINGS_NAME_CONNECTION, p_peer->storage_index, p_bond_data,
		      sizeof(*p_bond_data));

	uint16_t const status =
		gapc_le_get_peer_features(conidx, (uint32_t)p_peer, on_peer_features_cmp_cb);

	if (status != GAP_ERR_NO_ERROR) {
		LOG_ERR("Peer %u unable to get peer features! err:%u", conidx, status);
	}
}

static void on_gapc_pairing_failed(uint8_t const conidx, uint32_t const metainfo,
				   uint16_t const reason)
{
	LOG_ERR("Peer %u pairing FAILED! Reason %u", conidx, reason);

	struct peer_data *p_peer = get_peer_by_connection_index(conidx);

	if (!p_peer) {
		LOG_ERR("Peer %u not found on the connected list", conidx);
		return;
	}

	handle_disconnected_peer(p_peer, true);
	conn_status = reason;
	k_sem_give(&wait_connection_sem);
}

static void on_gapc_info_req(uint8_t const conidx, uint32_t const metainfo, uint8_t const exp_info)
{
	uint16_t err;
	struct peer_data *p_peer = get_peer_by_connection_index(conidx);

	if (!p_peer) {
		LOG_ERR("Peer %u not found on the connected list", conidx);
		return;
	}

	switch (exp_info) {
	case GAPC_INFO_IRK: {
		err = gapc_le_pairing_provide_irk(conidx, &gapm_irk);
		if (err != GAP_ERR_NO_ERROR) {
			LOG_ERR("Peer %u IRK provide failed", conidx);
			break;
		}
		LOG_INF("Peer %u IRK provided successful", conidx);
		break;
	}
	case GAPC_INFO_CSRK: {
		err = gapc_pairing_provide_csrk(
			conidx, &((gapc_bond_data_t *)p_peer->connection.data)->local_csrk);
		if (err != GAP_ERR_NO_ERROR) {
			LOG_ERR("Peer %u CSRK provide failed", conidx);
			break;
		}
		LOG_INF("Peer %u CSRK provided successful", conidx);
		break;
	}
	case GAPC_INFO_PASSKEY_ENTERED:
	case GAPC_INFO_PASSKEY_DISPLAYED: {
		err = gapc_pairing_provide_passkey(conidx, true, 123456);
		if (err != GAP_ERR_NO_ERROR) {
			LOG_ERR("Peer %u PASSKEY provide failed. err: %u", conidx, err);
			break;
		}
		LOG_INF("Peer %u PASSKEY 123456 provided", conidx);
		break;
	}
	default:
		LOG_WRN("Peer %u Unsupported info %u requested!", conidx, exp_info);
		break;
	}
}

static void on_gapc_sec_numeric_compare_req(uint8_t const conidx, uint32_t const metainfo,
					    uint32_t const value)
{
	LOG_DBG("Peer %u pairing numeric compare. value:%u", conidx, value);
	/* Automatically confirm */
	gapc_pairing_numeric_compare_rsp(conidx, true);
}

static void on_key_received(uint8_t const conidx, uint32_t const metainfo,
			    const gapc_pairing_keys_t *const p_keys)
{
	LOG_DBG("Peer %u key received. key_bf:%u, level:%u", conidx, p_keys->valid_key_bf,
		p_keys->pairing_lvl);

	struct peer_data *p_peer = get_peer_by_connection_index(conidx);

	if (!p_peer) {
		LOG_ERR("Peer %u not found on the connected list", conidx);
		return;
	}

	gapc_pairing_keys_t *p_appkeys = p_peer->keys.data;

	if (!p_appkeys) {
		p_appkeys = malloc(sizeof(*p_appkeys));
		if (!p_appkeys) {
			LOG_ERR("Failed to allocate memory for pairing keys");
			return;
		}
		p_peer->keys.data = p_appkeys;
		p_peer->keys.size = sizeof(*p_appkeys);
	}

	uint8_t key_bits = GAP_KDIST_NONE;
	uint8_t const valid_key_bf = p_keys->valid_key_bf;

	if (valid_key_bf & GAP_KDIST_ENCKEY) {
		memcpy(&p_appkeys->ltk, &p_keys->ltk, sizeof(p_appkeys->ltk));
		key_bits |= GAP_KDIST_ENCKEY;
	}

	if (valid_key_bf & GAP_KDIST_IDKEY) {
		memcpy(&p_appkeys->irk, &p_keys->irk, sizeof(p_appkeys->irk));
		key_bits |= GAP_KDIST_IDKEY;
	}

	if (valid_key_bf & GAP_KDIST_SIGNKEY) {
		memcpy(&p_appkeys->csrk, &p_keys->csrk, sizeof(p_appkeys->csrk));
		key_bits |= GAP_KDIST_SIGNKEY;
	}

	p_appkeys->pairing_lvl = p_keys->pairing_lvl;
	p_appkeys->valid_key_bf = key_bits;

	storage_store(SETTINGS_NAME_KEYS, p_peer->storage_index, p_appkeys, sizeof(*p_appkeys));
}

static const gapc_security_cb_t gapc_sec_cbs = {
	.auth_info = on_gapc_sec_auth_info,
	.pairing_succeed = on_gapc_pairing_succeed,
	.pairing_failed = on_gapc_pairing_failed,
	.info_req = on_gapc_info_req,
	.numeric_compare_req = on_gapc_sec_numeric_compare_req,
	.key_received = on_key_received,
	/* Others are useless for LE central device */
};

static void on_disconnection(uint8_t const conidx, uint32_t const metainfo, uint16_t const reason)
{
	LOG_INF("Peer %u disconnected for reason %u", conidx, reason);

	if (reason == LL_ERR_CON_TERM_BY_LOCAL_HOST) {
		return;
	} else if (reason == LL_ERR_CON_TIMEOUT) {
		LOG_ERR("Peer %u connection timeout!", conidx);
	}
	struct peer_data *p_peer = get_peer_by_connection_index(conidx);

	if (!p_peer) {
		LOG_ERR("Peer %u not found on the connected list", conidx);
		return;
	}

	handle_disconnected_peer(p_peer, false);
	conn_status = reason;
	k_sem_give(&wait_connection_sem);
}

static void on_bond_data_updated(uint8_t const conidx, uint32_t const metainfo,
				 const gapc_bond_data_updated_t *const p_data)
{
	LOG_INF("Peer %u bond data updated: gatt_start_hdl:%u, gatt_end_hdl:%u, "
		"svc_chg_hdl:%u, cli_info:%u, cli_feat:%u, srv_feat:%u",
		conidx, p_data->gatt_start_hdl, p_data->gatt_end_hdl, p_data->svc_chg_hdl,
		p_data->cli_info, p_data->cli_feat, p_data->srv_feat);

	struct peer_data *p_peer = get_peer_by_connection_index(conidx);

	if (!p_peer) {
		LOG_ERR("Peer %u not found on the connected list", conidx);
		return;
	}

	gapc_bond_data_t *p_bond_data = p_peer->connection.data;

	if (!p_bond_data) {
		p_bond_data = malloc(sizeof(*p_bond_data));
		if (!p_bond_data) {
			LOG_ERR("Failed to allocate memory for bond data");
			return;
		}
		p_peer->connection.data = p_bond_data;
		p_peer->connection.size = sizeof(*p_bond_data);
	}

	p_bond_data->local_sign_counter = p_data->local_sign_counter;
	p_bond_data->remote_sign_counter = p_data->peer_sign_counter;
	p_bond_data->gatt_start_hdl = p_data->gatt_start_hdl;
	p_bond_data->gatt_end_hdl = p_data->gatt_end_hdl;
	p_bond_data->svc_chg_hdl = p_data->svc_chg_hdl;
	p_bond_data->cli_info = p_data->cli_info;
	p_bond_data->cli_feat = p_data->cli_feat;
	p_bond_data->srv_feat = p_data->srv_feat;

	storage_store(SETTINGS_NAME_CONNECTION, p_peer->storage_index, p_bond_data,
		      sizeof(*p_bond_data));
}

static void on_name_get(uint8_t const conidx, uint32_t const metainfo, uint16_t const token,
			uint16_t const offset, uint16_t const max_len)
{
	const size_t device_name_len = sizeof(device_name) - 1;
	const size_t short_len = (device_name_len > max_len ? max_len : device_name_len);

	gapc_le_get_name_cfm(conidx, token, GAP_ERR_NO_ERROR, device_name_len, short_len,
			     (const uint8_t *)device_name);
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
	/* Other callbacks in this struct are optional */
};

static const gapc_le_config_cb_t gapc_le_cfg_cbs = {0};

static void on_gapm_err(uint32_t metainfo, uint8_t code)
{
	LOG_ERR("GAPM error %d", code);
}

static const gapm_cb_t gapm_err_cbs = {
	.cb_hw_error = on_gapm_err,
};

static const gapm_callbacks_t gapm_cbs = {
	.p_con_req_cbs = &gapc_con_cbs,
	.p_sec_cbs = &gapc_sec_cbs,
	.p_info_cbs = &gapc_inf_cbs,
	.p_le_config_cbs = &gapc_le_cfg_cbs,
	.p_bt_config_cbs = NULL, /* BT classic so not required */
	.p_gapm_cbs = &gapm_err_cbs,
};

/* ---------------------------------------------------------------------------------------- */
/* LE Connection Init */

static void on_gapm_le_init_proc_cmp(uint32_t const metainfo, uint8_t const proc_id,
				     uint8_t const actv_idx, uint16_t const status)
{
	switch (proc_id) {
	case GAPM_ACTV_START:
	case GAPM_ACTV_DELETE: {
		LOG_INF("GAPM activity %s. actv_idx:%u, metainfo: %X, status: %u",
			proc_id == GAPM_ACTV_START ? "START" : "DELETE", actv_idx, metainfo,
			status);
		break;
	}
	default: {
		LOG_ERR("GAPM activity unknown! proc_id:%u, actv_idx:%u, status: %u", proc_id,
			actv_idx, status);
		break;
	}
	}
	ASSERT_INFO(status == GAP_ERR_NO_ERROR, status, 0);
}

static void on_gapm_le_init_stopped(uint32_t const metainfo, uint8_t const actv_idx,
				    uint16_t const reason)
{

	LOG_INF("GAPM activity stopped. ID:%u, metainfo: %X, reason: %u", actv_idx, metainfo,
		reason);

	/* Activity context is not needed anymore */
	gapm_delete_activity(actv_idx);

	if (metainfo) {
		struct peer_data *p_peer = (struct peer_data *)metainfo;

		p_peer->actv_idx = GAP_INVALID_ACTV_IDX;
	}
	k_sem_give(&wait_stop_sem);
	if (reason != GAP_ERR_NO_ERROR) {
		LOG_ERR("GAPM LE connection init stopped with error! reason: %u", reason);
		conn_status = reason;
		k_sem_give(&wait_connection_sem);
	}
}

static void on_addr_updated(uint32_t const metainfo, uint8_t const actv_idx,
			    const gap_addr_t *p_addr)
{

	LOG_INF("idx %u (meta:%u) Address updated: %02X:%02X:%02X:%02X:%02X:%02X", actv_idx,
		metainfo, p_addr->addr[5], p_addr->addr[4], p_addr->addr[3], p_addr->addr[2],
		p_addr->addr[1], p_addr->addr[0]);
}

static void on_peer_name_received(uint32_t const metainfo, uint8_t const actv_idx,
				  const gap_bdaddr_t *const p_addr, uint16_t const name_len,
				  const uint8_t *const p_name)
{
	LOG_INF("Peer name received: %s", ((name_len && p_name) ? (char *)p_name : "Unknown"));
}

static const gapm_le_init_cb_actv_t cbs_gapm_le_init = {
	.hdr.actv.stopped = on_gapm_le_init_stopped,
	.hdr.actv.proc_cmp = on_gapm_le_init_proc_cmp,
	.hdr.addr_updated = on_addr_updated,
	.peer_name = on_peer_name_received,
};

static int connect_to_device(struct peer_data *p_peer)
{
#define CONN_INTERVAL_MIN 24                      /* 24 * 1.25 ms = 30ms */
#define CONN_INTERVAL_MAX (CONN_INTERVAL_MIN + 8) /* 32 * 1.25 ms = 40ms */
#define CE_LEN_MIN        1
#define CE_LEN_MAX        3
#define SUPERVISION_MS    5000

	gapc_pairing_keys_t *p_appkeys = p_peer->keys.data;

	alif_ble_mutex_lock(K_FOREVER);

	if (!p_appkeys || !is_bdaddr_valid(&p_appkeys->irk.identity)) {
		LOG_ERR("CTD:Invalid peer address");
		alif_ble_mutex_unlock();
		return -EINVAL;
	}

	/* clang-format off */

	gapm_le_init_param_t const init_params = {
		.prop = (GAPM_INIT_PROP_1M_BIT | GAPM_INIT_PROP_2M_BIT),
		.conn_to = 1500, /* Timeout in 10ms units = 1 second */
		.scan_param_1m = {
			.scan_intv = 160, /* (N * 0.625 ms) = 100ms */
			.scan_wd = 80,    /* (N * 0.625 ms) = 40ms */
		},
		.conn_param_1m = {
			.conn_intv_min = CONN_INTERVAL_MIN,
			.conn_intv_max = CONN_INTERVAL_MAX,
			.conn_latency = 0,
			.supervision_to = SUPERVISION_MS / 10,
			.ce_len_min = CE_LEN_MIN,
			.ce_len_max = CE_LEN_MAX,
		},
		.conn_param_2m = {
			.conn_intv_min = CONN_INTERVAL_MIN,
			.conn_intv_max = CONN_INTERVAL_MAX,
			.conn_latency = 0,
			.supervision_to = SUPERVISION_MS / 10,
			.ce_len_min = CE_LEN_MIN,
			.ce_len_max = CE_LEN_MAX,
		},
		.conn_param_coded = {
			.conn_intv_min = CONN_INTERVAL_MIN,
			.conn_intv_max = CONN_INTERVAL_MAX,
			.conn_latency = 0,
			.supervision_to = SUPERVISION_MS / 10,
			.ce_len_min = CE_LEN_MIN,
			.ce_len_max = CE_LEN_MAX,
		},
		.peer_addr = p_appkeys->irk.identity,
	};

	/* clang-format on */

	uint16_t err;

	if (p_peer->actv_idx == GAP_INVALID_ACTV_IDX) {
		LOG_ERR("Peer activity index is invalid, create new activity for the peer");
		err = gapm_le_create_init((uint32_t)p_peer, GAPM_STATIC_ADDR, &cbs_gapm_le_init,
					  &p_peer->actv_idx);
		if (err != GAP_ERR_NO_ERROR) {
			LOG_ERR("gapm_le_create_init error %u", err);
			alif_ble_mutex_unlock();
			return -1;
		}
	}

	k_sem_reset(&wait_connection_sem);

	err = gapm_le_start_direct_connection(p_peer->actv_idx, &init_params);
	if (err != GAP_ERR_NO_ERROR) {
		LOG_ERR("gapm_le_start_direct_connection error %u", err);
		alif_ble_mutex_unlock();
		return -1;
	}

	extern const char *bdaddr_str(const gap_bdaddr_t *p_addr);
	LOG_INF("Connecting to peer %s, type: %u", bdaddr_str(&init_params.peer_addr),
		init_params.peer_addr.addr_type);
	alif_ble_mutex_unlock();
	/* Wait for connection semaphore */
	if (k_sem_take(&wait_connection_sem, K_SECONDS(15)) != 0) {
		LOG_ERR("Connection timeout!");
		if (p_peer->actv_idx != GAP_INVALID_ACTV_IDX) {
			alif_ble_mutex_lock(K_FOREVER);
			k_sem_reset(&wait_stop_sem);
			gapm_stop_activity(p_peer->actv_idx);
			alif_ble_mutex_unlock();
			k_sem_take(&wait_stop_sem, K_FOREVER);
		}
		return -1;
	}

	if (conn_status != GAP_ERR_NO_ERROR) {
		LOG_ERR("Peer %u connection failed! err:%u", p_peer->conidx, conn_status);
		return -1;
	}

	return 0;
}

static void scanning_ready_callback(void)
{
	LOG_INF("Scanning ready - trigger connect...");

	k_sem_give(&central_state_sem);
}

static int peer_found(gap_bdaddr_t const *const p_addr, atc_csis_rsi_t const *const p_rsi)
{
	if (!is_bdaddr_valid(p_addr)) {
		LOG_ERR("PF: Invalid peer address %02X:%02X:%02X:%02X:%02X:%02X", p_addr->addr[5],
			p_addr->addr[4], p_addr->addr[3], p_addr->addr[2], p_addr->addr[1],
			p_addr->addr[0]);
		return -EINVAL;
	}

	if (csisc_dev_add(p_addr, p_rsi)) {
		LOG_ERR("Failed to add device with address %02X:%02X:%02X:%02X:%02X:%02X",
			p_addr->addr[5], p_addr->addr[4], p_addr->addr[3], p_addr->addr[2],
			p_addr->addr[1], p_addr->addr[0]);
		return -1;
	}

	struct peer_data *p_peer = get_peer_by_bdaddr(&found_peers_contexts, p_addr);

	if (p_peer) {
		return -EALREADY;
	}

	if (!resolve_peer_address(p_addr)) {
		/* rest is handled in on_address_resolved_cb */
		return 0;
	}

	p_peer = get_peer_by_bdaddr(&bond_data_contexts, p_addr);
	if (p_peer) {
		/* Don't create a new context. Just add to found peer list */
		sys_slist_find_and_remove(&bond_data_contexts, &p_peer->node);
		sys_slist_append(&found_peers_contexts, &p_peer->node);
		return 0;
	}

	return handle_found_peer(p_addr);
}

static void on_gapm_process_complete(uint32_t const metainfo, uint16_t const status)
{
	if (status != GAP_ERR_NO_ERROR) {
		LOG_ERR("GAPM process completed with error %u", status);
		return;
	}

	k_sem_give(&wait_procedure_sem);
}

static gap_addr_t private_address = {
	.addr = {0xE8, 0xEB, 0x9D, 0x44, 0x6B, 0x67},
};

static void on_gapm_le_random_addr_cb(uint16_t const status, const gap_addr_t *const p_addr)
{
	if (status != GAP_ERR_NO_ERROR) {
		LOG_ERR("GAPM address generation error %u", status);
		return;
	}

	LOG_DBG("Generated address: %02X:%02X:%02X:%02X:%02X:%02X", p_addr->addr[5],
		p_addr->addr[4], p_addr->addr[3], p_addr->addr[2], p_addr->addr[1],
		p_addr->addr[0]);

	private_address = *p_addr;

	k_sem_give(&wait_procedure_sem);
}

static int ble_stack_configure(void)
{
	/* Bluetooth stack configuration*/
	gapm_config_t gapm_cfg = {
		.role = GAP_ROLE_LE_CENTRAL,
		.pairing_mode = GAPM_PAIRING_SEC_CON,
		.privacy_cfg = 0,
		.renew_dur = 15 * 60, /* 15 minutes */
		.private_identity.addr = {0},
		.irk = gapm_irk,
		.gap_start_hdl = 0,
		.gatt_start_hdl = 0,
		.att_cfg = 0,
		.sugg_max_tx_octets = GAP_LE_MAX_OCTETS,
		/* Use the minimum transmission time to minimize latency */
		.sugg_max_tx_time = GAP_LE_MIN_TIME,
		.tx_pref_phy = GAP_PHY_ANY,
		.rx_pref_phy = GAP_PHY_ANY,
		.tx_path_comp = 0,
		.rx_path_comp = 0,
		/* BT Classic - not used */
		.class_of_device = 0x200408,
		.dflt_link_policy = 0,
	};

	int err;

	/* Configure GAPM to prepare address generation */
	err = gapm_configure(1, &gapm_cfg, &gapm_cbs, on_gapm_process_complete);
	if (err != GAP_ERR_NO_ERROR) {
		LOG_ERR("gapm_configure error %u", err);
		return -1;
	}
	if (k_sem_take(&wait_procedure_sem, K_MSEC(1000))) {
		LOG_ERR("  FAIL! GAPM config timeout!");
		return -1;
	}

	/* Generate resolvable random address */
	err = gapm_le_generate_random_addr(GAP_BD_ADDR_RSLV, on_gapm_le_random_addr_cb);
	if (err != GAP_ERR_NO_ERROR) {
		LOG_ERR("gapm_le_generate_random_addr error %u", err);
		return -1;
	}
	if (k_sem_take(&wait_procedure_sem, K_MSEC(1000))) {
		LOG_ERR("  FAIL! GAPM random address timeout!");
		return -1;
	}

	/* Reset GAPM to set address */
	err = gapm_reset(3, on_gapm_process_complete);
	if (err != GAP_ERR_NO_ERROR) {
		LOG_ERR("gapm_reset error %u", err);
		return -1;
	}
	if (k_sem_take(&wait_procedure_sem, K_MSEC(1000))) {
		LOG_ERR("  FAIL! GAPM reset timeout!");
		return -1;
	}

	/* Reconfigure GAPM with generated address */
	gapm_cfg.privacy_cfg = GAPM_PRIV_CFG_PRIV_ADDR_BIT | GAPM_PRIV_CFG_PRIV_EN_BIT;
	gapm_cfg.private_identity = private_address;

	err = gapm_configure(1, &gapm_cfg, &gapm_cbs, on_gapm_process_complete);
	if (err != GAP_ERR_NO_ERROR) {
		LOG_ERR("gapm_configure error %u", err);
		return -1;
	}
	if (k_sem_take(&wait_procedure_sem, K_MSEC(1000))) {
		LOG_ERR("  FAIL! GAPM config timeout!");
		return -1;
	}

	LOG_INF("Set name: %s", device_name);
	err = gapm_set_name(2, strlen(device_name), (const uint8_t *)device_name,
			    on_gapm_process_complete);
	if (err != GAP_ERR_NO_ERROR) {
		LOG_ERR("gapm_set_name error %u", err);
		return -1;
	}
	if (k_sem_take(&wait_procedure_sem, K_MSEC(1000))) {
		LOG_ERR("  FAIL! GAPM name set timeout!");
		return -1;
	}

	/* Configure security level */
	gapm_le_configure_security_level(GAP_SEC1_SEC_CON_PAIR_ENC);

	gap_bdaddr_t identity;

	gapm_get_identity(&identity);

	LOG_INF("Device address: %02X:%02X:%02X:%02X:%02X:%02X", identity.addr[5], identity.addr[4],
		identity.addr[3], identity.addr[2], identity.addr[1], identity.addr[0]);

	LOG_DBG("BLE init complete!");

	return 0;
}

static int storage_load_bond_data(void)
{
	if (storage_init() < 0) {
		return -1;
	}

	storage_load_all(&bond_data_contexts);

	/* Build known peers list */
	sec_key_list_size = sys_slist_len(&bond_data_contexts);
	if (!sec_key_list_size) {
		LOG_INF("No bond data found");
		sec_key_list = NULL;
		return 0;
	}
	sec_key_list = calloc(sec_key_list_size, sizeof(*sec_key_list));

	if (!sec_key_list) {
		LOG_ERR("Unable to reserve memory for SEC key list!");
		return -ENOMEM;
	}

	struct peer_data *p_peer;
	gap_sec_key_t *p_sec_key;
	gapc_pairing_keys_t *p_pairing_keys;
	sys_snode_t *node = NULL;
	size_t index = 0;

	SYS_SLIST_ITERATE_FROM_NODE(&bond_data_contexts, node)
	{
		p_peer = (struct peer_data *)node;
		p_pairing_keys = p_peer->keys.data;
		p_sec_key = &sec_key_list[index++];
		memcpy(p_sec_key->key, p_pairing_keys->irk.key.key, sizeof(p_sec_key->key));

		p_peer->actv_idx = GAP_INVALID_ACTV_IDX;
		p_peer->conidx = GAP_INVALID_CONIDX;
	}

	LOG_INF("Settings loaded successfully");
	return 0;
}

static bool clean_bond_data;

void on_disconnect_cmp(uint8_t const conidx, uint32_t const metainfo, uint16_t const status)
{
	struct peer_data *p_peer = get_peer_by_connection_index(conidx);

	if (p_peer) {
		handle_disconnected_peer(p_peer, clean_bond_data);
		if (clean_bond_data) {
			LOG_INF("Bond data for peer %u removed", conidx);
			storage_free_peer_context(p_peer);
		}
		clean_bond_data = false;
	}

	k_sem_give(&wait_disc_sem);

	LOG_WRN("Disconnect completed. conidx:%u, status:%u", conidx, status);
}

static int terminate_connection(struct peer_data *p_peer, bool remove_bond)
{
	LOG_INF("Terminating connection with peer %u", p_peer->conidx);
	alif_ble_mutex_lock(K_FOREVER);

	clean_bond_data = remove_bond;

	uint32_t const status = gapc_disconnect(p_peer->conidx, (uint32_t)p_peer,
						LL_ERR_REMOTE_USER_TERM_CON, on_disconnect_cmp);
	alif_ble_mutex_unlock();
	k_sem_take(&wait_disc_sem, K_FOREVER);

	return (status == GAP_ERR_NO_ERROR) ? 0 : -ENOEXEC;
}

int app_disconnect(uint8_t conidx, bool remove_bond)
{
	struct peer_data *entry;

	alif_ble_mutex_lock(K_FOREVER);
	entry = get_peer_by_connection_index(conidx);
	alif_ble_mutex_unlock();
	if (!entry) {
		LOG_ERR("Peer %u is not connected", conidx);
		return -ENOENT;
	}

	return terminate_connection(entry, remove_bond);
}

int app_connect_device(gap_bdaddr_t const *const p_addr, uint8_t *connidx)
{
	int ret;

	alif_ble_mutex_lock(K_FOREVER);
	struct peer_data *p_peer = get_peer_by_bdaddr(&found_peers_contexts, p_addr);

	if (!p_peer) {
		LOG_ERR("Peer with address %02X:%02X:%02X:%02X:%02X:%02X not found in connected "
			"list",
			p_addr->addr[5], p_addr->addr[4], p_addr->addr[3], p_addr->addr[2],
			p_addr->addr[1], p_addr->addr[0]);
		alif_ble_mutex_unlock();
		return -ENOENT;
	}

	sys_slist_find_and_remove(&found_peers_contexts, &p_peer->node);
	sys_slist_append(&connected_peers_contexts, &p_peer->node);
	alif_ble_mutex_unlock();
	ret = connect_to_device(p_peer);

	alif_ble_mutex_lock(K_FOREVER);
	if (ret) {
		/* Remove peer from connected and bond data contexts */
		sys_slist_find_and_remove(&connected_peers_contexts, &p_peer->node);
		sys_slist_find_and_remove(&bond_data_contexts, &p_peer->node);
		/* Free peer context */
		storage_free_peer_context(p_peer);
	} else {
		*connidx = p_peer->conidx;
	}
	alif_ble_mutex_unlock();
	return ret;
}

static void handle_scan_finnished(void)
{
	int found_count;

	while (num_of_resolves_ongoing) {
		k_sleep(K_MSEC(50));
	}

	alif_ble_mutex_lock(K_FOREVER);
	found_count = sys_slist_len(&found_peers_contexts);
	alif_ble_mutex_unlock();

	if (found_count) {
		csisc_process();
	}

	/* Restart scan */
	int err = le_gaf_scan_start(scanning_ready_callback, 0);

	if (err) {
		LOG_ERR("Scan start failed! err:%d", err);
	}
}

int main(void)
{
	uint16_t err;
	int ret;

	sys_slist_init(&found_peers_contexts);
	sys_slist_init(&connected_peers_contexts);
	sys_slist_init(&bond_data_contexts);

	if (storage_load_bond_data() < 0) {
		return -1;
	}

	/* Start up bluetooth host stack */
	alif_ble_enable(NULL);

	if (ble_stack_configure()) {
		return -1;
	}

	err = csisc_configure(get_connected_peer_ltk);
	if (err) {
		LOG_ERR("Server configuration failed %u", err);
		return -1;
	}

	ret = le_gaf_scan_configure(peer_found);
	if (ret) {
		return ret;
	}

	ret = le_gaf_scan_start(scanning_ready_callback, 0);
	if (ret) {
		return ret;
	}

	while (1) {
		k_sem_take(&central_state_sem, K_FOREVER);
		handle_scan_finnished();
	}
}

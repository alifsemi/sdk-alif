/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
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

#include <se_service.h>
#include <soc_common.h>

#include "alif_ble.h"
#include "gapm.h"
#include "gapm_le_init.h"
#include "gap_le.h"
#include "gapc.h"
#include "gapc_le.h"
#include "gapc_sec.h"

#include "main.h"
#include "unicast_initiator.h"
#include "storage.h"
#include "scan.h"

#define BUTTON_NODELABEL DT_NODELABEL(button0)

#define APPEARANCE_EARBUDS 0x0941
#define APPEARANCE_HEADSET 0x0942

/* TODO: Change appearance to Headset when bidir communication is implemented */
#define APPEARANCE APPEARANCE_EARBUDS

LOG_MODULE_REGISTER(main, CONFIG_MAIN_LOG_LEVEL);

K_SEM_DEFINE(wait_procedure_sem, 0, 1);
K_SEM_DEFINE(wait_connection_sem, 0, 1);

static sys_slist_t found_peers_contexts;
static sys_slist_t connected_peers_contexts;
static sys_slist_t bond_data_contexts;

K_QUEUE_DEFINE(discovery_list);

/* Load names from configuration file */
const char device_name[] = CONFIG_BLE_DEVICE_NAME;

static const gap_sec_key_t gapm_irk = {.key = {0xA1, 0xB2, 0xC3, 0xD5, 0xE6, 0xF7, 0x08, 0x09, 0x12,
					       0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89}};

/* ---------------------------------------------------------------------------------------- */

static const struct gpio_dt_spec led_red = GPIO_DT_SPEC_GET_OR(DT_NODELABEL(led_red), gpios, {0});
static const struct gpio_dt_spec led_blue = GPIO_DT_SPEC_GET_OR(DT_NODELABEL(led_blue), gpios, {0});
static const struct gpio_dt_spec led_green =
	GPIO_DT_SPEC_GET_OR(DT_NODELABEL(led_green), gpios, {0});

static int configure_led(struct gpio_dt_spec const *const p_led)
{
	if (!p_led->port) {
		LOG_DBG("LED not found");
		return 0;
	}

	if (!gpio_is_ready_dt(p_led)) {
		LOG_ERR("LED not ready");
		return -ENODEV;
	}

	if (gpio_pin_configure_dt(p_led, GPIO_OUTPUT_ACTIVE)) {
		LOG_ERR("LED configure failed");
		return -EIO;
	}

	gpio_pin_set_dt(p_led, false);
	return 0;
}

static int configure_all_leds(void)
{
	if (configure_led(&led_red)) {
		LOG_ERR("RED LED configure failed");
		return -1;
	}
	if (configure_led(&led_blue)) {
		LOG_ERR("BLUE LED configure failed");
		return -1;
	}
	if (configure_led(&led_green)) {
		LOG_ERR("GREEN LED configure failed");
		return -1;
	}
	return 0;
}

static void set_led(struct gpio_dt_spec const *const p_led, bool const state)
{
	if (!p_led->port) {
		return;
	}
	gpio_pin_set_dt(p_led, state);
}

void set_red_led(bool const state)
{
	set_led(&led_red, state);
	set_led(&led_blue, false);
	set_led(&led_green, false);
}

void set_blue_led(bool const state)
{
	set_led(&led_blue, state);
	set_led(&led_red, false);
}

void set_green_led(bool const state)
{
	set_led(&led_green, state);
	set_led(&led_red, false);
}

/* ---------------------------------------------------------------------------------------- */

static uint32_t get_next_free_index(void)
{
	struct peer_data *p_peer;
	sys_snode_t *node = NULL;
	uint32_t index = 0;

	SYS_SLIST_ITERATE_FROM_NODE(&bond_data_contexts, node)
	{
		p_peer = (struct peer_data *)node;
		if (index <= p_peer->storage_index) {
			index = p_peer->storage_index + 1;
		}
	}

	return index;
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

static bool is_bdaddr_valid(gap_bdaddr_t const *const p_addr)
{
	return (!!p_addr && p_addr->addr_type <= GAP_ADDR_RAND);
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

static void *get_found_peer_by_bdaddr(gap_bdaddr_t const *const p_addr)
{
	return get_peer_by_bdaddr(&found_peers_contexts, p_addr);
}

static void *get_bonded_peer_by_bdaddr(gap_bdaddr_t const *const p_addr)
{
	return get_peer_by_bdaddr(&bond_data_contexts, p_addr);
}

static void *get_peer_by_activity_index(uint32_t const actv_idx)
{
	struct peer_data *p_peer;
	sys_snode_t *node = NULL;

	SYS_SLIST_ITERATE_FROM_NODE(&found_peers_contexts, node)
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

	SYS_SLIST_ITERATE_FROM_NODE(&found_peers_contexts, node)
	{
		p_peer = (struct peer_data *)node;
		if (p_peer->conidx == conidx) {
			return p_peer;
		}
	}

	LOG_ERR("Peer not found by connection index %u", conidx);
	return NULL;
}

static void *get_peer_by_connection_index_or_meta(uint32_t const conidx, uint32_t const metainfo)
{
	struct peer_data *p_peer = (struct peer_data *)metainfo;

	if (p_peer) {
		return p_peer;
	}

	return get_peer_by_connection_index(conidx);
}

static void *get_connected_peer_by_connection_index_or_meta(uint32_t const conidx,
							    uint32_t const metainfo)
{
	struct peer_data *p_peer = (struct peer_data *)metainfo;

	if (p_peer) {
		return p_peer;
	}

	sys_snode_t *node = NULL;

	SYS_SLIST_ITERATE_FROM_NODE(&connected_peers_contexts, node)
	{
		p_peer = (struct peer_data *)node;
		if (p_peer->conidx == conidx) {
			return p_peer;
		}
	}

	LOG_ERR("Peer %u not on the connected list", conidx);
	return NULL;
}

static void handle_disconnected_peer(struct peer_data *p_peer, bool const cleanup)
{
	if (!p_peer) {
		return;
	}

	gapc_pairing_keys_t *p_pairing_keys = p_peer->keys.data;

	if (cleanup && p_pairing_keys) {
		memset(&p_pairing_keys->irk.identity, 0, sizeof(p_pairing_keys->irk.identity));
		p_pairing_keys->irk.identity.addr_type = 0xFF;
	}

	p_peer->conidx = GAP_INVALID_CONIDX;

	sys_slist_find_and_remove(&found_peers_contexts, &p_peer->node);
	sys_slist_find_and_remove(&connected_peers_contexts, &p_peer->node);
}

static int handle_found_peer(gap_bdaddr_t const *const p_addr)
{
	struct peer_data *p_peer = calloc(1, sizeof(*p_peer));

	if (!p_peer) {
		LOG_ERR("Failed to allocate memory for a new peer data");
		return -ENOMEM;
	}

	gapc_pairing_keys_t *p_keys = calloc(1, sizeof(*p_keys));

	if (!p_keys) {
		LOG_ERR("Failed to allocate memory for pairing keys");
		free(p_peer);
		return -ENOMEM;
	}

	gapc_bond_data_t *p_bond_data = calloc(1, sizeof(*p_bond_data));

	if (!p_bond_data) {
		LOG_ERR("Failed to allocate memory for bond data keys");
		free(p_bond_data);
		free(p_peer);
		return -ENOMEM;
	}

	p_peer->storage_index = get_next_free_index();
	p_peer->actv_idx = GAP_INVALID_ACTV_IDX;
	p_peer->conidx = GAP_INVALID_CONIDX;
	p_peer->keys.data = p_keys;
	p_peer->keys.size = sizeof(*p_keys);
	p_peer->connection.data = p_bond_data;
	p_peer->connection.size = sizeof(*p_bond_data);

	memcpy(&p_keys->irk.identity, p_addr, sizeof(p_keys->irk.identity));

	sys_slist_append(&found_peers_contexts, &p_peer->node);

	return 0;
}

/* ---------------------------------------------------------------------------------------- */
/* Bluetooth GAPM callbacks */

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
		handle_found_peer(&address);
		num_of_resolves_ongoing--;
		return;
	}

	/* Update address for a connection */
	((gapc_pairing_keys_t *)p_peer->keys.data)->irk.identity = address;

	sys_slist_append(&found_peers_contexts, &p_peer->node);

	num_of_resolves_ongoing--;
}

static gap_sec_key_t *sec_key_list;
static uint8_t sec_key_list_size;

static int resolve_peer_address(const gap_bdaddr_t *const p_peer_addr)
{
	if (!sec_key_list_size || !sec_key_list) {
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
#if CONFIG_BONDING_ALLOWED
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
		LOG_ERR("Peer %u unable to start pairing!", conidx);
		return -1;
	}

#if CONFIG_BONDING_ALLOWED
	LOG_DBG("Peer %u pairing and bonding initiated...", conidx);
#else
	LOG_DBG("Peer %u pairing initiated...", conidx);
#endif
	return 0;
}

static void on_get_peer_version_cmp_cb(uint8_t const conidx, uint32_t const metainfo,
				       uint16_t status, const gapc_version_t *const p_version)
{
	if (status != GAP_ERR_NO_ERROR) {
		LOG_ERR("Peer %u get peer version failed, status:%u. Restart pairing", conidx,
			status);
		request_pairing_and_bonding(conidx);
		return;
	}

	LOG_INF("Peer %u company_id:%u, lmp_subversion:%u, lmp_version:%u", conidx,
		p_version->company_id, p_version->lmp_subversion, p_version->lmp_version);

	struct peer_data *p_peer = get_connected_peer_by_connection_index_or_meta(conidx, metainfo);

	if (!p_peer) {
		return;
	}

	unicast_initiator_discover(conidx, p_peer->storage_index);
}

static void on_peer_features_cmp_cb(uint8_t const conidx, uint32_t const metainfo, uint16_t status,
				    const uint8_t *const p_features)
{
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

	struct peer_data *p_peer = get_peer_by_connection_index_or_meta(conidx, metainfo);

	if (!p_peer) {
		LOG_ERR("Something went wrong. Please reset and try again.");
		return;
	}

	sys_slist_append(&connected_peers_contexts, &p_peer->node);

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
	struct peer_data *p_peer = get_peer_by_connection_index_or_meta(conidx, metainfo);

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
		unicast_initiator_discover(conidx, p_peer->storage_index);
	}

	sys_slist_append(&connected_peers_contexts, &p_peer->node);
}

static void on_gapc_pairing_failed(uint8_t const conidx, uint32_t const metainfo,
				   uint16_t const reason)
{
	LOG_ERR("Peer %u pairing FAILED! Reason %u", conidx, reason);

	struct peer_data *p_peer = get_peer_by_connection_index_or_meta(conidx, metainfo);

	handle_disconnected_peer(p_peer, true);
	k_sem_give(&wait_connection_sem);
}

static void on_gapc_info_req(uint8_t const conidx, uint32_t const metainfo, uint8_t const exp_info)
{
	uint16_t err;
	struct peer_data *p_peer = get_peer_by_connection_index_or_meta(conidx, metainfo);

	if (!p_peer) {
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

	struct peer_data *p_peer = get_peer_by_connection_index_or_meta(conidx, metainfo);

	if (!p_peer) {
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

	struct peer_data *p_peer = get_connected_peer_by_connection_index_or_meta(conidx, metainfo);

	handle_disconnected_peer(p_peer, false);
}

static void on_bond_data_updated(uint8_t const conidx, uint32_t const metainfo,
				 const gapc_bond_data_updated_t *const p_data)
{
	LOG_INF("Peer %u bond data updated: gatt_start_hdl:%u, gatt_end_hdl:%u, "
		"svc_chg_hdl:%u, cli_info:%u, cli_feat:%u, srv_feat:%u",
		conidx, p_data->gatt_start_hdl, p_data->gatt_end_hdl, p_data->svc_chg_hdl,
		p_data->cli_info, p_data->cli_feat, p_data->srv_feat);

	struct peer_data *p_peer = get_connected_peer_by_connection_index_or_meta(conidx, metainfo);

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

	if (!sys_slist_find(&bond_data_contexts, &p_peer->node, NULL)) {
		/* Add to bond data contexts */
		sys_slist_append(&bond_data_contexts, &p_peer->node);
	}
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
		LOG_INF("GAPM activity %s. actv_idx:%u, metainfo: %X",
			proc_id == GAPM_ACTV_START ? "START" : "DELETE", actv_idx, metainfo);
		break;
	}
	default: {
		LOG_ERR("GAPM activity unknown! proc_id:%u, actv_idx:%u", proc_id, actv_idx);
		break;
	}
	}
	ASSERT_INFO(status == GAP_ERR_NO_ERROR, status, 0);
}

static void on_gapm_le_init_stopped(uint32_t const metainfo, uint8_t const actv_idx,
				    uint16_t const reason)
{
	__ASSERT(reason == GAP_ERR_NO_ERROR, "GAPM LE connection init stopped. Reason: %u", reason);

	LOG_INF("GAPM activity stopped. ID:%u, metainfo: %X", actv_idx, metainfo);

	/* Activity context is not needed anymore */
	gapm_delete_activity(actv_idx);

	if (metainfo) {
		struct peer_data *p_peer = (struct peer_data *)metainfo;

		p_peer->actv_idx = GAP_INVALID_ACTV_IDX;
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

	if (!p_appkeys || !is_bdaddr_valid(&p_appkeys->irk.identity)) {
		LOG_ERR("Invalid peer address");
		return -EINVAL;
	}

	/* clang-format off */

	gapm_le_init_param_t const init_params = {
		.prop = (GAPM_INIT_PROP_1M_BIT | GAPM_INIT_PROP_2M_BIT),
		.conn_to = 100, /* Timeout in 10ms units = 1 second */
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
		err = gapm_le_create_init((uint32_t)p_peer, GAPM_STATIC_ADDR, &cbs_gapm_le_init,
					  &p_peer->actv_idx);
		if (err != GAP_ERR_NO_ERROR) {
			LOG_ERR("gapm_le_create_init error %u", err);
			return -1;
		}
	}

	err = gapm_le_start_direct_connection(p_peer->actv_idx, &init_params);
	if (err != GAP_ERR_NO_ERROR) {
		LOG_ERR("gapm_le_start_direct_connection error %u", err);
		return -1;
	}

	extern const char *bdaddr_str(const gap_bdaddr_t *p_addr);
	LOG_INF("Connecting to peer %s, type: %u", bdaddr_str(&init_params.peer_addr),
		init_params.peer_addr.addr_type);

	return 0;
}

static void start_streaming(struct k_work *work)
{
	ARG_UNUSED(work);

	struct peer_data *p_peer;
	sys_snode_t *node = NULL;

	/* Setup streams */
	SYS_SLIST_ITERATE_FROM_NODE(&connected_peers_contexts, node)
	{
		p_peer = (struct peer_data *)node;
		if (p_peer && p_peer->conidx != GAP_INVALID_CONIDX) {
			unicast_setup_streams(p_peer->conidx);
		}
	}

	/* Enable streams */
	SYS_SLIST_ITERATE_FROM_NODE(&connected_peers_contexts, node)
	{
		p_peer = (struct peer_data *)node;
		if (p_peer && p_peer->conidx != GAP_INVALID_CONIDX) {
			unicast_enable_streams(p_peer->conidx);
		}
	}
}

static K_WORK_DEFINE(start_streaming_work, start_streaming);

static void connect_to_peers(struct k_work *work)
{
	ARG_UNUSED(work);

	while (num_of_resolves_ongoing) {
		k_sleep(K_MSEC(500));
	}

	if (!sys_slist_len(&found_peers_contexts)) {
		LOG_ERR("No peers to connect. Restart the device to try again...");
		return;
	}

	LOG_INF("Request connect to peers...");

	struct peer_data *p_peer;
	sys_snode_t *node = NULL;

	SYS_SLIST_ITERATE_FROM_NODE(&found_peers_contexts, node)
	{
		p_peer = (struct peer_data *)node;
		if (p_peer && p_peer->conidx == GAP_INVALID_CONIDX) {
			connect_to_device(p_peer);
			k_sem_take(&wait_connection_sem, K_FOREVER);
		}
	}

	LOG_DBG("All peers connected, starting streaming...");

	k_work_submit(&start_streaming_work);
}

static K_WORK_DEFINE(connect_work, connect_to_peers);

static void scanning_ready_callback(void)
{
	LOG_DBG("Scanning ready - trigger connect...");
	k_work_submit(&connect_work);
}

int peer_found(gap_bdaddr_t const *const p_addr)
{
	if (!is_bdaddr_valid(p_addr)) {
		return -EINVAL;
	}

	struct peer_data *p_peer = get_found_peer_by_bdaddr(p_addr);

	if (p_peer) {
		return -EALREADY;
	}

	if (!resolve_peer_address(p_addr)) {
		/* rest is handled in on_address_resolved_cb */
		return 0;
	}

	p_peer = get_bonded_peer_by_bdaddr(p_addr);
	if (p_peer) {
		/* Don't create a new context. Just add to found peer list */
		sys_slist_append(&found_peers_contexts, &p_peer->node);
		return 0;
	}

	return handle_found_peer(p_addr);
}

int peer_ready(uint32_t const conidx)
{
	ARG_UNUSED(conidx);

	k_sem_give(&wait_connection_sem);
	return 0;
}

void on_disconnect_cmp(uint8_t const conidx, uint32_t const metainfo, uint16_t const status)
{
	struct peer_data *p_peer = get_connected_peer_by_connection_index_or_meta(conidx, metainfo);

	if (p_peer) {
		p_peer->conidx = GAP_INVALID_CONIDX;

		gapc_bond_data_t *p_bond_data = p_peer->connection.data;
		gapc_pairing_keys_t *p_appkeys = p_peer->keys.data;

		if (p_bond_data) {
			memset(p_bond_data, 0, sizeof(*p_bond_data));
			storage_store(SETTINGS_NAME_CONNECTION, p_peer->storage_index, &p_bond_data,
				      sizeof(*p_bond_data));
		}

		if (p_appkeys) {
			memset(p_appkeys, 0, sizeof(*p_appkeys));
			p_appkeys->irk.identity.addr_type = 0xFF;
			storage_store(SETTINGS_NAME_KEYS, p_peer->storage_index, &p_appkeys,
				      sizeof(*p_appkeys));
		}

		handle_disconnected_peer(p_peer, false);
	}

	LOG_WRN("Disconnect completed. Please reset the application and try again.");
}

int terminate_connection(uint32_t const conidx)
{
	struct client_data *p_peer = get_connected_peer_by_connection_index_or_meta(conidx, 0);

	if (!p_peer) {
		return -EINVAL;
	}

	uint32_t const status = gapc_disconnect(conidx, (uint32_t)p_peer,
						LL_ERR_REMOTE_USER_TERM_CON, on_disconnect_cmp);

	return (status == GAP_ERR_NO_ERROR) ? 0 : -ENOEXEC;
}

/* ---------------------------------------------------------------------------------------- */
/**
 * BLE config (GAPM)
 */

static void on_gapm_process_complete(uint32_t const metainfo, uint16_t const status)
{
	if (status != GAP_ERR_NO_ERROR) {
		LOG_ERR("GAPM process completed with error %u", status);
		return;
	}

	switch (metainfo) {
	case 1:
		LOG_DBG("GAPM configured successfully");
		break;
	case 2:
		LOG_DBG("GAPM name set successfully");
		break;
	case 3:
		LOG_DBG("GAPM reset successfully");
		break;
	default:
		LOG_ERR("GAPM Unknown metadata!");
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

static int ble_stack_configure(uint8_t const role)
{
	/* Bluetooth stack configuration*/
	gapm_config_t gapm_cfg = {
		.role = role,
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

#if CONFIG_BONDING_ALLOWED
	/* Configure security level */
	gapm_le_configure_security_level(GAP_SEC1_SEC_CON_PAIR_ENC);
#else
	gapm_le_configure_security_level(GAP_SEC1_NOAUTH_PAIR_ENC);
#endif

	gap_bdaddr_t identity;

	gapm_get_identity(&identity);

	LOG_INF("Device address: %02X:%02X:%02X:%02X:%02X:%02X", identity.addr[5], identity.addr[4],
		identity.addr[3], identity.addr[2], identity.addr[1], identity.addr[0]);

	LOG_DBG("BLE init complete!");

	return 0;
}

/* ---------------------------------------------------------------------------------------- */

void joystick_press(struct k_work *work)
{
	ARG_UNUSED(work);

	if (sys_slist_len(&connected_peers_contexts)) {
		/* Just ignore to avoid scan while connected to peers */
		return;
	}

	unicast_scan_start(scanning_ready_callback);
}
#if !CONFIG_AUTO_START_SCAN
static K_WORK_DEFINE(joystick_press_work, joystick_press);
#endif

void joystick_up(struct k_work *work)
{
	ARG_UNUSED(work);
	unicast_volume_up_all();
}
static K_WORK_DEFINE(joystick_up_work, joystick_up);

void joystick_down(struct k_work *work)
{
	ARG_UNUSED(work);
	unicast_volume_down_all();
}
static K_WORK_DEFINE(joystick_down_work, joystick_down);

#if !DT_NODE_EXISTS(BUTTON_NODELABEL)
#error "Button is mandatory!"
#endif

#include <zephyr/drivers/gpio.h>

struct gpio_data {
	const struct gpio_dt_spec spec;
	struct gpio_callback cb_data;
	struct k_work *execute_func;
	uint32_t last_clicked_ms;
};

#define BUTTON_DEBOUNCE_MS 15

static bool check_debounce(uint32_t *const p_last_time_ms)
{
	uint32_t const current_time_ms = k_uptime_get_32();

	if ((current_time_ms - *p_last_time_ms) < BUTTON_DEBOUNCE_MS) {
		return false;
	}

	*p_last_time_ms = current_time_ms;
	return true;
}

static void gpio_isr_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	struct gpio_data *p_data = CONTAINER_OF(cb, struct gpio_data, cb_data);
	gpio_port_value_t value;

	if (gpio_port_get(dev, &value)) {
		LOG_ERR("Failed to read button state");
		return;
	}

	if (!check_debounce(&p_data->last_clicked_ms)) {
		return;
	}

	if (value & pins) {
		/* ignore invalid state */
		return;
	}

	if (!p_data->execute_func) {
		return;
	}

	k_work_submit(p_data->execute_func);
}

static int configure_button(struct gpio_data *p_button)
{
	if (!p_button->spec.port) {
		LOG_ERR("Button is not valid!");
		return -EEXIST;
	}

	/* Configure button */
	if (!gpio_is_ready_dt(&p_button->spec)) {
		LOG_ERR("Button is not ready");
		return -EEXIST;
	}

	if (gpio_pin_configure_dt(&p_button->spec, GPIO_INPUT)) {
		LOG_ERR("Button configure failed");
		return -EIO;
	}

	if (gpio_pin_interrupt_configure_dt(&p_button->spec, GPIO_INT_EDGE_FALLING)) {
		LOG_ERR("button int conf failed");
		return -EIO;
	}

	gpio_init_callback(&p_button->cb_data, gpio_isr_handler, BIT(p_button->spec.pin));
	if (gpio_add_callback(p_button->spec.port, &p_button->cb_data)) {
		LOG_ERR("cb add failed");
		return -EIO;
	}

	return 0;
}

static struct gpio_data joystick_conf[] = {
#if !CONFIG_AUTO_START_SCAN
	{.spec = GPIO_DT_SPEC_GET_OR(BUTTON_NODELABEL, gpios, {0}),
	 .execute_func = &joystick_press_work},
#endif
	{.spec = GPIO_DT_SPEC_GET_OR(DT_NODELABEL(button_up), gpios, {0}),
	 .execute_func = &joystick_up_work},
	{.spec = GPIO_DT_SPEC_GET_OR(DT_NODELABEL(button_down), gpios, {0}),
	 .execute_func = &joystick_down_work},
};

/* ---------------------------------------------------------------------------------------- */

static int storage_load_bond_data(void)
{
	if (storage_init() < 0) {
		return -1;
	}

	storage_load_all(&bond_data_contexts);

	/* Build known peers list */
	sec_key_list_size = sys_slist_len(&bond_data_contexts);
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

/* ---------------------------------------------------------------------------------------- */
/* Application entry point */

int main(void)
{
	int ret;

	sys_slist_init(&found_peers_contexts);
	sys_slist_init(&connected_peers_contexts);
	sys_slist_init(&bond_data_contexts);

	if (storage_load_bond_data() < 0) {
		return -1;
	}

	ret = alif_ble_enable(NULL);
	if (ret) {
		LOG_ERR("Failed to enable bluetooth, err %d", ret);
		return ret;
	}

	LOG_DBG("BLE enabled");

	if (ble_stack_configure(GAP_ROLE_LE_CENTRAL)) {
		return -1;
	}

	ret = unicast_initiator_configure();
	if (ret) {
		return ret;
	}

	ret = unicast_scan_configure();
	if (ret) {
		return ret;
	}

	for (size_t iter = 0; iter < ARRAY_SIZE(joystick_conf); iter++) {
		ret = configure_button(&joystick_conf[iter]);
		if (ret) {
			return ret;
		}
	}

	ret = configure_all_leds();
	if (ret) {
		return ret;
	}

	set_red_led(true);

	LOG_INF("-----------------------------------");
	LOG_INF("Alif Unicast Initiator app started");
	LOG_INF("   Press button to start scan");
	LOG_INF("-----------------------------------");

#if CONFIG_AUTO_START_SCAN
	/* auto start */
	joystick_press(NULL);
#endif

	return 0;
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

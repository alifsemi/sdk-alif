/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/*
 * This example will start an instance of a peripheral CGMS and send
 * periodic notification updates to the first device that connects to it.
 * Includes Battery Service support
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

/*  Profile definitions */
#include "prf.h"
#include "prf_types.h"
#include "rwprf_config.h"
#include "batt_svc.h"
#include "shared_control.h"
#include "cgms_app.h"

#include "gapc_msg.h"
#include "address_verification.h"

#include "se_service.h"
#include <zephyr/settings/settings.h>
#include <string.h>
#include <alif/bluetooth/bt_adv_data.h>
#include <alif/bluetooth/bt_scan_rsp.h>
#include "gapm_api.h"

/* Define advertising address type */
#define SAMPLE_ADDR_TYPE	ALIF_GEN_RSLV_RAND_ADDR

#define BLE_BOND_KEYS_KEY_0	"ble/bond_keys_0"
#define BLE_BOND_KEYS_NAME_0	"bond_keys_0"
#define BLE_BOND_DATA_KEY_0	"ble/bond_data_0"
#define BLE_BOND_DATA_NAME_0	"bond_data_0"

/* Device definitions */
/* Load name from configuration file */
#define DEVICE_NAME CONFIG_BLE_DEVICE_NAME

/* Store and share advertising address type */
static uint8_t adv_type;

/* State variables for BLE connection and services */
static bool connected;
static bool resolved;
static gapc_pairing_keys_t stored_keys;
static gapc_pairing_keys_t generated_keys;
static gapc_bond_data_t bond_data_saved;
static uint8_t temp_conidx;

/* Store advertising activity index for re-starting after disconnection */
static uint8_t adv_actv_idx;


/* Semaphores definition */
K_SEM_DEFINE(init_sem, 0, 1);

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

/* Exposed functions */
static int keys_retrieve(void);
static struct shared_control ctrl = {false, 0, 0};

/**
 * Bluetooth stack configuration
 */

static gapm_config_t gapm_cfg = {
	.role = GAP_ROLE_LE_PERIPHERAL,
	.pairing_mode = GAPM_PAIRING_MODE_ALL,
	.privacy_cfg = GAPM_PRIV_CFG_PRIV_ADDR_BIT,
	.renew_dur = 1500,
	.private_identity.addr = {0x78, 0x59, 0x94, 0xDE, 0x11, 0xFF},
	.irk.key = {0xA1, 0xB2, 0xC3, 0xD4, 0xE5, 0xF6, 0x07, 0x08, 0x11,
			0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88},
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

static gapc_pairing_t p_pairing_info = {
	.auth = GAP_AUTH_BOND | GAP_AUTH_SEC_CON | GAP_AUTH_MITM,
	.ikey_dist = GAP_KDIST_ENCKEY | GAP_KDIST_IDKEY,
	.iocap = GAP_IO_CAP_DISPLAY_ONLY,
	.key_size = 16,
	.oob = GAP_OOB_AUTH_DATA_NOT_PRESENT,
	.rkey_dist = GAP_KDIST_ENCKEY | GAP_KDIST_IDKEY,
};


void on_address_resolved_cb(uint16_t status, const gap_addr_t *p_addr, const gap_sec_key_t *pirk)
{
	resolved = (status != GAP_ERR_NO_ERROR) ? false : true;

	if (resolved) {
		LOG_INF("Known peer device");
		gapc_le_connection_cfm(temp_conidx, 0, &(bond_data_saved));
	} else {
		LOG_INF("Unknown peer device");
		gapc_le_connection_cfm(temp_conidx, 0, NULL);
	}
}


/**
 * Bluetooth GAPM callbacks
 */
static void on_le_connection_req(uint8_t conidx, uint32_t metainfo, uint8_t actv_idx, uint8_t role,
				 const gap_bdaddr_t *p_peer_addr,
				 const gapc_le_con_param_t *p_con_params, uint8_t clk_accuracy)
{
	/* Number of IRKs */
	uint8_t nb_irk = 1;

	LOG_DBG("Connection request on index %u", conidx);

	LOG_INF("Peer BD address %02X:%02X:%02X:%02X:%02X:%02X (conidx: %u)", p_peer_addr->addr[5],
		p_peer_addr->addr[4], p_peer_addr->addr[3], p_peer_addr->addr[2],
		p_peer_addr->addr[1], p_peer_addr->addr[0], conidx);

	LOG_INF("Peer address type: %s", (p_peer_addr->addr_type == 1) ? "private" : "public");
	temp_conidx = conidx;

	/* Resolve Address */
	gapm_le_resolve_address((gap_addr_t *)p_peer_addr->addr, nb_irk, &(stored_keys.irk.key),
		on_address_resolved_cb);
	LOG_INF("Connection parameters: interval %u, latency %u, supervision timeout %u",
		p_con_params->interval, p_con_params->latency, p_con_params->sup_to);

	connected = true;
	ctrl.connected = true;

	addr_res_done();
	LOG_INF("Please enable notifications on peer device..");
}

static const gapc_connection_req_cb_t gapc_con_cbs = {
	.le_connection_req = on_le_connection_req,
};


static void on_disconnection(uint8_t conidx, uint32_t metainfo, uint16_t reason)
{
	uint16_t err;

	LOG_INF("Connection index %u disconnected for reason %u", conidx, reason);
	err = bt_gapm_advertisement_continue(conidx);
	if (err) {
		LOG_ERR("Error restarting advertising: %u", err);
	} else {
		LOG_DBG("Restarting advertising");
	}

	connected = false;
	resolved = false;
	ctrl.connected = false;

	disc_notify(reason);
}

static void on_name_get(uint8_t conidx, uint32_t metainfo, uint16_t token, uint16_t offset,
			uint16_t max_len)
{
	LOG_WRN("Received unexpected name get from conidx: %u", conidx);
}

static void on_appearance_get(uint8_t conidx, uint32_t metainfo, uint16_t token)
{
	/* Send 'unknown' appearance */
	gapc_le_get_appearance_cfm(conidx, token, GAP_ERR_NO_ERROR, 0);
}

static const gapc_connection_info_cb_t gapc_con_inf_cbs = {
	.disconnected = on_disconnection,
	.name_get = on_name_get,
	.appearance_get = on_appearance_get,
	/* Other callbacks in this struct are optional */
};

/*
 * Security callbacks
 */
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

	/* Save under the key "ble/bond_keys_0" */
	err = settings_save_one(BLE_BOND_KEYS_KEY_0, &stored_keys, sizeof(gapc_pairing_keys_t));
	if (err) {
		LOG_ERR("Failed to store test_data (err %d)", err);
	}
}

static void on_pairing_req(uint8_t conidx, uint32_t metainfo, uint8_t auth_level)
{
	uint16_t err;

	err = gapc_le_pairing_accept(conidx, true, &p_pairing_info, 0);

	if (err != GAP_ERR_NO_ERROR) {
		LOG_ERR("Pairing error %u", err);
	}
}

static void on_pairing_failed(uint8_t conidx, uint32_t metainfo, uint16_t reason)
{
	LOG_DBG("Pairing failed conidx: %u, metainfo: %u, reason: 0x%02x\n",
		conidx, metainfo, reason);
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

	LOG_INF("PAIRING SUCCEED");

	bond_data_saved.pairing_lvl = pairing_level;
	bond_data_saved.enc_key_present = true;

	err = settings_save_one(BLE_BOND_DATA_KEY_0, &bond_data_saved, sizeof(gapc_bond_data_t));
	if (err) {
		LOG_ERR("Failed to store test_data (err %d)", err);
	}

	/* Verify bond */
	bool bonded = gapc_is_bonded(conidx);
	if (bonded) {
		LOG_INF("Peer device bonded");
	}
}

static void on_info_req(uint8_t conidx, uint32_t metainfo, uint8_t exp_info)
{
	uint16_t err;

	switch (exp_info) {
	case GAPC_INFO_IRK:
	{
		err = gapc_le_pairing_provide_irk(conidx, &(gapm_cfg.irk));
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

	for (cnt = 0; cnt < RAND_NB_LEN; cnt++)	{
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
};

/* All callbacks in this struct are optional */
static const gapc_le_config_cb_t gapc_le_cfg_cbs;

/*
 * Callbacks assignment
 */

const gapm_callbacks_t get_cbs(void)
{
	gapm_callbacks_t ret = {
		.p_con_req_cbs = &gapc_con_cbs,
		.p_sec_cbs = &gapc_sec_cbs,
		.p_info_cbs = &gapc_con_inf_cbs,
		.p_le_config_cbs = &gapc_le_cfg_cbs,
		.p_bt_config_cbs = NULL /* BT classic so not required */
	};
	ret = append_cbs(&ret);

	return ret;
}
/*
 * Advertising functions
 */

static uint16_t set_advertising_data(uint8_t actv_idx)
{
	int ret;
	uint16_t svc[2];

	/* gatt service identifier */
	svc[0] = GATT_SVC_CONTINUOUS_GLUCOSE_MONITORING;
	svc[1] = get_batt_id();

	ret = bt_adv_data_set_tlv(GAP_AD_TYPE_COMPLETE_LIST_16_BIT_UUID, svc, sizeof(svc));
	if (ret) {
		LOG_ERR("AD profile set fail %d", ret);
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

uint16_t read_sensor_value(uint16_t current_value)
{
	/* Generating dummy values between 70 and 130 */
	if (current_value >= 130) {
		current_value = 70;
	} else {
		current_value++;
	}
	return current_value;
}

static int keys_settings_set(const char *name, size_t len_rd,
			settings_read_cb read_cb, void *cb_arg)
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

static int keys_retrieve(void)
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
	}

	err = settings_load();
	if (err) {
		LOG_ERR("settings_load() failed, err %d", err);
	}

	return err;
}

int main(void)
{
	uint16_t err;
	gapm_callbacks_t gapm_cbs;
	uint16_t current_value = 70;

	/* Start up bluetooth host stack */
	alif_ble_enable(NULL);

	gapm_cbs = get_cbs();

	if (address_verification(SAMPLE_ADDR_TYPE, &adv_type, &gapm_cfg)) {
		LOG_ERR("Address verification failed");
		return -EADV;
	}

	/* Configure Bluetooth Stack */
	LOG_INF("Init gapm service");
	err = bt_gapm_init(&gapm_cfg, &gapm_cbs, DEVICE_NAME, strlen(DEVICE_NAME));
	if (err) {
		LOG_ERR("gapm_configure error %u", err);
		return -1;
	}

	/* Share connection info */
	service_conn(&ctrl);

	/* Adding battery service */
	config_battery_service();

	server_configure();

	keys_retrieve();

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
		/* Execute process every 1 second */
		k_sleep(K_SECONDS(1));

		current_value = read_sensor_value(current_value);

		cgms_process(current_value);
		battery_process();
	}
	return -ENOTSUP;
}

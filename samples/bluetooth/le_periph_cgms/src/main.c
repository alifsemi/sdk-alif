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
#include "cgmp_common.h"
#include "cgms.h"
#include "cgms_msg.h"
#include "prf_types.h"
#include "rwprf_config.h"

#include "bass.h"
#include "bas.h"
#include "gapc_msg.h"

#include "se_service.h"
#include <zephyr/settings/settings.h>
#include <string.h>

#define BATT_INSTANCE 0x00
#define BLE_BOND_KEYS_KEY_0	"ble/bond_keys_0"
#define BLE_BOND_KEYS_NAME_0	"bond_keys_0"
#define BLE_BOND_DATA_KEY_0	"ble/bond_data_0"
#define BLE_BOND_DATA_NAME_0	"bond_data_0"

/* Device definitions */
/* Load name from configuration file */
#define DEVICE_NAME CONFIG_BLE_DEVICE_NAME
static const char device_name[] = DEVICE_NAME;

/* BLE definitions */
#define local_sec_level GAP_SEC1_AUTH_PAIR_ENC

/* State variables for BLE connection and services */
static bool READY_TO_SEND;
static bool READY_TO_SEND_BASS;
static bool connected;
static bool resolved;
static gapc_pairing_keys_t stored_keys;
static gapc_pairing_keys_t generated_keys;
static gapc_bond_data_t bond_data_saved;
static uint8_t temp_conidx;
/* Store advertising activity index for re-starting after disconnection */
static uint8_t adv_actv_idx;

static cgm_status_t cgms_status;

/* Semaphores definition */
K_SEM_DEFINE(init_sem, 0, 1);
K_SEM_DEFINE(conn_sem, 0, 1);

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

/* Exposed functions */
static uint16_t start_le_adv(uint8_t actv_idx);

/**
 * Bluetooth stack configuration
 */

static const gapm_config_t gapm_cfg = {
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

	/* Continue app */
	k_sem_give(&conn_sem);
	LOG_INF("Please enable notifications on peer device..");
}

static const gapc_connection_req_cb_t gapc_con_cbs = {
	.le_connection_req = on_le_connection_req,
};


static void on_disconnection(uint8_t conidx, uint32_t metainfo, uint16_t reason)
{
	uint16_t err;

	LOG_INF("Connection index %u disconnected for reason %u", conidx, reason);
	err = start_le_adv(adv_actv_idx);
	if (err) {
		LOG_ERR("Error restarting advertising: %u", err);
	} else {
		LOG_DBG("Restarting advertising");
	}

	connected = false;
	READY_TO_SEND = false;
	resolved = false;
}

static void on_name_get(uint8_t conidx, uint32_t metainfo, uint16_t token, uint16_t offset,
			uint16_t max_len)
{
	uint16_t err;
	const size_t device_name_len = sizeof(device_name) - 1;
	const size_t short_len = (device_name_len > max_len ? max_len : device_name_len);

	err = gapc_le_get_name_cfm(conidx, token, GAP_ERR_NO_ERROR, device_name_len, short_len,
			     (const uint8_t *)device_name);

	if (err) {
		LOG_ERR("ERROR ON GET NAME CFM 0x%02x", err);
	}
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
 * CGMS callbacks
 */

static void on_cgms_meas_send_complete(uint8_t conidx, uint16_t status)
{
	READY_TO_SEND = true;
}

static void on_bond_data_upd(uint8_t conidx, uint8_t char_code, uint16_t cfg_val)
{
	switch (cfg_val) {
	case PRF_CLI_STOP_NTFIND:
		LOG_INF("Client requested stop notification/indication (conidx: %u)", conidx);
		READY_TO_SEND = false;
		break;
	case PRF_CLI_START_NTF:
		LOG_INF("Client requested start notification/indication (conidx: %u)", conidx);
			READY_TO_SEND = true;
			LOG_INF("Sending measurements");
		break;

	case PRF_CLI_START_IND:
	default:
		break;
	}
}

static void on_rd_status_req(uint8_t conidx, uint32_t token)
{
	uint16_t status = 0;
	uint16_t err = cgms_rd_status_cfm(conidx, token, status, &(cgms_status));

	if (err) {
		LOG_ERR(" Error sending status 0x%04x", err);
	}
}

static void on_re_sess_start_time_req(uint8_t conidx, uint32_t token)
{
}

static void on_rd_sess_run_time_req(uint8_t conidx, uint32_t token)
{
}

static void on_sess_start_time_upd(uint8_t conidx, const cgm_sess_start_time_t *p_sess_start_time)
{
}

static void on_racp_req(uint8_t conidx, uint8_t op_code, uint8_t func_operator, uint8_t filter_type,
			uint16_t min_time_offset, uint16_t max_time_offset)
{
}

static void on_racp_rsp_send_cmp(uint8_t conidx, uint16_t status)
{
}

static void on_ops_ctrl_pt_req(uint8_t conidx, uint8_t op_code,
			       const union cgm_ops_operand *p_operand)
{
}

static void on_ops_ctrl_pt_rsp_send_cmp(uint8_t conidx, uint16_t status)
{
}

static const cgms_cb_t cgms_cb = {
	.cb_meas_send_cmp = on_cgms_meas_send_complete,
	.cb_bond_data_upd = on_bond_data_upd,
	.cb_rd_status_req = on_rd_status_req,
	.cb_rd_sess_start_time_req = on_re_sess_start_time_req,
	.cb_rd_sess_run_time_req = on_rd_sess_run_time_req,
	.cb_sess_start_time_upd = on_sess_start_time_upd,
	.cb_racp_req = on_racp_req,
	.cb_racp_rsp_send_cmp = on_racp_rsp_send_cmp,
	.cb_ops_ctrl_pt_req = on_ops_ctrl_pt_req,
	.cb_ops_ctrl_pt_rsp_send_cmp = on_ops_ctrl_pt_rsp_send_cmp,
};

/*
 * Battery service callbacks
 */
static void on_bass_batt_level_upd_cmp(uint16_t status)
{
	READY_TO_SEND_BASS = true;
}

static void on_bass_bond_data_upd(uint8_t conidx, uint8_t ntf_ind_cfg)
{
	switch (ntf_ind_cfg) {
	case PRF_CLI_STOP_NTFIND:
		LOG_INF("Client requested BASS stop notification/indication (conidx: %u)", conidx);
		READY_TO_SEND_BASS = false;
		break;
	case PRF_CLI_START_NTF:
	case PRF_CLI_START_IND:
		LOG_INF("Client requested BASS start notification/indication (conidx: %u)", conidx);
		READY_TO_SEND_BASS = true;
		LOG_DBG("Sending battery level");
		break;
	default:
		break;
	}
}

static const bass_cb_t bass_cb = {
	.cb_batt_level_upd_cmp = on_bass_batt_level_upd_cmp,
	.cb_bond_data_upd = on_bass_bond_data_upd,
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
#if !CONFIG_ALIF_BLE_ROM_IMAGE_V1_0 /* ROM version > 1.0 */
static void on_gapm_err(uint32_t metainfo, uint8_t code)
{
	LOG_ERR("gapm error %d", code);
}
static const gapm_cb_t gapm_err_cbs = {
	.cb_hw_error = on_gapm_err,
};

static const gapm_callbacks_t gapm_cbs = {
	.p_con_req_cbs = &gapc_con_cbs,
	.p_sec_cbs = &gapc_sec_cbs,
	.p_info_cbs = &gapc_con_inf_cbs,
	.p_le_config_cbs = &gapc_le_cfg_cbs,
	.p_bt_config_cbs = NULL, /* BT classic so not required */
	.p_gapm_cbs = &gapm_err_cbs,
};
#else
static void on_gapm_err(enum co_error err)
{
	LOG_ERR("gapm error %d", err);
}
static const gapm_err_info_config_cb_t gapm_err_cbs = {
	.ctrl_hw_error = on_gapm_err,
};

static const gapm_callbacks_t gapm_cbs = {
	.p_con_req_cbs = &gapc_con_cbs,
	.p_sec_cbs = &gapc_sec_cbs,
	.p_info_cbs = &gapc_con_inf_cbs,
	.p_le_config_cbs = &gapc_le_cfg_cbs,
	.p_bt_config_cbs = NULL, /* BT classic so not required */
	.p_err_info_config_cbs = &gapm_err_cbs,
};
#endif /* !CONFIG_ALIF_BLE_ROM_IMAGE_V1_0 */

/*
 * Advertising functions
 */

static uint16_t set_advertising_data(uint8_t actv_idx)
{
	uint16_t err;
	uint8_t num_svc = 2;

	/* gatt service identifier */
	uint16_t svc = GATT_SVC_CONTINUOUS_GLUCOSE_MONITORING;
	uint16_t svc2 = GATT_SVC_BATTERY_SERVICE;

	const size_t device_name_len = sizeof(device_name) - 1;
	const uint16_t adv_device_name = GATT_HANDLE_LEN + device_name_len;
	const uint16_t adv_uuid_svc = GATT_HANDLE_LEN + (GATT_UUID_16_LEN * num_svc);

	/* Create advertising data with necessary services */
	const uint16_t adv_len = adv_device_name + adv_uuid_svc;

	co_buf_t *p_buf;

	err = co_buf_alloc(&p_buf, 0, adv_len, 0);
	__ASSERT(err == 0, "Buffer allocation failed");

	uint8_t *p_data = co_buf_data(p_buf);

	p_data[0] = device_name_len + 1;
	p_data[1] = GAP_AD_TYPE_COMPLETE_NAME;
	memcpy(p_data + 2, device_name, device_name_len);

	/* Update data pointer */
	p_data = p_data + adv_device_name;
	p_data[0] = (GATT_UUID_16_LEN * num_svc) + 1;
	p_data[1] = GAP_AD_TYPE_COMPLETE_LIST_16_BIT_UUID;

	/* Copy identifier */
	memcpy(p_data + 2, (void *)&svc, sizeof(svc));
	memcpy(p_data + 4, (void *)&svc2, sizeof(svc2));

	err = gapm_le_set_adv_data(actv_idx, p_buf);
	co_buf_release(p_buf); /* Release ownership of buffer so stack can free it when done */
	if (err) {
		LOG_ERR("Failed to set advertising data with error %u", err);
	}

	return err;
}

static uint16_t set_scan_data(uint8_t actv_idx)
{
	co_buf_t *p_buf;
	uint16_t err = co_buf_alloc(&p_buf, 0, 0, 0);

	__ASSERT(err == 0, "Buffer allocation failed");

	err = gapm_le_set_scan_response_data(actv_idx, p_buf);
	co_buf_release(p_buf); /* Release ownership of buffer so stack can free it when done */
	if (err) {
		LOG_ERR("Failed to set scan data with error %u", err);
	}

	return err;
}

static uint16_t start_le_adv(uint8_t actv_idx)
{
	uint16_t err;
	gapm_le_adv_param_t adv_params = {
		.duration = 0, /* Advertise indefinitely */
	};

	err = gapm_le_start_adv(actv_idx, &adv_params);
	if (err) {
		LOG_ERR("Failed to start LE advertising with error %u", err);
	}

	return err;
}


/**
 * Advertising callbacks
 */
static void on_adv_actv_stopped(uint32_t metainfo, uint8_t actv_idx, uint16_t reason)
{
	LOG_DBG("Advertising activity index %u stopped for reason %u", actv_idx, reason);
}

static void on_adv_actv_proc_cmp(uint32_t metainfo, uint8_t proc_id, uint8_t actv_idx,
				 uint16_t status)
{
	if (status) {
		LOG_ERR("Advertising activity process completed with error %u", status);
		return;
	}

	switch (proc_id) {
	case GAPM_ACTV_CREATE_LE_ADV:
		LOG_DBG("Advertising activity is created");
		adv_actv_idx = actv_idx;
		set_advertising_data(actv_idx);
		break;

	case GAPM_ACTV_SET_ADV_DATA:
		LOG_DBG("Advertising data is set");
		set_scan_data(actv_idx);
		break;

	case GAPM_ACTV_SET_SCAN_RSP_DATA:
		LOG_DBG("Scan data is set");
		start_le_adv(actv_idx);
		break;

	case GAPM_ACTV_START:
		LOG_DBG("Advertising was started");
		k_sem_give(&init_sem);
		break;

	default:
		LOG_WRN("Unexpected GAPM activity complete, proc_id %u", proc_id);
		break;
	}
}

static void on_adv_created(uint32_t metainfo, uint8_t actv_idx, int8_t tx_pwr)
{
	LOG_DBG("Advertising activity created, index %u, selected tx power %d", actv_idx, tx_pwr);
}

static const gapm_le_adv_cb_actv_t le_adv_cbs = {
	.hdr.actv.stopped = on_adv_actv_stopped,
	.hdr.actv.proc_cmp = on_adv_actv_proc_cmp,
	.created = on_adv_created,
};

static uint16_t create_advertising(void)
{
	uint16_t err;

	gapm_le_adv_create_param_t adv_create_params = {
		.prop = GAPM_ADV_PROP_UNDIR_CONN_MASK,
		.disc_mode = GAPM_ADV_MODE_GEN_DISC,
		.max_tx_pwr = 0,
		.filter_pol = GAPM_ADV_ALLOW_SCAN_ANY_CON_ANY,
		.prim_cfg = {
				.adv_intv_min = 160, /* 100 ms */
				.adv_intv_max = 800, /* 500 ms */
				.ch_map = ADV_ALL_CHNLS_EN,
				.phy = GAPM_PHY_TYPE_LE_1M,
			},
	};

	err = gapm_le_create_adv_legacy(0, GAPM_STATIC_ADDR, &adv_create_params, &le_adv_cbs);
	if (err) {
		LOG_ERR("Error %u creating advertising activity", err);
	}

	return err;
}

/* Add heart rate profile to the stack */
static void server_configure(void)
{
	uint16_t err;
	uint16_t start_hdl = 0;
	struct cgms_db_cfg cgms_cfg;

	cgms_cfg.cgm_feature = CGM_FEAT_HYPO_ALERT_SUP_BIT | CGM_FEAT_SENSOR_MALFUNC_DETEC_SUP_BIT;
	cgms_cfg.type_sample = CGM_TYPE_SMP_CAPILLARY_WHOLE_BLOOD;
	cgms_cfg.sample_location = CGM_SMP_LOC_FINGER;

	err = prf_add_profile(TASK_ID_CGMS, local_sec_level, 0, &cgms_cfg, &cgms_cb, &start_hdl);

	if (err) {
		LOG_ERR("Error %u adding profile", err);
	}
}

void on_gapm_process_complete(uint32_t metainfo, uint16_t status)
{
	if (status) {
		LOG_ERR("gapm process completed with error %u", status);
		return;
	}

	server_configure();

	LOG_DBG("gapm process completed successfully");

	/* After configuration completed, create an advertising activity */
	create_advertising();
}

/*  Generate and send dummy data*/
static void send_measurement(uint16_t current_value)
{
	uint16_t err;
	/* Dummy measurement data */
	cgm_meas_value_t p_meas = {
		.flags = CGM_MEAS_FLAGS_CGM_TREND_INFO_BIT | CGM_MEAS_FLAGS_CGM_QUALITY_BIT,
		.gluc_concent = current_value - 20,
		.time_offset = current_value - 69,
		.warn = 0,
		.cal_temp = 0,
		.sensor_status = CGM_MEAS_ANNUNC_STATUS_DEV_BATT_LOW_BIT,
		.trend_info = current_value - 50,
	};

	/* Send measurement to connected device */
	/* Set 0 to first parameter to send only to the first connected peer device */
	err = cgms_meas_send(0, &p_meas);

	if (err) {
		LOG_ERR("Error %u sending measurement", err);
	}
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

void cgms_process(uint16_t measurement)
{
	if (connected && READY_TO_SEND) {
		send_measurement(measurement);
		READY_TO_SEND = false;
	} else if (!connected) {
		LOG_DBG("Waiting for peer connection...\n");
		k_sem_take(&conn_sem, K_FOREVER);
	}
}

static void config_battery_service(void)
{
	uint16_t err;
	struct bass_db_cfg bass_cfg;
	uint16_t start_hdl = 0;

	bass_cfg.bas_nb = 1;
	bass_cfg.features[0] = 1;

	err = prf_add_profile(TASK_ID_BASS, 0, 0, &bass_cfg, &bass_cb, &start_hdl);
}

static void battery_process(void)
{
	uint16_t err;
	/* Fixed value for demonstrating purposes */
	uint8_t battery_level = 99;

	if (READY_TO_SEND_BASS) {
		/* Sending dummy battery level to first battery instance*/
		err = bass_batt_level_upd(BATT_INSTANCE, battery_level);

		if (err) {
			LOG_ERR("Error %u sending battery level", err);
		}
	}
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
	uint16_t current_value = 70;

	/* Start up bluetooth host stack */
	alif_ble_enable(NULL);

	err = gapm_configure(0, &gapm_cfg, &gapm_cbs, on_gapm_process_complete);
	if (err) {
		LOG_ERR("gapm_configure error %02x", err);
		return err;
	}

	config_battery_service();

	LOG_DBG("Waiting for init...\n");
	k_sem_take(&init_sem, K_FOREVER);

	LOG_DBG("Init complete!");

	/* keys retrieve */
	keys_retrieve();

	while (1) {
		/* Execute process every 1 second */
		k_sleep(K_SECONDS(1));

		current_value = read_sensor_value(current_value);

		cgms_process(current_value);
		battery_process();
	}
	return -ENOTSUP;
}

/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/*
 * This example will start an instance of a peripheral Proximity Profile (PRXP)
 * it will notify when the link has been lost with the alert level set from the
 * central device. Includes Battery Service support.
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


/*  Profiles definitions */
#include "batt_svc.h"
#include "shared_control.h"
#if !CONFIG_ALIF_BLE_ROM_IMAGE_V1_0 /* ROM version > 1.0 */
#include "llss.h"
#include "iass.h"
#include "tpss.h"
#else
#include "prf.h"
#include "proxr.h"
#include "proxr_msg.h"
#include "bass.h"
#endif /* !CONFIG_ALIF_BLE_ROM_IMAGE_V1_0 */


K_SEM_DEFINE(init_sem, 0, 1);

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

/* Bluetooth stack configuration */
static const gapm_config_t gapm_cfg = {
	.role = GAP_ROLE_LE_PERIPHERAL,
	.pairing_mode = GAPM_PAIRING_DISABLE,
	.privacy_cfg = 0,
	.renew_dur = 1500,
	.private_identity.addr = {0, 0, 0, 0, 0, 0},
	.irk.key = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	.gap_start_hdl = 0,
	.gatt_start_hdl = 0,
	.att_cfg = 0,
	.sugg_max_tx_octets = GAP_LE_MIN_OCTETS,
	.sugg_max_tx_time = GAP_LE_MIN_TIME,
	.tx_pref_phy = GAP_PHY_ANY,
	.rx_pref_phy = GAP_PHY_ANY,
	.tx_path_comp = 0,
	.rx_path_comp = 0,
};

static struct shared_control ctrl = { false, 0, 0 };

#if !CONFIG_ALIF_BLE_ROM_IMAGE_V1_0 /* ROM version > 1.0 */
void ll_notify(void);
void ias_reset(void);
#endif /* !CONFIG_ALIF_BLE_ROM_IMAGE_V1_0 */

/* Load name from configuration file */
#define DEVICE_NAME CONFIG_BLE_DEVICE_NAME
static const char device_name[] = DEVICE_NAME;

static uint8_t adv_actv_idx;

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
 * Bluetooth GAPM callbacks
 */
static void on_le_connection_req(uint8_t conidx, uint32_t metainfo, uint8_t actv_idx, uint8_t role,
				 const gap_bdaddr_t *p_peer_addr,
				 const gapc_le_con_param_t *p_con_params, uint8_t clk_accuracy)
{
	LOG_INF("Connection request on index %u", conidx);
	gapc_le_connection_cfm(conidx, 0, NULL);

	LOG_DBG("Connection parameters: interval %u, latency %u, supervision timeout %u",
		p_con_params->interval, p_con_params->latency, p_con_params->sup_to);

	LOG_HEXDUMP_DBG(p_peer_addr->addr, GAP_BD_ADDR_LEN, "Peer BD address");

	LOG_DBG("Please enable notifications on peer device..");

	ctrl.connected = true;

}

static void on_key_received(uint8_t conidx, uint32_t metainfo, const gapc_pairing_keys_t *p_keys)
{
	LOG_WRN("Unexpected key received key on conidx %u", conidx);
}

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

#if !CONFIG_ALIF_BLE_ROM_IMAGE_V1_0 /* ROM version > 1.0 */
	ll_notify();
	ias_reset();
	/* Update shared control */
	ctrl.connected = false;
#endif /* !CONFIG_ALIF_BLE_ROM_IMAGE_V1_0 */
}

static void on_name_get(uint8_t conidx, uint32_t metainfo, uint16_t token, uint16_t offset,
			uint16_t max_len)
{
	const size_t device_name_len = sizeof(device_name) - 1;
	const size_t short_len = (device_name_len > max_len ? max_len : device_name_len);

	gapc_le_get_name_cfm(conidx, token, GAP_ERR_NO_ERROR, device_name_len, short_len,
			     (const uint8_t *)device_name);
}

static void on_appearance_get(uint8_t conidx, uint32_t metainfo, uint16_t token)
{
	/* Send unknown appearance */
	gapc_le_get_appearance_cfm(conidx, token, GAP_ERR_NO_ERROR, 0);
}

static const gapc_connection_req_cb_t gapc_con_cbs = {
	.le_connection_req = on_le_connection_req,
};

static const gapc_security_cb_t gapc_sec_cbs = {
	.key_received = on_key_received,
	/* All other callbacks in this struct are optional */
};

static const gapc_connection_info_cb_t gapc_con_inf_cbs = {
	.disconnected = on_disconnection,
	.name_get = on_name_get,
	.appearance_get = on_appearance_get,
	/* Other callbacks in this struct are optional */
};

/* All callbacks in this struct are optional */
static const gapc_le_config_cb_t gapc_le_cfg_cbs;

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

static uint16_t set_advertising_data(uint8_t actv_idx)
{
	uint16_t err;
	uint16_t svc;

	/* gatt service identifier */
	svc = GATT_SVC_LINK_LOSS;
#if !CONFIG_ALIF_BLE_ROM_IMAGE_V1_0 /* ROM version > 1.0 */
	uint16_t svc2 = GATT_SVC_BATTERY;
#else
	uint16_t svc2 = GATT_SVC_BATTERY_SERVICE;
#endif /* !CONFIG_ALIF_BLE_ROM_IMAGE_V1_0 */

	uint8_t num_svc = 2;
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
#if !CONFIG_ALIF_BLE_ROM_IMAGE_V1_0 /* ROM version > 1.0 */
		.tx_pwr = 0,
#else
		.max_tx_pwr = 0,
#endif /* !CONFIG_ALIF_BLE_ROM_IMAGE_V1_0 */
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

/* profile callbacks */
#if !CONFIG_ALIF_BLE_ROM_IMAGE_V1_0 /* ROM version > 1.0 */

static uint8_t ll_level;
static uint8_t iass_level;
static int8_t tx_pwr_lvl;

void ll_notify(void)
{
	if (ll_level != LLS_ALERT_LEVEL_NONE) {
		LOG_WRN("Link lost alert with level 0x%02x", ll_level);
		ll_level = LLS_ALERT_LEVEL_NONE;
	}
}

void ias_reset(void)
{
	iass_level = IAS_ALERT_LEVEL_NONE;
}

static void on_get_level_req(uint8_t conidx, uint16_t token)
{
	co_buf_t *p_buf;

	prf_buf_alloc(&p_buf, LLS_ALERT_LEVEL_SIZE);
	*co_buf_data(p_buf) = ll_level;
	llss_get_level_cfm(conidx, token, p_buf);
	co_buf_release(p_buf);

	LOG_DBG("Level requested");
}

static void on_set_level_req(uint8_t conidx, uint16_t token, co_buf_t *p_buf)
{
	uint8_t level = *co_buf_data(p_buf);
	uint16_t status;

	if (level < LLS_ALERT_LEVEL_MAX) {
		ll_level = level;
		status = GAP_ERR_NO_ERROR;
		LOG_INF("Set level requested: %d", level);
	} else {
		status = ATT_ERR_VALUE_NOT_ALLOWED;
	}

	llss_set_level_cfm(conidx, status, token);
}

static const llss_cbs_t llss_cb = {
	.cb_get_level_req = on_get_level_req,
	.cb_set_level_req = on_set_level_req,
};

static void on_level(uint8_t conidx, co_buf_t *p_buf)
{
	uint8_t level = *co_buf_data(p_buf);

	if (level < IAS_ALERT_LEVEL_MAX) {
		iass_level = level;
	} else {
		LOG_ERR("Invalid Immediate Alert Level");
	}
}

static const iass_cbs_t iass_cb = {
	.cb_level = on_level,
};

void cmp_cb(uint8_t conidx, uint32_t metainfo, uint16_t status,
	uint8_t phy, int8_t power_level, int8_t max_power_level)
{
	tx_pwr_lvl = power_level;
}

static void on_level_req(uint8_t conidx, uint16_t token)
{
	co_buf_t *p_buf;

	prf_buf_alloc(&p_buf, TPS_LEVEL_SIZE);
	*co_buf_data(p_buf) = (uint8_t)tx_pwr_lvl;
	tpss_level_cfm(conidx, token, p_buf);
	co_buf_release(p_buf);
	/* Show Tx Power value in signed integer format */
	LOG_INF("Tx Power level sent:: %" PRId8 "\n", tx_pwr_lvl);
}

static const tpss_cbs_t tpss_cb = {
	.cb_level_req = on_level_req,
};

#else
static void on_alert_upd(uint8_t conidx, uint8_t char_code, uint8_t alert_lvl)
{
	LOG_WRN("ALERT UPDATE");
	switch (char_code) {
	case PROXR_ERR_CHAR:
		LOG_DBG("PROXR_ERR_CHAR");
		break;
	case PROXR_LLS_CHAR:
		LOG_DBG("PROXR_LLS_CHAR");
		break;
	default:
		LOG_DBG("alert char_code %02x", char_code);
		break;
	}

	switch (alert_lvl) {
	case PROXR_ALERT_NONE:
		LOG_DBG("PROXR_ALERT_NONE");
		break;
	case PROXR_ALERT_MILD:
		LOG_DBG("PROXR_ALERT_MILD");
		break;
	case PROXR_ALERT_HIGH:
		LOG_DBG("PROXR_ALERT_HIGH");
		break;
	default:
		LOG_DBG("alert level value %02x", alert_lvl);
		break;
	}
}

static const proxr_cb_t proxr_cb = {
	.cb_alert_upd = on_alert_upd,
};
#endif /* !CONFIG_ALIF_BLE_ROM_IMAGE_V1_0 */

/* Add profile to the stack */
static void server_configure(void)
{
	uint16_t err;

	/* Dynamic allocation of service start handle*/
	uint16_t start_hdl = 0;


#if !CONFIG_ALIF_BLE_ROM_IMAGE_V1_0 /* ROM version > 1.0 */
	alif_ble_mutex_lock(K_FOREVER);
	err = prf_add_profile(TASK_ID_LLSS, 0, 0, NULL, &llss_cb, &start_hdl);
	alif_ble_mutex_unlock();

	alif_ble_mutex_lock(K_FOREVER);
	err = prf_add_profile(TASK_ID_IASS, 0, 0, NULL, &iass_cb, &start_hdl);
	alif_ble_mutex_unlock();

	alif_ble_mutex_lock(K_FOREVER);
	err = prf_add_profile(TASK_ID_TPSS, 0, 0, NULL, &tpss_cb, &start_hdl);
	alif_ble_mutex_unlock();

#else
	/* Database configuration structure */
	struct proxr_db_cfg proxr_cfg = {
		.features = PROXR_IAS_TXPS_NOT_SUP,
	};
	err = prf_add_profile(TASK_ID_PROXR, 0, 0, &proxr_cfg, &proxr_cb, &start_hdl);
#endif /* !CONFIG_ALIF_BLE_ROM_IMAGE_V1_0 */

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

int main(void)
{
	uint16_t err;

	/* Start up bluetooth host stack */
	alif_ble_enable(NULL);

	err = gapm_configure(0, &gapm_cfg, &gapm_cbs, on_gapm_process_complete);
	if (err) {
		LOG_ERR("gapm_configure error %u", err);
		return err;
	}
	/* Share control structure */
	service_conn(&ctrl);
	/* Adding battery service */
	config_battery_service();

	LOG_DBG("Waiting for init...\n");
	k_sem_take(&init_sem, K_FOREVER);

	LOG_DBG("Init complete!\n");

	while (1) {
		k_sleep(K_SECONDS(2));
		battery_process();

#if !CONFIG_ALIF_BLE_ROM_IMAGE_V1_0 /* ROM version > 1.0 */
		/* Getting the tx power of the 1MBPS PHY */
		gapc_le_get_local_tx_power_level(0, 0, GAPC_PHY_PWR_1MBPS_VALUE, cmp_cb);

		/* IAS alert shall continue until disconnection or set to None*/
		if (iass_level == IAS_ALERT_LEVEL_MILD) {
			LOG_WRN("IAS mild alert");
		} else if (iass_level == IAS_ALERT_LEVEL_HIGH) {
			LOG_WRN("IAS high alert");
		}
#endif /* !CONFIG_ALIF_BLE_ROM_IMAGE_V1_0 */
	}

	return -EINVAL;
}

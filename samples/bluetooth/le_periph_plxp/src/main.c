/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/*
 * This example will start an instance of a peripheral Pulse Oximeter Service
 * (PLXS) and send periodic notification updates to the first device that connects to it.
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
#include "address_verification.h"
#include <alif/bluetooth/bt_adv_data.h>
#include <alif/bluetooth/bt_scan_rsp.h>
#include "gapm_api.h"

/*  Profile definitions */
#include "prf.h"
#include "plxs.h"
#include "plxp_common.h"
#include "plxs_msg.h"

#define TX_INTERVAL		   1

static uint8_t conn_status = BT_CONN_STATE_DISCONNECTED;

/* Variable to check if peer device is ready to receive data"*/
static bool ready_to_send;

K_SEM_DEFINE(conn_sem, 0, 1);

/* Define advertising address type */
#define SAMPLE_ADDR_TYPE	ALIF_STATIC_RAND_ADDR

/* Store and share advertising address type */
static uint8_t adv_type;

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

/* Measurement structure */
static plxp_spo2pr_t plx_value = {
	/* Initial dummy pulse rate value */
	.pr = 60,
	/* Initial dummy SpO2 value */
	.sp_o2 = 95,
};

/**
 * Bluetooth stack configuration
 */
static gapm_config_t gapm_cfg = {
	.role = GAP_ROLE_LE_PERIPHERAL,
	.pairing_mode = GAPM_PAIRING_DISABLE,
	.privacy_cfg = 0,
	.renew_dur = 1500,
	/*      Dummy address   */
	.private_identity.addr = {0xCB, 0xFE, 0xFB, 0xDE, 0x11, 0x07},
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


/* Load name from configuration file */
#define DEVICE_NAME CONFIG_BLE_DEVICE_NAME

static uint8_t adv_actv_idx;

static uint16_t set_advertising_data(uint8_t actv_idx)
{
	int ret;

	/* gatt service identifier */
	uint16_t svc = GATT_SVC_PULSE_OXIMETER;

	ret = bt_adv_data_set_tlv(GAP_AD_TYPE_COMPLETE_LIST_16_BIT_UUID, &svc, sizeof(svc));
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

/*
 * Server callbacks
 */
static void on_spot_meas_send_cmp(uint8_t conidx, uint16_t status)
{
}

static void on_cont_meas_send_cmp(uint8_t conidx, uint16_t status)
{
	/* Notification was correctly received, it is now allowed to send a new one */
	ready_to_send = true;
}

static void on_bond_data_upd(uint8_t conidx, uint8_t evt_cfg)
{
	if (evt_cfg & PLXS_FEATURES_IND_CFG_BIT) {

		LOG_DBG("Features Indications not supported for this example");
	}

	if (evt_cfg & PLXS_MEAS_SPOT_IND_CFG_BIT) {
		LOG_DBG("Spot-check Indications not supported for this example");
	}

	if (evt_cfg & PLXS_MEAS_CONT_NTF_CFG_BIT) {
		ready_to_send = true;
	} else {
		ready_to_send = false;
	}

	if (evt_cfg & PLXS_RACP_IND_CFG_BIT) {
		LOG_DBG("record Access Control Point not supported for this example");
	}
}

static void on_racp_req(uint8_t conidx, uint8_t op_code, uint8_t func_operator)
{
}

static void on_racp_rsp_send_cmp(uint8_t conidx, uint16_t status)
{
}

static void on_cmp_evt(uint8_t conidx, uint16_t status, uint8_t cmd_type)
{
}

/* profile callbacks */
static const plxs_cb_t plxs_cb = {
	.cb_spot_meas_send_cmp = on_spot_meas_send_cmp,
	.cb_cont_meas_send_cmp = on_cont_meas_send_cmp,
	.cb_bond_data_upd = on_bond_data_upd,
	.cb_racp_req = on_racp_req,
	.cb_racp_rsp_send_cmp = on_racp_rsp_send_cmp,
	.cb_cmp_evt = on_cmp_evt,
};

/* Add profile to the stack */
static void server_configure(void)
{
	uint16_t err;

	/* Dinamic allocation of service start handle*/
	uint16_t start_hdl = 0;

	/* Database configuration structure */
	struct plxs_db_cfg plxs_cfg = {
		.optype = PLXS_OPTYPE_CONTINUOUS_ONLY,
	};

	err = prf_add_profile(TASK_ID_PLXS, 0, 0, &plxs_cfg, &plxs_cb, &start_hdl);

	if (err) {
		LOG_ERR("Error %u adding profile", err);
	}
}

/* Dummy sensor reading emulation */
void read_sensor_value(void)
{
	/* Increment and wrap around the values within their respective ranges */
	plx_value.sp_o2++;

	if (plx_value.sp_o2 > 100) {
		plx_value.sp_o2 = 95;
	}

	plx_value.pr++;

	if (plx_value.pr > 100) {
		plx_value.pr = 60;
	}

	plx_value.sp_o2 = plx_value.sp_o2;
	plx_value.pr = plx_value.pr;
}

/*  Generate and send dummy data*/
static void send_measurement(void)
{
	uint16_t err;

	/*      Dummy measurements values       */
	plxp_cont_meas_t p_meas = {
		.cont_flags = 0,
		.normal = plx_value,
	};

	/* Using connection ndex 0 to notify to the first connected client*/
	err = plxs_cont_meas_send(0, &p_meas);

	if (err) {
		LOG_ERR("Error %u sending measurement", err);
	}
}

static void service_process(void)
{
	read_sensor_value();

	switch (conn_status) {
	case BT_CONN_STATE_CONNECTED:
		if (ready_to_send) {
			send_measurement();
			ready_to_send = false;
		}
		break;

	case BT_CONN_STATE_DISCONNECTED:
		LOG_DBG("Waiting for peer connection...\n");
		k_sem_take(&conn_sem, K_FOREVER);
	default:
		break;
	}
}

void app_connection_status_update(enum gapm_connection_event con_event, uint8_t con_idx,
				  uint16_t status)
{
	switch (con_event) {
	case GAPM_API_SEC_CONNECTED_KNOWN_DEVICE:
		conn_status = BT_CONN_STATE_CONNECTED;
		k_sem_give(&conn_sem);
		LOG_INF("Connection index %u connected to known device", con_idx);
		LOG_DBG("Please enable notifications on peer device..");
		break;
	case GAPM_API_DEV_CONNECTED:
		conn_status = BT_CONN_STATE_CONNECTED;
		k_sem_give(&conn_sem);
		LOG_INF("Connection index %u connected to new device", con_idx);
		LOG_DBG("Please enable notifications on peer device..");
		break;
	case GAPM_API_DEV_DISCONNECTED:
		LOG_INF("Connection index %u disconnected for reason %u", con_idx, status);
		conn_status = BT_CONN_STATE_DISCONNECTED;
		ready_to_send = false;
		break;
	case GAPM_API_PAIRING_FAIL:
		LOG_INF("Connection pairing index %u fail for reason %u", con_idx, status);
		break;
	}
}

static gapm_user_cb_t gapm_user_cb = {
	.connection_status_update = app_connection_status_update,
};

int main(void)
{
	uint16_t err;

	/* Start up bluetooth host stack */
	alif_ble_enable(NULL);

	if (address_verification(SAMPLE_ADDR_TYPE, &adv_type, &gapm_cfg)) {
		LOG_ERR("Address verification failed");
		return -EADV;
	}

	/* Configure Bluetooth Stack */
	LOG_INF("Init gapm service");
	err = bt_gapm_init(&gapm_cfg, &gapm_user_cb, DEVICE_NAME, strlen(DEVICE_NAME));
	if (err) {
		LOG_ERR("gapm_configure error %u", err);
		return -1;
	}

	server_configure();

	/* Create an advertising activity */
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
		/*
		 * Execute process every 1 second
		 * For example purposes
		 */
		k_sleep(K_SECONDS(TX_INTERVAL));
		service_process();
	}
	/* Should not come here */
	return -EINVAL;
}

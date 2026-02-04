/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/*
 * This example will start an instance of a peripheral Cycling Power and send
 * periodic notification updates to the first device that connects to it.
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

/*  Profile definitions */
#include "prf.h"
#include "cpps.h"
#include "cpps_msg.h"
#include "cpp_common.h"
#include <alif/bluetooth/bt_adv_data.h>
#include <alif/bluetooth/bt_scan_rsp.h>
#include "gapm_api.h"

#define TX_INTERVAL                2

static uint8_t conn_status = BT_CONN_STATE_DISCONNECTED;

/* Variable to check if peer device is ready to receive data"*/
static bool READY_TO_SEND;

K_SEM_DEFINE(conn_sem, 0, 1);

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

/* Define advertising address type */
#define SAMPLE_ADDR_TYPE ALIF_STATIC_RAND_ADDR

/* Store and share advertising address type */
static uint8_t adv_type;

static uint16_t current_value;

/**
 * Bluetooth stack configuration
 */
static gapm_config_t gapm_cfg = {
	.role = GAP_ROLE_LE_PERIPHERAL,
	.pairing_mode = GAPM_PAIRING_DISABLE,
	.privacy_cfg = 0,
	.renew_dur = 1500,
	/*      Dummy address   */
	.private_identity.addr = {0xCC, 0xFE, 0xFB, 0xDE, 0x11, 0x07},
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

/* Store advertising activity index for re-starting after disconnection */
static uint8_t adv_idx;
static uint8_t cssp_cfg_val;

/* server callbacks */

static void on_meas_send_complete(uint16_t status)
{
	READY_TO_SEND = true;
}

static void on_bond_data_upd(uint8_t conidx, uint8_t char_code, uint16_t cfg_val)
{

	switch (cfg_val) {
	case PRF_CLI_STOP_NTFIND: {
		LOG_INF("Client requested stop notification/indication (conidx: %u)", conidx);
		READY_TO_SEND = false;
		cssp_cfg_val = (uint8_t)cfg_val;
	} break;

	case PRF_CLI_START_NTF:
	case PRF_CLI_START_IND: {
		LOG_INF("Client requested start notification/indication (conidx: %u)", conidx);
		READY_TO_SEND = true;
		cssp_cfg_val = (uint8_t)cfg_val;
		LOG_DBG("Sending measurements ...");
	}
	}
}

static void on_ctnl_pt_req(uint8_t conidx, uint8_t op_code,
			   const union cpp_ctnl_pt_req_val *p_value)
{
	/* Not supported by this sample application */
}

static void on_cb_ctnl_pt_rsp_send_cmp(uint8_t conidx, uint16_t status)
{
	/* Not supported by this sample application */
}

static void on_vector_send_cmp(uint16_t status)
{
	/* Not supported by this sample application */
}

/*      profile callbacks */
static const cpps_cb_t cpps_cb = {
	.cb_meas_send_cmp = on_meas_send_complete,
	.cb_vector_send_cmp = on_vector_send_cmp,
	.cb_bond_data_upd = on_bond_data_upd,
	.cb_ctnl_pt_req = on_ctnl_pt_req,
	.cb_ctnl_pt_rsp_send_cmp = on_cb_ctnl_pt_rsp_send_cmp,
};

static uint16_t set_advertising_data(uint8_t actv_idx)
{
	int ret;

	/* gatt service identifier */
	uint16_t svc = GATT_SVC_CYCLING_POWER;

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

	return bt_gapm_le_create_advertisement_service(GAPM_STATIC_ADDR, &adv_create_params, NULL,
						      &adv_idx);
}

/* Add profile to the stack */
static void server_configure(void)
{
	uint16_t err;
	uint16_t start_hdl = 0;
	uint8_t sec_lvl = 0;
	uint8_t user_prio = 0;

	struct cpps_db_cfg cpps_cfg = {
		.sensor_loc = CPP_LOC_FRONT_WHEEL,
	};

	err = prf_add_profile(TASK_ID_CPPS, sec_lvl, user_prio, &cpps_cfg, &cpps_cb, &start_hdl);

	if (err) {
		LOG_ERR("Error %u adding profile", err);
	}
}

/*  Generate and send dummy data*/
static void send_measurement(uint16_t current_value)
{
	uint16_t err;

	/*      Dummy measurements values       */
	cpp_cp_meas_t p_meas = {
		.flags = 0,
		.inst_power = current_value,
	};

	err = cpps_meas_send(UINT32_MAX, 0, &p_meas);

	if (err) {
		LOG_ERR("Error %u sending measurement", err);
	}
}

uint16_t read_sensor_value(uint16_t current_value)
{
	/* Generating dummy values between 1 and 4 */
	if (current_value >= 4) {
		current_value = 1;
	} else {
		current_value++;
	}
	return current_value;
}

void service_process(void)
{
	current_value = read_sensor_value(current_value);

	switch (conn_status) {
	case BT_CONN_STATE_CONNECTED:
		if (READY_TO_SEND) {

			send_measurement(current_value);
			READY_TO_SEND = false;
		}

		break;
	case BT_CONN_STATE_DISCONNECTED:
		LOG_DBG("Waiting for peer connection\n");
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
		cpps_enable(con_idx, cssp_cfg_val);
		k_sem_give(&conn_sem);
		LOG_INF("Connection index %u connected to known device", con_idx);
		break;
	case GAPM_API_DEV_CONNECTED:
		conn_status = BT_CONN_STATE_CONNECTED;
		k_sem_give(&conn_sem);
		LOG_INF("Connection index %u connected to new device", con_idx);
		break;
	case GAPM_API_DEV_DISCONNECTED:
		LOG_INF("Connection index %u disconnected for reason %u", con_idx, status);
		conn_status = BT_CONN_STATE_DISCONNECTED;
		READY_TO_SEND = false;
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
	LOG_INF("Init gapm service");
	err = bt_gapm_init(&gapm_cfg, &gapm_user_cb, DEVICE_NAME, strlen(DEVICE_NAME));
	if (err) {
		LOG_ERR("gapm_configure error %u", err);
		return -1;
	}

	server_configure();

	err = create_advertising();
	if (err) {
		LOG_ERR("Advertisement create fail %u", err);
		return -1;
	}

	err = set_advertising_data(adv_idx);
	if (err) {
		LOG_ERR("Advertisement data set fail %u", err);
		return -1;
	}

	err = bt_gapm_scan_response_set(adv_idx);
	if (err) {
		LOG_ERR("Scan response set fail %u", err);
		return -1;
	}

	err = bt_gapm_advertisement_start(adv_idx);
	if (err) {
		LOG_ERR("Advertisement start fail %u", err);
		return -1;
	}
	print_device_identity();

	while (1) {
		k_sleep(K_SECONDS(TX_INTERVAL));
		service_process();
	}
}

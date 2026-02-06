/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/*
 * This example will start an instance of a peripheral Heart Rate and send
 * periodic notification updates to the first device that connects to it.
 * A battery service is included in the sample.
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

#include "prf.h"
#include "hrp_common.h"
#include "hrps.h"
#include "batt_svc.h"
#include "shared_control.h"
#include <alif/bluetooth/bt_adv_data.h>
#include <alif/bluetooth/bt_scan_rsp.h>
#include "gapm_api.h"

#define BODY_SENSOR_LOCATION_CHEST 0x01

/* Define advertising address type */
#define SAMPLE_ADDR_TYPE	ALIF_STATIC_RAND_ADDR

/* Store and share advertising address type */
static uint8_t adv_type;

struct shared_control ctrl = { false, 0, 0 };

enum hrps_feat_bf {
	/* Body Sensor Location Feature Supported */
	HRPS_BODY_SENSOR_LOC_CHAR_SUP_POS = 0,
	HRPS_BODY_SENSOR_LOC_CHAR_SUP_BIT = CO_BIT(HRPS_BODY_SENSOR_LOC_CHAR_SUP_POS),

	/* Energy Expanded Feature Supported */
	HRPS_ENGY_EXP_FEAT_SUP_POS = 1,
	HRPS_ENGY_EXP_FEAT_SUP_BIT = CO_BIT(HRPS_ENGY_EXP_FEAT_SUP_POS),

	/* Heart Rate Measurement Notification Supported */
	HRPS_HR_MEAS_NTF_CFG_POS = 2,
	HRPS_HR_MEAS_NTF_CFG_BIT = CO_BIT(HRPS_HR_MEAS_NTF_CFG_POS),
};

/* Initial dummy value for Heart Rate sensor */
static uint16_t current_value = 70;

/* Variable to check if peer device is ready to receive data"*/
static bool ready_to_send;

K_SEM_DEFINE(conn_sem, 0, 1);

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

/**
 * Bluetooth stack configuration
 */
static gapm_config_t gapm_cfg = {
	.role = GAP_ROLE_LE_PERIPHERAL,
	.pairing_mode = GAPM_PAIRING_DISABLE,
	.privacy_cfg = 0,
	.renew_dur = 1500,
	.private_identity.addr = {0xCA, 0xFE, 0xFB, 0xDE, 0x11, 0x07},
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
	.class_of_device = 0,  /* BT Classic only */
	.dflt_link_policy = 0, /* BT Classic only */
};

/* Load name from configuration file */
#define DEVICE_NAME CONFIG_BLE_DEVICE_NAME

/* Store advertising activity index for re-starting after disconnection */
static uint8_t adv_actv_idx;

/* HRPS callbacks */

static void on_hrps_meas_send_complete(uint16_t status)
{
	ready_to_send = true;
}

static void on_bond_data_upd(uint8_t conidx, uint16_t cfg_val)
{
	switch (cfg_val) {
	case PRF_CLI_STOP_NTFIND: {
		LOG_INF("Client requested stop notification/indication (conidx: %u)", conidx);
		ready_to_send = false;
	} break;

	case PRF_CLI_START_NTF:
	case PRF_CLI_START_IND: {
		LOG_INF("Client requested start notification/indication (conidx: %u)", conidx);
		ready_to_send = true;
		LOG_DBG("Sending measurements");
	}
	}
}

static void on_energy_exp_reset(uint8_t conidx)
{
}

static const hrps_cb_t hrps_cb = {
	.cb_bond_data_upd = on_bond_data_upd,
	.cb_meas_send_cmp = on_hrps_meas_send_complete,
	.cb_energy_exp_reset = on_energy_exp_reset,
};

static uint16_t set_advertising_data(uint8_t actv_idx)
{
	int ret;
	uint16_t svc[2];
	uint16_t comp_id = CONFIG_BLE_COMPANY_ID;

	/* This sample write AD Name, Profile list and Company ID */

	/* Define GATT profiles */
	svc[0] = GATT_SVC_HEART_RATE;
	svc[1] = get_batt_id();

	ret = bt_adv_data_set_tlv(GAP_AD_TYPE_COMPLETE_LIST_16_BIT_UUID, svc, sizeof(svc));
	if (ret) {
		LOG_ERR("AD profile set fail %d", ret);
		return ATT_ERR_INSUFF_RESOURCE;
	}

	ret = bt_adv_data_set_manufacturer(comp_id, NULL, 0);

	if (ret) {
		LOG_ERR("AD manufacturer data fail %d", ret);
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

/* Add heart rate profile to the stack */
static uint16_t  hr_server_configure(void)
{
	uint16_t start_hdl = 0;
	struct hrps_db_cfg hrps_cfg;

	/* Add the heart rate server profile and register our callbacks */
	hrps_cfg.features = HRPS_BODY_SENSOR_LOC_CHAR_SUP_BIT | HRPS_HR_MEAS_NTF_CFG_BIT;
	hrps_cfg.body_sensor_loc = BODY_SENSOR_LOCATION_CHEST;

	return prf_add_profile(TASK_ID_HRPS, 0, 0, &hrps_cfg, &hrps_cb, &start_hdl);
}

static void send_measurement(int16_t current_value)
{
	uint16_t err;
	hrs_hr_meas_t hr_meas = {
		.flags = HRS_FLAG_HR_VALUE_FORMAT_POS,
		.heart_rate = current_value,
		.nb_rr_interval = 0,
	};

	/* Set bit field to all 1's to send notification
	 * on all connections that are subscribed
	 */
	uint32_t conidx_bf = UINT32_MAX;
	err = hrps_meas_send(conidx_bf, &hr_meas);

	if (err) {
		LOG_ERR("Error %u sending measurement", err);
	}
}

void read_sensor_value(void)
{
	/* Generating dummy values between 70 and 130 */
	if (current_value >= 130) {
		current_value = 70;
	} else {
		current_value++;
	}
}

void service_process(void)
{
	read_sensor_value();

	if (ctrl.connected) {
		if (ready_to_send) {
			send_measurement(current_value);
			ready_to_send = false;
		}
	} else {
		LOG_DBG("Waiting for peer connection...\n");
		k_sem_take(&conn_sem, K_FOREVER);
	}
}

void app_connection_status_update(enum gapm_connection_event con_event, uint8_t con_idx,
				  uint16_t status)
{
	switch (con_event) {
	case GAPM_API_SEC_CONNECTED_KNOWN_DEVICE:
		ctrl.connected = true;
		k_sem_give(&conn_sem);
		LOG_INF("Connection index %u connected to known device", con_idx);
		LOG_DBG("Please enable notifications on peer device..");
		break;
	case GAPM_API_DEV_CONNECTED:
		ctrl.connected = true;
		k_sem_give(&conn_sem);
		LOG_INF("Connection index %u connected to new device", con_idx);
		LOG_DBG("Please enable notifications on peer device..");
		break;
	case GAPM_API_DEV_DISCONNECTED:
		LOG_INF("Connection index %u disconnected for reason %u", con_idx, status);
		ctrl.connected = false;
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

	/* Share connection info */
	service_conn(&ctrl);

	/* Adding battery service */
	config_battery_service();

	hr_server_configure();

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
		k_sleep(K_SECONDS(1));
		service_process();
		battery_process();
	}
}

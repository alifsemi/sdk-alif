/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/*
 * This example will start an instance of a peripheral RSCPS and send
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
#include <alif/bluetooth/bt_adv_data.h>
#include <alif/bluetooth/bt_scan_rsp.h>
#include "gapm_api.h"

/*  Profile definitions */
#include "prf.h"
#include "rscps.h"
#include "rscps_msg.h"
#include "rscp_common.h"
#include "batt_svc.h"
#include "shared_control.h"

struct shared_control ctrl = { false, 0, 0 };

#define RSCP_SENSOR_LOCATION_SUPPORT 0x01
#define TX_INTERVAL		     1

/* Define advertising address type */
#define SAMPLE_ADDR_TYPE	ALIF_STATIC_RAND_ADDR

/* Store and share advertising address type */
static uint8_t adv_type;

/* Variable to check if peer device is ready to receive data"*/
static bool ready_to_send;

static uint32_t total_distance;
static uint16_t current_value = 1;

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
static uint8_t adv_actv_idx;

/* server callbacks */

static void on_meas_send_complete(uint16_t status)
{
	ready_to_send = true;
}

static void on_bond_data_upd(uint8_t conidx, uint8_t char_code, uint16_t cfg_val)
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
		LOG_DBG("Sending measurements ...");
	}
	}
}

static void on_ctnl_pt_req(uint8_t conidx, uint8_t op_code,
			   const union rscp_sc_ctnl_pt_req_val *p_value)
{
}

static void on_cb_ctnl_pt_rsp_send_cmp(uint8_t conidx, uint16_t status)
{
}

/*      profile callbacks */
static const rscps_cb_t rscps_cb = {
	.cb_bond_data_upd = on_bond_data_upd,
	.cb_meas_send_cmp = on_meas_send_complete,
	.cb_ctnl_pt_req = on_ctnl_pt_req,
	.cb_ctnl_pt_rsp_send_cmp = on_cb_ctnl_pt_rsp_send_cmp,
};

static uint16_t set_advertising_data(uint8_t actv_idx)
{
	int ret;
	uint16_t svc[2];

	/* gatt service identifier */
	svc[0] = GATT_SVC_RUNNING_SPEED_CADENCE;
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

/* Add profile to the stack */
static void server_configure(void)
{
	uint16_t err;
	uint16_t start_hdl = 0;
	struct rscps_db_cfg rscps_cfg = {
		.rsc_feature = RSCP_FEAT_INST_STRIDE_LEN_SUPP_BIT |
			       RSCP_FEAT_WALK_RUN_STATUS_SUPP_BIT |
			       RSCP_FEAT_TOTAL_DST_MEAS_SUPP_BIT,
		.sensor_loc_supp = RSCP_SENSOR_LOCATION_SUPPORT,
		.sensor_loc = RSCP_LOC_CHEST,
	};

	err = prf_add_profile(TASK_ID_RSCPS, 0, 0, &rscps_cfg, &rscps_cb, &start_hdl);

	if (err) {
		LOG_ERR("Error %u adding profile", err);
	}
}

/*  Generate and send dummy data*/
static void send_measurement(void)
{
	uint16_t err;

	/*      Dummy measurements values       */
	rscp_rsc_meas_t p_meas = {
		.flags = RSCP_MEAS_ALL_PRESENT,
		.inst_cad = 0xA0 - current_value,
		.inst_speed = 0x1C2 - current_value,
		.inst_stride_len = 0x96 - current_value,
		.total_dist = total_distance,
	};

	/* Dummy cumulative total distance calculation */
	total_distance += (p_meas.inst_speed * 0.0039111 * 10 * TX_INTERVAL);

	/* Set bit field to all 1's to send notification
	 * on all connections that are subscribed
	 */
	err = rscps_meas_send(UINT32_MAX, &p_meas);

	if (err) {
		LOG_ERR("Error %u sending measurement", err);
	}
}

static void read_sensor_value(void)
{
	/* Generating dummy values between 1 and 5 */
	if (current_value >= 4) {
		current_value = 1;
	} else {
		current_value++;
	}
}

static void service_process(void)
{
	read_sensor_value();
	if (ctrl.connected) {
		if (ready_to_send) {
			send_measurement();
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
		LOG_DBG("Please enable notifications on peer device..");
		break;
	case GAPM_API_DEV_CONNECTED:
		ctrl.connected = true;
		k_sem_give(&conn_sem);
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

	/* Configure main service */
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
		/* Execute process every 1 second */
		k_sleep(K_SECONDS(TX_INTERVAL));
		service_process();
		battery_process();
	}
}

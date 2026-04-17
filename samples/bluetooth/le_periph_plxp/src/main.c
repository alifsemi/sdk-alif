/* Copyright (C) Alif Semiconductor - All Rights Reserved.
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
#if defined(CONFIG_IUT_TESTER_ENABLED)
#include "ble_gpio.h"
#endif

/*  Profile definitions */
#include "prf.h"
#include "plxs.h"
#include "plxp_common.h"
#include "plxs_msg.h"

#define TX_INTERVAL		   1

static uint8_t conn_status = BT_CONN_STATE_DISCONNECTED;

/* Peer device is ready to receive data */
static bool ready_to_send;

/* Variable to check if features indication is in progress */
static bool features_indication_ongoing;

/* Stored CCCD configuration for bonded peer (simplistic approach for PTS testing) */
static uint8_t saved_evt_cfg;

K_SEM_DEFINE(conn_sem, 0, 1);

/* Define advertising address type */
#define SAMPLE_ADDR_TYPE	ALIF_PUBLIC_ADDR

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
	.pairing_mode = GAPM_PAIRING_SEC_CON,
	.privacy_cfg = 0,
	.renew_dur = 1500,
	/* Dummy address */
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
	LOG_DBG("conidx: %" PRIu8 ", status: %" PRIx16, conidx, status);
}

static void on_cont_meas_send_cmp(uint8_t conidx, uint16_t status)
{
	LOG_DBG("conidx: %" PRIu8 ", status: %" PRIx16, conidx, status);
	/* Notification was correctly received, it is now allowed to send a new one */
	ready_to_send = true;
}

static void on_bond_data_upd(uint8_t conidx, uint8_t evt_cfg)
{
	/* Save CCCD configuration for future reconnections */
	saved_evt_cfg = evt_cfg;
	LOG_INF("Bond data updated for connection index %" PRIu8 ", evt_cfg: %" PRIx8,
		conidx, evt_cfg);

	if (evt_cfg & PLXS_FEATURES_IND_CFG_BIT) {
		uint16_t err;

		if (features_indication_ongoing) {
			LOG_WRN("Features indication already in progress, skipping");
		} else {
			LOG_INF("PLX Features indications enabled, sending feature update");
			err = plxs_features_updated(conidx);
			if (err) {
				LOG_ERR("Error 0x%" PRIx16 " sending feature indication", err);
			} else {
				features_indication_ongoing = true;
			}
		}
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
	LOG_INF("conidx: %" PRIu8 ", status: %" PRIu16 ", cmd_type: %" PRIu8,
		conidx, status, cmd_type);

	switch (cmd_type) {
	case PLXS_FEATURES_UPDATED_CMD_OP_CODE:
		features_indication_ongoing = false;
		if (status == GAP_ERR_NO_ERROR) {
			LOG_INF("PLX Features indication procedure completed successfully");
		} else {
			LOG_ERR("PLX Features indication procedure failed with error %" PRIu16,
				status);
		}
		break;
	case PLXS_SPOT_CHECK_MEAS_CMD_OP_CODE:
		if (status == GAP_ERR_NO_ERROR) {
			LOG_INF("Spot-check measurement procedure completed successfully");
		} else {
			LOG_ERR("Spot-check measurement procedure failed with error %" PRIu16,
				status);
		}
		break;
	case PLXS_CONTINUOUS_MEAS_CMD_OP_CODE:
		if (status == GAP_ERR_NO_ERROR) {
			LOG_INF("Continuous measurement procedure completed successfully");
		} else {
			LOG_ERR("Continuous measurement procedure failed with error %" PRIu16,
				status);
		}
		break;
	case PLXS_RACP_CMD_OP_CODE:
		if (status == GAP_ERR_NO_ERROR) {
			LOG_INF("Record Access Control Point procedure completed successfully");
		} else {
			LOG_ERR("Record Access Control Point procedure failed with error %" PRIu16,
				status);
		}
		break;
	default:
		LOG_WRN("Unknown command type %" PRIu8, cmd_type);
		break;
	}
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

/* Add PLXS profile to the stack */
static void server_configure(void)
{
	uint16_t err;

	/* Dynamic allocation of service start handle */
	uint16_t start_hdl = 0;

	/* Database configuration structure */
	struct plxs_db_cfg plxs_cfg = {
		.optype = PLXS_OPTYPE_CONTINUOUS_ONLY,
	};

	err = prf_add_profile(TASK_ID_PLXS, 0, 0, &plxs_cfg, &plxs_cb, &start_hdl);

	if (err) {
		LOG_ERR("Error 0x%" PRIx16 " adding profile", err);
	}
}

#if defined(CONFIG_IUT_TESTER_ENABLED)
static void button_update_handler(uint32_t button_state, uint32_t has_changed)
{
	uint16_t err;

	if (has_changed & 1) {
		if (!(button_state & 1)) {
			/* Button pressed (active low) */
			LOG_INF("Button 0 pressed - attempting to send PLX Features indication");

			if (conn_status != BT_CONN_STATE_CONNECTED) {
				LOG_WRN("Not connected - cannot send indication");
				return;
			}

			if (features_indication_ongoing) {
				LOG_WRN("Features indication already in progress, please wait");
				return;
			}

			err = plxs_features_updated(0);
			if (err) {
				LOG_ERR("Error 0x%" PRIx16 " sending features indication", err);
			} else {
				LOG_INF("Features indication sent successfully");
				features_indication_ongoing = true;
			}
		}
	}
}
#endif

/* Dummy sensor reading emulation */
static void read_sensor_value(void)
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

/* Generate and send dummy data */
static void send_measurement(void)
{
	uint16_t err;

	/* Dummy measurement values */
	plxp_cont_meas_t p_meas = {
		.cont_flags = 0,
		.normal = plx_value,
	};

	/* Using connection index 0 to notify the first connected client */
	err = plxs_cont_meas_send(0, &p_meas);

	if (err) {
		LOG_ERR("Error 0x%" PRIx16 " sending measurement", err);
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
	uint16_t err;
	uint8_t restored_evt_cfg;

	switch (con_event) {
	case GAPM_API_SEC_CONNECTED_KNOWN_DEVICE:
		conn_status = BT_CONN_STATE_CONNECTED;
		k_sem_give(&conn_sem);
		LOG_INF("Connection index %" PRIu8 " connected to known device", con_idx);

		/* Restore bond data for known peer device with saved CCCD values */
		restored_evt_cfg = saved_evt_cfg;
		LOG_INF("Restoring PLXS with evt_cfg: %" PRIu8 "", restored_evt_cfg);
		err = plxs_enable(con_idx, restored_evt_cfg);
		if (err) {
			LOG_ERR("Error %" PRIu16 " enabling PLXS for known device", err);
		} else {
			LOG_INF("PLXS enabled for known device");
		}
		break;
	case GAPM_API_DEV_CONNECTED:
		conn_status = BT_CONN_STATE_CONNECTED;
		k_sem_give(&conn_sem);
		LOG_INF("Connection index %" PRIu8 " connected to new device", con_idx);
		LOG_DBG("Please enable notifications on peer device..");
		break;
	case GAPM_API_DEV_DISCONNECTED:
		conn_status = BT_CONN_STATE_DISCONNECTED;
		ready_to_send = false;
		features_indication_ongoing = false;
		break;
	case GAPM_API_PAIRING_FAIL:
		LOG_INF("Connection pairing index %" PRIu8 " fail for reason 0x%" PRIx16,
			con_idx, status);
		break;
	}
}

static gapm_user_cb_t gapm_user_cb = {
	.connection_status_update = app_connection_status_update,
};

int main(void)
{
	uint16_t err;

#if defined(CONFIG_IUT_TESTER_ENABLED)
	/* Initialize button handler for manual features indication testing */
	err = ble_gpio_buttons_init(button_update_handler);
	if (err) {
		LOG_ERR("Button Init fail 0x%" PRIx16, err);
		return -1;
	}
#endif

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
		LOG_ERR("gapm_configure error 0x%" PRIx16, err);
		return -1;
	}

	server_configure();

	/* Create an advertising activity */
	err = create_advertising();
	if (err) {
		LOG_ERR("Advertisement create fail 0x%" PRIx16, err);
		return -1;
	}

	err = set_advertising_data(adv_actv_idx);
	if (err) {
		LOG_ERR("Advertisement data set fail 0x%" PRIx16, err);
		return -1;
	}

	err = bt_gapm_scan_response_set(adv_actv_idx);
	if (err) {
		LOG_ERR("Scan response set fail 0x%" PRIx16, err);
		return -1;
	}

	err = bt_gapm_advertisement_start(adv_actv_idx);
	if (err) {
		LOG_ERR("Advertisement start fail 0x%" PRIx16, err);
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

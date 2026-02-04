/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/*
 * This example will start an instance of a peripheral Glucose Profile Service (GLPS) and send
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
#include "address_verification.h"

/*  Profile definitions */
#include "prf.h"
#include "glps.h"
#include "glps_msg.h"
#include "prf_types.h"
#include "rtc_emulator.h"
#include "batt_svc.h"
#include "shared_control.h"
#include <alif/bluetooth/bt_adv_data.h>
#include <alif/bluetooth/bt_scan_rsp.h>
#include "gapm_api.h"

struct shared_control ctrl = { false, 0, 0 };

/* Short interval for demonstration purposes */
#define TX_INTERVAL	2000 /* in milliseconds */
#define GLPS_STORE_MAX	0xFFFF

/* Define advertising address type */
#define SAMPLE_ADDR_TYPE	ALIF_STATIC_RAND_ADDR

/* Store and share advertising address type */
static uint8_t adv_type;

/* Variable to check if peer device is ready to receive data"*/
static bool READY_TO_SEND;

static uint16_t seq_num;
static uint16_t store_idx;
static prf_sfloat meas_value;

/* Global index to cycle through the values */
static int current_index;

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

/**
 * Bluetooth stack configuration
 */
static gapm_config_t gapm_cfg = {
	.role = GAP_ROLE_LE_PERIPHERAL,
	.pairing_mode = GAPM_PAIRING_SEC_CON,
	.privacy_cfg = 0,
	.renew_dur = 1500,
	.private_identity.addr = {0xCD, 0xFE, 0xFB, 0xDE, 0x11, 0x07},
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

/* Load name from configuration file */
#define DEVICE_NAME CONFIG_BLE_DEVICE_NAME

/* Store advertising activity index for re-starting after disconnection */
static uint8_t adv_actv_idx;

/* Server callbacks */

static void on_bond_data_upd(uint8_t conidx, uint8_t evt_cfg)
{
	switch (evt_cfg) {
	case PRF_CLI_STOP_NTFIND:
		LOG_INF("Client requested stop notification/indication (conidx: %u)", conidx);
		READY_TO_SEND = false;
		break;

	case PRF_CLI_START_IND:
		READY_TO_SEND = false;
		break;

	case PRF_CLI_START_NTF:
		LOG_INF("Client requested start notification/indication (conidx: %u)", conidx);
		READY_TO_SEND = true;
		break;

	default:
		break;
	}
}
struct glps_racp_temp {
	uint8_t conidx;
	uint8_t op_code;
	uint8_t func_operator;
	uint8_t filter_type;
	const union glp_filter *p_filter;
};

static struct glps_racp_temp glps_temp;

static uint16_t send_idx = 1;
static uint16_t nb_stored;
static bool available_data;
static bool transfer_in_process;
struct extended_glucose_meas {
	uint16_t ext_seq_num;
	glp_meas_t p_meas;
};

struct extended_glucose_meas ext_meas[GLPS_STORE_MAX];

static void on_meas_send_complete(uint8_t conidx, uint16_t status)
{
	uint16_t err;

	READY_TO_SEND = true;

	if (nb_stored <= 1) {
		glps_racp_rsp_send(conidx, glps_temp.op_code, GLP_RSP_SUCCESS, 1);
		send_idx = 1;
	} else {
		err = glps_meas_send(glps_temp.conidx,
				ext_meas[send_idx].ext_seq_num,
				&ext_meas[send_idx].p_meas, NULL);
		if (err) {
			LOG_ERR("Error %u sending measurement", err);
		}
		send_idx++;
		nb_stored--;
	}
}


static void process_racp_req(uint8_t conidx, uint8_t op_code)
{
	uint16_t err;

	nb_stored = store_idx;
	store_idx = 0;

	if (READY_TO_SEND && available_data) {
		available_data = false;
		err = glps_meas_send(glps_temp.conidx, ext_meas[0].ext_seq_num,
				&ext_meas[0].p_meas, NULL);
		if (err) {
			LOG_ERR("Error %u sending measurement", err);
		}
	} else {
		glps_racp_rsp_send(conidx, glps_temp.op_code, GLP_RSP_NO_RECS_FOUND, 0);
	}
}

static void on_racp_rep(uint8_t conidx, uint8_t op_code, uint8_t func_operator, uint8_t filter_type,
			const union glp_filter *p_filter)
{
	if (!transfer_in_process) {
		transfer_in_process = true;

		glps_temp.conidx = conidx;
		glps_temp.filter_type = filter_type;
		glps_temp.func_operator = func_operator;
		glps_temp.op_code = op_code;
		glps_temp.p_filter = p_filter;

		process_racp_req(conidx, op_code);
	} else {
		LOG_ERR("TRANSFER IN PROCESS");
	}
}

static void racp_rsp_send_cmp(uint8_t conidx, uint16_t status)
{
	transfer_in_process = false;
}

static const glps_cb_t glps_cb = {
	.cb_bond_data_upd = on_bond_data_upd,
	.cb_meas_send_cmp = on_meas_send_complete,
	.cb_racp_req = on_racp_rep,
	.cb_racp_rsp_send_cmp = racp_rsp_send_cmp,
};

static uint16_t set_advertising_data(uint8_t actv_idx)
{
	int ret;
	uint16_t svc[2];

	/* gatt service identifier */
	svc[0] = GATT_SVC_GLUCOSE;
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

/* Add heart rate profile to the stack */
static void server_configure(void)
{
	uint16_t err;
	uint16_t start_hdl = 0;
	struct glps_db_cfg glps_cfg = {0};

	err = prf_add_profile(TASK_ID_GLPS, GAP_SEC1_NOAUTH_PAIR_ENC, 0, &glps_cfg, &glps_cb,
				&start_hdl);

	if (err) {
		LOG_ERR("Error %u adding profile", err);
	}
}

/* Function to convert glucose concentration in mg/dL to SFLOAT format */
prf_sfloat convert_to_sfloat(float glucose_mg_dL)
{
	int abs_value = abs((int)glucose_mg_dL);

	/* Mantissa - limit to 12 bits */
	unsigned short mantissa = abs_value & 0xFFF;
	/*
	 * Exponent - for simplicity, we set the MSB to 1 and add 0b100
	 * in 2's complement to convert to kg/L
	 */
	unsigned short exponent = 0b1011;

	prf_sfloat sfloat_value = (exponent << 12) | mantissa;

	return sfloat_value;
}

prf_sfloat read_sensor_value(void)
{
	/* Dummy generation of glucose concentration values */
	float glucose_values_mg_dL[] = {70.0, 75.0, 80.0, 85.0, 90.0, 95.0, 100.0};
	int num_values = ARRAY_SIZE(glucose_values_mg_dL);

	/* Select the next value in the array and convert to sfloat */
	float selected_value = glucose_values_mg_dL[current_index];
	prf_sfloat converted_value = convert_to_sfloat(selected_value);

	/* Update the index to cycle through the values */
	current_index = (current_index + 1) % num_values;


	/* TODO save the last value in NVM */

	return converted_value;
}


/*  Generate and send dummy data*/
static void store_measurement(prf_sfloat current_value)
{
	prf_date_time_t *timePtr = (prf_date_time_t *)get_device_time();

	prf_date_time_t updated_time = *timePtr;

	/* Dummy measurement data */
	if (store_idx >= GLPS_STORE_MAX) {
		store_idx = 0;
	}
	glp_meas_t glps_temp_meas = {
		.base_time = updated_time,
		.concentration = current_value,
		.type = GLP_TYPE_CAPILLARY_WHOLE_BLOOD,
		.location = GLP_LOC_FINGER,
		.flags = GLP_MEAS_GL_CTR_TYPE_AND_SPL_LOC_PRES_BIT,
	};

	ext_meas[store_idx].p_meas = glps_temp_meas;
	ext_meas[store_idx].ext_seq_num = seq_num;
	available_data = true;
	store_idx++;
	/* sequence number must be unique per measurement */
	seq_num += 1;
}

static void service_process(void)
{
	meas_value = read_sensor_value();

	store_measurement(meas_value);
}

void app_connection_status_update(enum gapm_connection_event con_event, uint8_t con_idx,
				  uint16_t status)
{
	switch (con_event) {
	case GAPM_API_SEC_CONNECTED_KNOWN_DEVICE:
		ctrl.connected = true;
		LOG_INF("Connection index %u connected to known device", con_idx);
		LOG_DBG("Please enable notifications on peer device..");
		break;
	case GAPM_API_DEV_CONNECTED:
		ctrl.connected = true;
		LOG_INF("Connection index %u connected to new device", con_idx);
		LOG_DBG("Please enable notifications on peer device..");
		break;
	case GAPM_API_DEV_DISCONNECTED:
		LOG_INF("Connection index %u disconnected for reason %u", con_idx, status);
		ctrl.connected = false;
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

	start_rtc_emulator();

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

	/* Share control structure */
	service_conn(&ctrl);

	/* Adding battery service */
	config_battery_service();

	server_configure();

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
		k_sleep(K_MSEC(TX_INTERVAL));
		service_process();
		battery_process();
	}
	/* Should not come here */
	return -EINVAL;
}

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
	.pairing_mode = GAPM_PAIRING_MODE_ALL,
	.privacy_cfg = 0,
	.renew_dur = 1500,
	.private_identity.addr = {0xCD, 0xFE, 0xFB, 0xDE, 0x11, 0x07},
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

static gapc_pairing_t p_pairing_info = {
	.auth = GAP_AUTH_NONE,
	.ikey_dist = GAP_KDIST_NONE,
	.iocap = GAP_IO_CAP_NO_INPUT_NO_OUTPUT,
	.key_size = 16,
	.oob = GAP_OOB_AUTH_DATA_NOT_PRESENT,
	.rkey_dist = GAP_KDIST_NONE,
};

/* Load name from configuration file */
#define DEVICE_NAME CONFIG_BLE_DEVICE_NAME

/* Store advertising activity index for re-starting after disconnection */
static uint8_t adv_actv_idx;

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

	LOG_INF("Peer BD address %02X:%02X:%02X:%02X:%02X:%02X (conidx: %u)", p_peer_addr->addr[5],
		p_peer_addr->addr[4], p_peer_addr->addr[3], p_peer_addr->addr[2],
		p_peer_addr->addr[1], p_peer_addr->addr[0], conidx);

	ctrl.connected = true;

	LOG_DBG("Please enable notifications on peer device..");
}

static void on_key_received(uint8_t conidx, uint32_t metainfo, const gapc_pairing_keys_t *p_keys)
{
	LOG_WRN("Unexpected key received key on conidx %u", conidx);
}

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

	ctrl.connected = false;

	READY_TO_SEND = false;
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

/*
 * Security callbacks
 */

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
}

static void on_auth_req(uint8_t conidx, uint32_t metainfo, uint8_t auth_level)
{
}

static void on_auth_info(uint8_t conidx, uint32_t metainfo, uint8_t sec_lvl,
				bool encrypted,
				uint8_t key_size)
{
}

static void on_pairing_succeed(uint8_t conidx, uint32_t metainfo, uint8_t pairing_level,
				bool enc_key_present,
				uint8_t key_type)
{
	LOG_INF("Pairing succeeded");
}

static void on_info_req(uint8_t conidx, uint32_t metainfo, uint8_t exp_info)
{
}

static void on_ltk_req(uint8_t conidx, uint32_t metainfo, uint8_t key_size)
{
}
static void on_numeric_compare_req(uint8_t conidx, uint32_t metainfo, uint32_t numeric_value)
{
}
static void on_key_pressed(uint8_t conidx, uint32_t metainfo, uint8_t notification_type)
{
}
static void on_repeated_attempt(uint8_t conidx, uint32_t metainfo)
{
}

static const gapc_connection_req_cb_t gapc_con_cbs = {
	.le_connection_req = on_le_connection_req,
};

static const gapc_security_cb_t gapc_sec_cbs = {
	.key_received = on_key_received,
	.pairing_req = on_pairing_req,
	.pairing_failed = on_pairing_failed,
	.le_encrypt_req = on_le_encrypt_req,
	.auth_req = on_auth_req,
	.auth_info = on_auth_info,
	.pairing_succeed = on_pairing_succeed,
	.info_req = on_info_req,
	.ltk_req = on_ltk_req,
	.numeric_compare_req = on_numeric_compare_req,
	.key_pressed = on_key_pressed,
	.repeated_attempt = on_repeated_attempt,
};

static const gapc_connection_info_cb_t gapc_con_inf_cbs = {
	.disconnected = on_disconnection,
	.name_get = on_name_get,
	.appearance_get = on_appearance_get,
	/* Other callbacks in this struct are optional */
};

/* All callbacks in this struct are optional */
static const gapc_le_config_cb_t gapc_le_cfg_cbs;

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
	err = bt_gapm_init(&gapm_cfg, &gapm_cbs, DEVICE_NAME, strlen(DEVICE_NAME));
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

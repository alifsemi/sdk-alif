/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/* This application demonstrates the communication and control of a device
 * allowing to remotely control an LED, and to transmit the state of a button.
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
#include "prf.h"
#include "gatt_db.h"
#include "gatt_srv.h"
#include "ke_mem.h"
#include "address_verification.h"
#include <zephyr/drivers/gpio.h>
#include <alif/bluetooth/bt_adv_data.h>
#include <alif/bluetooth/bt_scan_rsp.h>
#include "gapm_api.h"

#define LED0_NODE DT_ALIAS(led0)
#define LED2_NODE DT_ALIAS(led2)
#define SW0_NODE  DT_ALIAS(sw0)

static uint8_t adv_type; /* Advertising type, set by address_verification() */

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios, {0});

#define BT_CONN_STATE_CONNECTED    0x00
#define BT_CONN_STATE_DISCONNECTED 0x01

/* Service Definitions */
#define ATT_128_PRIMARY_SERVICE  ATT_16_TO_128_ARRAY(GATT_DECL_PRIMARY_SERVICE)
#define ATT_128_INCLUDED_SERVICE ATT_16_TO_128_ARRAY(GATT_DECL_INCLUDE)
#define ATT_128_CHARACTERISTIC   ATT_16_TO_128_ARRAY(GATT_DECL_CHARACTERISTIC)
#define ATT_128_CLIENT_CHAR_CFG  ATT_16_TO_128_ARRAY(GATT_DESC_CLIENT_CHAR_CFG)
/* LED-BUTTON SERVICE and attribute 128 bit UUIDs */
#define LBS_UUID_128_SVC                                                                           \
	{0x23, 0xd1, 0xbc, 0xea, 0x5f, 0x78, 0x23, 0x15,                                           \
	 0xde, 0xef, 0x12, 0x12, 0x23, 0x15, 0x00, 0x00}
#define LBS_UUID_128_CHAR0                                                                         \
	{0x23, 0xd1, 0xbc, 0xea, 0x5f, 0x78, 0x23, 0x15,                                           \
	 0xde, 0xef, 0x12, 0x12, 0x24, 0x15, 0x00, 0x00}
#define LBS_UUID_128_CHAR1                                                                         \
	{0x23, 0xd1, 0xbc, 0xea, 0x5f, 0x78, 0x23, 0x15,                                           \
	 0xde, 0xef, 0x12, 0x12, 0x25, 0x15, 0x00, 0x00}
#define LBS_METAINFO_CHAR0_NTF_SEND 0x1234
#define ATT_16_TO_128_ARRAY(uuid)                                                                  \
	{(uuid) & 0xFF, (uuid >> 8) & 0xFF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}

/* List of attributes in the service */
enum service_att_list {
	LBS_IDX_SERVICE = 0,
	/* First characteristic is readable + supports notifications */
	LBS_IDX_CHAR0_CHAR,
	LBS_IDX_CHAR0_VAL,
	LBS_IDX_CHAR0_NTF_CFG,
	/* Second characteristic is writable */
	LBS_IDX_CHAR1_CHAR,
	LBS_IDX_CHAR1_VAL,
	/* Number of items*/
	LBS_IDX_NB,
};

static uint8_t conn_status = BT_CONN_STATE_DISCONNECTED;
static uint8_t adv_actv_idx;
static struct service_env env;
static bool led_state;
static uint8_t led_cnt;

/* Load name from configuration file */
#define DEVICE_NAME      CONFIG_BLE_DEVICE_NAME
#define SAMPLE_ADDR_TYPE ALIF_STATIC_RAND_ADDR /* Static random address */

/* Service UUID to pass into gatt_db_svc_add */
static const uint8_t lbs_service_uuid[] = LBS_UUID_128_SVC;

/* GATT database for the service */
static const gatt_att_desc_t lbs_att_db[LBS_IDX_NB] = {
	[LBS_IDX_SERVICE] = {ATT_128_PRIMARY_SERVICE, ATT_UUID(16) | PROP(RD), 0},

	[LBS_IDX_CHAR0_CHAR] = {ATT_128_CHARACTERISTIC, ATT_UUID(16) | PROP(RD), 0},
	[LBS_IDX_CHAR0_VAL] = {LBS_UUID_128_CHAR0, ATT_UUID(128) | PROP(RD) | PROP(N),
			       OPT(NO_OFFSET)},
	[LBS_IDX_CHAR0_NTF_CFG] = {ATT_128_CLIENT_CHAR_CFG, ATT_UUID(16) | PROP(RD) | PROP(WR), 0},

	[LBS_IDX_CHAR1_CHAR] = {ATT_128_CHARACTERISTIC, ATT_UUID(16) | PROP(RD), 0},
	[LBS_IDX_CHAR1_VAL] = {LBS_UUID_128_CHAR1, ATT_UUID(128) | PROP(WR),
			       OPT(NO_OFFSET) | sizeof(uint16_t)},
};

/* Bluetooth stack configuration*/
static gapm_config_t gapm_cfg = {
	.role = GAP_ROLE_LE_PERIPHERAL,
	.pairing_mode = GAPM_PAIRING_DISABLE,
	.privacy_cfg = 0,
	.renew_dur = 1500,
	.private_identity.addr = {0xCF, 0xFE, 0xFB, 0xDE, 0x11, 0x07},
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

/* Environment for the service */
struct service_env {
	uint16_t start_hdl;
	uint8_t user_lid;
	uint8_t char0_val;
	uint8_t char1_val;
	bool ntf_ongoing;
	uint16_t ntf_cfg;
};

/* Macros */
LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

/* function headers */
static uint16_t service_init(void);

/* Functions */

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

	led_state = false;
	led_cnt = 0;
	gpio_pin_set_dt(&led2, led_state);
	conn_status = BT_CONN_STATE_CONNECTED;
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

	led_state = false;
	led_cnt = 0;
	gpio_pin_set_dt(&led0, 0);
	conn_status = BT_CONN_STATE_DISCONNECTED;
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

static int set_advertising_data(uint8_t actv_idx)
{
	int ret;
	/* gatt service identifier */
	uint16_t svc[8] = {0xd123, 0xeabc, 0x785f, 0x1523, 0xefde, 0x1212, 0x1523, 0x0000};

	ret = bt_adv_data_set_tlv(GAP_AD_TYPE_COMPLETE_LIST_128_BIT_UUID, svc, sizeof(svc));
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
				.adv_intv_min = 160,
				.adv_intv_max = 800,
				.ch_map = ADV_ALL_CHNLS_EN,
				.phy = GAPM_PHY_TYPE_LE_1M,
			},
	};

	return bt_gapm_le_create_advertisement_service(adv_type, &adv_create_params, NULL,
						      &adv_actv_idx);
}

/* Add service to the stack */
static void server_configure(void)
{
	uint16_t err;

	err = service_init();

	if (err) {
		LOG_ERR("Error %u adding profile", err);
	}
}

/* Service callbacks */
static void on_att_read_get(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t hdl,
			    uint16_t offset, uint16_t max_length)
{
	co_buf_t *p_buf = NULL;
	uint16_t status = GAP_ERR_NO_ERROR;
	uint16_t att_val_len = 0;
	void *att_val = NULL;

	do {
		if (offset != 0) {
			/* Long read not supported for any characteristics within this service */
			status = ATT_ERR_INVALID_OFFSET;
			break;
		}

		uint8_t att_idx = hdl - env.start_hdl;

		switch (att_idx) {
		case LBS_IDX_CHAR0_VAL:
			att_val_len = sizeof(env.char0_val);
			att_val = &env.char0_val;
			LOG_DBG("read button state");
			break;

		case LBS_IDX_CHAR0_NTF_CFG:
			att_val_len = sizeof(env.ntf_cfg);
			att_val = &env.ntf_cfg;
			break;

		default:
			break;
		}

		if (att_val == NULL) {
			status = ATT_ERR_REQUEST_NOT_SUPPORTED;
			break;
		}

		status = co_buf_alloc(&p_buf, GATT_BUFFER_HEADER_LEN, att_val_len,
				      GATT_BUFFER_TAIL_LEN);
		if (status != CO_BUF_ERR_NO_ERROR) {
			status = ATT_ERR_INSUFF_RESOURCE;
			break;
		}

		memcpy(co_buf_data(p_buf), att_val, att_val_len);
	} while (0);

	/* Send the GATT response */
	gatt_srv_att_read_get_cfm(conidx, user_lid, token, status, att_val_len, p_buf);
	if (p_buf != NULL) {
		co_buf_release(p_buf);
	}
}

static void on_att_val_set(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t hdl,
			   uint16_t offset, co_buf_t *p_data)
{
	uint16_t status = GAP_ERR_NO_ERROR;

	do {
		if (offset != 0) {
			/* Long write not supported for any characteristics in this service */
			status = ATT_ERR_INVALID_OFFSET;
			break;
		}

		uint8_t att_idx = hdl - env.start_hdl;

		switch (att_idx) {
		case LBS_IDX_CHAR1_VAL: {
			if (sizeof(env.char1_val) != co_buf_data_len(p_data)) {
				LOG_DBG("Incorrect buffer size");
				status = ATT_ERR_INVALID_ATTRIBUTE_VAL_LEN;
			} else {
				memcpy(&env.char1_val, co_buf_data(p_data), sizeof(env.char1_val));
				LOG_DBG("TOGGLE LED, state %d", env.char1_val);
				if (env.char1_val) {
					gpio_pin_set_dt(&led0, 1);
				} else {
					gpio_pin_set_dt(&led0, 0);
				}
			}
			break;
		}

		case LBS_IDX_CHAR0_NTF_CFG: {
			if (sizeof(uint16_t) != co_buf_data_len(p_data)) {
				LOG_DBG("Incorrect buffer size");
				status = ATT_ERR_INVALID_ATTRIBUTE_VAL_LEN;
			} else {
				uint16_t cfg;

				memcpy(&cfg, co_buf_data(p_data), sizeof(uint16_t));
				if (PRF_CLI_START_NTF == cfg || PRF_CLI_STOP_NTFIND == cfg) {
					env.ntf_cfg = cfg;
				} else {
					/* Indications not supported */
					status = ATT_ERR_REQUEST_NOT_SUPPORTED;
				}
			}
			break;
		}

		default:
			status = ATT_ERR_REQUEST_NOT_SUPPORTED;
			break;
		}
	} while (0);

	/* Send the GATT write confirmation */
	gatt_srv_att_val_set_cfm(conidx, user_lid, token, status);
}

static void on_event_sent(uint8_t conidx, uint8_t user_lid, uint16_t metainfo, uint16_t status)
{
	if (metainfo == LBS_METAINFO_CHAR0_NTF_SEND) {
		env.ntf_ongoing = false;
	}
}

static const gatt_srv_cb_t gatt_cbs = {
	.cb_att_event_get = NULL,
	.cb_att_info_get = NULL,
	.cb_att_read_get = on_att_read_get,
	.cb_att_val_set = on_att_val_set,
	.cb_event_sent = on_event_sent,
};

/*
 * Service functions
 */
static uint16_t service_init(void)
{
	uint16_t status;

	/* Register a GATT user */
	status = gatt_user_srv_register(L2CAP_LE_MTU_MIN, 0, &gatt_cbs, &env.user_lid);
	if (status != GAP_ERR_NO_ERROR) {
		return status;
	}

	/* Add the GATT service */
	status = gatt_db_svc_add(env.user_lid, SVC_UUID(128), lbs_service_uuid, LBS_IDX_NB, NULL,
				 lbs_att_db, LBS_IDX_NB, &env.start_hdl);
	if (status != GAP_ERR_NO_ERROR) {
		gatt_user_unregister(env.user_lid);
		return status;
	}

	return GAP_ERR_NO_ERROR;
}

static uint16_t service_notification_send(uint32_t conidx_mask, uint8_t val)
{
	co_buf_t *p_buf;
	uint16_t status;
	uint8_t conidx = 0;

	env.char0_val = val;

	/* Cannot send another notification unless previous one is completed */
	if (env.ntf_ongoing) {
		return PRF_ERR_REQ_DISALLOWED;
	}

	/* Check notification subscription */
	if (env.ntf_cfg != PRF_CLI_START_NTF) {
		return PRF_ERR_NTF_DISABLED;
	}

	/* Get a buffer to put the notification data into */
	status = co_buf_alloc(&p_buf, GATT_BUFFER_HEADER_LEN, sizeof(env.char0_val),
			      GATT_BUFFER_TAIL_LEN);
	if (status != CO_BUF_ERR_NO_ERROR) {
		return GAP_ERR_INSUFF_RESOURCES;
	}

	memcpy(co_buf_data(p_buf), &env.char0_val, sizeof(env.char0_val));

	status = gatt_srv_event_send(conidx, env.user_lid, LBS_METAINFO_CHAR0_NTF_SEND, GATT_NOTIFY,
				     env.start_hdl + LBS_IDX_CHAR0_VAL, p_buf);

	co_buf_release(p_buf);

	if (status == GAP_ERR_NO_ERROR) {
		env.ntf_ongoing = true;
	}

	return status;
}

int main(void)
{
	uint16_t err;
	int res;

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

	/* Configure LED 0 */
	if (!gpio_is_ready_dt(&led0)) {
		LOG_ERR("led0 is not ready!");
		return 0;
	}

	res = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
	if (res < 0) {
		LOG_ERR("led0 configure failed");
		return 0;
	}

	/* LED initial state */
	gpio_pin_set_dt(&led0, 0);

	/* Configure LED 2 */
	if (!gpio_is_ready_dt(&led2)) {
		LOG_ERR("led2 is not ready!");
		return 0;
	}

	res = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE);
	if (res < 0) {
		LOG_ERR("led2 configure failed");
		return 0;
	}

	/* LED initial state */
	gpio_pin_set_dt(&led2, 0);
	led_state = false;
	led_cnt = 0;

	/* Configure button */
	if (!gpio_is_ready_dt(&button)) {
		LOG_ERR("Button is not ready");
		return 0;
	}

	res = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (res != 0) {
		LOG_ERR("Button configure failed");
		return 0;
	}

	while (1) {
		k_sleep(K_MSEC(100));

		if (conn_status != BT_CONN_STATE_CONNECTED) {
			led_cnt++;
			if (led_cnt >= 10) {
				led_cnt = 0;
				led_state = !led_state;
				gpio_pin_set_dt(&led2, led_state);
			}
		} else if ((conn_status == BT_CONN_STATE_CONNECTED) &&
			   (env.ntf_cfg == PRF_CLI_START_NTF) && (!env.ntf_ongoing)) {
			err = service_notification_send(UINT32_MAX, !gpio_pin_get_dt(&button));
			if (err) {
				LOG_ERR("Error %u sending measurement", err);
			}
		}
	}
	return 0;
}

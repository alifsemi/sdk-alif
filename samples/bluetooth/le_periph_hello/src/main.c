/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/* This application sends a string to the connected peer device.
 * The string length can be configured.
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
#include <alif/bluetooth/bt_adv_data.h>
#include <alif/bluetooth/bt_scan_rsp.h>
#include "gapm_api.h"

static uint8_t hello_arr[] = "HelloHello";
static uint8_t hello_arr_index;

/* Define advertising address type */
#define SAMPLE_ADDR_TYPE	ALIF_STATIC_RAND_ADDR

/* Store and share advertising address type */
static uint8_t adv_type;

/* Service Definitions */
#define ATT_128_PRIMARY_SERVICE  ATT_16_TO_128_ARRAY(GATT_DECL_PRIMARY_SERVICE)
#define ATT_128_INCLUDED_SERVICE ATT_16_TO_128_ARRAY(GATT_DECL_INCLUDE)
#define ATT_128_CHARACTERISTIC   ATT_16_TO_128_ARRAY(GATT_DECL_CHARACTERISTIC)
#define ATT_128_CLIENT_CHAR_CFG  ATT_16_TO_128_ARRAY(GATT_DESC_CLIENT_CHAR_CFG)
/* HELLO SERVICE and attribute 128 bit UUIDs */
#define HELLO_UUID_128_SVC                                                                           \
	{                                                                                          \
		0x12, 0x34, 0x56, 0x78, 0x90, 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89,      \
			0x90, 0x00, 0x00                                                           \
	}
#define HELLO_UUID_128_CHAR0                                                                         \
	{                                                                                          \
		0x12, 0x34, 0x56, 0x78, 0x90, 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89,      \
			0x15, 0x00, 0x00                                                           \
	}
#define HELLO_UUID_128_CHAR1                                                                         \
	{                                                                                          \
		0x12, 0x34, 0x56, 0x78, 0x90, 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89,     \
			0x16, 0x00, 0x00                                                           \
	}
#define HELLO_METAINFO_CHAR0_NTF_SEND 0x4321
#define ATT_16_TO_128_ARRAY(uuid)                                                                  \
	{                                                                                          \
		(uuid) & 0xFF, (uuid >> 8) & 0xFF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0        \
	}

/* gatt service identifier for advertising */
static uint16_t gatt_svc_id[8] = {
	0x3412, 0x7856, 0x1290, 0x3423,
	0x5645, 0x7867, 0x9089, 0x0000
};

/* List of attributes in the service */
enum service_att_list {
	HELLO_IDX_SERVICE = 0,
	/* First characteristic is readable + supports notifications */
	HELLO_IDX_CHAR0_CHAR,
	HELLO_IDX_CHAR0_VAL,
	HELLO_IDX_CHAR0_NTF_CFG,
	/* Second characteristic is writable */
	HELLO_IDX_CHAR1_CHAR,
	HELLO_IDX_CHAR1_VAL,
	/* Number of items*/
	HELLO_IDX_NB,
};

static uint8_t conn_status = BT_CONN_STATE_DISCONNECTED;
static uint8_t adv_actv_idx;
static struct service_env env;

/* Load name from configuration file */
#define DEVICE_NAME CONFIG_BLE_DEVICE_NAME

/* Service UUID to pass into gatt_db_svc_add */
static const uint8_t hello_service_uuid[] = HELLO_UUID_128_SVC;

/* GATT database for the service */
static const gatt_att_desc_t hello_att_db[HELLO_IDX_NB] = {
	[HELLO_IDX_SERVICE] = {ATT_128_PRIMARY_SERVICE, ATT_UUID(16) | PROP(RD), 0},

	[HELLO_IDX_CHAR0_CHAR] = {ATT_128_CHARACTERISTIC, ATT_UUID(16) | PROP(RD), 0},
	[HELLO_IDX_CHAR0_VAL] = {HELLO_UUID_128_CHAR0, ATT_UUID(128) | PROP(RD) | PROP(N),
			      OPT(NO_OFFSET)},
	[HELLO_IDX_CHAR0_NTF_CFG] = {ATT_128_CLIENT_CHAR_CFG, ATT_UUID(16) | PROP(RD) | PROP(WR), 0},

	[HELLO_IDX_CHAR1_CHAR] = {ATT_128_CHARACTERISTIC, ATT_UUID(16) | PROP(RD), 0},
	[HELLO_IDX_CHAR1_VAL] = {HELLO_UUID_128_CHAR1, ATT_UUID(128) | PROP(WR),
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
	uint8_t char0_val[250];
	uint8_t char1_val;
	bool ntf_ongoing;
	uint16_t ntf_cfg;
};

/* Macros */
LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

/* function headers */
static uint16_t service_init(void);

static uint16_t set_advertising_data(uint8_t actv_idx)
{
	int ret;

	ret = bt_adv_data_set_tlv(GAP_AD_TYPE_COMPLETE_LIST_128_BIT_UUID, gatt_svc_id,
				  sizeof(gatt_svc_id));
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
		case HELLO_IDX_CHAR0_VAL:
			att_val_len = CONFIG_HELLO_STRING_LENGTH;
			uint8_t loop_count = (CONFIG_HELLO_STRING_LENGTH / 5);
			if (CONFIG_HELLO_STRING_LENGTH % 5) {
				loop_count += 1;
			}
			for (int i=0; i < loop_count; i++) {
				memcpy(env.char0_val + i*5, &hello_arr[hello_arr_index], 5);
			}
			att_val = env.char0_val;
			LOG_DBG("read hello text");
			break;

		case HELLO_IDX_CHAR0_NTF_CFG:
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
		case HELLO_IDX_CHAR1_VAL: {
			if (sizeof(env.char1_val) != co_buf_data_len(p_data)) {
				LOG_DBG("Incorrect buffer size");
				status = ATT_ERR_INVALID_ATTRIBUTE_VAL_LEN;
			} else {
				memcpy(&env.char1_val, co_buf_data(p_data),
				       sizeof(env.char1_val));
				LOG_DBG("TOGGLE LED, state %d", env.char1_val);
			}
			break;
		}

		case HELLO_IDX_CHAR0_NTF_CFG: {
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
	if (metainfo == HELLO_METAINFO_CHAR0_NTF_SEND) {
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
	status = gatt_db_svc_add(env.user_lid,
					SVC_UUID(128),
					hello_service_uuid,
					HELLO_IDX_NB,
					NULL,
					hello_att_db,
					HELLO_IDX_NB,
					&env.start_hdl);
	if (status != GAP_ERR_NO_ERROR) {
		gatt_user_unregister(env.user_lid);
		return status;
	}

	return GAP_ERR_NO_ERROR;
}

static uint16_t service_notification_send(uint32_t conidx_mask)
{
	co_buf_t *p_buf;
	uint16_t status;
	uint8_t conidx = 0;

	/* Cannot send another notification unless previous one is completed */
	if (env.ntf_ongoing) {
		return PRF_ERR_REQ_DISALLOWED;
	}

	/* Check notification subscription */
	if (env.ntf_cfg != PRF_CLI_START_NTF) {
		return PRF_ERR_NTF_DISABLED;
	}

	/* Get a buffer to put the notification data into */
	status = co_buf_alloc(&p_buf, GATT_BUFFER_HEADER_LEN, CONFIG_HELLO_STRING_LENGTH,
			      GATT_BUFFER_TAIL_LEN);
	if (status != CO_BUF_ERR_NO_ERROR) {
		return GAP_ERR_INSUFF_RESOURCES;
	}

	uint8_t loop_count = (CONFIG_HELLO_STRING_LENGTH / 5);
	if (CONFIG_HELLO_STRING_LENGTH % 5) {
		loop_count += 1;
	}
	for (int i=0; i < loop_count; i++) {
		memcpy(env.char0_val + i*5, &hello_arr[hello_arr_index], 5);
	}

	memcpy(co_buf_data(p_buf), env.char0_val, CONFIG_HELLO_STRING_LENGTH);
	hello_arr_index++;
	if (hello_arr_index > 4) {
		hello_arr_index = 0;
	}

	status = gatt_srv_event_send(conidx, env.user_lid, HELLO_METAINFO_CHAR0_NTF_SEND,
					 GATT_NOTIFY, env.start_hdl + HELLO_IDX_CHAR0_VAL, p_buf);

	co_buf_release(p_buf);

	if (status == GAP_ERR_NO_ERROR) {
		env.ntf_ongoing = true;
	}

	return status;
}

void app_connection_status_update(enum gapm_connection_event con_event, uint8_t con_idx,
				  uint16_t status)
{
	switch (con_event) {
	case GAPM_API_SEC_CONNECTED_KNOWN_DEVICE:
		conn_status = BT_CONN_STATE_CONNECTED;
		LOG_INF("Connection index %u connected to known device", con_idx);
		break;
	case GAPM_API_DEV_CONNECTED:
		conn_status = BT_CONN_STATE_CONNECTED;
		LOG_INF("Connection index %u connected to new device", con_idx);
		break;
	case GAPM_API_DEV_DISCONNECTED:
		LOG_INF("Connection index %u disconnected for reason %u", con_idx, status);
		conn_status = BT_CONN_STATE_DISCONNECTED;
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
	hello_arr_index = 0;

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

		if ((conn_status == BT_CONN_STATE_CONNECTED) &&
		    (env.ntf_cfg == PRF_CLI_START_NTF) && (!env.ntf_ongoing)) {
			err = service_notification_send(UINT32_MAX);
			if (err) {
				LOG_ERR("Error %u sending Hello", err);
			}
		}
	}
	return 0;
}

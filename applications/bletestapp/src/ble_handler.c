/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include "ble_handler.h"

#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <se_service.h>
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

#include "appl_shell.h"
#include <alif/bluetooth/bt_adv_data.h>
#include <alif/bluetooth/bt_scan_rsp.h>
#include "gapm_api.h"

LOG_MODULE_DECLARE(main, CONFIG_MAIN_LOG_LEVEL);

static uint8_t hello_arr[] = "HelloHello";
static uint8_t hello_arr_index __attribute__((noinit));

/* Service Definitions */
#define ATT_128_PRIMARY_SERVICE    ATT_16_TO_128_ARRAY(GATT_DECL_PRIMARY_SERVICE)
#define ATT_128_INCLUDED_SERVICE   ATT_16_TO_128_ARRAY(GATT_DECL_INCLUDE)
#define ATT_128_CHARACTERISTIC     ATT_16_TO_128_ARRAY(GATT_DECL_CHARACTERISTIC)
#define ATT_128_CLIENT_CHAR_CFG    ATT_16_TO_128_ARRAY(GATT_DESC_CLIENT_CHAR_CFG)
/* HELLO SERVICE and attribute 128 bit UUIDs */
#define HELLO_UUID_128_SVC                                                                         \
	{0x12, 0x34, 0x56, 0x78, 0x90, 0x12, 0x23, 0x34,                                           \
	 0x45, 0x56, 0x67, 0x78, 0x89, 0x90, 0x00, 0x00}
#define HELLO_UUID_128_CHAR0                                                                       \
	{0x12, 0x34, 0x56, 0x78, 0x90, 0x12, 0x23, 0x34,                                           \
	 0x45, 0x56, 0x67, 0x78, 0x89, 0x15, 0x00, 0x00}
#define HELLO_UUID_128_CHAR1                                                                       \
	{0x12, 0x34, 0x56, 0x78, 0x90, 0x12, 0x23, 0x34,                                           \
	 0x45, 0x56, 0x67, 0x78, 0x89, 0x16, 0x00, 0x00}
#define HELLO_METAINFO_CHAR0_NTF_SEND 0x4321
#define ATT_16_TO_128_ARRAY(uuid)                                                                  \
	{(uuid) & 0xFF, (uuid >> 8) & 0xFF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}

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

static volatile bool is_connected __attribute__((noinit));
/* Store advertising activity index for re-starting after disconnection */
static volatile uint8_t conn_idx __attribute__((noinit));
static uint8_t adv_actv_idx __attribute__((noinit));
static struct service_env env __attribute__((noinit));

static volatile bool wakeup_status;
static volatile int run_profile_error;

static const char *device_name = app_shell_device_name;

/* Service UUID to pass into gatt_db_svc_add */
static const uint8_t hello_service_uuid[] = HELLO_UUID_128_SVC;

/* GATT database for the service */
static const gatt_att_desc_t hello_att_db[HELLO_IDX_NB] = {
	[HELLO_IDX_SERVICE] = {ATT_128_PRIMARY_SERVICE, ATT_UUID(16) | PROP(RD), 0},

	[HELLO_IDX_CHAR0_CHAR] = {ATT_128_CHARACTERISTIC, ATT_UUID(16) | PROP(RD), 0},
	[HELLO_IDX_CHAR0_VAL] = {HELLO_UUID_128_CHAR0, ATT_UUID(128) | PROP(RD) | PROP(N),
				 OPT(NO_OFFSET)},
	[HELLO_IDX_CHAR0_NTF_CFG] = {ATT_128_CLIENT_CHAR_CFG, ATT_UUID(16) | PROP(RD) | PROP(WR),
				     0},

	[HELLO_IDX_CHAR1_CHAR] = {ATT_128_CHARACTERISTIC, ATT_UUID(16) | PROP(RD), 0},
	[HELLO_IDX_CHAR1_VAL] = {HELLO_UUID_128_CHAR1, ATT_UUID(128) | PROP(WR),
				 OPT(NO_OFFSET) | sizeof(uint16_t)},
};

/**
 * Bluetooth stack configuration
 */
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
	.sugg_max_tx_octets = GAP_LE_MAX_OCTETS,
	.sugg_max_tx_time = GAP_LE_MAX_TIME,
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
	volatile bool ntf_ongoing;
	volatile uint16_t ntf_cfg;
};

static gapc_le_con_param_nego_with_ce_len_t preferred_connection_param = {.ce_len_min = 5,
									  .ce_len_max = 10,
									  .hdr.interval_min = 0,
									  .hdr.interval_max = 10,
									  .hdr.latency = 0,
									  .hdr.sup_to = 800};

/* function headers */
static uint16_t service_init(void);

/* Functions */

/**
 * Bluetooth GAPM callbacks
 */
void on_gapc_proc_cmp_cb(uint8_t conidx, uint32_t metainfo, uint16_t status)
{
	LOG_INF("%s conn:%d status:%d\n", __func__, conidx, status);
}

void on_bond_data_updated(uint8_t conidx, uint32_t metainfo, const gapc_bond_data_updated_t *p_data)
{
	LOG_DBG("%s", __func__);
}
void on_auth_payload_timeout(uint8_t conidx, uint32_t metainfo)
{
	LOG_DBG("%s", __func__);
}
void on_no_more_att_bearer(uint8_t conidx, uint32_t metainfo)
{
	LOG_DBG("%s", __func__);
}
void on_cli_hash_info(uint8_t conidx, uint32_t metainfo, uint16_t handle, const uint8_t *p_hash)
{
	LOG_DBG("%s", __func__);
}
void on_name_set(uint8_t conidx, uint32_t metainfo, uint16_t token, co_buf_t *p_buf)
{
	LOG_DBG("%s", __func__);
	gapc_le_set_name_cfm(conidx, token, GAP_ERR_NO_ERROR);
}
void on_appearance_set(uint8_t conidx, uint32_t metainfo, uint16_t token, uint16_t appearance)
{
	LOG_DBG("%s", __func__);
	gapc_le_set_appearance_cfm(conidx, token, GAP_ERR_NO_ERROR);
}

void on_param_update_req(uint8_t conidx, uint32_t metainfo, const gapc_le_con_param_nego_t *p_param)
{
	LOG_DBG("%s:%d", __func__, conidx);
	gapc_le_update_params_cfm(conidx, true, preferred_connection_param.ce_len_min,
				  preferred_connection_param.ce_len_max);
}
void on_param_updated(uint8_t conidx, uint32_t metainfo, const gapc_le_con_param_t *p_param)
{
	LOG_DBG("%s conn:%d", __func__, conidx);
}
void on_packet_size_updated(uint8_t conidx, uint32_t metainfo, uint16_t max_tx_octets,
			    uint16_t max_tx_time, uint16_t max_rx_octets, uint16_t max_rx_time)
{
	LOG_DBG("%s conn:%d max_tx_octets:%d max_tx_time:%d  max_rx_octets:%d "
		"max_rx_time:%d",
		__func__, conidx, max_tx_octets, max_tx_time, max_rx_octets, max_rx_time);

	/* PeHo: Seppo why this is done here? */
	const uint16_t ret =
		gapc_le_update_params(conidx, 0, &preferred_connection_param, on_gapc_proc_cmp_cb);

	LOG_INF("Update connection %u ret:%d\n", conidx, ret);
}

void on_phy_updated(uint8_t conidx, uint32_t metainfo, uint8_t tx_phy, uint8_t rx_phy)
{
	LOG_DBG("%s conn:%d tx_phy:%d rx_phy:%d", __func__, conidx, tx_phy, rx_phy);
}
void on_subrate_updated(uint8_t conidx, uint32_t metainfo,
			const gapc_le_subrate_t *p_subrate_params)
{
	LOG_DBG("%s conn:%d", __func__, conidx);
}

void app_connection_status_update(enum gapm_connection_event con_event, uint8_t con_idx,
				  uint16_t status)
{
	switch (con_event) {
	case GAPM_API_SEC_CONNECTED_KNOWN_DEVICE:
		is_connected = true;
		LOG_INF("BLE Connected conn:%d.", con_idx);
		break;
	case GAPM_API_DEV_CONNECTED:
		is_connected = true;
		LOG_INF("BLE Connected conn:%d.", con_idx);
		break;
	case GAPM_API_DEV_DISCONNECTED:
		is_connected = false;
		LOG_INF("BLE disconnected conn:%d. Waiting new connection", con_idx);
		break;
	case GAPM_API_PAIRING_FAIL:
		LOG_INF("Connection pairing index %u fail for reason %u", con_idx, status);
		break;
	}
}

static gapm_user_cb_t gapm_user_cb = {
	.connection_status_update = app_connection_status_update,
};

static uint16_t set_advertising_data(uint8_t actv_idx)
{
	bt_adv_data_set_name_auto(device_name, strlen(device_name));

	return bt_gapm_advertiment_data_set(actv_idx);
}

static uint16_t set_scan_data(uint8_t actv_idx)
{
	int ret;

	/* gatt service identifier */
	uint16_t svc[8] = {0xd123, 0xeabc, 0x785f, 0x1523, 0xefde, 0x1212, 0x1523, 0x0000};

	ret = bt_scan_rsp_set_tlv(GAP_AD_TYPE_COMPLETE_LIST_128_BIT_UUID, svc, sizeof(svc));
	if (ret) {
		LOG_ERR("Scan response UUID set fail %d", ret);
		return ATT_ERR_INSUFF_RESOURCE;
	}

	ret = bt_scan_rsp_data_set_name_auto(device_name, strlen(device_name));

	if (ret) {
		LOG_ERR("Scan response device name data fail %d", ret);
		return ATT_ERR_INSUFF_RESOURCE;
	}

	return bt_gapm_scan_response_set(actv_idx);
}

static uint16_t create_advertising(void)
{
	gapm_le_adv_create_param_t adv_create_params = {
		.prop = GAPM_ADV_PROP_UNDIR_CONN_MASK,
		.disc_mode = GAPM_ADV_MODE_GEN_DISC,
		.tx_pwr = 0,
		.filter_pol = GAPM_ADV_ALLOW_SCAN_ANY_CON_ANY,
		.prim_cfg = {
				.adv_intv_min = ble_adv_int_min,
				.adv_intv_max = ble_adv_int_max,
				.ch_map = ADV_ALL_CHNLS_EN,
				.phy = GAPM_PHY_TYPE_LE_1M,
			},
	};

	return bt_gapm_le_create_advertisement_service(GAPM_STATIC_ADDR, &adv_create_params, NULL,
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
			att_val_len = CONFIG_DATA_STRING_LENGTH;
			uint8_t loop_count = (CONFIG_DATA_STRING_LENGTH / 5);

			if (CONFIG_DATA_STRING_LENGTH % 5) {
				loop_count += 1;
			}
			for (int i = 0; i < loop_count; i++) {
				memcpy(env.char0_val + i * 5, &hello_arr[hello_arr_index], 5);
			}
			att_val = env.char0_val;
			LOG_DBG("read hello text");
			break;

		case HELLO_IDX_CHAR0_NTF_CFG:
			att_val_len = sizeof(env.ntf_cfg);
			att_val = (void *)&env.ntf_cfg;
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
				memcpy(&env.char1_val, co_buf_data(p_data), sizeof(env.char1_val));
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
	status = gatt_user_srv_register(CFG_MAX_LE_MTU, 0, &gatt_cbs, &env.user_lid);
	if (status != GAP_ERR_NO_ERROR) {
		return status;
	}

	/* Add the GATT service */
	status = gatt_db_svc_add(env.user_lid, SVC_UUID(128), hello_service_uuid, HELLO_IDX_NB,
				 NULL, hello_att_db, HELLO_IDX_NB, &env.start_hdl);
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

	ARG_UNUSED(conidx_mask);

	/* Cannot send another notification unless previous one is completed */
	if (env.ntf_ongoing) {
		return PRF_ERR_REQ_DISALLOWED;
	}

	/* Check notification subscription */
	if (env.ntf_cfg != PRF_CLI_START_NTF) {
		return PRF_ERR_NTF_DISABLED;
	}

	/* Get a buffer to put the notification data into */
	status = co_buf_alloc(&p_buf, GATT_BUFFER_HEADER_LEN, CONFIG_DATA_STRING_LENGTH,
			      GATT_BUFFER_TAIL_LEN);
	if (status != CO_BUF_ERR_NO_ERROR) {
		return GAP_ERR_INSUFF_RESOURCES;
	}

	uint8_t const loop_count = ((CONFIG_DATA_STRING_LENGTH + 4) / 5);

	for (int i = 0; i < loop_count; i++) {
		memcpy(env.char0_val + i * 5, &hello_arr[hello_arr_index], 5);
	}

	memcpy(co_buf_data(p_buf), env.char0_val, CONFIG_DATA_STRING_LENGTH);
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

int ble_init(void)
{
	/* Update preferred connection parameters */
	preferred_connection_param.hdr.interval_min = ble_conn_int_min;
	preferred_connection_param.hdr.interval_max = ble_conn_int_max;

	/* Start up bluetooth host stack. */
	uint16_t ble_status = alif_ble_enable(NULL);

	if (ble_status == 0) {
		uint16_t rc;
		/* BLE initialized first time */
		hello_arr_index = 0;
		conn_idx = GAP_INVALID_CONIDX;
		memset(&env, 0, sizeof(struct service_env));
		is_connected = false;

		/* Generate random address */
		se_service_get_rnd_num(&gapm_cfg.private_identity.addr[3], 3);
		/* Configure Bluetooth Stack */
		LOG_INF("Init gapm service");
		rc = bt_gapm_init(&gapm_cfg, &gapm_user_cb, device_name, strlen(device_name));
		if (rc) {
			LOG_ERR("gapm_configure error %u", rc);
			return -1;
		}

		server_configure();

		/* Create an advertising activity */
		rc = create_advertising();
		if (rc) {
			LOG_ERR("Advertisement create fail %u", rc);
			return -1;
		}

		rc = set_advertising_data(adv_actv_idx);
		if (rc) {
			LOG_ERR("Advertisement data set fail %u", rc);
			return -1;
		}

		rc = set_scan_data(adv_actv_idx);
		if (rc) {
			LOG_ERR("Scan response data set fail %u", rc);
			return -1;
		}

		rc = bt_gapm_advertisement_start(adv_actv_idx);
		if (rc) {
			LOG_ERR("Advertisement start fail %u", rc);
			return -1;
		}

		LOG_INF("Init complete!");
	}

	if (ble_status && ble_status != -EALREADY) {
		LOG_ERR("alif_ble_enable error %d", ble_status);
		return -1;
	}

	return 0;
}

int ble_uninit(void)
{
	is_connected = false;
	return alif_ble_disable();
}

bool ble_is_connected(void)
{
	return is_connected;
}

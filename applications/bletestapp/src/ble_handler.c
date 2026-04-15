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
#include <zephyr/shell/shell.h>
#include <zephyr/shell/shell_uart.h>
#include <alif/bluetooth/bt_adv_data.h>
#include <alif/bluetooth/bt_scan_rsp.h>
#include "gapm_api.h"

LOG_MODULE_DECLARE(main, CONFIG_MAIN_LOG_LEVEL);

static uint8_t hello_arr[] = "HelloHello";
static uint8_t hello_arr_index __attribute__((noinit));

/* BLE configurations*/
char __attribute__((noinit)) app_shell_device_name[DEVICE_NAME_LEN];

uint16_t ble_adv_int_min __attribute__((noinit));
uint16_t ble_adv_int_max __attribute__((noinit));
uint16_t ble_conn_int_min __attribute__((noinit));
uint16_t ble_conn_int_max __attribute__((noinit));

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

static volatile bool conn_status __attribute__((noinit));
/* Store advertising activity index for re-starting after disconnection */
static volatile uint8_t conn_idx __attribute__((noinit));
static uint8_t adv_actv_idx __attribute__((noinit));
static struct service_env env __attribute__((noinit));

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

static gapc_le_con_param_nego_with_ce_len_t app_preferred_connection_param = {.ce_len_min = 5,
									      .ce_len_max = 10,
									      .hdr.interval_min = 0,
									      .hdr.interval_max =
										      10,
									      .hdr.latency = 0,
									      .hdr.sup_to = 800};

/* function headers */
static uint16_t service_init(void);

/* Functions */

int ble_shell_reset(void)
{
	ble_adv_int_min = 1000;
	ble_adv_int_max = 1000;
	ble_conn_int_min = 800;
	ble_conn_int_max = 800;
	strncpy(app_shell_device_name, "test_shl", DEVICE_NAME_LEN - 1);
	app_shell_device_name[8] = 0;
	return 0;
}

SYS_INIT(ble_shell_reset, POST_KERNEL, 91);

/**
 * Bluetooth GAPM callbacks
 */
void on_gapc_proc_cmp_cb(uint8_t conidx, uint32_t metainfo, uint16_t status)
{
	LOG_INF("%s conn:%d status:%d\n", __func__, conidx, status);
}
static void app_connected_handler(uint8_t con_idx)
{
	const struct shell *shell = shell_backend_uart_get_ptr();
	uint16_t ret;

	conn_status = BT_CONN_STATE_CONNECTED;
	conn_idx = con_idx;
	ret = gapc_le_update_params(con_idx, 0, &app_preferred_connection_param,
				    on_gapc_proc_cmp_cb);

	shell_print(shell, "BLE Connected conn:%d", con_idx);
}

void app_connection_status_update(enum gapm_connection_event con_event, uint8_t con_idx,
				  uint16_t status)
{
	const struct shell *shell = shell_backend_uart_get_ptr();

	switch (con_event) {
	case GAPM_API_SEC_CONNECTED_KNOWN_DEVICE:
		app_connected_handler(con_idx);
		break;
	case GAPM_API_DEV_CONNECTED:
		app_connected_handler(con_idx);
		break;
	case GAPM_API_DEV_DISCONNECTED:
		conn_status = BT_CONN_STATE_DISCONNECTED;
		conn_idx = GAP_INVALID_CONIDX;
		shell_print(shell, "BLE disconnected conn:%d. Waiting new connection", con_idx);
		break;
	case GAPM_API_PAIRING_FAIL:
		shell_print(shell, "Connection pairing index %u fail for reason %u", con_idx,
			    status);
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

int ble_init(void)
{
	/* Update preferred connection parameters */
	app_preferred_connection_param.hdr.interval_min = ble_conn_int_min;
	app_preferred_connection_param.hdr.interval_max = ble_conn_int_max;

	/* Start up bluetooth host stack. */
	uint16_t ble_status = alif_ble_enable(NULL);

	if (ble_status && ble_status != -EALREADY) {
		LOG_ERR("alif_ble_enable error %d", ble_status);
		return -1;
	}
	return 0;
}

int ble_start(void)
{
	uint16_t rc;
	/* BLE initialized first time */
	hello_arr_index = 0;
	conn_idx = GAP_INVALID_CONIDX;
	memset(&env, 0, sizeof(struct service_env));
	conn_status = BT_CONN_STATE_DISCONNECTED;

	/* Set a preferred connections params  */
	bt_gapm_preferred_connection_paras_set(&app_preferred_connection_param);
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

	LOG_INF("start complete!");
	return 0;
}

int ble_uninit(void)
{
	conn_status = BT_CONN_STATE_DISCONNECTED;
	return alif_ble_disable();
}

bool ble_is_connected(void)
{
	return (BT_CONN_STATE_DISCONNECTED == BT_CONN_STATE_CONNECTED);
}

static int cmd_bt_disable(const struct shell *shell, size_t argc, char **argv)
{
	int ret;

	ble_shell_reset();
	ret = ble_uninit();
	shell_print(shell, "BLE application is restarted %d", ret);
	return 0;
}

static int cmd_bt_init(const struct shell *shell, size_t argc, char **argv)
{
	int ret = ble_init();

	shell_print(shell, "BLE stack init %d", ret);
	return 0;
}

static int cmd_bt_name(const struct shell *shell, size_t argc, char **argv)
{
	/* app_shell_device_name has null-ending as per cold boot init */
	strncpy(app_shell_device_name, argv[1], DEVICE_NAME_LEN - 1);
	shell_print(shell, "BLE name set to %s", app_shell_device_name);
	return 0;
}

static int cmd_bt_advertise(const struct shell *shell, size_t argc, char **argv)
{
	int ret;

	ret = ble_start();
	shell_print(shell, "BLE advertisement started %d", ret);
	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_cmds,
	SHELL_CMD_ARG(init, NULL, "[none]", cmd_bt_init, 1, 10),
	SHELL_CMD_ARG(advertise, NULL, "<type: off, on, nconn>", cmd_bt_advertise, 1, 10),
	SHELL_CMD_ARG(disable, NULL, "[none]", cmd_bt_disable, 1, 10),
	SHELL_CMD_ARG(name, NULL, "[name]", cmd_bt_name, 2, 10),
	SHELL_SUBCMD_SET_END);
SHELL_CMD_REGISTER(bt, &sub_cmds, "bt - Bluetooth shell commands", NULL);

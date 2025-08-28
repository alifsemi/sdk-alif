/** @file
 * @brief Bluetooth shell module
 *
 * Provide some Bluetooth shell commands that can be useful to applications.
 *
 * Copyright (c) 2017 Intel Corporation
 * Copyright (c) 2018 Nordic Semiconductor ASA
 * Copyright (c) Alif Semiconductor
 *
 * Alif Semiconductor version uses the Zephyr's bt shell module as a base
 * but adapts it to the Ceva-Waves' BLE stack.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/services/bas.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/printk.h>
#include <zephyr/shell/shell.h>
#include <zephyr/shell/shell_uart.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
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

LOG_MODULE_REGISTER(bt_shell, CONFIG_BT_SHELL_LOG_LEVEL);

#define BT_CONN_STATE_CONNECTED	   0x00
#define BT_CONN_STATE_DISCONNECTED 0x01

#define HELLO_UUID_128_SVC                                                                         \
	{                                                                                          \
		0x12, 0x34, 0x56, 0x78, 0x90, 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89,      \
			0x90, 0x00, 0x00                                                           \
	}

#define HELLO_UUID_128_CHAR0                                                                       \
	{                                                                                          \
		0x12, 0x34, 0x56, 0x78, 0x90, 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89,      \
			0x15, 0x00, 0x00                                                           \
	}
#define HELLO_UUID_128_CHAR1                                                                       \
	{                                                                                          \
		0x12, 0x34, 0x56, 0x78, 0x90, 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89,      \
			0x16, 0x00, 0x00                                                           \
	}

#define ATT_16_TO_128_ARRAY(uuid)                                                          \
{                                                                                          \
	(uuid) & 0xFF, (uuid >> 8) & 0xFF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0        \
}

#define ATT_128_PRIMARY_SERVICE  ATT_16_TO_128_ARRAY(GATT_DECL_PRIMARY_SERVICE)
#define ATT_128_INCLUDED_SERVICE ATT_16_TO_128_ARRAY(GATT_DECL_INCLUDE)
#define ATT_128_CHARACTERISTIC   ATT_16_TO_128_ARRAY(GATT_DECL_CHARACTERISTIC)
#define ATT_128_CLIENT_CHAR_CFG  ATT_16_TO_128_ARRAY(GATT_DESC_CLIENT_CHAR_CFG)

#define HELLO_METAINFO_CHAR0_NTF_SEND 0x4321
#define NAME_LEN                      30
#define KEY_STR_LEN                   33
#define ADV_DATA_DELIMITER            ", "
#define AD_SIZE                       9

#ifndef CONFIG_HELLO_STRING_LENGTH
#define CONFIG_HELLO_STRING_LENGTH 10
#endif /* CONFIG_HELLO_STRING_LENGTH */

/*
 * Semaphores
 */
static K_SEM_DEFINE(bt_init_sem, 0, 1);
static K_SEM_DEFINE(bt_process_sem, 0, 1);

/*
 * Connection parameters
 */
static struct connections_params {
	bool no_settings_load; /* Start without loading settings */
	bool bt_initialized;   /* Bluetooth initialized */
	uint8_t status;        /* Connection status */
} cxn;

/*
 * Service environment
 */
static struct service_env {
	uint16_t start_hdl;
	uint8_t user_lid;
	uint8_t char0_val[250];
	uint8_t char1_val;
	bool ntf_ongoing;
	uint16_t ntf_cfg;
	uint8_t hello_arr[11]; /* "HelloHello" + null terminator */
	uint8_t hello_arr_index;
} srv_env;

/*
 * Advertising parameters
 */
static struct adv_params {
	gapm_le_adv_create_param_t param;
	bool valid;       /* Are the stored advertising parameters valid */
	uint8_t actv_idx; /* Activity index of the effective advertising parameters */
} stored_adv __attribute__((noinit));

/*
 * List of attributes in the service
 */
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

/* Device name */
static const char device_name[] = CONFIG_BLE_DEVICE_NAME;

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

/* Convert MAC address string from Kconfig to byte array */
static void init_private_addr(uint8_t *addr)
{
	/* Parse CONFIG_BT_SHELL_PRIVATE_ADDR string (format: XX:XX:XX:XX:XX:XX) */
	char addr_str[] = CONFIG_BT_SHELL_PRIVATE_ADDR;
	char *ptr = addr_str;
	char byte_str[3];

	byte_str[sizeof(byte_str) - 1] = '\0';

	/* Parse each byte of the address */
	for (int i = 5; i >= 0; i--) {
		/* Copy two hex chars */
		byte_str[0] = *ptr++;
		byte_str[1] = *ptr++;
		/* Convert to byte */
		addr[i] = strtoul(byte_str, NULL, 16);
		/* Skip colon */
		if (i > 0) {
			ptr++;
		}
	}
}

/* GAP role configuration based on Kconfig choice */
#if defined(CONFIG_BT_SHELL_GAP_ROLE_NONE)
#define BT_SHELL_GAP_ROLE GAP_ROLE_NONE
#elif defined(CONFIG_BT_SHELL_GAP_ROLE_LE_OBSERVER)
#define BT_SHELL_GAP_ROLE GAP_ROLE_LE_OBSERVER
#elif defined(CONFIG_BT_SHELL_GAP_ROLE_LE_BROADCASTER)
#define BT_SHELL_GAP_ROLE GAP_ROLE_LE_BROADCASTER
#elif defined(CONFIG_BT_SHELL_GAP_ROLE_LE_CENTRAL)
#define BT_SHELL_GAP_ROLE GAP_ROLE_LE_CENTRAL
#elif defined(CONFIG_BT_SHELL_GAP_ROLE_LE_PERIPHERAL)
#define BT_SHELL_GAP_ROLE GAP_ROLE_LE_PERIPHERAL
#elif defined(CONFIG_BT_SHELL_GAP_ROLE_LE_ALL)
#define BT_SHELL_GAP_ROLE GAP_ROLE_LE_ALL
#else
#error "Invalid GAP role configuration"
#endif

/* Pairing mode configuration */
#if defined(CONFIG_BT_SHELL_PAIRING_DISABLE)
#define BT_SHELL_PAIRING_MODE GAPM_PAIRING_DISABLE
#elif defined(CONFIG_BT_SHELL_PAIRING_LEGACY)
#define BT_SHELL_PAIRING_MODE GAPM_PAIRING_LEGACY
#elif defined(CONFIG_BT_SHELL_PAIRING_SEC_CON)
#define BT_SHELL_PAIRING_MODE GAPM_PAIRING_SEC_CON
#elif defined(CONFIG_BT_SHELL_PAIRING_CT2)
#define BT_SHELL_PAIRING_MODE GAPM_PAIRING_CT2
#elif defined(CONFIG_BT_SHELL_PAIRING_BT_SSP)
#define BT_SHELL_PAIRING_MODE GAPM_PAIRING_BT_SSP
#elif defined(CONFIG_BT_SHELL_PAIRING_ALL)
#define BT_SHELL_PAIRING_MODE GAPM_PAIRING_MODE_ALL
#else
#error "Invalid pairing mode configuration"
#endif

/* PHY preferences configuration */
#if defined(CONFIG_BT_SHELL_PHY_1MBPS_TX)
#define BT_SHELL_TX_PREF_PHY GAP_PHY_1MBPS
#elif defined(CONFIG_BT_SHELL_PHY_2MBPS_TX)
#define BT_SHELL_TX_PREF_PHY GAP_PHY_2MBPS
#elif defined(CONFIG_BT_SHELL_PHY_CODED_TX)
#define BT_SHELL_TX_PREF_PHY GAP_PHY_CODED
#elif defined(CONFIG_BT_SHELL_PHY_ANY_TX)
#define BT_SHELL_TX_PREF_PHY GAP_PHY_ANY
#else
#error "Invalid TX PHY preference configuration"
#endif

#if defined(CONFIG_BT_SHELL_PHY_1MBPS_RX)
#define BT_SHELL_RX_PREF_PHY GAP_PHY_1MBPS
#elif defined(CONFIG_BT_SHELL_PHY_2MBPS_RX)
#define BT_SHELL_RX_PREF_PHY GAP_PHY_2MBPS
#elif defined(CONFIG_BT_SHELL_PHY_CODED_RX)
#define BT_SHELL_RX_PREF_PHY GAP_PHY_CODED
#elif defined(CONFIG_BT_SHELL_PHY_ANY_RX)
#define BT_SHELL_RX_PREF_PHY GAP_PHY_ANY
#else
#error "Invalid RX PHY preference configuration"
#endif

/* Privacy configuration */
#if defined(CONFIG_BT_SHELL_PRIVACY_DISABLED)
#define BT_SHELL_PRIVACY_CFG 0 /* public address, controller privacy disabled*/
#elif defined(CONFIG_BT_SHELL_PRIVACY_ENABLED)
#define BT_SHELL_PRIVACY_CFG 1 /* static private random address, controller privacy disabled */
#else
#error "Invalid privacy configuration"
#endif

/* Bluetooth stack configuration*/
static const gapm_config_t gapm_cfg = {
	.role = BT_SHELL_GAP_ROLE,
	.pairing_mode = BT_SHELL_PAIRING_MODE,
	.privacy_cfg = BT_SHELL_PRIVACY_CFG,
	.renew_dur = CONFIG_BT_SHELL_RENEW_DUR,
	.private_identity.addr = {0}, /* Will be set from Kconfig in init_private_addr() */
	.irk.key = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	.gap_start_hdl = 0,
	.gatt_start_hdl = 0,
	.att_cfg = CONFIG_BT_SHELL_ATT_CFG,
	.sugg_max_tx_octets = CONFIG_BT_SHELL_MAX_TX_OCTETS,
	.sugg_max_tx_time = CONFIG_BT_SHELL_MAX_TX_TIME,
	.tx_pref_phy = BT_SHELL_TX_PREF_PHY,
	.rx_pref_phy = BT_SHELL_RX_PREF_PHY,
	.tx_path_comp = CONFIG_BT_SHELL_TX_PATH_COMP,
	.rx_path_comp = CONFIG_BT_SHELL_RX_PATH_COMP,
	.class_of_device = 0,  /* BT Classic only */
	.dflt_link_policy = 0, /* BT Classic only */
};

/**
 * Bluetooth GAPM callbacks
 */
static void on_le_connection_req(uint8_t conidx, uint32_t metainfo, uint8_t actv_idx, uint8_t role,
				 const gap_bdaddr_t *p_peer_addr,
				 const gapc_le_con_param_t *p_con_params, uint8_t clk_accuracy)
{
	LOG_INF("Connection request on index %u", conidx);
	gapc_le_connection_cfm(conidx, 0, NULL);

	LOG_INF("Connection parameters: interval %u, latency %u, supervision timeout %u",
		p_con_params->interval, p_con_params->latency, p_con_params->sup_to);

	LOG_INF("Peer BD address %02X:%02X:%02X:%02X:%02X:%02X (conidx: %u)", p_peer_addr->addr[5],
		p_peer_addr->addr[4], p_peer_addr->addr[3], p_peer_addr->addr[2],
		p_peer_addr->addr[1], p_peer_addr->addr[0], conidx);

	cxn.status = BT_CONN_STATE_CONNECTED;
}

static void on_key_received(uint8_t conidx, uint32_t metainfo, const gapc_pairing_keys_t *p_keys)
{
	LOG_INF("Unexpected key received key on conidx %u", conidx);
}

static void on_disconnection(uint8_t conidx, uint32_t metainfo, uint16_t reason)
{
	LOG_INF("Connection index %u disconnected for reason %u", conidx, reason);

	cxn.status = BT_CONN_STATE_DISCONNECTED;
}

static void on_name_get(uint8_t conidx, uint32_t metainfo, uint16_t token, uint16_t offset,
			uint16_t max_len)
{
	const size_t device_name_len = sizeof(device_name) - 1;
	const size_t short_len = (device_name_len > max_len ? max_len : device_name_len);

	gapc_le_get_name_cfm(conidx, token, GAP_ERR_NO_ERROR, device_name_len, short_len,
			     (const uint8_t *)device_name);
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

#if !CONFIG_ALIF_BLE_ROM_IMAGE_V1_0 /* ROM version > 1.0 */
static void on_gapm_err(uint32_t metainfo, uint8_t code)
{
	LOG_ERR("GAPM operation failed with error code: 0x%02x", code);
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
#else
static void on_gapm_err(enum co_error err)
{
	LOG_ERR("GAPM operation failed with error code: 0x%02x", err);
}

static const gapm_err_info_config_cb_t gapm_err_cbs = {
	.ctrl_hw_error = on_gapm_err,
};

static const gapm_callbacks_t gapm_cbs = {
	.p_con_req_cbs = &gapc_con_cbs,
	.p_sec_cbs = &gapc_sec_cbs,
	.p_info_cbs = &gapc_con_inf_cbs,
	.p_le_config_cbs = &gapc_le_cfg_cbs,
	.p_bt_config_cbs = NULL, /* BT classic so not required */
	.p_err_info_config_cbs = &gapm_err_cbs,
};
#endif

/**
 * Advertising callbacks
 */
static void on_adv_actv_stopped(uint32_t metainfo, uint8_t actv_idx, uint16_t reason)
{
	LOG_INF("Advertising activity index %u stopped for reason %u", actv_idx, reason);
}

static uint16_t set_advertising_data(uint8_t actv_idx)
{
	uint16_t err;

	/* gatt service identifier */
	uint16_t svc[8] = {
		0xd123, 0xeabc, 0x785f, 0x1523,
		0xefde, 0x1212, 0x1523, 0x0000
	};

	/* Name advertising length */
	const size_t device_name_len = sizeof(device_name) - 1;

	/* Check if device name is too long for advertising data */
	const uint16_t max_adv_data_size = 31; /* BLE spec limit for advertising data */
	const uint16_t adv_device_name = GATT_HANDLE_LEN + device_name_len;
	const uint16_t adv_uuid_svc = GATT_HANDLE_LEN + GATT_UUID_128_LEN;
	const uint16_t adv_len = adv_device_name + adv_uuid_svc;

	if (adv_len > max_adv_data_size) {
		LOG_ERR("Device name exceeds maximum length: %u bytes", device_name_len);
		LOG_ERR("Advertising data requires %u bytes (maximum allowed: %u)", adv_len,
			max_adv_data_size);
		LOG_ERR("Shorten CONFIG_BLE_DEVICE_NAME in prj.conf to fix this issue");
		return GAP_ERR_INVALID_PARAM;
	}

	co_buf_t *p_buf;
	uint8_t *p_data;

	err = co_buf_alloc(&p_buf, 0, adv_len, 0);
	if (err != 0) {
		LOG_ERR("Cannot allocate buffer for advertising data");
		return err;
	}

	p_data = co_buf_data(p_buf);

	/* Device name data */
	p_data[0] = device_name_len + 1;
	p_data[1] = GAP_AD_TYPE_COMPLETE_NAME;
	memcpy(p_data + 2, device_name, device_name_len);

	/* Update data pointer */
	p_data = p_data + adv_device_name;

	/* Service UUID data */
	p_data[0] = GATT_UUID_128_LEN + 1;
	p_data[1] = GAP_AD_TYPE_COMPLETE_LIST_128_BIT_UUID;
	memcpy(p_data + 2, &svc, sizeof(svc));

	err = gapm_le_set_adv_data(actv_idx, p_buf);
	co_buf_release(p_buf);

	if (err) {
		if (err == GAP_ERR_INVALID_PARAM) {
			LOG_ERR("Advertising data contains invalid parameter (0x%02x)", err);
			LOG_ERR("Verify device name length and service UUID format");
		} else {
			LOG_ERR("Cannot set advertising data, error code: 0x%02x", err);
		}
	}

	return err;
}

static uint16_t set_scan_data(uint8_t actv_idx)
{
	co_buf_t *p_buf;
	uint16_t err = co_buf_alloc(&p_buf, 0, 0, 0);

	if (err) {
		LOG_ERR("Cannot allocate buffer for scan response data");
		return err;
	}

	err = gapm_le_set_scan_response_data(actv_idx, p_buf);
	if (err) {
		LOG_ERR("Cannot set scan response data, error code: 0x%02x", err);
	}

	return err;
}

static void on_adv_actv_proc_cmp(uint32_t metainfo, uint8_t proc_id, uint8_t actv_idx,
				 uint16_t status)
{
	if (status) {
		LOG_ERR("Advertising activity failed with error code: 0x%02x", status);
		return;
	}

	switch (proc_id) {
	case GAPM_ACTV_CREATE_LE_ADV:
		LOG_INF("Created advertising activity");
		stored_adv.actv_idx = actv_idx;
		set_advertising_data(actv_idx);
		break;
	case GAPM_ACTV_SET_ADV_DATA:
		LOG_INF("Set advertising data");
		set_scan_data(actv_idx);
		break;
	case GAPM_ACTV_SET_SCAN_RSP_DATA:
		LOG_INF("Set scan response data");
		break;
	case GAPM_ACTV_START:
		LOG_INF("Started advertising");
		break;
	case GAPM_ACTV_STOP:
		LOG_INF("Stopped advertising");
		break;
	case GAPM_ACTV_DELETE:
		LOG_INF("Deleted advertising activity");
		stored_adv.actv_idx = 0xFF;
		break;
	default:
		__ASSERT(false, "Received unexpected GAPM activity completion, proc_id %u",
			 proc_id);
		LOG_WRN("Received unexpected GAPM activity completion, proc_id %u", proc_id);
		break;
	}

	k_sem_give(&bt_process_sem);
}

static void on_adv_created(uint32_t metainfo, uint8_t actv_idx, int8_t tx_pwr)
{
	stored_adv.actv_idx = actv_idx;
	LOG_INF("Created advertising activity with index %u, tx power %d dBm", actv_idx, tx_pwr);
}

static const gapm_le_adv_cb_actv_t le_adv_cbs = {
	.hdr.actv.stopped = on_adv_actv_stopped,
	.hdr.actv.proc_cmp = on_adv_actv_proc_cmp,
	.created = on_adv_created,
};

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

		uint8_t att_idx = hdl - srv_env.start_hdl;

		switch (att_idx) {
		case HELLO_IDX_CHAR0_VAL:
			att_val_len = CONFIG_HELLO_STRING_LENGTH;
			uint8_t loop_count = (CONFIG_HELLO_STRING_LENGTH / 5);

			if (CONFIG_HELLO_STRING_LENGTH % 5) {
				loop_count += 1;
			}
			for (int i = 0; i < loop_count; i++) {
				memcpy(srv_env.char0_val + i * 5,
				       &srv_env.hello_arr[srv_env.hello_arr_index], 5);
			}
			att_val = srv_env.char0_val;
			LOG_DBG("Preparing response for read request");
			break;

		case HELLO_IDX_CHAR0_NTF_CFG:
			att_val_len = sizeof(srv_env.ntf_cfg);
			att_val = &srv_env.ntf_cfg;
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

		uint8_t att_idx = hdl - srv_env.start_hdl;

		switch (att_idx) {
		case HELLO_IDX_CHAR1_VAL: {
			if (sizeof(srv_env.char1_val) != co_buf_data_len(p_data)) {
				LOG_ERR("Incorrect buffer size");
				status = ATT_ERR_INVALID_ATTRIBUTE_VAL_LEN;
			} else {
				memcpy(&srv_env.char1_val, co_buf_data(p_data),
				       sizeof(srv_env.char1_val));
				LOG_DBG("led toggle, state %d", srv_env.char1_val);
			}
			break;
		}

		case HELLO_IDX_CHAR0_NTF_CFG: {
			if (sizeof(uint16_t) != co_buf_data_len(p_data)) {
				LOG_ERR("Incorrect buffer size");
				status = ATT_ERR_INVALID_ATTRIBUTE_VAL_LEN;
			} else {
				uint16_t cfg;

				memcpy(&cfg, co_buf_data(p_data), sizeof(uint16_t));
				if (PRF_CLI_START_NTF == cfg || PRF_CLI_STOP_NTFIND == cfg) {
					srv_env.ntf_cfg = cfg;
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
		srv_env.ntf_ongoing = false;
	}
}

static const gatt_srv_cb_t gatt_cbs = {
	.cb_att_event_get = NULL,
	.cb_att_info_get = NULL,
	.cb_att_read_get = on_att_read_get,
	.cb_att_val_set = on_att_val_set,
	.cb_event_sent = on_event_sent,
};


static uint16_t service_init(void)
{
	uint16_t status;

	/* Initialize hello_arr with the string "HelloHello" */
	static const char hello_str[] = "HelloHello";

	memcpy(srv_env.hello_arr, hello_str, sizeof(hello_str));
	srv_env.hello_arr_index = 0;

	/* Register a GATT user */
	status = gatt_user_srv_register(L2CAP_LE_MTU_MIN, 0, &gatt_cbs, &srv_env.user_lid);
	if (status != GAP_ERR_NO_ERROR) {
		LOG_ERR("Cannot register GATT user service, error code: 0x%02x", status);
		return status;
	}

	/* Add the GATT service */
	status = gatt_db_svc_add(srv_env.user_lid, SVC_UUID(128), hello_service_uuid, HELLO_IDX_NB,
				 NULL, hello_att_db, HELLO_IDX_NB, &srv_env.start_hdl);
	if (status != GAP_ERR_NO_ERROR) {
		gatt_user_unregister(srv_env.user_lid);
		LOG_ERR("Cannot add GATT service to database, error code: 0x%02x", status);
		return status;
	}

	LOG_DBG("GATT service added");

	return GAP_ERR_NO_ERROR;
}

static void on_gapm_process_complete(uint32_t metainfo, uint16_t status)
{
	uint16_t err;

	if (status) {
		LOG_ERR("gapm process completed with error 0x%02x", status);
		goto end;
	}

	LOG_INF("GAPM configuration succeeded");

	err = service_init();

	if (err) {
		LOG_ERR("Cannot add BLE profile, error code: 0x%02x", err);
		goto end;
	}

	cxn.bt_initialized = true;

end:
	k_sem_give(&bt_init_sem);
}

static void on_ble_enabled(void)
{
	int err = gapm_configure(0, &gapm_cfg, &gapm_cbs, on_gapm_process_complete);

	if (err) {
		LOG_ERR("Cannot configure GAPM, error code: %u", err);
		return;
	}
}

static bool is_initialized(const struct shell *sh)
{
	if (!cxn.bt_initialized) {
		shell_error(sh, "BLE stack not initialized. Run 'bt init' first or wait for "
				"initialization to complete.");
		return false;
	}
	return true;
}

static int cmd_init(const struct shell *sh, size_t argc, char *argv[])
{
	int err;
	bool sync = false;

	stored_adv.actv_idx = 0xFF;
	stored_adv.valid = false;

	if (cxn.bt_initialized) {
		shell_error(sh, "BLE stack already initialized");
		return -EALREADY;
	}

	/* Initialize the private address from Kconfig directly into the gapm_cfg structure */
	init_private_addr((void *)gapm_cfg.private_identity.addr);

	for (size_t argn = 1; argn < argc; argn++) {
		const char *arg = argv[argn];

		if (!strcmp(arg, "no-settings-load")) {
			cxn.no_settings_load = true;
		} else if (!strcmp(arg, "sync")) {
			sync = true;
		} else {
			shell_help(sh);
			return SHELL_CMD_HELP_PRINTED;
		}
	}

	/* Enable only fails if it was already enabled */
	if (sync) {
		err = alif_ble_enable(NULL);
		on_ble_enabled();
		k_sem_take(&bt_init_sem, K_FOREVER);
	} else {
		err = alif_ble_enable(on_ble_enabled);
	}

	if (err) {
		shell_error(sh, "Failed to initialize BLE stack: %d", err);
	} else {
		shell_print(sh, "Initialized BLE stack");
	}

	return err;
}

static bool adv_param_parse(const struct shell *sh, size_t argc, char *argv[],
			    gapm_le_adv_create_param_t *param)
{
	if (argc < 2) {
		shell_error(sh, "Specify advertising type");
		return false;
	}

	/* Set default values */
	param->prop = GAPM_ADV_PROP_UNDIR_CONN_MASK, param->disc_mode = GAPM_ADV_MODE_GEN_DISC,
#if !CONFIG_ALIF_BLE_ROM_IMAGE_V1_0 /* ROM version > 1.0 */
	param->tx_pwr = 0,
#else
	param->max_tx_pwr = 0,
#endif /* !CONFIG_ALIF_BLE_ROM_IMAGE_V1_0 */
	param->filter_pol = GAPM_ADV_ALLOW_SCAN_ANY_CON_ANY,
	param->prim_cfg.adv_intv_max = 800; /* 500 ms */
	param->prim_cfg.adv_intv_min = 160; /* 100 ms */
	param->prim_cfg.ch_map = ADV_ALL_CHNLS_EN;
	param->prim_cfg.phy = GAPM_PHY_TYPE_LE_1M;

	if (!strcmp(argv[1], "conn-scan")) {
		param->prop = GAPM_ADV_PROP_UNDIR_CONN_MASK;
	} else if (!strcmp(argv[1], "conn-nscan")) {
		param->prop = GAPM_ADV_PROP_CONNECTABLE_BIT;
	} else if (!strcmp(argv[1], "nconn-scan")) {
		param->prop = GAPM_ADV_PROP_SCANNABLE_BIT;
	} else if (!strcmp(argv[1], "nconn-nscan")) {
		param->prop = GAPM_ADV_PROP_NON_CONN_NON_SCAN_MASK;
	} else {
		shell_error(sh, "Provide a valid advertising type");
		return false;
	}

	for (size_t argn = 2; argn < argc; argn++) {
		const char *arg = argv[argn];

		if (!strcmp(arg, "disable-37")) {
			param->prim_cfg.ch_map &= ~ADV_CHNL_37_EN;
		} else if (!strcmp(arg, "disable-38")) {
			param->prim_cfg.ch_map &= ~ADV_CHNL_38_EN;
		} else if (!strcmp(arg, "disable-39")) {
			param->prim_cfg.ch_map &= ~ADV_CHNL_39_EN;
		} else {
			shell_error(sh, "Provide valid advertising options");
			return false;
		}
	}

	return true;
}

static int cmd_adv_create(const struct shell *sh, size_t argc, char *argv[])
{
	gapm_le_adv_create_param_t param;
	int err;

	if (!is_initialized(sh)) {
		return -ENOEXEC;
	}

	if (!adv_param_parse(sh, argc, argv, &param)) {
		shell_help(sh);
		return -ENOEXEC;
	}

	err = gapm_le_create_adv_legacy(0, GAPM_STATIC_ADDR, &param, &le_adv_cbs);

	if (!err) {
		memcpy(&stored_adv.param, &param, sizeof(gapm_le_adv_create_param_t));
		stored_adv.valid = true;
	} else {
		shell_error(sh, "Failed to create advertiser set (0x%02x)", err);
	}

	return err;
}

static int cmd_adv_param(const struct shell *sh, size_t argc, char *argv[])
{
	gapm_le_adv_create_param_t param;
	int err;

	if (!is_initialized(sh)) {
		return -ENOEXEC;
	}

	if (!stored_adv.valid) {
		shell_error(
			sh,
			"Initialize advertising parameters first. Run 'bt adv-create' command.");
		return -EINVAL;
	}

	if (argc < 2) {
		/* Display current parameters */
		shell_print(sh, "Current advertising parameters:");
		shell_print(sh, "  Type: %s",
			    (stored_adv.param.prop == GAPM_ADV_PROP_UNDIR_CONN_MASK) ? "conn-scan"
			    : (stored_adv.param.prop == GAPM_ADV_PROP_CONNECTABLE_BIT)
				    ? "conn-nscan"
			    : (stored_adv.param.prop == GAPM_ADV_PROP_SCANNABLE_BIT) ? "nconn-scan"
			    : (stored_adv.param.prop == GAPM_ADV_PROP_NON_CONN_NON_SCAN_MASK)
				    ? "nconn-nscan"
				    : "unknown");
		shell_print(sh, "  Interval: min %u ms, max %u ms",
			    (stored_adv.param.prim_cfg.adv_intv_min * 625) / 1000,
			    (stored_adv.param.prim_cfg.adv_intv_max * 625) / 1000);
		shell_print(sh, "  Channels: %s %s %s",
			    (stored_adv.param.prim_cfg.ch_map & ADV_CHNL_37_EN) ? "37 " : "",
			    (stored_adv.param.prim_cfg.ch_map & ADV_CHNL_38_EN) ? "38 " : "",
			    (stored_adv.param.prim_cfg.ch_map & ADV_CHNL_39_EN) ? "39" : "");
		return 0;
	}

	/* Start with stored parameters */
	memcpy(&param, &stored_adv.param, sizeof(gapm_le_adv_create_param_t));

	/* Check for individual parameter updates */
	for (size_t argn = 1; argn < argc; argn++) {
		const char *arg = argv[argn];

		if (!strcmp(argv[argn], "conn-scan")) {
			param.prop = GAPM_ADV_PROP_UNDIR_CONN_MASK;
		} else if (!strcmp(argv[argn], "conn-nscan")) {
			param.prop = GAPM_ADV_PROP_CONNECTABLE_BIT;
		} else if (!strcmp(argv[argn], "nconn-scan")) {
			param.prop = GAPM_ADV_PROP_SCANNABLE_BIT;
		} else if (!strcmp(argv[argn], "nconn-nscan")) {
			param.prop = GAPM_ADV_PROP_NON_CONN_NON_SCAN_MASK;
		} else if (!strcmp(arg, "interval-min")) {
			if (++argn == argc) {
				shell_error(sh, "Specify interval value in milliseconds");
				return -EINVAL;
			}

			uint32_t interval_ms = strtoul(argv[argn], NULL, 10);
			/* Convert from ms to 0.625ms units */
			param.prim_cfg.adv_intv_min = (interval_ms * 1000) / 625;
			shell_print(sh, "Set minimum advertising interval to %u ms (%u units)",
				    interval_ms, param.prim_cfg.adv_intv_min);
		} else if (!strcmp(arg, "interval-max")) {
			if (++argn == argc) {
				shell_error(sh, "Specify interval value in milliseconds");
				return -EINVAL;
			}

			uint32_t interval_ms = strtoul(argv[argn], NULL, 10);
			/* Convert from ms to 0.625ms units */
			param.prim_cfg.adv_intv_max = (interval_ms * 1000) / 625;
			shell_print(sh, "Set maximum advertising interval to %u ms (%u units)",
				    interval_ms, param.prim_cfg.adv_intv_max);
		} else if (!strcmp(arg, "disable-37")) {
			param.prim_cfg.ch_map &= ~ADV_CHNL_37_EN;
			shell_print(sh, "Disabled advertising on channel 37");
		} else if (!strcmp(arg, "enable-37")) {
			param.prim_cfg.ch_map |= ADV_CHNL_37_EN;
			shell_print(sh, "Enabled advertising on channel 37");
		} else if (!strcmp(arg, "disable-38")) {
			param.prim_cfg.ch_map &= ~ADV_CHNL_38_EN;
			shell_print(sh, "Disabled advertising on channel 38");
		} else if (!strcmp(arg, "enable-38")) {
			param.prim_cfg.ch_map |= ADV_CHNL_38_EN;
			shell_print(sh, "Enabled advertising on channel 38");
		} else if (!strcmp(arg, "disable-39")) {
			param.prim_cfg.ch_map &= ~ADV_CHNL_39_EN;
			shell_print(sh, "Disabled advertising on channel 39");
		} else if (!strcmp(arg, "enable-39")) {
			param.prim_cfg.ch_map |= ADV_CHNL_39_EN;
			shell_print(sh, "Enabled advertising on channel 39");
		} else {
			shell_error(sh, "Unrecognized parameter: %s", arg);
			return -EINVAL;
		}
	}

	k_sem_reset(&bt_process_sem);

	/* Recreate advertising with updated parameters */
	err = gapm_delete_activity(stored_adv.actv_idx);
	if (err) {
		shell_error(sh, "Cannot delete existing advertising set, error code: 0x%02x", err);
		return err;
	}

	err = k_sem_take(&bt_process_sem, K_SECONDS(10));
	if (err < 0) {
		shell_error(sh, "BLE stack not responding within timeout period");
		return err;
	}

	k_sem_reset(&bt_process_sem);

	err = gapm_le_create_adv_legacy(0, GAPM_STATIC_ADDR, &param, &le_adv_cbs);
	if (err) {
		shell_error(sh, "Cannot modify advertising set, error code: 0x%02x", err);
		return err;
	}

	err = k_sem_take(&bt_process_sem, K_SECONDS(10));
	if (err < 0) {
		shell_error(sh, "BLE stack not responding within timeout period");
		return err;
	}

	/* Store updated parameters */
	memcpy(&stored_adv.param, &param, sizeof(gapm_le_adv_create_param_t));

	return 0;
}

static int cmd_adv_start(const struct shell *sh, size_t argc, char *argv[])
{
	if (!is_initialized(sh)) {
		return -ENOEXEC;
	}

	uint8_t num_events = 0;
	int32_t timeout = 0;
	int err;

	if (stored_adv.actv_idx == 0xFF) {
		shell_error(sh, "No advertising set created. Run 'bt adv-create' first.");
		return -EINVAL;
	}

	for (size_t argn = 1; argn < argc; argn++) {
		const char *arg = argv[argn];

		if (!strcmp(arg, "timeout")) {
			if (++argn == argc) {
				goto fail_show_help;
			}

			timeout = strtoul(argv[argn], NULL, 16);
			shell_print(sh, "Set advertising timeout to %d ms", timeout);
		}

		if (!strcmp(arg, "num-events")) {
			if (++argn == argc) {
				goto fail_show_help;
			}

			num_events = strtoul(argv[argn], NULL, 16);
			shell_print(sh, "Set advertising maximum events to %d", num_events);
		}
	}

	gapm_le_adv_param_t adv_params = {
		/* Advertise indefinitely by default */
		.duration = timeout,
		.max_adv_evt = num_events,
	};

	k_sem_reset(&bt_process_sem);

	err = gapm_le_start_adv(stored_adv.actv_idx, &adv_params);
	if (err) {
		shell_error(sh, "Cannot start LE advertising, error code: 0x%02x", err);
	} else {
		shell_print(sh, "Started advertising with activity index %d", stored_adv.actv_idx);
		if (timeout > 0) {
			shell_print(sh, "Advertising will stop after %d ms", timeout);
		}
		if (num_events > 0) {
			shell_print(sh, "Advertising will stop after %d events", num_events);
		}
	}

	err = k_sem_take(&bt_process_sem, K_SECONDS(10));
	if (err < 0) {
		shell_error(sh, "No response from the stack");
		return err;
	}

	return err;

fail_show_help:
	shell_help(sh);
	return -ENOEXEC;
}

static int cmd_adv_stop(const struct shell *sh, size_t argc, char *argv[])
{
	int err;

	if (!is_initialized(sh)) {
		return -ENETDOWN;
	}

	if (stored_adv.actv_idx == 0xFF) {
		shell_error(sh, "No advertising activity to stop");
		return -EINVAL;
	}

	k_sem_reset(&bt_process_sem);

	err = gapm_stop_activity(stored_adv.actv_idx);

	if (err) {
		shell_error(sh, "Cannot stop advertising, error code: 0x%02x", err);
		return err;
	}

	err = k_sem_take(&bt_process_sem, K_SECONDS(10));
	if (err < 0) {
		shell_error(sh, "No response from the stack");
		return err;
	}

	shell_print(sh, "Stopped advertising with activity index %d", stored_adv.actv_idx);
	return 0;
}

static int cmd_adv_delete(const struct shell *sh, size_t argc, char *argv[])
{
	int err;
	uint8_t preserved_actv_idx = stored_adv.actv_idx;

	if (!is_initialized(sh)) {
		return -ENETDOWN;
	}

	if (stored_adv.actv_idx == 0xFF) {
		shell_error(sh, "No advertising activity to delete");
		return -EINVAL;
	}

	k_sem_reset(&bt_process_sem);

	err = gapm_delete_activity(stored_adv.actv_idx);
	if (err) {
		shell_error(sh, "Cannot delete advertising, error code: 0x%02x", err);
		return err;
	}

	err = k_sem_take(&bt_process_sem, K_SECONDS(10));
	if (err < 0) {
		shell_error(sh, "No response from the stack");
		return err;
	}

	shell_print(sh, "Deleted advertising with activity index %d", preserved_actv_idx);
	stored_adv.valid = false;

	return 0;
}

static int cmd_default_handler(const struct shell *sh, size_t argc, char **argv)
{
	if (argc == 1) {
		shell_help(sh);
		return SHELL_CMD_HELP_PRINTED;
	}

	shell_error(sh, "%s unknown parameter: %s", argv[0], argv[1]);

	return -EINVAL;
}

#define HELP_NONE "[none]"
#define HELP_ADV_CREATE                                                                            \
	"<conn-scan | conn-nscan | nconn-scan | nconn-nscan> "                                     \
	"[disable-37] [disable-38] [disable-39]"
#define HELP_ADV_PARAM_OPT                                                                         \
	"[disable-37] [disable-38] [disable-39] "                                                  \
	"[enable-37] [enable-38] [enable-39]"

SHELL_STATIC_SUBCMD_SET_CREATE(
	bt_cmds, SHELL_CMD_ARG(init, NULL, "[no-settings-load] [sync]", cmd_init, 1, 2),

	SHELL_CMD_ARG(adv-create, NULL, HELP_ADV_CREATE, cmd_adv_create, 2, 3),
	SHELL_CMD_ARG(adv-param, NULL, HELP_ADV_PARAM_OPT, cmd_adv_param, 0, 4),
	SHELL_CMD_ARG(adv-start, NULL, "[timeout <timeout>] [num-events <num events>]",
		      cmd_adv_start, 0, 4),
	SHELL_CMD_ARG(adv-stop, NULL, HELP_NONE, cmd_adv_stop, 0, 0),
	SHELL_CMD_ARG(adv-delete, NULL, HELP_NONE, cmd_adv_delete, 0, 0),

	SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(bt, &bt_cmds, "Bluetooth shell commands", cmd_default_handler);

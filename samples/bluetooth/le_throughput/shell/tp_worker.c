/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
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
#include "gapm_le_scan.h"
#include "gapm_le_init.h"
#include "prf.h"
#include "gatt_db.h"
#include "gatt_srv.h"

#include "common.h"
#include "config.h"
#include "central.h"
#include "peripheral.h"
#include "gatt_client.h"

#include <alif/bluetooth/bt_adv_data.h>
#include <alif/bluetooth/bt_scan_rsp.h>
#include "gapm_api.h"

static struct app_con_info_ {
	/* Connection index */
	uint8_t conidx;
} app_con_info = {
	.conidx = GAP_INVALID_CONIDX,
};

/* Macros */
LOG_MODULE_REGISTER(main, LOG_LEVEL_ERR);

static enum app_state app_state;

enum gap_role tp_device_role;

static char *app_state_str[] = {
	"STANDBY",
	"INIT",
	"SCAN_START",
	"SCAN_ONGOING",
	"SCAN_READY",
	"PERIPHERAL_FOUND",
	"CONNECTING",
	"CONNECTED",
	"CONNECTED_PAIRED",
	"GET_FEATURES",
	"DISCOVER_SERVICES",
	"CENTRAL_READY",
	"DATA_TRANSMIT",
	"DATA_READ",
	"DATA_SEND_READY",
	"DATA_RECEIVE_READY",
	"STATS_RESET",
	"PERIPHERAL_START_ADVERTISING",
	"PERIPHERAL_RECEIVING",
	"PERIPHERAL_PREPARE_SENDING",
	"PERIPHERAL_SENDING",
	"PERIPHERAL_SEND_RESULTS",
	"DISCONNECT",
	"DISCONNECTED",
};

void app_transition_to(enum app_state const state)
{
	char const *p_from = app_state == APP_STATE_ERROR ? "ERROR" : app_state_str[app_state];
	char const *p_to = state == APP_STATE_ERROR ? "ERROR" : app_state_str[state];

	LOG_DBG("App transition to %s -> %s", p_from, p_to);
	app_state = state;
}

enum app_state get_app_state(void)
{
	return app_state;
}

uint8_t get_connection_index(void)
{
	return app_con_info.conidx;
}

uint16_t get_mtu_size(void)
{
	uint16_t mtu;

	alif_ble_mutex_lock(K_FOREVER);
	mtu = gatt_bearer_mtu_min_get(app_con_info.conidx);
	alif_ble_mutex_unlock();
	return mtu - GATT_NTF_HEADER_LEN;
}

void app_connection_status_update(enum gapm_connection_event con_event, uint8_t con_idx,
				  uint16_t status)
{
	switch (con_event) {
	case GAPM_API_SEC_CONNECTED_KNOWN_DEVICE:
		app_con_info.conidx = con_idx;
		app_transition_to(APP_STATE_CONNECTED);
		break;
	case GAPM_API_DEV_CONNECTED:
		app_con_info.conidx = con_idx;
		app_transition_to(APP_STATE_CONNECTED);

		break;
	case GAPM_API_DEV_DISCONNECTED:
		LOG_INF("Connection index %u disconnected for reason %u", con_idx, status);
		app_con_info.conidx = GAP_INVALID_CONIDX;
		app_transition_to(APP_STATE_DISCONNECTED);
		break;
	case GAPM_API_PAIRING_FAIL:
		LOG_INF("Connection pairing index %u fail for reason %u", con_idx, status);
		break;
	}
}

static gapm_user_cb_t gapm_user_cb = {
	.connection_status_update = app_connection_status_update,
};

/* ---------------------------------------------------------------------------------------- */
/* BLE config (GAPM) */

static int ble_configure(uint8_t const role)
{
	uint16_t rc;
	const char device_name[] = CONFIG_BLE_TP_DEVICE_NAME;

	gapc_le_con_param_nego_with_ce_len_t app_preferred_connection_param = {
		.ce_len_min = 5,
		.ce_len_max = 10,
		.hdr.interval_min = 6,
		.hdr.interval_max = 200,
		.hdr.latency = 0,
		.hdr.sup_to = 1000,
	};

	LOG_DBG("Configuring BLE to role %s", role == GAP_ROLE_LE_CENTRAL ? "CENTRAL" : "PERIPH");

	gap_addr_t private_address = {
		.addr = {0xCF, 0xFE, 0xFB, 0xDE, 0x11, 0x0},
	};

	/* Set Random Static Address */
	if (role == GAP_ROLE_LE_ALL) {
		private_address.addr[5] = 0x06;
	} else if (role == GAP_ROLE_LE_CENTRAL) {
		private_address.addr[5] = 0x07;
	} else {
		private_address.addr[5] = 0x08;
	}

	/* Bluetooth stack configuration*/
	gapm_config_t gapm_cfg = {
		.role = role,
		/* -------------- Security Config ------------------------------------ */
		.pairing_mode = GAPM_PAIRING_DISABLE,

		/* -------------- Privacy Config ------------------------------------- */
		.privacy_cfg = GAPM_PRIV_CFG_PRIV_ADDR_BIT,
		.renew_dur = 1500,
		.private_identity = private_address,

		.irk.key = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, /* ignored */

		/* -------------- ATT Database Config -------------------------------- */
		.gap_start_hdl = 0,
		.gatt_start_hdl = 0,
		.att_cfg = 0,

		/* -------------- LE Data Length Extension --------------------------- */
		.sugg_max_tx_octets = GAP_LE_MAX_OCTETS,
		.sugg_max_tx_time = GAP_LE_MAX_TIME,

		/* ------------------ LE PHY Management  ----------------------------- */
		.tx_pref_phy = GAP_PHY_LE_2MBPS,
		.rx_pref_phy = GAP_PHY_LE_2MBPS,

		/* ------------------ Radio Configuration ---------------------------- */
		.tx_path_comp = 0,
		.rx_path_comp = 0,

		/* ------------------ BT classic configuration ---------------------- */
		/* Not used */
		.class_of_device = 0,
		.dflt_link_policy = 0,
	};

	/* Set a preferred connections params  */
	bt_gapm_preferred_connection_paras_set(&app_preferred_connection_param);

	rc = bt_gapm_init(&gapm_cfg, &gapm_user_cb, device_name, strlen(device_name));
	if (rc != GAP_ERR_NO_ERROR) {
		LOG_ERR("gapm_configure error %u", rc);
		return -1;
	}

	gap_bdaddr_t identity;

	gapm_get_identity(&identity);

	LOG_DBG("Device address: %02X:%02X:%02X:%02X:%02X:%02X", identity.addr[5], identity.addr[4],
		identity.addr[3], identity.addr[2], identity.addr[1], identity.addr[0]);

	LOG_DBG("GAPM init complete!");
	return 0;
}

int get_private_address(char *p_str, size_t const len)
{
	if (!p_str || len < 18) {
		return -EINVAL;
	}
	gap_bdaddr_t identity;

	gapm_get_identity(&identity);
	snprintk(p_str, len, "%02X:%02X:%02X:%02X:%02X:%02X", identity.addr[5], identity.addr[4],
		 identity.addr[3], identity.addr[2], identity.addr[1], identity.addr[0]);
	return 0;
}

int convert_uuid_to_string(char *const p_str, size_t const max_len, uint8_t const *const p_uuid,
			   uint8_t const uuid_type)
{
	if (!p_uuid || !max_len) {
		return -EINVAL;
	}
	switch (uuid_type) {
	case GATT_UUID_16:
		snprintk(p_str, max_len, "%02x%02x", p_uuid[0], p_uuid[1]);
		break;
	case GATT_UUID_32:
		snprintk(p_str, max_len, "%02x%02x%02x%02x", p_uuid[0], p_uuid[1], p_uuid[2],
			 p_uuid[3]);
		break;
	case GATT_UUID_128:
		snprintk(p_str, max_len,
			 "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x"
			 "%02x%02x%02x",
			 p_uuid[0], p_uuid[1], p_uuid[2], p_uuid[3], p_uuid[4], p_uuid[5],
			 p_uuid[6], p_uuid[7], p_uuid[8], p_uuid[9], p_uuid[10], p_uuid[11],
			 p_uuid[12], p_uuid[13], p_uuid[14], p_uuid[15]);
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

int convert_uuid_with_len_to_string(char *const p_str, size_t const max_len,
				    uint8_t const *const p_uuid, size_t const uuid_len)
{
	uint8_t uuid_type = 0;

	if (GATT_UUID_128_LEN == uuid_len) {
		uuid_type = GATT_UUID_128;
	} else if (GATT_UUID_32_LEN == uuid_len) {
		uuid_type = GATT_UUID_32;
	} else if (GATT_UUID_16_LEN == uuid_len) {
		uuid_type = GATT_UUID_16;
	} else {
		return -EINVAL;
	}
	return convert_uuid_to_string(p_str, max_len, p_uuid, uuid_type);
}

void tp_worker(void *p1, void *p2, void *p3)
{
	LOG_DBG("Starting throughput app...");

	/* Start up bluetooth host stack */
	alif_ble_enable(NULL);

	app_state = APP_STATE_INIT;

	while (1) {
		switch (app_state) {
		case APP_STATE_INIT: {
			if (ble_configure(tp_device_role)) {
				LOG_ERR("ble_configure failed");
				break;
			}

			if (tp_device_role == GAP_ROLE_LE_CENTRAL) {
				central_app_init();
				app_transition_to(APP_STATE_SCAN_START);
			} else if (tp_device_role == GAP_ROLE_LE_PERIPHERAL) {
				peripheral_app_init();
				app_transition_to(APP_STATE_PERIPHERAL_START_ADVERTISING);
			} else {
				LOG_ERR("Unsupported device role");
			}
			break;
		}
		case APP_STATE_ERROR: {
			LOG_ERR("Error, set state to DISCONNECTED");

			if ((tp_device_role == GAP_ROLE_LE_CENTRAL) ||
			    (tp_device_role == GAP_ROLE_LE_PERIPHERAL)) {
				app_transition_to(APP_STATE_DISCONNECTED);
			} else {
				LOG_ERR("Unsupported device role");
			}
		}
		case APP_STATE_STANDBY: {
			k_sleep(K_MSEC(100));
			break;
		}
		default: {
			if (tp_device_role == GAP_ROLE_LE_CENTRAL) {
				if (0 == central_app_exec(app_state)) {
					break;
				}
			}
			if (tp_device_role == GAP_ROLE_LE_PERIPHERAL) {
				if (0 == peripheral_app_exec(app_state)) {
					break;
				}
			}
			LOG_ERR("Invalid state = %d", app_state);
			break;
		}
		};
	}
}

void set_device_role(enum gap_role role)
{
	tp_device_role = role;
}

enum gap_role get_device_role(void)
{
	return tp_device_role;
}

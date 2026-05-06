/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/*
 * BLE peripheral exposing the Device Information Service (DIS).
 * Responds to read requests for Manufacturer Name, Model Number,
 * Firmware Revision and System ID characteristics.
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <string.h>

#include <alif_protocol_const.h>
#include "alif_ble.h"
#include "gapm.h"
#include "gap_le.h"
#include "gapc_le.h"
#include "gapc_sec.h"
#include "gapm_le.h"
#include "gapm_le_adv.h"
#include "co_buf.h"
#include "gatt.h"
#include "address_verification.h"

/* Profile framework */
#include "prf.h"

/* Device Information Service – Server role */
#include "diss.h"

#include "rwip_task.h"

/* Common subsys helpers */
#include <alif/bluetooth/bt_adv_data.h>
#include <alif/bluetooth/bt_scan_rsp.h>
#include "gapm_api.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

/* -------------------------------------------------------------------------
 * Application state
 * -------------------------------------------------------------------------
 */

static bool connected;
static uint8_t adv_type;
static uint8_t adv_actv_idx;

/* -------------------------------------------------------------------------
 * Bluetooth stack configuration
 * -------------------------------------------------------------------------
 */

static gapm_config_t gapm_cfg = {
	.role              = GAP_ROLE_LE_PERIPHERAL,
	.pairing_mode      = GAPM_PAIRING_DISABLE,
	.privacy_cfg       = 0,
	.renew_dur         = 1500,
	.private_identity.addr = {0},
	.irk.key           = {0},
	.gap_start_hdl     = 0,
	.gatt_start_hdl    = 0,
	.att_cfg           = 0,
	.sugg_max_tx_octets = GAP_LE_MIN_OCTETS,
	.sugg_max_tx_time  = GAP_LE_MIN_TIME,
	.tx_pref_phy       = GAP_PHY_ANY,
	.rx_pref_phy       = GAP_PHY_ANY,
	.tx_path_comp      = 0,
	.rx_path_comp      = 0,
	.class_of_device   = 0,
	.dflt_link_policy  = 0,
};

#define DEVICE_NAME      CONFIG_BLE_DEVICE_NAME
#define SAMPLE_ADDR_TYPE ALIF_STATIC_RAND_ADDR

/* -------------------------------------------------------------------------
 * DIS characteristic values
 * -------------------------------------------------------------------------
 */

static const char dis_manufacturer[] = "Alif Semiconductor";
static const char dis_model[]        = "Balletto";
static const char dis_serial_num[]   = "000000000000";
static const char dis_hw_rev[]       = "1.0.0";
static const char dis_fw_rev[]       = "1.0.0";
static const char dis_sw_rev[]       = "1.0.0";

/*
 * System ID – 8 octets: 5-byte Manufacturer Identifier (LSB first)
 *             followed by 3-byte OUI (LSB first).
 * Uses ALIF_IEEE_MA_L_IDENTIFIER (0x785994) for the OUI.
 */
static const uint8_t dis_system_id[8] = {
	0x00, 0x00, 0x00, 0x00, 0x00, /* Manufacturer Identifier */
	(uint8_t)(ALIF_IEEE_MA_L_IDENTIFIER),        /* OUI byte 0 (LSB) */
	(uint8_t)(ALIF_IEEE_MA_L_IDENTIFIER >> 8),   /* OUI byte 1 */
	(uint8_t)(ALIF_IEEE_MA_L_IDENTIFIER >> 16),  /* OUI byte 2 (MSB) */
};

/*
 * PnP ID – 7 octets:
 *   - Vendor ID Source (1 byte): 0x01 = Bluetooth SIG
 *   - Vendor ID (2 bytes, little-endian): 0x0CDD = Alif Semiconductor
 *   - Product ID (2 bytes, little-endian): 0x0001
 *   - Product Version (2 bytes, little-endian): 0x0100 = v1.0
 */
static const uint8_t dis_pnp_id[7] = {
	0x01,       /* Vendor ID Source: Bluetooth SIG */
	0xDD, 0x0C, /* Vendor ID: 0x0CDD (Alif Semiconductor) */
	0x01, 0x00, /* Product ID: 0x0001 */
	0x00, 0x01, /* Product Version: 0x0100 */
};

/* -------------------------------------------------------------------------
 * DIS callback
 * -------------------------------------------------------------------------
 */

static void on_diss_value_req(uint8_t conidx, uint8_t char_type, uint16_t token)
{
	const char *str = NULL;

	switch (char_type) {
	case DISS_CHAR_TYPE_MANUFACTURER_NAME:
		str = dis_manufacturer;
		break;
	case DISS_CHAR_TYPE_MODEL_NUMBER:
		str = dis_model;
		break;
	case DISS_CHAR_TYPE_SERIAL_NUMBER:
		str = dis_serial_num;
		break;
	case DISS_CHAR_TYPE_HW_REVISION:
		str = dis_hw_rev;
		break;
	case DISS_CHAR_TYPE_FW_REVISION:
		str = dis_fw_rev;
		break;
	case DISS_CHAR_TYPE_SW_REVISION:
		str = dis_sw_rev;
		break;
	case DISS_CHAR_TYPE_SYSTEM_ID: {
		co_buf_t *p_buf = NULL;

		if (co_buf_alloc(&p_buf, GATT_BUFFER_HEADER_LEN, sizeof(dis_system_id),
				 GATT_BUFFER_TAIL_LEN) != CO_BUF_ERR_NO_ERROR) {
			LOG_ERR("DIS: co_buf_alloc failed for System ID");
			if (co_buf_alloc(&p_buf, GATT_BUFFER_HEADER_LEN, 0,
					 GATT_BUFFER_TAIL_LEN) == CO_BUF_ERR_NO_ERROR) {
				diss_value_cfm(conidx, token, p_buf);
				co_buf_release(p_buf);
			}
			return;
		}
		memcpy(co_buf_data(p_buf), dis_system_id, sizeof(dis_system_id));
		LOG_DBG("Responding to System ID 0x%02x request with value "
			"0x%02X%02X%02X%02X%02X%02X%02X%02X", char_type,
			dis_system_id[7], dis_system_id[6], dis_system_id[5],
			dis_system_id[4], dis_system_id[3], dis_system_id[2],
			dis_system_id[1], dis_system_id[0]);
		diss_value_cfm(conidx, token, p_buf);
		co_buf_release(p_buf);
		return;
	}
	case DISS_CHAR_TYPE_PNP_ID: {
		co_buf_t *p_buf = NULL;

		if (co_buf_alloc(&p_buf, GATT_BUFFER_HEADER_LEN, sizeof(dis_pnp_id),
				 GATT_BUFFER_TAIL_LEN) != CO_BUF_ERR_NO_ERROR) {
			LOG_ERR("DIS: co_buf_alloc failed for PnP ID");
			if (co_buf_alloc(&p_buf, GATT_BUFFER_HEADER_LEN, 0,
					 GATT_BUFFER_TAIL_LEN) == CO_BUF_ERR_NO_ERROR) {
				LOG_DBG("Responding PnP ID 0x%02x request with empty value "
					"due to allocation failure", char_type);
				diss_value_cfm(conidx, token, p_buf);
				co_buf_release(p_buf);
			}
			return;
		}
		memcpy(co_buf_data(p_buf), dis_pnp_id, sizeof(dis_pnp_id));
		LOG_DBG("Responding to PnP ID 0x%02x request", char_type);
		diss_value_cfm(conidx, token, p_buf);
		co_buf_release(p_buf);
		return;
	}
	default:
		LOG_WRN("DIS: unhandled char_type %u", char_type);
		break;
	}

	if (str == NULL) {
		co_buf_t *p_buf = NULL;

		if (co_buf_alloc(&p_buf, GATT_BUFFER_HEADER_LEN, 0,
				 GATT_BUFFER_TAIL_LEN) != CO_BUF_ERR_NO_ERROR) {
			LOG_ERR("DIS: co_buf_alloc failed for empty response");
			return;
		}
		diss_value_cfm(conidx, token, p_buf);
		co_buf_release(p_buf);
		return;
	}

	uint16_t len = (uint16_t)strlen(str);
	co_buf_t *p_buf = NULL;

	if (co_buf_alloc(&p_buf, GATT_BUFFER_HEADER_LEN, len,
			 GATT_BUFFER_TAIL_LEN) != CO_BUF_ERR_NO_ERROR) {
		LOG_ERR("DIS: co_buf_alloc failed for char_type %u", char_type);
		if (co_buf_alloc(&p_buf, GATT_BUFFER_HEADER_LEN, 0,
				 GATT_BUFFER_TAIL_LEN) == CO_BUF_ERR_NO_ERROR) {
			diss_value_cfm(conidx, token, p_buf);
			co_buf_release(p_buf);
		}
		return;
	}

	memcpy(co_buf_data(p_buf), str, len);
	diss_value_cfm(conidx, token, p_buf);
	co_buf_release(p_buf);
}

static const diss_cbs_t diss_cbs = {
	.cb_value_req = on_diss_value_req,
};

/* -------------------------------------------------------------------------
 * GAPM connection status callback
 * -------------------------------------------------------------------------
 */

static void on_connection_status_update(enum gapm_connection_event con_event,
					uint8_t con_idx,
					uint16_t status)
{
	switch (con_event) {
	case GAPM_API_DEV_CONNECTED:
	case GAPM_API_SEC_CONNECTED_KNOWN_DEVICE:
		connected = true;
		LOG_INF("Connected – index %u", con_idx);
		break;

	case GAPM_API_DEV_DISCONNECTED:
		connected = false;
		LOG_INF("Disconnected – index %u reason 0x%04X", con_idx, status);
		break;

	case GAPM_API_PAIRING_FAIL:
		LOG_WRN("Pairing failed – index %u status 0x%04X", con_idx, status);
		break;
	}
}

static gapm_user_cb_t gapm_user_cb = {
	.connection_status_update = on_connection_status_update,
};

/* -------------------------------------------------------------------------
 * DIS configuration
 * -------------------------------------------------------------------------
 */

static int configure_dis(void)
{
	int err;
	uint8_t sec_lvl = GAP_SEC_NOT_ENC;
	uint8_t user_prio = 0;
	uint16_t start_hdl = 0;

	uint16_t char_type_bf =
				CO_BIT(DISS_CHAR_TYPE_MANUFACTURER_NAME) |
				CO_BIT(DISS_CHAR_TYPE_MODEL_NUMBER) |
				CO_BIT(DISS_CHAR_TYPE_SERIAL_NUMBER) |
				CO_BIT(DISS_CHAR_TYPE_HW_REVISION) |
				CO_BIT(DISS_CHAR_TYPE_FW_REVISION) |
				CO_BIT(DISS_CHAR_TYPE_SW_REVISION) |
				CO_BIT(DISS_CHAR_TYPE_SYSTEM_ID) |
				CO_BIT(DISS_CHAR_TYPE_PNP_ID);

	err = prf_add_profile(TASK_ID_DISS, sec_lvl, user_prio,
							&char_type_bf, &diss_cbs, &start_hdl);

	if (err != GAP_ERR_NO_ERROR) {
		LOG_ERR("prf_add_profile(DISS) error %u", err);
	}

	return err;
}

/* -------------------------------------------------------------------------
 * Advertising helpers
 * -------------------------------------------------------------------------
 */

static int set_advertising_data(uint8_t actv_idx)
{
	int ret;

	ret = bt_adv_data_set_name_auto(DEVICE_NAME, strlen(DEVICE_NAME));
	if (ret) {
		LOG_ERR("AD device name set fail %d", ret);
		return ret;
	}

	return bt_gapm_advertiment_data_set(actv_idx);
}

static int create_advertising(void)
{
	gapm_le_adv_create_param_t adv_create_params = {
		.prop       = GAPM_ADV_PROP_UNDIR_CONN_MASK,
		.disc_mode  = GAPM_ADV_MODE_GEN_DISC,
		.tx_pwr     = 0,
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

/* -------------------------------------------------------------------------
 * main
 * -------------------------------------------------------------------------
 */

int main(void)
{
	int err;

	alif_ble_enable(NULL);

	err = address_verification(SAMPLE_ADDR_TYPE, &adv_type, &gapm_cfg);
	if (err) {
		LOG_ERR("Address verification failed");
		return err;
	}

	err = bt_gapm_init(&gapm_cfg, &gapm_user_cb, DEVICE_NAME, strlen(DEVICE_NAME));
	if (err) {
		LOG_ERR("bt_gapm_init error %u", err);
		return err;
	}

	err = configure_dis();
	if (err) {
		LOG_ERR("DIS configuration failed %u", err);
		return err;
	}

	err = create_advertising();
	if (err) {
		LOG_ERR("Advertisement create fail %u", err);
		return err;
	}

	err = set_advertising_data(adv_actv_idx);
	if (err) {
		LOG_ERR("Advertisement data set fail %u", err);
		return err;
	}

	err = bt_gapm_scan_response_set(adv_actv_idx);
	if (err) {
		LOG_ERR("Scan response set fail %u", err);
		return err;
	}

	err = bt_gapm_advertisement_start(adv_actv_idx);
	if (err) {
		LOG_ERR("Advertisement start fail %u", err);
		return err;
	}

	print_device_identity();

	LOG_INF("DIS peripheral running – waiting for connection...");

	return 0;
}

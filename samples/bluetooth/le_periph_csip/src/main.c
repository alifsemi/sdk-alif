/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <string.h>
#include "alif_ble.h"
#include "gapm.h"
#include "gap_le.h"
#include "gapc_le.h"
#include "gapc_sec.h"
#include "gapm_sec.h"
#include "gapm_le.h"
#include "gapm_le_adv.h"
#include "co_buf.h"
#include "address_verification.h"

#include "prf.h"
#include "atc_csism.h"
#include "atc_csism_msg.h"
#include "csis.h"
#include <alif/bluetooth/bt_adv_data.h>
#include <alif/bluetooth/bt_scan_rsp.h>
#include "gapm_api.h"
#include "rwip_task.h"


/* Define advertising address type */
#define SAMPLE_ADDR_TYPE	ALIF_STATIC_RAND_ADDR


/* Store and share advertising address type */
static uint8_t adv_type;

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

struct app_env {
	bool connected;
	uint8_t cli_cfg_bf;
	uint8_t set_lid;
};

static struct app_env env = {
	.connected = false,
	.cli_cfg_bf = 0,
	.set_lid = 0xFF
};

static struct connection_status app_con_info = {
	.conidx = GAP_INVALID_CONIDX,
	.addr.addr_type = 0xff,
};

/**
 * Bluetooth stack configuration
 */
static gapm_config_t gapm_cfg = {
	.role = GAP_ROLE_LE_PERIPHERAL,
	.pairing_mode = GAPM_PAIRING_SEC_CON,
	.privacy_cfg = 0,
	.renew_dur = 50,
	.private_identity.addr = {0},
	.irk.key = {0xA1, 0xB2, 0xC3, 0xD4, 0xE5, 0xF1, 0x07, 0x08, 0x11, 0x22, 0x33, 0x44, 0x55,
		    0x66, 0x77, 0x89},
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

K_SEM_DEFINE(csis_add_sem, 0, 1);

/* CSIP callbacks */
void app_csism_cb_lock(uint8_t set_lid, uint8_t lock, uint8_t con_lid, uint8_t reason)
{
	LOG_DBG("Lock state updated for set_lid %u, lock %u, con_lid %u, reason %u", set_lid, lock,
		con_lid, reason);
}


void app_csism_cb_bond_data(uint8_t set_lid, uint8_t con_lid, uint8_t cli_cfg_bf)
{
	LOG_DBG("Bond data for set_lid %u, con_lid %u, cli_cfg_bf %u", set_lid, con_lid,
		cli_cfg_bf);
	env.set_lid = set_lid;
	env.cli_cfg_bf = cli_cfg_bf;
}

void app_csism_cb_ltk_req(uint8_t set_lid, uint8_t con_lid)
{
	LOG_DBG("LTK request for set_lid %u, con_lid %u", set_lid, con_lid);
	atc_csism_ltk_cfm(gapm_sec_get_ltk(con_lid));
}

void app_csism_cb_rsi(uint8_t set_lid, const csis_rsi_t *p_rsi)
{
	LOG_DBG("New RSI generated for set_lid %u, rsi %02X%02X%02X%02X%02X%02X",
		set_lid, p_rsi->rsi[0], p_rsi->rsi[1], p_rsi->rsi[2], p_rsi->rsi[3],
		p_rsi->rsi[4], p_rsi->rsi[5]);
}

void app_csism_cb_cmp_evt(uint16_t cmd_code, uint16_t status, uint8_t set_lid)
{
	LOG_DBG("Command completed for cmd_code %u, status %u, set_lid %u", cmd_code, status,
		set_lid);

	if (cmd_code == ATC_CSISM_CMD_TYPE_ADD && status == GAP_ERR_NO_ERROR) {
		k_sem_give(&csis_add_sem);
	}
}

static atc_csism_cb_t csism_cbs = {
	.cb_lock = app_csism_cb_lock,
	.cb_bond_data = app_csism_cb_bond_data,
	.cb_ltk_req = app_csism_cb_ltk_req,
	.cb_rsi = app_csism_cb_rsi,
	.cb_cmp_evt = app_csism_cb_cmp_evt,
};

static uint16_t create_advertising(void)
{
	uint16_t err;

	err = bt_gaf_create_adv(DEVICE_NAME, strlen(DEVICE_NAME), &app_con_info.addr);
	if (err != GAF_ERR_NO_ERROR) {
		LOG_ERR("Unable to configure GAF advertiser! Error %u (0x%02X)", err, err);
		return err;
	}
	LOG_DBG("GAF advertiser is configured");

	return err;
}

/* static const uint8_t SIRK[16] = "16_byte_sirk_key"; */
static const uint8_t SIRK[16] = {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
				 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};

static uint16_t server_configure(void)
{
	uint16_t ret = 0;

	ret = atc_csism_configure(1, &csism_cbs);
	if (ret) {
		LOG_ERR("CSISM configuration failed %d", ret);
		return ret;
	}

	ret = atc_csism_add(CSISM_ADD_CFG_SIZE_BIT | CSISM_ADD_CFG_RANK_BIT
#if CONFIG_BLE_PRIVACY_ENABLED
	| CSISM_ADD_CFG_SIRK_ENCRYPT_BIT
#endif

			    , 2, 1, 30, 0, (csis_sirk_t *)SIRK);

	k_sem_take(&csis_add_sem, K_FOREVER);

	return ret;
}

void app_connection_status_update(enum gapm_connection_event con_event, uint8_t con_idx,
				  uint16_t status)
{
	switch (con_event) {
	case GAPM_API_SEC_CONNECTED_KNOWN_DEVICE:
		env.connected = true;
		LOG_INF("Connection index %u connected to known device", con_idx);
		LOG_DBG("Please enable notifications on peer device..");
		break;
	case GAPM_API_DEV_CONNECTED:
		env.connected = true;
		LOG_INF("Connection index %u connected to new device", con_idx);
		LOG_DBG("Please enable notifications on peer device..");
		break;
	case GAPM_API_DEV_DISCONNECTED:
		LOG_INF("Connection index %u disconnected for reason %u", con_idx, status);
		env.connected = false;
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

	env.connected = false;

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

	err = server_configure();
	if (err) {
		LOG_ERR("Server configuration failed %u", err);
		return -1;
	}

	err = create_advertising();
	if (err) {
		LOG_ERR("Advertisement create fail %u", err);
		return -1;
	}

	/* Start a Generic audio advertisement */
	err = bt_gaf_adv_start(&app_con_info.addr);
	if (err) {
		LOG_ERR("Advertisement start fail %u", err);
		return -1;
	}

	print_device_identity();

	while (1) {
		k_sleep(K_SECONDS(1));
	}
}

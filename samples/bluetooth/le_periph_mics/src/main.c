/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
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
#include <zephyr/drivers/gpio.h>
#include "address_verification.h"
#include "gapm_api.h"
#include "ble_gpio.h"
#include "arc_mics.h"
#include "ble_storage.h"

/* List of Mute value for MCS */
enum app_mic_mute {
	/* Mic not muted */
	MIC_MUTE_NOT_MUTED = 0,
	/* Mic muted */
	MIC_MUTE_MUTED,
	/* Local disabled  */
	MIC_MUTE_DISABLED,
};

/* Bluetooth stack configuration*/
static gapm_config_t gapm_cfg = {
	.role = GAP_ROLE_LE_PERIPHERAL,
	.pairing_mode = GAPM_PAIRING_SEC_CON,
	.privacy_cfg = GAPM_PRIV_CFG_PRIV_ADDR_BIT,
	.renew_dur = 1500,
	.private_identity.addr = {0},
	.irk.key = {0xA1, 0xB2, 0xC3, 0xD4, 0xE5, 0xF6, 0x07, 0x08, 0x11, 0x22, 0x33, 0x44, 0x55,
		    0x66, 0x77, 0x88},
	.gap_start_hdl = 0,
	.gatt_start_hdl = 0,
	.att_cfg = 0,
	.sugg_max_tx_octets = GAP_LE_MIN_OCTETS,
	.sugg_max_tx_time = GAP_LE_MIN_TIME,
	.tx_pref_phy = GAP_PHY_LE_2MBPS,
	.rx_pref_phy = GAP_PHY_LE_2MBPS,
	.tx_path_comp = 0,
	.rx_path_comp = 0,
	.class_of_device = 0,  /* BT Classic only */
	.dflt_link_policy = 0, /* BT Classic only */
};

struct service_env {
	uint8_t mute;
	uint16_t ntf_cfg;
};

static uint8_t conn_status = BT_CONN_STATE_DISCONNECTED;

/* Load name from configuration file */
#define DEVICE_NAME      CONFIG_BLE_DEVICE_NAME

static struct service_env env;

void LedWorkerHandler(struct k_work *work);

static K_WORK_DELAYABLE_DEFINE(ledWork, LedWorkerHandler);

static const struct gpio_dt_spec activeLed = GPIO_DT_SPEC_GET(DT_ALIAS(ledgreen), gpios);
static const struct gpio_dt_spec muteLed = GPIO_DT_SPEC_GET(DT_ALIAS(ledred), gpios);
static const struct gpio_dt_spec bleLed = GPIO_DT_SPEC_GET(DT_ALIAS(ledblue), gpios);

static struct connection_status app_con_info = {
	.conidx = GAP_INVALID_CONIDX,
	.addr.addr_type = 0xff,
};

/* Macros */
LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

/* function headers */
static uint16_t service_init(void);

/* Functions */

static void UpdateMuteLedstate(void)
{
	k_work_reschedule(&ledWork, K_MSEC(1));
}

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

static void server_configure(void)
{
	uint16_t err;

	err = service_init();

	if (err) {
		LOG_ERR("Error %u adding profile", err);
	}
}

void mics_cb_bond_data(uint8_t con_lid, uint8_t cli_cfg_bf)
{
	LOG_DBG("MCS Bond data %u con_lid %u cnf", con_lid, cli_cfg_bf);
	env.ntf_cfg = cli_cfg_bf;
	if (cli_cfg_bf) {
		arc_mics_set_mute(env.mute);
	}
}

static void mics_cb_mute(uint8_t mute)
{
	if (env.mute != mute) {
		env.mute = mute;
		UpdateMuteLedstate();
	}
}

static arc_mics_cb_t mics_cb;

/*
 * Service functions
 */
static uint16_t service_init(void)
{
	uint16_t status;

	mics_cb.cb_mute = mics_cb_mute;
	mics_cb.cb_bond_data = mics_cb_bond_data;

	env.mute = MIC_MUTE_NOT_MUTED;
	env.ntf_cfg = 0;

	status = arc_mics_configure(&mics_cb, 0, env.mute, 0, 0, NULL);

	if (status != GAP_ERR_NO_ERROR) {
		LOG_ERR("MCS configure problem %u", status);
	}

	return status;
}

void ButtonUpdateHandler(uint32_t button_state, uint32_t has_changed)
{
	if (has_changed & 1) {
		/* Press Button Update toggle led when state goes to 0 */
		if (!(button_state & 1)) {
			/* Toggle Light onOff server state */
			uint8_t new_state;

			switch (env.mute) {
			case MIC_MUTE_DISABLED:
				new_state = MIC_MUTE_NOT_MUTED;
				break;
			case MIC_MUTE_NOT_MUTED:
				new_state = MIC_MUTE_MUTED;
				break;
			default:
				new_state = MIC_MUTE_DISABLED;
				break;
			}

			LOG_DBG("Set MCS state %u", new_state);
			arc_mics_set_mute(new_state);

			env.mute = new_state;
			UpdateMuteLedstate();
		}
	}
}

void LedWorkerHandler(struct k_work *work)
{
	int res_schedule_time = 0;

	if (conn_status == BT_CONN_STATE_CONNECTED) {
		ble_gpio_led_set(&bleLed, false);
	} else {
		ble_gpio_led_toggle(&bleLed);
		res_schedule_time = 500;
	}

	switch (env.mute) {
	case MIC_MUTE_MUTED:
		res_schedule_time = 500;
		ble_gpio_led_set(&activeLed, false);
		ble_gpio_led_toggle(&muteLed);
		break;
	case MIC_MUTE_DISABLED:
		ble_gpio_led_set(&activeLed, false);
		ble_gpio_led_set(&muteLed, true);
		break;
	default:
		ble_gpio_led_set(&activeLed, true);
		ble_gpio_led_set(&muteLed, false);
		break;
	}

	if (res_schedule_time) {
		k_work_reschedule(&ledWork, K_MSEC(res_schedule_time));
	}
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

	UpdateMuteLedstate();
}

static gapm_user_cb_t gapm_user_cb = {
	.connection_status_update = app_connection_status_update,
};

int main(void)
{
	uint16_t err;

	ble_storage_init();

	err = ble_gpio_buttons_init(ButtonUpdateHandler);
	if (err) {
		LOG_ERR("Button Init fail %u", err);
		return -1;
	}

	err = ble_gpio_led_init();

	if (err) {
		LOG_ERR("Led Init fail %u", err);
		return -1;
	}

	/* Start up bluetooth host stack */
	alif_ble_enable(NULL);

	/* Define Private identity */
	bt_generate_private_identity(&gapm_cfg);

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

	/* Start a Generic audio advertisement */
	err = bt_gaf_adv_start(&app_con_info.addr);

	if (err) {
		LOG_ERR("Advertisement start fail %u", err);
		return -1;
	}

	print_device_identity();
	/* Set a Led init state */
	k_work_reschedule(&ledWork, K_MSEC(1));
	return 0;
}

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
#include "address_verification.h"
#include <zephyr/drivers/gpio.h>
#include <alif/bluetooth/bt_adv_data.h>
#include <alif/bluetooth/bt_scan_rsp.h>
#include "gapm_api.h"
#include "ble_gpio.h"
#include "arc_mics.h"


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
	.private_identity.addr = {0x78, 0x59, 0x94, 0xDE, 0x11, 0xFF},
	.irk.key = {0xA1, 0xB2, 0xC3, 0xD4, 0xE5, 0xF6, 0x07, 0x08, 0x11, 0x22, 0x33, 0x44, 0x55,
		    0x66, 0x77, 0x88},
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

struct service_env {
	uint8_t mute;
	uint16_t ntf_cfg;
};

static uint8_t conn_status = BT_CONN_STATE_DISCONNECTED;
static uint8_t adv_actv_idx;
static uint8_t adv_type; /* Advertising type, set by address_verification() */

/* Load name from configuration file */
#define DEVICE_NAME      CONFIG_BLE_DEVICE_NAME
#define SAMPLE_ADDR_TYPE ALIF_GEN_RSLV_RAND_ADDR

static struct service_env env;

void LedWorkerHandler(struct k_work *work);

static K_WORK_DELAYABLE_DEFINE(ledWork, LedWorkerHandler);

static const struct gpio_dt_spec activeLed = GPIO_DT_SPEC_GET(DT_ALIAS(ledgreen), gpios);
static const struct gpio_dt_spec muteLed = GPIO_DT_SPEC_GET(DT_ALIAS(ledred), gpios);
static const struct gpio_dt_spec bleLed = GPIO_DT_SPEC_GET(DT_ALIAS(ledblue), gpios);

/* Macros */
LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

/* function headers */
static uint16_t service_init(void);

/* Functions */

static void UpdateMuteLedstate(void)
{
	k_work_reschedule(&ledWork, K_MSEC(1));
}

/**
 * Bluetooth GAPM callbacks
 */

static int set_advertising_data(uint8_t actv_idx)
{
	int ret;
	/* gatt service identifier */
	uint16_t svc = GATT_SVC_MICROPHONE_CONTROL;

	ret = bt_adv_data_set_tlv(GAP_AD_TYPE_COMPLETE_LIST_16_BIT_UUID, &svc, sizeof(svc));
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
	/* Set a Led init state */
	k_work_reschedule(&ledWork, K_MSEC(1));
	return 0;
}

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
#include "gapm_le.h"
#include "gapm_le_adv.h"
#include "co_buf.h"
#include "prf.h"
#include "gatt_db.h"
#include "gatt_srv.h"
#include "ke_mem.h"
#include <zephyr/drivers/gpio.h>
#include "gapm_api.h"
#include "ble_gpio.h"
#include "address_verification.h"
#include "arc_vcs.h"
#include "arc_vocs.h"
#include "arc_aics.h"
#include "l2cap_coc.h"
#include "ble_storage.h"

#define BUTTON_PRESSED 1  /* SW5 */
#define BUTTON_UP      16 /* SW1 */
#define BUTTON_RIGHT   8  /* SW2 */
#define BUTTON_LEFT    4  /* SW3 */
#define BUTTON_DOWN    2  /* SW4 */

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
	.tx_pref_phy = GAP_PHY_ANY,
	.rx_pref_phy = GAP_PHY_ANY,
	.tx_path_comp = 0,
	.rx_path_comp = 0,
	.class_of_device = 0,  /* BT Classic only */
	.dflt_link_policy = 0, /* BT Classic only */
};

struct service_env {
	uint8_t mute;
	uint8_t volume;
	uint16_t ntf_cfg;
	uint8_t vocs_lid;
};

#define VOCS_MAX_DESC_LEN 20
#define VOCS_CFG_BF 0
#define DEFAULT_VCS_FLAGS 0

static uint8_t conn_status = BT_CONN_STATE_DISCONNECTED;

/* Load name from configuration file */
#define DEVICE_NAME      CONFIG_BLE_DEVICE_NAME

static struct connection_status app_con_info = {
	.conidx = GAP_INVALID_CONIDX,
	.addr.addr_type = 0xff,
};

static struct service_env env = {
	.volume = 10,
	.mute = 0,
	.ntf_cfg = 0,
	.vocs_lid = 0,
};
static arc_vcs_cb_t vcs_cb;
static arc_vocs_cb_t vocs_cb;
static arc_aics_cb_t aics_cb;

#define AICS_CFG_BF 0
#define AICS_DESC_MAX_LEN 20

static void led_worker_handler(struct k_work *work);

static K_WORK_DELAYABLE_DEFINE(ledWork, led_worker_handler);

static const struct gpio_dt_spec activeLed = GPIO_DT_SPEC_GET(DT_ALIAS(ledgreen), gpios);
static const struct gpio_dt_spec muteLed = GPIO_DT_SPEC_GET(DT_ALIAS(ledred), gpios);
static const struct gpio_dt_spec bleLed = GPIO_DT_SPEC_GET(DT_ALIAS(ledblue), gpios);

/* Macros */
LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

/* function headers */
static uint16_t service_init(void);

/* Functions */
#if defined(CONFIG_IUT_TESTER_ENABLED)
K_SEM_DEFINE(test_sem, 0, 1);
static int TEST_case_num = 1;

static void test_case_configure_set(void)
{
	switch (TEST_case_num) {
	case 1:
		env.volume = (CONFIG_VOLUME_STEP_SIZE * 2);
		env.mute = 0;
		break;
	case 2:
		env.volume = 255 - (CONFIG_VOLUME_STEP_SIZE * 2);
		env.mute = 0;
		break;
	case 3:
		env.volume = (CONFIG_VOLUME_STEP_SIZE * 2);
		env.mute = 1;
		break;
	case 4:
		env.volume = 255 - (CONFIG_VOLUME_STEP_SIZE * 2);
		env.mute = 1;
		break;
	case 5:
		env.volume = 255 - (CONFIG_VOLUME_STEP_SIZE * 2);
		env.mute = 1;
		break;
	case 6:
		env.volume = 255 - (CONFIG_VOLUME_STEP_SIZE * 2);
		env.mute = 1;
		break;
	case 7:
		env.volume = 253;
		env.mute = 1;
		break;
	default:
		env.volume = 0;
		env.mute = 0;
		break;
	}
}

static void test_case_print(void)
{
	LOG_INF("Test case VCS/SR/CP/BV-%d: Step size %u, Volume %u, Mute %u", TEST_case_num,
		CONFIG_VOLUME_STEP_SIZE, env.volume, env.mute);
}

static void test_case_default_set(void)
{
	TEST_case_num = 1;
	test_case_configure_set();
	test_case_print();
	LOG_INF("Update IUT test values by BUTTON_UP  and select by BUTTON_PRESSED");
	k_sem_take(&test_sem, K_FOREVER);
}

static void test_case_toggle(void)
{
	if (TEST_case_num < 7) {
		TEST_case_num++;
	} else {
		TEST_case_num = 1;
	}
	test_case_configure_set();
	test_case_print();
	LOG_INF("Update IUT test values by BUTTON_UP  and select by BUTTON_PRESSED");
}

static void test_setup_activate(void)
{
	LOG_INF("Take a current setup");
	k_sem_give(&test_sem);
}
#endif

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

static void vcs_cb_bond_data(uint8_t con_lid, uint8_t cli_cfg_bf)
{
	LOG_DBG("VCS Bond data updated, con_lid: %u, cfg: %u", con_lid, cli_cfg_bf);
	env.ntf_cfg = cli_cfg_bf;
}

static void vcs_cb_volume(uint8_t volume, uint8_t mute, bool local)
{
	(void)local;
	env.volume = volume;
	LOG_INF("Updated volume Level %u", env.volume);
	if (env.mute != mute) {
		env.mute = mute;
		LOG_INF("Updated Mute state: %u", mute);
		UpdateMuteLedstate();
	}

}

static void vcs_cb_flags(uint8_t flags)
{
	LOG_INF("Volume flags %u", flags);
}

static void vocs_cb_offset(uint8_t output_lid, int16_t offset)
{
	LOG_DBG("VOCS offset updated, output_lid: %u, offset: %d", output_lid, offset);
}

static void vocs_cb_bond_data(uint8_t output_lid, uint8_t con_lid, uint8_t cli_cfg_bf)
{
	LOG_DBG("VOCS Bond data updated, output_lid: %u, con_lid: %u, cfg: %u", output_lid, con_lid,
		cli_cfg_bf);
}

static void vocs_cb_description_req(uint8_t output_lid, uint8_t con_lid, uint8_t desc_len,
			     const char *p_desc)
{
	LOG_DBG("VOCS description req, output_lid: %u, con_lid: %u, desc_len: %u, desc: %s",
		output_lid, con_lid, desc_len, p_desc);
}

static void vocs_cb_location_req(uint8_t output_lid, uint8_t con_lid, uint32_t location_bf)
{
	LOG_DBG("VOCS location req, output_lid: %u, con_lid: %u, location_bf: %" PRIu32, output_lid,
		con_lid, location_bf);
}

static void aics_cb_bond_data(uint8_t input_lid, uint8_t con_lid, uint8_t cli_cfg_bf)
{
	LOG_DBG("AICS Bond data updated, input_lid: %u, con_lid: %u, cfg: %u", input_lid, con_lid,
		cli_cfg_bf);
}

static void aics_cb_state(uint8_t input_lid, arc_aic_state_t *p_state)
{
	LOG_DBG("AICS state updated, input_lid: %u, gain: %u, mute: %u, gain_mode: %u", input_lid,
		p_state->gain, p_state->mute, p_state->gain_mode);
}

static void aics_cb_description_req(uint8_t input_lid, uint8_t con_lid, uint8_t desc_len,
			     const char *p_desc)
{
	LOG_DBG("AICS description req, input_lid: %u, con_lid: %u, desc_len: %u, desc: %s",
		input_lid, con_lid, desc_len, p_desc);
}

/*
 * Service functions
 */
static uint16_t service_init(void)
{
	uint16_t status;

	/* First configure VOCS */
	vocs_cb.cb_bond_data = vocs_cb_bond_data;
	vocs_cb.cb_offset = vocs_cb_offset;
	vocs_cb.cb_description_req = vocs_cb_description_req;
	vocs_cb.cb_location_req = vocs_cb_location_req;

	status = arc_vocs_configure(&vocs_cb, 1, L2CAP_COC_MTU_MIN);
	if (status != GAP_ERR_NO_ERROR) {
		LOG_ERR("VOCS configure problem %u", status);
	}

	status = arc_vocs_add(VOCS_MAX_DESC_LEN, VOCS_CFG_BF, GATT_INVALID_HDL, &env.vocs_lid);
	if (status != GAP_ERR_NO_ERROR) {
		LOG_ERR("VOCS add problem %u", status);
	}

	/* then Configure AICS */
	aics_cb.cb_bond_data = aics_cb_bond_data;
	aics_cb.cb_state = aics_cb_state;
	aics_cb.cb_description_req = aics_cb_description_req;

	status = arc_aics_configure(&aics_cb, 1, L2CAP_COC_MTU_MIN);
	if (status != GAP_ERR_NO_ERROR) {
		LOG_ERR("AICS configure problem %u", status);
	}

	arc_aic_gain_prop_t props = {
		.gain_units = 5, /* 1/10 dB */
		.gain_min = -30,
		.gain_max = 10
	};

	uint8_t input_lid;

	status = arc_aics_add(&props, ARC_AIC_INPUT_TYPE_MICROPHONE, AICS_DESC_MAX_LEN, AICS_CFG_BF,
			      GATT_INVALID_HDL, &input_lid);
	if (status != GAP_ERR_NO_ERROR) {
		LOG_ERR("AICS add problem %u", status);
		return status;
	}

	/* Lastly Configure VCS */
	vcs_cb.cb_bond_data = vcs_cb_bond_data;
	vcs_cb.cb_volume = vcs_cb_volume;
	vcs_cb.cb_flags = vcs_cb_flags;

	uint8_t input_lids[1];

	input_lids[0] = input_lid;

	status = arc_vcs_configure(&vcs_cb, CONFIG_VOLUME_STEP_SIZE, DEFAULT_VCS_FLAGS, env.volume,
				   env.mute, GATT_INVALID_HDL, ARC_VCS_CFG_FLAGS_NTF_BIT, 1,
				   input_lids);

	if (status != GAP_ERR_NO_ERROR) {
		LOG_ERR("VCS configure problem %u", status);
	}

	return status;
}

static void button_update_handler(uint32_t button_state, uint32_t has_changed)
{
#if defined(CONFIG_IUT_TESTER_ENABLED)
	if (has_changed & BUTTON_PRESSED) {
		/* Press Button Update toggle led when state goes to 0 */
		if (!(button_state & BUTTON_PRESSED)) {
			test_setup_activate();
		}
	}

	if (has_changed & BUTTON_UP) {
		if (!(button_state & BUTTON_UP)) {
			test_case_toggle();
		}
	}
#else
	if (has_changed & BUTTON_PRESSED) {
		/* Press Button Update toggle led when state goes to 0 */
		if (!(button_state & BUTTON_PRESSED)) {
			if (env.mute == 0) {
				LOG_INF("Mute vcs");
				arc_vcs_mute();
			} else {
				LOG_INF("UnMute vcs");
				arc_vcs_unmute();
			}

			UpdateMuteLedstate();
		}
	}

	if (has_changed & BUTTON_UP) {
		if (!(button_state & BUTTON_UP)) {
			LOG_INF("Volume UP");
			arc_vcs_volume_increase();
		}
	}

	if (has_changed & BUTTON_DOWN) {
		if (!(button_state & BUTTON_DOWN)) {
			LOG_INF("Volume Down");
			arc_vcs_volume_decrease();
		}
	}
#endif
}

static void led_worker_handler(struct k_work *work)
{
	(void)work;
	int res_schedule_time = 0;

	if (conn_status == BT_CONN_STATE_CONNECTED) {
		ble_gpio_led_set(&bleLed, false);
	} else {
		ble_gpio_led_toggle(&bleLed);
		res_schedule_time = 500;
	}

	if (env.mute) {
		res_schedule_time = 500;
		ble_gpio_led_set(&activeLed, false);
		ble_gpio_led_toggle(&muteLed);
	} else {
		ble_gpio_led_set(&activeLed, true);
		ble_gpio_led_set(&muteLed, false);
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

	err = ble_gpio_buttons_init(button_update_handler);
	if (err) {
		LOG_ERR("Button Init fail %u", err);
		return -1;
	}

	err = ble_gpio_led_init();

	if (err) {
		LOG_ERR("Led Init fail %u", err);
		return -1;
	}

	ble_storage_init();

#if defined(CONFIG_IUT_TESTER_ENABLED)
	test_case_default_set();
#endif

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

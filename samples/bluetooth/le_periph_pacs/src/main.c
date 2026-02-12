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
#include "address_verification.h"

#include "prf.h"
#include "bap_capa_srv.h"
#include "bap.h"
#include "ke_mem.h"
#include <alif/bluetooth/bt_adv_data.h>
#include <alif/bluetooth/bt_scan_rsp.h>
#include "gapm_api.h"
#include "rwip_task.h"
#include "ble_storage.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

struct app_env {
	bool connected;
	uint8_t cli_cfg_bf;
	uint16_t pac_cli_cfg_bf;
	uint8_t con_lid;
};

static struct app_env env = {
	.connected = false,
	.cli_cfg_bf = 0xFF,
	.pac_cli_cfg_bf = 0xFFFF,
	.con_lid = 0xFF
};

/**
 * Bluetooth stack configuration
 */
static gapm_config_t gapm_cfg = {
	.role = GAP_ROLE_LE_PERIPHERAL,
	.pairing_mode = GAPM_PAIRING_SEC_CON,
	.privacy_cfg = GAPM_PRIV_CFG_PRIV_ADDR_BIT,
	.renew_dur = 1500,
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

static struct connection_status app_con_info = {
	.conidx = GAP_INVALID_CONIDX,
	.addr.addr_type = 0xff,
};

/* Load name from configuration file */
#define DEVICE_NAME CONFIG_BLE_DEVICE_NAME

/* PACS callbacks */
void app_bap_capa_srv_cb_bond_data(uint8_t con_lid, uint8_t cli_cfg_bf, uint16_t pac_cli_cfg_bf)
{
	LOG_DBG("Bond data requested for con_lid %u, cli_cfg_bf %u, pac_cli_cfg_bf %u", con_lid,
		cli_cfg_bf, pac_cli_cfg_bf);
	env.cli_cfg_bf = cli_cfg_bf;
	env.pac_cli_cfg_bf = pac_cli_cfg_bf;
	env.con_lid = con_lid;
}

void app_bap_capa_srv_cb_location_req(uint8_t con_lid, uint16_t token, uint8_t direction,
				      uint32_t location_bf)
{
	LOG_DBG("Location req for con_lid %u, token %u, direction %u, location_bf 0x%08x", con_lid,
		token, direction, location_bf);
}

static bap_capa_srv_cb_t pacs_cbs = {
	.cb_bond_data = app_bap_capa_srv_cb_bond_data,
	.cb_location_req = app_bap_capa_srv_cb_location_req,
};

static uint16_t create_advertising(void)
{
	uint16_t err = bt_gaf_create_adv(DEVICE_NAME, strlen(DEVICE_NAME), &app_con_info.addr);

	if (err != GAF_ERR_NO_ERROR) {
		LOG_ERR("Unable to configure GAF advertiser! Error %u (0x%02X)", err, err);
		return err;
	}
	LOG_DBG("GAF advertiser is configured");
	return err;
}

#define BAP_CAPA_SIZE          CO_ALIGN4_HI(sizeof(bap_capa_t))

static uint16_t server_configure(void)
{
	uint16_t ret = 0;
	bap_capa_srv_cfg_t srv_cfg = {
		.cfg_bf = 0,
		.nb_pacs_sink = 0,
		.nb_pacs_src = 1,
		.pref_mtu = 0,
		.shdl = GATT_INVALID_HDL,
		.location_bf_sink = 0,
		.location_bf_src = 0, /* Mono */
		.supp_context_bf_sink = 0,
		.supp_context_bf_src = BAP_CONTEXT_TYPE_ALL,
	};

	ret = bap_capa_srv_configure(&pacs_cbs, &srv_cfg);
	if (ret) {
		LOG_ERR("PACS server configuration failed %u", ret);
		return ret;
	}

	/* We must use HS malloc as we pass the ownership */
	bap_capa_t *p_capa = (bap_capa_t *)ke_malloc_user(BAP_CAPA_SIZE, KE_MEM_PROFILE);
	gaf_codec_id_t const codec_id = GAF_CODEC_ID_LC3;
	size_t pac_lid = 0;
	size_t record_id = 1; /* Records start from ID 1 */

	if (!p_capa) {
		LOG_ERR("PACS capability record malloc failed");
		return -ENOMEM;
	}

	p_capa->param.sampling_freq_bf = BAP_SAMPLING_FREQ_32000HZ_BIT;
	p_capa->param.frame_dur_bf = BAP_FRAME_DUR_10MS_BIT;
	p_capa->param.chan_cnt_bf = 1;
	p_capa->param.frame_octet_min = 60;
	p_capa->param.frame_octet_max = 80;
	p_capa->param.max_frames_sdu = 1;
	p_capa->add_capa.len = 0;

	ret = bap_capa_srv_set_record(pac_lid, record_id, &codec_id, p_capa, NULL);
	if (ret) {
		LOG_ERR("PACS capability record set failed %u", ret);
		ke_free(p_capa);
	}
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

	ble_storage_init();

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

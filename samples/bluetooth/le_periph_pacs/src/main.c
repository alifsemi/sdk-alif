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


/* Define advertising address type */
#define SAMPLE_ADDR_TYPE	ALIF_STATIC_RAND_ADDR

/* Store and share advertising address type */
static uint8_t adv_type;

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
	.private_identity.addr = {0xCA, 0xFE, 0xFA, 0xDE, 0x15, 0x08},
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

/* Store advertising activity index for re-starting after disconnection */
static uint8_t adv_actv_idx;

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

/* Advertisement functions */
static uint16_t set_advertising_data(uint8_t actv_idx)
{
	uint16_t ret;
	uint16_t comp_id = CONFIG_BLE_COMPANY_ID;

	ret = bt_adv_data_set_manufacturer(comp_id, NULL, 0);

	if (ret) {
		LOG_ERR("AD manufacturer data fail %d", ret);
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
				.adv_intv_min = 160, /* 100 ms */
				.adv_intv_max = 800, /* 500 ms */
				.ch_map = ADV_ALL_CHNLS_EN,
				.phy = GAPM_PHY_TYPE_LE_1M,
			},
	};

	return bt_gapm_le_create_advertisement_service(adv_type, &adv_create_params, NULL,
						      &adv_actv_idx);
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

	while (1) {
		k_sleep(K_SECONDS(1));
	}
}

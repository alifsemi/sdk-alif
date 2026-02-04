/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "alif_ble.h"
#include "gap_le.h"
#include "gapc_le.h"
#include "gapc_sec.h"
#include "gapm.h"
#include "gapm_le.h"
#include "gapm_le_adv.h"
#include "gapm_le_per_sync.h"
#include "address_verification.h"
#include <alif/bluetooth/bt_adv_data.h>
#include <alif/bluetooth/bt_scan_rsp.h>
#include "gapm_api.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

#define DEVICE_NAME CONFIG_BLE_DEVICE_NAME

/* Define advertising address type */
#define SAMPLE_ADDR_TYPE	ALIF_STATIC_RAND_ADDR

/* Store and share advertising address type */
static uint8_t adv_type;

static uint8_t adv_actv_idx;
static uint8_t sync_actv_idx;

static uint16_t start_per_adv_sync(uint8_t conidx)
{
	const gapm_le_per_sync_param_t sync_params = {
		.skip = 0,
		.sync_to = 1000,
		.type = GAPM_PER_SYNC_TYPE_PAST,
		.conidx = conidx,
		.adv_addr = {},
		.report_en_bf = GAPM_REPORT_ADV_EN_BIT,
		.cte_type = 0,
	};

	return gapm_le_start_per_sync(sync_actv_idx, &sync_params);
}

static uint16_t create_adv_data(uint8_t conidx)
{
	int ret;

	ret = bt_adv_data_set_name_auto(DEVICE_NAME, strlen(DEVICE_NAME));

	if (ret) {
		LOG_ERR("AD device name data fail %d", ret);
		return ATT_ERR_INSUFF_RESOURCE;
	}

	return bt_gapm_advertiment_data_set(conidx);
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

static void on_per_adv_proc_cmp(uint32_t metainfo, uint8_t proc_id, uint8_t actv_idx,
				uint16_t status)
{
	switch (proc_id) {
	case GAPM_ACTV_START:
		LOG_INF("Periodic advertising sync activity has been started");
		break;

	case GAPM_ACTV_STOP:
		LOG_INF("Periodic advertising sync activity has been stopped");
		break;
	}
}

static void on_per_adv_stopped(uint32_t metainfo, uint8_t actv_idx, uint16_t reason)
{
	if (reason == GAP_ERR_DISCONNECTED) {
		LOG_ERR("Periodic advertising sync lost");
		return;
	}

	LOG_INF("Periodic advertising sync stopped");
}

static void on_report_received(uint32_t metainfo, uint8_t actv_idx,
			       const gapm_le_adv_report_info_t *p_info, co_buf_t *p_report)
{
	LOG_INF("Periodic advertising report received");

	LOG_INF("trans_addr: %02X:%02X:%02X:%02X:%02X:%02X addr_type: %u",
		p_info->trans_addr.addr[5], p_info->trans_addr.addr[4], p_info->trans_addr.addr[3],
		p_info->trans_addr.addr[2], p_info->trans_addr.addr[1], p_info->trans_addr.addr[0],
		p_info->trans_addr.addr_type);

	LOG_INF("target_addr: %02X:%02X:%02X:%02X:%02X:%02X addr_type: %u",
		p_info->target_addr.addr[5], p_info->target_addr.addr[4],
		p_info->target_addr.addr[3], p_info->target_addr.addr[2],
		p_info->target_addr.addr[1], p_info->target_addr.addr[0],
		p_info->target_addr.addr_type);

	LOG_INF("info: %u, tx_pwr: %i rssi: %i, phy_prim: %u, phy_second: %u adv_sid: %u, "
		"period_adv_intv: %u",
		p_info->info, p_info->tx_pwr, p_info->rssi, p_info->phy_prim, p_info->phy_second,
		p_info->adv_sid, p_info->period_adv_intv);

	LOG_HEXDUMP_INF(co_buf_data(p_report), co_buf_data_len(p_report), "p_report:");
}

static void on_established(uint32_t metainfo, uint8_t actv_idx,
			   const gapm_le_per_sync_info_t *p_info)
{
	LOG_INF("Periodic advertising sync established");

	LOG_INF("addr: %02X:%02X:%02X:%02X:%02X:%02X addr_type: %u", p_info->addr.addr[5],
		p_info->addr.addr[4], p_info->addr.addr[3], p_info->addr.addr[2],
		p_info->addr.addr[1], p_info->addr.addr[0], p_info->addr.addr_type);

	LOG_INF("phy: %u, interval: %u, adv_sid: %u, clk_acc: %u, serv_data: %u", p_info->phy,
		p_info->interval, p_info->adv_sid, p_info->clk_acc, p_info->serv_data);
}

static uint16_t create_per_sync(void)
{
	static const gapm_le_per_sync_cb_actv_t sync_cbs = {
		.actv.proc_cmp = on_per_adv_proc_cmp,
		.actv.stopped = on_per_adv_stopped,
		.report_received = on_report_received,
		.established = on_established,
	};

	return gapm_le_create_per_sync(0, &sync_cbs, &sync_actv_idx);
}

void app_connection_status_update(enum gapm_connection_event con_event, uint8_t con_idx,
				  uint16_t status)
{
	uint16_t rc;

	switch (con_event) {
	case GAPM_API_SEC_CONNECTED_KNOWN_DEVICE:
		rc = start_per_adv_sync(con_idx);
		if (rc != GAP_ERR_NO_ERROR) {
			LOG_ERR("Failed to start periodic advertising sync (conidx: %u), error: %u",
				con_idx, rc);
			return;
		}

		LOG_INF("Started periodic advertising sync (conidx: %u)", con_idx);
		LOG_INF("Connection index %u connected to known device", con_idx);
		break;
	case GAPM_API_DEV_CONNECTED:
		rc = start_per_adv_sync(con_idx);
		if (rc != GAP_ERR_NO_ERROR) {
			LOG_ERR("Failed to start periodic advertising sync (conidx: %u), error: %u",
				con_idx, rc);
			return;
		}

		LOG_INF("Started periodic advertising sync (conidx: %u)", con_idx);
		LOG_INF("Connection index %u connected to new device", con_idx);
		break;
	case GAPM_API_DEV_DISCONNECTED:
		LOG_INF("Connection index %u disconnected for reason %u", con_idx, status);
		break;
	case GAPM_API_PAIRING_FAIL:
		LOG_INF("Connection pairing index %u fail for reason %u", con_idx, status);
		break;
	}
}

static gapm_user_cb_t gapm_user_cb = {
	.connection_status_update = app_connection_status_update,
};

static uint16_t config_gapm(void)
{
	static gapm_config_t gapm_cfg = {
		/* Observer role is needed for periodic sync */
		.role = GAP_ROLE_LE_PERIPHERAL | GAP_ROLE_LE_OBSERVER,
		.pairing_mode = GAPM_PAIRING_DISABLE,
		.pairing_min_req_key_size = 0,
		.privacy_cfg = 0,
		.renew_dur = 1500,
		.private_identity.addr = {0, 0, 0, 0, 0, 0},
		.irk.key = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		.gap_start_hdl = 0,
		.gatt_start_hdl = 0,
		.att_cfg = 0,
		.sugg_max_tx_octets = GAP_LE_MIN_OCTETS,
		.sugg_max_tx_time = GAP_LE_MIN_TIME,
		.tx_pref_phy = GAP_PHY_ANY,
		.rx_pref_phy = GAP_PHY_ANY,
		.tx_path_comp = 0,
		.rx_path_comp = 0,
		.class_of_device = 0,
		.dflt_link_policy = 0,
	};

	if (address_verification(SAMPLE_ADDR_TYPE, &adv_type, &gapm_cfg)) {
		LOG_ERR("Address verification failed");
		return -EADV;
	}

	return bt_gapm_init(&gapm_cfg, &gapm_user_cb, DEVICE_NAME, strlen(DEVICE_NAME));
}

int main(void)
{
	int ret;
	uint16_t rc;

	LOG_INF("Enabling Alif BLE stack");
	ret = alif_ble_enable(NULL);
	if (ret) {
		LOG_ERR("Failed to enable Alif BLE stack, error: %i", ret);
		return -1;
	}

	rc = config_gapm();
	if (rc != GAP_ERR_NO_ERROR) {
		LOG_ERR("Failed to configure GAP, error: %u", rc);
		return -1;
	}

	rc = create_per_sync();
	if (rc != GAP_ERR_NO_ERROR) {
		LOG_ERR("Failed to create periodic sync, error: %u", rc);
		return -1;
	}

	LOG_INF("Creating advertisement");
	rc = create_advertising();
	if (rc != GAP_ERR_NO_ERROR) {
		LOG_ERR("Failed to create advertising activity, error: %u", rc);
		return -1;
	}

	rc = create_adv_data(adv_actv_idx);
	if (rc) {
		LOG_ERR("Advertisement data set fail %u", rc);
		return -1;
	}

	rc = bt_gapm_scan_response_set(adv_actv_idx);
	if (rc) {
		LOG_ERR("Scan response set fail %u", rc);
		return -1;
	}

	rc = bt_gapm_advertisement_start(adv_actv_idx);
	if (rc) {
		LOG_ERR("Advertisement start fail %u", rc);
		return -1;
	}

	print_device_identity();

	while (1) {
		k_sleep(K_SECONDS(1));
	}

	return 0;
}

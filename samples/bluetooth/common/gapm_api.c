/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "gaf_adv.h"
#include "gapm.h"
#include "gap_le.h"
#include "gapc_le.h"
#include "gapm_le.h"
#include "gapm_le_adv.h"
#include <alif/bluetooth/bt_adv_data.h>
#include <alif/bluetooth/bt_scan_rsp.h>
#include "address_verification.h"
#include "gapm_api.h"
#include "power_mgr.h"
#include "gapm_sec.h"
#include "ble_storage.h"

K_SEM_DEFINE(gapm_sem, 0, 1);

LOG_MODULE_REGISTER(gapm, LOG_LEVEL_DBG);

static uint8_t adv_actv_idx = GAP_INVALID_CONIDX;
static uint16_t gapm_status;

/* GAF advertising */
#define ADV_SET_LOCAL_IDX     0
#define ADV_TIMEOUT           0 /* Infinite (until explicitly stopped) */
#define ADV_TIMEOUT_DIRECT    5
#define ADV_SID               0
#define ADV_INTERVAL_QUICK_MS 45
#define ADV_INTERVAL_MS       150
#define ADV_PHY               GAP_PHY_1MBPS
#define ADV_PHY_2nd           GAP_PHY_2MBPS
#define ADV_MAX_TX_PWR        -20
#define ADV_MAX_SKIP          1

static char gaf_adv_data[27];
static size_t gaf_adv_data_len;

static gapm_user_cb_t *user_gapm_cb;

static gap_bdaddr_t *gaf_gap_adr;

static gapc_le_con_param_nego_with_ce_len_t preferred_connection_param = {
	.ce_len_min = 5,
	.ce_len_max = 10,
	.hdr.interval_min = 100,
	.hdr.interval_max = 300,
	.hdr.latency = 0,
	.hdr.sup_to = 1000,
};

/**
 * Advertising callbacks
 */
static void on_adv_actv_stopped(uint32_t metainfo, uint8_t actv_idx, uint16_t reason)
{
	LOG_DBG("Advertising activity index %u stopped for reason %u", actv_idx, reason);
}

static void on_adv_actv_proc_cmp(uint32_t metainfo, uint8_t proc_id, uint8_t actv_idx,
				 uint16_t status)
{
	gapm_status = status;

	if (status) {
		LOG_ERR("Advertising activity process completed with error %u", status);
	}

	switch (proc_id) {
	case GAPM_ACTV_CREATE_LE_ADV:
		LOG_DBG("Advertising activity is created");
		adv_actv_idx = actv_idx;
		break;

	case GAPM_ACTV_SET_ADV_DATA:
		LOG_DBG("Advertising data is set");
		break;

	case GAPM_ACTV_SET_SCAN_RSP_DATA:
		LOG_DBG("Scan data is set");
		break;

	case GAPM_ACTV_START:
		address_verification_log_advertising_address(actv_idx);

		break;

	default:
		LOG_WRN("Unexpected GAPM activity complete, proc_id %u", proc_id);
		break;
	}
	k_sem_give(&gapm_sem);
}

static void on_adv_created(uint32_t metainfo, uint8_t actv_idx, int8_t tx_pwr)
{
	LOG_DBG("Advertising activity created, index %u, selected tx power %d", actv_idx, tx_pwr);
}

static void on_ext_adv_stopped(uint32_t const metainfo, uint8_t const actv_idx,
			       uint16_t const reason)
{
	LOG_DBG("Extended advertising activity stopped, index %u, reason=%d", actv_idx, reason);
}

static void on_gapm_process_complete(uint32_t metainfo, uint16_t status)
{
	if (status) {
		LOG_ERR("gapm process completed with error %u", status);
	}

	gapm_status = status;

	k_sem_give(&gapm_sem);
}

static uint16_t bt_gapm_device_name_set(const char *name, size_t name_len)
{
	uint16_t rc;

	k_sem_reset(&gapm_sem);

	rc = gapm_set_name(0, name_len, name, on_gapm_process_complete);
	if (rc != GAP_ERR_NO_ERROR) {
		LOG_ERR("Failed to set device name, error: %u", rc);
		return rc;
	}
	k_sem_take(&gapm_sem, K_FOREVER);

	return gapm_status;
}

static void on_le_connection_req(uint8_t conidx, uint32_t metainfo, uint8_t actv_idx, uint8_t role,
				 const gap_bdaddr_t *p_peer_addr,
				 const gapc_le_con_param_t *p_con_params, uint8_t clk_accuracy)
{
	LOG_INF("Connection request on index %u", conidx);

	LOG_DBG("Connection parameters: interval %u, latency %u, supervision timeout %u",
		p_con_params->interval, p_con_params->latency, p_con_params->sup_to);

	LOG_INF("Peer BD address %02X:%02X:%02X:%02X:%02X:%02X (conidx: %u)", p_peer_addr->addr[5],
		p_peer_addr->addr[4], p_peer_addr->addr[3], p_peer_addr->addr[2],
		p_peer_addr->addr[1], p_peer_addr->addr[0], conidx);

	gapm_connection_confirm(conidx, metainfo, p_peer_addr);
}

static void on_disconnection(uint8_t conidx, uint32_t metainfo, uint16_t reason)
{
	uint16_t err;

	LOG_INF("Connection index %u disconnected for reason %u", conidx, reason);
	if (adv_actv_idx != GAP_INVALID_CONIDX) {
		err = bt_gapm_advertisement_continue(adv_actv_idx);
	} else {
		/* Start a Generic audio advertisement */
		err = bt_gaf_adv_start(gaf_gap_adr);
	}

	if (err) {
		LOG_ERR("Error restarting advertising: %u", err);
	} else {
		LOG_DBG("Restarting advertising");
	}

	if (user_gapm_cb) {
		user_gapm_cb->connection_status_update(GAPM_API_DEV_DISCONNECTED, conidx, reason);
	}
}

static void on_name_get(uint8_t conidx, uint32_t metainfo, uint16_t token, uint16_t offset,
			uint16_t max_len)
{
	LOG_WRN("Received unexpected name get from conidx: %u", conidx);
}

static void on_appearance_get(uint8_t conidx, uint32_t metainfo, uint16_t token)
{
	/* Send 'unknown' appearance */
	gapc_le_get_appearance_cfm(conidx, token, GAP_ERR_NO_ERROR, 0);
}

static void on_bond_data_updated(uint8_t conidx, uint32_t metainfo,
				 const gapc_bond_data_updated_t *p_data)
{
	LOG_INF("Client %u bond data updated: gatt_start_hdl:%u, gatt_end_hdl:%u, "
		"svc_chg_hdl:%u, cli_info:%u, cli_feat:%u, srv_feat:%u",
		conidx, p_data->gatt_start_hdl, p_data->gatt_end_hdl, p_data->svc_chg_hdl,
		p_data->cli_info, p_data->cli_feat, p_data->srv_feat);

	gapm_sec_bondata_update(conidx, metainfo, p_data);
}

static void on_auth_payload_timeout(uint8_t conidx, uint32_t metainfo)
{
	LOG_DBG("%s", __func__);
}

static void on_no_more_att_bearer(uint8_t conidx, uint32_t metainfo)
{
	LOG_DBG("%s", __func__);
}

static void on_cli_hash_info(uint8_t conidx, uint32_t metainfo, uint16_t handle,
			     const uint8_t *p_hash)
{
	LOG_DBG("%s", __func__);
}

static void on_name_set(uint8_t conidx, uint32_t metainfo, uint16_t token, co_buf_t *p_buf)
{
	LOG_DBG("%s", __func__);
	gapc_le_set_name_cfm(conidx, token, GAP_ERR_NO_ERROR);
}

static void on_appearance_set(uint8_t conidx, uint32_t metainfo, uint16_t token,
			      uint16_t appearance)
{
	LOG_DBG("%s", __func__);
	gapc_le_set_appearance_cfm(conidx, token, GAP_ERR_NO_ERROR);
}

static void on_pref_param_get(uint8_t conidx, uint32_t metainfo, uint16_t token)
{

	gapc_le_preferred_periph_param_t prefs = {
		.con_intv_min = preferred_connection_param.hdr.interval_min,
		.con_intv_max = preferred_connection_param.hdr.interval_max,
		.latency = preferred_connection_param.hdr.latency,
		.conn_timeout = 3200*2,
	};
	LOG_DBG("%s", __func__);

	gapc_le_get_preferred_periph_params_cfm(conidx, token, GAP_ERR_NO_ERROR, prefs);
}

static const gapc_connection_req_cb_t gapc_con_cbs = {
	.le_connection_req = on_le_connection_req,
};

static const gapc_connection_info_cb_t gapc_con_inf_cbs = {
	.disconnected = on_disconnection,
	.name_get = on_name_get,
	.appearance_get = on_appearance_get,
	.slave_pref_param_get = on_pref_param_get,
	/* Other callbacks in this struct are optional */
	.bond_data_updated = on_bond_data_updated,
	.auth_payload_timeout = on_auth_payload_timeout,
	.no_more_att_bearer = on_no_more_att_bearer,
	.cli_hash_info = on_cli_hash_info,
	.name_set = on_name_set,
	.appearance_set = on_appearance_set,
};

static void on_param_update_req(uint8_t conidx, uint32_t metainfo,
				const gapc_le_con_param_nego_t *p_param)
{
	LOG_DBG("%s:%d", __func__, conidx);
	gapc_le_update_params_cfm(conidx, true, preferred_connection_param.ce_len_min,
				  preferred_connection_param.ce_len_max);
}
static void on_param_updated(uint8_t conidx, uint32_t metainfo, const gapc_le_con_param_t *p_param)
{
	LOG_DBG("%s conn:%d", __func__, conidx);
}

static void on_packet_size_updated(uint8_t conidx, uint32_t metainfo, uint16_t max_tx_octets,
				   uint16_t max_tx_time, uint16_t max_rx_octets,
				   uint16_t max_rx_time)
{
	LOG_DBG("%s conn:%d max_tx_octets:%d max_tx_time:%d  max_rx_octets:%d "
		"max_rx_time:%d",
		__func__, conidx, max_tx_octets, max_tx_time, max_rx_octets, max_rx_time);
}

static void on_phy_updated(uint8_t conidx, uint32_t metainfo, uint8_t tx_phy, uint8_t rx_phy)
{
	LOG_DBG("%s conn:%d tx_phy:%d rx_phy:%d", __func__, conidx, tx_phy, rx_phy);
}

static void on_subrate_updated(uint8_t conidx, uint32_t metainfo,
			       const gapc_le_subrate_t *p_subrate_params)
{
	LOG_DBG("%s conn:%d", __func__, conidx);
}

/* All callbacks in this struct are optional */
static const gapc_le_config_cb_t gapc_le_cfg_cbs = {
	.param_update_req = on_param_update_req,
	.param_updated = on_param_updated,
	.packet_size_updated = on_packet_size_updated,
	.phy_updated = on_phy_updated,
	.subrate_updated = on_subrate_updated,
};

static void on_gapm_err(uint32_t metainfo, uint8_t code)
{
	LOG_ERR("gapm error %d", code);
}
static const gapm_cb_t gapm_err_cbs = {
	.cb_hw_error = on_gapm_err,
};

static gapm_callbacks_t gapm_cbs = {
	.p_con_req_cbs = &gapc_con_cbs,
	.p_info_cbs = &gapc_con_inf_cbs,
	.p_le_config_cbs = &gapc_le_cfg_cbs,
	.p_bt_config_cbs = NULL, /* BT classic so not required */
	.p_gapm_cbs = &gapm_err_cbs,
};

static void app_pairing_status_cb(uint16_t status, uint8_t con_idx, bool known_peer)
{
	if (!user_gapm_cb) {
		return;
	}

	if (status == 0) {
		if (known_peer) {
			user_gapm_cb->connection_status_update(GAPM_API_SEC_CONNECTED_KNOWN_DEVICE,
							       con_idx, status);
		} else {
			user_gapm_cb->connection_status_update(GAPM_API_DEV_CONNECTED, con_idx,
							       status);
		}

	} else {
		LOG_INF("Connection confirm fail %u , %u id", status, con_idx);
		user_gapm_cb->connection_status_update(GAPM_API_PAIRING_FAIL, con_idx, status);
	}
}

void bt_gapm_preferred_connection_paras_set(
	const gapc_le_con_param_nego_with_ce_len_t *preferred_params)
{
	preferred_connection_param = *preferred_params;
}

uint16_t bt_gapm_init(const gapm_config_t *p_cfg, gapm_user_cb_t *p_cbs, const char *name,
		      size_t name_len)
{
	uint16_t rc;
	int ret;
	bool sec_pairing;

	if (!p_cbs || !p_cfg || !name) {
		return GAP_ERR_INVALID_PARAM;
	}

	if (p_cfg->pairing_mode & 0x0f) {
		sec_pairing = true;
	} else {
		sec_pairing = false;
	}
	user_gapm_cb = p_cbs;
	/* Define Security callbacks */
	gapm_cbs.p_sec_cbs = gapm_sec_init(sec_pairing, app_pairing_status_cb, &(p_cfg->irk));

	ret = bt_adv_data_init();
	if (ret) {
		LOG_ERR("AD data init fail %d", ret);
		return ATT_ERR_INSUFF_RESOURCE;
	}

	ret = bt_scan_rsp_init();
	if (ret) {
		LOG_ERR("Scan response init fail %d", ret);
		return ATT_ERR_INSUFF_RESOURCE;
	}

	k_sem_reset(&gapm_sem);

	LOG_INF("Init gapm service and set device name %s", name);

	rc = gapm_configure(0, p_cfg, &gapm_cbs, on_gapm_process_complete);
	if (rc) {
		LOG_ERR("gapm_configure error %u", rc);
		return rc;
	}
	/* Wait process complete */
	k_sem_take(&gapm_sem, K_FOREVER);
	if (gapm_status) {
		return gapm_status;
	}

	rc = bt_gapm_device_name_set(name, name_len);

	if (sec_pairing) {
		/* Enable security level */
		gapm_le_configure_security_level(GAP_SEC1_NOAUTH_PAIR_ENC);
	}

#if CONFIG_PM && SNIPPET_PM_BLE_USED
#if PREKERNEL_DISABLE_SLEEP
	/* Update PM policy to allow sleeps */
	power_mgr_allow_sleep();
#endif
	/* Give some time for the system to log before entering sleep */
	k_sleep(K_MSEC(50));
#endif

	return rc;
}

uint16_t bt_gapm_le_create_advertisement_service(enum gapm_le_own_addr addrstype,
						 gapm_le_adv_create_param_t *adv_create_params,
						 gapm_le_adv_user_cb_t *user_cb, uint8_t *adv_index)
{
	uint16_t err;
	static gapm_le_adv_cb_actv_t le_adv_cbs;

	memset(&le_adv_cbs, 0, sizeof(gapm_le_adv_cb_actv_t));

	k_sem_reset(&gapm_sem);
	/* This will be always local one */
	le_adv_cbs.hdr.actv.proc_cmp = on_adv_actv_proc_cmp;
	if (user_cb && user_cb->stopped) {
		le_adv_cbs.hdr.actv.stopped = user_cb->stopped;
	} else {
		le_adv_cbs.hdr.actv.stopped = on_adv_actv_stopped;
	}

	if (user_cb && user_cb->created) {
		le_adv_cbs.created = user_cb->created;
	} else {
		le_adv_cbs.created = on_adv_created;
	}

	if (user_cb && user_cb->ext_adv_stopped) {
		le_adv_cbs.ext_adv_stopped = user_cb->ext_adv_stopped;
	} else {
		le_adv_cbs.ext_adv_stopped = on_ext_adv_stopped;
	}

	LOG_INF("Allocate LE Advertisement service");

	err = gapm_le_create_adv_legacy(0, addrstype, adv_create_params, &le_adv_cbs);
	if (err) {
		LOG_ERR("Error %u creating advertising activity", err);
	}
	k_sem_take(&gapm_sem, K_FOREVER);

	*adv_index = adv_actv_idx;

	return gapm_status;
}

uint16_t bt_gapm_advertiment_data_set(uint8_t adv_index)
{
	uint16_t err;

	k_sem_reset(&gapm_sem);

	LOG_INF("Set Advertisement data to service %u", adv_index);

	err = bt_adv_data_set_update(adv_index);
	if (err) {
		LOG_ERR("Error %u creating advertising activity", err);
	}

	k_sem_take(&gapm_sem, K_FOREVER);
	return gapm_status;
}

uint16_t bt_gapm_scan_response_set(uint8_t adv_index)
{
	uint16_t rc;

	k_sem_reset(&gapm_sem);

	LOG_INF("Set Scan response buffer to service %u", adv_index);

	rc = bt_scan_rsp_set(adv_index);
	if (rc) {
		LOG_ERR("Failed to set scan data, error: %d", rc);
		return rc;
	}

	k_sem_take(&gapm_sem, K_FOREVER);

	return gapm_status;
}

uint16_t bt_gapm_advertisement_start(uint8_t adv_index)
{
	uint16_t rc;

	k_sem_reset(&gapm_sem);

	LOG_INF("Start LE Advertisement to service %u", adv_index);

	rc = bt_adv_start_le_adv(adv_index, 0, 0, 0);
	if (rc) {
		LOG_ERR("Failed to start advertising, error: %d", rc);
		return rc;
	}
	k_sem_take(&gapm_sem, K_FOREVER);

	return gapm_status;
}

uint16_t bt_gapm_advertisement_continue(uint8_t adv_index)
{
	uint16_t rc;

	rc = bt_adv_start_le_adv(adv_index, 0, 0, 0);
	if (rc) {
		LOG_ERR("Failed to start advertising, error: %d", rc);
	}

	return rc;
}

static void on_gaf_advertising_cmp_evt(uint8_t const cmd_type, uint16_t const status,
				       uint8_t const set_lid)
{
	uint8_t adv_index;

	__ASSERT(status == GAF_ERR_NO_ERROR, "GAF advertising error:%u, cmd:%u", status, cmd_type);
	adv_index = gaf_adv_get_actv_idx(set_lid);

	switch (cmd_type) {
	case GAF_ADV_CMD_TYPE_START: {
		LOG_DBG("GAF advertising started");
		address_verification_log_advertising_address(adv_index);
		break;
	}
	case GAF_ADV_CMD_TYPE_STOP: {
		LOG_INF("GAF advertising stopped");
		break;
	}
	case GAF_ADV_CMD_TYPE_START_DIRECTED: {
		LOG_DBG("GAF directed advertising started");
		address_verification_log_advertising_address(adv_index);
		break;
	}
	case GAF_ADV_CMD_TYPE_START_DIRECTED_FAST: {
		LOG_DBG("GAF high-duty cycle directed advertising started");
		address_verification_log_advertising_address(adv_index);
		break;
	}
	default:
		break;
	}
}

static void on_gaf_advertising_stopped(uint8_t const set_lid, uint8_t const reason)
{
	(void)set_lid;

	static const char *const reason_str[] = {"Requested by Upper Layer", "Internal error",
						 "Timeout", "Connection established"};

	LOG_DBG("GAF advertising stopped. Reason: %s",
		reason < ARRAY_SIZE(reason_str) ? reason_str[reason] : "Unknown");

	if (reason != GAF_ADV_STOP_REASON_CON_ESTABLISHED) {
		/* Restart normal advertising */
		bt_gaf_adv_start(NULL);
	}
}

static const struct gaf_adv_cb gaf_adv_cbs = {
	.cb_cmp_evt = on_gaf_advertising_cmp_evt,
	.cb_stopped = on_gaf_advertising_stopped,
};

uint16_t bt_gaf_create_adv(const char *name, size_t name_len, gap_bdaddr_t *p_gap_adr)
{

	uint16_t err;
	size_t adv_name_len = name_len;
	uint8_t ad_type;
	struct gaf_adv_cfg config = {
		.nb_sets = 1,
	};

	err = gaf_adv_configure(&config, &gaf_adv_cbs);
	if (err != GAF_ERR_NO_ERROR) {
		LOG_ERR("Unable to configure GAF advertiser! Error %u (0x%02X)", err, err);
		return err;
	}

	/* Pre config Gaf Adv data */
	if (adv_name_len > (sizeof(gaf_adv_data) - 2)) {
		adv_name_len = sizeof(gaf_adv_data) - 2;
		ad_type = GAP_AD_TYPE_SHORTENED_NAME;
	} else {
		ad_type = GAP_AD_TYPE_COMPLETE_NAME;
	}

	gaf_adv_data[0] = adv_name_len + 1;
	gaf_adv_data[1] = GAP_AD_TYPE_COMPLETE_NAME;
	strncpy(&gaf_adv_data[2], name, adv_name_len);
	gaf_adv_data_len = adv_name_len + 2;
	LOG_DBG("GAF advertiser is configured");
	gaf_gap_adr = p_gap_adr;

	gapm_sec_load_peer_address(p_gap_adr);

	return err;
}

uint16_t bt_gaf_adv_start(gap_bdaddr_t *p_client_addr)
{

	uint16_t err;

	err = gaf_adv_set_params(ADV_SET_LOCAL_IDX, ADV_INTERVAL_QUICK_MS, ADV_INTERVAL_MS, ADV_PHY,
				 ADV_PHY_2nd, ADV_ALL_CHNLS_EN, ADV_MAX_TX_PWR, ADV_MAX_SKIP);
	if (err != GAF_ERR_NO_ERROR) {
		LOG_ERR("Failed to set advertising params, err %u (0x%02X)", err, err);
		return err;
	}

	uint32_t adv_config = GAPM_ADV_MODE_GEN_DISC;

	if (p_client_addr && p_client_addr->addr_type != 0xff) {
		LOG_INF("Starting directed advertising with address %02X:%02X:%02X:%02X:%02X:%02X",
			p_client_addr->addr[5], p_client_addr->addr[4], p_client_addr->addr[3],
			p_client_addr->addr[2], p_client_addr->addr[1], p_client_addr->addr[0]);
		err = gaf_adv_start_directed(ADV_SET_LOCAL_IDX, adv_config, ADV_TIMEOUT_DIRECT,
					     ADV_SID, gaf_adv_data_len, (uint8_t *)gaf_adv_data,
					     NULL, p_client_addr);

	} else {

		LOG_INF("Starting general advertising");
		err = gaf_adv_start(
			ADV_SET_LOCAL_IDX, (adv_config | GAF_ADV_CFG_GENERAL_ANNOUNCEMENT_BIT),
			ADV_TIMEOUT, ADV_SID, gaf_adv_data_len, (uint8_t *)gaf_adv_data, NULL);
	}

	if (err != GAF_ERR_NO_ERROR) {
		LOG_ERR("Failed to start advertising, err %u (0x%02X)", err, err);
	}

	return err;
}
/* Temp buffer pointer */
static gap_addr_t *private_address;

static void on_gapm_le_random_addr_cb(uint16_t const status, const gap_addr_t *const p_addr)
{
	gapm_status = status;
	if (status != GAP_ERR_NO_ERROR) {
		LOG_ERR("GAPM address generation error %u", status);
	} else {

		LOG_DBG("Generated address: %02X:%02X:%02X:%02X:%02X:%02X", p_addr->addr[5],
			p_addr->addr[4], p_addr->addr[3], p_addr->addr[2], p_addr->addr[1],
			p_addr->addr[0]);

		*private_address = *p_addr;
	}

	k_sem_give(&gapm_sem);
}

static uint16_t bt_gapm_generate_random_addr(gapm_config_t *p_cfg,
					     enum gap_le_random_addr_type addr_type)
{
	uint16_t err;

	/* Temporary usage only */
	gapm_cbs.p_sec_cbs = gapm_sec_cb_got();

	private_address = &p_cfg->private_identity;

	err = gapm_configure(1, p_cfg, &gapm_cbs, on_gapm_process_complete);
	if (err != GAP_ERR_NO_ERROR) {
		LOG_ERR("gapm_configure error %u", err);
		return err;
	}
	if (k_sem_take(&gapm_sem, K_MSEC(1000)) != 0) {
		LOG_ERR("  FAIL! GAPM config timeout!");
		return GAP_ERR_TIMEOUT;
	}

	/* Generate random static address */
	err = gapm_le_generate_random_addr(addr_type, on_gapm_le_random_addr_cb);
	if (err != GAP_ERR_NO_ERROR) {
		LOG_ERR("gapm_le_generate_random_addr error %u", err);
		return err;
	}
	if (k_sem_take(&gapm_sem, K_MSEC(1000)) != 0) {
		LOG_ERR("  FAIL! GAPM random address timeout!");
		return GAP_ERR_TIMEOUT;
	}

	if (gapm_status != GAP_ERR_NO_ERROR) {
		LOG_ERR("gapm_le_generate_random_addr fail by status %u", gapm_status);
		return gapm_status;
	}

	/* Reset GAPM to set address */
	err = gapm_reset(3, on_gapm_process_complete);
	if (err != GAP_ERR_NO_ERROR) {
		LOG_ERR("gapm_reset error %u", err);
		return err;
	}
	if (k_sem_take(&gapm_sem, K_MSEC(1000)) != 0) {
		LOG_ERR("  FAIL! GAPM reset timeout!");
		return GAP_ERR_TIMEOUT;
	}
	return 0;
}

void bt_generate_private_identity(gapm_config_t *p_cfg)
{
	if (IS_ENABLED(CONFIG_SETTINGS)) {
		ble_storage_load(BLE_PRIV_ID_NAME, &p_cfg->private_identity, sizeof(gap_addr_t));
	}

	if (p_cfg->private_identity.addr[5] == 0) {
		/* Define Random static address */
		bt_gapm_generate_random_addr(p_cfg, GAP_BD_ADDR_STATIC);
		if (IS_ENABLED(CONFIG_SETTINGS)) {
			/* Save generated address */
			ble_storage_save(BLE_PRIV_ID_NAME, &p_cfg->private_identity,
					 sizeof(gap_addr_t));
		}
	}
	p_cfg->privacy_cfg = GAPM_PRIV_CFG_PRIV_ADDR_BIT; /*Privacy address bit*/
}

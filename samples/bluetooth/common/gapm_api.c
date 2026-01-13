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
#include "gapm.h"
#include "gap_le.h"
#include "gapc_le.h"
#include "gapm_le.h"
#include "gapm_le_adv.h"
#include <alif/bluetooth/bt_adv_data.h>
#include <alif/bluetooth/bt_scan_rsp.h>
#include "address_verification.h"
#include "gapm_api.h"

K_SEM_DEFINE(gapm_sem, 0, 1);

LOG_MODULE_REGISTER(gapm, LOG_LEVEL_DBG);

static uint8_t adv_actv_idx;
static uint16_t gapm_status;

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

uint16_t bt_gapm_init(const gapm_config_t *p_cfg, const gapm_callbacks_t *p_cbs, const char *name,
		      size_t name_len)
{
	uint16_t rc;
	int ret;

	ret = bt_adv_data_init();
	if (ret) {
		LOG_ERR("AD data init fail %d", ret);
		return ATT_ERR_INSUFF_RESOURCE;
	}

	ret = bt_scan_rsp_init();
	if (ret) {
		LOG_ERR("Sacan response init fail %d", ret);
		return ATT_ERR_INSUFF_RESOURCE;
	}

	k_sem_reset(&gapm_sem);

	LOG_INF("Init gapm servicer and set device name %s", name);

	rc = gapm_configure(0, p_cfg, p_cbs, on_gapm_process_complete);
	if (rc) {
		LOG_ERR("gapm_configure error %u", rc);
		return rc;
	}
	/* Wait process complete */
	k_sem_take(&gapm_sem, K_FOREVER);
	if (gapm_status) {
		return gapm_status;
	}

	return bt_gapm_device_name_set(name, name_len);
}

uint16_t bt_gapm_le_create_advertisement_service(enum gapm_le_own_addr addrstype,
						gapm_le_adv_create_param_t *adv_create_params,
						gapm_le_adv_user_cb_t *user_cb, uint8_t *adv_index)
{
	uint16_t err;
	static gapm_le_adv_cb_actv_t le_adv_cbs;

	memset(&le_adv_cbs, 0, sizeof(gapm_le_adv_cb_actv_t));

	k_sem_reset(&gapm_sem);
	/* This will be allways local one */
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

	LOG_INF("Allocate LE Advertiment service");

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

	LOG_INF("Set Advertiment data to service %u", adv_index);

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

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
#include "gaf_scan.h"
#include "gap_le.h"
#include "gapm.h"
#include "app_gaf_scan.h"
#include "alif_ble.h"

#define SCAN_DURATION_SEC 5
/** See enum gaf_scan_cfg_bf for more information */
#define SCAN_CONFIG_BITS  GAF_SCAN_CFG_ASCS_REQ_BIT

LOG_MODULE_REGISTER(le_gaf_scan, CONFIG_LE_GAF_SCAN_LOG_LEVEL);

static bool scan_ongoing;
static scanning_ready_callback_t scanning_ready_cb;
static peer_found_cb_t peer_match_cb;
static uint8_t scan_req_bits;

const char peripheral_name[] = CONFIG_PERIPHERAL_NAME;

const char *bdaddr_str(const gap_bdaddr_t *const p_addr)
{
	static char addr_str[32];

	snprintk(addr_str, sizeof(addr_str), "%02X:%02X:%02X:%02X:%02X:%02X (%u)", p_addr->addr[5],
		 p_addr->addr[4], p_addr->addr[3], p_addr->addr[2], p_addr->addr[1],
		 p_addr->addr[0], p_addr->addr_type);
	return addr_str;
}

/* ---------------------------------------------------------------------------------------- */
/** GAF Client callbacks */

static void on_gaf_scanning_cb_cmp_evt(uint8_t const cmd_type, uint16_t const status)
{
	if (cmd_type == GAF_SCAN_CMD_TYPE_STOP) {
		LOG_INF("Scan stopped");
		scan_ongoing = false;

		if (scanning_ready_cb) {
			scanning_ready_cb();
			scanning_ready_cb = NULL;
		}
	} else if (cmd_type == GAF_SCAN_CMD_TYPE_START) {
		/* LOG_INF("Scan started"); */
	} else {
		LOG_ERR("Unexpected GAF scanning command complete event: %u", cmd_type);
	}
}

static void on_gaf_scanning_cb_stopped(uint8_t const reason)
{
	static const char *const reason_str[] = {
		"Requested by Upper Layer",
		"Internal error",
		"Timeout",
	};

	LOG_DBG("GAF scanning stopped. Reason: %s",
		reason < ARRAY_SIZE(reason_str) ? reason_str[reason] : "??");

	if (reason == GAF_SCAN_STOP_REASON_UL) {
		LOG_INF("Scan stopped by upper layer request");
		return;
	}

	scan_ongoing = false;
	if (scanning_ready_cb) {
		scanning_ready_cb();
		scanning_ready_cb = NULL;
	}
}

static void on_gaf_scanning_cb_report(const gap_bdaddr_t *p_addr, uint8_t const info_bf,
				      const gaf_adv_report_air_info_t *p_air_info,
				      uint8_t const flags, uint16_t const appearance,
				      uint16_t const tmap_roles, const atc_csis_rsi_t *p_rsi,
				      uint16_t const length, const uint8_t *p_data)
{
	const uint8_t *p_reported_name;
	uint8_t name_length = 0;

	p_reported_name =
		gapm_get_ltv_value(GAP_AD_TYPE_SHORTENED_NAME, length, p_data, &name_length);
	if (!p_reported_name) {
		p_reported_name =
			gapm_get_ltv_value(GAP_AD_TYPE_COMPLETE_NAME, length, p_data, &name_length);
	}

	if (!p_reported_name || !name_length) {
		return;
	}

	/* Check that peripheral name matches */
	if (memcmp(p_reported_name, peripheral_name, sizeof(peripheral_name) - 1)) {
		return;
	}

	/* Notify the application about the found peripheral */
	if ((info_bf & GAF_SCAN_REPORT_INFO_RSI_BIT)) {
		peer_match_cb(p_addr, p_rsi);
	} else {
		peer_match_cb(p_addr, NULL);
	}
}

static void on_gaf_scanning_cb_announcement(const gap_bdaddr_t *const p_addr, uint8_t const type_bf,
					    uint32_t const context_bf,
					    const gaf_ltv_t *const p_metadata)
{

}

static const struct gaf_scan_cb gaf_scan_callbacks = {
	.cb_cmp_evt = on_gaf_scanning_cb_cmp_evt,
	.cb_stopped = on_gaf_scanning_cb_stopped,
	.cb_report = on_gaf_scanning_cb_report,
	.cb_announcement = on_gaf_scanning_cb_announcement,
};

int le_gaf_scan_configure(peer_found_cb_t peer_found_cb)
{
	if (!peer_found_cb) {
		return -EINVAL;
	}
	alif_ble_mutex_lock(K_FOREVER);

	if (peer_match_cb) {
		alif_ble_mutex_unlock();
		return 0;
	}

	uint16_t const err = gaf_scan_configure(&gaf_scan_callbacks);

	if (err != GAF_ERR_NO_ERROR) {
		LOG_ERR("GAF scanning configure error: %u", err);
		alif_ble_mutex_unlock();
		return -ENOEXEC;
	}

	LOG_INF("Scan configured");
	peer_match_cb = peer_found_cb;
	alif_ble_mutex_unlock();
	return 0;
}

int le_gaf_scan_start(scanning_ready_callback_t const ready_cb, uint8_t req_bits)
{
	alif_ble_mutex_lock(K_FOREVER);
	if (scan_ongoing) {
		alif_ble_mutex_unlock();
		return -EBUSY;
	}

	scanning_ready_cb = ready_cb;
	scan_req_bits = req_bits;

	uint16_t const err = gaf_scan_start(scan_req_bits, SCAN_DURATION_SEC, GAP_PHY_1MBPS);

	if (err != GAF_ERR_NO_ERROR) {
		LOG_ERR("GAF scanning start error: %u", err);
		alif_ble_mutex_unlock();
		return -ENOEXEC;
	}

	LOG_INF("Scan started");

	scan_ongoing = true;

	alif_ble_mutex_unlock();
	return 0;
}

int le_gaf_scan_stop(void)
{
	alif_ble_mutex_lock(K_FOREVER);
	if (!scan_ongoing) {
		alif_ble_mutex_unlock();
		return 0;
	}

	uint16_t const err = gaf_scan_stop();

	if (err != GAF_ERR_NO_ERROR) {
		LOG_ERR("GAF scanning stop error: %u", err);
		alif_ble_mutex_unlock();
		return -ENOEXEC;
	}

	alif_ble_mutex_unlock();
	return 0;
}

/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/util.h>
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <string.h>
#include <errno.h>

#include "alif_ble.h"
#include "gapm.h"
#include "gap_le.h"
#include "gapm_le.h"
#include "gapm_le_adv.h"
#include "co_buf.h"
#include "alif/bluetooth/bt_scan_rsp.h"
#include "gap.h"
#include "bt_ad_data.h"

LOG_MODULE_REGISTER(bt_scan_rsp, CONFIG_BT_HOST_LOG_LEVEL);

#define BLE_MUTEX_TIMEOUT_MS 10000

/* Buffer for scan response data */
static co_buf_t *stored_scan_rsp_buf;

/* Maximum scan response data length */
static uint16_t max_scan_rsp_data_len = 31; /* Default BLE spec value */

/**
 * @brief Initialize scan response data module
 *
 * @return 0 on success, negative error code otherwise
 */
int bt_scan_rsp_init(void)
{
	int err;

	/* Pre-allocate the buffer with maximum size */
	err = co_buf_alloc(&stored_scan_rsp_buf, 0, max_scan_rsp_data_len, 0);
	if (err) {
		LOG_ERR("Failed to pre-allocate scan response data buffer");
		return -ENOMEM;
	}

	/* Initialize buffer with zero data length and maximum tail length */
	stored_scan_rsp_buf->data_len = 0;
	stored_scan_rsp_buf->tail_len = max_scan_rsp_data_len;

	return 0;
}

/**
 * @brief Update scan response data for an advertising activity
 *
 * This function creates a copy of the stored scan response buffer and
 * sends it to the controller to update the scan response data for the
 * specified advertising activity. The original buffer is kept intact
 * for future modifications.
 *
 * @param actv_idx Activity index of the advertising set
 * @return 0 on success, negative error code otherwise
 */
static int update_scan_rsp_data(uint8_t actv_idx)
{
	co_buf_t *scan_rsp_buf_final = NULL;
	int err;

	/* Check if buffer exists */
	if (!stored_scan_rsp_buf) {
		/* If for some reason the buffer doesn't exist, allocate a new one */
		err = co_buf_alloc(&stored_scan_rsp_buf, 0, max_scan_rsp_data_len, 0);
		if (err) {
			LOG_ERR("Failed to allocate buffer for scan response data");
			return -ENOMEM;
		}
		/* Initialize with zero data length and maximum tail length */
		stored_scan_rsp_buf->data_len = 0;
		stored_scan_rsp_buf->tail_len = max_scan_rsp_data_len;
	}

	err = co_buf_duplicate(stored_scan_rsp_buf, &scan_rsp_buf_final, 0, 0);
	if (err) {
		LOG_ERR("Failed to duplicate buffer for final scan response, error: %d", err);
		return -ENOMEM;
	}

	/* Set scan response data using the copy */
	int lock_ret = alif_ble_mutex_lock(K_MSEC(BLE_MUTEX_TIMEOUT_MS));

	if (lock_ret) {
		__ASSERT(false, "BLE mutex lock timeout");
		co_buf_release(scan_rsp_buf_final);
		return -ETIMEDOUT;
	}
	err = gapm_le_set_scan_response_data(actv_idx, scan_rsp_buf_final);
	alif_ble_mutex_unlock();
	co_buf_release(scan_rsp_buf_final);

	if (err) {
		LOG_ERR("Failed to set scan response data, error code: 0x%02x", err);
		return -EIO;
	}

	return 0;
}

int bt_scan_rsp_set(uint8_t actv_idx)
{
	int err;

	/* Set scan response data */
	int lock_ret = alif_ble_mutex_lock(K_MSEC(BLE_MUTEX_TIMEOUT_MS));

	if (lock_ret) {
		__ASSERT(false, "BLE mutex lock timeout");
		return -ETIMEDOUT;
	}
	err = update_scan_rsp_data(actv_idx);
	alif_ble_mutex_unlock();

	if (err) {
		LOG_ERR("Cannot set scan response data, error code: 0x%02x", err);
		return -EIO;
	}

	return 0;
}

int bt_scan_rsp_set_name(uint8_t actv_idx, const char *name, size_t name_len)
{
	int err;

	if (name == NULL || name_len == 0) {
		return -EINVAL;
	}

	/* Add name to scan response data */
	err = bt_ad_data_set_tlv(GAP_AD_TYPE_COMPLETE_NAME, name, name_len, stored_scan_rsp_buf);
	if (err) {
		LOG_ERR("Failed to add name to scan response data: %d", err);
	}

	return err;
}

int bt_scan_rsp_set_tlv(uint8_t tlv_type, const void *data, size_t data_len)
{
	return bt_ad_data_set_tlv(tlv_type, data, data_len, stored_scan_rsp_buf);
}

/**
 * @brief Set device name in scan response data, automatically using shortened name if needed
 *
 * This function automatically determines whether to use a complete or shortened name
 * based on the available space in the scan response data. If the complete name doesn't fit,
 * it will be truncated and set as a shortened name.
 *
 * @param name Device name to set
 * @param name_len Length of the device name
 * @return 0 on success, negative error code otherwise
 */
int bt_scan_rsp_data_set_name_auto(const char *name, size_t name_len)
{
	if (!stored_scan_rsp_buf) {
		return -EINVAL;
	}
	return bt_ad_data_set_name_auto(name, name_len, stored_scan_rsp_buf);
}

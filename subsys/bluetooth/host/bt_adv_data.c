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
#include "alif/bluetooth/bt_adv_data.h"
#include "bt_ad_data.h"

LOG_MODULE_REGISTER(bt_adv_data, CONFIG_BT_HOST_LOG_LEVEL);

#define BLE_MUTEX_TIMEOUT_MS 10000

/**
 * @brief Advertising data buffer
 *
 */
static uint16_t max_adv_data_len = CONFIG_BLE_ADV_DATA_MAX; /* Will be updated from controller */
static co_buf_t *stored_adv_buf;

/* Semaphore for synchronizing buffer allocation */
K_SEM_DEFINE(adv_buf_sem, 0, 1);

/**
 * @brief Update advertising data for an activity
 *
 * @param actv_idx Activity index
 * @return 0 on success, negative error code otherwise
 */
static int update_adv_data(uint8_t actv_idx)
{
	int err;
	co_buf_t *adv_buf_final = NULL;

	/* Assert that buffer has been allocated */
	__ASSERT(stored_adv_buf != NULL, "Advertising data buffer not allocated");

	/* Create a copy of the buffer so that we can keep modifying the original buffer */
	err = co_buf_duplicate(stored_adv_buf, &adv_buf_final, 0, 0);
	if (err) {
		LOG_ERR("Failed to duplicate buffer for advertising data, error: %d", err);
		return -ENOMEM;
	}

	/* Get data length for logging */
	uint16_t data_len = co_buf_data_len(adv_buf_final);

	/* Log advertising data using hexdump */
	LOG_DBG("Advertising data (%u bytes):", data_len);
	LOG_HEXDUMP_DBG(co_buf_data(adv_buf_final), data_len, "ADV DATA");

	/* Set advertising data using the copy */
	int lock_ret = alif_ble_mutex_lock(K_MSEC(BLE_MUTEX_TIMEOUT_MS));

	if (lock_ret) {
		__ASSERT(false, "BLE mutex lock timeout");
		co_buf_release(adv_buf_final);
		return -ETIMEDOUT;
	}
	err = gapm_le_set_adv_data(actv_idx, adv_buf_final);
	alif_ble_mutex_unlock();

	/* If there was an error setting the advertising data */
	if (err) {
		__ASSERT(0, "Failed to set advertising data");
		/* Don't release the stored buffer since we want to keep it for future */
	}

	/* Always release the copy since it's no longer needed */
	co_buf_release(adv_buf_final);

	return err;
}

/**
 * @brief Get the name from advertising data (complete or shortened)
 *
 * @param name Buffer to store the device name
 * @param max_len Maximum length of the buffer
 * @return Length of the name on success, negative error code otherwise
 */
int bt_adv_data_get_name_auto(char *name, size_t max_len)
{
	return bt_ad_data_get_name_auto(name, max_len, stored_adv_buf);
}

/* Callback for max advertising data length query */
static void on_max_adv_data_len_cb(uint32_t metainfo, uint16_t status, uint16_t max_len)
{
	int err;

	/* TODO: Legacy advertising only for now */
	max_len = MIN(max_len, CONFIG_BLE_ADV_DATA_MAX);

	if (status == GAP_ERR_NO_ERROR) {
		LOG_INF("Controller supports maximum advertising data length of %u bytes", max_len);
		max_adv_data_len = max_len;
	} else {
		LOG_ERR("Failed to query maximum advertising data length, error: 0x%04x", status);
		/* Continue with default value */
	}

	/* Pre-allocate the buffer with maximum size from controller */
	err = co_buf_alloc(&stored_adv_buf, 0, max_adv_data_len, 0);
	if (err) {
		LOG_ERR("Failed to pre-allocate advertising data buffer");
	}

	/* Signal that buffer allocation is complete */
	k_sem_give(&adv_buf_sem);
}

/**
 * @brief Set default advertising data for an activity
 *
 * @param device_name Pointer to device name string
 * @param name_len Length of the device name
 * @return 0 on success, negative errno otherwise
 */
int bt_adv_data_set_default(const char *device_name, size_t name_len)
{
	int err;

	if (device_name == NULL) {
		LOG_ERR("Device name pointer is NULL");
		return -EINVAL;
	}

	/* Clear any existing advertising data */
	if (stored_adv_buf) {
		/* Release all data back to tail */
		uint16_t current_len = co_buf_data_len(stored_adv_buf);

		if (current_len > 0) {
			co_buf_tail_release(stored_adv_buf, current_len);
		}
	}

	/* Note: Flags are not needed as they're handled by the RivieraWaves API */

	/* Add service UUID (placeholder "Dead Beef" UUID) */
	uint8_t uuid[16] = {0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF,
			    0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF};
	err = bt_ad_data_set_tlv(GAP_AD_TYPE_COMPLETE_LIST_128_BIT_UUID, uuid, sizeof(uuid),
				 stored_adv_buf);
	if (err) {
		LOG_ERR("Failed to add service UUID to advertising data");
		return err;
	}

	/* Add device name with automatic shortening if needed */
	return bt_adv_data_set_name_auto(device_name, name_len);
}

/**
 * @brief Initialize advertising data module
 *
 * @return 0 on success, negative error code otherwise
 */
int bt_adv_data_init(void)
{
	int ret;

	/* Release any stored buffer */
	co_buf_release(stored_adv_buf);
	stored_adv_buf = NULL;

	/* Reset semaphore in case it was previously given */
	k_sem_reset(&adv_buf_sem);

	/* Query the controller for maximum advertising data length
	 * Buffer will be allocated in the callback
	 */
	int lock_ret = alif_ble_mutex_lock(K_MSEC(BLE_MUTEX_TIMEOUT_MS));

	if (lock_ret) {
		__ASSERT(false, "BLE mutex lock timeout");
		return -ETIMEDOUT;
	}
	uint16_t gap_err = gapm_le_get_max_adv_data_len(0, on_max_adv_data_len_cb);

	alif_ble_mutex_unlock();

	if (gap_err != GAP_ERR_NO_ERROR) {
		LOG_WRN("Failed to query maximum advertising data length, error: 0x%04x", gap_err);
		/* Continue with default value, buffer will be allocated in the callback */
	}

	/* Wait for the callback to complete and allocate the buffer */
	ret = k_sem_take(&adv_buf_sem, K_SECONDS(5));
	if (ret != 0) {
		LOG_ERR("Timeout waiting for advertising data buffer allocation");
		return -ETIMEDOUT;
	}

	/* Check if buffer was successfully allocated  */
	if (!stored_adv_buf) {
		__ASSERT(false, "Failed to allocate advertising data buffer");
		return -ENOMEM;
	}

	/* Initialize buffer with zero data length and maximum tail length */
	stored_adv_buf->data_len = 0;
	stored_adv_buf->tail_len = max_adv_data_len;

	return 0;
}

/**
 * @brief Set manufacturer data in advertising data
 *
 * @param company_id Company identifier
 * @param data Manufacturer specific data
 * @param data_len Length of the manufacturer data
 * @return 0 on success, negative error code otherwise
 */
int bt_adv_data_set_manufacturer(uint16_t company_id, const uint8_t *data, size_t data_len)
{

	if (data == NULL && data_len > 0) {
		LOG_ERR("Data pointer is NULL but data_len > 0");
		return -EINVAL;
	}

	/* add_ad_type will check for available space */
	if (!stored_adv_buf) {
		LOG_ERR("Advertising buffer not allocated");
		return -EINVAL;
	}

	/* TODO: Advertising data, legacy one, won't be more than 31 bytes in size and there will be
	 * other structures but let's consider this as the worst case scenario
	 */
	uint8_t manuf_data[CONFIG_BLE_ADV_DATA_MAX];

	/* Prepare manufacturer data with company ID in little-endian format */
	sys_put_le16(company_id, manuf_data);
	memcpy(manuf_data + 2, data, data_len);

	/* Add manufacturer data to advertising data */
	return bt_ad_data_set_tlv(GAP_AD_TYPE_MANU_SPECIFIC_DATA, manuf_data, data_len + 2,
				  stored_adv_buf);
}

/**
 * @brief Set service data in advertising data
 *
 * @param service_uuid Service UUID
 * @param data Service data
 * @param data_len Length of the service data
 * @return 0 on success, negative error code otherwise
 */
int bt_adv_data_set_service_data(uint16_t service_uuid, const uint8_t *data, size_t data_len)
{

	if (data == NULL && data_len > 0) {
		LOG_ERR("Data pointer is NULL but data_len > 0");
		return -EINVAL;
	}

	/* add_ad_type will check for available space */
	if (!stored_adv_buf) {
		LOG_ERR("Advertising buffer not allocated");
		return -EINVAL;
	}

	/* TODO: Advertising data, legacy one, won't be more than 31 bytes in size and there will be
	 * other structures but let's consider this as the worst case scenario
	 */
	uint8_t service_data[CONFIG_BLE_ADV_DATA_MAX];

	/* Prepare service data with UUID in little-endian format */
	sys_put_le16(service_uuid, service_data);
	memcpy(service_data + 2, data, data_len);

	/* Add service data to advertising data */
	return bt_ad_data_set_tlv(GAP_AD_TYPE_SERVICE_16_BIT_DATA, service_data, data_len + 2,
				  stored_adv_buf);
}

/**
 * @brief Set generic AD tlv data in advertising data
 *
 * @param tlv_type Tlv
 * @param data tlv data
 * @param data_len Length of the data
 * @return 0 on success, negative error code otherwise
 */
int bt_adv_data_set_tlv(uint8_t tlv_type, const void *data, size_t data_len)
{
	return bt_ad_data_set_tlv(tlv_type, data, data_len, stored_adv_buf);
}

/**
 * @brief Clear all advertising data
 *
 * @param actv_idx Activity index for the advertising set
 * @return 0 on success, negative error code otherwise
 */
int bt_adv_data_clear(uint8_t actv_idx)
{
	/* Reset advertising data */
	if (stored_adv_buf) {
		/* Release all data back to tail */
		uint16_t current_len = co_buf_data_len(stored_adv_buf);

		if (current_len > 0) {
			co_buf_tail_release(stored_adv_buf, current_len);
		}
	}

	/* Update advertising data (send empty data) */
	return update_adv_data(actv_idx);
}

/**
 * @brief Get current advertising data length
 *
 * @return Current advertising data length in bytes
 */
uint8_t bt_adv_data_get_length(void)
{
	if (!stored_adv_buf) {
		return 0;
	}
	return co_buf_data_len(stored_adv_buf);
}

/**
 * @brief Get pointer to raw advertising data
 *
 * @return Pointer to raw advertising data, or NULL if not available
 */
uint8_t *bt_adv_data_get_raw(void)
{
	if (!stored_adv_buf) {
		return NULL;
	}
	return co_buf_data(stored_adv_buf);
}

/**
 * @brief Check if advertising data contains a name
 *
 * @param name Buffer to store the name if found
 * @param max_len Maximum length of the name buffer
 * @return Length of the name on success, negative error code otherwise
 */
int bt_adv_data_check_name(char *name, size_t max_len)
{
	return bt_ad_data_get_name_auto(name, max_len, stored_adv_buf);
}

/**
 * @brief Set device name in advertising data, automatically using shortened name if needed
 *
 * This function automatically determines whether to use a complete or shortened name
 * based on the available space in the advertising data. If the complete name doesn't fit,
 * it will be truncated and set as a shortened name.
 *
 * @param name Device name to set
 * @param name_len Length of the device name
 * @return 0 on success, negative error code otherwise
 */
int bt_adv_data_set_name_auto(const char *name, size_t name_len)
{
	return bt_ad_data_set_name_auto(name, name_len, stored_adv_buf);
}

int bt_adv_data_set_update(uint8_t actv_idx)
{
	return update_adv_data(actv_idx);
}

int bt_adv_start_le_adv(uint8_t actv_idx, uint16_t duration, uint8_t max_adv_evt,
			uint8_t per_adv_info_bf)
{
	gapm_le_adv_param_t adv_params;

	adv_params.duration = duration;
	adv_params.max_adv_evt = max_adv_evt;
	adv_params.per_adv_info_bf = per_adv_info_bf;

	return gapm_le_start_adv(actv_idx, &adv_params);
}

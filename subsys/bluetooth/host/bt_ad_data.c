/**
 * @brief Bluetooth advertising and response data manipulation API
 *
 * Copyright Alif Semiconductor - All Rights Reserved.
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
#include "co_buf.h"
#include "gapm.h"

#define GAPM_ADV_AD_TYPE_FLAGS_LENGTH 3

LOG_MODULE_REGISTER(bt_ad_data, CONFIG_BT_HOST_LOG_LEVEL);

/**
 * @brief Find an AD type in the advertising data
 *
 * @param type AD type to find
 * @param data_offset Pointer to store the offset to the data
 * @param data_len Pointer to store the length of the data
 * @return Offset to the AD type if found, -ENOENT otherwise
 */
static int find_ad_type(uint8_t type, uint8_t *data_offset, uint8_t *data_len, co_buf_t *stored_buf)
{
	uint8_t offset = 0;
	uint8_t *data;
	uint16_t current_len;

	/* Check if buffer exists */
	if (!stored_buf) {
		__ASSERT(0, "%s called with NULL buffer", __func__);
		return -ENOENT;
	}

	data = co_buf_data(stored_buf);
	current_len = co_buf_data_len(stored_buf);

	while (offset < current_len) {
		uint8_t field_len = data[offset];

		/* Check if we've reached the end of the data */
		if (field_len == 0) {
			break;
		}

		/* Check if this is the type we're looking for */
		if (data[offset + 1] == type) {
			*data_offset = offset + 2;
			*data_len = field_len - 1;
			return offset;
		}

		/* Move to the next AD field */
		offset += field_len + 1;
	}

	return -ENOENT;
}

/**
 * @brief Remove an AD type from the advertising data
 *
 * @param type AD type to remove
 * @return 0 on success, negative error code otherwise
 */
static int remove_ad_type(uint8_t type, co_buf_t *stored_buf)
{
	uint8_t data_offset, data_len;
	int offset = find_ad_type(type, &data_offset, &data_len, stored_buf);
	uint8_t *data;
	uint16_t current_len;

	if (offset < 0) {
		return offset;
	}

	/* Buffer must exist since find_ad_type succeeded */
	__ASSERT(stored_buf != NULL, "stored_adv_buf is NULL after successful find_ad_type");

	/* Get pointer to data buffer */
	data = co_buf_data(stored_buf);

	/* Get current data length */
	current_len = co_buf_data_len(stored_buf);

	/* Calculate total field length (length byte + type + data) */
	uint8_t field_len = data_len + 2;

	/* Remove the field by shifting the rest of the data */
	memmove(&data[offset], &data[offset + field_len], current_len - (offset + field_len));

	/* Release the space back to the tail */
	co_buf_tail_release(stored_buf, field_len);

	LOG_DBG("Removed AD type 0x%02x, field length %u bytes", type, field_len);

	return 0;
}

/**
 * @brief Add an AD type to the advertising data
 *
 * @param type AD type to add
 * @param data Data to add
 * @param len Length of the data
 * @return 0 on success, negative error code otherwise
 */
static int add_ad_type(uint8_t type, const void *data, uint8_t len, co_buf_t *stored_buf)
{
	uint8_t *buf_data;
	uint16_t current_len;
	uint8_t data_offset = 0, data_len = 0;

	/* Check if buffer exists */
	if (!stored_buf) {
		__ASSERT(false, "stored_adv_buf is NULL");
		return -EINVAL;
	}

	/* Validate input parameters */
	__ASSERT(data != NULL || len == 0, "Data pointer is NULL but length > 0");
	__ASSERT(len <= 0xFF - 1, "Data length too large for AD structure");

	/* Check if there's an existing entry of this type */
	int find_result = find_ad_type(type, &data_offset, &data_len, stored_buf);

	/* Calculate how much space we need for the new data */
	uint8_t space_needed = len + 2; /* length byte + type + data */

	/* Check if there's enough space before making any changes */
	if (find_result >= 0) {
		/* We have existing data of this type */
		uint8_t existing_size = data_len + 2; /* length byte + type + data */

		/* Verify the existing data size makes sense */
		__ASSERT(data_len > 0, "Found AD type with zero data length");
		__ASSERT(existing_size <= co_buf_data_len(stored_buf),
			 "Existing AD size exceeds buffer data length");

		/* Check if we have enough space: existing data size + tail space */
		if (space_needed > existing_size + co_buf_tail_len(stored_buf)) {
			return -ENOMEM;
		}

		/* We have enough space, now remove the existing data */
		int remove_result = remove_ad_type(type, stored_buf);

		if (remove_result != 0) {
			__ASSERT(remove_result == 0, "Failed to remove existing AD type");
			return remove_result;
		}
	} else {
		/* No existing data, just check tail space */
		if (space_needed > co_buf_tail_len(stored_buf)) {
			return -ENOMEM;
		}
	}

	/* Get current data length after potential removal */
	current_len = co_buf_data_len(stored_buf);

	/* Get pointer to data buffer */
	buf_data = co_buf_data(stored_buf);

	/* Reserve space in the buffer for our data */
	uint8_t err = co_buf_tail_reserve(stored_buf, space_needed);

	if (err != CO_BUF_ERR_NO_ERROR) {
		/* This should not happen since we checked space earlier */
		__ASSERT(0, "Failed to reserve space even after space check");
		LOG_ERR("Failed to reserve space even after space check");
		return -ENOMEM;
	}

	/* Get updated pointer after reservation */
	buf_data = co_buf_data(stored_buf);

	/* Add length and type at the end of current data */
	buf_data[current_len] = len + 1;
	buf_data[current_len + 1] = type;

	/* Add data */
	memcpy(&buf_data[current_len + 2], data, len);

	return 0;
}

/**
 * @brief Get the name from advertising data (complete or shortened)
 *
 * @param name Buffer to store the device name
 * @param max_len Maximum length of the buffer
 * @return Length of the name on success, negative error code otherwise
 */
int bt_ad_data_get_name_auto(char *name, size_t max_len, co_buf_t *stored_buf)
{
	if (name == NULL || max_len == 0 || !stored_buf) {
		return -EINVAL;
	}

	/* Try to get name from advertising data */
	uint8_t data_offset, data_len;
	int err = find_ad_type(GAP_AD_TYPE_COMPLETE_NAME, &data_offset, &data_len, stored_buf);

	/* If not found, try shortened name */
	if (err < 0) {
		err = find_ad_type(GAP_AD_TYPE_SHORTENED_NAME, &data_offset, &data_len, stored_buf);
	}

	/* If found in advertising data */
	if (err >= 0) {
		/* Get pointer to data buffer */
		uint8_t *data = co_buf_data(stored_buf);

		/* Copy name to buffer, limited by max_len */
		size_t copy_len = MIN(data_len, max_len - 1);

		memcpy(name, &data[data_offset], copy_len);
		name[copy_len] = '\0';

		return data_len;
	}

	/* No name found in advertising data */
	name[0] = '\0';
	return -ENOENT;
}

int bt_ad_data_set_name_auto(const char *name, size_t name_len, co_buf_t *stored_buf)
{
	/* Parameter validation */

	if (!name) {
		LOG_ERR("Name pointer is NULL");
		return -EINVAL;
	}

	if (name_len == 0) {
		LOG_ERR("Name length is zero");
		return -EINVAL;
	}

	if (!stored_buf) {
		LOG_ERR("Advertising buffer not allocated");
		return -EINVAL;
	}

	/* Calculate available space for the name using buffer's tail length
	 * Each AD structure has 2 bytes overhead (length + type)
	 * Unfortunately we need to account for the flags field as well, even though those are set
	 * by the controller implicitly
	 */
	uint16_t available_space = co_buf_tail_len(stored_buf) - GAPM_ADV_AD_TYPE_FLAGS_LENGTH;

	/* Check if we have existing name fields that we need to account for */
	uint8_t data_offset_complete = 0, data_len_complete = 0;
	uint8_t data_offset_short = 0, data_len_short = 0;
	bool has_complete_name = (find_ad_type(GAP_AD_TYPE_COMPLETE_NAME, &data_offset_complete,
					       &data_len_complete, stored_buf) >= 0);
	bool has_short_name = (find_ad_type(GAP_AD_TYPE_SHORTENED_NAME, &data_offset_short,
					    &data_len_short, stored_buf) >= 0);

	/* If we have existing names, we'll remove them and reclaim their space */
	if (has_complete_name) {
		available_space +=
			data_len_complete + 2; /* Add back the space used by complete name */
	}

	if (has_short_name) {
		available_space +=
			data_len_short + 2; /* Add back the space used by shortened name */
	}

	/* Now determine if we need to use shortened name based on available space */
	uint8_t ad_type;
	size_t final_name_len;

	/* Account for length and type bytes in our new name */
	uint8_t name_overhead = 2; /* length + type */

	if (name_len + name_overhead <= available_space) {
		/* Complete name fits */
		ad_type = GAP_AD_TYPE_COMPLETE_NAME;
		final_name_len = name_len;
		LOG_DBG("Using complete name, length: %zu", final_name_len);
	} else if (available_space > name_overhead) {
		/* Need to use shortened name */
		ad_type = GAP_AD_TYPE_SHORTENED_NAME;
		final_name_len = available_space - name_overhead;
		LOG_DBG("Using shortened name, length: %zu (original: %zu)", final_name_len,
			name_len);
	} else {
		__ASSERT(0, "No space available for name in advertising data");
		LOG_ERR("No space available for name in advertising data");
		return -ENOMEM;
	}

	/* Now remove any existing names before adding the new one */
	if (has_complete_name) {
		/* Remove complete name */
		remove_ad_type(GAP_AD_TYPE_COMPLETE_NAME, stored_buf);
		LOG_DBG("Removed existing complete name");
	}

	if (has_short_name) {
		/* Remove shortened name */
		remove_ad_type(GAP_AD_TYPE_SHORTENED_NAME, stored_buf);
		LOG_DBG("Removed existing shortened name");
	}

	/* Add name to advertising data */
	return add_ad_type(ad_type, name, final_name_len, stored_buf);

}

int bt_ad_data_set_tlv(uint8_t tlv_type, const void *data, size_t data_len, co_buf_t *stored_buf)
{

	if (data == NULL && data_len > 0) {
		LOG_ERR("Data pointer is NULL but data_len > 0");
		return -EINVAL;
	}

	/* add_ad_type will check for available space */
	if (!stored_buf) {
		LOG_ERR("Advertising buffer not allocated");
		return -EINVAL;
	}

	/* Add service data to advertising data */
	return add_ad_type(tlv_type, data, data_len, stored_buf);
}

/**
 * @brief Bluetooth scan response data manipulation API
 *
 * Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */
#ifndef BT_SCAN_RSP_H_
#define BT_SCAN_RSP_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize scan response data module
 *
 * @return 0 on success, negative error code otherwise
 */
int bt_scan_rsp_init(void);

/**
 * @brief Set empty or configured scan response data
 *
 * @param actv_idx Activity index for the advertising set
 * @return 0 on success, negative error code otherwise
 */
int bt_scan_rsp_set(uint8_t actv_idx);

/**
 * @brief Set name in scan response data
 *
 * @param actv_idx Activity index for the advertising set
 * @param name Device name to set
 * @param name_len Length of the device name
 * @return 0 on success, negative error code otherwise
 */
int bt_scan_rsp_set_name(uint8_t actv_idx, const char *name, size_t name_len);

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
int bt_scan_rsp_data_set_name_auto(const char *name, size_t name_len);

/**
 * @brief Set generic AD tlv data in scan response data
 *
 * @param actv_idx Activity index for the advertising set
 * @param tlv_type Tlv
 * @param data tlv data
 * @param data_len Length of the data
 * @return 0 on success, negative error code otherwise
 */
int bt_scan_rsp_set_tlv(uint8_t tlv_type, const void *data, size_t data_len);

#ifdef __cplusplus
}
#endif

#endif /* BT_SCAN_RSP_H_ */

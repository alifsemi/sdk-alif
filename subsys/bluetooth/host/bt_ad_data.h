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
#ifndef BT_AD_DATA_H_
#define BT_AD_DATA_H_

#include <stddef.h>
#include <stdint.h>
#include "co_buf.h"

#ifdef __cplusplus
extern "C" {
#endif

int bt_ad_data_get_name_auto(char *name, size_t max_len, co_buf_t *stored_buf);
int bt_ad_data_set_name_auto(const char *name, size_t name_len, co_buf_t *stored_buf);
int bt_ad_data_set_tlv(uint8_t tlv_type, const void *data, size_t data_len, co_buf_t *stored_buf);

#ifdef __cplusplus
}
#endif

#endif /* BT_AD_DATA_H_ */

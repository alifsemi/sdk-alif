/* SPDX-License-Identifier: Apache-2.0 */

/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef TEST_CRC_COMMON_H_
#define TEST_CRC_COMMON_H_

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/crc.h>
#include <stdint.h>
#include <zephyr/ztest.h>
#include "software_crc.h"

#define CRC_TRUE  1
#define CRC_FALSE 0

/* Standard CRC-32 polynomial (normal form) */
#define CRC32_POLY_NORMAL        0x04C11DB7UL
/* Standard CRC-32 polynomial (reflected for SW) */
#define CRC32_POLY_REFLECTED     0xEDB88320UL

/* CRC-32C (Castagnoli) polynomial */
#define CRC32C_HW_POLY           0x1EDC6F41UL   /* Normal form for HW reg */
#define CRC32C_SW_POLY_REFL      0x82F63B78UL   /* Reflected form for SW comp */

/* Custom CRC-32 polynomial */
#define CRC32_CUSTOM_HW_POLY     0x2CEEA6C8UL   /* Normal form for HW reg */
#define CRC32_CUSTOM_SW_POLY_REFL 0x13657734UL  /* Reflected form for SW comp */

/* Shared test input data (defined in main.c) */
extern const uint8_t crc_test_data[];
extern const size_t crc_test_data_len;

/* CRC device handle (defined in main.c) */
extern const struct device *const crc_dev;

/* Shared CRC check string "123456789" — canonical CRC test vector */
extern const uint8_t crc_check_string[];
#define CRC_CHECK_STRING_LEN  9

/* Bit-reflect helpers (defined in main.c) */
uint32_t bit_reflect_n(uint32_t input, uint32_t width);
uint32_t bit_reflect_32(uint32_t input);

#endif /* TEST_CRC_COMMON_H_ */

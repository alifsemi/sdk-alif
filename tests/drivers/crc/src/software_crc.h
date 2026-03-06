/* SPDX-License-Identifier: Apache-2.0 */

/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef SOFTWARE_CRC_H_
#define SOFTWARE_CRC_H_

#include <stdint.h>
#include <stddef.h>

/* CRC algorithm identifiers */
#define CRC32_PRIME     0
#define CRC16_802_15_4  1
#define CRC16_ALT       2
#define CRC8_PRIME      3

#ifndef CHAR_BIT
#define CHAR_BIT        8
#endif

/**
 * @brief Compute CRC using a table-driven algorithm.
 *
 * @param id   CRC algorithm identifier (CRC8_PRIME, CRC16_802_15_4, etc.)
 * @param data Pointer to input data buffer.
 * @param len  Length of data in bytes.
 * @return     Computed CRC value.
 */
unsigned long gen_crc(int id, const unsigned char *data, size_t len);

/**
 * @brief Compute 32-bit CRC with a given polynomial.
 *
 * Uses a reflected (LSB-first) table-driven algorithm with
 * initial value 0xFFFFFFFF and final XOR 0xFFFFFFFF.
 *
 * @param input_str  Pointer to input data buffer.
 * @param num_bytes  Length of data in bytes.
 * @param crc_poly   Reflected polynomial to use.
 * @return           Computed 32-bit CRC value.
 */
uint32_t crc_32(const unsigned char *input_str, size_t num_bytes,
		unsigned long crc_poly);

#endif /* SOFTWARE_CRC_H_ */

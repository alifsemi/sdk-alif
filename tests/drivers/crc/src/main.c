// SPDX-License-Identifier: Apache-2.0

/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include "test_crc_common.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(alif_crc, CONFIG_LOG_DEFAULT_LEVEL);

/* ------------------------------------------------------------------ */
/* Shared test data                                                    */
/* ------------------------------------------------------------------ */
const uint8_t crc_test_data[] = {
	0x67, 0x3F, 0x90, 0xC9, 0x25, 0xF0, 0x4A, 0xB1, 0x13
};
const size_t crc_test_data_len = ARRAY_SIZE(crc_test_data);

/* Canonical CRC check string "123456789" */
const uint8_t crc_check_string[] = {
	'1', '2', '3', '4', '5', '6', '7', '8', '9'
};

const struct device *const crc_dev = DEVICE_DT_GET(DT_ALIAS(crc));

/* ------------------------------------------------------------------ */
/* Bit-reflect helpers                                                 */
/* ------------------------------------------------------------------ */
uint32_t bit_reflect_n(uint32_t input, uint32_t width)
{
	uint32_t reflected = 0;
	uint32_t i;

	for (i = 0; i < width; i++) {
		if (input & (1U << i)) {
			reflected |= 1U << (width - 1U - i);
		}
	}
	return reflected;
}

uint32_t bit_reflect_32(uint32_t input)
{
	return bit_reflect_n(input, 32);
}

/* ------------------------------------------------------------------ */
/* Common test fixture — validates device before each test             */
/* ------------------------------------------------------------------ */
static void crc_test_before(void *fixture)
{
	ARG_UNUSED(fixture);

	zassert_true(crc_dev != NULL, "CRC device not found");
	zassert_true(device_is_ready(crc_dev), "CRC device not ready");
	LOG_DBG("CRC device ready: %s", crc_dev->name);
}

/* ------------------------------------------------------------------ */
/* Suite registration                                                  */
/* ------------------------------------------------------------------ */
ZTEST_SUITE(crc_test, NULL, NULL, crc_test_before, NULL, NULL);

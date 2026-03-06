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
LOG_MODULE_DECLARE(alif_crc, CONFIG_LOG_DEFAULT_LEVEL);

#if IS_ENABLED(CONFIG_TEST_CRC_32_CUSTOM_POLY)

/* CRC-32 custom polynomial constants now defined in test_crc_common.h */

/* ZTC-1188 / ZTC-1379: 32-bit CRC with custom polynomial */
ZTEST(crc_test, crc_32_bit_custom_poly)
{
	uint32_t crc_output, software_output;
	struct crc_params params = {
		.data_in     = (uint8_t *)crc_test_data,
		.len         = crc_test_data_len,
		.bit_swap    = CRC_TRUE,
		.byte_swap   = CRC_TRUE,
		.reflect     = CRC_FALSE,
		.invert      = CRC_TRUE,
		.custom_poly = CRC_TRUE,
		.data_out    = &crc_output,
	};

	zassert_equal(crc_set_seed(crc_dev, 0xFFFFFFFF), 0,
		      "CRC set seed failed");
	zassert_equal(crc_set_polynomial(crc_dev, CRC32_CUSTOM_HW_POLY), 0,
		      "CRC set custom polynomial failed");
	zassert_equal(crc_compute(crc_dev, &params), 0,
		      "CRC compute failed");
	LOG_INF("32-bit custom poly CRC HW output: 0x%X", crc_output);

	software_output = crc_32((const unsigned char *)crc_test_data,
				 crc_test_data_len, CRC32_CUSTOM_SW_POLY_REFL);
	software_output = bit_reflect_32(software_output);
	LOG_INF("32-bit custom poly CRC SW output: 0x%X", software_output);

	zassert_equal(crc_output, software_output,
		      "HW/SW CRC mismatch: 0x%X != 0x%X",
		      crc_output, software_output);
}

/* ZTC-1386: 32-bit CRC custom poly with reflected output enabled */
ZTEST(crc_test, crc_32_bit_custom_poly_reflect_output_en)
{
	uint32_t crc_output, software_output;
	struct crc_params params = {
		.data_in     = (uint8_t *)crc_test_data,
		.len         = crc_test_data_len,
		.bit_swap    = CRC_TRUE,
		.byte_swap   = CRC_TRUE,
		.reflect     = CRC_TRUE,
		.invert      = CRC_TRUE,
		.custom_poly = CRC_TRUE,
		.data_out    = &crc_output,
	};

	zassert_equal(crc_set_seed(crc_dev, 0xFFFFFFFF), 0,
		      "CRC set seed failed");
	zassert_equal(crc_set_polynomial(crc_dev, CRC32_CUSTOM_HW_POLY), 0,
		      "CRC set custom polynomial failed");
	zassert_equal(crc_compute(crc_dev, &params), 0,
		      "CRC compute failed");
	LOG_INF("32-bit custom poly CRC HW output: 0x%X", crc_output);

	software_output = crc_32((const unsigned char *)crc_test_data,
				 crc_test_data_len, CRC32_CUSTOM_SW_POLY_REFL);
	LOG_INF("32-bit custom poly CRC SW output: 0x%X", software_output);

	zassert_equal(crc_output, software_output,
		      "HW/SW CRC mismatch: 0x%X != 0x%X",
		      crc_output, software_output);
}

#endif /* CONFIG_TEST_CRC_32_CUSTOM_POLY */

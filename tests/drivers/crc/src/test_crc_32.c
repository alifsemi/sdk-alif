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

#if IS_ENABLED(CONFIG_TEST_CRC_32)

/* CRC32_POLY_REFLECTED now defined in test_crc_common.h */

/* ZTC-1377 / ZTC-1379: Standard 32-bit CRC (full mode: */
/* swap + reflect + invert) */
ZTEST(crc_test, crc_32_bit)
{
	uint32_t crc_output, software_output;
	struct crc_params params = {
		.data_in     = (uint8_t *)crc_test_data,
		.len         = crc_test_data_len,
		.bit_swap    = CRC_TRUE,
		.byte_swap   = CRC_TRUE,
		.reflect     = CRC_TRUE,
		.invert      = CRC_TRUE,
		.custom_poly = CRC_FALSE,
		.data_out    = &crc_output,
	};

	zassert_equal(crc_set_seed(crc_dev, 0xFFFFFFFF), 0,
		      "CRC set seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0,
		      "CRC compute failed");
	LOG_INF("32-bit CRC HW output: 0x%X", crc_output);

	software_output = crc_32((const unsigned char *)crc_test_data,
				 crc_test_data_len, CRC32_POLY_REFLECTED);
	LOG_INF("32-bit CRC SW output: 0x%X",
		 software_output);

	zassert_equal(crc_output, software_output,
		      "HW/SW CRC mismatch: 0x%X != 0x%X",
		      crc_output, software_output);
}

/* ZTC-1187: 32-bit CRC with reflect output disabled */
ZTEST(crc_test, crc_32_bit_reflect_output_disable)
{
	uint32_t crc_output, software_output;
	struct crc_params params = {
		.data_in     = (uint8_t *)crc_test_data,
		.len         = crc_test_data_len,
		.bit_swap    = CRC_TRUE,
		.byte_swap   = CRC_TRUE,
		.reflect     = CRC_FALSE,
		.invert      = CRC_TRUE,
		.custom_poly = CRC_FALSE,
		.data_out    = &crc_output,
	};

	zassert_equal(crc_set_seed(crc_dev, 0xFFFFFFFF), 0,
		      "CRC set seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0,
		      "CRC compute failed");
	LOG_INF("32-bit CRC HW output: 0x%X", crc_output);

	software_output = crc_32((const unsigned char *)crc_test_data,
				 crc_test_data_len, CRC32_POLY_REFLECTED);
	software_output = bit_reflect_32(software_output);
	LOG_INF("32-bit CRC SW output (reflect disabled): 0x%X",
		 software_output);

	zassert_equal(crc_output, software_output,
		      "HW/SW CRC mismatch: 0x%X != 0x%X",
		      crc_output, software_output);
}

/* ZTC-NEW: 32-bit CRC with unaligned length (5 bytes — */
/* exercises SW fallback) */
ZTEST(crc_test, crc_32_bit_unaligned_5_bytes)
{
	uint32_t crc_output, software_output;
	uint8_t data_5[] = {0x67, 0x3F, 0x90, 0xC9, 0x25};
	struct crc_params params = {
		.data_in     = data_5,
		.len         = 5,
		.bit_swap    = CRC_TRUE,
		.byte_swap   = CRC_TRUE,
		.reflect     = CRC_TRUE,
		.invert      = CRC_TRUE,
		.custom_poly = CRC_FALSE,
		.data_out    = &crc_output,
	};

	zassert_equal(crc_set_seed(crc_dev, 0xFFFFFFFF), 0,
		      "CRC set seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0,
		      "CRC compute failed");
	LOG_INF("32-bit CRC HW 5-byte output: 0x%X",
		 crc_output);

	software_output = crc_32(data_5, 5, CRC32_POLY_REFLECTED);
	LOG_INF("32-bit CRC SW 5-byte output: 0x%X",
		 software_output);

	zassert_equal(crc_output, software_output,
		      "HW/SW CRC mismatch (5-byte unaligned): 0x%X != 0x%X",
		      crc_output, software_output);
}

/* ZTC-NEW: 32-bit CRC with unaligned length (7 bytes) */
ZTEST(crc_test, crc_32_bit_unaligned_7_bytes)
{
	uint32_t crc_output, software_output;
	uint8_t data_7[] = {0x67, 0x3F, 0x90, 0xC9, 0x25, 0xF0, 0x4A};
	struct crc_params params = {
		.data_in     = data_7,
		.len         = 7,
		.bit_swap    = CRC_TRUE,
		.byte_swap   = CRC_TRUE,
		.reflect     = CRC_TRUE,
		.invert      = CRC_TRUE,
		.custom_poly = CRC_FALSE,
		.data_out    = &crc_output,
	};

	zassert_equal(crc_set_seed(crc_dev, 0xFFFFFFFF), 0,
		      "CRC set seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0,
		      "CRC compute failed");
	LOG_INF("32-bit CRC HW 7-byte output: 0x%X",
		 crc_output);

	software_output = crc_32(data_7, 7, CRC32_POLY_REFLECTED);
	LOG_INF("32-bit CRC SW 7-byte output: 0x%X",
		 software_output);

	zassert_equal(crc_output, software_output,
		      "HW/SW CRC mismatch (7-byte unaligned): 0x%X != 0x%X",
		      crc_output, software_output);
}

/* ZTC-NEW: 32-bit CRC with unaligned length (1 byte — all SW fallback) */
ZTEST(crc_test, crc_32_bit_unaligned_1_byte)
{
	uint32_t crc_output, software_output;
	uint8_t data_1[] = {0xAB};
	struct crc_params params = {
		.data_in     = data_1,
		.len         = 1,
		.bit_swap    = CRC_TRUE,
		.byte_swap   = CRC_TRUE,
		.reflect     = CRC_TRUE,
		.invert      = CRC_TRUE,
		.custom_poly = CRC_FALSE,
		.data_out    = &crc_output,
	};

	zassert_equal(crc_set_seed(crc_dev, 0xFFFFFFFF), 0,
		      "CRC set seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0,
		      "CRC compute failed");
	LOG_INF("32-bit CRC HW 1-byte output: 0x%X",
		 crc_output);

	software_output = crc_32(data_1, 1, CRC32_POLY_REFLECTED);
	LOG_INF("32-bit CRC SW 1-byte output: 0x%X",
		 software_output);

	zassert_equal(crc_output, software_output,
		      "HW/SW CRC mismatch (1-byte unaligned): 0x%X != 0x%X",
		      crc_output, software_output);
}

/* ZTC-NEW: 32-bit CRC with exactly 4-byte aligned length */
ZTEST(crc_test, crc_32_bit_aligned_4_bytes)
{
	uint32_t crc_output, software_output;
	uint8_t data_4[] = {0x67, 0x3F, 0x90, 0xC9};
	struct crc_params params = {
		.data_in     = data_4,
		.len         = 4,
		.bit_swap    = CRC_TRUE,
		.byte_swap   = CRC_TRUE,
		.reflect     = CRC_TRUE,
		.invert      = CRC_TRUE,
		.custom_poly = CRC_FALSE,
		.data_out    = &crc_output,
	};

	zassert_equal(crc_set_seed(crc_dev, 0xFFFFFFFF), 0,
		      "CRC set seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0,
		      "CRC compute failed");
	LOG_INF("32-bit CRC HW 4-byte output: 0x%X", crc_output);

	software_output = crc_32(data_4, 4, CRC32_POLY_REFLECTED);
	LOG_INF("32-bit CRC SW 4-byte output: 0x%X", software_output);

	zassert_equal(crc_output, software_output,
		      "HW/SW CRC mismatch (4-byte aligned): 0x%X != 0x%X",
		      crc_output, software_output);
}

/* ZTC-NEW: 32-bit CRC with 8-byte aligned length */
ZTEST(crc_test, crc_32_bit_aligned_8_bytes)
{
	uint32_t crc_output, software_output;
	uint8_t data_8[] = {0x67, 0x3F, 0x90, 0xC9, 0x25, 0xF0, 0x4A, 0xB1};
	struct crc_params params = {
		.data_in     = data_8,
		.len         = 8,
		.bit_swap    = CRC_TRUE,
		.byte_swap   = CRC_TRUE,
		.reflect     = CRC_TRUE,
		.invert      = CRC_TRUE,
		.custom_poly = CRC_FALSE,
		.data_out    = &crc_output,
	};

	zassert_equal(crc_set_seed(crc_dev, 0xFFFFFFFF), 0,
		      "CRC set seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0,
		      "CRC compute failed");
	LOG_INF("32-bit CRC HW 8-byte output: 0x%X", crc_output);

	software_output = crc_32(data_8, 8, CRC32_POLY_REFLECTED);
	LOG_INF("32-bit CRC SW 8-byte output: 0x%X", software_output);

	zassert_equal(crc_output, software_output,
		      "HW/SW CRC mismatch (8-byte aligned): 0x%X != 0x%X",
		      crc_output, software_output);
}

#endif /* CONFIG_TEST_CRC_32 */

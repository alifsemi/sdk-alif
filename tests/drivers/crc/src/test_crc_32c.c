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

#if IS_ENABLED(CONFIG_TEST_CRC_32C)

/* CRC-32C polynomial constants now defined in test_crc_common.h */

/* Known CRC-32C check value: CRC32C("123456789") = 0xE3069283 */
#define CRC32C_CHECK_VALUE    0xE3069283UL

/* ZTC-NEW: Basic CRC-32C (Castagnoli) with standard test vector */
ZTEST(crc_test, crc_32c_basic)
{
	uint32_t crc_output, software_output;
	struct crc_params params = {
		.data_in     = crc_check_string,
		.len         = CRC_CHECK_STRING_LEN,
		.bit_swap    = CRC_TRUE,
		.byte_swap   = CRC_TRUE,
		.reflect     = CRC_TRUE,
		.invert      = CRC_TRUE,
		.custom_poly = CRC_TRUE,
		.data_out    = &crc_output,
	};

	zassert_equal(crc_set_seed(crc_dev, 0xFFFFFFFF), 0,
		      "CRC set seed failed");
	zassert_equal(crc_set_polynomial(crc_dev, CRC32C_HW_POLY), 0,
		      "CRC set CRC-32C polynomial failed");
	zassert_equal(crc_compute(crc_dev, &params), 0,
		      "CRC compute failed");
	LOG_INF("CRC-32C HW output for '123456789': 0x%08X",
		 crc_output);

	/* Validate against software reference */
	software_output = crc_32(crc_check_string, CRC_CHECK_STRING_LEN,
				 CRC32C_SW_POLY_REFL);
	LOG_INF("CRC-32C SW output for '123456789': 0x%08X",
		 software_output);

	zassert_equal(crc_output, software_output,
		      "HW/SW CRC-32C mismatch: HW=0x%08X SW=0x%08X",
		      crc_output, software_output);

	/* Validate against known check value */
	zassert_equal(software_output, CRC32C_CHECK_VALUE,
		      "CRC-32C known vector mismatch: got=0x%08X expected=0x%08X",
		      software_output, CRC32C_CHECK_VALUE);

	LOG_INF("CRC-32C '123456789' = 0x%08X (expected 0x%08X)",
		 software_output, CRC32C_CHECK_VALUE);
	LOG_INF("CRC-32C '123456789' check passed.");
}

/* ZTC-NEW: CRC-32C with test_data (HW vs SW) */
ZTEST(crc_test, crc_32c_test_data)
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
	zassert_equal(crc_set_polynomial(crc_dev, CRC32C_HW_POLY), 0,
		      "CRC set CRC-32C polynomial failed");
	zassert_equal(crc_compute(crc_dev, &params), 0,
		      "CRC compute failed");
	LOG_INF("CRC-32C HW test_data output: 0x%08X",
		 crc_output);

	software_output = crc_32((const unsigned char *)crc_test_data,
				 crc_test_data_len, CRC32C_SW_POLY_REFL);
	LOG_INF("CRC-32C SW test_data output: 0x%08X",
		 software_output);

	zassert_equal(crc_output, software_output,
		      "HW/SW CRC-32C mismatch: HW=0x%08X SW=0x%08X",
		      crc_output, software_output);
}

/* ZTC-NEW: CRC-32C with reflect output disabled */
ZTEST(crc_test, crc_32c_reflect_output_disable)
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
	zassert_equal(crc_set_polynomial(crc_dev, CRC32C_HW_POLY), 0,
		      "CRC set CRC-32C polynomial failed");
	zassert_equal(crc_compute(crc_dev, &params), 0,
		      "CRC compute failed");
	LOG_INF("CRC-32C HW reflect-disabled output: 0x%08X",
		 crc_output);

	software_output = crc_32((const unsigned char *)crc_test_data,
				 crc_test_data_len, CRC32C_SW_POLY_REFL);
	software_output = bit_reflect_32(software_output);
	LOG_INF("CRC-32C SW reflect-disabled output: 0x%08X",
		 software_output);

	zassert_equal(crc_output, software_output,
		      "HW/SW CRC-32C mismatch (reflect disabled): HW=0x%08X SW=0x%08X",
		      crc_output, software_output);
}

/* ZTC-NEW: CRC-32C with unaligned 5-byte input */
ZTEST(crc_test, crc_32c_unaligned_5_bytes)
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
		.custom_poly = CRC_TRUE,
		.data_out    = &crc_output,
	};

	zassert_equal(crc_set_seed(crc_dev, 0xFFFFFFFF), 0,
		      "CRC set seed failed");
	zassert_equal(crc_set_polynomial(crc_dev, CRC32C_HW_POLY), 0,
		      "CRC set CRC-32C polynomial failed");
	zassert_equal(crc_compute(crc_dev, &params), 0,
		      "CRC compute failed");
	LOG_INF("CRC-32C HW 5-byte output: 0x%08X",
		 crc_output);

	software_output = crc_32(data_5, 5, CRC32C_SW_POLY_REFL);
	LOG_INF("CRC-32C SW 5-byte output: 0x%08X",
		 software_output);

	zassert_equal(crc_output, software_output,
		      "HW/SW CRC-32C mismatch (5-byte): HW=0x%08X SW=0x%08X",
		      crc_output, software_output);
}

/* ZTC-NEW: CRC-32C with single byte input */
ZTEST(crc_test, crc_32c_single_byte)
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
		.custom_poly = CRC_TRUE,
		.data_out    = &crc_output,
	};

	zassert_equal(crc_set_seed(crc_dev, 0xFFFFFFFF), 0,
		      "CRC set seed failed");
	zassert_equal(crc_set_polynomial(crc_dev, CRC32C_HW_POLY), 0,
		      "CRC set CRC-32C polynomial failed");
	zassert_equal(crc_compute(crc_dev, &params), 0,
		      "CRC compute failed");
	LOG_INF("CRC-32C HW single-byte output: 0x%08X",
		 crc_output);

	software_output = crc_32(data_1, 1, CRC32C_SW_POLY_REFL);
	LOG_INF("CRC-32C SW single-byte output: 0x%08X",
		 software_output);

	zassert_equal(crc_output, software_output,
		      "HW/SW CRC-32C mismatch (1-byte): HW=0x%08X SW=0x%08X",
		      crc_output, software_output);
}

/* ZTC-NEW: CRC-32C vs CRC-32 — verify different */
/* polynomials produce different results */
ZTEST(crc_test, crc_32c_vs_crc32_different)
{
	uint32_t crc_32c_out = 0, crc_32_out = 0;
	struct crc_params params = {
		.data_in     = (uint8_t *)crc_test_data,
		.len         = crc_test_data_len,
		.bit_swap    = CRC_TRUE,
		.byte_swap   = CRC_TRUE,
		.reflect     = CRC_TRUE,
		.invert      = CRC_TRUE,
		.custom_poly = CRC_TRUE,
		.data_out    = &crc_32c_out,
	};

	/* Compute CRC-32C */
	zassert_equal(crc_set_seed(crc_dev, 0xFFFFFFFF), 0, "seed failed");
	zassert_equal(crc_set_polynomial(crc_dev, CRC32C_HW_POLY), 0,
		      "set CRC-32C poly failed");
	zassert_equal(crc_compute(crc_dev, &params), 0,
		      "CRC-32C compute failed");

	/* Compute standard CRC-32 (using custom poly mode with standard
	 * poly)
	 */
	params.data_out = &crc_32_out;
	zassert_equal(crc_set_seed(crc_dev, 0xFFFFFFFF), 0, "seed failed");
	zassert_equal(crc_set_polynomial(crc_dev, 0x04C11DB7), 0,
		      "set CRC-32 poly failed");
	zassert_equal(crc_compute(crc_dev, &params), 0,
		      "CRC-32 compute failed");

	LOG_INF("CRC-32C=0x%08X, CRC-32=0x%08X", crc_32c_out, crc_32_out);

	zassert_not_equal(crc_32c_out, crc_32_out,
			  "CRC-32C and CRC-32 should produce different results: both=0x%08X",
			  crc_32c_out);
}

#endif /* CONFIG_TEST_CRC_32C */

/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include "software_crc.h"

#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(alif_crc_test);

#if IS_ENABLED(CONFIG_TEST_CRC_32C)

static void crc_32c_before(void *fixture)
{
	ARG_UNUSED(fixture);

	crc_test_verify_device();
	crc_test_force_hw_algo(TEST_CRC_HW_ALGO_CRC32C);
}

/* CRC-32C polynomial constants now defined in test_crc_common.h */

/* Known CRC-32C check value: CRC32C("123456789") = 0xE3069283 */
#define CRC32C_CHECK_VALUE 0xE3069283UL

static void crc32c_validate(const uint8_t *data, size_t len, bool custom_poly,
			    uint32_t *crc_output)
{
	struct crc_params params = {
		.data_in = data,
		.len = len,
		.bit_swap = CRC_TRUE,
		.byte_swap = CRC_TRUE,
		.reflect = CRC_TRUE,
		.invert = CRC_TRUE,
		.custom_poly = custom_poly,
		.data_out = crc_output,
	};
	uint32_t software_output;

	zassert_equal(crc_set_seed(crc_dev, 0xFFFFFFFFU), 0,
		      "CRC set seed failed");
	if (custom_poly) {
		zassert_equal(crc_set_polynomial(crc_dev, CRC32C_HW_POLY), 0,
			      "CRC set CRC-32C polynomial failed");
	}
	zassert_equal(crc_compute(crc_dev, &params), 0, "CRC compute failed");

	software_output = crc32c_full_reference(data, len);
	zassert_equal(*crc_output, software_output,
		      "HW/SW CRC-32C mismatch: HW=0x%08X SW=0x%08X",
		      *crc_output, software_output);
}

/* ZTC-NEW: Basic CRC-32C (Castagnoli) with standard test vector */
ZTEST(crc_32c_tests, test_crc_32c_basic)
{
	uint32_t crc_output, software_output;
	struct crc_params params = {
		.data_in = crc_check_string,
		.len = CRC_CHECK_STRING_LEN,
		.bit_swap = CRC_TRUE,
		.byte_swap = CRC_TRUE,
		.reflect = CRC_TRUE,
		.invert = CRC_TRUE,
		.custom_poly = CRC_FALSE,
		.data_out = &crc_output,
	};

	zassert_equal(crc_set_seed(crc_dev, 0xFFFFFFFF), 0,
		      "CRC set seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "CRC compute failed");
	LOG_INF("CRC-32C HW output for '123456789': 0x%08X", crc_output);

	software_output =
		crc32c_full_reference(crc_check_string, CRC_CHECK_STRING_LEN);
	LOG_INF("CRC-32C SW output for '123456789': 0x%08X", software_output);

	zassert_equal(crc_output, software_output,
		      "HW/SW CRC-32C mismatch: HW=0x%08X SW=0x%08X", crc_output,
		      software_output);

	/* Validate against known check value */
	zassert_equal(
		software_output, CRC32C_CHECK_VALUE,
		"CRC-32C known vector mismatch: got=0x%08X expected=0x%08X",
		software_output, CRC32C_CHECK_VALUE);

	LOG_INF("CRC-32C '123456789' = 0x%08X (expected 0x%08X)",
		software_output, (uint32_t)CRC32C_CHECK_VALUE);
	LOG_INF("CRC-32C '123456789' check passed.");
}

ZTEST(crc_32c_tests, test_crc_32c_custom_poly_matches_native)
{
	uint32_t crc_output = 0U;

	crc32c_validate(crc_check_string, CRC_CHECK_STRING_LEN, true,
			&crc_output);
	zassert_equal(crc_output, CRC32C_CHECK_VALUE,
		      "CRC-32C custom polynomial mismatch: 0x%08X", crc_output);
}

/* ZTC-NEW: CRC-32C with test_data (HW vs SW) */
ZTEST(crc_32c_tests, test_crc_32c_test_data)
{
	uint32_t crc_output, software_output;
	struct crc_params params = {
		.data_in = (uint8_t *)crc_test_data,
		.len = crc_test_data_len,
		.bit_swap = CRC_TRUE,
		.byte_swap = CRC_TRUE,
		.reflect = CRC_TRUE,
		.invert = CRC_TRUE,
		.custom_poly = CRC_FALSE,
		.data_out = &crc_output,
	};

	zassert_equal(crc_set_seed(crc_dev, 0xFFFFFFFF), 0,
		      "CRC set seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "CRC compute failed");
	LOG_INF("CRC-32C HW test_data output: 0x%08X", crc_output);

	software_output = crc32c_full_reference((const uint8_t *)crc_test_data,
						crc_test_data_len);
	LOG_INF("CRC-32C SW test_data output: 0x%08X", software_output);

	zassert_equal(crc_output, software_output,
		      "HW/SW CRC-32C mismatch: HW=0x%08X SW=0x%08X", crc_output,
		      software_output);
}

/* ZTC-NEW: CRC-32C with reflect output disabled */
ZTEST(crc_32c_tests, test_crc_32c_reflect_output_disable)
{
	uint32_t crc_output, software_output;
	struct crc_params params = {
		.data_in = (uint8_t *)crc_test_data,
		.len = crc_test_data_len,
		.bit_swap = CRC_TRUE,
		.byte_swap = CRC_TRUE,
		.reflect = CRC_FALSE,
		.invert = CRC_TRUE,
		.custom_poly = CRC_FALSE,
		.data_out = &crc_output,
	};

	zassert_equal(crc_set_seed(crc_dev, 0xFFFFFFFF), 0,
		      "CRC set seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "CRC compute failed");
	LOG_INF("CRC-32C HW reflect-disabled output: 0x%08X", crc_output);

	software_output = crc32_driver_reference(
		(const uint8_t *)crc_test_data, crc_test_data_len, 0xFFFFFFFFU,
		true, true, false, true, CRC32C_HW_POLY, CRC32C_SW_POLY_REFL);
	LOG_INF("CRC-32C SW reflect-disabled output: 0x%08X", software_output);

	zassert_equal(crc_output, software_output,
		      "CRC-32C reflect-disabled mismatch: HW=0x%08X SW=0x%08X",
		      crc_output, software_output);
}

/* ZTC-NEW: CRC-32C with unaligned 5-byte input */
ZTEST(crc_32c_tests, test_crc_32c_unaligned_5_bytes)
{
	uint32_t crc_output, software_output;
	uint8_t data_5[] = {0x67, 0x3F, 0x90, 0xC9, 0x25};
	struct crc_params params = {
		.data_in = data_5,
		.len = 5,
		.bit_swap = CRC_TRUE,
		.byte_swap = CRC_TRUE,
		.reflect = CRC_TRUE,
		.invert = CRC_TRUE,
		.custom_poly = CRC_FALSE,
		.data_out = &crc_output,
	};

	zassert_equal(crc_set_seed(crc_dev, 0xFFFFFFFF), 0,
		      "CRC set seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "CRC compute failed");
	LOG_INF("CRC-32C HW 5-byte output: 0x%08X", crc_output);

	software_output = crc32c_full_reference(data_5, 5);
	LOG_INF("CRC-32C SW 5-byte output: 0x%08X", software_output);

	zassert_equal(crc_output, software_output,
		      "HW/SW CRC-32C mismatch (5-byte): HW=0x%08X SW=0x%08X",
		      crc_output, software_output);
}

/* ZTC-NEW: CRC-32C with single byte input */
ZTEST(crc_32c_tests, test_crc_32c_single_byte)
{
	uint32_t crc_output, software_output;
	uint8_t data_1[] = {0xAB};
	struct crc_params params = {
		.data_in = data_1,
		.len = 1,
		.bit_swap = CRC_TRUE,
		.byte_swap = CRC_TRUE,
		.reflect = CRC_TRUE,
		.invert = CRC_TRUE,
		.custom_poly = CRC_FALSE,
		.data_out = &crc_output,
	};

	zassert_equal(crc_set_seed(crc_dev, 0xFFFFFFFF), 0,
		      "CRC set seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "CRC compute failed");
	LOG_INF("CRC-32C HW single-byte output: 0x%08X", crc_output);

	software_output = crc32c_full_reference(data_1, 1);
	LOG_INF("CRC-32C SW single-byte output: 0x%08X", software_output);

	zassert_equal(crc_output, software_output,
		      "HW/SW CRC-32C mismatch (1-byte): HW=0x%08X SW=0x%08X",
		      crc_output, software_output);
}

/* ZTC-NEW: CRC-32C vs CRC-32 — verify different */
/* polynomials produce different results */
ZTEST(crc_32c_tests, test_crc_32c_vs_crc32_different)
{
	uint32_t crc_32c_out = 0, crc_32_out = 0;
	uint32_t crc_32c_sw;
	uint32_t crc_32_sw;
	struct crc_params params = {
		.data_in = (uint8_t *)crc_test_data,
		.len = crc_test_data_len,
		.bit_swap = CRC_TRUE,
		.byte_swap = CRC_TRUE,
		.reflect = CRC_TRUE,
		.invert = CRC_TRUE,
		.custom_poly = CRC_FALSE,
		.data_out = &crc_32c_out,
	};

	/* Compute native CRC-32C */
	crc_test_force_hw_algo(TEST_CRC_HW_ALGO_CRC32C);
	zassert_equal(crc_set_seed(crc_dev, 0xFFFFFFFF), 0, "seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0,
		      "CRC-32C compute failed");
	crc_32c_sw = crc32c_full_reference((const uint8_t *)crc_test_data,
					   crc_test_data_len);
	zassert_equal(crc_32c_out, crc_32c_sw,
		      "CRC-32C native mismatch: HW=0x%08X SW=0x%08X",
		      crc_32c_out, crc_32c_sw);

	/*
	 * Switch to native CRC-32 to verify algorithm selection, not just the
	 * programmed polynomial.
	 */
	crc_test_force_hw_algo(TEST_CRC_HW_ALGO_CRC32);
	params.data_out = &crc_32_out;
	zassert_equal(crc_set_seed(crc_dev, 0xFFFFFFFF), 0, "seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0,
		      "CRC-32 compute failed");
	crc_32_sw = crc32_full_reference((const uint8_t *)crc_test_data,
					 crc_test_data_len);
	zassert_equal(crc_32_out, crc_32_sw,
		      "CRC-32 native mismatch: HW=0x%08X SW=0x%08X", crc_32_out,
		      crc_32_sw);

	LOG_INF("CRC-32C=0x%08X, CRC-32=0x%08X", crc_32c_out, crc_32_out);

	zassert_not_equal(crc_32c_out, crc_32_out,
			  "CRC-32C and CRC-32 matched unexpectedly: 0x%08X",
			  crc_32c_out);
}

ZTEST(crc_32c_tests, test_crc_32c_misaligned_ptr_aligned_len)
{
	uint32_t crc_output = 0U;
	uint8_t raw_data[] = {0xA5, 0x67, 0x3F, 0x90, 0xC9,
			      0x25, 0xF0, 0x4A, 0xB1};
	const uint8_t *misaligned = &raw_data[1];

	zassert_true((((uintptr_t)misaligned) & 0x3U) != 0U,
		     "test buffer is unexpectedly aligned");

	crc32c_validate(misaligned, 8U, false, &crc_output);
}

ZTEST(crc_32c_tests, test_crc_32c_misaligned_ptr_tail_len)
{
	uint32_t crc_output = 0U;
	uint8_t raw_data[] = {0x5A, 0x67, 0x3F, 0x90, 0xC9, 0x25, 0xF0, 0x4A};
	const uint8_t *misaligned = &raw_data[1];

	zassert_true((((uintptr_t)misaligned) & 0x3U) != 0U,
		     "test buffer is unexpectedly aligned");

	crc32c_validate(misaligned, 7U, false, &crc_output);
}

ZTEST(crc_32c_tests, test_stress_unaligned_std_tail)
{
	uint32_t crc_hw = 0;
	uint32_t crc_sw;
	uint8_t unaligned_data[7] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE};
	struct crc_params params = {
		.data_in = unaligned_data,
		.len = sizeof(unaligned_data),
		.bit_swap = CRC_TRUE,
		.byte_swap = CRC_TRUE,
		.reflect = CRC_TRUE,
		.invert = CRC_TRUE,
		.custom_poly = CRC_FALSE,
		.data_out = &crc_hw,
	};

	zassert_equal(crc_set_seed(crc_dev, 0xFFFFFFFFU), 0, "seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "compute failed");

	crc_sw = crc32c_full_reference(unaligned_data, sizeof(unaligned_data));

	zassert_equal(crc_hw, crc_sw,
		      "CRC-32C full unaligned tail mismatch: HW=0x%X SW=0x%X",
		      crc_hw, crc_sw);
}

ZTEST(crc_32c_tests, test_stress_aligned_8_bytes)
{
	uint32_t crc_hw = 0;
	uint32_t crc_sw;
	uint8_t aligned_data[8] = {0x12, 0x34, 0x56, 0x78,
				   0x9A, 0xBC, 0xDE, 0xF0};
	struct crc_params params = {
		.data_in = aligned_data,
		.len = sizeof(aligned_data),
		.bit_swap = CRC_TRUE,
		.byte_swap = CRC_TRUE,
		.reflect = CRC_TRUE,
		.invert = CRC_TRUE,
		.custom_poly = CRC_FALSE,
		.data_out = &crc_hw,
	};

	zassert_equal(crc_set_seed(crc_dev, 0xFFFFFFFFU), 0, "seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "compute failed");

	crc_sw = crc32c_full_reference(aligned_data, sizeof(aligned_data));

	zassert_equal(crc_hw, crc_sw,
		      "CRC-32C full aligned tail mismatch: HW=0x%X SW=0x%X",
		      crc_hw, crc_sw);
}

ZTEST(crc_32c_tests, test_stress_aligned_4_bytes)
{
	uint32_t crc_hw = 0;
	uint32_t crc_sw;
	uint8_t aligned_data[4] = {0x12, 0x34, 0x56, 0x78};
	struct crc_params params = {
		.data_in = aligned_data,
		.len = sizeof(aligned_data),
		.bit_swap = CRC_TRUE,
		.byte_swap = CRC_TRUE,
		.reflect = CRC_TRUE,
		.invert = CRC_TRUE,
		.custom_poly = CRC_FALSE,
		.data_out = &crc_hw,
	};

	zassert_equal(crc_set_seed(crc_dev, 0xFFFFFFFFU), 0, "seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "compute failed");

	crc_sw = crc32c_full_reference(aligned_data, sizeof(aligned_data));

	zassert_equal(crc_hw, crc_sw,
		      "CRC-32C full aligned 4-byte mismatch: HW=0x%X SW=0x%X",
		      crc_hw, crc_sw);
}

ZTEST_SUITE(crc_32c_tests, NULL, NULL, crc_32c_before, NULL, NULL);

#endif /* CONFIG_TEST_CRC_32C */

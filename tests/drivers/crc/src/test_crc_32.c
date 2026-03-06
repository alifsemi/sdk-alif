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

#if IS_ENABLED(CONFIG_TEST_CRC_32)

static void crc_32_before(void *fixture)
{
	ARG_UNUSED(fixture);

	crc_test_verify_device();
	crc_test_force_hw_algo(TEST_CRC_HW_ALGO_CRC32);
}

/* CRC32_POLY_REFLECTED now defined in test_crc_common.h */

static void crc32_validate_native_mode(const uint8_t *data, size_t len,
				       uint32_t *crc_output)
{
	struct crc_params params = {
		.data_in = data,
		.len = len,
		.bit_swap = CRC_TRUE,
		.byte_swap = CRC_TRUE,
		.reflect = CRC_TRUE,
		.invert = CRC_TRUE,
		.custom_poly = CRC_FALSE,
		.data_out = crc_output,
	};
	uint32_t software_output;

	zassert_equal(crc_set_seed(crc_dev, 0xFFFFFFFFU), 0,
		      "CRC set seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "CRC compute failed");

	software_output = crc32_full_reference(data, len);
	zassert_equal(*crc_output, software_output,
		      "HW/SW CRC mismatch: 0x%X != 0x%X", *crc_output,
		      software_output);
}

/* ZTC-1377 / ZTC-1379: Standard 32-bit CRC (full mode: */
/* swap + reflect + invert) */
ZTEST(crc_32_tests, test_crc_32_bit)
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
	LOG_INF("32-bit CRC HW output: 0x%X", crc_output);

	software_output = crc32_full_reference((const uint8_t *)crc_test_data,
					       crc_test_data_len);
	LOG_INF("32-bit CRC SW output: 0x%X", software_output);

	zassert_equal(crc_output, software_output,
		      "HW/SW CRC mismatch: 0x%X != 0x%X", crc_output,
		      software_output);
}

/* ZTC-1187: 32-bit CRC with reflect output disabled */
ZTEST(crc_32_tests, test_crc_32_bit_reflect_output_disable)
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
	LOG_INF("32-bit CRC HW output: 0x%X", crc_output);

	software_output = crc32_driver_reference(
		(const uint8_t *)crc_test_data, crc_test_data_len, 0xFFFFFFFFU,
		true, true, false, true, CRC32_POLY_NORMAL,
		CRC32_POLY_REFLECTED);
	LOG_INF("32-bit CRC SW output (reflect disabled): 0x%X",
		software_output);

	zassert_equal(crc_output, software_output,
		      "HW/SW CRC mismatch: 0x%X != 0x%X", crc_output,
		      software_output);
}

/* ZTC-NEW: 32-bit CRC with unaligned length (5 bytes — */
/* exercises SW fallback) */
ZTEST(crc_32_tests, test_crc_32_bit_unaligned_5_bytes)
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
	LOG_INF("32-bit CRC HW 5-byte output: 0x%X", crc_output);

	software_output = crc32_full_reference(data_5, 5);
	LOG_INF("32-bit CRC SW 5-byte output: 0x%X", software_output);

	zassert_equal(crc_output, software_output,
		      "HW/SW CRC mismatch (5-byte unaligned): 0x%X != 0x%X",
		      crc_output, software_output);
}

/* ZTC-NEW: 32-bit CRC with unaligned length (7 bytes) */
ZTEST(crc_32_tests, test_crc_32_bit_unaligned_7_bytes)
{
	uint32_t crc_output, software_output;
	uint8_t data_7[] = {0x67, 0x3F, 0x90, 0xC9, 0x25, 0xF0, 0x4A};
	struct crc_params params = {
		.data_in = data_7,
		.len = 7,
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
	LOG_INF("32-bit CRC HW 7-byte output: 0x%X", crc_output);

	software_output = crc32_full_reference(data_7, 7);
	LOG_INF("32-bit CRC SW 7-byte output: 0x%X", software_output);

	zassert_equal(crc_output, software_output,
		      "HW/SW CRC mismatch (7-byte unaligned): 0x%X != 0x%X",
		      crc_output, software_output);
}

/* ZTC-NEW: 32-bit CRC with unaligned length (1 byte — all SW fallback) */
ZTEST(crc_32_tests, test_crc_32_bit_unaligned_1_byte)
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
	LOG_INF("32-bit CRC HW 1-byte output: 0x%X", crc_output);

	software_output = crc32_full_reference(data_1, 1);
	LOG_INF("32-bit CRC SW 1-byte output: 0x%X", software_output);

	zassert_equal(crc_output, software_output,
		      "HW/SW CRC mismatch (1-byte unaligned): 0x%X != 0x%X",
		      crc_output, software_output);
}

/* ZTC-NEW: 32-bit CRC with exactly 4-byte aligned length */
ZTEST(crc_32_tests, test_crc_32_bit_aligned_4_bytes)
{
	uint32_t crc_output, software_output;
	uint8_t data_4[] = {0x67, 0x3F, 0x90, 0xC9};
	struct crc_params params = {
		.data_in = data_4,
		.len = 4,
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
	LOG_INF("32-bit CRC HW 4-byte output: 0x%X", crc_output);

	software_output = crc32_full_reference(data_4, 4);
	LOG_INF("32-bit CRC SW 4-byte output: 0x%X", software_output);

	zassert_equal(crc_output, software_output,
		      "HW/SW CRC mismatch (4-byte aligned): 0x%X != 0x%X",
		      crc_output, software_output);
}

/* ZTC-NEW: 32-bit CRC with 8-byte aligned length */
ZTEST(crc_32_tests, test_crc_32_bit_aligned_8_bytes)
{
	uint32_t crc_output, software_output;
	uint8_t data_8[] = {0x67, 0x3F, 0x90, 0xC9, 0x25, 0xF0, 0x4A, 0xB1};
	struct crc_params params = {
		.data_in = data_8,
		.len = 8,
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
	LOG_INF("32-bit CRC HW 8-byte output: 0x%X", crc_output);

	software_output = crc32_full_reference(data_8, 8);
	LOG_INF("32-bit CRC SW 8-byte output: 0x%X", software_output);

	zassert_equal(crc_output, software_output,
		      "HW/SW CRC mismatch (8-byte aligned): 0x%X != 0x%X",
		      crc_output, software_output);
}

ZTEST(crc_32_tests, test_crc_32_bit_single_byte)
{
	uint32_t crc_output;
	uint32_t software_output;
	uint8_t single_byte[] = {0x55};
	struct crc_params params = {
		.data_in = single_byte,
		.len = ARRAY_SIZE(single_byte),
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

	software_output =
		crc32_full_reference(single_byte, ARRAY_SIZE(single_byte));
	zassert_equal(crc_output, software_output,
		      "HW/SW CRC mismatch: 0x%X != 0x%X", crc_output,
		      software_output);
}

ZTEST(crc_32_tests, test_crc_32_bit_misaligned_ptr_aligned_len)
{
	uint32_t crc_output = 0U;
	uint8_t raw_data[] = {0xA5, 0x67, 0x3F, 0x90, 0xC9,
			      0x25, 0xF0, 0x4A, 0xB1};
	const uint8_t *misaligned = &raw_data[1];

	zassert_true((((uintptr_t)misaligned) & 0x3U) != 0U,
		     "test buffer is unexpectedly aligned");

	crc32_validate_native_mode(misaligned, 8U, &crc_output);
}

ZTEST(crc_32_tests, test_crc_32_bit_misaligned_ptr_tail_len)
{
	uint32_t crc_output = 0U;
	uint8_t raw_data[] = {0x5A, 0x67, 0x3F, 0x90, 0xC9, 0x25, 0xF0, 0x4A};
	const uint8_t *misaligned = &raw_data[1];

	zassert_true((((uintptr_t)misaligned) & 0x3U) != 0U,
		     "test buffer is unexpectedly aligned");

	crc32_validate_native_mode(misaligned, 7U, &crc_output);
}

ZTEST(crc_32_tests, test_stress_vector_123456789)
{
	uint32_t crc_hw = 0;
	uint32_t crc_sw;
	struct crc_params params = {
		.data_in = crc_check_string,
		.len = CRC_CHECK_STRING_LEN,
		.bit_swap = CRC_TRUE,
		.byte_swap = CRC_TRUE,
		.reflect = CRC_TRUE,
		.invert = CRC_TRUE,
		.custom_poly = CRC_TRUE,
		.data_out = &crc_hw,
	};

	zassert_equal(crc_set_seed(crc_dev, 0xFFFFFFFFU), 0, "seed failed");
	zassert_equal(crc_set_polynomial(crc_dev, CRC32_POLY_NORMAL), 0,
		      "set CRC-32 polynomial failed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "compute failed");

	crc_sw = crc32_full_reference(crc_check_string, CRC_CHECK_STRING_LEN);

	zassert_equal(crc_hw, crc_sw,
		      "CRC-32 full known vector mismatch: HW=0x%X SW=0x%X",
		      crc_hw, crc_sw);
}

ZTEST(crc_32_tests, test_stress_32x_std_repeat)
{
	uint32_t crc_first = 0;
	uint32_t crc_repeat = 0;
	uint32_t crc_sw;
	struct crc_params params = {
		.data_in = (uint8_t *)crc_test_data,
		.len = crc_test_data_len,
		.bit_swap = CRC_TRUE,
		.byte_swap = CRC_TRUE,
		.reflect = CRC_TRUE,
		.invert = CRC_TRUE,
		.custom_poly = CRC_FALSE,
		.data_out = &crc_first,
	};

	zassert_equal(crc_set_seed(crc_dev, 0xFFFFFFFFU), 0, "seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "compute first failed");

	crc_sw = crc32_full_reference((const uint8_t *)crc_test_data,
				      crc_test_data_len);
	zassert_equal(crc_first, crc_sw,
		      "CRC-32 full HW/SW mismatch: HW=0x%X SW=0x%X", crc_first,
		      crc_sw);

	for (int i = 0; i < 50; i++) {
		params.data_out = &crc_repeat;
		zassert_equal(crc_set_seed(crc_dev, 0xFFFFFFFFU), 0,
			      "seed iter %d failed", i);
		zassert_equal(crc_compute(crc_dev, &params), 0,
			      "compute iter %d failed", i);
		zassert_equal(crc_first, crc_repeat,
			      "CRC-32 full repeatability iter %d: 0x%X != 0x%X",
			      i, crc_first, crc_repeat);
	}
}

ZTEST(crc_32_tests, test_stress_unaligned_7_bytes)
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
		.custom_poly = CRC_TRUE,
		.data_out = &crc_hw,
	};

	zassert_equal(crc_set_seed(crc_dev, 0xFFFFFFFFU), 0, "seed failed");
	zassert_equal(crc_set_polynomial(crc_dev, CRC32_POLY_NORMAL), 0,
		      "set CRC-32 polynomial failed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "compute failed");

	crc_sw = crc32_full_reference(unaligned_data, sizeof(unaligned_data));

	zassert_equal(crc_hw, crc_sw,
		      "CRC-32 full unaligned tail mismatch: HW=0x%X SW=0x%X",
		      crc_hw, crc_sw);
}

ZTEST_SUITE(crc_32_tests, NULL, NULL, crc_32_before, NULL, NULL);

#endif /* CONFIG_TEST_CRC_32 */

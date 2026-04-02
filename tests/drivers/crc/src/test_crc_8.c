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

#if IS_ENABLED(CONFIG_TEST_CRC_8)

static void crc_8_before(void *fixture)
{
	ARG_UNUSED(fixture);

	crc_test_verify_device();
	crc_test_force_hw_algo(TEST_CRC_HW_ALGO_CRC8_CCITT);
}

/* ZTC-1184: Basic 8-bit CRC */
ZTEST(crc_8_tests, test_crc_8_bit)
{
	uint32_t crc_output, software_output;
	struct crc_params params = {
		.data_in = (uint8_t *)crc_test_data,
		.len = crc_test_data_len,
		.bit_swap = CRC_FALSE,
		.byte_swap = CRC_FALSE,
		.reflect = CRC_FALSE,
		.invert = CRC_FALSE,
		.custom_poly = CRC_FALSE,
		.data_out = &crc_output,
	};

	zassert_equal(crc_set_seed(crc_dev, 0x00), 0, "CRC set seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "CRC compute failed");
	LOG_INF("8-bit CRC HW output: 0x%X", crc_output);

	software_output =
		gen_crc(CRC8_PRIME, (const unsigned char *)crc_test_data,
			crc_test_data_len);
	LOG_INF("8-bit CRC SW output: 0x%X", software_output);

	zassert_equal(crc_output, software_output,
		      "HW/SW CRC mismatch: 0x%X != 0x%X", crc_output,
		      software_output);
}

/* ZTC-1375: 8-bit CRC with reflected output */
ZTEST(crc_8_tests, test_crc_8_bit_reflect_output_en)
{
	uint32_t crc_output, software_output;
	struct crc_params params = {
		.data_in = (uint8_t *)crc_test_data,
		.len = crc_test_data_len,
		.bit_swap = CRC_FALSE,
		.byte_swap = CRC_FALSE,
		.reflect = CRC_TRUE,
		.invert = CRC_FALSE,
		.custom_poly = CRC_FALSE,
		.data_out = &crc_output,
	};

	zassert_equal(crc_set_seed(crc_dev, 0x00), 0, "CRC set seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "CRC compute failed");
	LOG_INF("8-bit CRC HW output: 0x%X", crc_output);

	software_output =
		gen_crc(CRC8_PRIME, (const unsigned char *)crc_test_data,
			crc_test_data_len);
	software_output = bit_reflect_n(software_output, 8);
	LOG_INF("8-bit CRC SW output (reflected): 0x%X", software_output);

	zassert_equal(crc_output, software_output,
		      "HW/SW CRC mismatch: 0x%X != 0x%X", crc_output,
		      software_output);
}

/* ZTC-1390: 8-bit CRC with bit/byte swap */
ZTEST(crc_8_tests, test_crc_8_bit_swap_en)
{
	uint32_t crc_output, software_output;
	uint8_t input[] = {0x67, 0x37};
	uint8_t reflected_input[] = {0xE6, 0xEC};
	struct crc_params params = {
		.data_in = (uint8_t *)input,
		.len = ARRAY_SIZE(input),
		.bit_swap = CRC_TRUE,
		.byte_swap = CRC_TRUE,
		.reflect = CRC_FALSE,
		.invert = CRC_FALSE,
		.custom_poly = CRC_FALSE,
		.data_out = &crc_output,
	};

	zassert_equal(crc_set_seed(crc_dev, 0x00), 0, "CRC set seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "CRC compute failed");
	LOG_INF("8-bit CRC HW output: 0x%X", crc_output);

	software_output =
		gen_crc(CRC8_PRIME, reflected_input, sizeof(reflected_input));
	LOG_INF("8-bit CRC SW output (swapped): 0x%X", software_output);

	zassert_equal(crc_output, software_output,
		      "HW/SW CRC mismatch: 0x%X != 0x%X", crc_output,
		      software_output);
}

/* ZTC-1392: 8-bit CRC with inverted output */
ZTEST(crc_8_tests, test_crc_8_bit_invert_output_en)
{
	uint32_t crc_output, software_output;
	struct crc_params params = {
		.data_in = (uint8_t *)crc_test_data,
		.len = crc_test_data_len,
		.bit_swap = CRC_FALSE,
		.byte_swap = CRC_FALSE,
		.reflect = CRC_FALSE,
		.invert = CRC_TRUE,
		.custom_poly = CRC_FALSE,
		.data_out = &crc_output,
	};

	zassert_equal(crc_set_seed(crc_dev, 0x00), 0, "CRC set seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "CRC compute failed");
	LOG_INF("8-bit CRC HW output: 0x%X", crc_output);

	software_output =
		gen_crc(CRC8_PRIME, (const unsigned char *)crc_test_data,
			crc_test_data_len);
	software_output = (software_output ^ 0xFF);
	LOG_INF("8-bit CRC SW output (inverted): 0x%X", software_output);

	crc_output = (crc_output & 0xFF);

	zassert_equal(crc_output, software_output,
		      "HW/SW CRC mismatch: 0x%X != 0x%X", crc_output,
		      software_output);
}

/* ZTC-NEW: 8-bit CRC with single byte input */
ZTEST(crc_8_tests, test_crc_8_bit_single_byte)
{
	uint32_t crc_output, software_output;
	uint8_t single_byte[] = {0x55};
	struct crc_params params = {
		.data_in = single_byte,
		.len = 1,
		.bit_swap = CRC_FALSE,
		.byte_swap = CRC_FALSE,
		.reflect = CRC_FALSE,
		.invert = CRC_FALSE,
		.custom_poly = CRC_FALSE,
		.data_out = &crc_output,
	};

	zassert_equal(crc_set_seed(crc_dev, 0x00), 0, "CRC set seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "CRC compute failed");
	LOG_INF("8-bit CRC HW single byte output: 0x%X", crc_output);

	software_output = gen_crc(CRC8_PRIME, single_byte, 1);
	LOG_INF("8-bit CRC SW single byte output: 0x%X", software_output);

	zassert_equal((crc_output & 0xFF), software_output,
		      "HW/SW CRC mismatch: 0x%X != 0x%X", (crc_output & 0xFF),
		      software_output);
}

/* ZTC-NEW: 8-bit CRC with non-zero seed */
ZTEST(crc_8_tests, test_crc_8_bit_nonzero_seed)
{
	uint32_t crc_seed0 = 0, crc_seedFF = 0;
	struct crc_params params = {
		.data_in = (uint8_t *)crc_test_data,
		.len = crc_test_data_len,
		.bit_swap = CRC_FALSE,
		.byte_swap = CRC_FALSE,
		.reflect = CRC_FALSE,
		.invert = CRC_FALSE,
		.custom_poly = CRC_FALSE,
		.data_out = &crc_seed0,
	};

	zassert_equal(crc_set_seed(crc_dev, 0x00), 0, "seed 0 failed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "compute seed 0");

	params.data_out = &crc_seedFF;
	zassert_equal(crc_set_seed(crc_dev, 0xFF), 0, "seed FF failed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "compute seed FF");

	LOG_INF("8-bit CRC seed=0x00: 0x%X, seed=0xFF: 0x%X", crc_seed0,
		crc_seedFF);

	zassert_not_equal((crc_seed0 & 0xFF), (crc_seedFF & 0xFF),
			  "Different seeds produced same 8-bit CRC: 0x%X",
			  (crc_seed0 & 0xFF));
}

/* ZTC-NEW: 8-bit CRC with reflect + invert combined */
ZTEST(crc_8_tests, test_crc_8_bit_reflect_invert_combined)
{
	uint32_t crc_output, software_output;
	struct crc_params params = {
		.data_in = (uint8_t *)crc_test_data,
		.len = crc_test_data_len,
		.bit_swap = CRC_FALSE,
		.byte_swap = CRC_FALSE,
		.reflect = CRC_TRUE,
		.invert = CRC_TRUE,
		.custom_poly = CRC_FALSE,
		.data_out = &crc_output,
	};

	zassert_equal(crc_set_seed(crc_dev, 0x00), 0, "CRC set seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "CRC compute failed");
	LOG_INF("8-bit CRC HW reflect+invert output: 0x%X", crc_output);

	software_output =
		gen_crc(CRC8_PRIME, (const unsigned char *)crc_test_data,
			crc_test_data_len);
	software_output = bit_reflect_n(software_output, 8);
	software_output = (software_output ^ 0xFF);
	LOG_INF("8-bit CRC SW reflect+invert output: 0x%X", software_output);

	zassert_equal((crc_output & 0xFF), software_output,
		      "HW/SW CRC mismatch: 0x%X != 0x%X", (crc_output & 0xFF),
		      software_output);
}

ZTEST_SUITE(crc_8_tests, NULL, NULL, crc_8_before, NULL, NULL);

#endif /* CONFIG_TEST_CRC_8 */

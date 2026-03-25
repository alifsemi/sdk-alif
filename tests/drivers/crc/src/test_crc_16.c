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

#if IS_ENABLED(CONFIG_TEST_CRC_16)

static void crc_16_before(void *fixture)
{
	ARG_UNUSED(fixture);

	crc_test_verify_device();
	crc_test_force_hw_algo(TEST_CRC_HW_ALGO_CRC16_CCITT);
}

/* ZTC-1185 / ZTC-1186: Basic 16-bit CRC */
ZTEST(crc_16_tests, test_crc_16_bit)
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
	LOG_INF("16-bit CRC HW output: 0x%X", crc_output);

	software_output =
		gen_crc(CRC16_802_15_4, (const unsigned char *)crc_test_data,
			crc_test_data_len);
	LOG_INF("16-bit CRC SW output: 0x%X", software_output);

	zassert_equal(crc_output, software_output,
		      "HW/SW CRC mismatch: 0x%X != 0x%X", crc_output,
		      software_output);
}

/* ZTC-1376: 16-bit CRC with reflected output */
ZTEST(crc_16_tests, test_crc_16_bit_reflect_output_en)
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
	LOG_INF("16-bit CRC HW output: 0x%X", crc_output);

	software_output =
		gen_crc(CRC16_802_15_4, (const unsigned char *)crc_test_data,
			crc_test_data_len);
	software_output = bit_reflect_n(software_output, 16);
	LOG_INF("16-bit CRC SW output (reflected): 0x%X", software_output);

	zassert_equal(crc_output, software_output,
		      "HW/SW CRC mismatch: 0x%X != 0x%X", crc_output,
		      software_output);
}

/* ZTC-1391: 16-bit CRC with bit/byte swap */
ZTEST(crc_16_tests, test_crc_16_bit_swap_en)
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
	LOG_INF("16-bit CRC HW output: 0x%X", crc_output);

	software_output = gen_crc(CRC16_802_15_4, reflected_input,
				  sizeof(reflected_input));
	LOG_INF("16-bit CRC SW output (swapped): 0x%X", software_output);

	zassert_equal(crc_output, software_output,
		      "HW/SW CRC mismatch: 0x%X != 0x%X", crc_output,
		      software_output);
}

/* ZTC-1393: 16-bit CRC with inverted output */
ZTEST(crc_16_tests, test_crc_16_bit_invert_output)
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
	LOG_INF("16-bit CRC HW output: 0x%X", crc_output);

	software_output =
		gen_crc(CRC16_802_15_4, (const unsigned char *)crc_test_data,
			crc_test_data_len);
	software_output = (software_output ^ 0xFFFF);
	LOG_INF("16-bit CRC SW output (inverted): 0x%X", software_output);

	crc_output = (crc_output & 0xFFFF);

	zassert_equal(crc_output, software_output,
		      "HW/SW CRC mismatch: 0x%X != 0x%X", crc_output,
		      software_output);
}

/* ZTC-NEW: 16-bit CRC with single byte input */
ZTEST(crc_16_tests, test_crc_16_bit_single_byte)
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
	LOG_INF("16-bit CRC HW single byte output: 0x%X", crc_output);

	software_output = gen_crc(CRC16_802_15_4, single_byte, 1);
	LOG_INF("16-bit CRC SW single byte output: 0x%X", software_output);

	zassert_equal((crc_output & 0xFFFF), software_output,
		      "HW/SW CRC mismatch: 0x%X != 0x%X", (crc_output & 0xFFFF),
		      software_output);
}

/* ZTC-NEW: 16-bit CRC with non-zero seed */
ZTEST(crc_16_tests, test_crc_16_bit_nonzero_seed)
{
	uint32_t crc_seed0 = 0, crc_seedFFFF = 0;
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

	zassert_equal(crc_set_seed(crc_dev, 0x0000), 0, "seed 0 failed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "compute seed 0");

	params.data_out = &crc_seedFFFF;
	zassert_equal(crc_set_seed(crc_dev, 0xFFFF), 0, "seed FFFF failed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "compute seed FFFF");

	LOG_INF("16-bit CRC seed=0x0000: 0x%X, seed=0xFFFF: 0x%X", crc_seed0,
		crc_seedFFFF);

	zassert_not_equal((crc_seed0 & 0xFFFF), (crc_seedFFFF & 0xFFFF),
			  "Different seeds produced same 16-bit CRC: 0x%X",
			  (crc_seed0 & 0xFFFF));
}

/* ZTC-NEW: 16-bit CRC with reflect + invert combined */
ZTEST(crc_16_tests, test_crc_16_bit_reflect_invert_combined)
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
	LOG_INF("16-bit CRC HW reflect+invert output: 0x%X", crc_output);

	software_output =
		gen_crc(CRC16_802_15_4, (const unsigned char *)crc_test_data,
			crc_test_data_len);
	software_output = bit_reflect_n(software_output, 16);
	software_output = (software_output ^ 0xFFFF);
	LOG_INF("16-bit CRC SW reflect+invert output: 0x%X", software_output);

	zassert_equal((crc_output & 0xFFFF), software_output,
		      "HW/SW CRC mismatch: 0x%X != 0x%X", (crc_output & 0xFFFF),
		      software_output);
}

ZTEST_SUITE(crc_16_tests, NULL, NULL, crc_16_before, NULL, NULL);

#endif /* CONFIG_TEST_CRC_16 */

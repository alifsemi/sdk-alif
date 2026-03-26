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

#if IS_ENABLED(CONFIG_TEST_CRC_16_ALT)

static void crc_16_alt_before(void *fixture)
{
	ARG_UNUSED(fixture);

	crc_test_verify_device();
	crc_test_force_hw_algo(TEST_CRC_HW_ALGO_CRC16);
}

static uint32_t crc16_alt_sw(const uint8_t *data, size_t len)
{
	return gen_crc(CRC16_ALT, data, len);
}

ZTEST(crc_16_alt_tests, test_crc_16_alt)
{
	uint32_t crc_output;
	uint32_t software_output;
	struct crc_params params = {
		.data_in = (uint8_t *)crc_test_data,
		.len = crc_test_data_len,
		.bit_swap = CRC_FALSE,
		.byte_swap = CRC_FALSE,
		.reflect = CRC_FALSE,
		.invert = CRC_FALSE,
		.custom_poly = CRC_FALSE, /* Use native hardware support */
		.data_out = &crc_output,
	};

	zassert_equal(crc_set_seed(crc_dev, 0x0000), 0, "CRC set seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "CRC compute failed");

	software_output = crc16_alt_sw(crc_test_data, crc_test_data_len);
	LOG_INF("16-bit ALT HW output: 0x%X", crc_output);
	LOG_INF("16-bit ALT SW output: 0x%X", software_output);

	zassert_equal(crc_output, software_output,
		      "HW/SW CRC mismatch: 0x%X != 0x%X", crc_output,
		      software_output);
}

ZTEST(crc_16_alt_tests, test_crc_16_alt_reflect_output_en)
{
	uint32_t crc_output;
	uint32_t software_output;
	struct crc_params params = {
		.data_in = (uint8_t *)crc_test_data,
		.len = crc_test_data_len,
		.bit_swap = CRC_FALSE,
		.byte_swap = CRC_FALSE,
		.reflect = CRC_TRUE,
		.invert = CRC_FALSE,
		.custom_poly = CRC_FALSE, /* Use native hardware support */
		.data_out = &crc_output,
	};

	zassert_equal(crc_set_seed(crc_dev, 0x0000), 0, "CRC set seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "CRC compute failed");

	software_output = crc16_alt_sw(crc_test_data, crc_test_data_len);
	software_output = bit_reflect_n(software_output, 16);
	zassert_equal(crc_output & 0xFFFF, software_output,
		      "HW/SW CRC mismatch: 0x%X != 0x%X", crc_output & 0xFFFF,
		      software_output);
}

ZTEST(crc_16_alt_tests, test_crc_16_alt_bit_swap_en)
{
	uint32_t crc_output;
	uint32_t software_output;
	uint8_t input[] = {0x67, 0x37};
	uint8_t reflected_input[] = {0xE6, 0xEC};
	struct crc_params params = {
		.data_in = input,
		.len = ARRAY_SIZE(input),
		.bit_swap = CRC_TRUE,
		.byte_swap = CRC_TRUE,
		.reflect = CRC_FALSE,
		.invert = CRC_FALSE,
		.custom_poly = CRC_FALSE, /* Use native hardware support */
		.data_out = &crc_output,
	};

	zassert_equal(crc_set_seed(crc_dev, 0x0000), 0, "CRC set seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "CRC compute failed");

	software_output =
		crc16_alt_sw(reflected_input, ARRAY_SIZE(reflected_input));
	zassert_equal(crc_output & 0xFFFF, software_output,
		      "HW/SW CRC mismatch: 0x%X != 0x%X", crc_output & 0xFFFF,
		      software_output);
}

ZTEST(crc_16_alt_tests, test_crc_16_alt_invert_output)
{
	uint32_t crc_output;
	uint32_t software_output;
	struct crc_params params = {
		.data_in = (uint8_t *)crc_test_data,
		.len = crc_test_data_len,
		.bit_swap = CRC_FALSE,
		.byte_swap = CRC_FALSE,
		.reflect = CRC_FALSE,
		.invert = CRC_TRUE,
		.custom_poly = CRC_FALSE, /* Use native hardware support */
		.data_out = &crc_output,
	};

	zassert_equal(crc_set_seed(crc_dev, 0x0000), 0, "CRC set seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "CRC compute failed");

	software_output =
		crc16_alt_sw(crc_test_data, crc_test_data_len) ^ 0xFFFFU;
	zassert_equal(crc_output & 0xFFFF, software_output,
		      "HW/SW CRC mismatch: 0x%X != 0x%X", crc_output & 0xFFFF,
		      software_output);
}

ZTEST(crc_16_alt_tests, test_crc_16_alt_single_byte)
{
	uint32_t crc_output;
	uint32_t software_output;
	uint8_t single_byte[] = {0x55};
	struct crc_params params = {
		.data_in = single_byte,
		.len = ARRAY_SIZE(single_byte),
		.bit_swap = CRC_FALSE,
		.byte_swap = CRC_FALSE,
		.reflect = CRC_FALSE,
		.invert = CRC_FALSE,
		.custom_poly = CRC_FALSE, /* Use native hardware support */
		.data_out = &crc_output,
	};

	zassert_equal(crc_set_seed(crc_dev, 0x0000), 0, "CRC set seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "CRC compute failed");

	software_output = crc16_alt_sw(single_byte, ARRAY_SIZE(single_byte));
	zassert_equal(crc_output & 0xFFFF, software_output,
		      "HW/SW CRC mismatch: 0x%X != 0x%X", crc_output & 0xFFFF,
		      software_output);
}

ZTEST(crc_16_alt_tests, test_crc_16_alt_nonzero_seed)
{
	uint32_t crc_seed0 = 0;
	uint32_t crc_seedffff = 0;
	struct crc_params params = {
		.data_in = (uint8_t *)crc_test_data,
		.len = crc_test_data_len,
		.bit_swap = CRC_FALSE,
		.byte_swap = CRC_FALSE,
		.reflect = CRC_FALSE,
		.invert = CRC_FALSE,
		.custom_poly = CRC_FALSE, /* Use native hardware support */
		.data_out = &crc_seed0,
	};

	zassert_equal(crc_set_seed(crc_dev, 0x0000), 0, "seed 0 failed");
	zassert_equal(crc_compute(crc_dev, &params), 0,
		      "compute seed 0 failed");

	params.data_out = &crc_seedffff;
	zassert_equal(crc_set_seed(crc_dev, 0xFFFF), 0, "seed FFFF failed");
	zassert_equal(crc_compute(crc_dev, &params), 0,
		      "compute seed FFFF failed");

	zassert_not_equal(crc_seed0 & 0xFFFF, crc_seedffff & 0xFFFF,
			  "Different seeds produced same CRC16_ALT: 0x%X",
			  crc_seed0 & 0xFFFF);
}

ZTEST(crc_16_alt_tests, test_crc_16_alt_reflect_invert_combined)
{
	uint32_t crc_output;
	uint32_t software_output;
	struct crc_params params = {
		.data_in = (uint8_t *)crc_test_data,
		.len = crc_test_data_len,
		.bit_swap = CRC_FALSE,
		.byte_swap = CRC_FALSE,
		.reflect = CRC_TRUE,
		.invert = CRC_TRUE,
		.custom_poly = CRC_FALSE, /* Use native hardware support */
		.data_out = &crc_output,
	};

	zassert_equal(crc_set_seed(crc_dev, 0x0000), 0, "CRC set seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "CRC compute failed");

	software_output = crc16_alt_sw(crc_test_data, crc_test_data_len);
	software_output = bit_reflect_n(software_output, 16);
	software_output = (software_output ^ 0xFFFF);
	zassert_equal(crc_output & 0xFFFF, software_output,
		      "HW/SW CRC mismatch: 0x%X != 0x%X", crc_output & 0xFFFF,
		      software_output);
}

ZTEST_SUITE(crc_16_alt_tests, NULL, NULL, crc_16_alt_before, NULL, NULL);

#endif /* CONFIG_TEST_CRC_16_ALT */

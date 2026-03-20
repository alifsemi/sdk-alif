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

LOG_MODULE_REGISTER(alif_crc_test);

const uint8_t crc_test_data[] = {0x67, 0x3F, 0x90, 0xC9, 0x25,
				 0xF0, 0x4A, 0xB1, 0x13};

const size_t crc_test_data_len = ARRAY_SIZE(crc_test_data);

const uint8_t crc_check_string[] = {'1', '2', '3', '4', '5',
				    '6', '7', '8', '9'};

const struct device *const crc_dev = DEVICE_DT_GET(DT_ALIAS(crc));

/*
 * Mirror the first field of the driver-private runtime data so the tests can
 * steer the active CRC algorithm without changing the public driver API.
 *
 * This keeps multi-algorithm validation in a single image possible while the
 * driver still selects its algorithm from devicetree at init time.
 */
struct crc_test_runtime_data {
	uint8_t crc_algo;
};

void crc_test_force_hw_algo(enum test_crc_hw_algo algo)
{
	struct crc_test_runtime_data *data =
		(struct crc_test_runtime_data *)crc_dev->data;

	zassert_not_null(data, "CRC driver data not available");
	zassert_true(algo <= TEST_CRC_HW_ALGO_CRC32C, "invalid CRC algo %u",
		     (unsigned int)algo);

	data->crc_algo = (uint8_t)algo;
}

uint32_t bit_reflect_n(uint32_t input, uint32_t width)
{
	uint32_t reflected = 0U;

	for (uint32_t i = 0U; i < width; i++) {
		if (input & BIT(i)) {
			reflected |= BIT(width - 1U - i);
		}
	}

	return reflected;
}

uint32_t bit_reflect_32(uint32_t input) { return bit_reflect_n(input, 32U); }

void crc_test_verify_device(void)
{
	zassert_true(crc_dev != NULL, "CRC device not found");
	zassert_true(device_is_ready(crc_dev), "CRC device not ready");
	LOG_DBG("CRC device ready: %s", crc_dev->name);
}

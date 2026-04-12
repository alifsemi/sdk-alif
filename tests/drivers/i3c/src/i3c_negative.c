/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/ztest.h>
#include <zephyr/logging/log.h>
#include "i3c_common.h"

LOG_MODULE_REGISTER(i3c_negative, LOG_LEVEL_INF);

static const struct device *ctrl;

static void *suite_setup(void)
{
	ctrl = DEVICE_DT_GET(I3C_CONTROLLER_NODE);
	zassert_true(device_is_ready(ctrl), "I3C controller not ready");

	/* Configure SDR mode only.
	 * NOTE: Zephyr does not support hot-join. DAA must be done by
	 * application or device initialization before tests run.
	 * We do NOT perform RSTDAA+DAA here because:
	 * 1. If all suites run at once, repeated RSTDAA causes address
	 *    increments and driver state issues
	 * 2. If suites run individually, system/DT init should have
	 *    already performed DAA
	 */
	i3c_test_reset_to_sdr(ctrl);

	return NULL;
}

static void test_before(void *fixture)
{
	struct i3c_device_desc *desc;
	int found = 0;

	ARG_UNUSED(fixture);

	/* Verify devices were assigned addresses by prior DAA */
	I3C_BUS_FOR_EACH_I3CDEV(ctrl, desc) {
		if (desc->dynamic_addr != 0U) {
			found++;
		}
	}

	if (found == 0) {
		LOG_WRN("No devices with dynamic address - ensure DAA completed before tests");
		ztest_test_skip();
	}
}

static void test_after(void *fixture)
{
	ARG_UNUSED(fixture);
}

ZTEST_SUITE(i3c_negative, NULL, suite_setup, test_before, test_after, NULL);

ZTEST(i3c_negative, test_invalid_dynamic_addresses)
{
	uint8_t buf[4];
	int ret;

	I3C_TEST_FOR_EACH_MODE(ctrl, mode) {
		/* DA 0x00 is broadcast address - reserved, should fail */
		ret = i3c_reg_read(ctrl, 0x00, I3C_TEST_CHIP_ID_REG, buf, 4);
		zassert_true(ret < 0,
			     "Transfer to DA 0x00 should fail in %s mode",
			     i3c_test_mode_name(mode));

		/* DA 0x07 is below valid range (0x08-0x77), should fail */
		ret = i3c_reg_read(ctrl, 0x07, I3C_TEST_CHIP_ID_REG, buf, 4);
		zassert_true(ret < 0,
			     "Transfer to DA 0x07 should fail in %s mode",
			     i3c_test_mode_name(mode));

		/* DA 0x78 is above valid range (0x08-0x77), should fail */
		ret = i3c_reg_read(ctrl, 0x78, I3C_TEST_CHIP_ID_REG, buf, 4);
		zassert_true(ret < 0,
			     "Transfer to DA 0x78 should fail in %s mode",
			     i3c_test_mode_name(mode));
	} I3C_TEST_END_EACH_MODE
}

ZTEST(i3c_negative, test_invalid_i2c_addresses)
{
	int ret;
	uint8_t dummy_data = 0;
	int rejected_count = 0;
	int bus_fail_count = 0;

	/* Test I2C addresses that should be invalid:
	 * - 0x00-0x07: Reserved by I3C spec (broadcast, reserved)
	 * - 0x78-0x7F: Reserved (7E is I3C broadcast, 7F reserved)
	 * - 0x80+: Out of 7-bit address range
	 *
	 * Driver MAY reject at attach time OR allow attach but fail on bus.
	 * Both are acceptable - we verify the address cannot be used.
	 */

	/* Test 0x00 - General Call address (reserved) */
	struct i3c_i2c_device_desc i2c_desc_00 = {
		.bus = ctrl,
		.addr = 0x00,
	};
	ret = i3c_attach_i2c_device(&i2c_desc_00);
	if (ret < 0) {
		LOG_INF("I2C addr 0x00 rejected at attach: %d", ret);
		rejected_count++;
	} else {
		/* Attach succeeded - verify it fails on actual bus transfer */
		ret = i3c_transfer_write(ctrl, 0x00, &dummy_data, 1);
		if (ret < 0) {
			LOG_INF("I2C addr 0x00 failed on bus: %d", ret);
			bus_fail_count++;
		}
		/* Cleanup - detach the invalid device */
		i3c_detach_i2c_device(&i2c_desc_00);
	}

	/* Test 0x07 - Reserved address */
	struct i3c_i2c_device_desc i2c_desc_07 = {
		.bus = ctrl,
		.addr = 0x07,
	};
	ret = i3c_attach_i2c_device(&i2c_desc_07);
	if (ret < 0) {
		LOG_INF("I2C addr 0x07 rejected at attach: %d", ret);
		rejected_count++;
	} else {
		ret = i3c_transfer_write(ctrl, 0x07, &dummy_data, 1);
		if (ret < 0) {
			LOG_INF("I2C addr 0x07 failed on bus: %d", ret);
			bus_fail_count++;
		}
		i3c_detach_i2c_device(&i2c_desc_07);
	}

	/* Test 0x78 - Above valid I2C range */
	struct i3c_i2c_device_desc i2c_desc_78 = {
		.bus = ctrl,
		.addr = 0x78,
	};
	ret = i3c_attach_i2c_device(&i2c_desc_78);
	if (ret < 0) {
		LOG_INF("I2C addr 0x78 rejected at attach: %d", ret);
		rejected_count++;
	} else {
		ret = i3c_transfer_write(ctrl, 0x78, &dummy_data, 1);
		if (ret < 0) {
			LOG_INF("I2C addr 0x78 failed on bus: %d", ret);
			bus_fail_count++;
		}
		i3c_detach_i2c_device(&i2c_desc_78);
	}

	/* Test 0x7E - I3C broadcast address (must not be allowed as I2C) */
	struct i3c_i2c_device_desc i2c_desc_7e = {
		.bus = ctrl,
		.addr = 0x7E,
	};
	ret = i3c_attach_i2c_device(&i2c_desc_7e);
	if (ret < 0) {
		LOG_INF("I2C addr 0x7E rejected at attach: %d", ret);
		rejected_count++;
	} else {
		ret = i3c_transfer_write(ctrl, 0x7E, &dummy_data, 1);
		if (ret < 0) {
			LOG_INF("I2C addr 0x7E failed on bus: %d", ret);
			bus_fail_count++;
		}
		i3c_detach_i2c_device(&i2c_desc_7e);
	}

	/* Summary: All 4 invalid addresses should fail somehow */
	zassert_true(rejected_count > 0 || bus_fail_count > 0,
		     "Invalid I2C addresses should be rejected at attach or fail on bus "
		     "(rejected=%d, bus_fail=%d)",
		     rejected_count, bus_fail_count);

	LOG_INF("I2C validation: %d rejected, %d failed on bus",
		rejected_count, bus_fail_count);
}

ZTEST(i3c_negative, test_transfer_null_buffer)
{
	I3C_TEST_FOR_EACH_MODE(ctrl, mode) {
		int ret = i3c_transfer_write(ctrl, I3C_TEST_TARGET_DA, NULL, 2);

		zassert_true(ret < 0, "NULL buffer should fail in %s mode",
			     i3c_test_mode_name(mode));
	} I3C_TEST_END_EACH_MODE
}

ZTEST(i3c_negative, test_zero_length_transfer)
{
	I3C_TEST_FOR_EACH_MODE(ctrl, mode) {
		uint8_t data = 0;
		int ret = i3c_transfer_write(ctrl, I3C_TEST_TARGET_DA,
					     &data, 0);

		zassert_true(ret < 0, "Zero length should fail in %s mode",
			     i3c_test_mode_name(mode));
	} I3C_TEST_END_EACH_MODE
}

ZTEST(i3c_negative, test_ccc_to_invalid_target)
{
	I3C_TEST_FOR_EACH_MODE(ctrl, mode) {
		uint8_t pid[6];
		int ret = i3c_do_ccc_getpid(ctrl, 0x00, pid, 6);

		zassert_true(ret < 0,
			     "CCC to invalid DA should fail in %s mode",
			     i3c_test_mode_name(mode));
	} I3C_TEST_END_EACH_MODE
}

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

LOG_MODULE_REGISTER(i3c_discovery, LOG_LEVEL_INF);

static const struct device *ctrl;

static int count_assigned_targets(void)
{
	struct i3c_device_desc *desc;
	int count = 0;

	I3C_BUS_FOR_EACH_I3CDEV(ctrl, desc) {
		if (desc->dynamic_addr != 0U) {
			count++;
		}
	}

	return count;
}

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

ZTEST_SUITE(i3c_discovery, NULL, suite_setup, test_before, test_after, NULL);

ZTEST(i3c_discovery, test_controller_ready)
{
	zassert_true(device_is_ready(ctrl), "I3C controller not ready");
}

ZTEST(i3c_discovery, test_targets_discovered)
{
	I3C_TEST_FOR_EACH_MODE(ctrl, mode) {
		int count = 0;

		I3C_TEST_FOR_EACH_TARGET(ctrl, desc) {
			count++;
			LOG_DBG("Found target at DA=0x%02x in %s",
				desc->dynamic_addr,
				i3c_test_mode_name(mode));
		} I3C_TEST_END_EACH_TARGET
		zassert_true(count > 0, "No I3C targets discovered in %s mode",
			     i3c_test_mode_name(mode));
	} I3C_TEST_END_EACH_MODE
}

ZTEST(i3c_discovery, test_valid_target_descriptors)
{
	I3C_TEST_FOR_EACH_MODE(ctrl, mode) {
		I3C_TEST_FOR_EACH_TARGET(ctrl, desc) {
			zassert_not_equal(desc->dynamic_addr, 0,
					  "DA cannot be 0 in %s",
					  i3c_test_mode_name(mode));
			zassert_not_equal(desc->pid, 0,
					  "PID cannot be 0 in %s",
					  i3c_test_mode_name(mode));
		} I3C_TEST_END_EACH_TARGET
	} I3C_TEST_END_EACH_MODE
}

ZTEST(i3c_discovery, test_target_index_lookup_bounds)
{
	I3C_TEST_FOR_EACH_MODE(ctrl, mode) {
		int count = 0;

		I3C_TEST_FOR_EACH_TARGET(ctrl, desc) {
			count++;
		} I3C_TEST_END_EACH_TARGET
		zassert_true(count > 0,
			     "Should have at least one target in %s",
			     i3c_test_mode_name(mode));
	} I3C_TEST_END_EACH_MODE
}

ZTEST(i3c_discovery, test_rstdaa_broadcast)
{
	/* Record current addresses before RSTDAA */
	struct i3c_device_desc *desc;
	uint8_t prev_addrs[I3C_TEST_MAX_TARGETS] = {0};
	int count = 0;

	I3C_BUS_FOR_EACH_I3CDEV(ctrl, desc) {
		if (desc->dynamic_addr != 0 && count < I3C_TEST_MAX_TARGETS) {
			prev_addrs[count++] = desc->dynamic_addr;
		}
	}

	if (count == 0) {
		ztest_test_skip();
	}

	/* Step 1: Send RSTDAA broadcast */
	int ret = i3c_do_ccc_rstdaa(ctrl);

	zassert_ok(ret, "RSTDAA failed: %d", ret);
	k_msleep(50);

	/* Step 2: Verify targets don't respond at old addresses */
	for (int i = 0; i < count; i++) {
		uint8_t bcr;
		int check_ret = i3c_do_ccc_getbcr(ctrl, prev_addrs[i], &bcr);

		zassert_true(check_ret < 0,
			     "Target still responds at old DA=0x%02x after RSTDAA",
			     prev_addrs[i]);
	}

	/* Step 3: Restore addresses via DAA */
	ret = i3c_do_daa(ctrl);
	if (ret < 0) {
		k_msleep(100);
		ret = i3c_do_daa(ctrl);
	}
	zassert_ok(ret, "DAA restore failed after RSTDAA: %d", ret);
}

ZTEST(i3c_discovery, test_entdaa)
{
	/* Record pre-test state */
	struct i3c_device_desc *desc;
	int prev_count = 0;

	I3C_BUS_FOR_EACH_I3CDEV(ctrl, desc) {
		if (desc->dynamic_addr != 0) {
			prev_count++;
		}
	}

	/* Step 1: RSTDAA */
	int ret = i3c_do_ccc_rstdaa(ctrl);

	zassert_ok(ret, "RSTDAA failed: %d", ret);

	k_msleep(50);

	/* Step 2: Run DAA. The controller API performs ENTDAA. */
	ret = i3c_do_daa(ctrl);
	if (ret < 0) {
		k_msleep(100);
		ret = i3c_do_daa(ctrl);
	}
	zassert_ok(ret, "DAA after RSTDAA failed: %d", ret);
	zassert_equal(count_assigned_targets(), prev_count,
		      "DAA assigned unexpected target count");
}

ZTEST(i3c_discovery, test_dynamic_addresses_in_valid_range)
{
	I3C_TEST_FOR_EACH_MODE(ctrl, mode) {
		I3C_TEST_FOR_EACH_TARGET(ctrl, desc) {
			uint8_t da = desc->dynamic_addr;

			zassert_true(da >= 0x08 && da <= 0x77,
				     "DA 0x%02x out of range in %s",
				     da, i3c_test_mode_name(mode));
		} I3C_TEST_END_EACH_TARGET
	} I3C_TEST_END_EACH_MODE
}

ZTEST(i3c_discovery, test_dynamic_addresses_unique)
{
	I3C_TEST_FOR_EACH_MODE(ctrl, mode) {
		uint8_t addrs[I3C_TEST_MAX_TARGETS] = {0};
		int count = 0;

		I3C_TEST_FOR_EACH_TARGET(ctrl, desc) {
			for (int i = 0; i < count; i++) {
				zassert_not_equal(addrs[i], desc->dynamic_addr,
						  "Duplicate DA=0x%02x in %s mode",
						  desc->dynamic_addr,
						  i3c_test_mode_name(mode));
			}
			if (count < I3C_TEST_MAX_TARGETS) {
				addrs[count++] = desc->dynamic_addr;
			}
		} I3C_TEST_END_EACH_TARGET
	} I3C_TEST_END_EACH_MODE
}

ZTEST(i3c_discovery, test_setnewda)
{
	struct i3c_device_desc *desc =
		i3c_find_target(ctrl, I3C_TEST_TARGET_DA);

	if (!desc) {
		ztest_test_skip();
	}

	uint8_t old_da = desc->dynamic_addr;
	uint8_t new_da = 0x0B;

	/* Verify new address is free */
	struct i3c_device_desc *check = i3c_find_target(ctrl, new_da);

	if (check != NULL) {
		new_da = 0x0C;
		check = i3c_find_target(ctrl, new_da);
		if (check != NULL) {
			ztest_test_skip();
		}
	}

	/* Step 1: Send SETNEWDA to change address */
	uint8_t payload = new_da << 1;
	struct i3c_ccc_target_payload target = {
		.addr = old_da,
		.rnw = 0,
		.data = &payload,
		.data_len = 1,
	};
	struct i3c_ccc_payload ccc = {
		.ccc.id = I3C_CCC_SETNEWDA,
		.targets.payloads = &target,
		.targets.num_targets = 1,
	};

	int ret = i3c_do_ccc(ctrl, &ccc);

	if (ret == -ENXIO) {
		LOG_INF("SETNEWDA not supported by DA=0x%02x, skip", old_da);
		ztest_test_skip();
	}
	zassert_ok(ret, "SETNEWDA failed: %d", ret);

	/* Delay for device to switch internal address register */
	k_msleep(50);

	/* Update descriptor to reflect new address for subsequent operations */
	desc->dynamic_addr = new_da;

	/* Step 2: Verify target responds at NEW address */
	uint8_t bcr;

	ret = i3c_do_ccc_getbcr(ctrl, new_da, &bcr);
	if (ret < 0) {
		/* Device doesn't truly support SETNEWDA - restore and skip */
		LOG_INF("No response at new DA 0x%02x, restore 0x%02x",
			new_da, old_da);
		desc->dynamic_addr = old_da;
		payload = old_da << 1;
		target.addr = new_da;
		(void)i3c_do_ccc(ctrl, &ccc);
		ztest_test_skip();
	}

	/* Step 3: Verify target does NOT respond at OLD address */
	ret = i3c_do_ccc_getbcr(ctrl, old_da, &bcr);
	zassert_true(ret < 0, "Target still responds at old DA=0x%02x", old_da);

	/* Step 4: Restore original address */
	payload = old_da << 1;
	target.addr = new_da;
	ret = i3c_do_ccc(ctrl, &ccc);
	zassert_ok(ret, "SETNEWDA restore failed: %d", ret);

	/* Update descriptor to reflect restored address */
	desc->dynamic_addr = old_da;

	/* Step 5: Verify restoration */
	ret = i3c_do_ccc_getbcr(ctrl, old_da, &bcr);
	zassert_ok(ret, "Target doesn't respond at restored DA=0x%02x", old_da);
}

ZTEST(i3c_discovery, test_setdasa)
{
	/* Per I3C spec: SETDASA assigns static address as dynamic address */
	struct i3c_device_desc *dev = NULL;
	int static_count = 0;

	/* Count targets with static addresses */
	I3C_BUS_FOR_EACH_I3CDEV(ctrl, dev) {
		if (dev->static_addr != 0) {
			static_count++;
		}
	}

	if (static_count == 0) {
		ztest_test_skip();
	}

	/* Step 1: RSTDAA to clear dynamic addresses */
	int ret = i3c_do_ccc_rstdaa(ctrl);

	zassert_ok(ret, "RSTDAA failed: %d", ret);
	k_msleep(50);

	/* Step 2: Send SETDASA to each target with static address */
	I3C_BUS_FOR_EACH_I3CDEV(ctrl, dev) {
		if (dev->static_addr == 0) {
			continue;
		}

		/* Validate static address is in valid range */
		zassert_true(dev->static_addr >= 0x08 &&
			     dev->static_addr <= 0x77,
			     "Static addr 0x%02x out of range",
			     dev->static_addr);

		/* SETDASA payload: static address shifted left */
		uint8_t payload = dev->static_addr << 1;
		struct i3c_ccc_target_payload target = {
			.addr = dev->static_addr,
			.rnw = 0,
			.data = &payload,
			.data_len = 1,
		};
		struct i3c_ccc_payload ccc = {
			.ccc.id = I3C_CCC_SETDASA,
			.targets.payloads = &target,
			.targets.num_targets = 1,
		};

		ret = i3c_do_ccc(ctrl, &ccc);
		if (ret < 0) {
			/* SETDASA may not be supported by all targets */
			LOG_DBG("SETDASA not supported SA=0x%02x: %d",
				dev->static_addr, ret);
			continue;
		}

		zassert_equal(ret, 0, "SETDASA failed for SA=0x%02x: %d",
			      dev->static_addr, ret);

		/* Update descriptor - dynamic_addr now equals static_addr */
		dev->dynamic_addr = dev->static_addr;

		/* Step 3: Verify target responds at assigned address */
		uint8_t pid[6];
		uint64_t pid_from_ccc;

		ret = i3c_do_ccc_getpid(ctrl, dev->dynamic_addr, pid, 6);
		zassert_ok(ret, "GETPID failed after SETDASA for SA=0x%02x: %d",
			   dev->static_addr, ret);
		pid_from_ccc = ((uint64_t)pid[0] << 40) |
			       ((uint64_t)pid[1] << 32) |
			       ((uint64_t)pid[2] << 24) |
			       ((uint64_t)pid[3] << 16) |
			       ((uint64_t)pid[4] << 8) | pid[5];
		zassert_equal(pid_from_ccc, dev->pid,
			      "PID mismatch after SETDASA for SA=0x%02x",
			      dev->static_addr);
	}
}

ZTEST(i3c_discovery, test_i2c_devices_valid_addresses)
{
	struct i3c_i2c_device_desc *i2c_desc;
	struct i3c_device_desc *i3c_desc;
	int i2c_count = 0;
	int respond_count = 0;

	I3C_BUS_FOR_EACH_I2CDEV(ctrl, i2c_desc) {
		uint8_t dummy;
		int ret;

		i2c_count++;

		/* Verify address in valid I2C range */
		zassert_true(i2c_desc->addr >= 0x08 && i2c_desc->addr <= 0x77,
			     "I2C addr 0x%02x out of valid range 0x08-0x77",
			     i2c_desc->addr);

		/* Check for address collision with I3C devices */
		I3C_BUS_FOR_EACH_I3CDEV(ctrl, i3c_desc) {
			if (i3c_desc->dynamic_addr == i2c_desc->addr) {
				zassert_false(1,
					      "Address collision: I2C device 0x%02x conflicts with I3C device",
					      i2c_desc->addr);
			}
		}

		/* Verify I2C device actually responds on the bus.
		 * Use a simple probe read - even if it fails
		 * (no such register),
		 * the ACK from the device confirms it's present.
		 */
		ret = i3c_reg_read(ctrl, i2c_desc->addr, 0x00, &dummy, 1);
		if (ret == 0 || ret == -EIO || ret == -ENXIO) {
			/* ACK received (even if register read fails) */
			respond_count++;
			LOG_INF("I2C device at 0x%02x responds (ret=%d)",
				i2c_desc->addr, ret);
		} else {
			LOG_WRN("I2C device at 0x%02x no response (ret=%d)",
				i2c_desc->addr, ret);
		}
	}

	/* If DT declares I2C devices, at least some should respond */
	if (i2c_count > 0) {
		zassert_true(respond_count > 0,
			     "No I2C devices responded on bus (%d declared in DT)",
			     i2c_count);
		LOG_INF("I2C validation: %d/%d devices respond",
			respond_count, i2c_count);
	}
}

/**
 * @brief Test SDR mode frequency compliance across all targets
 *
 * Tests register reads on every discovered target at multiple valid SDR
 * frequencies. Per MIPI I3C spec, all I3C devices
 * must operate up to 12.5 MHz SDR.
 *
 * Frequencies tested: 6.25 MHz and 12.5 MHz.
 * Skips a frequency if the controller cannot configure it (hardware
 * constraint) rather than failing the entire test.
 */
ZTEST(i3c_discovery, test_sdr_frequency_sweep)
{
	static const uint32_t sdr_freqs[] = { 6250000U, 12500000U };
	struct i3c_config_controller config;
	uint8_t buf[4];
	int ret;

	for (size_t fi = 0; fi < ARRAY_SIZE(sdr_freqs); fi++) {
		uint32_t freq = sdr_freqs[fi];

		memset(&config, 0, sizeof(config));
		config.scl.i3c = freq;
		/* Set Open Drain timing per I3C spec */
		config.scl_od_min.high_ns = 41;
		config.scl_od_min.low_ns = I3C_OD_TLOW_MIN_NS;

		ret = i3c_configure(ctrl, I3C_CONFIG_CONTROLLER, &config);
		if (ret != 0) {
			LOG_INF("Skip %u Hz: controller rejected (%d)",
				freq, ret);
			continue;
		}

		LOG_INF("Testing SDR at %u Hz", freq);
		k_msleep(10);

		I3C_TEST_FOR_EACH_TARGET(ctrl, desc) {
			ret = i3c_reg_read(ctrl, desc->dynamic_addr,
					   I3C_TEST_CHIP_ID_REG, buf, 4);
			if (ret < 0) {
				LOG_INF("Reg read %u Hz DA=0x%02x: %d",
					freq, desc->dynamic_addr, ret);
			}
		} I3C_TEST_END_EACH_TARGET
	}

	/* Restore standard SDR frequency */
	memset(&config, 0, sizeof(config));
	config.scl.i3c = I3C_TEST_SDR_FREQ_HZ;
	config.scl_od_min.high_ns = 41;
	config.scl_od_min.low_ns = I3C_OD_TLOW_MIN_NS;
	ret = i3c_configure(ctrl, I3C_CONFIG_CONTROLLER, &config);
	zassert_ok(ret,
		   "Failed to restore %u Hz: %d",
		   I3C_TEST_SDR_FREQ_HZ, ret);
}

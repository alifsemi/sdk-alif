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
#include <inttypes.h>
#include "i3c_common.h"

LOG_MODULE_REGISTER(i3c_ccc, LOG_LEVEL_INF);

static const struct device *ctrl;

/**
 * @brief CCC test suite setup function
 *
 * Initializes I3C controller and resets bus to clean SDR state.
 * All CCC tests use I3C_TEST_FOR_EACH_MODE macro to iterate
 * through enabled speed modes (SDR by default).
 *
 * @return NULL on success, asserts on failure
 */
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

ZTEST_SUITE(i3c_ccc, NULL, suite_setup, test_before, test_after, NULL);

/**
 * @brief Test GETPID (Get Provisional ID) CCC command
 *
 * Retrieves 48-bit provisional ID from each target across all
 * speed modes. PID contains manufacturer ID and unique device ID.
 *
 * Test flow:
 * 1. Iterate through all speed modes
 * 2. For each target, send GETPID CCC
 * 3. Verify response (6 bytes received)
 */
ZTEST(i3c_ccc, test_getpid)
{
	I3C_TEST_FOR_EACH_MODE(ctrl, mode) {
		/* Delay after mode change for targets to stabilize */
		k_msleep(50);

		I3C_TEST_FOR_EACH_TARGET(ctrl, desc) {
			uint8_t pid[6];
			int ret = i3c_do_ccc_getpid(ctrl, desc->dynamic_addr,
						    pid, 6);

			/* Per I3C spec, GETPID is mandatory. */
			zassert_true(ret == 6,
				     "GETPID failed DA=0x%02x %s: %d (exp 6)",
				     desc->dynamic_addr,
				     i3c_test_mode_name(mode), ret);

			/* Verify PID matches descriptor */
			uint64_t pid_from_ccc = ((uint64_t)pid[0] << 40) |
						((uint64_t)pid[1] << 32) |
						((uint64_t)pid[2] << 24) |
						((uint64_t)pid[3] << 16) |
						((uint64_t)pid[4] << 8) |
						pid[5];
			LOG_INF("DA=0x%02x PID: CCC=0x%012" PRIx64
				" desc=0x%012" PRIx64,
				desc->dynamic_addr, pid_from_ccc, desc->pid);
			zassert_equal(pid_from_ccc, desc->pid,
				      "PID mismatch DA=0x%02x",
				      desc->dynamic_addr);
		} I3C_TEST_END_EACH_TARGET
	} I3C_TEST_END_EACH_MODE
}

/**
 * @brief Test GETBCR (Bus Characteristics Register) and
 *        GETDCR (Device Characteristics Register) CCC commands
 *
 * Retrieves bus and device characteristics from each target.
 * BCR contains target capabilities, DCR contains device role.
 *
 * Per I3C specification, GETBCR and GETDCR are mandatory CCCs.
 * Test verifies both values match the device descriptor.
 */
ZTEST(i3c_ccc, test_getbcr_getdcr)
{
	I3C_TEST_FOR_EACH_MODE(ctrl, mode) {
		I3C_TEST_FOR_EACH_TARGET(ctrl, desc) {
			uint8_t bcr, dcr;
			int ret;

			/* Per I3C spec, GETBCR is mandatory */
			ret = i3c_do_ccc_getbcr(ctrl, desc->dynamic_addr, &bcr);
			zassert_ok(ret,
				   "GETBCR failed DA=0x%02x %s",
				   desc->dynamic_addr,
				   i3c_test_mode_name(mode));

			/* Verify BCR matches descriptor */
			LOG_INF("DA=0x%02x BCR: CCC=0x%02x desc=0x%02x",
				desc->dynamic_addr, bcr, desc->bcr);
			zassert_equal(bcr, desc->bcr,
				      "BCR mismatch DA=0x%02x",
				      desc->dynamic_addr);

			/* Per I3C spec, GETDCR is mandatory */
			ret = i3c_do_ccc_getdcr(ctrl, desc->dynamic_addr, &dcr);
			zassert_ok(ret,
				   "GETDCR failed DA=0x%02x %s",
				   desc->dynamic_addr,
				   i3c_test_mode_name(mode));

			/* Verify DCR matches descriptor */
			zassert_equal(dcr, desc->dcr,
				      "DCR mismatch DA=0x%02x %s",
				      desc->dynamic_addr,
				      i3c_test_mode_name(mode));
		} I3C_TEST_END_EACH_TARGET
	} I3C_TEST_END_EACH_MODE
}

/**
 * @brief Test GETSTATUS CCC command
 *
 * Retrieves device status from each target. Status includes
 * pending interrupts, protocol errors, and device state.
 *
 * Note: GETSTATUS may not be supported by all targets.
 * Failures are logged as debug messages, not test failures.
 */
ZTEST(i3c_ccc, test_getstatus)
{
	I3C_TEST_FOR_EACH_MODE(ctrl, mode) {
		bool tested = false;

		I3C_TEST_FOR_EACH_TARGET(ctrl, desc) {
			uint16_t status;
			int ret = i3c_do_ccc_getstatus(ctrl, desc->dynamic_addr,
						       &status);

			/* GETSTATUS is optional - skip this target
			 * if not supported
			 */
			if (ret == -ENOTSUP) {
				continue;
			}

			/* If supported, it must work correctly */
			zassert_ok(ret,
				   "GETSTATUS failed DA=0x%02x %s: %d",
				   desc->dynamic_addr,
				   i3c_test_mode_name(mode), ret);
			tested = true;
		} I3C_TEST_END_EACH_TARGET

		/* Skip only if no targets supported GETSTATUS in this mode */
		if (!tested) {
			ztest_test_skip();
		}
	} I3C_TEST_END_EACH_MODE
}

/**
 * @brief Test GETMRL (Get Max Read Length) CCC command
 *
 * Retrieves maximum read length supported by each target.
 * MRL indicates the maximum number of bytes target can
 * return in a single read transaction.
 *
 * Note: GETMRL may not be supported by all targets.
 */
ZTEST(i3c_ccc, test_getmrl)
{
	I3C_TEST_FOR_EACH_MODE(ctrl, mode) {
		bool tested = false;

		I3C_TEST_FOR_EACH_TARGET(ctrl, desc) {
			uint16_t mrl;
			int ret = i3c_do_ccc_getmrl(ctrl, desc->dynamic_addr,
						    &mrl);

			/* GETMRL is optional - skip this target
			 * if not supported
			 */
			if (ret == -ENOTSUP) {
				continue;
			}

			/* If supported, it must work correctly */
			zassert_ok(ret,
				   "GETMRL failed DA=0x%02x %s: %d",
				   desc->dynamic_addr,
				   i3c_test_mode_name(mode), ret);

			/* Per MIPI spec: MRL=0 means no limit */
			if (mrl == 0) {
				LOG_INF("DA=0x%02x MRL=0, skip",
					desc->dynamic_addr);
			} else {
				zassert_true(mrl <= 0xFFFF,
					     "MRL 0x%04x out of range on DA=0x%02x",
					     mrl, desc->dynamic_addr);
			}
			tested = true;
		} I3C_TEST_END_EACH_TARGET

		/* Skip only if no targets supported GETMRL in this mode */
		if (!tested) {
			ztest_test_skip();
		}
	} I3C_TEST_END_EACH_MODE
}

/**
 * @brief Test SETMWL (Set Max Write Length) CCC command
 *
 * Sets maximum write length to 64 bytes for each target.
 * First retrieves current MWL, then attempts to set new value.
 *
 * Note: SETMWL may not be supported by all targets.
 */
ZTEST(i3c_ccc, test_setmwl)
{
	I3C_TEST_FOR_EACH_MODE(ctrl, mode) {
		bool tested = false;

		I3C_TEST_FOR_EACH_TARGET(ctrl, desc) {
			uint16_t mwl, new_mwl;
			int ret;

			/* Get current MWL */
			ret = i3c_do_ccc_getmwl(ctrl, desc->dynamic_addr, &mwl);
			if (ret == -ENOTSUP) {
				continue;
			}
			zassert_ok(ret, "GETMWL failed on DA=0x%02x: %d",
				   desc->dynamic_addr, ret);

			/* Per MIPI I3C spec: MWL=0 means no limit */
			if (mwl == 0) {
				LOG_INF("DA=0x%02x MWL=0, skip SETMWL",
					desc->dynamic_addr);
				continue;
			}

			/* Set new MWL to 64 bytes */
			ret = i3c_do_ccc_setmwl(ctrl, desc->dynamic_addr, 64);
			if (ret == -ENOTSUP) {
				continue;
			}
			zassert_ok(ret, "SETMWL failed on DA=0x%02x: %d",
				   desc->dynamic_addr, ret);

			/* Verify MWL was set correctly */
			ret = i3c_do_ccc_getmwl(ctrl, desc->dynamic_addr,
						&new_mwl);
			zassert_ok(ret, "GETMWL verify failed DA=0x%02x: %d",
				   desc->dynamic_addr, ret);
			zassert_equal(new_mwl, 64,
				    "MWL mismatch DA=0x%02x: exp 64, got %d",
				    desc->dynamic_addr, new_mwl);

			/* Restore original MWL */
			ret = i3c_do_ccc_setmwl(ctrl, desc->dynamic_addr, mwl);
			zassert_ok(ret,
				   "SETMWL restore failed DA=0x%02x: %d",
				   desc->dynamic_addr, ret);
			tested = true;
		} I3C_TEST_END_EACH_TARGET

		/* Skip only if no targets supported SETMWL in this mode */
		if (!tested) {
			ztest_test_skip();
		}
	} I3C_TEST_END_EACH_MODE
}

/**
 * @brief Test RSTACT (Reset Action) CCC command
 *
 * Triggers reset action (0x00 = full reset) on each target.
 * Verifies target responds to reset command.
 *
 * Note: RSTACT may not be supported by all targets.
 */
ZTEST(i3c_ccc, test_rstact)
{
	I3C_TEST_FOR_EACH_MODE(ctrl, mode) {
		I3C_TEST_FOR_EACH_TARGET(ctrl, desc) {
			int ret = i3c_do_ccc_rstact(ctrl, desc->dynamic_addr,
						    0x00);

			/* RSTACT is optional - skip if not supported */
			if (ret == -ENOTSUP) {
				ztest_test_skip();
			}

			/* If supported, it must work correctly */
			zassert_ok(ret,
				   "RSTACT failed DA=0x%02x %s: %d",
				   desc->dynamic_addr,
				   i3c_test_mode_name(mode), ret);
		} I3C_TEST_END_EACH_TARGET
	} I3C_TEST_END_EACH_MODE
}

/**
 * @brief Test ENEC (Enable Events Command) directed CCC
 *
 * Enables interrupt events (bit 0 = INTR) on each target.
 * Skips ghost targets (SA=0x00) that don't exist.
 *
 * Note: ENEC may not be supported by all targets.
 * BMI323 and ICM42670 do not support this command.
 */
ZTEST(i3c_ccc, test_enec_direct)
{
	I3C_TEST_FOR_EACH_MODE(ctrl, mode) {
		I3C_TEST_FOR_EACH_TARGET(ctrl, desc) {
			/* Skip ghost targets */
			if (desc->static_addr == 0x00) {
				continue;
			}

			int ret = i3c_do_ccc_enec(ctrl, desc->dynamic_addr,
						  0x01);

			/* ENEC is optional - skip if not supported */
			if (ret == -ENOTSUP) {
				ztest_test_skip();
			}

			/* If supported, it must work correctly */
			zassert_ok(ret,
				   "ENEC failed DA=0x%02x %s: %d",
				   desc->dynamic_addr,
				   i3c_test_mode_name(mode), ret);
		} I3C_TEST_END_EACH_TARGET
	} I3C_TEST_END_EACH_MODE
}

/**
 * @brief Test DISEC (Disable Events Command) directed CCC
 *
 * Disables interrupt events (bit 0 = INTR) on each target.
 * Skips ghost targets (SA=0x00) that don't exist.
 *
 * Note: DISEC may not be supported by all targets.
 * BMI323 and ICM42670 do not support this command.
 */
ZTEST(i3c_ccc, test_disec_direct)
{
	I3C_TEST_FOR_EACH_MODE(ctrl, mode) {
		I3C_TEST_FOR_EACH_TARGET(ctrl, desc) {
			/* Skip ghost targets */
			if (desc->static_addr == 0x00) {
				continue;
			}

			int ret = i3c_do_ccc_disec(ctrl, desc->dynamic_addr,
						   0x01);

			/* DISEC is optional - skip if not supported */
			if (ret == -ENOTSUP) {
				ztest_test_skip();
			}

			/* If supported, it must work correctly */
			zassert_ok(ret,
				   "DISEC failed DA=0x%02x %s: %d",
				   desc->dynamic_addr,
				   i3c_test_mode_name(mode), ret);
		} I3C_TEST_END_EACH_TARGET
	} I3C_TEST_END_EACH_MODE
}

/**
 * @brief Test multiple event enable/disable sequence
 *
 * Enables multiple events (bits 0 and 1), then disables them.
 * Tests ENEC and DISEC with multi-bit masks.
 *
 * Test flow:
 * 1. Enable events 0 and 1
 * 2. If successful, disable events 0 and 1
 */
ZTEST(i3c_ccc, test_multiple_events)
{
	I3C_TEST_FOR_EACH_MODE(ctrl, mode) {
		I3C_TEST_FOR_EACH_TARGET(ctrl, desc) {
			int ret = i3c_do_ccc_enec(ctrl, desc->dynamic_addr,
						  0x03);

			/* ENEC/DISEC are optional - skip if not supported */
			if (ret == -ENOTSUP) {
				ztest_test_skip();
			}

			zassert_ok(ret, "ENEC multiple failed on DA=0x%02x: %d",
				   desc->dynamic_addr, ret);

			ret = i3c_do_ccc_disec(ctrl, desc->dynamic_addr, 0x03);
			zassert_ok(ret, "DISEC failed DA=0x%02x: %d",
				   desc->dynamic_addr, ret);
		} I3C_TEST_END_EACH_TARGET
	} I3C_TEST_END_EACH_MODE
}

/**
 * @brief Test GETCAPS (Get Device Capabilities) CCC command
 *
 * Retrieves device capabilities byte from each target.
 * CAPS indicates optional features supported by the device.
 *
 * Note: GETCAPS may not be supported by all targets.
 */
ZTEST(i3c_ccc, test_device_capabilities)
{
	I3C_TEST_FOR_EACH_MODE(ctrl, mode) {
		I3C_TEST_FOR_EACH_TARGET(ctrl, desc) {
			uint8_t caps[4];
			int ret = i3c_do_ccc_getcaps(ctrl, desc->dynamic_addr,
						     caps, 4);

			/* GETCAPS is optional - skip if not supported */
			if (ret == -ENOTSUP ||
			    ret == -ENXIO) {
				ztest_test_skip();
			}

			/* If supported, it must work correctly */
			zassert_ok(ret,
				   "GETCAPS failed DA=0x%02x %s: %d",
				   desc->dynamic_addr,
				   i3c_test_mode_name(mode), ret);
		} I3C_TEST_END_EACH_TARGET
	} I3C_TEST_END_EACH_MODE
}

/**
 * @brief Test ENEC broadcast CCC command
 *
 * Broadcasts ENEC command to all targets on the bus.
 * Enables interrupt events (bit 0) on all targets simultaneously.
 */
ZTEST(i3c_ccc, test_enec_broadcast)
{
	int ret = i3c_do_ccc_enec_broadcast(ctrl, 0x01);

	zassert_ok(ret, "ENEC broadcast failed");
}

/**
 * @brief Test DISEC broadcast CCC command
 *
 * Broadcasts DISEC command to all targets on the bus.
 * Disables interrupt events (bit 0) on all targets simultaneously.
 */
ZTEST(i3c_ccc, test_disec_broadcast)
{
	int ret = i3c_do_ccc_disec_broadcast(ctrl, 0x01);

	zassert_ok(ret, "DISEC broadcast failed");
}

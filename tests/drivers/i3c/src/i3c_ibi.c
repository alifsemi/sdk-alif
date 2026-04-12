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

LOG_MODULE_REGISTER(i3c_ibi, LOG_LEVEL_INF);

/* Per MIPI I3C spec: BCR bit 1 = IBI Request Capable */
#define I3C_IBI_CAPABLE BIT(1)

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

ZTEST_SUITE(i3c_ibi, NULL, suite_setup, test_before, test_after, NULL);

ZTEST(i3c_ibi, test_ibi_capability_in_bcr)
{
	I3C_TEST_FOR_EACH_MODE(ctrl, mode) {
		struct i3c_device_desc *desc;
		uint8_t bcr;
		int ret;

		desc = i3c_find_target(ctrl, I3C_TEST_TARGET_DA);
		if (!desc) {
			ztest_test_skip();
		}

		/* Verify BCR from descriptor matches GETBCR CCC result */
		ret = i3c_do_ccc_getbcr(ctrl, desc->dynamic_addr, &bcr);
		zassert_ok(ret, "GETBCR failed in %s mode: %d",
			   i3c_test_mode_name(mode), ret);

		zassert_equal(desc->bcr, bcr,
			      "Descriptor BCR (0x%02x) mismatch with GETBCR (0x%02x) in %s mode",
			      desc->bcr, bcr, i3c_test_mode_name(mode));

		LOG_INF("Target at DA=0x%02x BCR=0x%02x IBI capable: %s in %s",
			desc->dynamic_addr, bcr,
			(bcr & I3C_IBI_CAPABLE) ? "yes" : "no",
			i3c_test_mode_name(mode));
	} I3C_TEST_END_EACH_MODE
}

ZTEST(i3c_ibi, test_enable_disable_cycling)
{
	I3C_TEST_FOR_EACH_MODE(ctrl, mode) {
		struct i3c_device_desc *desc;
		int ret;

		desc = i3c_find_target(ctrl, I3C_TEST_TARGET_DA);
		if (!desc) {
			ztest_test_skip();
		}

		/* Verify ENEC/DISEC cycling works correctly */
		for (int i = 0; i < 3; i++) {
			ret = i3c_do_ccc_enec(ctrl, desc->dynamic_addr, 0x01);
			if (ret == -ENOTSUP) {
				ztest_test_skip();
			}
			zassert_ok(ret, "ENEC cycle %d failed in %s mode: %d",
				   i + 1, i3c_test_mode_name(mode), ret);

			ret = i3c_do_ccc_disec(ctrl, desc->dynamic_addr, 0x01);
			zassert_ok(ret, "DISEC cycle %d failed in %s mode: %d",
				   i + 1, i3c_test_mode_name(mode), ret);
		}
	} I3C_TEST_END_EACH_MODE
}

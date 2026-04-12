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

LOG_MODULE_REGISTER(i3c_stress, LOG_LEVEL_INF);

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

ZTEST_SUITE(i3c_stress, NULL, suite_setup, test_before, test_after, NULL);

ZTEST(i3c_stress, test_repeated_getpid)
{
	I3C_TEST_FOR_EACH_MODE(ctrl, mode) {
		I3C_TEST_FOR_EACH_TARGET(ctrl, desc) {
			uint8_t pid_buf[6];
			int success_count = 0;

			for (int i = 0; i < 10; i++) {
				int ret = i3c_do_ccc_getpid(ctrl,
						    desc->dynamic_addr,
						    pid_buf, 6);

				if (ret == 6) {
					success_count++;
				}
			}

			/* All 10 iterations must succeed for
			 * SDR mode compliance
			 */
			zassert_equal(success_count, 10,
				      "GETPID stress: %d/10 on DA=0x%02x %s",
				      success_count, desc->dynamic_addr,
				      i3c_test_mode_name(mode));
		} I3C_TEST_END_EACH_TARGET
	} I3C_TEST_END_EACH_MODE
}

/* Read endurance test on ALL targets at ALL modes */
ZTEST(i3c_stress, test_read_endurance)
{
	I3C_TEST_FOR_EACH_MODE(ctrl, mode) {
		I3C_TEST_FOR_EACH_TARGET(ctrl, desc) {
			uint8_t data[4];
			int success_count = 0;

			for (int i = 0; i < 100; i++) {
				int ret = i3c_reg_read(ctrl,
						     desc->dynamic_addr,
						     I3C_TEST_CHIP_ID_REG,
						     data, 4);

				if (ret == 0) {
					success_count++;
				}
			}

			/* All 100 iterations must succeed for
			 * SDR mode compliance
			 */
			zassert_equal(success_count, 100,
				      "Read endurance: %d/100 on DA=0x%02x %s",
				      success_count, desc->dynamic_addr,
				      i3c_test_mode_name(mode));
		} I3C_TEST_END_EACH_TARGET
	} I3C_TEST_END_EACH_MODE
}

/* Multi-message endurance test on ALL targets at ALL modes */
ZTEST(i3c_stress, test_multi_msg_endurance)
{
	I3C_TEST_FOR_EACH_MODE(ctrl, mode) {
		I3C_TEST_FOR_EACH_TARGET(ctrl, desc) {
			uint8_t buf1[4], buf2[4];
			int success_count = 0;

			for (int i = 0; i < 20; i++) {
				int ret1 = i3c_reg_read(ctrl,
					desc->dynamic_addr,
					I3C_TEST_CHIP_ID_REG,
					buf1, 4);
				int ret2 = i3c_reg_read(ctrl,
					desc->dynamic_addr,
					I3C_TEST_CHIP_ID_REG,
					buf2, 4);

				if (ret1 == 0 && ret2 == 0) {
					success_count++;
				}
			}

			/* All 20 multi-message iterations must succeed */
			zassert_equal(success_count, 20,
				      "Multi-msg endurance failed: %d/20 succeeded on DA=0x%02x",
				      success_count, desc->dynamic_addr);
		} I3C_TEST_END_EACH_TARGET
	} I3C_TEST_END_EACH_MODE
}

/* CCC transfer interleave test on ALL targets at ALL modes */
ZTEST(i3c_stress, test_ccc_transfer_interleave)
{
	I3C_TEST_FOR_EACH_MODE(ctrl, mode) {
		I3C_TEST_FOR_EACH_TARGET(ctrl, desc) {
			uint8_t pid_buf[6];
			uint8_t dcr;
			uint8_t buf[4];
			int success_count = 0;

			for (int i = 0; i < 20; i++) {
				int ret1 = i3c_do_ccc_getpid(ctrl,
					desc->dynamic_addr,
					pid_buf, 6);
				int ret2 = i3c_reg_read(ctrl,
					desc->dynamic_addr,
					I3C_TEST_CHIP_ID_REG,
					buf, 2);
				int ret3 = i3c_do_ccc_getpid(ctrl,
					desc->dynamic_addr,
					pid_buf, 6);
				int ret4 = i3c_do_ccc_getdcr(ctrl,
					desc->dynamic_addr,
					&dcr);
				int ret5 = i3c_reg_read(ctrl,
					desc->dynamic_addr,
					I3C_TEST_CHIP_ID_REG,
					buf, 2);

				/*
				 * All 5 operations must succeed
				 * for a successful iteration.
				 */
				if (ret1 == 6 && ret2 == 0 && ret3 == 6 &&
				    ret4 == 0 && ret5 == 0) {
					success_count++;
				}
			}

			/* All 20 interleave iterations must succeed */
			zassert_equal(success_count, 20,
				      "CCC/transfer interleave failed: %d/20 succeeded on DA=0x%02x",
				      success_count, desc->dynamic_addr);
		} I3C_TEST_END_EACH_TARGET
	} I3C_TEST_END_EACH_MODE
}

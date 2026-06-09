/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include "test_cmp.h"

LOG_MODULE_DECLARE(ALIF_CMP);

ZTEST(cmp_Negative, test_cmp_device_ready)
{
	const struct device *const dev = DEVICE_DT_GET(NODE_LABEL);

	zassert_true(device_is_ready(dev),
		"CMP device should be ready after init");
	LOG_INF("Device readiness check passed");
}

ZTEST(cmp_Negative, test_Comp_set_trigger_without_callback)
{
	const struct device *const dev = DEVICE_DT_GET(NODE_LABEL);
	int ret;

	zassert_true(device_is_ready(dev), "CMP device not ready");

	/* Disable callback first */
	ret = comparator_set_trigger_callback(dev, NULL, NULL);
	zassert_equal(ret, 0, "set_trigger_callback(NULL) failed (%d)", ret);

	/* Set trigger without a callback registered */
	ret = comparator_set_trigger(dev, COMPARATOR_TRIGGER_BOTH_EDGES);
	zassert_equal(ret, 0, "set_trigger should succeed even without callback (%d)", ret);
	LOG_INF("Trigger without callback: returned %d", ret);
}

ZTEST(cmp_Negative, test_get_output_multiple_reads)
{
	const struct device *const dev = DEVICE_DT_GET(NODE_LABEL);
	int ret1, ret2;

	zassert_true(device_is_ready(dev), "CMP device not ready");

	ret1 = comparator_get_output(dev);
	zassert_true((ret1 == 0) || (ret1 == 1),
		"First read unexpected value %d", ret1);

	/* Back-to-back read should not crash or return error */
	ret2 = comparator_get_output(dev);
	zassert_true((ret2 == 0) || (ret2 == 1),
		"Second read unexpected value %d", ret2);

	LOG_INF("Multiple reads: %d, %d", ret1, ret2);
}

ZTEST(cmp_Negative, test_trigger_is_pending_no_event)
{
	const struct device *const dev = DEVICE_DT_GET(NODE_LABEL);
	int ret;

	zassert_true(device_is_ready(dev), "CMP device not ready");

	/* Disable trigger first to ensure clean state */
	ret = comparator_set_trigger(dev, COMPARATOR_TRIGGER_NONE);
	zassert_equal(ret, 0, "set_trigger NONE failed (%d)", ret);

	/* With no trigger configured, pending should return 0 */
	ret = comparator_trigger_is_pending(dev);
	zassert_equal(ret, 0, "trigger_is_pending should be 0 after TRIGGER_NONE (%d)", ret);
	LOG_INF("Pending after TRIGGER_NONE: %d", ret);
}

ZTEST(cmp_Negative, test_callback_overwrite)
{
	const struct device *const dev = DEVICE_DT_GET(NODE_LABEL);
	int ret;

	zassert_true(device_is_ready(dev), "CMP device not ready");

	/* Set callback */
	ret = comparator_set_trigger_callback(dev, cmp_callback, &value);
	zassert_equal(ret, 0, "First set_trigger_callback failed (%d)", ret);

	/* Overwrite with NULL - should not crash */
	ret = comparator_set_trigger_callback(dev, NULL, NULL);
	zassert_equal(ret, 0, "Overwrite callback to NULL failed (%d)", ret);

	/* Set callback again - should work */
	ret = comparator_set_trigger_callback(dev, cmp_callback, &value);
	zassert_equal(ret, 0, "Re-set callback failed (%d)", ret);

	LOG_INF("Callback overwrite sequence OK");
}

ZTEST_SUITE(cmp_Negative, NULL, NULL, NULL, NULL, NULL);

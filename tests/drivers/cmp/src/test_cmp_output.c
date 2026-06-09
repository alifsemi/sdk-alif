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

ZTEST(cmp_Output, test_Comp_get_output_ready)
{
	const struct device *const dev = DEVICE_DT_GET(NODE_LABEL);
	int ret;

	zassert_true(device_is_ready(dev), "CMP device not ready");

	ret = comparator_get_output(dev);
	zassert_true((ret == 0) || (ret == 1),
		"comparator_get_output returned unexpected value %d", ret);
	LOG_INF("CMP output state: %d", ret);
}

ZTEST(cmp_Output, test_Comp_get_output_after_trigger)
{
	const struct device *const dev = DEVICE_DT_GET(NODE_LABEL);
	int ret;

	zassert_true(device_is_ready(dev), "CMP device not ready");

	ret = comparator_set_trigger_callback(dev, cmp_callback, &value);
	zassert_equal(ret, 0, "set_trigger_callback failed (%d)", ret);

	ret = comparator_set_trigger(dev, COMPARATOR_TRIGGER_BOTH_EDGES);
	zassert_equal(ret, 0, "set_trigger failed (%d)", ret);

	ret = comparator_get_output(dev);
	zassert_true((ret == 0) || (ret == 1),
		"comparator_get_output returned unexpected value %d", ret);
	LOG_INF("CMP output after trigger setup: %d", ret);
}

ZTEST(cmp_Output, test_Comp_trigger_is_pending_clear)
{
	const struct device *const dev = DEVICE_DT_GET(NODE_LABEL);
	int ret;

	zassert_true(device_is_ready(dev), "CMP device not ready");

	ret = comparator_trigger_is_pending(dev);
	zassert_true((ret == 0) || (ret == 1),
		"trigger_is_pending returned unexpected value %d", ret);
	LOG_INF("Trigger pending state: %d", ret);
}

ZTEST(cmp_Output, test_Comp_get_output_stability)
{
	const struct device *const dev = DEVICE_DT_GET(NODE_LABEL);
	int r1, r2, r3;

	zassert_true(device_is_ready(dev), "CMP device not ready");

	r1 = comparator_get_output(dev);
	r2 = comparator_get_output(dev);
	r3 = comparator_get_output(dev);

	zassert_true((r1 == 0) || (r1 == 1), "Unexpected output r1=%d", r1);
	zassert_true((r2 == 0) || (r2 == 1), "Unexpected output r2=%d", r2);
	zassert_true((r3 == 0) || (r3 == 1), "Unexpected output r3=%d", r3);
}

ZTEST(cmp_Output, test_Comp_trigger_pending_after_none)
{
	const struct device *const dev = DEVICE_DT_GET(NODE_LABEL);
	int ret;

	zassert_true(device_is_ready(dev), "CMP device not ready");

	ret = comparator_set_trigger(dev, COMPARATOR_TRIGGER_NONE);
	zassert_equal(ret, 0, "set_trigger NONE failed (%d)", ret);

	ret = comparator_trigger_is_pending(dev);
	zassert_true((ret == 0) || (ret == 1),
		"trigger_is_pending unexpected value %d", ret);
}

ZTEST(cmp_Output, test_Comp_null_callback)
{
	const struct device *const dev = DEVICE_DT_GET(NODE_LABEL);
	int ret;

	zassert_true(device_is_ready(dev), "CMP device not ready");

	/* Setting callback to NULL should disable callbacks per Zephyr API */
	ret = comparator_set_trigger_callback(dev, NULL, NULL);
	zassert_equal(ret, 0, "set_trigger_callback(NULL) failed (%d)", ret);
	LOG_INF("Callback disabled (NULL) OK");
}

ZTEST_SUITE(cmp_Output, NULL, NULL, NULL, NULL, NULL);

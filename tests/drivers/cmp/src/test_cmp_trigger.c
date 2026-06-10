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

ZTEST(cmp_Trigger, test_Comp_trigger_both_edges)
{
	const struct device *const dev = DEVICE_DT_GET(NODE_LABEL);
	int ret;

	zassert_true(device_is_ready(dev), "CMP device not ready");
	ret = comparator_set_trigger_callback(dev, cmp_callback, &value);
	zassert_equal(ret, 0, "set_trigger_callback failed (%d)", ret);
	ret = comparator_set_trigger(dev, COMPARATOR_TRIGGER_BOTH_EDGES);
	zassert_equal(ret, 0, "set_trigger BOTH_EDGES failed (%d)", ret);
	LOG_INF("Trigger BOTH_EDGES OK");
}

ZTEST(cmp_Trigger, test_Comp_trigger_rising_edge)
{
	const struct device *const dev = DEVICE_DT_GET(NODE_LABEL);
	int ret;

	zassert_true(device_is_ready(dev), "CMP device not ready");
	ret = comparator_set_trigger_callback(dev, cmp_callback, &value);
	zassert_equal(ret, 0, "set_trigger_callback failed (%d)", ret);
	ret = comparator_set_trigger(dev, COMPARATOR_TRIGGER_RISING_EDGE);
	zassert_equal(ret, 0, "set_trigger RISING_EDGE failed (%d)", ret);
	LOG_INF("Trigger RISING_EDGE OK");
}

ZTEST(cmp_Trigger, test_Comp_trigger_falling_edge)
{
	const struct device *const dev = DEVICE_DT_GET(NODE_LABEL);
	int ret;

	zassert_true(device_is_ready(dev), "CMP device not ready");
	ret = comparator_set_trigger_callback(dev, cmp_callback, &value);
	zassert_equal(ret, 0, "set_trigger_callback failed (%d)", ret);
	ret = comparator_set_trigger(dev, COMPARATOR_TRIGGER_FALLING_EDGE);
	zassert_equal(ret, 0, "set_trigger FALLING_EDGE failed (%d)", ret);
	LOG_INF("Trigger FALLING_EDGE OK");
}

ZTEST(cmp_Trigger, test_Comp_trigger_none)
{
	const struct device *const dev = DEVICE_DT_GET(NODE_LABEL);
	int ret;

	zassert_true(device_is_ready(dev), "CMP device not ready");
	ret = comparator_set_trigger(dev, COMPARATOR_TRIGGER_NONE);
	zassert_equal(ret, 0, "set_trigger NONE failed (%d)", ret);
	LOG_INF("Trigger NONE OK");
}

ZTEST(cmp_Trigger, test_Comp_trigger_mode_switch_sequence)
{
	const struct device *const dev = DEVICE_DT_GET(NODE_LABEL);
	int ret;

	zassert_true(device_is_ready(dev), "CMP device not ready");
	ret = comparator_set_trigger_callback(dev, cmp_callback, &value);
	zassert_equal(ret, 0, "set_trigger_callback failed (%d)", ret);

	ret = comparator_set_trigger(dev, COMPARATOR_TRIGGER_RISING_EDGE);
	zassert_equal(ret, 0, "set_trigger RISING_EDGE failed (%d)", ret);
	ret = comparator_set_trigger(dev, COMPARATOR_TRIGGER_FALLING_EDGE);
	zassert_equal(ret, 0, "set_trigger FALLING_EDGE failed (%d)", ret);
	ret = comparator_set_trigger(dev, COMPARATOR_TRIGGER_BOTH_EDGES);
	zassert_equal(ret, 0, "set_trigger BOTH_EDGES failed (%d)", ret);
	ret = comparator_set_trigger(dev, COMPARATOR_TRIGGER_NONE);
	zassert_equal(ret, 0, "set_trigger NONE failed (%d)", ret);
}

ZTEST(cmp_Trigger, test_Comp_trigger_with_null_callback)
{
	const struct device *const dev = DEVICE_DT_GET(NODE_LABEL);
	int ret;

	zassert_true(device_is_ready(dev), "CMP device not ready");
	ret = comparator_set_trigger_callback(dev, NULL, NULL);
	zassert_equal(ret, 0, "set_trigger_callback(NULL) failed (%d)", ret);
	ret = comparator_set_trigger(dev, COMPARATOR_TRIGGER_BOTH_EDGES);
	zassert_equal(ret, 0, "set_trigger BOTH_EDGES with NULL callback failed (%d)", ret);
}

ZTEST(cmp_Trigger, test_Comp_start_twice)
{
	const struct device *const dev = DEVICE_DT_GET(NODE_LABEL);
	int ret;

	zassert_true(device_is_ready(dev), "CMP device not ready");
	ret = comparator_set_trigger(dev, COMPARATOR_TRIGGER_BOTH_EDGES);
	zassert_equal(ret, 0, "first set_trigger failed (%d)", ret);
	ret = comparator_set_trigger(dev, COMPARATOR_TRIGGER_BOTH_EDGES);
	zassert_equal(ret, 0, "second set_trigger failed (%d)", ret);
}

ZTEST(cmp_Trigger, test_Comp_rapid_callback)
{
	const struct device *const dev = DEVICE_DT_GET(NODE_LABEL);
	int ret;

	zassert_true(device_is_ready(dev), "CMP device not ready");
	ret = comparator_set_trigger_callback(dev, cmp_callback, &value);
	zassert_equal(ret, 0, "set_trigger_callback failed (%d)", ret);
	ret = comparator_set_trigger(dev, COMPARATOR_TRIGGER_BOTH_EDGES);
	zassert_equal(ret, 0, "set_trigger failed (%d)", ret);

	Comp_func(2);
}

ZTEST_SUITE(cmp_Trigger, NULL, NULL, NULL, NULL, NULL);

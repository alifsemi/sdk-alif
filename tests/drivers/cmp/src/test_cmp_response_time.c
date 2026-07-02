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

ZTEST(cmp_ResponseTime, test_Comp_response_time_manual_hw_validation)
{
	const struct device *const cmp_dev = DEVICE_DT_GET(NODE_LABEL);
	int ret;

	zassert_true(device_is_ready(cmp_dev), "CMP device is not ready");

	ret = comparator_set_trigger_callback(cmp_dev, cmp_callback, &value);
	zassert_equal(ret, 0, "set_trigger_callback failed (%d)", ret);

	ret = comparator_set_trigger(cmp_dev, COMPARATOR_TRIGGER_BOTH_EDGES);
	zassert_equal(ret, 0, "set_trigger failed (%d)", ret);

	LOG_INF("CMP response-time feature is configured");
	LOG_INF("Manual validation required: measure propagation delay with scope");
	LOG_INF("Pass criteria: comparator propagation delay < 5 ns");

	ztest_test_skip();
}

ZTEST_SUITE(cmp_ResponseTime, NULL, NULL, NULL, NULL, NULL);

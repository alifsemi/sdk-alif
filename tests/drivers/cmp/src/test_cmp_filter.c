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

ZTEST(cmp_Filter, test_Comp_filter_taps_4)
{
	if (CMP_FILTER_TAPS != 4) {
		ztest_test_skip();
	}
	Comp_func(3);
}

ZTEST(cmp_Filter, test_Comp_filter_taps_3)
{
	if (CMP_FILTER_TAPS != 3) {
		ztest_test_skip();
	}
	Comp_func(5);
}

ZTEST(cmp_Filter, test_Comp_filter_taps_2)
{
	if (CMP_FILTER_TAPS != 2) {
		ztest_test_skip();
	}
	Comp_func(5);
}

ZTEST(cmp_Filter, test_Comp_filter_taps_1)
{
	if (CMP_FILTER_TAPS != 1) {
		ztest_test_skip();
	}
	Comp_func(4);
}

ZTEST(cmp_Filter, test_Comp_filter_taps_invalid)
{
	if (CMP_FILTER_TAPS >= 1 && CMP_FILTER_TAPS <= 4) {
		ztest_test_skip();
	}
	zassert_true((CMP_FILTER_TAPS < 1) || (CMP_FILTER_TAPS > 4),
		"Expected invalid filter taps configuration");
}

ZTEST(cmp_Filter, test_Comp_Int_gen_after_filtering)
{
	const struct device *const dev = DEVICE_DT_GET(NODE_LABEL);
	int ret;

	zassert_true(device_is_ready(dev), "CMP device not ready");
	ret = comparator_set_trigger_callback(dev, cmp_callback, &value);
	zassert_equal(ret, 0, "set_trigger_callback failed (%d)", ret);
	ret = comparator_set_trigger(dev, COMPARATOR_TRIGGER_BOTH_EDGES);
	zassert_equal(ret, 0, "set_trigger failed (%d)", ret);

	Comp_func(3);
}

ZTEST_SUITE(cmp_Filter, NULL, NULL, NULL, NULL, NULL);

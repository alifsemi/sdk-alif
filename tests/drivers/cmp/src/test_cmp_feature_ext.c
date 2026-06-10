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

ZTEST(cmp_Hysteresis, test_Comp_Hys_0mV)
{
	if (strcmp(CMP_HYSTERESIS, "0mV") != 0) {
		ztest_test_skip();
	}
	Comp_func(4);
}

ZTEST(cmp_Hysteresis, test_Comp_Hys_6mV)
{
	if (strcmp(CMP_HYSTERESIS, "6mV") != 0) {
		ztest_test_skip();
	}
	Comp_func(4);
}

ZTEST(cmp_Hysteresis, test_Comp_Hys_12mV)
{
	if (strcmp(CMP_HYSTERESIS, "12mV") != 0) {
		ztest_test_skip();
	}
	Comp_func(4);
}

ZTEST(cmp_Hysteresis, test_Comp_Hys_18mV)
{
	if (strcmp(CMP_HYSTERESIS, "18mV") != 0) {
		ztest_test_skip();
	}
	Comp_func(4);
}

ZTEST(cmp_Hysteresis, test_Comp_Hys_24mV)
{
	if (strcmp(CMP_HYSTERESIS, "24mV") != 0) {
		ztest_test_skip();
	}
	Comp_func(4);
}

ZTEST(cmp_Hysteresis, test_Comp_Hys_30mV)
{
	if (strcmp(CMP_HYSTERESIS, "30mV") != 0) {
		ztest_test_skip();
	}
	Comp_func(4);
}

ZTEST(cmp_Hysteresis, test_Comp_Hys_36mV)
{
	if (strcmp(CMP_HYSTERESIS, "36mV") != 0) {
		ztest_test_skip();
	}
	Comp_func(4);
}

ZTEST_SUITE(cmp_Hysteresis, NULL, NULL, NULL, NULL, NULL);

ZTEST(cmp_FeatureExt, test_Comp_windowing_feature)
{
	ztest_test_skip();
}

ZTEST_SUITE(cmp_FeatureExt, NULL, NULL, NULL, NULL, NULL);

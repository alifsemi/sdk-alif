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

ZTEST(cmp_Basic, test_Comp_BasicTest)
{
	LOG_INF("Basic test with hysteresis %s", CMP_HYSTERESIS);
	zassert_true(!strcmp(CMP_HYSTERESIS, "42mV"),
		"Expected hysteresis 42mV before running basic comparison, got %s",
		CMP_HYSTERESIS);
	LOG_INF("Hysteresis is 42mV");
	Comp_func(20);
}

ZTEST(cmp_Basic, test_Comp_prescaler_min_max)
{
	zassert_true((CMP_PRESCALER >= 0) && (CMP_PRESCALER <= 63),
		"Prescaler out of range: %d", CMP_PRESCALER);
}

ZTEST(cmp_Basic, test_Comp_SampleApp)
{
	Comp_func(6);
}

ZTEST_SUITE(cmp_Basic, NULL, NULL, NULL, NULL, NULL);

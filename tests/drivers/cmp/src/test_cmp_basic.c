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

ZTEST(cmp_Basic, test_Comp_dt_properties)
{
	LOG_INF("CMP node properties: driver_instance idx=%d lpcmp=%d",
		CMP_DRV_INSTANCE, CMP_IS_LPCMP);
	LOG_INF("  positive_input=%s negative_input=%s hysteresis=%s",
		CMP_POSITIVE_INPUT, CMP_NEGATIVE_INPUT, CMP_HYSTERESIS);

	zassert_true(DT_NODE_HAS_PROP(CMP_NODE, driver_instance),
		"driver_instance is required");
	zassert_true(DT_NODE_HAS_PROP(CMP_NODE, positive_input),
		"positive_input is required");
	zassert_true(DT_NODE_HAS_PROP(CMP_NODE, negative_input),
		"negative_input is required");
	zassert_true(DT_NODE_HAS_PROP(CMP_NODE, hysteresis_level),
		"hysteresis_level is required");

	if (CMP_IS_LPCMP) {
		zassert_false(CMP_HAS_FILTER_TAPS,
			"filter_taps is not applicable to LPCMP");
		zassert_false(CMP_HAS_PRESCALER,
			"prescaler is not applicable to LPCMP");
		/*
		 * polarity_en is a boolean in the alif,cmp binding, so
		 * DT_NODE_HAS_PROP() can be true even when the overlay does
		 * not set it (value is 0). LPCMP does not program polarity;
		 * only require that inversion is not enabled.
		 */
		zassert_false(CMP_POLARITY_EN,
			"LPCMP must not enable polarity inversion");
		zassert_false(CMP_HAS_CMP_GPIOS,
			"cmp-gpios is not applicable to LPCMP (no board output pinout)");
		LOG_INF("LPCMP: filter/prescaler/cmp-gpios omitted, polarity_en=%d",
			CMP_POLARITY_EN);
	} else {
		zassert_true(CMP_HAS_FILTER_TAPS,
			"filter_taps is required for high-speed CMP");
		zassert_true(CMP_HAS_PRESCALER,
			"prescaler is required for high-speed CMP");
		zassert_true(CMP_HAS_CMP_GPIOS,
			"cmp-gpios is required for high-speed CMP output");
		LOG_INF("CMP: filter_taps=%d prescaler=%d polarity_en=%d",
			CMP_FILTER_TAPS, CMP_PRESCALER, CMP_POLARITY_EN);
	}
}

ZTEST(cmp_Basic, test_Comp_BasicTest)
{
	LOG_INF("Basic test with hysteresis %s", CMP_HYSTERESIS);
	Comp_func(20);
}

ZTEST(cmp_Basic, test_Comp_prescaler_min_max)
{
#if !CMP_HAS_PRESCALER
	LOG_INF("Skipping prescaler test: not applicable to this instance");
	ztest_test_skip();
#else
	zassert_true((CMP_PRESCALER >= 0) && (CMP_PRESCALER <= 63),
		"Prescaler out of range: %d", CMP_PRESCALER);
#endif
}

ZTEST(cmp_Basic, test_Comp_SampleApp)
{
	Comp_func(6);
}

ZTEST_SUITE(cmp_Basic, NULL, NULL, NULL, NULL, NULL);

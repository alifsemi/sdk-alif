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

ZTEST(cmp_Reference, test_Comp_ref_dac6)
{
#if defined(CONFIG_REF_DAC6)
	zassert_true(!strcmp(CMP_NEGATIVE_INPUT, "CMP_NEG_IN3"),
		"Expected negative_input CMP_NEG_IN3 for DAC6, got %s", CMP_NEGATIVE_INPUT);
	LOG_INF("CMP reference test: DAC6 via %s", CMP_NEGATIVE_INPUT);
	Comp_func(6);
#else
	ztest_test_skip();
#endif
}

ZTEST(cmp_Reference, test_Comp_int_vref)
{
#if defined(CONFIG_REF_VREF)
	zassert_true(!strcmp(CMP_NEGATIVE_INPUT, "CMP_NEG_IN2"),
		"Expected negative_input CMP_NEG_IN2 for internal Vref, got %s",
		CMP_NEGATIVE_INPUT);
	LOG_INF("CMP reference test: internal Vref via %s", CMP_NEGATIVE_INPUT);
	Comp_func(6);
#else
	ztest_test_skip();
#endif
}

ZTEST(cmp_Reference, test_Comp_ref_external_pin)
{
#if defined(CONFIG_REF_EXT)
	zassert_true(!strcmp(CMP_NEGATIVE_INPUT, "CMP_NEG_IN0"),
		"Expected negative_input CMP_NEG_IN0 for external pin, got %s", CMP_NEGATIVE_INPUT);
	LOG_INF("CMP reference test: external pin via %s", CMP_NEGATIVE_INPUT);
	Comp_func_ref_ext(6);
#else
	ztest_test_skip();
#endif
}

ZTEST_SUITE(cmp_Reference, NULL, NULL, NULL, NULL, NULL);

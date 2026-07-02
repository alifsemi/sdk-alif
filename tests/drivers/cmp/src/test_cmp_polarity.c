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

ZTEST(cmp_Polarity, test_Comp_Polarity)
{
	if (!CMP_POLARITY_EN) {
		TC_PRINT("Skipping polarity test: polarity_en is not enabled in DTS\n");
		ztest_test_skip();
		return;
	}

	Comp_func(5);
}

ZTEST_SUITE(cmp_Polarity, NULL, NULL, NULL, NULL, NULL);

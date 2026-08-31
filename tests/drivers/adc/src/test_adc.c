/*
 * Copyright (C) 2024 Alif Semiconductor.
 * SPDX-License-Identifier: Apache-2.0
 */
#include <string.h>

#include "test_adc.h"

LOG_MODULE_REGISTER(ALIF_ADC, LOG_LEVEL_INF);

uint32_t buffer[8];
uint32_t All_channel;
uint32_t m_samplings_done;
uint8_t comparator;
uint32_t comp_value[MAX_NUM_THRESHOLD];

void adc_test_reset(void)
{
	m_samplings_done = 0;
	comparator = 0;
	memset(comp_value, 0, sizeof(comp_value));
	memset(buffer, 0xFF, sizeof(buffer));
}

static void adc_before(void *fixture)
{
	ARG_UNUSED(fixture);
	adc_test_reset();
}

#if CONFIG_TEST_ADC_MultiCH
#if !DT_NODE_HAS_PROP(DT_ALIAS(test_adc), adc_channel_scan)
#error "MultiCH needs adc_channel_scan (use alif_adc1/24_multich.overlay)"
#elif !DT_ENUM_HAS_VALUE(DT_ALIAS(test_adc), adc_channel_scan, MULTIPLE_CHANNEL_SCAN)
#error "adc_channel_scan must be MULTIPLE_CHANNEL_SCAN"
#endif
#if !DT_NODE_HAS_PROP(DT_ALIAS(test_adc), adc_conversion_mode)
#error "MultiCH needs CONTINUOUS_CONVERSION (single-shot + scan unsupported)"
#elif !DT_ENUM_HAS_VALUE(DT_ALIAS(test_adc), adc_conversion_mode, CONTINUOUS_CONVERSION)
#error "MultiCH needs CONTINUOUS_CONVERSION (single-shot + scan unsupported)"
#endif
#endif

#if CONFIG_TEST_ADC_CONTINUOUS
#if !DT_NODE_HAS_PROP(DT_ALIAS(test_adc), adc_conversion_mode)
#error "CONTINUOUS needs adc_conversion_mode (use continuous overlay)"
#elif !DT_ENUM_HAS_VALUE(DT_ALIAS(test_adc), adc_conversion_mode, CONTINUOUS_CONVERSION)
#error "adc_conversion_mode must be CONTINUOUS_CONVERSION"
#endif
#endif

#if (CONFIG_TEST_ADC24)
#if (CONFIG_TEST_ADC_MultiCH)
ZTEST_SUITE(adc24_multi_channel, NULL, NULL, adc_before, NULL, NULL);
#else
ZTEST_SUITE(adc24_differential, NULL, NULL, adc_before, NULL, NULL);
#endif
#else
#if (CONFIG_TEST_ADC_MultiCH)
ZTEST_SUITE(adc12_multi_channel, NULL, NULL, adc_before, NULL, NULL);
#elif (CONFIG_TEST_ADC_DIFFERENTIAL)
ZTEST_SUITE(adc12_differential, NULL, NULL, adc_before, NULL, NULL);
#else
ZTEST_SUITE(adc_single_ended, NULL, NULL, adc_before, NULL, NULL);
#endif
#endif

/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <string.h>

#include "test_adc.h"

LOG_MODULE_REGISTER(ALIF_ADC, LOG_LEVEL_INF);

uint32_t buffer[8];
uint32_t All_channel;
uint32_t m_samplings_done;
uint8_t comparator;
uint32_t comp_value[MAX_NUM_THRESHOLD];

enum adc_action adc_call_back(const struct device *dev,
			      const struct adc_sequence *sequence,
			      uint16_t sampling_index)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(sequence);

	LOG_INF("sampling index 0x%X", sampling_index);

	if (comparator & ADC_COMPARATOR_THRESHOLD_ABOVE_A) {
		comp_value[0] += 1;
	}
	if (comparator & ADC_COMPARATOR_THRESHOLD_ABOVE_B) {
		comp_value[1] += 1;
	}
	if (comparator & ADC_COMPARATOR_THRESHOLD_BELOW_A) {
		comp_value[2] += 1;
	}
	if (comparator & ADC_COMPARATOR_THRESHOLD_BELOW_B) {
		comp_value[3] += 1;
	}
	if (comparator & ADC_COMPARATOR_THRESHOLD_BETWEEN_A_B) {
		comp_value[4] += 1;
	}
	if (comparator & ADC_COMPARATOR_THRESHOLD_OUTSIDE_A_B) {
		comp_value[5] += 1;
	}

	++m_samplings_done;

	if (m_samplings_done < 2) {
		return ADC_ACTION_REPEAT;
	}

	return ADC_ACTION_FINISH;
}

void adc_test_reset(void)
{
	uint32_t i;

	m_samplings_done = 0;
	comparator = 0;
	memset(comp_value, 0, sizeof(comp_value));
	for (i = 0; i < ARRAY_SIZE(buffer); i++) {
		buffer[i] = ADC_SAMPLE_SENTINEL;
	}
}

void adc_assert_samples(uint32_t channels)
{
	uint32_t count = POPCOUNT(channels);
	uint32_t i;

	zassert_equal(m_samplings_done, 2,
		      "sampling callback count %u, expected 2",
		      m_samplings_done);
	zassert_true(count > 0 && count <= ARRAY_SIZE(buffer),
		     "channel mask 0x%x does not fit the sample buffer",
		     channels);

	for (i = 0; i < count; i++) {
		zassert_not_equal(buffer[i], ADC_SAMPLE_SENTINEL,
				  "requested channel sample %u was not written", i);
	}
}

static void adc_before(void *fixture)
{
	ARG_UNUSED(fixture);
	adc_test_reset();
}

#if CONFIG_TEST_ADC_MULTICH
#if !DT_NODE_HAS_PROP(DT_ALIAS(test_adc), adc_channel_scan)
#error "MULTICH needs adc_channel_scan (use alif_adc1/24_multich.overlay)"
#elif !DT_ENUM_HAS_VALUE(DT_ALIAS(test_adc), adc_channel_scan, MULTIPLE_CHANNEL_SCAN)
#error "adc_channel_scan must be MULTIPLE_CHANNEL_SCAN"
#endif
#if !DT_NODE_HAS_PROP(DT_ALIAS(test_adc), adc_conversion_mode)
#error "MULTICH needs CONTINUOUS_CONVERSION (single-shot + scan unsupported)"
#elif !DT_ENUM_HAS_VALUE(DT_ALIAS(test_adc), adc_conversion_mode, CONTINUOUS_CONVERSION)
#error "MULTICH needs CONTINUOUS_CONVERSION (single-shot + scan unsupported)"
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
#if (CONFIG_TEST_ADC_MULTICH)
ZTEST_SUITE(adc24_multi_channel, NULL, NULL, adc_before, NULL, NULL);
#else
ZTEST_SUITE(adc24_differential, NULL, NULL, adc_before, NULL, NULL);
#endif
#else
#if (CONFIG_TEST_ADC_MULTICH)
ZTEST_SUITE(adc12_multi_channel, NULL, NULL, adc_before, NULL, NULL);
#elif (CONFIG_TEST_ADC_DIFFERENTIAL)
ZTEST_SUITE(adc12_differential, NULL, NULL, adc_before, NULL, NULL);
#else
ZTEST_SUITE(adc_single_ended, NULL, NULL, adc_before, NULL, NULL);
#endif
#endif

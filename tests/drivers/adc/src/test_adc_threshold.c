/*
 * Copyright (C) 2026 Alif Semiconductor.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Comparator threshold and window detection. The hardware raises comp_a and
 * comp_b interrupts when the sample crosses threshold A or B. The ztest name
 * matches the overlay mode: above, below, or window — not a plain channel test.
 */
#include "test_adc.h"

#if CONFIG_TEST_ADC_THRESHOLD

LOG_MODULE_DECLARE(ALIF_ADC, LOG_LEVEL_INF);

static int threshold_channel_id(void)
{
#if !CONFIG_TEST_ADC24 && CONFIG_TEST_ADC_DIFFERENTIAL
	/*
	 * ADC12 differential mode uses logical channel IDs, same pairs as
	 * test_adc12.c: CH0+CH1 -> 0, CH2+CH3 -> 1, CH4+CH5 -> 2.
	 */
#if CONFIG_TEST_ADC_CH4 && CONFIG_TEST_ADC_CH5
	return ADC_CHANNEL_2;
#elif CONFIG_TEST_ADC_CH2 && CONFIG_TEST_ADC_CH3
	return ADC_CHANNEL_1;
#elif CONFIG_TEST_ADC_CH0 && CONFIG_TEST_ADC_CH1
	return ADC_CHANNEL_0;
#endif
#endif
#if CONFIG_TEST_ADC_CH7
	return ADC_CHANNEL_7;
#elif CONFIG_TEST_ADC_CH6
	return ADC_CHANNEL_6;
#elif CONFIG_TEST_ADC_CH5
	return ADC_CHANNEL_5;
#elif CONFIG_TEST_ADC_CH4
	return ADC_CHANNEL_4;
#elif CONFIG_TEST_ADC_CH3
	return ADC_CHANNEL_3;
#elif CONFIG_TEST_ADC_CH2
	return ADC_CHANNEL_2;
#elif CONFIG_TEST_ADC_CH1
	return ADC_CHANNEL_1;
#elif CONFIG_TEST_ADC_CH0
	return ADC_CHANNEL_0;
#else
	return ADC_CHANNEL_0;
#endif
}

static enum adc_action threshold_done(const struct device *dev,
				      const struct adc_sequence *sequence,
				      uint16_t sampling_index)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(sequence);
	ARG_UNUSED(sampling_index);

	return ADC_ACTION_FINISH;
}

static void log_comparator_events(uint32_t sample)
{
	LOG_INF("Sample 0x%x, threshold A 0x%x, threshold B 0x%x",
		sample,
		DT_PROP(ADC, comparator_threshold_a),
		DT_PROP(ADC, comparator_threshold_b));

#if DT_ENUM_HAS_VALUE(ADC, comparator_threshold_comparison, ABOVE_THRESHOLD_A_AND_B)
	LOG_INF("Comparator mode: above threshold A and B");
#elif DT_ENUM_HAS_VALUE(ADC, comparator_threshold_comparison, BELOW_THRESHOLD_A_AND_B)
	LOG_INF("Comparator mode: below threshold A and B");
#else
	LOG_INF("Comparator mode: window between or outside A and B");
#endif

	LOG_INF("Comparator interrupt status 0x%x", comparator);
	if (comparator & ADC_COMPARATOR_THRESHOLD_ABOVE_A) {
		LOG_INF("Interrupt: sample above threshold A");
	}
	if (comparator & ADC_COMPARATOR_THRESHOLD_ABOVE_B) {
		LOG_INF("Interrupt: sample above threshold B");
	}
	if (comparator & ADC_COMPARATOR_THRESHOLD_BELOW_A) {
		LOG_INF("Interrupt: sample below threshold A");
	}
	if (comparator & ADC_COMPARATOR_THRESHOLD_BELOW_B) {
		LOG_INF("Interrupt: sample below threshold B");
	}
	if (comparator & ADC_COMPARATOR_THRESHOLD_BETWEEN_A_B) {
		LOG_INF("Interrupt: sample inside window A..B");
	}
	if (comparator & ADC_COMPARATOR_THRESHOLD_OUTSIDE_A_B) {
		LOG_INF("Interrupt: sample outside window A..B");
	}
}

static void test_threshold_interrupt(bool differential)
{
	int ret;
	int channel = threshold_channel_id();
	uint32_t mask = BIT(channel);
	const struct device *adc_dev = DEVICE_DT_GET(ADC);
	struct adc_channel_cfg channel_cfg = {
		.differential = differential,
		.channel_id = channel,
	};
	struct adc_sequence_options options = {
		.callback = threshold_done,
		.user_data = &comparator,
	};
	struct adc_sequence sequence = {
		.options = &options,
		.buffer = buffer,
		.buffer_size = sizeof(buffer),
		.channels = mask,
	};

	zassert_true(device_is_ready(adc_dev), "ADC device is not ready");

#if DT_ENUM_HAS_VALUE(ADC, comparator_threshold_comparison, ABOVE_THRESHOLD_A_AND_B)
	LOG_INF("Threshold above test, sampling channel %d", channel);
#elif DT_ENUM_HAS_VALUE(ADC, comparator_threshold_comparison, BELOW_THRESHOLD_A_AND_B)
	LOG_INF("Threshold below test, sampling channel %d", channel);
#else
	LOG_INF("Threshold window test, sampling channel %d", channel);
#endif

	ret = adc_channel_setup(adc_dev, &channel_cfg);
	zassert_equal(ret, 0, "ADC channel setup failed with code %d", ret);

	comparator = 0;
	buffer[0] = 0;

	ret = adc_read(adc_dev, &sequence);
	zassert_equal(ret, 0, "adc_read() failed with code %d", ret);

	log_comparator_events(buffer[0]);
}

#if CONFIG_TEST_ADC24
#if CONFIG_TEST_ADC_MULTICH
#if DT_ENUM_HAS_VALUE(ADC, comparator_threshold_comparison, ABOVE_THRESHOLD_A_AND_B)
ZTEST(adc24_multi_channel, test_adc24_threshold_above)
#elif DT_ENUM_HAS_VALUE(ADC, comparator_threshold_comparison, BELOW_THRESHOLD_A_AND_B)
ZTEST(adc24_multi_channel, test_adc24_threshold_below)
#else
ZTEST(adc24_multi_channel, test_adc24_threshold_window)
#endif
{
	test_threshold_interrupt(true);
}
#else
#if DT_ENUM_HAS_VALUE(ADC, comparator_threshold_comparison, ABOVE_THRESHOLD_A_AND_B)
ZTEST(adc24_differential, test_adc24_threshold_above)
#elif DT_ENUM_HAS_VALUE(ADC, comparator_threshold_comparison, BELOW_THRESHOLD_A_AND_B)
ZTEST(adc24_differential, test_adc24_threshold_below)
#else
ZTEST(adc24_differential, test_adc24_threshold_window)
#endif
{
	test_threshold_interrupt(true);
}
#endif
#else /* ADC12 */
#if CONFIG_TEST_ADC_MULTICH
#if DT_ENUM_HAS_VALUE(ADC, comparator_threshold_comparison, ABOVE_THRESHOLD_A_AND_B)
ZTEST(adc12_multi_channel, test_adc12_threshold_above)
#elif DT_ENUM_HAS_VALUE(ADC, comparator_threshold_comparison, BELOW_THRESHOLD_A_AND_B)
ZTEST(adc12_multi_channel, test_adc12_threshold_below)
#else
ZTEST(adc12_multi_channel, test_adc12_threshold_window)
#endif
{
	test_threshold_interrupt(false);
}
#elif CONFIG_TEST_ADC_DIFFERENTIAL
#if DT_ENUM_HAS_VALUE(ADC, comparator_threshold_comparison, ABOVE_THRESHOLD_A_AND_B)
ZTEST(adc12_differential, test_adc12_threshold_above)
#elif DT_ENUM_HAS_VALUE(ADC, comparator_threshold_comparison, BELOW_THRESHOLD_A_AND_B)
ZTEST(adc12_differential, test_adc12_threshold_below)
#else
ZTEST(adc12_differential, test_adc12_threshold_window)
#endif
{
	test_threshold_interrupt(true);
}
#else
#if DT_ENUM_HAS_VALUE(ADC, comparator_threshold_comparison, ABOVE_THRESHOLD_A_AND_B)
ZTEST(adc_single_ended, test_adc12_threshold_above)
#elif DT_ENUM_HAS_VALUE(ADC, comparator_threshold_comparison, BELOW_THRESHOLD_A_AND_B)
ZTEST(adc_single_ended, test_adc12_threshold_below)
#else
ZTEST(adc_single_ended, test_adc12_threshold_window)
#endif
{
	test_threshold_interrupt(false);
}
#endif
#endif /* CONFIG_TEST_ADC24 */

#endif /* CONFIG_TEST_ADC_THRESHOLD */

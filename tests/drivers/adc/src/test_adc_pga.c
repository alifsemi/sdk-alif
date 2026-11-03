/*
 * Copyright (C) 2026 Alif Semiconductor.
 * SPDX-License-Identifier: Apache-2.0
 *
 * PGA gain is programmed by a gain overlay (pga_enable + pga_gain). This
 * testcase reads one channel and logs the configured gain so the suite name
 * is clearly a PGA gain run, not a plain single-ended channel test.
 */
#include "test_adc.h"

#if CONFIG_TEST_ADC_PGA

LOG_MODULE_DECLARE(ALIF_ADC, LOG_LEVEL_INF);

static int pga_channel_id(void)
{
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
	return ADC_CHANNEL_1;
#endif
}

static void test_pga_gain(bool differential)
{
	int ret;
	int channel = pga_channel_id();
	uint32_t mask = BIT(channel);
	const struct device *adc_dev = DEVICE_DT_GET(ADC);
	struct adc_channel_cfg channel_cfg = {
		.differential = differential,
		.channel_id = channel,
	};
	struct adc_sequence_options options = {
		.callback = adc_call_back,
		.user_data = &comparator,
	};
	struct adc_sequence sequence = {
		.options = &options,
		.buffer = buffer,
		.buffer_size = sizeof(buffer),
		.channels = mask,
	};

	zassert_true(device_is_ready(adc_dev), "ADC device is not ready");

	LOG_INF("PGA gain test: %s", DT_PROP(ADC, pga_gain));
	LOG_INF("PGA enabled, sampling channel %d", channel);

	ret = adc_channel_setup(adc_dev, &channel_cfg);
	zassert_equal(ret, 0, "ADC channel setup failed with code %d", ret);

	ret = adc_read(adc_dev, &sequence);
	zassert_equal(ret, 0, "adc_read() failed with code %d", ret);
	adc_assert_samples(mask);

	LOG_INF("Channel %d value is %x", channel, buffer[0]);
	LOG_INF("ADC PGA sampling Done");
}

#if CONFIG_TEST_ADC24
#if CONFIG_TEST_ADC_MULTICH
ZTEST(adc24_multi_channel, test_adc24_pga_gain)
#else
ZTEST(adc24_differential, test_adc24_pga_gain)
#endif
{
	test_pga_gain(true);
}
#else
#if CONFIG_TEST_ADC_MULTICH
ZTEST(adc12_multi_channel, test_adc12_pga_gain)
#elif CONFIG_TEST_ADC_DIFFERENTIAL
ZTEST(adc12_differential, test_adc12_pga_gain)
#else
ZTEST(adc_single_ended, test_adc12_pga_gain)
#endif
{
	test_pga_gain(IS_ENABLED(CONFIG_TEST_ADC_DIFFERENTIAL));
}
#endif

#endif /* CONFIG_TEST_ADC_PGA */

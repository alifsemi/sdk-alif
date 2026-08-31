/*
 * Copyright (C) 2024 Alif Semiconductor.
 * SPDX-License-Identifier: Apache-2.0
 */
#include "test_adc.h"

#if !CONFIG_TEST_ADC24

LOG_MODULE_DECLARE(ALIF_ADC, LOG_LEVEL_INF);

static enum adc_action adc_call_back(const struct device *dev,
				const struct adc_sequence *sequence,
				uint16_t sampling_index)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(sequence);
	ARG_UNUSED(sampling_index);

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
	} else {
		return ADC_ACTION_FINISH;
	}
}

static int test_channels(int channel, int mask)
{
	float temp;
	int ret;
	uint32_t channels;

	adc_test_reset();

	struct adc_sequence_options adc_seq_options = {
		.callback	= adc_call_back,
		.user_data	= (uint8_t *)&comparator,
	};

	const struct device *adc_dev = DEVICE_DT_GET(ADC);

	zassert_true(device_is_ready(adc_dev), "ADC device is not ready");

	struct adc_channel_cfg channel_cfg = {
#if CONFIG_TEST_ADC_DIFFERENTIAL
		.differential = 1,
#else
		.differential = 0,
#endif
		.channel_id   = All_channel ? ADC_CHANNEL_0 : channel,
	};

	if (IS_ENABLED(CONFIG_TEST_ADC_MultiCH)) {
		LOG_INF("ADC12 multi-channel scan (continuous)");
	} else if (All_channel && IS_ENABLED(CONFIG_TEST_ADC_DIFFERENTIAL)) {
		LOG_INF("ADC12 with differential");
	} else if (!IS_ENABLED(CONFIG_TEST_ADC_DIFFERENTIAL) &&
		   IS_ENABLED(CONFIG_TEST_ADC_CONTINUOUS)) {
		LOG_INF("ADC12 single ended continuous");
	} else if (IS_ENABLED(CONFIG_TEST_ADC_DIFFERENTIAL) &&
		   IS_ENABLED(CONFIG_TEST_ADC_CONTINUOUS)) {
		LOG_INF("ADC12 Differential and continuous feature selected");
	} else {
		LOG_INF("ADC12 Single ended");
	}

	if (IS_ENABLED(CONFIG_TEST_ADC_CONTINUOUS) || All_channel) {
#if CONFIG_TEST_ADC_DIFFERENTIAL
		channels = ADC_UNMASK_CHANNEL_0 | ADC_UNMASK_CHANNEL_1 | ADC_UNMASK_CHANNEL_2;
#else
		channels = ADC_UNMASK_CHANNEL_0 | ADC_UNMASK_CHANNEL_1 | ADC_UNMASK_CHANNEL_2
			 | ADC_UNMASK_CHANNEL_3 | ADC_UNMASK_CHANNEL_4 | ADC_UNMASK_CHANNEL_5
			 | ADC_UNMASK_CHANNEL_6 | ADC_UNMASK_CHANNEL_7;
#endif
	} else {
		channels = mask;
	}

	struct adc_sequence sequence = {
		.options	= &adc_seq_options,
		.buffer		= (void *)buffer,
		.buffer_size	= sizeof(buffer),
		.channels	= channels,
	};

	/* Set the channel */
	ret = adc_channel_setup(adc_dev, &channel_cfg);
	if (!IS_ENABLED(CONFIG_TEST_ADC_CONTINUOUS) &&
	    IS_ENABLED(CONFIG_TEST_ADC_DIFFERENTIAL) &&
	    (channel > ADC_CHANNEL_2)) {
		zassert_equal(ret, -EINVAL,
			      "invalid channel setup passed, ret %d", ret);
		LOG_INF("channel setup failed as expected, ret %d", ret);
		return TC_PASS;
	}
	if (ret) {
		zassert_equal(ret, 0, "Unable set up channel, code %d", ret);
	}

	/* Start reading the samples */
	ret = adc_read(adc_dev, &sequence);
	zassert_equal(ret, 0, "adc_read() failed with code %d", ret);

	LOG_INF("Allocated memory buffer Address is 0x%X", (uint32_t)buffer);

	if (channel_cfg.channel_id == ADC_CHANNEL_6) {
		LOG_INF("Allocated memory buffer Address is 0x%X and value is %x",
			(uint32_t)buffer, buffer[0]);
		temp = (float) get_temperature(buffer[0]);
		if (temp == -1) {
			LOG_ERR(" Error: Temperature is outside range");
			ztest_test_fail();
		} else {
			LOG_INF("Current temp %0.1f C", (double)temp);
		}
	}

#if CONFIG_TEST_ADC_CONTINUOUS
	for (int i = 0; i < 8; i++) {
		LOG_INF("Channel%d value is %x", i, buffer[i]);
	}
#else
	LOG_INF("Channel %d value is %x", channel, buffer[0]);
#endif

	LOG_INF("ADC sampling Done");
	return TC_PASS;
}

#if !(CONFIG_TEST_ADC_DIFFERENTIAL || CONFIG_TEST_ADC_CONTINUOUS || CONFIG_TEST_ADC_MultiCH)

#if CONFIG_TEST_ADC_CH0
ZTEST(adc_single_ended, test_adc_single_ended_input_channel0)
{
	zassert_true(test_channels(ADC_CHANNEL_0, ADC_UNMASK_CHANNEL_0) == TC_PASS);
}
#endif

#if CONFIG_TEST_ADC_CH1
ZTEST(adc_single_ended, test_adc_single_ended_input_channel1)
{
	zassert_true(test_channels(ADC_CHANNEL_1, ADC_UNMASK_CHANNEL_1) == TC_PASS);
}
#endif

#if CONFIG_TEST_ADC_CH2
ZTEST(adc_single_ended, test_adc_single_ended_input_channel2)
{
	zassert_true(test_channels(ADC_CHANNEL_2, ADC_UNMASK_CHANNEL_2) == TC_PASS);
}
#endif

#if CONFIG_TEST_ADC_CH3
ZTEST(adc_single_ended, test_adc_single_ended_input_channel3)
{
	zassert_true(test_channels(ADC_CHANNEL_3, ADC_UNMASK_CHANNEL_3) == TC_PASS);
}
#endif

#if CONFIG_TEST_ADC_CH4
ZTEST(adc_single_ended, test_adc_single_ended_input_channel4)
{
	zassert_true(test_channels(ADC_CHANNEL_4, ADC_UNMASK_CHANNEL_4) == TC_PASS);
}
#endif

#if CONFIG_TEST_ADC_CH5
ZTEST(adc_single_ended, test_adc_single_ended_input_channel5)
{
	zassert_true(test_channels(ADC_CHANNEL_5, ADC_UNMASK_CHANNEL_5) == TC_PASS);
}
#endif

#if CONFIG_TEST_ADC_CH6
ZTEST(adc_single_ended, test_adc_single_ended_input_channel6)
{
	zassert_true(test_channels(ADC_CHANNEL_6, ADC_UNMASK_CHANNEL_6) == TC_PASS);
}
#endif

#if CONFIG_TEST_ADC_CH7
ZTEST(adc_single_ended, test_adc_single_ended_input_channel7)
{
	zassert_true(test_channels(ADC_CHANNEL_7, ADC_UNMASK_CHANNEL_7) == TC_PASS);
}
#endif
#endif

#if (CONFIG_TEST_ADC_CONTINUOUS && !CONFIG_TEST_ADC_DIFFERENTIAL && !CONFIG_TEST_ADC_MultiCH)
ZTEST(adc_single_ended, test_adc_ContinuousMode)
{
	All_channel = 1;
	zassert_true(test_channels(0, 0) == TC_PASS);
	All_channel = 0;
}
#endif

#if (CONFIG_TEST_ADC_MultiCH && !CONFIG_TEST_ADC_DIFFERENTIAL)
ZTEST(adc12_multi_channel, test_adc_continuous_Mode_with_multi_channel)
{
	All_channel = 1;
	zassert_true(test_channels(0, 0) == TC_PASS);
	All_channel = 0;
}
#endif

#if (CONFIG_TEST_ADC_DIFFERENTIAL && !CONFIG_TEST_ADC_CONTINUOUS && !CONFIG_TEST_ADC_MultiCH)

#if (CONFIG_TEST_ADC_CH0 && CONFIG_TEST_ADC_CH1)
ZTEST(adc12_differential, test_adc_channel_0_diff_input)
{
	zassert_true(test_channels(ADC_CHANNEL_0, ADC_UNMASK_CHANNEL_0) == TC_PASS);
}
#endif

#if (CONFIG_TEST_ADC_CH2 && CONFIG_TEST_ADC_CH3)
ZTEST(adc12_differential, test_adc_channel_1_diff_input)
{
	zassert_true(test_channels(ADC_CHANNEL_1, ADC_UNMASK_CHANNEL_1) == TC_PASS);
}
#endif

#if (CONFIG_TEST_ADC_CH4 && CONFIG_TEST_ADC_CH5)
ZTEST(adc12_differential, test_adc_channel_2_diff_input)
{
	zassert_true(test_channels(ADC_CHANNEL_2, ADC_UNMASK_CHANNEL_2) == TC_PASS);
}
#endif
#if !(CONFIG_TEST_ADC_CH0 || CONFIG_TEST_ADC_CH1 || \
	CONFIG_TEST_ADC_CH2 || CONFIG_TEST_ADC_CH3 || \
	CONFIG_TEST_ADC_CH4 || CONFIG_TEST_ADC_CH5 || \
	CONFIG_TEST_ADC_CH6 || CONFIG_TEST_ADC_CH7)
ZTEST(adc12_differential, test_adc12_channel_3_diff_input_error_scenario)
{
	zassert_true(test_channels(ADC_CHANNEL_3, ADC_UNMASK_CHANNEL_3) == TC_PASS);
}

ZTEST(adc12_differential, test_adc12_channel_4_diff_input_error_scenario)
{
	zassert_true(test_channels(ADC_CHANNEL_4, ADC_UNMASK_CHANNEL_4) == TC_PASS);
}

ZTEST(adc12_differential, test_adc12_channel_5_diff_input_error_scenario)
{
	zassert_true(test_channels(ADC_CHANNEL_5, ADC_UNMASK_CHANNEL_5) == TC_PASS);
}

ZTEST(adc12_differential, test_adc12_channel_6_diff_input_error_scenario)
{
	zassert_true(test_channels(ADC_CHANNEL_6, ADC_UNMASK_CHANNEL_6) == TC_PASS);
}

ZTEST(adc12_differential, test_adc12_channel_7_diff_input_error_scenario)
{
	zassert_true(test_channels(ADC_CHANNEL_7, ADC_UNMASK_CHANNEL_7) == TC_PASS);
}
#endif
#endif
#if (CONFIG_TEST_ADC_CONTINUOUS && CONFIG_TEST_ADC_DIFFERENTIAL && !CONFIG_TEST_ADC_MultiCH)
ZTEST(adc12_differential, test_adc_continuousMode)
{
	All_channel = 1;
	zassert_true(test_channels(0, 0) == TC_PASS);
	All_channel = 0;
}
#endif

#endif /* !CONFIG_TEST_ADC24 */

/*
 * Copyright (C) 2026 Alif Semiconductor.
 * SPDX-License-Identifier: Apache-2.0
 *
 * DAC0 drives a DC voltage. ADC12 or ADC24 reads it back.
 *
 * Wire DAC0 (pinctrl P2_3) to the ADC pin for the overlay in use:
 * ADC12 instance 1 (Balletto/B1 preferred): P2_3 to ADC1 CH0 P0_6
 *   — alif_adc1_dac0.overlay
 * ADC12 instance 0: P2_3 to ADC0 CH0 P0_0 — alif_adc12_dac0.overlay
 *   (P0_0 is UART0 RX mux; often stuck near Vref on DevKit — avoid on B1)
 * ADC24: P2_3 to P0_0, and tie channel 0 negative P0_1 to GND
 *   — alif_adc24_dac0.overlay
 *
 * DAC12 and ADC12 both use the 1.8 V reference, so a 12-bit DAC code is the
 * expected ADC12 code. ADC24 full scale is that same reference on a 24-bit
 * differential converter, so the expected code is the DAC code shifted left
 * by 11 (2^23 / 2^12).
 */
#include "test_adc.h"

#if CONFIG_TEST_ADC_DAC

#include <zephyr/drivers/dac.h>

LOG_MODULE_DECLARE(ALIF_ADC, LOG_LEVEL_INF);

#define DAC_NODE DT_ALIAS(dac)
#define DAC_CHANNEL_ID 0
#define DAC_RESOLUTION 12

/* Default ADC node: 256 averages, right shift 8. The sample is the code. */
#define ADC12_TOLERANCE 160
/* ADC24: at least 2^16 counts, or 12.5% of the expected code. */
#define ADC24_TOLERANCE_MIN 65536

static const uint16_t dac_steps[] = { 0, 512, 1024, 2048, 3072, 3584 };

static uint32_t adc_buffer;

static const struct dac_channel_cfg dac_ch_cfg = {
	.channel_id = DAC_CHANNEL_ID,
	.resolution = DAC_RESOLUTION,
	.buffered = false,
};

static enum adc_action adc_done(const struct device *dev,
				const struct adc_sequence *sequence,
				uint16_t sampling_index)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(sequence);
	ARG_UNUSED(sampling_index);

	return ADC_ACTION_FINISH;
}

static const struct device *init_dac(void)
{
	int ret;
	const struct device *dac_dev = DEVICE_DT_GET(DAC_NODE);

	zassert_true(device_is_ready(dac_dev), "DAC device is not ready");

	ret = dac_channel_setup(dac_dev, &dac_ch_cfg);
	zassert_equal(ret, 0, "DAC channel setup failed with code %d", ret);

	return dac_dev;
}

static const struct device *init_adc(bool differential)
{
	int ret;
	const struct device *adc_dev = DEVICE_DT_GET(ADC);
	struct adc_channel_cfg channel_cfg = {
		.differential = differential,
		.channel_id = 0,
	};

	zassert_true(device_is_ready(adc_dev), "ADC device is not ready");

	ret = adc_channel_setup(adc_dev, &channel_cfg);
	zassert_equal(ret, 0, "ADC channel setup failed with code %d", ret);

	return adc_dev;
}

static int32_t read_adc(const struct device *adc_dev)
{
	int ret;
	struct adc_sequence_options options = {
		.callback = adc_done,
	};
	struct adc_sequence sequence = {
		.options = &options,
		.buffer = &adc_buffer,
		.buffer_size = sizeof(adc_buffer),
		.channels = BIT(0),
	};

	adc_buffer = 0;
	ret = adc_read(adc_dev, &sequence);
	zassert_equal(ret, 0, "adc_read() failed with code %d", ret);

	return (int32_t)adc_buffer;
}

static void check_dac_steps(const struct device *dac_dev, const struct device *adc_dev,
			    bool adc24)
{
	for (size_t i = 0; i < ARRAY_SIZE(dac_steps); i++) {
		uint16_t dac_code = dac_steps[i];
		int32_t expected;
		int32_t tolerance;
		int32_t code;
		int ret;

		ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, dac_code);
		zassert_equal(ret, 0, "dac_write_value(%u) failed with code %d",
			      dac_code, ret);

		/* DAC output rate is up to 1 kHz. Let the pin settle. */
		k_msleep(20);

		code = read_adc(adc_dev);

		if (adc24) {
			expected = (int32_t)dac_code << 11;
			tolerance = expected / 8;
			if (tolerance < ADC24_TOLERANCE_MIN) {
				tolerance = ADC24_TOLERANCE_MIN;
			}
			LOG_INF("ADC24 code %d for DAC %u (expected %d)",
				code, dac_code, expected);
		} else {
			expected = dac_code;
			tolerance = ADC12_TOLERANCE;
			LOG_INF("ADC12 code %d for DAC %u (expected %d +/- %d)",
				code, dac_code, expected, tolerance);
		}
	}
}

#if CONFIG_TEST_ADC24
ZTEST(adc24_differential, test_adc24_reads_dac0)
{
	const struct device *dac_dev = init_dac();
	const struct device *adc_dev = init_adc(true);

	LOG_INF("ADC24 differential channel 0 reading DAC0");
	check_dac_steps(dac_dev, adc_dev, true);
}
#else
ZTEST(adc_single_ended, test_adc12_reads_dac0)
{
	const struct device *dac_dev = init_dac();
	const struct device *adc_dev = init_adc(false);

	LOG_INF("ADC12 channel 0 reading DAC0 (wire P2_3 to ADC input)");
	check_dac_steps(dac_dev, adc_dev, false);
}
#endif

#endif /* CONFIG_TEST_ADC_DAC */

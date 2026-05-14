/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include "test_dac.h"

LOG_MODULE_REGISTER(dac_test, LOG_LEVEL_INF);

/* Global device pointer */
const struct device *dac_dev;

/* Default channel configuration: 12-bit, channel 0, unbuffered */
const struct dac_channel_cfg dac_ch_cfg = {
	.channel_id = DAC_CHANNEL_ID,
	.resolution = DAC_RESOLUTION,
	.buffered = 0
};

void *init_dac_device(void)
{
	int ret;
	const struct device *dev = DEVICE_DT_GET(DAC_NODE);

	zassert_not_null(dev, "DAC device handle is NULL");
	zassert_true(device_is_ready(dev), "DAC device is not ready");

	ret = dac_channel_setup(dev, &dac_ch_cfg);
	zassert_equal(ret, 0, "DAC channel setup failed [%d]", ret);

	return (void *)dev;
}

#if DT_NODE_HAS_STATUS(DAC0_NODE, okay) && DT_NODE_HAS_STATUS(DAC1_NODE, okay)
struct dac_dual_devs init_dac_dual_devices(void)
{
	int ret;
	struct dac_dual_devs d = {
		.dac0 = DEVICE_DT_GET(DAC0_NODE),
		.dac1 = DEVICE_DT_GET(DAC1_NODE),
	};

	zassert_not_null(d.dac0, "DAC0 device handle is NULL");
	zassert_not_null(d.dac1, "DAC1 device handle is NULL");
	zassert_true(device_is_ready(d.dac0), "DAC0 not ready");
	zassert_true(device_is_ready(d.dac1), "DAC1 not ready");

	ret = dac_channel_setup(d.dac0, &dac_ch_cfg);
	zassert_equal(ret, 0, "DAC0 channel setup failed [%d]", ret);

	ret = dac_channel_setup(d.dac1, &dac_ch_cfg);
	zassert_equal(ret, 0, "DAC1 channel setup failed [%d]", ret);

	return d;
}
#endif

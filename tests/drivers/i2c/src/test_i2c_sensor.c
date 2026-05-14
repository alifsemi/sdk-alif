/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/*
 * Generic I2C sensor integration tests.
 *
 * Tests are compiled automatically for every sensor whose compatible
 * string has status "okay" in the merged devicetree.  To add a new
 * sensor, just:
 *
 *   1. Add a #if DT_HAS_COMPAT_STATUS_OKAY(vendor_part) block below
 *      with the sensor-specific ZTEST test function.
 *
 *   2. Add a testcase entry in testcase.yaml that applies the overlay:
 *        drivers.i2c.<sensor>:
 *          tags: [drivers, i2c, sensor]
 *          depends_on: [i2c, sensor]
 *          filter: dt_alias_exists("i2c_controller")
 *          extra_dtc_overlay_files:
 *            - boards/i2c_<sensor>.overlay
 *          extra_configs:
 *            - CONFIG_I2C_SENSOR_TESTS=y
 *            - CONFIG_SENSOR=y
 */

#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
#include <zephyr/ztest.h>
#include <zephyr/dt-bindings/i2c/i2c.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>


LOG_MODULE_REGISTER(i2c_sensor, LOG_LEVEL_INF);

/* ------------------------------------------------------------------ */
/* BME680 - temperature, pressure, humidity, gas resistance           */
/* ------------------------------------------------------------------ */
#if DT_HAS_COMPAT_STATUS_OKAY(bosch_bme680)
ZTEST(i2c_sensor, test_bme680_read)
{
	const struct device *dev = DEVICE_DT_GET_ONE(bosch_bme680);
	struct sensor_value temp, press, humidity, gas_res;
	int i = 0;
	int ret;

	zassert_true(device_is_ready(dev), "BME680 device not ready");
	LOG_INF("Device %p name is %s", dev, dev->name);

	while (i < 10) {
		k_sleep(K_MSEC(3000));
		ret = sensor_sample_fetch(dev);
		zassert_ok(ret, "sensor_sample_fetch failed: %d", ret);
		ret = sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &temp);
		zassert_ok(ret, "Failed to read temperature: %d", ret);
		ret = sensor_channel_get(dev, SENSOR_CHAN_PRESS, &press);
		zassert_ok(ret, "Failed to read pressure: %d", ret);
		ret = sensor_channel_get(dev, SENSOR_CHAN_HUMIDITY, &humidity);
		zassert_ok(ret, "Failed to read humidity: %d", ret);
		ret = sensor_channel_get(dev, SENSOR_CHAN_GAS_RES, &gas_res);
		zassert_ok(ret, "Failed to read gas resistance: %d", ret);
		LOG_INF("T: %d.%06d; P: %d.%06d; H: %d.%06d; G: %d.%06d",
			temp.val1, temp.val2, press.val1, press.val2,
			humidity.val1, humidity.val2, gas_res.val1,
			gas_res.val2);
		i++;
	}
}
#endif
/* ------------------------------------------------------------------ */
/* Add new sensors here, for example:                                 */
/*                                                                    */
/* #if DT_HAS_COMPAT_STATUS_OKAY(bosch_bme280)                        */
/* ZTEST(i2c_sensor, test_bme280_read)                                */
/* {                                                                  */
/*     const struct device *dev = DEVICE_DT_GET_ONE(bosch_bme280);    */
/*     ...                                                            */
/* }                                                                  */
/* #endif                                                             */
/* ------------------------------------------------------------------ */

ZTEST_SUITE(i2c_sensor, NULL, NULL, NULL, NULL, NULL);

/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/ztest.h>
#include <zephyr/drivers/i3c.h>
#include <zephyr/drivers/i3c/ccc.h>
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include "i3c_common.h"

#ifdef CONFIG_I3C_SENSOR_TESTS
#include <zephyr/drivers/sensor.h>
#endif

LOG_MODULE_REGISTER(i3c_sensor, LOG_LEVEL_INF);

static const struct device *ctrl;

#ifdef CONFIG_I3C_SENSOR_TESTS
#define BMI323_NODE   DT_NODELABEL(bmi323)
#define ICM42670_NODE DT_NODELABEL(icm42670)
static const struct device *bmi323_dev;
static const struct device *icm42670_dev;
#endif

/**
 * @brief Sensor test suite setup function
 *
 * Initializes I3C controller and resets bus to SDR mode.
 * Performs DAA to ensure targets have dynamic addresses assigned.
 *
 * @return NULL on success, asserts on failure
 */
static void *suite_setup(void)
{
	ctrl = DEVICE_DT_GET(I3C_CONTROLLER_NODE);
	zassert_true(device_is_ready(ctrl), "I3C controller not ready");

	/*
	 * Do NOT reset I3C bus here - use boot-assigned addresses.
	 * RSTDAA would invalidate the cached addresses in sensor drivers.
	 */
	k_msleep(50);

#ifdef CONFIG_I3C_SENSOR_TESTS
	bmi323_dev = NULL;
	icm42670_dev = NULL;
#if DT_NODE_EXISTS(BMI323_NODE)
	bmi323_dev = DEVICE_DT_GET(BMI323_NODE);
	LOG_INF("BMI323 device: %s", bmi323_dev->name);
#endif
#if DT_NODE_EXISTS(ICM42670_NODE) && defined(CONFIG_ICM42X70)
	icm42670_dev = DEVICE_DT_GET(ICM42670_NODE);
	LOG_INF("ICM42670 device: %s", icm42670_dev->name);
#endif
#endif /* CONFIG_I3C_SENSOR_TESTS */

	return NULL;
}

static void test_before(void *fixture)
{
	ARG_UNUSED(fixture);

	/*
	 * Note: Sensor drivers cache I3C addresses at boot in i3c_dt_spec.
	 * Do NOT run RSTDAA+DAA here — it would change device addresses
	 * and break sensor driver communication. Use boot-assigned
	 * addresses (from SETDASA) directly.
	 */
	k_msleep(50);
}

static void test_after(void *fixture)
{
	ARG_UNUSED(fixture);
}

ZTEST_SUITE(i3c_sensor, NULL, suite_setup, test_before, test_after, NULL);

#ifdef CONFIG_I3C_SENSOR_TESTS

/**
 * @brief Test BMI323 sensor data read functionality
 *
 * Fetches accelerometer, gyroscope and temperature data from BMI323
 * for 5 iterations. Verifies sensor communication over I3C bus.
 */
ZTEST(i3c_sensor, test_bmi323_data_read)
{
	struct sensor_value full_scale, sampling_freq, feat_mask;
	struct sensor_value accel[3], gyro[3], temp;
	int success_count = 0;
	int fail_count = 0;
	int ret;

	if (bmi323_dev == NULL) {
		LOG_WRN("BMI323 device pointer is NULL");
		ztest_test_skip();
		return;
	}

	LOG_INF("Checking BMI323 device readiness...");
	zassert_true(device_is_ready(bmi323_dev), "BMI323 not ready");

	/* Configure accelerometer: 2G range, high-performance mode, 100Hz */
	full_scale.val1 = 2;    full_scale.val2 = 0;
	feat_mask.val1  = 1;    feat_mask.val2  = 0;
	sampling_freq.val1 = 100; sampling_freq.val2 = 0;

	ret = sensor_attr_set(bmi323_dev, SENSOR_CHAN_ACCEL_XYZ,
			      SENSOR_ATTR_FULL_SCALE, &full_scale);
	zassert_ok(ret, "BMI323 accel full_scale failed: %d", ret);

	ret = sensor_attr_set(bmi323_dev, SENSOR_CHAN_ACCEL_XYZ,
			      SENSOR_ATTR_FEATURE_MASK, &feat_mask);
	zassert_ok(ret, "BMI323 accel feature_mask failed: %d", ret);

	ret = sensor_attr_set(bmi323_dev, SENSOR_CHAN_ACCEL_XYZ,
			      SENSOR_ATTR_SAMPLING_FREQUENCY, &sampling_freq);
	zassert_ok(ret, "BMI323 accel sampling_freq failed: %d", ret);

	/* Configure gyroscope: 500 dps, high-performance mode, 100Hz */
	full_scale.val1 = 500;

	ret = sensor_attr_set(bmi323_dev, SENSOR_CHAN_GYRO_XYZ,
			      SENSOR_ATTR_FULL_SCALE, &full_scale);
	zassert_ok(ret, "BMI323 gyro full_scale failed: %d", ret);

	ret = sensor_attr_set(bmi323_dev, SENSOR_CHAN_GYRO_XYZ,
			      SENSOR_ATTR_FEATURE_MASK, &feat_mask);
	zassert_ok(ret, "BMI323 gyro feature_mask failed: %d", ret);

	ret = sensor_attr_set(bmi323_dev, SENSOR_CHAN_GYRO_XYZ,
			      SENSOR_ATTR_SAMPLING_FREQUENCY, &sampling_freq);
	zassert_ok(ret, "BMI323 gyro sampling_freq failed: %d", ret);

	/* Wait for sensor to enter high-performance mode */
	k_msleep(50);

	LOG_INF("Testing BMI323 - 20 iterations");

	for (int iter = 0; iter < 20; iter++) {
		ret = sensor_sample_fetch(bmi323_dev);
		if (ret < 0) {
			LOG_ERR("BMI323 sample fetch failed iter %d: %d",
				iter + 1, ret);
			fail_count++;
			continue;
		}
		success_count++;

		/* Read accelerometer with zassert */
		ret = sensor_channel_get(bmi323_dev,
					 SENSOR_CHAN_ACCEL_XYZ, accel);
		zassert_ok(ret, "Accel read failed iter %d: %d", iter + 1, ret);
		LOG_INF("Iter %d: Accel X=%d.%06d Y=%d.%06d Z=%d.%06d",
			iter + 1,
			accel[0].val1, accel[0].val2,
			accel[1].val1, accel[1].val2,
			accel[2].val1, accel[2].val2);

		/* Read gyroscope with zassert */
		ret = sensor_channel_get(bmi323_dev,
					 SENSOR_CHAN_GYRO_XYZ, gyro);
		zassert_ok(ret, "Gyro read failed iter %d: %d", iter + 1, ret);
		LOG_INF("Iter %d: Gyro X=%d.%06d Y=%d.%06d Z=%d.%06d",
			iter + 1,
			gyro[0].val1, gyro[0].val2,
			gyro[1].val1, gyro[1].val2,
			gyro[2].val1, gyro[2].val2);

		/* Temperature (optional) */
		ret = sensor_channel_get(bmi323_dev,
					 SENSOR_CHAN_DIE_TEMP, &temp);
		if (ret == 0) {
			LOG_INF("Iter %d: Temp %d.%06d C",
				iter + 1, temp.val1, temp.val2);
		}

		k_msleep(20);
	}

	LOG_INF("BMI323 completed: %d success, %d failures",
		success_count, fail_count);

	zassert_equal(success_count, 20, "BMI323 failed %d/%d iterations",
		      fail_count, 20);
}

#ifdef CONFIG_ICM42X70
/**
 * @brief Test ICM42670 sensor data read functionality
 *
 * ICM42670 is configured at boot from DT (accel-hz/fs, gyro-hz/fs).
 * Fetches accelerometer, gyroscope and temperature for 20 iterations.
 */
ZTEST(i3c_sensor, test_icm42670_data_read)
{
	struct sensor_value accel[3], gyro[3], temp;
	int success_count = 0;
	int fail_count = 0;
	int ret;

	if (icm42670_dev == NULL) {
		LOG_WRN("ICM42670 device pointer is NULL");
		ztest_test_skip();
		return;
	}

	LOG_INF("Checking ICM42670 device readiness...");
	if (!device_is_ready(icm42670_dev)) {
		LOG_WRN("ICM42670 not ready, skipping");
		ztest_test_skip();
		return;
	}

	LOG_INF("Testing ICM42670 - 20 iterations");

	for (int iter = 0; iter < 20; iter++) {
		ret = sensor_sample_fetch(icm42670_dev);
		if (ret < 0) {
			LOG_ERR("ICM42670 sample fetch failed iter %d: %d",
				iter + 1, ret);
			fail_count++;
			continue;
		}
		success_count++;

		ret = sensor_channel_get(icm42670_dev,
					 SENSOR_CHAN_ACCEL_XYZ, accel);
		zassert_ok(ret, "ICM42670 accel read failed iter %d: %d",
			   iter + 1, ret);
		LOG_INF("Iter %d: Accel X=%d.%06d Y=%d.%06d Z=%d.%06d",
			iter + 1,
			accel[0].val1, accel[0].val2,
			accel[1].val1, accel[1].val2,
			accel[2].val1, accel[2].val2);

		ret = sensor_channel_get(icm42670_dev,
					 SENSOR_CHAN_GYRO_XYZ, gyro);
		zassert_ok(ret, "ICM42670 gyro read failed iter %d: %d",
			   iter + 1, ret);
		LOG_INF("Iter %d: Gyro X=%d.%06d Y=%d.%06d Z=%d.%06d",
			iter + 1,
			gyro[0].val1, gyro[0].val2,
			gyro[1].val1, gyro[1].val2,
			gyro[2].val1, gyro[2].val2);

		ret = sensor_channel_get(icm42670_dev,
					 SENSOR_CHAN_DIE_TEMP, &temp);
		if (ret == 0) {
			LOG_INF("Iter %d: Temp %d.%06d C",
				iter + 1, temp.val1, temp.val2);
		}

		k_msleep(20);
	}

	LOG_INF("ICM42670 completed: %d success, %d failures",
		success_count, fail_count);

	zassert_equal(success_count, 20, "ICM42670 failed %d/%d iterations",
		      fail_count, 20);
}
#endif /* CONFIG_ICM42X70 */

#endif /* CONFIG_I3C_SENSOR_TESTS */

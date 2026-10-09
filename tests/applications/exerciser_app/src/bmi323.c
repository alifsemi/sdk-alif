#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <stdio.h>
#include <zephyr/logging/log.h>
#include "exerciser_app.h"

LOG_MODULE_REGISTER(exerciser_bmi323, LOG_LEVEL_INF);

#if BMI323_NODE_OKAY
void bmi323_thread(void)
{
	const struct device *const dev = DEVICE_DT_GET_ONE(bosch_bmi323);
	struct sensor_value acc[3], gyr[3];
	struct sensor_value full_scale, sampling_freq, feat_mask;

	if (!device_is_ready(dev)) {
		LOG_ERR("Device %s is not ready", dev->name);
		return;
	}

	LOG_INF("Device %p name is %s", dev, dev->name);

	/* Setting scale in G, due to loss of precision if the SI unit m/s^2
	 * is used
	 */
	full_scale.val1 = 2;            /* G */
	full_scale.val2 = 0;
	sampling_freq.val1 = 100;       /* Hz. Performance mode */
	sampling_freq.val2 = 0;
	feat_mask.val1 = 1;          /* High performance mode */
	feat_mask.val2 = 0;

	sensor_attr_set(dev, SENSOR_CHAN_ACCEL_XYZ, SENSOR_ATTR_FULL_SCALE,
			&full_scale);
	sensor_attr_set(dev, SENSOR_CHAN_ACCEL_XYZ, SENSOR_ATTR_FEATURE_MASK,
			&feat_mask);
	/* Set sampling frequency last as this also sets the appropriate
	 * power mode. If already sampling, change to 0.0Hz before changing
	 * other attributes
	 */
	sensor_attr_set(dev, SENSOR_CHAN_ACCEL_XYZ,
			SENSOR_ATTR_SAMPLING_FREQUENCY,
			&sampling_freq);

	/* Setting scale in degrees/s to match the sensor scale */
	full_scale.val1 = 500;          /* dps */
	full_scale.val2 = 0;
	sampling_freq.val1 = 100;       /* Hz. Performance mode */
	sampling_freq.val2 = 0;
	feat_mask.val1 = 1;          /* High performance mode */
	feat_mask.val2 = 0;

	sensor_attr_set(dev, SENSOR_CHAN_GYRO_XYZ, SENSOR_ATTR_FULL_SCALE,
			&full_scale);
	sensor_attr_set(dev, SENSOR_CHAN_GYRO_XYZ, SENSOR_ATTR_FEATURE_MASK,
			&feat_mask);
	/* Set sampling frequency last as this also sets the appropriate
	 * power mode. If already sampling, change sampling frequency to
	 * 0.0Hz before changing other attributes
	 */
	sensor_attr_set(dev, SENSOR_CHAN_GYRO_XYZ,
			SENSOR_ATTR_SAMPLING_FREQUENCY,
			&sampling_freq);

	while (1) {
		/* 100ms period, 100Hz Sampling frequency */
		k_sleep(K_MSEC(500));

		ret = sensor_sample_fetch(dev);
		if (ret < 0) {
			LOG_ERR("BMI323 sample fetch failed: %d", ret);
			continue;
		}

		ret = sensor_channel_get(dev, SENSOR_CHAN_ACCEL_XYZ, acc);
		if (ret < 0) {
			LOG_ERR("BMI323 accel read failed: %d", ret);
			continue;
		}

		ret = sensor_channel_get(dev, SENSOR_CHAN_GYRO_XYZ, gyr);
		if (ret < 0) {
			LOG_ERR("BMI323 gyro read failed: %d", ret);
			continue;
		}

		LOG_INF("Accel AX: %f; AY: %f; AZ: %f g Gyro GX: %f; GY: %f; GZ: %f deg/s",
			sensor_value_to_double(&acc[0]),
			sensor_value_to_double(&acc[1]),
			sensor_value_to_double(&acc[2]),
			sensor_value_to_double(&gyr[0]),
			sensor_value_to_double(&gyr[1]),
			sensor_value_to_double(&gyr[2]));
	}
}
#endif /* BMI323_NODE_OKAY */

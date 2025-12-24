/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <ch201.h>

/* Maximum detection range for the sensor in mm */
#define CH201_DETECTION_RANGE_MM      (2000)

/* Measurement frequency is 1 Second
 * Note: The max frequency can be set is 1000Hz
 */
#define CH201_MEASUREMENT_FREQUENCY   (1)

/* Target detection thresholds */
/* threshold 0 - starts at zero as per datasheet*/
#define CH201_THRESH_0_START    (0)
#define CH201_THRESH_0_LEVEL    (5000)

#define CH201_THRESH_1_START    (26)
#define CH201_THRESH_1_LEVEL    (2000)

#define CH201_THRESH_2_START    (39)
#define CH201_THRESH_2_LEVEL    (800)

#define CH201_THRESH_3_START    (56)
#define CH201_THRESH_3_LEVEL    (400)

#define CH201_THRESH_4_START    (79)
#define CH201_THRESH_4_LEVEL    (250)

#define CH201_THRESH_5_START    (89)
#define CH201_THRESH_5_LEVEL    (175)

LOG_MODULE_REGISTER(CH201_TOF, LOG_LEVEL_INF);

struct sensor_ch201_threshold thresholds[CH201_MAX_SUPPORTED_THRESHOLDS] = {
	{CH201_THRESH_0_START, CH201_THRESH_0_LEVEL},      /* threshold 0 */
	{CH201_THRESH_1_START, CH201_THRESH_1_LEVEL},      /* threshold 1 */
	{CH201_THRESH_2_START, CH201_THRESH_2_LEVEL},      /* threshold 2 */
	{CH201_THRESH_3_START, CH201_THRESH_3_LEVEL},      /* threshold 3 */
	{CH201_THRESH_4_START, CH201_THRESH_4_LEVEL},      /* threshold 4 */
	{CH201_THRESH_5_START, CH201_THRESH_5_LEVEL},      /* threshold 5 */
};

/* Timer trigger	*/
static struct k_timer ch201_timer;
static volatile bool timer_triggered;
static volatile bool ch201_data_ready;

static void ch201_periodic_timer_trigger(struct k_timer *timer_id)
{
	timer_triggered = true;
}

static void ch201_data_ready_handler(const struct device *dev,
				const struct sensor_trigger *trigger)
{
	ch201_data_ready = true;
}

int main(void)
{
	const struct device *const dev = DEVICE_DT_GET_ONE(invensense_ch201);
	struct sensor_value ch201_data[2];
	struct sensor_value config_val;
	struct sensor_trigger ch201_trig;
	int ret;

	if (!device_is_ready(dev)) {
		LOG_ERR("Device %s is not ready\n", dev->name);
		return 0;
	}

	k_msleep(10);

	LOG_INF("Device %p name is %s\n", dev, dev->name);

	/* Initialize the timer */
	k_timer_init(&ch201_timer, ch201_periodic_timer_trigger, NULL);
	timer_triggered = false;

	ch201_data_ready = false;
	ch201_trig.type  = SENSOR_TRIG_DATA_READY;
	ch201_trig.chan = SENSOR_CHAN_ALL;
	sensor_trigger_set(dev, &ch201_trig, ch201_data_ready_handler);

	/* Set measurement frequency */
	config_val.val1 = CH201_MEASUREMENT_FREQUENCY;
	ret = sensor_attr_set(dev, SENSOR_CHAN_ALL,
			SENSOR_ATTR_SAMPLING_FREQUENCY, &config_val);
	if (ret != 0) {
		LOG_ERR("SENSOR_ATTR_SAMPLING_FREQUENCY set failed\n");
	}

	/* Set ampliture threshold values */
	ret = sensor_attr_set(dev, SENSOR_CHAN_ALL, SENSOR_CH201_ATTR_THRESHOLD_TABLE,
			(struct sensor_value *)&thresholds);
	if (ret != 0) {
		LOG_ERR("SENSOR_CH201_ATTR_THRESHOLD_TABLE set failed\n");
	}

	/* Set detection range max */
	config_val.val1 = CH201_DETECTION_RANGE_MM;
	ret = sensor_attr_set(dev, SENSOR_CHAN_DISTANCE, SENSOR_ATTR_FULL_SCALE, &config_val);
	if (ret != 0) {
		LOG_ERR("SENSOR_ATTR_FULL_SCALE set failed\n");
	}

	/* Set sensing mode */
	config_val.val1 = SENSOR_CH201_MODE_TRIGGERED_TX_RX;
	ret = sensor_attr_set(dev, SENSOR_CHAN_ALL, SENSOR_CH201_ATTR_MODE, &config_val);
	if (ret != 0) {
		LOG_ERR("SENSOR_CH201_ATTR_MODE set failed\n");
	}

	/* Enable sensing */
	config_val.val1 = 1;
	ret = sensor_attr_set(dev, SENSOR_CHAN_ALL, SENSOR_CH201_ATTR_ENABLE, &config_val);
	if (ret != 0) {
		LOG_ERR("SENSOR_CH201_ATTR_ENABLE set failed\n");
	}

	/* start timer triggering */
	k_timer_start(&ch201_timer, K_MSEC(CH201_MEASUREMENT_FREQUENCY * 1000),
			K_MSEC(CH201_MEASUREMENT_FREQUENCY * 1000));

	while (true) {
		if (timer_triggered) {
			timer_triggered = false;
			/* Fetch sample only in Tx-RX mode, not in Free-Running mode.
			 * The sensor triggers data ready interrupt
			 * once timeout happens in Free-Running mode.
			 */
			ret = sensor_sample_fetch(dev);
			if (ret != 0) {
				LOG_ERR("Sample fetch failed\n");
			}
		}
		/* Read data if available */
		if (ch201_data_ready) {
			sensor_channel_get(dev, SENSOR_CHAN_ALL, ch201_data);

			LOG_INF("\tObject Range --> %f m, Amplitude --> %u\n",
				sensor_value_to_double(&ch201_data[0]),
				ch201_data[1].val1);
			ch201_data_ready = false;
		}
	}
	return 0;
}


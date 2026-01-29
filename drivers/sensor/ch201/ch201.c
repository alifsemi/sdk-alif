/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <errno.h>
#include <zephyr/logging/log.h>

/* ch201 chirp sonic library includes */
#include "invn/soniclib/sensor_fw/ch201/ch201_gprmt.h"
#include "invn/soniclib/soniclib.h"

/* Project Includes */
#include "ch201_private.h"

/* micro-units per 1 unit */
#define MICROS_PER_UNIT (1000000LL)

/* milli-units per 1 unit */
#define MILLIS_PER_UNIT (1000LL)

/* CH201 GPRMT - multi threshold */
#define CHIRP_SENSOR_FW_INIT_FUNC   ch201_gprmt_init

/* Static target rejection range */
#define CHIRP_STATIC_REJECT_RANGE   (0)

/* Receive sensor pre-triggering */
#define CHIRP_RX_PRETRIGGER_ENABLE  (0)

/* Acceleration value in ug */
#define CH201_RANGE_VAL(x)          ((x * MILLIS_PER_UNIT) / 32.0)

/* macro to extract integer and fractional part of sensor data */
#define CH201_SENSOR_EXTRACT_INT_FRACT_PART(x, conv_val) {	\
	(x).val1 = (conv_val / MICROS_PER_UNIT);		\
	(x).val2 = (conv_val % MICROS_PER_UNIT);		\
}

#define CONV_FREQ_TO_MS(x)	    (1000 / x)
#define CONV_MS_TO_FREQ(x)	    (1000 * x)

LOG_MODULE_REGISTER(ch201_tdk, CONFIG_SENSOR_LOG_LEVEL);

/* Object to store current device's reference */
const struct device *ch201_cur_dev;

/**
 * @brief       Get current ch201 device's reference
 * @param[in]   dev   Pointer to refer the device address.
 * @return	None
 */
void ch201_get_cur_dev(const struct device **dev)
{
	*dev = ch201_cur_dev;
}

/**
 * @brief       Data ready Interrupt callback.
 * @param[in]   grp_ptr  Ch201 group pointer
 * @param[in]   dev_num  Ch201 device number
 * @param[in]   int_type Interrupt type
 * @return      None
 */
static void ch201_int_callback(ch_group_t *grp_ptr,
				uint8_t dev_num,
				ch_interrupt_type_t int_type)
{
	const struct device *cur_dev;
	struct ch201_data *data;

	ch201_get_cur_dev(&cur_dev);
	data = cur_dev->data;

	ARG_UNUSED(grp_ptr);
	ARG_UNUSED(dev_num);

	k_mutex_lock(&data->lock, K_FOREVER);

	if (int_type == CH_INTERRUPT_TYPE_DATA_RDY) {
		/* Set data-ready flag */
		data->data_ready = true;
		if (data->data_ready_handler) {
			data->data_ready_handler(cur_dev, &data->data_ready_trigger);
		}
	}

	k_mutex_unlock(&data->lock);
}

/**
 * @brief       Read the sampled data.
 * @param[in]   dev	Pointer to the device structure for the driver instance.
 * @param[in]   dev_ptr Ch201 device pointer
 * @param[in]   data    Sensor data
 * @return      None
 */
static void ch201_handle_data(const struct device *dev, ch_dev_t *dev_ptr,
				struct sensor_value *val)
{
	uint64_t loc_range;
	struct sensor_value loc_val;

	ARG_UNUSED(dev);

	loc_range = ch_get_range(dev_ptr, CH_RANGE_ECHO_ONE_WAY);

	if (loc_range != CH_NO_TARGET) {
		/* Target object was successfully detected (range available)
		 * Get the new amplitude value - it's only updated if range
		 * was successfully measured.
		 */
		loc_range = CH201_RANGE_VAL(loc_range);
		CH201_SENSOR_EXTRACT_INT_FRACT_PART(loc_val, loc_range);
		val[0].val1 = loc_val.val1;
		val[0].val2 = loc_val.val2;
		val[1].val1 = ch_get_amplitude(dev_ptr);

	} else {
		/* No target object was detected.
		 * no updated amplitude and range value
		 */
		val[1].val1 = 0;
	}
}

/**
 * @brief       Initializes CH201 Interface.
 * @param[in]   dev	Pointer to the sensor device instance.
 * @return      0 on success, negative value on failure
 */
static int ch201_init(const struct device *dev)
{
	struct ch201_data *data = dev->data;
	int ret;

	/* store current device reference */
	ch201_cur_dev = dev;

	ret = ch201_gpios_configure(dev);
	if (ret != 0) {
		LOG_ERR("ch201 driver init failed\n");
		return -ENODEV;
	}

	ch_group_init(&data->ch201_group, CHIRP_MAX_NUM_SENSORS,
			CHIRP_NUM_BUSES, CHIRP_RTC_CAL_PULSE_MS);

	/* check if the number of devices are same as configured */
	if (ch_get_num_ports(&data->ch201_group) != CHIRP_MAX_NUM_SENSORS) {
		LOG_ERR("Invalid number of devices\n");
		return -ENODEV;
	}

	ret = ch_init(&data->ch201_dev, &data->ch201_group,
			data->dev_num, CHIRP_SENSOR_FW_INIT_FUNC);
	if (ret != 0) {
		return -ENODEV;
	}

	ret = ch_group_start(&data->ch201_group);
	if (ret != 0) {
		return -ENODEV;
	}

	/* Register callback function to be called when CH201 sensor interrupts */
	ch_io_int_callback_set(&data->ch201_group, ch201_int_callback);

	/* Initializes driver state and data received status */
	data->grp_ptr        = NULL;
	data->int1_wait_mode = false;
	data->dev_num        = 0;

	/* Initialize mutex */
	k_mutex_init(&data->lock);

	return 0;
}

/**
 * @brief       Set sensor attributes
 * @param[in]   dev	Pointer to the sensor device instance.
 * @param[in]   chan	Sensor channel
 * @param[in]   attr	Sensor attribute
 * @param[in]   val	attribute value
 * @return      0 on success, negative value on failure
 */
static int ch201_attr_set(const struct device *dev, enum sensor_channel chan,
			enum sensor_attribute attr, const struct sensor_value *val)
{
	struct ch201_data *data = dev->data;
	uint8_t iter;
	int ret = 0;

	struct sensor_ch201_threshold *val_ptr;

	ch_dev_t *dev_ptr = ch_get_dev_ptr(&data->ch201_group,
						data->dev_num);
	if (!val) {
		return -EINVAL;
	}
	if (data->state != CH201_DRIVER_READY) {
		return -EINVAL;
	}

	if (!ch_sensor_is_connected(dev_ptr)) {
		return -EINVAL;
	}

	k_mutex_lock(&data->lock, K_FOREVER);

	switch (chan) {
	case SENSOR_CHAN_ALL:
		switch ((int) attr) {
		case SENSOR_ATTR_SAMPLING_FREQUENCY:
			/* Convert frequency to millisec */
			data->meas_interval_ms = CONV_FREQ_TO_MS(val->val1);
			break;
		case SENSOR_CH201_ATTR_MODE:
			if (val->val1 == SENSOR_CH201_MODE_TRIGGERED_TX_RX) {
				data->dev_config.mode = CH_MODE_TRIGGERED_TX_RX;
			} else if (val->val1 == SENSOR_CH201_MODE_FREERUN) {
				data->dev_config.mode = CH_MODE_FREERUN;
			} else {
				data->dev_config.mode = CH_MODE_IDLE;
			}
			break;
		case SENSOR_CH201_ATTR_THRESHOLD_TABLE:
			val_ptr = (struct sensor_ch201_threshold *)val;

			for (iter = 0; iter < CH_NUM_THRESHOLDS; iter++) {
				data->dev_threshold.threshold[iter].start_sample =
				val_ptr->start_sample;
				data->dev_threshold.threshold[iter].level = val_ptr->level;
				val_ptr++;
			}
			data->dev_algo_config.thresh_ptr =
			(ch_thresholds_t *) &data->dev_threshold;
			break;
		case SENSOR_CH201_ATTR_ENABLE:
			if (val->val1) {
				/* Perform below to start measuring */
				data->dev_algo_config.static_range = CHIRP_STATIC_REJECT_RANGE;
				/* Set threshold */
				ch_rangefinder_set_algo_config(dev_ptr, &data->dev_algo_config);

				/* Disable receive sensor pre-triggering */
				ch_set_rx_pretrigger(&data->ch201_group,
						CHIRP_RX_PRETRIGGER_ENABLE);

				if (data->dev_config.mode == CH_MODE_TRIGGERED_TX_RX) {
					/* Set the board's periodic timer trigger */
					data->dev_config.sample_interval = 0;

				} else {
					/* Configure sample interval */
					data->dev_config.sample_interval = data->meas_interval_ms;
					ch_set_sample_interval(&data->ch201_dev,
						data->dev_config.sample_interval);

					/* Sensor is in freerun mode, so configure INT line
					 * input to receive interrupt from sensor
					 */
					chdrv_int_group_set_dir_in(&data->ch201_group);
					chdrv_int_group_interrupt_enable(&data->ch201_group);
				}

				if (ch_set_config(&data->ch201_dev,
					&data->dev_config) != 0) {
					ret = -EIO;
					goto attr_set_err;
				}
			} else {
				/* Perform below to stop measuring */
				if (data->dev_config.mode == CH_MODE_FREERUN) {
					/* Disable INT interrupt */
					chdrv_int_group_interrupt_disable(&data->ch201_group);
				}
			}
			break;

		default:
			break;
		}
		break;

	case SENSOR_CHAN_DISTANCE:
		if ((int) attr == SENSOR_ATTR_FULL_SCALE) {
			if (val->val1 > CH201_MAX_DETECTION_RANGE) {
				ret = -EINVAL;
				goto attr_set_err;
			}
			data->dev_config.max_range = val->val1;
		}
		break;
	default:
		ret = -EINVAL;
		goto attr_set_err;
	}

attr_set_err:
	k_mutex_unlock(&data->lock);

	return ret;
}

/**
 * @brief       Get sensor attributes
 * @param[in]   dev	Pointer to the sensor device instance.
 * @param[in]   chan	Sensor channel
 * @param[in]   attr	Sensor attribute
 * @param[in]   val	attribute value
 * @return      0 on success, negative value on failure
 */
static int ch201_attr_get(const struct device *dev,
			enum sensor_channel chan,
			enum sensor_attribute attr,
			struct sensor_value *val)
{
	struct ch201_data *data	= dev->data;
	struct sensor_ch201_threshold *val_ptr;
	uint8_t iter;

	if (!val) {
		LOG_ERR("Invalid argument\n");
		return -EINVAL;
	}

	k_mutex_lock(&data->lock, K_FOREVER);

	switch (chan) {
	case SENSOR_CHAN_ALL:
		switch ((int) attr) {
		case SENSOR_ATTR_SAMPLING_FREQUENCY:
			/* Convert millisec to frequency */
			val->val1 = CONV_MS_TO_FREQ(data->meas_interval_ms);
			break;
		case SENSOR_CH201_ATTR_MODE:
			if (data->dev_config.mode == CH_MODE_TRIGGERED_TX_RX) {
				val->val1 = SENSOR_CH201_MODE_TRIGGERED_TX_RX;
			} else if (data->dev_config.mode == CH_MODE_FREERUN) {
				val->val1 = SENSOR_CH201_MODE_FREERUN;
			} else {
				val->val1 = SENSOR_CH201_MODE_IDLE;
			}
			break;
		case SENSOR_CH201_ATTR_THRESHOLD_TABLE:
			val_ptr = (struct sensor_ch201_threshold *)val;

			for (iter = 0; iter < CH_NUM_THRESHOLDS; iter++) {
				val_ptr->start_sample =
					data->dev_threshold.threshold[iter].start_sample;
				val_ptr->level = data->dev_threshold.threshold[iter].level;
				val_ptr++;
			}
			break;
		default:
			break;
		}
	case SENSOR_CHAN_DISTANCE:
		if ((int) attr == SENSOR_ATTR_FULL_SCALE) {
			val->val1 = data->dev_config.max_range;
		}
		break;
	default:
		break;

	}

	k_mutex_unlock(&data->lock);

	return 0;
}

/**
 * @brief       Set sensor trigger
 * @param[in]   dev	Pointer to the sensor device instance.
 * @param[in]   trig	Trigger type
 * @param[in]   handler	Trigger Handler
 * @return      0 on success, negative value on failure
 */
static int ch201_trigger_set(const struct device *dev,
			const struct sensor_trigger *trig,
			sensor_trigger_handler_t handler)
{
	struct ch201_data *data = dev->data;
	int ret = 0;

	k_mutex_lock(&data->lock, K_FOREVER);

	if ((trig->type == SENSOR_TRIG_DATA_READY) &&
	    (trig->chan == SENSOR_CHAN_ALL)) {
		data->data_ready_trigger.type = SENSOR_TRIG_DATA_READY;
		data->data_ready_trigger.chan = SENSOR_CHAN_ALL;
		data->data_ready_handler      = handler;
	} else {
		ret = -EINVAL;
		goto trig_set_err;
	}

trig_set_err:
	k_mutex_unlock(&data->lock);

	return ret;
}

/**
 * @brief       Set sensor trigger
 * @param[in]   dev	Pointer to the sensor device instance.
 * @param[in]   chan	Sensor channel
 * @param[in]   val	Pointer to store sensor value
 * @return      0 on success, negative value on failure
 */
static int ch201_channel_get(const struct device *dev,
			enum sensor_channel chan,
			struct sensor_value *val)
{
	struct ch201_data *data = dev->data;

	if ((!val) ||
	   (chan != SENSOR_CHAN_ALL)) {
		return -EINVAL;
	}

	k_mutex_lock(&data->lock, K_FOREVER);

	if (ch_sensor_is_connected(&data->ch201_dev)) {
		/* Read the sampled data */
		ch201_handle_data(dev, &data->ch201_dev, val);

		if (data->dev_config.mode == CH_MODE_FREERUN) {
			/* Sensor is in freerun mode, so configure INT line
			 * input to receive interrupt from sensor
			 */
			chdrv_int_group_set_dir_in(&data->ch201_group);
			chdrv_int_group_interrupt_enable(&data->ch201_group);
		}
	}

	k_mutex_unlock(&data->lock);

	return 0;
}

/**
 * @brief       Fetch sensor sample
 * @param[in]   dev	Pointer to the sensor device instance.
 * @param[in]   chan	Sensor channel
 * @return      0 on success, negative value on failure
 */
static int ch201_sample_fetch(const struct device *dev, enum sensor_channel chan)
{
	struct ch201_data *data = dev->data;

	if (data->dev_config.mode != CH_MODE_TRIGGERED_TX_RX) {
		return -EINVAL;
	}

	k_mutex_lock(&data->lock, K_FOREVER);

	ch_group_trigger(&data->ch201_group);

	k_mutex_unlock(&data->lock);

	return 0;
}

/* CH201 sensor api instance */
static DEVICE_API(sensor, ch201_driver_api) = {
	.trigger_set = ch201_trigger_set,
	.channel_get = ch201_channel_get,
	.attr_set = ch201_attr_set,
	.attr_get = ch201_attr_get,
	.sample_fetch = ch201_sample_fetch
};

#define CH201_DEFINE(inst)									\
												\
	static const struct ch201_config ch201_config_##inst = {				\
		.i2c = I2C_DT_SPEC_INST_GET(inst),						\
		.prog_gpio = GPIO_DT_SPEC_INST_GET(inst, prog_gpios),				\
		.int_gpio = GPIO_DT_SPEC_INST_GET(inst, int_gpios),				\
		.reset_gpio = GPIO_DT_SPEC_INST_GET(inst, reset_gpios)				\
	};											\
	struct ch201_data ch201_data_##inst;							\
												\
	SENSOR_DEVICE_DT_INST_DEFINE(inst, ch201_init, NULL, &ch201_data_##inst,		\
				     &ch201_config_##inst, POST_KERNEL,				\
				     CONFIG_SENSOR_INIT_PRIORITY, &ch201_driver_api);

#define DT_DRV_COMPAT invensense_ch201
#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)
DT_INST_FOREACH_STATUS_OKAY(CH201_DEFINE)
#endif


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
#include <errno.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/sensor.h>

/* ch201 chirp sonic library includes */
#include "invn/soniclib/sensor_fw/ch201/ch201_gprmt.h"
#include "invn/soniclib/soniclib.h"
#include "ch201_bsp.h"

/* CH201 GPR - general purpose rangefinding, standard range */
#define CH201_SENSOR_FW_INIT_FUNC   ch201_gprmt_init

/* Maximum detection range for the sensor in mm */
#define CH201_MAX_RANGE_MM          (2000)

/* Measurement interval is 1 Second (1000ms) */
#define CH201_MEAS_INTERVAL_MS      (1000)

#define CH201_FIRST_SENSOR_MODE     CH_MODE_TRIGGERED_TX_RX
#define CH201_OTHER_SENSOR_MODE     CH_MODE_TRIGGERED_RX_ONLY

#define	CH201_STATIC_REJECT_SAMPLES (0)

#define CH201_RX_PRETRIGGER_ENABLE  1

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

/* micro-units per 1 unit */
#define MICROS_PER_UNIT (1000000LL)
/* milli-units per 1 unit */
#define MILLIS_PER_UNIT (1000LL)

/* Acceleration value in ug */
#define CH201_RANGE_VAL(x)          ((x * MILLIS_PER_UNIT) / 32.0)

/* macro to extract integer and fractional part of sensor data */
#define CH201_SENSOR_EXTRACT_INT_FRACT_PART(x, conv_val) {      \
	(x).val1 = (conv_val / MICROS_PER_UNIT);                \
	(x).val2 = (conv_val % MICROS_PER_UNIT);                \
}

LOG_MODULE_REGISTER(CH201_TOF, LOG_LEVEL_INF);

ch_thresholds_t thresholds = {
	.threshold = {
	{CH201_THRESH_0_START, CH201_THRESH_0_LEVEL},      /* threshold 0 */
	{CH201_THRESH_1_START, CH201_THRESH_1_LEVEL},      /* threshold 1 */
	{CH201_THRESH_2_START, CH201_THRESH_2_LEVEL},      /* threshold 2 */
	{CH201_THRESH_3_START, CH201_THRESH_3_LEVEL},      /* threshold 3 */
	{CH201_THRESH_4_START, CH201_THRESH_4_LEVEL},      /* threshold 4 */
	{CH201_THRESH_5_START, CH201_THRESH_5_LEVEL},      /* threshold 5 */
	},
};

/* chirp device data */
typedef struct chirp_dev_data {
	uint32_t range;
	uint16_t amplitude;
} chirp_dev_data_t;

/* chirp device information */
typedef struct chirp_dev_info {
	ch_dev_t              dev;
	struct sensor_value   data[2];
	volatile bool         data_ready;
} chirp_dev_info_t;

/* chirp group of devices */
typedef struct ch201_group {
	ch_group_t            group;
	chirp_dev_info_t      dev_list[CONFIG_CH201_MAX_NUM_SENSORS];
	ch_version_t          lib_ver;
	uint8_t               connected_dev_cnt;
	uint8_t               num_trig_sensors;
	struct k_timer        timer;
	volatile bool         timer_triggered;
	volatile uint8_t      data_avail_cnt;
} ch201_group_t;

static ch201_group_t ch201_grp;

/**
 * @brief       Timer triggered Interrupt callback.
 * @param[in]   timer_id  Triggered Timer identifier
 * @return      None
 */
static void ch201_periodic_timer_trigger(struct k_timer *timer_id)
{
	ch201_grp.timer_triggered = true;
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
	ARG_UNUSED(grp_ptr);

	if (int_type == CH_INTERRUPT_TYPE_DATA_RDY) {
		/* Set data-ready flag */
		ch201_grp.dev_list[dev_num].data_ready = true;
		ch201_grp.data_avail_cnt++;
	}
}

/**
 * @brief       Read the sampled data.
 * @param[in]   dev     Pointer to the device structure for the driver instance.
 * @param[in]   dev_ptr Ch201 device pointer
 * @param[in]   data    Sensor data
 * @return      None
 */
static void ch201_handle_data(ch_dev_t *dev_ptr, struct sensor_value *val)
{
	uint64_t loc_range;
	struct sensor_value loc_val;

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
 * @brief       Configures sensors
 * @param[in]   gpr_ptr   Pointer to the CH201 device group
 * @param[in]   num_ports Number of CH201 devices
 * @return      Configuration Error status
 */
static uint8_t configure_sensors(ch_group_t *grp_ptr, uint8_t num_ports)
{
	uint8_t dev_num;
	ch_dev_t *dev_ptr;
	uint8_t ch_error;
	ch_config_t dev_config;
	ch_rangefinder_algo_config_t ch_rng_alg_cfg;

	/* Set pointer to struct containing detection thresholds */
	ch_rng_alg_cfg.thresh_ptr = &thresholds;
	ch_rng_alg_cfg.static_range = CH201_STATIC_REJECT_SAMPLES;

	ch_error = 0;

	for (dev_num = 0; dev_num < num_ports; dev_num++) {
		dev_ptr = ch_get_dev_ptr(grp_ptr, dev_num);

		if (!ch_error && ch_sensor_is_connected(dev_ptr)) {
			ch201_grp.connected_dev_cnt++;

			/* Set rangefinder algo config */
			ch_rangefinder_set_algo_config(dev_ptr, &ch_rng_alg_cfg);

			if (ch201_grp.connected_dev_cnt == 1) {
				/* if this is the first sensor */
				dev_config.mode = CH201_FIRST_SENSOR_MODE;
			} else {
				dev_config.mode = CH201_OTHER_SENSOR_MODE;
			}

			/* Init config structure with values from app_config.h */
			dev_config.max_range = CH201_MAX_RANGE_MM;

			/* If sensor will be free-running, set internal sample interval */
			if (dev_config.mode == CH_MODE_FREERUN) {
				dev_config.sample_interval = CH201_MEAS_INTERVAL_MS;
			} else {
				dev_config.sample_interval = 0;
				ch201_grp.num_trig_sensors++;
			}

			/* Set intr1 direction as input and enable it
			 * for freerun mode
			 */
			if ((!ch_error) && (dev_config.mode == CH_MODE_FREERUN)) {
				chdrv_int_set_dir_in(dev_ptr);
				chdrv_int_interrupt_enable(dev_ptr);
			}

			if (!ch_error) {
				/* Apply sensor configuration */
				ch_error = ch_set_config(dev_ptr, &dev_config);
			}

			/* Enable receive sensor pre-triggering, if specified */
			ch_set_rx_pretrigger(grp_ptr, CH201_RX_PRETRIGGER_ENABLE);
		}
	}
	return ch_error;
}

/**
 * @brief       main entry point
 * @param[in]   None
 * @return      main execution status
 */
int main(void)
{
	uint8_t ch201_error = 0;
	uint8_t num_ports;
	uint8_t dev_num;
	chirp_dev_info_t *dev_inf;

	memset(&ch201_grp, 0x00, sizeof(ch201_group_t));

	LOG_INF("\t*****CH201 Range Sensor Demo*****\n");

	/* Perform board init */
	chbsp_board_init(&ch201_grp.group);

	/* Fetch sonic library version */
	ch_get_version(&ch201_grp.lib_ver);
	LOG_INF("\tSonicLib version: %u.%u.%u\n", ch201_grp.lib_ver.major,
		ch201_grp.lib_ver.minor, ch201_grp.lib_ver.rev);

	num_ports = ch_get_num_ports(&ch201_grp.group);
	for (dev_num = 0; dev_num < num_ports; dev_num++) {
		dev_inf = &ch201_grp.dev_list[dev_num];

		/* Initialize ch201 device */
		ch201_error  |= ch_init(&dev_inf->dev, &ch201_grp.group,
				dev_num, CH201_SENSOR_FW_INIT_FUNC);

		dev_inf->data_ready = false;
	}

	if (ch201_error == 0) {
		LOG_INF("\tStarting CH201 group...\n");
		ch201_error = ch_group_start(&ch201_grp.group);
		if (ch201_error != 0) {
			LOG_ERR("\tStarting CH201 group failed...\n");
			return -1;
		}
	}

	/* Register callback function to be called when CH201 sensor interrupts */
	ch_io_int_callback_set(&ch201_grp.group, ch201_int_callback);

	/* Configure sensors */
	ch201_error |= configure_sensors(&ch201_grp.group, num_ports);

	if (ch201_grp.num_trig_sensors == 0) {
		/* set INT line as input no sensor is in Tx/Rx mode */
		chdrv_int_group_set_dir_in(&ch201_grp.group);
		/* enable interrupt */
		chdrv_int_group_interrupt_enable(&ch201_grp.group);
	} else {
		/* Initialize the timer as any one sensor is in Tx/Rx mode */
		k_timer_init(&ch201_grp.timer, ch201_periodic_timer_trigger, NULL);
		ch201_grp.timer_triggered = false;

		/* start timer triggering */
		k_timer_start(&ch201_grp.timer, K_MSEC(CH201_MEAS_INTERVAL_MS),
				K_MSEC(CH201_MEAS_INTERVAL_MS));
	}

	while (true) {
		if (ch201_grp.timer_triggered) {
			ch201_grp.timer_triggered = false;
			/* Fetch sample only in Tx-RX mode, not in Free-Running mode.
			 * The sensor triggers data ready interrupt
			 * once timeout happens in Free-Running mode.
			 */
			ch_group_trigger(&ch201_grp.group);
		}

		if (ch201_grp.data_avail_cnt) {
			/* Read data if available */
			for (dev_num = 0; dev_num < num_ports; dev_num++) {
				if (ch201_grp.dev_list[dev_num].data_ready) {
					dev_inf = &ch201_grp.dev_list[dev_num];
					ch201_handle_data(&dev_inf->dev, dev_inf->data);

					LOG_INF("\tObject Range --> %f m, Amplitude --> %u\n",
						sensor_value_to_double(&(dev_inf->data[0])),
						dev_inf->data[1].val1);
					dev_inf->data_ready = false;
					ch201_grp.data_avail_cnt--;
				}
			}
		}
	}
	return 0;
}


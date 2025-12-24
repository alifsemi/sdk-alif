/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#ifndef ZEPHYR_DRIVERS_SENSOR_CH201_PRIVATE_H_
#define ZEPHYR_DRIVERS_SENSOR_CH201_PRIVATE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <ch201.h>

#include "chirp_board_config.h"
#include "invn/soniclib/chirp_bsp.h"
#include "invn/soniclib/ch_rangefinder.h"


/* CH201 Driver status */
#define CH201_GPIO_DRIVER_READY       (1 << 0U)
#define CH201_I2C_DRIVER_READY        (1 << 1U)
#define CH201_DRIVER_READY            (3 << 0U)
#define CH201_RTC_DRIVER_READY	      (1 << 2U)


/* Supported Max detection range is 5000 mm */
#define CH201_MAX_DETECTION_RANGE     5000

/* I2C Address assignments for each possible device */
#define CH201_I2C_ADDRS       0x29
#define CH201_I2C_BUS_NUM     0
#define CH201_I2C_DEV_NUM     0
/* Reg address size in bytes */
#define CH201_REG_ADDR_SIZE   1

#define CONVERT_MS_TO_US(n)   (n * 1000U)

/* CH201 driver configuration */
struct ch201_config {
	struct i2c_dt_spec i2c;
	struct gpio_dt_spec prog_gpio;
	struct gpio_dt_spec int_gpio;
	struct gpio_dt_spec reset_gpio;
};

/* CH201 driver data */
struct ch201_data {
	/* Configuration for group of sensors    */
	ch_group_t			ch201_group;
	/* CH201 sensor group pointer            */
	ch_group_t			*grp_ptr;
	/* CH201 sensor device                   */
	ch_dev_t			ch201_dev;
	/* CH201 I2C device number               */
	uint8_t				dev_num;
	/* CH201 Device configuration            */
	ch_config_t			dev_config;
	/* CH201 Device algo config              */
	ch_rangefinder_algo_config_t	dev_algo_config;
	/* Device Multiple detection threshold   */
	ch_thresholds_t			dev_threshold;
	/* I2C Event status                      */
	volatile uint32_t		ch201_i2c_event;
	/* Measurement interval in millisec      */
	uint16_t			meas_interval_ms;
	/* Driver state                          */
	uint8_t				state;
	/* INT gpio interrupt status             */
	volatile bool			gpio_int_sts;
	/* Wait mode on GPIO INT1                */
	volatile bool			int1_wait_mode;
	/* Data ready flag                       */
	volatile bool			data_ready;
	struct gpio_callback		gpio_cb;
	/* Sensor data ready triiger and handler */
	sensor_trigger_handler_t	data_ready_handler;
	struct sensor_trigger		data_ready_trigger;
	struct k_mutex			lock;
};

/**
 * @brief       Setup gpios required for ch201.
 * @param[in]   dev	Pointer to the device structure for the driver instance.
 * @return      Execution status
 */
int ch201_gpios_configure(const struct device *dev);

/**
 * @brief       Initialize the periodic timer.
 * @param[in]   dev		Pointer to the sensor device instance.
 * @param[in]   interval_ms	Interval in millisec
 * @return	None
 */
void ch201_periodic_timer_init(const struct device *dev, uint16_t interval_ms);

/**
 * @brief       Get current ch201 device's reference
 * @param[in]   dev   Pointer to refer the device address.
 * @return	None
 */
void ch201_get_cur_dev(const struct device **dev);

#ifdef __cplusplus
}
#endif
#endif /* ZEPHYR_DRIVERS_SENSOR_CH201_PRIVATE_H_ */

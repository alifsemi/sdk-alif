/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/* Project Includes */
#include <zephyr/drivers/sensor.h>

/*Number of internal detection thresholds (CH201 only). */
#define CH201_MAX_SUPPORTED_THRESHOLDS 0x6

/**
 * brief CH201 Sensor operating modes
 */
enum sensor_mode_ch201 {
	SENSOR_CH201_MODE_IDLE		  = 0x0,	/* Idle mode - Idle mode */
	SENSOR_CH201_MODE_FREERUN	  = 0x1,	/* Free-running mode -sensor uses */
							/* internal clk to wake and measure */
	SENSOR_CH201_MODE_TRIGGERED_TX_RX = 0x2		/*  Triggered transmit/receive mode */
};

/**
 * @brief CH201 Sensor attribute types.
 */
enum sensor_attribute_ch201 {
	SENSOR_CH201_ATTR_MODE = SENSOR_ATTR_PRIV_START,
	SENSOR_CH201_ATTR_THRESHOLD_TABLE,
	SENSOR_CH201_ATTR_ENABLE
};

/**
 * @brief CH201 Sensor thresholds.
 */
struct sensor_ch201_threshold {
	uint16_t start_sample;
	uint16_t level;
};

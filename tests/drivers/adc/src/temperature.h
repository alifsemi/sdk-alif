/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef TEMPERATURE_H_
#define TEMPERATURE_H_

#include <stdint.h>

#include <zephyr/sys/util.h>

#define MAX_TEMP_RANGE 402

/* Declaration of the 2D array and function */
extern const float tempData[][2];
float get_temperature(uint32_t adc_value);

#endif /* TEMPERATURE_H_ */

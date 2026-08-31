/*
 * Copyright (C) 2024 Alif Semiconductor.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef TEMPERATURE_H_
#define TEMPERATURE_H

#include <zephyr/sys/util.h>

#define MAX_TEMP_RANGE 402

/* Declaration of the 2D array and function */
extern const float tempData[][2];
float get_temperature(uint32_t adc_value);

#endif /* TEMPERATURE_H_ */

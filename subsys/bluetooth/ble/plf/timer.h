/*
 * Copyright (c) 2023 Alif Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _TIMER_H_
#define _TIMER_H_

#include <stdint.h>
#include <stdbool.h>

typedef void (*timer_cb)(void);

/**
 * Initialize a timer
 */
void timer_init(void);

/**
 * Set timeout to trigger after amount of time in microseconds
 * @param Timeout Timeout value
 * @param cb callback function to call after to triggers
 */
void timer_set_timeout(uint32_t timeout, timer_cb cb);

/**
 * Enable timer
 * @param enable True to enable the timer, False to disable it
 */
void timer_enable(bool enable);

/**
 * Get current time
 * @return Current time in microseconds
 */
uint32_t timer_get_time(void);

#endif /* _TIMER_H_ */

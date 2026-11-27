/*
 * Copyright (c) 2023 Alif Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _SYNC_TIMER_H
#define _SYNC_TIMER_H

#include <stdint.h>

/**
 * @file sync_timer.h
 *
 * @brief This file contains the sync app hook implementation to allow the BLE host stack to
 * synchronize a host timer with the controller timer
 */

/**
 * @brief  Initialise synchronization timer. A free running counter in UP direction
 *
 *         Note: Overflow event(irq) priority should be higher than capture event(irq)
 *
 *         Note: The sync timer clock speed should be at least 1Mhz
 *
 * @return 0 on success
 */
int32_t sync_timer_init(void);

/**
 * @brief Start the synchronization timer and set overflow and capture callbacks
 *
 * @param sync_timer_capture_evt_cb  Callback function invoked when capture event is triggered
 * @param sync_timer_overflow_evt_cb Callback function invoked when overflow event is triggered
 *
 * @return Input clock speed of the sync timer
 */
uint32_t sync_timer_start(void (*sync_timer_capture_evt_cb)(void),
			  void (*sync_timer_overflow_evt_cb)(void));

/**
 * @brief Gets current counter value
 *        Counter tick = (1 / SYSTEM_CLOCK) seconds
 *
 * @return Up counter value
 */
uint32_t sync_timer_get_curr_cnt(void);

/**
 * @brief  Gets the last captured counter value on iso event
 *         Counter tick = (1 / SYSTEM_CLOCK) seconds
 *
 * @return Captured counter value when iso signal is generated from controller
 */
uint32_t sync_timer_get_last_capture(void);

/**
 * @brief  Disables sync timer events briefly
 *
 */
void sync_timer_disable_evts(void);

/**
 * @brief  Restores pending sync timer events
 *
 */
void sync_timer_restore_evts(void);

#endif /* _SYNC_TIMER_H */

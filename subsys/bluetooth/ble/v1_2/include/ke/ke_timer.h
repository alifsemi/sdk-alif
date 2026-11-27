/**
 ****************************************************************************************
 *
 * @file ke_timer.h
 *
 * @brief This file contains the definitions used for timer management
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

#ifndef KE_TIMER_H_
#define KE_TIMER_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup TIMER BT Time
 * @ingroup KERNEL
 * @brief Timer management module.
 *
 * This module implements the functions used for managing kernel timers.
 *
 ****************************************************************************************
 */

#include "rwip.h"                 // RW definitions
#include "rwip_config.h"          // stack configuration
#include "ke_msg.h"               // messaging definition


/*
 * DEFINITIONS
 ****************************************************************************************
 */


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


/*
 * FUNCTION PROTOTYPES
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief This function flushes all timers pending in the kernel.
 *
 ****************************************************************************************
 */
void ke_timer_flush(void);

/**
 ****************************************************************************************
 * @brief Set a timer.
 *
 * The function first cancel the timer if it is already existing, then
 * it creates a new one.
 *
 * When the timer expires, a message is sent to the task provided as
 * argument, with the timer id as message id.
 *
 *
 * @param[in] timer_id      Timer identifier (message identifier type).
 * @param[in] task_id       Task identifier which will be notified
 * @param[in] delay_ms         Delay in time milliseconds.
 ****************************************************************************************
 */
void ke_timer_set(ke_msg_id_t const timer_id, ke_task_id_t const task_id, uint32_t delay_ms);

/**
 ****************************************************************************************
 * @brief Remove an registered timer.
 *
 * This function search for the timer identified by its id and its task id.
 * If found it is stopped and freed.
 *
 * @param[in] timer_id  Timer identifier.
 * @param[in] task_id      Task identifier.
 ****************************************************************************************
 */
void ke_timer_clear(ke_msg_id_t const timer_id, ke_task_id_t const task_id);

/**
 ****************************************************************************************
 * @brief Checks if a timer is active.
 *
 * @param[in] timer_id  Timer identifier.
 * @param[in] task      Task identifier.
 *
 * @return True if timer is active, False otherwise
 ****************************************************************************************
 */
bool ke_timer_active(ke_msg_id_t const timer_id, ke_task_id_t const task_id);

/// @} TIMER

#endif // KE_TIMER_H_

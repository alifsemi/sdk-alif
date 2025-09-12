/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#pragma once

#include <stdint.h>

/**
 * Init BLE central functionality
 */
void central_app_init(void);

/**
 * App central state machine execution
 *
 * @param[in] state Application state
 *
 * @return 0 in case of state was handled, otherwise -1
 */
int central_app_exec(uint32_t const state);

int central_get_service_uuid_str(char *p_uuid, uint8_t max_len);

int central_set_send_intervall(uint32_t intervall);

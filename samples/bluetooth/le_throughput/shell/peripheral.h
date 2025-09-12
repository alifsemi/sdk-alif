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

void peripheral_app_init(void);

int peripheral_app_exec(uint32_t const state);

/**
 * Get peripheral serive UUID
 *
 * @return 0 in case of success, negative value is case of error
 */
int peripheral_get_service_uuid_str(char *p_uuid, uint8_t max_len);

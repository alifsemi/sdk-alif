/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <inttypes.h>
#include <stdbool.h>

#define DEVICE_NAME_LEN 9
extern char app_shell_device_name[];

extern uint16_t ble_adv_int_min;
extern uint16_t ble_adv_int_max;
extern uint16_t ble_conn_int_min;
extern uint16_t ble_conn_int_max;

int ble_init(void);
int ble_start(void);
int ble_uninit(void);
bool ble_is_connected(void);

/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

extern uint16_t ble_adv_int_min;
extern uint16_t ble_adv_int_max;
extern uint16_t ble_conn_int_min;
extern uint16_t ble_conn_int_max;
extern uint32_t ble_rtc_wakeup;
extern uint32_t ble_rtc_connected_wakeup;
extern char app_shell_device_name[];

void appl_wait_to_continue(void);
bool appl_allow_sleep(void);
void cmd_appl_shell_init(void);
bool appl_restart_ble(void);

/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

extern uint32_t ble_rtc_wakeup;
extern uint32_t ble_rtc_connected_wakeup;
extern uint16_t reset_after_sleep;

int64_t param_get_int(size_t argc, char **argv, char *p_param, int def_value);
char *param_get_char(size_t argc, char **argv, char *p_param, char *def_value);
void appl_shell_reset(void);
void appl_wait_to_continue(void);
void appl_shell_sleep_period_end(void);

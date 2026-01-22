/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

enum pm_state_mode_type {
	PM_STATE_MODE_IDLE,
	PM_STATE_MODE_STANDBY,
	PM_STATE_MODE_STOP
};

int pwm_init(void);

void app_ready_for_sleep(void);
int set_off_profile(const enum pm_state_mode_type pm_mode);
int app_set_run_params(void);
uint32_t get_wakeup_irq_status(void);
bool is_cold_boot(void);
uint32_t get_current_ticks(void);
uint32_t s_to_ticks(const uint32_t s);
void app_prevent_off(void);

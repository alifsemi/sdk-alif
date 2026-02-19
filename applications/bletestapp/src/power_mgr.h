/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

 #include "power_defines.h"

  /* Power modes. Based on E8 power modes so same tests can be run with E8 and B1. */
enum pm_state_mode_type {
	PM_STATE_MODE_GO_1,
	PM_STATE_MODE_GO_2,
	PM_STATE_MODE_GO_3,
	PM_STATE_MODE_GO_4,
	PM_STATE_MODE_GO_5,
	PM_STATE_MODE_READY_1,
	PM_STATE_MODE_READY_2,
	PM_STATE_MODE_IDLE_1,
	PM_STATE_MODE_IDLE_2,
	PM_STATE_MODE_STANDBY_1,
	PM_STATE_MODE_STOP_1,
	PM_STATE_MODE_STOP_2,
	PM_STATE_MODE_STOP_3,
	PM_STATE_MODE_STOP_4,
	PM_STATE_MODE_STOP_5
};

int select_timer_source(enum lp_counter_source source);
void app_sleep_start(uint32_t time_s);
void get_default_run_cfg(run_profile_t *runp);
void get_default_off_cfg(off_profile_t *offp);
int set_current_off_profile(void);
int set_off_profile(const enum pm_state_mode_type pm_mode);
int app_set_run_params(void);
void app_prevent_off(void);

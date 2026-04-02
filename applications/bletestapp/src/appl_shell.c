/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <zephyr/shell/shell_uart.h>
#include <zephyr/logging/log.h>
#include <stdlib.h>
#include "power_mgr.h"
#include "ble_handler.h"

LOG_MODULE_DECLARE(main, CONFIG_MAIN_LOG_LEVEL);

static struct k_sem waiting_semaphore;
static bool __attribute__((noinit)) force_ble_restart;
static bool __attribute__((noinit)) skip_wait;
static uint32_t __attribute__((noinit)) wakeup_time;

uint32_t ble_rtc_wakeup __attribute__((noinit));
uint32_t ble_rtc_connected_wakeup __attribute__((noinit));
uint16_t reset_after_sleep __attribute__((noinit));
enum lp_counter_source counter_source = -1;

void appl_shell_reset(void)
{
	skip_wait = false;
	force_ble_restart = false;
	wakeup_time = 0;
	ble_adv_int_min = 1000;
	ble_adv_int_max = 1000;
	ble_conn_int_min = 800;
	ble_conn_int_max = 800;
	ble_rtc_wakeup = 20000;
	strncpy(app_shell_device_name, "APPL_SHL", DEVICE_NAME_LEN - 1);
	app_shell_device_name[8] = 0;
	reset_after_sleep = 0;
}

void appl_shell_init(void)
{
	/* Mark a cold boot */
	appl_shell_reset();
	k_sem_init(&waiting_semaphore, 0, 1);
}

static int app_shell_init(void)
{
	/* If we are sleeping dont set the shell prompt off */
	const struct shell *shell = shell_backend_uart_get_ptr();

	if (!shell) {
		return 0;
	}
	shell_start(shell);
	appl_shell_init();

	return 0;
}

SYS_INIT(app_shell_init, APPLICATION, 1);

int64_t param_get_int(size_t argc, char **argv, char *p_param, int def_value)
{
	if (p_param && argc > 1) {
		for (int n = 0; n < (argc - 1); n++) {
			if (strcmp(argv[n], p_param) == 0) {
				return strtoll(argv[n + 1], NULL, 0);
			}
		}
	}
	return def_value;
}

char *param_get_char(size_t argc, char **argv, char *p_param, char *def_value)
{
	if (p_param && argc > 1) {
		for (int n = 0; n < (argc - 1); n++) {
			if (strcmp(argv[n], p_param) == 0) {
				return argv[n + 1];
			}
		}
	}
	return def_value;
}

void appl_wait_to_continue(void)
{
	const struct shell *shell = shell_backend_uart_get_ptr();

	shell_print(shell, "BLE testapp started!\n");

	if (!skip_wait) {
		k_sem_take(&waiting_semaphore, K_FOREVER);
		LOG_INF("continuing.");
	}
}

/**
 * Application command functions
 **/
static int cmd_adv_int(const struct shell *shell, size_t argc, char **argv)
{
	ble_adv_int_min = param_get_int(argc, argv, "--min", ble_adv_int_min);
	ble_adv_int_max = param_get_int(argc, argv, "--max", ble_adv_int_max);

	shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT,
			"set advertisement interval min: %d max %d\n",
			ble_adv_int_min, ble_adv_int_max);
	return 0;
}
static int cmd_conn_int(const struct shell *shell, size_t argc, char **argv)
{
	ble_conn_int_min = param_get_int(argc, argv, "--min", ble_conn_int_min);
	ble_conn_int_max = param_get_int(argc, argv, "--max", ble_conn_int_max);

	shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT, "set connection interval min: %d max %d\n",
		      ble_conn_int_min, ble_conn_int_max);
	return 0;
}

static int cmd_int(const struct shell *shell, size_t argc, char **argv)
{
	unsigned long wakeup_time = strtoul(argv[1], 0, 10);

	if (wakeup_time) {
		ble_rtc_wakeup = wakeup_time;
	}
	ble_rtc_connected_wakeup =
		param_get_int(argc, argv, "--connected", ble_rtc_connected_wakeup);

	shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT, "set RTC wakeup: %d when connected %d\n",
		      ble_rtc_wakeup, ble_rtc_connected_wakeup);
	return 0;
}

static int cmd_continue(const struct shell *shell, size_t argc, char **argv)
{
	reset_after_sleep = param_get_int(argc, argv, "--reset_after", reset_after_sleep);
	skip_wait = true;
	k_sem_give(&waiting_semaphore);
	shell_print(shell, "BLEtestapp continue");
	return 0;
}

void appl_shell_sleep_period_end(void)
{
	if (reset_after_sleep) {
		const struct shell *shell = shell_backend_uart_get_ptr();
		int ret;

		appl_shell_reset();
		ret = ble_uninit();
		shell_print(shell, "BLE application is restarted %d", ret);
	}
}

static int cmd_sleep(const struct shell *shell, size_t argc, char **argv)
{
	uint32_t sleeptime_s = strtoul(argv[1], 0, 10);

	shell_print(shell, "start sleep cycle %d s", sleeptime_s);

	app_sleep_start(sleeptime_s);
	return 0;
}

static int cmd_set_name(const struct shell *shell, size_t argc, char **argv)
{
	if (skip_wait) {
		shell_error(shell, "Name must be set on POR, before continue is called.");
		return -ENOEXEC;
	}
	/* app_shell_device_name has null-ending as per cold boot init */
	strncpy(app_shell_device_name, argv[1],	DEVICE_NAME_LEN - 1);
	shell_print(shell, "BLE name set to %s", app_shell_device_name);
	return 0;
}

static int cmd_select_timer(const struct shell *shell, size_t argc, char **argv)
{
	if (!strcmp(argv[1], "LPRTC")) {
		counter_source = LPRTC;
		shell_print(shell, "Selected LPRTC as wakeup source");
	} else if (!strcmp(argv[1], "LPTIMER")) {
		counter_source = LPTIMER;
		shell_print(shell, "Selected LPTIMER as wakeup source");
	} else {
		shell_error(shell, "Select timer source from LPRTC and LPTIMER");
		return -ENOEXEC;
	}
	return 0;
}


static int cmd_set_offprofile(const struct shell *shell, size_t argc, char **argv)
{
	int ret;

	if (!strcmp(argv[1], "STOP")) {
		ret = set_off_profile(PM_STATE_MODE_STOP_1);

		if (ret) {
			shell_error(shell, "Failed to set off profile: %d", ret);
			return -ENOEXEC;
		}
		shell_print(shell, "Off profile set to STOP");
	} else if (!strcmp(argv[1], "IDLE")) {
		ret = set_off_profile(PM_STATE_MODE_IDLE_1);

		if (ret) {
			shell_error(shell, "Failed to set off profile: %d", ret);
			return -ENOEXEC;
		}
		shell_print(shell, "Off profile set to IDLE");
	} else if (!strcmp(argv[1], "STANDBY")) {
		ret = set_off_profile(PM_STATE_MODE_STANDBY_1);

		if (ret) {
			shell_error(shell, "Failed to set off profile: %d", ret);
			return -ENOEXEC;
		}
		shell_print(shell, "Off profile set to STANDBY");
	} else {
		shell_error(shell, "Select off profile from STOP, IDLE, STANDBY");
		return -ENOEXEC;
	}
	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_cmds,
	SHELL_CMD_ARG(adv-interval, NULL,
		      "configure advertisement interval --min <slots> --max <slots>", cmd_adv_int,
		      1, 10),
	SHELL_CMD_ARG(conn-interval, NULL,
		      "configure connection interval --min <slots> --max <slots>", cmd_conn_int, 1,
		      10),
	SHELL_CMD_ARG(interval, NULL,
		      "configure M55 wakeup interval <time in ms> --connected <time in ms>",
		      cmd_int, 1, 10),
	SHELL_CMD_ARG(continue, NULL, "Start ble application --reset_after <0/1>",
		cmd_continue, 1, 10),
	SHELL_CMD_ARG(sleep, NULL, "allow sleep in <seconds>", cmd_sleep, 2, 10),
	SHELL_CMD_ARG(init, NULL, "BLE stack initialize", cmd_sleep, 2, 10),
	SHELL_CMD_ARG(set-name, NULL, "set name for BLE <name>", cmd_set_name, 2, 10),
	SHELL_CMD_ARG(set_offprofile, NULL, "Set off profile", cmd_set_offprofile, 2, 10),
	SHELL_CMD_ARG(select_timer, NULL, "Select timer source <LPRTC/LPTIMER>",
		cmd_select_timer, 2, 10),
	SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(ble_appl, &sub_cmds, "Ble configuration commands", NULL);

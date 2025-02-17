/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/shell/shell.h>
#include <stdio.h>
#include "config.h"
#include "common.h"
#include "peripheral.h"
#include "central.h"
#include "gap.h"

LOG_MODULE_REGISTER(shell_peripheral, LOG_LEVEL_ERR);

static struct k_thread tp_thread;
static K_THREAD_STACK_DEFINE(tp_stack, CONFIG_ALIF_TP_SHELL_THREAD_STACKSIZE);

static int cmd_info(const struct shell *shell, size_t argc, char **argv)
{
	char temp[64];

	shell_fprintf(shell, SHELL_VT100_COLOR_YELLOW, "Throughput device config:\n");
	shell_fprintf(shell, SHELL_VT100_COLOR_GREEN, "  Name: ");
	shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT, CONFIG_BLE_TP_DEVICE_NAME "\n");

	if (get_device_role() == GAP_ROLE_NONE) {
		shell_fprintf(shell, SHELL_VT100_COLOR_GREEN, "Device type not set\n");
		return 0;
	}

	shell_fprintf(shell, SHELL_VT100_COLOR_GREEN, "  Role: ");
	shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT,
		      (get_device_role() == GAP_ROLE_LE_PERIPHERAL) ? "Peripheral\n" : "Central\n");

	get_private_address(temp, sizeof(temp));
	shell_fprintf(shell, SHELL_VT100_COLOR_GREEN, "  Private Address: ");
	shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT, "%s\n", temp);

	peripheral_get_service_uuid_str(temp, sizeof(temp));
	shell_fprintf(shell, SHELL_VT100_COLOR_GREEN, "  Service UUID: ");
	shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT, "%s\n", temp);

	return 0;
}

static int cmd_peripheral_start(const struct shell *sh, size_t argc, char **argv)
{
	if (get_device_role() != GAP_ROLE_NONE) {
		LOG_ERR("device role already defined");
		return -1;
	}

	set_device_role(GAP_ROLE_LE_PERIPHERAL);

	printk("Start advertising\n");

	k_thread_create(&tp_thread, tp_stack, K_THREAD_STACK_SIZEOF(tp_stack), tp_worker, NULL,
			NULL, NULL, CONFIG_ALIF_TP_SHELL_THREAD_PRIORITY, 0, K_NO_WAIT);
	return 0;
}

static int cmd_central_start(const struct shell *sh, size_t argc, char **argv)
{
	if (get_device_role() != GAP_ROLE_NONE) {
		LOG_ERR("Device role already set");
		return -1;
	}

	set_device_role(GAP_ROLE_LE_CENTRAL);

	k_thread_create(&tp_thread, tp_stack, K_THREAD_STACK_SIZEOF(tp_stack), tp_worker, NULL,
			NULL, NULL, CONFIG_ALIF_TP_SHELL_THREAD_PRIORITY, 0, K_NO_WAIT);

	return 0;
}

int cmd_tp_test_start(void)
{
	LOG_DBG("start TP test");

	if (get_app_state() == APP_STATE_CENTRAL_READY) {
		app_transition_to(APP_STATE_STATS_RESET);
	} else {
		LOG_ERR("Peripheral not ready");
	}
	return 0;
}

int cmd_tp_set_intervall(const struct shell *shell, size_t argc, char **argv)
{
	uint32_t intervall = strtol(argv[1], NULL, 10);

	if (get_device_role() != GAP_ROLE_LE_CENTRAL) {
		LOG_ERR("Device must be central");
		return -1;
	}

	return central_set_send_intervall(intervall);
}

SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_tp, SHELL_CMD_ARG(info, NULL, "Throughput device info", cmd_info, 1, 10),
	SHELL_CMD_ARG(peripheral, NULL, "Peripheral config", cmd_peripheral_start, 1, 10),
	SHELL_CMD_ARG(central, NULL, "Central config", cmd_central_start, 1, 10),
	SHELL_CMD_ARG(run, NULL, "Run throughput test", cmd_tp_test_start, 1, 10),
	SHELL_CMD_ARG(send - intervall - set, NULL, "Set send intervall (ms)", cmd_tp_set_intervall,
			2, 10),
	SHELL_SUBCMD_SET_END /* Array terminated. */
);

SHELL_CMD_REGISTER(tp, &sub_tp, "Throughput device config", NULL);

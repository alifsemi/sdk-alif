/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/shell/shell.h>
#include <es0_power_manager.h>
#include <stdlib.h>
#include <inttypes.h>

#define LOG_MODULE_NAME alif_power_mgr_shell
#define LOG_LEVEL       LOG_LEVEL_INFO

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

static bool param_get_flag(size_t argc, char **argv, char *p_flag)
{
	if (p_flag && argc) {
		for (int n = 0; n < argc; n++) {
			if (strcmp(argv[n], p_flag) == 0) {
				return true;
			}
		}
	}
	return false;
}

static int cmd_start(const struct shell *shell, size_t argc, char **argv)
{
	int8_t ret = take_es0_into_use();

	shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT, "Start ES0 ret:%d\n", ret);
	return 0;
}

static int cmd_stop(const struct shell *shell, size_t argc, char **argv)
{
	int8_t ret = stop_using_es0();

	shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT, "Start ES0 ret:%d\n", ret);
	return 0;
}
static int cmd_uart_wiggle(const struct shell *shell, size_t argc, char **argv)
{
	/*
	 * sys_write32(0x10000, 0x4903F00C);
	 * sys_set_bits(M55HE_CFG_HE_CLK_ENA, BIT(8));
	 * data =  sys_read32(0x1A602008);
	 */

	shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT, "set HCI uart flowcontrol to manual\n");
	sys_write8(0x00, 0x4300A010);
	sys_write8(0x02, 0x4300A010);
	sys_write8(0x00, 0x4300A010);
	shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT, "set HCI uart flowcontrol to automatic\n");
	sys_write8(0x2b, 0x4300A010);
	return 0;
}

static int cmd_uart_wakeup(const struct shell *shell, size_t argc, char **argv)
{
	shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT, "set HCI uart flowcontrol to automatic\n");
	sys_write8(0x2b, 0x4300A010);
	return 0;
}

static int cmd_uart_sleep(const struct shell *shell, size_t argc, char **argv)
{
	shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT,
		      "set HCI uart flowcontrol to manual and sleep\n");
	sys_write8(0x00, 0x4300A010);
	return 0;
}

static int cmd_uart(const struct shell *shell, size_t argc, char **argv)
{

	if (param_get_flag(argc, argv, "--wiggle")) {
		cmd_uart_wiggle(shell, argc, argv);
		return 0;
	}
	if (param_get_flag(argc, argv, "--sleep")) {
		cmd_uart_sleep(shell, argc, argv);
		return 0;
	}
	cmd_uart_wakeup(shell, argc, argv);
	return 0;
}

static int cmd_hci(const struct shell *shell, size_t argc, char **argv)
{

	/*
	 * 0x1a60_5008.
	 * AHI select register is bit 0
	 * HCI select register is bit 1
	 * AHI/HCI trace select register is bit 2.

	 * pinmux for B
	 * 0x10C	RTS
	 * 0x110	CTS
	 * 0x114	Trace
	 * 0x118	RX
	 * 0x11C	TX
	 */
	uint32_t state = 0;

	if (param_get_flag(argc, argv, "--ahi")) {
		state |= 0x01;
	}
	if (param_get_flag(argc, argv, "--hci")) {
		state |= 0x02;
	}
	if (param_get_flag(argc, argv, "--trace")) {
		state |= 0x04;
	}

	sys_write32(state, 0x1a605008);

	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_cmds, SHELL_CMD_ARG(start, NULL, "es0 start", cmd_start, 1, 10),
			       SHELL_CMD_ARG(stop, NULL, "es0 stop", cmd_stop, 1, 10),
			       SHELL_CMD_ARG(uart, NULL, "es0 uart wakeup --sleep --wiggle",
					     cmd_uart, 1, 10),
			       SHELL_CMD_ARG(hci, NULL, "Configure ext HCI", cmd_hci, 1, 10),
			       SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(pwr, &sub_cmds, "Power management test commands", NULL);

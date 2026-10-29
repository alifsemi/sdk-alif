/*
 * Copyright (c) Alif Semiconductor
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include "exerciser_app.h"

/* Shell command to start all exerciser threads */
static int cmd_start_exerciser(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	shell_print(sh, "Starting all exerciser threads...");
	start_all_exerciser_threads();
	shell_print(sh, "Exerciser threads initialization complete!");
	return 0;
}

/* Register shell command */
SHELL_CMD_REGISTER(start_exerciser, NULL, "Start all exerciser threads", cmd_start_exerciser);

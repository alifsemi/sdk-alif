/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <stdlib.h>
#include <string.h>

#define CERT_BUF_SIZE 16

/* Runtime context for the interactive cert menu. Reset on each invocation. */
static struct {
	char buf[CERT_BUF_SIZE];
	int buf_pos;
} cert_ctx;

static void show_cert_menu(const struct shell *sh)
{
	shell_print(sh, "Select certification:");
	shell_print(sh, "1. FCC");
	shell_print(sh, "2. ISED");
	shell_print(sh, "3. NCC");
	shell_print(sh, "4. JP");
	shell_print(sh, "0. Exit");
	shell_print(sh, "");
}

/* Prints the certification ID, brand name, and model name for the given
 * cert type. IDs are sourced from board Kconfig.defconfig.
 */
static void print_cert_info(const struct shell *sh, int cert_type)
{
	const char *id_label;
	const char *id_value;

	switch (cert_type) {
	case 1:
		id_label = "FCC ID";
		id_value = CONFIG_ALIF_CERT_FCC_ID;
		break;
	case 2:
		id_label = "ISED ID";
		id_value = CONFIG_ALIF_CERT_ISED_ID;
		break;
	case 3:
		id_label = "NCC ID";
		id_value = CONFIG_ALIF_CERT_NCC_ID;
		break;
	case 4:
		id_label = "JP ID";
		id_value = CONFIG_ALIF_CERT_JP_ID;
		break;
	default:
		return;
	}

	shell_print(sh, "");
	shell_print(sh, "%s: %s", id_label, id_value);
	shell_print(sh, "Brand Name: %s", CONFIG_ALIF_CERT_BRAND_NAME);
	shell_print(sh, "Model Name: %s", CONFIG_ALIF_CERT_MODEL_NAME);
	shell_print(sh, "");
}

/* Raw keystroke handler. The shell calls this instead of its normal command
 * processing while the menu is active. Calling shell_set_bypass(sh, NULL)
 * exits bypass mode and returns control to the shell.
 */
static void cert_bypass_cb(const struct shell *sh, uint8_t *recv, size_t len)
{
	for (size_t i = 0; i < len; i++) {
		char c = (char)recv[i];

		if (c == '\r' || c == '\n') {
			if (cert_ctx.buf_pos == 0) {
				continue;
			}

			shell_fprintf(sh, SHELL_NORMAL, "\n");
			cert_ctx.buf[cert_ctx.buf_pos] = '\0';
			cert_ctx.buf_pos = 0;

			int choice = atoi(cert_ctx.buf);

			if (choice == 0) {
				shell_set_bypass(sh, NULL);
			} else if (choice >= 1 && choice <= 4) {
				print_cert_info(sh, choice);
				show_cert_menu(sh);
			} else {
				shell_error(sh, "Invalid choice. Please enter 0-4.");
				show_cert_menu(sh);
			}
		} else if (cert_ctx.buf_pos < CERT_BUF_SIZE - 1) {
			/* Echo the character so the user can see what they typed */
			shell_fprintf(sh, SHELL_NORMAL, "%c", c);
			cert_ctx.buf[cert_ctx.buf_pos++] = c;
		}
	}
}

/* Shell command handler for "cert". Shows the certification type menu and
 * hands raw input over to cert_bypass_cb until the user exits.
 */
static int cmd_cert(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	memset(&cert_ctx, 0, sizeof(cert_ctx));
	show_cert_menu(sh);
	shell_set_bypass(sh, cert_bypass_cb);

	return 0;
}

SHELL_CMD_REGISTER(cert, NULL, "Display certification information", cmd_cert);

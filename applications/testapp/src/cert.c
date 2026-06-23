/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
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

/* Tracks which menu step the user is currently on. */
enum cert_state {
	CERT_STATE_MODEL,
	CERT_STATE_CERT_TYPE,
};

/* Runtime context for the interactive cert menu. Reset on each invocation. */
static struct {
	enum cert_state state;
	int model; /* 1 = DevKit, 2 = StartKit */
	char buf[CERT_BUF_SIZE];
	int buf_pos;
} cert_ctx;

/* Holds the display strings for a single product model. */
struct cert_info {
	const char *brand;
	const char *model_name;
	const char *fcc_id;
	const char *ised_id;
	const char *ncc_id;
	const char *jp_id;
};

/* Update the ID strings below once real cert IDs are available. Index 0 unused. */
static const struct cert_info cert_db[] = {
	[1] = { /* DevKit */
		.brand      = "Balletto DevKit",
		.model_name = "DK-B1",
		.fcc_id     = "2A58G-DKB1",
		.ised_id    = "xyz",
		.ncc_id     = "xyz",
		.jp_id      = "xyz",
	},
	[2] = { /* StartKit */
		.brand      = "Balletto StartKit",
		.model_name = "SK-B1",
		.fcc_id     = "xyz",
		.ised_id    = "xyz",
		.ncc_id     = "xyz",
		.jp_id      = "xyz",
	},
};

/* Prints the certification ID, brand name, and model name for the given
 * model + cert type selection.
 */
static void print_cert_info(const struct shell *sh, int model, int cert_type)
{
	const struct cert_info *info = &cert_db[model];
	const char *id_label;
	const char *id_value;

	switch (cert_type) {
	case 1:
		id_label = "FCC ID";
		id_value = info->fcc_id;
		break;
	case 2:
		id_label = "ISED ID";
		id_value = info->ised_id;
		break;
	case 3:
		id_label = "NCC ID";
		id_value = info->ncc_id;
		break;
	case 4:
		id_label = "JP ID";
		id_value = info->jp_id;
		break;
	default:
		return;
	}

	shell_print(sh, "%s: %s", id_label, id_value);
	shell_print(sh, "Brand Name: %s", info->brand);
	shell_print(sh, "Model Name: %s", info->model_name);
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

			if (cert_ctx.state == CERT_STATE_MODEL) {
				if (choice == 1 || choice == 2) {
					cert_ctx.model = choice;
					const char *name = (choice == 1) ? "DevKit" : "StartKit";

					shell_print(sh, "Selected: %s", name);
					cert_ctx.state = CERT_STATE_CERT_TYPE;
					shell_print(sh, "\nSelect certification:");
					shell_print(sh, "1. FCC");
					shell_print(sh, "2. ISED");
					shell_print(sh, "3. NCC");
					shell_print(sh, "4. JP");
					shell_print(sh, "0. Exit");
					shell_print(sh, "");
				} else {
					shell_error(sh, "Invalid choice. Please enter 1 or 2.");
					shell_print(sh, "1. DevKit");
					shell_print(sh, "2. StartKit");
					shell_print(sh, "");
				}
			} else {
				if (choice == 0) {
					shell_set_bypass(sh, NULL);
				} else if (choice >= 1 && choice <= 4) {
					shell_print(sh, "");
					print_cert_info(sh, cert_ctx.model, choice);
					shell_print(sh, "\nSelect certification:");
					shell_print(sh, "1. FCC");
					shell_print(sh, "2. ISED");
					shell_print(sh, "3. NCC");
					shell_print(sh, "4. JP");
					shell_print(sh, "0. Exit");
					shell_print(sh, "");
				} else {
					shell_error(sh, "Invalid choice. Please enter 0-4.");
					shell_print(sh, "1. FCC");
					shell_print(sh, "2. ISED");
					shell_print(sh, "3. NCC");
					shell_print(sh, "4. JP");
					shell_print(sh, "0. Exit");
					shell_print(sh, "");
				}
			}
		} else if (cert_ctx.buf_pos < CERT_BUF_SIZE - 1) {
			/* Echo the character so the user can see what they typed */
			shell_fprintf(sh, SHELL_NORMAL, "%c", c);
			cert_ctx.buf[cert_ctx.buf_pos++] = c;
		}
	}
}

/* Shell command handler for "cert". Resets state and shows the first menu,
 * then hands raw input over to cert_bypass_cb until the flow completes.
 */
static int cmd_cert(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	memset(&cert_ctx, 0, sizeof(cert_ctx));
	cert_ctx.state = CERT_STATE_MODEL;

	shell_print(sh, "Select model:");
	shell_print(sh, "1. DevKit");
	shell_print(sh, "2. StartKit");
	shell_print(sh, "");

	shell_set_bypass(sh, cert_bypass_cb);

	return 0;
}

SHELL_CMD_REGISTER(cert, NULL, "Display certification information", cmd_cert);

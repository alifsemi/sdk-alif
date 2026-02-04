/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/cache.h>
#include <zephyr/shell/shell.h>
#include <stdlib.h>

static uint32_t param_get_hex(size_t argc, char **argv, char *p_param, uint32_t def_value)
{
	if (p_param && argc > 1) {
		for (int n = 0; n < (argc - 1); n++) {
			if (strcmp(argv[n], p_param) == 0) {
				return (uint32_t)strtoul(argv[n + 1], NULL, 16);
			}
		}
	}
	return def_value;
}

static void cmd_read32(const struct shell *shell, size_t argc, char **argv)
{
	uint32_t from_addr = 0;

	from_addr = param_get_hex(argc, argv, "read32", from_addr);
	const volatile uint32_t *from_addr_ptr = (const volatile uint32_t *)from_addr;
	/* Ensure we really read the data from original memory instead of cache */
	sys_cache_data_invd_range((void *)from_addr_ptr, sizeof(uint32_t));

	shell_print(shell, "read32: 0x%08" PRIX32 " = 0x%08" PRIX32 "\n",
		from_addr, *from_addr_ptr);
}

static void cmd_write32(const struct shell *shell, size_t argc, char **argv)
{
	uint32_t to_addr = 0;
	uint32_t to_data = 0;

	to_addr = param_get_hex(argc, argv, "write32", to_addr);
	volatile uint32_t *to_addr_ptr = (volatile uint32_t *)to_addr;

	to_data = param_get_hex(argc, argv, "--data", to_data);

	*to_addr_ptr = to_data;
	sys_cache_data_flush_range((void *)to_addr_ptr, sizeof(uint32_t));

	shell_print(shell, "write32: 0x%08" PRIX32 " = 0x%08" PRIX32 "\n", to_addr, to_data);
}

SHELL_STATIC_SUBCMD_SET_CREATE(
	memory_sub_cmds,
	SHELL_CMD_ARG(read32, NULL, "Read 32 bit memory address. Address in hex after command.",
		cmd_read32, 0, 0),
	SHELL_CMD_ARG(write32, NULL, "Write 32 bit memory address. Address in hex after command.",
		cmd_write32, 0, 0),
	SHELL_SUBCMD_SET_END);
SHELL_CMD_REGISTER(memtest, &memory_sub_cmds, "Memory test commands", NULL);

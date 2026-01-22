/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/shell/shell.h>

/**
 * Implement bare minimum commands to run the internal Pytest test harness for automated tests.
 *
 * NOTE: This does not yet implement everything correctly in order to run all tests in 'tests'
 * folder currently run with baremetal testapp. Just the bare minimum for the harness initialization
 *       to go through and allow execution of selected tests. See TODOs below.
 *
 * TODO:
 *       - Reserve test memory areas from all retention lines and return correct pointers to them
 *         (might mean custom linker file).
 *       - Correctly deduce the used clock and dividers to determine and return current clock
 * frequency.
 *       - Some tests potentially rely on predetermined 'BOARD' definition strings that differ from
 * the ones used within Zephyr. Might beed mapping either here or in the Pytest harness.
 */

static int cmd_command_handler(const struct shell *sh, size_t argc, char **argv, void *data)
{
	(void)argc;
	(void)argv;
	int selection = (int)data;

	switch (selection) {
	case 1:
#if CONFIG_RTSS_HE
		shell_print(sh, "M55_HE");
#elif CONFIG_RTSS_HP
		shell_print(sh, "M55_HP");
#else
		shell_print(sh, "UNKNOWN");
#endif
		break;
	/* TODO: all the mem values to be configured if any mem tests are to be run */
	case 2:
		shell_print(sh, "DTCM1 = 0x00000000");
		break;
	case 3:
		shell_print(sh, "DTCM2 = 0x00000000");
		break;
	case 4:
		shell_print(sh, "DTCM3 = 0x00000000");
		break;
	case 5:
		shell_print(sh, "DTCM4 = 0x00000000");
		break;
	case 6:
		shell_print(sh, "DTCM5 = 0x00000000");
		break;
	case 7:
		shell_print(sh, "ITCM1 = 0x00000000");
		break;
	case 8:
		shell_print(sh, "ITCM2 = 0x00000000");
		break;
	case 9:
		shell_print(sh, "ITCM3 = 0x00000000");
		break;
	case 10:
		shell_print(sh, "ITCM4 = 0x00000000");
		break;
	case 11:
#if CONFIG_RTSS_HE
		shell_print(sh, "CLK = 160");
#elif CONFIG_RTSS_HP
		shell_print(sh, "CLK = 400");
#else
		shell_print(sh, "CLK = UNKNOWN");
#endif
		break;
	case 12:
#if CONFIG_FLASH_BASE_ADDRESS >= 0x80000000
		/* will report XIP to OSPI XIP build as well */
		shell_print(sh, "Run mode: XIP");
#elif CONFIG_FLASH_BASE_ADDRESS >= 0x20000000
		/* some config at somepoint pointed application to DTCM... */
		shell_print(sh, "Run mode: UNKNOWN");
#else
		shell_print(sh, "Run mode: TCM");
#endif
		break;
	case 13:
#if defined(__ARMCC_VERSION)
		shell_print(sh, "Compiler: COMPILER_ARM_CLANG");
#elif defined(__clang__)
		/* must be before GCC, as clang defines __GNUC__ too and after ARMClang as that
		 * defines __clang__ too.
		 */
		shell_print(sh, "Compiler: COMPILER_LLVM_CLANG");
#elif defined(__GNUC__)
		shell_print(sh, "Compiler: COMPILER_GNU_GCC");
#elif defined(__ICCARM__)
		shell_print(sh, "Compiler: COMPILER_IAR");
#else
		shell_print(sh, "Compiler: UNKNOWN");
#endif
		break;
	case 14:
		shell_print(sh, "Board: " CONFIG_BOARD);
		break;
	default:
		return -1;
	}

	return 0;
}

SHELL_SUBCMD_DICT_SET_CREATE(sub_commands, cmd_command_handler, (cpuid, 1, NULL),
			     (mem_dtcm1, 2, NULL), (mem_dtcm2, 3, NULL), (mem_dtcm3, 4, NULL),
			     (mem_dtcm4, 5, NULL), (mem_dtcm5, 6, NULL), (mem_itcm1, 7, NULL),
			     (mem_itcm2, 8, NULL), (mem_itcm3, 9, NULL), (mem_itcm4, 10, NULL),
			     (cpu_clk, 11, NULL), (run_mode, 12, NULL), (compiler, 13, NULL),
			     (board, 14, NULL));

SHELL_CMD_REGISTER(command, &sub_commands, NULL, NULL);

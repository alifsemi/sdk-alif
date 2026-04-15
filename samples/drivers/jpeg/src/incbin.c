/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#include "stdint.h"

#define INCBIN_DEF(name, path) \
	__asm__( \
	".global "#name"_start\n" \
	".global "#name"_end\n" \
	".section \".rodata\"\n" \
	""#name"_start:\n" \
	".incbin \"" INCBIN_BASE_DIR "/" path "\"\n" \
	""#name"_end:\n" \
	".previous\n" \
	);

INCBIN_DEF(testimg, "src/testing_images/1280x720.bin");

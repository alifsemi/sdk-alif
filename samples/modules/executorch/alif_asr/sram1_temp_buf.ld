/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/* Route the ExecuTorch temporary allocation pool (sTmpAllocationPool in
 * EtModel.cc, section .bss.NoInit.temp_buf_sram) into the 4 MB SRAM1 region.
 * The generic *(.bss.*) rule would otherwise place it in the 256 KB DTCM,
 * which overflows. NOLOAD keeps it out of the image and it is left un-zeroed. */
.temp_buf_sram (NOLOAD) :
{
	. = ALIGN(16);
	KEEP(*(.bss.NoInit.temp_buf_sram))
	. = ALIGN(16);
} > SRAM1

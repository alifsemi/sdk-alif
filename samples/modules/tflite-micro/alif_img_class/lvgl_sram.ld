/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/*
 * Relocate the LVGL rendering buffers (.lvgl_buf) and memory pool (.lvgl_heap)
 * into SRAM0. The D/AVE 2D GPU writes the partial-mode draw buffer and reads
 * glyph bitmaps from the pool as a bus master, so both must live in globally
 * addressable memory rather than DTCM.
 */
#if DT_NODE_HAS_STATUS(DT_NODELABEL(sram0), okay)
#if defined(CONFIG_LV_Z_VDB_CUSTOM_SECTION) || \
	defined(CONFIG_LV_Z_MEMORY_POOL_CUSTOM_SECTION)
GROUP_START(SRAM0)

	SECTION_PROLOGUE(.lvgl_sram, (NOLOAD),)
	{
#if defined(CONFIG_LV_Z_VDB_CUSTOM_SECTION)
		. = ALIGN(8);
		KEEP(*(.lvgl_buf))
		KEEP(*(".lvgl_buf.*"))
#endif
#if defined(CONFIG_LV_Z_MEMORY_POOL_CUSTOM_SECTION)
		. = ALIGN(8);
		KEEP(*(.lvgl_heap))
		KEEP(*(".lvgl_heap.*"))
#endif
	} GROUP_LINK_IN(SRAM0)

GROUP_END(SRAM0)
#endif
#endif

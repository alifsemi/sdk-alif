/*
 * SPDX-FileCopyrightText: Copyright Alif Semiconductor
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Relocate the LVGL rendering buffers (.lvgl_buf) and memory pool (.lvgl_heap)
 * into SRAM0. Both are rendered into by the D/AVE 2D GPU, which is a bus master;
 * SRAM0 is globally addressable, so the CPU, GPU and CDC200 all share the same
 * view. In DIRECT mode the VDBs also serve as the CDC200 scan-out framebuffers
 * (zero-copy page-flip).
 */
#if DT_NODE_HAS_STATUS(DT_NODELABEL(sram0), okay)
GROUP_START(SRAM0)

	SECTION_PROLOGUE(.lvgl_sram, (NOLOAD),)
	{
		. = ALIGN(8);
		KEEP(*(.lvgl_buf))
		KEEP(*(".lvgl_buf.*"))
		. = ALIGN(8);
		KEEP(*(.lvgl_heap))
		KEEP(*(".lvgl_heap.*"))
	} GROUP_LINK_IN(SRAM0)

GROUP_END(SRAM0)
#endif

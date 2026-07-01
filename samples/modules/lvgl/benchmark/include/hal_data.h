/*
 * SPDX-FileCopyrightText: Copyright Alif Semiconductor
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef HAL_DATA_H
#define HAL_DATA_H

/*
 * Compatibility shim for LVGL's D/AVE 2D draw unit.
 *
 * The upstream LVGL draw unit (src/draw/renesas/dave2d) was written against the
 * Renesas FSP, where "hal_data.h" transitively provides the TES D/AVE 2D driver
 * and the BSP cache configuration. On Alif the very same TES D/AVE 2D IP is
 * provided by the dave2d module, so we map the driver headers here.
 *
 * The draw unit performs its D-cache maintenance (clean/invalidate of the GPU
 * source and destination buffers) inside blocks guarded by RENESAS_CORTEX_M85
 * and BSP_CFG_DCACHE_ENABLED. The Alif Cortex-M55 has its D-cache enabled and
 * the SRAM holding the buffers is cacheable, so this maintenance is required for
 * CPU/GPU coherency. The code uses only portable CMSIS (SCB_*) and the Alif
 * d1_cacheblockflush() helper, both available here. Those two macros are defined
 * for the whole build (the LVGL module needs them too) by this sample's
 * CMakeLists.txt, so they are not redefined here.
 */
#include <cmsis_core.h>

#include <dave_driver.h>
#include <dave_d0lib.h>

/*
 * lv_draw_dave2d_cf_fb_get() determines the D/AVE 2D framebuffer mode from the
 * Renesas display driver configuration (g_display0_cfg). On Alif the LVGL
 * framebuffer always matches the configured LVGL colour depth, so we provide a
 * minimal stand-in that reports that fixed format.
 */
typedef enum {
	DISPLAY_IN_FORMAT_16BITS_RGB565 = 0,
	DISPLAY_IN_FORMAT_32BITS_ARGB8888,
	DISPLAY_IN_FORMAT_32BITS_RGB888,
	DISPLAY_IN_FORMAT_16BITS_ARGB4444,
	DISPLAY_IN_FORMAT_16BITS_ARGB1555,
	DISPLAY_IN_FORMAT_CLUT8,
	DISPLAY_IN_FORMAT_CLUT4,
	DISPLAY_IN_FORMAT_CLUT1,
} display_in_format_t;

typedef struct {
	display_in_format_t format;
} display_input_cfg_t;

typedef struct {
	const display_input_cfg_t *input;
} display_cfg_t;

static const display_input_cfg_t _alif_dave2d_fb_input = {
#if LV_COLOR_DEPTH == 32
	.format = DISPLAY_IN_FORMAT_32BITS_ARGB8888,
#else
	.format = DISPLAY_IN_FORMAT_16BITS_RGB565,
#endif
};

static const display_cfg_t g_display0_cfg = {
	.input = &_alif_dave2d_fb_input,
};

#endif /* HAL_DATA_H */

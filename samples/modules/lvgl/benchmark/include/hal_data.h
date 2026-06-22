/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#ifndef HAL_DATA_H
#define HAL_DATA_H

/*
 * Compatibility shim for LVGL's D/AVE 2D draw unit.
 *
 * The upstream D/AVE 2D draw unit transitively provides the driver
 * and the BSP cache configuration. This file is required to provide
 * a minimal stand-in for the D/AVE 2D driver and BSP cache
 * configuration, so that the D/AVE 2D draw unit can be used on Alif.
 */
#include <cmsis_core.h>

#include <dave_driver.h>
#include <dave_d0lib.h>

/*
 * lv_draw_dave2d_cf_fb_get() determines the D/AVE 2D framebuffer mode
 * from the display driver configuration (g_display0_cfg). On Alif the
 * LVGL framebuffer always matches the configured LVGL colour depth,
 * so we provide a minimal stand-in that reports that fixed format.
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

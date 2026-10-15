/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#ifndef BSP_API_H
#define BSP_API_H

#include <cmsis_core.h>
#include <dave_driver.h>
#include <dave_d0lib.h>
#include <zephyr/devicetree.h>

/*
 * Geometry the D/AVE 2D draw unit's framebuffer helper expects. Taken from the
 * chosen display so it follows the CDC200 width and height in the board
 * overlay. The live draw path sizes each buffer from LVGL; these macros are
 * the full-panel target used by that helper.
 */
#define DISPLAY_HSIZE_INPUT0                DT_PROP(DT_CHOSEN(zephyr_display), width)
#define DISPLAY_VSIZE_INPUT0                DT_PROP(DT_CHOSEN(zephyr_display), height)
#define DISPLAY_BUFFER_STRIDE_PIXELS_INPUT0 DISPLAY_HSIZE_INPUT0

static inline d2_s32 lv_draw_dave2d_cf_fb_get(void)
{
#if defined(LV_COLOR_DEPTH) && LV_COLOR_DEPTH == 32
	return d2_mode_argb8888;
#else
	return d2_mode_rgb565;
#endif
}

#endif /* BSP_API_H */

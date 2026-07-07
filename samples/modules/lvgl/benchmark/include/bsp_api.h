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

/*
 * Compatibility shim for LVGL's D/AVE 2D draw unit (LVGL 9.5.0+).
 *
 * Since v9.5.0 the upstream D/AVE 2D draw unit header (lv_draw_dave2d.h)
 * includes "bsp_api.h" - the FSP BSP umbrella header - instead of
 * "hal_data.h". On the FSP that header transitively provides CMSIS, the
 * generated display-geometry macros (DISPLAY_*_INPUT0) and the framebuffer
 * colour-format helper lv_draw_dave2d_cf_fb_get(). Alif has no FSP, so this
 * shim supplies the small subset the draw unit actually uses.
 *
 * The draw unit's D-cache maintenance runs under ARM_CORTEX_M55_M85 (defined by
 * this sample's CMakeLists.txt), so it uses portable CMSIS SCB_* helpers and the
 * Alif d1_cacheblockflush() helper - both provided by the headers below.
 */

#include <cmsis_core.h>   /* SCB_CleanInvalidateDCache_by_Addr() on the M55 */
#include <dave_driver.h>  /* d2_* types used by lv_draw_dave2d_cf_fb_get() */
#include <dave_d0lib.h>   /* d1_cacheblockflush() */

/*
 * D/AVE 2D render-target geometry. In DIRECT mode the LVGL VDBs are full-screen
 * framebuffers, so this matches the 480x800 MW405 panel configured for the
 * CDC200 controller in the board overlay (keep in sync with cdc200 width/height).
 */
#define DISPLAY_HSIZE_INPUT0                480
#define DISPLAY_VSIZE_INPUT0                800
#define DISPLAY_BUFFER_STRIDE_PIXELS_INPUT0 480

/*
 * D/AVE 2D framebuffer colour format. On Alif the LVGL framebuffer always
 * matches the configured LVGL colour depth.
 */
static inline d2_s32 lv_draw_dave2d_cf_fb_get(void)
{
#if defined(LV_COLOR_DEPTH) && LV_COLOR_DEPTH == 32
	return d2_mode_argb8888;
#else
	return d2_mode_rgb565;
#endif
}

#endif /* BSP_API_H */

/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/******************************************************************************
 * @file    aipl_color_conversion_default.c
 * @brief   Helium accelerated color conversion function implementations
 *
 ******************************************************************************/

/*********************
 *      INCLUDES
 *********************/
#include "aipl_color_conversion_default.h"

#include <string.h>

#include "aipl_config.h"
#include "aipl_utils.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
#if (AIPL_CONVERT_ALPHA8_I400 & TO_BGR888 | AIPL_CONVERT_ALPHA8_I400 & TO_RGB888) &&               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_alpha8_to_24bit_default(const void *input, void *output,
							uint32_t pitch, uint32_t width,
							uint32_t height, uint8_t r_offset,
							uint8_t g_offset, uint8_t b_offset);
#endif
#if (AIPL_CONVERT_ARGB8888 & TO_BGR888 | AIPL_CONVERT_ARGB8888 & TO_RGB888) &&                     \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb8888_to_24bit_default(const void *input, void *output,
							  uint32_t pitch, uint32_t width,
							  uint32_t height, uint8_t r_offset,
							  uint8_t g_offset, uint8_t b_offset);
#endif
#if (AIPL_CONVERT_ARGB4444 & TO_BGR888 | AIPL_CONVERT_ARGB4444 & TO_RGB888) &&                     \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb4444_to_24bit_default(const void *input, void *output,
							  uint32_t pitch, uint32_t width,
							  uint32_t height, uint8_t r_offset,
							  uint8_t g_offset, uint8_t b_offset);
#endif
#if (AIPL_CONVERT_ARGB1555 & TO_BGR888 | AIPL_CONVERT_ARGB1555 & TO_RGB888) &&                     \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb1555_to_24bit_default(const void *input, void *output,
							  uint32_t pitch, uint32_t width,
							  uint32_t height, uint8_t r_offset,
							  uint8_t g_offset, uint8_t b_offset);
#endif
#if (AIPL_CONVERT_RGBA8888 & TO_BGR888 | AIPL_CONVERT_RGBA8888 & TO_RGB888) &&                     \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba8888_to_24bit_default(const void *input, void *output,
							  uint32_t pitch, uint32_t width,
							  uint32_t height, uint8_t r_offset,
							  uint8_t g_offset, uint8_t b_offset);
#endif
#if (AIPL_CONVERT_RGBA4444 & TO_BGR888 | AIPL_CONVERT_RGBA4444 & TO_RGB888) &&                     \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba4444_to_24bit_default(const void *input, void *output,
							  uint32_t pitch, uint32_t width,
							  uint32_t height, uint8_t r_offset,
							  uint8_t g_offset, uint8_t b_offset);
#endif
#if (AIPL_CONVERT_RGBA5551 & TO_BGR888 | AIPL_CONVERT_RGBA5551 & TO_RGB888) &&                     \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba5551_to_24bit_default(const void *input, void *output,
							  uint32_t pitch, uint32_t width,
							  uint32_t height, uint8_t r_offset,
							  uint8_t g_offset, uint8_t b_offset);
#endif
#if (AIPL_CONVERT_RGB565 & TO_BGR888 | AIPL_CONVERT_RGB565 & TO_RGB888) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb565_to_24bit_default(const void *input, void *output,
							uint32_t pitch, uint32_t width,
							uint32_t height, uint8_t r_offset,
							uint8_t g_offset, uint8_t b_offset);
#endif
#if (AIPL_CONVERT_RGB888 & TO_BGR888 | AIPL_CONVERT_BGR888 & TO_RGB888) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_24bit_to_24bit_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height, uint8_t r_in_offset,
						       uint8_t g_in_offset, uint8_t b_in_offset,
						       uint8_t r_out_offset, uint8_t g_out_offset,
						       uint8_t b_out_offset);
#endif
#if (AIPL_CONVERT_RGB888 & TO_ALPHA8_I400 | AIPL_CONVERT_BGR888 & TO_ALPHA8_I400) &&               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_24bit_to_alpha8_default(const void *input, void *output,
							uint32_t pitch, uint32_t width,
							uint32_t height, uint8_t r_offset,
							uint8_t g_offset, uint8_t b_offset);
#endif
#if (AIPL_CONVERT_RGB888 & TO_ARGB8888 | AIPL_CONVERT_BGR888 & TO_ARGB8888) &&                     \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_24bit_to_argb8888_default(const void *input, void *output,
							  uint32_t pitch, uint32_t width,
							  uint32_t height, uint8_t r_offset,
							  uint8_t g_offset, uint8_t b_offset);
#endif
#if (AIPL_CONVERT_RGB888 & TO_ARGB4444 | AIPL_CONVERT_BGR888 & TO_ARGB4444) &&                     \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_24bit_to_argb4444_default(const void *input, void *output,
							  uint32_t pitch, uint32_t width,
							  uint32_t height, uint8_t r_offset,
							  uint8_t g_offset, uint8_t b_offset);
#endif
#if (AIPL_CONVERT_RGB888 & TO_ARGB1555 | AIPL_CONVERT_BGR888 & TO_ARGB1555) &&                     \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_24bit_to_argb1555_default(const void *input, void *output,
							  uint32_t pitch, uint32_t width,
							  uint32_t height, uint8_t r_offset,
							  uint8_t g_offset, uint8_t b_offset);
#endif
#if (AIPL_CONVERT_RGB888 & TO_RGBA8888 | AIPL_CONVERT_BGR888 & TO_RGBA8888) &&                     \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_24bit_to_rgba8888_default(const void *input, void *output,
							  uint32_t pitch, uint32_t width,
							  uint32_t height, uint8_t r_offset,
							  uint8_t g_offset, uint8_t b_offset);
#endif
#if (AIPL_CONVERT_RGB888 & TO_RGBA4444 | AIPL_CONVERT_BGR888 & TO_RGBA4444) &&                     \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_24bit_to_rgba4444_default(const void *input, void *output,
							  uint32_t pitch, uint32_t width,
							  uint32_t height, uint8_t r_offset,
							  uint8_t g_offset, uint8_t b_offset);
#endif
#if (AIPL_CONVERT_RGB888 & TO_RGBA5551 | AIPL_CONVERT_BGR888 & TO_RGBA5551) &&                     \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_24bit_to_rgba5551_default(const void *input, void *output,
							  uint32_t pitch, uint32_t width,
							  uint32_t height, uint8_t r_offset,
							  uint8_t g_offset, uint8_t b_offset);
#endif
#if (AIPL_CONVERT_RGB888 & TO_RGB565 | AIPL_CONVERT_BGR888 & TO_RGB565) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_24bit_to_rgb565_default(const void *input, void *output,
							uint32_t pitch, uint32_t width,
							uint32_t height, uint8_t r_offset,
							uint8_t g_offset, uint8_t b_offset);
#endif
#if (AIPL_CONVERT_ARGB8888 & TO_YV12 | AIPL_CONVERT_ARGB8888 & TO_I420) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb8888_to_yuv_planar_default(const void *input, uint32_t pitch,
							       uint32_t width, uint32_t height,
							       uint8_t *y_ptr, uint8_t *u_ptr,
							       uint8_t *v_ptr);
#endif
#if (AIPL_CONVERT_ARGB8888 & TO_NV12 | AIPL_CONVERT_ARGB8888 & TO_NV21) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb8888_to_yuv_semi_planar_default(const void *input,
								    uint32_t pitch, uint32_t width,
								    uint32_t height, uint8_t *y_ptr,
								    uint8_t *u_ptr, uint8_t *v_ptr);
#endif
#if (AIPL_CONVERT_ARGB8888 & TO_YUY2 | AIPL_CONVERT_ARGB8888 & TO_UYVY) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb8888_to_yuv_packed_default(const void *input, uint32_t pitch,
							       uint32_t width, uint32_t height,
							       uint8_t *y_ptr, uint8_t *u_ptr,
							       uint8_t *v_ptr);
#endif
#if (AIPL_CONVERT_ARGB4444 & TO_YV12 | AIPL_CONVERT_ARGB4444 & TO_I420) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb4444_to_yuv_planar_default(const void *input, uint32_t pitch,
							       uint32_t width, uint32_t height,
							       uint8_t *y_ptr, uint8_t *u_ptr,
							       uint8_t *v_ptr);
#endif
#if (AIPL_CONVERT_ARGB4444 & TO_NV12 | AIPL_CONVERT_ARGB4444 & TO_NV21) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb4444_to_yuv_semi_planar_default(const void *input,
								    uint32_t pitch, uint32_t width,
								    uint32_t height, uint8_t *y_ptr,
								    uint8_t *u_ptr, uint8_t *v_ptr);
#endif
#if (AIPL_CONVERT_ARGB4444 & TO_YUY2 | AIPL_CONVERT_ARGB4444 & TO_UYVY) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb4444_to_yuv_packed_default(const void *input, uint32_t pitch,
							       uint32_t width, uint32_t height,
							       uint8_t *y_ptr, uint8_t *u_ptr,
							       uint8_t *v_ptr);
#endif
#if (AIPL_CONVERT_ARGB1555 & TO_YV12 | AIPL_CONVERT_ARGB1555 & TO_I420) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb1555_to_yuv_planar_default(const void *input, uint32_t pitch,
							       uint32_t width, uint32_t height,
							       uint8_t *y_ptr, uint8_t *u_ptr,
							       uint8_t *v_ptr);
#endif
#if (AIPL_CONVERT_ARGB1555 & TO_NV12 | AIPL_CONVERT_ARGB1555 & TO_NV21) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb1555_to_yuv_semi_planar_default(const void *input,
								    uint32_t pitch, uint32_t width,
								    uint32_t height, uint8_t *y_ptr,
								    uint8_t *u_ptr, uint8_t *v_ptr);
#endif
#if (AIPL_CONVERT_ARGB1555 & TO_YUY2 | AIPL_CONVERT_ARGB1555 & TO_UYVY) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb1555_to_yuv_packed_default(const void *input, uint32_t pitch,
							       uint32_t width, uint32_t height,
							       uint8_t *y_ptr, uint8_t *u_ptr,
							       uint8_t *v_ptr);
#endif
#if (AIPL_CONVERT_RGBA8888 & TO_YV12 | AIPL_CONVERT_RGBA8888 & TO_I420) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba8888_to_yuv_planar_default(const void *input, uint32_t pitch,
							       uint32_t width, uint32_t height,
							       uint8_t *y_ptr, uint8_t *u_ptr,
							       uint8_t *v_ptr);
#endif
#if (AIPL_CONVERT_RGBA8888 & TO_NV12 | AIPL_CONVERT_RGBA8888 & TO_NV21) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba8888_to_yuv_semi_planar_default(const void *input,
								    uint32_t pitch, uint32_t width,
								    uint32_t height, uint8_t *y_ptr,
								    uint8_t *u_ptr, uint8_t *v_ptr);
#endif
#if (AIPL_CONVERT_RGBA8888 & TO_YUY2 | AIPL_CONVERT_RGBA8888 & TO_UYVY) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba8888_to_yuv_packed_default(const void *input, uint32_t pitch,
							       uint32_t width, uint32_t height,
							       uint8_t *y_ptr, uint8_t *u_ptr,
							       uint8_t *v_ptr);
#endif
#if (AIPL_CONVERT_RGBA4444 & TO_YV12 | AIPL_CONVERT_RGBA4444 & TO_I420) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba4444_to_yuv_planar_default(const void *input, uint32_t pitch,
							       uint32_t width, uint32_t height,
							       uint8_t *y_ptr, uint8_t *u_ptr,
							       uint8_t *v_ptr);
#endif
#if (AIPL_CONVERT_RGBA4444 & TO_NV12 | AIPL_CONVERT_RGBA4444 & TO_NV21) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba4444_to_yuv_semi_planar_default(const void *input,
								    uint32_t pitch, uint32_t width,
								    uint32_t height, uint8_t *y_ptr,
								    uint8_t *u_ptr, uint8_t *v_ptr);
#endif
#if (AIPL_CONVERT_RGBA4444 & TO_YUY2 | AIPL_CONVERT_RGBA4444 & TO_UYVY) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba4444_to_yuv_packed_default(const void *input, uint32_t pitch,
							       uint32_t width, uint32_t height,
							       uint8_t *y_ptr, uint8_t *u_ptr,
							       uint8_t *v_ptr);
#endif
#if (AIPL_CONVERT_RGBA5551 & TO_YV12 | AIPL_CONVERT_RGBA5551 & TO_I420) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba5551_to_yuv_planar_default(const void *input, uint32_t pitch,
							       uint32_t width, uint32_t height,
							       uint8_t *y_ptr, uint8_t *u_ptr,
							       uint8_t *v_ptr);
#endif
#if (AIPL_CONVERT_RGBA5551 & TO_NV12 | AIPL_CONVERT_RGBA5551 & TO_NV21) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba5551_to_yuv_semi_planar_default(const void *input,
								    uint32_t pitch, uint32_t width,
								    uint32_t height, uint8_t *y_ptr,
								    uint8_t *u_ptr, uint8_t *v_ptr);
#endif
#if (AIPL_CONVERT_RGBA5551 & TO_YUY2 | AIPL_CONVERT_RGBA5551 & TO_UYVY) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba5551_to_yuv_packed_default(const void *input, uint32_t pitch,
							       uint32_t width, uint32_t height,
							       uint8_t *y_ptr, uint8_t *u_ptr,
							       uint8_t *v_ptr);
#endif
#if (AIPL_CONVERT_BGR888 & TO_YV12 | AIPL_CONVERT_BGR888 & TO_I420 |                               \
	 AIPL_CONVERT_RGB888 & TO_YV12 | AIPL_CONVERT_RGB888 & TO_I420) &&                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_24bit_to_yuv_planar_default(const void *input, uint32_t pitch,
							    uint32_t width, uint32_t height,
							    uint8_t *y_ptr, uint8_t *u_ptr,
							    uint8_t *v_ptr, uint8_t r_offset,
							    uint8_t g_offset, uint8_t b_offset);
#endif
#if (AIPL_CONVERT_BGR888 & TO_NV12 | AIPL_CONVERT_BGR888 & TO_NV21 |                               \
	 AIPL_CONVERT_RGB888 & TO_NV12 | AIPL_CONVERT_RGB888 & TO_NV21) &&                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_24bit_to_yuv_semi_planar_default(
	const void *input, uint32_t pitch, uint32_t width, uint32_t height, uint8_t *y_ptr,
	uint8_t *u_ptr, uint8_t *v_ptr, uint8_t r_offset, uint8_t g_offset, uint8_t b_offset);
#endif
#if (AIPL_CONVERT_RGB888 & TO_YUY2 | AIPL_CONVERT_RGB888 & TO_UYVY) &&                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb888_to_yuv_packed_default(const void *input, uint32_t pitch,
							     uint32_t width, uint32_t height,
							     uint8_t *y_ptr, uint8_t *u_ptr,
							     uint8_t *v_ptr);
#endif
#if (AIPL_CONVERT_BGR888 & TO_YUY2 | AIPL_CONVERT_BGR888 & TO_UYVY) &&                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_bgr888_to_yuv_packed_default(const void *input, uint32_t pitch,
							     uint32_t width, uint32_t height,
							     uint8_t *y_ptr, uint8_t *u_ptr,
							     uint8_t *v_ptr);
#endif
#if (AIPL_CONVERT_I422 & TO_BGR888 | AIPL_CONVERT_I422 & TO_RGB888) &&                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i422_to_24bit_default(const void *input, void *output,
						      uint32_t pitch, uint32_t width,
						      uint32_t height, uint8_t r_offset,
						      uint8_t g_offset, uint8_t b_offset);
#endif
#if (AIPL_CONVERT_I444 & TO_BGR888 | AIPL_CONVERT_I444 & TO_RGB888) &&                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i444_to_24bit_default(const void *input, void *output,
						      uint32_t pitch, uint32_t width,
						      uint32_t height, uint8_t r_offset,
						      uint8_t g_offset, uint8_t b_offset);
#endif
#if (AIPL_CONVERT_RGB565 & TO_YV12 | AIPL_CONVERT_RGB565 & TO_I420) &&                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb565_to_yuv_planar_default(const void *input, uint32_t pitch,
							     uint32_t width, uint32_t height,
							     uint8_t *y_ptr, uint8_t *u_ptr,
							     uint8_t *v_ptr);
#endif
#if (AIPL_CONVERT_RGB565 & TO_NV12 | AIPL_CONVERT_RGB565 & TO_NV21) &&                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb565_to_yuv_semi_planar_default(const void *input, uint32_t pitch,
								  uint32_t width, uint32_t height,
								  uint8_t *y_ptr, uint8_t *u_ptr,
								  uint8_t *v_ptr);
#endif
#if (AIPL_CONVERT_RGB565 & TO_YUY2 | AIPL_CONVERT_RGB565 & TO_UYVY) &&                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb565_to_yuv_packed_default(const void *input, uint32_t pitch,
							     uint32_t width, uint32_t height,
							     uint8_t *y_ptr, uint8_t *u_ptr,
							     uint8_t *v_ptr);
#endif
#if (AIPL_CONVERT_YV12 & TO_ALPHA8_I400 | AIPL_CONVERT_I420 & TO_ALPHA8_I400 |                     \
	 AIPL_CONVERT_NV12 & TO_ALPHA8_I400 | AIPL_CONVERT_NV21 & TO_ALPHA8_I400 |                     \
	 AIPL_CONVERT_I422 & TO_ALPHA8_I400 | AIPL_CONVERT_I444 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_yuv_planar_to_alpha8_default(const void *input, void *output,
							     uint32_t pitch, uint32_t width,
							     uint32_t height);
#endif
#if (AIPL_CONVERT_YUY2 & TO_ALPHA8_I400 | AIPL_CONVERT_UYVY & TO_ALPHA8_I400) &&                   \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_packed_to_alpha8_default(const void *input, void *output,
							     uint32_t pitch, uint32_t width,
							     uint32_t height);
#endif
#if (AIPL_CONVERT_YV12 & TO_ARGB8888 | AIPL_CONVERT_I420 & TO_ARGB8888) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_planar_to_argb8888_default(const uint8_t *y_ptr,
							       const uint8_t *u_ptr,
							       const uint8_t *v_ptr, void *output,
							       uint32_t pitch, uint32_t width,
							       uint32_t height);
#endif
#if (AIPL_CONVERT_NV12 & TO_ARGB8888 | AIPL_CONVERT_NV21 & TO_ARGB8888) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_semi_planar_to_argb8888_default(
	const uint8_t *y_ptr, const uint8_t *u_ptr, const uint8_t *v_ptr, void *output,
	uint32_t pitch, uint32_t width, uint32_t height);
#endif
#if (AIPL_CONVERT_YUY2 & TO_ARGB8888 | AIPL_CONVERT_UYVY & TO_ARGB8888) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_packed_to_argb8888_default(const uint8_t *y_ptr,
							       const uint8_t *u_ptr,
							       const uint8_t *v_ptr, void *output,
							       uint32_t pitch, uint32_t width,
							       uint32_t height);
#endif
#if (AIPL_CONVERT_YV12 & TO_ARGB4444 | AIPL_CONVERT_I420 & TO_ARGB4444) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_planar_to_argb4444_default(const uint8_t *y_ptr,
							       const uint8_t *u_ptr,
							       const uint8_t *v_ptr, void *output,
							       uint32_t pitch, uint32_t width,
							       uint32_t height);
#endif
#if (AIPL_CONVERT_NV12 & TO_ARGB4444 | AIPL_CONVERT_NV21 & TO_ARGB4444) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_semi_planar_to_argb4444_default(
	const uint8_t *y_ptr, const uint8_t *u_ptr, const uint8_t *v_ptr, void *output,
	uint32_t pitch, uint32_t width, uint32_t height);
#endif
#if (AIPL_CONVERT_YUY2 & TO_ARGB4444 | AIPL_CONVERT_UYVY & TO_ARGB4444) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_packed_to_argb4444_default(const uint8_t *y_ptr,
							       const uint8_t *u_ptr,
							       const uint8_t *v_ptr, void *output,
							       uint32_t pitch, uint32_t width,
							       uint32_t height);
#endif
#if (AIPL_CONVERT_YV12 & TO_ARGB1555 | AIPL_CONVERT_I420 & TO_ARGB1555) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_planar_to_argb1555_default(const uint8_t *y_ptr,
							       const uint8_t *u_ptr,
							       const uint8_t *v_ptr, void *output,
							       uint32_t pitch, uint32_t width,
							       uint32_t height);
#endif
#if (AIPL_CONVERT_NV12 & TO_ARGB1555 | AIPL_CONVERT_NV21 & TO_ARGB1555) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_semi_planar_to_argb1555_default(
	const uint8_t *y_ptr, const uint8_t *u_ptr, const uint8_t *v_ptr, void *output,
	uint32_t pitch, uint32_t width, uint32_t height);
#endif
#if (AIPL_CONVERT_YUY2 & TO_ARGB1555 | AIPL_CONVERT_UYVY & TO_ARGB1555) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_packed_to_argb1555_default(const uint8_t *y_ptr,
							       const uint8_t *u_ptr,
							       const uint8_t *v_ptr, void *output,
							       uint32_t pitch, uint32_t width,
							       uint32_t height);
#endif
#if (AIPL_CONVERT_YV12 & TO_RGBA8888 | AIPL_CONVERT_I420 & TO_RGBA8888) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_planar_to_rgba8888_default(const uint8_t *y_ptr,
							       const uint8_t *u_ptr,
							       const uint8_t *v_ptr, void *output,
							       uint32_t pitch, uint32_t width,
							       uint32_t height);
#endif
#if (AIPL_CONVERT_NV12 & TO_RGBA8888 | AIPL_CONVERT_NV21 & TO_RGBA8888) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_semi_planar_to_rgba8888_default(
	const uint8_t *y_ptr, const uint8_t *u_ptr, const uint8_t *v_ptr, void *output,
	uint32_t pitch, uint32_t width, uint32_t height);
#endif
#if (AIPL_CONVERT_YUY2 & TO_RGBA8888 | AIPL_CONVERT_UYVY & TO_RGBA8888) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_packed_to_rgba8888_default(const uint8_t *y_ptr,
							       const uint8_t *u_ptr,
							       const uint8_t *v_ptr, void *output,
							       uint32_t pitch, uint32_t width,
							       uint32_t height);
#endif
#if (AIPL_CONVERT_YV12 & TO_RGBA4444 | AIPL_CONVERT_I420 & TO_RGBA4444) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_planar_to_rgba4444_default(const uint8_t *y_ptr,
							       const uint8_t *u_ptr,
							       const uint8_t *v_ptr, void *output,
							       uint32_t pitch, uint32_t width,
							       uint32_t height);
#endif
#if (AIPL_CONVERT_NV12 & TO_RGBA4444 | AIPL_CONVERT_NV21 & TO_RGBA4444) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_semi_planar_to_rgba4444_default(
	const uint8_t *y_ptr, const uint8_t *u_ptr, const uint8_t *v_ptr, void *output,
	uint32_t pitch, uint32_t width, uint32_t height);
#endif
#if (AIPL_CONVERT_YUY2 & TO_RGBA4444 | AIPL_CONVERT_UYVY & TO_RGBA4444) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_packed_to_rgba4444_default(const uint8_t *y_ptr,
							       const uint8_t *u_ptr,
							       const uint8_t *v_ptr, void *output,
							       uint32_t pitch, uint32_t width,
							       uint32_t height);
#endif
#if (AIPL_CONVERT_YV12 & TO_RGBA5551 | AIPL_CONVERT_I420 & TO_RGBA5551) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_planar_to_rgba5551_default(const uint8_t *y_ptr,
							       const uint8_t *u_ptr,
							       const uint8_t *v_ptr, void *output,
							       uint32_t pitch, uint32_t width,
							       uint32_t height);
#endif
#if (AIPL_CONVERT_NV12 & TO_RGBA5551 | AIPL_CONVERT_NV21 & TO_RGBA5551) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_semi_planar_to_rgba5551_default(
	const uint8_t *y_ptr, const uint8_t *u_ptr, const uint8_t *v_ptr, void *output,
	uint32_t pitch, uint32_t width, uint32_t height);
#endif
#if (AIPL_CONVERT_YUY2 & TO_RGBA5551 | AIPL_CONVERT_UYVY & TO_RGBA5551) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_packed_to_rgba5551_default(const uint8_t *y_ptr,
							       const uint8_t *u_ptr,
							       const uint8_t *v_ptr, void *output,
							       uint32_t pitch, uint32_t width,
							       uint32_t height);
#endif
#if (AIPL_CONVERT_YV12 & TO_BGR888 | AIPL_CONVERT_I420 & TO_BGR888 |                               \
	 AIPL_CONVERT_YV12 & TO_RGB888 | AIPL_CONVERT_I420 & TO_RGB888) &&                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_planar_to_24bit_default(const uint8_t *y_ptr,
							    const uint8_t *u_ptr,
							    const uint8_t *v_ptr, void *output,
							    uint32_t pitch, uint32_t width,
							    uint32_t height, uint8_t r_offset,
							    uint8_t g_offset, uint8_t b_offset);
#endif
#if (AIPL_CONVERT_NV12 & TO_BGR888 | AIPL_CONVERT_NV21 & TO_BGR888 |                               \
	 AIPL_CONVERT_NV12 & TO_RGB888 | AIPL_CONVERT_NV21 & TO_RGB888) &&                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_semi_planar_to_24bit_default(
	const uint8_t *y_ptr, const uint8_t *u_ptr, const uint8_t *v_ptr, void *output,
	uint32_t pitch, uint32_t width, uint32_t height, uint8_t r_offset, uint8_t g_offset,
	uint8_t b_offset);
#endif
#if (AIPL_CONVERT_YUY2 & TO_BGR888 | AIPL_CONVERT_UYVY & TO_BGR888 |                               \
	 AIPL_CONVERT_YUY2 & TO_RGB888 | AIPL_CONVERT_UYVY & TO_RGB888) &&                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_packed_to_24bit_default(const uint8_t *y_ptr,
							    const uint8_t *u_ptr,
							    const uint8_t *v_ptr, void *output,
							    uint32_t pitch, uint32_t width,
							    uint32_t height, uint8_t r_offset,
							    uint8_t g_offset, uint8_t b_offset);
#endif
#if (AIPL_CONVERT_YV12 & TO_RGB565 | AIPL_CONVERT_I420 & TO_RGB565) &&                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_planar_to_rgb565_default(const uint8_t *y_ptr,
							     const uint8_t *u_ptr,
							     const uint8_t *v_ptr, void *output,
							     uint32_t pitch, uint32_t width,
							     uint32_t height);
#endif
#if (AIPL_CONVERT_NV12 & TO_RGB565 | AIPL_CONVERT_NV21 & TO_RGB565) &&                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_semi_planar_to_rgb565_default(const uint8_t *y_ptr,
								  const uint8_t *u_ptr,
								  const uint8_t *v_ptr,
								  void *output, uint32_t pitch,
								  uint32_t width, uint32_t height);
#endif
#if (AIPL_CONVERT_YUY2 & TO_RGB565 | AIPL_CONVERT_UYVY & TO_RGB565) &&                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_packed_to_rgb565_default(const uint8_t *y_ptr,
							     const uint8_t *u_ptr,
							     const uint8_t *v_ptr, void *output,
							     uint32_t pitch, uint32_t width,
							     uint32_t height);
#endif
#if (AIPL_CONVERT_YV12 & TO_I420 | AIPL_CONVERT_I420 & TO_YV12)
aipl_error_t aipl_color_convert_yuv_planar_to_planar_default(
	const uint8_t *y_src, const uint8_t *u_src, const uint8_t *v_src, uint8_t *y_dst,
	uint8_t *u_dst, uint8_t *v_dst, uint32_t pitch, uint32_t width, uint32_t height);
#endif
#if (AIPL_CONVERT_YV12 & (TO_NV12 | TO_NV21) | AIPL_CONVERT_I420 & (TO_NV12 | TO_NV21)) &&         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_planar_to_semi_default(
	const uint8_t *y_src, const uint8_t *u_src, const uint8_t *v_src, uint8_t *y_dst,
	uint8_t *u_dst, uint8_t *v_dst, uint32_t pitch, uint32_t width, uint32_t height);
#endif
#if (AIPL_CONVERT_YV12 & (TO_YUY2 | TO_UYVY) | AIPL_CONVERT_I420 & (TO_YUY2 | TO_UYVY))
aipl_error_t aipl_color_convert_yuv_planar_to_packed_default(
	const uint8_t *y_src, const uint8_t *u_src, const uint8_t *v_src, uint8_t *y_dst,
	uint8_t *u_dst, uint8_t *v_dst, uint32_t pitch, uint32_t width, uint32_t height);
#endif
#if (AIPL_CONVERT_NV12 & (TO_YV12 | TO_I420) | AIPL_CONVERT_NV21 & (TO_YV12 | TO_I420)) &&         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_semi_to_planar_default(
	const uint8_t *y_src, const uint8_t *u_src, const uint8_t *v_src, uint8_t *y_dst,
	uint8_t *u_dst, uint8_t *v_dst, uint32_t pitch, uint32_t width, uint32_t height);
#endif
#if (AIPL_CONVERT_NV12 & TO_NV21 | AIPL_CONVERT_NV21 & TO_NV12)
aipl_error_t aipl_color_convert_yuv_semi_to_semi_planar_default(
	const uint8_t *y_src, const uint8_t *u_src, const uint8_t *v_src, uint8_t *y_dst,
	uint8_t *u_dst, uint8_t *v_dst, uint32_t pitch, uint32_t width, uint32_t height);
#endif
#if (AIPL_CONVERT_NV12 & (TO_YUY2 | TO_UYVY) | AIPL_CONVERT_NV21 & (TO_YUY2 | TO_UYVY))
aipl_error_t aipl_color_convert_yuv_semi_to_packed_default(
	const uint8_t *y_src, const uint8_t *u_src, const uint8_t *v_src, uint8_t *y_dst,
	uint8_t *u_dst, uint8_t *v_dst, uint32_t pitch, uint32_t width, uint32_t height);
#endif
#if (AIPL_CONVERT_YUY2 & (TO_YV12 | TO_I420) | AIPL_CONVERT_UYVY & (TO_YV12 | TO_I420)) &&         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_packed_to_planar_default(
	const uint8_t *y_src, const uint8_t *u_src, const uint8_t *v_src, uint8_t *y_dst,
	uint8_t *u_dst, uint8_t *v_dst, uint32_t pitch, uint32_t width, uint32_t height);
#endif
#if (AIPL_CONVERT_YUY2 & (TO_NV12 | TO_NV21) | AIPL_CONVERT_UYVY & (TO_NV12 | TO_NV21)) &&         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_packed_to_semi_default(
	const uint8_t *y_src, const uint8_t *u_src, const uint8_t *v_src, uint8_t *y_dst,
	uint8_t *u_dst, uint8_t *v_dst, uint32_t pitch, uint32_t width, uint32_t height);
#endif
#if (AIPL_CONVERT_YUY2 & TO_UYVY | AIPL_CONVERT_UYVY & TO_YUY2)
aipl_error_t aipl_color_convert_yuv_packed_to_packed_default(
	const uint8_t *y_src, const uint8_t *u_src, const uint8_t *v_src, uint8_t *y_dst,
	uint8_t *u_dst, uint8_t *v_dst, uint32_t pitch, uint32_t width, uint32_t height);
#endif
#if (AIPL_CONVERT_ALPHA8_I400 & (TO_YV12 | TO_I420))
aipl_error_t aipl_color_convert_alpha8_to_yuv_planar_default(const uint8_t *input, uint8_t *output,
							     uint32_t pitch, uint32_t width,
							     uint32_t height);
#endif
#if (AIPL_CONVERT_ALPHA8_I400 & (TO_NV12 | TO_NV21))
aipl_error_t aipl_color_convert_alpha8_to_yuv_semi_planar_default(const uint8_t *input,
								  uint8_t *output, uint32_t pitch,
								  uint32_t width, uint32_t height);
#endif
#if (AIPL_CONVERT_ALPHA8_I400 & (TO_YUY2 | TO_UYVY)) &&                                            \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_alpha8_to_yuv_packed_default(const uint8_t *input, uint8_t *y_dst,
							     uint8_t *u_dst, uint8_t *v_dst,
							     uint32_t pitch, uint32_t width,
							     uint32_t height);
#endif

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
aipl_error_t aipl_color_convert_default(const void *input, void *output, uint32_t pitch,
					uint32_t width, uint32_t height,
					aipl_color_format_t input_format,
					aipl_color_format_t output_format)
{

	switch (input_format) {
		/* Alpha color formats */
#if AIPL_CONVERT_ALPHA8_I400
	case AIPL_COLOR_ALPHA8:
		return aipl_color_convert_alpha8_default(input, output, pitch, width, height,
							 output_format);
#endif

		/* RGB color formats */
#if AIPL_CONVERT_ARGB8888
	case AIPL_COLOR_ARGB8888:
		return aipl_color_convert_argb8888_default(input, output, pitch, width, height,
							   output_format);
#endif
#if AIPL_CONVERT_RGBA8888
	case AIPL_COLOR_RGBA8888:
		return aipl_color_convert_rgba8888_default(input, output, pitch, width, height,
							   output_format);
#endif
#if AIPL_CONVERT_ARGB4444
	case AIPL_COLOR_ARGB4444:
		return aipl_color_convert_argb4444_default(input, output, pitch, width, height,
							   output_format);
#endif
#if AIPL_CONVERT_ARGB1555
	case AIPL_COLOR_ARGB1555:
		return aipl_color_convert_argb1555_default(input, output, pitch, width, height,
							   output_format);
#endif
#if AIPL_CONVERT_RGBA4444
	case AIPL_COLOR_RGBA4444:
		return aipl_color_convert_rgba4444_default(input, output, pitch, width, height,
							   output_format);
#endif
#if AIPL_CONVERT_RGBA5551
	case AIPL_COLOR_RGBA5551:
		return aipl_color_convert_rgba5551_default(input, output, pitch, width, height,
							   output_format);
#endif
#if AIPL_CONVERT_RGB565
	case AIPL_COLOR_RGB565:
		return aipl_color_convert_rgb565_default(input, output, pitch, width, height,
							 output_format);
#endif
#if AIPL_CONVERT_BGR888
	case AIPL_COLOR_BGR888:
		return aipl_color_convert_bgr888_default(input, output, pitch, width, height,
							 output_format);
#endif
#if AIPL_CONVERT_RGB888
	case AIPL_COLOR_RGB888:
		return aipl_color_convert_rgb888_default(input, output, pitch, width, height,
							 output_format);
#endif

		/* YUV color formats */
#if AIPL_CONVERT_YV12
	case AIPL_COLOR_YV12:
		return aipl_color_convert_yv12_default(input, output, pitch, width, height,
						       output_format);
#endif
#if AIPL_CONVERT_I420
	case AIPL_COLOR_I420:
		return aipl_color_convert_i420_default(input, output, pitch, width, height,
						       output_format);
#endif
#if AIPL_CONVERT_NV12
	case AIPL_COLOR_NV12:
		return aipl_color_convert_nv12_default(input, output, pitch, width, height,
						       output_format);
#endif
#if AIPL_CONVERT_NV21
	case AIPL_COLOR_NV21:
		return aipl_color_convert_nv21_default(input, output, pitch, width, height,
						       output_format);
#endif
#if AIPL_CONVERT_I422
	case AIPL_COLOR_I422:
		return aipl_color_convert_i422_default(input, output, pitch, width, height,
						       output_format);
#endif
#if AIPL_CONVERT_YUY2
	case AIPL_COLOR_YUY2:
		return aipl_color_convert_yuy2_default(input, output, pitch, width, height,
						       output_format);
#endif
#if AIPL_CONVERT_UYVY
	case AIPL_COLOR_UYVY:
		return aipl_color_convert_uyvy_default(input, output, pitch, width, height,
						       output_format);
#endif
#if AIPL_CONVERT_I444
	case AIPL_COLOR_I444:
		return aipl_color_convert_i444_default(input, output, pitch, width, height,
						       output_format);
#endif
#if AIPL_CONVERT_ALPHA8_I400
	case AIPL_COLOR_I400:
		return aipl_color_convert_i400_default(input, output, pitch, width, height,
						       output_format);
#endif

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

aipl_error_t aipl_color_convert_img_default(const aipl_image_t *input, aipl_image_t *output)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	if (input->width != output->width || input->height != output->height) {
		return AIPL_ERR_SIZE_MISMATCH;
	}

	return aipl_color_convert_default(input->data, output->data, input->pitch, input->width,
					  input->height, input->format, output->format);
}

#if AIPL_CONVERT_ALPHA8_I400
aipl_error_t aipl_color_convert_alpha8_default(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height,
					       aipl_color_format_t format)
{
	switch (format) {
	case AIPL_COLOR_ALPHA8:
		return AIPL_ERR_FORMAT_MISMATCH;

	case AIPL_COLOR_I400:
		return aipl_color_convert_alpha8_to_i400_default(input, output, pitch, width,
								 height);

	default:
		return aipl_color_convert_i400_default(input, output, pitch, width, height, format);
	}
}

aipl_error_t aipl_color_convert_alpha8_to_i400_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	return aipl_color_convert_i400_to_alpha8_default(input, output, pitch, width, height);
}

#if (AIPL_CONVERT_ALPHA8_I400 & TO_ARGB8888) &&                                                    \
	((!defined(AIPL_DAVE2D_ACCELERATION) && !defined(AIPL_HELIUM_ACCELERATION)) ||             \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_alpha8_to_argb8888_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	return aipl_color_convert_i400_to_argb8888_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_ARGB4444) &&                                                    \
	((!(defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD)) &&               \
	  !defined(AIPL_HELIUM_ACCELERATION)) ||                                                   \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_alpha8_to_argb4444_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	return aipl_color_convert_i400_to_argb4444_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_ARGB1555) &&                                                    \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_alpha8_to_argb1555_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	return aipl_color_convert_i400_to_argb1555_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_RGBA8888) &&                                                    \
	((!defined(AIPL_DAVE2D_ACCELERATION) && !defined(AIPL_HELIUM_ACCELERATION)) ||             \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_alpha8_to_rgba8888_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	return aipl_color_convert_i400_to_rgba8888_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_RGBA4444) &&                                                    \
	((!(defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD)) &&               \
	  !defined(AIPL_HELIUM_ACCELERATION)) ||                                                   \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_alpha8_to_rgba4444_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	return aipl_color_convert_i400_to_rgba4444_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_RGBA5551) &&                                                    \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_alpha8_to_rgba5551_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	return aipl_color_convert_i400_to_rgba5551_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_BGR888) &&                                                      \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_alpha8_to_bgr888_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	return aipl_color_convert_i400_to_bgr888_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_RGB888) &&                                                      \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_alpha8_to_rgb888_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	return aipl_color_convert_i400_to_rgb888_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_RGB565) &&                                                      \
	((!(defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD)) &&               \
	  !defined(AIPL_HELIUM_ACCELERATION)) ||                                                   \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_alpha8_to_rgb565_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	return aipl_color_convert_i400_to_rgb565_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_YV12)
aipl_error_t aipl_color_convert_alpha8_to_yv12_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	return aipl_color_convert_i400_to_yv12_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_I420)
aipl_error_t aipl_color_convert_alpha8_to_i420_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	return aipl_color_convert_i400_to_i420_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_I422)
aipl_error_t aipl_color_convert_alpha8_to_i422_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	return aipl_color_convert_i400_to_i422_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_I444)
aipl_error_t aipl_color_convert_alpha8_to_i444_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	return aipl_color_convert_i400_to_i444_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_NV12)
aipl_error_t aipl_color_convert_alpha8_to_nv12_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	return aipl_color_convert_i400_to_nv12_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_NV21)
aipl_error_t aipl_color_convert_alpha8_to_nv21_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	return aipl_color_convert_i400_to_nv21_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_YUY2) &&                                                        \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_alpha8_to_yuy2_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	return aipl_color_convert_i400_to_yuy2_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_UYVY) &&                                                        \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_alpha8_to_uyvy_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	return aipl_color_convert_i400_to_uyvy_default(input, output, pitch, width, height);
}
#endif
#endif

#if AIPL_CONVERT_ARGB8888
aipl_error_t aipl_color_convert_argb8888_default(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height,
						 aipl_color_format_t format)
{
	switch (format) {
		/* Alpha color formats */
#if (AIPL_CONVERT_ARGB8888 & TO_ALPHA8_I400) &&                                                    \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ALPHA8:
		return aipl_color_convert_argb8888_to_alpha8_default(input, output, pitch, width,
								     height);
#endif
	/* RGB color formats */
	case AIPL_COLOR_ARGB8888:
		return AIPL_ERR_FORMAT_MISMATCH;
#if (AIPL_CONVERT_ARGB8888 & TO_RGBA8888) &&                                                       \
	((!defined(AIPL_DAVE2D_ACCELERATION) && !defined(AIPL_HELIUM_ACCELERATION)) ||             \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA8888:
		return aipl_color_convert_argb8888_to_rgba8888_default(input, output, pitch, width,
								       height);
#endif
#if (AIPL_CONVERT_ARGB8888 & TO_ARGB4444) && !defined(AIPL_HELIUM_ACCELERATION) &&                 \
	(!(defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD)) ||                \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB4444:
		return aipl_color_convert_argb8888_to_argb4444_default(input, output, pitch, width,
								       height);
#endif
#if (AIPL_CONVERT_ARGB8888 & TO_ARGB1555) && !defined(AIPL_HELIUM_ACCELERATION)
	case AIPL_COLOR_ARGB1555:
		return aipl_color_convert_argb8888_to_argb1555_default(input, output, pitch, width,
								       height);
#endif
#if (AIPL_CONVERT_ARGB8888 & TO_RGBA4444) && !defined(AIPL_HELIUM_ACCELERATION) &&                 \
	(!(defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD)) ||                \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA4444:
		return aipl_color_convert_argb8888_to_rgba4444_default(input, output, pitch, width,
								       height);
#endif
#if (AIPL_CONVERT_ARGB8888 & TO_RGBA5551) && !defined(AIPL_HELIUM_ACCELERATION)
	case AIPL_COLOR_RGBA5551:
		return aipl_color_convert_argb8888_to_rgba5551_default(input, output, pitch, width,
								       height);
#endif
#if (AIPL_CONVERT_ARGB8888 & TO_RGB565) && !defined(AIPL_HELIUM_ACCELERATION) &&                   \
	(!defined(AIPL_DAVE2D_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGB565:
		return aipl_color_convert_argb8888_to_rgb565_default(input, output, pitch, width,
								     height);
#endif
#if (AIPL_CONVERT_ARGB8888 & TO_BGR888) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_BGR888:
		return aipl_color_convert_argb8888_to_bgr888_default(input, output, pitch, width,
								     height);
#endif
#if (AIPL_CONVERT_ARGB8888 & TO_RGB888) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGB888:
		return aipl_color_convert_argb8888_to_rgb888_default(input, output, pitch, width,
								     height);
#endif /* YUV color formats */
#if (AIPL_CONVERT_ARGB8888 & TO_YV12) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_YV12:
		return aipl_color_convert_argb8888_to_yv12_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_ARGB8888 & TO_I420) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I420:
		return aipl_color_convert_argb8888_to_i420_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_ARGB8888 & TO_NV12) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_NV12:
		return aipl_color_convert_argb8888_to_nv12_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_ARGB8888 & TO_NV21) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_NV21:
		return aipl_color_convert_argb8888_to_nv21_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_ARGB8888 & TO_I422) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I422:
		return aipl_color_convert_argb8888_to_i422_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_ARGB8888 & TO_YUY2) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_YUY2:
		return aipl_color_convert_argb8888_to_yuy2_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_ARGB8888 & TO_UYVY) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_UYVY:
		return aipl_color_convert_argb8888_to_uyvy_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_ARGB8888 & TO_I444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I444:
		return aipl_color_convert_argb8888_to_i444_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_ARGB8888 & TO_ALPHA8_I400) &&                                                    \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I400:
		return aipl_color_convert_argb8888_to_i400_default(input, output, pitch, width,
								   height);
#endif

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

#if (AIPL_CONVERT_ARGB8888 & TO_ALPHA8_I400) &&                                                    \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb8888_to_alpha8_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_argb8888_px_t *src_ptr = input;
	uint8_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_argb8888_px_t *src = src_ptr + i * pitch;
		uint8_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_argb8888_to_yuv_y(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ARGB8888 & TO_ARGB4444) && !defined(AIPL_HELIUM_ACCELERATION) &&                 \
	(!(defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD)) ||                \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb8888_to_argb4444_default(const void *input, void *output,
							     uint32_t pitch, uint32_t width,
							     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_argb8888_px_t *src_ptr = input;
	aipl_argb4444_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_argb8888_px_t *src = src_ptr + i * pitch;
		aipl_argb4444_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_argb8888_to_argb4444(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ARGB8888 & TO_ARGB1555) && !defined(AIPL_HELIUM_ACCELERATION)
aipl_error_t aipl_color_convert_argb8888_to_argb1555_default(const void *input, void *output,
							     uint32_t pitch, uint32_t width,
							     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_argb8888_px_t *src_ptr = input;
	aipl_argb1555_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_argb8888_px_t *src = src_ptr + i * pitch;
		aipl_argb1555_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_argb8888_to_argb1555(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ARGB8888 & TO_RGBA8888) &&                                                       \
	((!defined(AIPL_DAVE2D_ACCELERATION) && !defined(AIPL_HELIUM_ACCELERATION)) ||             \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb8888_to_rgba8888_default(const void *input, void *output,
							     uint32_t pitch, uint32_t width,
							     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_argb8888_px_t *src_ptr = input;
	aipl_rgba8888_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_argb8888_px_t *src = src_ptr + i * pitch;
		aipl_rgba8888_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_argb8888_to_rgba8888(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ARGB8888 & TO_RGBA4444) && !defined(AIPL_HELIUM_ACCELERATION) &&                 \
	(!(defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD)) ||                \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb8888_to_rgba4444_default(const void *input, void *output,
							     uint32_t pitch, uint32_t width,
							     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_argb8888_px_t *src_ptr = input;
	aipl_rgba4444_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_argb8888_px_t *src = src_ptr + i * pitch;
		aipl_rgba4444_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_argb8888_to_rgba4444(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ARGB8888 & TO_RGBA5551) && !defined(AIPL_HELIUM_ACCELERATION)
aipl_error_t aipl_color_convert_argb8888_to_rgba5551_default(const void *input, void *output,
							     uint32_t pitch, uint32_t width,
							     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_argb8888_px_t *src_ptr = input;
	aipl_rgba5551_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_argb8888_px_t *src = src_ptr + i * pitch;
		aipl_rgba5551_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_argb8888_to_rgba5551(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ARGB8888 & TO_BGR888) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb8888_to_bgr888_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	return aipl_color_convert_argb8888_to_24bit_default(input, output, pitch, width, height, 2,
							    1, 0);
}
#endif

#if (AIPL_CONVERT_ARGB8888 & TO_RGB888) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb8888_to_rgb888_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	return aipl_color_convert_argb8888_to_24bit_default(input, output, pitch, width, height, 0,
							    1, 2);
}
#endif

#if (AIPL_CONVERT_ARGB8888 & TO_RGB565) && !defined(AIPL_HELIUM_ACCELERATION) &&                   \
	(!defined(AIPL_DAVE2D_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb8888_to_rgb565_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_argb8888_px_t *src_ptr = input;
	aipl_rgb565_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_argb8888_px_t *src = src_ptr + i * pitch;
		aipl_rgb565_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_argb8888_to_rgb565(dst, src);

			++dst;
			++src;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ARGB8888 & TO_YV12) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb8888_to_yv12_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *v_ptr = output + yuv_size;
	uint8_t *u_ptr = v_ptr + yuv_size / 4;

	return aipl_color_convert_argb8888_to_yuv_planar_default(input, pitch, width, height, y_ptr,
								 u_ptr, v_ptr);
}
#endif

#if (AIPL_CONVERT_ARGB8888 & TO_I420) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb8888_to_i420_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + yuv_size;
	uint8_t *v_ptr = u_ptr + yuv_size / 4;

	return aipl_color_convert_argb8888_to_yuv_planar_default(input, pitch, width, height, y_ptr,
								 u_ptr, v_ptr);
}
#endif

#if (AIPL_CONVERT_ARGB8888 & TO_I422) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb8888_to_i422_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + yuv_size;
	uint8_t *v_ptr = u_ptr + yuv_size / 2;

	const aipl_argb8888_px_t *src_ptr = input;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_argb8888_px_t *src = src_ptr + i * pitch;

		uint8_t *y_dst = y_ptr + i * width;
		uint8_t *v_dst = v_ptr + i * width / 2;
		uint8_t *u_dst = u_ptr + i * width / 2;

		for (uint32_t j = 0; j < width; j += 2) {
			aipl_cnvt_px_argb8888_to_yuv(y_dst, u_dst, v_dst, src);

			++src;
			++y_dst;
			++u_dst;
			++v_dst;

			aipl_cnvt_px_argb8888_to_yuv_y(y_dst, src);

			++src;
			++y_dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ARGB8888 & TO_I444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb8888_to_i444_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + yuv_size;
	uint8_t *v_ptr = u_ptr + yuv_size;

	const aipl_argb8888_px_t *src_ptr = input;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_argb8888_px_t *src = src_ptr + i * pitch;

		uint8_t *y_dst = y_ptr + i * width;
		uint8_t *v_dst = v_ptr + i * width;
		uint8_t *u_dst = u_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_argb8888_to_yuv(y_dst, u_dst, v_dst, src);

			++src;
			++y_dst;
			++u_dst;
			++v_dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ARGB8888 & TO_ALPHA8_I400) &&                                                    \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb8888_to_i400_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	return aipl_color_convert_argb8888_to_alpha8_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_ARGB8888 & TO_NV21) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb8888_to_nv21_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *v_ptr = output + yuv_size;
	uint8_t *u_ptr = v_ptr + 1;

	return aipl_color_convert_argb8888_to_yuv_semi_planar_default(input, pitch, width, height,
								      y_ptr, u_ptr, v_ptr);
}
#endif

#if (AIPL_CONVERT_ARGB8888 & TO_NV12) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb8888_to_nv12_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + yuv_size;
	uint8_t *v_ptr = u_ptr + 1;

	return aipl_color_convert_argb8888_to_yuv_semi_planar_default(input, pitch, width, height,
								      y_ptr, u_ptr, v_ptr);
}
#endif

#if (AIPL_CONVERT_ARGB8888 & TO_YUY2) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb8888_to_yuy2_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + 1;
	uint8_t *v_ptr = u_ptr + 2;

	return aipl_color_convert_argb8888_to_yuv_packed_default(input, pitch, width, height, y_ptr,
								 u_ptr, v_ptr);
}
#endif

#if (AIPL_CONVERT_ARGB8888 & TO_UYVY) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb8888_to_uyvy_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint8_t *y_ptr = output + 1;
	uint8_t *u_ptr = output;
	uint8_t *v_ptr = u_ptr + 2;

	return aipl_color_convert_argb8888_to_yuv_packed_default(input, pitch, width, height, y_ptr,
								 u_ptr, v_ptr);
}
#endif
#endif

#if AIPL_CONVERT_ARGB4444
aipl_error_t aipl_color_convert_argb4444_default(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height,
						 aipl_color_format_t format)
{
	switch (format) {
		/* Alpha color formats */
#if (AIPL_CONVERT_ARGB4444 & TO_ALPHA8_I400) &&                                                    \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ALPHA8:
		return aipl_color_convert_argb4444_to_alpha8_default(input, output, pitch, width,
								     height);
#endif
		/* RGB color formats */
#if (AIPL_CONVERT_ARGB4444 & TO_ARGB8888) && !defined(AIPL_HELIUM_ACCELERATION) &&                 \
	(!defined(AIPL_DAVE2D_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB8888:
		return aipl_color_convert_argb4444_to_argb8888_default(input, output, pitch, width,
								       height);
#endif
#if (AIPL_CONVERT_ARGB4444 & TO_RGBA8888) && !defined(AIPL_HELIUM_ACCELERATION) &&                 \
	(!defined(AIPL_DAVE2D_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA8888:
		return aipl_color_convert_argb4444_to_rgba8888_default(input, output, pitch, width,
								       height);
#endif
	case AIPL_COLOR_ARGB4444:
		return AIPL_ERR_FORMAT_MISMATCH;
#if (AIPL_CONVERT_ARGB4444 & TO_ARGB1555)
	case AIPL_COLOR_ARGB1555:
		return aipl_color_convert_argb4444_to_argb1555_default(input, output, pitch, width,
								       height);
#endif
#if (AIPL_CONVERT_ARGB4444 & TO_RGBA4444) &&                                                       \
	((!(defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD)) &&               \
	  !defined(AIPL_HELIUM_ACCELERATION)) ||                                                   \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA4444:
		return aipl_color_convert_argb4444_to_rgba4444_default(input, output, pitch, width,
								       height);
#endif
#if (AIPL_CONVERT_ARGB4444 & TO_RGBA5551)
	case AIPL_COLOR_RGBA5551:
		return aipl_color_convert_argb4444_to_rgba5551_default(input, output, pitch, width,
								       height);
#endif
#if (AIPL_CONVERT_ARGB4444 & TO_RGB565) &&                                                         \
	(!(defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD)) ||                \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGB565:
		return aipl_color_convert_argb4444_to_rgb565_default(input, output, pitch, width,
								     height);
#endif
#if (AIPL_CONVERT_ARGB4444 & TO_BGR888) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_BGR888:
		return aipl_color_convert_argb4444_to_bgr888_default(input, output, pitch, width,
								     height);
#endif
#if (AIPL_CONVERT_ARGB4444 & TO_RGB888) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGB888:
		return aipl_color_convert_argb4444_to_rgb888_default(input, output, pitch, width,
								     height);
#endif
		/* YUV color formats */
#if (AIPL_CONVERT_ARGB4444 & TO_YV12) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_YV12:
		return aipl_color_convert_argb4444_to_yv12_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_ARGB4444 & TO_I420) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I420:
		return aipl_color_convert_argb4444_to_i420_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_ARGB4444 & TO_NV12) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_NV12:
		return aipl_color_convert_argb4444_to_nv12_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_ARGB4444 & TO_NV21) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_NV21:
		return aipl_color_convert_argb4444_to_nv21_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_ARGB4444 & TO_I422) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I422:
		return aipl_color_convert_argb4444_to_i422_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_ARGB4444 & TO_YUY2) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_YUY2:
		return aipl_color_convert_argb4444_to_yuy2_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_ARGB4444 & TO_UYVY) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_UYVY:
		return aipl_color_convert_argb4444_to_uyvy_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_ARGB4444 & TO_I444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I444:
		return aipl_color_convert_argb4444_to_i444_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_ARGB4444 & TO_ALPHA8_I400) &&                                                    \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I400:
		return aipl_color_convert_argb4444_to_i400_default(input, output, pitch, width,
								   height);
#endif

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

#if (AIPL_CONVERT_ARGB4444 & TO_ALPHA8_I400) &&                                                    \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb4444_to_alpha8_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_argb4444_px_t *src_ptr = input;
	uint8_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_argb4444_px_t *src = src_ptr + i * pitch;
		uint8_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_argb4444_to_yuv_y(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ARGB4444 & TO_ARGB8888) && !defined(AIPL_HELIUM_ACCELERATION) &&                 \
	(!defined(AIPL_DAVE2D_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb4444_to_argb8888_default(const void *input, void *output,
							     uint32_t pitch, uint32_t width,
							     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_argb4444_px_t *src_ptr = input;
	aipl_argb8888_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_argb4444_px_t *src = src_ptr + i * pitch;
		aipl_argb8888_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_argb4444_to_argb8888(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ARGB4444 & TO_ARGB1555)
aipl_error_t aipl_color_convert_argb4444_to_argb1555_default(const void *input, void *output,
							     uint32_t pitch, uint32_t width,
							     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_argb4444_px_t *src_ptr = input;
	aipl_argb1555_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_argb4444_px_t *src = src_ptr + i * pitch;
		aipl_argb1555_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_argb4444_to_argb1555(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ARGB4444 & TO_RGBA8888) && !defined(AIPL_HELIUM_ACCELERATION) &&                 \
	(!defined(AIPL_DAVE2D_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb4444_to_rgba8888_default(const void *input, void *output,
							     uint32_t pitch, uint32_t width,
							     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_argb4444_px_t *src_ptr = input;
	aipl_rgba8888_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_argb4444_px_t *src = src_ptr + i * pitch;
		aipl_rgba8888_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_argb4444_to_rgba8888(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ARGB4444 & TO_RGBA4444) &&                                                       \
	((!(defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD)) &&               \
	  !defined(AIPL_HELIUM_ACCELERATION)) ||                                                   \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb4444_to_rgba4444_default(const void *input, void *output,
							     uint32_t pitch, uint32_t width,
							     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_argb4444_px_t *src_ptr = input;
	aipl_rgba4444_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_argb4444_px_t *src = src_ptr + i * pitch;
		aipl_rgba4444_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_argb4444_to_rgba4444(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ARGB4444 & TO_RGBA5551)
aipl_error_t aipl_color_convert_argb4444_to_rgba5551_default(const void *input, void *output,
							     uint32_t pitch, uint32_t width,
							     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_argb4444_px_t *src_ptr = input;
	aipl_rgba5551_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_argb4444_px_t *src = src_ptr + i * pitch;
		aipl_rgba5551_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_argb4444_to_rgba5551(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ARGB4444 & TO_BGR888) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb4444_to_bgr888_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	return aipl_color_convert_argb4444_to_24bit_default(input, output, pitch, width, height, 2,
							    1, 0);
}
#endif

#if (AIPL_CONVERT_ARGB4444 & TO_RGB888) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb4444_to_rgb888_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	return aipl_color_convert_argb4444_to_24bit_default(input, output, pitch, width, height, 0,
							    1, 2);
}
#endif

#if (AIPL_CONVERT_ARGB4444 & TO_RGB565) &&                                                         \
	(!(defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD)) ||                \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb4444_to_rgb565_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_argb4444_px_t *src_ptr = input;
	aipl_rgb565_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_argb4444_px_t *src = src_ptr + i * pitch;
		aipl_rgb565_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_argb4444_to_rgb565(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ARGB4444 & TO_YV12) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb4444_to_yv12_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *v_ptr = output + yuv_size;
	uint8_t *u_ptr = v_ptr + yuv_size / 4;

	return aipl_color_convert_argb4444_to_yuv_planar_default(input, pitch, width, height, y_ptr,
								 u_ptr, v_ptr);
}
#endif

#if (AIPL_CONVERT_ARGB4444 & TO_I420) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb4444_to_i420_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + yuv_size;
	uint8_t *v_ptr = u_ptr + yuv_size / 4;

	return aipl_color_convert_argb4444_to_yuv_planar_default(input, pitch, width, height, y_ptr,
								 u_ptr, v_ptr);
}
#endif

#if (AIPL_CONVERT_ARGB4444 & TO_I422) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb4444_to_i422_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + yuv_size;
	uint8_t *v_ptr = u_ptr + yuv_size / 2;

	const aipl_argb4444_px_t *src_ptr = input;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_argb4444_px_t *src = src_ptr + i * pitch;
		uint8_t *y_dst = y_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_argb4444_to_yuv_y(y_dst, src);

			++src;
			++y_dst;
		}

		src = src_ptr + i * pitch;
		uint8_t *u_dst = u_ptr + i * width / 2;
		uint8_t *v_dst = v_ptr + i * width / 2;

		for (uint32_t j = 0; j < width / 2; ++j) {
			aipl_cnvt_px_argb4444_to_yuv_uv(u_dst, v_dst, src);

			src += 2;
			++u_dst;
			++v_dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ARGB4444 & TO_I444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb4444_to_i444_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + yuv_size;
	uint8_t *v_ptr = u_ptr + yuv_size;

	const aipl_argb4444_px_t *src_ptr = input;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_argb4444_px_t *src = src_ptr + i * pitch;

		uint8_t *y_dst = y_ptr + i * width;
		uint8_t *v_dst = v_ptr + i * width;
		uint8_t *u_dst = u_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_argb4444_to_yuv(y_dst, u_dst, v_dst, src);

			++src;
			++y_dst;
			++u_dst;
			++v_dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ARGB4444 & TO_ALPHA8_I400) &&                                                    \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb4444_to_i400_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	return aipl_color_convert_argb4444_to_alpha8_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_ARGB4444 & TO_NV21) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb4444_to_nv21_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *v_ptr = output + yuv_size;
	uint8_t *u_ptr = v_ptr + 1;

	return aipl_color_convert_argb4444_to_yuv_semi_planar_default(input, pitch, width, height,
								      y_ptr, u_ptr, v_ptr);
}
#endif

#if (AIPL_CONVERT_ARGB4444 & TO_NV12) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb4444_to_nv12_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + yuv_size;
	uint8_t *v_ptr = u_ptr + 1;

	return aipl_color_convert_argb4444_to_yuv_semi_planar_default(input, pitch, width, height,
								      y_ptr, u_ptr, v_ptr);
}
#endif

#if (AIPL_CONVERT_ARGB4444 & TO_YUY2) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb4444_to_yuy2_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + 1;
	uint8_t *v_ptr = u_ptr + 2;

	return aipl_color_convert_argb4444_to_yuv_packed_default(input, pitch, width, height, y_ptr,
								 u_ptr, v_ptr);
}
#endif

#if (AIPL_CONVERT_ARGB4444 & TO_UYVY) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb4444_to_uyvy_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint8_t *y_ptr = output + 1;
	uint8_t *u_ptr = output;
	uint8_t *v_ptr = u_ptr + 2;

	return aipl_color_convert_argb4444_to_yuv_packed_default(input, pitch, width, height, y_ptr,
								 u_ptr, v_ptr);
}
#endif
#endif

#if AIPL_CONVERT_ARGB1555
aipl_error_t aipl_color_convert_argb1555_default(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height,
						 aipl_color_format_t format)
{
	switch (format) {
		/* Alpha color formats */
#if (AIPL_CONVERT_ARGB1555 & TO_ALPHA8_I400) &&                                                    \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ALPHA8:
		return aipl_color_convert_argb1555_to_alpha8_default(input, output, pitch, width,
								     height);
#endif
		/* RGB color formats */
#if (AIPL_CONVERT_ARGB1555 & TO_ARGB8888) && !defined(AIPL_HELIUM_ACCELERATION) &&                 \
	(!defined(AIPL_DAVE2D_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB8888:
		return aipl_color_convert_argb1555_to_argb8888_default(input, output, pitch, width,
								       height);
#endif
#if (AIPL_CONVERT_ARGB1555 & TO_RGBA8888) && !defined(AIPL_HELIUM_ACCELERATION) &&                 \
	(!defined(AIPL_DAVE2D_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA8888:
		return aipl_color_convert_argb1555_to_rgba8888_default(input, output, pitch, width,
								       height);
#endif
#if (AIPL_CONVERT_ARGB1555 & TO_ARGB4444) &&                                                       \
	(!(defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD)) ||                \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB4444:
		return aipl_color_convert_argb1555_to_argb4444_default(input, output, pitch, width,
								       height);
#endif
	case AIPL_COLOR_ARGB1555:
		return AIPL_ERR_FORMAT_MISMATCH;
#if (AIPL_CONVERT_ARGB1555 & TO_RGBA4444) &&                                                       \
	(!(defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD)) ||                \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA4444:
		return aipl_color_convert_argb1555_to_rgba4444_default(input, output, pitch, width,
								       height);
#endif
#if (AIPL_CONVERT_ARGB1555 & TO_RGBA5551)
	case AIPL_COLOR_RGBA5551:
		return aipl_color_convert_argb1555_to_rgba5551_default(input, output, pitch, width,
								       height);
#endif
#if (AIPL_CONVERT_ARGB1555 & TO_RGB565) &&                                                         \
	(!(defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD)) ||                \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGB565:
		return aipl_color_convert_argb1555_to_rgb565_default(input, output, pitch, width,
								     height);
#endif
#if (AIPL_CONVERT_ARGB1555 & TO_BGR888) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_BGR888:
		return aipl_color_convert_argb1555_to_bgr888_default(input, output, pitch, width,
								     height);
#endif
#if (AIPL_CONVERT_ARGB1555 & TO_RGB888) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGB888:
		return aipl_color_convert_argb1555_to_rgb888_default(input, output, pitch, width,
								     height);
#endif
		/* YUV color formats */
#if (AIPL_CONVERT_ARGB1555 & TO_YV12) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_YV12:
		return aipl_color_convert_argb1555_to_yv12_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_ARGB1555 & TO_I420) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I420:
		return aipl_color_convert_argb1555_to_i420_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_ARGB1555 & TO_NV12) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_NV12:
		return aipl_color_convert_argb1555_to_nv12_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_ARGB1555 & TO_NV21) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_NV21:
		return aipl_color_convert_argb1555_to_nv21_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_ARGB1555 & TO_I422) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I422:
		return aipl_color_convert_argb1555_to_i422_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_ARGB1555 & TO_YUY2) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_YUY2:
		return aipl_color_convert_argb1555_to_yuy2_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_ARGB1555 & TO_UYVY) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_UYVY:
		return aipl_color_convert_argb1555_to_uyvy_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_ARGB1555 & TO_I444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I444:
		return aipl_color_convert_argb1555_to_i444_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_ARGB1555 & TO_ALPHA8_I400) &&                                                    \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I400:
		return aipl_color_convert_argb1555_to_i400_default(input, output, pitch, width,
								   height);
#endif

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

#if (AIPL_CONVERT_ARGB1555 & TO_ALPHA8_I400) &&                                                    \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb1555_to_alpha8_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_argb1555_px_t *src_ptr = input;
	uint8_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_argb1555_px_t *src = src_ptr + i * pitch;
		uint8_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_argb1555_to_yuv_y(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ARGB1555 & TO_ARGB8888) && !defined(AIPL_HELIUM_ACCELERATION) &&                 \
	(!defined(AIPL_DAVE2D_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb1555_to_argb8888_default(const void *input, void *output,
							     uint32_t pitch, uint32_t width,
							     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_argb1555_px_t *src_ptr = input;
	aipl_argb8888_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_argb1555_px_t *src = src_ptr + i * pitch;
		aipl_argb8888_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_argb1555_to_argb8888(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ARGB1555 & TO_ARGB4444) &&                                                       \
	(!(defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD)) ||                \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb1555_to_argb4444_default(const void *input, void *output,
							     uint32_t pitch, uint32_t width,
							     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_argb1555_px_t *src_ptr = input;
	aipl_argb4444_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_argb1555_px_t *src = src_ptr + i * pitch;
		aipl_argb4444_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_argb1555_to_argb4444(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ARGB1555 & TO_RGBA8888) && !defined(AIPL_HELIUM_ACCELERATION) &&                 \
	(!defined(AIPL_DAVE2D_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb1555_to_rgba8888_default(const void *input, void *output,
							     uint32_t pitch, uint32_t width,
							     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_argb1555_px_t *src_ptr = input;
	aipl_rgba8888_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_argb1555_px_t *src = src_ptr + i * pitch;
		aipl_rgba8888_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_argb1555_to_rgba8888(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ARGB1555 & TO_RGBA4444) &&                                                       \
	(!(defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD)) ||                \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb1555_to_rgba4444_default(const void *input, void *output,
							     uint32_t pitch, uint32_t width,
							     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_argb1555_px_t *src_ptr = input;
	aipl_rgba4444_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_argb1555_px_t *src = src_ptr + i * pitch;
		aipl_rgba4444_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_argb1555_to_rgba4444(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ARGB1555 & TO_RGBA5551)
aipl_error_t aipl_color_convert_argb1555_to_rgba5551_default(const void *input, void *output,
							     uint32_t pitch, uint32_t width,
							     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_argb1555_px_t *src_ptr = input;
	aipl_rgba5551_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_argb1555_px_t *src = src_ptr + i * pitch;
		aipl_rgba5551_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_argb1555_to_rgba5551(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ARGB1555 & TO_BGR888) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb1555_to_bgr888_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	return aipl_color_convert_argb1555_to_24bit_default(input, output, pitch, width, height, 2,
							    1, 0);
}
#endif

#if (AIPL_CONVERT_ARGB1555 & TO_RGB888) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb1555_to_rgb888_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	return aipl_color_convert_argb1555_to_24bit_default(input, output, pitch, width, height, 0,
							    1, 2);
}
#endif

#if (AIPL_CONVERT_ARGB1555 & TO_RGB565) &&                                                         \
	(!(defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD)) ||                \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb1555_to_rgb565_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_argb1555_px_t *src_ptr = input;
	aipl_rgb565_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_argb1555_px_t *src = src_ptr + i * pitch;
		aipl_rgb565_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_argb1555_to_rgb565(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ARGB1555 & TO_YV12) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb1555_to_yv12_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *v_ptr = output + yuv_size;
	uint8_t *u_ptr = v_ptr + yuv_size / 4;

	return aipl_color_convert_argb1555_to_yuv_planar_default(input, pitch, width, height, y_ptr,
								 u_ptr, v_ptr);
}
#endif

#if (AIPL_CONVERT_ARGB1555 & TO_I420) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb1555_to_i420_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + yuv_size;
	uint8_t *v_ptr = u_ptr + yuv_size / 4;

	return aipl_color_convert_argb1555_to_yuv_planar_default(input, pitch, width, height, y_ptr,
								 u_ptr, v_ptr);
}
#endif

#if (AIPL_CONVERT_ARGB1555 & TO_I422) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb1555_to_i422_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + yuv_size;
	uint8_t *v_ptr = u_ptr + yuv_size / 2;

	const aipl_argb1555_px_t *src_ptr = input;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_argb1555_px_t *src = src_ptr + i * pitch;
		uint8_t *y_dst = y_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_argb1555_to_yuv_y(y_dst, src);

			++src;
			++y_dst;
		}

		src = src_ptr + i * pitch;
		uint8_t *u_dst = u_ptr + i * width / 2;
		uint8_t *v_dst = v_ptr + i * width / 2;

		for (uint32_t j = 0; j < width / 2; ++j) {
			aipl_cnvt_px_argb1555_to_yuv_uv(u_dst, v_dst, src);

			src += 2;
			++u_dst;
			++v_dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ARGB1555 & TO_I444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb1555_to_i444_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + yuv_size;
	uint8_t *v_ptr = u_ptr + yuv_size;

	const aipl_argb1555_px_t *src_ptr = input;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_argb1555_px_t *src = src_ptr + i * pitch;

		uint8_t *y_dst = y_ptr + i * width;
		uint8_t *v_dst = v_ptr + i * width;
		uint8_t *u_dst = u_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_argb1555_to_yuv(y_dst, u_dst, v_dst, src);

			++src;
			++y_dst;
			++u_dst;
			++v_dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ARGB1555 & TO_ALPHA8_I400) &&                                                    \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb1555_to_i400_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	return aipl_color_convert_argb1555_to_alpha8_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_ARGB1555 & TO_NV21) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb1555_to_nv21_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *v_ptr = output + yuv_size;
	uint8_t *u_ptr = v_ptr + 1;

	return aipl_color_convert_argb1555_to_yuv_semi_planar_default(input, pitch, width, height,
								      y_ptr, u_ptr, v_ptr);
}
#endif

#if (AIPL_CONVERT_ARGB1555 & TO_NV12) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb1555_to_nv12_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + yuv_size;
	uint8_t *v_ptr = u_ptr + 1;

	return aipl_color_convert_argb1555_to_yuv_semi_planar_default(input, pitch, width, height,
								      y_ptr, u_ptr, v_ptr);
}
#endif

#if (AIPL_CONVERT_ARGB1555 & TO_YUY2) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb1555_to_yuy2_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + 1;
	uint8_t *v_ptr = u_ptr + 2;

	return aipl_color_convert_argb1555_to_yuv_packed_default(input, pitch, width, height, y_ptr,
								 u_ptr, v_ptr);
}
#endif

#if (AIPL_CONVERT_ARGB1555 & TO_UYVY) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb1555_to_uyvy_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint8_t *y_ptr = output + 1;
	uint8_t *u_ptr = output;
	uint8_t *v_ptr = u_ptr + 2;

	return aipl_color_convert_argb1555_to_yuv_packed_default(input, pitch, width, height, y_ptr,
								 u_ptr, v_ptr);
}
#endif
#endif

#if AIPL_CONVERT_RGBA8888
aipl_error_t aipl_color_convert_rgba8888_default(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height,
						 aipl_color_format_t format)
{
	switch (format) {
		/* Alpha color formats */
#if (AIPL_CONVERT_RGBA8888 & TO_ALPHA8_I400) &&                                                    \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ALPHA8:
		return aipl_color_convert_rgba8888_to_alpha8_default(input, output, pitch, width,
								     height);
#endif
		/* RGB color formats */
#if (AIPL_CONVERT_RGBA8888 & TO_ARGB8888) &&                                                       \
	((!defined(AIPL_DAVE2D_ACCELERATION) && !defined(AIPL_HELIUM_ACCELERATION)) ||             \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB8888:
		return aipl_color_convert_rgba8888_to_argb8888_default(input, output, pitch, width,
								       height);
#endif
	case AIPL_COLOR_RGBA8888:
		return AIPL_ERR_FORMAT_MISMATCH;
#if (AIPL_CONVERT_RGBA8888 & TO_ARGB4444) && !defined(AIPL_HELIUM_ACCELERATION) &&                 \
	(!(defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD)) ||                \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB4444:
		return aipl_color_convert_rgba8888_to_argb4444_default(input, output, pitch, width,
								       height);
#endif
#if (AIPL_CONVERT_RGBA8888 & TO_ARGB1555) && !defined(AIPL_HELIUM_ACCELERATION)
	case AIPL_COLOR_ARGB1555:
		return aipl_color_convert_rgba8888_to_argb1555_default(input, output, pitch, width,
								       height);
#endif
#if (AIPL_CONVERT_RGBA8888 & TO_RGBA4444) && !defined(AIPL_HELIUM_ACCELERATION) &&                 \
	(!(defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD)) ||                \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA4444:
		return aipl_color_convert_rgba8888_to_rgba4444_default(input, output, pitch, width,
								       height);
#endif
#if (AIPL_CONVERT_RGBA8888 & TO_RGBA5551) && !defined(AIPL_HELIUM_ACCELERATION)
	case AIPL_COLOR_RGBA5551:
		return aipl_color_convert_rgba8888_to_rgba5551_default(input, output, pitch, width,
								       height);
#endif
#if (AIPL_CONVERT_RGBA8888 & TO_RGB565) && !defined(AIPL_HELIUM_ACCELERATION) &&                   \
	(!defined(AIPL_DAVE2D_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGB565:
		return aipl_color_convert_rgba8888_to_rgb565_default(input, output, pitch, width,
								     height);
#endif
#if (AIPL_CONVERT_RGBA8888 & TO_BGR888) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_BGR888:
		return aipl_color_convert_rgba8888_to_bgr888_default(input, output, pitch, width,
								     height);
#endif
#if (AIPL_CONVERT_RGBA8888 & TO_RGB888) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGB888:
		return aipl_color_convert_rgba8888_to_rgb888_default(input, output, pitch, width,
								     height);
#endif
		/* YUV color formats */
#if (AIPL_CONVERT_RGBA8888 & TO_YV12) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_YV12:
		return aipl_color_convert_rgba8888_to_yv12_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_RGBA8888 & TO_I420) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I420:
		return aipl_color_convert_rgba8888_to_i420_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_RGBA8888 & TO_NV12) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_NV12:
		return aipl_color_convert_rgba8888_to_nv12_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_RGBA8888 & TO_NV21) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_NV21:
		return aipl_color_convert_rgba8888_to_nv21_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_RGBA8888 & TO_I422) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I422:
		return aipl_color_convert_rgba8888_to_i422_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_RGBA8888 & TO_YUY2) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_YUY2:
		return aipl_color_convert_rgba8888_to_yuy2_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_RGBA8888 & TO_UYVY) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_UYVY:
		return aipl_color_convert_rgba8888_to_uyvy_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_RGBA8888 & TO_I444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I444:
		return aipl_color_convert_rgba8888_to_i444_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_RGBA8888 & TO_ALPHA8_I400) &&                                                    \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I400:
		return aipl_color_convert_rgba8888_to_i400_default(input, output, pitch, width,
								   height);
#endif

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

#if (AIPL_CONVERT_RGBA8888 & TO_ALPHA8_I400) &&                                                    \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba8888_to_alpha8_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgba8888_px_t *src_ptr = input;
	uint8_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgba8888_px_t *src = src_ptr + i * pitch;
		uint8_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgba8888_to_yuv_y(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGBA8888 & TO_ARGB8888) &&                                                       \
	((!defined(AIPL_DAVE2D_ACCELERATION) && !defined(AIPL_HELIUM_ACCELERATION)) ||             \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba8888_to_argb8888_default(const void *input, void *output,
							     uint32_t pitch, uint32_t width,
							     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgba8888_px_t *src_ptr = input;
	aipl_argb8888_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgba8888_px_t *src = src_ptr + i * pitch;
		aipl_argb8888_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgba8888_to_argb8888(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGBA8888 & TO_ARGB4444) && !defined(AIPL_HELIUM_ACCELERATION) &&                 \
	(!(defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD)) ||                \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba8888_to_argb4444_default(const void *input, void *output,
							     uint32_t pitch, uint32_t width,
							     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgba8888_px_t *src_ptr = input;
	aipl_argb4444_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgba8888_px_t *src = src_ptr + i * pitch;
		aipl_argb4444_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgba8888_to_argb4444(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGBA8888 & TO_ARGB1555) && !defined(AIPL_HELIUM_ACCELERATION)
aipl_error_t aipl_color_convert_rgba8888_to_argb1555_default(const void *input, void *output,
							     uint32_t pitch, uint32_t width,
							     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgba8888_px_t *src_ptr = input;
	aipl_argb1555_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgba8888_px_t *src = src_ptr + i * pitch;
		aipl_argb1555_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgba8888_to_argb1555(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGBA8888 & TO_RGBA4444) && !defined(AIPL_HELIUM_ACCELERATION) &&                 \
	(!(defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD)) ||                \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba8888_to_rgba4444_default(const void *input, void *output,
							     uint32_t pitch, uint32_t width,
							     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgba8888_px_t *src_ptr = input;
	aipl_rgba4444_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgba8888_px_t *src = src_ptr + i * pitch;
		aipl_rgba4444_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgba8888_to_rgba4444(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGBA8888 & TO_RGBA5551) && !defined(AIPL_HELIUM_ACCELERATION)
aipl_error_t aipl_color_convert_rgba8888_to_rgba5551_default(const void *input, void *output,
							     uint32_t pitch, uint32_t width,
							     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgba8888_px_t *src_ptr = input;
	aipl_rgba5551_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgba8888_px_t *src = src_ptr + i * pitch;
		aipl_rgba5551_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgba8888_to_rgba5551(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGBA8888 & TO_BGR888) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba8888_to_bgr888_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	return aipl_color_convert_rgba8888_to_24bit_default(input, output, pitch, width, height, 2,
							    1, 0);
}
#endif

#if (AIPL_CONVERT_RGBA8888 & TO_RGB888) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba8888_to_rgb888_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	return aipl_color_convert_rgba8888_to_24bit_default(input, output, pitch, width, height, 0,
							    1, 2);
}
#endif

#if (AIPL_CONVERT_RGBA8888 & TO_RGB565) && !defined(AIPL_HELIUM_ACCELERATION) &&                   \
	(!defined(AIPL_DAVE2D_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba8888_to_rgb565_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgba8888_px_t *src_ptr = input;
	aipl_rgb565_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgba8888_px_t *src = src_ptr + i * pitch;
		aipl_rgb565_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgba8888_to_rgb565(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGBA8888 & TO_YV12) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba8888_to_yv12_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *v_ptr = output + yuv_size;
	uint8_t *u_ptr = v_ptr + yuv_size / 4;

	return aipl_color_convert_rgba8888_to_yuv_planar_default(input, pitch, width, height, y_ptr,
								 u_ptr, v_ptr);
}
#endif

#if (AIPL_CONVERT_RGBA8888 & TO_I420) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba8888_to_i420_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + yuv_size;
	uint8_t *v_ptr = u_ptr + yuv_size / 4;

	return aipl_color_convert_rgba8888_to_yuv_planar_default(input, pitch, width, height, y_ptr,
								 u_ptr, v_ptr);
}
#endif

#if (AIPL_CONVERT_RGBA8888 & TO_I422) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba8888_to_i422_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + yuv_size;
	uint8_t *v_ptr = u_ptr + yuv_size / 2;

	const aipl_rgba8888_px_t *src_ptr = input;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgba8888_px_t *src = src_ptr + i * pitch;

		uint8_t *y_dst = y_ptr + i * width;
		uint8_t *v_dst = v_ptr + i * width / 2;
		uint8_t *u_dst = u_ptr + i * width / 2;

		for (uint32_t j = 0; j < width; j += 2) {
			aipl_cnvt_px_rgba8888_to_yuv(y_dst, u_dst, v_dst, src);

			++src;
			++y_dst;
			++u_dst;
			++v_dst;

			aipl_cnvt_px_rgba8888_to_yuv_y(y_dst, src);

			++src;
			++y_dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGBA8888 & TO_I444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba8888_to_i444_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + yuv_size;
	uint8_t *v_ptr = u_ptr + yuv_size;

	const aipl_rgba8888_px_t *src_ptr = input;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgba8888_px_t *src = src_ptr + i * pitch;

		uint8_t *y_dst = y_ptr + i * width;
		uint8_t *v_dst = v_ptr + i * width;
		uint8_t *u_dst = u_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgba8888_to_yuv(y_dst, u_dst, v_dst, src);

			++src;
			++y_dst;
			++u_dst;
			++v_dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGBA8888 & TO_ALPHA8_I400) &&                                                    \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba8888_to_i400_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	return aipl_color_convert_rgba8888_to_alpha8_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_RGBA8888 & TO_NV21) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba8888_to_nv21_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *v_ptr = output + yuv_size;
	uint8_t *u_ptr = v_ptr + 1;

	return aipl_color_convert_rgba8888_to_yuv_semi_planar_default(input, pitch, width, height,
								      y_ptr, u_ptr, v_ptr);
}
#endif

#if (AIPL_CONVERT_RGBA8888 & TO_NV12) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba8888_to_nv12_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + yuv_size;
	uint8_t *v_ptr = u_ptr + 1;

	return aipl_color_convert_rgba8888_to_yuv_semi_planar_default(input, pitch, width, height,
								      y_ptr, u_ptr, v_ptr);
}
#endif

#if (AIPL_CONVERT_RGBA8888 & TO_YUY2) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba8888_to_yuy2_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + 1;
	uint8_t *v_ptr = u_ptr + 2;

	return aipl_color_convert_rgba8888_to_yuv_packed_default(input, pitch, width, height, y_ptr,
								 u_ptr, v_ptr);
}
#endif

#if (AIPL_CONVERT_RGBA8888 & TO_UYVY) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba8888_to_uyvy_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint8_t *y_ptr = output + 1;
	uint8_t *u_ptr = output;
	uint8_t *v_ptr = u_ptr + 2;

	return aipl_color_convert_rgba8888_to_yuv_packed_default(input, pitch, width, height, y_ptr,
								 u_ptr, v_ptr);
}
#endif
#endif

#if AIPL_CONVERT_RGBA4444
aipl_error_t aipl_color_convert_rgba4444_default(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height,
						 aipl_color_format_t format)
{
	switch (format) {
		/* Alpha color formats */
#if (AIPL_CONVERT_RGBA4444 & TO_ALPHA8_I400) &&                                                    \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ALPHA8:
		return aipl_color_convert_rgba4444_to_alpha8_default(input, output, pitch, width,
								     height);
#endif
		/* RGB color formats */
#if (AIPL_CONVERT_RGBA4444 & TO_ARGB8888) && !defined(AIPL_HELIUM_ACCELERATION) &&                 \
	(!defined(AIPL_DAVE2D_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB8888:
		return aipl_color_convert_rgba4444_to_argb8888_default(input, output, pitch, width,
								       height);
#endif
#if (AIPL_CONVERT_RGBA4444 & TO_RGBA8888) && !defined(AIPL_HELIUM_ACCELERATION) &&                 \
	(!defined(AIPL_DAVE2D_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA8888:
		return aipl_color_convert_rgba4444_to_rgba8888_default(input, output, pitch, width,
								       height);
#endif
#if (AIPL_CONVERT_RGBA4444 & TO_ARGB4444) &&                                                       \
	((!(defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD)) &&               \
	  !defined(AIPL_HELIUM_ACCELERATION)) ||                                                   \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB4444:
		return aipl_color_convert_rgba4444_to_argb4444_default(input, output, pitch, width,
								       height);
#endif
#if (AIPL_CONVERT_RGBA4444 & TO_ARGB1555)
	case AIPL_COLOR_ARGB1555:
		return aipl_color_convert_rgba4444_to_argb1555_default(input, output, pitch, width,
								       height);
#endif
	case AIPL_COLOR_RGBA4444:
		return AIPL_ERR_FORMAT_MISMATCH;
#if (AIPL_CONVERT_RGBA4444 & TO_RGBA5551)
	case AIPL_COLOR_RGBA5551:
		return aipl_color_convert_rgba4444_to_rgba5551_default(input, output, pitch, width,
								       height);
#endif
#if (AIPL_CONVERT_RGBA4444 & TO_RGB565) &&                                                         \
	(!(defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD)) ||                \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGB565:
		return aipl_color_convert_rgba4444_to_rgb565_default(input, output, pitch, width,
								     height);
#endif
#if (AIPL_CONVERT_RGBA4444 & TO_BGR888) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_BGR888:
		return aipl_color_convert_rgba4444_to_bgr888_default(input, output, pitch, width,
								     height);
#endif
#if (AIPL_CONVERT_RGBA4444 & TO_RGB888) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGB888:
		return aipl_color_convert_rgba4444_to_rgb888_default(input, output, pitch, width,
								     height);
#endif
		/* YUV color formats */
#if (AIPL_CONVERT_RGBA4444 & TO_YV12) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_YV12:
		return aipl_color_convert_rgba4444_to_yv12_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_RGBA4444 & TO_I420) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I420:
		return aipl_color_convert_rgba4444_to_i420_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_RGBA4444 & TO_NV12) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_NV12:
		return aipl_color_convert_rgba4444_to_nv12_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_RGBA4444 & TO_NV21) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_NV21:
		return aipl_color_convert_rgba4444_to_nv21_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_RGBA4444 & TO_I422) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I422:
		return aipl_color_convert_rgba4444_to_i422_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_RGBA4444 & TO_YUY2) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_YUY2:
		return aipl_color_convert_rgba4444_to_yuy2_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_RGBA4444 & TO_UYVY) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_UYVY:
		return aipl_color_convert_rgba4444_to_uyvy_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_RGBA4444 & TO_I444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I444:
		return aipl_color_convert_rgba4444_to_i444_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_RGBA4444 & TO_ALPHA8_I400) &&                                                    \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I400:
		return aipl_color_convert_rgba4444_to_i400_default(input, output, pitch, width,
								   height);
#endif

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

#if (AIPL_CONVERT_RGBA4444 & TO_ALPHA8_I400) &&                                                    \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba4444_to_alpha8_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgba4444_px_t *src_ptr = input;
	uint8_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgba4444_px_t *src = src_ptr + i * pitch;
		uint8_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgba4444_to_yuv_y(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGBA4444 & TO_ARGB8888) && !defined(AIPL_HELIUM_ACCELERATION) &&                 \
	(!defined(AIPL_DAVE2D_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba4444_to_argb8888_default(const void *input, void *output,
							     uint32_t pitch, uint32_t width,
							     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgba4444_px_t *src_ptr = input;
	aipl_argb8888_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgba4444_px_t *src = src_ptr + i * pitch;
		aipl_argb8888_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgba4444_to_argb8888(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGBA4444 & TO_ARGB1555)
aipl_error_t aipl_color_convert_rgba4444_to_argb1555_default(const void *input, void *output,
							     uint32_t pitch, uint32_t width,
							     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgba4444_px_t *src_ptr = input;
	aipl_argb1555_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgba4444_px_t *src = src_ptr + i * pitch;
		aipl_argb1555_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgba4444_to_argb1555(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGBA4444 & TO_ARGB4444) &&                                                       \
	((!(defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD)) &&               \
	  !defined(AIPL_HELIUM_ACCELERATION)) ||                                                   \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba4444_to_argb4444_default(const void *input, void *output,
							     uint32_t pitch, uint32_t width,
							     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgba4444_px_t *src_ptr = input;
	aipl_argb4444_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgba4444_px_t *src = src_ptr + i * pitch;
		aipl_argb4444_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgba4444_to_argb4444(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGBA4444 & TO_RGBA8888) && !defined(AIPL_HELIUM_ACCELERATION) &&                 \
	(!defined(AIPL_DAVE2D_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba4444_to_rgba8888_default(const void *input, void *output,
							     uint32_t pitch, uint32_t width,
							     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgba4444_px_t *src_ptr = input;
	aipl_rgba8888_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgba4444_px_t *src = src_ptr + i * pitch;
		aipl_rgba8888_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgba4444_to_rgba8888(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGBA4444 & TO_RGBA5551)
aipl_error_t aipl_color_convert_rgba4444_to_rgba5551_default(const void *input, void *output,
							     uint32_t pitch, uint32_t width,
							     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgba4444_px_t *src_ptr = input;
	aipl_rgba5551_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgba4444_px_t *src = src_ptr + i * pitch;
		aipl_rgba5551_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgba4444_to_rgba5551(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGBA4444 & TO_BGR888) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba4444_to_bgr888_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	return aipl_color_convert_rgba4444_to_24bit_default(input, output, pitch, width, height, 2,
							    1, 0);
}
#endif

#if (AIPL_CONVERT_RGBA4444 & TO_RGB888) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba4444_to_rgb888_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	return aipl_color_convert_rgba4444_to_24bit_default(input, output, pitch, width, height, 0,
							    1, 2);
}
#endif

#if (AIPL_CONVERT_RGBA4444 & TO_RGB565) &&                                                         \
	(!(defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD)) ||                \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba4444_to_rgb565_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgba4444_px_t *src_ptr = input;
	aipl_rgb565_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgba4444_px_t *src = src_ptr + i * pitch;
		aipl_rgb565_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgba4444_to_rgb565(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGBA4444 & TO_YV12) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba4444_to_yv12_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *v_ptr = output + yuv_size;
	uint8_t *u_ptr = v_ptr + yuv_size / 4;

	return aipl_color_convert_rgba4444_to_yuv_planar_default(input, pitch, width, height, y_ptr,
								 u_ptr, v_ptr);
}
#endif

#if (AIPL_CONVERT_RGBA4444 & TO_I420) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba4444_to_i420_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + yuv_size;
	uint8_t *v_ptr = u_ptr + yuv_size / 4;

	return aipl_color_convert_rgba4444_to_yuv_planar_default(input, pitch, width, height, y_ptr,
								 u_ptr, v_ptr);
}
#endif

#if (AIPL_CONVERT_RGBA4444 & TO_I422) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba4444_to_i422_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + yuv_size;
	uint8_t *v_ptr = u_ptr + yuv_size / 2;

	const aipl_rgba4444_px_t *src_ptr = input;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgba4444_px_t *src = src_ptr + i * pitch;
		uint8_t *y_dst = y_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgba4444_to_yuv_y(y_dst, src);

			++src;
			++y_dst;
		}

		src = src_ptr + i * pitch;
		uint8_t *u_dst = u_ptr + i * width / 2;
		uint8_t *v_dst = v_ptr + i * width / 2;

		for (uint32_t j = 0; j < width / 2; ++j) {
			aipl_cnvt_px_rgba4444_to_yuv_uv(u_dst, v_dst, src);

			src += 2;
			++u_dst;
			++v_dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGBA4444 & TO_I444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba4444_to_i444_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + yuv_size;
	uint8_t *v_ptr = u_ptr + yuv_size;

	const aipl_rgba4444_px_t *src_ptr = input;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgba4444_px_t *src = src_ptr + i * pitch;

		uint8_t *y_dst = y_ptr + i * width;
		uint8_t *v_dst = v_ptr + i * width;
		uint8_t *u_dst = u_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgba4444_to_yuv(y_dst, u_dst, v_dst, src);

			++src;
			++y_dst;
			++u_dst;
			++v_dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGBA4444 & TO_ALPHA8_I400) &&                                                    \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba4444_to_i400_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	return aipl_color_convert_rgba4444_to_alpha8_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_RGBA4444 & TO_NV21) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba4444_to_nv21_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *v_ptr = output + yuv_size;
	uint8_t *u_ptr = v_ptr + 1;

	return aipl_color_convert_rgba4444_to_yuv_semi_planar_default(input, pitch, width, height,
								      y_ptr, u_ptr, v_ptr);
}
#endif

#if (AIPL_CONVERT_RGBA4444 & TO_NV12) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba4444_to_nv12_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + yuv_size;
	uint8_t *v_ptr = u_ptr + 1;

	return aipl_color_convert_rgba4444_to_yuv_semi_planar_default(input, pitch, width, height,
								      y_ptr, u_ptr, v_ptr);
}
#endif

#if (AIPL_CONVERT_RGBA4444 & TO_YUY2) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba4444_to_yuy2_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + 1;
	uint8_t *v_ptr = u_ptr + 2;

	return aipl_color_convert_rgba4444_to_yuv_packed_default(input, pitch, width, height, y_ptr,
								 u_ptr, v_ptr);
}
#endif

#if (AIPL_CONVERT_RGBA4444 & TO_UYVY) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba4444_to_uyvy_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint8_t *y_ptr = output + 1;
	uint8_t *u_ptr = output;
	uint8_t *v_ptr = u_ptr + 2;

	return aipl_color_convert_rgba4444_to_yuv_packed_default(input, pitch, width, height, y_ptr,
								 u_ptr, v_ptr);
}
#endif
#endif

#if AIPL_CONVERT_RGBA5551
aipl_error_t aipl_color_convert_rgba5551_default(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height,
						 aipl_color_format_t format)
{
	switch (format) {
		/* Alpha color formats */
#if (AIPL_CONVERT_RGBA5551 & TO_ALPHA8_I400) &&                                                    \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ALPHA8:
		return aipl_color_convert_rgba5551_to_alpha8_default(input, output, pitch, width,
								     height);
#endif
		/* RGB color formats */
#if (AIPL_CONVERT_RGBA5551 & TO_ARGB8888) && !defined(AIPL_HELIUM_ACCELERATION) &&                 \
	(!defined(AIPL_DAVE2D_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB8888:
		return aipl_color_convert_rgba5551_to_argb8888_default(input, output, pitch, width,
								       height);
#endif
#if (AIPL_CONVERT_RGBA5551 & TO_RGBA8888) && !defined(AIPL_HELIUM_ACCELERATION) &&                 \
	(!defined(AIPL_DAVE2D_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA8888:
		return aipl_color_convert_rgba5551_to_rgba8888_default(input, output, pitch, width,
								       height);
#endif
#if (AIPL_CONVERT_RGBA5551 & TO_ARGB4444) &&                                                       \
	(!(defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD)) ||                \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB4444:
		return aipl_color_convert_rgba5551_to_argb4444_default(input, output, pitch, width,
								       height);
#endif
#if (AIPL_CONVERT_RGBA5551 & TO_ARGB1555)
	case AIPL_COLOR_ARGB1555:
		return aipl_color_convert_rgba5551_to_argb1555_default(input, output, pitch, width,
								       height);
#endif
#if (AIPL_CONVERT_RGBA5551 & TO_RGBA4444) &&                                                       \
	(!(defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD)) ||                \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA4444:
		return aipl_color_convert_rgba5551_to_rgba4444_default(input, output, pitch, width,
								       height);
#endif
	case AIPL_COLOR_RGBA5551:
		return AIPL_ERR_FORMAT_MISMATCH;
#if (AIPL_CONVERT_RGBA5551 & TO_RGB565) &&                                                         \
	(!(defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD)) ||                \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGB565:
		return aipl_color_convert_rgba5551_to_rgb565_default(input, output, pitch, width,
								     height);
#endif
#if (AIPL_CONVERT_RGBA5551 & TO_BGR888) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_BGR888:
		return aipl_color_convert_rgba5551_to_bgr888_default(input, output, pitch, width,
								     height);
#endif
#if (AIPL_CONVERT_RGBA5551 & TO_RGB888) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGB888:
		return aipl_color_convert_rgba5551_to_rgb888_default(input, output, pitch, width,
								     height);
#endif
		/* YUV color formats */
#if (AIPL_CONVERT_RGBA5551 & TO_YV12) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_YV12:
		return aipl_color_convert_rgba5551_to_yv12_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_RGBA5551 & TO_I420) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I420:
		return aipl_color_convert_rgba5551_to_i420_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_RGBA5551 & TO_NV12) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_NV12:
		return aipl_color_convert_rgba5551_to_nv12_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_RGBA5551 & TO_NV21) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_NV21:
		return aipl_color_convert_rgba5551_to_nv21_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_RGBA5551 & TO_I422) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I422:
		return aipl_color_convert_rgba5551_to_i422_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_RGBA5551 & TO_YUY2) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_YUY2:
		return aipl_color_convert_rgba5551_to_yuy2_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_RGBA5551 & TO_UYVY) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_UYVY:
		return aipl_color_convert_rgba5551_to_uyvy_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_RGBA5551 & TO_I444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I444:
		return aipl_color_convert_rgba5551_to_i444_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_RGBA5551 & TO_ALPHA8_I400) &&                                                    \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I400:
		return aipl_color_convert_rgba5551_to_i400_default(input, output, pitch, width,
								   height);
#endif

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

#if (AIPL_CONVERT_RGBA5551 & TO_ALPHA8_I400) &&                                                    \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba5551_to_alpha8_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgba5551_px_t *src_ptr = input;
	uint8_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgba5551_px_t *src = src_ptr + i * pitch;
		uint8_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgba5551_to_yuv_y(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGBA5551 & TO_ARGB8888) && !defined(AIPL_HELIUM_ACCELERATION) &&                 \
	(!defined(AIPL_DAVE2D_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba5551_to_argb8888_default(const void *input, void *output,
							     uint32_t pitch, uint32_t width,
							     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgba5551_px_t *src_ptr = input;
	aipl_argb8888_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgba5551_px_t *src = src_ptr + i * pitch;
		aipl_argb8888_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgba5551_to_argb8888(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGBA5551 & TO_ARGB1555)
aipl_error_t aipl_color_convert_rgba5551_to_argb1555_default(const void *input, void *output,
							     uint32_t pitch, uint32_t width,
							     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgba5551_px_t *src_ptr = input;
	aipl_argb1555_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgba5551_px_t *src = src_ptr + i * pitch;
		aipl_argb1555_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgba5551_to_argb1555(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGBA5551 & TO_ARGB4444) &&                                                       \
	(!(defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD)) ||                \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba5551_to_argb4444_default(const void *input, void *output,
							     uint32_t pitch, uint32_t width,
							     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgba5551_px_t *src_ptr = input;
	aipl_argb4444_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgba5551_px_t *src = src_ptr + i * pitch;
		aipl_argb4444_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgba5551_to_argb4444(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGBA5551 & TO_RGBA8888) && !defined(AIPL_HELIUM_ACCELERATION) &&                 \
	(!defined(AIPL_DAVE2D_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba5551_to_rgba8888_default(const void *input, void *output,
							     uint32_t pitch, uint32_t width,
							     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgba5551_px_t *src_ptr = input;
	aipl_rgba8888_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgba5551_px_t *src = src_ptr + i * pitch;
		aipl_rgba8888_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgba5551_to_rgba8888(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGBA5551 & TO_RGBA4444) &&                                                       \
	(!(defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD)) ||                \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba5551_to_rgba4444_default(const void *input, void *output,
							     uint32_t pitch, uint32_t width,
							     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgba5551_px_t *src_ptr = input;
	aipl_rgba4444_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgba5551_px_t *src = src_ptr + i * pitch;
		aipl_rgba4444_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgba5551_to_rgba4444(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGBA5551 & TO_BGR888) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba5551_to_bgr888_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	return aipl_color_convert_rgba5551_to_24bit_default(input, output, pitch, width, height, 2,
							    1, 0);
}
#endif

#if (AIPL_CONVERT_RGBA5551 & TO_RGB888) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba5551_to_rgb888_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	return aipl_color_convert_rgba5551_to_24bit_default(input, output, pitch, width, height, 0,
							    1, 2);
}
#endif

#if (AIPL_CONVERT_RGBA5551 & TO_RGB565) &&                                                         \
	(!(defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD)) ||                \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba5551_to_rgb565_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgba5551_px_t *src_ptr = input;
	aipl_rgb565_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgba5551_px_t *src = src_ptr + i * pitch;
		aipl_rgb565_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgba5551_to_rgb565(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGBA5551 & TO_YV12) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba5551_to_yv12_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *v_ptr = output + yuv_size;
	uint8_t *u_ptr = v_ptr + yuv_size / 4;

	return aipl_color_convert_rgba5551_to_yuv_planar_default(input, pitch, width, height, y_ptr,
								 u_ptr, v_ptr);
}
#endif

#if (AIPL_CONVERT_RGBA5551 & TO_I420) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba5551_to_i420_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + yuv_size;
	uint8_t *v_ptr = u_ptr + yuv_size / 4;

	return aipl_color_convert_rgba5551_to_yuv_planar_default(input, pitch, width, height, y_ptr,
								 u_ptr, v_ptr);
}
#endif

#if (AIPL_CONVERT_RGBA5551 & TO_I422) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba5551_to_i422_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + yuv_size;
	uint8_t *v_ptr = u_ptr + yuv_size / 2;

	const aipl_rgba5551_px_t *src_ptr = input;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgba5551_px_t *src = src_ptr + i * pitch;
		uint8_t *y_dst = y_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgba5551_to_yuv_y(y_dst, src);

			++src;
			++y_dst;
		}

		src = src_ptr + i * pitch;
		uint8_t *u_dst = u_ptr + i * width / 2;
		uint8_t *v_dst = v_ptr + i * width / 2;

		for (uint32_t j = 0; j < width / 2; ++j) {
			aipl_cnvt_px_rgba5551_to_yuv_uv(u_dst, v_dst, src);

			src += 2;
			++u_dst;
			++v_dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGBA5551 & TO_I444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba5551_to_i444_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + yuv_size;
	uint8_t *v_ptr = u_ptr + yuv_size;

	const aipl_rgba5551_px_t *src_ptr = input;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgba5551_px_t *src = src_ptr + i * pitch;

		uint8_t *y_dst = y_ptr + i * width;
		uint8_t *v_dst = v_ptr + i * width;
		uint8_t *u_dst = u_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgba5551_to_yuv(y_dst, u_dst, v_dst, src);

			++src;
			++y_dst;
			++u_dst;
			++v_dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGBA5551 & TO_ALPHA8_I400) &&                                                    \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba5551_to_i400_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	return aipl_color_convert_rgba5551_to_alpha8_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_RGBA5551 & TO_NV21) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba5551_to_nv21_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *v_ptr = output + yuv_size;
	uint8_t *u_ptr = v_ptr + 1;

	return aipl_color_convert_rgba5551_to_yuv_semi_planar_default(input, pitch, width, height,
								      y_ptr, u_ptr, v_ptr);
}
#endif

#if (AIPL_CONVERT_RGBA5551 & TO_NV12) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba5551_to_nv12_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + yuv_size;
	uint8_t *v_ptr = u_ptr + 1;

	return aipl_color_convert_rgba5551_to_yuv_semi_planar_default(input, pitch, width, height,
								      y_ptr, u_ptr, v_ptr);
}
#endif

#if (AIPL_CONVERT_RGBA5551 & TO_YUY2) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba5551_to_yuy2_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + 1;
	uint8_t *v_ptr = u_ptr + 2;

	return aipl_color_convert_rgba5551_to_yuv_packed_default(input, pitch, width, height, y_ptr,
								 u_ptr, v_ptr);
}
#endif

#if (AIPL_CONVERT_RGBA5551 & TO_UYVY) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba5551_to_uyvy_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint8_t *y_ptr = output + 1;
	uint8_t *u_ptr = output;
	uint8_t *v_ptr = u_ptr + 2;

	return aipl_color_convert_rgba5551_to_yuv_packed_default(input, pitch, width, height, y_ptr,
								 u_ptr, v_ptr);
}
#endif
#endif

#if AIPL_CONVERT_BGR888
aipl_error_t aipl_color_convert_bgr888_default(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height,
					       aipl_color_format_t format)
{
	switch (format) {
		/* Alpha color formats */
#if (AIPL_CONVERT_BGR888 & TO_ALPHA8_I400) &&                                                      \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ALPHA8:
		return aipl_color_convert_bgr888_to_alpha8_default(input, output, pitch, width,
								   height);
#endif
		/* RGB color formats */
#if (AIPL_CONVERT_BGR888 & TO_ARGB8888) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB8888:
		return aipl_color_convert_bgr888_to_argb8888_default(input, output, pitch, width,
								     height);
#endif
#if (AIPL_CONVERT_BGR888 & TO_RGBA8888) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA8888:
		return aipl_color_convert_bgr888_to_rgba8888_default(input, output, pitch, width,
								     height);
#endif
#if (AIPL_CONVERT_BGR888 & TO_ARGB4444) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB4444:
		return aipl_color_convert_bgr888_to_argb4444_default(input, output, pitch, width,
								     height);
#endif
#if (AIPL_CONVERT_BGR888 & TO_ARGB1555) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB1555:
		return aipl_color_convert_bgr888_to_argb1555_default(input, output, pitch, width,
								     height);
#endif
#if (AIPL_CONVERT_BGR888 & TO_RGBA4444) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA4444:
		return aipl_color_convert_bgr888_to_rgba4444_default(input, output, pitch, width,
								     height);
#endif
#if (AIPL_CONVERT_BGR888 & TO_RGBA5551) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA5551:
		return aipl_color_convert_bgr888_to_rgba5551_default(input, output, pitch, width,
								     height);
#endif
#if (AIPL_CONVERT_BGR888 & TO_RGB565) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGB565:
		return aipl_color_convert_bgr888_to_rgb565_default(input, output, pitch, width,
								   height);
#endif
	case AIPL_COLOR_BGR888:
		return AIPL_ERR_FORMAT_MISMATCH;
#if (AIPL_CONVERT_BGR888 & TO_RGB888) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGB888:
		return aipl_color_convert_bgr888_to_rgb888_default(input, output, pitch, width,
								   height);
#endif
		/* YUV color formats */
#if (AIPL_CONVERT_BGR888 & TO_YV12) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_YV12:
		return aipl_color_convert_bgr888_to_yv12_default(input, output, pitch, width,
								 height);
#endif
#if (AIPL_CONVERT_BGR888 & TO_I420) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I420:
		return aipl_color_convert_bgr888_to_i420_default(input, output, pitch, width,
								 height);
#endif
#if (AIPL_CONVERT_BGR888 & TO_NV12) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_NV12:
		return aipl_color_convert_bgr888_to_nv12_default(input, output, pitch, width,
								 height);
#endif
#if (AIPL_CONVERT_BGR888 & TO_NV21) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_NV21:
		return aipl_color_convert_bgr888_to_nv21_default(input, output, pitch, width,
								 height);
#endif
#if (AIPL_CONVERT_BGR888 & TO_I422) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I422:
		return aipl_color_convert_bgr888_to_i422_default(input, output, pitch, width,
								 height);
#endif
#if (AIPL_CONVERT_BGR888 & TO_YUY2) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_YUY2:
		return aipl_color_convert_bgr888_to_yuy2_default(input, output, pitch, width,
								 height);
#endif
#if (AIPL_CONVERT_BGR888 & TO_UYVY) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_UYVY:
		return aipl_color_convert_bgr888_to_uyvy_default(input, output, pitch, width,
								 height);
#endif
#if (AIPL_CONVERT_BGR888 & TO_I444) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I444:
		return aipl_color_convert_bgr888_to_i444_default(input, output, pitch, width,
								 height);
#endif
#if (AIPL_CONVERT_BGR888 & TO_ALPHA8_I400) &&                                                      \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I400:
		return aipl_color_convert_bgr888_to_i400_default(input, output, pitch, width,
								 height);
#endif

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

#if (AIPL_CONVERT_BGR888 & TO_ALPHA8_I400) &&                                                      \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_bgr888_to_alpha8_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	return aipl_color_convert_24bit_to_alpha8_default(input, output, pitch, width, height, 2, 1,
							  0);
}
#endif

#if (AIPL_CONVERT_BGR888 & TO_ARGB8888) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_bgr888_to_argb8888_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	return aipl_color_convert_24bit_to_argb8888_default(input, output, pitch, width, height, 2,
							    1, 0);
}
#endif

#if (AIPL_CONVERT_BGR888 & TO_ARGB1555) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_bgr888_to_argb1555_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	return aipl_color_convert_24bit_to_argb1555_default(input, output, pitch, width, height, 2,
							    1, 0);
}
#endif

#if (AIPL_CONVERT_BGR888 & TO_ARGB4444) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_bgr888_to_argb4444_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	return aipl_color_convert_24bit_to_argb4444_default(input, output, pitch, width, height, 2,
							    1, 0);
}
#endif

#if (AIPL_CONVERT_BGR888 & TO_RGBA8888) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_bgr888_to_rgba8888_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	return aipl_color_convert_24bit_to_rgba8888_default(input, output, pitch, width, height, 2,
							    1, 0);
}
#endif

#if (AIPL_CONVERT_BGR888 & TO_RGBA4444) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_bgr888_to_rgba4444_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	return aipl_color_convert_24bit_to_rgba4444_default(input, output, pitch, width, height, 2,
							    1, 0);
}
#endif

#if (AIPL_CONVERT_BGR888 & TO_RGBA5551) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_bgr888_to_rgba5551_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	return aipl_color_convert_24bit_to_rgba5551_default(input, output, pitch, width, height, 2,
							    1, 0);
}
#endif

#if (AIPL_CONVERT_BGR888 & TO_RGB565) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_bgr888_to_rgb565_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	return aipl_color_convert_24bit_to_rgb565_default(input, output, pitch, width, height, 2, 1,
							  0);
}
#endif

#if (AIPL_CONVERT_BGR888 & TO_RGB888) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_bgr888_to_rgb888_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	return aipl_color_convert_24bit_to_24bit_default(input, output, pitch, width, height, 2, 1,
							 0, 0, 1, 2);
}
#endif

#if (AIPL_CONVERT_BGR888 & TO_YV12) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_bgr888_to_yv12_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *v_ptr = output + yuv_size;
	uint8_t *u_ptr = v_ptr + yuv_size / 4;

	return aipl_color_convert_24bit_to_yuv_planar_default(input, pitch, width, height, y_ptr,
							      u_ptr, v_ptr, 2, 1, 0);
}
#endif

#if (AIPL_CONVERT_BGR888 & TO_I420) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_bgr888_to_i420_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + yuv_size;
	uint8_t *v_ptr = u_ptr + yuv_size / 4;

	return aipl_color_convert_24bit_to_yuv_planar_default(input, pitch, width, height, y_ptr,
							      u_ptr, v_ptr, 2, 1, 0);
}
#endif

#if (AIPL_CONVERT_BGR888 & TO_I422) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_bgr888_to_i422_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint8_t *src_ptr = input;
	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + yuv_size;
	uint8_t *v_ptr = u_ptr + yuv_size / 2;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *src = src_ptr + i * pitch * 3;

		uint8_t *y_dst = y_ptr + i * width;
		uint8_t *v_dst = v_ptr + i * width / 2;
		uint8_t *u_dst = u_ptr + i * width / 2;

		for (uint32_t j = 0; j < width; j += 2) {
			aipl_cnvt_px_24bit_to_yuv(y_dst, u_dst, v_dst, src, 2, 1, 0);

			src += 3;
			++y_dst;
			++u_dst;
			++v_dst;

			aipl_cnvt_px_24bit_to_yuv_y(y_dst, src, 2, 1, 0);

			src += 3;
			++y_dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_BGR888 & TO_I444) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_bgr888_to_i444_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint8_t *src_ptr = input;
	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + yuv_size;
	uint8_t *v_ptr = u_ptr + yuv_size;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *src = src_ptr + i * pitch * 3;

		uint8_t *y_dst = y_ptr + i * width;
		uint8_t *v_dst = v_ptr + i * width;
		uint8_t *u_dst = u_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_24bit_to_yuv(y_dst, u_dst, v_dst, src, 2, 1, 0);

			src += 3;
			++y_dst;
			++u_dst;
			++v_dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_BGR888 & TO_ALPHA8_I400) &&                                                      \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_bgr888_to_i400_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	return aipl_color_convert_24bit_to_alpha8_default(input, output, pitch, width, height, 2, 1,
							  0);
}
#endif

#if (AIPL_CONVERT_BGR888 & TO_NV21) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_bgr888_to_nv21_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *v_ptr = output + yuv_size;
	uint8_t *u_ptr = v_ptr + 1;

	return aipl_color_convert_24bit_to_yuv_semi_planar_default(input, pitch, width, height,
								   y_ptr, u_ptr, v_ptr, 2, 1, 0);
}
#endif

#if (AIPL_CONVERT_BGR888 & TO_NV12) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_bgr888_to_nv12_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + yuv_size;
	uint8_t *v_ptr = u_ptr + 1;

	return aipl_color_convert_24bit_to_yuv_semi_planar_default(input, pitch, width, height,
								   y_ptr, u_ptr, v_ptr, 2, 1, 0);
}
#endif

#if (AIPL_CONVERT_BGR888 & TO_YUY2) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_bgr888_to_yuy2_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + 1;
	uint8_t *v_ptr = u_ptr + 2;

	return aipl_color_convert_bgr888_to_yuv_packed_default(input, pitch, width, height, y_ptr,
							       u_ptr, v_ptr);
}
#endif

#if (AIPL_CONVERT_BGR888 & TO_UYVY) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_bgr888_to_uyvy_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	uint8_t *y_ptr = output + 1;
	uint8_t *u_ptr = output;
	uint8_t *v_ptr = u_ptr + 2;

	return aipl_color_convert_bgr888_to_yuv_packed_default(input, pitch, width, height, y_ptr,
							       u_ptr, v_ptr);
}
#endif
#endif

#if AIPL_CONVERT_RGB888
aipl_error_t aipl_color_convert_rgb888_default(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height,
					       aipl_color_format_t format)
{
	switch (format) {
		/* Alpha color formats */
#if (AIPL_CONVERT_RGB888 & TO_ALPHA8_I400) &&                                                      \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ALPHA8:
		return aipl_color_convert_rgb888_to_alpha8_default(input, output, pitch, width,
								   height);
#endif
		/* RGB color formats */
#if (AIPL_CONVERT_RGB888 & TO_ARGB8888) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB8888:
		return aipl_color_convert_rgb888_to_argb8888_default(input, output, pitch, width,
								     height);
#endif
#if (AIPL_CONVERT_RGB888 & TO_RGBA8888) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA8888:
		return aipl_color_convert_rgb888_to_rgba8888_default(input, output, pitch, width,
								     height);
#endif
#if (AIPL_CONVERT_RGB888 & TO_ARGB4444) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB4444:
		return aipl_color_convert_rgb888_to_argb4444_default(input, output, pitch, width,
								     height);
#endif
#if (AIPL_CONVERT_RGB888 & TO_ARGB1555) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB1555:
		return aipl_color_convert_rgb888_to_argb1555_default(input, output, pitch, width,
								     height);
#endif
#if (AIPL_CONVERT_RGB888 & TO_RGBA4444) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA4444:
		return aipl_color_convert_rgb888_to_rgba4444_default(input, output, pitch, width,
								     height);
#endif
#if (AIPL_CONVERT_RGB888 & TO_RGBA5551) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA5551:
		return aipl_color_convert_rgb888_to_rgba5551_default(input, output, pitch, width,
								     height);
#endif
#if (AIPL_CONVERT_RGB888 & TO_RGB565) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGB565:
		return aipl_color_convert_rgb888_to_rgb565_default(input, output, pitch, width,
								   height);
#endif
	case AIPL_COLOR_RGB888:
		return AIPL_ERR_FORMAT_MISMATCH;
#if (AIPL_CONVERT_RGB888 & TO_BGR888) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_BGR888:
		return aipl_color_convert_rgb888_to_bgr888_default(input, output, pitch, width,
								   height);
#endif
		/* YUV color formats */
#if (AIPL_CONVERT_RGB888 & TO_YV12) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_YV12:
		return aipl_color_convert_rgb888_to_yv12_default(input, output, pitch, width,
								 height);
#endif
#if (AIPL_CONVERT_RGB888 & TO_I420) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I420:
		return aipl_color_convert_rgb888_to_i420_default(input, output, pitch, width,
								 height);
#endif
#if (AIPL_CONVERT_RGB888 & TO_NV12) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_NV12:
		return aipl_color_convert_rgb888_to_nv12_default(input, output, pitch, width,
								 height);
#endif
#if (AIPL_CONVERT_RGB888 & TO_NV21) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_NV21:
		return aipl_color_convert_rgb888_to_nv21_default(input, output, pitch, width,
								 height);
#endif
#if (AIPL_CONVERT_RGB888 & TO_I422) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I422:
		return aipl_color_convert_rgb888_to_i422_default(input, output, pitch, width,
								 height);
#endif
#if (AIPL_CONVERT_RGB888 & TO_YUY2) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_YUY2:
		return aipl_color_convert_rgb888_to_yuy2_default(input, output, pitch, width,
								 height);
#endif
#if (AIPL_CONVERT_RGB888 & TO_UYVY) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_UYVY:
		return aipl_color_convert_rgb888_to_uyvy_default(input, output, pitch, width,
								 height);
#endif
#if (AIPL_CONVERT_RGB888 & TO_I444) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I444:
		return aipl_color_convert_rgb888_to_i444_default(input, output, pitch, width,
								 height);
#endif
#if (AIPL_CONVERT_RGB888 & TO_ALPHA8_I400) &&                                                      \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I400:
		return aipl_color_convert_rgb888_to_i400_default(input, output, pitch, width,
								 height);
#endif

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

#if (AIPL_CONVERT_RGB888 & TO_ALPHA8_I400) &&                                                      \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb888_to_alpha8_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	return aipl_color_convert_24bit_to_alpha8_default(input, output, pitch, width, height, 0, 1,
							  2);
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_ARGB8888) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb888_to_argb8888_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	return aipl_color_convert_24bit_to_argb8888_default(input, output, pitch, width, height, 0,
							    1, 2);
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_ARGB1555) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb888_to_argb1555_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	return aipl_color_convert_24bit_to_argb1555_default(input, output, pitch, width, height, 0,
							    1, 2);
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_ARGB4444) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb888_to_argb4444_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	return aipl_color_convert_24bit_to_argb4444_default(input, output, pitch, width, height, 0,
							    1, 2);
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_RGBA8888) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb888_to_rgba8888_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	return aipl_color_convert_24bit_to_rgba8888_default(input, output, pitch, width, height, 0,
							    1, 2);
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_RGBA4444) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb888_to_rgba4444_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	return aipl_color_convert_24bit_to_rgba4444_default(input, output, pitch, width, height, 0,
							    1, 2);
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_RGBA5551) &&                                                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb888_to_rgba5551_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	return aipl_color_convert_24bit_to_rgba5551_default(input, output, pitch, width, height, 0,
							    1, 2);
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_RGB565) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb888_to_rgb565_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	return aipl_color_convert_24bit_to_rgb565_default(input, output, pitch, width, height, 0, 1,
							  2);
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_BGR888) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb888_to_bgr888_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	return aipl_color_convert_24bit_to_24bit_default(input, output, pitch, width, height, 0, 1,
							 2, 2, 1, 0);
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_YV12) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb888_to_yv12_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *v_ptr = output + yuv_size;
	uint8_t *u_ptr = v_ptr + yuv_size / 4;

	return aipl_color_convert_24bit_to_yuv_planar_default(input, pitch, width, height, y_ptr,
							      u_ptr, v_ptr, 0, 1, 2);
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_I420) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb888_to_i420_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + yuv_size;
	uint8_t *v_ptr = u_ptr + yuv_size / 4;

	return aipl_color_convert_24bit_to_yuv_planar_default(input, pitch, width, height, y_ptr,
							      u_ptr, v_ptr, 0, 1, 2);
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_I422) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb888_to_i422_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint8_t *src_ptr = input;
	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + yuv_size;
	uint8_t *v_ptr = u_ptr + yuv_size / 2;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *src = src_ptr + i * pitch * 3;

		uint8_t *y_dst = y_ptr + i * width;
		uint8_t *v_dst = v_ptr + i * width / 2;
		uint8_t *u_dst = u_ptr + i * width / 2;

		for (uint32_t j = 0; j < width; j += 2) {
			aipl_cnvt_px_24bit_to_yuv(y_dst, u_dst, v_dst, src, 0, 1, 2);

			src += 3;
			++y_dst;
			++u_dst;
			++v_dst;

			aipl_cnvt_px_24bit_to_yuv_y(y_dst, src, 0, 1, 2);

			src += 3;
			++y_dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_I444) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb888_to_i444_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint8_t *src_ptr = input;
	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + yuv_size;
	uint8_t *v_ptr = u_ptr + yuv_size;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *src = src_ptr + i * pitch * 3;

		uint8_t *y_dst = y_ptr + i * width;
		uint8_t *v_dst = v_ptr + i * width;
		uint8_t *u_dst = u_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_24bit_to_yuv(y_dst, u_dst, v_dst, src, 0, 1, 2);

			src += 3;
			++y_dst;
			++u_dst;
			++v_dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_ALPHA8_I400) &&                                                      \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb888_to_i400_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	return aipl_color_convert_24bit_to_alpha8_default(input, output, pitch, width, height, 0, 1,
							  2);
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_NV21) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb888_to_nv21_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *v_ptr = output + yuv_size;
	uint8_t *u_ptr = v_ptr + 1;

	return aipl_color_convert_24bit_to_yuv_semi_planar_default(input, pitch, width, height,
								   y_ptr, u_ptr, v_ptr, 0, 1, 2);
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_NV12) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb888_to_nv12_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + yuv_size;
	uint8_t *v_ptr = u_ptr + 1;

	return aipl_color_convert_24bit_to_yuv_semi_planar_default(input, pitch, width, height,
								   y_ptr, u_ptr, v_ptr, 0, 1, 2);
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_YUY2) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb888_to_yuy2_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + 1;
	uint8_t *v_ptr = u_ptr + 2;

	return aipl_color_convert_rgb888_to_yuv_packed_default(input, pitch, width, height, y_ptr,
							       u_ptr, v_ptr);
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_UYVY) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb888_to_uyvy_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	uint8_t *y_ptr = output + 1;
	uint8_t *u_ptr = output;
	uint8_t *v_ptr = u_ptr + 2;

	return aipl_color_convert_rgb888_to_yuv_packed_default(input, pitch, width, height, y_ptr,
							       u_ptr, v_ptr);
}
#endif
#endif

#if AIPL_CONVERT_RGB565
aipl_error_t aipl_color_convert_rgb565_default(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height,
					       aipl_color_format_t format)
{
	switch (format) {
		/* Alpha color formats */
#if (AIPL_CONVERT_RGB565 & TO_ALPHA8_I400) &&                                                      \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ALPHA8:
		return aipl_color_convert_rgb565_to_alpha8_default(input, output, pitch, width,
								   height);
#endif
		/* RGB color formats */
#if (AIPL_CONVERT_RGB565 & TO_ARGB8888) &&                                                         \
	(!defined(AIPL_DAVE2D_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB8888:
		return aipl_color_convert_rgb565_to_argb8888_default(input, output, pitch, width,
								     height);
#endif
#if (AIPL_CONVERT_RGB565 & TO_RGBA8888) &&                                                         \
	(!defined(AIPL_DAVE2D_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA8888:
		return aipl_color_convert_rgb565_to_rgba8888_default(input, output, pitch, width,
								     height);
#endif
#if (AIPL_CONVERT_RGB565 & TO_ARGB4444) &&                                                         \
	(!(defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD)) ||                \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB4444:
		return aipl_color_convert_rgb565_to_argb4444_default(input, output, pitch, width,
								     height);
#endif
#if (AIPL_CONVERT_RGB565 & TO_ARGB1555)
	case AIPL_COLOR_ARGB1555:
		return aipl_color_convert_rgb565_to_argb1555_default(input, output, pitch, width,
								     height);
#endif
#if (AIPL_CONVERT_RGB565 & TO_RGBA4444) &&                                                         \
	(!(defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD)) ||                \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA4444:
		return aipl_color_convert_rgb565_to_rgba4444_default(input, output, pitch, width,
								     height);
#endif
#if (AIPL_CONVERT_RGB565 & TO_RGBA5551)
	case AIPL_COLOR_RGBA5551:
		return aipl_color_convert_rgb565_to_rgba5551_default(input, output, pitch, width,
								     height);
#endif
	case AIPL_COLOR_RGB565:
		return AIPL_ERR_FORMAT_MISMATCH;
#if (AIPL_CONVERT_RGB565 & TO_BGR888) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_BGR888:
		return aipl_color_convert_rgb565_to_bgr888_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_RGB565 & TO_RGB888) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGB888:
		return aipl_color_convert_rgb565_to_rgb888_default(input, output, pitch, width,
								   height);
#endif
		/* YUV color formats */
#if (AIPL_CONVERT_RGB565 & TO_YV12) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_YV12:
		return aipl_color_convert_rgb565_to_yv12_default(input, output, pitch, width,
								 height);
#endif
#if (AIPL_CONVERT_RGB565 & TO_I420) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I420:
		return aipl_color_convert_rgb565_to_i420_default(input, output, pitch, width,
								 height);
#endif
#if (AIPL_CONVERT_RGB565 & TO_NV12) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_NV12:
		return aipl_color_convert_rgb565_to_nv12_default(input, output, pitch, width,
								 height);
#endif
#if (AIPL_CONVERT_RGB565 & TO_NV21) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_NV21:
		return aipl_color_convert_rgb565_to_nv21_default(input, output, pitch, width,
								 height);
#endif
#if (AIPL_CONVERT_RGB565 & TO_I422) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I422:
		return aipl_color_convert_rgb565_to_i422_default(input, output, pitch, width,
								 height);
#endif
#if (AIPL_CONVERT_RGB565 & TO_YUY2) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_YUY2:
		return aipl_color_convert_rgb565_to_yuy2_default(input, output, pitch, width,
								 height);
#endif
#if (AIPL_CONVERT_RGB565 & TO_UYVY) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_UYVY:
		return aipl_color_convert_rgb565_to_uyvy_default(input, output, pitch, width,
								 height);
#endif
#if (AIPL_CONVERT_RGB565 & TO_I444) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I444:
		return aipl_color_convert_rgb565_to_i444_default(input, output, pitch, width,
								 height);
#endif
#if (AIPL_CONVERT_RGB565 & TO_ALPHA8_I400) &&                                                      \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I400:
		return aipl_color_convert_rgb565_to_i400_default(input, output, pitch, width,
								 height);
#endif

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

#if (AIPL_CONVERT_RGB565 & TO_ALPHA8_I400) &&                                                      \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb565_to_alpha8_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgb565_px_t *src_ptr = input;
	uint8_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgb565_px_t *src = src_ptr + i * pitch;
		uint8_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgb565_to_yuv_y(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGB565 & TO_ARGB8888) &&                                                         \
	(!defined(AIPL_DAVE2D_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb565_to_argb8888_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgb565_px_t *src_ptr = input;
	aipl_argb8888_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgb565_px_t *src = src_ptr + i * pitch;
		aipl_argb8888_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgb565_to_argb8888(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGB565 & TO_ARGB1555)
aipl_error_t aipl_color_convert_rgb565_to_argb1555_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgb565_px_t *src_ptr = input;
	aipl_argb1555_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgb565_px_t *src = src_ptr + i * pitch;
		aipl_argb1555_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgb565_to_argb1555(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGB565 & TO_ARGB4444) &&                                                         \
	(!(defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD)) ||                \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb565_to_argb4444_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgb565_px_t *src_ptr = input;
	aipl_argb4444_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgb565_px_t *src = src_ptr + i * pitch;
		aipl_argb4444_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgb565_to_argb4444(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGB565 & TO_RGBA8888) &&                                                         \
	(!defined(AIPL_DAVE2D_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb565_to_rgba8888_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgb565_px_t *src_ptr = input;
	aipl_rgba8888_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgb565_px_t *src = src_ptr + i * pitch;
		aipl_rgba8888_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgb565_to_rgba8888(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGB565 & TO_RGBA4444) &&                                                         \
	(!(defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD)) ||                \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb565_to_rgba4444_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgb565_px_t *src_ptr = input;
	aipl_rgba4444_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgb565_px_t *src = src_ptr + i * pitch;
		aipl_rgba4444_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgb565_to_rgba4444(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGB565 & TO_RGBA5551)
aipl_error_t aipl_color_convert_rgb565_to_rgba5551_default(const void *input, void *output,
							   uint32_t pitch, uint32_t width,
							   uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgb565_px_t *src_ptr = input;
	aipl_rgba5551_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgb565_px_t *src = src_ptr + i * pitch;
		aipl_rgba5551_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgb565_to_rgba5551(dst, src);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGB565 & TO_BGR888) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb565_to_bgr888_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	return aipl_color_convert_rgb565_to_24bit_default(input, output, pitch, width, height, 2, 1,
							  0);
}
#endif

#if (AIPL_CONVERT_RGB565 & TO_RGB888) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb565_to_rgb888_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	return aipl_color_convert_rgb565_to_24bit_default(input, output, pitch, width, height, 0, 1,
							  2);
}
#endif

#if (AIPL_CONVERT_RGB565 & TO_YV12) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb565_to_yv12_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *v_ptr = output + yuv_size;
	uint8_t *u_ptr = v_ptr + yuv_size / 4;

	return aipl_color_convert_rgb565_to_yuv_planar_default(input, pitch, width, height, y_ptr,
							       u_ptr, v_ptr);
}
#endif

#if (AIPL_CONVERT_RGB565 & TO_I420) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb565_to_i420_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + yuv_size;
	uint8_t *v_ptr = u_ptr + yuv_size / 4;

	return aipl_color_convert_rgb565_to_yuv_planar_default(input, pitch, width, height, y_ptr,
							       u_ptr, v_ptr);
}
#endif

#if (AIPL_CONVERT_RGB565 & TO_I422) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb565_to_i422_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + yuv_size;
	uint8_t *v_ptr = u_ptr + yuv_size / 2;

	const aipl_rgb565_px_t *src_ptr = input;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgb565_px_t *src = src_ptr + i * pitch;
		uint8_t *y_dst = y_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgb565_to_yuv_y(y_dst, src);

			++src;
			++y_dst;
		}

		src = src_ptr + i * pitch;
		uint8_t *u_dst = u_ptr + i * width / 2;
		uint8_t *v_dst = v_ptr + i * width / 2;

		for (uint32_t j = 0; j < width / 2; ++j) {
			aipl_cnvt_px_rgb565_to_yuv_uv(u_dst, v_dst, src);

			src += 2;
			++u_dst;
			++v_dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGB565 & TO_I444) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb565_to_i444_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + yuv_size;
	uint8_t *v_ptr = u_ptr + yuv_size;

	const aipl_rgb565_px_t *src_ptr = input;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgb565_px_t *src = src_ptr + i * pitch;

		uint8_t *y_dst = y_ptr + i * width;
		uint8_t *v_dst = v_ptr + i * width;
		uint8_t *u_dst = u_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgb565_to_yuv(y_dst, u_dst, v_dst, src);

			++src;
			++y_dst;
			++u_dst;
			++v_dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGB565 & TO_ALPHA8_I400) &&                                                      \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb565_to_i400_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	return aipl_color_convert_rgb565_to_alpha8_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_RGB565 & TO_NV21) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb565_to_nv21_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *v_ptr = output + yuv_size;
	uint8_t *u_ptr = v_ptr + 1;

	return aipl_color_convert_rgb565_to_yuv_semi_planar_default(input, pitch, width, height,
								    y_ptr, u_ptr, v_ptr);
}
#endif

#if (AIPL_CONVERT_RGB565 & TO_NV12) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb565_to_nv12_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	uint32_t yuv_size = width * height;
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + yuv_size;
	uint8_t *v_ptr = u_ptr + 1;

	return aipl_color_convert_rgb565_to_yuv_semi_planar_default(input, pitch, width, height,
								    y_ptr, u_ptr, v_ptr);
}
#endif

#if (AIPL_CONVERT_RGB565 & TO_YUY2) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb565_to_yuy2_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	uint8_t *y_ptr = output;
	uint8_t *u_ptr = output + 1;
	uint8_t *v_ptr = u_ptr + 2;

	return aipl_color_convert_rgb565_to_yuv_packed_default(input, pitch, width, height, y_ptr,
							       u_ptr, v_ptr);
}
#endif

#if (AIPL_CONVERT_RGB565 & TO_UYVY) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb565_to_uyvy_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	uint8_t *y_ptr = output + 1;
	uint8_t *u_ptr = output;
	uint8_t *v_ptr = u_ptr + 2;

	return aipl_color_convert_rgb565_to_yuv_packed_default(input, pitch, width, height, y_ptr,
							       u_ptr, v_ptr);
}
#endif
#endif

#if AIPL_CONVERT_YV12
aipl_error_t aipl_color_convert_yv12_default(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height,
					     aipl_color_format_t format)
{
	switch (format) {
		/* Alpha color formats */
#if (AIPL_CONVERT_YV12 & TO_ALPHA8_I400)
	case AIPL_COLOR_ALPHA8:
		return aipl_color_convert_yv12_to_alpha8_default(input, output, pitch, width,
								 height);
#endif
		/* RGB color formats */
#if (AIPL_CONVERT_YV12 & TO_ARGB8888) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB8888:
		return aipl_color_convert_yv12_to_argb8888_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_YV12 & TO_RGBA8888) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA8888:
		return aipl_color_convert_yv12_to_rgba8888_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_YV12 & TO_ARGB4444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB4444:
		return aipl_color_convert_yv12_to_argb4444_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_YV12 & TO_ARGB1555) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB1555:
		return aipl_color_convert_yv12_to_argb1555_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_YV12 & TO_RGBA4444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA4444:
		return aipl_color_convert_yv12_to_rgba4444_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_YV12 & TO_RGBA5551) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA5551:
		return aipl_color_convert_yv12_to_rgba5551_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_YV12 & TO_RGB565) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGB565:
		return aipl_color_convert_yv12_to_rgb565_default(input, output, pitch, width,
								 height);
#endif
#if (AIPL_CONVERT_YV12 & TO_BGR888) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_BGR888:
		return aipl_color_convert_yv12_to_bgr888_default(input, output, pitch, width,
								 height);
#endif
#if (AIPL_CONVERT_YV12 & TO_RGB888) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGB888:
		return aipl_color_convert_yv12_to_rgb888_default(input, output, pitch, width,
								 height);
#endif
	/* YUV color formats */
	case AIPL_COLOR_YV12:
		return AIPL_ERR_FORMAT_MISMATCH;
#if (AIPL_CONVERT_YV12 & TO_I420)
	case AIPL_COLOR_I420:
		return aipl_color_convert_yv12_to_i420_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_YV12 & TO_NV12) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_NV12:
		return aipl_color_convert_yv12_to_nv12_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_YV12 & TO_NV21) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_NV21:
		return aipl_color_convert_yv12_to_nv21_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_YV12 & TO_I422)
	case AIPL_COLOR_I422:
		return aipl_color_convert_yv12_to_i422_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_YV12 & TO_YUY2)
	case AIPL_COLOR_YUY2:
		return aipl_color_convert_yv12_to_yuy2_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_YV12 & TO_UYVY)
	case AIPL_COLOR_UYVY:
		return aipl_color_convert_yv12_to_uyvy_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_YV12 & TO_I444) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I444:
		return aipl_color_convert_yv12_to_i444_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_YV12 & TO_ALPHA8_I400)
	case AIPL_COLOR_I400:
		return aipl_color_convert_yv12_to_i400_default(input, output, pitch, width, height);
#endif

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

#if (AIPL_CONVERT_YV12 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_yv12_to_alpha8_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	return aipl_color_convert_yuv_planar_to_alpha8_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_YV12 & TO_ARGB8888) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yv12_to_argb8888_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *v_ptr = y_ptr + yuv_size;
	const uint8_t *u_ptr = v_ptr + yuv_size / 4;

	return aipl_color_convert_yuv_planar_to_argb8888_default(y_ptr, u_ptr, v_ptr, output, pitch,
								 width, height);
}
#endif

#if (AIPL_CONVERT_YV12 & TO_ARGB4444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yv12_to_argb4444_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *v_ptr = y_ptr + yuv_size;
	const uint8_t *u_ptr = v_ptr + yuv_size / 4;

	return aipl_color_convert_yuv_planar_to_argb4444_default(y_ptr, u_ptr, v_ptr, output, pitch,
								 width, height);
}
#endif

#if (AIPL_CONVERT_YV12 & TO_ARGB1555) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yv12_to_argb1555_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *v_ptr = y_ptr + yuv_size;
	const uint8_t *u_ptr = v_ptr + yuv_size / 4;

	return aipl_color_convert_yuv_planar_to_argb1555_default(y_ptr, u_ptr, v_ptr, output, pitch,
								 width, height);
}
#endif

#if (AIPL_CONVERT_YV12 & TO_RGBA8888) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yv12_to_rgba8888_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *v_ptr = y_ptr + yuv_size;
	const uint8_t *u_ptr = v_ptr + yuv_size / 4;

	return aipl_color_convert_yuv_planar_to_rgba8888_default(y_ptr, u_ptr, v_ptr, output, pitch,
								 width, height);
}
#endif

#if (AIPL_CONVERT_YV12 & TO_RGBA4444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yv12_to_rgba4444_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *v_ptr = y_ptr + yuv_size;
	const uint8_t *u_ptr = v_ptr + yuv_size / 4;

	return aipl_color_convert_yuv_planar_to_rgba4444_default(y_ptr, u_ptr, v_ptr, output, pitch,
								 width, height);
}
#endif

#if (AIPL_CONVERT_YV12 & TO_RGBA5551) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yv12_to_rgba5551_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *v_ptr = y_ptr + yuv_size;
	const uint8_t *u_ptr = v_ptr + yuv_size / 4;

	return aipl_color_convert_yuv_planar_to_rgba5551_default(y_ptr, u_ptr, v_ptr, output, pitch,
								 width, height);
}
#endif

#if (AIPL_CONVERT_YV12 & TO_BGR888) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yv12_to_bgr888_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *v_ptr = y_ptr + yuv_size;
	const uint8_t *u_ptr = v_ptr + yuv_size / 4;

	return aipl_color_convert_yuv_planar_to_24bit_default(y_ptr, u_ptr, v_ptr, output, pitch,
							      width, height, 2, 1, 0);
}
#endif

#if (AIPL_CONVERT_YV12 & TO_RGB888) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yv12_to_rgb888_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *v_ptr = y_ptr + yuv_size;
	const uint8_t *u_ptr = v_ptr + yuv_size / 4;

	return aipl_color_convert_yuv_planar_to_24bit_default(y_ptr, u_ptr, v_ptr, output, pitch,
							      width, height, 0, 1, 2);
}
#endif

#if (AIPL_CONVERT_YV12 & TO_RGB565) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yv12_to_rgb565_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *v_ptr = y_ptr + yuv_size;
	const uint8_t *u_ptr = v_ptr + yuv_size / 4;

	return aipl_color_convert_yuv_planar_to_rgb565_default(y_ptr, u_ptr, v_ptr, output, pitch,
							       width, height);
}
#endif

#if (AIPL_CONVERT_YV12 & TO_I420)
aipl_error_t aipl_color_convert_yv12_to_i420_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_src = input;
	const uint8_t *v_src = y_src + yuv_size;
	const uint8_t *u_src = v_src + yuv_size / 4;

	yuv_size = width * height;
	uint8_t *y_dst = output;
	uint8_t *u_dst = y_dst + yuv_size;
	uint8_t *v_dst = u_dst + yuv_size / 4;

	return aipl_color_convert_yuv_planar_to_planar_default(y_src, u_src, v_src, y_dst, u_dst,
							       v_dst, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_YV12 & TO_I422)
aipl_error_t aipl_color_convert_yv12_to_i422_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = pitch * height;
	const uint8_t *y_src_ptr = input;
	const uint8_t *v_src_ptr = y_src_ptr + yuv_size;
	const uint8_t *u_src_ptr = v_src_ptr + yuv_size / 4;

	yuv_size = width * height;
	uint8_t *y_dst_ptr = output;
	uint8_t *u_dst_ptr = y_dst_ptr + yuv_size;
	uint8_t *v_dst_ptr = u_dst_ptr + yuv_size / 2;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_src_ptr + i * pitch;
		uint8_t *y_dst = y_dst_ptr + i * width;
		const uint8_t *u_src = u_src_ptr + i / 2 * pitch / 2;
		uint8_t *u_dst = u_dst_ptr + i * width / 2;
		const uint8_t *v_src = v_src_ptr + i / 2 * pitch / 2;
		uint8_t *v_dst = v_dst_ptr + i * width / 2;

		memcpy(y_dst, y_src, width);
		memcpy(u_dst, u_src, width / 2);
		memcpy(v_dst, v_src, width / 2);
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_YV12 & TO_I444) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yv12_to_i444_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = pitch * height;
	const uint8_t *y_src_ptr = input;
	const uint8_t *v_src_ptr = y_src_ptr + yuv_size;
	const uint8_t *u_src_ptr = v_src_ptr + yuv_size / 4;

	yuv_size = width * height;
	uint8_t *y_dst_ptr = output;
	uint8_t *u_dst_ptr = y_dst_ptr + yuv_size;
	uint8_t *v_dst_ptr = u_dst_ptr + yuv_size;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_src_ptr + i * pitch;
		uint8_t *y_dst = y_dst_ptr + i * width;
		const uint8_t *u_src = u_src_ptr + i / 2 * pitch / 2;
		uint8_t *u_dst = u_dst_ptr + i * width;
		const uint8_t *v_src = v_src_ptr + i / 2 * pitch / 2;
		uint8_t *v_dst = v_dst_ptr + i * width;

		memcpy(y_dst, y_src, width);

		if (i & 1) {
			memcpy(u_dst, u_dst - width, width);
			memcpy(v_dst, v_dst - width, width);
		} else {
			for (uint32_t j = 0; j < width; ++j) {
				u_dst[j] = u_src[j / 2];
				v_dst[j] = v_src[j / 2];
			}
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_YV12 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_yv12_to_i400_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	return aipl_color_convert_yuv_planar_to_alpha8_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_YV12 & TO_NV21) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yv12_to_nv21_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_src = input;
	const uint8_t *v_src = y_src + yuv_size;
	const uint8_t *u_src = v_src + yuv_size / 4;

	yuv_size = width * height;
	uint8_t *y_dst = output;
	uint8_t *v_dst = y_dst + yuv_size;
	uint8_t *u_dst = v_dst + 1;

	return aipl_color_convert_yuv_planar_to_semi_default(y_src, u_src, v_src, y_dst, u_dst,
							     v_dst, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_YV12 & TO_NV12) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yv12_to_nv12_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_src = input;
	const uint8_t *v_src = y_src + yuv_size;
	const uint8_t *u_src = v_src + yuv_size / 4;

	yuv_size = width * height;
	uint8_t *y_dst = output;
	uint8_t *u_dst = y_dst + yuv_size;
	uint8_t *v_dst = u_dst + 1;

	return aipl_color_convert_yuv_planar_to_semi_default(y_src, u_src, v_src, y_dst, u_dst,
							     v_dst, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_YV12 & TO_YUY2)
aipl_error_t aipl_color_convert_yv12_to_yuy2_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_src = input;
	const uint8_t *v_src = y_src + yuv_size;
	const uint8_t *u_src = v_src + yuv_size / 4;

	uint8_t *y_dst = output;
	uint8_t *u_dst = y_dst + 1;
	uint8_t *v_dst = u_dst + 2;

	return aipl_color_convert_yuv_planar_to_packed_default(y_src, u_src, v_src, y_dst, u_dst,
							       v_dst, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_YV12 & TO_UYVY)
aipl_error_t aipl_color_convert_yv12_to_uyvy_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_src = input;
	const uint8_t *v_src = y_src + yuv_size;
	const uint8_t *u_src = v_src + yuv_size / 4;

	uint8_t *u_dst = output;
	uint8_t *y_dst = u_dst + 1;
	uint8_t *v_dst = u_dst + 2;

	return aipl_color_convert_yuv_planar_to_packed_default(y_src, u_src, v_src, y_dst, u_dst,
							       v_dst, pitch, width, height);
}
#endif
#endif

#if AIPL_CONVERT_I420
aipl_error_t aipl_color_convert_i420_default(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height,
					     aipl_color_format_t format)
{
	switch (format) {
		/* Alpha color formats */
#if (AIPL_CONVERT_I420 & TO_ALPHA8_I400)
	case AIPL_COLOR_ALPHA8:
		return aipl_color_convert_i420_to_alpha8_default(input, output, pitch, width,
								 height);
#endif
		/* RGB color formats */
#if (AIPL_CONVERT_I420 & TO_ARGB8888) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB8888:
		return aipl_color_convert_i420_to_argb8888_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_I420 & TO_RGBA8888) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA8888:
		return aipl_color_convert_i420_to_rgba8888_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_I420 & TO_ARGB4444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB4444:
		return aipl_color_convert_i420_to_argb4444_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_I420 & TO_ARGB1555) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB1555:
		return aipl_color_convert_i420_to_argb1555_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_I420 & TO_RGBA4444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA4444:
		return aipl_color_convert_i420_to_rgba4444_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_I420 & TO_RGBA5551) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA5551:
		return aipl_color_convert_i420_to_rgba5551_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_I420 & TO_RGB565) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGB565:
		return aipl_color_convert_i420_to_rgb565_default(input, output, pitch, width,
								 height);
#endif
#if (AIPL_CONVERT_I420 & TO_BGR888) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_BGR888:
		return aipl_color_convert_i420_to_bgr888_default(input, output, pitch, width,
								 height);
#endif
#if (AIPL_CONVERT_I420 & TO_RGB888) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGB888:
		return aipl_color_convert_i420_to_rgb888_default(input, output, pitch, width,
								 height);
#endif
		/* YUV color formats */
#if (AIPL_CONVERT_I420 & TO_YV12)
	case AIPL_COLOR_YV12:
		return aipl_color_convert_i420_to_yv12_default(input, output, pitch, width, height);
#endif
	case AIPL_COLOR_I420:
		return AIPL_ERR_FORMAT_MISMATCH;
#if (AIPL_CONVERT_I420 & TO_NV12) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_NV12:
		return aipl_color_convert_i420_to_nv12_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I420 & TO_NV21) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_NV21:
		return aipl_color_convert_i420_to_nv21_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I420 & TO_I422)
	case AIPL_COLOR_I422:
		return aipl_color_convert_i420_to_i422_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I420 & TO_YUY2)
	case AIPL_COLOR_YUY2:
		return aipl_color_convert_i420_to_yuy2_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I420 & TO_UYVY)
	case AIPL_COLOR_UYVY:
		return aipl_color_convert_i420_to_uyvy_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I420 & TO_I444) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I444:
		return aipl_color_convert_i420_to_i444_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I420 & TO_ALPHA8_I400)
	case AIPL_COLOR_I400:
		return aipl_color_convert_i420_to_i400_default(input, output, pitch, width, height);
#endif

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

#if (AIPL_CONVERT_I420 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_i420_to_alpha8_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	return aipl_color_convert_yuv_planar_to_alpha8_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_I420 & TO_ARGB8888) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i420_to_argb8888_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *u_ptr = y_ptr + yuv_size;
	const uint8_t *v_ptr = u_ptr + yuv_size / 4;

	return aipl_color_convert_yuv_planar_to_argb8888_default(y_ptr, u_ptr, v_ptr, output, pitch,
								 width, height);
}
#endif

#if (AIPL_CONVERT_I420 & TO_ARGB4444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i420_to_argb4444_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *u_ptr = y_ptr + yuv_size;
	const uint8_t *v_ptr = u_ptr + yuv_size / 4;

	return aipl_color_convert_yuv_planar_to_argb4444_default(y_ptr, u_ptr, v_ptr, output, pitch,
								 width, height);
}
#endif

#if (AIPL_CONVERT_I420 & TO_ARGB1555) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i420_to_argb1555_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *u_ptr = y_ptr + yuv_size;
	const uint8_t *v_ptr = u_ptr + yuv_size / 4;

	return aipl_color_convert_yuv_planar_to_argb1555_default(y_ptr, u_ptr, v_ptr, output, pitch,
								 width, height);
}
#endif

#if (AIPL_CONVERT_I420 & TO_RGBA8888) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i420_to_rgba8888_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *u_ptr = y_ptr + yuv_size;
	const uint8_t *v_ptr = u_ptr + yuv_size / 4;

	return aipl_color_convert_yuv_planar_to_rgba8888_default(y_ptr, u_ptr, v_ptr, output, pitch,
								 width, height);
}
#endif

#if (AIPL_CONVERT_I420 & TO_RGBA4444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i420_to_rgba4444_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *u_ptr = y_ptr + yuv_size;
	const uint8_t *v_ptr = u_ptr + yuv_size / 4;

	return aipl_color_convert_yuv_planar_to_rgba4444_default(y_ptr, u_ptr, v_ptr, output, pitch,
								 width, height);
}
#endif

#if (AIPL_CONVERT_I420 & TO_RGBA5551) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i420_to_rgba5551_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *u_ptr = y_ptr + yuv_size;
	const uint8_t *v_ptr = u_ptr + yuv_size / 4;

	return aipl_color_convert_yuv_planar_to_rgba5551_default(y_ptr, u_ptr, v_ptr, output, pitch,
								 width, height);
}
#endif

#if (AIPL_CONVERT_I420 & TO_BGR888) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i420_to_bgr888_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *u_ptr = y_ptr + yuv_size;
	const uint8_t *v_ptr = u_ptr + yuv_size / 4;

	return aipl_color_convert_yuv_planar_to_24bit_default(y_ptr, u_ptr, v_ptr, output, pitch,
							      width, height, 2, 1, 0);
}
#endif

#if (AIPL_CONVERT_I420 & TO_RGB888) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i420_to_rgb888_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *u_ptr = y_ptr + yuv_size;
	const uint8_t *v_ptr = u_ptr + yuv_size / 4;

	return aipl_color_convert_yuv_planar_to_24bit_default(y_ptr, u_ptr, v_ptr, output, pitch,
							      width, height, 0, 1, 2);
}
#endif

#if (AIPL_CONVERT_I420 & TO_RGB565) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i420_to_rgb565_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *u_ptr = y_ptr + yuv_size;
	const uint8_t *v_ptr = u_ptr + yuv_size / 4;

	return aipl_color_convert_yuv_planar_to_rgb565_default(y_ptr, u_ptr, v_ptr, output, pitch,
							       width, height);
}
#endif

#if (AIPL_CONVERT_I420 & TO_YV12)
aipl_error_t aipl_color_convert_i420_to_yv12_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_src = input;
	const uint8_t *u_src = y_src + yuv_size;
	const uint8_t *v_src = u_src + yuv_size / 4;

	yuv_size = width * height;
	uint8_t *y_dst = output;
	uint8_t *v_dst = y_dst + yuv_size;
	uint8_t *u_dst = v_dst + yuv_size / 4;

	return aipl_color_convert_yuv_planar_to_planar_default(y_src, u_src, v_src, y_dst, u_dst,
							       v_dst, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_I420 & TO_I422)
aipl_error_t aipl_color_convert_i420_to_i422_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = pitch * height;
	const uint8_t *y_src_ptr = input;
	const uint8_t *u_src_ptr = y_src_ptr + yuv_size;
	const uint8_t *v_src_ptr = u_src_ptr + yuv_size / 4;

	yuv_size = width * height;
	uint8_t *y_dst_ptr = output;
	uint8_t *u_dst_ptr = y_dst_ptr + yuv_size;
	uint8_t *v_dst_ptr = u_dst_ptr + yuv_size / 2;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_src_ptr + i * pitch;
		uint8_t *y_dst = y_dst_ptr + i * width;
		const uint8_t *u_src = u_src_ptr + i / 2 * pitch / 2;
		uint8_t *u_dst = u_dst_ptr + i * width / 2;
		const uint8_t *v_src = v_src_ptr + i / 2 * pitch / 2;
		uint8_t *v_dst = v_dst_ptr + i * width / 2;

		memcpy(y_dst, y_src, width);
		memcpy(u_dst, u_src, width / 2);
		memcpy(v_dst, v_src, width / 2);
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_I420 & TO_I444) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i420_to_i444_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = pitch * height;
	const uint8_t *y_src_ptr = input;
	const uint8_t *u_src_ptr = y_src_ptr + yuv_size;
	const uint8_t *v_src_ptr = u_src_ptr + yuv_size / 4;

	yuv_size = width * height;
	uint8_t *y_dst_ptr = output;
	uint8_t *u_dst_ptr = y_dst_ptr + yuv_size;
	uint8_t *v_dst_ptr = u_dst_ptr + yuv_size;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_src_ptr + i * pitch;
		uint8_t *y_dst = y_dst_ptr + i * width;
		const uint8_t *u_src = u_src_ptr + i / 2 * pitch / 2;
		uint8_t *u_dst = u_dst_ptr + i * width;
		const uint8_t *v_src = v_src_ptr + i / 2 * pitch / 2;
		uint8_t *v_dst = v_dst_ptr + i * width;

		memcpy(y_dst, y_src, width);

		if (i & 1) {
			memcpy(u_dst, u_dst - width, width);
			memcpy(v_dst, v_dst - width, width);
		} else {
			for (uint32_t j = 0; j < width; ++j) {
				u_dst[j] = u_src[j / 2];
				v_dst[j] = v_src[j / 2];
			}
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_I420 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_i420_to_i400_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	return aipl_color_convert_yuv_planar_to_alpha8_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_I420 & TO_NV21) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i420_to_nv21_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_src = input;
	const uint8_t *u_src = y_src + yuv_size;
	const uint8_t *v_src = u_src + yuv_size / 4;

	yuv_size = width * height;
	uint8_t *y_dst = output;
	uint8_t *v_dst = y_dst + yuv_size;
	uint8_t *u_dst = v_dst + 1;

	return aipl_color_convert_yuv_planar_to_semi_default(y_src, u_src, v_src, y_dst, u_dst,
							     v_dst, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_I420 & TO_NV12) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i420_to_nv12_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_src = input;
	const uint8_t *u_src = y_src + yuv_size;
	const uint8_t *v_src = u_src + yuv_size / 4;

	yuv_size = width * height;
	uint8_t *y_dst = output;
	uint8_t *u_dst = y_dst + yuv_size;
	uint8_t *v_dst = u_dst + 1;

	return aipl_color_convert_yuv_planar_to_semi_default(y_src, u_src, v_src, y_dst, u_dst,
							     v_dst, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_I420 & TO_YUY2)
aipl_error_t aipl_color_convert_i420_to_yuy2_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_src = input;
	const uint8_t *u_src = y_src + yuv_size;
	const uint8_t *v_src = u_src + yuv_size / 4;

	uint8_t *y_dst = output;
	uint8_t *u_dst = y_dst + 1;
	uint8_t *v_dst = u_dst + 2;

	return aipl_color_convert_yuv_planar_to_packed_default(y_src, u_src, v_src, y_dst, u_dst,
							       v_dst, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_I420 & TO_UYVY)
aipl_error_t aipl_color_convert_i420_to_uyvy_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_src = input;
	const uint8_t *u_src = y_src + yuv_size;
	const uint8_t *v_src = u_src + yuv_size / 4;

	uint8_t *u_dst = output;
	uint8_t *y_dst = u_dst + 1;
	uint8_t *v_dst = u_dst + 2;

	return aipl_color_convert_yuv_planar_to_packed_default(y_src, u_src, v_src, y_dst, u_dst,
							       v_dst, pitch, width, height);
}
#endif
#endif

#if AIPL_CONVERT_I422
aipl_error_t aipl_color_convert_i422_default(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height,
					     aipl_color_format_t format)
{
	switch (format) {
		/* Alpha color formats */
#if (AIPL_CONVERT_I422 & TO_ALPHA8_I400)
	case AIPL_COLOR_ALPHA8:
		return aipl_color_convert_i422_to_alpha8_default(input, output, pitch, width,
								 height);
#endif
		/* RGB color formats */
#if (AIPL_CONVERT_I422 & TO_ARGB8888) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB8888:
		return aipl_color_convert_i422_to_argb8888_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_I422 & TO_RGBA8888) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA8888:
		return aipl_color_convert_i422_to_rgba8888_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_I422 & TO_ARGB4444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB4444:
		return aipl_color_convert_i422_to_argb4444_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_I422 & TO_ARGB1555) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB1555:
		return aipl_color_convert_i422_to_argb1555_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_I422 & TO_RGBA4444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA4444:
		return aipl_color_convert_i422_to_rgba4444_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_I422 & TO_RGBA5551) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA5551:
		return aipl_color_convert_i422_to_rgba5551_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_I422 & TO_RGB565) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGB565:
		return aipl_color_convert_i422_to_rgb565_default(input, output, pitch, width,
								 height);
#endif
#if (AIPL_CONVERT_I422 & TO_BGR888) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_BGR888:
		return aipl_color_convert_i422_to_bgr888_default(input, output, pitch, width,
								 height);
#endif
#if (AIPL_CONVERT_I422 & TO_RGB888) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGB888:
		return aipl_color_convert_i422_to_rgb888_default(input, output, pitch, width,
								 height);
#endif
		/* YUV color formats */
#if (AIPL_CONVERT_I422 & TO_YV12) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_YV12:
		return aipl_color_convert_i422_to_yv12_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I422 & TO_I420) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I420:
		return aipl_color_convert_i422_to_i420_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I422 & TO_NV12) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_NV12:
		return aipl_color_convert_i422_to_nv12_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I422 & TO_NV21) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_NV21:
		return aipl_color_convert_i422_to_nv21_default(input, output, pitch, width, height);
#endif
	case AIPL_COLOR_I422:
		return AIPL_ERR_FORMAT_MISMATCH;
#if (AIPL_CONVERT_I422 & TO_YUY2)
	case AIPL_COLOR_YUY2:
		return aipl_color_convert_i422_to_yuy2_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I422 & TO_UYVY)
	case AIPL_COLOR_UYVY:
		return aipl_color_convert_i422_to_uyvy_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I422 & TO_I444) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I444:
		return aipl_color_convert_i422_to_i444_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I422 & TO_ALPHA8_I400)
	case AIPL_COLOR_I400:
		return aipl_color_convert_i422_to_i400_default(input, output, pitch, width, height);
#endif

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

#if (AIPL_CONVERT_I422 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_i422_to_alpha8_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	return aipl_color_convert_yuv_planar_to_alpha8_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_I422 & TO_ARGB8888) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i422_to_argb8888_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *u_ptr = y_ptr + yuv_size;
	const uint8_t *v_ptr = u_ptr + yuv_size / 2;

	aipl_argb8888_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_ptr + i * pitch;
		const uint8_t *u_src = u_ptr + i * pitch / 2;
		const uint8_t *v_src = v_ptr + i * pitch / 2;
		aipl_argb8888_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; j += 2) {
			uint32_t j2 = j / 2;
			int32_t c0, c1;
			int32_t r, g, b;

			aipl_pre_cnvt_px_yuv_to_rgb(&r, &g, &b, u_src[j2], v_src[j2]);
			aipl_pre_cnvt_2px_y(&c0, &c1, y_src[j], y_src[j + 1]);

			aipl_cnvt_px_yuv_to_argb8888(dst + j, c0, r, g, b);
			aipl_cnvt_px_yuv_to_argb8888(dst + j + 1, c1, r, g, b);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_I422 & TO_ARGB4444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i422_to_argb4444_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *u_ptr = y_ptr + yuv_size;
	const uint8_t *v_ptr = u_ptr + yuv_size / 2;

	aipl_argb4444_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_ptr + i * pitch;
		const uint8_t *u_src = u_ptr + i * pitch / 2;
		const uint8_t *v_src = v_ptr + i * pitch / 2;
		aipl_argb4444_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; j += 2) {
			uint32_t j2 = j / 2;
			int32_t c0, c1;
			int32_t r, g, b;

			aipl_pre_cnvt_px_yuv_to_rgb(&r, &g, &b, u_src[j2], v_src[j2]);
			aipl_pre_cnvt_2px_y(&c0, &c1, y_src[j], y_src[j + 1]);

			aipl_cnvt_px_yuv_to_argb4444(dst + j, c0, r, g, b);
			aipl_cnvt_px_yuv_to_argb4444(dst + j + 1, c1, r, g, b);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_I422 & TO_ARGB1555) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i422_to_argb1555_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *u_ptr = y_ptr + yuv_size;
	const uint8_t *v_ptr = u_ptr + yuv_size / 2;

	aipl_argb1555_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_ptr + i * pitch;
		const uint8_t *u_src = u_ptr + i * pitch / 2;
		const uint8_t *v_src = v_ptr + i * pitch / 2;
		aipl_argb1555_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; j += 2) {
			uint32_t j2 = j / 2;
			int32_t c0, c1;
			int32_t r, g, b;

			aipl_pre_cnvt_px_yuv_to_rgb(&r, &g, &b, u_src[j2], v_src[j2]);
			aipl_pre_cnvt_2px_y(&c0, &c1, y_src[j], y_src[j + 1]);

			aipl_cnvt_px_yuv_to_argb1555(dst + j, c0, r, g, b);
			aipl_cnvt_px_yuv_to_argb1555(dst + j + 1, c1, r, g, b);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_I422 & TO_RGBA8888) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i422_to_rgba8888_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *u_ptr = y_ptr + yuv_size;
	const uint8_t *v_ptr = u_ptr + yuv_size / 2;

	aipl_rgba8888_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_ptr + i * pitch;
		const uint8_t *u_src = u_ptr + i * pitch / 2;
		const uint8_t *v_src = v_ptr + i * pitch / 2;
		aipl_rgba8888_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; j += 2) {
			uint32_t j2 = j / 2;
			int32_t c0, c1;
			int32_t r, g, b;

			aipl_pre_cnvt_px_yuv_to_rgb(&r, &g, &b, u_src[j2], v_src[j2]);
			aipl_pre_cnvt_2px_y(&c0, &c1, y_src[j], y_src[j + 1]);

			aipl_cnvt_px_yuv_to_rgba8888(dst + j, c0, r, g, b);
			aipl_cnvt_px_yuv_to_rgba8888(dst + j + 1, c1, r, g, b);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_I422 & TO_RGBA4444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i422_to_rgba4444_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *u_ptr = y_ptr + yuv_size;
	const uint8_t *v_ptr = u_ptr + yuv_size / 2;

	aipl_rgba4444_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_ptr + i * pitch;
		const uint8_t *u_src = u_ptr + i * pitch / 2;
		const uint8_t *v_src = v_ptr + i * pitch / 2;
		aipl_rgba4444_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; j += 2) {
			uint32_t j2 = j / 2;
			int32_t c0, c1;
			int32_t r, g, b;

			aipl_pre_cnvt_px_yuv_to_rgb(&r, &g, &b, u_src[j2], v_src[j2]);
			aipl_pre_cnvt_2px_y(&c0, &c1, y_src[j], y_src[j + 1]);

			aipl_cnvt_px_yuv_to_rgba4444(dst + j, c0, r, g, b);
			aipl_cnvt_px_yuv_to_rgba4444(dst + j + 1, c1, r, g, b);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_I422 & TO_RGBA5551) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i422_to_rgba5551_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *u_ptr = y_ptr + yuv_size;
	const uint8_t *v_ptr = u_ptr + yuv_size / 2;

	aipl_rgba5551_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_ptr + i * pitch;
		const uint8_t *u_src = u_ptr + i * pitch / 2;
		const uint8_t *v_src = v_ptr + i * pitch / 2;
		aipl_rgba5551_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; j += 2) {
			uint32_t j2 = j / 2;
			int32_t c0, c1;
			int32_t r, g, b;

			aipl_pre_cnvt_px_yuv_to_rgb(&r, &g, &b, u_src[j2], v_src[j2]);
			aipl_pre_cnvt_2px_y(&c0, &c1, y_src[j], y_src[j + 1]);

			aipl_cnvt_px_yuv_to_rgba5551(dst + j, c0, r, g, b);
			aipl_cnvt_px_yuv_to_rgba5551(dst + j + 1, c1, r, g, b);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_I422 & TO_BGR888) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i422_to_bgr888_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	return aipl_color_convert_i422_to_24bit_default(input, output, pitch, width, height, 2, 1,
							0);
}
#endif

#if (AIPL_CONVERT_I422 & TO_RGB888) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i422_to_rgb888_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	return aipl_color_convert_i422_to_24bit_default(input, output, pitch, width, height, 0, 1,
							2);
}
#endif

#if (AIPL_CONVERT_I422 & TO_RGB565) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i422_to_rgb565_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *u_ptr = y_ptr + yuv_size;
	const uint8_t *v_ptr = u_ptr + yuv_size / 2;

	aipl_rgb565_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_ptr + i * pitch;
		const uint8_t *u_src = u_ptr + i * pitch / 2;
		const uint8_t *v_src = v_ptr + i * pitch / 2;
		aipl_rgb565_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; j += 2) {
			uint32_t j2 = j / 2;
			int32_t c0, c1;
			int32_t r, g, b;

			aipl_pre_cnvt_px_yuv_to_rgb(&r, &g, &b, u_src[j2], v_src[j2]);
			aipl_pre_cnvt_2px_y(&c0, &c1, y_src[j], y_src[j + 1]);

			aipl_cnvt_px_yuv_to_rgb565(dst + j, c0, r, g, b);
			aipl_cnvt_px_yuv_to_rgb565(dst + j + 1, c1, r, g, b);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_I422 & TO_YV12) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i422_to_yv12_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = pitch * height;
	const uint8_t *y_src_ptr = input;
	const uint8_t *u_src_ptr = y_src_ptr + yuv_size;
	const uint8_t *v_src_ptr = u_src_ptr + yuv_size / 2;

	yuv_size = width * height;
	uint8_t *y_dst_ptr = output;
	uint8_t *v_dst_ptr = y_dst_ptr + yuv_size;
	uint8_t *u_dst_ptr = v_dst_ptr + yuv_size / 4;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_src_ptr + i * pitch;
		uint8_t *y_dst = y_dst_ptr + i * width;

		memcpy(y_dst, y_src, width);

		if (!(i & 1)) {
			const uint8_t *u_src = u_src_ptr + i * pitch / 2;
			uint8_t *u_dst = u_dst_ptr + i * width / 4;
			const uint8_t *v_src = v_src_ptr + i * pitch / 2;
			uint8_t *v_dst = v_dst_ptr + i * width / 4;

			memcpy(u_dst, u_src, width / 2);
			memcpy(v_dst, v_src, width / 2);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_I422 & TO_I420) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i422_to_i420_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = pitch * height;
	const uint8_t *y_src_ptr = input;
	const uint8_t *u_src_ptr = y_src_ptr + yuv_size;
	const uint8_t *v_src_ptr = u_src_ptr + yuv_size / 2;

	yuv_size = width * height;
	uint8_t *y_dst_ptr = output;
	uint8_t *u_dst_ptr = y_dst_ptr + yuv_size;
	uint8_t *v_dst_ptr = u_dst_ptr + yuv_size / 4;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_src_ptr + i * pitch;
		uint8_t *y_dst = y_dst_ptr + i * width;

		memcpy(y_dst, y_src, width);

		if (!(i & 1)) {
			const uint8_t *u_src = u_src_ptr + i * pitch / 2;
			uint8_t *u_dst = u_dst_ptr + i * width / 4;
			const uint8_t *v_src = v_src_ptr + i * pitch / 2;
			uint8_t *v_dst = v_dst_ptr + i * width / 4;

			memcpy(u_dst, u_src, width / 2);
			memcpy(v_dst, v_src, width / 2);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_I422 & TO_I444) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i422_to_i444_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = pitch * height;
	const uint8_t *y_src_ptr = input;
	const uint8_t *u_src_ptr = y_src_ptr + yuv_size;
	const uint8_t *v_src_ptr = u_src_ptr + yuv_size / 2;

	yuv_size = width * height;
	uint8_t *y_dst_ptr = output;
	uint8_t *u_dst_ptr = y_dst_ptr + yuv_size;
	uint8_t *v_dst_ptr = u_dst_ptr + yuv_size;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_src_ptr + i * pitch;
		uint8_t *y_dst = y_dst_ptr + i * width;
		const uint8_t *u_src = u_src_ptr + i * pitch / 2;
		uint8_t *u_dst = u_dst_ptr + i * width;
		const uint8_t *v_src = v_src_ptr + i * pitch / 2;
		uint8_t *v_dst = v_dst_ptr + i * width;

		memcpy(y_dst, y_src, width);

		for (uint32_t j = 0; j < width; ++j) {
			u_dst[j] = u_src[j / 2];
			v_dst[j] = v_src[j / 2];
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_I422 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_i422_to_i400_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	return aipl_color_convert_yuv_planar_to_alpha8_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_I422 & TO_NV21) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i422_to_nv21_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = pitch * height;
	const uint8_t *y_src_ptr = input;
	const uint8_t *u_src_ptr = y_src_ptr + yuv_size;
	const uint8_t *v_src_ptr = u_src_ptr + yuv_size / 2;

	yuv_size = width * height;
	uint8_t *y_dst_ptr = output;
	uint8_t *v_dst_ptr = y_dst_ptr + yuv_size;
	uint8_t *u_dst_ptr = v_dst_ptr + 1;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_src_ptr + i * pitch;
		uint8_t *y_dst = y_dst_ptr + i * width;

		memcpy(y_dst, y_src, width);

		if (!(i & 1)) {
			const uint8_t *u_src = u_src_ptr + i * pitch / 2;
			uint8_t *u_dst = u_dst_ptr + i / 2 * width;
			const uint8_t *v_src = v_src_ptr + i * pitch / 2;
			uint8_t *v_dst = v_dst_ptr + i / 2 * width;

			for (uint32_t j = 0; j < width; j += 2) {
				u_dst[j] = u_src[j / 2];
				v_dst[j] = v_src[j / 2];
			}
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_I422 & TO_NV12) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i422_to_nv12_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = pitch * height;
	const uint8_t *y_src_ptr = input;
	const uint8_t *u_src_ptr = y_src_ptr + yuv_size;
	const uint8_t *v_src_ptr = u_src_ptr + yuv_size / 2;

	yuv_size = width * height;
	uint8_t *y_dst_ptr = output;
	uint8_t *u_dst_ptr = y_dst_ptr + yuv_size;
	uint8_t *v_dst_ptr = u_dst_ptr + 1;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_src_ptr + i * pitch;
		uint8_t *y_dst = y_dst_ptr + i * width;

		memcpy(y_dst, y_src, width);

		if (!(i & 1)) {
			const uint8_t *u_src = u_src_ptr + i * pitch / 2;
			uint8_t *u_dst = u_dst_ptr + i / 2 * width;
			const uint8_t *v_src = v_src_ptr + i * pitch / 2;
			uint8_t *v_dst = v_dst_ptr + i / 2 * width;

			for (uint32_t j = 0; j < width; j += 2) {
				u_dst[j] = u_src[j / 2];
				v_dst[j] = v_src[j / 2];
			}
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_I422 & TO_YUY2)
aipl_error_t aipl_color_convert_i422_to_yuy2_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = pitch * height;
	const uint8_t *y_src_ptr = input;
	const uint8_t *u_src_ptr = y_src_ptr + yuv_size;
	const uint8_t *v_src_ptr = u_src_ptr + yuv_size / 2;

	uint8_t *y_dst_ptr = output;
	uint8_t *u_dst_ptr = y_dst_ptr + 1;
	uint8_t *v_dst_ptr = u_dst_ptr + 2;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_src_ptr + i * pitch;
		uint8_t *y_dst = y_dst_ptr + i * width * 2;
		const uint8_t *u_src = u_src_ptr + i * pitch / 2;
		uint8_t *u_dst = u_dst_ptr + i * width * 2;
		const uint8_t *v_src = v_src_ptr + i * pitch / 2;
		uint8_t *v_dst = v_dst_ptr + i * width * 2;

		for (uint32_t j = 0; j < width / 2; ++j) {
			y_dst[j * 4] = y_src[j * 2];
			y_dst[j * 4 + 2] = y_src[j * 2 + 1];
			u_dst[j * 4] = u_src[j];
			v_dst[j * 4] = v_src[j];
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_I422 & TO_UYVY)
aipl_error_t aipl_color_convert_i422_to_uyvy_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = pitch * height;
	const uint8_t *y_src_ptr = input;
	const uint8_t *u_src_ptr = y_src_ptr + yuv_size;
	const uint8_t *v_src_ptr = u_src_ptr + yuv_size / 2;

	uint8_t *u_dst_ptr = output;
	uint8_t *y_dst_ptr = u_dst_ptr + 1;
	uint8_t *v_dst_ptr = u_dst_ptr + 2;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_src_ptr + i * pitch;
		uint8_t *y_dst = y_dst_ptr + i * width * 2;
		const uint8_t *u_src = u_src_ptr + i * pitch / 2;
		uint8_t *u_dst = u_dst_ptr + i * width * 2;
		const uint8_t *v_src = v_src_ptr + i * pitch / 2;
		uint8_t *v_dst = v_dst_ptr + i * width * 2;

		for (uint32_t j = 0; j < width / 2; ++j) {
			y_dst[j * 4] = y_src[j * 2];
			y_dst[j * 4 + 2] = y_src[j * 2 + 1];
			u_dst[j * 4] = u_src[j];
			v_dst[j * 4] = v_src[j];
		}
	}

	return AIPL_ERR_OK;
}
#endif
#endif

#if AIPL_CONVERT_I444
aipl_error_t aipl_color_convert_i444_default(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height,
					     aipl_color_format_t format)
{
	switch (format) {
		/* Alpha color formats */
#if (AIPL_CONVERT_I444 & TO_ALPHA8_I400)
	case AIPL_COLOR_ALPHA8:
		return aipl_color_convert_i444_to_alpha8_default(input, output, pitch, width,
								 height);
#endif
		/* RGB color formats */
#if (AIPL_CONVERT_I444 & TO_ARGB8888) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB8888:
		return aipl_color_convert_i444_to_argb8888_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_I444 & TO_RGBA8888) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA8888:
		return aipl_color_convert_i444_to_rgba8888_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_I444 & TO_ARGB4444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB4444:
		return aipl_color_convert_i444_to_argb4444_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_I444 & TO_ARGB1555) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB1555:
		return aipl_color_convert_i444_to_argb1555_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_I444 & TO_RGBA4444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA4444:
		return aipl_color_convert_i444_to_rgba4444_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_I444 & TO_RGBA5551) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA5551:
		return aipl_color_convert_i444_to_rgba5551_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_I444 & TO_RGB565) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGB565:
		return aipl_color_convert_i444_to_rgb565_default(input, output, pitch, width,
								 height);
#endif
#if (AIPL_CONVERT_I444 & TO_BGR888) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_BGR888:
		return aipl_color_convert_i444_to_bgr888_default(input, output, pitch, width,
								 height);
#endif
#if (AIPL_CONVERT_I444 & TO_RGB888) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGB888:
		return aipl_color_convert_i444_to_rgb888_default(input, output, pitch, width,
								 height);
#endif
		/* YUV color formats */
#if (AIPL_CONVERT_I444 & TO_YV12) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_YV12:
		return aipl_color_convert_i444_to_yv12_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I444 & TO_I420) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I420:
		return aipl_color_convert_i444_to_i420_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I444 & TO_NV12) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_NV12:
		return aipl_color_convert_i444_to_nv12_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I444 & TO_NV21) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_NV21:
		return aipl_color_convert_i444_to_nv21_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I444 & TO_I422) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I422:
		return aipl_color_convert_i444_to_i422_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I444 & TO_YUY2)
	case AIPL_COLOR_YUY2:
		return aipl_color_convert_i444_to_yuy2_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I444 & TO_UYVY)
	case AIPL_COLOR_UYVY:
		return aipl_color_convert_i444_to_uyvy_default(input, output, pitch, width, height);
#endif
	case AIPL_COLOR_I444:
		return AIPL_ERR_FORMAT_MISMATCH;
#if (AIPL_CONVERT_I444 & TO_ALPHA8_I400)
	case AIPL_COLOR_I400:
		return aipl_color_convert_i444_to_i400_default(input, output, pitch, width, height);
#endif

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

#if (AIPL_CONVERT_I444 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_i444_to_alpha8_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	return aipl_color_convert_yuv_planar_to_alpha8_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_I444 & TO_ARGB8888) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i444_to_argb8888_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *u_ptr = y_ptr + yuv_size;
	const uint8_t *v_ptr = u_ptr + yuv_size;

	aipl_argb8888_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_ptr + i * pitch;
		const uint8_t *u_src = u_ptr + i * pitch;
		const uint8_t *v_src = v_ptr + i * pitch;
		aipl_argb8888_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			int32_t c;
			int32_t r, g, b;

			aipl_pre_cnvt_px_yuv_to_rgb(&r, &g, &b, u_src[j], v_src[j]);
			aipl_pre_cnvt_px_y(&c, y_src[j]);

			aipl_cnvt_px_yuv_to_argb8888(dst + j, c, r, g, b);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_I444 & TO_ARGB4444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i444_to_argb4444_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *u_ptr = y_ptr + yuv_size;
	const uint8_t *v_ptr = u_ptr + yuv_size;

	aipl_argb4444_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_ptr + i * pitch;
		const uint8_t *u_src = u_ptr + i * pitch;
		const uint8_t *v_src = v_ptr + i * pitch;
		aipl_argb4444_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			int32_t c;
			int32_t r, g, b;

			aipl_pre_cnvt_px_yuv_to_rgb(&r, &g, &b, u_src[j], v_src[j]);
			aipl_pre_cnvt_px_y(&c, y_src[j]);

			aipl_cnvt_px_yuv_to_argb4444(dst + j, c, r, g, b);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_I444 & TO_ARGB1555) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i444_to_argb1555_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *u_ptr = y_ptr + yuv_size;
	const uint8_t *v_ptr = u_ptr + yuv_size;

	aipl_argb1555_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_ptr + i * pitch;
		const uint8_t *u_src = u_ptr + i * pitch;
		const uint8_t *v_src = v_ptr + i * pitch;
		aipl_argb1555_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			int32_t c;
			int32_t r, g, b;

			aipl_pre_cnvt_px_yuv_to_rgb(&r, &g, &b, u_src[j], v_src[j]);
			aipl_pre_cnvt_px_y(&c, y_src[j]);

			aipl_cnvt_px_yuv_to_argb1555(dst + j, c, r, g, b);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_I444 & TO_RGBA8888) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i444_to_rgba8888_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *u_ptr = y_ptr + yuv_size;
	const uint8_t *v_ptr = u_ptr + yuv_size;

	aipl_rgba8888_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_ptr + i * pitch;
		const uint8_t *u_src = u_ptr + i * pitch;
		const uint8_t *v_src = v_ptr + i * pitch;
		aipl_rgba8888_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			int32_t c;
			int32_t r, g, b;

			aipl_pre_cnvt_px_yuv_to_rgb(&r, &g, &b, u_src[j], v_src[j]);
			aipl_pre_cnvt_px_y(&c, y_src[j]);

			aipl_cnvt_px_yuv_to_rgba8888(dst + j, c, r, g, b);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_I444 & TO_RGBA4444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i444_to_rgba4444_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *u_ptr = y_ptr + yuv_size;
	const uint8_t *v_ptr = u_ptr + yuv_size;

	aipl_rgba4444_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_ptr + i * pitch;
		const uint8_t *u_src = u_ptr + i * pitch;
		const uint8_t *v_src = v_ptr + i * pitch;
		aipl_rgba4444_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			int32_t c;
			int32_t r, g, b;

			aipl_pre_cnvt_px_yuv_to_rgb(&r, &g, &b, u_src[j], v_src[j]);
			aipl_pre_cnvt_px_y(&c, y_src[j]);

			aipl_cnvt_px_yuv_to_rgba4444(dst + j, c, r, g, b);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_I444 & TO_RGBA5551) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i444_to_rgba5551_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *u_ptr = y_ptr + yuv_size;
	const uint8_t *v_ptr = u_ptr + yuv_size;

	aipl_rgba5551_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_ptr + i * pitch;
		const uint8_t *u_src = u_ptr + i * pitch;
		const uint8_t *v_src = v_ptr + i * pitch;
		aipl_rgba5551_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			int32_t c;
			int32_t r, g, b;

			aipl_pre_cnvt_px_yuv_to_rgb(&r, &g, &b, u_src[j], v_src[j]);
			aipl_pre_cnvt_px_y(&c, y_src[j]);

			aipl_cnvt_px_yuv_to_rgba5551(dst + j, c, r, g, b);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_I444 & TO_BGR888) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i444_to_bgr888_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	return aipl_color_convert_i444_to_24bit_default(input, output, pitch, width, height, 2, 1,
							0);
}
#endif

#if (AIPL_CONVERT_I444 & TO_RGB888) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i444_to_rgb888_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	return aipl_color_convert_i444_to_24bit_default(input, output, pitch, width, height, 0, 1,
							2);
}
#endif

#if (AIPL_CONVERT_I444 & TO_RGB565) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i444_to_rgb565_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *u_ptr = y_ptr + yuv_size;
	const uint8_t *v_ptr = u_ptr + yuv_size;

	aipl_rgb565_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_ptr + i * pitch;
		const uint8_t *u_src = u_ptr + i * pitch;
		const uint8_t *v_src = v_ptr + i * pitch;
		aipl_rgb565_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			int32_t c;
			int32_t r, g, b;

			aipl_pre_cnvt_px_yuv_to_rgb(&r, &g, &b, u_src[j], v_src[j]);
			aipl_pre_cnvt_px_y(&c, y_src[j]);

			aipl_cnvt_px_yuv_to_rgb565(dst + j, c, r, g, b);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_I444 & TO_YV12) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i444_to_yv12_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = pitch * height;
	const uint8_t *y_src_ptr = input;
	const uint8_t *u_src_ptr = y_src_ptr + yuv_size;
	const uint8_t *v_src_ptr = u_src_ptr + yuv_size;

	yuv_size = width * height;
	uint8_t *y_dst_ptr = output;
	uint8_t *v_dst_ptr = y_dst_ptr + yuv_size;
	uint8_t *u_dst_ptr = v_dst_ptr + yuv_size / 4;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_src_ptr + i * pitch;
		uint8_t *y_dst = y_dst_ptr + i * width;

		memcpy(y_dst, y_src, width);

		if (!(i & 1)) {
			const uint8_t *u_src = u_src_ptr + i * pitch;
			uint8_t *u_dst = u_dst_ptr + i * width / 4;
			const uint8_t *v_src = v_src_ptr + i * pitch;
			uint8_t *v_dst = v_dst_ptr + i * width / 4;

			for (uint32_t j = 0; j < width; j += 2) {
				u_dst[j / 2] = u_src[j];
				v_dst[j / 2] = v_src[j];
			}
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_I444 & TO_I420) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i444_to_i420_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = pitch * height;
	const uint8_t *y_src_ptr = input;
	const uint8_t *u_src_ptr = y_src_ptr + yuv_size;
	const uint8_t *v_src_ptr = u_src_ptr + yuv_size;

	yuv_size = width * height;
	uint8_t *y_dst_ptr = output;
	uint8_t *u_dst_ptr = y_dst_ptr + yuv_size;
	uint8_t *v_dst_ptr = u_dst_ptr + yuv_size / 4;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_src_ptr + i * pitch;
		uint8_t *y_dst = y_dst_ptr + i * width;

		memcpy(y_dst, y_src, width);

		if (!(i & 1)) {
			const uint8_t *u_src = u_src_ptr + i * pitch;
			uint8_t *u_dst = u_dst_ptr + i * width / 4;
			const uint8_t *v_src = v_src_ptr + i * pitch;
			uint8_t *v_dst = v_dst_ptr + i * width / 4;

			for (uint32_t j = 0; j < width; j += 2) {
				u_dst[j / 2] = u_src[j];
				v_dst[j / 2] = v_src[j];
			}
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_I444 & TO_I422) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i444_to_i422_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = pitch * height;
	const uint8_t *y_src_ptr = input;
	const uint8_t *u_src_ptr = y_src_ptr + yuv_size;
	const uint8_t *v_src_ptr = u_src_ptr + yuv_size;

	yuv_size = width * height;
	uint8_t *y_dst_ptr = output;
	uint8_t *u_dst_ptr = y_dst_ptr + yuv_size;
	uint8_t *v_dst_ptr = u_dst_ptr + yuv_size / 2;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_src_ptr + i * pitch;
		uint8_t *y_dst = y_dst_ptr + i * width;
		const uint8_t *u_src = u_src_ptr + i * pitch;
		uint8_t *u_dst = u_dst_ptr + i * width / 2;
		const uint8_t *v_src = v_src_ptr + i * pitch;
		uint8_t *v_dst = v_dst_ptr + i * width / 2;

		memcpy(y_dst, y_src, width);

		for (uint32_t j = 0; j < width; j += 2) {
			u_dst[j / 2] = u_src[j];
			v_dst[j / 2] = v_src[j];
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_I444 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_i444_to_i400_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	return aipl_color_convert_yuv_planar_to_alpha8_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_I444 & TO_NV21) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i444_to_nv21_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = pitch * height;
	const uint8_t *y_src_ptr = input;
	const uint8_t *u_src_ptr = y_src_ptr + yuv_size;
	const uint8_t *v_src_ptr = u_src_ptr + yuv_size;

	yuv_size = width * height;
	uint8_t *y_dst_ptr = output;
	uint8_t *v_dst_ptr = y_dst_ptr + yuv_size;
	uint8_t *u_dst_ptr = v_dst_ptr + 1;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_src_ptr + i * pitch;
		uint8_t *y_dst = y_dst_ptr + i * width;

		memcpy(y_dst, y_src, width);

		if (!(i & 1)) {
			const uint8_t *u_src = u_src_ptr + i * pitch;
			uint8_t *u_dst = u_dst_ptr + i / 2 * width;
			const uint8_t *v_src = v_src_ptr + i * pitch;
			uint8_t *v_dst = v_dst_ptr + i / 2 * width;

			for (uint32_t j = 0; j < width; j += 2) {
				u_dst[j] = u_src[j];
				v_dst[j] = v_src[j];
			}
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_I444 & TO_NV12) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i444_to_nv12_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = pitch * height;
	const uint8_t *y_src_ptr = input;
	const uint8_t *u_src_ptr = y_src_ptr + yuv_size;
	const uint8_t *v_src_ptr = u_src_ptr + yuv_size;

	yuv_size = width * height;
	uint8_t *y_dst_ptr = output;
	uint8_t *u_dst_ptr = y_dst_ptr + yuv_size;
	uint8_t *v_dst_ptr = u_dst_ptr + 1;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_src_ptr + i * pitch;
		uint8_t *y_dst = y_dst_ptr + i * width;

		memcpy(y_dst, y_src, width);

		if (!(i & 1)) {
			const uint8_t *u_src = u_src_ptr + i * pitch;
			uint8_t *u_dst = u_dst_ptr + i / 2 * width;
			const uint8_t *v_src = v_src_ptr + i * pitch;
			uint8_t *v_dst = v_dst_ptr + i / 2 * width;

			for (uint32_t j = 0; j < width; j += 2) {
				u_dst[j] = u_src[j];
				v_dst[j] = v_src[j];
			}
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_I444 & TO_YUY2)
aipl_error_t aipl_color_convert_i444_to_yuy2_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = pitch * height;
	const uint8_t *y_src_ptr = input;
	const uint8_t *u_src_ptr = y_src_ptr + yuv_size;
	const uint8_t *v_src_ptr = u_src_ptr + yuv_size;

	uint8_t *y_dst_ptr = output;
	uint8_t *u_dst_ptr = y_dst_ptr + 1;
	uint8_t *v_dst_ptr = u_dst_ptr + 2;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_src_ptr + i * pitch;
		uint8_t *y_dst = y_dst_ptr + i * width * 2;
		const uint8_t *u_src = u_src_ptr + i * pitch;
		uint8_t *u_dst = u_dst_ptr + i * width * 2;
		const uint8_t *v_src = v_src_ptr + i * pitch;
		uint8_t *v_dst = v_dst_ptr + i * width * 2;

		for (uint32_t j = 0; j < width / 2; ++j) {
			y_dst[j * 4] = y_src[j * 2];
			y_dst[j * 4 + 2] = y_src[j * 2 + 1];
			u_dst[j * 4] = u_src[j * 2];
			v_dst[j * 4] = v_src[j * 2];
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_I444 & TO_UYVY)
aipl_error_t aipl_color_convert_i444_to_uyvy_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = pitch * height;
	const uint8_t *y_src_ptr = input;
	const uint8_t *u_src_ptr = y_src_ptr + yuv_size;
	const uint8_t *v_src_ptr = u_src_ptr + yuv_size;

	uint8_t *u_dst_ptr = output;
	uint8_t *y_dst_ptr = u_dst_ptr + 1;
	uint8_t *v_dst_ptr = u_dst_ptr + 2;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_src_ptr + i * pitch;
		uint8_t *y_dst = y_dst_ptr + i * width * 2;
		const uint8_t *u_src = u_src_ptr + i * pitch;
		uint8_t *u_dst = u_dst_ptr + i * width * 2;
		const uint8_t *v_src = v_src_ptr + i * pitch;
		uint8_t *v_dst = v_dst_ptr + i * width * 2;

		for (uint32_t j = 0; j < width / 2; ++j) {
			y_dst[j * 4] = y_src[j * 2];
			y_dst[j * 4 + 2] = y_src[j * 2 + 1];
			u_dst[j * 4] = u_src[j * 2];
			v_dst[j * 4] = v_src[j * 2];
		}
	}

	return AIPL_ERR_OK;
}
#endif
#endif

#if AIPL_CONVERT_NV12
aipl_error_t aipl_color_convert_nv12_default(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height,
					     aipl_color_format_t format)
{
	switch (format) {
		/* Alpha color formats */
#if (AIPL_CONVERT_NV12 & TO_ALPHA8_I400)
	case AIPL_COLOR_ALPHA8:
		return aipl_color_convert_nv12_to_alpha8_default(input, output, pitch, width,
								 height);
#endif
		/* RGB color formats */
#if (AIPL_CONVERT_NV12 & TO_ARGB8888) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB8888:
		return aipl_color_convert_nv12_to_argb8888_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_NV12 & TO_RGBA8888) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA8888:
		return aipl_color_convert_nv12_to_rgba8888_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_NV12 & TO_ARGB4444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB4444:
		return aipl_color_convert_nv12_to_argb4444_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_NV12 & TO_ARGB1555) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB1555:
		return aipl_color_convert_nv12_to_argb1555_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_NV12 & TO_RGBA4444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA4444:
		return aipl_color_convert_nv12_to_rgba4444_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_NV12 & TO_RGBA5551) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA5551:
		return aipl_color_convert_nv12_to_rgba5551_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_NV12 & TO_RGB565) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGB565:
		return aipl_color_convert_nv12_to_rgb565_default(input, output, pitch, width,
								 height);
#endif
#if (AIPL_CONVERT_NV12 & TO_BGR888) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_BGR888:
		return aipl_color_convert_nv12_to_bgr888_default(input, output, pitch, width,
								 height);
#endif
#if (AIPL_CONVERT_NV12 & TO_RGB888) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGB888:
		return aipl_color_convert_nv12_to_rgb888_default(input, output, pitch, width,
								 height);
#endif
		/* YUV color formats */
#if (AIPL_CONVERT_NV12 & TO_YV12) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_YV12:
		return aipl_color_convert_nv12_to_yv12_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_NV12 & TO_I420) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I420:
		return aipl_color_convert_nv12_to_i420_default(input, output, pitch, width, height);
#endif
	case AIPL_COLOR_NV12:
		return AIPL_ERR_FORMAT_MISMATCH;
#if (AIPL_CONVERT_NV12 & TO_NV21)
	case AIPL_COLOR_NV21:
		return aipl_color_convert_nv12_to_nv21_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_NV12 & TO_I422) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I422:
		return aipl_color_convert_nv12_to_i422_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_NV12 & TO_YUY2)
	case AIPL_COLOR_YUY2:
		return aipl_color_convert_nv12_to_yuy2_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_NV12 & TO_UYVY)
	case AIPL_COLOR_UYVY:
		return aipl_color_convert_nv12_to_uyvy_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_NV12 & TO_I444)
	case AIPL_COLOR_I444:
		return aipl_color_convert_nv12_to_i444_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_NV12 & TO_ALPHA8_I400)
	case AIPL_COLOR_I400:
		return aipl_color_convert_nv12_to_i400_default(input, output, pitch, width, height);
#endif

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

#if (AIPL_CONVERT_NV12 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_nv12_to_alpha8_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	return aipl_color_convert_yuv_planar_to_alpha8_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_NV12 & TO_ARGB8888) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_nv12_to_argb8888_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *u_ptr = y_ptr + yuv_size;
	const uint8_t *v_ptr = u_ptr + 1;

	return aipl_color_convert_yuv_semi_planar_to_argb8888_default(y_ptr, u_ptr, v_ptr, output,
								      pitch, width, height);
}
#endif

#if (AIPL_CONVERT_NV12 & TO_ARGB4444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_nv12_to_argb4444_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *u_ptr = y_ptr + yuv_size;
	const uint8_t *v_ptr = u_ptr + 1;

	return aipl_color_convert_yuv_semi_planar_to_argb4444_default(y_ptr, u_ptr, v_ptr, output,
								      pitch, width, height);
}
#endif

#if (AIPL_CONVERT_NV12 & TO_ARGB1555) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_nv12_to_argb1555_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *u_ptr = y_ptr + yuv_size;
	const uint8_t *v_ptr = u_ptr + 1;

	return aipl_color_convert_yuv_semi_planar_to_argb1555_default(y_ptr, u_ptr, v_ptr, output,
								      pitch, width, height);
}
#endif

#if (AIPL_CONVERT_NV12 & TO_RGBA8888) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_nv12_to_rgba8888_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *u_ptr = y_ptr + yuv_size;
	const uint8_t *v_ptr = u_ptr + 1;

	return aipl_color_convert_yuv_semi_planar_to_rgba8888_default(y_ptr, u_ptr, v_ptr, output,
								      pitch, width, height);
}
#endif

#if (AIPL_CONVERT_NV12 & TO_RGBA4444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_nv12_to_rgba4444_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *u_ptr = y_ptr + yuv_size;
	const uint8_t *v_ptr = u_ptr + 1;

	return aipl_color_convert_yuv_semi_planar_to_rgba4444_default(y_ptr, u_ptr, v_ptr, output,
								      pitch, width, height);
}
#endif

#if (AIPL_CONVERT_NV12 & TO_RGBA5551) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_nv12_to_rgba5551_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *u_ptr = y_ptr + yuv_size;
	const uint8_t *v_ptr = u_ptr + 1;

	return aipl_color_convert_yuv_semi_planar_to_rgba5551_default(y_ptr, u_ptr, v_ptr, output,
								      pitch, width, height);
}
#endif

#if (AIPL_CONVERT_NV12 & TO_BGR888) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_nv12_to_bgr888_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *u_ptr = y_ptr + yuv_size;
	const uint8_t *v_ptr = u_ptr + 1;

	return aipl_color_convert_yuv_semi_planar_to_24bit_default(y_ptr, u_ptr, v_ptr, output,
								   pitch, width, height, 2, 1, 0);
}
#endif

#if (AIPL_CONVERT_NV12 & TO_RGB888) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_nv12_to_rgb888_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *u_ptr = y_ptr + yuv_size;
	const uint8_t *v_ptr = u_ptr + 1;

	return aipl_color_convert_yuv_semi_planar_to_24bit_default(y_ptr, u_ptr, v_ptr, output,
								   pitch, width, height, 0, 1, 2);
}
#endif

#if (AIPL_CONVERT_NV12 & TO_RGB565) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_nv12_to_rgb565_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *u_ptr = y_ptr + yuv_size;
	const uint8_t *v_ptr = u_ptr + 1;

	return aipl_color_convert_yuv_semi_planar_to_rgb565_default(y_ptr, u_ptr, v_ptr, output,
								    pitch, width, height);
}
#endif

#if (AIPL_CONVERT_NV12 & TO_YV12) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_nv12_to_yv12_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_src = input;
	const uint8_t *u_src = y_src + yuv_size;
	const uint8_t *v_src = u_src + 1;

	yuv_size = width * height;
	uint8_t *y_dst = output;
	uint8_t *v_dst = y_dst + yuv_size;
	uint8_t *u_dst = v_dst + yuv_size / 4;

	return aipl_color_convert_yuv_semi_to_planar_default(y_src, u_src, v_src, y_dst, u_dst,
							     v_dst, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_NV12 & TO_I420) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_nv12_to_i420_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_src = input;
	const uint8_t *u_src = y_src + yuv_size;
	const uint8_t *v_src = u_src + 1;

	yuv_size = width * height;
	uint8_t *y_dst = output;
	uint8_t *u_dst = y_dst + yuv_size;
	uint8_t *v_dst = u_dst + yuv_size / 4;

	return aipl_color_convert_yuv_semi_to_planar_default(y_src, u_src, v_src, y_dst, u_dst,
							     v_dst, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_NV12 & TO_I422) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_nv12_to_i422_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = pitch * height;
	const uint8_t *y_src_ptr = input;
	const uint8_t *u_src_ptr = y_src_ptr + yuv_size;
	const uint8_t *v_src_ptr = u_src_ptr + 1;

	yuv_size = width * height;
	uint8_t *y_dst_ptr = output;
	uint8_t *u_dst_ptr = y_dst_ptr + yuv_size;
	uint8_t *v_dst_ptr = u_dst_ptr + yuv_size / 2;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_src_ptr + i * pitch;
		uint8_t *y_dst = y_dst_ptr + i * width;
		const uint8_t *u_src = u_src_ptr + i / 2 * pitch;
		uint8_t *u_dst = u_dst_ptr + i * width / 2;
		const uint8_t *v_src = v_src_ptr + i / 2 * pitch;
		uint8_t *v_dst = v_dst_ptr + i * width / 2;

		memcpy(y_dst, y_src, width);

		for (uint32_t j = 0; j < width; j += 2) {
			u_dst[j / 2] = u_src[j];
			v_dst[j / 2] = v_src[j];
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_NV12 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_nv12_to_i400_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	return aipl_color_convert_yuv_planar_to_alpha8_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_NV12 & TO_NV21)
aipl_error_t aipl_color_convert_nv12_to_nv21_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_src = input;
	const uint8_t *u_src = y_src + yuv_size;
	const uint8_t *v_src = u_src + 1;

	yuv_size = width * height;
	uint8_t *y_dst = output;
	uint8_t *v_dst = y_dst + yuv_size;
	uint8_t *u_dst = v_dst + 1;

	return aipl_color_convert_yuv_semi_to_semi_planar_default(y_src, u_src, v_src, y_dst, u_dst,
								  v_dst, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_NV12 & TO_I444)
aipl_error_t aipl_color_convert_nv12_to_i444_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = pitch * height;
	const uint8_t *y_src_ptr = input;
	const uint8_t *u_src_ptr = y_src_ptr + yuv_size;
	const uint8_t *v_src_ptr = u_src_ptr + 1;

	yuv_size = width * height;
	uint8_t *y_dst_ptr = output;
	uint8_t *u_dst_ptr = y_dst_ptr + yuv_size;
	uint8_t *v_dst_ptr = u_dst_ptr + yuv_size;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_src_ptr + i * pitch;
		uint8_t *y_dst = y_dst_ptr + i * width;

		memcpy(y_dst, y_src, width);

		const uint8_t *u_src = u_src_ptr + i / 2 * pitch;
		uint8_t *u_dst = u_dst_ptr + i * width;
		const uint8_t *v_src = v_src_ptr + i / 2 * pitch;
		uint8_t *v_dst = v_dst_ptr + i * width;

		for (uint32_t j = 0; j < width; j += 2) {
			u_dst[j] = u_dst[j + 1] = u_src[j];
			v_dst[j] = v_dst[j + 1] = v_src[j];
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_NV12 & TO_YUY2)
aipl_error_t aipl_color_convert_nv12_to_yuy2_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_src = input;
	const uint8_t *u_src = y_src + yuv_size;
	const uint8_t *v_src = u_src + 1;

	uint8_t *y_dst = output;
	uint8_t *u_dst = y_dst + 1;
	uint8_t *v_dst = u_dst + 2;

	return aipl_color_convert_yuv_semi_to_packed_default(y_src, u_src, v_src, y_dst, u_dst,
							     v_dst, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_NV12 & TO_UYVY)
aipl_error_t aipl_color_convert_nv12_to_uyvy_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_src = input;
	const uint8_t *u_src = y_src + yuv_size;
	const uint8_t *v_src = u_src + 1;

	uint8_t *u_dst = output;
	uint8_t *y_dst = u_dst + 1;
	uint8_t *v_dst = u_dst + 2;

	return aipl_color_convert_yuv_semi_to_packed_default(y_src, u_src, v_src, y_dst, u_dst,
							     v_dst, pitch, width, height);
}
#endif
#endif

#if AIPL_CONVERT_NV21
aipl_error_t aipl_color_convert_nv21_default(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height,
					     aipl_color_format_t format)
{
	switch (format) {
		/* Alpha color formats */
#if (AIPL_CONVERT_NV21 & TO_ALPHA8_I400)
	case AIPL_COLOR_ALPHA8:
		return aipl_color_convert_nv21_to_alpha8_default(input, output, pitch, width,
								 height);
#endif
		/* RGB color formats */
#if (AIPL_CONVERT_NV21 & TO_ARGB8888) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB8888:
		return aipl_color_convert_nv21_to_argb8888_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_NV21 & TO_RGBA8888) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA8888:
		return aipl_color_convert_nv21_to_rgba8888_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_NV21 & TO_ARGB4444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB4444:
		return aipl_color_convert_nv21_to_argb4444_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_NV21 & TO_ARGB1555) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB1555:
		return aipl_color_convert_nv21_to_argb1555_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_NV21 & TO_RGBA4444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA4444:
		return aipl_color_convert_nv21_to_rgba4444_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_NV21 & TO_RGBA5551) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA5551:
		return aipl_color_convert_nv21_to_rgba5551_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_NV21 & TO_RGB565) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGB565:
		return aipl_color_convert_nv21_to_rgb565_default(input, output, pitch, width,
								 height);
#endif
#if (AIPL_CONVERT_NV21 & TO_BGR888) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_BGR888:
		return aipl_color_convert_nv21_to_bgr888_default(input, output, pitch, width,
								 height);
#endif
#if (AIPL_CONVERT_NV21 & TO_RGB888) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGB888:
		return aipl_color_convert_nv21_to_rgb888_default(input, output, pitch, width,
								 height);
#endif
		/* YUV color formats */
#if (AIPL_CONVERT_NV21 & TO_YV12) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_YV12:
		return aipl_color_convert_nv21_to_yv12_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_NV21 & TO_I420) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I420:
		return aipl_color_convert_nv21_to_i420_default(input, output, pitch, width, height);
#endif
	case AIPL_COLOR_NV21:
		return AIPL_ERR_FORMAT_MISMATCH;
#if (AIPL_CONVERT_NV21 & TO_NV12)
	case AIPL_COLOR_NV12:
		return aipl_color_convert_nv21_to_nv12_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_NV21 & TO_I422) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I422:
		return aipl_color_convert_nv21_to_i422_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_NV21 & TO_YUY2)
	case AIPL_COLOR_YUY2:
		return aipl_color_convert_nv21_to_yuy2_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_NV21 & TO_UYVY)
	case AIPL_COLOR_UYVY:
		return aipl_color_convert_nv21_to_uyvy_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_NV21 & TO_I444)
	case AIPL_COLOR_I444:
		return aipl_color_convert_nv21_to_i444_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_NV21 & TO_ALPHA8_I400)
	case AIPL_COLOR_I400:
		return aipl_color_convert_nv21_to_i400_default(input, output, pitch, width, height);
#endif

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

#if (AIPL_CONVERT_NV21 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_nv21_to_alpha8_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	return aipl_color_convert_yuv_planar_to_alpha8_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_NV21 & TO_ARGB8888) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_nv21_to_argb8888_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *v_ptr = y_ptr + yuv_size;
	const uint8_t *u_ptr = v_ptr + 1;

	return aipl_color_convert_yuv_semi_planar_to_argb8888_default(y_ptr, u_ptr, v_ptr, output,
								      pitch, width, height);
}
#endif

#if (AIPL_CONVERT_NV21 & TO_ARGB4444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_nv21_to_argb4444_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *v_ptr = y_ptr + yuv_size;
	const uint8_t *u_ptr = v_ptr + 1;

	return aipl_color_convert_yuv_semi_planar_to_argb4444_default(y_ptr, u_ptr, v_ptr, output,
								      pitch, width, height);
}
#endif

#if (AIPL_CONVERT_NV21 & TO_ARGB1555) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_nv21_to_argb1555_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *v_ptr = y_ptr + yuv_size;
	const uint8_t *u_ptr = v_ptr + 1;

	return aipl_color_convert_yuv_semi_planar_to_argb1555_default(y_ptr, u_ptr, v_ptr, output,
								      pitch, width, height);
}
#endif

#if (AIPL_CONVERT_NV21 & TO_RGBA8888) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_nv21_to_rgba8888_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *v_ptr = y_ptr + yuv_size;
	const uint8_t *u_ptr = v_ptr + 1;

	return aipl_color_convert_yuv_semi_planar_to_rgba8888_default(y_ptr, u_ptr, v_ptr, output,
								      pitch, width, height);
}
#endif

#if (AIPL_CONVERT_NV21 & TO_RGBA4444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_nv21_to_rgba4444_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *v_ptr = y_ptr + yuv_size;
	const uint8_t *u_ptr = v_ptr + 1;

	return aipl_color_convert_yuv_semi_planar_to_rgba4444_default(y_ptr, u_ptr, v_ptr, output,
								      pitch, width, height);
}
#endif

#if (AIPL_CONVERT_NV21 & TO_RGBA5551) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_nv21_to_rgba5551_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *v_ptr = y_ptr + yuv_size;
	const uint8_t *u_ptr = v_ptr + 1;

	return aipl_color_convert_yuv_semi_planar_to_rgba5551_default(y_ptr, u_ptr, v_ptr, output,
								      pitch, width, height);
}
#endif

#if (AIPL_CONVERT_NV21 & TO_BGR888) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_nv21_to_bgr888_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *v_ptr = y_ptr + yuv_size;
	const uint8_t *u_ptr = v_ptr + 1;

	return aipl_color_convert_yuv_semi_planar_to_24bit_default(y_ptr, u_ptr, v_ptr, output,
								   pitch, width, height, 2, 1, 0);
}
#endif

#if (AIPL_CONVERT_NV21 & TO_RGB888) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_nv21_to_rgb888_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *v_ptr = y_ptr + yuv_size;
	const uint8_t *u_ptr = v_ptr + 1;

	return aipl_color_convert_yuv_semi_planar_to_24bit_default(y_ptr, u_ptr, v_ptr, output,
								   pitch, width, height, 0, 1, 2);
}
#endif

#if (AIPL_CONVERT_NV21 & TO_RGB565) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_nv21_to_rgb565_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *v_ptr = y_ptr + yuv_size;
	const uint8_t *u_ptr = v_ptr + 1;

	return aipl_color_convert_yuv_semi_planar_to_rgb565_default(y_ptr, u_ptr, v_ptr, output,
								    pitch, width, height);
}
#endif

#if (AIPL_CONVERT_NV21 & TO_YV12) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_nv21_to_yv12_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_src = input;
	const uint8_t *v_src = y_src + yuv_size;
	const uint8_t *u_src = v_src + 1;

	yuv_size = width * height;
	uint8_t *y_dst = output;
	uint8_t *v_dst = y_dst + yuv_size;
	uint8_t *u_dst = v_dst + yuv_size / 4;

	return aipl_color_convert_yuv_semi_to_planar_default(y_src, u_src, v_src, y_dst, u_dst,
							     v_dst, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_NV21 & TO_I420) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_nv21_to_i420_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_src = input;
	const uint8_t *v_src = y_src + yuv_size;
	const uint8_t *u_src = v_src + 1;

	yuv_size = width * height;
	uint8_t *y_dst = output;
	uint8_t *u_dst = y_dst + yuv_size;
	uint8_t *v_dst = u_dst + yuv_size / 4;

	return aipl_color_convert_yuv_semi_to_planar_default(y_src, u_src, v_src, y_dst, u_dst,
							     v_dst, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_NV21 & TO_I422) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_nv21_to_i422_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = pitch * height;
	const uint8_t *y_src_ptr = input;
	const uint8_t *v_src_ptr = y_src_ptr + yuv_size;
	const uint8_t *u_src_ptr = v_src_ptr + 1;

	yuv_size = width * height;
	uint8_t *y_dst_ptr = output;
	uint8_t *u_dst_ptr = y_dst_ptr + yuv_size;
	uint8_t *v_dst_ptr = u_dst_ptr + yuv_size / 2;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_src_ptr + i * pitch;
		uint8_t *y_dst = y_dst_ptr + i * width;
		const uint8_t *u_src = u_src_ptr + i / 2 * pitch;
		uint8_t *u_dst = u_dst_ptr + i * width / 2;
		const uint8_t *v_src = v_src_ptr + i / 2 * pitch;
		uint8_t *v_dst = v_dst_ptr + i * width / 2;

		memcpy(y_dst, y_src, width);

		for (uint32_t j = 0; j < width; j += 2) {
			u_dst[j / 2] = u_src[j];
			v_dst[j / 2] = v_src[j];
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_NV21 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_nv21_to_i400_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	return aipl_color_convert_yuv_planar_to_alpha8_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_NV21 & TO_NV12)
aipl_error_t aipl_color_convert_nv21_to_nv12_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_src = input;
	const uint8_t *v_src = y_src + yuv_size;
	const uint8_t *u_src = v_src + 1;

	yuv_size = width * height;
	uint8_t *y_dst = output;
	uint8_t *u_dst = y_dst + yuv_size;
	uint8_t *v_dst = u_dst + 1;

	return aipl_color_convert_yuv_semi_to_semi_planar_default(y_src, u_src, v_src, y_dst, u_dst,
								  v_dst, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_NV21 & TO_I444)
aipl_error_t aipl_color_convert_nv21_to_i444_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = pitch * height;
	const uint8_t *y_src_ptr = input;
	const uint8_t *v_src_ptr = y_src_ptr + yuv_size;
	const uint8_t *u_src_ptr = v_src_ptr + 1;

	yuv_size = width * height;
	uint8_t *y_dst_ptr = output;
	uint8_t *u_dst_ptr = y_dst_ptr + yuv_size;
	uint8_t *v_dst_ptr = u_dst_ptr + yuv_size;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_src_ptr + i * pitch;
		uint8_t *y_dst = y_dst_ptr + i * width;

		memcpy(y_dst, y_src, width);

		const uint8_t *u_src = u_src_ptr + i / 2 * pitch;
		uint8_t *u_dst = u_dst_ptr + i * width;
		const uint8_t *v_src = v_src_ptr + i / 2 * pitch;
		uint8_t *v_dst = v_dst_ptr + i * width;

		for (uint32_t j = 0; j < width; j += 2) {
			u_dst[j] = u_dst[j + 1] = u_src[j];
			v_dst[j] = v_dst[j + 1] = v_src[j];
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_NV21 & TO_YUY2)
aipl_error_t aipl_color_convert_nv21_to_yuy2_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_src = input;
	const uint8_t *v_src = y_src + yuv_size;
	const uint8_t *u_src = v_src + 1;

	uint8_t *y_dst = output;
	uint8_t *u_dst = y_dst + 1;
	uint8_t *v_dst = u_dst + 2;

	return aipl_color_convert_yuv_semi_to_packed_default(y_src, u_src, v_src, y_dst, u_dst,
							     v_dst, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_NV21 & TO_UYVY)
aipl_error_t aipl_color_convert_nv21_to_uyvy_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	uint32_t yuv_size = pitch * height;
	const uint8_t *y_src = input;
	const uint8_t *v_src = y_src + yuv_size;
	const uint8_t *u_src = v_src + 1;

	uint8_t *u_dst = output;
	uint8_t *y_dst = u_dst + 1;
	uint8_t *v_dst = u_dst + 2;

	return aipl_color_convert_yuv_semi_to_packed_default(y_src, u_src, v_src, y_dst, u_dst,
							     v_dst, pitch, width, height);
}
#endif
#endif

#if AIPL_CONVERT_YUY2
aipl_error_t aipl_color_convert_yuy2_default(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height,
					     aipl_color_format_t format)
{
	switch (format) {
		/* Alpha color formats */
#if (AIPL_CONVERT_YUY2 & TO_ALPHA8_I400) &&                                                        \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ALPHA8:
		return aipl_color_convert_yuy2_to_alpha8_default(input, output, pitch, width,
								 height);
#endif
		/* RGB color formats */
#if (AIPL_CONVERT_YUY2 & TO_ARGB8888) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB8888:
		return aipl_color_convert_yuy2_to_argb8888_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_YUY2 & TO_RGBA8888) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA8888:
		return aipl_color_convert_yuy2_to_rgba8888_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_YUY2 & TO_ARGB4444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB4444:
		return aipl_color_convert_yuy2_to_argb4444_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_YUY2 & TO_ARGB1555) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB1555:
		return aipl_color_convert_yuy2_to_argb1555_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_YUY2 & TO_RGBA4444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA4444:
		return aipl_color_convert_yuy2_to_rgba4444_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_YUY2 & TO_RGBA5551) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA5551:
		return aipl_color_convert_yuy2_to_rgba5551_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_YUY2 & TO_RGB565) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGB565:
		return aipl_color_convert_yuy2_to_rgb565_default(input, output, pitch, width,
								 height);
#endif
#if (AIPL_CONVERT_YUY2 & TO_BGR888) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_BGR888:
		return aipl_color_convert_yuy2_to_bgr888_default(input, output, pitch, width,
								 height);
#endif
#if (AIPL_CONVERT_YUY2 & TO_RGB888) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGB888:
		return aipl_color_convert_yuy2_to_rgb888_default(input, output, pitch, width,
								 height);
#endif
		/* YUV color formats */
#if (AIPL_CONVERT_YUY2 & TO_YV12) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_YV12:
		return aipl_color_convert_yuy2_to_yv12_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_YUY2 & TO_I420) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I420:
		return aipl_color_convert_yuy2_to_i420_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_YUY2 & TO_NV21) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_NV21:
		return aipl_color_convert_yuy2_to_nv21_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_YUY2 & TO_NV12) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_NV12:
		return aipl_color_convert_yuy2_to_nv12_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_YUY2 & TO_I422) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I422:
		return aipl_color_convert_yuy2_to_i422_default(input, output, pitch, width, height);
#endif
	case AIPL_COLOR_YUY2:
		return AIPL_ERR_FORMAT_MISMATCH;
#if (AIPL_CONVERT_YUY2 & TO_UYVY)
	case AIPL_COLOR_UYVY:
		return aipl_color_convert_yuy2_to_uyvy_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_YUY2 & TO_I444) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I444:
		return aipl_color_convert_yuy2_to_i444_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_YUY2 & TO_ALPHA8_I400) &&                                                        \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I400:
		return aipl_color_convert_yuy2_to_i400_default(input, output, pitch, width, height);
#endif

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

#if (AIPL_CONVERT_YUY2 & TO_ALPHA8_I400) &&                                                        \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuy2_to_alpha8_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	return aipl_color_convert_yuv_packed_to_alpha8_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_ARGB8888) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuy2_to_argb8888_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	const uint8_t *y_ptr = input;
	const uint8_t *u_ptr = y_ptr + 1;
	const uint8_t *v_ptr = u_ptr + 2;

	return aipl_color_convert_yuv_packed_to_argb8888_default(y_ptr, u_ptr, v_ptr, output, pitch,
								 width, height);
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_ARGB4444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuy2_to_argb4444_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	const uint8_t *y_ptr = input;
	const uint8_t *u_ptr = y_ptr + 1;
	const uint8_t *v_ptr = u_ptr + 2;

	return aipl_color_convert_yuv_packed_to_argb4444_default(y_ptr, u_ptr, v_ptr, output, pitch,
								 width, height);
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_ARGB1555) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuy2_to_argb1555_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	const uint8_t *y_ptr = input;
	const uint8_t *u_ptr = y_ptr + 1;
	const uint8_t *v_ptr = u_ptr + 2;

	return aipl_color_convert_yuv_packed_to_argb1555_default(y_ptr, u_ptr, v_ptr, output, pitch,
								 width, height);
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_RGBA8888) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuy2_to_rgba8888_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	const uint8_t *y_ptr = input;
	const uint8_t *u_ptr = y_ptr + 1;
	const uint8_t *v_ptr = u_ptr + 2;

	return aipl_color_convert_yuv_packed_to_rgba8888_default(y_ptr, u_ptr, v_ptr, output, pitch,
								 width, height);
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_RGBA4444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuy2_to_rgba4444_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	const uint8_t *y_ptr = input;
	const uint8_t *u_ptr = y_ptr + 1;
	const uint8_t *v_ptr = u_ptr + 2;

	return aipl_color_convert_yuv_packed_to_rgba4444_default(y_ptr, u_ptr, v_ptr, output, pitch,
								 width, height);
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_RGBA5551) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuy2_to_rgba5551_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	const uint8_t *y_ptr = input;
	const uint8_t *u_ptr = y_ptr + 1;
	const uint8_t *v_ptr = u_ptr + 2;

	return aipl_color_convert_yuv_packed_to_rgba5551_default(y_ptr, u_ptr, v_ptr, output, pitch,
								 width, height);
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_BGR888) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuy2_to_bgr888_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	const uint8_t *y_ptr = input;
	const uint8_t *u_ptr = y_ptr + 1;
	const uint8_t *v_ptr = u_ptr + 2;

	return aipl_color_convert_yuv_packed_to_24bit_default(y_ptr, u_ptr, v_ptr, output, pitch,
							      width, height, 2, 1, 0);
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_RGB888) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuy2_to_rgb888_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	const uint8_t *y_ptr = input;
	const uint8_t *u_ptr = y_ptr + 1;
	const uint8_t *v_ptr = u_ptr + 2;

	return aipl_color_convert_yuv_packed_to_24bit_default(y_ptr, u_ptr, v_ptr, output, pitch,
							      width, height, 0, 1, 2);
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_RGB565) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuy2_to_rgb565_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	const uint8_t *y_ptr = input;
	const uint8_t *u_ptr = y_ptr + 1;
	const uint8_t *v_ptr = u_ptr + 2;

	return aipl_color_convert_yuv_packed_to_rgb565_default(y_ptr, u_ptr, v_ptr, output, pitch,
							       width, height);
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_YV12) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuy2_to_yv12_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	const uint8_t *y_src = input;
	const uint8_t *u_src = y_src + 1;
	const uint8_t *v_src = u_src + 2;

	uint32_t yuv_size = width * height;
	uint8_t *y_dst = output;
	uint8_t *v_dst = y_dst + yuv_size;
	uint8_t *u_dst = v_dst + yuv_size / 4;

	return aipl_color_convert_yuv_packed_to_planar_default(y_src, u_src, v_src, y_dst, u_dst,
							       v_dst, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_I420) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuy2_to_i420_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	const uint8_t *y_src = input;
	const uint8_t *u_src = y_src + 1;
	const uint8_t *v_src = u_src + 2;

	uint32_t yuv_size = width * height;
	uint8_t *y_dst = output;
	uint8_t *u_dst = y_dst + yuv_size;
	uint8_t *v_dst = u_dst + yuv_size / 4;

	return aipl_color_convert_yuv_packed_to_planar_default(y_src, u_src, v_src, y_dst, u_dst,
							       v_dst, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_I422) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuy2_to_i422_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint8_t *y_src_ptr = input;
	const uint8_t *u_src_ptr = y_src_ptr + 1;
	const uint8_t *v_src_ptr = u_src_ptr + 2;

	uint32_t yuv_size = width * height;
	uint8_t *y_dst_ptr = output;
	uint8_t *u_dst_ptr = y_dst_ptr + yuv_size;
	uint8_t *v_dst_ptr = u_dst_ptr + yuv_size / 2;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_src_ptr + i * pitch * 2;
		uint8_t *y_dst = y_dst_ptr + i * width;

		for (uint32_t j = 0; j < width * 2; j += 2) {
			y_dst[j / 2] = y_src[j];
		}

		const uint8_t *u_src = u_src_ptr + i * pitch * 2;
		uint8_t *u_dst = u_dst_ptr + i * width / 2;
		const uint8_t *v_src = v_src_ptr + i * pitch * 2;
		uint8_t *v_dst = v_dst_ptr + i * width / 2;

		for (uint32_t j = 0; j < width * 2; j += 4) {
			u_dst[j / 4] = u_src[j];
			v_dst[j / 4] = v_src[j];
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_ALPHA8_I400) &&                                                        \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuy2_to_i400_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	return aipl_color_convert_yuv_packed_to_alpha8_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_NV12) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuy2_to_nv12_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	const uint8_t *y_src = input;
	const uint8_t *u_src = y_src + 1;
	const uint8_t *v_src = u_src + 2;

	uint32_t yuv_size = width * height;
	uint8_t *y_dst = output;
	uint8_t *u_dst = y_dst + yuv_size;
	uint8_t *v_dst = u_dst + 1;

	return aipl_color_convert_yuv_packed_to_semi_default(y_src, u_src, v_src, y_dst, u_dst,
							     v_dst, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_I444) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuy2_to_i444_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint8_t *y_src_ptr = input;
	const uint8_t *u_src_ptr = y_src_ptr + 1;
	const uint8_t *v_src_ptr = u_src_ptr + 2;

	uint32_t yuv_size = width * height;
	uint8_t *y_dst_ptr = output;
	uint8_t *u_dst_ptr = y_dst_ptr + yuv_size;
	uint8_t *v_dst_ptr = u_dst_ptr + yuv_size;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_src_ptr + i * pitch * 2;
		uint8_t *y_dst = y_dst_ptr + i * width;

		for (uint32_t j = 0; j < width * 2; j += 2) {
			y_dst[j / 2] = y_src[j];
		}

		const uint8_t *u_src = u_src_ptr + i * pitch * 2;
		uint8_t *u_dst = u_dst_ptr + i * width;
		const uint8_t *v_src = v_src_ptr + i * pitch * 2;
		uint8_t *v_dst = v_dst_ptr + i * width;

		for (uint32_t j = 0; j < width * 2; j += 4) {
			u_dst[j / 2] = u_dst[j / 2 + 1] = u_src[j];
			v_dst[j / 2] = v_dst[j / 2 + 1] = v_src[j];
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_NV21) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuy2_to_nv21_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	const uint8_t *y_src = input;
	const uint8_t *u_src = y_src + 1;
	const uint8_t *v_src = u_src + 2;

	uint32_t yuv_size = width * height;
	uint8_t *y_dst = output;
	uint8_t *v_dst = y_dst + yuv_size;
	uint8_t *u_dst = v_dst + 1;

	return aipl_color_convert_yuv_packed_to_semi_default(y_src, u_src, v_src, y_dst, u_dst,
							     v_dst, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_UYVY)
aipl_error_t aipl_color_convert_yuy2_to_uyvy_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	const uint8_t *y_src = input;
	const uint8_t *u_src = y_src + 1;
	const uint8_t *v_src = u_src + 2;

	uint8_t *u_dst = output;
	uint8_t *y_dst = u_dst + 1;
	uint8_t *v_dst = u_dst + 2;

	return aipl_color_convert_yuv_packed_to_packed_default(y_src, u_src, v_src, y_dst, u_dst,
							       v_dst, pitch, width, height);
}
#endif
#endif

#if AIPL_CONVERT_UYVY
aipl_error_t aipl_color_convert_uyvy_default(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height,
					     aipl_color_format_t format)
{
	switch (format) {
		/* Alpha color formats */
#if (AIPL_CONVERT_UYVY & TO_ALPHA8_I400) &&                                                        \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ALPHA8:
		return aipl_color_convert_uyvy_to_alpha8_default(input, output, pitch, width,
								 height);
#endif
		/* RGB color formats */
#if (AIPL_CONVERT_UYVY & TO_ARGB8888) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB8888:
		return aipl_color_convert_uyvy_to_argb8888_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_UYVY & TO_RGBA8888) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA8888:
		return aipl_color_convert_uyvy_to_rgba8888_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_UYVY & TO_ARGB4444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB4444:
		return aipl_color_convert_uyvy_to_argb4444_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_UYVY & TO_ARGB1555) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB1555:
		return aipl_color_convert_uyvy_to_argb1555_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_UYVY & TO_RGBA4444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA4444:
		return aipl_color_convert_uyvy_to_rgba4444_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_UYVY & TO_RGBA5551) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA5551:
		return aipl_color_convert_uyvy_to_rgba5551_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_UYVY & TO_RGB565) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGB565:
		return aipl_color_convert_uyvy_to_rgb565_default(input, output, pitch, width,
								 height);
#endif
#if (AIPL_CONVERT_UYVY & TO_BGR888) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_BGR888:
		return aipl_color_convert_uyvy_to_bgr888_default(input, output, pitch, width,
								 height);
#endif
#if (AIPL_CONVERT_UYVY & TO_RGB888) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGB888:
		return aipl_color_convert_uyvy_to_rgb888_default(input, output, pitch, width,
								 height);
#endif
		/* YUV color formats */
#if (AIPL_CONVERT_UYVY & TO_YV12) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_YV12:
		return aipl_color_convert_uyvy_to_yv12_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_UYVY & TO_I420) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I420:
		return aipl_color_convert_uyvy_to_i420_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_UYVY & TO_NV21) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_NV21:
		return aipl_color_convert_uyvy_to_nv21_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_UYVY & TO_NV12) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_NV12:
		return aipl_color_convert_uyvy_to_nv12_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_UYVY & TO_I422) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I422:
		return aipl_color_convert_uyvy_to_i422_default(input, output, pitch, width, height);
#endif
	case AIPL_COLOR_UYVY:
		return AIPL_ERR_FORMAT_MISMATCH;
#if (AIPL_CONVERT_UYVY & TO_YUY2)
	case AIPL_COLOR_YUY2:
		return aipl_color_convert_uyvy_to_yuy2_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_UYVY & TO_I444) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I444:
		return aipl_color_convert_uyvy_to_i444_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_UYVY & TO_ALPHA8_I400) &&                                                        \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_I400:
		return aipl_color_convert_uyvy_to_i400_default(input, output, pitch, width, height);
#endif

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

#if (AIPL_CONVERT_UYVY & TO_ALPHA8_I400) &&                                                        \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_uyvy_to_alpha8_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	const uint8_t *y_ptr = input + 1;

	return aipl_color_convert_yuv_packed_to_alpha8_default(y_ptr, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_UYVY & TO_ARGB8888) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_uyvy_to_argb8888_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	const uint8_t *u_ptr = input;
	const uint8_t *y_ptr = u_ptr + 1;
	const uint8_t *v_ptr = u_ptr + 2;

	return aipl_color_convert_yuv_packed_to_argb8888_default(y_ptr, u_ptr, v_ptr, output, pitch,
								 width, height);
}
#endif

#if (AIPL_CONVERT_UYVY & TO_ARGB4444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_uyvy_to_argb4444_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	const uint8_t *u_ptr = input;
	const uint8_t *y_ptr = u_ptr + 1;
	const uint8_t *v_ptr = u_ptr + 2;

	return aipl_color_convert_yuv_packed_to_argb4444_default(y_ptr, u_ptr, v_ptr, output, pitch,
								 width, height);
}
#endif

#if (AIPL_CONVERT_UYVY & TO_ARGB1555) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_uyvy_to_argb1555_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	const uint8_t *u_ptr = input;
	const uint8_t *y_ptr = u_ptr + 1;
	const uint8_t *v_ptr = u_ptr + 2;

	return aipl_color_convert_yuv_packed_to_argb1555_default(y_ptr, u_ptr, v_ptr, output, pitch,
								 width, height);
}
#endif

#if (AIPL_CONVERT_UYVY & TO_RGBA8888) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_uyvy_to_rgba8888_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	const uint8_t *u_ptr = input;
	const uint8_t *y_ptr = u_ptr + 1;
	const uint8_t *v_ptr = u_ptr + 2;

	return aipl_color_convert_yuv_packed_to_rgba8888_default(y_ptr, u_ptr, v_ptr, output, pitch,
								 width, height);
}
#endif

#if (AIPL_CONVERT_UYVY & TO_RGBA4444) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_uyvy_to_rgba4444_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	const uint8_t *u_ptr = input;
	const uint8_t *y_ptr = u_ptr + 1;
	const uint8_t *v_ptr = u_ptr + 2;

	return aipl_color_convert_yuv_packed_to_rgba4444_default(y_ptr, u_ptr, v_ptr, output, pitch,
								 width, height);
}
#endif

#if (AIPL_CONVERT_UYVY & TO_RGBA5551) &&                                                           \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_uyvy_to_rgba5551_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	const uint8_t *u_ptr = input;
	const uint8_t *y_ptr = u_ptr + 1;
	const uint8_t *v_ptr = u_ptr + 2;

	return aipl_color_convert_yuv_packed_to_rgba5551_default(y_ptr, u_ptr, v_ptr, output, pitch,
								 width, height);
}
#endif

#if (AIPL_CONVERT_UYVY & TO_BGR888) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_uyvy_to_bgr888_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	const uint8_t *u_ptr = input;
	const uint8_t *y_ptr = u_ptr + 1;
	const uint8_t *v_ptr = u_ptr + 2;

	return aipl_color_convert_yuv_packed_to_24bit_default(y_ptr, u_ptr, v_ptr, output, pitch,
							      width, height, 2, 1, 0);
}
#endif

#if (AIPL_CONVERT_UYVY & TO_RGB888) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_uyvy_to_rgb888_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	const uint8_t *u_ptr = input;
	const uint8_t *y_ptr = u_ptr + 1;
	const uint8_t *v_ptr = u_ptr + 2;

	return aipl_color_convert_yuv_packed_to_24bit_default(y_ptr, u_ptr, v_ptr, output, pitch,
							      width, height, 0, 1, 2);
}
#endif

#if (AIPL_CONVERT_UYVY & TO_RGB565) &&                                                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_uyvy_to_rgb565_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	const uint8_t *u_ptr = input;
	const uint8_t *y_ptr = u_ptr + 1;
	const uint8_t *v_ptr = u_ptr + 2;

	return aipl_color_convert_yuv_packed_to_rgb565_default(y_ptr, u_ptr, v_ptr, output, pitch,
							       width, height);
}
#endif

#if (AIPL_CONVERT_UYVY & TO_YV12) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_uyvy_to_yv12_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	const uint8_t *u_src = input;
	const uint8_t *y_src = u_src + 1;
	const uint8_t *v_src = u_src + 2;

	uint32_t yuv_size = width * height;
	uint8_t *y_dst = output;
	uint8_t *v_dst = y_dst + yuv_size;
	uint8_t *u_dst = v_dst + yuv_size / 4;

	return aipl_color_convert_yuv_packed_to_planar_default(y_src, u_src, v_src, y_dst, u_dst,
							       v_dst, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_UYVY & TO_I420) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_uyvy_to_i420_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	const uint8_t *u_src = input;
	const uint8_t *y_src = u_src + 1;
	const uint8_t *v_src = u_src + 2;

	uint32_t yuv_size = width * height;
	uint8_t *y_dst = output;
	uint8_t *u_dst = y_dst + yuv_size;
	uint8_t *v_dst = u_dst + yuv_size / 4;

	return aipl_color_convert_yuv_packed_to_planar_default(y_src, u_src, v_src, y_dst, u_dst,
							       v_dst, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_UYVY & TO_I422) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_uyvy_to_i422_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint8_t *u_src_ptr = input;
	const uint8_t *y_src_ptr = u_src_ptr + 1;
	const uint8_t *v_src_ptr = u_src_ptr + 2;

	uint32_t yuv_size = width * height;
	uint8_t *y_dst_ptr = output;
	uint8_t *u_dst_ptr = y_dst_ptr + yuv_size;
	uint8_t *v_dst_ptr = u_dst_ptr + yuv_size / 2;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_src_ptr + i * pitch * 2;
		uint8_t *y_dst = y_dst_ptr + i * width;

		for (uint32_t j = 0; j < width * 2; j += 2) {
			y_dst[j / 2] = y_src[j];
		}

		const uint8_t *u_src = u_src_ptr + i * pitch * 2;
		uint8_t *u_dst = u_dst_ptr + i * width / 2;
		const uint8_t *v_src = v_src_ptr + i * pitch * 2;
		uint8_t *v_dst = v_dst_ptr + i * width / 2;

		for (uint32_t j = 0; j < width * 2; j += 4) {
			u_dst[j / 4] = u_src[j];
			v_dst[j / 4] = v_src[j];
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_UYVY & TO_ALPHA8_I400) &&                                                        \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_uyvy_to_i400_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	const uint8_t *y_ptr = input + 1;

	return aipl_color_convert_yuv_packed_to_alpha8_default(y_ptr, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_UYVY & TO_NV12) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_uyvy_to_nv12_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	const uint8_t *u_src = input;
	const uint8_t *y_src = u_src + 1;
	const uint8_t *v_src = u_src + 2;

	uint32_t yuv_size = width * height;
	uint8_t *y_dst = output;
	uint8_t *u_dst = y_dst + yuv_size;
	uint8_t *v_dst = u_dst + 1;

	return aipl_color_convert_yuv_packed_to_semi_default(y_src, u_src, v_src, y_dst, u_dst,
							     v_dst, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_UYVY & TO_I444) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_uyvy_to_i444_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint8_t *u_src_ptr = input;
	const uint8_t *y_src_ptr = u_src_ptr + 1;
	const uint8_t *v_src_ptr = u_src_ptr + 2;

	uint32_t yuv_size = width * height;
	uint8_t *y_dst_ptr = output;
	uint8_t *u_dst_ptr = y_dst_ptr + yuv_size;
	uint8_t *v_dst_ptr = u_dst_ptr + yuv_size;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_src_ptr + i * pitch * 2;
		uint8_t *y_dst = y_dst_ptr + i * width;

		for (uint32_t j = 0; j < width * 2; j += 2) {
			y_dst[j / 2] = y_src[j];
		}

		const uint8_t *u_src = u_src_ptr + i * pitch * 2;
		uint8_t *u_dst = u_dst_ptr + i * width;
		const uint8_t *v_src = v_src_ptr + i * pitch * 2;
		uint8_t *v_dst = v_dst_ptr + i * width;

		for (uint32_t j = 0; j < width * 2; j += 4) {
			u_dst[j / 2] = u_dst[j / 2 + 1] = u_src[j];
			v_dst[j / 2] = v_dst[j / 2 + 1] = v_src[j];
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_UYVY & TO_NV21) &&                                                               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_uyvy_to_nv21_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	const uint8_t *u_src = input;
	const uint8_t *y_src = u_src + 1;
	const uint8_t *v_src = u_src + 2;

	uint32_t yuv_size = width * height;
	uint8_t *y_dst = output;
	uint8_t *v_dst = y_dst + yuv_size;
	uint8_t *u_dst = v_dst + 1;

	return aipl_color_convert_yuv_packed_to_semi_default(y_src, u_src, v_src, y_dst, u_dst,
							     v_dst, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_UYVY & TO_YUY2)
aipl_error_t aipl_color_convert_uyvy_to_yuy2_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	const uint8_t *u_src = input;
	const uint8_t *y_src = u_src + 1;
	const uint8_t *v_src = u_src + 2;

	uint8_t *y_dst = output;
	uint8_t *u_dst = y_dst + 1;
	uint8_t *v_dst = u_dst + 2;

	return aipl_color_convert_yuv_packed_to_packed_default(y_src, u_src, v_src, y_dst, u_dst,
							       v_dst, pitch, width, height);
}
#endif
#endif

#if (AIPL_CONVERT_ALPHA8_I400)
aipl_error_t aipl_color_convert_i400_default(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height,
					     aipl_color_format_t format)
{
	switch (format) {
	/* Alpha color formats */
	case AIPL_COLOR_ALPHA8:
		return aipl_color_convert_i400_to_alpha8_default(input, output, pitch, width,
								 height);
		/* RGB color formats */
#if (AIPL_CONVERT_ALPHA8_I400 & TO_ARGB8888) &&                                                    \
	((!defined(AIPL_DAVE2D_ACCELERATION) && !defined(AIPL_HELIUM_ACCELERATION)) ||             \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB8888:
		return aipl_color_convert_i400_to_argb8888_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_ALPHA8_I400 & TO_RGBA8888) &&                                                    \
	((!defined(AIPL_DAVE2D_ACCELERATION) && !defined(AIPL_HELIUM_ACCELERATION)) ||             \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA8888:
		return aipl_color_convert_i400_to_rgba8888_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_ALPHA8_I400 & TO_ARGB4444) &&                                                    \
	((!(defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD)) &&               \
	  !defined(AIPL_HELIUM_ACCELERATION)) ||                                                   \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB4444:
		return aipl_color_convert_i400_to_argb4444_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_ALPHA8_I400 & TO_ARGB1555) &&                                                    \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_ARGB1555:
		return aipl_color_convert_i400_to_argb1555_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_ALPHA8_I400 & TO_RGBA4444) &&                                                    \
	((!(defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD)) &&               \
	  !defined(AIPL_HELIUM_ACCELERATION)) ||                                                   \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA4444:
		return aipl_color_convert_i400_to_rgba4444_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_ALPHA8_I400 & TO_RGBA5551) &&                                                    \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGBA5551:
		return aipl_color_convert_i400_to_rgba5551_default(input, output, pitch, width,
								   height);
#endif
#if (AIPL_CONVERT_ALPHA8_I400 & TO_RGB565) &&                                                      \
	((!(defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD)) &&               \
	  !defined(AIPL_HELIUM_ACCELERATION)) ||                                                   \
	 defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGB565:
		return aipl_color_convert_i400_to_rgb565_default(input, output, pitch, width,
								 height);
#endif
#if (AIPL_CONVERT_ALPHA8_I400 & TO_BGR888) &&                                                      \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_BGR888:
		return aipl_color_convert_i400_to_bgr888_default(input, output, pitch, width,
								 height);
#endif
#if (AIPL_CONVERT_ALPHA8_I400 & TO_RGB888) &&                                                      \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_RGB888:
		return aipl_color_convert_i400_to_rgb888_default(input, output, pitch, width,
								 height);
#endif
		/* YUV color formats */
#if (AIPL_CONVERT_ALPHA8_I400 & TO_YV12)
	case AIPL_COLOR_YV12:
		return aipl_color_convert_i400_to_yv12_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ALPHA8_I400 & TO_I420)
	case AIPL_COLOR_I420:
		return aipl_color_convert_i400_to_i420_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ALPHA8_I400 & TO_NV21)
	case AIPL_COLOR_NV21:
		return aipl_color_convert_i400_to_nv21_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ALPHA8_I400 & TO_NV12)
	case AIPL_COLOR_NV12:
		return aipl_color_convert_i400_to_nv12_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ALPHA8_I400 & TO_I422)
	case AIPL_COLOR_I422:
		return aipl_color_convert_i400_to_i422_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ALPHA8_I400 & TO_UYVY) &&                                                        \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_UYVY:
		return aipl_color_convert_i400_to_uyvy_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ALPHA8_I400 & TO_YUY2) &&                                                        \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
	case AIPL_COLOR_YUY2:
		return aipl_color_convert_i400_to_yuy2_default(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ALPHA8_I400 & TO_I444)
	case AIPL_COLOR_I444:
		return aipl_color_convert_i400_to_i444_default(input, output, pitch, width, height);
#endif
	case AIPL_COLOR_I400:
		return AIPL_ERR_FORMAT_MISMATCH;

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 || AIPL_CONVERT_I420 || AIPL_CONVERT_YV12 || AIPL_CONVERT_I422 ||    \
	 AIPL_CONVERT_I444 || AIPL_CONVERT_NV12 || AIPL_CONVERT_NV21)
aipl_error_t aipl_color_convert_i400_to_alpha8_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint8_t *y_ptr = input;
	uint8_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_ptr + i * pitch;
		uint8_t *dst = dst_ptr + i * width;

		memcpy(dst, y_src, width);
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400)
#if (AIPL_CONVERT_ALPHA8_I400 & TO_ARGB8888) &&                                                    \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i400_to_argb8888_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint8_t *y_ptr = input;

	aipl_argb8888_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_ptr + i * pitch;
		aipl_argb8888_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_i400_to_argb8888(dst, y_src);

			++y_src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_ARGB4444) &&                                                    \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i400_to_argb4444_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint8_t *y_ptr = input;

	aipl_argb4444_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_ptr + i * pitch;
		aipl_argb4444_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_i400_to_argb4444(dst, y_src);

			++y_src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_ARGB1555) &&                                                    \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i400_to_argb1555_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint8_t *y_ptr = input;

	aipl_argb1555_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_ptr + i * pitch;
		aipl_argb1555_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_i400_to_argb1555(dst, y_src);

			++y_src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_RGBA8888) &&                                                    \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i400_to_rgba8888_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint8_t *y_ptr = input;

	aipl_rgba8888_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_ptr + i * pitch;
		aipl_rgba8888_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_i400_to_rgba8888(dst, y_src);

			++y_src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_RGBA4444) &&                                                    \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i400_to_rgba4444_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint8_t *y_ptr = input;

	aipl_rgba4444_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_ptr + i * pitch;
		aipl_rgba4444_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_i400_to_rgba4444(dst, y_src);

			++y_src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_RGBA5551) &&                                                    \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i400_to_rgba5551_default(const void *input, void *output,
							 uint32_t pitch, uint32_t width,
							 uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint8_t *y_ptr = input;

	aipl_rgba5551_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_ptr + i * pitch;
		aipl_rgba5551_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_i400_to_rgba5551(dst, y_src);

			++y_src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_BGR888) &&                                                      \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i400_to_bgr888_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	return aipl_color_convert_alpha8_to_24bit_default(input, output, pitch, width, height, 2, 1,
							  0);
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_RGB888) &&                                                      \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i400_to_rgb888_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	return aipl_color_convert_alpha8_to_24bit_default(input, output, pitch, width, height, 0, 1,
							  2);
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_RGB565) &&                                                      \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i400_to_rgb565_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint8_t *y_ptr = input;

	aipl_rgb565_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_ptr + i * pitch;
		aipl_rgb565_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_i400_to_rgb565(dst, y_src);

			++y_src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_YV12)
aipl_error_t aipl_color_convert_i400_to_yv12_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	return aipl_color_convert_alpha8_to_yuv_planar_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_I420)
aipl_error_t aipl_color_convert_i400_to_i420_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	return aipl_color_convert_alpha8_to_yuv_planar_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_I422)
aipl_error_t aipl_color_convert_i400_to_i422_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint8_t *y_src_ptr = input;

	uint32_t yuv_size = pitch * height;
	uint8_t *y_dst_ptr = output;
	uint8_t *u_dst_ptr = y_dst_ptr + yuv_size;
	uint8_t *v_dst_ptr = u_dst_ptr + yuv_size / 2;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_src_ptr + i * pitch;
		uint8_t *y_dst = y_dst_ptr + i * width;

		memcpy(y_dst, y_src, width);

		uint8_t *v_dst = v_dst_ptr + i * width / 2;
		uint8_t *u_dst = u_dst_ptr + i * width / 2;

		memset(u_dst, 0x80, width / 2);
		memset(v_dst, 0x80, width / 2);
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_NV12)
aipl_error_t aipl_color_convert_i400_to_nv12_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	return aipl_color_convert_alpha8_to_yuv_semi_planar_default(input, output, pitch, width,
								    height);
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_I444)
aipl_error_t aipl_color_convert_i400_to_i444_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint8_t *y_src_ptr = input;

	uint32_t yuv_size = pitch * height;
	uint8_t *y_dst_ptr = output;
	uint8_t *u_dst_ptr = y_dst_ptr + yuv_size;
	uint8_t *v_dst_ptr = u_dst_ptr + yuv_size;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_src_ptr + i * pitch;
		uint8_t *y_dst = y_dst_ptr + i * width;

		memcpy(y_dst, y_src, width);

		uint8_t *v_dst = v_dst_ptr + i * width;
		uint8_t *u_dst = u_dst_ptr + i * width;

		memset(u_dst, 0x80, width);
		memset(v_dst, 0x80, width);
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_NV21)
aipl_error_t aipl_color_convert_i400_to_nv21_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	return aipl_color_convert_alpha8_to_yuv_semi_planar_default(input, output, pitch, width,
								    height);
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_YUY2) &&                                                        \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i400_to_yuy2_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	const uint8_t *y_src = input;

	uint8_t *y_dst = output;
	uint8_t *u_dst = y_dst + 1;
	uint8_t *v_dst = u_dst + 2;

	return aipl_color_convert_alpha8_to_yuv_packed_default(y_src, y_dst, u_dst, v_dst, pitch,
							       width, height);
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_UYVY) &&                                                        \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i400_to_uyvy_default(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	const uint8_t *y_src = input;

	uint8_t *u_dst = output;
	uint8_t *y_dst = u_dst + 1;
	uint8_t *v_dst = u_dst + 2;

	return aipl_color_convert_alpha8_to_yuv_packed_default(y_src, y_dst, u_dst, v_dst, pitch,
							       width, height);
}
#endif
#endif

/**********************
 *   STATIC FUNCTIONS
 **********************/
#if (AIPL_CONVERT_ALPHA8_I400 & TO_BGR888 | AIPL_CONVERT_ALPHA8_I400 & TO_RGB888) &&               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_alpha8_to_24bit_default(const void *input, void *output,
							uint32_t pitch, uint32_t width,
							uint32_t height, uint8_t r_offset,
							uint8_t g_offset, uint8_t b_offset)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint8_t *y_ptr = input;

	uint8_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_ptr + i * pitch;
		uint8_t *dst = dst_ptr + i * width * 3;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_i400_to_24bit(dst, y_src);

			++y_src;
			dst += 3;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ARGB8888 & TO_BGR888 | AIPL_CONVERT_ARGB8888 & TO_RGB888) &&                     \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb8888_to_24bit_default(const void *input, void *output,
							  uint32_t pitch, uint32_t width,
							  uint32_t height, uint8_t r_offset,
							  uint8_t g_offset, uint8_t b_offset)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_argb8888_px_t *src_ptr = input;
	uint8_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_argb8888_px_t *src = src_ptr + i * pitch;
		uint8_t *dst = dst_ptr + i * width * 3;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_argb8888_to_24bit(dst, src, r_offset, g_offset, b_offset);

			++src;
			dst += 3;
		}
	}

	return AIPL_ERR_OK;
}
#endif
#if (AIPL_CONVERT_ARGB4444 & TO_BGR888 | AIPL_CONVERT_ARGB4444 & TO_RGB888) &&                     \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb4444_to_24bit_default(const void *input, void *output,
							  uint32_t pitch, uint32_t width,
							  uint32_t height, uint8_t r_offset,
							  uint8_t g_offset, uint8_t b_offset)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_argb4444_px_t *src_ptr = input;
	uint8_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_argb4444_px_t *src = src_ptr + i * pitch;
		uint8_t *dst = dst_ptr + i * width * 3;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_argb4444_to_24bit(dst, src, r_offset, g_offset, b_offset);

			++src;
			dst += 3;
		}
	}

	return AIPL_ERR_OK;
}
#endif
#if (AIPL_CONVERT_ARGB1555 & TO_BGR888 | AIPL_CONVERT_ARGB1555 & TO_RGB888) &&                     \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb1555_to_24bit_default(const void *input, void *output,
							  uint32_t pitch, uint32_t width,
							  uint32_t height, uint8_t r_offset,
							  uint8_t g_offset, uint8_t b_offset)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_argb1555_px_t *src_ptr = input;
	uint8_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_argb1555_px_t *src = src_ptr + i * pitch;
		uint8_t *dst = dst_ptr + i * width * 3;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_argb1555_to_24bit(dst, src, r_offset, g_offset, b_offset);

			++src;
			dst += 3;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGBA8888 & TO_BGR888 | AIPL_CONVERT_RGBA8888 & TO_RGB888) &&                     \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba8888_to_24bit_default(const void *input, void *output,
							  uint32_t pitch, uint32_t width,
							  uint32_t height, uint8_t r_offset,
							  uint8_t g_offset, uint8_t b_offset)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgba8888_px_t *src_ptr = input;
	uint8_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgba8888_px_t *src = src_ptr + i * pitch;
		uint8_t *dst = dst_ptr + i * width * 3;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgba8888_to_24bit(dst, src, r_offset, g_offset, b_offset);

			++src;
			dst += 3;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGBA4444 & TO_BGR888 | AIPL_CONVERT_RGBA4444 & TO_RGB888) &&                     \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba4444_to_24bit_default(const void *input, void *output,
							  uint32_t pitch, uint32_t width,
							  uint32_t height, uint8_t r_offset,
							  uint8_t g_offset, uint8_t b_offset)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgba4444_px_t *src_ptr = input;
	uint8_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgba4444_px_t *src = src_ptr + i * pitch;
		uint8_t *dst = dst_ptr + i * width * 3;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgba4444_to_24bit(dst, src, r_offset, g_offset, b_offset);

			++src;
			dst += 3;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGBA5551 & TO_BGR888 | AIPL_CONVERT_RGBA5551 & TO_RGB888) &&                     \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba5551_to_24bit_default(const void *input, void *output,
							  uint32_t pitch, uint32_t width,
							  uint32_t height, uint8_t r_offset,
							  uint8_t g_offset, uint8_t b_offset)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgba5551_px_t *src_ptr = input;
	uint8_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgba5551_px_t *src = src_ptr + i * pitch;
		uint8_t *dst = dst_ptr + i * width * 3;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgba5551_to_24bit(dst, src, r_offset, g_offset, b_offset);

			++src;
			dst += 3;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGB565 & TO_BGR888 | AIPL_CONVERT_RGB565 & TO_RGB888) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb565_to_24bit_default(const void *input, void *output,
							uint32_t pitch, uint32_t width,
							uint32_t height, uint8_t r_offset,
							uint8_t g_offset, uint8_t b_offset)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgb565_px_t *src_ptr = input;
	uint8_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgb565_px_t *src = src_ptr + i * pitch;
		uint8_t *dst = dst_ptr + i * width * 3;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgb565_to_24bit(dst, src, r_offset, g_offset, b_offset);

			++src;
			dst += 3;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_BGR888 | AIPL_CONVERT_BGR888 & TO_RGB888) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_24bit_to_24bit_default(const void *input, void *output,
						       uint32_t pitch, uint32_t width,
						       uint32_t height, uint8_t r_in_offset,
						       uint8_t g_in_offset, uint8_t b_in_offset,
						       uint8_t r_out_offset, uint8_t g_out_offset,
						       uint8_t b_out_offset)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint8_t *src_ptr = input;
	uint8_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *src = src_ptr + i * pitch * 3;
		uint8_t *dst = dst_ptr + i * width * 3;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_24bit_to_24bit(dst, src, r_in_offset, g_in_offset, b_in_offset,
						    r_out_offset, g_out_offset, b_out_offset);

			src += 3;
			dst += 3;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_ALPHA8_I400 | AIPL_CONVERT_BGR888 & TO_ALPHA8_I400) &&               \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_24bit_to_alpha8_default(const void *input, void *output,
							uint32_t pitch, uint32_t width,
							uint32_t height, uint8_t r_offset,
							uint8_t g_offset, uint8_t b_offset)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint8_t *src_ptr = input;
	uint8_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *src = src_ptr + i * pitch * 3;
		uint8_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_24bit_to_yuv_y(dst, src, r_offset, g_offset, b_offset);

			src += 3;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_ARGB8888 | AIPL_CONVERT_BGR888 & TO_ARGB8888) &&                     \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_24bit_to_argb8888_default(const void *input, void *output,
							  uint32_t pitch, uint32_t width,
							  uint32_t height, uint8_t r_offset,
							  uint8_t g_offset, uint8_t b_offset)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint8_t *src_ptr = input;
	aipl_argb8888_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *src = src_ptr + i * pitch * 3;
		aipl_argb8888_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_24bit_to_argb8888(dst, src, r_offset, g_offset, b_offset);

			src += 3;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_ARGB4444 | AIPL_CONVERT_BGR888 & TO_ARGB4444) &&                     \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_24bit_to_argb4444_default(const void *input, void *output,
							  uint32_t pitch, uint32_t width,
							  uint32_t height, uint8_t r_offset,
							  uint8_t g_offset, uint8_t b_offset)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint8_t *src_ptr = input;
	aipl_argb4444_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *src = src_ptr + i * pitch * 3;
		aipl_argb4444_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_24bit_to_argb4444(dst, src, r_offset, g_offset, b_offset);

			src += 3;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_ARGB1555 | AIPL_CONVERT_BGR888 & TO_ARGB1555) &&                     \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_24bit_to_argb1555_default(const void *input, void *output,
							  uint32_t pitch, uint32_t width,
							  uint32_t height, uint8_t r_offset,
							  uint8_t g_offset, uint8_t b_offset)
{

	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint8_t *src_ptr = input;
	aipl_argb1555_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *src = src_ptr + i * pitch * 3;
		aipl_argb1555_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_24bit_to_argb1555(dst, src, r_offset, g_offset, b_offset);

			src += 3;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_RGBA8888 | AIPL_CONVERT_BGR888 & TO_RGBA8888) &&                     \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_24bit_to_rgba8888_default(const void *input, void *output,
							  uint32_t pitch, uint32_t width,
							  uint32_t height, uint8_t r_offset,
							  uint8_t g_offset, uint8_t b_offset)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint8_t *src_ptr = input;
	aipl_rgba8888_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *src = src_ptr + i * pitch * 3;
		aipl_rgba8888_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_24bit_to_rgba8888(dst, src, r_offset, g_offset, b_offset);

			src += 3;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_RGBA4444 | AIPL_CONVERT_BGR888 & TO_RGBA4444) &&                     \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_24bit_to_rgba4444_default(const void *input, void *output,
							  uint32_t pitch, uint32_t width,
							  uint32_t height, uint8_t r_offset,
							  uint8_t g_offset, uint8_t b_offset)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint8_t *src_ptr = input;
	aipl_rgba4444_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *src = src_ptr + i * pitch * 3;
		aipl_rgba4444_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_24bit_to_rgba4444(dst, src, r_offset, g_offset, b_offset);

			src += 3;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_RGBA5551 | AIPL_CONVERT_BGR888 & TO_RGBA5551) &&                     \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_24bit_to_rgba5551_default(const void *input, void *output,
							  uint32_t pitch, uint32_t width,
							  uint32_t height, uint8_t r_offset,
							  uint8_t g_offset, uint8_t b_offset)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint8_t *src_ptr = input;
	aipl_rgba5551_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *src = src_ptr + i * pitch * 3;
		aipl_rgba5551_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_24bit_to_rgba5551(dst, src, r_offset, g_offset, b_offset);

			src += 3;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_RGB565 | AIPL_CONVERT_BGR888 & TO_RGB565) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_24bit_to_rgb565_default(const void *input, void *output,
							uint32_t pitch, uint32_t width,
							uint32_t height, uint8_t r_offset,
							uint8_t g_offset, uint8_t b_offset)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint8_t *src_ptr = input;
	aipl_rgb565_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *src = src_ptr + i * pitch * 3;
		aipl_rgb565_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_24bit_to_rgb565(dst, src, r_offset, g_offset, b_offset);

			src += 3;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ARGB8888 & TO_YV12 | AIPL_CONVERT_ARGB8888 & TO_I420) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb8888_to_yuv_planar_default(const void *input, uint32_t pitch,
							       uint32_t width, uint32_t height,
							       uint8_t *y_ptr, uint8_t *u_ptr,
							       uint8_t *v_ptr)
{
	if (input == NULL || y_ptr == NULL || u_ptr == NULL || v_ptr == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_argb8888_px_t *src_ptr = input;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_argb8888_px_t *src = src_ptr + i * pitch;
		uint8_t *y_dst = y_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_argb8888_to_yuv_y(y_dst, src);

			++src;
			++y_dst;
		}
	}

	for (uint32_t i = 0; i < height; i += 2) {
		const aipl_argb8888_px_t *src = src_ptr + i * pitch;
		uint8_t *u_dst = u_ptr + i / 2 * width / 2;
		uint8_t *v_dst = v_ptr + i / 2 * width / 2;

		for (uint32_t j = 0; j < width / 2; ++j) {
			aipl_cnvt_px_argb8888_to_yuv_uv(u_dst, v_dst, src);

			src += 2;
			++u_dst;
			++v_dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ARGB8888 & TO_NV12 | AIPL_CONVERT_ARGB8888 & TO_NV21) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb8888_to_yuv_semi_planar_default(const void *input,
								    uint32_t pitch, uint32_t width,
								    uint32_t height, uint8_t *y_ptr,
								    uint8_t *u_ptr, uint8_t *v_ptr)
{
	if (input == NULL || y_ptr == NULL || u_ptr == NULL || v_ptr == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_argb8888_px_t *src_ptr = input;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_argb8888_px_t *src = src_ptr + i * pitch;
		uint8_t *y_dst = y_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_argb8888_to_yuv_y(y_dst, src);

			++src;
			++y_dst;
		}
	}

	for (uint32_t i = 0; i < height; i += 2) {
		const aipl_argb8888_px_t *src = src_ptr + i * pitch;
		uint8_t *u_dst = u_ptr + i * width / 2;
		uint8_t *v_dst = v_ptr + i * width / 2;

		for (uint32_t j = 0; j < width / 2; ++j) {
			aipl_cnvt_px_argb8888_to_yuv_uv(u_dst, v_dst, src);

			src += 2;
			u_dst += 2;
			v_dst += 2;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ARGB8888 & TO_YUY2 | AIPL_CONVERT_ARGB8888 & TO_UYVY) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb8888_to_yuv_packed_default(const void *input, uint32_t pitch,
							       uint32_t width, uint32_t height,
							       uint8_t *y_ptr, uint8_t *u_ptr,
							       uint8_t *v_ptr)
{
	if (input == NULL || y_ptr == NULL || u_ptr == NULL || v_ptr == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_argb8888_px_t *src_ptr = input;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_argb8888_px_t *src = src_ptr + i * pitch;
		uint8_t *y_dst = y_ptr + i * width * 2;
		uint8_t *u_dst = u_ptr + i * width * 2;
		uint8_t *v_dst = v_ptr + i * width * 2;

		for (uint32_t j = 0; j < width; j += 2) {
			aipl_cnvt_px_argb8888_to_yuv(y_dst, u_dst, v_dst, src);

			++src;
			y_dst += 2;
			u_dst += 4;
			v_dst += 4;

			aipl_cnvt_px_argb8888_to_yuv_y(y_dst, src);

			++src;
			y_dst += 2;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ARGB4444 & TO_YV12 | AIPL_CONVERT_ARGB4444 & TO_I420) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb4444_to_yuv_planar_default(const void *input, uint32_t pitch,
							       uint32_t width, uint32_t height,
							       uint8_t *y_ptr, uint8_t *u_ptr,
							       uint8_t *v_ptr)
{
	if (input == NULL || y_ptr == NULL || u_ptr == NULL || v_ptr == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_argb4444_px_t *src_ptr = input;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_argb4444_px_t *src = src_ptr + i * pitch;
		uint8_t *y_dst = y_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_argb4444_to_yuv_y(y_dst, src);

			++src;
			++y_dst;
		}
	}

	for (uint32_t i = 0; i < height; i += 2) {
		const aipl_argb4444_px_t *src = src_ptr + i * pitch;
		uint8_t *u_dst = u_ptr + i / 2 * width / 2;
		uint8_t *v_dst = v_ptr + i / 2 * width / 2;

		for (uint32_t j = 0; j < width / 2; ++j) {
			aipl_cnvt_px_argb4444_to_yuv_uv(u_dst, v_dst, src);

			src += 2;
			++u_dst;
			++v_dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ARGB4444 & TO_NV12 | AIPL_CONVERT_ARGB4444 & TO_NV21) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb4444_to_yuv_semi_planar_default(const void *input,
								    uint32_t pitch, uint32_t width,
								    uint32_t height, uint8_t *y_ptr,
								    uint8_t *u_ptr, uint8_t *v_ptr)
{
	if (input == NULL || y_ptr == NULL || u_ptr == NULL || v_ptr == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_argb4444_px_t *src_ptr = input;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_argb4444_px_t *src = src_ptr + i * pitch;
		uint8_t *y_dst = y_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_argb4444_to_yuv_y(y_dst, src);

			++src;
			++y_dst;
		}
	}

	for (uint32_t i = 0; i < height; i += 2) {
		const aipl_argb4444_px_t *src = src_ptr + i * pitch;
		uint8_t *u_dst = u_ptr + i * width / 2;
		uint8_t *v_dst = v_ptr + i * width / 2;

		for (uint32_t j = 0; j < width / 2; ++j) {
			aipl_cnvt_px_argb4444_to_yuv_uv(u_dst, v_dst, src);

			src += 2;
			u_dst += 2;
			v_dst += 2;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ARGB4444 & TO_YUY2 | AIPL_CONVERT_ARGB4444 & TO_UYVY) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb4444_to_yuv_packed_default(const void *input, uint32_t pitch,
							       uint32_t width, uint32_t height,
							       uint8_t *y_ptr, uint8_t *u_ptr,
							       uint8_t *v_ptr)
{
	if (input == NULL || y_ptr == NULL || u_ptr == NULL || v_ptr == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_argb4444_px_t *src_ptr = input;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_argb4444_px_t *src = src_ptr + i * pitch;

		uint8_t *y_dst = y_ptr + i * width * 2;
		uint8_t *v_dst = v_ptr + i * width * 2;
		uint8_t *u_dst = u_ptr + i * width * 2;

		for (uint32_t j = 0; j < width * 2; j += 4) {
			aipl_cnvt_px_argb4444_to_yuv(y_dst, u_dst, v_dst, src);

			++src;
			y_dst += 2;
			u_dst += 4;
			v_dst += 4;

			aipl_cnvt_px_argb4444_to_yuv_y(y_dst, src);

			++src;
			y_dst += 2;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ARGB1555 & TO_YV12 | AIPL_CONVERT_ARGB1555 & TO_I420) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb1555_to_yuv_planar_default(const void *input, uint32_t pitch,
							       uint32_t width, uint32_t height,
							       uint8_t *y_ptr, uint8_t *u_ptr,
							       uint8_t *v_ptr)
{
	if (input == NULL || y_ptr == NULL || u_ptr == NULL || v_ptr == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_argb1555_px_t *src_ptr = input;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_argb1555_px_t *src = src_ptr + i * pitch;
		uint8_t *y_dst = y_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_argb1555_to_yuv_y(y_dst, src);

			++src;
			++y_dst;
		}
	}

	for (uint32_t i = 0; i < height; i += 2) {
		const aipl_argb1555_px_t *src = src_ptr + i * pitch;
		uint8_t *u_dst = u_ptr + i / 2 * width / 2;
		uint8_t *v_dst = v_ptr + i / 2 * width / 2;

		for (uint32_t j = 0; j < width / 2; ++j) {
			aipl_cnvt_px_argb1555_to_yuv_uv(u_dst, v_dst, src);

			src += 2;
			++u_dst;
			++v_dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ARGB1555 & TO_NV12 | AIPL_CONVERT_ARGB1555 & TO_NV21) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb1555_to_yuv_semi_planar_default(const void *input,
								    uint32_t pitch, uint32_t width,
								    uint32_t height, uint8_t *y_ptr,
								    uint8_t *u_ptr, uint8_t *v_ptr)
{
	if (input == NULL || y_ptr == NULL || u_ptr == NULL || v_ptr == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_argb1555_px_t *src_ptr = input;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_argb1555_px_t *src = src_ptr + i * pitch;
		uint8_t *y_dst = y_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_argb1555_to_yuv_y(y_dst, src);

			++src;
			++y_dst;
		}
	}

	for (uint32_t i = 0; i < height; i += 2) {
		const aipl_argb1555_px_t *src = src_ptr + i * pitch;
		uint8_t *u_dst = u_ptr + i * width / 2;
		uint8_t *v_dst = v_ptr + i * width / 2;

		for (uint32_t j = 0; j < width / 2; ++j) {
			aipl_cnvt_px_argb1555_to_yuv_uv(u_dst, v_dst, src);

			src += 2;
			u_dst += 2;
			v_dst += 2;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ARGB1555 & TO_YUY2 | AIPL_CONVERT_ARGB1555 & TO_UYVY) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_argb1555_to_yuv_packed_default(const void *input, uint32_t pitch,
							       uint32_t width, uint32_t height,
							       uint8_t *y_ptr, uint8_t *u_ptr,
							       uint8_t *v_ptr)
{
	if (input == NULL || y_ptr == NULL || u_ptr == NULL || v_ptr == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_argb1555_px_t *src_ptr = input;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_argb1555_px_t *src = src_ptr + i * pitch;

		uint8_t *y_dst = y_ptr + i * width * 2;
		uint8_t *v_dst = v_ptr + i * width * 2;
		uint8_t *u_dst = u_ptr + i * width * 2;

		for (uint32_t j = 0; j < width * 2; j += 4) {
			aipl_cnvt_px_argb1555_to_yuv(y_dst, u_dst, v_dst, src);

			++src;
			y_dst += 2;
			u_dst += 4;
			v_dst += 4;

			aipl_cnvt_px_argb1555_to_yuv_y(y_dst, src);

			++src;
			y_dst += 2;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGBA8888 & TO_YV12 | AIPL_CONVERT_RGBA8888 & TO_I420) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba8888_to_yuv_planar_default(const void *input, uint32_t pitch,
							       uint32_t width, uint32_t height,
							       uint8_t *y_ptr, uint8_t *u_ptr,
							       uint8_t *v_ptr)
{
	if (input == NULL || y_ptr == NULL || u_ptr == NULL || v_ptr == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgba8888_px_t *src_ptr = input;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgba8888_px_t *src = src_ptr + i * pitch;
		uint8_t *y_dst = y_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgba8888_to_yuv_y(y_dst, src);

			++src;
			++y_dst;
		}
	}

	for (uint32_t i = 0; i < height; i += 2) {
		const aipl_rgba8888_px_t *src = src_ptr + i * pitch;
		uint8_t *u_dst = u_ptr + i / 2 * width / 2;
		uint8_t *v_dst = v_ptr + i / 2 * width / 2;

		for (uint32_t j = 0; j < width / 2; ++j) {
			aipl_cnvt_px_rgba8888_to_yuv_uv(u_dst, v_dst, src);

			src += 2;
			++u_dst;
			++v_dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGBA8888 & TO_NV12 | AIPL_CONVERT_RGBA8888 & TO_NV21) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba8888_to_yuv_semi_planar_default(const void *input,
								    uint32_t pitch, uint32_t width,
								    uint32_t height, uint8_t *y_ptr,
								    uint8_t *u_ptr, uint8_t *v_ptr)
{
	if (input == NULL || y_ptr == NULL || u_ptr == NULL || v_ptr == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgba8888_px_t *src_ptr = input;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgba8888_px_t *src = src_ptr + i * pitch;
		uint8_t *y_dst = y_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgba8888_to_yuv_y(y_dst, src);

			++src;
			++y_dst;
		}
	}

	for (uint32_t i = 0; i < height; i += 2) {
		const aipl_rgba8888_px_t *src = src_ptr + i * pitch;
		uint8_t *u_dst = u_ptr + i * width / 2;
		uint8_t *v_dst = v_ptr + i * width / 2;

		for (uint32_t j = 0; j < width / 2; ++j) {
			aipl_cnvt_px_rgba8888_to_yuv_uv(u_dst, v_dst, src);

			src += 2;
			u_dst += 2;
			v_dst += 2;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGBA8888 & TO_YUY2 | AIPL_CONVERT_RGBA8888 & TO_UYVY) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba8888_to_yuv_packed_default(const void *input, uint32_t pitch,
							       uint32_t width, uint32_t height,
							       uint8_t *y_ptr, uint8_t *u_ptr,
							       uint8_t *v_ptr)
{
	if (input == NULL || y_ptr == NULL || u_ptr == NULL || v_ptr == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgba8888_px_t *src_ptr = input;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgba8888_px_t *src = src_ptr + i * pitch;

		uint8_t *y_dst = y_ptr + i * width * 2;
		uint8_t *v_dst = v_ptr + i * width * 2;
		uint8_t *u_dst = u_ptr + i * width * 2;

		for (uint32_t j = 0; j < width; j += 2) {
			aipl_cnvt_px_rgba8888_to_yuv(y_dst, u_dst, v_dst, src);

			++src;
			y_dst += 2;
			u_dst += 4;
			v_dst += 4;

			aipl_cnvt_px_rgba8888_to_yuv_y(y_dst, src);

			++src;
			y_dst += 2;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGBA4444 & TO_YV12 | AIPL_CONVERT_RGBA4444 & TO_I420) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba4444_to_yuv_planar_default(const void *input, uint32_t pitch,
							       uint32_t width, uint32_t height,
							       uint8_t *y_ptr, uint8_t *u_ptr,
							       uint8_t *v_ptr)
{
	if (input == NULL || y_ptr == NULL || u_ptr == NULL || v_ptr == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgba4444_px_t *src_ptr = input;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgba4444_px_t *src = src_ptr + i * pitch;
		uint8_t *y_dst = y_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgba4444_to_yuv_y(y_dst, src);

			++src;
			++y_dst;
		}
	}

	for (uint32_t i = 0; i < height; i += 2) {
		const aipl_rgba4444_px_t *src = src_ptr + i * pitch;
		uint8_t *u_dst = u_ptr + i / 2 * width / 2;
		uint8_t *v_dst = v_ptr + i / 2 * width / 2;

		for (uint32_t j = 0; j < width / 2; ++j) {
			aipl_cnvt_px_rgba4444_to_yuv_uv(u_dst, v_dst, src);

			src += 2;
			++u_dst;
			++v_dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGBA4444 & TO_NV12 | AIPL_CONVERT_RGBA4444 & TO_NV21) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba4444_to_yuv_semi_planar_default(const void *input,
								    uint32_t pitch, uint32_t width,
								    uint32_t height, uint8_t *y_ptr,
								    uint8_t *u_ptr, uint8_t *v_ptr)
{
	if (input == NULL || y_ptr == NULL || u_ptr == NULL || v_ptr == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgba4444_px_t *src_ptr = input;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgba4444_px_t *src = src_ptr + i * pitch;
		uint8_t *y_dst = y_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgba4444_to_yuv_y(y_dst, src);

			++src;
			++y_dst;
		}
	}

	for (uint32_t i = 0; i < height; i += 2) {
		const aipl_rgba4444_px_t *src = src_ptr + i * pitch;
		uint8_t *u_dst = u_ptr + i * width / 2;
		uint8_t *v_dst = v_ptr + i * width / 2;

		for (uint32_t j = 0; j < width / 2; ++j) {
			aipl_cnvt_px_rgba4444_to_yuv_uv(u_dst, v_dst, src);

			src += 2;
			u_dst += 2;
			v_dst += 2;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGBA4444 & TO_YUY2 | AIPL_CONVERT_RGBA4444 & TO_UYVY) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba4444_to_yuv_packed_default(const void *input, uint32_t pitch,
							       uint32_t width, uint32_t height,
							       uint8_t *y_ptr, uint8_t *u_ptr,
							       uint8_t *v_ptr)
{
	if (input == NULL || y_ptr == NULL || u_ptr == NULL || v_ptr == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgba4444_px_t *src_ptr = input;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgba4444_px_t *src = src_ptr + i * pitch;

		uint8_t *y_dst = y_ptr + i * width * 2;
		uint8_t *v_dst = v_ptr + i * width * 2;
		uint8_t *u_dst = u_ptr + i * width * 2;

		for (uint32_t j = 0; j < width * 2; j += 4) {
			aipl_cnvt_px_rgba4444_to_yuv(y_dst, u_dst, v_dst, src);

			++src;
			y_dst += 2;
			u_dst += 4;
			v_dst += 4;

			aipl_cnvt_px_rgba4444_to_yuv_y(y_dst, src);

			++src;
			y_dst += 2;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGBA5551 & TO_YV12 | AIPL_CONVERT_RGBA5551 & TO_I420) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba5551_to_yuv_planar_default(const void *input, uint32_t pitch,
							       uint32_t width, uint32_t height,
							       uint8_t *y_ptr, uint8_t *u_ptr,
							       uint8_t *v_ptr)
{
	if (input == NULL || y_ptr == NULL || u_ptr == NULL || v_ptr == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgba5551_px_t *src_ptr = input;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgba5551_px_t *src = src_ptr + i * pitch;
		uint8_t *y_dst = y_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgba5551_to_yuv_y(y_dst, src);

			++src;
			++y_dst;
		}
	}

	for (uint32_t i = 0; i < height; i += 2) {
		const aipl_rgba5551_px_t *src = src_ptr + i * pitch;
		uint8_t *u_dst = u_ptr + i / 2 * width / 2;
		uint8_t *v_dst = v_ptr + i / 2 * width / 2;

		for (uint32_t j = 0; j < width / 2; ++j) {
			aipl_cnvt_px_rgba5551_to_yuv_uv(u_dst, v_dst, src);

			src += 2;
			++u_dst;
			++v_dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGBA5551 & TO_NV12 | AIPL_CONVERT_RGBA5551 & TO_NV21) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba5551_to_yuv_semi_planar_default(const void *input,
								    uint32_t pitch, uint32_t width,
								    uint32_t height, uint8_t *y_ptr,
								    uint8_t *u_ptr, uint8_t *v_ptr)
{
	if (input == NULL || y_ptr == NULL || u_ptr == NULL || v_ptr == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgba5551_px_t *src_ptr = input;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgba5551_px_t *src = src_ptr + i * pitch;
		uint8_t *y_dst = y_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgba5551_to_yuv_y(y_dst, src);

			++src;
			++y_dst;
		}
	}

	for (uint32_t i = 0; i < height; i += 2) {
		const aipl_rgba5551_px_t *src = src_ptr + i * pitch;
		uint8_t *u_dst = u_ptr + i * width / 2;
		uint8_t *v_dst = v_ptr + i * width / 2;

		for (uint32_t j = 0; j < width / 2; ++j) {
			aipl_cnvt_px_rgba5551_to_yuv_uv(u_dst, v_dst, src);

			src += 2;
			u_dst += 2;
			v_dst += 2;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGBA5551 & TO_YUY2 | AIPL_CONVERT_RGBA5551 & TO_UYVY) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgba5551_to_yuv_packed_default(const void *input, uint32_t pitch,
							       uint32_t width, uint32_t height,
							       uint8_t *y_ptr, uint8_t *u_ptr,
							       uint8_t *v_ptr)
{
	if (input == NULL || y_ptr == NULL || u_ptr == NULL || v_ptr == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgba5551_px_t *src_ptr = input;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgba5551_px_t *src = src_ptr + i * pitch;

		uint8_t *y_dst = y_ptr + i * width * 2;
		uint8_t *v_dst = v_ptr + i * width * 2;
		uint8_t *u_dst = u_ptr + i * width * 2;

		for (uint32_t j = 0; j < width * 2; j += 4) {
			aipl_cnvt_px_rgba5551_to_yuv(y_dst, u_dst, v_dst, src);

			++src;
			y_dst += 2;
			u_dst += 4;
			v_dst += 4;

			aipl_cnvt_px_rgba5551_to_yuv_y(y_dst, src);

			++src;
			y_dst += 2;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_BGR888 & TO_YV12 | AIPL_CONVERT_BGR888 & TO_I420 |                               \
	 AIPL_CONVERT_RGB888 & TO_YV12 | AIPL_CONVERT_RGB888 & TO_I420) &&                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_24bit_to_yuv_planar_default(const void *input, uint32_t pitch,
							    uint32_t width, uint32_t height,
							    uint8_t *y_ptr, uint8_t *u_ptr,
							    uint8_t *v_ptr, uint8_t r_offset,
							    uint8_t g_offset, uint8_t b_offset)
{
	if (input == NULL || y_ptr == NULL || u_ptr == NULL || v_ptr == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint8_t *src_ptr = input;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *src = src_ptr + i * pitch * 3;
		uint8_t *y_dst = y_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_24bit_to_yuv_y(y_dst, src, r_offset, g_offset, b_offset);

			src += 3;
			++y_dst;
		}
	}

	for (uint32_t i = 0; i < height; i += 2) {
		const uint8_t *src = src_ptr + i * pitch * 3;
		uint8_t *u_dst = u_ptr + i / 2 * width / 2;
		uint8_t *v_dst = v_ptr + i / 2 * width / 2;

		for (uint32_t j = 0; j < width / 2; ++j) {
			aipl_cnvt_px_24bit_to_yuv_uv(u_dst, v_dst, src, r_offset, g_offset,
						     b_offset);

			src += 6;
			++u_dst;
			++v_dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_BGR888 & TO_NV12 | AIPL_CONVERT_BGR888 & TO_NV21 |                               \
	 AIPL_CONVERT_RGB888 & TO_NV12 | AIPL_CONVERT_RGB888 & TO_NV21) &&                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_24bit_to_yuv_semi_planar_default(const void *input, uint32_t pitch,
								 uint32_t width, uint32_t height,
								 uint8_t *y_ptr, uint8_t *u_ptr,
								 uint8_t *v_ptr, uint8_t r_offset,
								 uint8_t g_offset, uint8_t b_offset)
{
	if (input == NULL || y_ptr == NULL || u_ptr == NULL || v_ptr == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint8_t *src_ptr = input;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *src = src_ptr + i * pitch * 3;
		uint8_t *y_dst = y_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_24bit_to_yuv_y(y_dst, src, r_offset, g_offset, b_offset);

			src += 3;
			++y_dst;
		}
	}

	for (uint32_t i = 0; i < height; i += 2) {
		const uint8_t *src = src_ptr + i * pitch * 3;
		uint8_t *u_dst = u_ptr + i * width / 2;
		uint8_t *v_dst = v_ptr + i * width / 2;

		for (uint32_t j = 0; j < width / 2; ++j) {
			aipl_cnvt_px_24bit_to_yuv_uv(u_dst, v_dst, src, r_offset, g_offset,
						     b_offset);

			src += 6;
			u_dst += 2;
			v_dst += 2;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_YUY2 | AIPL_CONVERT_RGB888 & TO_UYVY) &&                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb888_to_yuv_packed_default(const void *input, uint32_t pitch,
							     uint32_t width, uint32_t height,
							     uint8_t *y_ptr, uint8_t *u_ptr,
							     uint8_t *v_ptr)
{
	if (input == NULL || y_ptr == NULL || u_ptr == NULL || v_ptr == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint8_t *src_ptr = input;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *src = src_ptr + i * pitch * 3;

		uint8_t *y_dst = y_ptr + i * width * 2;
		uint8_t *v_dst = v_ptr + i * width * 2;
		uint8_t *u_dst = u_ptr + i * width * 2;

		for (uint32_t j = 0; j < width; j += 2) {
			aipl_cnvt_px_24bit_to_yuv(y_dst, u_dst, v_dst, src, 0, 1, 2);

			src += 3;
			y_dst += 2;
			u_dst += 4;
			v_dst += 4;

			aipl_cnvt_px_24bit_to_yuv_y(y_dst, src, 0, 1, 2);

			src += 3;
			y_dst += 2;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_BGR888 & TO_YUY2 | AIPL_CONVERT_BGR888 & TO_UYVY) &&                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_bgr888_to_yuv_packed_default(const void *input, uint32_t pitch,
							     uint32_t width, uint32_t height,
							     uint8_t *y_ptr, uint8_t *u_ptr,
							     uint8_t *v_ptr)
{
	if (input == NULL || y_ptr == NULL || u_ptr == NULL || v_ptr == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint8_t *src_ptr = input;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *src = src_ptr + i * pitch * 3;

		uint8_t *y_dst = y_ptr + i * width * 2;
		uint8_t *v_dst = v_ptr + i * width * 2;
		uint8_t *u_dst = u_ptr + i * width * 2;

		for (uint32_t j = 0; j < width; j += 2) {
			aipl_cnvt_px_24bit_to_yuv(y_dst, u_dst, v_dst, src, 2, 1, 0);

			src += 3;
			y_dst += 2;
			u_dst += 4;
			v_dst += 4;

			aipl_cnvt_px_24bit_to_yuv_y(y_dst, src, 2, 1, 0);

			src += 3;
			y_dst += 2;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_I422 & TO_BGR888 | AIPL_CONVERT_I422 & TO_RGB888) &&                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i422_to_24bit_default(const void *input, void *output,
						      uint32_t pitch, uint32_t width,
						      uint32_t height, uint8_t r_offset,
						      uint8_t g_offset, uint8_t b_offset)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *u_ptr = y_ptr + yuv_size;
	const uint8_t *v_ptr = u_ptr + yuv_size / 2;

	uint8_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_ptr + i * pitch;
		const uint8_t *u_src = u_ptr + i * pitch / 2;
		const uint8_t *v_src = v_ptr + i * pitch / 2;
		uint8_t *dst = dst_ptr + i * width * 3;

		for (uint32_t j = 0; j < width; j += 2) {
			uint32_t j2 = j / 2;
			int32_t c0, c1;
			int32_t r, g, b;

			aipl_pre_cnvt_px_yuv_to_rgb(&r, &g, &b, u_src[j2], v_src[j2]);
			aipl_pre_cnvt_2px_y(&c0, &c1, y_src[j], y_src[j + 1]);

			aipl_cnvt_px_yuv_to_24bit(dst + j * 3, c0, r, g, b, r_offset, g_offset,
						  b_offset);
			aipl_cnvt_px_yuv_to_24bit(dst + (j + 1) * 3, c1, r, g, b, r_offset,
						  g_offset, b_offset);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_I444 & TO_BGR888 | AIPL_CONVERT_I444 & TO_RGB888) &&                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_i444_to_24bit_default(const void *input, void *output,
						      uint32_t pitch, uint32_t width,
						      uint32_t height, uint8_t r_offset,
						      uint8_t g_offset, uint8_t b_offset)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = pitch * height;
	const uint8_t *y_ptr = input;
	const uint8_t *u_ptr = y_ptr + yuv_size;
	const uint8_t *v_ptr = u_ptr + yuv_size;

	uint8_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_ptr + i * pitch;
		const uint8_t *u_src = u_ptr + i * pitch;
		const uint8_t *v_src = v_ptr + i * pitch;
		uint8_t *dst = dst_ptr + i * width * 3;

		for (uint32_t j = 0; j < width; ++j) {
			int32_t c;
			int32_t r, g, b;

			aipl_pre_cnvt_px_yuv_to_rgb(&r, &g, &b, u_src[j], v_src[j]);
			aipl_pre_cnvt_px_y(&c, y_src[j]);

			aipl_cnvt_px_yuv_to_24bit(dst + j * 3, c, r, g, b, r_offset, g_offset,
						  b_offset);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGB565 & TO_YV12 | AIPL_CONVERT_RGB565 & TO_I420) &&                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb565_to_yuv_planar_default(const void *input, uint32_t pitch,
							     uint32_t width, uint32_t height,
							     uint8_t *y_ptr, uint8_t *u_ptr,
							     uint8_t *v_ptr)
{
	if (input == NULL || y_ptr == NULL || u_ptr == NULL || v_ptr == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgb565_px_t *src_ptr = input;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgb565_px_t *src = src_ptr + i * pitch;
		uint8_t *y_dst = y_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgb565_to_yuv_y(y_dst, src);

			++src;
			++y_dst;
		}
	}

	for (uint32_t i = 0; i < height; i += 2) {
		const aipl_rgb565_px_t *src = src_ptr + i * pitch;
		uint8_t *u_dst = u_ptr + i / 2 * width / 2;
		uint8_t *v_dst = v_ptr + i / 2 * width / 2;

		for (uint32_t j = 0; j < width / 2; ++j) {
			aipl_cnvt_px_rgb565_to_yuv_uv(u_dst, v_dst, src);

			src += 2;
			++u_dst;
			++v_dst;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGB565 & TO_NV12 | AIPL_CONVERT_RGB565 & TO_NV21) &&                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb565_to_yuv_semi_planar_default(const void *input, uint32_t pitch,
								  uint32_t width, uint32_t height,
								  uint8_t *y_ptr, uint8_t *u_ptr,
								  uint8_t *v_ptr)
{
	if (input == NULL || y_ptr == NULL || u_ptr == NULL || v_ptr == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgb565_px_t *src_ptr = input;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgb565_px_t *src = src_ptr + i * pitch;
		uint8_t *y_dst = y_ptr + i * width;

		for (uint32_t j = 0; j < width; ++j) {
			aipl_cnvt_px_rgb565_to_yuv_y(y_dst, src);

			++src;
			++y_dst;
		}
	}

	for (uint32_t i = 0; i < height; i += 2) {
		const aipl_rgb565_px_t *src = src_ptr + i * pitch;
		uint8_t *u_dst = u_ptr + i * width / 2;
		uint8_t *v_dst = v_ptr + i * width / 2;

		for (uint32_t j = 0; j < width / 2; ++j) {
			aipl_cnvt_px_rgb565_to_yuv_uv(u_dst, v_dst, src);

			src += 2;
			u_dst += 2;
			v_dst += 2;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_RGB565 & TO_YUY2 | AIPL_CONVERT_RGB565 & TO_UYVY) &&                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_rgb565_to_yuv_packed_default(const void *input, uint32_t pitch,
							     uint32_t width, uint32_t height,
							     uint8_t *y_ptr, uint8_t *u_ptr,
							     uint8_t *v_ptr)
{
	if (input == NULL || y_ptr == NULL || u_ptr == NULL || v_ptr == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgb565_px_t *src_ptr = input;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgb565_px_t *src = src_ptr + i * pitch;

		uint8_t *y_dst = y_ptr + i * width * 2;
		uint8_t *v_dst = v_ptr + i * width * 2;
		uint8_t *u_dst = u_ptr + i * width * 2;

		for (uint32_t j = 0; j < width * 2; j += 4) {
			aipl_cnvt_px_rgb565_to_yuv(y_dst, u_dst, v_dst, src);

			++src;
			y_dst += 2;
			u_dst += 4;
			v_dst += 4;

			aipl_cnvt_px_rgb565_to_yuv_y(y_dst, src);

			++src;
			y_dst += 2;
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_YV12 & TO_ALPHA8_I400 | AIPL_CONVERT_I420 & TO_ALPHA8_I400 |                     \
	 AIPL_CONVERT_NV12 & TO_ALPHA8_I400 | AIPL_CONVERT_NV21 & TO_ALPHA8_I400 |                     \
	 AIPL_CONVERT_I422 & TO_ALPHA8_I400 | AIPL_CONVERT_I444 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_yuv_planar_to_alpha8_default(const void *input, void *output,
							     uint32_t pitch, uint32_t width,
							     uint32_t height)
{
	return aipl_color_convert_i400_to_alpha8_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_ALPHA8_I400 | AIPL_CONVERT_UYVY & TO_ALPHA8_I400) &&                   \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_packed_to_alpha8_default(const void *input, void *output,
							     uint32_t pitch, uint32_t width,
							     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint8_t *y_ptr = input;
	uint8_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_ptr + i * pitch * 2;
		uint8_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width * 2; j += 2) {
			dst[j / 2] = y_src[j];
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_YV12 & TO_ARGB8888 | AIPL_CONVERT_I420 & TO_ARGB8888) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_planar_to_argb8888_default(const uint8_t *y_ptr,
							       const uint8_t *u_ptr,
							       const uint8_t *v_ptr, void *output,
							       uint32_t pitch, uint32_t width,
							       uint32_t height)
{
	if (output == NULL || y_ptr == NULL || u_ptr == NULL || v_ptr == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	aipl_argb8888_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; i += 2) {
		const uint8_t *y_src0 = y_ptr + i * pitch;
		const uint8_t *y_src1 = y_src0 + pitch;
		const uint8_t *u_src = u_ptr + i / 2 * pitch / 2;
		const uint8_t *v_src = v_ptr + i / 2 * pitch / 2;
		aipl_argb8888_px_t *dst0 = dst_ptr + i * width;
		aipl_argb8888_px_t *dst1 = dst0 + width;

		for (uint32_t j = 0; j < width; j += 2) {
			uint32_t j2 = j / 2;
			int32_t c00, c01, c10, c11;
			int32_t r, g, b;

			aipl_pre_cnvt_px_yuv_to_rgb(&r, &g, &b, u_src[j2], v_src[j2]);
			aipl_pre_cnvt_2x2px_y(&c00, &c01, &c10, &c11, y_src0[j], y_src0[j + 1],
					      y_src1[j], y_src1[j + 1]);

			aipl_cnvt_px_yuv_to_argb8888(dst0 + j, c00, r, g, b);
			aipl_cnvt_px_yuv_to_argb8888(dst0 + j + 1, c01, r, g, b);
			aipl_cnvt_px_yuv_to_argb8888(dst1 + j, c10, r, g, b);
			aipl_cnvt_px_yuv_to_argb8888(dst1 + j + 1, c11, r, g, b);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_NV12 & TO_ARGB8888 | AIPL_CONVERT_NV21 & TO_ARGB8888) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_semi_planar_to_argb8888_default(const uint8_t *y_ptr,
								    const uint8_t *u_ptr,
								    const uint8_t *v_ptr,
								    void *output, uint32_t pitch,
								    uint32_t width, uint32_t height)
{
	if (output == NULL || y_ptr == NULL || u_ptr == NULL || v_ptr == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	aipl_argb8888_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; i += 2) {
		const uint8_t *y_src0 = y_ptr + i * pitch;
		const uint8_t *y_src1 = y_src0 + pitch;
		const uint8_t *v_src = v_ptr + i / 2 * pitch;
		const uint8_t *u_src = u_ptr + i / 2 * pitch;
		aipl_argb8888_px_t *dst0 = dst_ptr + i * width;
		aipl_argb8888_px_t *dst1 = dst0 + width;

		for (uint32_t j = 0; j < width; j += 2) {
			int32_t c00, c01, c10, c11;
			int32_t r, g, b;

			aipl_pre_cnvt_px_yuv_to_rgb(&r, &g, &b, u_src[j], v_src[j]);
			aipl_pre_cnvt_2x2px_y(&c00, &c01, &c10, &c11, y_src0[j], y_src0[j + 1],
					      y_src1[j], y_src1[j + 1]);

			aipl_cnvt_px_yuv_to_argb8888(dst0 + j, c00, r, g, b);
			aipl_cnvt_px_yuv_to_argb8888(dst0 + j + 1, c01, r, g, b);
			aipl_cnvt_px_yuv_to_argb8888(dst1 + j, c10, r, g, b);
			aipl_cnvt_px_yuv_to_argb8888(dst1 + j + 1, c11, r, g, b);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_ARGB8888 | AIPL_CONVERT_UYVY & TO_ARGB8888) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_packed_to_argb8888_default(const uint8_t *y_ptr,
							       const uint8_t *u_ptr,
							       const uint8_t *v_ptr, void *output,
							       uint32_t pitch, uint32_t width,
							       uint32_t height)
{
	if (output == NULL || y_ptr == NULL || u_ptr == NULL || v_ptr == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	aipl_argb8888_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_ptr + i * pitch * 2;
		const uint8_t *v_src = v_ptr + i * pitch * 2;
		const uint8_t *u_src = u_ptr + i * pitch * 2;
		aipl_argb8888_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width * 2; j += 4) {
			uint32_t j2 = j / 2;
			int32_t c0, c1;
			int32_t r, g, b;

			aipl_pre_cnvt_px_yuv_to_rgb(&r, &g, &b, u_src[j], v_src[j]);
			aipl_pre_cnvt_2px_y(&c0, &c1, y_src[j], y_src[j + 2]);

			aipl_cnvt_px_yuv_to_argb8888(dst + j2, c0, r, g, b);
			aipl_cnvt_px_yuv_to_argb8888(dst + j2 + 1, c1, r, g, b);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_YV12 & TO_ARGB4444 | AIPL_CONVERT_I420 & TO_ARGB4444) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_planar_to_argb4444_default(const uint8_t *y_ptr,
							       const uint8_t *u_ptr,
							       const uint8_t *v_ptr, void *output,
							       uint32_t pitch, uint32_t width,
							       uint32_t height)
{
	if (y_ptr == NULL || u_ptr == NULL || v_ptr == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	aipl_argb4444_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; i += 2) {
		const uint8_t *y_src0 = y_ptr + i * pitch;
		const uint8_t *y_src1 = y_src0 + pitch;
		const uint8_t *u_src = u_ptr + i / 2 * pitch / 2;
		const uint8_t *v_src = v_ptr + i / 2 * pitch / 2;
		aipl_argb4444_px_t *dst0 = dst_ptr + i * width;
		aipl_argb4444_px_t *dst1 = dst0 + width;

		for (uint32_t j = 0; j < width; j += 2) {
			uint32_t j2 = j / 2;
			int32_t c00, c01, c10, c11;
			int32_t r, g, b;

			aipl_pre_cnvt_px_yuv_to_rgb(&r, &g, &b, u_src[j2], v_src[j2]);
			aipl_pre_cnvt_2x2px_y(&c00, &c01, &c10, &c11, y_src0[j], y_src0[j + 1],
					      y_src1[j], y_src1[j + 1]);

			aipl_cnvt_px_yuv_to_argb4444(dst0 + j, c00, r, g, b);
			aipl_cnvt_px_yuv_to_argb4444(dst0 + j + 1, c01, r, g, b);
			aipl_cnvt_px_yuv_to_argb4444(dst1 + j, c10, r, g, b);
			aipl_cnvt_px_yuv_to_argb4444(dst1 + j + 1, c11, r, g, b);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_NV12 & TO_ARGB4444 | AIPL_CONVERT_NV21 & TO_ARGB4444) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_semi_planar_to_argb4444_default(const uint8_t *y_ptr,
								    const uint8_t *u_ptr,
								    const uint8_t *v_ptr,
								    void *output, uint32_t pitch,
								    uint32_t width, uint32_t height)
{
	if (y_ptr == NULL || u_ptr == NULL || v_ptr == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	aipl_argb4444_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; i += 2) {
		const uint8_t *y_src0 = y_ptr + i * pitch;
		const uint8_t *y_src1 = y_src0 + pitch;
		const uint8_t *v_src = v_ptr + i / 2 * pitch;
		const uint8_t *u_src = u_ptr + i / 2 * pitch;
		aipl_argb4444_px_t *dst0 = dst_ptr + i * width;
		aipl_argb4444_px_t *dst1 = dst0 + width;

		for (uint32_t j = 0; j < width; j += 2) {
			int32_t c00, c01, c10, c11;
			int32_t r, g, b;

			aipl_pre_cnvt_px_yuv_to_rgb(&r, &g, &b, u_src[j], v_src[j]);
			aipl_pre_cnvt_2x2px_y(&c00, &c01, &c10, &c11, y_src0[j], y_src0[j + 1],
					      y_src1[j], y_src1[j + 1]);

			aipl_cnvt_px_yuv_to_argb4444(dst0 + j, c00, r, g, b);
			aipl_cnvt_px_yuv_to_argb4444(dst0 + j + 1, c01, r, g, b);
			aipl_cnvt_px_yuv_to_argb4444(dst1 + j, c10, r, g, b);
			aipl_cnvt_px_yuv_to_argb4444(dst1 + j + 1, c11, r, g, b);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_ARGB4444 | AIPL_CONVERT_UYVY & TO_ARGB4444) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_packed_to_argb4444_default(const uint8_t *y_ptr,
							       const uint8_t *u_ptr,
							       const uint8_t *v_ptr, void *output,
							       uint32_t pitch, uint32_t width,
							       uint32_t height)
{
	if (y_ptr == NULL || u_ptr == NULL || v_ptr == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	aipl_argb4444_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_ptr + i * pitch * 2;
		const uint8_t *v_src = v_ptr + i * pitch * 2;
		const uint8_t *u_src = u_ptr + i * pitch * 2;
		aipl_argb4444_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width * 2; j += 4) {
			uint32_t j2 = j / 2;
			int32_t c0, c1;
			int32_t r, g, b;

			aipl_pre_cnvt_px_yuv_to_rgb(&r, &g, &b, u_src[j], v_src[j]);
			aipl_pre_cnvt_2px_y(&c0, &c1, y_src[j], y_src[j + 2]);

			aipl_cnvt_px_yuv_to_argb4444(dst + j2, c0, r, g, b);
			aipl_cnvt_px_yuv_to_argb4444(dst + j2 + 1, c1, r, g, b);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_YV12 & TO_ARGB1555 | AIPL_CONVERT_I420 & TO_ARGB1555) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_planar_to_argb1555_default(const uint8_t *y_ptr,
							       const uint8_t *u_ptr,
							       const uint8_t *v_ptr, void *output,
							       uint32_t pitch, uint32_t width,
							       uint32_t height)
{
	if (y_ptr == NULL || u_ptr == NULL || v_ptr == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	aipl_argb1555_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; i += 2) {
		const uint8_t *y_src0 = y_ptr + i * pitch;
		const uint8_t *y_src1 = y_src0 + pitch;
		const uint8_t *u_src = u_ptr + i / 2 * pitch / 2;
		const uint8_t *v_src = v_ptr + i / 2 * pitch / 2;
		aipl_argb1555_px_t *dst0 = dst_ptr + i * width;
		aipl_argb1555_px_t *dst1 = dst0 + width;

		for (uint32_t j = 0; j < width; j += 2) {
			uint32_t j2 = j / 2;
			int32_t c00, c01, c10, c11;
			int32_t r, g, b;

			aipl_pre_cnvt_px_yuv_to_rgb(&r, &g, &b, u_src[j2], v_src[j2]);
			aipl_pre_cnvt_2x2px_y(&c00, &c01, &c10, &c11, y_src0[j], y_src0[j + 1],
					      y_src1[j], y_src1[j + 1]);

			aipl_cnvt_px_yuv_to_argb1555(dst0 + j, c00, r, g, b);
			aipl_cnvt_px_yuv_to_argb1555(dst0 + j + 1, c01, r, g, b);
			aipl_cnvt_px_yuv_to_argb1555(dst1 + j, c10, r, g, b);
			aipl_cnvt_px_yuv_to_argb1555(dst1 + j + 1, c11, r, g, b);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_NV12 & TO_ARGB1555 | AIPL_CONVERT_NV21 & TO_ARGB1555) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_semi_planar_to_argb1555_default(const uint8_t *y_ptr,
								    const uint8_t *u_ptr,
								    const uint8_t *v_ptr,
								    void *output, uint32_t pitch,
								    uint32_t width, uint32_t height)
{
	if (y_ptr == NULL || u_ptr == NULL || v_ptr == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	aipl_argb1555_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; i += 2) {
		const uint8_t *y_src0 = y_ptr + i * pitch;
		const uint8_t *y_src1 = y_src0 + pitch;
		const uint8_t *v_src = v_ptr + i / 2 * pitch;
		const uint8_t *u_src = u_ptr + i / 2 * pitch;
		aipl_argb1555_px_t *dst0 = dst_ptr + i * width;
		aipl_argb1555_px_t *dst1 = dst0 + width;

		for (uint32_t j = 0; j < width; j += 2) {
			int32_t c00, c01, c10, c11;
			int32_t r, g, b;

			aipl_pre_cnvt_px_yuv_to_rgb(&r, &g, &b, u_src[j], v_src[j]);
			aipl_pre_cnvt_2x2px_y(&c00, &c01, &c10, &c11, y_src0[j], y_src0[j + 1],
					      y_src1[j], y_src1[j + 1]);

			aipl_cnvt_px_yuv_to_argb1555(dst0 + j, c00, r, g, b);
			aipl_cnvt_px_yuv_to_argb1555(dst0 + j + 1, c01, r, g, b);
			aipl_cnvt_px_yuv_to_argb1555(dst1 + j, c10, r, g, b);
			aipl_cnvt_px_yuv_to_argb1555(dst1 + j + 1, c11, r, g, b);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_ARGB1555 | AIPL_CONVERT_UYVY & TO_ARGB1555) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_packed_to_argb1555_default(const uint8_t *y_ptr,
							       const uint8_t *u_ptr,
							       const uint8_t *v_ptr, void *output,
							       uint32_t pitch, uint32_t width,
							       uint32_t height)
{
	if (y_ptr == NULL || u_ptr == NULL || v_ptr == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	aipl_argb1555_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_ptr + i * pitch * 2;
		const uint8_t *v_src = v_ptr + i * pitch * 2;
		const uint8_t *u_src = u_ptr + i * pitch * 2;
		aipl_argb1555_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width * 2; j += 4) {
			uint32_t j2 = j / 2;
			int32_t c0, c1;
			int32_t r, g, b;

			aipl_pre_cnvt_px_yuv_to_rgb(&r, &g, &b, u_src[j], v_src[j]);
			aipl_pre_cnvt_2px_y(&c0, &c1, y_src[j], y_src[j + 2]);

			aipl_cnvt_px_yuv_to_argb1555(dst + j2, c0, r, g, b);
			aipl_cnvt_px_yuv_to_argb1555(dst + j2 + 1, c1, r, g, b);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_YV12 & TO_RGBA8888 | AIPL_CONVERT_I420 & TO_RGBA8888) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_planar_to_rgba8888_default(const uint8_t *y_ptr,
							       const uint8_t *u_ptr,
							       const uint8_t *v_ptr, void *output,
							       uint32_t pitch, uint32_t width,
							       uint32_t height)
{
	if (y_ptr == NULL || u_ptr == NULL || v_ptr == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	aipl_rgba8888_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; i += 2) {
		const uint8_t *y_src0 = y_ptr + i * pitch;
		const uint8_t *y_src1 = y_src0 + pitch;
		const uint8_t *u_src = u_ptr + i / 2 * pitch / 2;
		const uint8_t *v_src = v_ptr + i / 2 * pitch / 2;
		aipl_rgba8888_px_t *dst0 = dst_ptr + i * width;
		aipl_rgba8888_px_t *dst1 = dst0 + width;

		for (uint32_t j = 0; j < width; j += 2) {
			uint32_t j2 = j / 2;
			int32_t c00, c01, c10, c11;
			int32_t r, g, b;

			aipl_pre_cnvt_px_yuv_to_rgb(&r, &g, &b, u_src[j2], v_src[j2]);
			aipl_pre_cnvt_2x2px_y(&c00, &c01, &c10, &c11, y_src0[j], y_src0[j + 1],
					      y_src1[j], y_src1[j + 1]);

			aipl_cnvt_px_yuv_to_rgba8888(dst0 + j, c00, r, g, b);
			aipl_cnvt_px_yuv_to_rgba8888(dst0 + j + 1, c01, r, g, b);
			aipl_cnvt_px_yuv_to_rgba8888(dst1 + j, c10, r, g, b);
			aipl_cnvt_px_yuv_to_rgba8888(dst1 + j + 1, c11, r, g, b);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_NV12 & TO_RGBA8888 | AIPL_CONVERT_NV21 & TO_RGBA8888) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_semi_planar_to_rgba8888_default(const uint8_t *y_ptr,
								    const uint8_t *u_ptr,
								    const uint8_t *v_ptr,
								    void *output, uint32_t pitch,
								    uint32_t width, uint32_t height)
{
	if (y_ptr == NULL || u_ptr == NULL || v_ptr == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	aipl_rgba8888_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; i += 2) {
		const uint8_t *y_src0 = y_ptr + i * pitch;
		const uint8_t *y_src1 = y_src0 + pitch;
		const uint8_t *u_src = u_ptr + i / 2 * pitch;
		const uint8_t *v_src = v_ptr + i / 2 * pitch;
		aipl_rgba8888_px_t *dst0 = dst_ptr + i * width;
		aipl_rgba8888_px_t *dst1 = dst0 + width;

		for (uint32_t j = 0; j < width; j += 2) {
			int32_t c00, c01, c10, c11;
			int32_t r, g, b;

			aipl_pre_cnvt_px_yuv_to_rgb(&r, &g, &b, u_src[j], v_src[j]);
			aipl_pre_cnvt_2x2px_y(&c00, &c01, &c10, &c11, y_src0[j], y_src0[j + 1],
					      y_src1[j], y_src1[j + 1]);

			aipl_cnvt_px_yuv_to_rgba8888(dst0 + j, c00, r, g, b);
			aipl_cnvt_px_yuv_to_rgba8888(dst0 + j + 1, c01, r, g, b);
			aipl_cnvt_px_yuv_to_rgba8888(dst1 + j, c10, r, g, b);
			aipl_cnvt_px_yuv_to_rgba8888(dst1 + j + 1, c11, r, g, b);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_RGBA8888 | AIPL_CONVERT_UYVY & TO_RGBA8888) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_packed_to_rgba8888_default(const uint8_t *y_ptr,
							       const uint8_t *u_ptr,
							       const uint8_t *v_ptr, void *output,
							       uint32_t pitch, uint32_t width,
							       uint32_t height)
{
	if (y_ptr == NULL || u_ptr == NULL || v_ptr == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	aipl_rgba8888_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_ptr + i * pitch * 2;
		const uint8_t *v_src = v_ptr + i * pitch * 2;
		const uint8_t *u_src = u_ptr + i * pitch * 2;
		aipl_rgba8888_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width * 2; j += 4) {
			uint32_t j2 = j / 2;
			int32_t c0, c1;
			int32_t r, g, b;

			aipl_pre_cnvt_px_yuv_to_rgb(&r, &g, &b, u_src[j], v_src[j]);
			aipl_pre_cnvt_2px_y(&c0, &c1, y_src[j], y_src[j + 2]);

			aipl_cnvt_px_yuv_to_rgba8888(dst + j2, c0, r, g, b);
			aipl_cnvt_px_yuv_to_rgba8888(dst + j2 + 1, c1, r, g, b);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_YV12 & TO_RGBA4444 | AIPL_CONVERT_I420 & TO_RGBA4444) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_planar_to_rgba4444_default(const uint8_t *y_ptr,
							       const uint8_t *u_ptr,
							       const uint8_t *v_ptr, void *output,
							       uint32_t pitch, uint32_t width,
							       uint32_t height)
{
	if (y_ptr == NULL || u_ptr == NULL || v_ptr == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	aipl_rgba4444_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; i += 2) {
		const uint8_t *y_src0 = y_ptr + i * pitch;
		const uint8_t *y_src1 = y_src0 + pitch;
		const uint8_t *u_src = u_ptr + i / 2 * pitch / 2;
		const uint8_t *v_src = v_ptr + i / 2 * pitch / 2;
		aipl_rgba4444_px_t *dst0 = dst_ptr + i * width;
		aipl_rgba4444_px_t *dst1 = dst0 + width;

		for (uint32_t j = 0; j < width; j += 2) {
			uint32_t j2 = j / 2;
			int32_t c00, c01, c10, c11;
			int32_t r, g, b;

			aipl_pre_cnvt_px_yuv_to_rgb(&r, &g, &b, u_src[j2], v_src[j2]);
			aipl_pre_cnvt_2x2px_y(&c00, &c01, &c10, &c11, y_src0[j], y_src0[j + 1],
					      y_src1[j], y_src1[j + 1]);

			aipl_cnvt_px_yuv_to_rgba4444(dst0 + j, c00, r, g, b);
			aipl_cnvt_px_yuv_to_rgba4444(dst0 + j + 1, c01, r, g, b);
			aipl_cnvt_px_yuv_to_rgba4444(dst1 + j, c10, r, g, b);
			aipl_cnvt_px_yuv_to_rgba4444(dst1 + j + 1, c11, r, g, b);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_NV12 & TO_RGBA4444 | AIPL_CONVERT_NV21 & TO_RGBA4444) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_semi_planar_to_rgba4444_default(const uint8_t *y_ptr,
								    const uint8_t *u_ptr,
								    const uint8_t *v_ptr,
								    void *output, uint32_t pitch,
								    uint32_t width, uint32_t height)
{
	if (y_ptr == NULL || u_ptr == NULL || v_ptr == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	aipl_rgba4444_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; i += 2) {
		const uint8_t *y_src0 = y_ptr + i * pitch;
		const uint8_t *y_src1 = y_src0 + pitch;
		const uint8_t *v_src = v_ptr + i / 2 * pitch;
		const uint8_t *u_src = u_ptr + i / 2 * pitch;
		aipl_rgba4444_px_t *dst0 = dst_ptr + i * width;
		aipl_rgba4444_px_t *dst1 = dst0 + width;

		for (uint32_t j = 0; j < width; j += 2) {
			int32_t c00, c01, c10, c11;
			int32_t r, g, b;

			aipl_pre_cnvt_px_yuv_to_rgb(&r, &g, &b, u_src[j], v_src[j]);
			aipl_pre_cnvt_2x2px_y(&c00, &c01, &c10, &c11, y_src0[j], y_src0[j + 1],
					      y_src1[j], y_src1[j + 1]);

			aipl_cnvt_px_yuv_to_rgba4444(dst0 + j, c00, r, g, b);
			aipl_cnvt_px_yuv_to_rgba4444(dst0 + j + 1, c01, r, g, b);
			aipl_cnvt_px_yuv_to_rgba4444(dst1 + j, c10, r, g, b);
			aipl_cnvt_px_yuv_to_rgba4444(dst1 + j + 1, c11, r, g, b);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_RGBA4444 | AIPL_CONVERT_UYVY & TO_RGBA4444) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_packed_to_rgba4444_default(const uint8_t *y_ptr,
							       const uint8_t *u_ptr,
							       const uint8_t *v_ptr, void *output,
							       uint32_t pitch, uint32_t width,
							       uint32_t height)
{
	if (y_ptr == NULL || u_ptr == NULL || v_ptr == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	aipl_rgba4444_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_ptr + i * pitch * 2;
		const uint8_t *v_src = v_ptr + i * pitch * 2;
		const uint8_t *u_src = u_ptr + i * pitch * 2;
		aipl_rgba4444_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width * 2; j += 4) {
			uint32_t j2 = j / 2;
			int32_t c0, c1;
			int32_t r, g, b;

			aipl_pre_cnvt_px_yuv_to_rgb(&r, &g, &b, u_src[j], v_src[j]);
			aipl_pre_cnvt_2px_y(&c0, &c1, y_src[j], y_src[j + 2]);

			aipl_cnvt_px_yuv_to_rgba4444(dst + j2, c0, r, g, b);
			aipl_cnvt_px_yuv_to_rgba4444(dst + j2 + 1, c1, r, g, b);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_YV12 & TO_RGBA5551 | AIPL_CONVERT_I420 & TO_RGBA5551) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_planar_to_rgba5551_default(const uint8_t *y_ptr,
							       const uint8_t *u_ptr,
							       const uint8_t *v_ptr, void *output,
							       uint32_t pitch, uint32_t width,
							       uint32_t height)
{
	if (y_ptr == NULL || u_ptr == NULL || v_ptr == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	aipl_rgba5551_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; i += 2) {
		const uint8_t *y_src0 = y_ptr + i * pitch;
		const uint8_t *y_src1 = y_src0 + pitch;
		const uint8_t *u_src = u_ptr + i / 2 * pitch / 2;
		const uint8_t *v_src = v_ptr + i / 2 * pitch / 2;
		aipl_rgba5551_px_t *dst0 = dst_ptr + i * width;
		aipl_rgba5551_px_t *dst1 = dst0 + width;

		for (uint32_t j = 0; j < width; j += 2) {
			uint32_t j2 = j / 2;
			int32_t c00, c01, c10, c11;
			int32_t r, g, b;

			aipl_pre_cnvt_px_yuv_to_rgb(&r, &g, &b, u_src[j2], v_src[j2]);
			aipl_pre_cnvt_2x2px_y(&c00, &c01, &c10, &c11, y_src0[j], y_src0[j + 1],
					      y_src1[j], y_src1[j + 1]);

			aipl_cnvt_px_yuv_to_rgba5551(dst0 + j, c00, r, g, b);
			aipl_cnvt_px_yuv_to_rgba5551(dst0 + j + 1, c01, r, g, b);
			aipl_cnvt_px_yuv_to_rgba5551(dst1 + j, c10, r, g, b);
			aipl_cnvt_px_yuv_to_rgba5551(dst1 + j + 1, c11, r, g, b);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_NV12 & TO_RGBA5551 | AIPL_CONVERT_NV21 & TO_RGBA5551) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_semi_planar_to_rgba5551_default(const uint8_t *y_ptr,
								    const uint8_t *u_ptr,
								    const uint8_t *v_ptr,
								    void *output, uint32_t pitch,
								    uint32_t width, uint32_t height)
{
	if (y_ptr == NULL || u_ptr == NULL || v_ptr == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	aipl_rgba5551_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; i += 2) {
		const uint8_t *y_src0 = y_ptr + i * pitch;
		const uint8_t *y_src1 = y_src0 + pitch;
		const uint8_t *v_src = v_ptr + i / 2 * pitch;
		const uint8_t *u_src = u_ptr + i / 2 * pitch;
		aipl_rgba5551_px_t *dst0 = dst_ptr + i * width;
		aipl_rgba5551_px_t *dst1 = dst0 + width;

		for (uint32_t j = 0; j < width; j += 2) {
			int32_t c00, c01, c10, c11;
			int32_t r, g, b;

			aipl_pre_cnvt_px_yuv_to_rgb(&r, &g, &b, u_src[j], v_src[j]);
			aipl_pre_cnvt_2x2px_y(&c00, &c01, &c10, &c11, y_src0[j], y_src0[j + 1],
					      y_src1[j], y_src1[j + 1]);

			aipl_cnvt_px_yuv_to_rgba5551(dst0 + j, c00, r, g, b);
			aipl_cnvt_px_yuv_to_rgba5551(dst0 + j + 1, c01, r, g, b);
			aipl_cnvt_px_yuv_to_rgba5551(dst1 + j, c10, r, g, b);
			aipl_cnvt_px_yuv_to_rgba5551(dst1 + j + 1, c11, r, g, b);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_RGBA5551 | AIPL_CONVERT_UYVY & TO_RGBA5551) &&                         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_packed_to_rgba5551_default(const uint8_t *y_ptr,
							       const uint8_t *u_ptr,
							       const uint8_t *v_ptr, void *output,
							       uint32_t pitch, uint32_t width,
							       uint32_t height)
{
	if (y_ptr == NULL || u_ptr == NULL || v_ptr == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	aipl_rgba5551_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_ptr + i * pitch * 2;
		const uint8_t *v_src = v_ptr + i * pitch * 2;
		const uint8_t *u_src = u_ptr + i * pitch * 2;
		aipl_rgba5551_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width * 2; j += 4) {
			uint32_t j2 = j / 2;
			int32_t c0, c1;
			int32_t r, g, b;

			aipl_pre_cnvt_px_yuv_to_rgb(&r, &g, &b, u_src[j], v_src[j]);
			aipl_pre_cnvt_2px_y(&c0, &c1, y_src[j], y_src[j + 2]);

			aipl_cnvt_px_yuv_to_rgba5551(dst + j2, c0, r, g, b);
			aipl_cnvt_px_yuv_to_rgba5551(dst + j2 + 1, c1, r, g, b);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_YV12 & TO_BGR888 | AIPL_CONVERT_I420 & TO_BGR888) &&                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_planar_to_24bit_default(const uint8_t *y_ptr,
							    const uint8_t *u_ptr,
							    const uint8_t *v_ptr, void *output,
							    uint32_t pitch, uint32_t width,
							    uint32_t height, uint8_t r_offset,
							    uint8_t g_offset, uint8_t b_offset)
{
	if (y_ptr == NULL || u_ptr == NULL || v_ptr == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint8_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; i += 2) {
		const uint8_t *y_src0 = y_ptr + i * pitch;
		const uint8_t *y_src1 = y_src0 + pitch;
		const uint8_t *u_src = u_ptr + i / 2 * pitch / 2;
		const uint8_t *v_src = v_ptr + i / 2 * pitch / 2;
		uint8_t *dst0 = dst_ptr + i * width * 3;
		uint8_t *dst1 = dst0 + width * 3;

		for (uint32_t j = 0; j < width; j += 2) {
			uint32_t j2 = j / 2;
			int32_t c00, c01, c10, c11;
			int32_t r, g, b;

			aipl_pre_cnvt_px_yuv_to_rgb(&r, &g, &b, u_src[j2], v_src[j2]);
			aipl_pre_cnvt_2x2px_y(&c00, &c01, &c10, &c11, y_src0[j], y_src0[j + 1],
					      y_src1[j], y_src1[j + 1]);

			aipl_cnvt_px_yuv_to_24bit(dst0 + j * 3, c00, r, g, b, r_offset, g_offset,
						  b_offset);
			aipl_cnvt_px_yuv_to_24bit(dst0 + (j + 1) * 3, c01, r, g, b, r_offset,
						  g_offset, b_offset);
			aipl_cnvt_px_yuv_to_24bit(dst1 + j * 3, c10, r, g, b, r_offset, g_offset,
						  b_offset);
			aipl_cnvt_px_yuv_to_24bit(dst1 + (j + 1) * 3, c11, r, g, b, r_offset,
						  g_offset, b_offset);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_NV12 & TO_BGR888 | AIPL_CONVERT_NV21 & TO_BGR888) &&                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_semi_planar_to_24bit_default(const uint8_t *y_ptr,
								 const uint8_t *u_ptr,
								 const uint8_t *v_ptr, void *output,
								 uint32_t pitch, uint32_t width,
								 uint32_t height, uint8_t r_offset,
								 uint8_t g_offset, uint8_t b_offset)
{
	if (y_ptr == NULL || u_ptr == NULL || v_ptr == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint8_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; i += 2) {
		const uint8_t *y_src0 = y_ptr + i * pitch;
		const uint8_t *y_src1 = y_src0 + pitch;
		const uint8_t *v_src = v_ptr + i / 2 * pitch;
		const uint8_t *u_src = u_ptr + i / 2 * pitch;
		uint8_t *dst0 = dst_ptr + i * width * 3;
		uint8_t *dst1 = dst0 + width * 3;

		for (uint32_t j = 0; j < width; j += 2) {
			int32_t c00, c01, c10, c11;
			int32_t r, g, b;

			aipl_pre_cnvt_px_yuv_to_rgb(&r, &g, &b, u_src[j], v_src[j]);
			aipl_pre_cnvt_2x2px_y(&c00, &c01, &c10, &c11, y_src0[j], y_src0[j + 1],
					      y_src1[j], y_src1[j + 1]);

			aipl_cnvt_px_yuv_to_24bit(dst0 + j * 3, c00, r, g, b, r_offset, g_offset,
						  b_offset);
			aipl_cnvt_px_yuv_to_24bit(dst0 + (j + 1) * 3, c01, r, g, b, r_offset,
						  g_offset, b_offset);
			aipl_cnvt_px_yuv_to_24bit(dst1 + j * 3, c10, r, g, b, r_offset, g_offset,
						  b_offset);
			aipl_cnvt_px_yuv_to_24bit(dst1 + (j + 1) * 3, c11, r, g, b, r_offset,
						  g_offset, b_offset);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_BGR888 | AIPL_CONVERT_UYVY & TO_BGR888) &&                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_packed_to_24bit_default(const uint8_t *y_ptr,
							    const uint8_t *u_ptr,
							    const uint8_t *v_ptr, void *output,
							    uint32_t pitch, uint32_t width,
							    uint32_t height, uint8_t r_offset,
							    uint8_t g_offset, uint8_t b_offset)
{
	if (y_ptr == NULL || u_ptr == NULL || v_ptr == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint8_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_ptr + i * pitch * 2;
		const uint8_t *v_src = v_ptr + i * pitch * 2;
		const uint8_t *u_src = u_ptr + i * pitch * 2;
		uint8_t *dst = dst_ptr + i * width * 3;

		for (uint32_t j = 0; j < width * 2; j += 4) {
			uint32_t j2 = j / 2;
			int32_t c0, c1;
			int32_t r, g, b;

			aipl_pre_cnvt_px_yuv_to_rgb(&r, &g, &b, u_src[j], v_src[j]);
			aipl_pre_cnvt_2px_y(&c0, &c1, y_src[j], y_src[j + 2]);

			aipl_cnvt_px_yuv_to_24bit(dst + j2 * 3, c0, r, g, b, r_offset, g_offset,
						  b_offset);
			aipl_cnvt_px_yuv_to_24bit(dst + (j2 + 1) * 3, c1, r, g, b, r_offset,
						  g_offset, b_offset);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_YV12 & TO_RGB565 | AIPL_CONVERT_I420 & TO_RGB565) &&                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_planar_to_rgb565_default(const uint8_t *y_ptr,
							     const uint8_t *u_ptr,
							     const uint8_t *v_ptr, void *output,
							     uint32_t pitch, uint32_t width,
							     uint32_t height)
{
	if (y_ptr == NULL || u_ptr == NULL || v_ptr == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	aipl_rgb565_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; i += 2) {
		const uint8_t *y_src0 = y_ptr + i * pitch;
		const uint8_t *y_src1 = y_src0 + pitch;
		const uint8_t *u_src = u_ptr + i / 2 * pitch / 2;
		const uint8_t *v_src = v_ptr + i / 2 * pitch / 2;
		aipl_rgb565_px_t *dst0 = dst_ptr + i * width;
		aipl_rgb565_px_t *dst1 = dst0 + width;

		for (uint32_t j = 0; j < width; j += 2) {
			uint32_t j2 = j / 2;
			int32_t c00, c01, c10, c11;
			int32_t r, g, b;

			aipl_pre_cnvt_px_yuv_to_rgb(&r, &g, &b, u_src[j2], v_src[j2]);
			aipl_pre_cnvt_2x2px_y(&c00, &c01, &c10, &c11, y_src0[j], y_src0[j + 1],
					      y_src1[j], y_src1[j + 1]);

			aipl_cnvt_px_yuv_to_rgb565(dst0 + j, c00, r, g, b);
			aipl_cnvt_px_yuv_to_rgb565(dst0 + j + 1, c01, r, g, b);
			aipl_cnvt_px_yuv_to_rgb565(dst1 + j, c10, r, g, b);
			aipl_cnvt_px_yuv_to_rgb565(dst1 + j + 1, c11, r, g, b);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_NV12 & TO_RGB565 | AIPL_CONVERT_NV21 & TO_RGB565) &&                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_semi_planar_to_rgb565_default(const uint8_t *y_ptr,
								  const uint8_t *u_ptr,
								  const uint8_t *v_ptr,
								  void *output, uint32_t pitch,
								  uint32_t width, uint32_t height)
{
	if (y_ptr == NULL || u_ptr == NULL || v_ptr == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	aipl_rgb565_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; i += 2) {
		const uint8_t *y_src0 = y_ptr + i * pitch;
		const uint8_t *y_src1 = y_src0 + pitch;
		const uint8_t *v_src = v_ptr + i / 2 * pitch;
		const uint8_t *u_src = u_ptr + i / 2 * pitch;
		aipl_rgb565_px_t *dst0 = dst_ptr + i * width;
		aipl_rgb565_px_t *dst1 = dst0 + width;

		for (uint32_t j = 0; j < width; j += 2) {
			int32_t c00, c01, c10, c11;
			int32_t r, g, b;

			aipl_pre_cnvt_px_yuv_to_rgb(&r, &g, &b, u_src[j], v_src[j]);
			aipl_pre_cnvt_2x2px_y(&c00, &c01, &c10, &c11, y_src0[j], y_src0[j + 1],
					      y_src1[j], y_src1[j + 1]);

			aipl_cnvt_px_yuv_to_rgb565(dst0 + j, c00, r, g, b);
			aipl_cnvt_px_yuv_to_rgb565(dst0 + j + 1, c01, r, g, b);
			aipl_cnvt_px_yuv_to_rgb565(dst1 + j, c10, r, g, b);
			aipl_cnvt_px_yuv_to_rgb565(dst1 + j + 1, c11, r, g, b);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_RGB565 | AIPL_CONVERT_UYVY & TO_RGB565) &&                             \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_packed_to_rgb565_default(const uint8_t *y_ptr,
							     const uint8_t *u_ptr,
							     const uint8_t *v_ptr, void *output,
							     uint32_t pitch, uint32_t width,
							     uint32_t height)
{
	if (y_ptr == NULL || u_ptr == NULL || v_ptr == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	aipl_rgb565_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_src = y_ptr + i * pitch * 2;
		const uint8_t *v_src = v_ptr + i * pitch * 2;
		const uint8_t *u_src = u_ptr + i * pitch * 2;
		aipl_rgb565_px_t *dst = dst_ptr + i * width;

		for (uint32_t j = 0; j < width * 2; j += 4) {
			uint32_t j2 = j / 2;
			int32_t c0, c1;
			int32_t r, g, b;

			aipl_pre_cnvt_px_yuv_to_rgb(&r, &g, &b, u_src[j], v_src[j]);
			aipl_pre_cnvt_2px_y(&c0, &c1, y_src[j], y_src[j + 2]);

			aipl_cnvt_px_yuv_to_rgb565(dst + j2, c0, r, g, b);
			aipl_cnvt_px_yuv_to_rgb565(dst + j2 + 1, c1, r, g, b);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_YV12 & TO_I420 | AIPL_CONVERT_I420 & TO_YV12)
aipl_error_t aipl_color_convert_yuv_planar_to_planar_default(
	const uint8_t *y_src, const uint8_t *u_src, const uint8_t *v_src, uint8_t *y_dst,
	uint8_t *u_dst, uint8_t *v_dst, uint32_t pitch, uint32_t width, uint32_t height)
{
	if (y_src == NULL || u_src == NULL || v_src == NULL || y_dst == NULL || u_dst == NULL ||
	    v_dst == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_s = y_src + i * pitch;
		uint8_t *y_d = y_dst + i * width;

		memcpy(y_d, y_s, width);

		if (!(i & 1)) {
			const uint8_t *u_s = u_src + i * pitch / 4;
			uint8_t *u_d = u_dst + i * width / 4;
			const uint8_t *v_s = v_src + i * pitch / 4;
			uint8_t *v_d = v_dst + i * width / 4;

			memcpy(u_d, u_s, width / 2);
			memcpy(v_d, v_s, width / 2);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_YV12 & (TO_NV12 | TO_NV21) | AIPL_CONVERT_I420 & (TO_NV12 | TO_NV21)) &&         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_planar_to_semi_default(
	const uint8_t *y_src, const uint8_t *u_src, const uint8_t *v_src, uint8_t *y_dst,
	uint8_t *u_dst, uint8_t *v_dst, uint32_t pitch, uint32_t width, uint32_t height)
{
	if (y_src == NULL || u_src == NULL || v_src == NULL || y_dst == NULL || u_dst == NULL ||
	    v_dst == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_s = y_src + i * pitch;
		uint8_t *y_d = y_dst + i * width;

		memcpy(y_d, y_s, width);

		if (!(i & 1)) {
			const uint8_t *u_s = u_src + i / 2 * pitch / 2;
			uint8_t *u_d = u_dst + i / 2 * width;
			const uint8_t *v_s = v_src + i / 2 * pitch / 2;
			uint8_t *v_d = v_dst + i / 2 * width;

			for (uint32_t j = 0; j < width; j += 2) {
				u_d[j] = u_s[j / 2];
				v_d[j] = v_s[j / 2];
			}
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_YV12 & (TO_YUY2 | TO_UYVY) | AIPL_CONVERT_I420 & (TO_YUY2 | TO_UYVY))
aipl_error_t aipl_color_convert_yuv_planar_to_packed_default(
	const uint8_t *y_src, const uint8_t *u_src, const uint8_t *v_src, uint8_t *y_dst,
	uint8_t *u_dst, uint8_t *v_dst, uint32_t pitch, uint32_t width, uint32_t height)
{
	if (y_src == NULL || u_src == NULL || v_src == NULL || y_dst == NULL || u_dst == NULL ||
	    v_dst == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_s = y_src + i * pitch;
		uint8_t *y_d = y_dst + i * width * 2;
		const uint8_t *u_s = u_src + i / 2 * pitch / 2;
		uint8_t *u_d = u_dst + i * width * 2;
		const uint8_t *v_s = v_src + i / 2 * pitch / 2;
		uint8_t *v_d = v_dst + i * width * 2;

		for (uint32_t j = 0; j < width / 2; ++j) {
			y_d[j * 4] = y_s[j * 2];
			y_d[j * 4 + 2] = y_s[j * 2 + 1];
			u_d[j * 4] = u_s[j];
			v_d[j * 4] = v_s[j];
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_NV12 & (TO_YV12 | TO_I420) | AIPL_CONVERT_NV21 & (TO_YV12 | TO_I420)) &&         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_semi_to_planar_default(
	const uint8_t *y_src, const uint8_t *u_src, const uint8_t *v_src, uint8_t *y_dst,
	uint8_t *u_dst, uint8_t *v_dst, uint32_t pitch, uint32_t width, uint32_t height)
{
	if (y_src == NULL || u_src == NULL || v_src == NULL || y_dst == NULL || u_dst == NULL ||
	    v_dst == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_s = y_src + i * pitch;
		uint8_t *y_d = y_dst + i * width;

		memcpy(y_d, y_s, width);

		if (!(i & 1)) {
			const uint8_t *u_s = u_src + i / 2 * pitch;
			uint8_t *u_d = u_dst + i / 2 * width / 2;
			const uint8_t *v_s = v_src + i / 2 * pitch;
			uint8_t *v_d = v_dst + i / 2 * width / 2;

			for (uint32_t j = 0; j < width; j += 2) {
				u_d[j / 2] = u_s[j];
				v_d[j / 2] = v_s[j];
			}
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_NV12 & TO_NV21 | AIPL_CONVERT_NV21 & TO_NV12)
aipl_error_t aipl_color_convert_yuv_semi_to_semi_planar_default(
	const uint8_t *y_src, const uint8_t *u_src, const uint8_t *v_src, uint8_t *y_dst,
	uint8_t *u_dst, uint8_t *v_dst, uint32_t pitch, uint32_t width, uint32_t height)
{
	if (y_src == NULL || u_src == NULL || v_src == NULL || y_dst == NULL || u_dst == NULL ||
	    v_dst == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_s = y_src + i * pitch;
		uint8_t *y_d = y_dst + i * width;

		memcpy(y_d, y_s, width);

		if (!(i & 1)) {
			const uint8_t *u_s = u_src + i / 2 * pitch;
			uint8_t *u_d = u_dst + i / 2 * width;
			const uint8_t *v_s = v_src + i / 2 * pitch;
			uint8_t *v_d = v_dst + i / 2 * width;

			for (uint32_t j = 0; j < width; j += 2) {
				u_d[j] = u_s[j];
				v_d[j] = v_s[j];
			}
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_NV12 & (TO_YUY2 | TO_UYVY) | AIPL_CONVERT_NV21 & (TO_YUY2 | TO_UYVY))
aipl_error_t aipl_color_convert_yuv_semi_to_packed_default(
	const uint8_t *y_src, const uint8_t *u_src, const uint8_t *v_src, uint8_t *y_dst,
	uint8_t *u_dst, uint8_t *v_dst, uint32_t pitch, uint32_t width, uint32_t height)
{
	if (y_src == NULL || u_src == NULL || v_src == NULL || y_dst == NULL || u_dst == NULL ||
	    v_dst == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_s = y_src + i * pitch;
		uint8_t *y_d = y_dst + i * width * 2;
		const uint8_t *u_s = u_src + i / 2 * pitch;
		uint8_t *u_d = u_dst + i * width * 2;
		const uint8_t *v_s = v_src + i / 2 * pitch;
		uint8_t *v_d = v_dst + i * width * 2;

		for (uint32_t j = 0; j < width / 2; ++j) {
			y_d[j * 4] = y_s[j * 2];
			y_d[j * 4 + 2] = y_s[j * 2 + 1];
			u_d[j * 4] = u_s[j * 2];
			v_d[j * 4] = v_s[j * 2];
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_YUY2 & (TO_YV12 | TO_I420) | AIPL_CONVERT_UYVY & (TO_YV12 | TO_I420)) &&         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_packed_to_planar_default(
	const uint8_t *y_src, const uint8_t *u_src, const uint8_t *v_src, uint8_t *y_dst,
	uint8_t *u_dst, uint8_t *v_dst, uint32_t pitch, uint32_t width, uint32_t height)
{
	if (y_src == NULL || u_src == NULL || v_src == NULL || y_dst == NULL || u_dst == NULL ||
	    v_dst == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_s = y_src + i * pitch * 2;
		uint8_t *y_d = y_dst + i * width;

		for (uint32_t j = 0; j < width * 2; j += 2) {
			y_d[j / 2] = y_s[j];
		}

		if (!(i & 1)) {
			const uint8_t *u_s = u_src + i * pitch * 2;
			uint8_t *u_d = u_dst + i / 2 * width / 2;
			const uint8_t *v_s = v_src + i * pitch * 2;
			uint8_t *v_d = v_dst + i / 2 * width / 2;

			for (uint32_t j = 0; j < width * 2; j += 4) {
				u_d[j / 4] = u_s[j];
				v_d[j / 4] = v_s[j];
			}
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_YUY2 & (TO_NV12 | TO_NV21) | AIPL_CONVERT_UYVY & (TO_NV12 | TO_NV21)) &&         \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_yuv_packed_to_semi_default(
	const uint8_t *y_src, const uint8_t *u_src, const uint8_t *v_src, uint8_t *y_dst,
	uint8_t *u_dst, uint8_t *v_dst, uint32_t pitch, uint32_t width, uint32_t height)
{
	if (y_src == NULL || u_src == NULL || v_src == NULL || y_dst == NULL || u_dst == NULL ||
	    v_dst == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_s = y_src + i * pitch * 2;
		uint8_t *y_d = y_dst + i * width;

		for (uint32_t j = 0; j < width * 2; j += 2) {
			y_d[j / 2] = y_s[j];
		}

		if (!(i & 1)) {
			const uint8_t *u_s = u_src + i * pitch * 2;
			uint8_t *u_d = u_dst + i / 2 * width;
			const uint8_t *v_s = v_src + i * pitch * 2;
			uint8_t *v_d = v_dst + i / 2 * width;

			for (uint32_t j = 0; j < width * 2; j += 4) {
				u_d[j / 2] = u_s[j];
				v_d[j / 2] = v_s[j];
			}
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_UYVY | AIPL_CONVERT_UYVY & TO_YUY2)
aipl_error_t aipl_color_convert_yuv_packed_to_packed_default(
	const uint8_t *y_src, const uint8_t *u_src, const uint8_t *v_src, uint8_t *y_dst,
	uint8_t *u_dst, uint8_t *v_dst, uint32_t pitch, uint32_t width, uint32_t height)
{
	if (y_src == NULL || u_src == NULL || v_src == NULL || y_dst == NULL || u_dst == NULL ||
	    v_dst == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_s = y_src + i * pitch * 2;
		uint8_t *y_d = y_dst + i * width * 2;
		const uint8_t *u_s = u_src + i * pitch * 2;
		uint8_t *u_d = u_dst + i * width * 2;
		const uint8_t *v_s = v_src + i * pitch * 2;
		uint8_t *v_d = v_dst + i * width * 2;

		for (uint32_t j = 0; j < width * 2; j += 4) {
			y_d[j] = y_s[j];
			y_d[j + 2] = y_s[j + 2];
			u_d[j] = u_s[j];
			v_d[j] = v_s[j];
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & (TO_YV12 | TO_I420))
aipl_error_t aipl_color_convert_alpha8_to_yuv_planar_default(const uint8_t *input, uint8_t *output,
							     uint32_t pitch, uint32_t width,
							     uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t yuv_size = width * height;
	uint8_t *uv_dst0 = output + yuv_size;
	uint8_t *uv_dst1 = uv_dst0 + yuv_size / 4;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_s = input + i * pitch;
		uint8_t *y_d = output + i * width;

		memcpy(y_d, y_s, width);

		if (!(i & 1)) {
			uint8_t *uv_d0 = uv_dst0 + i / 2 * width / 2;
			uint8_t *uv_d1 = uv_dst1 + i / 2 * width / 2;

			memset(uv_d0, 0x80, width / 2);
			memset(uv_d1, 0x80, width / 2);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & (TO_NV12 | TO_NV21))
aipl_error_t aipl_color_convert_alpha8_to_yuv_semi_planar_default(const uint8_t *input,
								  uint8_t *output, uint32_t pitch,
								  uint32_t width, uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint8_t *uv_dst = output + width * height;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_s = input + i * pitch;
		uint8_t *y_d = output + i * width;

		memcpy(y_d, y_s, width);

		if (!(i & 1)) {
			uint8_t *uv_d = uv_dst + i / 2 * width;

			memset(uv_d, 0x80, width);
		}
	}

	return AIPL_ERR_OK;
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & (TO_YUY2 | TO_UYVY)) &&                                            \
	(!defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT))
aipl_error_t aipl_color_convert_alpha8_to_yuv_packed_default(const uint8_t *input, uint8_t *y_dst,
							     uint8_t *u_dst, uint8_t *v_dst,
							     uint32_t pitch, uint32_t width,
							     uint32_t height)
{
	if (input == NULL || y_dst == NULL || u_dst == NULL || v_dst == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint8_t *start = y_dst;

	if (u_dst < start) {
		start = u_dst;
	}
	if (v_dst < start) {
		start = v_dst;
	}
	memset(start, 0x80, width * height * 2);

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *y_s = input + i * pitch;
		uint8_t *y_d = y_dst + i * width * 2;

		for (uint32_t j = 0; j < width * 2; j += 4) {
			y_d[j] = y_s[j / 2];
			y_d[j + 2] = y_s[j / 2 + 1];
		}
	}

	return AIPL_ERR_OK;
}
#endif

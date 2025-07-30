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
 * @file    aipl_color_conversion.c
 * @brief   Color conversion function implementations
 *
 ******************************************************************************/

/*********************
 *      INCLUDES
 *********************/
#include "aipl_color_conversion.h"

#include <stddef.h>

#include "aipl_config.h"
#ifdef AIPL_DAVE2D_ACCELERATION
#include "aipl_color_conversion_dave2d.h"
#include "aipl_dave2d.h"
#endif
#ifdef AIPL_HELIUM_ACCELERATION
#include "aipl_color_conversion_helium.h"
#endif
#include "aipl_color_conversion_default.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
aipl_error_t aipl_color_convert(const void *input, void *output, uint32_t pitch, uint32_t width,
				uint32_t height, aipl_color_format_t input_format,
				aipl_color_format_t output_format)
{
#ifdef AIPL_DAVE2D_ACCELERATION
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	if (aipl_dave2d_color_convert_suitable(input_format, output_format)) {
		d2_s32 ret =
			aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
						       aipl_dave2d_format_to_mode(input_format),
						       aipl_dave2d_format_to_mode(output_format));

		return aipl_dave2d_error_convert(ret);
	}
#endif

	switch (input_format) {
		/* Alpha color formats */
#if AIPL_CONVERT_ALPHA8_I400
	case AIPL_COLOR_ALPHA8:
		return aipl_color_convert_alpha8(input, output, pitch, width, height,
						 output_format);
#endif

		/* RGB color formats */
#if AIPL_CONVERT_ARGB8888
	case AIPL_COLOR_ARGB8888:
		return aipl_color_convert_argb8888(input, output, pitch, width, height,
						   output_format);
#endif
#if AIPL_CONVERT_RGBA8888
	case AIPL_COLOR_RGBA8888:
		return aipl_color_convert_rgba8888(input, output, pitch, width, height,
						   output_format);
#endif
#if AIPL_CONVERT_ARGB4444
	case AIPL_COLOR_ARGB4444:
		return aipl_color_convert_argb4444(input, output, pitch, width, height,
						   output_format);
#endif
#if AIPL_CONVERT_ARGB1555
	case AIPL_COLOR_ARGB1555:
		return aipl_color_convert_argb1555(input, output, pitch, width, height,
						   output_format);
#endif
#if AIPL_CONVERT_RGBA4444
	case AIPL_COLOR_RGBA4444:
		return aipl_color_convert_rgba4444(input, output, pitch, width, height,
						   output_format);
#endif
#if AIPL_CONVERT_RGBA5551
	case AIPL_COLOR_RGBA5551:
		return aipl_color_convert_rgba5551(input, output, pitch, width, height,
						   output_format);
#endif
#if AIPL_CONVERT_RGB565
	case AIPL_COLOR_RGB565:
		return aipl_color_convert_rgb565(input, output, pitch, width, height,
						 output_format);
#endif
#if AIPL_CONVERT_BGR888
	case AIPL_COLOR_BGR888:
		return aipl_color_convert_bgr888(input, output, pitch, width, height,
						 output_format);
#endif
#if AIPL_CONVERT_RGB888
	case AIPL_COLOR_RGB888:
		return aipl_color_convert_rgb888(input, output, pitch, width, height,
						 output_format);
#endif

		/* YUV color formats */
#if AIPL_CONVERT_YV12
	case AIPL_COLOR_YV12:
		return aipl_color_convert_yv12(input, output, pitch, width, height, output_format);
#endif
#if AIPL_CONVERT_I420
	case AIPL_COLOR_I420:
		return aipl_color_convert_i420(input, output, pitch, width, height, output_format);
#endif
#if AIPL_CONVERT_NV12
	case AIPL_COLOR_NV12:
		return aipl_color_convert_nv12(input, output, pitch, width, height, output_format);
#endif
#if AIPL_CONVERT_NV21
	case AIPL_COLOR_NV21:
		return aipl_color_convert_nv21(input, output, pitch, width, height, output_format);
#endif
#if AIPL_CONVERT_I422
	case AIPL_COLOR_I422:
		return aipl_color_convert_i422(input, output, pitch, width, height, output_format);
#endif
#if AIPL_CONVERT_YUY2
	case AIPL_COLOR_YUY2:
		return aipl_color_convert_yuy2(input, output, pitch, width, height, output_format);
#endif
#if AIPL_CONVERT_UYVY
	case AIPL_COLOR_UYVY:
		return aipl_color_convert_uyvy(input, output, pitch, width, height, output_format);
#endif
#if AIPL_CONVERT_I444
	case AIPL_COLOR_I444:
		return aipl_color_convert_i444(input, output, pitch, width, height, output_format);
#endif
#if AIPL_CONVERT_ALPHA8_I400
	case AIPL_COLOR_I400:
		return aipl_color_convert_i400(input, output, pitch, width, height, output_format);
#endif

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

aipl_error_t aipl_color_convert_img(const aipl_image_t *input, aipl_image_t *output)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	if (input->width != output->width || input->height != output->height) {
		return AIPL_ERR_SIZE_MISMATCH;
	}

	return aipl_color_convert(input->data, output->data, input->pitch, input->width,
				  input->height, input->format, output->format);
}

#if AIPL_CONVERT_ALPHA8_I400
aipl_error_t aipl_color_convert_alpha8(const void *input, void *output, uint32_t pitch,
				       uint32_t width, uint32_t height, aipl_color_format_t format)
{
#ifdef AIPL_DAVE2D_ACCELERATION
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	if (aipl_dave2d_color_convert_suitable(AIPL_COLOR_ALPHA8, format)) {
		d2_s32 ret = aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
							    d2_mode_alpha8,
							    aipl_dave2d_format_to_mode(format));

		return aipl_dave2d_error_convert(ret);
	}
#endif

	switch (format) {
	case AIPL_COLOR_ALPHA8:
		return AIPL_ERR_FORMAT_MISMATCH;

	case AIPL_COLOR_I400:
		return aipl_color_convert_alpha8_to_i400(input, output, pitch, width, height);

	default:
		return aipl_color_convert_i400(input, output, pitch, width, height, format);
	}
}

aipl_error_t aipl_color_convert_alpha8_to_i400(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
	return aipl_color_convert_alpha8_to_i400_default(input, output, pitch, width, height);
}

#if (AIPL_CONVERT_ALPHA8_I400 & TO_ARGB8888)
aipl_error_t aipl_color_convert_alpha8_to_argb8888(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#ifdef AIPL_DAVE2D_ACCELERATION
	return aipl_color_convert_alpha8_to_argb8888_dave2d(input, output, pitch, width, height);
#else
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_alpha8_to_argb8888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_alpha8_to_argb8888_default(input, output, pitch, width, height);
#endif
#endif
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_ARGB4444)
aipl_error_t aipl_color_convert_alpha8_to_argb4444(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#ifdef AIPL_DAVE2D_ACCELERATION
	return aipl_color_convert_alpha8_to_argb4444_dave2d(input, output, pitch, width, height);
#else
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_alpha8_to_argb4444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_alpha8_to_argb4444_default(input, output, pitch, width, height);
#endif
#endif
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_ARGB1555)
aipl_error_t aipl_color_convert_alpha8_to_argb1555(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_alpha8_to_argb1555_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_alpha8_to_argb1555_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_RGBA8888)
aipl_error_t aipl_color_convert_alpha8_to_rgba8888(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#ifdef AIPL_DAVE2D_ACCELERATION
	return aipl_color_convert_alpha8_to_rgba8888_dave2d(input, output, pitch, width, height);
#else
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_alpha8_to_rgba8888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_alpha8_to_rgba8888_default(input, output, pitch, width, height);
#endif
#endif
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_RGBA4444)
aipl_error_t aipl_color_convert_alpha8_to_rgba4444(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#ifdef AIPL_DAVE2D_ACCELERATION
	return aipl_color_convert_alpha8_to_rgba4444_dave2d(input, output, pitch, width, height);
#else
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_alpha8_to_rgba4444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_alpha8_to_rgba4444_default(input, output, pitch, width, height);
#endif
#endif
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_RGBA5551)
aipl_error_t aipl_color_convert_alpha8_to_rgba5551(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_alpha8_to_rgba5551_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_alpha8_to_rgba5551_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_BGR888)
aipl_error_t aipl_color_convert_alpha8_to_bgr888(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_alpha8_to_bgr888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_alpha8_to_bgr888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_RGB888)
aipl_error_t aipl_color_convert_alpha8_to_rgb888(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_alpha8_to_rgb888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_alpha8_to_rgb888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_RGB565)
aipl_error_t aipl_color_convert_alpha8_to_rgb565(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_DAVE2D_ACCELERATION
	return aipl_color_convert_alpha8_to_rgb565_dave2d(input, output, pitch, width, height);
#else
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_alpha8_to_rgb565_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_alpha8_to_rgb565_default(input, output, pitch, width, height);
#endif
#endif
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_YV12)
aipl_error_t aipl_color_convert_alpha8_to_yv12(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
	return aipl_color_convert_alpha8_to_yv12_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_I420)
aipl_error_t aipl_color_convert_alpha8_to_i420(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
	return aipl_color_convert_alpha8_to_i420_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_I422)
aipl_error_t aipl_color_convert_alpha8_to_i422(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
	return aipl_color_convert_alpha8_to_i422_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_I444)
aipl_error_t aipl_color_convert_alpha8_to_i444(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
	return aipl_color_convert_alpha8_to_i444_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_NV12)
aipl_error_t aipl_color_convert_alpha8_to_nv12(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
	return aipl_color_convert_alpha8_to_nv12_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_NV21)
aipl_error_t aipl_color_convert_alpha8_to_nv21(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
	return aipl_color_convert_alpha8_to_nv21_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_YUY2)
aipl_error_t aipl_color_convert_alpha8_to_yuy2(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_alpha8_to_yuy2_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_alpha8_to_yuy2_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_UYVY)
aipl_error_t aipl_color_convert_alpha8_to_uyvy(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_alpha8_to_uyvy_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_alpha8_to_uyvy_default(input, output, pitch, width, height);
#endif
}
#endif
#endif

#if AIPL_CONVERT_ARGB8888
aipl_error_t aipl_color_convert_argb8888(const void *input, void *output, uint32_t pitch,
					 uint32_t width, uint32_t height,
					 aipl_color_format_t format)
{
#ifdef AIPL_DAVE2D_ACCELERATION
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	if (aipl_dave2d_color_convert_suitable(AIPL_COLOR_ARGB8888, format)) {
		d2_s32 ret = aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
							    d2_mode_argb8888,
							    aipl_dave2d_format_to_mode(format));

		return aipl_dave2d_error_convert(ret);
	}
#endif

	switch (format) {
	/* Alpha color formats */
#if (AIPL_CONVERT_ARGB8888 & TO_ALPHA8_I400)
	case AIPL_COLOR_ALPHA8:
		return aipl_color_convert_argb8888_to_alpha8(input, output, pitch, width, height);
#endif
	/* RGB color formats */
	case AIPL_COLOR_ARGB8888:
		return AIPL_ERR_FORMAT_MISMATCH;
#if (AIPL_CONVERT_ARGB8888 & TO_RGBA8888)
	case AIPL_COLOR_RGBA8888:
		return aipl_color_convert_argb8888_to_rgba8888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ARGB8888 & TO_ARGB4444)
	case AIPL_COLOR_ARGB4444:
		return aipl_color_convert_argb8888_to_argb4444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ARGB8888 & TO_ARGB1555)
	case AIPL_COLOR_ARGB1555:
		return aipl_color_convert_argb8888_to_argb1555(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ARGB8888 & TO_RGBA4444)
	case AIPL_COLOR_RGBA4444:
		return aipl_color_convert_argb8888_to_rgba4444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ARGB8888 & TO_RGBA5551)
	case AIPL_COLOR_RGBA5551:
		return aipl_color_convert_argb8888_to_rgba5551(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ARGB8888 & TO_RGB565)
	case AIPL_COLOR_RGB565:
		return aipl_color_convert_argb8888_to_rgb565(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ARGB8888 & TO_BGR888)
	case AIPL_COLOR_BGR888:
		return aipl_color_convert_argb8888_to_bgr888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ARGB8888 & TO_RGB888)
	case AIPL_COLOR_RGB888:
		return aipl_color_convert_argb8888_to_rgb888(input, output, pitch, width, height);
#endif
    /* YUV color formats */
#if (AIPL_CONVERT_ARGB8888 & TO_YV12)
	case AIPL_COLOR_YV12:
		return aipl_color_convert_argb8888_to_yv12(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ARGB8888 & TO_I420)
	case AIPL_COLOR_I420:
		return aipl_color_convert_argb8888_to_i420(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ARGB8888 & TO_NV12)
	case AIPL_COLOR_NV12:
		return aipl_color_convert_argb8888_to_nv12(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ARGB8888 & TO_NV21)
	case AIPL_COLOR_NV21:
		return aipl_color_convert_argb8888_to_nv21(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ARGB8888 & TO_I422)
	case AIPL_COLOR_I422:
		return aipl_color_convert_argb8888_to_i422(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ARGB8888 & TO_YUY2)
	case AIPL_COLOR_YUY2:
		return aipl_color_convert_argb8888_to_yuy2(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ARGB8888 & TO_UYVY)
	case AIPL_COLOR_UYVY:
		return aipl_color_convert_argb8888_to_uyvy(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ARGB8888 & TO_I444)
	case AIPL_COLOR_I444:
		return aipl_color_convert_argb8888_to_i444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ARGB8888 & TO_ALPHA8_I400)
	case AIPL_COLOR_I400:
		return aipl_color_convert_argb8888_to_i400(input, output, pitch, width, height);
#endif

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

#if (AIPL_CONVERT_ARGB8888 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_argb8888_to_alpha8(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb8888_to_alpha8_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb8888_to_alpha8_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ARGB8888 & TO_ARGB4444)
aipl_error_t aipl_color_convert_argb8888_to_argb4444(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
#if (defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD))
	return aipl_color_convert_argb8888_to_argb4444_dave2d(input, output, pitch, width, height);
#else
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb8888_to_argb4444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb8888_to_argb4444_default(input, output, pitch, width, height);
#endif
#endif
}
#endif

#if (AIPL_CONVERT_ARGB8888 & TO_ARGB1555)
aipl_error_t aipl_color_convert_argb8888_to_argb1555(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb8888_to_argb1555_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb8888_to_argb1555_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ARGB8888 & TO_RGBA8888)
aipl_error_t aipl_color_convert_argb8888_to_rgba8888(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
#ifdef AIPL_DAVE2D_ACCELERATION
	return aipl_color_convert_argb8888_to_rgba8888_dave2d(input, output, pitch, width, height);
#else
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb8888_to_rgba8888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb8888_to_rgba8888_default(input, output, pitch, width, height);
#endif
#endif
}
#endif

#if (AIPL_CONVERT_ARGB8888 & TO_RGBA4444)
aipl_error_t aipl_color_convert_argb8888_to_rgba4444(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
#if (defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD))
	return aipl_color_convert_argb8888_to_rgba4444_dave2d(input, output, pitch, width, height);
#else
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb8888_to_rgba4444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb8888_to_rgba4444_default(input, output, pitch, width, height);
#endif
#endif
}
#endif

#if (AIPL_CONVERT_ARGB8888 & TO_RGBA5551)
aipl_error_t aipl_color_convert_argb8888_to_rgba5551(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb8888_to_rgba5551_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb8888_to_rgba5551_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ARGB8888 & TO_BGR888)
aipl_error_t aipl_color_convert_argb8888_to_bgr888(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb8888_to_bgr888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb8888_to_bgr888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ARGB8888 & TO_RGB888)
aipl_error_t aipl_color_convert_argb8888_to_rgb888(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb8888_to_rgb888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb8888_to_rgb888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ARGB8888 & TO_RGB565)
aipl_error_t aipl_color_convert_argb8888_to_rgb565(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#ifdef AIPL_DAVE2D_ACCELERATION
	return aipl_color_convert_argb8888_to_rgb565_dave2d(input, output, pitch, width, height);
#else
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb8888_to_rgb565_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb8888_to_rgb565_default(input, output, pitch, width, height);
#endif
#endif
}
#endif

#if (AIPL_CONVERT_ARGB8888 & TO_YV12)
aipl_error_t aipl_color_convert_argb8888_to_yv12(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb8888_to_yv12_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb8888_to_yv12_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ARGB8888 & TO_I420)
aipl_error_t aipl_color_convert_argb8888_to_i420(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb8888_to_i420_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb8888_to_i420_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ARGB8888 & TO_I422)
aipl_error_t aipl_color_convert_argb8888_to_i422(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb8888_to_i422_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb8888_to_i422_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ARGB8888 & TO_I444)
aipl_error_t aipl_color_convert_argb8888_to_i444(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb8888_to_i444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb8888_to_i444_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ARGB8888 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_argb8888_to_i400(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb8888_to_i400_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb8888_to_i400_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ARGB8888 & TO_NV21)
aipl_error_t aipl_color_convert_argb8888_to_nv21(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb8888_to_nv21_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb8888_to_nv21_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ARGB8888 & TO_NV12)
aipl_error_t aipl_color_convert_argb8888_to_nv12(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb8888_to_nv12_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb8888_to_nv12_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ARGB8888 & TO_YUY2)
aipl_error_t aipl_color_convert_argb8888_to_yuy2(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb8888_to_yuy2_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb8888_to_yuy2_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ARGB8888 & TO_UYVY)
aipl_error_t aipl_color_convert_argb8888_to_uyvy(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb8888_to_uyvy_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb8888_to_uyvy_default(input, output, pitch, width, height);
#endif
}
#endif
#endif

#if AIPL_CONVERT_ARGB4444
aipl_error_t aipl_color_convert_argb4444(const void *input, void *output, uint32_t pitch,
					 uint32_t width, uint32_t height,
					 aipl_color_format_t format)
{
#ifdef AIPL_DAVE2D_ACCELERATION
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	if (aipl_dave2d_color_convert_suitable(AIPL_COLOR_ARGB4444, format)) {
		d2_s32 ret = aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
							    d2_mode_argb4444,
							    aipl_dave2d_format_to_mode(format));

		return aipl_dave2d_error_convert(ret);
	}
#endif

	switch (format) {
	/* Alpha color formats */
#if (AIPL_CONVERT_ARGB4444 & TO_ALPHA8_I400)
	case AIPL_COLOR_ALPHA8:
		return aipl_color_convert_argb4444_to_alpha8(input, output, pitch, width, height);
#endif
		/* RGB color formats */
#if (AIPL_CONVERT_ARGB4444 & TO_ARGB8888)
	case AIPL_COLOR_ARGB8888:
		return aipl_color_convert_argb4444_to_argb8888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ARGB4444 & TO_RGBA8888)
	case AIPL_COLOR_RGBA8888:
		return aipl_color_convert_argb4444_to_rgba8888(input, output, pitch, width, height);
#endif
	case AIPL_COLOR_ARGB4444:
		return AIPL_ERR_FORMAT_MISMATCH;
#if (AIPL_CONVERT_ARGB4444 & TO_ARGB1555)
	case AIPL_COLOR_ARGB1555:
		return aipl_color_convert_argb4444_to_argb1555(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ARGB4444 & TO_RGBA4444)
	case AIPL_COLOR_RGBA4444:
		return aipl_color_convert_argb4444_to_rgba4444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ARGB4444 & TO_RGBA5551)
	case AIPL_COLOR_RGBA5551:
		return aipl_color_convert_argb4444_to_rgba5551(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ARGB4444 & TO_RGB565)
	case AIPL_COLOR_RGB565:
		return aipl_color_convert_argb4444_to_rgb565(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ARGB4444 & TO_BGR888)
	case AIPL_COLOR_BGR888:
		return aipl_color_convert_argb4444_to_bgr888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ARGB4444 & TO_RGB888)
	case AIPL_COLOR_RGB888:
		return aipl_color_convert_argb4444_to_rgb888(input, output, pitch, width, height);
#endif
	/* YUV color formats */
#if (AIPL_CONVERT_ARGB4444 & TO_YV12)
	case AIPL_COLOR_YV12:
		return aipl_color_convert_argb4444_to_yv12(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ARGB4444 & TO_I420)
	case AIPL_COLOR_I420:
		return aipl_color_convert_argb4444_to_i420(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ARGB4444 & TO_NV12)
	case AIPL_COLOR_NV12:
		return aipl_color_convert_argb4444_to_nv12(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ARGB4444 & TO_NV21)
	case AIPL_COLOR_NV21:
		return aipl_color_convert_argb4444_to_nv21(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ARGB4444 & TO_I422)
	case AIPL_COLOR_I422:
		return aipl_color_convert_argb4444_to_i422(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ARGB4444 & TO_YUY2)
	case AIPL_COLOR_YUY2:
		return aipl_color_convert_argb4444_to_yuy2(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ARGB4444 & TO_UYVY)
	case AIPL_COLOR_UYVY:
		return aipl_color_convert_argb4444_to_uyvy(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ARGB4444 & TO_I444)
	case AIPL_COLOR_I444:
		return aipl_color_convert_argb4444_to_i444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ARGB4444 & TO_ALPHA8_I400)
	case AIPL_COLOR_I400:
		return aipl_color_convert_argb4444_to_i400(input, output, pitch, width, height);
#endif

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

#if (AIPL_CONVERT_ARGB4444 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_argb4444_to_alpha8(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb4444_to_alpha8_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb4444_to_alpha8_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ARGB4444 & TO_ARGB8888)
aipl_error_t aipl_color_convert_argb4444_to_argb8888(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
#ifdef AIPL_DAVE2D_ACCELERATION
	return aipl_color_convert_argb4444_to_argb8888_dave2d(input, output, pitch, width, height);
#else
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb4444_to_argb8888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb4444_to_argb8888_default(input, output, pitch, width, height);
#endif
#endif
}
#endif

#if (AIPL_CONVERT_ARGB4444 & TO_ARGB1555)
aipl_error_t aipl_color_convert_argb4444_to_argb1555(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	return aipl_color_convert_argb4444_to_argb1555_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_ARGB4444 & TO_RGBA8888)
aipl_error_t aipl_color_convert_argb4444_to_rgba8888(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
#ifdef AIPL_DAVE2D_ACCELERATION
	return aipl_color_convert_argb4444_to_rgba8888_dave2d(input, output, pitch, width, height);
#else
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb4444_to_rgba8888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb4444_to_rgba8888_default(input, output, pitch, width, height);
#endif
#endif
}
#endif

#if (AIPL_CONVERT_ARGB4444 & TO_RGBA4444)
aipl_error_t aipl_color_convert_argb4444_to_rgba4444(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
#if (defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD))
	return aipl_color_convert_argb4444_to_rgba4444_dave2d(input, output, pitch, width, height);
#else
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb4444_to_rgba4444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb4444_to_rgba4444_default(input, output, pitch, width, height);
#endif
#endif
}
#endif

#if (AIPL_CONVERT_ARGB4444 & TO_RGBA5551)
aipl_error_t aipl_color_convert_argb4444_to_rgba5551(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	return aipl_color_convert_argb4444_to_rgba5551_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_ARGB4444 & TO_BGR888)
aipl_error_t aipl_color_convert_argb4444_to_bgr888(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb4444_to_bgr888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb4444_to_bgr888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ARGB4444 & TO_RGB888)
aipl_error_t aipl_color_convert_argb4444_to_rgb888(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb4444_to_rgb888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb4444_to_rgb888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ARGB4444 & TO_RGB565)
aipl_error_t aipl_color_convert_argb4444_to_rgb565(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#if (defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD))
	return aipl_color_convert_argb4444_to_rgb565_dave2d(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb4444_to_rgb565_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ARGB4444 & TO_YV12)
aipl_error_t aipl_color_convert_argb4444_to_yv12(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb4444_to_yv12_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb4444_to_yv12_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ARGB4444 & TO_I420)
aipl_error_t aipl_color_convert_argb4444_to_i420(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb4444_to_i420_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb4444_to_i420_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ARGB4444 & TO_I422)
aipl_error_t aipl_color_convert_argb4444_to_i422(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb4444_to_i422_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb4444_to_i422_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ARGB4444 & TO_I444)
aipl_error_t aipl_color_convert_argb4444_to_i444(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb4444_to_i444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb4444_to_i444_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ARGB4444 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_argb4444_to_i400(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb4444_to_i400_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb4444_to_i400_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ARGB4444 & TO_NV21)
aipl_error_t aipl_color_convert_argb4444_to_nv21(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb4444_to_nv21_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb4444_to_nv21_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ARGB4444 & TO_NV12)
aipl_error_t aipl_color_convert_argb4444_to_nv12(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb4444_to_nv12_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb4444_to_nv12_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ARGB4444 & TO_YUY2)
aipl_error_t aipl_color_convert_argb4444_to_yuy2(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb4444_to_yuy2_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb4444_to_yuy2_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ARGB4444 & TO_UYVY)
aipl_error_t aipl_color_convert_argb4444_to_uyvy(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb4444_to_uyvy_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb4444_to_uyvy_default(input, output, pitch, width, height);
#endif
}
#endif
#endif

#if AIPL_CONVERT_ARGB1555
aipl_error_t aipl_color_convert_argb1555(const void *input, void *output, uint32_t pitch,
					 uint32_t width, uint32_t height,
					 aipl_color_format_t format)
{
#ifdef AIPL_DAVE2D_ACCELERATION
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	if (aipl_dave2d_color_convert_suitable(AIPL_COLOR_ARGB1555, format)) {
		d2_s32 ret = aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
							    d2_mode_argb1555,
							    aipl_dave2d_format_to_mode(format));

		return aipl_dave2d_error_convert(ret);
	}
#endif

	switch (format) {
	/* Alpha color formats */
#if (AIPL_CONVERT_ARGB1555 & TO_ALPHA8_I400)
	case AIPL_COLOR_ALPHA8:
		return aipl_color_convert_argb1555_to_alpha8(input, output, pitch, width, height);
#endif
	/* RGB color formats */
#if (AIPL_CONVERT_ARGB1555 & TO_ARGB8888)
	case AIPL_COLOR_ARGB8888:
		return aipl_color_convert_argb1555_to_argb8888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ARGB1555 & TO_RGBA8888)
	case AIPL_COLOR_RGBA8888:
		return aipl_color_convert_argb1555_to_rgba8888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ARGB1555 & TO_ARGB4444)
	case AIPL_COLOR_ARGB4444:
		return aipl_color_convert_argb1555_to_argb4444(input, output, pitch, width, height);
#endif
	case AIPL_COLOR_ARGB1555:
		return AIPL_ERR_FORMAT_MISMATCH;
#if (AIPL_CONVERT_ARGB1555 & TO_RGBA4444)
	case AIPL_COLOR_RGBA4444:
		return aipl_color_convert_argb1555_to_rgba4444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ARGB1555 & TO_RGBA5551)
	case AIPL_COLOR_RGBA5551:
		return aipl_color_convert_argb1555_to_rgba5551(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ARGB1555 & TO_RGB565)
	case AIPL_COLOR_RGB565:
		return aipl_color_convert_argb1555_to_rgb565(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ARGB1555 & TO_BGR888)
	case AIPL_COLOR_BGR888:
		return aipl_color_convert_argb1555_to_bgr888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ARGB1555 & TO_RGB888)
	case AIPL_COLOR_RGB888:
		return aipl_color_convert_argb1555_to_rgb888(input, output, pitch, width, height);
#endif
	/* YUV color formats */
#if (AIPL_CONVERT_ARGB1555 & TO_YV12)
	case AIPL_COLOR_YV12:
		return aipl_color_convert_argb1555_to_yv12(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ARGB1555 & TO_I420)
	case AIPL_COLOR_I420:
		return aipl_color_convert_argb1555_to_i420(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ARGB1555 & TO_NV12)
	case AIPL_COLOR_NV12:
		return aipl_color_convert_argb1555_to_nv12(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ARGB1555 & TO_NV21)
	case AIPL_COLOR_NV21:
		return aipl_color_convert_argb1555_to_nv21(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ARGB1555 & TO_I422)
	case AIPL_COLOR_I422:
		return aipl_color_convert_argb1555_to_i422(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ARGB1555 & TO_YUY2)
	case AIPL_COLOR_YUY2:
		return aipl_color_convert_argb1555_to_yuy2(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ARGB1555 & TO_UYVY)
	case AIPL_COLOR_UYVY:
		return aipl_color_convert_argb1555_to_uyvy(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ARGB1555 & TO_I444)
	case AIPL_COLOR_I444:
		return aipl_color_convert_argb1555_to_i444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ARGB1555 & TO_ALPHA8_I400)
	case AIPL_COLOR_I400:
		return aipl_color_convert_argb1555_to_i400(input, output, pitch, width, height);
#endif

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

#if (AIPL_CONVERT_ARGB1555 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_argb1555_to_alpha8(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb1555_to_alpha8_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb1555_to_alpha8_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ARGB1555 & TO_ARGB8888)
aipl_error_t aipl_color_convert_argb1555_to_argb8888(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
#ifdef AIPL_DAVE2D_ACCELERATION
	return aipl_color_convert_argb1555_to_argb8888_dave2d(input, output, pitch, width, height);
#else
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb1555_to_argb8888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb1555_to_argb8888_default(input, output, pitch, width, height);
#endif
#endif
}
#endif

#if (AIPL_CONVERT_ARGB1555 & TO_ARGB4444)
aipl_error_t aipl_color_convert_argb1555_to_argb4444(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
#if (defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD))
	return aipl_color_convert_argb1555_to_argb4444_dave2d(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb1555_to_argb4444_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ARGB1555 & TO_RGBA8888)
aipl_error_t aipl_color_convert_argb1555_to_rgba8888(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
#ifdef AIPL_DAVE2D_ACCELERATION
	return aipl_color_convert_argb1555_to_rgba8888_dave2d(input, output, pitch, width, height);
#else
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb1555_to_rgba8888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb1555_to_rgba8888_default(input, output, pitch, width, height);
#endif
#endif
}
#endif

#if (AIPL_CONVERT_ARGB1555 & TO_RGBA4444)
aipl_error_t aipl_color_convert_argb1555_to_rgba4444(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
#if (defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD))
	return aipl_color_convert_argb1555_to_rgba4444_dave2d(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb1555_to_rgba4444_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ARGB1555 & TO_RGBA5551)
aipl_error_t aipl_color_convert_argb1555_to_rgba5551(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	return aipl_color_convert_argb1555_to_rgba5551_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_ARGB1555 & TO_BGR888)
aipl_error_t aipl_color_convert_argb1555_to_bgr888(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb1555_to_bgr888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb1555_to_bgr888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ARGB1555 & TO_RGB888)
aipl_error_t aipl_color_convert_argb1555_to_rgb888(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb1555_to_rgb888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb1555_to_rgb888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ARGB1555 & TO_RGB565)
aipl_error_t aipl_color_convert_argb1555_to_rgb565(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#if (defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD))
	return aipl_color_convert_argb1555_to_rgb565_dave2d(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb1555_to_rgb565_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ARGB1555 & TO_YV12)
aipl_error_t aipl_color_convert_argb1555_to_yv12(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb1555_to_yv12_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb1555_to_yv12_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ARGB1555 & TO_I420)
aipl_error_t aipl_color_convert_argb1555_to_i420(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb1555_to_i420_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb1555_to_i420_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ARGB1555 & TO_I422)
aipl_error_t aipl_color_convert_argb1555_to_i422(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb1555_to_i422_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb1555_to_i422_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ARGB1555 & TO_I444)
aipl_error_t aipl_color_convert_argb1555_to_i444(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb1555_to_i444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb1555_to_i444_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ARGB1555 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_argb1555_to_i400(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb1555_to_i400_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb1555_to_i400_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ARGB1555 & TO_NV21)
aipl_error_t aipl_color_convert_argb1555_to_nv21(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb1555_to_nv21_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb1555_to_nv21_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ARGB1555 & TO_NV12)
aipl_error_t aipl_color_convert_argb1555_to_nv12(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb1555_to_nv12_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb1555_to_nv12_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ARGB1555 & TO_YUY2)
aipl_error_t aipl_color_convert_argb1555_to_yuy2(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb1555_to_yuy2_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb1555_to_yuy2_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ARGB1555 & TO_UYVY)
aipl_error_t aipl_color_convert_argb1555_to_uyvy(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_argb1555_to_uyvy_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_argb1555_to_uyvy_default(input, output, pitch, width, height);
#endif
}
#endif
#endif

#if AIPL_CONVERT_RGBA8888
aipl_error_t aipl_color_convert_rgba8888(const void *input, void *output, uint32_t pitch,
					 uint32_t width, uint32_t height,
					 aipl_color_format_t format)
{
#ifdef AIPL_DAVE2D_ACCELERATION
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	if (aipl_dave2d_color_convert_suitable(AIPL_COLOR_RGBA8888, format)) {
		d2_s32 ret = aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
							    d2_mode_rgba8888,
							    aipl_dave2d_format_to_mode(format));

		return aipl_dave2d_error_convert(ret);
	}
#endif

	switch (format) {
	/* Alpha color formats */
#if (AIPL_CONVERT_RGBA8888 & TO_ALPHA8_I400)
	case AIPL_COLOR_ALPHA8:
		return aipl_color_convert_rgba8888_to_alpha8(input, output, pitch, width, height);
#endif
	/* RGB color formats */
#if (AIPL_CONVERT_RGBA8888 & TO_ARGB8888)
	case AIPL_COLOR_ARGB8888:
		return aipl_color_convert_rgba8888_to_argb8888(input, output, pitch, width, height);
#endif
	case AIPL_COLOR_RGBA8888:
		return AIPL_ERR_FORMAT_MISMATCH;
#if (AIPL_CONVERT_RGBA8888 & TO_ARGB4444)
	case AIPL_COLOR_ARGB4444:
		return aipl_color_convert_rgba8888_to_argb4444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGBA8888 & TO_ARGB1555)
	case AIPL_COLOR_ARGB1555:
		return aipl_color_convert_rgba8888_to_argb1555(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGBA8888 & TO_RGBA4444)
	case AIPL_COLOR_RGBA4444:
		return aipl_color_convert_rgba8888_to_rgba4444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGBA8888 & TO_RGBA5551)
	case AIPL_COLOR_RGBA5551:
		return aipl_color_convert_rgba8888_to_rgba5551(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGBA8888 & TO_RGB565)
	case AIPL_COLOR_RGB565:
		return aipl_color_convert_rgba8888_to_rgb565(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGBA8888 & TO_BGR888)
	case AIPL_COLOR_BGR888:
		return aipl_color_convert_rgba8888_to_bgr888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGBA8888 & TO_RGB888)
	case AIPL_COLOR_RGB888:
		return aipl_color_convert_rgba8888_to_rgb888(input, output, pitch, width, height);
#endif
	/* YUV color formats */
#if (AIPL_CONVERT_RGBA8888 & TO_YV12)
	case AIPL_COLOR_YV12:
		return aipl_color_convert_rgba8888_to_yv12(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGBA8888 & TO_I420)
	case AIPL_COLOR_I420:
		return aipl_color_convert_rgba8888_to_i420(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGBA8888 & TO_NV12)
	case AIPL_COLOR_NV12:
		return aipl_color_convert_rgba8888_to_nv12(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGBA8888 & TO_NV21)
	case AIPL_COLOR_NV21:
		return aipl_color_convert_rgba8888_to_nv21(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGBA8888 & TO_I422)
	case AIPL_COLOR_I422:
		return aipl_color_convert_rgba8888_to_i422(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGBA8888 & TO_YUY2)
	case AIPL_COLOR_YUY2:
		return aipl_color_convert_rgba8888_to_yuy2(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGBA8888 & TO_UYVY)
	case AIPL_COLOR_UYVY:
		return aipl_color_convert_rgba8888_to_uyvy(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGBA8888 & TO_I444)
	case AIPL_COLOR_I444:
		return aipl_color_convert_rgba8888_to_i444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGBA8888 & TO_ALPHA8_I400)
	case AIPL_COLOR_I400:
		return aipl_color_convert_rgba8888_to_i400(input, output, pitch, width, height);
#endif

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

#if (AIPL_CONVERT_RGBA8888 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_rgba8888_to_alpha8(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba8888_to_alpha8_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba8888_to_alpha8_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGBA8888 & TO_ARGB8888)
aipl_error_t aipl_color_convert_rgba8888_to_argb8888(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
#ifdef AIPL_DAVE2D_ACCELERATION
	return aipl_color_convert_rgba8888_to_argb8888_dave2d(input, output, pitch, width, height);
#else
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba8888_to_argb8888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba8888_to_argb8888_default(input, output, pitch, width, height);
#endif
#endif
}
#endif

#if (AIPL_CONVERT_RGBA8888 & TO_ARGB4444)
aipl_error_t aipl_color_convert_rgba8888_to_argb4444(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
#if (defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD))
	return aipl_color_convert_rgba8888_to_argb4444_dave2d(input, output, pitch, width, height);
#else
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba8888_to_argb4444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba8888_to_argb4444_default(input, output, pitch, width, height);
#endif
#endif
}
#endif

#if (AIPL_CONVERT_RGBA8888 & TO_ARGB1555)
aipl_error_t aipl_color_convert_rgba8888_to_argb1555(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba8888_to_argb1555_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba8888_to_argb1555_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGBA8888 & TO_RGBA4444)
aipl_error_t aipl_color_convert_rgba8888_to_rgba4444(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
#if (defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD))
	return aipl_color_convert_rgba8888_to_rgba4444_dave2d(input, output, pitch, width, height);
#else
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba8888_to_rgba4444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba8888_to_rgba4444_default(input, output, pitch, width, height);
#endif
#endif
}
#endif

#if (AIPL_CONVERT_RGBA8888 & TO_RGBA5551)
aipl_error_t aipl_color_convert_rgba8888_to_rgba5551(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba8888_to_rgba5551_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba8888_to_rgba5551_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGBA8888 & TO_BGR888)
aipl_error_t aipl_color_convert_rgba8888_to_bgr888(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba8888_to_bgr888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba8888_to_bgr888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGBA8888 & TO_RGB888)
aipl_error_t aipl_color_convert_rgba8888_to_rgb888(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba8888_to_rgb888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba8888_to_rgb888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGBA8888 & TO_RGB565)
aipl_error_t aipl_color_convert_rgba8888_to_rgb565(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#ifdef AIPL_DAVE2D_ACCELERATION
	return aipl_color_convert_rgba8888_to_rgb565_dave2d(input, output, pitch, width, height);
#else
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba8888_to_rgb565_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba8888_to_rgb565_default(input, output, pitch, width, height);
#endif
#endif
}
#endif

#if (AIPL_CONVERT_RGBA8888 & TO_YV12)
aipl_error_t aipl_color_convert_rgba8888_to_yv12(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba8888_to_yv12_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba8888_to_yv12_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGBA8888 & TO_I420)
aipl_error_t aipl_color_convert_rgba8888_to_i420(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba8888_to_i420_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba8888_to_i420_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGBA8888 & TO_I422)
aipl_error_t aipl_color_convert_rgba8888_to_i422(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba8888_to_i422_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba8888_to_i422_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGBA8888 & TO_I444)
aipl_error_t aipl_color_convert_rgba8888_to_i444(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba8888_to_i444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba8888_to_i444_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGBA8888 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_rgba8888_to_i400(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba8888_to_i400_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba8888_to_i400_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGBA8888 & TO_NV21)
aipl_error_t aipl_color_convert_rgba8888_to_nv21(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba8888_to_nv21_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba8888_to_nv21_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGBA8888 & TO_NV12)
aipl_error_t aipl_color_convert_rgba8888_to_nv12(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba8888_to_nv12_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba8888_to_nv12_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGBA8888 & TO_YUY2)
aipl_error_t aipl_color_convert_rgba8888_to_yuy2(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba8888_to_yuy2_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba8888_to_yuy2_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGBA8888 & TO_UYVY)
aipl_error_t aipl_color_convert_rgba8888_to_uyvy(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba8888_to_uyvy_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba8888_to_uyvy_default(input, output, pitch, width, height);
#endif
}
#endif
#endif

#if AIPL_CONVERT_RGBA4444
aipl_error_t aipl_color_convert_rgba4444(const void *input, void *output, uint32_t pitch,
					 uint32_t width, uint32_t height,
					 aipl_color_format_t format)
{
#ifdef AIPL_DAVE2D_ACCELERATION
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	if (aipl_dave2d_color_convert_suitable(AIPL_COLOR_RGBA4444, format)) {
		d2_s32 ret = aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
							    d2_mode_rgba4444,
							    aipl_dave2d_format_to_mode(format));

		return aipl_dave2d_error_convert(ret);
	}
#endif

	switch (format) {
	/* Alpha color formats */
#if (AIPL_CONVERT_RGBA4444 & TO_ALPHA8_I400)
	case AIPL_COLOR_ALPHA8:
		return aipl_color_convert_rgba4444_to_alpha8(input, output, pitch, width, height);
#endif
	/* RGB color formats */
#if (AIPL_CONVERT_RGBA4444 & TO_ARGB8888)
	case AIPL_COLOR_ARGB8888:
		return aipl_color_convert_rgba4444_to_argb8888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGBA4444 & TO_RGBA8888)
	case AIPL_COLOR_RGBA8888:
		return aipl_color_convert_rgba4444_to_rgba8888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGBA4444 & TO_ARGB4444)
	case AIPL_COLOR_ARGB4444:
		return aipl_color_convert_rgba4444_to_argb4444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGBA4444 & TO_ARGB1555)
	case AIPL_COLOR_ARGB1555:
		return aipl_color_convert_rgba4444_to_argb1555(input, output, pitch, width, height);
#endif
	case AIPL_COLOR_RGBA4444:
		return AIPL_ERR_FORMAT_MISMATCH;
#if (AIPL_CONVERT_RGBA4444 & TO_RGBA5551)
	case AIPL_COLOR_RGBA5551:
		return aipl_color_convert_rgba4444_to_rgba5551(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGBA4444 & TO_RGB565)
	case AIPL_COLOR_RGB565:
		return aipl_color_convert_rgba4444_to_rgb565(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGBA4444 & TO_BGR888)
	case AIPL_COLOR_BGR888:
		return aipl_color_convert_rgba4444_to_bgr888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGBA4444 & TO_RGB888)
	case AIPL_COLOR_RGB888:
		return aipl_color_convert_rgba4444_to_rgb888(input, output, pitch, width, height);
#endif
	/* YUV color formats */
#if (AIPL_CONVERT_RGBA4444 & TO_YV12)
	case AIPL_COLOR_YV12:
		return aipl_color_convert_rgba4444_to_yv12(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGBA4444 & TO_I420)
	case AIPL_COLOR_I420:
		return aipl_color_convert_rgba4444_to_i420(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGBA4444 & TO_NV12)
	case AIPL_COLOR_NV12:
		return aipl_color_convert_rgba4444_to_nv12(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGBA4444 & TO_NV21)
	case AIPL_COLOR_NV21:
		return aipl_color_convert_rgba4444_to_nv21(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGBA4444 & TO_I422)
	case AIPL_COLOR_I422:
		return aipl_color_convert_rgba4444_to_i422(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGBA4444 & TO_YUY2)
	case AIPL_COLOR_YUY2:
		return aipl_color_convert_rgba4444_to_yuy2(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGBA4444 & TO_UYVY)
	case AIPL_COLOR_UYVY:
		return aipl_color_convert_rgba4444_to_uyvy(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGBA4444 & TO_I444)
	case AIPL_COLOR_I444:
		return aipl_color_convert_rgba4444_to_i444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGBA4444 & TO_ALPHA8_I400)
	case AIPL_COLOR_I400:
		return aipl_color_convert_rgba4444_to_i400(input, output, pitch, width, height);
#endif

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

#if (AIPL_CONVERT_RGBA4444 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_rgba4444_to_alpha8(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba4444_to_alpha8_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba4444_to_alpha8_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGBA4444 & TO_ARGB8888)
aipl_error_t aipl_color_convert_rgba4444_to_argb8888(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
#ifdef AIPL_DAVE2D_ACCELERATION
	return aipl_color_convert_rgba4444_to_argb8888_dave2d(input, output, pitch, width, height);
#else
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba4444_to_argb8888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba4444_to_argb8888_default(input, output, pitch, width, height);
#endif
#endif
}
#endif

#if (AIPL_CONVERT_RGBA4444 & TO_ARGB1555)
aipl_error_t aipl_color_convert_rgba4444_to_argb1555(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	return aipl_color_convert_rgba4444_to_argb1555_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_RGBA4444 & TO_ARGB4444)
aipl_error_t aipl_color_convert_rgba4444_to_argb4444(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
#if (defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD))
	return aipl_color_convert_rgba4444_to_argb4444_dave2d(input, output, pitch, width, height);
#else
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba4444_to_argb4444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba4444_to_argb4444_default(input, output, pitch, width, height);
#endif
#endif
}
#endif

#if (AIPL_CONVERT_RGBA4444 & TO_RGBA8888)
aipl_error_t aipl_color_convert_rgba4444_to_rgba8888(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
#ifdef AIPL_DAVE2D_ACCELERATION
	return aipl_color_convert_rgba4444_to_rgba8888_dave2d(input, output, pitch, width, height);
#else
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba4444_to_rgba8888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba4444_to_rgba8888_default(input, output, pitch, width, height);
#endif
#endif
}
#endif

#if (AIPL_CONVERT_RGBA4444 & TO_RGBA5551)
aipl_error_t aipl_color_convert_rgba4444_to_rgba5551(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	return aipl_color_convert_rgba4444_to_rgba5551_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_RGBA4444 & TO_BGR888)
aipl_error_t aipl_color_convert_rgba4444_to_bgr888(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba4444_to_bgr888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba4444_to_bgr888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGBA4444 & TO_RGB888)
aipl_error_t aipl_color_convert_rgba4444_to_rgb888(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba4444_to_rgb888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba4444_to_rgb888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGBA4444 & TO_RGB565)
aipl_error_t aipl_color_convert_rgba4444_to_rgb565(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#if (defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD))
	return aipl_color_convert_rgba4444_to_rgb565_dave2d(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba4444_to_rgb565_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGBA4444 & TO_YV12)
aipl_error_t aipl_color_convert_rgba4444_to_yv12(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba4444_to_yv12_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba4444_to_yv12_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGBA4444 & TO_I420)
aipl_error_t aipl_color_convert_rgba4444_to_i420(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba4444_to_i420_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba4444_to_i420_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGBA4444 & TO_I422)
aipl_error_t aipl_color_convert_rgba4444_to_i422(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba4444_to_i422_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba4444_to_i422_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGBA4444 & TO_I444)
aipl_error_t aipl_color_convert_rgba4444_to_i444(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba4444_to_i444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba4444_to_i444_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGBA4444 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_rgba4444_to_i400(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba4444_to_i400_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba4444_to_i400_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGBA4444 & TO_NV21)
aipl_error_t aipl_color_convert_rgba4444_to_nv21(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba4444_to_nv21_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba4444_to_nv21_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGBA4444 & TO_NV12)
aipl_error_t aipl_color_convert_rgba4444_to_nv12(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba4444_to_nv12_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba4444_to_nv12_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGBA4444 & TO_YUY2)
aipl_error_t aipl_color_convert_rgba4444_to_yuy2(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba4444_to_yuy2_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba4444_to_yuy2_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGBA4444 & TO_UYVY)
aipl_error_t aipl_color_convert_rgba4444_to_uyvy(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba4444_to_uyvy_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba4444_to_uyvy_default(input, output, pitch, width, height);
#endif
}
#endif
#endif

#if AIPL_CONVERT_RGBA5551
aipl_error_t aipl_color_convert_rgba5551(const void *input, void *output, uint32_t pitch,
					 uint32_t width, uint32_t height,
					 aipl_color_format_t format)
{
#ifdef AIPL_DAVE2D_ACCELERATION
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	if (aipl_dave2d_color_convert_suitable(AIPL_COLOR_RGBA5551, format)) {
		d2_s32 ret = aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
							    d2_mode_rgba5551,
							    aipl_dave2d_format_to_mode(format));

		return aipl_dave2d_error_convert(ret);
	}
#endif

	switch (format) {
	/* Alpha color formats */
#if (AIPL_CONVERT_RGBA5551 & TO_ALPHA8_I400)
	case AIPL_COLOR_ALPHA8:
		return aipl_color_convert_rgba5551_to_alpha8(input, output, pitch, width, height);
#endif
	/* RGB color formats */
#if (AIPL_CONVERT_RGBA5551 & TO_ARGB8888)
	case AIPL_COLOR_ARGB8888:
		return aipl_color_convert_rgba5551_to_argb8888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGBA5551 & TO_RGBA8888)
	case AIPL_COLOR_RGBA8888:
		return aipl_color_convert_rgba5551_to_rgba8888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGBA5551 & TO_ARGB4444)
	case AIPL_COLOR_ARGB4444:
		return aipl_color_convert_rgba5551_to_argb4444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGBA5551 & TO_ARGB1555)
	case AIPL_COLOR_ARGB1555:
		return aipl_color_convert_rgba5551_to_argb1555(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGBA5551 & TO_RGBA4444)
	case AIPL_COLOR_RGBA4444:
		return aipl_color_convert_rgba5551_to_rgba4444(input, output, pitch, width, height);
#endif
	case AIPL_COLOR_RGBA5551:
		return AIPL_ERR_FORMAT_MISMATCH;
#if (AIPL_CONVERT_RGBA5551 & TO_RGB565)
	case AIPL_COLOR_RGB565:
		return aipl_color_convert_rgba5551_to_rgb565(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGBA5551 & TO_BGR888)
	case AIPL_COLOR_BGR888:
		return aipl_color_convert_rgba5551_to_bgr888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGBA5551 & TO_RGB888)
	case AIPL_COLOR_RGB888:
		return aipl_color_convert_rgba5551_to_rgb888(input, output, pitch, width, height);
#endif
	/* YUV color formats */
#if (AIPL_CONVERT_RGBA5551 & TO_YV12)
	case AIPL_COLOR_YV12:
		return aipl_color_convert_rgba5551_to_yv12(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGBA5551 & TO_I420)
	case AIPL_COLOR_I420:
		return aipl_color_convert_rgba5551_to_i420(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGBA5551 & TO_NV12)
	case AIPL_COLOR_NV12:
		return aipl_color_convert_rgba5551_to_nv12(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGBA5551 & TO_NV21)
	case AIPL_COLOR_NV21:
		return aipl_color_convert_rgba5551_to_nv21(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGBA5551 & TO_I422)
	case AIPL_COLOR_I422:
		return aipl_color_convert_rgba5551_to_i422(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGBA5551 & TO_YUY2)
	case AIPL_COLOR_YUY2:
		return aipl_color_convert_rgba5551_to_yuy2(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGBA5551 & TO_UYVY)
	case AIPL_COLOR_UYVY:
		return aipl_color_convert_rgba5551_to_uyvy(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGBA5551 & TO_I444)
	case AIPL_COLOR_I444:
		return aipl_color_convert_rgba5551_to_i444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGBA5551 & TO_ALPHA8_I400)
	case AIPL_COLOR_I400:
		return aipl_color_convert_rgba5551_to_i400(input, output, pitch, width, height);
#endif

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

#if (AIPL_CONVERT_RGBA5551 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_rgba5551_to_alpha8(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba5551_to_alpha8_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba5551_to_alpha8_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGBA5551 & TO_ARGB8888)
aipl_error_t aipl_color_convert_rgba5551_to_argb8888(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
#ifdef AIPL_DAVE2D_ACCELERATION
	return aipl_color_convert_rgba5551_to_argb8888_dave2d(input, output, pitch, width, height);
#else
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba5551_to_argb8888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba5551_to_argb8888_default(input, output, pitch, width, height);
#endif
#endif
}
#endif

#if (AIPL_CONVERT_RGBA5551 & TO_ARGB1555)
aipl_error_t aipl_color_convert_rgba5551_to_argb1555(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
	return aipl_color_convert_rgba5551_to_argb1555_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_RGBA5551 & TO_ARGB4444)
aipl_error_t aipl_color_convert_rgba5551_to_argb4444(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
#if (defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD))
	return aipl_color_convert_rgba5551_to_argb4444_dave2d(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba5551_to_argb4444_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGBA5551 & TO_RGBA8888)
aipl_error_t aipl_color_convert_rgba5551_to_rgba8888(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
#ifdef AIPL_DAVE2D_ACCELERATION
	return aipl_color_convert_rgba5551_to_rgba8888_dave2d(input, output, pitch, width, height);
#else
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba5551_to_rgba8888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba5551_to_rgba8888_default(input, output, pitch, width, height);
#endif
#endif
}
#endif

#if (AIPL_CONVERT_RGBA5551 & TO_RGBA4444)
aipl_error_t aipl_color_convert_rgba5551_to_rgba4444(const void *input, void *output,
						     uint32_t pitch, uint32_t width,
						     uint32_t height)
{
#if (defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD))
	return aipl_color_convert_rgba5551_to_rgba4444_dave2d(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba5551_to_rgba4444_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGBA5551 & TO_BGR888)
aipl_error_t aipl_color_convert_rgba5551_to_bgr888(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba5551_to_bgr888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba5551_to_bgr888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGBA5551 & TO_RGB888)
aipl_error_t aipl_color_convert_rgba5551_to_rgb888(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba5551_to_rgb888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba5551_to_rgb888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGBA5551 & TO_RGB565)
aipl_error_t aipl_color_convert_rgba5551_to_rgb565(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#if (defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD))
	return aipl_color_convert_rgba5551_to_rgb565_dave2d(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba5551_to_rgb565_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGBA5551 & TO_YV12)
aipl_error_t aipl_color_convert_rgba5551_to_yv12(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba5551_to_yv12_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba5551_to_yv12_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGBA5551 & TO_I420)
aipl_error_t aipl_color_convert_rgba5551_to_i420(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba5551_to_i420_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba5551_to_i420_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGBA5551 & TO_I422)
aipl_error_t aipl_color_convert_rgba5551_to_i422(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba5551_to_i422_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba5551_to_i422_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGBA5551 & TO_I444)
aipl_error_t aipl_color_convert_rgba5551_to_i444(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba5551_to_i444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba5551_to_i444_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGBA5551 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_rgba5551_to_i400(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba5551_to_i400_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba5551_to_i400_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGBA5551 & TO_NV21)
aipl_error_t aipl_color_convert_rgba5551_to_nv21(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba5551_to_nv21_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba5551_to_nv21_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGBA5551 & TO_NV12)
aipl_error_t aipl_color_convert_rgba5551_to_nv12(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba5551_to_nv12_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba5551_to_nv12_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGBA5551 & TO_YUY2)
aipl_error_t aipl_color_convert_rgba5551_to_yuy2(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba5551_to_yuy2_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba5551_to_yuy2_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGBA5551 & TO_UYVY)
aipl_error_t aipl_color_convert_rgba5551_to_uyvy(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgba5551_to_uyvy_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgba5551_to_uyvy_default(input, output, pitch, width, height);
#endif
}
#endif
#endif

#if AIPL_CONVERT_BGR888
aipl_error_t aipl_color_convert_bgr888(const void *input, void *output, uint32_t pitch,
				       uint32_t width, uint32_t height, aipl_color_format_t format)
{
	switch (format) {
	/* Alpha color formats */
#if (AIPL_CONVERT_BGR888 & TO_ALPHA8_I400)
	case AIPL_COLOR_ALPHA8:
		return aipl_color_convert_bgr888_to_alpha8(input, output, pitch, width, height);
#endif
	/* RGB color formats */
#if (AIPL_CONVERT_BGR888 & TO_ARGB8888)
	case AIPL_COLOR_ARGB8888:
		return aipl_color_convert_bgr888_to_argb8888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_BGR888 & TO_RGBA8888)
	case AIPL_COLOR_RGBA8888:
		return aipl_color_convert_bgr888_to_rgba8888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_BGR888 & TO_ARGB4444)
	case AIPL_COLOR_ARGB4444:
		return aipl_color_convert_bgr888_to_argb4444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_BGR888 & TO_ARGB1555)
	case AIPL_COLOR_ARGB1555:
		return aipl_color_convert_bgr888_to_argb1555(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_BGR888 & TO_RGBA4444)
	case AIPL_COLOR_RGBA4444:
		return aipl_color_convert_bgr888_to_rgba4444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_BGR888 & TO_RGBA5551)
	case AIPL_COLOR_RGBA5551:
		return aipl_color_convert_bgr888_to_rgba5551(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_BGR888 & TO_RGB565)
	case AIPL_COLOR_RGB565:
		return aipl_color_convert_bgr888_to_rgb565(input, output, pitch, width, height);
#endif
	case AIPL_COLOR_BGR888:
		return AIPL_ERR_FORMAT_MISMATCH;
#if (AIPL_CONVERT_BGR888 & TO_RGB888)
	case AIPL_COLOR_RGB888:
		return aipl_color_convert_bgr888_to_rgb888(input, output, pitch, width, height);
#endif
	/* YUV color formats */
#if (AIPL_CONVERT_BGR888 & TO_YV12)
	case AIPL_COLOR_YV12:
		return aipl_color_convert_bgr888_to_yv12(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_BGR888 & TO_I420)
	case AIPL_COLOR_I420:
		return aipl_color_convert_bgr888_to_i420(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_BGR888 & TO_NV12)
	case AIPL_COLOR_NV12:
		return aipl_color_convert_bgr888_to_nv12(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_BGR888 & TO_NV21)
	case AIPL_COLOR_NV21:
		return aipl_color_convert_bgr888_to_nv21(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_BGR888 & TO_I422)
	case AIPL_COLOR_I422:
		return aipl_color_convert_bgr888_to_i422(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_BGR888 & TO_YUY2)
	case AIPL_COLOR_YUY2:
		return aipl_color_convert_bgr888_to_yuy2(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_BGR888 & TO_UYVY)
	case AIPL_COLOR_UYVY:
		return aipl_color_convert_bgr888_to_uyvy(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_BGR888 & TO_I444)
	case AIPL_COLOR_I444:
		return aipl_color_convert_bgr888_to_i444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_BGR888 & TO_ALPHA8_I400)
	case AIPL_COLOR_I400:
		return aipl_color_convert_bgr888_to_i400(input, output, pitch, width, height);
#endif

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

#if (AIPL_CONVERT_BGR888 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_bgr888_to_alpha8(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_bgr888_to_alpha8_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_bgr888_to_alpha8_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_BGR888 & TO_ARGB8888)
aipl_error_t aipl_color_convert_bgr888_to_argb8888(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_bgr888_to_argb8888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_bgr888_to_argb8888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_BGR888 & TO_ARGB1555)
aipl_error_t aipl_color_convert_bgr888_to_argb1555(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_bgr888_to_argb1555_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_bgr888_to_argb1555_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_BGR888 & TO_ARGB4444)
aipl_error_t aipl_color_convert_bgr888_to_argb4444(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_bgr888_to_argb4444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_bgr888_to_argb4444_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_BGR888 & TO_RGBA8888)
aipl_error_t aipl_color_convert_bgr888_to_rgba8888(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_bgr888_to_rgba8888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_bgr888_to_rgba8888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_BGR888 & TO_RGBA4444)
aipl_error_t aipl_color_convert_bgr888_to_rgba4444(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_bgr888_to_rgba4444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_bgr888_to_rgba4444_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_BGR888 & TO_RGBA5551)
aipl_error_t aipl_color_convert_bgr888_to_rgba5551(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_bgr888_to_rgba5551_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_bgr888_to_rgba5551_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_BGR888 & TO_RGB565)
aipl_error_t aipl_color_convert_bgr888_to_rgb565(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_bgr888_to_rgb565_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_bgr888_to_rgb565_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_BGR888 & TO_RGB888)
aipl_error_t aipl_color_convert_bgr888_to_rgb888(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_bgr888_to_rgb888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_bgr888_to_rgb888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_BGR888 & TO_YV12)
aipl_error_t aipl_color_convert_bgr888_to_yv12(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_bgr888_to_yv12_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_bgr888_to_yv12_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_BGR888 & TO_I420)
aipl_error_t aipl_color_convert_bgr888_to_i420(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_bgr888_to_i420_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_bgr888_to_i420_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_BGR888 & TO_I422)
aipl_error_t aipl_color_convert_bgr888_to_i422(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_bgr888_to_i422_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_bgr888_to_i422_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_BGR888 & TO_I444)
aipl_error_t aipl_color_convert_bgr888_to_i444(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_bgr888_to_i444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_bgr888_to_i444_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_BGR888 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_bgr888_to_i400(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_bgr888_to_i400_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_bgr888_to_i400_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_BGR888 & TO_NV21)
aipl_error_t aipl_color_convert_bgr888_to_nv21(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_bgr888_to_nv21_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_bgr888_to_nv21_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_BGR888 & TO_NV12)
aipl_error_t aipl_color_convert_bgr888_to_nv12(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_bgr888_to_nv12_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_bgr888_to_nv12_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_BGR888 & TO_YUY2)
aipl_error_t aipl_color_convert_bgr888_to_yuy2(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_bgr888_to_yuy2_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_bgr888_to_yuy2_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_BGR888 & TO_UYVY)
aipl_error_t aipl_color_convert_bgr888_to_uyvy(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_bgr888_to_uyvy_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_bgr888_to_uyvy_default(input, output, pitch, width, height);
#endif
}
#endif
#endif

#if AIPL_CONVERT_RGB888
aipl_error_t aipl_color_convert_rgb888(const void *input, void *output, uint32_t pitch,
				       uint32_t width, uint32_t height, aipl_color_format_t format)
{
	switch (format) {
	/* Alpha color formats */
#if (AIPL_CONVERT_RGB888 & TO_ALPHA8_I400)
	case AIPL_COLOR_ALPHA8:
		return aipl_color_convert_rgb888_to_alpha8(input, output, pitch, width, height);
#endif
	/* RGB color formats */
#if (AIPL_CONVERT_RGB888 & TO_ARGB8888)
	case AIPL_COLOR_ARGB8888:
		return aipl_color_convert_rgb888_to_argb8888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGB888 & TO_RGBA8888)
	case AIPL_COLOR_RGBA8888:
		return aipl_color_convert_rgb888_to_rgba8888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGB888 & TO_ARGB4444)
	case AIPL_COLOR_ARGB4444:
		return aipl_color_convert_rgb888_to_argb4444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGB888 & TO_ARGB1555)
	case AIPL_COLOR_ARGB1555:
		return aipl_color_convert_rgb888_to_argb1555(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGB888 & TO_RGBA4444)
	case AIPL_COLOR_RGBA4444:
		return aipl_color_convert_rgb888_to_rgba4444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGB888 & TO_RGBA5551)
	case AIPL_COLOR_RGBA5551:
		return aipl_color_convert_rgb888_to_rgba5551(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGB888 & TO_RGB565)
	case AIPL_COLOR_RGB565:
		return aipl_color_convert_rgb888_to_rgb565(input, output, pitch, width, height);
#endif
	case AIPL_COLOR_RGB888:
		return AIPL_ERR_FORMAT_MISMATCH;
#if (AIPL_CONVERT_RGB888 & TO_BGR888)
	case AIPL_COLOR_BGR888:
		return aipl_color_convert_rgb888_to_bgr888(input, output, pitch, width, height);
#endif
	/* YUV color formats */
#if (AIPL_CONVERT_RGB888 & TO_YV12)
	case AIPL_COLOR_YV12:
		return aipl_color_convert_rgb888_to_yv12(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGB888 & TO_I420)
	case AIPL_COLOR_I420:
		return aipl_color_convert_rgb888_to_i420(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGB888 & TO_NV12)
	case AIPL_COLOR_NV12:
		return aipl_color_convert_rgb888_to_nv12(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGB888 & TO_NV21)
	case AIPL_COLOR_NV21:
		return aipl_color_convert_rgb888_to_nv21(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGB888 & TO_I422)
	case AIPL_COLOR_I422:
		return aipl_color_convert_rgb888_to_i422(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGB888 & TO_YUY2)
	case AIPL_COLOR_YUY2:
		return aipl_color_convert_rgb888_to_yuy2(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGB888 & TO_UYVY)
	case AIPL_COLOR_UYVY:
		return aipl_color_convert_rgb888_to_uyvy(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGB888 & TO_I444)
	case AIPL_COLOR_I444:
		return aipl_color_convert_rgb888_to_i444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGB888 & TO_ALPHA8_I400)
	case AIPL_COLOR_I400:
		return aipl_color_convert_rgb888_to_i400(input, output, pitch, width, height);
#endif

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

#if (AIPL_CONVERT_RGB888 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_rgb888_to_alpha8(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgb888_to_alpha8_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgb888_to_alpha8_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_ARGB8888)
aipl_error_t aipl_color_convert_rgb888_to_argb8888(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgb888_to_argb8888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgb888_to_argb8888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_ARGB1555)
aipl_error_t aipl_color_convert_rgb888_to_argb1555(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgb888_to_argb1555_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgb888_to_argb1555_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_ARGB4444)
aipl_error_t aipl_color_convert_rgb888_to_argb4444(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgb888_to_argb4444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgb888_to_argb4444_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_RGBA8888)
aipl_error_t aipl_color_convert_rgb888_to_rgba8888(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgb888_to_rgba8888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgb888_to_rgba8888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_RGBA4444)
aipl_error_t aipl_color_convert_rgb888_to_rgba4444(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgb888_to_rgba4444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgb888_to_rgba4444_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_RGBA5551)
aipl_error_t aipl_color_convert_rgb888_to_rgba5551(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgb888_to_rgba5551_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgb888_to_rgba5551_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_RGB565)
aipl_error_t aipl_color_convert_rgb888_to_rgb565(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgb888_to_rgb565_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgb888_to_rgb565_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_BGR888)
aipl_error_t aipl_color_convert_rgb888_to_bgr888(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgb888_to_bgr888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgb888_to_bgr888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_YV12)
aipl_error_t aipl_color_convert_rgb888_to_yv12(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgb888_to_yv12_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgb888_to_yv12_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_I420)
aipl_error_t aipl_color_convert_rgb888_to_i420(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgb888_to_i420_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgb888_to_i420_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_I422)
aipl_error_t aipl_color_convert_rgb888_to_i422(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgb888_to_i422_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgb888_to_i422_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_I444)
aipl_error_t aipl_color_convert_rgb888_to_i444(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgb888_to_i444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgb888_to_i444_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_rgb888_to_i400(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgb888_to_i400_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgb888_to_i400_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_NV21)
aipl_error_t aipl_color_convert_rgb888_to_nv21(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgb888_to_nv21_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgb888_to_nv21_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_NV12)
aipl_error_t aipl_color_convert_rgb888_to_nv12(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgb888_to_nv12_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgb888_to_nv12_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_YUY2)
aipl_error_t aipl_color_convert_rgb888_to_yuy2(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgb888_to_yuy2_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgb888_to_yuy2_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGB888 & TO_UYVY)
aipl_error_t aipl_color_convert_rgb888_to_uyvy(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgb888_to_uyvy_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgb888_to_uyvy_default(input, output, pitch, width, height);
#endif
}
#endif
#endif

#if AIPL_CONVERT_RGB565
aipl_error_t aipl_color_convert_rgb565(const void *input, void *output, uint32_t pitch,
				       uint32_t width, uint32_t height, aipl_color_format_t format)
{
#ifdef AIPL_DAVE2D_ACCELERATION
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	if (aipl_dave2d_color_convert_suitable(AIPL_COLOR_RGB565, format)) {
		d2_s32 ret = aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
							    d2_mode_rgb565,
							    aipl_dave2d_format_to_mode(format));

		return aipl_dave2d_error_convert(ret);
	}
#endif

	switch (format) {
	/* Alpha color formats */
#if (AIPL_CONVERT_RGB565 & TO_ALPHA8_I400)
	case AIPL_COLOR_ALPHA8:
		return aipl_color_convert_rgb565_to_alpha8(input, output, pitch, width, height);
#endif
	/* RGB color formats */
#if (AIPL_CONVERT_RGB565 & TO_ARGB8888)
	case AIPL_COLOR_ARGB8888:
		return aipl_color_convert_rgb565_to_argb8888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGB565 & TO_RGBA8888)
	case AIPL_COLOR_RGBA8888:
		return aipl_color_convert_rgb565_to_rgba8888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGB565 & TO_ARGB4444)
	case AIPL_COLOR_ARGB4444:
		return aipl_color_convert_rgb565_to_argb4444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGB565 & TO_ARGB1555)
	case AIPL_COLOR_ARGB1555:
		return aipl_color_convert_rgb565_to_argb1555(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGB565 & TO_RGBA4444)
	case AIPL_COLOR_RGBA4444:
		return aipl_color_convert_rgb565_to_rgba4444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGB565 & TO_RGBA5551)
	case AIPL_COLOR_RGBA5551:
		return aipl_color_convert_rgb565_to_rgba5551(input, output, pitch, width, height);
#endif
	case AIPL_COLOR_RGB565:
		return AIPL_ERR_FORMAT_MISMATCH;
#if (AIPL_CONVERT_RGB565 & TO_BGR888)
	case AIPL_COLOR_BGR888:
		return aipl_color_convert_rgb565_to_bgr888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGB565 & TO_RGB888)
	case AIPL_COLOR_RGB888:
		return aipl_color_convert_rgb565_to_rgb888(input, output, pitch, width, height);
#endif
	/* YUV color formats */
#if (AIPL_CONVERT_RGB565 & TO_YV12)
	case AIPL_COLOR_YV12:
		return aipl_color_convert_rgb565_to_yv12(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGB565 & TO_I420)
	case AIPL_COLOR_I420:
		return aipl_color_convert_rgb565_to_i420(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGB565 & TO_NV12)
	case AIPL_COLOR_NV12:
		return aipl_color_convert_rgb565_to_nv12(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGB565 & TO_NV21)
	case AIPL_COLOR_NV21:
		return aipl_color_convert_rgb565_to_nv21(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGB565 & TO_I422)
	case AIPL_COLOR_I422:
		return aipl_color_convert_rgb565_to_i422(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGB565 & TO_YUY2)
	case AIPL_COLOR_YUY2:
		return aipl_color_convert_rgb565_to_yuy2(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGB565 & TO_UYVY)
	case AIPL_COLOR_UYVY:
		return aipl_color_convert_rgb565_to_uyvy(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGB565 & TO_I444)
	case AIPL_COLOR_I444:
		return aipl_color_convert_rgb565_to_i444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_RGB565 & TO_ALPHA8_I400)
	case AIPL_COLOR_I400:
		return aipl_color_convert_rgb565_to_i400(input, output, pitch, width, height);
#endif

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

#if (AIPL_CONVERT_RGB565 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_rgb565_to_alpha8(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgb565_to_alpha8_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgb565_to_alpha8_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGB565 & TO_ARGB8888)
aipl_error_t aipl_color_convert_rgb565_to_argb8888(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#ifdef AIPL_DAVE2D_ACCELERATION
	return aipl_color_convert_rgb565_to_argb8888_dave2d(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgb565_to_argb8888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGB565 & TO_ARGB1555)
aipl_error_t aipl_color_convert_rgb565_to_argb1555(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
	return aipl_color_convert_rgb565_to_argb1555_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_RGB565 & TO_ARGB4444)
aipl_error_t aipl_color_convert_rgb565_to_argb4444(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#if (defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD))
	return aipl_color_convert_rgb565_to_argb4444_dave2d(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgb565_to_argb4444_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGB565 & TO_RGBA8888)
aipl_error_t aipl_color_convert_rgb565_to_rgba8888(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#ifdef AIPL_DAVE2D_ACCELERATION
	return aipl_color_convert_rgb565_to_rgba8888_dave2d(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgb565_to_rgba8888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGB565 & TO_RGBA4444)
aipl_error_t aipl_color_convert_rgb565_to_rgba4444(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
#if (defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD))
	return aipl_color_convert_rgb565_to_rgba4444_dave2d(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgb565_to_rgba4444_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGB565 & TO_RGBA5551)
aipl_error_t aipl_color_convert_rgb565_to_rgba5551(const void *input, void *output, uint32_t pitch,
						   uint32_t width, uint32_t height)
{
	return aipl_color_convert_rgb565_to_rgba5551_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_RGB565 & TO_BGR888)
aipl_error_t aipl_color_convert_rgb565_to_bgr888(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgb565_to_bgr888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgb565_to_bgr888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGB565 & TO_RGB888)
aipl_error_t aipl_color_convert_rgb565_to_rgb888(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgb565_to_rgb888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgb565_to_rgb888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGB565 & TO_YV12)
aipl_error_t aipl_color_convert_rgb565_to_yv12(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgb565_to_yv12_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgb565_to_yv12_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGB565 & TO_I420)
aipl_error_t aipl_color_convert_rgb565_to_i420(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgb565_to_i420_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgb565_to_i420_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGB565 & TO_I422)
aipl_error_t aipl_color_convert_rgb565_to_i422(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgb565_to_i422_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgb565_to_i422_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGB565 & TO_I444)
aipl_error_t aipl_color_convert_rgb565_to_i444(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgb565_to_i444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgb565_to_i444_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGB565 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_rgb565_to_i400(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgb565_to_i400_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgb565_to_i400_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGB565 & TO_NV21)
aipl_error_t aipl_color_convert_rgb565_to_nv21(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgb565_to_nv21_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgb565_to_nv21_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGB565 & TO_NV12)
aipl_error_t aipl_color_convert_rgb565_to_nv12(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgb565_to_nv12_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgb565_to_nv12_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGB565 & TO_YUY2)
aipl_error_t aipl_color_convert_rgb565_to_yuy2(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgb565_to_yuy2_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgb565_to_yuy2_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_RGB565 & TO_UYVY)
aipl_error_t aipl_color_convert_rgb565_to_uyvy(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_rgb565_to_uyvy_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_rgb565_to_uyvy_default(input, output, pitch, width, height);
#endif
}
#endif
#endif

#if AIPL_CONVERT_YV12
aipl_error_t aipl_color_convert_yv12(const void *input, void *output, uint32_t pitch,
				     uint32_t width, uint32_t height, aipl_color_format_t format)
{
	switch (format) {
	/* Alpha color formats */
#if (AIPL_CONVERT_YV12 & TO_ALPHA8_I400)
	case AIPL_COLOR_ALPHA8:
		return aipl_color_convert_yv12_to_alpha8(input, output, pitch, width, height);
#endif
	/* RGB color formats */
#if (AIPL_CONVERT_YV12 & TO_ARGB8888)
	case AIPL_COLOR_ARGB8888:
		return aipl_color_convert_yv12_to_argb8888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_YV12 & TO_RGBA8888)
	case AIPL_COLOR_RGBA8888:
		return aipl_color_convert_yv12_to_rgba8888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_YV12 & TO_ARGB4444)
	case AIPL_COLOR_ARGB4444:
		return aipl_color_convert_yv12_to_argb4444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_YV12 & TO_ARGB1555)
	case AIPL_COLOR_ARGB1555:
		return aipl_color_convert_yv12_to_argb1555(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_YV12 & TO_RGBA4444)
	case AIPL_COLOR_RGBA4444:
		return aipl_color_convert_yv12_to_rgba4444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_YV12 & TO_RGBA5551)
	case AIPL_COLOR_RGBA5551:
		return aipl_color_convert_yv12_to_rgba5551(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_YV12 & TO_RGB565)
	case AIPL_COLOR_RGB565:
		return aipl_color_convert_yv12_to_rgb565(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_YV12 & TO_BGR888)
	case AIPL_COLOR_BGR888:
		return aipl_color_convert_yv12_to_bgr888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_YV12 & TO_RGB888)
	case AIPL_COLOR_RGB888:
		return aipl_color_convert_yv12_to_rgb888(input, output, pitch, width, height);
#endif
	/* YUV color formats */
	case AIPL_COLOR_YV12:
		return AIPL_ERR_FORMAT_MISMATCH;
#if (AIPL_CONVERT_YV12 & TO_I420)
	case AIPL_COLOR_I420:
		return aipl_color_convert_yv12_to_i420(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_YV12 & TO_NV12)
	case AIPL_COLOR_NV12:
		return aipl_color_convert_yv12_to_nv12(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_YV12 & TO_NV21)
	case AIPL_COLOR_NV21:
		return aipl_color_convert_yv12_to_nv21(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_YV12 & TO_I422)
	case AIPL_COLOR_I422:
		return aipl_color_convert_yv12_to_i422(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_YV12 & TO_YUY2)
	case AIPL_COLOR_YUY2:
		return aipl_color_convert_yv12_to_yuy2(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_YV12 & TO_UYVY)
	case AIPL_COLOR_UYVY:
		return aipl_color_convert_yv12_to_uyvy(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_YV12 & TO_I444)
	case AIPL_COLOR_I444:
		return aipl_color_convert_yv12_to_i444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_YV12 & TO_ALPHA8_I400)
	case AIPL_COLOR_I400:
		return aipl_color_convert_yv12_to_i400(input, output, pitch, width, height);
#endif

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

#if (AIPL_CONVERT_YV12 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_yv12_to_alpha8(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
	return aipl_color_convert_yv12_to_alpha8_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_YV12 & TO_ARGB8888)
aipl_error_t aipl_color_convert_yv12_to_argb8888(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_yv12_to_argb8888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_yv12_to_argb8888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_YV12 & TO_ARGB4444)
aipl_error_t aipl_color_convert_yv12_to_argb4444(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_yv12_to_argb4444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_yv12_to_argb4444_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_YV12 & TO_ARGB1555)
aipl_error_t aipl_color_convert_yv12_to_argb1555(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_yv12_to_argb1555_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_yv12_to_argb1555_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_YV12 & TO_RGBA8888)
aipl_error_t aipl_color_convert_yv12_to_rgba8888(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_yv12_to_rgba8888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_yv12_to_rgba8888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_YV12 & TO_RGBA4444)
aipl_error_t aipl_color_convert_yv12_to_rgba4444(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_yv12_to_rgba4444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_yv12_to_rgba4444_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_YV12 & TO_RGBA5551)
aipl_error_t aipl_color_convert_yv12_to_rgba5551(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_yv12_to_rgba5551_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_yv12_to_rgba5551_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_YV12 & TO_BGR888)
aipl_error_t aipl_color_convert_yv12_to_bgr888(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_yv12_to_bgr888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_yv12_to_bgr888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_YV12 & TO_RGB888)
aipl_error_t aipl_color_convert_yv12_to_rgb888(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_yv12_to_rgb888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_yv12_to_rgb888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_YV12 & TO_RGB565)
aipl_error_t aipl_color_convert_yv12_to_rgb565(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_yv12_to_rgb565_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_yv12_to_rgb565_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_YV12 & TO_I420)
aipl_error_t aipl_color_convert_yv12_to_i420(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
	return aipl_color_convert_yv12_to_i420_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_YV12 & TO_I422)
aipl_error_t aipl_color_convert_yv12_to_i422(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
	return aipl_color_convert_yv12_to_i422_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_YV12 & TO_I444)
aipl_error_t aipl_color_convert_yv12_to_i444(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_yv12_to_i444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_yv12_to_i444_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_YV12 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_yv12_to_i400(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
	return aipl_color_convert_yv12_to_i400_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_YV12 & TO_NV21)
aipl_error_t aipl_color_convert_yv12_to_nv21(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_yv12_to_nv21_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_yv12_to_nv21_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_YV12 & TO_NV12)
aipl_error_t aipl_color_convert_yv12_to_nv12(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_yv12_to_nv12_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_yv12_to_nv12_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_YV12 & TO_YUY2)
aipl_error_t aipl_color_convert_yv12_to_yuy2(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
	return aipl_color_convert_yv12_to_yuy2_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_YV12 & TO_UYVY)
aipl_error_t aipl_color_convert_yv12_to_uyvy(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
	return aipl_color_convert_yv12_to_uyvy_default(input, output, pitch, width, height);
}
#endif
#endif

#if AIPL_CONVERT_I420
aipl_error_t aipl_color_convert_i420(const void *input, void *output, uint32_t pitch,
				     uint32_t width, uint32_t height, aipl_color_format_t format)
{
	switch (format) {
	/* Alpha color formats */
#if (AIPL_CONVERT_I420 & TO_ALPHA8_I400)
	case AIPL_COLOR_ALPHA8:
		return aipl_color_convert_i420_to_alpha8(input, output, pitch, width, height);
#endif
	/* RGB color formats */
#if (AIPL_CONVERT_I420 & TO_ARGB8888)
	case AIPL_COLOR_ARGB8888:
		return aipl_color_convert_i420_to_argb8888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I420 & TO_RGBA8888)
	case AIPL_COLOR_RGBA8888:
		return aipl_color_convert_i420_to_rgba8888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I420 & TO_ARGB4444)
	case AIPL_COLOR_ARGB4444:
		return aipl_color_convert_i420_to_argb4444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I420 & TO_ARGB1555)
	case AIPL_COLOR_ARGB1555:
		return aipl_color_convert_i420_to_argb1555(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I420 & TO_RGBA4444)
	case AIPL_COLOR_RGBA4444:
		return aipl_color_convert_i420_to_rgba4444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I420 & TO_RGBA5551)
	case AIPL_COLOR_RGBA5551:
		return aipl_color_convert_i420_to_rgba5551(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I420 & TO_RGB565)
	case AIPL_COLOR_RGB565:
		return aipl_color_convert_i420_to_rgb565(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I420 & TO_BGR888)
	case AIPL_COLOR_BGR888:
		return aipl_color_convert_i420_to_bgr888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I420 & TO_RGB888)
	case AIPL_COLOR_RGB888:
		return aipl_color_convert_i420_to_rgb888(input, output, pitch, width, height);
#endif
	/* YUV color formats */
#if (AIPL_CONVERT_I420 & TO_YV12)
	case AIPL_COLOR_YV12:
		return aipl_color_convert_i420_to_yv12(input, output, pitch, width, height);
#endif
	case AIPL_COLOR_I420:
		return AIPL_ERR_FORMAT_MISMATCH;
#if (AIPL_CONVERT_I420 & TO_NV12)
	case AIPL_COLOR_NV12:
		return aipl_color_convert_i420_to_nv12(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I420 & TO_NV21)
	case AIPL_COLOR_NV21:
		return aipl_color_convert_i420_to_nv21(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I420 & TO_I422)
	case AIPL_COLOR_I422:
		return aipl_color_convert_i420_to_i422(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I420 & TO_YUY2)
	case AIPL_COLOR_YUY2:
		return aipl_color_convert_i420_to_yuy2(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I420 & TO_UYVY)
	case AIPL_COLOR_UYVY:
		return aipl_color_convert_i420_to_uyvy(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I420 & TO_I444)
	case AIPL_COLOR_I444:
		return aipl_color_convert_i420_to_i444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I420 & TO_ALPHA8_I400)
	case AIPL_COLOR_I400:
		return aipl_color_convert_i420_to_i400(input, output, pitch, width, height);
#endif

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

#if (AIPL_CONVERT_I420 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_i420_to_alpha8(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
	return aipl_color_convert_i420_to_alpha8_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_I420 & TO_ARGB8888)
aipl_error_t aipl_color_convert_i420_to_argb8888(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i420_to_argb8888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i420_to_argb8888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_I420 & TO_ARGB4444)
aipl_error_t aipl_color_convert_i420_to_argb4444(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i420_to_argb4444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i420_to_argb4444_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_I420 & TO_ARGB1555)
aipl_error_t aipl_color_convert_i420_to_argb1555(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i420_to_argb1555_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i420_to_argb1555_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_I420 & TO_RGBA8888)
aipl_error_t aipl_color_convert_i420_to_rgba8888(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i420_to_rgba8888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i420_to_rgba8888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_I420 & TO_RGBA4444)
aipl_error_t aipl_color_convert_i420_to_rgba4444(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i420_to_rgba4444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i420_to_rgba4444_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_I420 & TO_RGBA5551)
aipl_error_t aipl_color_convert_i420_to_rgba5551(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i420_to_rgba5551_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i420_to_rgba5551_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_I420 & TO_BGR888)
aipl_error_t aipl_color_convert_i420_to_bgr888(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i420_to_bgr888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i420_to_bgr888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_I420 & TO_RGB888)
aipl_error_t aipl_color_convert_i420_to_rgb888(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i420_to_rgb888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i420_to_rgb888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_I420 & TO_RGB565)
aipl_error_t aipl_color_convert_i420_to_rgb565(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i420_to_rgb565_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i420_to_rgb565_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_I420 & TO_YV12)
aipl_error_t aipl_color_convert_i420_to_yv12(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
	return aipl_color_convert_i420_to_yv12_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_I420 & TO_I422)
aipl_error_t aipl_color_convert_i420_to_i422(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
	return aipl_color_convert_i420_to_i422_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_I420 & TO_I444)
aipl_error_t aipl_color_convert_i420_to_i444(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i420_to_i444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i420_to_i444_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_I420 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_i420_to_i400(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
	return aipl_color_convert_i420_to_i400_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_I420 & TO_NV21)
aipl_error_t aipl_color_convert_i420_to_nv21(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i420_to_nv21_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i420_to_nv21_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_I420 & TO_NV12)
aipl_error_t aipl_color_convert_i420_to_nv12(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i420_to_nv12_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i420_to_nv12_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_I420 & TO_YUY2)
aipl_error_t aipl_color_convert_i420_to_yuy2(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
	return aipl_color_convert_i420_to_yuy2_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_I420 & TO_UYVY)
aipl_error_t aipl_color_convert_i420_to_uyvy(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
	return aipl_color_convert_i420_to_uyvy_default(input, output, pitch, width, height);
}
#endif
#endif

#if AIPL_CONVERT_I422
aipl_error_t aipl_color_convert_i422(const void *input, void *output, uint32_t pitch,
				     uint32_t width, uint32_t height, aipl_color_format_t format)
{
	switch (format) {
	/* Alpha color formats */
#if (AIPL_CONVERT_I422 & TO_ALPHA8_I400)
	case AIPL_COLOR_ALPHA8:
		return aipl_color_convert_i422_to_alpha8(input, output, pitch, width, height);
#endif
	/* RGB color formats */
#if (AIPL_CONVERT_I422 & TO_ARGB8888)
	case AIPL_COLOR_ARGB8888:
		return aipl_color_convert_i422_to_argb8888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I422 & TO_RGBA8888)
	case AIPL_COLOR_RGBA8888:
		return aipl_color_convert_i422_to_rgba8888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I422 & TO_ARGB4444)
	case AIPL_COLOR_ARGB4444:
		return aipl_color_convert_i422_to_argb4444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I422 & TO_ARGB1555)
	case AIPL_COLOR_ARGB1555:
		return aipl_color_convert_i422_to_argb1555(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I422 & TO_RGBA4444)
	case AIPL_COLOR_RGBA4444:
		return aipl_color_convert_i422_to_rgba4444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I422 & TO_RGBA5551)
	case AIPL_COLOR_RGBA5551:
		return aipl_color_convert_i422_to_rgba5551(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I422 & TO_RGB565)
	case AIPL_COLOR_RGB565:
		return aipl_color_convert_i422_to_rgb565(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I422 & TO_BGR888)
	case AIPL_COLOR_BGR888:
		return aipl_color_convert_i422_to_bgr888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I422 & TO_RGB888)
	case AIPL_COLOR_RGB888:
		return aipl_color_convert_i422_to_rgb888(input, output, pitch, width, height);
#endif
	/* YUV color formats */
#if (AIPL_CONVERT_I422 & TO_YV12)
	case AIPL_COLOR_YV12:
		return aipl_color_convert_i422_to_yv12(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I422 & TO_I420)
	case AIPL_COLOR_I420:
		return aipl_color_convert_i422_to_i420(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I422 & TO_NV12)
	case AIPL_COLOR_NV12:
		return aipl_color_convert_i422_to_nv12(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I422 & TO_NV21)
	case AIPL_COLOR_NV21:
		return aipl_color_convert_i422_to_nv21(input, output, pitch, width, height);
#endif
	case AIPL_COLOR_I422:
		return AIPL_ERR_FORMAT_MISMATCH;
#if (AIPL_CONVERT_I422 & TO_YUY2)
	case AIPL_COLOR_YUY2:
		return aipl_color_convert_i422_to_yuy2(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I422 & TO_UYVY)
	case AIPL_COLOR_UYVY:
		return aipl_color_convert_i422_to_uyvy(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I422 & TO_I444)
	case AIPL_COLOR_I444:
		return aipl_color_convert_i422_to_i444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I422 & TO_ALPHA8_I400)
	case AIPL_COLOR_I400:
		return aipl_color_convert_i422_to_i400(input, output, pitch, width, height);
#endif

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

#if (AIPL_CONVERT_I422 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_i422_to_alpha8(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
	return aipl_color_convert_i422_to_alpha8_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_I422 & TO_ARGB8888)
aipl_error_t aipl_color_convert_i422_to_argb8888(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i422_to_argb8888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i422_to_argb8888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_I422 & TO_ARGB4444)
aipl_error_t aipl_color_convert_i422_to_argb4444(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i422_to_argb4444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i422_to_argb4444_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_I422 & TO_ARGB1555)
aipl_error_t aipl_color_convert_i422_to_argb1555(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i422_to_argb1555_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i422_to_argb1555_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_I422 & TO_RGBA8888)
aipl_error_t aipl_color_convert_i422_to_rgba8888(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i422_to_rgba8888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i422_to_rgba8888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_I422 & TO_RGBA4444)
aipl_error_t aipl_color_convert_i422_to_rgba4444(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i422_to_rgba4444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i422_to_rgba4444_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_I422 & TO_RGBA5551)
aipl_error_t aipl_color_convert_i422_to_rgba5551(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i422_to_rgba5551_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i422_to_rgba5551_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_I422 & TO_BGR888)
aipl_error_t aipl_color_convert_i422_to_bgr888(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i422_to_bgr888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i422_to_bgr888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_I422 & TO_RGB888)
aipl_error_t aipl_color_convert_i422_to_rgb888(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i422_to_rgb888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i422_to_rgb888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_I422 & TO_RGB565)
aipl_error_t aipl_color_convert_i422_to_rgb565(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i422_to_rgb565_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i422_to_rgb565_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_I422 & TO_YV12)
aipl_error_t aipl_color_convert_i422_to_yv12(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i422_to_yv12_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i422_to_yv12_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_I422 & TO_I420)
aipl_error_t aipl_color_convert_i422_to_i420(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i422_to_i420_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i422_to_i420_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_I422 & TO_I444)
aipl_error_t aipl_color_convert_i422_to_i444(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i422_to_i444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i422_to_i444_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_I422 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_i422_to_i400(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
	return aipl_color_convert_i422_to_i400_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_I422 & TO_NV21)
aipl_error_t aipl_color_convert_i422_to_nv21(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i422_to_nv21_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i422_to_nv21_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_I422 & TO_NV12)
aipl_error_t aipl_color_convert_i422_to_nv12(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i422_to_nv12_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i422_to_nv12_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_I422 & TO_YUY2)
aipl_error_t aipl_color_convert_i422_to_yuy2(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
	return aipl_color_convert_i422_to_yuy2_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_I422 & TO_UYVY)
aipl_error_t aipl_color_convert_i422_to_uyvy(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
	return aipl_color_convert_i422_to_uyvy_default(input, output, pitch, width, height);
}
#endif
#endif

#if AIPL_CONVERT_I444
aipl_error_t aipl_color_convert_i444(const void *input, void *output, uint32_t pitch,
				     uint32_t width, uint32_t height, aipl_color_format_t format)
{
	switch (format) {
	/* Alpha color formats */
#if (AIPL_CONVERT_I444 & TO_ALPHA8_I400)
	case AIPL_COLOR_ALPHA8:
		return aipl_color_convert_i444_to_alpha8(input, output, pitch, width, height);
#endif
	/* RGB color formats */
#if (AIPL_CONVERT_I444 & TO_ARGB8888)
	case AIPL_COLOR_ARGB8888:
		return aipl_color_convert_i444_to_argb8888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I444 & TO_RGBA8888)
	case AIPL_COLOR_RGBA8888:
		return aipl_color_convert_i444_to_rgba8888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I444 & TO_ARGB4444)
	case AIPL_COLOR_ARGB4444:
		return aipl_color_convert_i444_to_argb4444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I444 & TO_ARGB1555)
	case AIPL_COLOR_ARGB1555:
		return aipl_color_convert_i444_to_argb1555(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I444 & TO_RGBA4444)
	case AIPL_COLOR_RGBA4444:
		return aipl_color_convert_i444_to_rgba4444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I444 & TO_RGBA5551)
	case AIPL_COLOR_RGBA5551:
		return aipl_color_convert_i444_to_rgba5551(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I444 & TO_RGB565)
	case AIPL_COLOR_RGB565:
		return aipl_color_convert_i444_to_rgb565(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I444 & TO_BGR888)
	case AIPL_COLOR_BGR888:
		return aipl_color_convert_i444_to_bgr888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I444 & TO_RGB888)
	case AIPL_COLOR_RGB888:
		return aipl_color_convert_i444_to_rgb888(input, output, pitch, width, height);
#endif
	/* YUV color formats */
#if (AIPL_CONVERT_I444 & TO_YV12)
	case AIPL_COLOR_YV12:
		return aipl_color_convert_i444_to_yv12(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I444 & TO_I420)
	case AIPL_COLOR_I420:
		return aipl_color_convert_i444_to_i420(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I444 & TO_NV12)
	case AIPL_COLOR_NV12:
		return aipl_color_convert_i444_to_nv12(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I444 & TO_NV21)
	case AIPL_COLOR_NV21:
		return aipl_color_convert_i444_to_nv21(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I444 & TO_I422)
	case AIPL_COLOR_I422:
		return aipl_color_convert_i444_to_i422(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I444 & TO_YUY2)
	case AIPL_COLOR_YUY2:
		return aipl_color_convert_i444_to_yuy2(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_I444 & TO_UYVY)
	case AIPL_COLOR_UYVY:
		return aipl_color_convert_i444_to_uyvy(input, output, pitch, width, height);
#endif
	case AIPL_COLOR_I444:
		return AIPL_ERR_FORMAT_MISMATCH;
#if (AIPL_CONVERT_I444 & TO_ALPHA8_I400)
	case AIPL_COLOR_I400:
		return aipl_color_convert_i444_to_i400(input, output, pitch, width, height);
#endif

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

#if (AIPL_CONVERT_I444 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_i444_to_alpha8(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
	return aipl_color_convert_i444_to_alpha8_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_I444 & TO_ARGB8888)
aipl_error_t aipl_color_convert_i444_to_argb8888(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i444_to_argb8888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i444_to_argb8888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_I444 & TO_ARGB4444)
aipl_error_t aipl_color_convert_i444_to_argb4444(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i444_to_argb4444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i444_to_argb4444_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_I444 & TO_ARGB1555)
aipl_error_t aipl_color_convert_i444_to_argb1555(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i444_to_argb1555_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i444_to_argb1555_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_I444 & TO_RGBA8888)
aipl_error_t aipl_color_convert_i444_to_rgba8888(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i444_to_rgba8888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i444_to_rgba8888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_I444 & TO_RGBA4444)
aipl_error_t aipl_color_convert_i444_to_rgba4444(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i444_to_rgba4444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i444_to_rgba4444_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_I444 & TO_RGBA5551)
aipl_error_t aipl_color_convert_i444_to_rgba5551(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i444_to_rgba5551_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i444_to_rgba5551_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_I444 & TO_BGR888)
aipl_error_t aipl_color_convert_i444_to_bgr888(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i444_to_bgr888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i444_to_bgr888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_I444 & TO_RGB888)
aipl_error_t aipl_color_convert_i444_to_rgb888(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i444_to_rgb888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i444_to_rgb888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_I444 & TO_RGB565)
aipl_error_t aipl_color_convert_i444_to_rgb565(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i444_to_rgb565_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i444_to_rgb565_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_I444 & TO_YV12)
aipl_error_t aipl_color_convert_i444_to_yv12(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i444_to_yv12_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i444_to_yv12_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_I444 & TO_I420)
aipl_error_t aipl_color_convert_i444_to_i420(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i444_to_i420_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i444_to_i420_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_I444 & TO_I422)
aipl_error_t aipl_color_convert_i444_to_i422(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i444_to_i422_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i444_to_i422_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_I444 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_i444_to_i400(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
	return aipl_color_convert_i444_to_i400_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_I444 & TO_NV21)
aipl_error_t aipl_color_convert_i444_to_nv21(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i444_to_nv21_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i444_to_nv21_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_I444 & TO_NV12)
aipl_error_t aipl_color_convert_i444_to_nv12(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i444_to_nv12_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i444_to_nv12_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_I444 & TO_YUY2)
aipl_error_t aipl_color_convert_i444_to_yuy2(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
	return aipl_color_convert_i444_to_yuy2_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_I444 & TO_UYVY)
aipl_error_t aipl_color_convert_i444_to_uyvy(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
	return aipl_color_convert_i444_to_uyvy_default(input, output, pitch, width, height);
}
#endif
#endif

#if AIPL_CONVERT_NV12
aipl_error_t aipl_color_convert_nv12(const void *input, void *output, uint32_t pitch,
				     uint32_t width, uint32_t height, aipl_color_format_t format)
{
	switch (format) {
	/* Alpha color formats */
#if (AIPL_CONVERT_NV12 & TO_ALPHA8_I400)
	case AIPL_COLOR_ALPHA8:
		return aipl_color_convert_nv12_to_alpha8(input, output, pitch, width, height);
#endif
	/* RGB color formats */
#if (AIPL_CONVERT_NV12 & TO_ARGB8888)
	case AIPL_COLOR_ARGB8888:
		return aipl_color_convert_nv12_to_argb8888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_NV12 & TO_RGBA8888)
	case AIPL_COLOR_RGBA8888:
		return aipl_color_convert_nv12_to_rgba8888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_NV12 & TO_ARGB4444)
	case AIPL_COLOR_ARGB4444:
		return aipl_color_convert_nv12_to_argb4444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_NV12 & TO_ARGB1555)
	case AIPL_COLOR_ARGB1555:
		return aipl_color_convert_nv12_to_argb1555(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_NV12 & TO_RGBA4444)
	case AIPL_COLOR_RGBA4444:
		return aipl_color_convert_nv12_to_rgba4444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_NV12 & TO_RGBA5551)
	case AIPL_COLOR_RGBA5551:
		return aipl_color_convert_nv12_to_rgba5551(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_NV12 & TO_RGB565)
	case AIPL_COLOR_RGB565:
		return aipl_color_convert_nv12_to_rgb565(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_NV12 & TO_BGR888)
	case AIPL_COLOR_BGR888:
		return aipl_color_convert_nv12_to_bgr888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_NV12 & TO_RGB888)
	case AIPL_COLOR_RGB888:
		return aipl_color_convert_nv12_to_rgb888(input, output, pitch, width, height);
#endif
	/* YUV color formats */
#if (AIPL_CONVERT_NV12 & TO_YV12)
	case AIPL_COLOR_YV12:
		return aipl_color_convert_nv12_to_yv12(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_NV12 & TO_I420)
	case AIPL_COLOR_I420:
		return aipl_color_convert_nv12_to_i420(input, output, pitch, width, height);
#endif
	case AIPL_COLOR_NV12:
		return AIPL_ERR_FORMAT_MISMATCH;
#if (AIPL_CONVERT_NV12 & TO_NV21)
	case AIPL_COLOR_NV21:
		return aipl_color_convert_nv12_to_nv21(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_NV12 & TO_I422)
	case AIPL_COLOR_I422:
		return aipl_color_convert_nv12_to_i422(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_NV12 & TO_YUY2)
	case AIPL_COLOR_YUY2:
		return aipl_color_convert_nv12_to_yuy2(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_NV12 & TO_UYVY)
	case AIPL_COLOR_UYVY:
		return aipl_color_convert_nv12_to_uyvy(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_NV12 & TO_I444)
	case AIPL_COLOR_I444:
		return aipl_color_convert_nv12_to_i444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_NV12 & TO_ALPHA8_I400)
	case AIPL_COLOR_I400:
		return aipl_color_convert_nv12_to_i400(input, output, pitch, width, height);
#endif

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

#if (AIPL_CONVERT_NV12 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_nv12_to_alpha8(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
	return aipl_color_convert_nv12_to_alpha8_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_NV12 & TO_ARGB8888)
aipl_error_t aipl_color_convert_nv12_to_argb8888(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_nv12_to_argb8888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_nv12_to_argb8888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_NV12 & TO_ARGB4444)
aipl_error_t aipl_color_convert_nv12_to_argb4444(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_nv12_to_argb4444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_nv12_to_argb4444_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_NV12 & TO_ARGB1555)
aipl_error_t aipl_color_convert_nv12_to_argb1555(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_nv12_to_argb1555_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_nv12_to_argb1555_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_NV12 & TO_RGBA8888)
aipl_error_t aipl_color_convert_nv12_to_rgba8888(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_nv12_to_rgba8888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_nv12_to_rgba8888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_NV12 & TO_RGBA4444)
aipl_error_t aipl_color_convert_nv12_to_rgba4444(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_nv12_to_rgba4444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_nv12_to_rgba4444_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_NV12 & TO_RGBA5551)
aipl_error_t aipl_color_convert_nv12_to_rgba5551(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_nv12_to_rgba5551_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_nv12_to_rgba5551_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_NV12 & TO_BGR888)
aipl_error_t aipl_color_convert_nv12_to_bgr888(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_nv12_to_bgr888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_nv12_to_bgr888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_NV12 & TO_RGB888)
aipl_error_t aipl_color_convert_nv12_to_rgb888(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_nv12_to_rgb888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_nv12_to_rgb888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_NV12 & TO_RGB565)
aipl_error_t aipl_color_convert_nv12_to_rgb565(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_nv12_to_rgb565_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_nv12_to_rgb565_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_NV12 & TO_YV12)
aipl_error_t aipl_color_convert_nv12_to_yv12(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_nv12_to_yv12_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_nv12_to_yv12_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_NV12 & TO_I420)
aipl_error_t aipl_color_convert_nv12_to_i420(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_nv12_to_i420_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_nv12_to_i420_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_NV12 & TO_I422)
aipl_error_t aipl_color_convert_nv12_to_i422(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_nv12_to_i422_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_nv12_to_i422_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_NV12 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_nv12_to_i400(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
	return aipl_color_convert_nv12_to_i400_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_NV12 & TO_NV21)
aipl_error_t aipl_color_convert_nv12_to_nv21(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
	return aipl_color_convert_nv12_to_nv21_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_NV12 & TO_I444)
aipl_error_t aipl_color_convert_nv12_to_i444(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
	return aipl_color_convert_nv12_to_i444_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_NV12 & TO_YUY2)
aipl_error_t aipl_color_convert_nv12_to_yuy2(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
	return aipl_color_convert_nv12_to_yuy2_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_NV12 & TO_UYVY)
aipl_error_t aipl_color_convert_nv12_to_uyvy(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
	return aipl_color_convert_nv12_to_uyvy_default(input, output, pitch, width, height);
}
#endif
#endif

#if AIPL_CONVERT_NV21
aipl_error_t aipl_color_convert_nv21(const void *input, void *output, uint32_t pitch,
				     uint32_t width, uint32_t height, aipl_color_format_t format)
{
	switch (format) {
	/* Alpha color formats */
#if (AIPL_CONVERT_NV21 & TO_ALPHA8_I400)
	case AIPL_COLOR_ALPHA8:
		return aipl_color_convert_nv21_to_alpha8(input, output, pitch, width, height);
#endif
	/* RGB color formats */
#if (AIPL_CONVERT_NV21 & TO_ARGB8888)
	case AIPL_COLOR_ARGB8888:
		return aipl_color_convert_nv21_to_argb8888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_NV21 & TO_RGBA8888)
	case AIPL_COLOR_RGBA8888:
		return aipl_color_convert_nv21_to_rgba8888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_NV21 & TO_ARGB4444)
	case AIPL_COLOR_ARGB4444:
		return aipl_color_convert_nv21_to_argb4444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_NV21 & TO_ARGB1555)
	case AIPL_COLOR_ARGB1555:
		return aipl_color_convert_nv21_to_argb1555(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_NV21 & TO_RGBA4444)
	case AIPL_COLOR_RGBA4444:
		return aipl_color_convert_nv21_to_rgba4444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_NV21 & TO_RGBA5551)
	case AIPL_COLOR_RGBA5551:
		return aipl_color_convert_nv21_to_rgba5551(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_NV21 & TO_RGB565)
	case AIPL_COLOR_RGB565:
		return aipl_color_convert_nv21_to_rgb565(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_NV21 & TO_BGR888)
	case AIPL_COLOR_BGR888:
		return aipl_color_convert_nv21_to_bgr888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_NV21 & TO_RGB888)
	case AIPL_COLOR_RGB888:
		return aipl_color_convert_nv21_to_rgb888(input, output, pitch, width, height);
#endif
	/* YUV color formats */
#if (AIPL_CONVERT_NV21 & TO_YV12)
	case AIPL_COLOR_YV12:
		return aipl_color_convert_nv21_to_yv12(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_NV21 & TO_I420)
	case AIPL_COLOR_I420:
		return aipl_color_convert_nv21_to_i420(input, output, pitch, width, height);
#endif
	case AIPL_COLOR_NV21:
		return AIPL_ERR_FORMAT_MISMATCH;
#if (AIPL_CONVERT_NV21 & TO_NV12)
	case AIPL_COLOR_NV12:
		return aipl_color_convert_nv21_to_nv12(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_NV21 & TO_I422)
	case AIPL_COLOR_I422:
		return aipl_color_convert_nv21_to_i422(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_NV21 & TO_YUY2)
	case AIPL_COLOR_YUY2:
		return aipl_color_convert_nv21_to_yuy2(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_NV21 & TO_UYVY)
	case AIPL_COLOR_UYVY:
		return aipl_color_convert_nv21_to_uyvy(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_NV21 & TO_I444)
	case AIPL_COLOR_I444:
		return aipl_color_convert_nv21_to_i444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_NV21 & TO_ALPHA8_I400)
	case AIPL_COLOR_I400:
		return aipl_color_convert_nv21_to_i400(input, output, pitch, width, height);
#endif

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

#if (AIPL_CONVERT_NV21 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_nv21_to_alpha8(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
	return aipl_color_convert_nv21_to_alpha8_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_NV21 & TO_ARGB8888)
aipl_error_t aipl_color_convert_nv21_to_argb8888(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_nv21_to_argb8888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_nv21_to_argb8888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_NV21 & TO_ARGB4444)
aipl_error_t aipl_color_convert_nv21_to_argb4444(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_nv21_to_argb4444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_nv21_to_argb4444_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_NV21 & TO_ARGB1555)
aipl_error_t aipl_color_convert_nv21_to_argb1555(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_nv21_to_argb1555_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_nv21_to_argb1555_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_NV21 & TO_RGBA8888)
aipl_error_t aipl_color_convert_nv21_to_rgba8888(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_nv21_to_rgba8888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_nv21_to_rgba8888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_NV21 & TO_RGBA4444)
aipl_error_t aipl_color_convert_nv21_to_rgba4444(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_nv21_to_rgba4444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_nv21_to_rgba4444_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_NV21 & TO_RGBA5551)
aipl_error_t aipl_color_convert_nv21_to_rgba5551(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_nv21_to_rgba5551_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_nv21_to_rgba5551_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_NV21 & TO_BGR888)
aipl_error_t aipl_color_convert_nv21_to_bgr888(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_nv21_to_bgr888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_nv21_to_bgr888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_NV21 & TO_RGB888)
aipl_error_t aipl_color_convert_nv21_to_rgb888(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_nv21_to_rgb888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_nv21_to_rgb888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_NV21 & TO_RGB565)
aipl_error_t aipl_color_convert_nv21_to_rgb565(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_nv21_to_rgb565_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_nv21_to_rgb565_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_NV21 & TO_YV12)
aipl_error_t aipl_color_convert_nv21_to_yv12(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_nv21_to_yv12_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_nv21_to_yv12_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_NV21 & TO_I420)
aipl_error_t aipl_color_convert_nv21_to_i420(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_nv21_to_i420_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_nv21_to_i420_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_NV21 & TO_I422)
aipl_error_t aipl_color_convert_nv21_to_i422(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_nv21_to_i422_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_nv21_to_i422_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_NV21 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_nv21_to_i400(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
	return aipl_color_convert_nv21_to_i400_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_NV21 & TO_NV12)
aipl_error_t aipl_color_convert_nv21_to_nv12(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
	return aipl_color_convert_nv21_to_nv12_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_NV21 & TO_I444)
aipl_error_t aipl_color_convert_nv21_to_i444(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
	return aipl_color_convert_nv21_to_i444_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_NV21 & TO_YUY2)
aipl_error_t aipl_color_convert_nv21_to_yuy2(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
	return aipl_color_convert_nv21_to_yuy2_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_NV21 & TO_UYVY)
aipl_error_t aipl_color_convert_nv21_to_uyvy(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
	return aipl_color_convert_nv21_to_uyvy_default(input, output, pitch, width, height);
}
#endif
#endif

#if AIPL_CONVERT_YUY2
aipl_error_t aipl_color_convert_yuy2(const void *input, void *output, uint32_t pitch,
				     uint32_t width, uint32_t height, aipl_color_format_t format)
{
	switch (format) {
	/* Alpha color formats */
#if (AIPL_CONVERT_YUY2 & TO_ALPHA8_I400)
	case AIPL_COLOR_ALPHA8:
		return aipl_color_convert_yuy2_to_alpha8(input, output, pitch, width, height);
#endif
	/* RGB color formats */
#if (AIPL_CONVERT_YUY2 & TO_ARGB8888)
	case AIPL_COLOR_ARGB8888:
		return aipl_color_convert_yuy2_to_argb8888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_YUY2 & TO_RGBA8888)
	case AIPL_COLOR_RGBA8888:
		return aipl_color_convert_yuy2_to_rgba8888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_YUY2 & TO_ARGB4444)
	case AIPL_COLOR_ARGB4444:
		return aipl_color_convert_yuy2_to_argb4444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_YUY2 & TO_ARGB1555)
	case AIPL_COLOR_ARGB1555:
		return aipl_color_convert_yuy2_to_argb1555(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_YUY2 & TO_RGBA4444)
	case AIPL_COLOR_RGBA4444:
		return aipl_color_convert_yuy2_to_rgba4444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_YUY2 & TO_RGBA5551)
	case AIPL_COLOR_RGBA5551:
		return aipl_color_convert_yuy2_to_rgba5551(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_YUY2 & TO_RGB565)
	case AIPL_COLOR_RGB565:
		return aipl_color_convert_yuy2_to_rgb565(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_YUY2 & TO_BGR888)
	case AIPL_COLOR_BGR888:
		return aipl_color_convert_yuy2_to_bgr888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_YUY2 & TO_RGB888)
	case AIPL_COLOR_RGB888:
		return aipl_color_convert_yuy2_to_rgb888(input, output, pitch, width, height);
#endif
	/* YUV color formats */
#if (AIPL_CONVERT_YUY2 & TO_YV12)
	case AIPL_COLOR_YV12:
		return aipl_color_convert_yuy2_to_yv12(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_YUY2 & TO_I420)
	case AIPL_COLOR_I420:
		return aipl_color_convert_yuy2_to_i420(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_YUY2 & TO_NV21)
	case AIPL_COLOR_NV21:
		return aipl_color_convert_yuy2_to_nv21(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_YUY2 & TO_NV12)
	case AIPL_COLOR_NV12:
		return aipl_color_convert_yuy2_to_nv12(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_YUY2 & TO_I422)
	case AIPL_COLOR_I422:
		return aipl_color_convert_yuy2_to_i422(input, output, pitch, width, height);
#endif
	case AIPL_COLOR_YUY2:
		return AIPL_ERR_FORMAT_MISMATCH;
#if (AIPL_CONVERT_YUY2 & TO_UYVY)
	case AIPL_COLOR_UYVY:
		return aipl_color_convert_yuy2_to_uyvy(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_YUY2 & TO_I444)
	case AIPL_COLOR_I444:
		return aipl_color_convert_yuy2_to_i444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_YUY2 & TO_ALPHA8_I400)
	case AIPL_COLOR_I400:
		return aipl_color_convert_yuy2_to_i400(input, output, pitch, width, height);
#endif

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

#if (AIPL_CONVERT_YUY2 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_yuy2_to_alpha8(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_yuy2_to_alpha8_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_yuy2_to_alpha8_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_ARGB8888)
aipl_error_t aipl_color_convert_yuy2_to_argb8888(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_yuy2_to_argb8888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_yuy2_to_argb8888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_ARGB4444)
aipl_error_t aipl_color_convert_yuy2_to_argb4444(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_yuy2_to_argb4444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_yuy2_to_argb4444_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_ARGB1555)
aipl_error_t aipl_color_convert_yuy2_to_argb1555(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_yuy2_to_argb1555_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_yuy2_to_argb1555_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_RGBA8888)
aipl_error_t aipl_color_convert_yuy2_to_rgba8888(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_yuy2_to_rgba8888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_yuy2_to_rgba8888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_RGBA4444)
aipl_error_t aipl_color_convert_yuy2_to_rgba4444(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_yuy2_to_rgba4444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_yuy2_to_rgba4444_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_RGBA5551)
aipl_error_t aipl_color_convert_yuy2_to_rgba5551(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_yuy2_to_rgba5551_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_yuy2_to_rgba5551_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_BGR888)
aipl_error_t aipl_color_convert_yuy2_to_bgr888(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_yuy2_to_bgr888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_yuy2_to_bgr888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_RGB888)
aipl_error_t aipl_color_convert_yuy2_to_rgb888(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_yuy2_to_rgb888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_yuy2_to_rgb888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_RGB565)
aipl_error_t aipl_color_convert_yuy2_to_rgb565(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_yuy2_to_rgb565_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_yuy2_to_rgb565_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_YV12)
aipl_error_t aipl_color_convert_yuy2_to_yv12(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_yuy2_to_yv12_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_yuy2_to_yv12_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_I420)
aipl_error_t aipl_color_convert_yuy2_to_i420(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_yuy2_to_i420_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_yuy2_to_i420_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_I422)
aipl_error_t aipl_color_convert_yuy2_to_i422(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_yuy2_to_i422_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_yuy2_to_i422_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_yuy2_to_i400(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_yuy2_to_i400_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_yuy2_to_i400_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_NV12)
aipl_error_t aipl_color_convert_yuy2_to_nv12(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_yuy2_to_nv12_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_yuy2_to_nv12_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_I444)
aipl_error_t aipl_color_convert_yuy2_to_i444(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_yuy2_to_i444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_yuy2_to_i444_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_NV21)
aipl_error_t aipl_color_convert_yuy2_to_nv21(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_yuy2_to_nv21_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_yuy2_to_nv21_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_YUY2 & TO_UYVY)
aipl_error_t aipl_color_convert_yuy2_to_uyvy(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
	return aipl_color_convert_yuy2_to_uyvy_default(input, output, pitch, width, height);
}
#endif
#endif

#if AIPL_CONVERT_UYVY
aipl_error_t aipl_color_convert_uyvy(const void *input, void *output, uint32_t pitch,
				     uint32_t width, uint32_t height, aipl_color_format_t format)
{
	switch (format) {
	/* Alpha color formats */
#if (AIPL_CONVERT_UYVY & TO_ALPHA8_I400)
	case AIPL_COLOR_ALPHA8:
		return aipl_color_convert_uyvy_to_alpha8(input, output, pitch, width, height);
#endif
	/* RGB color formats */
#if (AIPL_CONVERT_UYVY & TO_ARGB8888)
	case AIPL_COLOR_ARGB8888:
		return aipl_color_convert_uyvy_to_argb8888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_UYVY & TO_RGBA8888)
	case AIPL_COLOR_RGBA8888:
		return aipl_color_convert_uyvy_to_rgba8888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_UYVY & TO_ARGB4444)
	case AIPL_COLOR_ARGB4444:
		return aipl_color_convert_uyvy_to_argb4444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_UYVY & TO_ARGB1555)
	case AIPL_COLOR_ARGB1555:
		return aipl_color_convert_uyvy_to_argb1555(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_UYVY & TO_RGBA4444)
	case AIPL_COLOR_RGBA4444:
		return aipl_color_convert_uyvy_to_rgba4444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_UYVY & TO_RGBA5551)
	case AIPL_COLOR_RGBA5551:
		return aipl_color_convert_uyvy_to_rgba5551(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_UYVY & TO_RGB565)
	case AIPL_COLOR_RGB565:
		return aipl_color_convert_uyvy_to_rgb565(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_UYVY & TO_BGR888)
	case AIPL_COLOR_BGR888:
		return aipl_color_convert_uyvy_to_bgr888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_UYVY & TO_RGB888)
	case AIPL_COLOR_RGB888:
		return aipl_color_convert_uyvy_to_rgb888(input, output, pitch, width, height);
#endif
	/* YUV color formats */
#if (AIPL_CONVERT_UYVY & TO_YV12)
	case AIPL_COLOR_YV12:
		return aipl_color_convert_uyvy_to_yv12(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_UYVY & TO_I420)
	case AIPL_COLOR_I420:
		return aipl_color_convert_uyvy_to_i420(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_UYVY & TO_NV21)
	case AIPL_COLOR_NV21:
		return aipl_color_convert_uyvy_to_nv21(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_UYVY & TO_NV12)
	case AIPL_COLOR_NV12:
		return aipl_color_convert_uyvy_to_nv12(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_UYVY & TO_I422)
	case AIPL_COLOR_I422:
		return aipl_color_convert_uyvy_to_i422(input, output, pitch, width, height);
#endif
	case AIPL_COLOR_UYVY:
		return AIPL_ERR_FORMAT_MISMATCH;
#if (AIPL_CONVERT_UYVY & TO_YUY2)
	case AIPL_COLOR_YUY2:
		return aipl_color_convert_uyvy_to_yuy2(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_UYVY & TO_I444)
	case AIPL_COLOR_I444:
		return aipl_color_convert_uyvy_to_i444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_UYVY & TO_ALPHA8_I400)
	case AIPL_COLOR_I400:
		return aipl_color_convert_uyvy_to_i400(input, output, pitch, width, height);
#endif

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

#if (AIPL_CONVERT_UYVY & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_uyvy_to_alpha8(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_uyvy_to_alpha8_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_uyvy_to_alpha8_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_UYVY & TO_ARGB8888)
aipl_error_t aipl_color_convert_uyvy_to_argb8888(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_uyvy_to_argb8888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_uyvy_to_argb8888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_UYVY & TO_ARGB4444)
aipl_error_t aipl_color_convert_uyvy_to_argb4444(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_uyvy_to_argb4444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_uyvy_to_argb4444_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_UYVY & TO_ARGB1555)
aipl_error_t aipl_color_convert_uyvy_to_argb1555(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_uyvy_to_argb1555_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_uyvy_to_argb1555_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_UYVY & TO_RGBA8888)
aipl_error_t aipl_color_convert_uyvy_to_rgba8888(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_uyvy_to_rgba8888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_uyvy_to_rgba8888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_UYVY & TO_RGBA4444)
aipl_error_t aipl_color_convert_uyvy_to_rgba4444(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_uyvy_to_rgba4444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_uyvy_to_rgba4444_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_UYVY & TO_RGBA5551)
aipl_error_t aipl_color_convert_uyvy_to_rgba5551(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_uyvy_to_rgba5551_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_uyvy_to_rgba5551_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_UYVY & TO_BGR888)
aipl_error_t aipl_color_convert_uyvy_to_bgr888(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_uyvy_to_bgr888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_uyvy_to_bgr888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_UYVY & TO_RGB888)
aipl_error_t aipl_color_convert_uyvy_to_rgb888(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_uyvy_to_rgb888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_uyvy_to_rgb888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_UYVY & TO_RGB565)
aipl_error_t aipl_color_convert_uyvy_to_rgb565(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_uyvy_to_rgb565_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_uyvy_to_rgb565_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_UYVY & TO_YV12)
aipl_error_t aipl_color_convert_uyvy_to_yv12(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_uyvy_to_yv12_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_uyvy_to_yv12_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_UYVY & TO_I420)
aipl_error_t aipl_color_convert_uyvy_to_i420(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_uyvy_to_i420_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_uyvy_to_i420_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_UYVY & TO_I422)
aipl_error_t aipl_color_convert_uyvy_to_i422(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_uyvy_to_i422_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_uyvy_to_i422_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_UYVY & TO_ALPHA8_I400)
aipl_error_t aipl_color_convert_uyvy_to_i400(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_uyvy_to_i400_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_uyvy_to_i400_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_UYVY & TO_NV12)
aipl_error_t aipl_color_convert_uyvy_to_nv12(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_uyvy_to_nv12_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_uyvy_to_nv12_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_UYVY & TO_I444)
aipl_error_t aipl_color_convert_uyvy_to_i444(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_uyvy_to_i444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_uyvy_to_i444_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_UYVY & TO_NV21)
aipl_error_t aipl_color_convert_uyvy_to_nv21(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_uyvy_to_nv21_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_uyvy_to_nv21_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_UYVY & TO_YUY2)
aipl_error_t aipl_color_convert_uyvy_to_yuy2(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
	return aipl_color_convert_uyvy_to_yuy2_default(input, output, pitch, width, height);
}
#endif
#endif

#if (AIPL_CONVERT_ALPHA8_I400)
aipl_error_t aipl_color_convert_i400(const void *input, void *output, uint32_t pitch,
				     uint32_t width, uint32_t height, aipl_color_format_t format)
{
	switch (format) {
	/* Alpha color formats */
	case AIPL_COLOR_ALPHA8:
		return aipl_color_convert_i400_to_alpha8(input, output, pitch, width, height);
	/* RGB color formats */
#if (AIPL_CONVERT_ALPHA8_I400 & TO_ARGB8888)
	case AIPL_COLOR_ARGB8888:
		return aipl_color_convert_i400_to_argb8888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ALPHA8_I400 & TO_RGBA8888)
	case AIPL_COLOR_RGBA8888:
		return aipl_color_convert_i400_to_rgba8888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ALPHA8_I400 & TO_ARGB4444)
	case AIPL_COLOR_ARGB4444:
		return aipl_color_convert_i400_to_argb4444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ALPHA8_I400 & TO_ARGB1555)
	case AIPL_COLOR_ARGB1555:
		return aipl_color_convert_i400_to_argb1555(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ALPHA8_I400 & TO_RGBA4444)
	case AIPL_COLOR_RGBA4444:
		return aipl_color_convert_i400_to_rgba4444(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ALPHA8_I400 & TO_RGBA5551)
	case AIPL_COLOR_RGBA5551:
		return aipl_color_convert_i400_to_rgba5551(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ALPHA8_I400 & TO_RGB565)
	case AIPL_COLOR_RGB565:
		return aipl_color_convert_i400_to_rgb565(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ALPHA8_I400 & TO_BGR888)
	case AIPL_COLOR_BGR888:
		return aipl_color_convert_i400_to_bgr888(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ALPHA8_I400 & TO_RGB888)
	case AIPL_COLOR_RGB888:
		return aipl_color_convert_i400_to_rgb888(input, output, pitch, width, height);
#endif
	/* YUV color formats */
#if (AIPL_CONVERT_ALPHA8_I400 & TO_YV12)
	case AIPL_COLOR_YV12:
		return aipl_color_convert_i400_to_yv12(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ALPHA8_I400 & TO_I420)
	case AIPL_COLOR_I420:
		return aipl_color_convert_i400_to_i420(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ALPHA8_I400 & TO_NV21)
	case AIPL_COLOR_NV21:
		return aipl_color_convert_i400_to_nv21(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ALPHA8_I400 & TO_NV12)
	case AIPL_COLOR_NV12:
		return aipl_color_convert_i400_to_nv12(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ALPHA8_I400 & TO_I422)
	case AIPL_COLOR_I422:
		return aipl_color_convert_i400_to_i422(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ALPHA8_I400 & TO_UYVY)
	case AIPL_COLOR_UYVY:
		return aipl_color_convert_i400_to_uyvy(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ALPHA8_I400 & TO_YUY2)
	case AIPL_COLOR_YUY2:
		return aipl_color_convert_i400_to_yuy2(input, output, pitch, width, height);
#endif
#if (AIPL_CONVERT_ALPHA8_I400 & TO_I444)
	case AIPL_COLOR_I444:
		return aipl_color_convert_i400_to_i444(input, output, pitch, width, height);
#endif
	case AIPL_COLOR_I400:
		return AIPL_ERR_FORMAT_MISMATCH;

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

aipl_error_t aipl_color_convert_i400_to_alpha8(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
	return aipl_color_convert_i400_to_alpha8_default(input, output, pitch, width, height);
}

#if (AIPL_CONVERT_ALPHA8_I400 & TO_ARGB8888)
aipl_error_t aipl_color_convert_i400_to_argb8888(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i400_to_argb8888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i400_to_argb8888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_ARGB4444)
aipl_error_t aipl_color_convert_i400_to_argb4444(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i400_to_argb4444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i400_to_argb4444_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_ARGB1555)
aipl_error_t aipl_color_convert_i400_to_argb1555(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i400_to_argb1555_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i400_to_argb1555_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_RGBA8888)
aipl_error_t aipl_color_convert_i400_to_rgba8888(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i400_to_rgba8888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i400_to_rgba8888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_RGBA4444)
aipl_error_t aipl_color_convert_i400_to_rgba4444(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i400_to_rgba4444_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i400_to_rgba4444_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_RGBA5551)
aipl_error_t aipl_color_convert_i400_to_rgba5551(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i400_to_rgba5551_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i400_to_rgba5551_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_BGR888)
aipl_error_t aipl_color_convert_i400_to_bgr888(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i400_to_bgr888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i400_to_bgr888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_RGB888)
aipl_error_t aipl_color_convert_i400_to_rgb888(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i400_to_rgb888_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i400_to_rgb888_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_RGB565)
aipl_error_t aipl_color_convert_i400_to_rgb565(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i400_to_rgb565_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i400_to_rgb565_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_YV12)
aipl_error_t aipl_color_convert_i400_to_yv12(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
	return aipl_color_convert_i400_to_yv12_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_I420)
aipl_error_t aipl_color_convert_i400_to_i420(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
	return aipl_color_convert_i400_to_i420_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_I422)
aipl_error_t aipl_color_convert_i400_to_i422(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
	return aipl_color_convert_i400_to_i422_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_NV12)
aipl_error_t aipl_color_convert_i400_to_nv12(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
	return aipl_color_convert_i400_to_nv12_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_I444)
aipl_error_t aipl_color_convert_i400_to_i444(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
	return aipl_color_convert_i400_to_i444_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_NV21)
aipl_error_t aipl_color_convert_i400_to_nv21(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
	return aipl_color_convert_i400_to_nv21_default(input, output, pitch, width, height);
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_YUY2)
aipl_error_t aipl_color_convert_i400_to_yuy2(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i400_to_yuy2_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i400_to_yuy2_default(input, output, pitch, width, height);
#endif
}
#endif

#if (AIPL_CONVERT_ALPHA8_I400 & TO_UYVY)
aipl_error_t aipl_color_convert_i400_to_uyvy(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_convert_i400_to_uyvy_helium(input, output, pitch, width, height);
#else
	return aipl_color_convert_i400_to_uyvy_default(input, output, pitch, width, height);
#endif
}
#endif

#endif

/**********************
 *   STATIC FUNCTIONS
 **********************/

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
 * @file    aipl_color_conversion_dave2d.c
 * @brief   D/AVE2D accelerated color conversion function implementations
 *
 ******************************************************************************/

/*********************
 *      INCLUDES
 *********************/
#include "aipl_color_conversion_dave2d.h"

#include <stddef.h>

#include "aipl_config.h"
#include "aipl_dave2d.h"

#ifdef AIPL_DAVE2D_ACCELERATION

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
aipl_error_t aipl_color_convert_dave2d(const void *input, void *output, uint32_t pitch,
				       uint32_t width, uint32_t height,
				       aipl_color_format_t input_format,
				       aipl_color_format_t output_format)
{
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

	return AIPL_ERR_UNSUPPORTED_FORMAT;
}

aipl_error_t aipl_color_convert_img_dave2d(const aipl_image_t *input, aipl_image_t *output)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	if (input->width != output->width || input->height != output->height) {
		return AIPL_ERR_SIZE_MISMATCH;
	}

	return aipl_color_convert_dave2d(input->data, output->data, input->pitch, input->width,
					 input->height, input->format, output->format);
}

aipl_error_t aipl_color_convert_argb8888_dave2d(const void *input, void *output, uint32_t pitch,
						uint32_t width, uint32_t height,
						aipl_color_format_t format)
{
	switch (format) {
	/* RGB color formats */
	case AIPL_COLOR_ARGB8888:
		return AIPL_ERR_FORMAT_MISMATCH;
	case AIPL_COLOR_RGBA8888:
		return aipl_color_convert_argb8888_to_rgba8888_dave2d(input, output, pitch, width,
								      height);
	case AIPL_COLOR_ARGB4444:
		return aipl_color_convert_argb8888_to_argb4444_dave2d(input, output, pitch, width,
								      height);
	case AIPL_COLOR_RGBA4444:
		return aipl_color_convert_argb8888_to_rgba4444_dave2d(input, output, pitch, width,
								      height);
	case AIPL_COLOR_RGB565:
		return aipl_color_convert_argb8888_to_rgb565_dave2d(input, output, pitch, width,
								    height);
	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

aipl_error_t aipl_color_convert_argb8888_to_argb4444_dave2d(const void *input, void *output,
							    uint32_t pitch, uint32_t width,
							    uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	d2_s32 ret = aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
						    d2_mode_argb8888, d2_mode_argb4444);

	return aipl_dave2d_error_convert(ret);
}

aipl_error_t aipl_color_convert_argb8888_to_rgba8888_dave2d(const void *input, void *output,
							    uint32_t pitch, uint32_t width,
							    uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	d2_s32 ret = aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
						    d2_mode_argb8888, d2_mode_rgba8888);

	return aipl_dave2d_error_convert(ret);
}

aipl_error_t aipl_color_convert_argb8888_to_rgba4444_dave2d(const void *input, void *output,
							    uint32_t pitch, uint32_t width,
							    uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	d2_s32 ret = aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
						    d2_mode_argb8888, d2_mode_rgba4444);

	return aipl_dave2d_error_convert(ret);
}

aipl_error_t aipl_color_convert_argb8888_to_rgb565_dave2d(const void *input, void *output,
							  uint32_t pitch, uint32_t width,
							  uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	d2_s32 ret = aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
						    d2_mode_argb8888, d2_mode_rgb565);

	return aipl_dave2d_error_convert(ret);
}

aipl_error_t aipl_color_convert_argb4444_dave2d(const void *input, void *output, uint32_t pitch,
						uint32_t width, uint32_t height,
						aipl_color_format_t format)
{
	switch (format) {
	/* RGB color formats */
	case AIPL_COLOR_ARGB8888:
		return aipl_color_convert_argb4444_to_argb8888_dave2d(input, output, pitch, width,
								      height);
	case AIPL_COLOR_RGBA8888:
		return aipl_color_convert_argb4444_to_rgba8888_dave2d(input, output, pitch, width,
								      height);
	case AIPL_COLOR_ARGB4444:
		return AIPL_ERR_FORMAT_MISMATCH;
	case AIPL_COLOR_RGBA4444:
		return aipl_color_convert_argb4444_to_rgba4444_dave2d(input, output, pitch, width,
								      height);
	case AIPL_COLOR_RGB565:
		return aipl_color_convert_argb4444_to_rgb565_dave2d(input, output, pitch, width,
								    height);

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

aipl_error_t aipl_color_convert_argb4444_to_argb8888_dave2d(const void *input, void *output,
							    uint32_t pitch, uint32_t width,
							    uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	d2_s32 ret = aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
						    d2_mode_argb4444, d2_mode_argb8888);

	return aipl_dave2d_error_convert(ret);
}

aipl_error_t aipl_color_convert_argb4444_to_rgba8888_dave2d(const void *input, void *output,
							    uint32_t pitch, uint32_t width,
							    uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	d2_s32 ret = aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
						    d2_mode_argb4444, d2_mode_rgba8888);

	return aipl_dave2d_error_convert(ret);
}

aipl_error_t aipl_color_convert_argb4444_to_rgba4444_dave2d(const void *input, void *output,
							    uint32_t pitch, uint32_t width,
							    uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	d2_s32 ret = aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
						    d2_mode_argb4444, d2_mode_rgba4444);

	return aipl_dave2d_error_convert(ret);
}

aipl_error_t aipl_color_convert_argb4444_to_rgb565_dave2d(const void *input, void *output,
							  uint32_t pitch, uint32_t width,
							  uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	d2_s32 ret = aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
						    d2_mode_argb4444, d2_mode_rgb565);

	return aipl_dave2d_error_convert(ret);
}

aipl_error_t aipl_color_convert_argb1555_dave2d(const void *input, void *output, uint32_t pitch,
						uint32_t width, uint32_t height,
						aipl_color_format_t format)
{
	switch (format) {
	/* Alpha color formats */
	/* RGB color formats */
	case AIPL_COLOR_ARGB8888:
		return aipl_color_convert_argb1555_to_argb8888_dave2d(input, output, pitch, width,
								      height);
	case AIPL_COLOR_RGBA8888:
		return aipl_color_convert_argb1555_to_rgba8888_dave2d(input, output, pitch, width,
								      height);
	case AIPL_COLOR_ARGB4444:
		return aipl_color_convert_argb1555_to_argb4444_dave2d(input, output, pitch, width,
								      height);
	case AIPL_COLOR_ARGB1555:
		return AIPL_ERR_FORMAT_MISMATCH;
	case AIPL_COLOR_RGBA4444:
		return aipl_color_convert_argb1555_to_rgba4444_dave2d(input, output, pitch, width,
								      height);
	case AIPL_COLOR_RGB565:
		return aipl_color_convert_argb1555_to_rgb565_dave2d(input, output, pitch, width,
								    height);
	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

aipl_error_t aipl_color_convert_argb1555_to_argb8888_dave2d(const void *input, void *output,
							    uint32_t pitch, uint32_t width,
							    uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	d2_s32 ret = aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
						    d2_mode_argb1555, d2_mode_argb8888);

	return aipl_dave2d_error_convert(ret);
}

aipl_error_t aipl_color_convert_argb1555_to_argb4444_dave2d(const void *input, void *output,
							    uint32_t pitch, uint32_t width,
							    uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	d2_s32 ret = aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
						    d2_mode_argb1555, d2_mode_argb4444);

	return aipl_dave2d_error_convert(ret);
}

aipl_error_t aipl_color_convert_argb1555_to_rgba8888_dave2d(const void *input, void *output,
							    uint32_t pitch, uint32_t width,
							    uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	d2_s32 ret = aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
						    d2_mode_argb1555, d2_mode_rgba8888);

	return aipl_dave2d_error_convert(ret);
}

aipl_error_t aipl_color_convert_argb1555_to_rgba4444_dave2d(const void *input, void *output,
							    uint32_t pitch, uint32_t width,
							    uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	d2_s32 ret = aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
						    d2_mode_argb1555, d2_mode_rgba4444);

	return aipl_dave2d_error_convert(ret);
}

aipl_error_t aipl_color_convert_argb1555_to_rgb565_dave2d(const void *input, void *output,
							  uint32_t pitch, uint32_t width,
							  uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	d2_s32 ret = aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
						    d2_mode_argb1555, d2_mode_rgb565);

	return aipl_dave2d_error_convert(ret);
}

aipl_error_t aipl_color_convert_rgba8888_dave2d(const void *input, void *output, uint32_t pitch,
						uint32_t width, uint32_t height,
						aipl_color_format_t format)
{
	switch (format) {
	/* RGB color formats */
	case AIPL_COLOR_ARGB8888:
		return aipl_color_convert_rgba8888_to_argb8888_dave2d(input, output, pitch, width,
								      height);
	case AIPL_COLOR_RGBA8888:
		return AIPL_ERR_FORMAT_MISMATCH;
	case AIPL_COLOR_ARGB4444:
		return aipl_color_convert_rgba8888_to_argb4444_dave2d(input, output, pitch, width,
								      height);
	case AIPL_COLOR_RGBA4444:
		return aipl_color_convert_rgba8888_to_rgba4444_dave2d(input, output, pitch, width,
								      height);
	case AIPL_COLOR_RGB565:
		return aipl_color_convert_rgba8888_to_rgb565_dave2d(input, output, pitch, width,
								    height);

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

aipl_error_t aipl_color_convert_rgba8888_to_argb8888_dave2d(const void *input, void *output,
							    uint32_t pitch, uint32_t width,
							    uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	d2_s32 ret = aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
						    d2_mode_rgba8888, d2_mode_argb8888);

	return aipl_dave2d_error_convert(ret);
}

aipl_error_t aipl_color_convert_rgba8888_to_argb4444_dave2d(const void *input, void *output,
							    uint32_t pitch, uint32_t width,
							    uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	d2_s32 ret = aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
						    d2_mode_rgba8888, d2_mode_argb4444);

	return aipl_dave2d_error_convert(ret);
}

aipl_error_t aipl_color_convert_rgba8888_to_rgba4444_dave2d(const void *input, void *output,
							    uint32_t pitch, uint32_t width,
							    uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	d2_s32 ret = aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
						    d2_mode_rgba8888, d2_mode_rgba4444);

	return aipl_dave2d_error_convert(ret);
}

aipl_error_t aipl_color_convert_rgba8888_to_rgb565_dave2d(const void *input, void *output,
							  uint32_t pitch, uint32_t width,
							  uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	d2_s32 ret = aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
						    d2_mode_rgba8888, d2_mode_rgb565);

	return aipl_dave2d_error_convert(ret);
}

aipl_error_t aipl_color_convert_rgba4444_dave2d(const void *input, void *output, uint32_t pitch,
						uint32_t width, uint32_t height,
						aipl_color_format_t format)
{
	switch (format) {
	/* RGB color formats */
	case AIPL_COLOR_ARGB8888:
		return aipl_color_convert_rgba4444_to_argb8888_dave2d(input, output, pitch, width,
								      height);
	case AIPL_COLOR_RGBA8888:
		return aipl_color_convert_rgba4444_to_rgba8888_dave2d(input, output, pitch, width,
								      height);
	case AIPL_COLOR_ARGB4444:
		return aipl_color_convert_rgba4444_to_argb4444_dave2d(input, output, pitch, width,
								      height);
	case AIPL_COLOR_RGBA4444:
		return AIPL_ERR_FORMAT_MISMATCH;
	case AIPL_COLOR_RGB565:
		return aipl_color_convert_rgba4444_to_rgb565_dave2d(input, output, pitch, width,
								    height);

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

aipl_error_t aipl_color_convert_rgba4444_to_argb8888_dave2d(const void *input, void *output,
							    uint32_t pitch, uint32_t width,
							    uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	d2_s32 ret = aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
						    d2_mode_rgba4444, d2_mode_argb8888);

	return aipl_dave2d_error_convert(ret);
}

aipl_error_t aipl_color_convert_rgba4444_to_argb4444_dave2d(const void *input, void *output,
							    uint32_t pitch, uint32_t width,
							    uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	d2_s32 ret = aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
						    d2_mode_rgba4444, d2_mode_rgba8888);

	return aipl_dave2d_error_convert(ret);
}

aipl_error_t aipl_color_convert_rgba4444_to_rgba8888_dave2d(const void *input, void *output,
							    uint32_t pitch, uint32_t width,
							    uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	d2_s32 ret = aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
						    d2_mode_rgba4444, d2_mode_rgba4444);

	return aipl_dave2d_error_convert(ret);
}

aipl_error_t aipl_color_convert_rgba4444_to_rgb565_dave2d(const void *input, void *output,
							  uint32_t pitch, uint32_t width,
							  uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	d2_s32 ret = aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
						    d2_mode_rgba4444, d2_mode_rgb565);

	return aipl_dave2d_error_convert(ret);
}

aipl_error_t aipl_color_convert_rgba5551_dave2d(const void *input, void *output, uint32_t pitch,
						uint32_t width, uint32_t height,
						aipl_color_format_t format)
{
	switch (format) {
	/* RGB color formats */
	case AIPL_COLOR_ARGB8888:
		return aipl_color_convert_rgba5551_to_argb8888_dave2d(input, output, pitch, width,
								      height);
	case AIPL_COLOR_RGBA8888:
		return aipl_color_convert_rgba5551_to_rgba8888_dave2d(input, output, pitch, width,
								      height);
	case AIPL_COLOR_ARGB4444:
		return aipl_color_convert_rgba5551_to_argb4444_dave2d(input, output, pitch, width,
								      height);
	case AIPL_COLOR_RGBA4444:
		return aipl_color_convert_rgba5551_to_rgba4444_dave2d(input, output, pitch, width,
								      height);
	case AIPL_COLOR_RGBA5551:
		return AIPL_ERR_FORMAT_MISMATCH;
	case AIPL_COLOR_RGB565:
		return aipl_color_convert_rgba5551_to_rgb565_dave2d(input, output, pitch, width,
								    height);

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

aipl_error_t aipl_color_convert_rgba5551_to_argb8888_dave2d(const void *input, void *output,
							    uint32_t pitch, uint32_t width,
							    uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	d2_s32 ret = aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
						    d2_mode_rgba5551, d2_mode_argb8888);

	return aipl_dave2d_error_convert(ret);
}

aipl_error_t aipl_color_convert_rgba5551_to_argb4444_dave2d(const void *input, void *output,
							    uint32_t pitch, uint32_t width,
							    uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	d2_s32 ret = aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
						    d2_mode_rgba5551, d2_mode_rgba8888);

	return aipl_dave2d_error_convert(ret);
}

aipl_error_t aipl_color_convert_rgba5551_to_rgba8888_dave2d(const void *input, void *output,
							    uint32_t pitch, uint32_t width,
							    uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	d2_s32 ret = aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
						    d2_mode_rgba5551, d2_mode_rgba5551);

	return aipl_dave2d_error_convert(ret);
}

aipl_error_t aipl_color_convert_rgba5551_to_rgba4444_dave2d(const void *input, void *output,
							    uint32_t pitch, uint32_t width,
							    uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	d2_s32 ret = aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
						    d2_mode_rgba5551, d2_mode_rgba4444);

	return aipl_dave2d_error_convert(ret);
}

aipl_error_t aipl_color_convert_rgba5551_to_rgb565_dave2d(const void *input, void *output,
							  uint32_t pitch, uint32_t width,
							  uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	d2_s32 ret = aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
						    d2_mode_rgba5551, d2_mode_rgb565);

	return aipl_dave2d_error_convert(ret);
}

aipl_error_t aipl_color_convert_rgb565_dave2d(const void *input, void *output, uint32_t pitch,
					      uint32_t width, uint32_t height,
					      aipl_color_format_t format)
{
	switch (format) {
	/* RGB color formats */
	case AIPL_COLOR_ARGB8888:
		return aipl_color_convert_rgb565_to_argb8888_dave2d(input, output, pitch, width,
								    height);
	case AIPL_COLOR_RGBA8888:
		return aipl_color_convert_rgb565_to_rgba8888_dave2d(input, output, pitch, width,
								    height);
	case AIPL_COLOR_ARGB4444:
		return aipl_color_convert_rgb565_to_argb4444_dave2d(input, output, pitch, width,
								    height);
	case AIPL_COLOR_RGBA4444:
		return aipl_color_convert_rgb565_to_rgba4444_dave2d(input, output, pitch, width,
								    height);
	case AIPL_COLOR_RGB565:
		return AIPL_ERR_FORMAT_MISMATCH;

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

aipl_error_t aipl_color_convert_rgb565_to_argb8888_dave2d(const void *input, void *output,
							  uint32_t pitch, uint32_t width,
							  uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	d2_s32 ret = aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
						    d2_mode_rgb565, d2_mode_argb8888);

	return aipl_dave2d_error_convert(ret);
}

aipl_error_t aipl_color_convert_rgb565_to_argb4444_dave2d(const void *input, void *output,
							  uint32_t pitch, uint32_t width,
							  uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	d2_s32 ret = aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
						    d2_mode_rgb565, d2_mode_rgba8888);

	return aipl_dave2d_error_convert(ret);
}

aipl_error_t aipl_color_convert_rgb565_to_rgba8888_dave2d(const void *input, void *output,
							  uint32_t pitch, uint32_t width,
							  uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	d2_s32 ret = aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
						    d2_mode_rgb565, d2_mode_rgb565);

	return aipl_dave2d_error_convert(ret);
}

aipl_error_t aipl_color_convert_rgb565_to_rgba4444_dave2d(const void *input, void *output,
							  uint32_t pitch, uint32_t width,
							  uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	d2_s32 ret = aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
						    d2_mode_rgb565, d2_mode_rgba4444);

	return aipl_dave2d_error_convert(ret);
}

aipl_error_t aipl_color_convert_alpha8_dave2d(const void *input, void *output, uint32_t pitch,
					      uint32_t width, uint32_t height,
					      aipl_color_format_t format)
{
	switch (format) {
	/* RGB color formats */
	case AIPL_COLOR_ARGB8888:
		return aipl_color_convert_alpha8_to_argb8888_dave2d(input, output, pitch, width,
								    height);
	case AIPL_COLOR_RGBA8888:
		return aipl_color_convert_alpha8_to_rgba8888_dave2d(input, output, pitch, width,
								    height);
	case AIPL_COLOR_ARGB4444:
		return aipl_color_convert_alpha8_to_argb4444_dave2d(input, output, pitch, width,
								    height);
	case AIPL_COLOR_RGBA4444:
		return aipl_color_convert_alpha8_to_rgba4444_dave2d(input, output, pitch, width,
								    height);
	case AIPL_COLOR_RGB565:
		return aipl_color_convert_alpha8_to_rgb565_dave2d(input, output, pitch, width,
								  height);

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

aipl_error_t aipl_color_convert_alpha8_to_argb8888_dave2d(const void *input, void *output,
							  uint32_t pitch, uint32_t width,
							  uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	d2_s32 ret = aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
						    d2_mode_alpha8, d2_mode_argb8888);

	return aipl_dave2d_error_convert(ret);
}

aipl_error_t aipl_color_convert_alpha8_to_argb4444_dave2d(const void *input, void *output,
							  uint32_t pitch, uint32_t width,
							  uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	d2_s32 ret = aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
						    d2_mode_alpha8, d2_mode_argb4444);

	return aipl_dave2d_error_convert(ret);
}

aipl_error_t aipl_color_convert_alpha8_to_rgba8888_dave2d(const void *input, void *output,
							  uint32_t pitch, uint32_t width,
							  uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	d2_s32 ret = aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
						    d2_mode_alpha8, d2_mode_rgba8888);

	return aipl_dave2d_error_convert(ret);
}

aipl_error_t aipl_color_convert_alpha8_to_rgba4444_dave2d(const void *input, void *output,
							  uint32_t pitch, uint32_t width,
							  uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	d2_s32 ret = aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
						    d2_mode_alpha8, d2_mode_rgba4444);

	return aipl_dave2d_error_convert(ret);
}

aipl_error_t aipl_color_convert_alpha8_to_rgb565_dave2d(const void *input, void *output,
							uint32_t pitch, uint32_t width,
							uint32_t height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	d2_s32 ret = aipl_dave2d_color_mode_convert(input, output, pitch, width, height,
						    d2_mode_alpha8, d2_mode_rgb565);

	return aipl_dave2d_error_convert(ret);
}

#endif /* AIPL_DAVE2D_ACCELERATION */

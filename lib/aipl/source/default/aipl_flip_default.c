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
 * @file    aipl_flip_default.c
 * @brief   Default flip function implementations
 *
 ******************************************************************************/

/*********************
 *      INCLUDES
 *********************/
#include "aipl_flip_default.h"

#include <string.h>
#include <stddef.h>

#include "aipl_cache.h"

#if !defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT)

/*********************
 *      DEFINES
 *********************/
#define INLINE inline __attribute__((always_inline))

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static INLINE void aipl_flip_row_hor(const uint8_t *src, uint8_t *dst, uint32_t width,
				     int rgbBytes);
static INLINE void aipl_flip_rows_hor_ver(const uint8_t *src_top, const uint8_t *src_bottom,
					  uint8_t *dst_top, uint8_t *dst_bottom, uint32_t width,
					  int rgbBytes);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

aipl_error_t aipl_flip_default(const void *input, void *output, uint32_t pitch, uint32_t width,
			       uint32_t height, aipl_color_format_t format, bool flip_horizontal,
			       bool flip_vertical)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	if (format >= AIPL_COLOR_YV12) {
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}

	const uint8_t *src = input;
	uint8_t *dst = output;

	const int rgbBytes = aipl_color_format_depth(format) / 8;

	if (flip_horizontal && flip_vertical) {
		const uint8_t *src_top = src;
		const uint8_t *src_bottom = src + (height - 1) * pitch * rgbBytes;
		uint8_t *dst_top = dst;
		uint8_t *dst_bottom = dst + (height - 1) * width * rgbBytes;

		for (uint32_t y = 0; y < height / 2; ++y) {
			aipl_flip_rows_hor_ver(src_top, src_bottom, dst_top, dst_bottom, width,
					       rgbBytes);

			src_top += pitch * rgbBytes;
			src_bottom -= pitch * rgbBytes;
			dst_top += width * rgbBytes;
			dst_bottom -= width * rgbBytes;
		}

		if (height % 2) {
			aipl_flip_row_hor(src_top, dst_top, width, rgbBytes);
		}
	} else if (flip_horizontal) {
		const uint8_t *src_row = src;
		uint8_t *dst_row = dst;

		for (uint32_t y = 0; y < height; ++y) {
			aipl_flip_row_hor(src_row, dst_row, width, rgbBytes);

			src_row += pitch * rgbBytes;
			dst_row += width * rgbBytes;
		}
	} else if (flip_vertical) {
		const uint8_t *src_top = src;
		const uint8_t *src_bottom = src + (height - 1) * pitch * rgbBytes;
		uint8_t *dst_top = dst;
		uint8_t *dst_bottom = dst + (height - 1) * width * rgbBytes;

		for (uint32_t y = 0; y < height / 2; ++y) {
			memmove(dst_top, src_bottom, width * rgbBytes);
			memmove(dst_bottom, src_top, width * rgbBytes);

			src_top += pitch * rgbBytes;
			src_bottom -= pitch * rgbBytes;
			dst_top += width * rgbBytes;
			dst_bottom -= width * rgbBytes;
		}

		if (height % 2) {
			memmove(dst_top, src_top, width * rgbBytes);
		}
	} else {
		return AIPL_ERR_NOT_SUPPORTED;
	}

	size_t size = width * height * rgbBytes;

	aipl_cpu_cache_clean(dst, size);

	return AIPL_ERR_OK;
}

aipl_error_t aipl_flip_img_default(const aipl_image_t *input, aipl_image_t *output,
				   bool flip_horizontal, bool flip_vertical)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	if (output->format != input->format) {
		return AIPL_ERR_FORMAT_MISMATCH;
	}

	return aipl_flip_default(input->data, output->data, input->pitch, input->width,
				 input->height, input->format, flip_horizontal, flip_vertical);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static INLINE void aipl_flip_row_hor(const uint8_t *src, uint8_t *dst, uint32_t width, int rgbBytes)
{
	uint32_t row_last = (width - 1) * rgbBytes;
	uint32_t row_half = width / 2 * rgbBytes;

	for (uint32_t i = 0; i < row_half; i += rgbBytes) {
		for (uint32_t j = 0; j < rgbBytes; ++j) {
			dst[i + j] = src[row_last - i + j];
			dst[row_last - i + j] = src[i + j];
		}
	}

	if (width % 2) {
		for (uint32_t j = 0; j < rgbBytes; ++j) {
			dst[row_half + j] = src[row_half + j];
		}
	}
}

static INLINE void aipl_flip_rows_hor_ver(const uint8_t *src_top, const uint8_t *src_bottom,
					  uint8_t *dst_top, uint8_t *dst_bottom, uint32_t width,
					  int rgbBytes)
{
	uint32_t row_last = (width - 1) * rgbBytes;
	uint32_t row_half = width / 2 * rgbBytes;

	for (uint32_t i = 0; i < row_half; i += rgbBytes) {
		for (uint32_t j = 0; j < rgbBytes; ++j) {
			dst_top[i + j] = src_bottom[row_last - i + j];
			dst_top[row_last - i + j] = src_bottom[i + j];
			dst_bottom[i + j] = src_top[row_last - i + j];
			dst_bottom[row_last - i + j] = src_top[i + j];
		}
	}

	if (width % 2) {
		for (uint32_t j = 0; j < rgbBytes; ++j) {
			dst_top[row_half + j] = src_bottom[row_half + j];
			dst_bottom[row_half + j] = src_top[row_half + j];
		}
	}
}

#endif

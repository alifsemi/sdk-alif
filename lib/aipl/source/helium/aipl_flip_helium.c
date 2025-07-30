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
 * @file    aipl_flip_helium.c
 * @brief   Helium accelerated flip function implementations
 *
 ******************************************************************************/

/*********************
 *      INCLUDES
 *********************/
#include "aipl_flip_helium.h"

#include <string.h>
#include <stddef.h>

#include "aipl_arm_mve.h"
#include "aipl_cache.h"

#ifdef AIPL_HELIUM_ACCELERATION

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
aipl_error_t aipl_flip_helium(const void *input, void *output, uint32_t pitch, uint32_t width,
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

aipl_error_t aipl_flip_img_helium(const aipl_image_t *input, aipl_image_t *output,
				  bool flip_horizontal, bool flip_vertical)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	if (output->format != input->format) {
		return AIPL_ERR_FORMAT_MISMATCH;
	}

	return aipl_flip_helium(input->data, output->data, input->pitch, input->width,
				input->height, input->format, flip_horizontal, flip_vertical);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static INLINE void aipl_flip_row_hor(const uint8_t *src, uint8_t *dst, uint32_t width, int rgbBytes)
{
	uint32_t row_last = (width - 16) * rgbBytes;
	uint32_t row_half = (width - width / 2) * rgbBytes;

	for (uint32_t i = 0; i < row_half; i += 16 * rgbBytes) {
		mve_pred16_t tail_p = vctp8q((row_half - i) / rgbBytes);

		uint8x16_t frw_off = vidupq_n_u8(0, 1);

		frw_off = vmulq_n_u8(frw_off, rgbBytes);

#if defined(__ARMCC_VERSION) || (GCC_VERSION >= 120300)
		uint8x16_t rvr_off = vcreateq_u8(0x08090a0b0c0d0e0f, 0x0001020304050607);
#else
		uint8x16_t rvr_off = vcreateq_u8(0x0001020304050607, 0x08090a0b0c0d0e0f);
#endif
		rvr_off = vmulq_n_u8(rvr_off, rgbBytes);

		for (uint32_t j = 0; j < rgbBytes; ++j) {
			uint8x16_t src_left = vldrbq_gather_offset_z(src + i + j, frw_off, tail_p);
			uint8x16_t src_right =
				vldrbq_gather_offset_z(src + row_last - i + j, rvr_off, tail_p);

			vstrbq_scatter_offset_p(dst + i + j, frw_off, src_right, tail_p);
			vstrbq_scatter_offset_p(dst + row_last - i + j, rvr_off, src_left, tail_p);
		}
	}
}

static INLINE void aipl_flip_rows_hor_ver(const uint8_t *src_top, const uint8_t *src_bottom,
					  uint8_t *dst_top, uint8_t *dst_bottom, uint32_t width,
					  int rgbBytes)
{
	uint32_t row_last = (width - 16) * rgbBytes;
	uint32_t row_half = (width - width / 2) * rgbBytes;

	for (uint32_t i = 0; i < row_half; i += 16 * rgbBytes) {
		mve_pred16_t tail_p = vctp8q((row_half - i) / rgbBytes);

		uint8x16_t frw_off = vidupq_n_u8(0, 1);

		frw_off = vmulq_n_u8(frw_off, rgbBytes);

#if defined(__ARMCC_VERSION) || (GCC_VERSION >= 120300)
		uint8x16_t rvr_off = vcreateq_u8(0x08090a0b0c0d0e0f, 0x0001020304050607);
#else
		uint8x16_t rvr_off = vcreateq_u8(0x0001020304050607, 0x08090a0b0c0d0e0f);
#endif
		rvr_off = vmulq_n_u8(rvr_off, rgbBytes);

		for (uint32_t j = 0; j < rgbBytes; ++j) {
			uint8x16_t src_top_left =
				vldrbq_gather_offset_z(src_top + i + j, frw_off, tail_p);
			uint8x16_t src_top_right =
				vldrbq_gather_offset_z(src_top + row_last - i + j, rvr_off, tail_p);
			uint8x16_t src_bottom_left =
				vldrbq_gather_offset_z(src_bottom + i + j, frw_off, tail_p);
			uint8x16_t src_bottom_right = vldrbq_gather_offset_z(
				src_bottom + row_last - i + j, rvr_off, tail_p);

			vstrbq_scatter_offset_p(dst_top + i + j, frw_off, src_bottom_right, tail_p);
			vstrbq_scatter_offset_p(dst_top + row_last - i + j, rvr_off,
						src_bottom_left, tail_p);
			vstrbq_scatter_offset_p(dst_bottom + i + j, frw_off, src_top_right, tail_p);
			vstrbq_scatter_offset_p(dst_bottom + row_last - i + j, rvr_off,
						src_top_left, tail_p);
		}
	}
}

#endif

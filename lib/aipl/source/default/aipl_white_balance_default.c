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
 * @file    aipl_white_balance_default.c
 * @brief   Default white balance function implementations
 *
 ******************************************************************************/

/*********************
 *      INCLUDES
 *********************/
#include "aipl_white_balance_default.h"

#include <stddef.h>

#include "aipl_utils.h"

#if !defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT)

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
aipl_error_t aipl_white_balance_24bit_default(const void *input, void *output, uint32_t pitch,
					      uint32_t width, uint32_t height, float ar, float ag,
					      float ab, uint8_t r_offset, uint8_t g_offset,
					      uint8_t b_offset);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
aipl_error_t aipl_white_balance_rgb_default(const void *input, void *output, uint32_t pitch,
					    uint32_t width, uint32_t height,
					    aipl_color_format_t format, float ar, float ag,
					    float ab)
{
	switch (format) {
	case AIPL_COLOR_ARGB8888:
		return aipl_white_balance_argb8888_default(input, output, pitch, width, height, ar,
							   ag, ab);
	case AIPL_COLOR_RGBA8888:
		return aipl_white_balance_rgba8888_default(input, output, pitch, width, height, ar,
							   ag, ab);
	case AIPL_COLOR_ARGB4444:
		return aipl_white_balance_argb4444_default(input, output, pitch, width, height, ar,
							   ag, ab);
	case AIPL_COLOR_ARGB1555:
		return aipl_white_balance_argb1555_default(input, output, pitch, width, height, ar,
							   ag, ab);
	case AIPL_COLOR_RGBA4444:
		return aipl_white_balance_rgba4444_default(input, output, pitch, width, height, ar,
							   ag, ab);
	case AIPL_COLOR_RGBA5551:
		return aipl_white_balance_rgba5551_default(input, output, pitch, width, height, ar,
							   ag, ab);
	case AIPL_COLOR_BGR888:
		return aipl_white_balance_bgr888_default(input, output, pitch, width, height, ar,
							 ag, ab);
	case AIPL_COLOR_RGB888:
		return aipl_white_balance_rgb888_default(input, output, pitch, width, height, ar,
							 ag, ab);
	case AIPL_COLOR_RGB565:
		return aipl_white_balance_rgb565_default(input, output, pitch, width, height, ar,
							 ag, ab);

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}

	return AIPL_ERR_OK;
}

aipl_error_t aipl_white_balance_rgb_img_default(const aipl_image_t *input, aipl_image_t *output,
						float ar, float ag, float ab)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	if (input->width != output->width || input->height != output->height) {
		return AIPL_ERR_SIZE_MISMATCH;
	}

	if (input->format != output->format) {
		return AIPL_ERR_FORMAT_MISMATCH;
	}

	return aipl_white_balance_rgb_default(input->data, output->data, input->pitch, input->width,
					      input->height, input->format, ar, ag, ab);

	return AIPL_ERR_OK;
}

aipl_error_t aipl_white_balance_argb8888_default(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height, float ar,
						 float ag, float ab)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_argb8888_px_t *src_ptr = input;
	aipl_argb8888_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_argb8888_px_t *src = src_ptr + (i * pitch);
		aipl_argb8888_px_t *dst = dst_ptr + (i * width);

		for (uint32_t j = 0; j < width; ++j) {
			int16_t r = src->r * ar;
			int16_t g = src->g * ag;
			int16_t b = src->b * ab;

			dst->a = src->a;
			dst->r = aipl_channel_cap(r);
			dst->g = aipl_channel_cap(g);
			dst->b = aipl_channel_cap(b);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}

aipl_error_t aipl_white_balance_argb4444_default(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height, float ar,
						 float ag, float ab)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_argb4444_px_t *src_ptr = input;
	aipl_argb4444_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_argb4444_px_t *src = src_ptr + (i * pitch);
		aipl_argb4444_px_t *dst = dst_ptr + (i * width);

		for (uint32_t j = 0; j < width; ++j) {
			aipl_argb8888_px_t px;

			aipl_cnvt_px_argb4444_to_argb8888(&px, src);

			int16_t r = px.r * ar;
			int16_t g = px.g * ag;
			int16_t b = px.b * ab;

			px.r = aipl_channel_cap(r);
			px.g = aipl_channel_cap(g);
			px.b = aipl_channel_cap(b);

			aipl_cnvt_px_argb8888_to_argb4444(dst, &px);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}

aipl_error_t aipl_white_balance_argb1555_default(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height, float ar,
						 float ag, float ab)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_argb1555_px_t *src_ptr = input;
	aipl_argb1555_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_argb1555_px_t *src = src_ptr + (i * pitch);
		aipl_argb1555_px_t *dst = dst_ptr + (i * width);

		for (uint32_t j = 0; j < width; ++j) {
			aipl_argb8888_px_t px;

			aipl_cnvt_px_argb1555_to_argb8888(&px, src);

			int16_t r = px.r * ar;
			int16_t g = px.g * ag;
			int16_t b = px.b * ab;

			px.r = aipl_channel_cap(r);
			px.g = aipl_channel_cap(g);
			px.b = aipl_channel_cap(b);

			aipl_cnvt_px_argb8888_to_argb1555(dst, &px);

			++src;
			++dst;
		}
	}
	return AIPL_ERR_OK;
}

aipl_error_t aipl_white_balance_rgba8888_default(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height, float ar,
						 float ag, float ab)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgba8888_px_t *src_ptr = input;
	aipl_rgba8888_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgba8888_px_t *src = src_ptr + (i * pitch);
		aipl_rgba8888_px_t *dst = dst_ptr + (i * width);

		for (uint32_t j = 0; j < width; ++j) {
			int16_t r = src->r * ar;
			int16_t g = src->g * ag;
			int16_t b = src->b * ab;

			dst->r = aipl_channel_cap(r);
			dst->g = aipl_channel_cap(g);
			dst->b = aipl_channel_cap(b);
			dst->a = src->a;

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}

aipl_error_t aipl_white_balance_rgba4444_default(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height, float ar,
						 float ag, float ab)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgba4444_px_t *src_ptr = input;
	aipl_rgba4444_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgba4444_px_t *src = src_ptr + (i * pitch);
		aipl_rgba4444_px_t *dst = dst_ptr + (i * width);

		for (uint32_t j = 0; j < width; ++j) {
			aipl_argb8888_px_t px;

			aipl_cnvt_px_rgba4444_to_argb8888(&px, src);

			int16_t r = px.r * ar;
			int16_t g = px.g * ag;
			int16_t b = px.b * ab;

			px.r = aipl_channel_cap(r);
			px.g = aipl_channel_cap(g);
			px.b = aipl_channel_cap(b);

			aipl_cnvt_px_argb8888_to_rgba4444(dst, &px);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}

aipl_error_t aipl_white_balance_rgba5551_default(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height, float ar,
						 float ag, float ab)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgba5551_px_t *src_ptr = input;
	aipl_rgba5551_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgba5551_px_t *src = src_ptr + (i * pitch);
		aipl_rgba5551_px_t *dst = dst_ptr + (i * width);

		for (uint32_t j = 0; j < width; ++j) {
			aipl_argb8888_px_t px;

			aipl_cnvt_px_rgba5551_to_argb8888(&px, src);

			int16_t r = px.r * ar;
			int16_t g = px.g * ag;
			int16_t b = px.b * ab;

			px.r = aipl_channel_cap(r);
			px.g = aipl_channel_cap(g);
			px.b = aipl_channel_cap(b);

			aipl_cnvt_px_argb8888_to_rgba5551(dst, &px);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}

aipl_error_t aipl_white_balance_bgr888_default(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height, float ar, float ag,
					       float ab)
{
	return aipl_white_balance_24bit_default(input, output, pitch, width, height, ar, ag, ab, 2,
						1, 0);
}

aipl_error_t aipl_white_balance_rgb888_default(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height, float ar, float ag,
					       float ab)
{
	return aipl_white_balance_24bit_default(input, output, pitch, width, height, ar, ag, ab, 0,
						1, 2);
}

aipl_error_t aipl_white_balance_rgb565_default(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height, float ar, float ag,
					       float ab)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const aipl_rgb565_px_t *src_ptr = input;
	aipl_rgb565_px_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const aipl_rgb565_px_t *src = src_ptr + (i * pitch);
		aipl_rgb565_px_t *dst = dst_ptr + (i * width);

		for (uint32_t j = 0; j < width; ++j) {
			uint8_t px[3];

			aipl_cnvt_px_rgb565_to_24bit(px, src, 2, 1, 0);

			int16_t r = px[2] * ar;
			int16_t g = px[1] * ag;
			int16_t b = px[0] * ab;

			px[2] = aipl_channel_cap(r);
			px[1] = aipl_channel_cap(g);
			px[0] = aipl_channel_cap(b);

			aipl_cnvt_px_24bit_to_rgb565(dst, px, 2, 1, 0);

			++src;
			++dst;
		}
	}

	return AIPL_ERR_OK;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
aipl_error_t aipl_white_balance_24bit_default(const void *input, void *output, uint32_t pitch,
					      uint32_t width, uint32_t height, float ar, float ag,
					      float ab, uint8_t r_offset, uint8_t g_offset,
					      uint8_t b_offset)
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
			int16_t r = src[r_offset] * ar;
			int16_t g = src[g_offset] * ag;
			int16_t b = src[b_offset] * ab;

			dst[r_offset] = aipl_channel_cap(r);
			dst[g_offset] = aipl_channel_cap(g);
			dst[b_offset] = aipl_channel_cap(b);

			src += 3;
			dst += 3;
		}
	}

	return AIPL_ERR_OK;
}

#endif

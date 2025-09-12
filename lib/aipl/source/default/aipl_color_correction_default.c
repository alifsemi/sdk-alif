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
 * @file    aipl_color_correction_default.c
 * @brief   Default color correction function implementations
 *
 ******************************************************************************/

/*********************
 *      INCLUDES
 *********************/
#include "aipl_color_correction_default.h"

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
aipl_error_t aipl_color_correction_24bit_default(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height, const float *ccm,
						 uint8_t r_offset, uint8_t g_offset,
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
aipl_error_t aipl_color_correction_rgb_default(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height,
					       aipl_color_format_t format, const float *ccm)
{
	switch (format) {
	case AIPL_COLOR_ARGB8888:
		return aipl_color_correction_argb8888_default(input, output, pitch, width, height,
							      ccm);
	case AIPL_COLOR_RGBA8888:
		return aipl_color_correction_rgba8888_default(input, output, pitch, width, height,
							      ccm);
	case AIPL_COLOR_ARGB4444:
		return aipl_color_correction_argb4444_default(input, output, pitch, width, height,
							      ccm);
	case AIPL_COLOR_ARGB1555:
		return aipl_color_correction_argb1555_default(input, output, pitch, width, height,
							      ccm);
	case AIPL_COLOR_RGBA4444:
		return aipl_color_correction_rgba4444_default(input, output, pitch, width, height,
							      ccm);
	case AIPL_COLOR_RGBA5551:
		return aipl_color_correction_rgba5551_default(input, output, pitch, width, height,
							      ccm);
	case AIPL_COLOR_BGR888:
		return aipl_color_correction_bgr888_default(input, output, pitch, width, height,
							    ccm);
	case AIPL_COLOR_RGB888:
		return aipl_color_correction_rgb888_default(input, output, pitch, width, height,
							    ccm);
	case AIPL_COLOR_RGB565:
		return aipl_color_correction_rgb565_default(input, output, pitch, width, height,
							    ccm);

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

aipl_error_t aipl_color_correction_rgb_img_default(const aipl_image_t *input, aipl_image_t *output,
						   const float *ccm)
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

	return aipl_color_correction_rgb_default(input->data, output->data, input->pitch,
						 input->width, input->height, input->format, ccm);
}

aipl_error_t aipl_color_correction_argb8888_default(const void *input, void *output, uint32_t pitch,
						    uint32_t width, uint32_t height,
						    const float *ccm)
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
			int16_t r = src->r * ccm[0] + src->g * ccm[1] + src->b * ccm[2];
			int16_t g = src->r * ccm[3] + src->g * ccm[4] + src->b * ccm[5];
			int16_t b = src->r * ccm[6] + src->g * ccm[7] + src->b * ccm[8];

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

aipl_error_t aipl_color_correction_argb4444_default(const void *input, void *output, uint32_t pitch,
						    uint32_t width, uint32_t height,
						    const float *ccm)
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

			int16_t r = px.r * ccm[0] + px.g * ccm[1] + px.b * ccm[2];
			int16_t g = px.r * ccm[3] + px.g * ccm[4] + px.b * ccm[5];
			int16_t b = px.r * ccm[6] + px.g * ccm[7] + px.b * ccm[8];

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

aipl_error_t aipl_color_correction_argb1555_default(const void *input, void *output, uint32_t pitch,
						    uint32_t width, uint32_t height,
						    const float *ccm)
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

			int16_t r = px.r * ccm[0] + px.g * ccm[1] + px.b * ccm[2];
			int16_t g = px.r * ccm[3] + px.g * ccm[4] + px.b * ccm[5];
			int16_t b = px.r * ccm[6] + px.g * ccm[7] + px.b * ccm[8];

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

aipl_error_t aipl_color_correction_rgba8888_default(const void *input, void *output, uint32_t pitch,
						    uint32_t width, uint32_t height,
						    const float *ccm)
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
			int16_t r = src->r * ccm[0] + src->g * ccm[1] + src->b * ccm[2];
			int16_t g = src->r * ccm[3] + src->g * ccm[4] + src->b * ccm[5];
			int16_t b = src->r * ccm[6] + src->g * ccm[7] + src->b * ccm[8];

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

aipl_error_t aipl_color_correction_rgba4444_default(const void *input, void *output, uint32_t pitch,
						    uint32_t width, uint32_t height,
						    const float *ccm)
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

			int16_t r = px.r * ccm[0] + px.g * ccm[1] + px.b * ccm[2];
			int16_t g = px.r * ccm[3] + px.g * ccm[4] + px.b * ccm[5];
			int16_t b = px.r * ccm[6] + px.g * ccm[7] + px.b * ccm[8];

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

aipl_error_t aipl_color_correction_rgba5551_default(const void *input, void *output, uint32_t pitch,
						    uint32_t width, uint32_t height,
						    const float *ccm)
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

			int16_t r = px.r * ccm[0] + px.g * ccm[1] + px.b * ccm[2];
			int16_t g = px.r * ccm[3] + px.g * ccm[4] + px.b * ccm[5];
			int16_t b = px.r * ccm[6] + px.g * ccm[7] + px.b * ccm[8];

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

aipl_error_t aipl_color_correction_bgr888_default(const void *input, void *output, uint32_t pitch,
						  uint32_t width, uint32_t height, const float *ccm)
{
	return aipl_color_correction_24bit_default(input, output, pitch, width, height, ccm, 2, 1,
						   0);
}

aipl_error_t aipl_color_correction_rgb888_default(const void *input, void *output, uint32_t pitch,
						  uint32_t width, uint32_t height, const float *ccm)
{
	return aipl_color_correction_24bit_default(input, output, pitch, width, height, ccm, 0, 1,
						   2);
}

aipl_error_t aipl_color_correction_rgb565_default(const void *input, void *output, uint32_t pitch,
						  uint32_t width, uint32_t height, const float *ccm)
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

			int16_t r = px[2] * ccm[0] + px[1] * ccm[1] + px[0] * ccm[2];
			int16_t g = px[2] * ccm[3] + px[1] * ccm[4] + px[0] * ccm[5];
			int16_t b = px[2] * ccm[6] + px[1] * ccm[7] + px[0] * ccm[8];

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
aipl_error_t aipl_color_correction_24bit_default(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height, const float *ccm,
						 uint8_t r_offset, uint8_t g_offset,
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
			int16_t r = src[r_offset] * ccm[0] + src[g_offset] * ccm[1] +
				    src[b_offset] * ccm[2];
			int16_t g = src[r_offset] * ccm[3] + src[g_offset] * ccm[4] +
				    src[b_offset] * ccm[5];
			int16_t b = src[r_offset] * ccm[6] + src[g_offset] * ccm[7] +
				    src[b_offset] * ccm[8];

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

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
 * @file    aipl_lut_transform_helium.c
 * @brief   Helium accelerated LUT transformation function implementations
 *
 ******************************************************************************/

/*********************
 *      INCLUDES
 *********************/
#include "aipl_lut_transform_helium.h"

#include <stddef.h>

#include "aipl_mve_utils.h"

#ifdef AIPL_HELIUM_ACCELERATION

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
aipl_error_t aipl_lut_transform_24bit_helium(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height, uint8_t *lut,
					     uint8_t r_offset, uint8_t g_offset, uint8_t b_offset);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
aipl_error_t aipl_lut_transform_rgb_helium(const void *input, void *output, uint32_t pitch,
					   uint32_t width, uint32_t height,
					   aipl_color_format_t format, uint8_t *lut)
{
	switch (format) {
	case AIPL_COLOR_ARGB8888:
		return aipl_lut_transform_argb8888_helium(input, output, pitch, width, height, lut);
	case AIPL_COLOR_RGBA8888:
		return aipl_lut_transform_rgba8888_helium(input, output, pitch, width, height, lut);
	case AIPL_COLOR_ARGB4444:
		return aipl_lut_transform_argb4444_helium(input, output, pitch, width, height, lut);
	case AIPL_COLOR_ARGB1555:
		return aipl_lut_transform_argb1555_helium(input, output, pitch, width, height, lut);
	case AIPL_COLOR_RGBA4444:
		return aipl_lut_transform_rgba4444_helium(input, output, pitch, width, height, lut);
	case AIPL_COLOR_RGBA5551:
		return aipl_lut_transform_rgba5551_helium(input, output, pitch, width, height, lut);
	case AIPL_COLOR_BGR888:
		return aipl_lut_transform_bgr888_helium(input, output, pitch, width, height, lut);
	case AIPL_COLOR_RGB888:
		return aipl_lut_transform_rgb888_helium(input, output, pitch, width, height, lut);
	case AIPL_COLOR_RGB565:
		return aipl_lut_transform_rgb565_helium(input, output, pitch, width, height, lut);

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}

	return AIPL_ERR_OK;
}

aipl_error_t aipl_lut_transform_rgb_img_helium(const aipl_image_t *input, aipl_image_t *output,
					       uint8_t *lut)
{
	if (input == NULL || output == NULL || lut == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	if (input->width != output->width || input->height != output->height) {
		return AIPL_ERR_SIZE_MISMATCH;
	}

	if (input->format != output->format) {
		return AIPL_ERR_FORMAT_MISMATCH;
	}

	return aipl_lut_transform_rgb_helium(input->data, output->data, input->pitch, input->width,
					     input->height, input->format, lut);
}

aipl_error_t aipl_lut_transform_argb8888_helium(const void *input, void *output, uint32_t pitch,
						uint32_t width, uint32_t height, uint8_t *lut)
{
	if (input == NULL || output == NULL || lut == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint8_t *src_ptr = input;
	uint8_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *src = src_ptr + (i * pitch) * 4;
		uint8_t *dst = dst_ptr + (i * width) * 4;

		int32_t cnt = width;

		for (; cnt > 16; cnt -= 16) {
			aipl_mve_argb_x16_t pix;

			aipl_mve_ldr_16px_argb8888_uncut(&pix, src);

			aipl_mve_lut_transform_argb_x16(&pix, lut);

			aipl_mve_str_16px_argb8888_uncut(dst, pix);

			src += 64;
			dst += 64;
		}

		if (cnt > 0) {
			mve_pred16_t tail_p = vctp8q(cnt);

			aipl_mve_argb_x16_t pix;

			aipl_mve_ldr_16px_argb8888(&pix, src, tail_p);

			aipl_mve_lut_transform_argb_x16(&pix, lut);

			aipl_mve_str_16px_argb8888(dst, pix, tail_p);
		}
	}

	return AIPL_ERR_OK;
}

aipl_error_t aipl_lut_transform_argb4444_helium(const void *input, void *output, uint32_t pitch,
						uint32_t width, uint32_t height, uint8_t *lut)
{
	if (input == NULL || output == NULL || lut == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint16_t *src_ptr = input;
	uint16_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint16_t *src = src_ptr + (i * pitch);
		uint16_t *dst = dst_ptr + (i * width);

		int32_t cnt = width;

		for (; cnt > 16; cnt -= 16) {
			aipl_mve_argb_x16_t pix;

			aipl_mve_ldr_16px_argb4444_uncut(&pix, src);

			aipl_mve_lut_transform_argb_x16(&pix, lut);

			aipl_mve_str_16px_argb4444_uncut(dst, pix);

			src += 16;
			dst += 16;
		}

		if (cnt > 0) {
			mve_pred16_t tail_p = vctp8q(cnt);

			aipl_mve_argb_x16_t pix;

			aipl_mve_ldr_16px_argb4444(&pix, src, tail_p);

			aipl_mve_lut_transform_argb_x16(&pix, lut);

			aipl_mve_str_16px_argb4444(dst, pix, tail_p);
		}
	}

	return AIPL_ERR_OK;
}

aipl_error_t aipl_lut_transform_argb1555_helium(const void *input, void *output, uint32_t pitch,
						uint32_t width, uint32_t height, uint8_t *lut)
{
	if (input == NULL || output == NULL || lut == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint16_t *src_ptr = input;
	uint16_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint16_t *src = src_ptr + (i * pitch);
		uint16_t *dst = dst_ptr + (i * width);

		int32_t cnt = width;

		for (; cnt > 16; cnt -= 16) {
			aipl_mve_argb_x16_t pix;

			aipl_mve_ldr_16px_argb1555_uncut(&pix, src);

			aipl_mve_lut_transform_argb_x16(&pix, lut);

			aipl_mve_str_16px_argb1555_uncut(dst, pix);

			src += 16;
			dst += 16;
		}

		if (cnt > 0) {
			mve_pred16_t tail_p = vctp8q(cnt);

			aipl_mve_argb_x16_t pix;

			aipl_mve_ldr_16px_argb1555(&pix, src, tail_p);

			aipl_mve_lut_transform_argb_x16(&pix, lut);

			aipl_mve_str_16px_argb1555(dst, pix, tail_p);
		}
	}

	return AIPL_ERR_OK;
}

aipl_error_t aipl_lut_transform_rgba8888_helium(const void *input, void *output, uint32_t pitch,
						uint32_t width, uint32_t height, uint8_t *lut)
{
	if (input == NULL || output == NULL || lut == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint8_t *src_ptr = input;
	uint8_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *src = src_ptr + (i * pitch) * 4;
		uint8_t *dst = dst_ptr + (i * width) * 4;

		int32_t cnt = width;

		for (; cnt > 16; cnt -= 16) {
			aipl_mve_argb_x16_t pix;

			aipl_mve_ldr_16px_rgba8888_uncut(&pix, src);

			aipl_mve_lut_transform_argb_x16(&pix, lut);

			aipl_mve_str_16px_rgba8888_uncut(dst, pix);

			src += 64;
			dst += 64;
		}

		if (cnt > 0) {
			mve_pred16_t tail_p = vctp8q(cnt);

			aipl_mve_argb_x16_t pix;

			aipl_mve_ldr_16px_rgba8888(&pix, src, tail_p);

			aipl_mve_lut_transform_argb_x16(&pix, lut);

			aipl_mve_str_16px_rgba8888(dst, pix, tail_p);
		}
	}

	return AIPL_ERR_OK;
}

aipl_error_t aipl_lut_transform_rgba4444_helium(const void *input, void *output, uint32_t pitch,
						uint32_t width, uint32_t height, uint8_t *lut)
{
	if (input == NULL || output == NULL || lut == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint16_t *src_ptr = input;
	uint16_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint16_t *src = src_ptr + (i * pitch);
		uint16_t *dst = dst_ptr + (i * width);

		int32_t cnt = width;

		for (; cnt > 16; cnt -= 16) {
			aipl_mve_argb_x16_t pix;

			aipl_mve_ldr_16px_rgba4444_uncut(&pix, src);

			aipl_mve_lut_transform_argb_x16(&pix, lut);

			aipl_mve_str_16px_rgba4444_uncut(dst, pix);

			src += 16;
			dst += 16;
		}

		if (cnt > 0) {
			mve_pred16_t tail_p = vctp8q(cnt);

			aipl_mve_argb_x16_t pix;

			aipl_mve_ldr_16px_rgba4444(&pix, src, tail_p);

			aipl_mve_lut_transform_argb_x16(&pix, lut);

			aipl_mve_str_16px_rgba4444(dst, pix, tail_p);
		}
	}

	return AIPL_ERR_OK;
}

aipl_error_t aipl_lut_transform_rgba5551_helium(const void *input, void *output, uint32_t pitch,
						uint32_t width, uint32_t height, uint8_t *lut)
{
	if (input == NULL || output == NULL || lut == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint16_t *src_ptr = input;
	uint16_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint16_t *src = src_ptr + (i * pitch);
		uint16_t *dst = dst_ptr + (i * width);

		int32_t cnt = width;

		for (; cnt > 16; cnt -= 16) {
			aipl_mve_argb_x16_t pix;

			aipl_mve_ldr_16px_rgba5551_uncut(&pix, src);

			aipl_mve_lut_transform_argb_x16(&pix, lut);

			aipl_mve_str_16px_rgba5551_uncut(dst, pix);

			src += 16;
			dst += 16;
		}

		if (cnt > 0) {
			mve_pred16_t tail_p = vctp8q(cnt);

			aipl_mve_argb_x16_t pix;

			aipl_mve_ldr_16px_rgba5551(&pix, src, tail_p);

			aipl_mve_lut_transform_argb_x16(&pix, lut);

			aipl_mve_str_16px_rgba5551(dst, pix, tail_p);
		}
	}

	return AIPL_ERR_OK;
}

aipl_error_t aipl_lut_transform_bgr888_helium(const void *input, void *output, uint32_t pitch,
					      uint32_t width, uint32_t height, uint8_t *lut)
{
	return aipl_lut_transform_24bit_helium(input, output, pitch, width, height, lut, 2, 1, 0);
}

aipl_error_t aipl_lut_transform_rgb888_helium(const void *input, void *output, uint32_t pitch,
					      uint32_t width, uint32_t height, uint8_t *lut)
{
	return aipl_lut_transform_24bit_helium(input, output, pitch, width, height, lut, 0, 1, 2);
}

aipl_error_t aipl_lut_transform_rgb565_helium(const void *input, void *output, uint32_t pitch,
					      uint32_t width, uint32_t height, uint8_t *lut)
{
	if (input == NULL || output == NULL || lut == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint16_t *src_ptr = input;
	uint16_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint16_t *src = src_ptr + (i * pitch);
		uint16_t *dst = dst_ptr + (i * width);

		int32_t cnt = width;

		for (; cnt > 16; cnt -= 16) {
			aipl_mve_rgb_x16_t pix;

			aipl_mve_ldr_16px_rgb565_uncut(&pix, src);

			aipl_mve_lut_transform_rgb_x16(&pix, lut);

			aipl_mve_str_16px_rgb565_uncut(dst, pix);

			src += 16;
			dst += 16;
		}

		if (cnt > 0) {
			mve_pred16_t tail_p = vctp8q(cnt);

			aipl_mve_rgb_x16_t pix;

			aipl_mve_ldr_16px_rgb565(&pix, src, tail_p);

			aipl_mve_lut_transform_rgb_x16(&pix, lut);

			aipl_mve_str_16px_rgb565(dst, pix, tail_p);
		}
	}

	return AIPL_ERR_OK;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
aipl_error_t aipl_lut_transform_24bit_helium(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height, uint8_t *lut,
					     uint8_t r_offset, uint8_t g_offset, uint8_t b_offset)
{
	if (input == NULL || output == NULL || lut == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint8_t *src_ptr = input;
	uint8_t *dst_ptr = output;

	for (uint32_t i = 0; i < height; ++i) {
		const uint8_t *src = src_ptr + (i * pitch) * 3;
		uint8_t *dst = dst_ptr + (i * width) * 3;

		for (int32_t cnt = width; cnt > 0; cnt -= 16) {
			mve_pred16_t tail_p = vctp8q(cnt);

			aipl_mve_rgb_x16_t pix;

			aipl_mve_ldr_16px_rgb(&pix, src, tail_p, r_offset, g_offset, b_offset);

			aipl_mve_lut_transform_rgb_x16(&pix, lut);

			aipl_mve_str_16px_rgb(dst, pix, tail_p, r_offset, g_offset, b_offset);

			src += 48;
			dst += 48;
		}
	}

	return AIPL_ERR_OK;
}

#endif

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
 * @file    aipl_bayer_helium.c
 * @brief   Bayer pattern decoding function implementations using Helium
 *          acceleration
 *
 ******************************************************************************/

/*********************
 *      INCLUDES
 *********************/
#include "aipl_bayer_helium.h"

#include <stddef.h>
#include <string.h>

#include "aipl_mve_utils.h"

/*********************
 *      DEFINES
 *********************/

#ifdef AIPL_HELIUM_ACCELERATION

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static INLINE void aipl_bayer_to_rgb_x8(aipl_mve_rgb_x8_t *rgb0, aipl_mve_rgb_x8_t *rgb1,
					aipl_bayer_tile_t *tile, uint32_t idx, mve_pred16_t pred);
static INLINE void aipl_bayer_to_rgb_x16(aipl_mve_rgb_x16_t *rgb, aipl_bayer_tile_t *tile,
					 uint32_t idx, mve_pred16_t pred);
static aipl_error_t aipl_bayer_decoding_yuv_planar(const void *input, uint8_t *y, uint8_t *u,
						   uint8_t *v, uint32_t pitch, uint32_t width,
						   uint32_t height, aipl_bayer_filter_t filter);
static aipl_error_t aipl_bayer_decoding_yuv_semi_planar(const void *input, uint8_t *y, uint8_t *u,
							uint8_t *v, uint32_t pitch, uint32_t width,
							uint32_t height,
							aipl_bayer_filter_t filter);
static aipl_error_t aipl_bayer_decoding_yuv_packed(const void *input, uint8_t *y, uint8_t *u,
						   uint8_t *v, uint32_t pitch, uint32_t width,
						   uint32_t height, aipl_bayer_filter_t filter);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

aipl_error_t aipl_bayer_decoding_helium(const void *input, void *output, uint32_t pitch,
					uint32_t width, uint32_t height, aipl_bayer_filter_t filter,
					aipl_color_format_t format)
{
	switch (format) {
	case AIPL_COLOR_ALPHA8:
		return aipl_bayer_decoding_alpha8_helium(input, output, pitch, width, height,
							 filter);
	case AIPL_COLOR_ARGB8888:
		return aipl_bayer_decoding_argb8888_helium(input, output, pitch, width, height,
							   filter);
	case AIPL_COLOR_ARGB4444:
		return aipl_bayer_decoding_argb4444_helium(input, output, pitch, width, height,
							   filter);
	case AIPL_COLOR_ARGB1555:
		return aipl_bayer_decoding_argb1555_helium(input, output, pitch, width, height,
							   filter);
	case AIPL_COLOR_RGBA8888:
		return aipl_bayer_decoding_rgba8888_helium(input, output, pitch, width, height,
							   filter);
	case AIPL_COLOR_RGBA4444:
		return aipl_bayer_decoding_rgba4444_helium(input, output, pitch, width, height,
							   filter);
	case AIPL_COLOR_RGBA5551:
		return aipl_bayer_decoding_rgba5551_helium(input, output, pitch, width, height,
							   filter);
	case AIPL_COLOR_RGB888:
		return aipl_bayer_decoding_rgb888_helium(input, output, pitch, width, height,
							 filter);
	case AIPL_COLOR_BGR888:
		return aipl_bayer_decoding_bgr888_helium(input, output, pitch, width, height,
							 filter);
	case AIPL_COLOR_RGB565:
		return aipl_bayer_decoding_rgb565_helium(input, output, pitch, width, height,
							 filter);
	case AIPL_COLOR_YV12:
		return aipl_bayer_decoding_yv12_helium(input, output, pitch, width, height, filter);
	case AIPL_COLOR_I420:
		return aipl_bayer_decoding_i420_helium(input, output, pitch, width, height, filter);
	case AIPL_COLOR_I422:
		return aipl_bayer_decoding_i422_helium(input, output, pitch, width, height, filter);
	case AIPL_COLOR_I444:
		return aipl_bayer_decoding_i444_helium(input, output, pitch, width, height, filter);
	case AIPL_COLOR_I400:
		return aipl_bayer_decoding_i400_helium(input, output, pitch, width, height, filter);
	case AIPL_COLOR_NV12:
		return aipl_bayer_decoding_nv12_helium(input, output, pitch, width, height, filter);
	case AIPL_COLOR_NV21:
		return aipl_bayer_decoding_nv21_helium(input, output, pitch, width, height, filter);
	case AIPL_COLOR_YUY2:
		return aipl_bayer_decoding_yuy2_helium(input, output, pitch, width, height, filter);
	case AIPL_COLOR_UYVY:
		return aipl_bayer_decoding_uyvy_helium(input, output, pitch, width, height, filter);

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

aipl_error_t aipl_bayer_decoding_img_helium(const void *input, aipl_image_t *output, uint32_t pitch,
					    aipl_bayer_filter_t filter)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	return aipl_bayer_decoding_helium(input, output->data, pitch, output->width, output->height,
					  filter, output->format);
}

aipl_error_t aipl_bayer_decoding_alpha8_helium(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height,
					       aipl_bayer_filter_t filter)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	aipl_bayer_tile_t tile = aipl_bayer_tile(input, pitch, filter);

	for (uint32_t i = 0; i < height - 1; ++i) {
		uint8_t *dst = (uint8_t *)output + i * width;

		int32_t cnt = width / 2;
		uint32_t j = 0;

		for (; cnt > 0; cnt -= 8, j += 16) {
			mve_pred16_t tail_p = vctp16q(cnt);

			aipl_mve_rgb_x8_t rgb0;
			aipl_mve_rgb_x8_t rgb1;

			aipl_bayer_to_rgb_x8(&rgb0, &rgb1, &tile, j, tail_p);

			uint16x8_t alpha0;
			uint16x8_t alpha1;

			aipl_mve_cnvt_8px_xrgb8888_to_yuv_y(&alpha0, rgb0);
			aipl_mve_cnvt_8px_xrgb8888_to_yuv_y(&alpha1, rgb1);

			uint8x16_t alpha = vmovntq(vreinterpretq_u8(alpha0), alpha1);

			vstrbq_p(dst + j, alpha, tail_p);
		}

		tile.red_src += (i + tile.blue_row) % 2 * pitch * 2;
		tile.blue_src += (i + tile.red_row) % 2 * pitch * 2;
	}

	memcpy((uint8_t *)output + (height - 1) * width, (uint8_t *)output + (height - 2) * width,
	       width);

	return AIPL_ERR_OK;
}

aipl_error_t aipl_bayer_decoding_argb8888_helium(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height,
						 aipl_bayer_filter_t filter)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	aipl_bayer_tile_t tile = aipl_bayer_tile(input, pitch, filter);

	for (uint32_t i = 0; i < height - 1; ++i) {
		uint8_t *dst = (uint8_t *)output + i * width * 4;

		int32_t cnt = width / 2;
		uint32_t j = 0;

		for (; cnt > 0; cnt -= 8, j += 16) {
			mve_pred16_t tail_p = vctp16q(cnt);

			aipl_mve_rgb_x16_t rgb;

			aipl_bayer_to_rgb_x16(&rgb, &tile, j, tail_p);

			aipl_mve_str_16px_xrgb8888(dst + j * 4, rgb, tail_p);
		}

		tile.red_src += (i + tile.blue_row) % 2 * pitch * 2;
		tile.blue_src += (i + tile.red_row) % 2 * pitch * 2;
	}

	memcpy((uint8_t *)output + (height - 1) * width * 4,
	       (uint8_t *)output + (height - 2) * width * 4, width * 4);

	return AIPL_ERR_OK;
}

aipl_error_t aipl_bayer_decoding_argb4444_helium(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height,
						 aipl_bayer_filter_t filter)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	aipl_bayer_tile_t tile = aipl_bayer_tile(input, pitch, filter);

	for (uint32_t i = 0; i < height - 1; ++i) {
		uint16_t *dst = (uint16_t *)output + i * width;

		int32_t cnt = width / 2;
		uint32_t j = 0;

		for (; cnt > 0; cnt -= 8, j += 16) {
			mve_pred16_t tail_p = vctp16q(cnt);

			aipl_mve_rgb_x16_t rgb;

			aipl_bayer_to_rgb_x16(&rgb, &tile, j, tail_p);

			aipl_mve_str_16px_xrgb4444(dst + j, rgb, tail_p);
		}

		tile.red_src += (i + tile.blue_row) % 2 * pitch * 2;
		tile.blue_src += (i + tile.red_row) % 2 * pitch * 2;
	}

	memcpy((uint8_t *)output + (height - 1) * width * 2,
	       (uint8_t *)output + (height - 2) * width * 2, width * 2);

	return AIPL_ERR_OK;
}

aipl_error_t aipl_bayer_decoding_argb1555_helium(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height,
						 aipl_bayer_filter_t filter)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	aipl_bayer_tile_t tile = aipl_bayer_tile(input, pitch, filter);

	for (uint32_t i = 0; i < height - 1; ++i) {
		uint16_t *dst = (uint16_t *)output + i * width;

		int32_t cnt = width / 2;
		uint32_t j = 0;

		for (; cnt > 0; cnt -= 8, j += 16) {
			mve_pred16_t tail_p = vctp16q(cnt);

			aipl_mve_rgb_x16_t rgb;

			aipl_bayer_to_rgb_x16(&rgb, &tile, j, tail_p);

			aipl_mve_str_16px_xrgb1555(dst + j, rgb, tail_p);
		}

		tile.red_src += (i + tile.blue_row) % 2 * pitch * 2;
		tile.blue_src += (i + tile.red_row) % 2 * pitch * 2;
	}

	memcpy((uint8_t *)output + (height - 1) * width * 2,
	       (uint8_t *)output + (height - 2) * width * 2, width * 2);

	return AIPL_ERR_OK;
}

aipl_error_t aipl_bayer_decoding_rgba8888_helium(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height,
						 aipl_bayer_filter_t filter)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	aipl_bayer_tile_t tile = aipl_bayer_tile(input, pitch, filter);

	for (uint32_t i = 0; i < height - 1; ++i) {
		uint8_t *dst = (uint8_t *)output + i * width * 4;

		int32_t cnt = width / 2;
		uint32_t j = 0;

		for (; cnt > 0; cnt -= 8, j += 16) {
			mve_pred16_t tail_p = vctp16q(cnt);

			aipl_mve_rgb_x16_t rgb;

			aipl_bayer_to_rgb_x16(&rgb, &tile, j, tail_p);

			aipl_mve_str_16px_rgbx8888(dst + j * 4, rgb, tail_p);
		}

		tile.red_src += (i + tile.blue_row) % 2 * pitch * 2;
		tile.blue_src += (i + tile.red_row) % 2 * pitch * 2;
	}

	memcpy((uint8_t *)output + (height - 1) * width * 4,
	       (uint8_t *)output + (height - 2) * width * 4, width * 4);

	return AIPL_ERR_OK;
}

aipl_error_t aipl_bayer_decoding_rgba4444_helium(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height,
						 aipl_bayer_filter_t filter)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	aipl_bayer_tile_t tile = aipl_bayer_tile(input, pitch, filter);

	for (uint32_t i = 0; i < height - 1; ++i) {
		uint16_t *dst = (uint16_t *)output + i * width;

		int32_t cnt = width / 2;
		uint32_t j = 0;

		for (; cnt > 0; cnt -= 8, j += 16) {
			mve_pred16_t tail_p = vctp16q(cnt);

			aipl_mve_rgb_x16_t rgb;

			aipl_bayer_to_rgb_x16(&rgb, &tile, j, tail_p);

			aipl_mve_str_16px_rgbx4444(dst + j, rgb, tail_p);
		}

		tile.red_src += (i + tile.blue_row) % 2 * pitch * 2;
		tile.blue_src += (i + tile.red_row) % 2 * pitch * 2;
	}

	memcpy((uint8_t *)output + (height - 1) * width * 2,
	       (uint8_t *)output + (height - 2) * width * 2, width * 2);

	return AIPL_ERR_OK;
}

aipl_error_t aipl_bayer_decoding_rgba5551_helium(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height,
						 aipl_bayer_filter_t filter)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	aipl_bayer_tile_t tile = aipl_bayer_tile(input, pitch, filter);

	for (uint32_t i = 0; i < height - 1; ++i) {
		uint16_t *dst = (uint16_t *)output + i * width;

		int32_t cnt = width / 2;
		uint32_t j = 0;

		for (; cnt > 0; cnt -= 8, j += 16) {
			mve_pred16_t tail_p = vctp16q(cnt);

			aipl_mve_rgb_x16_t rgb;

			aipl_bayer_to_rgb_x16(&rgb, &tile, j, tail_p);

			aipl_mve_str_16px_rgbx5551(dst + j, rgb, tail_p);
		}

		tile.red_src += (i + tile.blue_row) % 2 * pitch * 2;
		tile.blue_src += (i + tile.red_row) % 2 * pitch * 2;
	}

	memcpy((uint8_t *)output + (height - 1) * width * 2,
	       (uint8_t *)output + (height - 2) * width * 2, width * 2);

	return AIPL_ERR_OK;
}

aipl_error_t aipl_bayer_decoding_rgb888_helium(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height,
					       aipl_bayer_filter_t filter)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	aipl_bayer_tile_t tile = aipl_bayer_tile(input, pitch, filter);

	for (uint32_t i = 0; i < height - 1; ++i) {
		uint8_t *dst = (uint8_t *)output + i * width * 3;

		int32_t cnt = width / 2;
		uint32_t j = 0;

		for (; cnt > 0; cnt -= 8, j += 16) {
			mve_pred16_t tail_p = vctp16q(cnt);

			aipl_mve_rgb_x16_t rgb;

			aipl_bayer_to_rgb_x16(&rgb, &tile, j, tail_p);

			aipl_mve_str_16px_rgb(dst + j * 3, rgb, tail_p, 0, 1, 2);
		}

		tile.red_src += (i + tile.blue_row) % 2 * pitch * 2;
		tile.blue_src += (i + tile.red_row) % 2 * pitch * 2;
	}

	memcpy((uint8_t *)output + (height - 1) * width * 3,
	       (uint8_t *)output + (height - 2) * width * 3, width * 3);

	return AIPL_ERR_OK;
}

aipl_error_t aipl_bayer_decoding_bgr888_helium(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height,
					       aipl_bayer_filter_t filter)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	aipl_bayer_tile_t tile = aipl_bayer_tile(input, pitch, filter);

	for (uint32_t i = 0; i < height - 1; ++i) {
		uint8_t *dst = (uint8_t *)output + i * width * 3;

		int32_t cnt = width / 2;
		uint32_t j = 0;

		for (; cnt > 0; cnt -= 8, j += 16) {
			mve_pred16_t tail_p = vctp16q(cnt);

			aipl_mve_rgb_x16_t rgb;

			aipl_bayer_to_rgb_x16(&rgb, &tile, j, tail_p);

			aipl_mve_str_16px_rgb(dst + j * 3, rgb, tail_p, 2, 1, 0);
		}

		tile.red_src += (i + tile.blue_row) % 2 * pitch * 2;
		tile.blue_src += (i + tile.red_row) % 2 * pitch * 2;
	}

	memcpy((uint8_t *)output + (height - 1) * width * 3,
	       (uint8_t *)output + (height - 2) * width * 3, width * 3);

	return AIPL_ERR_OK;
}

aipl_error_t aipl_bayer_decoding_rgb565_helium(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height,
					       aipl_bayer_filter_t filter)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	aipl_bayer_tile_t tile = aipl_bayer_tile(input, pitch, filter);

	for (uint32_t i = 0; i < height - 1; ++i) {
		uint16_t *dst = (uint16_t *)output + i * width;

		int32_t cnt = width / 2;
		uint32_t j = 0;

		for (; cnt > 0; cnt -= 8, j += 16) {
			mve_pred16_t tail_p = vctp16q(cnt);

			aipl_mve_rgb_x16_t rgb;

			aipl_bayer_to_rgb_x16(&rgb, &tile, j, tail_p);

			aipl_mve_str_16px_rgb565(dst + j, rgb, tail_p);
		}

		tile.red_src += (i + tile.blue_row) % 2 * pitch * 2;
		tile.blue_src += (i + tile.red_row) % 2 * pitch * 2;
	}

	memcpy((uint8_t *)output + (height - 1) * width * 2,
	       (uint8_t *)output + (height - 2) * width * 2, width * 2);

	return AIPL_ERR_OK;
}

aipl_error_t aipl_bayer_decoding_yv12_helium(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height,
					     aipl_bayer_filter_t filter)
{
	uint8_t *y = output;
	uint8_t *v = y + width * height;
	uint8_t *u = v + width * height / 4;

	return aipl_bayer_decoding_yuv_planar(input, y, u, v, pitch, width, height, filter);
}

aipl_error_t aipl_bayer_decoding_i420_helium(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height,
					     aipl_bayer_filter_t filter)
{
	uint8_t *y = output;
	uint8_t *u = y + width * height;
	uint8_t *v = u + width * height / 4;

	return aipl_bayer_decoding_yuv_planar(input, y, u, v, pitch, width, height, filter);
}

aipl_error_t aipl_bayer_decoding_i422_helium(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height,
					     aipl_bayer_filter_t filter)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint8_t *y = output;
	uint8_t *u = y + width * height;
	uint8_t *v = u + width * height / 2;

	aipl_bayer_tile_t tile = aipl_bayer_tile(input, pitch, filter);

	for (uint32_t i = 0; i < height - 1; ++i) {
		uint8_t *y_dst = y + i * width;
		uint8_t *u_dst = u + i * width / 2;
		uint8_t *v_dst = v + i * width / 2;

		int32_t cnt = width / 2;
		uint32_t j = 0;

		for (; cnt > 0; cnt -= 8, j += 16) {
			mve_pred16_t tail_p = vctp16q(cnt);

			aipl_mve_rgb_x8_t rgb0;
			aipl_mve_rgb_x8_t rgb1;

			aipl_bayer_to_rgb_x8(&rgb0, &rgb1, &tile, j, tail_p);

			uint16x8_t y_val0, y_val1;

			aipl_mve_cnvt_8px_xrgb8888_to_yuv_y(&y_val0, rgb0);
			aipl_mve_cnvt_8px_xrgb8888_to_yuv_y(&y_val1, rgb1);

			uint8x16_t y_val = vmovntq(vreinterpretq_u8(y_val0), y_val1);

			uint16x8_t u_val, v_val;

			aipl_mve_cnvt_8px_xrgb8888_to_yuv_u(&u_val, rgb0);
			aipl_mve_cnvt_8px_xrgb8888_to_yuv_v(&v_val, rgb0);

			vstrbq_p(u_dst + j / 2, u_val, tail_p);
			vstrbq_p(v_dst + j / 2, v_val, tail_p);

			vstrbq_p(y_dst + j, y_val, tail_p);
		}

		tile.red_src += (i + tile.blue_row) % 2 * pitch * 2;
		tile.blue_src += (i + tile.red_row) % 2 * pitch * 2;
	}

	memcpy(y + (height - 1) * width, y + (height - 2) * width, width);
	memcpy(u + (height - 1) * width / 2, u + (height - 2) * width / 2, width / 2);
	memcpy(v + (height - 1) * width / 2, v + (height - 2) * width / 2, width / 2);

	return AIPL_ERR_OK;
}

aipl_error_t aipl_bayer_decoding_i444_helium(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height,
					     aipl_bayer_filter_t filter)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint8_t *y = output;
	uint8_t *u = y + width * height;
	uint8_t *v = u + width * height;

	aipl_bayer_tile_t tile = aipl_bayer_tile(input, pitch, filter);

	for (uint32_t i = 0; i < height - 1; ++i) {
		uint8_t *y_dst = y + i * width;
		uint8_t *u_dst = u + i * width;
		uint8_t *v_dst = v + i * width;

		int32_t cnt = width / 2;
		uint32_t j = 0;

		for (; cnt > 0; cnt -= 8, j += 16) {
			mve_pred16_t tail_p = vctp16q(cnt);

			aipl_mve_rgb_x8_t rgb0;
			aipl_mve_rgb_x8_t rgb1;

			aipl_bayer_to_rgb_x8(&rgb0, &rgb1, &tile, j, tail_p);

			uint16x8_t y_val0, y_val1;

			aipl_mve_cnvt_8px_xrgb8888_to_yuv_y(&y_val0, rgb0);
			aipl_mve_cnvt_8px_xrgb8888_to_yuv_y(&y_val1, rgb1);

			uint8x16_t y_val = vmovntq(vreinterpretq_u8(y_val0), y_val1);

			uint16x8_t u_val0, u_val1, v_val0, v_val1;

			aipl_mve_cnvt_8px_xrgb8888_to_yuv_u(&u_val0, rgb0);
			aipl_mve_cnvt_8px_xrgb8888_to_yuv_u(&u_val1, rgb1);
			aipl_mve_cnvt_8px_xrgb8888_to_yuv_v(&v_val0, rgb0);
			aipl_mve_cnvt_8px_xrgb8888_to_yuv_v(&v_val1, rgb1);

			uint8x16_t u_val = vmovntq(vreinterpretq_u8(u_val0), u_val1);
			uint8x16_t v_val = vmovntq(vreinterpretq_u8(v_val0), v_val1);

			vstrbq_p(u_dst + j, u_val, tail_p);
			vstrbq_p(v_dst + j, v_val, tail_p);
			vstrbq_p(y_dst + j, y_val, tail_p);
		}

		tile.red_src += (i + tile.blue_row) % 2 * pitch * 2;
		tile.blue_src += (i + tile.red_row) % 2 * pitch * 2;
	}

	memcpy(y + (height - 1) * width, y + (height - 2) * width, width);
	memcpy(u + (height - 1) * width, u + (height - 2) * width, width);
	memcpy(v + (height - 1) * width, v + (height - 2) * width, width);

	return AIPL_ERR_OK;
}

aipl_error_t aipl_bayer_decoding_i400_helium(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height,
					     aipl_bayer_filter_t filter)
{
	return aipl_bayer_decoding_alpha8_helium(input, output, pitch, width, height, filter);
}

aipl_error_t aipl_bayer_decoding_nv12_helium(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height,
					     aipl_bayer_filter_t filter)
{
	uint8_t *y = output;
	uint8_t *u = y + width * height;
	uint8_t *v = u + 1;

	return aipl_bayer_decoding_yuv_semi_planar(input, y, u, v, pitch, width, height, filter);
}

aipl_error_t aipl_bayer_decoding_nv21_helium(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height,
					     aipl_bayer_filter_t filter)
{
	uint8_t *y = output;
	uint8_t *v = y + width * height;
	uint8_t *u = v + 1;

	return aipl_bayer_decoding_yuv_semi_planar(input, y, u, v, pitch, width, height, filter);
}

aipl_error_t aipl_bayer_decoding_yuy2_helium(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height,
					     aipl_bayer_filter_t filter)
{
	uint8_t *y = output;
	uint8_t *u = y + 1;
	uint8_t *v = u + 2;

	return aipl_bayer_decoding_yuv_packed(input, y, u, v, pitch, width, height, filter);
}

aipl_error_t aipl_bayer_decoding_uyvy_helium(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height,
					     aipl_bayer_filter_t filter)
{
	uint8_t *u = output;
	uint8_t *y = u + 1;
	uint8_t *v = u + 2;

	return aipl_bayer_decoding_yuv_packed(input, y, u, v, pitch, width, height, filter);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static INLINE void aipl_bayer_to_rgb_x8(aipl_mve_rgb_x8_t *rgb0, aipl_mve_rgb_x8_t *rgb1,
					aipl_bayer_tile_t *tile, uint32_t idx, mve_pred16_t pred)
{
	uint8x16_t rg = vldrbq_z_u8(tile->red_src + idx + 1, pred);
	uint8x16_t bg = vldrbq_z_u8(tile->blue_src + idx + 1, pred);

	uint16x8_t gr0, gr1, gb0, gb1;

	if (tile->red_col) {
		rgb1->r = vmovlbq(rg);
		rgb1->b = vmovltq(bg);
		gr1 = vmovltq(rg);
		gb1 = vmovlbq(bg);

		uint32_t tmp = *(tile->blue_src + idx + tile->blue_col);

		rgb0->r = rgb1->r;
		rgb0->b = vshlcq(rgb1->b, &tmp, 16);
		tmp = *(tile->red_src + idx + tile->blue_col);
		gr0 = vshlcq(gr1, &tmp, 16);
		gb0 = gb1;
	} else {
		rgb1->r = vmovltq(rg);
		rgb1->b = vmovlbq(bg);
		gr1 = vmovlbq(rg);
		gb1 = vmovltq(bg);

		uint32_t tmp = *(tile->red_src + idx + tile->red_col);

		rgb0->r = vshlcq(rgb1->r, &tmp, 16);
		rgb0->b = rgb1->b;
		tmp = *(tile->blue_src + idx + tile->red_col);
		gr0 = gr1;
		gb0 = vshlcq(gb1, &tmp, 16);
	}

	rgb1->g = vhaddq(gr1, gb1);
	rgb0->g = vhaddq(gr0, gb0);
}

static INLINE void aipl_bayer_to_rgb_x16(aipl_mve_rgb_x16_t *rgb, aipl_bayer_tile_t *tile,
					 uint32_t idx, mve_pred16_t pred)
{
	uint8x16_t rg = vldrbq_z_u8(tile->red_src + idx + 1, pred);
	uint8x16_t bg = vldrbq_z_u8(tile->blue_src + idx + 1, pred);

	uint16x8_t rg_rintr = vreinterpretq_u16(rg);
	uint16x8_t bg_rintr = vreinterpretq_u16(bg);

	uint8x16_t gr, gb;

	if (tile->red_col) {
		rgb->r = vmovntq(rg, rg_rintr);
		rgb->b = vmovnbq(bg, vshrq(bg_rintr, 8));
		uint32_t tmp = *(tile->blue_src + idx + tile->blue_col);

		rgb->b = vshlcq(rgb->b, &tmp, 8);
		gr = vmovnbq(rg, vshrq(rg_rintr, 8));
		tmp = *(tile->red_src + idx + tile->blue_col);
		gr = vshlcq(gr, &tmp, 8);
		gb = vmovntq(bg, bg_rintr);
	} else {
		rgb->r = vmovnbq(rg, vshrq(rg_rintr, 8));
		rgb->b = vmovntq(bg, bg_rintr);
		uint32_t tmp = *(tile->red_src + idx + tile->red_col);

		rgb->r = vshlcq(rgb->r, &tmp, 8);
		gr = vmovntq(rg, rg_rintr);
		gb = vmovnbq(bg, vshrq(bg_rintr, 8));
		tmp = *(tile->blue_src + idx + tile->red_col);
		gb = vshlcq(gb, &tmp, 8);
	}

	rgb->g = vhaddq(gr, gb);
}

static aipl_error_t aipl_bayer_decoding_yuv_planar(const void *input, uint8_t *y, uint8_t *u,
						   uint8_t *v, uint32_t pitch, uint32_t width,
						   uint32_t height, aipl_bayer_filter_t filter)
{
	if (input == NULL || y == NULL || u == NULL || v == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	aipl_bayer_tile_t tile = aipl_bayer_tile(input, pitch, filter);

	for (uint32_t i = 0; i < height - 1; ++i) {
		uint8_t *y_dst = y + i * width;
		uint8_t *u_dst = u + i / 2 * width / 2;
		uint8_t *v_dst = v + i / 2 * width / 2;

		int32_t cnt = width / 2;
		uint32_t j = 0;

		for (; cnt > 0; cnt -= 8, j += 16) {
			mve_pred16_t tail_p = vctp16q(cnt);

			aipl_mve_rgb_x8_t rgb0;
			aipl_mve_rgb_x8_t rgb1;

			aipl_bayer_to_rgb_x8(&rgb0, &rgb1, &tile, j, tail_p);

			uint16x8_t y_val0, y_val1;

			aipl_mve_cnvt_8px_xrgb8888_to_yuv_y(&y_val0, rgb0);
			aipl_mve_cnvt_8px_xrgb8888_to_yuv_y(&y_val1, rgb1);

			uint8x16_t y_val = vmovntq(vreinterpretq_u8(y_val0), y_val1);

			if (i % 2 == 0) {
				uint16x8_t u_val, v_val;

				aipl_mve_cnvt_8px_xrgb8888_to_yuv_u(&u_val, rgb0);
				aipl_mve_cnvt_8px_xrgb8888_to_yuv_v(&v_val, rgb0);

				vstrbq_p(u_dst + j / 2, u_val, tail_p);
				vstrbq_p(v_dst + j / 2, v_val, tail_p);
			}

			vstrbq_p(y_dst + j, y_val, tail_p);
		}

		tile.red_src += (i + tile.blue_row) % 2 * pitch * 2;
		tile.blue_src += (i + tile.red_row) % 2 * pitch * 2;
	}

	memcpy(y + (height - 1) * width, y + (height - 2) * width, width);

	return AIPL_ERR_OK;
}

static aipl_error_t aipl_bayer_decoding_yuv_semi_planar(const void *input, uint8_t *y, uint8_t *u,
							uint8_t *v, uint32_t pitch, uint32_t width,
							uint32_t height, aipl_bayer_filter_t filter)
{
	if (input == NULL || y == NULL || u == NULL || v == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	aipl_bayer_tile_t tile = aipl_bayer_tile(input, pitch, filter);

	for (uint32_t i = 0; i < height - 1; ++i) {
		uint8_t *y_dst = y + i * width;
		uint8_t *u_dst = u + i / 2 * width;
		uint8_t *v_dst = v + i / 2 * width;

		int32_t cnt = width / 2;
		uint32_t j = 0;

		for (; cnt > 0; cnt -= 8, j += 16) {
			mve_pred16_t tail_p = vctp16q(cnt);

			aipl_mve_rgb_x8_t rgb0;
			aipl_mve_rgb_x8_t rgb1;

			aipl_bayer_to_rgb_x8(&rgb0, &rgb1, &tile, j, tail_p);

			uint16x8_t y_val0, y_val1;

			aipl_mve_cnvt_8px_xrgb8888_to_yuv_y(&y_val0, rgb0);
			aipl_mve_cnvt_8px_xrgb8888_to_yuv_y(&y_val1, rgb1);

			uint8x16_t y_val = vmovntq(vreinterpretq_u8(y_val0), y_val1);

			if (i % 2 == 0) {
				uint16x8_t u_val, v_val;

				aipl_mve_cnvt_8px_xrgb8888_to_yuv_u(&u_val, rgb0);
				aipl_mve_cnvt_8px_xrgb8888_to_yuv_v(&v_val, rgb0);

				vstrbq_scatter_offset_p(u_dst + j, AIPL_2_BYTE_OFFSETS_U16, u_val,
							tail_p);
				vstrbq_scatter_offset_p(v_dst + j, AIPL_2_BYTE_OFFSETS_U16, v_val,
							tail_p);
			}

			vstrbq_p(y_dst + j, y_val, tail_p);
		}

		tile.red_src += (i + tile.blue_row) % 2 * pitch * 2;
		tile.blue_src += (i + tile.red_row) % 2 * pitch * 2;
	}

	memcpy(y + (height - 1) * width, y + (height - 2) * width, width);
	memcpy(y + (2 * height - 1) * width, y + (2 * height - 2) * width, width);

	return AIPL_ERR_OK;
}

static aipl_error_t aipl_bayer_decoding_yuv_packed(const void *input, uint8_t *y, uint8_t *u,
						   uint8_t *v, uint32_t pitch, uint32_t width,
						   uint32_t height, aipl_bayer_filter_t filter)
{
	if (input == NULL || y == NULL || u == NULL || v == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	aipl_bayer_tile_t tile = aipl_bayer_tile(input, pitch, filter);

	for (uint32_t i = 0; i < height - 1; ++i) {
		uint8_t *y_dst = y + i * width * 2;
		uint8_t *u_dst = u + i * width * 2;
		uint8_t *v_dst = v + i * width * 2;

		int32_t cnt = width / 2;
		uint32_t j = 0;

		for (; cnt > 0; cnt -= 8, j += 16) {
			mve_pred16_t tail_p = vctp16q(cnt);

			aipl_mve_rgb_x8_t rgb0;
			aipl_mve_rgb_x8_t rgb1;

			aipl_bayer_to_rgb_x8(&rgb0, &rgb1, &tile, j, tail_p);

			uint16x8_t y_val0, y_val1;

			aipl_mve_cnvt_8px_xrgb8888_to_yuv_y(&y_val0, rgb0);
			aipl_mve_cnvt_8px_xrgb8888_to_yuv_y(&y_val1, rgb1);

			uint8x16_t y_val = vmovntq(vreinterpretq_u8(y_val0), y_val1);

			uint16x8_t u_val, v_val;

			aipl_mve_cnvt_8px_xrgb8888_to_yuv_u(&u_val, rgb0);
			aipl_mve_cnvt_8px_xrgb8888_to_yuv_v(&v_val, rgb0);

			vstrbq_scatter_offset_p(y_dst + j * 2, AIPL_2_BYTE_OFFSETS_U8, y_val,
						tail_p);
			vstrbq_scatter_offset_p(u_dst + j * 2, AIPL_4_BYTE_OFFSETS_U16, u_val,
						tail_p);
			vstrbq_scatter_offset_p(v_dst + j * 2, AIPL_4_BYTE_OFFSETS_U16, v_val,
						tail_p);
		}

		tile.red_src += (i + tile.blue_row) % 2 * pitch * 2;
		tile.blue_src += (i + tile.red_row) % 2 * pitch * 2;
	}

	uint8_t *output = y < u ? y : u;

	memcpy(output + (height - 1) * width * 2, output + (height - 2) * width * 2, width * 2);

	return AIPL_ERR_OK;
}

#endif /* AIPL_HELIUM_ACCELERATION */

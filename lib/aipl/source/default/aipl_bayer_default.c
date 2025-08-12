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
 * @file    aipl_bayer_default.c
 * @brief   Bayer pattern decoding function implementations using default
 *          compiler optimizations
 *
 ******************************************************************************/

/*********************
 *      INCLUDES
 *********************/
#include "aipl_bayer_default.h"

#include <stddef.h>
#include <string.h>

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
static INLINE void aipl_bayer_to_rgb(uint8_t *rgb0, uint8_t *rgb1, aipl_bayer_tile_t *tile,
				     uint32_t idx);
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

aipl_error_t aipl_bayer_decoding_default(const void *input, void *output, uint32_t pitch,
					 uint32_t width, uint32_t height,
					 aipl_bayer_filter_t filter, aipl_color_format_t format)
{
	switch (format) {
	case AIPL_COLOR_ALPHA8:
		return aipl_bayer_decoding_alpha8_default(input, output, pitch, width, height,
							  filter);
	case AIPL_COLOR_ARGB8888:
		return aipl_bayer_decoding_argb8888_default(input, output, pitch, width, height,
							    filter);
	case AIPL_COLOR_ARGB4444:
		return aipl_bayer_decoding_argb4444_default(input, output, pitch, width, height,
							    filter);
	case AIPL_COLOR_ARGB1555:
		return aipl_bayer_decoding_argb1555_default(input, output, pitch, width, height,
							    filter);
	case AIPL_COLOR_RGBA8888:
		return aipl_bayer_decoding_rgba8888_default(input, output, pitch, width, height,
							    filter);
	case AIPL_COLOR_RGBA4444:
		return aipl_bayer_decoding_rgba4444_default(input, output, pitch, width, height,
							    filter);
	case AIPL_COLOR_RGBA5551:
		return aipl_bayer_decoding_rgba5551_default(input, output, pitch, width, height,
							    filter);
	case AIPL_COLOR_RGB888:
		return aipl_bayer_decoding_rgb888_default(input, output, pitch, width, height,
							  filter);
	case AIPL_COLOR_BGR888:
		return aipl_bayer_decoding_bgr888_default(input, output, pitch, width, height,
							  filter);
	case AIPL_COLOR_RGB565:
		return aipl_bayer_decoding_rgb565_default(input, output, pitch, width, height,
							  filter);
	case AIPL_COLOR_YV12:
		return aipl_bayer_decoding_yv12_default(input, output, pitch, width, height,
							filter);
	case AIPL_COLOR_I420:
		return aipl_bayer_decoding_i420_default(input, output, pitch, width, height,
							filter);
	case AIPL_COLOR_I422:
		return aipl_bayer_decoding_i422_default(input, output, pitch, width, height,
							filter);
	case AIPL_COLOR_I444:
		return aipl_bayer_decoding_i444_default(input, output, pitch, width, height,
							filter);
	case AIPL_COLOR_I400:
		return aipl_bayer_decoding_i400_default(input, output, pitch, width, height,
							filter);
	case AIPL_COLOR_NV12:
		return aipl_bayer_decoding_nv12_default(input, output, pitch, width, height,
							filter);
	case AIPL_COLOR_NV21:
		return aipl_bayer_decoding_nv21_default(input, output, pitch, width, height,
							filter);
	case AIPL_COLOR_YUY2:
		return aipl_bayer_decoding_yuy2_default(input, output, pitch, width, height,
							filter);
	case AIPL_COLOR_UYVY:
		return aipl_bayer_decoding_uyvy_default(input, output, pitch, width, height,
							filter);

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

aipl_error_t aipl_bayer_decoding_img_default(const void *input, aipl_image_t *output,
					     uint32_t pitch, aipl_bayer_filter_t filter)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	return aipl_bayer_decoding_default(input, output->data, pitch, output->width,
					   output->height, filter, output->format);
}

aipl_error_t aipl_bayer_decoding_alpha8_default(const void *input, void *output, uint32_t pitch,
						uint32_t width, uint32_t height,
						aipl_bayer_filter_t filter)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	aipl_bayer_tile_t tile = aipl_bayer_tile(input, pitch, filter);

	for (uint32_t i = 0; i < height - 1; ++i) {
		uint8_t *dst = (uint8_t *)output + i * width;

		for (uint32_t j = 0; j < width; j += 2) {
			uint8_t rgb0[3];
			uint8_t rgb1[3];

			aipl_bayer_to_rgb(rgb0, rgb1, &tile, j);

			aipl_cnvt_px_rgb_to_yuv_y(dst + j, rgb0[0], rgb0[1], rgb0[2]);
			aipl_cnvt_px_rgb_to_yuv_y(dst + j + 1, rgb1[0], rgb1[1], rgb1[2]);
		}

		tile.red_src += (i + tile.blue_row) % 2 * pitch * 2;
		tile.blue_src += (i + tile.red_row) % 2 * pitch * 2;
	}

	memcpy((uint8_t *)output + (height - 1) * width, (uint8_t *)output + (height - 2) * width,
	       width);

	return AIPL_ERR_OK;
}

aipl_error_t aipl_bayer_decoding_argb8888_default(const void *input, void *output, uint32_t pitch,
						  uint32_t width, uint32_t height,
						  aipl_bayer_filter_t filter)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	aipl_bayer_tile_t tile = aipl_bayer_tile(input, pitch, filter);

	for (uint32_t i = 0; i < height - 1; ++i) {
		aipl_argb8888_px_t *dst = (aipl_argb8888_px_t *)output + i * width;

		for (uint32_t j = 0; j < width; j += 2) {
			uint8_t rgb0[3];
			uint8_t rgb1[3];

			aipl_bayer_to_rgb(rgb0, rgb1, &tile, j);

			aipl_cnvt_px_24bit_to_argb8888(dst + j, rgb0, 0, 1, 2);
			aipl_cnvt_px_24bit_to_argb8888(dst + j + 1, rgb1, 0, 1, 2);
		}

		tile.red_src += (i + tile.blue_row) % 2 * pitch * 2;
		tile.blue_src += (i + tile.red_row) % 2 * pitch * 2;
	}

	memcpy((aipl_argb8888_px_t *)output + (height - 1) * width,
	       (aipl_argb8888_px_t *)output + (height - 2) * width,
	       width * sizeof(aipl_argb8888_px_t));

	return AIPL_ERR_OK;
}

aipl_error_t aipl_bayer_decoding_argb4444_default(const void *input, void *output, uint32_t pitch,
						  uint32_t width, uint32_t height,
						  aipl_bayer_filter_t filter)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	aipl_bayer_tile_t tile = aipl_bayer_tile(input, pitch, filter);

	for (uint32_t i = 0; i < height - 1; ++i) {
		aipl_argb4444_px_t *dst = (aipl_argb4444_px_t *)output + i * width;

		for (uint32_t j = 0; j < width; j += 2) {
			uint8_t rgb0[3];
			uint8_t rgb1[3];

			aipl_bayer_to_rgb(rgb0, rgb1, &tile, j);

			aipl_cnvt_px_24bit_to_argb4444(dst + j, rgb0, 0, 1, 2);
			aipl_cnvt_px_24bit_to_argb4444(dst + j + 1, rgb1, 0, 1, 2);
		}

		tile.red_src += (i + tile.blue_row) % 2 * pitch * 2;
		tile.blue_src += (i + tile.red_row) % 2 * pitch * 2;
	}

	memcpy((aipl_argb4444_px_t *)output + (height - 1) * width,
	       (aipl_argb4444_px_t *)output + (height - 2) * width,
	       width * sizeof(aipl_argb4444_px_t));

	return AIPL_ERR_OK;
}

aipl_error_t aipl_bayer_decoding_argb1555_default(const void *input, void *output, uint32_t pitch,
						  uint32_t width, uint32_t height,
						  aipl_bayer_filter_t filter)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	aipl_bayer_tile_t tile = aipl_bayer_tile(input, pitch, filter);

	for (uint32_t i = 0; i < height - 1; ++i) {
		aipl_argb1555_px_t *dst = (aipl_argb1555_px_t *)output + i * width;

		for (uint32_t j = 0; j < width; j += 2) {
			uint8_t rgb0[3];
			uint8_t rgb1[3];

			aipl_bayer_to_rgb(rgb0, rgb1, &tile, j);

			aipl_cnvt_px_24bit_to_argb1555(dst + j, rgb0, 0, 1, 2);
			aipl_cnvt_px_24bit_to_argb1555(dst + j + 1, rgb1, 0, 1, 2);
		}

		tile.red_src += (i + tile.blue_row) % 2 * pitch * 2;
		tile.blue_src += (i + tile.red_row) % 2 * pitch * 2;
	}

	memcpy((aipl_argb1555_px_t *)output + (height - 1) * width,
	       (aipl_argb1555_px_t *)output + (height - 2) * width,
	       width * sizeof(aipl_argb1555_px_t));

	return AIPL_ERR_OK;
}

aipl_error_t aipl_bayer_decoding_rgba8888_default(const void *input, void *output, uint32_t pitch,
						  uint32_t width, uint32_t height,
						  aipl_bayer_filter_t filter)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	aipl_bayer_tile_t tile = aipl_bayer_tile(input, pitch, filter);

	for (uint32_t i = 0; i < height - 1; ++i) {
		aipl_rgba8888_px_t *dst = (aipl_rgba8888_px_t *)output + i * width;

		for (uint32_t j = 0; j < width; j += 2) {
			uint8_t rgb0[3];
			uint8_t rgb1[3];

			aipl_bayer_to_rgb(rgb0, rgb1, &tile, j);

			aipl_cnvt_px_24bit_to_rgba8888(dst + j, rgb0, 0, 1, 2);
			aipl_cnvt_px_24bit_to_rgba8888(dst + j + 1, rgb1, 0, 1, 2);
		}

		tile.red_src += (i + tile.blue_row) % 2 * pitch * 2;
		tile.blue_src += (i + tile.red_row) % 2 * pitch * 2;
	}

	memcpy((aipl_rgba8888_px_t *)output + (height - 1) * width,
	       (aipl_rgba8888_px_t *)output + (height - 2) * width,
	       width * sizeof(aipl_rgba8888_px_t));

	return AIPL_ERR_OK;
}

aipl_error_t aipl_bayer_decoding_rgba4444_default(const void *input, void *output, uint32_t pitch,
						  uint32_t width, uint32_t height,
						  aipl_bayer_filter_t filter)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	aipl_bayer_tile_t tile = aipl_bayer_tile(input, pitch, filter);

	for (uint32_t i = 0; i < height - 1; ++i) {
		aipl_rgba4444_px_t *dst = (aipl_rgba4444_px_t *)output + i * width;

		for (uint32_t j = 0; j < width; j += 2) {
			uint8_t rgb0[3];
			uint8_t rgb1[3];

			aipl_bayer_to_rgb(rgb0, rgb1, &tile, j);

			aipl_cnvt_px_24bit_to_rgba4444(dst + j, rgb0, 0, 1, 2);
			aipl_cnvt_px_24bit_to_rgba4444(dst + j + 1, rgb1, 0, 1, 2);
		}

		tile.red_src += (i + tile.blue_row) % 2 * pitch * 2;
		tile.blue_src += (i + tile.red_row) % 2 * pitch * 2;
	}

	memcpy((aipl_rgba4444_px_t *)output + (height - 1) * width,
	       (aipl_rgba4444_px_t *)output + (height - 2) * width,
	       width * sizeof(aipl_rgba4444_px_t));

	return AIPL_ERR_OK;
}

aipl_error_t aipl_bayer_decoding_rgba5551_default(const void *input, void *output, uint32_t pitch,
						  uint32_t width, uint32_t height,
						  aipl_bayer_filter_t filter)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	aipl_bayer_tile_t tile = aipl_bayer_tile(input, pitch, filter);

	for (uint32_t i = 0; i < height - 1; ++i) {
		aipl_rgba5551_px_t *dst = (aipl_rgba5551_px_t *)output + i * width;

		for (uint32_t j = 0; j < width; j += 2) {
			uint8_t rgb0[3];
			uint8_t rgb1[3];

			aipl_bayer_to_rgb(rgb0, rgb1, &tile, j);

			aipl_cnvt_px_24bit_to_rgba5551(dst + j, rgb0, 0, 1, 2);
			aipl_cnvt_px_24bit_to_rgba5551(dst + j + 1, rgb1, 0, 1, 2);
		}

		tile.red_src += (i + tile.blue_row) % 2 * pitch * 2;
		tile.blue_src += (i + tile.red_row) % 2 * pitch * 2;
	}

	memcpy((aipl_rgba5551_px_t *)output + (height - 1) * width,
	       (aipl_rgba5551_px_t *)output + (height - 2) * width,
	       width * sizeof(aipl_rgba5551_px_t));

	return AIPL_ERR_OK;
}

aipl_error_t aipl_bayer_decoding_rgb888_default(const void *input, void *output, uint32_t pitch,
						uint32_t width, uint32_t height,
						aipl_bayer_filter_t filter)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	aipl_bayer_tile_t tile = aipl_bayer_tile(input, pitch, filter);

	for (uint32_t i = 0; i < height - 1; ++i) {
		uint8_t *dst = (uint8_t *)output + i * width * 3;

		for (uint32_t j = 0; j < width; j += 2) {
			aipl_bayer_to_rgb(dst + j * 3, dst + (j + 1) * 3, &tile, j);
		}

		tile.red_src += (i + tile.blue_row) % 2 * pitch * 2;
		tile.blue_src += (i + tile.red_row) % 2 * pitch * 2;
	}

	memcpy((uint8_t *)output + (height - 1) * width * 3,
	       (uint8_t *)output + (height - 2) * width * 3, width * 3);

	return AIPL_ERR_OK;
}

aipl_error_t aipl_bayer_decoding_bgr888_default(const void *input, void *output, uint32_t pitch,
						uint32_t width, uint32_t height,
						aipl_bayer_filter_t filter)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	filter = aipl_bayer_filter_swap_rb(filter);

	aipl_bayer_tile_t tile = aipl_bayer_tile(input, pitch, filter);

	for (uint32_t i = 0; i < height - 1; ++i) {
		uint8_t *dst = (uint8_t *)output + i * width * 3;

		for (uint32_t j = 0; j < width; j += 2) {
			aipl_bayer_to_rgb(dst + j * 3, dst + (j + 1) * 3, &tile, j);
		}

		tile.red_src += (i + tile.blue_row) % 2 * pitch * 2;
		tile.blue_src += (i + tile.red_row) % 2 * pitch * 2;
	}

	memcpy((uint8_t *)output + (height - 1) * width * 3,
	       (uint8_t *)output + (height - 2) * width * 3, width * 3);

	return AIPL_ERR_OK;
}

aipl_error_t aipl_bayer_decoding_rgb565_default(const void *input, void *output, uint32_t pitch,
						uint32_t width, uint32_t height,
						aipl_bayer_filter_t filter)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	aipl_bayer_tile_t tile = aipl_bayer_tile(input, pitch, filter);

	for (uint32_t i = 0; i < height - 1; ++i) {
		aipl_rgb565_px_t *dst = (aipl_rgb565_px_t *)output + i * width;

		for (uint32_t j = 0; j < width; j += 2) {
			uint8_t rgb0[3];
			uint8_t rgb1[3];

			aipl_bayer_to_rgb(rgb0, rgb1, &tile, j);

			aipl_cnvt_px_24bit_to_rgb565(dst + j, rgb0, 0, 1, 2);
			aipl_cnvt_px_24bit_to_rgb565(dst + j + 1, rgb1, 0, 1, 2);
		}

		tile.red_src += (i + tile.blue_row) % 2 * pitch * 2;
		tile.blue_src += (i + tile.red_row) % 2 * pitch * 2;
	}

	memcpy((aipl_rgb565_px_t *)output + (height - 1) * width,
	       (aipl_rgb565_px_t *)output + (height - 2) * width, width * sizeof(aipl_rgb565_px_t));

	return AIPL_ERR_OK;
}

aipl_error_t aipl_bayer_decoding_yv12_default(const void *input, void *output, uint32_t pitch,
					      uint32_t width, uint32_t height,
					      aipl_bayer_filter_t filter)
{
	uint8_t *y = output;
	uint8_t *v = y + width * height;
	uint8_t *u = v + width * height / 4;

	return aipl_bayer_decoding_yuv_planar(input, y, u, v, pitch, width, height, filter);
}

aipl_error_t aipl_bayer_decoding_i420_default(const void *input, void *output, uint32_t pitch,
					      uint32_t width, uint32_t height,
					      aipl_bayer_filter_t filter)
{
	uint8_t *y = output;
	uint8_t *u = y + width * height;
	uint8_t *v = u + width * height / 4;

	return aipl_bayer_decoding_yuv_planar(input, y, u, v, pitch, width, height, filter);
}

aipl_error_t aipl_bayer_decoding_i422_default(const void *input, void *output, uint32_t pitch,
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

		for (uint32_t j = 0; j < width; j += 2) {
			uint8_t rgb0[3];
			uint8_t rgb1[3];

			aipl_bayer_to_rgb(rgb0, rgb1, &tile, j);

			aipl_cnvt_px_rgb_to_yuv(y_dst + j, u_dst + j / 2, v_dst + j / 2, rgb0[0],
						rgb0[1], rgb0[2]);
			aipl_cnvt_px_rgb_to_yuv_y(y_dst + j + 1, rgb1[0], rgb1[1], rgb1[2]);
		}

		tile.red_src += (i + tile.blue_row) % 2 * pitch * 2;
		tile.blue_src += (i + tile.red_row) % 2 * pitch * 2;
	}

	memcpy(y + (height - 1) * width, y + (height - 2) * width, width);
	memcpy(u + (height - 1) * width / 2, u + (height - 2) * width / 2, width / 2);
	memcpy(v + (height - 1) * width / 2, v + (height - 2) * width / 2, width / 2);

	return AIPL_ERR_OK;
}

aipl_error_t aipl_bayer_decoding_i444_default(const void *input, void *output, uint32_t pitch,
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

		for (uint32_t j = 0; j < width; j += 2) {
			uint8_t rgb0[3];
			uint8_t rgb1[3];

			aipl_bayer_to_rgb(rgb0, rgb1, &tile, j);

			aipl_cnvt_px_rgb_to_yuv(y_dst + j, u_dst + j, v_dst + j, rgb0[0], rgb0[1],
						rgb0[2]);
			aipl_cnvt_px_rgb_to_yuv(y_dst + j + 1, u_dst + j + 1, v_dst + j + 1,
						rgb1[0], rgb1[1], rgb1[2]);
		}

		tile.red_src += (i + tile.blue_row) % 2 * pitch * 2;
		tile.blue_src += (i + tile.red_row) % 2 * pitch * 2;
	}

	memcpy(y + (height - 1) * width, y + (height - 2) * width, width);
	memcpy(u + (height - 1) * width, u + (height - 2) * width, width);
	memcpy(v + (height - 1) * width, v + (height - 2) * width, width);

	return AIPL_ERR_OK;
}

aipl_error_t aipl_bayer_decoding_i400_default(const void *input, void *output, uint32_t pitch,
					      uint32_t width, uint32_t height,
					      aipl_bayer_filter_t filter)
{
	return aipl_bayer_decoding_alpha8_default(input, output, pitch, width, height, filter);
}

aipl_error_t aipl_bayer_decoding_nv12_default(const void *input, void *output, uint32_t pitch,
					      uint32_t width, uint32_t height,
					      aipl_bayer_filter_t filter)
{
	uint8_t *y = output;
	uint8_t *u = y + width * height;
	uint8_t *v = u + 1;

	return aipl_bayer_decoding_yuv_semi_planar(input, y, u, v, pitch, width, height, filter);
}

aipl_error_t aipl_bayer_decoding_nv21_default(const void *input, void *output, uint32_t pitch,
					      uint32_t width, uint32_t height,
					      aipl_bayer_filter_t filter)
{
	uint8_t *y = output;
	uint8_t *v = y + width * height;
	uint8_t *u = v + 1;

	return aipl_bayer_decoding_yuv_semi_planar(input, y, u, v, pitch, width, height, filter);
}

aipl_error_t aipl_bayer_decoding_yuy2_default(const void *input, void *output, uint32_t pitch,
					      uint32_t width, uint32_t height,
					      aipl_bayer_filter_t filter)
{
	uint8_t *y = output;
	uint8_t *u = y + 1;
	uint8_t *v = u + 2;

	return aipl_bayer_decoding_yuv_packed(input, y, u, v, pitch, width, height, filter);
}

aipl_error_t aipl_bayer_decoding_uyvy_default(const void *input, void *output, uint32_t pitch,
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
static INLINE void aipl_bayer_to_rgb(uint8_t *rgb0, uint8_t *rgb1, aipl_bayer_tile_t *tile,
				     uint32_t idx)
{
	uint32_t red_col_idx = idx + tile->red_col;
	uint32_t blue_col_idx = idx + tile->blue_col;

	rgb0[0] = tile->red_src[red_col_idx];
	rgb0[1] = (tile->red_src[blue_col_idx] + tile->blue_src[red_col_idx]) >> 1;
	rgb0[2] = tile->blue_src[blue_col_idx];

	uint32_t red_col_2dx = tile->red_col << 1;
	uint32_t blue_col_2dx = tile->blue_col << 1;

	rgb1[0] = tile->red_src[red_col_idx + blue_col_2dx];
	rgb1[1] = (tile->red_src[blue_col_idx + red_col_2dx] +
		   tile->blue_src[red_col_idx + blue_col_2dx]) >>
		  1;
	rgb1[2] = tile->blue_src[blue_col_idx + red_col_2dx];
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

		for (uint32_t j = 0; j < width; j += 2) {
			uint8_t rgb0[3];
			uint8_t rgb1[3];

			aipl_bayer_to_rgb(rgb0, rgb1, &tile, j);

			if (i % 2 == 0) {
				aipl_cnvt_px_rgb_to_yuv(y_dst + j, u_dst + j / 2, v_dst + j / 2,
							rgb0[0], rgb0[1], rgb0[2]);
			} else {
				aipl_cnvt_px_rgb_to_yuv_y(y_dst + j, rgb0[0], rgb0[1], rgb0[2]);
			}

			aipl_cnvt_px_rgb_to_yuv_y(y_dst + j + 1, rgb1[0], rgb1[1], rgb1[2]);
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

		for (uint32_t j = 0; j < width; j += 2) {
			uint8_t rgb0[3];
			uint8_t rgb1[3];

			aipl_bayer_to_rgb(rgb0, rgb1, &tile, j);

			if (i % 2 == 0) {
				aipl_cnvt_px_rgb_to_yuv(y_dst + j, u_dst + j, v_dst + j, rgb0[0],
							rgb0[1], rgb0[2]);
			} else {
				aipl_cnvt_px_rgb_to_yuv_y(y_dst + j, rgb0[0], rgb0[1], rgb0[2]);
			}

			aipl_cnvt_px_rgb_to_yuv_y(y_dst + j + 1, rgb1[0], rgb1[1], rgb1[2]);
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

		for (uint32_t j = 0; j < width; j += 2) {
			uint8_t rgb0[3];
			uint8_t rgb1[3];

			aipl_bayer_to_rgb(rgb0, rgb1, &tile, j);

			aipl_cnvt_px_rgb_to_yuv(y_dst + j * 2, u_dst + j * 2, v_dst + j * 2,
						rgb0[0], rgb0[1], rgb0[2]);
			aipl_cnvt_px_rgb_to_yuv_y(y_dst + (j + 1) * 2, rgb1[0], rgb1[1], rgb1[2]);
		}

		tile.red_src += (i + tile.blue_row) % 2 * pitch * 2;
		tile.blue_src += (i + tile.red_row) % 2 * pitch * 2;
	}

	uint8_t *output = y < u ? y : u;

	memcpy(output + (height - 1) * width * 2, output + (height - 2) * width * 2, width * 2);

	return AIPL_ERR_OK;
}

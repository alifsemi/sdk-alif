/*
 * 1394-Based Digital Camera Control Library
 *
 * Bayer pattern decoding functions
 *
 * The original Bayer decoding functions are copied from libdc1394:Add commentMore actions
 * https://sourceforge.net/p/libdc1394/code/ci/master/tree/dc1394/bayer.c
 *
 * Written by Damien Douxchamps and Frederic Devernay
 * The original VNG and AHD Bayer decoding are from Dave Coffin's DCRAW.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/******************************************************************************
 * @file    aipl_bayer.c
 * @brief   Bayer pattern decoding function implementations
 *
 ******************************************************************************/

/*********************
 *      INCLUDES
 *********************/
#include <stddef.h>

#include "aipl_arm_mve.h"
#include "aipl_bayer.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void aipl_bayer_aipl_bayer_clear_borders(uint8_t *rgb, int sx, int sy, int w, int rgb_bytes);
/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

aipl_error_t aipl_bayer_decoding_img(const void *bayer, aipl_image_t *output,
				     aipl_color_filter_t tile, aipl_bayer_method_t method)
{
	if (bayer == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	if (output->data == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	return aipl_bayer_decoding(bayer, output->data, output->width, output->height, tile, method,
				   output->format);
}

aipl_error_t aipl_bayer_decoding(const void *bayer, void *rgb, uint32_t width, uint32_t height,
				 aipl_color_filter_t tile, aipl_bayer_method_t method,
				 aipl_color_format_t format)
{
	switch (format) {
	case AIPL_COLOR_RGB565:
		if (method == AIPL_BAYER_METHOD_NEAREST) {
			return aipl_bayer_decoding_nearest_rgb565(
				(const uint8_t *)bayer, (uint8_t *)rgb, width, height, tile);
		} else if (method == AIPL_BAYER_METHOD_SIMPLE) {
			return aipl_bayer_decoding_simple_rgb565(
				(const uint8_t *)bayer, (uint8_t *)rgb, width, height, tile);
		} else if (method == AIPL_BAYER_METHOD_BILINEAR) {
			return aipl_bayer_decoding_linear_rgb565(
				(const uint8_t *)bayer, (uint8_t *)rgb, width, height, tile);
		} else {
			return AIPL_ERR_BAYER_INVALID_METHOD;
		}
	case AIPL_COLOR_RGB888:
		if (method == AIPL_BAYER_METHOD_NEAREST) {
			return aipl_bayer_decoding_nearest_rgb888(
				(const uint8_t *)bayer, (uint8_t *)rgb, width, height, tile);
		} else if (method == AIPL_BAYER_METHOD_SIMPLE) {
			return aipl_bayer_decoding_simple_rgb888(
				(const uint8_t *)bayer, (uint8_t *)rgb, width, height, tile);
		} else if (method == AIPL_BAYER_METHOD_BILINEAR) {
			return aipl_bayer_decoding_linear_rgb888(
				(const uint8_t *)bayer, (uint8_t *)rgb, width, height, tile);
		} else {
			return AIPL_ERR_BAYER_INVALID_METHOD;
		}
	case AIPL_COLOR_BGR888:
		if (method == AIPL_BAYER_METHOD_NEAREST) {
			return aipl_bayer_decoding_nearest_bgr888(
				(const uint8_t *)bayer, (uint8_t *)rgb, width, height, tile);
		} else if (method == AIPL_BAYER_METHOD_SIMPLE) {
			return aipl_bayer_decoding_simple_bgr888(
				(const uint8_t *)bayer, (uint8_t *)rgb, width, height, tile);
		} else if (method == AIPL_BAYER_METHOD_BILINEAR) {
			return aipl_bayer_decoding_linear_bgr888(
				(const uint8_t *)bayer, (uint8_t *)rgb, width, height, tile);
		} else {
			return AIPL_ERR_BAYER_INVALID_METHOD;
		}
	case AIPL_COLOR_ARGB8888:
		if (method == AIPL_BAYER_METHOD_NEAREST) {
			return aipl_bayer_decoding_nearest_argb8888(
				(const uint8_t *)bayer, (uint8_t *)rgb, width, height, tile);
		} else if (method == AIPL_BAYER_METHOD_SIMPLE) {
			return aipl_bayer_decoding_simple_argb8888(
				(const uint8_t *)bayer, (uint8_t *)rgb, width, height, tile);
		} else if (method == AIPL_BAYER_METHOD_BILINEAR) {
			return aipl_bayer_decoding_linear_argb8888(
				(const uint8_t *)bayer, (uint8_t *)rgb, width, height, tile);
		} else {
			return AIPL_ERR_BAYER_INVALID_METHOD;
		}
	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

aipl_error_t aipl_bayer_decoding_nearest_rgb565(const uint8_t *restrict bayer,
						uint8_t *restrict rgb, int width, int height,
						int tile)
{
	if ((tile > AIPL_COLOR_FILTER_MAX) || (tile < AIPL_COLOR_FILTER_MIN)) {
		return AIPL_ERR_BAYER_INVALID_FILTER;
	}

	const int rgb_bytes = 2;
	const int bayer_step = width;
	const int rgb_step = rgb_bytes * width;
	int blue = tile == AIPL_COLOR_FILTER_BGGR || tile == AIPL_COLOR_FILTER_GBRG ? -1 : 1;
	int start_with_green = tile == AIPL_COLOR_FILTER_GBRG || tile == AIPL_COLOR_FILTER_GRBG;
	int i;

	/* Add black border */
	int i_max = width * height * rgb_bytes;

	for (i = width * (height - 1) * rgb_bytes; i < i_max; i++) {
		rgb[i] = 0;
	}
	int i_inc = (width - 1) * rgb_bytes;

	for (i = (width - 1) * rgb_bytes; i < i_max; i += i_inc) {
		int j;

		for (j = 0; j < rgb_bytes; j++) {
			rgb[i++] = 0;
		}
	}

	width -= 1;
	height -= 1;

	/* Index table into 16 RGB pairs for scatter stores: { 0, 6, 12, .. } */
	const uint8x16_t inc = vmulq_n_u8(vidupq_n_u8(0, 1), 2 * rgb_bytes);

	for (; height--; bayer += bayer_step, rgb += rgb_step) {
		const uint8_t *bayer_end = bayer + width;

		if (start_with_green) {
			if (blue > 0) {
				rgb[0] = ((bayer[bayer_step + 1] & 0xfc) << 3) |
					 (bayer[bayer_step] >> 3);
				rgb[1] = (bayer[1] & 0xf8) | (bayer[bayer_step + 1] >> 5);
			} else {
				rgb[0] = ((bayer[bayer_step + 1] & 0xfc) << 3) | (bayer[1] >> 3);
				rgb[1] = (bayer[bayer_step] & 0xf8) | (bayer[bayer_step + 1] >> 5);
			}

			bayer++;
			rgb += rgb_bytes;
		}

		/* Helium lets us process 16 at a time (8 per beat on Cortex-M55) */
		int pairs_to_go = (bayer_end - bayer) / 2;

		while (pairs_to_go > 0) {
			mve_pred16_t p = vctp8q(pairs_to_go);
			uint8x16x2_t rg = vld2q(bayer);
			uint8x16x2_t gr = vld2q(bayer + 1);
			uint8x16x2_t gb = vld2q(bayer + bayer_step);
			uint8x16x2_t bg = vld2q(bayer + bayer_step + 1);
#ifndef __ICCARM__
			__builtin_prefetch(bayer + bayer_step + 16 * 2 * 2);
#endif
			uint8x16_t upper;
			uint8x16_t lower;

			if (blue > 0) {
				upper = vsriq(rg.val[0], rg.val[1], 5);
				lower = vshlq_n(rg.val[1], 3);
				lower = vsriq(lower, gb.val[1], 3);
			} else {
				upper = vsriq(gb.val[1], rg.val[1], 5);
				lower = vshlq_n(rg.val[1], 3);
				lower = vsriq(lower, rg.val[0], 3);
			}
			vstrbq_scatter_offset_p(rgb, inc, lower, p);
			vstrbq_scatter_offset_p(rgb + 1, inc, upper, p);

			if (blue > 0) {
				upper = vsriq(gr.val[1], bg.val[1], 5);
				lower = vshlq_n(bg.val[1], 3);
				lower = vsriq(lower, bg.val[0], 3);
			} else {
				upper = vsriq(bg.val[0], bg.val[1], 5);
				lower = vshlq_n(bg.val[1], 3);
				lower = vsriq(lower, gr.val[1], 3);
			}
			vstrbq_scatter_offset_p(rgb + rgb_bytes, inc, lower, p);
			vstrbq_scatter_offset_p(rgb + rgb_bytes + 1, inc, upper, p);

			bayer += 16 * 2;
			rgb += 16 * 2 * rgb_bytes;
			pairs_to_go -= 16;
		}

		bayer += pairs_to_go * 2;
		rgb += pairs_to_go * 2 * rgb_bytes;

		if (bayer < bayer_end) {
			if (blue > 0) {
				rgb[0] = ((bayer[1] & 0xfc) << 3) | (bayer[bayer_step + 1] >> 3);
				rgb[1] = (bayer[0] & 0xf8) | (bayer[1] >> 5);
			} else {
				rgb[0] = ((bayer[1] & 0xfc) << 3) | (bayer[0] >> 3);
				rgb[1] = (bayer[bayer_step + 1] & 0xf8) | (bayer[1] >> 5);
			}

			bayer++;
			rgb += rgb_bytes;
		}

		bayer -= width;
		rgb -= width * rgb_bytes;

		blue = -blue;
		start_with_green = !start_with_green;
	}

	return AIPL_ERR_OK;
}

aipl_error_t aipl_bayer_decoding_nearest_bgr888(const uint8_t *restrict bayer,
						uint8_t *restrict rgb, int width, int height,
						int tile)
{
	if ((tile > AIPL_COLOR_FILTER_MAX) || (tile < AIPL_COLOR_FILTER_MIN)) {
		return AIPL_ERR_BAYER_INVALID_FILTER;
	}

	const int rgb_bytes = 3;
	const int bayer_step = width;
	const int rgb_step = rgb_bytes * width;
	int blue = tile == AIPL_COLOR_FILTER_BGGR || tile == AIPL_COLOR_FILTER_GBRG ? -1 : 1;
	int start_with_green = tile == AIPL_COLOR_FILTER_GBRG || tile == AIPL_COLOR_FILTER_GRBG;
	int i;

	/* Add black border */
	int i_max = width * height * rgb_bytes;

	for (i = width * (height - 1) * rgb_bytes; i < i_max; i++) {
		rgb[i] = 0;
	}
	int i_inc = (width - 1) * rgb_bytes;

	for (i = (width - 1) * rgb_bytes; i < i_max; i += i_inc) {
		int j;

		for (j = 0; j < rgb_bytes; j++) {
			rgb[i++] = 0;
		}
	}

	rgb += 1;
	width -= 1;
	height -= 1;

	/* Index table into 16 RGB pairs for scatter stores: { 0, 6, 12, .. } */
	const uint8x16_t inc = vmulq_n_u8(vidupq_n_u8(0, 1), 2 * rgb_bytes);

	for (; height--; bayer += bayer_step, rgb += rgb_step) {
		const uint8_t *bayer_end = bayer + width;

		if (start_with_green) {
			rgb[-blue] = bayer[bayer_step];
			rgb[0] = bayer[bayer_step + 1];
			rgb[blue] = bayer[1];

			bayer++;
			rgb += rgb_bytes;
		}

		/* Helium lets us process 16 at a time (8 per beat on Cortex-M55) */
		int pairs_to_go = (bayer_end - bayer) / 2;

		while (pairs_to_go > 0) {
			mve_pred16_t p = vctp8q(pairs_to_go);
			uint8x16x2_t rg = vld2q(bayer);
			uint8x16x2_t gr = vld2q(bayer + 1);
			uint8x16x2_t gb = vld2q(bayer + bayer_step);
			uint8x16x2_t bg = vld2q(bayer + bayer_step + 1);
#ifndef __ICCARM__
			__builtin_prefetch(bayer + bayer_step + 16 * 2 * 2);
#endif

			vstrbq_scatter_offset_p(rgb - blue, inc, gb.val[1], p);
			vstrbq_scatter_offset_p(rgb, inc, rg.val[1], p);
			vstrbq_scatter_offset_p(rgb + blue, inc, rg.val[0], p);

			vstrbq_scatter_offset_p(rgb + rgb_bytes - blue, inc, bg.val[0], p);
			vstrbq_scatter_offset_p(rgb + rgb_bytes, inc, bg.val[1], p);
			vstrbq_scatter_offset_p(rgb + rgb_bytes + blue, inc, gr.val[1], p);

			bayer += 16 * 2;
			rgb += 16 * 2 * rgb_bytes;
			pairs_to_go -= 16;
		}

		bayer += pairs_to_go * 2;
		rgb += pairs_to_go * 2 * rgb_bytes;

		if (bayer < bayer_end) {
			rgb[-blue] = bayer[bayer_step + 1];
			rgb[0] = bayer[1];
			rgb[blue] = bayer[0];

			bayer++;
			rgb += rgb_bytes;
		}

		bayer -= width;
		rgb -= width * rgb_bytes;

		blue = -blue;
		start_with_green = !start_with_green;
	}

	return AIPL_ERR_OK;
}

aipl_error_t aipl_bayer_decoding_nearest_rgb888(const uint8_t *restrict bayer,
						uint8_t *restrict rgb, int width, int height,
						int tile)
{
	if ((tile > AIPL_COLOR_FILTER_MAX) || (tile < AIPL_COLOR_FILTER_MIN)) {
		return AIPL_ERR_BAYER_INVALID_FILTER;
	}

	const int rgb_bytes = 3;
	const int bayer_step = width;
	const int rgb_step = rgb_bytes * width;
	int blue = tile == AIPL_COLOR_FILTER_BGGR || tile == AIPL_COLOR_FILTER_GBRG ? -1 : 1;
	int start_with_green = tile == AIPL_COLOR_FILTER_GBRG || tile == AIPL_COLOR_FILTER_GRBG;
	int i;

	/* Add black border */
	int i_max = width * height * rgb_bytes;

	for (i = width * (height - 1) * rgb_bytes; i < i_max; i++) {
		rgb[i] = 0;
	}
	int i_inc = (width - 1) * rgb_bytes;

	for (i = (width - 1) * rgb_bytes; i < i_max; i += i_inc) {
		int j;

		for (j = 0; j < rgb_bytes; j++) {
			rgb[i++] = 0;
		}
	}

	rgb += 1;
	width -= 1;
	height -= 1;

	/* Index table into 16 RGB pairs for scatter stores: { 0, 6, 12, .. } */
	const uint8x16_t inc = vmulq_n_u8(vidupq_n_u8(0, 1), 2 * rgb_bytes);

	for (; height--; bayer += bayer_step, rgb += rgb_step) {
		const uint8_t *bayer_end = bayer + width;

		if (start_with_green) {
			rgb[-blue] = bayer[1];
			rgb[0] = bayer[bayer_step + 1];
			rgb[blue] = bayer[bayer_step];

			bayer++;
			rgb += rgb_bytes;
		}

		/* Helium lets us process 16 at a time (8 per beat on Cortex-M55) */
		int pairs_to_go = (bayer_end - bayer) / 2;

		while (pairs_to_go > 0) {
			mve_pred16_t p = vctp8q(pairs_to_go);
			uint8x16x2_t rg = vld2q(bayer);
			uint8x16x2_t gr = vld2q(bayer + 1);
			uint8x16x2_t gb = vld2q(bayer + bayer_step);
			uint8x16x2_t bg = vld2q(bayer + bayer_step + 1);
#ifndef __ICCARM__
			__builtin_prefetch(bayer + bayer_step + 16 * 2 * 2);
#endif
			vstrbq_scatter_offset_p(rgb - blue, inc, rg.val[0], p);
			vstrbq_scatter_offset_p(rgb, inc, rg.val[1], p);
			vstrbq_scatter_offset_p(rgb + blue, inc, gb.val[1], p);

			vstrbq_scatter_offset_p(rgb + rgb_bytes - blue, inc, gr.val[1], p);
			vstrbq_scatter_offset_p(rgb + rgb_bytes, inc, bg.val[1], p);
			vstrbq_scatter_offset_p(rgb + rgb_bytes + blue, inc, bg.val[0], p);

			bayer += 16 * 2;
			rgb += 16 * 2 * rgb_bytes;
			pairs_to_go -= 16;
		}

		bayer += pairs_to_go * 2;
		rgb += pairs_to_go * 2 * rgb_bytes;

		if (bayer < bayer_end) {
			rgb[-blue] = bayer[0];
			rgb[0] = bayer[1];
			rgb[blue] = bayer[bayer_step + 1];

			bayer++;
			rgb += rgb_bytes;
		}

		bayer -= width;
		rgb -= width * rgb_bytes;

		blue = -blue;
		start_with_green = !start_with_green;
	}

	return AIPL_ERR_OK;
}

aipl_error_t aipl_bayer_decoding_nearest_argb8888(const uint8_t *restrict bayer,
						  uint8_t *restrict rgb, int width, int height,
						  int tile)
{
	if ((tile > AIPL_COLOR_FILTER_MAX) || (tile < AIPL_COLOR_FILTER_MIN)) {
		return AIPL_ERR_BAYER_INVALID_FILTER;
	}

	const int rgb_bytes = 4;
	const int bayer_step = width;
	const int rgb_step = rgb_bytes * width;
	int blue = tile == AIPL_COLOR_FILTER_BGGR || tile == AIPL_COLOR_FILTER_GBRG ? -1 : 1;
	int start_with_green = tile == AIPL_COLOR_FILTER_GBRG || tile == AIPL_COLOR_FILTER_GRBG;
	int i;

	/* Add black border */
	int i_max = width * height * rgb_bytes;

	for (i = width * (height - 1) * rgb_bytes; i < i_max; i++) {
		rgb[i] = 0;
	}
	int i_inc = (width - 1) * rgb_bytes;

	for (i = (width - 1) * rgb_bytes; i < i_max; i += i_inc) {
		int j;

		for (j = 0; j < rgb_bytes; j++) {
			rgb[i++] = 0;
		}
	}

	rgb += 1;
	width -= 1;
	height -= 1;

	/* Index table into 16 RGB pairs for scatter stores: { 0, 6, 12, .. } */
	const uint8x16_t inc = vmulq_n_u8(vidupq_n_u8(0, 1), 2 * rgb_bytes);

	for (; height--; bayer += bayer_step, rgb += rgb_step) {
		const uint8_t *bayer_end = bayer + width;

		if (start_with_green) {
			rgb[-blue] = bayer[bayer_step];
			rgb[0] = bayer[bayer_step + 1];
			rgb[blue] = bayer[1];
			rgb[2] = 0xff;

			bayer++;
			rgb += rgb_bytes;
		}

		/* Helium lets us process 16 at a time (8 per beat on Cortex-M55) */
		int pairs_to_go = (bayer_end - bayer) / 2;

		while (pairs_to_go > 0) {
			mve_pred16_t p = vctp8q(pairs_to_go);
			uint8x16x2_t rg = vld2q(bayer);
			uint8x16x2_t gr = vld2q(bayer + 1);
			uint8x16x2_t gb = vld2q(bayer + bayer_step);
			uint8x16x2_t bg = vld2q(bayer + bayer_step + 1);
			uint8x16_t a = vcreateq_u8(0xffffffffffffffff, 0xffffffffffffffff);
#ifndef __ICCARM__
			__builtin_prefetch(bayer + bayer_step + 16 * 2 * 2);
#endif
			vstrbq_scatter_offset_p(rgb - blue, inc, gb.val[1], p);
			vstrbq_scatter_offset_p(rgb, inc, rg.val[1], p);
			vstrbq_scatter_offset_p(rgb + blue, inc, rg.val[0], p);
			vstrbq_scatter_offset_p(rgb + 2, inc, a, p);

			vstrbq_scatter_offset_p(rgb + rgb_bytes - blue, inc, bg.val[0], p);
			vstrbq_scatter_offset_p(rgb + rgb_bytes, inc, bg.val[1], p);
			vstrbq_scatter_offset_p(rgb + rgb_bytes + blue, inc, gr.val[1], p);
			vstrbq_scatter_offset_p(rgb + rgb_bytes + 2, inc, a, p);

			bayer += 16 * 2;
			rgb += 16 * 2 * rgb_bytes;
			pairs_to_go -= 16;
		}

		bayer += pairs_to_go * 2;
		rgb += pairs_to_go * 2 * rgb_bytes;

		if (bayer < bayer_end) {
			rgb[-blue] = bayer[bayer_step + 1];
			rgb[0] = bayer[1];
			rgb[blue] = bayer[0];
			rgb[2] = 0xff;

			bayer++;
			rgb += rgb_bytes;
		}

		bayer -= width;
		rgb -= width * rgb_bytes;

		blue = -blue;
		start_with_green = !start_with_green;
	}

	return AIPL_ERR_OK;
}

aipl_error_t aipl_bayer_decoding_simple_rgb565(const uint8_t *restrict bayer, uint8_t *restrict rgb,
					       int width, int height, int tile)
{
	if ((tile > AIPL_COLOR_FILTER_MAX) || (tile < AIPL_COLOR_FILTER_MIN)) {
		return AIPL_ERR_BAYER_INVALID_FILTER;
	}

	const int rgb_bytes = 2;
	const int bayer_step = width;
	const int rgb_step = rgb_bytes * width;
	int blue = tile == AIPL_COLOR_FILTER_BGGR || tile == AIPL_COLOR_FILTER_GBRG ? -1 : 1;
	int start_with_green = tile == AIPL_COLOR_FILTER_GBRG || tile == AIPL_COLOR_FILTER_GRBG;
	int i;

	/* Add black border */
	int i_max = width * height * rgb_bytes;

	for (i = width * (height - 1) * rgb_bytes; i < i_max; i++) {
		rgb[i] = 0;
	}
	int i_inc = (width - 1) * rgb_bytes;

	for (i = (width - 1) * rgb_bytes; i < i_max; i += i_inc) {
		int j;

		for (j = 0; j < rgb_bytes; j++) {
			rgb[i++] = 0;
		}
	}

	width -= 1;
	height -= 1;

	/* Index table into 16 RGB pairs for scatter stores: { 0, 6, 12, .. } */
	const uint8x16_t inc = vmulq_n_u8(vidupq_n_u8(0, 1), 2 * rgb_bytes);

	for (; height--; bayer += bayer_step, rgb += rgb_step) {
		const uint8_t *bayer_end = bayer + width;

		if (start_with_green) {
			uint8_t g = (bayer[0] + bayer[bayer_step + 1] + 1) >> 1;

			if (blue > 0) {
				rgb[0] = ((g & 0xfc) << 3) | (bayer[bayer_step] >> 3);
				rgb[1] = (bayer[1] & 0xf8) | (g >> 5);
			} else {
				rgb[0] = ((g & 0xfc) << 3) | (bayer[1] >> 3);
				rgb[1] = (bayer[bayer_step] & 0xf8) | (g >> 5);
			}

			bayer++;
			rgb += rgb_bytes;
		}

		/* Helium lets us process 16 at a time (8 per beat on Cortex-M55) */
		int pairs_to_go = (bayer_end - bayer) / 2;

		while (pairs_to_go > 0) {
			mve_pred16_t p = vctp8q(pairs_to_go);
			uint8x16x2_t rg = vld2q(bayer);
			uint8x16x2_t gr = vld2q(bayer + 1);
			uint8x16x2_t gb = vld2q(bayer + bayer_step);
			uint8x16x2_t bg = vld2q(bayer + bayer_step + 1);
#ifndef __ICCARM__
			__builtin_prefetch(bayer + bayer_step + 16 * 2 * 2);
#endif
			uint8x16_t upper;
			uint8x16_t lower;

			uint8x16_t g0 = vrhaddq_x(rg.val[1], gb.val[0], p);

			lower = vshlq_n(g0, 3);
			if (blue > 0) {
				lower = vsriq(lower, gb.val[1], 3);
				upper = vsriq(rg.val[0], g0, 5);
			} else {
				lower = vsriq(lower, rg.val[0], 3);
				upper = vsriq(gb.val[1], g0, 5);
			}
			vstrbq_scatter_offset_p(rgb, inc, lower, p);
			vstrbq_scatter_offset_p(rgb + 1, inc, upper, p);

			uint8x16_t g1 = vrhaddq_x(gr.val[0], bg.val[1], p);

			lower = vshlq_n(g1, 3);
			if (blue > 0) {
				upper = vsriq(gr.val[1], g1, 5);
				lower = vsriq(lower, bg.val[0], 3);
			} else {
				upper = vsriq(bg.val[0], g1, 5);
				lower = vsriq(lower, gr.val[1], 3);
			}
			vstrbq_scatter_offset_p(rgb + rgb_bytes, inc, lower, p);
			vstrbq_scatter_offset_p(rgb + rgb_bytes + 1, inc, upper, p);

			bayer += 16 * 2;
			rgb += 16 * 2 * rgb_bytes;
			pairs_to_go -= 16;
		}

		bayer += pairs_to_go * 2;
		rgb += pairs_to_go * 2 * rgb_bytes;

		if (bayer < bayer_end) {
			uint8_t g = (bayer[1] + bayer[bayer_step] + 1) >> 1;

			if (blue > 0) {
				rgb[0] = ((g & 0xfc) << 3) | (bayer[bayer_step + 1] >> 3);
				rgb[1] = (bayer[0] & 0xf8) | (g >> 5);
			} else {
				rgb[0] = ((g & 0xfc) << 3) | (bayer[0] >> 3);
				rgb[1] = (bayer[bayer_step + 1] & 0xf8) | (g >> 5);
			}

			bayer++;
			rgb += rgb_bytes;
		}

		bayer -= width;
		rgb -= width * rgb_bytes;

		blue = -blue;
		start_with_green = !start_with_green;
	}

	return AIPL_ERR_OK;
}

aipl_error_t aipl_bayer_decoding_simple_bgr888(const uint8_t *restrict bayer, uint8_t *restrict rgb,
					       int width, int height, int tile)
{
	if ((tile > AIPL_COLOR_FILTER_MAX) || (tile < AIPL_COLOR_FILTER_MIN)) {
		return AIPL_ERR_BAYER_INVALID_FILTER;
	}

	const int rgb_bytes = 3;
	const int bayer_step = width;
	const int rgb_step = rgb_bytes * width;
	int blue = tile == AIPL_COLOR_FILTER_BGGR || tile == AIPL_COLOR_FILTER_GBRG ? -1 : 1;
	int start_with_green = tile == AIPL_COLOR_FILTER_GBRG || tile == AIPL_COLOR_FILTER_GRBG;
	int i;

	/* Add black border */
	int i_max = width * height * rgb_bytes;

	for (i = width * (height - 1) * rgb_bytes; i < i_max; i++) {
		rgb[i] = 0;
	}
	int i_inc = (width - 1) * rgb_bytes;

	for (i = (width - 1) * rgb_bytes; i < i_max; i += i_inc) {
		int j;

		for (j = 0; j < rgb_bytes; j++) {
			rgb[i++] = 0;
		}
	}

	rgb += 1;
	width -= 1;
	height -= 1;

	/* Index table into 16 RGB pairs for scatter stores: { 0, 6, 12, .. } */
	const uint8x16_t inc = vmulq_n_u8(vidupq_n_u8(0, 1), 2 * rgb_bytes);

	for (; height--; bayer += bayer_step, rgb += rgb_step) {
		const uint8_t *bayer_end = bayer + width;

		if (start_with_green) {
			rgb[-blue] = bayer[bayer_step];
			rgb[0] = (bayer[0] + bayer[bayer_step + 1] + 1) >> 1;
			rgb[blue] = bayer[1];

			bayer++;
			rgb += rgb_bytes;
		}

		/* Helium lets us process 16 at a time (8 per beat on Cortex-M55) */
		int pairs_to_go = (bayer_end - bayer) / 2;

		while (pairs_to_go > 0) {
			mve_pred16_t p = vctp8q(pairs_to_go);
			uint8x16x2_t rg = vld2q(bayer);
			uint8x16x2_t gr = vld2q(bayer + 1);
			uint8x16x2_t gb = vld2q(bayer + bayer_step);
			uint8x16x2_t bg = vld2q(bayer + bayer_step + 1);
#ifndef __ICCARM__
			__builtin_prefetch(bayer + bayer_step + 16 * 2 * 2);
#endif
			vstrbq_scatter_offset_p(rgb - blue, inc, gb.val[1], p);
			uint8x16_t g0 = vrhaddq_x(rg.val[1], gb.val[0], p);

			vstrbq_scatter_offset_p(rgb, inc, g0, p);
			vstrbq_scatter_offset_p(rgb + blue, inc, rg.val[0], p);

			vstrbq_scatter_offset_p(rgb + rgb_bytes - blue, inc, bg.val[0], p);
			uint8x16_t g1 = vrhaddq_x(gr.val[0], bg.val[1], p);

			vstrbq_scatter_offset_p(rgb + rgb_bytes, inc, g1, p);
			vstrbq_scatter_offset_p(rgb + rgb_bytes + blue, inc, gr.val[1], p);

			bayer += 16 * 2;
			rgb += 16 * 2 * rgb_bytes;
			pairs_to_go -= 16;
		}

		bayer += pairs_to_go * 2;
		rgb += pairs_to_go * 2 * rgb_bytes;

		if (bayer < bayer_end) {
			rgb[-blue] = bayer[bayer_step + 1];
			rgb[0] = (bayer[1] + bayer[bayer_step] + 1) >> 1;
			rgb[blue] = bayer[0];

			bayer++;
			rgb += rgb_bytes;
		}

		bayer -= width;
		rgb -= width * rgb_bytes;

		blue = -blue;
		start_with_green = !start_with_green;
	}

	return AIPL_ERR_OK;
}

aipl_error_t aipl_bayer_decoding_simple_rgb888(const uint8_t *restrict bayer, uint8_t *restrict rgb,
					       int width, int height, int tile)
{
	if ((tile > AIPL_COLOR_FILTER_MAX) || (tile < AIPL_COLOR_FILTER_MIN)) {
		return AIPL_ERR_BAYER_INVALID_FILTER;
	}

	const int rgb_bytes = 3;
	const int bayer_step = width;
	const int rgb_step = rgb_bytes * width;
	int blue = tile == AIPL_COLOR_FILTER_BGGR || tile == AIPL_COLOR_FILTER_GBRG ? -1 : 1;
	int start_with_green = tile == AIPL_COLOR_FILTER_GBRG || tile == AIPL_COLOR_FILTER_GRBG;
	int i;

	/* Add black border */
	int i_max = width * height * rgb_bytes;

	for (i = width * (height - 1) * rgb_bytes; i < i_max; i++) {
		rgb[i] = 0;
	}
	int i_inc = (width - 1) * rgb_bytes;

	for (i = (width - 1) * rgb_bytes; i < i_max; i += i_inc) {
		int j;

		for (j = 0; j < rgb_bytes; j++) {
			rgb[i++] = 0;
		}
	}

	rgb += 1;
	width -= 1;
	height -= 1;

	/* Index table into 16 RGB pairs for scatter stores: { 0, 6, 12, .. } */
	const uint8x16_t inc = vmulq_n_u8(vidupq_n_u8(0, 1), 2 * rgb_bytes);

	for (; height--; bayer += bayer_step, rgb += rgb_step) {
		const uint8_t *bayer_end = bayer + width;

		if (start_with_green) {
			rgb[-blue] = bayer[1];
			rgb[0] = (bayer[0] + bayer[bayer_step + 1] + 1) >> 1;
			rgb[blue] = bayer[bayer_step];

			bayer++;
			rgb += rgb_bytes;
		}

		/* Helium lets us process 16 at a time (8 per beat on Cortex-M55) */
		int pairs_to_go = (bayer_end - bayer) / 2;

		while (pairs_to_go > 0) {
			mve_pred16_t p = vctp8q(pairs_to_go);
			uint8x16x2_t rg = vld2q(bayer);
			uint8x16x2_t gr = vld2q(bayer + 1);
			uint8x16x2_t gb = vld2q(bayer + bayer_step);
			uint8x16x2_t bg = vld2q(bayer + bayer_step + 1);
#ifndef __ICCARM__
			__builtin_prefetch(bayer + bayer_step + 16 * 2 * 2);
#endif
			vstrbq_scatter_offset_p(rgb - blue, inc, rg.val[0], p);
			uint8x16_t g0 = vrhaddq_x(rg.val[1], gb.val[0], p);

			vstrbq_scatter_offset_p(rgb, inc, g0, p);
			vstrbq_scatter_offset_p(rgb + blue, inc, gb.val[1], p);

			vstrbq_scatter_offset_p(rgb + rgb_bytes - blue, inc, gr.val[1], p);
			uint8x16_t g1 = vrhaddq_x(gr.val[0], bg.val[1], p);

			vstrbq_scatter_offset_p(rgb + rgb_bytes, inc, g1, p);
			vstrbq_scatter_offset_p(rgb + rgb_bytes + blue, inc, bg.val[0], p);

			bayer += 16 * 2;
			rgb += 16 * 2 * rgb_bytes;
			pairs_to_go -= 16;
		}

		bayer += pairs_to_go * 2;
		rgb += pairs_to_go * 2 * rgb_bytes;

		if (bayer < bayer_end) {
			rgb[-blue] = bayer[0];
			rgb[0] = (bayer[1] + bayer[bayer_step] + 1) >> 1;
			rgb[blue] = bayer[bayer_step + 1];

			bayer++;
			rgb += rgb_bytes;
		}

		bayer -= width;
		rgb -= width * rgb_bytes;

		blue = -blue;
		start_with_green = !start_with_green;
	}

	return AIPL_ERR_OK;
}

aipl_error_t aipl_bayer_decoding_simple_argb8888(const uint8_t *restrict bayer,
						 uint8_t *restrict rgb, int width, int height,
						 int tile)
{
	if ((tile > AIPL_COLOR_FILTER_MAX) || (tile < AIPL_COLOR_FILTER_MIN)) {
		return AIPL_ERR_BAYER_INVALID_FILTER;
	}

	const int rgb_bytes = 4;
	const int bayer_step = width;
	const int rgb_step = rgb_bytes * width;
	int blue = tile == AIPL_COLOR_FILTER_BGGR || tile == AIPL_COLOR_FILTER_GBRG ? -1 : 1;
	int start_with_green = tile == AIPL_COLOR_FILTER_GBRG || tile == AIPL_COLOR_FILTER_GRBG;
	int i;

	/* Add black border */
	int i_max = width * height * rgb_bytes;

	for (i = width * (height - 1) * rgb_bytes; i < i_max; i++) {
		rgb[i] = 0;
	}
	int i_inc = (width - 1) * rgb_bytes;

	for (i = (width - 1) * rgb_bytes; i < i_max; i += i_inc) {
		int j;

		for (j = 0; j < rgb_bytes; j++) {
			rgb[i++] = 0;
		}
	}

	rgb += 1;
	width -= 1;
	height -= 1;

	/* Index table into 16 RGB pairs for scatter stores: { 0, 6, 12, .. } */
	const uint8x16_t inc = vmulq_n_u8(vidupq_n_u8(0, 1), 2 * rgb_bytes);

	for (; height--; bayer += bayer_step, rgb += rgb_step) {
		const uint8_t *bayer_end = bayer + width;

		if (start_with_green) {
			rgb[-blue] = bayer[bayer_step];
			rgb[0] = (bayer[0] + bayer[bayer_step + 1] + 1) >> 1;
			rgb[blue] = bayer[1];
			rgb[2] = 0xff;

			bayer++;
			rgb += rgb_bytes;
		}

		/* Helium lets us process 16 at a time (8 per beat on Cortex-M55) */
		int pairs_to_go = (bayer_end - bayer) / 2;

		while (pairs_to_go > 0) {
			mve_pred16_t p = vctp8q(pairs_to_go);
			uint8x16x2_t rg = vld2q(bayer);
			uint8x16x2_t gr = vld2q(bayer + 1);
			uint8x16x2_t gb = vld2q(bayer + bayer_step);
			uint8x16x2_t bg = vld2q(bayer + bayer_step + 1);
			uint8x16_t a = vcreateq_u8(0xffffffffffffffff, 0xffffffffffffffff);
#ifndef __ICCARM__
			__builtin_prefetch(bayer + bayer_step + 16 * 2 * 2);
#endif
			vstrbq_scatter_offset_p(rgb - blue, inc, gb.val[1], p);
			uint8x16_t g0 = vrhaddq_x(rg.val[1], gb.val[0], p);

			vstrbq_scatter_offset_p(rgb, inc, g0, p);
			vstrbq_scatter_offset_p(rgb + blue, inc, rg.val[0], p);
			vstrbq_scatter_offset_p(rgb + 2, inc, a, p);

			vstrbq_scatter_offset_p(rgb + rgb_bytes - blue, inc, bg.val[0], p);
			uint8x16_t g1 = vrhaddq_x(gr.val[0], bg.val[1], p);

			vstrbq_scatter_offset_p(rgb + rgb_bytes, inc, g1, p);
			vstrbq_scatter_offset_p(rgb + rgb_bytes + blue, inc, gr.val[1], p);
			vstrbq_scatter_offset_p(rgb + rgb_bytes + 2, inc, a, p);

			bayer += 16 * 2;
			rgb += 16 * 2 * rgb_bytes;
			pairs_to_go -= 16;
		}

		bayer += pairs_to_go * 2;
		rgb += pairs_to_go * 2 * rgb_bytes;

		if (bayer < bayer_end) {
			rgb[-blue] = bayer[bayer_step + 1];
			rgb[0] = (bayer[1] + bayer[bayer_step] + 1) >> 1;
			rgb[blue] = bayer[0];
			rgb[2] = 0xff;

			bayer++;
			rgb += rgb_bytes;
		}

		bayer -= width;
		rgb -= width * rgb_bytes;

		blue = -blue;
		start_with_green = !start_with_green;
	}

	return AIPL_ERR_OK;
}

aipl_error_t aipl_bayer_decoding_linear_rgb565(const uint8_t *restrict bayer, uint8_t *restrict rgb,
					       int width, int height, int tile)
{
	if ((tile > AIPL_COLOR_FILTER_MAX) || (tile < AIPL_COLOR_FILTER_MIN)) {
		return AIPL_ERR_BAYER_INVALID_FILTER;
	}

	const int rgb_bytes = 2;
	const int bayer_step = width;
	const int rgb_step = rgb_bytes * width;
	int blue = tile == AIPL_COLOR_FILTER_BGGR || tile == AIPL_COLOR_FILTER_GBRG ? -1 : 1;
	int start_with_green = tile == AIPL_COLOR_FILTER_GBRG || tile == AIPL_COLOR_FILTER_GRBG;

	aipl_bayer_aipl_bayer_clear_borders(rgb, width, height, 1, rgb_bytes);
	rgb += rgb_step + rgb_bytes;
	height -= 2;
	width -= 2;

	/* Index table into 16 RGB pairs for scatter stores: { 0, 6, 12, .. } */
	const uint8x16_t inc = vmulq_n_u8(vidupq_n_u8(0, 1), 2 * rgb_bytes);

	for (; height--; bayer += bayer_step, rgb += rgb_step) {
		int t0, t1;
		const uint8_t *bayer_end = bayer + width;

		if (start_with_green) {
			t0 = (bayer[1] + bayer[bayer_step * 2 + 1] + 1) >> 1;
			t1 = (bayer[bayer_step] + bayer[bayer_step + 2] + 1) >> 1;
			if (blue > 0) {
				rgb[0] = ((bayer[bayer_step + 1] & 0xfc) << 3) | ((uint8_t)t1 >> 3);
				rgb[1] = ((uint8_t)t0 & 0xf8) | (bayer[bayer_step + 1] >> 5);
			} else {
				rgb[0] = ((bayer[bayer_step + 1] & 0xfc) << 3) | ((uint8_t)t0 >> 3);
				rgb[1] = ((uint8_t)t1 & 0xf8) | (bayer[bayer_step + 1] >> 5);
			}

			bayer++;
			rgb += rgb_bytes;
		}

		/* Helium lets us process 16 at a time (8 per beat on Cortex-M55) */
		int pairs_to_go = (bayer_end - bayer) / 2;

		while (pairs_to_go > 0) {
			mve_pred16_t p = vctp8q(pairs_to_go);
			uint8x16x2_t rg0 = vld2q(bayer);
			uint8x16x2_t gr0 = vld2q(bayer + 1);
			uint8x16x2_t gb0 = vld2q(bayer + bayer_step);
			uint8x16x2_t gb1 = vld2q(bayer + bayer_step + 2);
			uint8x16x2_t rg1 = vld2q(bayer + bayer_step * 2);
			uint8x16x2_t gr1 = vld2q(bayer + bayer_step * 2 + 1);

			uint8x16_t tmp0 = vrhaddq_x(rg0.val[0], gr0.val[1], p);
			uint8x16_t tmp1 = vrhaddq_x(rg1.val[0], gr1.val[1], p);
			uint8x16_t r0 = vrhaddq_x(tmp0, tmp1, p);

			uint8x16_t tmp2 = vrhaddq_x(rg0.val[1], gb0.val[0], p);
			uint8x16_t tmp3 = vrhaddq_x(gb1.val[0], rg1.val[1], p);
			uint8x16_t g0 = vrhaddq_x(tmp2, tmp3, p);

			uint8x16_t b0 = gb0.val[1];

			uint8x16_t upper;
			uint8x16_t lower;

			lower = vshlq_n(g0, 3);
			if (blue > 0) {
				lower = vsriq(lower, b0, 3);
				upper = vsriq(r0, g0, 5);
			} else {
				lower = vsriq(lower, r0, 3);
				upper = vsriq(b0, g0, 5);
			}
			vstrbq_scatter_offset_p(rgb, inc, lower, p);
			vstrbq_scatter_offset_p(rgb + 1, inc, upper, p);

			uint8x16_t r1 = vrhaddq_x(gr0.val[1], gr1.val[1], p);
			uint8x16_t g1 = gb1.val[0];
			uint8x16_t b1 = vrhaddq_x(gb0.val[1], gb1.val[1], p);

			lower = vshlq_n(g1, 3);
			if (blue > 0) {
				lower = vsriq(lower, b1, 3);
				upper = vsriq(r1, g1, 5);
			} else {
				lower = vsriq(lower, r1, 3);
				upper = vsriq(b1, g1, 5);
			}
			vstrbq_scatter_offset_p(rgb + rgb_bytes, inc, lower, p);
			vstrbq_scatter_offset_p(rgb + rgb_bytes + 1, inc, upper, p);

			bayer += 16 * 2;
			rgb += 16 * 2 * rgb_bytes;
			pairs_to_go -= 16;
		}

		bayer += pairs_to_go * 2;
		rgb += pairs_to_go * 2 * rgb_bytes;

		if (bayer < bayer_end) {
			t0 = (bayer[0] + bayer[2] + bayer[bayer_step * 2] +
			      bayer[bayer_step * 2 + 2] + 2) >>
			     2;
			t1 = (bayer[1] + bayer[bayer_step] + bayer[bayer_step + 2] +
			      bayer[bayer_step * 2 + 1] + 2) >>
			     2;
			if (blue > 0) {
				rgb[0] = (((uint8_t)t1 & 0xfc) << 3) | (bayer[bayer_step + 1] >> 3);
				rgb[1] = ((uint8_t)t0 & 0xf8) | ((uint8_t)t1 >> 5);
			} else {
				rgb[0] = (((uint8_t)t1 & 0xfc) << 3) | ((uint8_t)t0 >> 3);
				rgb[1] = (bayer[bayer_step + 1] & 0xf8) | ((uint8_t)t1 >> 5);
			}

			bayer++;
			rgb += rgb_bytes;
		}

		bayer -= width;
		rgb -= width * rgb_bytes;

		blue = -blue;
		start_with_green = !start_with_green;
	}

	return AIPL_ERR_OK;
}

aipl_error_t aipl_bayer_decoding_linear_bgr888(const uint8_t *restrict bayer, uint8_t *restrict rgb,
					       int width, int height, int tile)
{
	if ((tile > AIPL_COLOR_FILTER_MAX) || (tile < AIPL_COLOR_FILTER_MIN)) {
		return AIPL_ERR_BAYER_INVALID_FILTER;
	}

	const int rgb_bytes = 3;
	const int bayer_step = width;
	const int rgb_step = rgb_bytes * width;
	int blue = tile == AIPL_COLOR_FILTER_BGGR || tile == AIPL_COLOR_FILTER_GBRG ? -1 : 1;
	int start_with_green = tile == AIPL_COLOR_FILTER_GBRG || tile == AIPL_COLOR_FILTER_GRBG;

	aipl_bayer_aipl_bayer_clear_borders(rgb, width, height, 1, rgb_bytes);
	rgb += rgb_step + rgb_bytes + 1;
	height -= 2;
	width -= 2;

	/* Index table into 16 RGB pairs for scatter stores: { 0, 6, 12, .. } */
	const uint8x16_t inc = vmulq_n_u8(vidupq_n_u8(0, 1), 2 * rgb_bytes);

	for (; height--; bayer += bayer_step, rgb += rgb_step) {
		int t0, t1;
		const uint8_t *bayer_end = bayer + width;

		if (start_with_green) {
			t0 = (bayer[1] + bayer[bayer_step * 2 + 1] + 1) >> 1;
			t1 = (bayer[bayer_step] + bayer[bayer_step + 2] + 1) >> 1;
			rgb[-blue] = (uint8_t)t1;
			rgb[0] = bayer[bayer_step + 1];
			rgb[blue] = (uint8_t)t0;

			bayer++;
			rgb += rgb_bytes;
		}

		/* Helium lets us process 16 at a time (8 per beat on Cortex-M55) */
		int pairs_to_go = (bayer_end - bayer) / 2;

		while (pairs_to_go > 0) {
			mve_pred16_t p = vctp8q(pairs_to_go);
			uint8x16x2_t rg0 = vld2q(bayer);
			uint8x16x2_t gr0 = vld2q(bayer + 1);
			uint8x16x2_t gb0 = vld2q(bayer + bayer_step);
			uint8x16x2_t gb1 = vld2q(bayer + bayer_step + 2);
			uint8x16x2_t rg1 = vld2q(bayer + bayer_step * 2);
			uint8x16x2_t gr1 = vld2q(bayer + bayer_step * 2 + 1);

			uint8x16_t b0 = gb0.val[1];

			vstrbq_scatter_offset_p(rgb - blue, inc, b0, p);

			uint8x16_t tmp2 = vrhaddq_x(rg0.val[1], gb0.val[0], p);
			uint8x16_t tmp3 = vrhaddq_x(gb1.val[0], rg1.val[1], p);
			uint8x16_t g0 = vrhaddq_x(tmp2, tmp3, p);

			vstrbq_scatter_offset_p(rgb, inc, g0, p);

			uint8x16_t tmp0 = vrhaddq_x(rg0.val[0], gr0.val[1], p);
			uint8x16_t tmp1 = vrhaddq_x(rg1.val[0], gr1.val[1], p);
			uint8x16_t r0 = vrhaddq_x(tmp0, tmp1, p);

			vstrbq_scatter_offset_p(rgb + blue, inc, r0, p);

			uint8x16_t b1 = vrhaddq_x(gb0.val[1], gb1.val[1], p);

			vstrbq_scatter_offset_p(rgb + rgb_bytes - blue, inc, b1, p);

			uint8x16_t g1 = gb1.val[0];

			vstrbq_scatter_offset_p(rgb + rgb_bytes, inc, g1, p);

			uint8x16_t r1 = vrhaddq_x(gr0.val[1], gr1.val[1], p);

			vstrbq_scatter_offset_p(rgb + rgb_bytes + blue, inc, r1, p);

			bayer += 16 * 2;
			rgb += 16 * 2 * rgb_bytes;
			pairs_to_go -= 16;
		}

		bayer += pairs_to_go * 2;
		rgb += pairs_to_go * 2 * rgb_bytes;

		if (bayer < bayer_end) {
			t0 = (bayer[0] + bayer[2] + bayer[bayer_step * 2] +
			      bayer[bayer_step * 2 + 2] + 2) >>
			     2;
			t1 = (bayer[1] + bayer[bayer_step] + bayer[bayer_step + 2] +
			      bayer[bayer_step * 2 + 1] + 2) >>
			     2;
			rgb[-blue] = bayer[bayer_step + 1];
			rgb[0] = (uint8_t)t1;
			rgb[blue] = (uint8_t)t0;

			bayer++;
			rgb += rgb_bytes;
		}

		bayer -= width;
		rgb -= width * rgb_bytes;

		blue = -blue;
		start_with_green = !start_with_green;
	}

	return AIPL_ERR_OK;
}

aipl_error_t aipl_bayer_decoding_linear_rgb888(const uint8_t *restrict bayer, uint8_t *restrict rgb,
					       int width, int height, int tile)
{
	if ((tile > AIPL_COLOR_FILTER_MAX) || (tile < AIPL_COLOR_FILTER_MIN)) {
		return AIPL_ERR_BAYER_INVALID_FILTER;
	}

	const int rgb_bytes = 3;
	const int bayer_step = width;
	const int rgb_step = rgb_bytes * width;
	int blue = tile == AIPL_COLOR_FILTER_BGGR || tile == AIPL_COLOR_FILTER_GBRG ? -1 : 1;
	int start_with_green = tile == AIPL_COLOR_FILTER_GBRG || tile == AIPL_COLOR_FILTER_GRBG;

	aipl_bayer_aipl_bayer_clear_borders(rgb, width, height, 1, rgb_bytes);
	rgb += rgb_step + rgb_bytes + 1;
	height -= 2;
	width -= 2;

	/* Index table into 16 RGB pairs for scatter stores: { 0, 6, 12, .. } */
	const uint8x16_t inc = vmulq_n_u8(vidupq_n_u8(0, 1), 2 * rgb_bytes);

	for (; height--; bayer += bayer_step, rgb += rgb_step) {
		int t0, t1;
		const uint8_t *bayer_end = bayer + width;

		if (start_with_green) {
			t0 = (bayer[1] + bayer[bayer_step * 2 + 1] + 1) >> 1;
			t1 = (bayer[bayer_step] + bayer[bayer_step + 2] + 1) >> 1;
			rgb[-blue] = (uint8_t)t0;
			rgb[0] = bayer[bayer_step + 1];
			rgb[blue] = (uint8_t)t1;

			bayer++;
			rgb += rgb_bytes;
		}

		/* Helium lets us process 16 at a time (8 per beat on Cortex-M55) */
		int pairs_to_go = (bayer_end - bayer) / 2;

		while (pairs_to_go > 0) {
			mve_pred16_t p = vctp8q(pairs_to_go);
			uint8x16x2_t rg0 = vld2q(bayer);
			uint8x16x2_t gr0 = vld2q(bayer + 1);
			uint8x16x2_t gb0 = vld2q(bayer + bayer_step);
			uint8x16x2_t gb1 = vld2q(bayer + bayer_step + 2);
			uint8x16x2_t rg1 = vld2q(bayer + bayer_step * 2);
			uint8x16x2_t gr1 = vld2q(bayer + bayer_step * 2 + 1);

			uint8x16_t tmp0 = vrhaddq_x(rg0.val[0], gr0.val[1], p);
			uint8x16_t tmp1 = vrhaddq_x(rg1.val[0], gr1.val[1], p);
			uint8x16_t r0 = vrhaddq_x(tmp0, tmp1, p);

			vstrbq_scatter_offset_p(rgb - blue, inc, r0, p);

			uint8x16_t tmp2 = vrhaddq_x(rg0.val[1], gb0.val[0], p);
			uint8x16_t tmp3 = vrhaddq_x(gb1.val[0], rg1.val[1], p);
			uint8x16_t g0 = vrhaddq_x(tmp2, tmp3, p);

			vstrbq_scatter_offset_p(rgb, inc, g0, p);

			uint8x16_t b0 = gb0.val[1];

			vstrbq_scatter_offset_p(rgb + blue, inc, b0, p);

			uint8x16_t r1 = vrhaddq_x(gr0.val[1], gr1.val[1], p);

			vstrbq_scatter_offset_p(rgb + rgb_bytes - blue, inc, r1, p);

			uint8x16_t g1 = gb1.val[0];

			vstrbq_scatter_offset_p(rgb + rgb_bytes, inc, g1, p);

			uint8x16_t b1 = vrhaddq_x(gb0.val[1], gb1.val[1], p);

			vstrbq_scatter_offset_p(rgb + rgb_bytes + blue, inc, b1, p);

			bayer += 16 * 2;
			rgb += 16 * 2 * rgb_bytes;
			pairs_to_go -= 16;
		}

		bayer += pairs_to_go * 2;
		rgb += pairs_to_go * 2 * rgb_bytes;

		if (bayer < bayer_end) {
			t0 = (bayer[0] + bayer[2] + bayer[bayer_step * 2] +
			      bayer[bayer_step * 2 + 2] + 2) >>
			     2;
			t1 = (bayer[1] + bayer[bayer_step] + bayer[bayer_step + 2] +
			      bayer[bayer_step * 2 + 1] + 2) >>
			     2;
			rgb[-blue] = (uint8_t)t0;
			rgb[0] = (uint8_t)t1;
			rgb[blue] = bayer[bayer_step + 1];

			bayer++;
			rgb += rgb_bytes;
		}

		bayer -= width;
		rgb -= width * rgb_bytes;

		blue = -blue;
		start_with_green = !start_with_green;
	}

	return AIPL_ERR_OK;
}

aipl_error_t aipl_bayer_decoding_linear_argb8888(const uint8_t *restrict bayer,
						 uint8_t *restrict rgb, int width, int height,
						 int tile)
{
	if ((tile > AIPL_COLOR_FILTER_MAX) || (tile < AIPL_COLOR_FILTER_MIN)) {
		return AIPL_ERR_BAYER_INVALID_FILTER;
	}

	const int rgb_bytes = 4;
	const int bayer_step = width;
	const int rgb_step = rgb_bytes * width;
	int blue = tile == AIPL_COLOR_FILTER_BGGR || tile == AIPL_COLOR_FILTER_GBRG ? -1 : 1;
	int start_with_green = tile == AIPL_COLOR_FILTER_GBRG || tile == AIPL_COLOR_FILTER_GRBG;

	aipl_bayer_aipl_bayer_clear_borders(rgb, width, height, 1, rgb_bytes);
	rgb += rgb_step + rgb_bytes + 1;
	height -= 2;
	width -= 2;

	/* Index table into 16 RGB pairs for scatter stores: { 0, 6, 12, .. } */
	const uint8x16_t inc = vmulq_n_u8(vidupq_n_u8(0, 1), 2 * rgb_bytes);

	for (; height--; bayer += bayer_step, rgb += rgb_step) {
		int t0, t1;
		const uint8_t *bayer_end = bayer + width;

		if (start_with_green) {
			t0 = (bayer[1] + bayer[bayer_step * 2 + 1] + 1) >> 1;
			t1 = (bayer[bayer_step] + bayer[bayer_step + 2] + 1) >> 1;
			rgb[-blue] = (uint8_t)t1;
			rgb[0] = bayer[bayer_step + 1];
			rgb[blue] = (uint8_t)t0;
			rgb[2] = 0xff;

			bayer++;
			rgb += rgb_bytes;
		}

		/* Helium lets us process 16 at a time (8 per beat on Cortex-M55) */
		int pairs_to_go = (bayer_end - bayer) / 2;

		while (pairs_to_go > 0) {
			mve_pred16_t p = vctp8q(pairs_to_go);
			uint8x16x2_t rg0 = vld2q(bayer);
			uint8x16x2_t gr0 = vld2q(bayer + 1);
			uint8x16x2_t gb0 = vld2q(bayer + bayer_step);
			uint8x16x2_t gb1 = vld2q(bayer + bayer_step + 2);
			uint8x16x2_t rg1 = vld2q(bayer + bayer_step * 2);
			uint8x16x2_t gr1 = vld2q(bayer + bayer_step * 2 + 1);
			uint8x16_t a = vcreateq_u8(0xffffffffffffffff, 0xffffffffffffffff);

			uint8x16_t b0 = gb0.val[1];

			vstrbq_scatter_offset_p(rgb - blue, inc, b0, p);

			uint8x16_t tmp2 = vrhaddq_x(rg0.val[1], gb0.val[0], p);
			uint8x16_t tmp3 = vrhaddq_x(gb1.val[0], rg1.val[1], p);
			uint8x16_t g0 = vrhaddq_x(tmp2, tmp3, p);

			vstrbq_scatter_offset_p(rgb, inc, g0, p);

			uint8x16_t tmp0 = vrhaddq_x(rg0.val[0], gr0.val[1], p);
			uint8x16_t tmp1 = vrhaddq_x(rg1.val[0], gr1.val[1], p);
			uint8x16_t r0 = vrhaddq_x(tmp0, tmp1, p);

			vstrbq_scatter_offset_p(rgb + blue, inc, r0, p);

			vstrbq_scatter_offset_p(rgb + 2, inc, a, p);

			uint8x16_t b1 = vrhaddq_x(gb0.val[1], gb1.val[1], p);

			vstrbq_scatter_offset_p(rgb + rgb_bytes - blue, inc, b1, p);

			uint8x16_t g1 = gb1.val[0];

			vstrbq_scatter_offset_p(rgb + rgb_bytes, inc, g1, p);

			uint8x16_t r1 = vrhaddq_x(gr0.val[1], gr1.val[1], p);

			vstrbq_scatter_offset_p(rgb + rgb_bytes + blue, inc, r1, p);

			vstrbq_scatter_offset_p(rgb + rgb_bytes + 2, inc, a, p);

			bayer += 16 * 2;
			rgb += 16 * 2 * rgb_bytes;
			pairs_to_go -= 16;
		}

		bayer += pairs_to_go * 2;
		rgb += pairs_to_go * 2 * rgb_bytes;

		if (bayer < bayer_end) {
			t0 = (bayer[0] + bayer[2] + bayer[bayer_step * 2] +
			      bayer[bayer_step * 2 + 2] + 2) >>
			     2;
			t1 = (bayer[1] + bayer[bayer_step] + bayer[bayer_step + 2] +
			      bayer[bayer_step * 2 + 1] + 2) >>
			     2;
			rgb[-blue] = bayer[bayer_step + 1];
			rgb[0] = (uint8_t)t1;
			rgb[blue] = (uint8_t)t0;
			rgb[2] = 0xff;

			bayer++;
			rgb += rgb_bytes;
		}

		bayer -= width;
		rgb -= width * rgb_bytes;

		blue = -blue;
		start_with_green = !start_with_green;
	}

	return AIPL_ERR_OK;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void aipl_bayer_aipl_bayer_clear_borders(uint8_t *rgb, int width, int height, int w,
						int rgb_bytes)
{
	int i, j;
	/* black edges are added with a width w: */
	i = rgb_bytes * width * w - 1;
	j = rgb_bytes * width * height - 1;
	while (i >= 0) {
		rgb[i--] = 0;
		rgb[j--] = 0;
	}

	int low = width * (w - 1) * rgb_bytes - 1 + w * rgb_bytes;

	i = low + width * (height - w * 2 + 1) * rgb_bytes;
	while (i > low) {
		j = 2 * rgb_bytes * w;
		while (j > 0) {
			rgb[i--] = 0;
			j--;
		}
		i -= (width - 2 * w) * rgb_bytes;
	}
}

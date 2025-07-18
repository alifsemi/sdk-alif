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
 * @file    aipl_bayer.c
 * @brief   Bayer pattern decoding function implementations
 *
 ******************************************************************************/

/*********************
 *      INCLUDES
 *********************/
#include "aipl_bayer.h"

#include "aipl_bayer_default.h"
#include "aipl_bayer_helium.h"

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
aipl_bayer_tile_t aipl_bayer_tile(const void *input, uint32_t pitch, aipl_bayer_filter_t filter)
{
	aipl_bayer_tile_t tile;

	switch (filter) {
	case AIPL_BAYER_RGGB:
		tile.red_row = 0;
		tile.blue_row = 1;
		tile.red_col = 0;
		tile.blue_col = 1;
		tile.red_src = input;
		tile.blue_src = tile.red_src + pitch;
		break;

	case AIPL_BAYER_GBRG:
		tile.red_row = 1;
		tile.blue_row = 0;
		tile.red_col = 0;
		tile.blue_col = 1;
		tile.blue_src = input;
		tile.red_src = tile.blue_src + pitch;
		break;

	case AIPL_BAYER_GRBG:
		tile.red_row = 0;
		tile.blue_row = 1;
		tile.red_col = 1;
		tile.blue_col = 0;
		tile.red_src = input;
		tile.blue_src = tile.red_src + pitch;
		break;

	default:
		tile.red_row = 1;
		tile.blue_row = 0;
		tile.red_col = 1;
		tile.blue_col = 0;
		tile.blue_src = input;
		tile.red_src = tile.blue_src + pitch;
	}

	return tile;
}

aipl_bayer_filter_t aipl_bayer_filter_swap_rb(aipl_bayer_filter_t filter)
{
	switch (filter) {
	case AIPL_BAYER_RGGB:
		return AIPL_BAYER_BGGR;
	case AIPL_BAYER_GBRG:
		return AIPL_BAYER_GRBG;
	case AIPL_BAYER_GRBG:
		return AIPL_BAYER_GBRG;
	default:
		return AIPL_BAYER_RGGB;
	}
}

aipl_error_t aipl_bayer_decoding(const void *input, void *output, uint32_t pitch, uint32_t width,
				 uint32_t height, aipl_bayer_filter_t filter,
				 aipl_color_format_t format)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_bayer_decoding_helium(input, output, pitch, width, height, filter, format);
#else
	return aipl_bayer_decoding_default(input, output, pitch, width, height, filter, format);
#endif
}

aipl_error_t aipl_bayer_decoding_img(const void *input, aipl_image_t *output, uint32_t pitch,
				     aipl_bayer_filter_t filter)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_bayer_decoding_img_helium(input, output, pitch, filter);
#else
	return aipl_bayer_decoding_img_default(input, output, pitch, filter);
#endif
}

aipl_error_t aipl_bayer_decoding_alpha8(const void *input, void *output, uint32_t pitch,
					uint32_t width, uint32_t height, aipl_bayer_filter_t filter)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_bayer_decoding_alpha8_helium(input, output, pitch, width, height, filter);
#else
	return aipl_bayer_decoding_alpha8_default(input, output, pitch, width, height, filter);
#endif
}

aipl_error_t aipl_bayer_decoding_argb8888(const void *input, void *output, uint32_t pitch,
					  uint32_t width, uint32_t height,
					  aipl_bayer_filter_t filter)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_bayer_decoding_argb8888_helium(input, output, pitch, width, height, filter);
#else
	return aipl_bayer_decoding_argb8888_default(input, output, pitch, width, height, filter);
#endif
}

aipl_error_t aipl_bayer_decoding_argb4444(const void *input, void *output, uint32_t pitch,
					  uint32_t width, uint32_t height,
					  aipl_bayer_filter_t filter)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_bayer_decoding_argb4444_helium(input, output, pitch, width, height, filter);
#else
	return aipl_bayer_decoding_argb4444_default(input, output, pitch, width, height, filter);
#endif
}

aipl_error_t aipl_bayer_decoding_argb1555(const void *input, void *output, uint32_t pitch,
					  uint32_t width, uint32_t height,
					  aipl_bayer_filter_t filter)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_bayer_decoding_argb1555_helium(input, output, pitch, width, height, filter);
#else
	return aipl_bayer_decoding_argb1555_default(input, output, pitch, width, height, filter);
#endif
}

aipl_error_t aipl_bayer_decoding_rgba8888(const void *input, void *output, uint32_t pitch,
					  uint32_t width, uint32_t height,
					  aipl_bayer_filter_t filter)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_bayer_decoding_rgba8888_helium(input, output, pitch, width, height, filter);
#else
	return aipl_bayer_decoding_rgba8888_default(input, output, pitch, width, height, filter);
#endif
}

aipl_error_t aipl_bayer_decoding_rgba4444(const void *input, void *output, uint32_t pitch,
					  uint32_t width, uint32_t height,
					  aipl_bayer_filter_t filter)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_bayer_decoding_rgba4444_helium(input, output, pitch, width, height, filter);
#else
	return aipl_bayer_decoding_rgba4444_default(input, output, pitch, width, height, filter);
#endif
}

aipl_error_t aipl_bayer_decoding_rgba5551(const void *input, void *output, uint32_t pitch,
					  uint32_t width, uint32_t height,
					  aipl_bayer_filter_t filter)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_bayer_decoding_rgba5551_helium(input, output, pitch, width, height, filter);
#else
	return aipl_bayer_decoding_rgba5551_default(input, output, pitch, width, height, filter);
#endif
}

aipl_error_t aipl_bayer_decoding_rgb888(const void *input, void *output, uint32_t pitch,
					uint32_t width, uint32_t height, aipl_bayer_filter_t filter)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_bayer_decoding_rgb888_helium(input, output, pitch, width, height, filter);
#else
	return aipl_bayer_decoding_rgb888_default(input, output, pitch, width, height, filter);
#endif
}

aipl_error_t aipl_bayer_decoding_bgr888(const void *input, void *output, uint32_t pitch,
					uint32_t width, uint32_t height, aipl_bayer_filter_t filter)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_bayer_decoding_bgr888_helium(input, output, pitch, width, height, filter);
#else
	return aipl_bayer_decoding_bgr888_default(input, output, pitch, width, height, filter);
#endif
}

aipl_error_t aipl_bayer_decoding_rgb565(const void *input, void *output, uint32_t pitch,
					uint32_t width, uint32_t height, aipl_bayer_filter_t filter)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_bayer_decoding_rgb565_helium(input, output, pitch, width, height, filter);
#else
	return aipl_bayer_decoding_rgb565_default(input, output, pitch, width, height, filter);
#endif
}

aipl_error_t aipl_bayer_decoding_yv12(const void *input, void *output, uint32_t pitch,
				      uint32_t width, uint32_t height, aipl_bayer_filter_t filter)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_bayer_decoding_yv12_helium(input, output, pitch, width, height, filter);
#else
	return aipl_bayer_decoding_yv12_default(input, output, pitch, width, height, filter);
#endif
}

aipl_error_t aipl_bayer_decoding_i420(const void *input, void *output, uint32_t pitch,
				      uint32_t width, uint32_t height, aipl_bayer_filter_t filter)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_bayer_decoding_i420_helium(input, output, pitch, width, height, filter);
#else
	return aipl_bayer_decoding_i420_default(input, output, pitch, width, height, filter);
#endif
}

aipl_error_t aipl_bayer_decoding_i422(const void *input, void *output, uint32_t pitch,
				      uint32_t width, uint32_t height, aipl_bayer_filter_t filter)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_bayer_decoding_i422_helium(input, output, pitch, width, height, filter);
#else
	return aipl_bayer_decoding_i422_default(input, output, pitch, width, height, filter);
#endif
}

aipl_error_t aipl_bayer_decoding_i444(const void *input, void *output, uint32_t pitch,
				      uint32_t width, uint32_t height, aipl_bayer_filter_t filter)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_bayer_decoding_i444_helium(input, output, pitch, width, height, filter);
#else
	return aipl_bayer_decoding_i444_default(input, output, pitch, width, height, filter);
#endif
}

aipl_error_t aipl_bayer_decoding_i400(const void *input, void *output, uint32_t pitch,
				      uint32_t width, uint32_t height, aipl_bayer_filter_t filter)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_bayer_decoding_i400_helium(input, output, pitch, width, height, filter);
#else
	return aipl_bayer_decoding_i400_default(input, output, pitch, width, height, filter);
#endif
}

aipl_error_t aipl_bayer_decoding_nv12(const void *input, void *output, uint32_t pitch,
				      uint32_t width, uint32_t height, aipl_bayer_filter_t filter)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_bayer_decoding_nv12_helium(input, output, pitch, width, height, filter);
#else
	return aipl_bayer_decoding_nv12_default(input, output, pitch, width, height, filter);
#endif
}

aipl_error_t aipl_bayer_decoding_nv21(const void *input, void *output, uint32_t pitch,
				      uint32_t width, uint32_t height, aipl_bayer_filter_t filter)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_bayer_decoding_nv21_helium(input, output, pitch, width, height, filter);
#else
	return aipl_bayer_decoding_nv21_default(input, output, pitch, width, height, filter);
#endif
}

aipl_error_t aipl_bayer_decoding_yuy2(const void *input, void *output, uint32_t pitch,
				      uint32_t width, uint32_t height, aipl_bayer_filter_t filter)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_bayer_decoding_yuy2_helium(input, output, pitch, width, height, filter);
#else
	return aipl_bayer_decoding_yuy2_default(input, output, pitch, width, height, filter);
#endif
}

aipl_error_t aipl_bayer_decoding_uyvy(const void *input, void *output, uint32_t pitch,
				      uint32_t width, uint32_t height, aipl_bayer_filter_t filter)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_bayer_decoding_uyvy_helium(input, output, pitch, width, height, filter);
#else
	return aipl_bayer_decoding_uyvy_default(input, output, pitch, width, height, filter);
#endif
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

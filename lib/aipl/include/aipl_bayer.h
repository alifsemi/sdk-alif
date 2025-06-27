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
 * @file    aipl_bayer.h
 * @brief   Bayer pattern decoding function definitions
 *
 ******************************************************************************/

#ifndef AIPL_BAYER_H
#define AIPL_BAYER_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "aipl_color_formats.h"
#include "aipl_error.h"
#include "aipl_image.h"

/**********************
 *      TYPEDEFS
 **********************/

/**
 * A list of supported de-mosaicing techniques for Bayer-patterns.
 */
typedef enum {
	AIPL_BAYER_METHOD_NEAREST = 0,
	AIPL_BAYER_METHOD_SIMPLE,
	AIPL_BAYER_METHOD_BILINEAR
} aipl_bayer_method_t;

/**
 * Color filter tiles
 */
typedef enum {
	AIPL_COLOR_FILTER_RGGB = 512,
	AIPL_COLOR_FILTER_GBRG,
	AIPL_COLOR_FILTER_GRBG,
	AIPL_COLOR_FILTER_BGGR
} aipl_color_filter_t;

/*********************
 *      DEFINES
 *********************/
#define AIPL_COLOR_FILTER_MIN AIPL_COLOR_FILTER_RGGB
#define AIPL_COLOR_FILTER_MAX AIPL_COLOR_FILTER_BGGR
#define AIPL_COLOR_FILTER_NUM (AIPL_COLOR_FILTER_MAX - AIPL_COLOR_FILTER_MIN + 1)

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Perform de-mosaicing on an 8-bit raw image buffer
 *
 * @param bayer             input raw image pointer
 * @param output            output RGB image pointer
 * @param width             image width
 * @param height            image height
 * @param tile              color filter tiles
 * @param method            de-mosaicing techniques
 * @param format            output color format
 * @return error code
 */
aipl_error_t aipl_bayer_decoding(const void *bayer, void *output, uint32_t width, uint32_t height,
				 aipl_color_filter_t tile, aipl_bayer_method_t method,
				 aipl_color_format_t format);

/**
 * Perform de-mosaicing on an 8-bit raw image buffer
 *
 * @param bayer             input raw image pointer
 * @param output            output image
 * @param tile              color filter tiles
 * @param method            de-mosaicing techniques
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_img(const void *bayer, aipl_image_t *output,
				     aipl_color_filter_t tile, aipl_bayer_method_t method);

/**
 * Perform de-mosaicing on an 8-bit raw image buffer
 * into RGB565 image using nearest inetrpolation method
 *
 * @param bayer             input raw image pointer
 * @param output            output RGB image pointer
 * @param width             image width
 * @param height            image height
 * @param tile              color filter tiles
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_nearest_rgb565(const uint8_t *restrict bayer,
						uint8_t *restrict rgb, int sx, int sy, int tile);

/**
 * Perform de-mosaicing on an 8-bit raw image buffer
 * into RGB888 image using nearest inetrpolation method
 *
 * @param bayer             input raw image pointer
 * @param output            output RGB image pointer
 * @param width             image width
 * @param height            image height
 * @param tile              color filter tiles
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_nearest_rgb888(const uint8_t *restrict bayer,
						uint8_t *restrict rgb, int sx, int sy, int tile);

/**
 * Perform de-mosaicing on an 8-bit raw image buffer
 * into BGR888 image using nearest inetrpolation method
 *
 * @param bayer             input raw image pointer
 * @param output            output RGB image pointer
 * @param width             image width
 * @param height            image height
 * @param tile              color filter tiles
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_nearest_bgr888(const uint8_t *restrict bayer,
						uint8_t *restrict rgb, int sx, int sy, int tile);

/**
 * Perform de-mosaicing on an 8-bit raw image buffer
 * into ARGB8888 image using nearest inetrpolation method
 *
 * @param bayer             input raw image pointer
 * @param output            output RGB image pointer
 * @param width             image width
 * @param height            image height
 * @param tile              color filter tiles
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_nearest_argb8888(const uint8_t *restrict bayer,
						  uint8_t *restrict rgb, int sx, int sy, int tile);

/**
 * Perform de-mosaicing on an 8-bit raw image buffer
 * into RGB565 image using simple method
 *
 * @param bayer             input raw image pointer
 * @param output            output RGB image pointer
 * @param width             image width
 * @param height            image height
 * @param tile              color filter tiles
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_simple_rgb565(const uint8_t *restrict bayer, uint8_t *restrict rgb,
					       int sx, int sy, int tile);

/**
 * Perform de-mosaicing on an 8-bit raw image buffer
 * into RGB888 image using simple method
 *
 * @param bayer             input raw image pointer
 * @param output            output RGB image pointer
 * @param width             image width
 * @param height            image height
 * @param tile              color filter tiles
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_simple_rgb888(const uint8_t *restrict bayer, uint8_t *restrict rgb,
					       int sx, int sy, int tile);

/**
 * Perform de-mosaicing on an 8-bit raw image buffer
 * into BGR888 image using simple method
 *
 * @param bayer             input raw image pointer
 * @param output            output RGB image pointer
 * @param width             image width
 * @param height            image height
 * @param tile              color filter tiles
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_simple_bgr888(const uint8_t *restrict bayer, uint8_t *restrict rgb,
					       int sx, int sy, int tile);

/**
 * Perform de-mosaicing on an 8-bit raw image buffer
 * into ARGB8888 image using simple method
 *
 * @param bayer             input raw image pointer
 * @param output            output RGB image pointer
 * @param width             image width
 * @param height            image height
 * @param tile              color filter tiles
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_simple_argb8888(const uint8_t *restrict bayer,
						 uint8_t *restrict rgb, int sx, int sy, int tile);

/**
 * Perform de-mosaicing on an 8-bit raw image buffer
 * into RGB565 image using linear interpolation method
 *
 * @param bayer             input raw image pointer
 * @param output            output RGB image pointer
 * @param width             image width
 * @param height            image height
 * @param tile              color filter tiles
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_linear_rgb565(const uint8_t *restrict bayer, uint8_t *restrict rgb,
					       int sx, int sy, int tile);

/**
 * Perform de-mosaicing on an 8-bit raw image buffer
 * into RGB888 image using linear interpolation method
 *
 * @param bayer             input raw image pointer
 * @param output            output RGB image pointer
 * @param width             image width
 * @param height            image height
 * @param tile              color filter tiles
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_linear_rgb888(const uint8_t *restrict bayer, uint8_t *restrict rgb,
					       int sx, int sy, int tile);

/**
 * Perform de-mosaicing on an 8-bit raw image buffer
 * into BGR888 image using linear interpolation method
 *
 * @param bayer             input raw image pointer
 * @param output            output RGB image pointer
 * @param width             image width
 * @param height            image height
 * @param tile              color filter tiles
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_linear_bgr888(const uint8_t *restrict bayer, uint8_t *restrict rgb,
					       int sx, int sy, int tile);

/**
 * Perform de-mosaicing on an 8-bit raw image buffer
 * into ARGB8888 image using linear interpolation method
 *
 * @param bayer             input raw image pointer
 * @param output            output RGB image pointer
 * @param width             image width
 * @param height            image height
 * @param tile              color filter tiles
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_linear_argb8888(const uint8_t *restrict bayer,
						 uint8_t *restrict rgb, int sx, int sy, int tile);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* AIPL_RESIZE_H */

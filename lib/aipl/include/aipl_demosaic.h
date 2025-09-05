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
 * @file    aipl_demosaic.h
 * @brief   Bayer pattern demosaicing function definitions
 *
 ******************************************************************************/

#ifndef AIPL_DEMOSAICING_H
#define AIPL_DEMOSAICING_H

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
typedef enum {
	AIPL_BAYER_RGGB,
	AIPL_BAYER_GBRG,
	AIPL_BAYER_GRBG,
	AIPL_BAYER_BGGR,
	AIPL_BAYER_FILTER_NUM,
} aipl_bayer_filter_t;

typedef struct {
	uint8_t red_row;
	uint8_t blue_row;
	uint8_t red_col;
	uint8_t blue_col;
	const uint8_t *red_src;
	const uint8_t *blue_src;
} aipl_bayer_tile_t;

/*********************
 *      DEFINES
 *********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Get raw image bayer tile
 *
 * @param input  raw image input pointer
 * @param pitch  raw image pitch
 * @param filter bayer filter
 * @return bayer tile
 */
aipl_bayer_tile_t aipl_bayer_tile(const void *input, uint32_t pitch, aipl_bayer_filter_t filter);

/**
 * Get bayer filter with swapped R and B channels
 *
 * @param filter input bayer filter
 * @return bayer filter with channnels swapped
 */
aipl_bayer_filter_t aipl_bayer_filter_swap_rb(aipl_bayer_filter_t filter);

/**
 * Perform demosaicing on an 8-bit raw image buffer
 *
 * @param input             input raw image pointer
 * @param output            output RGB image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @param format            output color format
 * @return error code
 */
aipl_error_t aipl_demosaic(const void *input, void *output, uint32_t pitch, uint32_t width,
			   uint32_t height, aipl_bayer_filter_t filter, aipl_color_format_t format);

/**
 * Perform demosaicing on an 8-bit raw image buffer
 * using aipl_image_t interface
 *
 * @param input             input raw image pointer
 * @param output            output image
 * @param pitch             input raw image pitch
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_demosaic_img(const void *input, aipl_image_t *output, uint32_t pitch,
			       aipl_bayer_filter_t filter);

/**
 * Perform demosaicing on an 8-bit raw image buffer
 * into ALPHA8 image
 *
 * @param input             input raw image pointer
 * @param output            output ALPHA8 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_demosaic_alpha8(const void *input, void *output, uint32_t pitch, uint32_t width,
				  uint32_t height, aipl_bayer_filter_t filter);

/**
 * Perform demosaicing on an 8-bit raw image buffer
 * into ARGB8888 image
 *
 * @param input             input raw image pointer
 * @param output            output ARGB8888 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_demosaic_argb8888(const void *input, void *output, uint32_t pitch, uint32_t width,
				    uint32_t height, aipl_bayer_filter_t filter);

/**
 * Perform demosaicing on an 8-bit raw image buffer
 * into ARGB4444 image
 *
 * @param input             input raw image pointer
 * @param output            output ARGB4444 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_demosaic_argb4444(const void *input, void *output, uint32_t pitch, uint32_t width,
				    uint32_t height, aipl_bayer_filter_t filter);

/**
 * Perform demosaicing on an 8-bit raw image buffer
 * into ARGB1555 image
 *
 * @param input             input raw image pointer
 * @param output            output ARGB1555 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_demosaic_argb1555(const void *input, void *output, uint32_t pitch, uint32_t width,
				    uint32_t height, aipl_bayer_filter_t filter);

/**
 * Perform demosaicing on an 8-bit raw image buffer
 * into RGBA8888 image
 *
 * @param input             input raw image pointer
 * @param output            output RGBA8888 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_demosaic_rgba8888(const void *input, void *output, uint32_t pitch, uint32_t width,
				    uint32_t height, aipl_bayer_filter_t filter);

/**
 * Perform demosaicing on an 8-bit raw image buffer
 * into RGBA4444 image
 *
 * @param input             input raw image pointer
 * @param output            output RGBA4444 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_demosaic_rgba4444(const void *input, void *output, uint32_t pitch, uint32_t width,
				    uint32_t height, aipl_bayer_filter_t filter);

/**
 * Perform demosaicing on an 8-bit raw image buffer
 * into RGBA5551 image
 *
 * @param input             input raw image pointer
 * @param output            output RGBA5551 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_demosaic_rgba5551(const void *input, void *output, uint32_t pitch, uint32_t width,
				    uint32_t height, aipl_bayer_filter_t filter);

/**
 * Perform demosaicing on an 8-bit raw image buffer
 * into RGB888 image
 *
 * @param input             input raw image pointer
 * @param output            output RGB888 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_demosaic_rgb888(const void *input, void *output, uint32_t pitch, uint32_t width,
				  uint32_t height, aipl_bayer_filter_t filter);

/**
 * Perform demosaicing on an 8-bit raw image buffer
 * into BGR888 image
 *
 * @param input             input raw image pointer
 * @param output            output BGR888 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_demosaic_bgr888(const void *input, void *output, uint32_t pitch, uint32_t width,
				  uint32_t height, aipl_bayer_filter_t filter);

/**
 * Perform demosaicing on an 8-bit raw image buffer
 * into RGB565 image
 *
 * @param input             input raw image pointer
 * @param output            output RGB565 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_demosaic_rgb565(const void *input, void *output, uint32_t pitch, uint32_t width,
				  uint32_t height, aipl_bayer_filter_t filter);

/**
 * Perform demosaicing on an 8-bit raw image buffer
 * into YV12 image
 *
 * @param input             input raw image pointer
 * @param output            output YV12 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_demosaic_yv12(const void *input, void *output, uint32_t pitch, uint32_t width,
				uint32_t height, aipl_bayer_filter_t filter);

/**
 * Perform demosaicing on an 8-bit raw image buffer
 * into I420 image
 *
 * @param input             input raw image pointer
 * @param output            output I420 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_demosaic_i420(const void *input, void *output, uint32_t pitch, uint32_t width,
				uint32_t height, aipl_bayer_filter_t filter);

/**
 * Perform demosaicing on an 8-bit raw image buffer
 * into I422 image
 *
 * @param input             input raw image pointer
 * @param output            output I422 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_demosaic_i422(const void *input, void *output, uint32_t pitch, uint32_t width,
				uint32_t height, aipl_bayer_filter_t filter);

/**
 * Perform demosaicing on an 8-bit raw image buffer
 * into I444 image
 *
 * @param input             input raw image pointer
 * @param output            output I444 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_demosaic_i444(const void *input, void *output, uint32_t pitch, uint32_t width,
				uint32_t height, aipl_bayer_filter_t filter);

/**
 * Perform demosaicing on an 8-bit raw image buffer
 * into I400 image
 *
 * @param input             input raw image pointer
 * @param output            output I400 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_demosaic_i400(const void *input, void *output, uint32_t pitch, uint32_t width,
				uint32_t height, aipl_bayer_filter_t filter);

/**
 * Perform demosaicing on an 8-bit raw image buffer
 * into NV12 image
 *
 * @param input             input raw image pointer
 * @param output            output NV12 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_demosaic_nv12(const void *input, void *output, uint32_t pitch, uint32_t width,
				uint32_t height, aipl_bayer_filter_t filter);

/**
 * Perform demosaicing on an 8-bit raw image buffer
 * into NV21 image
 *
 * @param input             input raw image pointer
 * @param output            output NV21 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_demosaic_nv21(const void *input, void *output, uint32_t pitch, uint32_t width,
				uint32_t height, aipl_bayer_filter_t filter);

/**
 * Perform demosaicing on an 8-bit raw image buffer
 * into YUY2 image
 *
 * @param input             input raw image pointer
 * @param output            output YUY2 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_demosaic_yuy2(const void *input, void *output, uint32_t pitch, uint32_t width,
				uint32_t height, aipl_bayer_filter_t filter);

/**
 * Perform demosaicing on an 8-bit raw image buffer
 * into UYVY image
 *
 * @param input             input raw image pointer
 * @param output            output UYVY image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_demosaic_uyvy(const void *input, void *output, uint32_t pitch, uint32_t width,
				uint32_t height, aipl_bayer_filter_t filter);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* AIPL_DEMOSAICING_H */

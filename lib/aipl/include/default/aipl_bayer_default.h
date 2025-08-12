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
 * @file    aipl_bayer_default.h
 * @brief   Bayer pattern decoding function definitions with default
 *          compiler optimizations
 *
 ******************************************************************************/

#ifndef AIPL_BAYER_DEFAULT_H
#define AIPL_BAYER_DEFAULT_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "aipl_bayer.h"

/**********************
 *      TYPEDEFS
 **********************/

/*********************
 *      DEFINES
 *********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Perform de-bayering on an 8-bit raw image buffer
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
aipl_error_t aipl_bayer_decoding_default(const void *input, void *output, uint32_t pitch,
					 uint32_t width, uint32_t height,
					 aipl_bayer_filter_t filter, aipl_color_format_t format);

/**
 * Perform de-bayering on an 8-bit raw image buffer
 * using aipl_image_t interface
 *
 * @param input             input raw image pointer
 * @param output            output image
 * @param pitch             input raw image pitch
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_img_default(const void *input, aipl_image_t *output,
					     uint32_t pitch, aipl_bayer_filter_t filter);

/**
 * Perform de-bayering on an 8-bit raw image buffer
 * into ALPHA8 image using default compiler optimizations
 *
 * @param input             input raw image pointer
 * @param output            output ALPHA8 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_alpha8_default(const void *input, void *output, uint32_t pitch,
						uint32_t width, uint32_t height,
						aipl_bayer_filter_t filter);

/**
 * Perform de-bayering on an 8-bit raw image buffer
 * into ARGB8888 image using default compiler optimizations
 *
 * @param input             input raw image pointer
 * @param output            output ARGB8888 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_argb8888_default(const void *input, void *output, uint32_t pitch,
						  uint32_t width, uint32_t height,
						  aipl_bayer_filter_t filter);

/**
 * Perform de-bayering on an 8-bit raw image buffer
 * into ARGB4444 image using default compiler optimizations
 *
 * @param input             input raw image pointer
 * @param output            output ARGB4444 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_argb4444_default(const void *input, void *output, uint32_t pitch,
						  uint32_t width, uint32_t height,
						  aipl_bayer_filter_t filter);

/**
 * Perform de-bayering on an 8-bit raw image buffer
 * into ARGB1555 image using default compiler optimizations
 *
 * @param input             input raw image pointer
 * @param output            output ARGB1555 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_argb1555_default(const void *input, void *output, uint32_t pitch,
						  uint32_t width, uint32_t height,
						  aipl_bayer_filter_t filter);

/**
 * Perform de-bayering on an 8-bit raw image buffer
 * into RGBA8888 image using default compiler optimizations
 *
 * @param input             input raw image pointer
 * @param output            output RGBA8888 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_rgba8888_default(const void *input, void *output, uint32_t pitch,
						  uint32_t width, uint32_t height,
						  aipl_bayer_filter_t filter);

/**
 * Perform de-bayering on an 8-bit raw image buffer
 * into RGBA4444 image using default compiler optimizations
 *
 * @param input             input raw image pointer
 * @param output            output RGBA4444 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_rgba4444_default(const void *input, void *output, uint32_t pitch,
						  uint32_t width, uint32_t height,
						  aipl_bayer_filter_t filter);

/**
 * Perform de-bayering on an 8-bit raw image buffer
 * into RGBA5551 image using default compiler optimizations
 *
 * @param input             input raw image pointer
 * @param output            output RGBA5551 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_rgba5551_default(const void *input, void *output, uint32_t pitch,
						  uint32_t width, uint32_t height,
						  aipl_bayer_filter_t filter);

/**
 * Perform de-bayering on an 8-bit raw image buffer
 * into RGB888 image using default compiler optimizations
 *
 * @param input             input raw image pointer
 * @param output            output RGB888 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_rgb888_default(const void *input, void *output, uint32_t pitch,
						uint32_t width, uint32_t height,
						aipl_bayer_filter_t filter);

/**
 * Perform de-bayering on an 8-bit raw image buffer
 * into BGR888 image using default compiler optimizations
 *
 * @param input             input raw image pointer
 * @param output            output BGR888 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_bgr888_default(const void *input, void *output, uint32_t pitch,
						uint32_t width, uint32_t height,
						aipl_bayer_filter_t filter);

/**
 * Perform de-bayering on an 8-bit raw image buffer
 * into RGB565 image using default compiler optimizations
 *
 * @param input             input raw image pointer
 * @param output            output RGB565 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_rgb565_default(const void *input, void *output, uint32_t pitch,
						uint32_t width, uint32_t height,
						aipl_bayer_filter_t filter);

/**
 * Perform de-bayering on an 8-bit raw image buffer
 * into YV12 image using default compiler optimizations
 *
 * @param input             input raw image pointer
 * @param output            output YV12 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_yv12_default(const void *input, void *output, uint32_t pitch,
					      uint32_t width, uint32_t height,
					      aipl_bayer_filter_t filter);

/**
 * Perform de-bayering on an 8-bit raw image buffer
 * into I420 image using default compiler optimizations
 *
 * @param input             input raw image pointer
 * @param output            output I420 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_i420_default(const void *input, void *output, uint32_t pitch,
					      uint32_t width, uint32_t height,
					      aipl_bayer_filter_t filter);

/**
 * Perform de-bayering on an 8-bit raw image buffer
 * into I422 image using default compiler optimizations
 *
 * @param input             input raw image pointer
 * @param output            output I422 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_i422_default(const void *input, void *output, uint32_t pitch,
					      uint32_t width, uint32_t height,
					      aipl_bayer_filter_t filter);

/**
 * Perform de-bayering on an 8-bit raw image buffer
 * into I444 image using default compiler optimizations
 *
 * @param input             input raw image pointer
 * @param output            output I444 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_i444_default(const void *input, void *output, uint32_t pitch,
					      uint32_t width, uint32_t height,
					      aipl_bayer_filter_t filter);

/**
 * Perform de-bayering on an 8-bit raw image buffer
 * into I400 image using default compiler optimizations
 *
 * @param input             input raw image pointer
 * @param output            output I400 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_i400_default(const void *input, void *output, uint32_t pitch,
					      uint32_t width, uint32_t height,
					      aipl_bayer_filter_t filter);

/**
 * Perform de-bayering on an 8-bit raw image buffer
 * into NV12 image using default compiler optimizations
 *
 * @param input             input raw image pointer
 * @param output            output NV12 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_nv12_default(const void *input, void *output, uint32_t pitch,
					      uint32_t width, uint32_t height,
					      aipl_bayer_filter_t filter);

/**
 * Perform de-bayering on an 8-bit raw image buffer
 * into NV21 image using default compiler optimizations
 *
 * @param input             input raw image pointer
 * @param output            output NV21 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_nv21_default(const void *input, void *output, uint32_t pitch,
					      uint32_t width, uint32_t height,
					      aipl_bayer_filter_t filter);

/**
 * Perform de-bayering on an 8-bit raw image buffer
 * into YUY2 image using default compiler optimizations
 *
 * @param input             input raw image pointer
 * @param output            output YUY2 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_yuy2_default(const void *input, void *output, uint32_t pitch,
					      uint32_t width, uint32_t height,
					      aipl_bayer_filter_t filter);

/**
 * Perform de-bayering on an 8-bit raw image buffer
 * into UYVY image using default compiler optimizations
 *
 * @param input             input raw image pointer
 * @param output            output UYVY image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_uyvy_default(const void *input, void *output, uint32_t pitch,
					      uint32_t width, uint32_t height,
					      aipl_bayer_filter_t filter);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* AIPL_BAYER_DEFAULT_H */

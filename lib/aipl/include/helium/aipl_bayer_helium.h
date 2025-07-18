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
 * @file    aipl_bayer_helium.h
 * @brief   Bayer pattern decoding function definitions with Helium
 *          vector acceleration
 *
 ******************************************************************************/

#ifndef AIPL_BAYER_HELIUM_H
#define AIPL_BAYER_HELIUM_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "aipl_bayer.h"
#include "aipl_config.h"

/**********************
 *      TYPEDEFS
 **********************/

/*********************
 *      DEFINES
 *********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

#ifdef AIPL_HELIUM_ACCELERATION

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
aipl_error_t aipl_bayer_decoding_helium(const void *input, void *output, uint32_t pitch,
					uint32_t width, uint32_t height, aipl_bayer_filter_t filter,
					aipl_color_format_t format);

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
aipl_error_t aipl_bayer_decoding_img_helium(const void *input, aipl_image_t *output, uint32_t pitch,
					    aipl_bayer_filter_t filter);

/**
 * Perform de-bayering on an 8-bit raw image buffer
 * into ALPHA8 image using Helium vector acceleration
 *
 * @param input             input raw image pointer
 * @param output            output ALPHA8 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_alpha8_helium(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height,
					       aipl_bayer_filter_t filter);

/**
 * Perform de-bayering on an 8-bit raw image buffer
 * into ARGB8888 image using Helium vector acceleration
 *
 * @param input             input raw image pointer
 * @param output            output ARGB8888 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_argb8888_helium(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height,
						 aipl_bayer_filter_t filter);

/**
 * Perform de-bayering on an 8-bit raw image buffer
 * into ARGB4444 image using Helium vector acceleration
 *
 * @param input             input raw image pointer
 * @param output            output ARGB4444 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_argb4444_helium(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height,
						 aipl_bayer_filter_t filter);

/**
 * Perform de-bayering on an 8-bit raw image buffer
 * into ARGB1555 image using Helium vector acceleration
 *
 * @param input             input raw image pointer
 * @param output            output ARGB1555 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_argb1555_helium(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height,
						 aipl_bayer_filter_t filter);

/**
 * Perform de-bayering on an 8-bit raw image buffer
 * into RGBA8888 image using Helium vector acceleration
 *
 * @param input             input raw image pointer
 * @param output            output RGBA8888 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_rgba8888_helium(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height,
						 aipl_bayer_filter_t filter);

/**
 * Perform de-bayering on an 8-bit raw image buffer
 * into RGBA4444 image using Helium vector acceleration
 *
 * @param input             input raw image pointer
 * @param output            output RGBA4444 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_rgba4444_helium(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height,
						 aipl_bayer_filter_t filter);

/**
 * Perform de-bayering on an 8-bit raw image buffer
 * into RGBA5551 image using Helium vector acceleration
 *
 * @param input             input raw image pointer
 * @param output            output RGBA5551 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_rgba5551_helium(const void *input, void *output, uint32_t pitch,
						 uint32_t width, uint32_t height,
						 aipl_bayer_filter_t filter);

/**
 * Perform de-bayering on an 8-bit raw image buffer
 * into RGB888 image using Helium vector acceleration
 *
 * @param input             input raw image pointer
 * @param output            output RGB888 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_rgb888_helium(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height,
					       aipl_bayer_filter_t filter);

/**
 * Perform de-bayering on an 8-bit raw image buffer
 * into BGR888 image using Helium vector acceleration
 *
 * @param input             input raw image pointer
 * @param output            output BGR888 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_bgr888_helium(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height,
					       aipl_bayer_filter_t filter);

/**
 * Perform de-bayering on an 8-bit raw image buffer
 * into RGB565 image using Helium vector acceleration
 *
 * @param input             input raw image pointer
 * @param output            output RGB565 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_rgb565_helium(const void *input, void *output, uint32_t pitch,
					       uint32_t width, uint32_t height,
					       aipl_bayer_filter_t filter);

/**
 * Perform de-bayering on an 8-bit raw image buffer
 * into YV12 image using Helium vector acceleration
 *
 * @param input             input raw image pointer
 * @param output            output YV12 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_yv12_helium(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height,
					     aipl_bayer_filter_t filter);

/**
 * Perform de-bayering on an 8-bit raw image buffer
 * into I420 image using Helium vector acceleration
 *
 * @param input             input raw image pointer
 * @param output            output I420 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_i420_helium(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height,
					     aipl_bayer_filter_t filter);

/**
 * Perform de-bayering on an 8-bit raw image buffer
 * into I422 image using Helium vector acceleration
 *
 * @param input             input raw image pointer
 * @param output            output I422 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_i422_helium(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height,
					     aipl_bayer_filter_t filter);

/**
 * Perform de-bayering on an 8-bit raw image buffer
 * into I444 image using Helium vector acceleration
 *
 * @param input             input raw image pointer
 * @param output            output I444 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_i444_helium(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height,
					     aipl_bayer_filter_t filter);

/**
 * Perform de-bayering on an 8-bit raw image buffer
 * into I400 image using Helium vector acceleration
 *
 * @param input             input raw image pointer
 * @param output            output I400 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_i400_helium(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height,
					     aipl_bayer_filter_t filter);

/**
 * Perform de-bayering on an 8-bit raw image buffer
 * into NV12 image using Helium vector acceleration
 *
 * @param input             input raw image pointer
 * @param output            output NV12 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_nv12_helium(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height,
					     aipl_bayer_filter_t filter);

/**
 * Perform de-bayering on an 8-bit raw image buffer
 * into NV21 image using Helium vector acceleration
 *
 * @param input             input raw image pointer
 * @param output            output NV21 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_nv21_helium(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height,
					     aipl_bayer_filter_t filter);

/**
 * Perform de-bayering on an 8-bit raw image buffer
 * into YUY2 image using Helium vector acceleration
 *
 * @param input             input raw image pointer
 * @param output            output YUY2 image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_yuy2_helium(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height,
					     aipl_bayer_filter_t filter);

/**
 * Perform de-bayering on an 8-bit raw image buffer
 * into UYVY image using Helium vector acceleration
 *
 * @param input             input raw image pointer
 * @param output            output UYVY image pointer
 * @param pitch             input raw image pitch
 * @param width             image width
 * @param height            image height
 * @param filter            bayer filter
 * @return error code
 */
aipl_error_t aipl_bayer_decoding_uyvy_helium(const void *input, void *output, uint32_t pitch,
					     uint32_t width, uint32_t height,
					     aipl_bayer_filter_t filter);

#endif /* AIPL_HELIUM_ACCELERATION */

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* AIPL_BAYER_HELIUM_H */

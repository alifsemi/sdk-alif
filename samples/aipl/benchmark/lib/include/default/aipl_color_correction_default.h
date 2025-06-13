/* Copyright (C) 2022-2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/******************************************************************************
 * @file    aipl_color_correction_default.h
 * @brief   Default color correction function definitions
 *
******************************************************************************/

#ifndef AIPL_COLOR_CORRECTION_DEFAULT_H
#define AIPL_COLOR_CORRECTION_DEFAULT_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "aipl_color_formats.h"
#include "aipl_error.h"
#include "aipl_image.h"
#include "aipl_config.h"

#if !defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT)

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
/**
 * Apply a color correction matrix to an RGB image
 * with raw pointer interface using default
 * implementation with compiler imposed optimization
 *
 * The operation is a 3x3 on 3x1 matrix multiplication:
 * | r'|   | ccm0 ccm1 ccm2 |   | r |
 * | g'| = | ccm3 ccm4 ccm5 | x | g |
 * | b'|   | ccm6 ccm7 ccm8 |   | b |
 *
 * @param input     input image pointer
 * @param output    output image buffer pointer
 * @param pitch     input image pitch
 * @param width     image width
 * @param height    image height
 * @param format    image color format
 * @param ccm       color correction matrix (9 elements len)
 * @return error code
 */
aipl_error_t aipl_color_correction_rgb_default(const void* input, void* output,
                                               uint32_t pitch,
                                               uint32_t width, uint32_t height,
                                               aipl_color_format_t format,
                                               const float* ccm);

/**
 * Apply a color correction matrix to an RGB image
 * with aipl_image_t interface using default
 * implementation with compiler imposed optimization
 *
 * See aipl_color_correction_rgb_default()
 *
 * @param input     input image
 * @param output    output image
 * @param ccm       color correction matrix
 */
aipl_error_t aipl_color_correction_rgb_img_default(const aipl_image_t* input,
                                                   aipl_image_t* output,
                                                   const float* ccm);

/**
 * Apply a color correction matrix to an ARGB8888 image
 * using default implementation with compiler imposed optimization
 *
 * See aipl_color_correction_rgb_default()
 *
 * @param input     input image pointer
 * @param output    output image buffer pointer
 * @param pitch     input image pitch
 * @param width     image width
 * @param height    image height
 * @param ccm       color correction matrix (9 elements len)
 * @return error code
 */
aipl_error_t aipl_color_correction_argb8888_default(const void* input, void* output,
                                                    uint32_t pitch,
                                                    uint32_t width, uint32_t height,
                                                    const float* ccm);

/**
 * Apply a color correction matrix to an ARGB4444 image
 * using default implementation with compiler imposed optimization
 *
 * See aipl_color_correction_rgb_default()
 *
 * @param input     input image pointer
 * @param output    output image buffer pointer
 * @param pitch     input image pitch
 * @param width     image width
 * @param height    image height
 * @param ccm       color correction matrix (9 elements len)
 * @return error code
 */
aipl_error_t aipl_color_correction_argb4444_default(const void* input, void* output,
                                                    uint32_t pitch,
                                                    uint32_t width, uint32_t height,
                                                    const float* ccm);

/**
 * Apply a color correction matrix to an ARGB1555 image
 * using default implementation with compiler imposed optimization
 *
 * See aipl_color_correction_rgb_default()
 *
 * @param input     input image pointer
 * @param output    output image buffer pointer
 * @param pitch     input image pitch
 * @param width     image width
 * @param height    image height
 * @param ccm       color correction matrix (9 elements len)
 * @return error code
 */
aipl_error_t aipl_color_correction_argb1555_default(const void* input, void* output,
                                                    uint32_t pitch,
                                                    uint32_t width, uint32_t height,
                                                    const float* ccm);

/**
 * Apply a color correction matrix to an RGBA8888 image
 * using default implementation with compiler imposed optimization
 *
 * See aipl_color_correction_rgb_default()
 *
 * @param input     input image pointer
 * @param output    output image buffer pointer
 * @param pitch     input image pitch
 * @param width     image width
 * @param height    image height
 * @param ccm       color correction matrix (9 elements len)
 * @return error code
 */
aipl_error_t aipl_color_correction_rgba8888_default(const void* input, void* output,
                                                    uint32_t pitch,
                                                    uint32_t width, uint32_t height,
                                                    const float* ccm);

/**
 * Apply a color correction matrix to an RGBA4444 image
 * using default implementation with compiler imposed optimization
 *
 * See aipl_color_correction_rgb_default()
 *
 * @param input     input image pointer
 * @param output    output image buffer pointer
 * @param pitch     input image pitch
 * @param width     image width
 * @param height    image height
 * @param ccm       color correction matrix (9 elements len)
 * @return error code
 */
aipl_error_t aipl_color_correction_rgba4444_default(const void* input, void* output,
                                                    uint32_t pitch,
                                                    uint32_t width, uint32_t height,
                                                    const float* ccm);

/**
 * Apply a color correction matrix to an RGBA5551 image
 * using default implementation with compiler imposed optimization
 *
 * See aipl_color_correction_rgb_default()
 *
 * @param input     input image pointer
 * @param output    output image buffer pointer
 * @param pitch     input image pitch
 * @param width     image width
 * @param height    image height
 * @param ccm       color correction matrix (9 elements len)
 * @return error code
 */
aipl_error_t aipl_color_correction_rgba5551_default(const void* input, void* output,
                                                    uint32_t pitch,
                                                    uint32_t width, uint32_t height,
                                                    const float* ccm);

/**
 * Apply a color correction matrix to an BGR888 image
 * using default implementation with compiler imposed optimization
 *
 * See aipl_color_correction_rgb_default()
 *
 * @param input     input image pointer
 * @param output    output image buffer pointer
 * @param pitch     input image pitch
 * @param width     image width
 * @param height    image height
 * @param ccm       color correction matrix (9 elements len)
 * @return error code
 */
aipl_error_t aipl_color_correction_bgr888_default(const void* input, void* output,
                                                  uint32_t pitch,
                                                  uint32_t width, uint32_t height,
                                                  const float* ccm);

/**
 * Apply a color correction matrix to an RGB888 image
 * using default implementation with compiler imposed optimization
 *
 * See aipl_color_correction_rgb_default()
 *
 * @param input     input image pointer
 * @param output    output image buffer pointer
 * @param pitch     input image pitch
 * @param width     image width
 * @param height    image height
 * @param ccm       color correction matrix (9 elements len)
 * @return error code
 */
aipl_error_t aipl_color_correction_rgb888_default(const void* input, void* output,
                                                  uint32_t pitch,
                                                  uint32_t width, uint32_t height,
                                                  const float* ccm);


/**
 * Apply a color correction matrix to an RGB565 image
 * using default implementation with compiler imposed optimization
 *
 * See aipl_color_correction_rgb_default()
 *
 * @param input     input image pointer
 * @param output    output image buffer pointer
 * @param pitch     input image pitch
 * @param width     image width
 * @param height    image height
 * @param ccm       color correction matrix (9 elements len)
 * @return error code
 */
aipl_error_t aipl_color_correction_rgb565_default(const void* input, void* output,
                                                  uint32_t pitch,
                                                  uint32_t width, uint32_t height,
                                                  const float* ccm);

/**********************
 *      MACROS
 **********************/

#endif

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  /* AIPL_COLOR_CORRECTION_DEFAULT_H */

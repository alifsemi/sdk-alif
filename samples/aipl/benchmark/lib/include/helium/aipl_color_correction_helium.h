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
 * @file    aipl_color_correction_helium.h
 * @brief   Helium accelerated color correction function definitions
 *
******************************************************************************/

#ifndef AIPL_COLOR_CORRECTION_HELIUM_H
#define AIPL_COLOR_CORRECTION_HELIUM_H

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

#ifdef AIPL_HELIUM_ACCELERATION

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
 * using raw pointer interface with Helium acceleration
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
aipl_error_t aipl_color_correction_rgb_helium(const void* input, void* output,
                                              uint32_t pitch,
                                              uint32_t width, uint32_t height,
                                              aipl_color_format_t format,
                                              const float* ccm);

/**
 * Apply a color correction matrix to an RGB image
 * with Helium acceleration using aipl_image_t interface
 *
 * See aipl_color_correction_rgb_helium()
 *
 * @param input     input image
 * @param output    output image
 * @param ccm       color correction matrix
 */
aipl_error_t aipl_color_correction_rgb_img_helium(const aipl_image_t* input,
                                                  aipl_image_t* output,
                                                  const float* ccm);

/**
 * Apply a color correction matrix to an ARGB8888 image
 * with Helium acceleration
 *
 * See aipl_color_correction_rgb_helium()
 *
 * @param input     input image pointer
 * @param output    output image buffer pointer
 * @param pitch     input image pitch
 * @param width     image width
 * @param height    image height
 * @param ccm       color correction matrix (9 elements len)
 * @return error code
 */
aipl_error_t aipl_color_correction_argb8888_helium(const void* input, void* output,
                                                   uint32_t pitch,
                                                   uint32_t width, uint32_t height,
                                                   const float* ccm);

/**
 * Apply a color correction matrix to an ARGB4444 image
 * with Helium acceleration
 *
 * See aipl_color_correction_rgb_helium()
 *
 * @param input     input image pointer
 * @param output    output image buffer pointer
 * @param pitch     input image pitch
 * @param width     image width
 * @param height    image height
 * @param ccm       color correction matrix (9 elements len)
 * @return error code
 */
aipl_error_t aipl_color_correction_argb4444_helium(const void* input, void* output,
                                                   uint32_t pitch,
                                                   uint32_t width, uint32_t height,
                                                   const float* ccm);

/**
 * Apply a color correction matrix to an ARGB1555 image
 * with Helium acceleration
 *
 * See aipl_color_correction_rgb_helium()
 *
 * @param input     input image pointer
 * @param output    output image buffer pointer
 * @param pitch     input image pitch
 * @param width     image width
 * @param height    image height
 * @param ccm       color correction matrix (9 elements len)
 * @return error code
 */
aipl_error_t aipl_color_correction_argb1555_helium(const void* input, void* output,
                                                   uint32_t pitch,
                                                   uint32_t width, uint32_t height,
                                                   const float* ccm);

/**
 * Apply a color correction matrix to an RGBA8888 image
 * with Helium acceleration
 *
 * See aipl_color_correction_rgb_helium()
 *
 * @param input     input image pointer
 * @param output    output image buffer pointer
 * @param pitch     input image pitch
 * @param width     image width
 * @param height    image height
 * @param ccm       color correction matrix (9 elements len)
 * @return error code
 */
aipl_error_t aipl_color_correction_rgba8888_helium(const void* input, void* output,
                                                   uint32_t pitch,
                                                   uint32_t width, uint32_t height,
                                                   const float* ccm);

/**
 * Apply a color correction matrix to an RGBA4444 image
 * with Helium acceleration
 *
 * See aipl_color_correction_rgb_helium()
 *
 * @param input     input image pointer
 * @param output    output image buffer pointer
 * @param pitch     input image pitch
 * @param width     image width
 * @param height    image height
 * @param ccm       color correction matrix (9 elements len)
 * @return error code
 */
aipl_error_t aipl_color_correction_rgba4444_helium(const void* input, void* output,
                                                   uint32_t pitch,
                                                   uint32_t width, uint32_t height,
                                                   const float* ccm);

/**
 * Apply a color correction matrix to an RGBA5551 image
 * with Helium acceleration
 *
 * See aipl_color_correction_rgb_helium()
 *
 * @param input     input image pointer
 * @param output    output image buffer pointer
 * @param pitch     input image pitch
 * @param width     image width
 * @param height    image height
 * @param ccm       color correction matrix (9 elements len)
 * @return error code
 */
aipl_error_t aipl_color_correction_rgba5551_helium(const void* input, void* output,
                                                   uint32_t pitch,
                                                   uint32_t width, uint32_t height,
                                                   const float* ccm);

/**
 * Apply a color correction matrix to an BGR888 image
 * with Helium acceleration
 *
 * See aipl_color_correction_rgb_helium()
 *
 * @param input     input image pointer
 * @param output    output image buffer pointer
 * @param pitch     input image pitch
 * @param width     image width
 * @param height    image height
 * @param ccm       color correction matrix (9 elements len)
 * @return error code
 */
aipl_error_t aipl_color_correction_bgr888_helium(const void* input, void* output,
                                                 uint32_t pitch,
                                                 uint32_t width, uint32_t height,
                                                 const float* ccm);

/**
 * Apply a color correction matrix to an RGB888 image
 * with Helium acceleration
 *
 * See aipl_color_correction_rgb_helium()
 *
 * @param input     input image pointer
 * @param output    output image buffer pointer
 * @param pitch     input image pitch
 * @param width     image width
 * @param height    image height
 * @param ccm       color correction matrix (9 elements len)
 * @return error code
 */
aipl_error_t aipl_color_correction_rgb888_helium(const void* input, void* output,
                                                 uint32_t pitch,
                                                 uint32_t width, uint32_t height,
                                                 const float* ccm);


/**
 * Apply a color correction matrix to an RGB565 image
 * with Helium acceleration
 *
 * See aipl_color_correction_rgb_helium()
 *
 * @param input     input image pointer
 * @param output    output image buffer pointer
 * @param pitch     input image pitch
 * @param width     image width
 * @param height    image height
 * @param ccm       color correction matrix (9 elements len)
 * @return error code
 */
aipl_error_t aipl_color_correction_rgb565_helium(const void* input, void* output,
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

#endif  /* AIPL_COLOR_CORRECTION_HELIUM_H */

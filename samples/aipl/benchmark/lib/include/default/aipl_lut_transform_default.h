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
 * @file    aipl_lut_transform_default.h
 * @brief   Default LUT transformation function definitions
 *
******************************************************************************/

#ifndef AIPL_LUT_TRANSFORM_DEFAULT_H
#define AIPL_LUT_TRANSFORM_DEFAULT_H

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
 * Perform a LUT transformation of RGB channels of an image
 * with raw pointer interface using default implementation with
 * compiler imposed optimization
 *
 * This function perform gamma correction using
 * 256 byte Lookup Table
 *
 * @param input     input image pointer
 * @param output    output image buffer pointer
 * @param pitch     input image pitch
 * @param width     image width
 * @param height    image height
 * @param format    image color format
 * @param lut       lookup table
 * @return error code
 */
aipl_error_t aipl_lut_transform_rgb_default(const void* input, void* output,
                                            uint32_t pitch,
                                            uint32_t width, uint32_t height,
                                            aipl_color_format_t format,
                                            uint8_t* lut);

/**
 * Perform a LUT transformation of RGB channels of an image
 * with aipl_image_t interface using default implementation with
 * compiler imposed optimization
 *
 * @param input     input image
 * @param output    output image
 * @param lut       lookup table
 * @return error code
 */
aipl_error_t aipl_lut_transform_rgb_img_default(const aipl_image_t* input,
                                                aipl_image_t* output,
                                                uint8_t* lut);

/**
 * Perform a gamma correcton of an ARGB8888 image
 * using default implementation with compiler imposed optimization
 *
 *
 * @param input     input image pointer
 * @param output    output image buffer pointer
 * @param pitch     input image pitch
 * @param width     image width
 * @param height    image height
 * @param lut       lookup table
 * @return error code
 */
aipl_error_t aipl_lut_transform_argb8888_default(const void* input, void* output,
                                                 uint32_t pitch,
                                                 uint32_t width, uint32_t height,
                                                 uint8_t* lut);

/**
 * Perform a gamma correcton of an ARGB4444 image
 * using default implementation with compiler imposed optimization
 *
 *
 * @param input     input image pointer
 * @param output    output image buffer pointer
 * @param pitch     input image pitch
 * @param width     image width
 * @param height    image height
 * @param lut       lookup table
 * @return error code
 */
aipl_error_t aipl_lut_transform_argb4444_default(const void* input, void* output,
                                                 uint32_t pitch,
                                                 uint32_t width, uint32_t height,
                                                 uint8_t* lut);

/**
 * Perform a gamma correcton of an ARGB1555 image
 * using default implementation with compiler imposed optimization
 *
 *
 * @param input     input image pointer
 * @param output    output image buffer pointer
 * @param pitch     input image pitch
 * @param width     image width
 * @param height    image height
 * @param lut       lookup table
 * @return error code
 */
aipl_error_t aipl_lut_transform_argb1555_default(const void* input, void* output,
                                                 uint32_t pitch,
                                                 uint32_t width, uint32_t height,
                                                 uint8_t* lut);

/**
 * Perform a gamma correcton of an RGBA8888 image
 * using default implementation with compiler imposed optimization
 *
 *
 * @param input     input image pointer
 * @param output    output image buffer pointer
 * @param pitch     input image pitch
 * @param width     image width
 * @param height    image height
 * @param lut       lookup table
 * @return error code
 */
aipl_error_t aipl_lut_transform_rgba8888_default(const void* input, void* output,
                                                 uint32_t pitch,
                                                 uint32_t width, uint32_t height,
                                                 uint8_t* lut);

/**
 * Perform a gamma correcton of an RGBA4444 image
 * using default implementation with compiler imposed optimization
 *
 *
 * @param input     input image pointer
 * @param output    output image buffer pointer
 * @param pitch     input image pitch
 * @param width     image width
 * @param height    image height
 * @param lut       lookup table
 * @return error code
 */
aipl_error_t aipl_lut_transform_rgba4444_default(const void* input, void* output,
                                                 uint32_t pitch,
                                                 uint32_t width, uint32_t height,
                                                 uint8_t* lut);

/**
 * Perform a gamma correcton of an RGBA5551 image
 * using default implementation with compiler imposed optimization
 *
 *
 * @param input     input image pointer
 * @param output    output image buffer pointer
 * @param pitch     input image pitch
 * @param width     image width
 * @param height    image height
 * @param lut       lookup table
 * @return error code
 */
aipl_error_t aipl_lut_transform_rgba5551_default(const void* input, void* output,
                                                 uint32_t pitch,
                                                 uint32_t width, uint32_t height,
                                                 uint8_t* lut);

/**
 * Perform a gamma correcton of an BGR888 image
 * using default implementation with compiler imposed optimization
 *
 *
 * @param input     input image pointer
 * @param output    output image buffer pointer
 * @param pitch     input image pitch
 * @param width     image width
 * @param height    image height
 * @param lut       lookup table
 * @return error code
 */
aipl_error_t aipl_lut_transform_bgr888_default(const void* input, void* output,
                                               uint32_t pitch,
                                               uint32_t width, uint32_t height,
                                               uint8_t* lut);

/**
 * Perform a gamma correcton of an RGB888 image
 * using default implementation with compiler imposed optimization
 *
 *
 * @param input     input image pointer
 * @param output    output image buffer pointer
 * @param pitch     input image pitch
 * @param width     image width
 * @param height    image height
 * @param lut       lookup table
 * @return error code
 */
aipl_error_t aipl_lut_transform_rgb888_default(const void* input, void* output,
                                               uint32_t pitch,
                                               uint32_t width, uint32_t height,
                                               uint8_t* lut);


/**
 * Perform a gamma correcton of an RGB565 image
 * using default implementation with compiler imposed optimization
 *
 *
 * @param input     input image pointer
 * @param output    output image buffer pointer
 * @param pitch     input image pitch
 * @param width     image width
 * @param height    image height
 * @param lut       lookup table
 * @return error code
 */
aipl_error_t aipl_lut_transform_rgb565_default(const void* input, void* output,
                                               uint32_t pitch,
                                               uint32_t width, uint32_t height,
                                               uint8_t* lut);

/**********************
 *      MACROS
 **********************/

#endif

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  /* AIPL_LUT_TRANSFORM_DEFAULT_H */

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
 * @file    aipl_white_balance_default.h
 * @brief   Default white balance function definitions
 *
******************************************************************************/

#ifndef AIPL_WHITE_BALANCE_DEFAULT_H
#define AIPL_WHITE_BALANCE_DEFAULT_H

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
 * Apply given color miltipliers to white balance an RGB image
 * with raw pointer interface using default implementation with
 * compiler imposed optimization
 *
 * r' = ar * r
 * g' = ag * g
 * b' = ab * b
 *
 * @param input     input image pointer
 * @param output    output image buffer pointer
 * @param pitch     input image pitch
 * @param width     image width
 * @param height    image height
 * @param format    image color format
 * @param ar        red channel multiplier
 * @param ag        green channel multiplier
 * @param ab        blue channel multiplier
 * @return error code
 */
aipl_error_t aipl_white_balance_rgb_default(const void* input, void* output,
                                            uint32_t pitch,
                                            uint32_t width, uint32_t height,
                                            aipl_color_format_t format,
                                            float ar, float ag, float ab);

/**
 * Apply given color multipliers to white balance an RGB image
 * with aipl_image_t interface using default implementation with
 * compiler imposed optimization
 *
 * @param input     input image
 * @param output    output image
 * @param ar        red channel multiplier
 * @param ag        green channel multiplier
 * @param ab        blue channel multiplier
 * @return error code
 */
aipl_error_t aipl_white_balance_rgb_img_default(const aipl_image_t* input,
                                                aipl_image_t* output,
                                                float ar, float ag, float ab);

/**
 * Apply given color multipliers to white balance an ARGB8888 image
 * using default implementation with compiler imposed optimization
 *
 *
 * @param input         input image pointer
 * @param output        output image buffer pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @param format        image color format
 * @param ar            red channel multiplier
 * @param ag            green channel multiplier
 * @param ab            blue channel multiplier
 * @return error code
 */
aipl_error_t aipl_white_balance_argb8888_default(const void* input, void* output,
                                                 uint32_t pitch,
                                                 uint32_t width, uint32_t height,
                                                 float ar, float ag, float ab);

/**
 * Apply given color multipliers to white balance an ARGB4444 image
 * using default implementation with compiler imposed optimization
 *
 *
 * @param input     input image pointer
 * @param output    output image buffer pointer
 * @param pitch     input image pitch
 * @param width     image width
 * @param height    image height
 * @param format    image color format
 * @param ar        red channel multiplier
 * @param ag        green channel multiplier
 * @param ab        blue channel multiplier
 * @return error code
 */
aipl_error_t aipl_white_balance_argb4444_default(const void* input, void* output,
                                                 uint32_t pitch,
                                                 uint32_t width, uint32_t height,
                                                 float ar, float ag, float ab);

/**
 * Apply given color multipliers to white balance an ARGB1555 image
 * using default implementation with compiler imposed optimization
 *
 *
 * @param input     input image pointer
 * @param output    output image buffer pointer
 * @param pitch     input image pitch
 * @param width     image width
 * @param height    image height
 * @param format    image color format
 * @param ar        red channel multiplier
 * @param ag        green channel multiplier
 * @param ab        blue channel multiplier
 * @return error code
 */
aipl_error_t aipl_white_balance_argb1555_default(const void* input, void* output,
                                                 uint32_t pitch,
                                                 uint32_t width, uint32_t height,
                                                 float ar, float ag, float ab);

/**
 * Apply given color multipliers to white balance an RGBA8888 image
 * using default implementation with compiler imposed optimization
 *
 *
 * @param input     input image pointer
 * @param output    output image buffer pointer
 * @param pitch     input image pitch
 * @param width     image width
 * @param height    image height
 * @param format    image color format
 * @param ar        red channel multiplier
 * @param ag        green channel multiplier
 * @param ab        blue channel multiplier
 * @return error code
 */
aipl_error_t aipl_white_balance_rgba8888_default(const void* input, void* output,
                                                 uint32_t pitch,
                                                 uint32_t width, uint32_t height,
                                                 float ar, float ag, float ab);

/**
 * Apply given color multipliers to white balance an RGBA4444 image
 * using default implementation with compiler imposed optimization
 *
 *
 * @param input     input image pointer
 * @param output    output image buffer pointer
 * @param pitch     input image pitch
 * @param width     image width
 * @param height    image height
 * @param format    image color format
 * @param ar        red channel multiplier
 * @param ag        green channel multiplier
 * @param ab        blue channel multiplier
 * @return error code
 */
aipl_error_t aipl_white_balance_rgba4444_default(const void* input, void* output,
                                                 uint32_t pitch,
                                                 uint32_t width, uint32_t height,
                                                 float ar, float ag, float ab);

/**
 * Apply given color multipliers to white balance an RGBA5551 image
 * using default implementation with compiler imposed optimization
 *
 *
 * @param input     input image pointer
 * @param output    output image buffer pointer
 * @param pitch     input image pitch
 * @param width     image width
 * @param height    image height
 * @param format    image color format
 * @param ar        red channel multiplier
 * @param ag        green channel multiplier
 * @param ab        blue channel multiplier
 * @return error code
 */
aipl_error_t aipl_white_balance_rgba5551_default(const void* input, void* output,
                                                 uint32_t pitch,
                                                 uint32_t width, uint32_t height,
                                                 float ar, float ag, float ab);

/**
 * Apply given color multipliers to white balance an BGR888 image
 * using default implementation with compiler imposed optimization
 *
 *
 * @param input     input image pointer
 * @param output    output image buffer pointer
 * @param pitch     input image pitch
 * @param width     image width
 * @param height    image height
 * @param format    image color format
 * @param ar        red channel multiplier
 * @param ag        green channel multiplier
 * @param ab        blue channel multiplier
 * @return error code
 */
aipl_error_t aipl_white_balance_bgr888_default(const void* input, void* output,
                                               uint32_t pitch,
                                               uint32_t width, uint32_t height,
                                               float ar, float ag, float ab);

/**
 * Apply given color multipliers to white balance an RGB888 image
 * using default implementation with compiler imposed optimization
 *
 *
 * @param input     input image pointer
 * @param output    output image buffer pointer
 * @param pitch     input image pitch
 * @param width     image width
 * @param height    image height
 * @param format    image color format
 * @param ar        red channel multiplier
 * @param ag        green channel multiplier
 * @param ab        blue channel multiplier
 * @return error code
 */
aipl_error_t aipl_white_balance_rgb888_default(const void* input, void* output,
                                               uint32_t pitch,
                                               uint32_t width, uint32_t height,
                                               float ar, float ag, float ab);

/**
 * Apply given color multipliers to white balance an RGB565 image
 * using default implementation with compiler imposed optimization
 *
 *
 * @param input     input image pointer
 * @param output    output image buffer pointer
 * @param pitch     input image pitch
 * @param width     image width
 * @param height    image height
 * @param format    image color format
 * @param ar        red channel multiplier
 * @param ag        green channel multiplier
 * @param ab        blue channel multiplier
 * @return error code
 */
aipl_error_t aipl_white_balance_rgb565_default(const void* input, void* output,
                                               uint32_t pitch,
                                               uint32_t width, uint32_t height,
                                               float ar, float ag, float ab);

/**********************
 *      MACROS
 **********************/

#endif

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  /* AIPL_WHITE_BALANCE_DEFAULT_H */

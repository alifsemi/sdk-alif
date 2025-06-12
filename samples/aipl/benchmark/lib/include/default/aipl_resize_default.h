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
 * @file    aipl_resize_default.h
 * @brief   Default resize function definitions
 *
******************************************************************************/

#ifndef AIPL_RESIZE_DEFAULT_H
#define AIPL_RESIZE_DEFAULT_H

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
 * Resize image with raw pointer interface using default
 * implementation with compiler imposed optimization
 *
 * @param input             input image pointer
 * @param output            output image pointer
 * @param pitch             input image pitch
 * @param width             input image width
 * @param height            input image height
 * @param format            input image format
 * @param output_width      output image width
 * @param output_height     output image height
 * @param interpolate       apply bilinear interpolation
 * @return error code
 */
aipl_error_t aipl_resize_default(const void* input, void* output,
                                 uint32_t pitch,
                                 uint32_t width, uint32_t height,
                                 aipl_color_format_t format,
                                 uint32_t output_width, uint32_t output_height,
                                 bool interpolate);

/**
 * Resize image with aipl_image_t interface using default
 * implementation with compiler imposed optimization
 *
 * @param input             input image
 * @param output            output image
 * @param interpolate       apply bilinear interpolation
 * @return error code
 */
aipl_error_t aipl_resize_img_default(const aipl_image_t* input,
                                     aipl_image_t* output,
                                     bool interpolate);

/**********************
 *      MACROS
 **********************/

#endif

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  /* AIPL_RESIZE_DEFAULT_H */

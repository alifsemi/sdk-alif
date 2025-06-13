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
 * @file    aipl_dave2d.h
 * @brief   D/AVE2D related function definitions
 *
******************************************************************************/

#ifndef AIPL_DAVE2D_H
#define AIPL_DAVE2D_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <stdbool.h>

#include "dave_driver.h"

#include "aipl_config.h"
#include "aipl_color_formats.h"
#include "aipl_error.h"

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
 * AIPL implementation of D/AVE2D initialization
 *
 * D/AVE2D device should be initialized before usage.
 *
 * This function is used when AIPL_CUSTOM_DAVE2D_INIT set to 0
 *
 * @return D/AVE2D driver error code (see dave_errorcodes.h)
 */
#if !AIPL_CUSTOM_DAVE2D_INIT
d2_s32 aipl_dave2d_init(void);
#endif

/**
 * Returns a handle to the initialized D/AVE2D device that
 * is responsible for image processing
 *
 * @return D/AVE2D device handle
 */
d2_device* aipl_dave2d_handle(void);

/**
 * Check if D/AVE2D color format has alpha channel
 *
 * @param mode color mode in question
 * @return true if mode has alpha; false otherwise
 */
bool aipl_dave2d_mode_has_alpha(d2_u32 mode);

/**
 * Check if D/AVE2D supports color format
 *
 * @param format AIPL color format in question
 * @return true if supports; false otherwise
 */
bool aipl_dave2d_format_supported(aipl_color_format_t format);

/**
 * Convert AIPL color format to D/AVE2D
 *
 * @param format AIPL color format
 * @return D/AVE2D color format
 */
d2_u32 aipl_dave2d_format_to_mode(aipl_color_format_t format);

/**
 * Get D/AVE2D mode pixel size
 *
 * @param mode D/AVE2D color mode
 * @return color mode pixel size in bytes
 */
d2_u32 aipl_dave2d_mode_px_size(d2_u32 mode);

/**
 * Convert image color modes using D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @param input_format  input image format
 * @param output_format output image format
 * @return D/AVE2D driver error code (see dave_errorcodes.h)
 */
d2_u32 aipl_dave2d_color_mode_convert(const void* input, void* output,
                                      uint32_t pitch,
                                      uint32_t width, uint32_t height,
                                      d2_u32 input_mode,
                                      d2_u32 output_mode);


/**
 * Apply input image as texture and output to the output image
 * using D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @param format        input image format
 * @param output_width  output image width
 * @param output_height output image height
 * @param x             x
 * @param y             y
 * @param rotation      rotation
 * @param flip_u        flip horizontally
 * @param flip_v        flip vertically
 * @param scale         enable scaling
 * @param interpolate   apply bilinear filter
 * @return D/AVE2D driver error code (see dave_errorcodes.h)
 */
d2_u32 aipl_dave2d_texturing(const void* input, void* output,
                             uint32_t pitch,
                             uint32_t width, uint32_t height,
                             d2_u32 format,
                             uint32_t output_width, uint32_t output_height,
                             int32_t x, int32_t y,
                             int32_t rotation,
                             bool flip_u, bool flip_v,
                             bool scale, bool interpolate);
/**
 * Convert D/AVE2D driver error code to AIPL error code
 * Save the last converted original D/AVE2D driver error code
 *
 * @param error D/AVE2D error code
 * @return AIPL error code
 */
aipl_error_t aipl_dave2d_error_convert(d2_s32 error);

/**
 * Get the last converted original D/AVE2D error code
 *
 * @return D/AVE2D driver error code
 */
d2_u32 aipl_dave2d_get_last_converted_error(void);

/**
 * Check if D/AVE2D produces valid result with specified output format
 *
 * @param format output color format
 * @return true if D/AVE2D produces valid result
 */
bool aipl_dave2d_check_output_format(aipl_color_format_t format);

/**
 * Determine if the color format conversion shuld be performed on D/AVE2D
 *
 * @param input_format  input color format
 * @param output_format output color format
 * @return true if D/AVE2D is suitable
 */
bool aipl_dave2d_color_convert_suitable(aipl_color_format_t input_format,
                                        aipl_color_format_t output_format);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*AIPL_DAVE2D_H*/

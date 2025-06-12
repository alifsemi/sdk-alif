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
 * @file    aipl_flip_dave2d.h
 * @brief   D/AVE2D accelerated flip function definitions
 *
******************************************************************************/

#ifndef AIPL_FLIP_DAVE2D_H
#define AIPL_FLIP_DAVE2D_H

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

#ifdef AIPL_DAVE2D_ACCELERATION

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
 * Flip image using raw pointer interface with D/AVE2D
 *
 * @param input             input image pointer
 * @param output            output image pointer
 * @param pitch             input image pitch
 * @param width             input image width
 * @param height            input image height
 * @param format            input image format
 * @param flip_horizontal   flip horizontal
 * @param flip_vertical     flip vertical
 * @return error code
 */
aipl_error_t aipl_flip_dave2d(const void* input, void* output,
                              uint32_t pitch,
                              uint32_t width, uint32_t height,
                              aipl_color_format_t format,
                              bool flip_horizontal, bool flip_vertical);

/**
 * Flip image using raw pointer interface with D/AVE2D
 *
 * @param input             input image
 * @param output            output image
 * @param flip_horizontal   flip horizontal
 * @param flip_vertical     flip vertical
 * @return error code
 */
aipl_error_t aipl_flip_img_dave2d(const aipl_image_t* input,
                                  aipl_image_t* output,
                                  bool flip_horizontal, bool flip_vertical);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* AIPL_DAVE2D_ACCELERATION */

#endif  /* AIPL_FLIP_DAVE2D_H */


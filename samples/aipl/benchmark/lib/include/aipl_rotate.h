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
 * @file    aipl_rotate.h
 * @brief   Rotate function definitions
 *
******************************************************************************/

#ifndef AIPL_ROTATE_H
#define AIPL_ROTATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "aipl_color_formats.h"
#include "aipl_error.h"
#include "aipl_image.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
    AIPL_ROTATE_0 = 0,
    AIPL_ROTATE_90 = 90,
    AIPL_ROTATE_180 = 180,
    AIPL_ROTATE_270 = 270
} aipl_rotation_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Rotate an image in 90-degree steps using raw pointer interface
 *
 * @param input             input image pointer
 * @param output            output image pointer
 * @param pitch             input image pitch
 * @param input_width       input image width
 * @param input_height      input image height
 * @param format            input image format
 * @param rotation          rotation angle
 * @return error code
 */
aipl_error_t aipl_rotate(const void* input, void* output,
                         uint32_t pitch,
                         uint32_t width, uint32_t height,
                         aipl_color_format_t format,
                         aipl_rotation_t rotation);

/**
 * Rotate an image in 90-degree steps using aipl_image_t interface
 *
 * @param input             input image
 * @param output            output image
 * @param rotation          rotation angle
 * @return error code
 */
aipl_error_t aipl_rotate_img(const aipl_image_t* input,
                             aipl_image_t* output,
                             aipl_rotation_t rotation);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  /* AIPL_ROTATE_H */


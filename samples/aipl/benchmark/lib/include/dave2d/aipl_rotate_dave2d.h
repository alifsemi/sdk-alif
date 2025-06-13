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
 * @file    aipl_rotate_dave2d.h
 * @brief   D/AVE2D accelerated rotate function definitions
 *
******************************************************************************/

#ifndef AIPL_ROTATE_DAVE2D_H
#define AIPL_ROTATE_DAVE2D_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "aipl_color_formats.h"
#include "aipl_error.h"
#include "aipl_image.h"
#include "aipl_rotate.h"
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
 * Rotate an image in 90-degree steps using raw pointer interface
 * with D/AVE2D
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
aipl_error_t aipl_rotate_dave2d(const void* input, void* output,
                                uint32_t pitch,
                                uint32_t width, uint32_t height,
                                aipl_color_format_t format,
                                aipl_rotation_t rotation);

/**
 * Rotate an image in 90-degree steps using aipl_image_t interface
 * with D/AVE2D
 *
 * @param input             input image
 * @param output            output image
 * @param rotation          rotation angle
 * @return error code
 */
aipl_error_t aipl_rotate_img_dave2d(const aipl_image_t* input,
                                    aipl_image_t* output,
                                    aipl_rotation_t rotation);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* AIPL_DAVE2D_ACCELERATION */

#endif  /* AIPL_ROTATE_DAVE2D_H */


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
 * @file    aipl_image.h
 * @brief   Image type and utils definitions
 *
******************************************************************************/

#ifndef AIPL_IMAGE_H
#define AIPL_IMAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "aipl_color_formats.h"
#include "aipl_error.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    void* data;
    uint32_t pitch;
    uint32_t width;
    uint32_t height;
    aipl_color_format_t format;
} aipl_image_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
/**
 * Create AIPL image structure. Allocate image buffer in videomemory
 *
 * @param image  image struct pointer
 * @param pitch  image pitch
 * @param width  image width
 * @param height image height
 * @param format image format
 * @return AIPL error code
 */
aipl_error_t aipl_image_create(aipl_image_t* image, uint32_t pitch,
                               uint32_t width, uint32_t height,
                               aipl_color_format_t format);

/**
 * Deallocate image buffer
 *
 * @param image image pointer
 */
void aipl_image_destroy(aipl_image_t* image);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  /* AIPL_IMAGE_H */

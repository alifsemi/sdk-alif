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
 * @file    aipl_crop.h
 * @brief   Crop function definitions
 *
******************************************************************************/

#ifndef AIPL_CROP_H
#define AIPL_CROP_H

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

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Crop a rectangular part of the image
 * using raw pointer interface
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         input image width
 * @param height        input image height
 * @param format        input image format
 * @param left          left coordinate of the cropping area
 * @param top           top coordinate of the cropping area
 * @param right         right coordinate of the cropping area
 * @param bottom        bottom coordinate of the cropping area
 * @return error code
 */
aipl_error_t aipl_crop(const void* input, void* output,
                       uint32_t pitch,
                       uint32_t width, uint32_t height,
                       aipl_color_format_t format,
                       uint32_t left, uint32_t top,
                       uint32_t right, uint32_t bottom);

/**
 * Crop a rectangular part of the image
 * using aipl_image_t interface
 *
 * @param input         input image
 * @param output        output image
 * @param left          left coordinate of the cropping area
 * @param top           top coordinate of the cropping area
 * @param right         right coordinate of the cropping area
 * @param bottom        bottom coordinate of the cropping area
 * @return error code
 */
aipl_error_t aipl_crop_img(const aipl_image_t* input,
                           aipl_image_t* output,
                           uint32_t left, uint32_t top,
                           uint32_t right, uint32_t bottom);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  /* AIPL_CROP_H */


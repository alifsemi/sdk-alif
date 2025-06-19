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
 * @file    aipl_color_formats.h
 * @brief   Color formats type and utils definitions
 *
******************************************************************************/

#ifndef AIPL_COLOR_FORMATS_H
#define AIPL_COLOR_FORMATS_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <stdint.h>
#include <stdbool.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
    AIPL_SPACE_RGB,
    AIPL_SPACE_YUV
} aipl_color_space_t;

typedef enum {
    /* Alpha color formats */
    AIPL_COLOR_ALPHA8,

    /* RGB color formats */
    AIPL_COLOR_ARGB8888,
    AIPL_COLOR_ARGB4444,
    AIPL_COLOR_ARGB1555,
    AIPL_COLOR_RGBA8888,
    AIPL_COLOR_RGBA4444,
    AIPL_COLOR_RGBA5551,
    AIPL_COLOR_BGR888,
    AIPL_COLOR_RGB888,
    AIPL_COLOR_RGB565,

    /* YUV color formats */
    AIPL_COLOR_YV12,
    AIPL_COLOR_I420,
    AIPL_COLOR_I422,
    AIPL_COLOR_I444,
    AIPL_COLOR_I400,
    AIPL_COLOR_NV21,
    AIPL_COLOR_NV12,
    AIPL_COLOR_YUY2,
    AIPL_COLOR_UYVY,

    /* Other format flags */
    AIPL_COLOR_RLE = 0x80000000,

    /* Unknown color value */
    AIPL_COLOR_UNKNOWN = 0xffffffff
} aipl_color_format_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
/**
 * Get the color format space
 *
 * @param format color format
 * @return color format space
 */
aipl_color_space_t aipl_color_format_space(aipl_color_format_t format);

/**
 * Get the color format depth in bits
 *
 * @param format color format
 * @return color format depth
 */
uint8_t aipl_color_format_depth(aipl_color_format_t format);

/**
 * Get color format string representation
 *
 * @param format color format
 * @return static color format string
 */
const char* aipl_color_format_str(aipl_color_format_t format);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  /* AIPL_COLOR_FORMATS_H */

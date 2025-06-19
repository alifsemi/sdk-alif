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
 * @file    aipl_video_alloc.h
 * @brief   Video memory allocation function definitions
 *
******************************************************************************/

#ifndef AIPL_VIDEO_ALLOC_H
#define AIPL_VIDEO_ALLOC_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <stdint.h>

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
 * Allocate a buffer in the video memory
 *
 * In the case config option AIPL_CUSTOM_VIDEO_ALLOC is set to 1,
 * should be provided by the user
 *
 * @param size desired buffer size
 * @return pointer to the allocated memory; NULL on failure
 */
void* aipl_video_alloc(uint32_t size);

/**
 * Free a buffer in the video memory
 *
 * In the case config option AIPL_CUSTOM_VIDEO_ALLOC is set to 1,
 * should be provided by the user
 *
 * @param ptr buffer pointer
 */
void aipl_video_free(void* ptr);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  /* AIPL_VIDEO_ALLOC_H */

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
 * @file    aipl_cache.h
 * @brief   CPU cache management function definitions
 *
******************************************************************************/

#ifndef AIPL_CACHE_H
#define AIPL_CACHE_H

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
 * Clean CPU cache
 *
 * The function implementation should be provided by the user
 *
 * @param ptr   pointer of the buffer to clean
 * @param size  buffer size
 */
void aipl_cpu_cache_clean(const void* ptr, uint32_t size);

/**
 * Invalidate CPU cache
 *
 * The function implementation should be provided by the user
 *
 * @param ptr   pointer of the buffer to invalidate
 * @param size  buffer size
 */
void aipl_cpu_cache_invalidate(const void* ptr, uint32_t size);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  /* AIPL_CACHE_H */

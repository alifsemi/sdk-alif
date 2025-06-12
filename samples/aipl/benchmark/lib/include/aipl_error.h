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
 * @file    aipl_error.h
 * @brief   Error type and utils definitions
 *
******************************************************************************/

#ifndef AIPL_ERROR_H
#define AIPL_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
    AIPL_ERR_OK = 0,                    /* Success */
    AIPL_ERR_NO_MEM,                    /* Failed memory allocation */
    AIPL_ERR_NULL_POINTER,              /* A null pointer was passed to a function */
    AIPL_ERR_UNSUPPORTED_FORMAT,        /* The color format is unsupported by the called function */
    AIPL_ERR_FORMAT_MISMATCH,           /* Incorrect color format arguments */
    AIPL_ERR_SIZE_MISMATCH,             /* Input and output image size mismatch */
    AIPL_ERR_FRAME_OUT_OF_RANGE,        /* Function arguments are out of frame bounds */
    AIPL_ERR_D2,                        /* D/AVE2D driver error. See aipl_dave2d_get_last_converted_error() */
    AIPL_ERR_BAYER_INVALID_METHOD,      /* Invalid bayer method (See aipl_bayer_method_t) */
    AIPL_ERR_BAYER_INVALID_FILTER,      /* Invalid bayer filter value (See aipl_color_filter_t) */
    AIPL_ERR_NOT_SUPPORTED = 0xffff,    /* The specified function is not supported*/
} aipl_error_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
/**
 * Get string representation of an error code
 *
 * @param error error code
 * @return static error string
 */
const char* aipl_error_str(aipl_error_t error);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  /* AIPL_ERROR_H */

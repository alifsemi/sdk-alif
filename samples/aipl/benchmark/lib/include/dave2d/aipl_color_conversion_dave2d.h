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
 * @file    aipl_color_conversion_dave2d.h
 * @brief   D/AVE2D accelerated color conversion function definitions
 *
******************************************************************************/

#ifndef AIPL_COLOR_CONVERSION_DAVE2D_H
#define AIPL_COLOR_CONVERSION_DAVE2D_H

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
 * Convert image color format using raw pointer interface
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @param input_format  input image format
 * @param output_format output image format
 * @return error code
 */
aipl_error_t aipl_color_convert_dave2d(const void* input, void* output,
                                       uint32_t pitch,
                                       uint32_t width, uint32_t height,
                                       aipl_color_format_t input_format,
                                       aipl_color_format_t output_format);

/**
 * Convert image color format using aipl_image_t interface
 * with D/AVE2D
 *
 * @param input         input image
 * @param output        output image
 * @return error code
 */
aipl_error_t aipl_color_convert_img_dave2d(const aipl_image_t* input,
                                           aipl_image_t* output);

/**
 * Convert ALPHA8 image to specified format
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @param format        desired color format
 * @return error code
 */
aipl_error_t aipl_color_convert_alpha8_dave2d(const void* input, void* output,
                                              uint32_t pitch,
                                              uint32_t width, uint32_t height,
                                              aipl_color_format_t format);

/**
 * Convert ALPHA8 image to ARGB8888
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
aipl_error_t aipl_color_convert_alpha8_to_argb8888_dave2d(const void* input,
                                                          void* output,
                                                          uint32_t pitch,
                                                          uint32_t width,
                                                          uint32_t height);

/**
 * Convert ALPHA8 image to ARGB4444
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
aipl_error_t aipl_color_convert_alpha8_to_argb4444_dave2d(const void* input,
                                                          void* output,
                                                          uint32_t pitch,
                                                          uint32_t width,
                                                          uint32_t height);

/**
 * Convert ALPHA8 image to ARGB1555
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
// aipl_error_t aipl_color_convert_alpha8_to_argb1555_dave2d(const void* input,
//                                                           void* output,
//                                                           uint32_t pitch,
//                                                           uint32_t width,
//                                                           uint32_t height);

/**
 * Convert ALPHA8 image to RGBA8888
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
aipl_error_t aipl_color_convert_alpha8_to_rgba8888_dave2d(const void* input,
                                                          void* output,
                                                          uint32_t pitch,
                                                          uint32_t width,
                                                          uint32_t height);

/**
 * Convert ALPHA8 image to RGBA4444
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
aipl_error_t aipl_color_convert_alpha8_to_rgba4444_dave2d(const void* input,
                                                          void* output,
                                                          uint32_t pitch,
                                                          uint32_t width,
                                                          uint32_t height);

/**
 * Convert ALPHA8 image to RGBA5551
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
// aipl_error_t aipl_color_convert_alpha8_to_rgba5551_dave2d(const void* input,
//                                                           void* output,
//                                                           uint32_t pitch,
//                                                           uint32_t width,
//                                                           uint32_t height);

/**
 * Convert ALPHA8 image to RGB565
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
aipl_error_t aipl_color_convert_alpha8_to_rgb565_dave2d(const void* input,
                                                        void* output,
                                                        uint32_t pitch,
                                                        uint32_t width,
                                                        uint32_t height);


/**
 * Convert ARGB8888 image to specified format
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @param format        desired color format
 * @return error code
 */
aipl_error_t aipl_color_convert_argb8888_dave2d(const void* input, void* output,
                                                uint32_t pitch,
                                                uint32_t width, uint32_t height,
                                                aipl_color_format_t format);

/**
 * Convert ARGB8888 image to ALPHA8
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
// aipl_error_t aipl_color_convert_argb8888_to_alpha8_dave2d(const void* input,
//                                                           void* output,
//                                                           uint32_t pitch,
//                                                           uint32_t width,
//                                                           uint32_t height);


/**
 * Convert ARGB8888 image to ARGB4444
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
aipl_error_t aipl_color_convert_argb8888_to_argb4444_dave2d(const void* input,
                                                            void* output,
                                                            uint32_t pitch,
                                                            uint32_t width,
                                                            uint32_t height);

/**
 * Convert ARGB8888 image to ARGB1555
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
// aipl_error_t aipl_color_convert_argb8888_to_argb1555_dave2d(const void* input,
//                                                             void* output,
//                                                             uint32_t pitch,
//                                                             uint32_t width,
//                                                             uint32_t height);

/**
 * Convert ARGB8888 image to RGBA8888
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
aipl_error_t aipl_color_convert_argb8888_to_rgba8888_dave2d(const void* input,
                                                            void* output,
                                                            uint32_t pitch,
                                                            uint32_t width,
                                                            uint32_t height);

/**
 * Convert ARGB8888 image to RGBA4444
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
aipl_error_t aipl_color_convert_argb8888_to_rgba4444_dave2d(const void* input,
                                                            void* output,
                                                            uint32_t pitch,
                                                            uint32_t width,
                                                            uint32_t height);

/**
 * Convert ARGB8888 image to RGBA5551
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
// aipl_error_t aipl_color_convert_argb8888_to_rgba5551_dave2d(const void* input,
//                                                             void* output,
//                                                             uint32_t pitch,
//                                                             uint32_t width,
//                                                             uint32_t height);

/**
 * Convert ARGB8888 image to RGB565
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
aipl_error_t aipl_color_convert_argb8888_to_rgb565_dave2d(const void* input,
                                                          void* output,
                                                          uint32_t pitch,
                                                          uint32_t width,
                                                          uint32_t height);

/**
 * Convert ARGB4444 image to specified format
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @param format        desired color format
 * @return error code
 */
aipl_error_t aipl_color_convert_argb4444_dave2d(const void* input, void* output,
                                                uint32_t pitch,
                                                uint32_t width, uint32_t height,
                                                aipl_color_format_t format);

/**
 * Convert ARGB4444 image to ALPHA8
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
// aipl_error_t aipl_color_convert_argb4444_to_alpha8_dave2d(const void* input,
//                                                           void* output,
//                                                           uint32_t pitch,
//                                                           uint32_t width,
//                                                           uint32_t height);

/**
 * Convert ARGB4444 image to ARGB8888
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
aipl_error_t aipl_color_convert_argb4444_to_argb8888_dave2d(const void* input,
                                                            void* output,
                                                            uint32_t pitch,
                                                            uint32_t width,
                                                            uint32_t height);

/**
 * Convert ARGB4444 image to ARGB1555
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
// aipl_error_t aipl_color_convert_argb4444_to_argb1555_dave2d(const void* input,
//                                                             void* output,
//                                                             uint32_t pitch,
//                                                             uint32_t width,
//                                                             uint32_t height);

/**
 * Convert ARGB4444 image to RGBA8888
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
aipl_error_t aipl_color_convert_argb4444_to_rgba8888_dave2d(const void* input,
                                                            void* output,
                                                            uint32_t pitch,
                                                            uint32_t width,
                                                            uint32_t height);

/**
 * Convert ARGB4444 image to RGBA4444
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
aipl_error_t aipl_color_convert_argb4444_to_rgba4444_dave2d(const void* input,
                                                            void* output,
                                                            uint32_t pitch,
                                                            uint32_t width,
                                                            uint32_t height);

/**
 * Convert ARGB4444 image to RGBA5551
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
// aipl_error_t aipl_color_convert_argb4444_to_rgba5551_dave2d(const void* input,
//                                                             void* output,
//                                                             uint32_t pitch,
//                                                             uint32_t width,
//                                                             uint32_t height);

/**
 * Convert ARGB4444 image to RGB565
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
aipl_error_t aipl_color_convert_argb4444_to_rgb565_dave2d(const void* input,
                                                          void* output,
                                                          uint32_t pitch,
                                                          uint32_t width,
                                                          uint32_t height);

/**
 * Convert ARGB1555 image to specified format
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @param format        desired color format
 * @return error code
 */
aipl_error_t aipl_color_convert_argb1555_dave2d(const void* input, void* output,
                                                uint32_t pitch,
                                                uint32_t width, uint32_t height,
                                                aipl_color_format_t format);

/**
 * Convert ARGB1555 image to ALPHA8
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
// aipl_error_t aipl_color_convert_argb1555_to_alpha8_dave2d(const void* input,
//                                                           void* output,
//                                                           uint32_t pitch,
//                                                           uint32_t width,
//                                                           uint32_t height);

/**
 * Convert ARGB1555 image to ARGB8888
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
aipl_error_t aipl_color_convert_argb1555_to_argb8888_dave2d(const void* input,
                                                            void* output,
                                                            uint32_t pitch,
                                                            uint32_t width,
                                                            uint32_t height);

/**
 * Convert ARGB1555 image to ARGB4444
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
aipl_error_t aipl_color_convert_argb1555_to_argb4444_dave2d(const void* input,
                                                            void* output,
                                                            uint32_t pitch,
                                                            uint32_t width,
                                                            uint32_t height);

/**
 * Convert ARGB1555 image to RGBA8888
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
aipl_error_t aipl_color_convert_argb1555_to_rgba8888_dave2d(const void* input,
                                                            void* output,
                                                            uint32_t pitch,
                                                            uint32_t width,
                                                            uint32_t height);

/**
 * Convert ARGB1555 image to RGBA4444
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
aipl_error_t aipl_color_convert_argb1555_to_rgba4444_dave2d(const void* input,
                                                            void* output,
                                                            uint32_t pitch,
                                                            uint32_t width,
                                                            uint32_t height);

/**
 * Convert ARGB1555 image to RGBA5551
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
// aipl_error_t aipl_color_convert_argb1555_to_rgba5551_dave2d(const void* input,
//                                                             void* output,
//                                                             uint32_t pitch,
//                                                             uint32_t width,
//                                                             uint32_t height);

/**
 * Convert ARGB1555 image to RGB565
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
aipl_error_t aipl_color_convert_argb1555_to_rgb565_dave2d(const void* input,
                                                          void* output,
                                                          uint32_t pitch,
                                                          uint32_t width,
                                                          uint32_t height);

/**
 * Convert RGBA8888 image to specified format
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @param format        desired color format
 * @return error code
 */
aipl_error_t aipl_color_convert_rgba8888_dave2d(const void* input, void* output,
                                                uint32_t pitch,
                                                uint32_t width, uint32_t height,
                                                aipl_color_format_t format);

/**
 * Convert RGBA8888 image to ALPHA8
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
// aipl_error_t aipl_color_convert_rgba8888_to_alpha8_dave2d(const void* input,
//                                                           void* output,
//                                                           uint32_t pitch,
//                                                           uint32_t width,
//                                                           uint32_t height);

/**
 * Convert RGBA8888 image to ARGB8888
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
aipl_error_t aipl_color_convert_rgba8888_to_argb8888_dave2d(const void* input,
                                                            void* output,
                                                            uint32_t pitch,
                                                            uint32_t width,
                                                            uint32_t height);

/**
 * Convert RGBA8888 image to ARGB4444
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
aipl_error_t aipl_color_convert_rgba8888_to_argb4444_dave2d(const void* input,
                                                            void* output,
                                                            uint32_t pitch,
                                                            uint32_t width,
                                                            uint32_t height);

/**
 * Convert RGBA8888 image to ARGB1555
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
// aipl_error_t aipl_color_convert_rgba8888_to_argb1555_dave2d(const void* input,
//                                                             void* output,
//                                                             uint32_t pitch,
//                                                             uint32_t width,
//                                                             uint32_t height);

/**
 * Convert RGBA8888 image to RGBA4444
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
aipl_error_t aipl_color_convert_rgba8888_to_rgba4444_dave2d(const void* input,
                                                            void* output,
                                                            uint32_t pitch,
                                                            uint32_t width,
                                                            uint32_t height);

/**
 * Convert RGBA8888 image to RGBA5551
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
// aipl_error_t aipl_color_convert_rgba8888_to_rgba5551_dave2d(const void* input,
//                                                             void* output,
//                                                             uint32_t pitch,
//                                                             uint32_t width,
//                                                             uint32_t height);

/**
 * Convert RGBA8888 image to RGB565
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
aipl_error_t aipl_color_convert_rgba8888_to_rgb565_dave2d(const void* input,
                                                          void* output,
                                                          uint32_t pitch,
                                                          uint32_t width,
                                                          uint32_t height);

/**
 * Convert RGBA4444 image to specified format
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @param format        desired color format
 * @return error code
 */
aipl_error_t aipl_color_convert_rgba4444_dave2d(const void* input, void* output,
                                                uint32_t pitch,
                                                uint32_t width, uint32_t height,
                                                aipl_color_format_t format);

/**
 * Convert RGBA4444 image to ALPHA8
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
// aipl_error_t aipl_color_convert_rgba4444_to_alpha8_dave2d(const void* input,
//                                                           void* output,
//                                                           uint32_t pitch,
//                                                           uint32_t width,
//                                                           uint32_t height);

/**
 * Convert RGBA4444 image to ARGB8888
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
aipl_error_t aipl_color_convert_rgba4444_to_argb8888_dave2d(const void* input,
                                                            void* output,
                                                            uint32_t pitch,
                                                            uint32_t width,
                                                            uint32_t height);

/**
 * Convert RGBA4444 image to ARGB4444
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
aipl_error_t aipl_color_convert_rgba4444_to_argb4444_dave2d(const void* input,
                                                            void* output,
                                                            uint32_t pitch,
                                                            uint32_t width,
                                                            uint32_t height);

/**
 * Convert RGBA4444 image to ARGB1555
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
// aipl_error_t aipl_color_convert_rgba4444_to_argb1555_dave2d(const void* input,
//                                                             void* output,
//                                                             uint32_t pitch,
//                                                             uint32_t width,
//                                                             uint32_t height);

/**
 * Convert RGBA4444 image to RGBA8888
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
aipl_error_t aipl_color_convert_rgba4444_to_rgba8888_dave2d(const void* input,
                                                            void* output,
                                                            uint32_t pitch,
                                                            uint32_t width,
                                                            uint32_t height);

/**
 * Convert RGBA4444 image to RGBA5551
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
// aipl_error_t aipl_color_convert_rgba4444_to_rgba5551_dave2d(const void* input,
//                                                             void* output,
//                                                             uint32_t pitch,
//                                                             uint32_t width,
//                                                             uint32_t height);

/**
 * Convert RGBA4444 image to RGB565
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
aipl_error_t aipl_color_convert_rgba4444_to_rgb565_dave2d(const void* input,
                                                          void* output,
                                                          uint32_t pitch,
                                                          uint32_t width,
                                                          uint32_t height);

/**
 * Convert RGBA5551 image to specified format
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @param format        desired color format
 * @return error code
 */
aipl_error_t aipl_color_convert_rgba5551_dave2d(const void* input, void* output,
                                                uint32_t pitch,
                                                uint32_t width, uint32_t height,
                                                aipl_color_format_t format);


/**
 * Convert RGBA5551 image to ALPHA8
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
// aipl_error_t aipl_color_convert_rgba5551_to_alpha8_dave2d(const void* input,
//                                                           void* output,
//                                                           uint32_t pitch,
//                                                           uint32_t width,
//                                                           uint32_t height);
/**
 * Convert RGBA5551 image to ARGB8888
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
aipl_error_t aipl_color_convert_rgba5551_to_argb8888_dave2d(const void* input,
                                                            void* output,
                                                            uint32_t pitch,
                                                            uint32_t width,
                                                            uint32_t height);

/**
 * Convert RGBA5551 image to ARGB4444
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
aipl_error_t aipl_color_convert_rgba5551_to_argb4444_dave2d(const void* input,
                                                            void* output,
                                                            uint32_t pitch,
                                                            uint32_t width,
                                                            uint32_t height);

/**
 * Convert RGBA5551 image to ARGB1555
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
// aipl_error_t aipl_color_convert_rgba5551_to_argb1555_dave2d(const void* input,
//                                                             void* output,
//                                                             uint32_t pitch,
//                                                             uint32_t width,
//                                                             uint32_t height);

/**
 * Convert RGBA5551 image to RGBA8888
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
aipl_error_t aipl_color_convert_rgba5551_to_rgba8888_dave2d(const void* input,
                                                            void* output,
                                                            uint32_t pitch,
                                                            uint32_t width,
                                                            uint32_t height);

/**
 * Convert RGBA5551 image to RGBA4444
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
aipl_error_t aipl_color_convert_rgba5551_to_rgba4444_dave2d(const void* input,
                                                            void* output,
                                                            uint32_t pitch,
                                                            uint32_t width,
                                                            uint32_t height);

/**
 * Convert RGBA5551 image to RGB565
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
aipl_error_t aipl_color_convert_rgba5551_to_rgb565_dave2d(const void* input,
                                                          void* output,
                                                          uint32_t pitch,
                                                          uint32_t width,
                                                          uint32_t height);


/**
 * Convert RGB565 image to specified format
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @param format        desired color format
 * @return error code
 */
aipl_error_t aipl_color_convert_rgb565_dave2d(const void* input, void* output,
                                              uint32_t pitch,
                                              uint32_t width, uint32_t height,
                                              aipl_color_format_t format);

/**
 * Convert RGB565 image to ALPHA8
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
// aipl_error_t aipl_color_convert_rgb565_to_alpha8_dave2d(const void* input,
//                                                         void* output,
//                                                         uint32_t pitch,
//                                                         uint32_t width,
//                                                         uint32_t height);

/**
 * Convert RGB565 image to ARGB8888
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
aipl_error_t aipl_color_convert_rgb565_to_argb8888_dave2d(const void* input,
                                                          void* output,
                                                          uint32_t pitch,
                                                          uint32_t width,
                                                          uint32_t height);

/**
 * Convert RGB565 image to ARGB4444
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
aipl_error_t aipl_color_convert_rgb565_to_argb4444_dave2d(const void* input,
                                                          void* output,
                                                          uint32_t pitch,
                                                          uint32_t width,
                                                          uint32_t height);

/**
 * Convert RGB565 image to ARGB1555
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
// aipl_error_t aipl_color_convert_rgb565_to_argb1555_dave2d(const void* input,
//                                                           void* output,
//                                                           uint32_t pitch,
//                                                           uint32_t width,
//                                                           uint32_t height);

/**
 * Convert RGB565 image to RGBA8888
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
aipl_error_t aipl_color_convert_rgb565_to_rgba8888_dave2d(const void* input,
                                                          void* output,
                                                          uint32_t pitch,
                                                          uint32_t width,
                                                          uint32_t height);

/**
 * Convert RGB565 image to RGBA4444
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
aipl_error_t aipl_color_convert_rgb565_to_rgba4444_dave2d(const void* input,
                                                          void* output,
                                                          uint32_t pitch,
                                                          uint32_t width,
                                                          uint32_t height);

/**
 * Convert RGB565 image to RGBA5551
 * with D/AVE2D
 *
 * @param input         input image pointer
 * @param output        output image pointer
 * @param pitch         input image pitch
 * @param width         image width
 * @param height        image height
 * @return error code
 */
// aipl_error_t aipl_color_convert_rgb565_to_rgba5551_dave2d(const void* input,
//                                                           void* output,
//                                                           uint32_t pitch,
//                                                           uint32_t width,
//                                                           uint32_t height);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* AIPL_DAVE2D_ACCELERATION */

#endif  /* AIPL_COLOR_CONVERSION_DAVE2D_H */

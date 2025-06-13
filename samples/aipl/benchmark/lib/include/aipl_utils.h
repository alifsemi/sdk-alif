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
 * @file    aipl_utils.h
 * @brief   Utility types and function definitions
 *
******************************************************************************/

#ifndef AIPL_UTILS_H
#define AIPL_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <stdint.h>

#include "aipl_error.h"

/*********************
 *      DEFINES
 *********************/
#define INLINE inline __attribute__((always_inline))

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    uint8_t b;
    uint8_t g;
    uint8_t r;
    uint8_t a;
} aipl_argb8888_px_t;

typedef struct {
    union {
        struct {
            uint8_t b : 4;
            uint8_t g : 4;
        };
        uint8_t gb;
    };
    union {
        struct {
            uint8_t r : 4;
            uint8_t a : 4;
        };
        uint8_t ar;
    };
} aipl_argb4444_px_t;

typedef struct {
    union {
        struct {
            uint8_t b;
            uint8_t t;
        };
        uint16_t h;
    };
} aipl_argb1555_px_t;

typedef struct {
    uint8_t a;
    uint8_t b;
    uint8_t g;
    uint8_t r;
} aipl_rgba8888_px_t;

typedef struct {
    union {
        struct {
            uint8_t a : 4;
            uint8_t b : 4;
        };
        uint8_t ba;
    };
    union {
        struct {
            uint8_t g : 4;
            uint8_t r : 4;
        };
        uint8_t rg;
    };
} aipl_rgba4444_px_t;

typedef struct {
    union {
        struct {
            uint8_t b;
            uint8_t t;
        };
        uint16_t h;
    };
} aipl_rgba5551_px_t;

typedef struct {
    union {
        struct {
            uint8_t b;
            uint8_t t;
        };
        uint16_t h;
    };
} aipl_rgb565_px_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
/**
 * Convert ARGB8888 pixel into ARGB4444
 *
 * @param dst desitnation ARGB4444 pixel pointer
 * @param src source ARGB8888 pixel pointer
 */
INLINE void aipl_cnvt_px_argb8888_to_argb4444(aipl_argb4444_px_t* dst,
                                              const aipl_argb8888_px_t* src)
{
    dst->ar = (src->a & 0xf0) | (src->r >> 4);
    dst->gb = (src->g & 0xf0) | (src->b >> 4);
}

/**
 * Convert ARGB8888 pixel into ARGB1555
 *
 * @param dst desitnation ARGB1555 pixel pointer
 * @param src source ARGB8888 pixel pointer
 */
INLINE void aipl_cnvt_px_argb8888_to_argb1555(aipl_argb1555_px_t* dst,
                                              const aipl_argb8888_px_t* src)
{
    dst->t = (src->a & 0x80) | ((src->r >> 1) & 0x7c) | (src->g >> 6);
    dst->b = ((src->g << 2) & 0xe0) | (src->b >> 3);
}

/**
 * Convert ARGB8888 pixel into RGBA8888
 *
 * @param dst desitnation RGBA8888 pixel pointer
 * @param src source ARGB8888 pixel pointer
 */
INLINE void aipl_cnvt_px_argb8888_to_rgba8888(aipl_rgba8888_px_t* dst,
                                              const aipl_argb8888_px_t* src)
{
    dst->r = src->r;
    dst->g = src->g;
    dst->b = src->b;
    dst->a = src->a;
}

/**
 * Convert ARGB8888 pixel into RGBA4444
 *
 * @param dst desitnation RGBA4444 pixel pointer
 * @param src source ARGB8888 pixel pointer
 */
INLINE void aipl_cnvt_px_argb8888_to_rgba4444(aipl_rgba4444_px_t* dst,
                                              const aipl_argb8888_px_t* src)
{
    dst->rg = (src->r & 0xf0) | (src->g >> 4);
    dst->ba = (src->b & 0xf0) | (src->a >> 4);
}

/**
 * Convert ARGB8888 pixel into RGBA5551
 *
 * @param dst desitnation RGBA5551 pixel pointer
 * @param src source ARGB8888 pixel pointer
 */
INLINE void aipl_cnvt_px_argb8888_to_rgba5551(aipl_rgba5551_px_t* dst,
                                              const aipl_argb8888_px_t* src)
{
    dst->t = (src->r & 0xf8) | (src->g >> 5);
    dst->b = ((src->g << 3) & 0xc0) | ((src->b >> 2) & 0x3e) | (src->a >> 7);
}

/**
 * Convert ARGB8888 pixel into RGB565
 *
 * @param dst desitnation RGB565 pixel pointer
 * @param src source ARGB8888 pixel pointer
 */
INLINE void aipl_cnvt_px_argb8888_to_rgb565(aipl_rgb565_px_t* dst,
                                            const aipl_argb8888_px_t* src)
{
    dst->t = (src->r & 0xf8) | (src->g >> 5);
    dst->b = ((src->g << 3) & 0xe0) | (src->b >> 3);
}

/**
 * Convert ARGB8888 pixel into 24bit format
 *
 * @param dst       desitnation 24bit pixel pointer
 * @param src       source ARGB8888 pixel pointer
 * @param r_offset  red channel offset
 * @param g_offset  green channel offset
 * @param b_offset  blue channel offset
 */
INLINE void aipl_cnvt_px_argb8888_to_24bit(uint8_t* dst,
                                           const aipl_argb8888_px_t* src,
                                           uint8_t r_offset,
                                           uint8_t g_offset,
                                           uint8_t b_offset)
{
    dst[r_offset] = src->r;
    dst[g_offset] = src->g;
    dst[b_offset] = src->b;
}

/**
 * Convert ARGB8888 pixel into Y channel
 *
 * @param dst destination Y channel pointer
 * @param src source ARGB8888 pixel pointer
 */
INLINE void aipl_cnvt_px_argb8888_to_yuv_y(uint8_t* dst,
                                           const aipl_argb8888_px_t* src)
{
    *dst = ((66 * src->r + 129 * src->g + 25 * src->b + 128) >> 8) + 16;
}

/**
 * Convert ARGB8888 pixel into V channel
 *
 * @param u_dst destination U channel pointer
 * @param v_dst destination V channel pointer
 * @param src source ARGB8888 pixel pointer
 */
INLINE void aipl_cnvt_px_argb8888_to_yuv_uv(uint8_t* u_dst,
                                            uint8_t* v_dst,
                                            const aipl_argb8888_px_t* src)
{
    uint8_t r = src->r;
    uint8_t g = src->g;
    uint8_t b = src->b;

    *u_dst = ((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128;
    *v_dst = ((112 * r - 94 * g - 18 * b + 128) >> 8) + 128;
}

/**
 * Convert ARGB8888 pixel into Y, U and V channels
 *
 * @param y_dst destination Y channel pointer
 * @param u_dst destination U channel pointer
 * @param v_dst destination V channel pointer
 * @param src   source ARGB8888 pixel pointer
 */
INLINE void aipl_cnvt_px_argb8888_to_yuv(uint8_t* y_dst,
                                         uint8_t* u_dst,
                                         uint8_t* v_dst,
                                         const aipl_argb8888_px_t* src)
{
    uint8_t r = src->r;
    uint8_t g = src->g;
    uint8_t b = src->b;

    *y_dst = ((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;
    *u_dst = ((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128;
    *v_dst = ((112 * r - 94 * g - 18 * b + 128) >> 8) + 128;
}

/**
 * Convert ARGB4444 pixel into ARGB8888
 *
 * @param dst destination ARGB8888 pixel pointer
 * @param src source ARGB4444 pixel pointer
 */
INLINE void aipl_cnvt_px_argb4444_to_argb8888(aipl_argb8888_px_t* dst,
                                              const aipl_argb4444_px_t* src)
{
    dst->a = (src->ar & 0xf0) | (src->ar >> 4);
    dst->r = (src->ar << 4) | (src->ar & 0x0f);
    dst->g = (src->gb & 0xf0) | (src->gb >> 4);
    dst->b = (src->gb << 4) | (src->gb & 0x0f);
}

/**
 * Convert ARGB4444 pixel into ARGB1555
 *
 * @param dst destination ARGB1555 pixel pointer
 * @param src source ARGB4444 pixel pointer
 */
INLINE void aipl_cnvt_px_argb4444_to_argb1555(aipl_argb1555_px_t* dst,
                                              const aipl_argb4444_px_t* src)
{
    dst->t = (src->ar & 0x80) | ((src->ar << 3) & 0x7c)
               | (src->gb >> 6);
    dst->b = ((src->gb << 2) & 0xc0) | ((src->gb << 1) & 0x1f);
}

/**
 * Convert ARGB4444 pixel into RGBA8888
 *
 * @param dst destination RGBA8888 pixel pointer
 * @param src source ARGB4444 pixel pointer
 */
INLINE void aipl_cnvt_px_argb4444_to_rgba8888(aipl_rgba8888_px_t* dst,
                                              const aipl_argb4444_px_t* src)
{
    dst->r = (src->ar << 4) | (src->ar & 0x0f);
    dst->g = (src->gb & 0xf0) | (src->gb >> 4);
    dst->b = (src->gb << 4) | (src->gb & 0x0f);
    dst->a = (src->ar & 0xf0) | (src->ar >> 4);
}

/**
 * Convert ARGB4444 pixel into RGBA4444
 *
 * @param dst destination RGBA4444 pixel pointer
 * @param src source ARGB4444 pixel pointer
 */
INLINE void aipl_cnvt_px_argb4444_to_rgba4444(aipl_rgba4444_px_t* dst,
                                              const aipl_argb4444_px_t* src)
{
    dst->rg = (src->ar << 4) | (src->gb >> 4);
    dst->ba = (src->gb << 4) | (src->ar >> 4);
}

/**
 * Convert ARGB4444 pixel into RGBA5551
 *
 * @param dst destination RGBA5551 pixel pointer
 * @param src source ARGB4444 pixel pointer
 */
INLINE void aipl_cnvt_px_argb4444_to_rgba5551(aipl_rgba5551_px_t* dst,
                                              const aipl_argb4444_px_t* src)
{
    dst->t = (src->ar << 4) | (src->gb >> 5);
    dst->b = ((src->gb << 3) & 0x80) | ((src->gb << 2) & 0x3e)
             | (src->ar >> 7);
}

/**
 * Convert ARGB4444 pixel into 24bit
 *
 * @param dst       destination 24bit pixel pointer
 * @param src       source ARGB4444 pixel pointer
 * @param r_offset  red channel offset
 * @param g_offset  green channel offset
 * @param b_offset  blue channel offset
 */
INLINE void aipl_cnvt_px_argb4444_to_24bit(uint8_t* dst,
                                           const aipl_argb4444_px_t* src,
                                           uint8_t r_offset,
                                           uint8_t g_offset,
                                           uint8_t b_offset)
{
    dst[r_offset] = (src->ar << 4) | (src->ar & 0x0f);
    dst[g_offset] = (src->gb & 0xf0) | (src->gb >> 4);
    dst[b_offset] = (src->gb << 4) | (src->gb & 0x0f);
}

/**
 * Convert ARGB4444 pixel into RGB565
 *
 * @param dst destination RGB565 pixel pointer
 * @param src source ARGB4444 pixel pointer
 */
INLINE void aipl_cnvt_px_argb4444_to_rgb565(aipl_rgb565_px_t* dst,
                                            const aipl_argb4444_px_t* src)
{
    dst->t = (src->ar << 4) | (src->gb >> 5);
    dst->b = ((src->gb << 3) & 0x80) | ((src->gb << 1) & 0x1f);
}

/**
 * Convert ARGB4444 pixel into Y channel
 *
 * @param dst destination Y channel pointer
 * @param src source ARGB4444 pixel pointer
 */
INLINE void aipl_cnvt_px_argb4444_to_yuv_y(uint8_t* dst,
                                           const aipl_argb4444_px_t* src)
{
    uint8_t r = src->ar & 0x0f;
    uint8_t g = src->gb >> 4;
    uint8_t b = src->gb & 0x0f;

    *dst = ((66 * 0x11 * r + 129 * 0x11 * g + 25 * 0x11 * b + 128) >> 8) + 16;
}

/**
 * Convert ARGB4444 pixel into U and V channels
 *
 * @param u_dst destination U channel pointer
 * @param v_dst destination V channel pointer
 * @param src   source ARGB4444 pixel pointer
 */
INLINE void aipl_cnvt_px_argb4444_to_yuv_uv(uint8_t* u_dst,
                                            uint8_t* v_dst,
                                            const aipl_argb4444_px_t* src)
{
    uint8_t r = src->ar & 0x0f;
    uint8_t g = src->gb >> 4;
    uint8_t b = src->gb & 0x0f;

    *u_dst = ((-38 * 0x11 * r - 74 * 0x11 * g + 112 * 0x11 * b + 128) >> 8) + 128;
    *v_dst = ((112 * 0x11 * r - 94 * 0x11 * g - 18 * 0x11 * b + 128) >> 8) + 128;
}

/**
 * Convert ARGB4444 pixel into Y, U and V channels
 *
 * @param y_dst destination Y channel pointer
 * @param u_dst destination U channel pointer
 * @param v_dst destination V channel pointer
 * @param src   source ARGB4444 pixel pointer
 */
INLINE void aipl_cnvt_px_argb4444_to_yuv(uint8_t* y_dst,
                                         uint8_t* u_dst,
                                         uint8_t* v_dst,
                                         const aipl_argb4444_px_t* src)
{
    uint8_t r = src->ar & 0x0f;
    uint8_t g = src->gb >> 4;
    uint8_t b = src->gb & 0x0f;

    *y_dst = ((66 * 0x11 * r + 129 * 0x11 * g + 25 * 0x11 * b + 128) >> 8) + 16;
    *u_dst = ((-38 * 0x11 * r - 74 * 0x11 * g + 112 * 0x11 * b + 128) >> 8) + 128;
    *v_dst = ((112 * 0x11 * r - 94 * 0x11 * g - 18 * 0x11 * b + 128) >> 8) + 128;
}

/**
 * Convert ARGB1555 pixel into ARGB8888
 *
 * @param dst destination ARGB8888 pixel pointer
 * @param src source ARGB1555 pixel pointer
 */
INLINE void aipl_cnvt_px_argb1555_to_argb8888(aipl_argb8888_px_t* dst,
                                              const aipl_argb1555_px_t* src)
{
    dst->a = (src->t >> 7) * 0xff;
    dst->r = ((src->t << 1) & 0xf8) | ((src->t >> 4) & 0x07);
    dst->g = (src->t << 6) | ((src->b >> 2) & 0x38) | (src->t & 0x03);
    dst->b = ((src->b & 0x1f) << 3) | ((src->b >> 2) & 0x07);
}

/**
 * Convert ARGB1555 pixel into ARGB4444
 *
 * @param dst destination ARGB4444 pixel pointer
 * @param src source ARGB1555 pixel pointer
 */
INLINE void aipl_cnvt_px_argb1555_to_argb4444(aipl_argb4444_px_t* dst,
                                              const aipl_argb1555_px_t* src)
{
    dst->ar = ((src->t >> 7) * 0xf0) | ((src->t >> 3) & 0x0f);
    dst->gb = (src->t << 6) | ((src->b >> 2) & 0x30)
              | ((src->b >> 1) & 0x0f);
}

/**
 * Convert ARGB1555 pixel into RGBA8888
 *
 * @param dst destination RGBA8888 pixel pointer
 * @param src source ARGB1555 pixel pointer
 */
INLINE void aipl_cnvt_px_argb1555_to_rgba8888(aipl_rgba8888_px_t* dst,
                                              const aipl_argb1555_px_t* src)
{
    dst->r = ((src->t << 1) & 0xf8) | ((src->t >> 4) & 0x07);
    dst->g = (src->t << 6) | ((src->b >> 2) & 0x38) | (src->t & 0x03);
    dst->b = ((src->b & 0x1f) << 3) | ((src->b >> 2) & 0x07);
    dst->a = (src->t >> 7) * 0xff;
}

/**
 * Convert ARGB1555 pixel into RGBA4444
 *
 * @param dst destination RGBA4444 pixel pointer
 * @param src source ARGB1555 pixel pointer
 */
INLINE void aipl_cnvt_px_argb1555_to_rgba4444(aipl_rgba4444_px_t* dst,
                                              const aipl_argb1555_px_t* src)
{
    dst->rg = ((src->t << 1) & 0xf0) | ((src->t << 2) & 0x0f)
              | (src->b >> 6);
    dst->ba = ((src->b << 3) & 0xf0) | (src->t >> 7) * 0x0f;
}

/**
 * Convert ARGB1555 pixel into RGBA5551
 *
 * @param dst destination RGBA5551 pixel pointer
 * @param src source ARGB1555 pixel pointer
 */
INLINE void aipl_cnvt_px_argb1555_to_rgba5551(aipl_rgba5551_px_t* dst,
                                              const aipl_argb1555_px_t* src)
{
    dst->h = (src->h << 1) | (src->h >> 15);
}

/**
 * Convert ARGB1555 pixel into 24bit
 *
 * @param dst       destination 24bit pixel pointer
 * @param src       source ARGB1555 pixel pointer
 * @param r_offset  red channel offset
 * @param g_offset  green channel offset
 * @param b_offset  blue channel offset
 */
INLINE void aipl_cnvt_px_argb1555_to_24bit(uint8_t* dst,
                                           const aipl_argb1555_px_t* src,
                                           uint8_t r_offset,
                                           uint8_t g_offset,
                                           uint8_t b_offset)
{
    dst[r_offset] = ((src->t << 1) & 0xf8) | ((src->t >> 4) & 0x07);
    dst[g_offset] = (src->t << 6) | ((src->b >> 2) & 0x38) | (src->t & 0x03);
    dst[b_offset] = ((src->b & 0x1f) << 3) | ((src->b >> 2) & 0x07);
}

/**
 * Convert ARGB1555 pixel into RGB565
 *
 * @param dst destination RGB565 pixel pointer
 * @param src source ARGB1555 pixel pointer
 */
INLINE void aipl_cnvt_px_argb1555_to_rgb565(aipl_rgb565_px_t* dst,
                                            const aipl_argb1555_px_t* src)
{
    dst->t = (src->t << 1) | (src->b >> 7);
    dst->b = ((src->b << 1) & 0xe0) | (src->b & 0x1f);

}

/**
 * Convert ARGB1555 pixel into Y channel
 *
 * @param dst destination Y channel pointer
 * @param src source ARGB1555 pixel pointer
 */
INLINE void aipl_cnvt_px_argb1555_to_yuv_y(uint8_t* dst,
                                           const aipl_argb1555_px_t* src)
{
    uint8_t r = (src->t >> 2) & 0x1f;
    uint8_t g = ((src->t << 3) & 0x1f) | (src->b >> 5);
    uint8_t b = src->b & 0x1f;

    *dst = ((543 * r + 1061 * g + 205 * b + 128) >> 8) + 16;
}

/**
 * Convert ARGB1555 pixel into U and V channels
 *
 * @param u_dst destination U channel pointer
 * @param v_dst destination V channel pointer
 * @param src   source ARGB1555 pixel pointer
 */
INLINE void aipl_cnvt_px_argb1555_to_yuv_uv(uint8_t* u_dst,
                                            uint8_t* v_dst,
                                            const aipl_argb1555_px_t* src)
{
    uint8_t r = (src->t >> 2) & 0x1f;
    uint8_t g = ((src->t << 3) & 0x1f) | (src->b >> 5);
    uint8_t b = src->b & 0x1f;

    *u_dst = ((-312 * r - 608 * g + 920 * b + 128) >> 8) + 128;
    *v_dst = ((920 * r - 773 * g - 147 * b + 128) >> 8) + 128;
}

/**
 * Convert ARGB1555 pixel into Y, U and V channels
 *
 * @param y_dst destination Y channel pointer
 * @param u_dst destination U channel pointer
 * @param v_dst destination V channel pointer
 * @param src   source ARGB1555 pixel pointer
 */
INLINE void aipl_cnvt_px_argb1555_to_yuv(uint8_t* y_dst,
                                         uint8_t* u_dst,
                                         uint8_t* v_dst,
                                         const aipl_argb1555_px_t* src)
{
    uint8_t r = (src->t >> 2) & 0x1f;
    uint8_t g = ((src->t << 3) & 0x1f) | (src->b >> 5);
    uint8_t b = src->b & 0x1f;

    *y_dst = ((543 * r + 1061 * g + 205 * b + 128) >> 8) + 16;
    *u_dst = ((-312 * r - 608 * g + 920 * b + 128) >> 8) + 128;
    *v_dst = ((920 * r - 773 * g - 147 * b + 128) >> 8) + 128;
}

/**
 * Convert RGBA8888 pixel into ARGB8888
 *
 * @param dst destination ARGB8888 pixel pointer
 * @param src source RGBA8888 pixel pointer
 */
INLINE void aipl_cnvt_px_rgba8888_to_argb8888(aipl_argb8888_px_t* dst,
                                              const aipl_rgba8888_px_t* src)
{
    dst->a = src->a;
    dst->r = src->r;
    dst->g = src->g;
    dst->b = src->b;
}

/**
 * Convert RGBA8888 pixel into ARGB4444
 *
 * @param dst destination ARGB4444 pixel pointer
 * @param src source RGBA8888 pixel pointer
 */
INLINE void aipl_cnvt_px_rgba8888_to_argb4444(aipl_argb4444_px_t* dst,
                                              const aipl_rgba8888_px_t* src)
{
    dst->ar = (src->a & 0xf0) | (src->r >> 4);
    dst->gb = (src->g & 0xf0) | (src->b >> 4);
}

/**
 * Convert RGBA8888 pixel into ARGB1555
 *
 * @param dst destination ARGB1555 pixel pointer
 * @param src source RGBA8888 pixel pointer
 */
INLINE void aipl_cnvt_px_rgba8888_to_argb1555(aipl_argb1555_px_t* dst,
                                              const aipl_rgba8888_px_t* src)
{
    dst->t = (src->a & 0x80) | ((src->r >> 1) & 0x7c) | (src->g >> 6);
    dst->b = ((src->g << 2) & 0xe0) | (src->b >> 3);

}

/**
 * Convert RGBA8888 pixel into RGBA4444
 *
 * @param dst destination RGBA4444 pixel pointer
 * @param src source RGBA8888 pixel pointer
 */
INLINE void aipl_cnvt_px_rgba8888_to_rgba4444(aipl_rgba4444_px_t* dst,
                                              const aipl_rgba8888_px_t* src)
{
    dst->rg = (src->r & 0xf0) | (src->g >> 4);
    dst->ba = (src->b & 0xf0) | (src->a >> 4);
}

/**
 * Convert RGBA8888 pixel into RGBA5551
 *
 * @param dst destination RGBA5551 pixel pointer
 * @param src source RGBA8888 pixel pointer
 */
INLINE void aipl_cnvt_px_rgba8888_to_rgba5551(aipl_rgba5551_px_t* dst,
                                              const aipl_rgba8888_px_t* src)
{
    dst->t = (src->r & 0xf8) | (src->g >> 5);
    dst->b = ((src->g << 3) & 0xc0) | ((src->b >> 2) & 0x3e) | (src->a >> 7);
}

/**
 * Convert RGBA8888 pixel into 24bit
 *
 * @param dst       destination 24bit pixel pointer
 * @param src       source RGBA8888 pixel pointer
 * @param r_offset  red channel offset
 * @param g_offset  green channel offset
 * @param b_offset  blue channel offset
 */
INLINE void aipl_cnvt_px_rgba8888_to_24bit(uint8_t* dst,
                                           const aipl_rgba8888_px_t* src,
                                           uint8_t r_offset,
                                           uint8_t g_offset,
                                           uint8_t b_offset)
{
    dst[r_offset] = src->r;
    dst[g_offset] = src->g;
    dst[b_offset] = src->b;
}

/**
 * Convert RGBA8888 pixel into RGB565
 *
 * @param dst destination RGB565 pixel pointer
 * @param src source RGBA8888 pixel pointer
 */
INLINE void aipl_cnvt_px_rgba8888_to_rgb565(aipl_rgb565_px_t* dst,
                                            const aipl_rgba8888_px_t* src)
{
    dst->t = (src->r & 0xf8) | (src->g >> 5);
    dst->b = ((src->g << 3) & 0xe0) | (src->b >> 3);
}

/**
 * Convert RGBA8888 pixel into Y channel
 *
 * @param dst destination Y channel pointer
 * @param src source RGBA8888 pixel pointer
 */
INLINE void aipl_cnvt_px_rgba8888_to_yuv_y(uint8_t* dst,
                                           const aipl_rgba8888_px_t* src)
{
    *dst = ((66 * src->r + 129 * src->g + 25 * src->b + 128) >> 8) + 16;
}

/**
 * Convert RGBA8888 pixel into U and V channels
 *
 * @param u_dst destination U channel pointer
 * @param v_dst destination V channel pointer
 * @param src   source RGBA8888 pixel pointer
 */
INLINE void aipl_cnvt_px_rgba8888_to_yuv_uv(uint8_t* u_dst,
                                            uint8_t* v_dst,
                                            const aipl_rgba8888_px_t* src)
{
    uint8_t r = src->r;
    uint8_t g = src->g;
    uint8_t b = src->b;

    *u_dst = ((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128;
    *v_dst = ((112 * r - 94 * g - 18 * b + 128) >> 8) + 128;

}

/**
 * Convert RGBA8888 pixel into Y, U and V channels
 *
 * @param y_dst destination Y channel pointer
 * @param u_dst destination U channel pointer
 * @param v_dst destination V channel pointer
 * @param src   source RGBA8888 pixel pointer
 */
INLINE void aipl_cnvt_px_rgba8888_to_yuv(uint8_t* y_dst,
                                         uint8_t* u_dst,
                                         uint8_t* v_dst,
                                         const aipl_rgba8888_px_t* src)
{
    uint8_t r = src->r;
    uint8_t g = src->g;
    uint8_t b = src->b;

    *y_dst = ((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;
    *u_dst = ((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128;
    *v_dst = ((112 * r - 94 * g - 18 * b + 128) >> 8) + 128;

}

/**
 * Convert RGBA4444 pixel into ARGB8888
 *
 * @param dst destination ARGB8888 pixel pointer
 * @param src source RGBA4444 pixel pointer
 */
INLINE void aipl_cnvt_px_rgba4444_to_argb8888(aipl_argb8888_px_t* dst,
                                              const aipl_rgba4444_px_t* src)
{
    dst->r = (src->rg & 0xf0) | (src->rg >> 4);
    dst->g = (src->rg << 4) | (src->rg & 0x0f);
    dst->b = (src->ba & 0xf0) | (src->ba >> 4);
    dst->a = (src->ba << 4) | (src->ba & 0x0f);
}

/**
 * Convert RGBA4444 pixel into ARGB4444
 *
 * @param dst destination ARGB4444 pixel pointer
 * @param src source RGBA4444 pixel pointer
 */
INLINE void aipl_cnvt_px_rgba4444_to_argb4444(aipl_argb4444_px_t* dst,
                                              const aipl_rgba4444_px_t* src)
{
    dst->ar = (src->rg >> 4) | (src->ba << 4);
    dst->gb = (src->rg << 4) | (src->ba >> 4);
}

/**
 * Convert RGBA4444 pixel into ARGB1555
 *
 * @param dst destination ARGB1555 pixel pointer
 * @param src source RGBA4444 pixel pointer
 */
INLINE void aipl_cnvt_px_rgba4444_to_argb1555(aipl_argb1555_px_t* dst,
                                              const aipl_rgba4444_px_t* src)
{
    dst->t = ((src->ba << 4) & 0x80) | ((src->rg >> 1) & 0x78)
                | ((src->rg >> 2) & 0x03);
    dst->b = (src->rg << 6) | ((src->ba >> 3) & 0x1e);
}

/**
 * Convert RGBA4444 pixel into RGBA8888
 *
 * @param dst destination RGBA8888 pixel pointer
 * @param src source RGBA4444 pixel pointer
 */
INLINE void aipl_cnvt_px_rgba4444_to_rgba8888(aipl_rgba8888_px_t* dst,
                                              const aipl_rgba4444_px_t* src)
{
    dst->r = (src->rg & 0xf0) | (src->rg >> 4);
    dst->g = (src->rg << 4) | (src->rg & 0x0f);
    dst->b = (src->ba & 0xf0) | (src->ba >> 4);
    dst->a = (src->ba << 4) | (src->ba & 0x0f);
}

/**
 * Convert RGBA4444 pixel into RGBA5551
 *
 * @param dst destination RGBA5551 pixel pointer
 * @param src source RGBA4444 pixel pointer
 */
INLINE void aipl_cnvt_px_rgba4444_to_rgba5551(aipl_rgba5551_px_t* dst,
                                              const aipl_rgba4444_px_t* src)
{
    dst->t = (src->rg & 0xf0) | ((src->rg >> 1) & 0x07);
    dst->b = (src->rg << 7) | ((src->ba >> 2) & 0x3c)
                | ((src->ba >> 3) & 0x01);
}

/**
 * Convert RGBA4444 pixel into 24bit
 *
 * @param dst       destination 24bit pixel pointer
 * @param src       source RGBA4444 pixel pointer
 * @param r_offset  red channel offset
 * @param g_offset  green channel offset
 * @param b_offset  blue channel offset
 */
INLINE void aipl_cnvt_px_rgba4444_to_24bit(uint8_t* dst,
                                           const aipl_rgba4444_px_t* src,
                                           uint8_t r_offset,
                                           uint8_t g_offset,
                                           uint8_t b_offset)
{
    dst[r_offset] = (src->rg & 0xf0) | (src->rg >> 4);
    dst[g_offset] = (src->rg << 4) | (src->rg & 0x0f);
    dst[b_offset] = (src->ba & 0xf0) | (src->ba >> 4);
}

/**
 * Convert RGBA4444 pixel into RGB565
 *
 * @param dst destination RGB565 pixel pointer
 * @param src source RGBA4444 pixel pointer
 */
INLINE void aipl_cnvt_px_rgba4444_to_rgb565(aipl_rgb565_px_t* dst,
                                            const aipl_rgba4444_px_t* src)
{
    dst->t = (src->rg & 0xf0) | ((src->rg >> 1) & 0x07);
    dst->b = (src->rg << 7) | ((src->ba >> 3) & 0x1e);
}

/**
 * Convert RGBA4444 pixel into Y channel
 *
 * @param dst destination Y channel pointer
 * @param src source RGBA4444 pixel pointer
 */
INLINE void aipl_cnvt_px_rgba4444_to_yuv_y(uint8_t* dst,
                                           const aipl_rgba4444_px_t* src)
{
    uint8_t r = src->rg >> 4;
    uint8_t g = src->rg & 0x0f;
    uint8_t b = src->ba >> 4;

    *dst = ((66 * 0x11 * r + 129 * 0x11 * g + 25 * 0x11 * b + 128) >> 8) + 16;
}

/**
 * Convert RGBA4444 pixel into U and V channels
 *
 * @param u_dst destination U channel pointer
 * @param v_dst destination V channel pointer
 * @param src   source RGBA4444 pixel pointer
 */
INLINE void aipl_cnvt_px_rgba4444_to_yuv_uv(uint8_t* u_dst,
                                            uint8_t* v_dst,
                                            const aipl_rgba4444_px_t* src)
{
    uint8_t r = src->rg >> 4;
    uint8_t g = src->rg & 0x0f;
    uint8_t b = src->ba >> 4;

    *u_dst = ((-38 * 0x11 * r - 74 * 0x11 * g + 112 * 0x11 * b + 128) >> 8) + 128;
    *v_dst = ((112 * 0x11 * r - 94 * 0x11 * g - 18 * 0x11 * b + 128) >> 8) + 128;
}

/**
 * Convert RGBA4444 pixel into Y, U and V channels
 *
 * @param y_dst destination Y channel pointer
 * @param u_dst destination U channel pointer
 * @param v_dst destination V channel pointer
 * @param src   source RGBA4444 pixel pointer
 */
INLINE void aipl_cnvt_px_rgba4444_to_yuv(uint8_t* y_dst,
                                         uint8_t* u_dst,
                                         uint8_t* v_dst,
                                         const aipl_rgba4444_px_t* src)
{
    uint8_t r = src->rg >> 4;
    uint8_t g = src->rg & 0x0f;
    uint8_t b = src->ba >> 4;

    *y_dst = ((66 * 0x11 * r + 129 * 0x11 * g + 25 * 0x11 * b + 128) >> 8) + 16;
    *u_dst = ((-38 * 0x11 * r - 74 * 0x11 * g + 112 * 0x11 * b + 128) >> 8) + 128;
    *v_dst = ((112 * 0x11 * r - 94 * 0x11 * g - 18 * 0x11 * b + 128) >> 8) + 128;
}

/**
 * Convert RGBA5551 pixel into ARGB8888
 *
 * @param dst destination ARGB8888 pixel pointer
 * @param src source RGBA5551 pixel pointer
 */
INLINE void aipl_cnvt_px_rgba5551_to_argb8888(aipl_argb8888_px_t* dst,
                                              const aipl_rgba5551_px_t* src)
{
    dst->r = (src->t & 0xf8) | ((src->t >> 5) & 0x07);
    dst->g = (src->t & 0x07) | (src->t << 5) | ((src->b >> 3) & 0x18);
    dst->b = ((src->b << 2) & 0xf8) | ((src->b >> 4) & 0x07);
    dst->a = (src->b & 0x01) * 0xff;
}

/**
 * Convert RGBA5551 pixel into ARGB4444
 *
 * @param dst destination ARGB4444 pixel pointer
 * @param src source RGBA5551 pixel pointer
 */
INLINE void aipl_cnvt_px_rgba5551_to_argb4444(aipl_argb4444_px_t* dst,
                                              const aipl_rgba5551_px_t* src)
{
    dst->ar = ((src->b & 0x01) * 0xf0) | (src->t >> 4);
    dst->gb = (src->t << 5) | ((src->b >> 3) & 0x10) | ((src->b >> 2) & 0x0f);
}

/**
 * Convert RGBA5551 pixel into ARGB1555
 *
 * @param dst destination ARGB1555 pixel pointer
 * @param src source RGBA5551 pixel pointer
 */
INLINE void aipl_cnvt_px_rgba5551_to_argb1555(aipl_argb1555_px_t* dst,
                                              const aipl_rgba5551_px_t* src)
{
    dst->h = (src->h >> 1) | (src->h << 15);
}

/**
 * Convert RGBA5551 pixel into RGBA8888
 *
 * @param dst destination RGBA8888 pixel pointer
 * @param src source RGBA5551 pixel pointer
 */
INLINE void aipl_cnvt_px_rgba5551_to_rgba8888(aipl_rgba8888_px_t* dst,
                                              const aipl_rgba5551_px_t* src)
{
    dst->r = (src->t & 0xf8) | ((src->t >> 5) & 0x07);
    dst->g = (src->t & 0x07) | (src->t << 5) | ((src->b >> 3) & 0x18);
    dst->b = ((src->b << 2) & 0xf8) | ((src->b >> 4) & 0x07);
    dst->a = (src->b & 0x01) * 0xff;
}

/**
 * Convert RGBA5551 pixel into RGBA4444
 *
 * @param dst destination RGBA4444 pixel pointer
 * @param src source RGBA5551 pixel pointer
 */
INLINE void aipl_cnvt_px_rgba5551_to_rgba4444(aipl_rgba4444_px_t* dst,
                                              const aipl_rgba5551_px_t* src)
{
    dst->rg = (src->t & 0xf0) | ((src->t << 1) & 0x0f) | (src->b >> 7);
    dst->ba = ((src->b << 2) & 0xf0) | ((src->b & 0x01) * 0x0f);
}

/**
 * Convert RGBA5551 pixel into 24bit
 *
 * @param dst       destination 24bit pixel pointer
 * @param src       source RGBA5551 pixel pointer
 * @param r_offset  red channel offset
 * @param g_offset  green channel offset
 * @param b_offset  blue channel offset
 */
INLINE void aipl_cnvt_px_rgba5551_to_24bit(uint8_t* dst,
                                           const aipl_rgba5551_px_t* src,
                                           uint8_t r_offset,
                                           uint8_t g_offset,
                                           uint8_t b_offset)
{
    dst[r_offset] = (src->t & 0xf8) | ((src->t >> 5) & 0x07);
    dst[g_offset] = (src->t & 0x07) | (src->t << 5) | ((src->b >> 3) & 0x18);
    dst[b_offset] = ((src->b << 2) & 0xf8) | ((src->b >> 4) & 0x07);
}

/**
 * Convert RGBA5551 pixel into RGB565
 *
 * @param dst destination RGB565 pixel pointer
 * @param src source RGBA5551 pixel pointer
 */
INLINE void aipl_cnvt_px_rgba5551_to_rgb565(aipl_rgb565_px_t* dst,
                                            const aipl_rgba5551_px_t* src)
{
    dst->t = src->t;
    dst->b = (src->b & 0xc0) | ((src->b >> 1) & 0x1f);
}

/**
 * Convert RGBA5551 pixel into Y channel
 *
 * @param dst destination Y channel pointer
 * @param src source RGBA5551 pixel pointer
 */
INLINE void aipl_cnvt_px_rgba5551_to_yuv_y(uint8_t* dst,
                                           const aipl_rgba5551_px_t* src)
{
    uint8_t r = (src->t >> 3) & 0x1f;
    uint8_t g = ((src->t << 2) & 0x1f) | (src->b >> 6);
    uint8_t b = (src->b >> 1) & 0x1f;

    *dst = ((543 * r + 1061 * g + 205 * b + 128) >> 8) + 16;
}

/**
 * Convert RGBA5551 pixel into U and V channels
 *
 * @param u_dst destination U channel pointer
 * @param v_dst destination V channel pointer
 * @param src   source RGBA5551 pixel pointer
 */
INLINE void aipl_cnvt_px_rgba5551_to_yuv_uv(uint8_t* u_dst,
                                            uint8_t* v_dst,
                                            const aipl_rgba5551_px_t* src)
{
    uint8_t r = (src->t >> 3) & 0x1f;
    uint8_t g = ((src->t << 2) & 0x1f) | (src->b >> 6);
    uint8_t b = (src->b >> 1) & 0x1f;

    *u_dst = ((-312 * r - 608 * g + 920 * b + 128) >> 8) + 128;
    *v_dst = ((920 * r - 773 * g - 147 * b + 128) >> 8) + 128;
}

/**
 * Convert RGBA5551 pixel into Y, U and V channels
 *
 * @param y_dst destination Y channel pointer
 * @param u_dst destination U channel pointer
 * @param v_dst destination V channel pointer
 * @param src   source RGBA5551 pixel pointer
 */
INLINE void aipl_cnvt_px_rgba5551_to_yuv(uint8_t* y_dst,
                                         uint8_t* u_dst,
                                         uint8_t* v_dst,
                                         const aipl_rgba5551_px_t* src)
{
    uint8_t r = (src->t >> 3) & 0x1f;
    uint8_t g = ((src->t << 2) & 0x1f) | (src->b >> 6);
    uint8_t b = (src->b >> 1) & 0x1f;

    *y_dst = ((543 * r + 1061 * g + 205 * b + 128) >> 8) + 16;
    *u_dst = ((-312 * r - 608 * g + 920 * b + 128) >> 8) + 128;
    *v_dst = ((920 * r - 773 * g - 147 * b + 128) >> 8) + 128;
}

/**
 * Convert 24bit pixel into ARGB8888 pixel
 *
 * @param dst       destination ARGB8888 pixel pointer
 * @param src       source 24bit pixel pointer
 * @param r_offset  red channel offset
 * @param g_offset  green channel offset
 * @param b_offset  blue channel offset
 */
INLINE void aipl_cnvt_px_24bit_to_argb8888(aipl_argb8888_px_t* dst,
                                           const uint8_t* src,
                                           uint8_t r_offset,
                                           uint8_t g_offset,
                                           uint8_t b_offset)
{
    dst->a = 0xff;
    dst->r = src[r_offset];
    dst->g = src[g_offset];
    dst->b = src[b_offset];
}

/**
 * Convert 24bit pixel into ARGB4444 pixel
 *
 * @param dst       destination ARGB4444 pixel pointer
 * @param src       source 24bit pixel pointer
 * @param r_offset  red channel offset
 * @param g_offset  green channel offset
 * @param b_offset  blue channel offset
 */
INLINE void aipl_cnvt_px_24bit_to_argb4444(aipl_argb4444_px_t* dst,
                                           const uint8_t* src,
                                           uint8_t r_offset,
                                           uint8_t g_offset,
                                           uint8_t b_offset)
{
    dst->ar = 0xf0 | (src[r_offset] >> 4);
    dst->gb = (src[g_offset] & 0xf0) | (src[b_offset] >> 4);
}

/**
 * Convert 24bit pixel into ARGB1555 pixel
 *
 * @param dst       destination ARGB1555 pixel pointer
 * @param src       source 24bit pixel pointer
 * @param r_offset  red channel offset
 * @param g_offset  green channel offset
 * @param b_offset  blue channel offset
 */
INLINE void aipl_cnvt_px_24bit_to_argb1555(aipl_argb1555_px_t* dst,
                                           const uint8_t* src,
                                           uint8_t r_offset,
                                           uint8_t g_offset,
                                           uint8_t b_offset)
{
    dst->t = 0x80 | ((src[r_offset] >> 1) & 0x7c) | (src[g_offset] >> 6);
    dst->b = ((src[g_offset] << 2) & 0xe0) | (src[b_offset] >> 3);
}

/**
 * Convert 24bit pixel into RGBA8888 pixel
 *
 * @param dst       destination RGBA8888 pixel pointer
 * @param src       source 24bit pixel pointer
 * @param r_offset  red channel offset
 * @param g_offset  green channel offset
 * @param b_offset  blue channel offset
 */
INLINE void aipl_cnvt_px_24bit_to_rgba8888(aipl_rgba8888_px_t* dst,
                                           const uint8_t* src,
                                           uint8_t r_offset,
                                           uint8_t g_offset,
                                           uint8_t b_offset)
{
    dst->r = src[r_offset];
    dst->g = src[g_offset];
    dst->b = src[b_offset];
    dst->a = 0xff;
}

/**
 * Convert 24bit pixel into RGBA4444 pixel
 *
 * @param dst       destination RGBA4444 pixel pointer
 * @param src       source 24bit pixel pointer
 * @param r_offset  red channel offset
 * @param g_offset  green channel offset
 * @param b_offset  blue channel offset
 */
INLINE void aipl_cnvt_px_24bit_to_rgba4444(aipl_rgba4444_px_t* dst,
                                           const uint8_t* src,
                                           uint8_t r_offset,
                                           uint8_t g_offset,
                                           uint8_t b_offset)
{
    dst->rg = (src[r_offset] & 0xf0) | (src[g_offset] >> 4);
    dst->ba = (src[b_offset] & 0xf0) | 0x0f;
}

/**
 * Convert 24bit pixel into RGBA5551 pixel
 *
 * @param dst       destination RGBA5551 pixel pointer
 * @param src       source 24bit pixel pointer
 * @param r_offset  red channel offset
 * @param g_offset  green channel offset
 * @param b_offset  blue channel offset
 */
INLINE void aipl_cnvt_px_24bit_to_rgba5551(aipl_rgba5551_px_t* dst,
                                           const uint8_t* src,
                                           uint8_t r_offset,
                                           uint8_t g_offset,
                                           uint8_t b_offset)
{
    dst->t = (src[r_offset] & 0xf8) | (src[g_offset] >> 5);
    dst->b = ((src[g_offset] << 3) & 0xc0) | ((src[b_offset] >> 2) & 0x3e) | 0x01;
}

/**
 * Convert 24bit pixel into 24bit pixel
 *
 * @param dst           destination 24bit pixel pointer
 * @param src           source 24bit pixel pointer
 * @param r_src_offset  source red channel offset
 * @param g_src_offset  source green channel offset
 * @param b_src_offset  source blue channel offset
 * @param r_src_offset  destination red channel offset
 * @param g_src_offset  destination green channel offset
 * @param b_src_offset  destination blue channel offset
 */
INLINE void aipl_cnvt_px_24bit_to_24bit(uint8_t* dst,
                                        const uint8_t* src,
                                        uint8_t r_src_offset,
                                        uint8_t g_src_offset,
                                        uint8_t b_src_offset,
                                        uint8_t r_dst_offset,
                                        uint8_t g_dst_offset,
                                        uint8_t b_dst_offset)
{
    dst[r_dst_offset] = src[r_src_offset];
    dst[g_dst_offset] = src[g_src_offset];
    dst[b_dst_offset] = src[b_src_offset];
}

/**
 * Convert 24bit pixel into RGB565 pixel
 *
 * @param dst       destination RGB565 pixel pointer
 * @param src       source 24bit pixel pointer
 * @param r_offset  red channel offset
 * @param g_offset  green channel offset
 * @param b_offset  blue channel offset
 */
INLINE void aipl_cnvt_px_24bit_to_rgb565(aipl_rgb565_px_t* dst,
                                         const uint8_t* src,
                                         uint8_t r_offset,
                                         uint8_t g_offset,
                                         uint8_t b_offset)
{
    dst->t = (src[r_offset] & 0xf8) | (src[g_offset] >> 5);
    dst->b = ((src[g_offset] << 3) & 0xe0) | (src[b_offset] >> 3);
}

/**
 * Convert 24bit pixel into Y channel
 *
 * @param dst       destination Y channel pointer
 * @param src       source 24bit pixel pointer
 * @param r_offset  red channel offset
 * @param g_offset  green channel offset
 * @param b_offset  blue channel offset
 */
INLINE void aipl_cnvt_px_24bit_to_yuv_y(uint8_t* dst,
                                        const uint8_t* src,
                                        uint8_t r_offset,
                                        uint8_t g_offset,
                                        uint8_t b_offset)
{
    *dst = ((66 * src[r_offset] + 129 * src[g_offset] + 25 * src[b_offset] + 128) >> 8) + 16;
}

/**
 * Convert 24bit pixel into U and V channels
 *
 * @param u_dst     destination U channel pointer
 * @param v_dst     destination V channel pointer
 * @param src       source 24bit pixel pointer
 * @param r_offset  red channel offset
 * @param g_offset  green channel offset
 * @param b_offset  blue channel offset
 */
INLINE void aipl_cnvt_px_24bit_to_yuv_uv(uint8_t* u_dst,
                                         uint8_t* v_dst,
                                         const uint8_t* src,
                                         uint8_t r_offset,
                                         uint8_t g_offset,
                                         uint8_t b_offset)
{
    *u_dst = ((-38 * src[r_offset] - 74 * src[g_offset] + 112 * src[b_offset] + 128) >> 8) + 128;
    *v_dst = ((112 * src[r_offset] - 94 * src[g_offset] - 18 * src[b_offset] + 128) >> 8) + 128;
}

/**
 * Convert 24bit pixel into Y, U and V channels
 *
 * @param y_dst     destination Y channel pointer
 * @param u_dst     destination U channel pointer
 * @param v_dst     destination V channel pointer
 * @param src       source 24bit pixel pointer
 * @param r_offset  red channel offset
 * @param g_offset  green channel offset
 * @param b_offset  blue channel offset
 */
INLINE void aipl_cnvt_px_24bit_to_yuv(uint8_t* y_dst,
                                      uint8_t* u_dst,
                                      uint8_t* v_dst,
                                      const uint8_t* src,
                                      uint8_t r_offset,
                                      uint8_t g_offset,
                                      uint8_t b_offset)
{
    *y_dst = ((66 * src[r_offset] + 129 * src[g_offset] + 25 * src[b_offset] + 128) >> 8) + 16;
    *u_dst = ((-38 * src[r_offset] - 74 * src[g_offset] + 112 * src[b_offset] + 128) >> 8) + 128;
    *v_dst = ((112 * src[r_offset] - 94 * src[g_offset] - 18 * src[b_offset] + 128) >> 8) + 128;
}

/**
 * Convert RGB565 pixel into ARGB8888
 *
 * @param dst destination ARGB8888 pixel pointer
 * @param src source RGB565 pixel pointer
 */
INLINE void aipl_cnvt_px_rgb565_to_argb8888(aipl_argb8888_px_t* dst,
                                            const aipl_rgb565_px_t* src)
{
    dst->a = 0xff;
    dst->r = (src->t & 0xf8) | ((src->t >> 5) & 0x07);
    dst->g = (src->t & 0x03) | (src->t << 5) | ((src->b >> 3) & 0x1c);
    dst->b = ((src->b & 0x1f) << 3) | ((src->b >> 2) & 0x07);
}

/**
 * Convert RGB565 pixel into ARGB4444
 *
 * @param dst destination ARGB4444 pixel pointer
 * @param src source RGB565 pixel pointer
 */
INLINE void aipl_cnvt_px_rgb565_to_argb4444(aipl_argb4444_px_t* dst,
                                            const aipl_rgb565_px_t* src)
{
    dst->ar = 0xf0 | (src->t >> 4);
    dst->gb = (src->t << 5) | ((src->b >> 3) & 0x10) | ((src->b >> 1) & 0x0f);
}

/**
 * Convert RGB565 pixel into ARGB1555
 *
 * @param dst destination ARGB1555 pixel pointer
 * @param src source RGB565 pixel pointer
 */
INLINE void aipl_cnvt_px_rgb565_to_argb1555(aipl_argb1555_px_t* dst,
                                            const aipl_rgb565_px_t* src)
{
    dst->t = 0x80 | (src->t >> 1);
    dst->b = (src->t << 7) | ((src->b >> 1) & 0x60) | (src->b & 0x1f);
}

/**
 * Convert RGB565 pixel into RGBA8888
 *
 * @param dst destination RGBA8888 pixel pointer
 * @param src source RGB565 pixel pointer
 */
INLINE void aipl_cnvt_px_rgb565_to_rgba8888(aipl_rgba8888_px_t* dst,
                                            const aipl_rgb565_px_t* src)
{
    dst->r = (src->t & 0xf8) | ((src->t >> 5) & 0x07);
    dst->g = (src->t & 0x03) | (src->t << 5) | ((src->b >> 3) & 0x1c);
    dst->b = ((src->b & 0x1f) << 3) | ((src->b >> 2) & 0x07);
    dst->a = 0xff;
}

/**
 * Convert RGB565 pixel into RGBA4444
 *
 * @param dst destination RGBA4444 pixel pointer
 * @param src source RGB565 pixel pointer
 */
INLINE void aipl_cnvt_px_rgb565_to_rgba4444(aipl_rgba4444_px_t* dst,
                                            const aipl_rgb565_px_t* src)
{
    dst->rg = (src->t & 0xf0) | ((src->t << 1) & 0x0f) | (src->b >> 7);
    dst->ba = ((src->b << 3) & 0xf0) | 0x0f;
}

/**
 * Convert RGB565 pixel into RGBA5551
 *
 * @param dst destination RGBA5551 pixel pointer
 * @param src source RGB565 pixel pointer
 */
INLINE void aipl_cnvt_px_rgb565_to_rgba5551(aipl_rgba5551_px_t* dst,
                                            const aipl_rgb565_px_t* src)
{
    dst->t = src->t;
    dst->b = (src->b & 0xc0) | ((src->b << 1) & 0x3e) | 0x01;
}

/**
 * Convert RGB565 pixel into 24bit
 *
 * @param dst       destination 24bit pixel pointer
 * @param src       source RGB565 pixel pointer
 * @param r_offset  red channel offset
 * @param g_offset  green channel offset
 * @param b_offset  blue channel offset
 */
INLINE void aipl_cnvt_px_rgb565_to_24bit(uint8_t* dst,
                                         const aipl_rgb565_px_t* src,
                                         uint8_t r_offset,
                                         uint8_t g_offset,
                                         uint8_t b_offset)
{
    dst[r_offset] = (src->t & 0xf8) | ((src->t >> 5) & 0x07);
    dst[g_offset] = (src->t & 0x03) | (src->t << 5) | ((src->b >> 3) & 0x1c);
    dst[b_offset] = ((src->b & 0x1f) << 3) | ((src->b >> 2) & 0x07);
}

/**
 * Convert RGB565 pixel into Y channel
 *
 * @param dst destination Y channel pointer
 * @param src source RGB565 pixel pointer
 */
INLINE void aipl_cnvt_px_rgb565_to_yuv_y(uint8_t* dst,
                                         const aipl_rgb565_px_t* src)
{
    uint8_t r = (src->t >> 3) & 0x1f;
    uint8_t g = ((src->t << 3) & 0x3f) | (src->b >> 5);
    uint8_t b = src->b & 0x1f;

    *dst = ((543 * r + 522 * g + 205 * b + 128) >> 8) + 16;
}

/**
 * Convert RGB565 pixel into U and V channels
 *
 * @param u_dst destination U channel pointer
 * @param v_dst destination V channel pointer
 * @param src   source RGBA5551 pixel pointer
 */
INLINE void aipl_cnvt_px_rgb565_to_yuv_uv(uint8_t* u_dst,
                                          uint8_t* v_dst,
                                          const aipl_rgb565_px_t* src)
{
    uint8_t r = (src->t >> 3) & 0x1f;
    uint8_t g = ((src->t << 3) & 0x3f) | (src->b >> 5);
    uint8_t b = src->b & 0x1f;

    *u_dst = ((-312 * r - 299 * g + 920 * b + 128) >> 8) + 128;
    *v_dst = ((920 * r - 380 * g - 147 * b + 128) >> 8) + 128;
}

/**
 * Convert RGB565 pixel into Y, U and V channels
 *
 * @param y_dst destination Y channel pointer
 * @param u_dst destination U channel pointer
 * @param v_dst destination V channel pointer
 * @param src   source RGB565 pixel pointer
 */
INLINE void aipl_cnvt_px_rgb565_to_yuv(uint8_t* y_dst,
                                       uint8_t* u_dst,
                                       uint8_t* v_dst,
                                       const aipl_rgb565_px_t* src)
{
    uint8_t r = (src->t >> 3) & 0x1f;
    uint8_t g = ((src->t << 3) & 0x3f) | (src->b >> 5);
    uint8_t b = src->b & 0x1f;

    *y_dst = ((543 * r + 522 * g + 205 * b + 128) >> 8) + 16;
    *u_dst = ((-312 * r - 299 * g + 920 * b + 128) >> 8) + 128;
    *v_dst = ((920 * r - 380 * g - 147 * b + 128) >> 8) + 128;
}

/**
 * Convert ALPHA8/I400 pixel into ARGB8888
 *
 * @param dst destination ARGB8888 pixel pointer
 * @param src source ALPHA8/I400 pixel pointer
 */
INLINE void aipl_cnvt_px_i400_to_argb8888(aipl_argb8888_px_t* dst,
                                          const uint8_t* src)
{
    dst->a = 0xff;
    dst->r = *src;
    dst->g = *src;
    dst->b = *src;
}

/**
 * Convert ALPHA8/I400 pixel into ARGB4444
 *
 * @param dst destination ARGB4444 pixel pointer
 * @param src source ALPHA8/I400 pixel pointer
 */
INLINE void aipl_cnvt_px_i400_to_argb4444(aipl_argb4444_px_t* dst,
                                          const uint8_t* src)
{
    dst->ar = 0xf0 | (*src >> 4);
    dst->gb = (*src & 0xf0) | (*src >> 4);
}

/**
 * Convert ALPHA8/I400 pixel into ARGB1555
 *
 * @param dst destination ARGB1555 pixel pointer
 * @param src source ALPHA8/I400 pixel pointer
 */
INLINE void aipl_cnvt_px_i400_to_argb1555(aipl_argb1555_px_t* dst,
                                          const uint8_t* src)
{
    dst->t = 0x80 | ((*src & 0xf8) >> 1) | (*src >> 6);
    dst->b = ((*src & 0xf8) << 2) | (*src >> 3);
}

/**
 * Convert ALPHA8/I400 pixel into RGBA8888
 *
 * @param dst destination RGBA8888 pixel pointer
 * @param src source ALPHA8/I400 pixel pointer
 */
INLINE void aipl_cnvt_px_i400_to_rgba8888(aipl_rgba8888_px_t* dst,
                                          const uint8_t* src)
{
    dst->a = 0xff;
    dst->r = *src;
    dst->g = *src;
    dst->b = *src;
}

/**
 * Convert ALPHA8/I400 pixel into RGBA4444
 *
 * @param dst destination RGBA4444 pixel pointer
 * @param src source ALPHA8/I400 pixel pointer
 */
INLINE void aipl_cnvt_px_i400_to_rgba4444(aipl_rgba4444_px_t* dst,
                                          const uint8_t* src)
{
    dst->rg = (*src & 0xf0) | (*src >> 4);
    dst->ba = (*src & 0xf0) | 0x0f;
}

/**
 * Convert ALPHA8/I400 pixel into RGBA5551
 *
 * @param dst destination RGBA5551 pixel pointer
 * @param src source ALPHA8/I400 pixel pointer
 */
INLINE void aipl_cnvt_px_i400_to_rgba5551(aipl_rgba5551_px_t* dst,
                                          const uint8_t* src)
{
    dst->t = (*src & 0xf8) | (*src >> 5);
    dst->b = ((*src & 0xf8) << 3) | (*src >> 2) | 0x01;
}

/**
 * Convert ALPHA8/I400 pixel into 24bit
 *
 * @param dst destination 24bit pixel pointer
 * @param src source ALPHA8/I400 pixel pointer
 */
INLINE void aipl_cnvt_px_i400_to_24bit(uint8_t* dst,
                                       const uint8_t* src)
{
    uint8_t tmp = *src;
    dst[0] = tmp;
    dst[1] = tmp;
    dst[2] = tmp;
}

/**
 * Convert ALPHA8/I400 pixel into RGB565
 *
 * @param dst destination RGB565 pixel pointer
 * @param src source ALPHA8/I400 pixel pointer
 */
INLINE void aipl_cnvt_px_i400_to_rgb565(aipl_rgb565_px_t* dst,
                                        const uint8_t* src)
{
    dst->t = (*src & 0xf8) | (*src >> 5);
    dst->b = ((*src & 0xfc) << 3) | (*src >> 3);
}

/**
 * Convert singed channel value to unsigned
 * and cap the value to range [0;255]
 *
 * @param val uncapped channel value
 * @return capped channel value
 */
INLINE uint8_t aipl_channel_cap(int16_t val)
{
    if (val & 0x8000) return 0;
    else if (val & 0xff00) return 0xff;
    else return val;
}

/**
 * Convert signed channel value to usigned 4 bits,
 * cap the value to [0; 15] and shift the bits
 * to upper half
 *
 * @param val uncapped channel value
 * @return capped channel value
 */
INLINE uint8_t aipl_channel_cap_upper_4bit(int8_t val)
{
    if (val & 0x80) return 0;
    else if (val & 0xf0) return 0xf0;
    else return val << 4;
}

/**
 * Convert signed channel value to usigned 5 bits,
 * cap the value to [0; 31] and shift the bits by
 * specified value
 *
 * @param val   uncapped channel value
 * @param shift bit shift value [0; 7]
 * @return capped channel value
 */
INLINE uint8_t aipl_channel_cap_5bit(int8_t val, uint8_t shift)
{
    if (val & 0x80) return 0;
    else if (val & 0xe0) return 0x1f;
    else return val << shift;
}

/**
 * Convert signed channel value to usigned 6 bits
 * and cap the value to [0; 63]
 *
 * @param val   uncapped channel value
 * @return capped channel value
 */
INLINE uint8_t aipl_channel_cap_6bit(int8_t val)
{
    if (val & 0x80) return 0;
    else if (val & 0xc0) return 0x3f;
    else return val;
}

/**
 * Convert signed channel value to usigned 4 bits,
 * cap the value to [0; 15] and shift the bits
 * to upper half
 *
 * @param val uncapped channel value
 * @return capped channel value
 */
INLINE uint8_t aipl_channel_cap_4bit(int8_t val)
{
    if (val & 0x80) return 0;
    else if (val & 0xf0) return 0x0f;
    else return val;
}

/**
 * A util function for color conversions from YUV formats.
 * Precalculate the values of RGB channels
 *
 * @param r     a pointer to output R precalculated channel data
 * @param g     a pointer to output G precalculated channel data
 * @param b     a pointer to output B precalculated channel data
 * @param u     U channel value
 * @param v     V channel value
 */
INLINE void aipl_pre_cnvt_px_yuv_to_rgb(int32_t* r, int32_t* g, int32_t* b,
                                        uint8_t u, uint8_t v)
{
    int16_t d = u - 128;
    int16_t e = v - 128;

    *r = 409 * e + 128;
    *g = -100 * d - 208 * e + 128;
    *b = 516 * d + 128;
}

/**
 * A util function for color conversions from YUV formats
 * Precalculate Y channel related values for RGB channels
 *
 * @param c precalculated value
 * @param y Y channel value
 */
INLINE void aipl_pre_cnvt_px_y(int32_t* c, uint8_t y)
{
    *c = (y - 16) * 298;
}

/**
 * A util function for color conversions from YUV formats
 * Precalculate 2x Y channel related values for RGB channels
 *
 * @param c0   0 pixel precalculated value
 * @param c1   1 pixel precalculated value
 * @param y0   0 pixel Y channel value
 * @param y1   1 pixel Y channel value
 */
INLINE void aipl_pre_cnvt_2px_y(int32_t* c0, int32_t* c1,
                                uint8_t y0, uint8_t y1)
{
    aipl_pre_cnvt_px_y(c0, y0);
    aipl_pre_cnvt_px_y(c1, y1);
}


/**
 * A util function for color conversions from YUV formats
 * Precalculate 2x2 Y channel related values for RGB channels
 *
 * @param c00   [0, 0] pixel precalculated value
 * @param c01   [0, 1] pixel precalculated value
 * @param c10   [1, 0] pixel precalculated value
 * @param c11   [1, 1] pixel precalculated value
 * @param y00   [0, 0] pixel Y channel value
 * @param y01   [0, 1] pixel Y channel value
 * @param y10   [1, 0] pixel Y channel value
 * @param y11   [1, 1] pixel Y channel value
 */
INLINE void aipl_pre_cnvt_2x2px_y(int32_t* c00, int32_t* c01,
                                  int32_t* c10, int32_t* c11,
                                  uint8_t y00, uint8_t y01,
                                  uint8_t y10, uint8_t y11)
{
    aipl_pre_cnvt_2px_y(c00, c01, y00, y01);
    aipl_pre_cnvt_2px_y(c10, c11, y10, y11);
}

/**
 * Convert YUV precalculated values to ARGB8888 pixel
 *
 * @param dst   destination ARGB8888 pixel pointer
 * @param c     source pixel Y precalculated value
 * @param r     source pixel R precalculated value
 * @param g     source pixel G precalculated value
 * @param b     source pixel B precalculated value
 */
INLINE void aipl_cnvt_px_yuv_to_argb8888(aipl_argb8888_px_t* dst,
                                         int32_t c, int32_t r,
                                         int32_t g, int32_t b)
{
    dst->a = 0xff;
    dst->r = aipl_channel_cap((c + r) >> 8);
    dst->g = aipl_channel_cap((c + g) >> 8);
    dst->b = aipl_channel_cap((c + b) >> 8);
}

/**
 * Convert YUV precalculated values to ARGB4444 pixel
 *
 * @param dst   destination ARGB4444 pixel pointer
 * @param c     source pixel Y precalculated value
 * @param r     source pixel R precalculated value
 * @param g     source pixel G precalculated value
 * @param b     source pixel B precalculated value
 */
INLINE void aipl_cnvt_px_yuv_to_argb4444(aipl_argb4444_px_t* dst,
                                         int32_t c, int32_t r,
                                         int32_t g, int32_t b)
{
    dst->ar = 0xf0 | aipl_channel_cap_4bit((c + r) >> 12);
    dst->gb = aipl_channel_cap_upper_4bit((c + g) >> 12)
              | aipl_channel_cap_4bit((c + b) >> 12);
}

/**
 * Convert YUV precalculated values to ARGB1555 pixel
 *
 * @param dst   destination ARGB1555 pixel pointer
 * @param c     source pixel Y precalculated value
 * @param r     source pixel R precalculated value
 * @param g     source pixel G precalculated value
 * @param b     source pixel B precalculated value
 */
INLINE void aipl_cnvt_px_yuv_to_argb1555(aipl_argb1555_px_t* dst,
                                         int32_t c, int32_t r,
                                         int32_t g, int32_t b)
{
    uint8_t g0 = aipl_channel_cap_5bit((c + g) >> 11, 0);
    dst->t = 0x80 | aipl_channel_cap_5bit((c + r) >> 11, 2)
             | (g0 >> 3);
    dst->b = (g0 << 5) | aipl_channel_cap_5bit((c + b) >> 11, 0);
}

/**
 * Convert YUV precalculated values to RGBA8888 pixel
 *
 * @param dst   destination RGBA8888 pixel pointer
 * @param c     source pixel Y precalculated value
 * @param r     source pixel R precalculated value
 * @param g     source pixel G precalculated value
 * @param b     source pixel B precalculated value
 */
INLINE void aipl_cnvt_px_yuv_to_rgba8888(aipl_rgba8888_px_t* dst,
                                         int32_t c, int32_t r,
                                         int32_t g, int32_t b)
{
    dst->r = aipl_channel_cap((c + r) >> 8);
    dst->g = aipl_channel_cap((c + g) >> 8);
    dst->b = aipl_channel_cap((c + b) >> 8);
    dst->a = 0xff;
}

/**
 * Convert YUV precalculated values to RGBA4444 pixel
 *
 * @param dst   destination RGBA4444 pixel pointer
 * @param c     source pixel Y precalculated value
 * @param r     source pixel R precalculated value
 * @param g     source pixel G precalculated value
 * @param b     source pixel B precalculated value
 */
INLINE void aipl_cnvt_px_yuv_to_rgba4444(aipl_rgba4444_px_t* dst,
                                         int32_t c, int32_t r,
                                         int32_t g, int32_t b)
{
    dst->rg = aipl_channel_cap_upper_4bit((c + r) >> 12)
              | aipl_channel_cap_4bit((c + g) >> 12);
    dst->ba = aipl_channel_cap_upper_4bit((c + b) >> 12)
              | 0x0f;
}

/**
 * Convert YUV precalculated values to RGBA5551 pixel
 *
 * @param dst   destination RGBA5551 pixel pointer
 * @param c     source pixel Y precalculated value
 * @param r     source pixel R precalculated value
 * @param g     source pixel G precalculated value
 * @param b     source pixel B precalculated value
 */
INLINE void aipl_cnvt_px_yuv_to_rgba5551(aipl_rgba5551_px_t* dst,
                                         int32_t c, int32_t r,
                                         int32_t g, int32_t b)
{
    uint8_t g0 = aipl_channel_cap_5bit((c + g) >> 11, 0);
    dst->t = aipl_channel_cap_5bit((c + r) >> 11, 3) | (g0 >> 2);
    dst->b = (g0 << 6) | aipl_channel_cap_5bit((c + b) >> 11, 1)
             | 0x01;
}

/**
 * Convert YUV precalculated values to 24bit pixel
 *
 * @param dst       destination 24bit pixel pointer
 * @param c         source pixel Y precalculated value
 * @param r         source pixel R precalculated value
 * @param g         source pixel G precalculated value
 * @param b         source pixel B precalculated value
 * @param r_offset  red channel offset
 * @param g_offset  green channel offset
 * @param b_offset  blue channel offset
 */
INLINE void aipl_cnvt_px_yuv_to_24bit(uint8_t* dst,
                                      int32_t c, int32_t r,
                                      int32_t g, int32_t b,
                                      uint8_t r_offset,
                                      uint8_t g_offset,
                                      uint8_t b_offset)
{
    dst[r_offset] = aipl_channel_cap((c + r) >> 8);
    dst[g_offset] = aipl_channel_cap((c + g) >> 8);
    dst[b_offset] = aipl_channel_cap((c + b) >> 8);
}

/**
 * Convert YUV precalculated values to RGB565 pixel
 *
 * @param dst   destination RGB565 pixel pointer
 * @param c     source pixel Y precalculated value
 * @param r     source pixel R precalculated value
 * @param g     source pixel G precalculated value
 * @param b     source pixel B precalculated value
 */
INLINE void aipl_cnvt_px_yuv_to_rgb565(aipl_rgb565_px_t* dst,
                                       int32_t c, int32_t r,
                                       int32_t g, int32_t b)
{
    uint8_t g0 = aipl_channel_cap_6bit((c + g) >> 10);
    dst->t = aipl_channel_cap_5bit((c + r) >> 11, 3) | (g0 >> 3);
    dst->b = (g0 << 5) | aipl_channel_cap_5bit((c + b) >> 11, 0);
}

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*AIPL_UTILS_H*/

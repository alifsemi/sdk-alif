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
 * @file    aipl_mve_utils.h
 * @brief   MVE utility types and functions
 *
******************************************************************************/

#ifndef AIPL_MVE_UTILS_H
#define AIPL_MVE_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "aipl_config.h"

#ifdef AIPL_HELIUM_ACCELERATION
#include "aipl_arm_mve.h"

/*********************
 *      DEFINES
 *********************/
/**
 * Create 16 offset values
 *
 * @param N number of pixels
 * @param S pixel size
 */
#define AIPL_OFFSETS_U8(N, S)   (vmulq_n_u8(vidupq_n_u8(0, S), N))

#define AIPL_4_BYTE_OFFSETS_U8  (vidupq_n_u8(0, 4))
#define AIPL_3_BYTE_OFFSETS_U8  (AIPL_OFFSETS_U8(3, 1))
#define AIPL_2_BYTE_OFFSETS_U8  (vidupq_n_u8(0, 2))

/**
 * Create 8 offset values
 *
 * @param N number of pixels
 * @param S pixel size
 */
#define AIPL_OFFSETS_U16(N, S)  (vmulq_n_u16(vidupq_n_u16(0, S), N))

#define AIPL_4_BYTE_OFFSETS_U16 (vidupq_n_u16(0, 4))
#define AIPL_3_BYTE_OFFSETS_U16 (AIPL_OFFSETS_U16(3, 1))
#define AIPL_2_BYTE_OFFSETS_U16 (vidupq_n_u16(0, 2))

/**
 * Create 4 offset values
 *
 * @param N number of pixels
 * @param S pixel size
 */
#define AIPL_OFFSETS_U32(N, S) (vmulq_n_u32(vidupq_n_u32(0, S), N))

#define AIPL_8_BYTE_OFFSETS_U32 (vidupq_n_u32(0, 8))
#define AIPL_4_BYTE_OFFSETS_U32 (vidupq_n_u32(0, 4))
#define AIPL_3_BYTE_OFFSETS_U32 (AIPL_OFFSETS_U32(3, 1))
#define AIPL_2_BYTE_OFFSETS_U32 (vidupq_n_u32(0, 2))

#define INLINE inline __attribute__((always_inline))

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    uint16x8_t a;
    uint16x8_t r;
    uint16x8_t g;
    uint16x8_t b;
} aipl_mve_argb_x8_t;

typedef struct {
    uint8x16_t a;
    uint8x16_t r;
    uint8x16_t g;
    uint8x16_t b;
} aipl_mve_argb_x16_t;

typedef struct {
    uint32x4_t r;
    uint32x4_t g;
    uint32x4_t b;
} aipl_mve_rgb_x4_t;

typedef struct {
    uint16x8_t r;
    uint16x8_t g;
    uint16x8_t b;
} aipl_mve_rgb_x8_t;

typedef struct {
    uint8x16_t r;
    uint8x16_t g;
    uint8x16_t b;
} aipl_mve_rgb_x16_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Get 8 even-numbered pixels from 16-pixel ARGB struct
 *
 * @param dst   8-pixel struct pointer
 * @param src   16-pixel struct pointer
 */
INLINE void aipl_mve_convert_argb_x16_to_x8_evn(aipl_mve_argb_x8_t* dst,
                                                const aipl_mve_argb_x16_t* src)
{
    dst->b = vmovlbq(src->b);
    dst->g = vmovlbq(src->g);
    dst->r = vmovlbq(src->r);
    dst->a = vmovlbq(src->a);
}

/**
 * Get 8 odd-numbered pixels from 16-pixel ARGB struct
 *
 * @param dst   8-pixel struct pointer
 * @param src   16-pixel struct pointer
 */
INLINE void aipl_mve_convert_argb_x16_to_x8_odd(aipl_mve_argb_x8_t* dst,
                                                const aipl_mve_argb_x16_t* src)
{
    dst->b = vmovltq(src->b);
    dst->g = vmovltq(src->g);
    dst->r = vmovltq(src->r);
    dst->a = vmovltq(src->a);
}

/**
 * Get 8 even-numbered pixels from 16-pixel RGB struct
 *
 * @param dst   8-pixel struct pointer
 * @param src   16-pixel struct pointer
 */
INLINE void aipl_mve_convert_rgb_x16_to_x8_evn(aipl_mve_rgb_x8_t* dst,
                                               const aipl_mve_rgb_x16_t* src)
{
    dst->b = vmovlbq(src->b);
    dst->g = vmovlbq(src->g);
    dst->r = vmovlbq(src->r);
}

/**
 * Get 8 odd-numbered pixels from 16-pixel RGB struct
 *
 * @param dst   8-pixel struct pointer
 * @param src   16-pixel struct pointer
 */
INLINE void aipl_mve_convert_rgb_x16_to_x8_odd(aipl_mve_rgb_x8_t* dst,
                                               const aipl_mve_rgb_x16_t* src)
{
    dst->b = vmovltq(src->b);
    dst->g = vmovltq(src->g);
    dst->r = vmovltq(src->r);
}

/**
 * Get 16-pixel struct from two 8-pixel ARGBs
 *
 * @param dst       16-pixel struct pointer
 * @param src_evn   8-pixel even struct pointer
 * @param src_odd   8-pixel odd struct pointer
 */
INLINE void aipl_mve_convert_2_argb_x8_to_x16(aipl_mve_argb_x16_t* dst,
                                              const aipl_mve_argb_x8_t* src_evn,
                                              const aipl_mve_argb_x8_t* src_odd)
{
    dst->b = vmovntq(vreinterpretq_u8(src_evn->b), src_odd->b);
    dst->g = vmovntq(vreinterpretq_u8(src_evn->g), src_odd->g);
    dst->r = vmovntq(vreinterpretq_u8(src_evn->r), src_odd->r);
    dst->a = vmovntq(vreinterpretq_u8(src_evn->a), src_odd->a);
}

/**
 * Get 16-pixel struct from two 8-pixel RGBs
 *
 * @param dst       16-pixel struct pointer
 * @param src_evn   8-pixel even struct pointer
 * @param src_odd   8-pixel odd struct pointer
 */
INLINE void aipl_mve_convert_2_rgb_x8_to_x16(aipl_mve_rgb_x16_t* dst,
                                             const aipl_mve_rgb_x8_t* src_evn,
                                             const aipl_mve_rgb_x8_t* src_odd)
{
    dst->b = vmovntq(vreinterpretq_u8(src_evn->b), src_odd->b);
    dst->g = vmovntq(vreinterpretq_u8(src_evn->g), src_odd->g);
    dst->r = vmovntq(vreinterpretq_u8(src_evn->r), src_odd->r);
}

/**
 * Load 4 ARGB8888 pixels from memory to
 * Helium vector register
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pointer
 * @param pred  vector predicate
 */
INLINE void aipl_mve_ldr_4px_argb8888(uint32x4_t* dst,
                                      const uint32_t* src,
                                      mve_pred16_t pred)
{
    *dst = vld1q_z(src, pred);
}

/**
 * Load 8 ARGB8888 pixels from memory to
 * A, R, G and B Helium vector registers
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pointer
 * @param pred  vector predicate
 */
INLINE void aipl_mve_ldr_8px_argb8888(aipl_mve_argb_x8_t* dst,
                                      const uint8_t* src,
                                      mve_pred16_t pred)
{
    dst->b = vldrbq_gather_offset_z(src, AIPL_4_BYTE_OFFSETS_U16, pred);
    dst->g = vldrbq_gather_offset_z(src + 1, AIPL_4_BYTE_OFFSETS_U16, pred);
    dst->r = vldrbq_gather_offset_z(src + 2, AIPL_4_BYTE_OFFSETS_U16, pred);
    dst->a = vldrbq_gather_offset_z(src + 3, AIPL_4_BYTE_OFFSETS_U16, pred);
}

/**
 * Load 8 XRGB8888 pixels from memory to
 * R, G and B Helium vector registers
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pointer
 * @param pred  vector predicate
 */
INLINE void aipl_mve_ldr_8px_xrgb8888(aipl_mve_rgb_x8_t* dst,
                                      const uint8_t* src,
                                      mve_pred16_t pred)
{
    dst->b = vldrbq_gather_offset_z(src, AIPL_4_BYTE_OFFSETS_U16, pred);
    dst->g = vldrbq_gather_offset_z(src + 1, AIPL_4_BYTE_OFFSETS_U16, pred);
    dst->r = vldrbq_gather_offset_z(src + 2, AIPL_4_BYTE_OFFSETS_U16, pred);
}

/**
 * Load 8 XRGB8888 pixels from memory with offset to
 * R, G and B Helium vector registers
 *
 * @param dst       destination pixel vector pointer
 * @param src       source pointer
 * @param offset    pixel offset
 * @param pred      vector predicate
 */
INLINE void aipl_mve_ldr_8px_offset_xrgb8888(aipl_mve_rgb_x8_t* dst,
                                             const uint8_t* src,
                                             uint8_t offset,
                                             mve_pred16_t pred)
{
    dst->b = vldrbq_gather_offset_z(src, AIPL_OFFSETS_U16(offset, 4), pred);
    dst->g = vldrbq_gather_offset_z(src + 1, AIPL_OFFSETS_U16(offset, 4), pred);
    dst->r = vldrbq_gather_offset_z(src + 2, AIPL_OFFSETS_U16(offset, 4), pred);
}

/**
 * Load 16 XRGB8888 pixels from memory to
 * R, G and B Helium vector registers
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pointer
 * @param pred  vector predicate
 */
INLINE void aipl_mve_ldr_16px_xrgb8888(aipl_mve_rgb_x16_t* dst,
                                       const uint8_t* src,
                                       mve_pred16_t pred)
{
    dst->b = vldrbq_gather_offset_z(src, AIPL_4_BYTE_OFFSETS_U8, pred);
    dst->g = vldrbq_gather_offset_z(src + 1, AIPL_4_BYTE_OFFSETS_U8, pred);
    dst->r = vldrbq_gather_offset_z(src + 2, AIPL_4_BYTE_OFFSETS_U8, pred);
}

/**
 * Load 16 ARGB8888 pixels from memory to
 * A, R, G and B Helium vector registers
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pointer
 */
INLINE void aipl_mve_ldr_16px_argb8888_uncut(aipl_mve_argb_x16_t* dst,
                                             const uint8_t* src)
{
    uint8x16x4_t argb = vld4q(src);

    dst->b = argb.val[0];
    dst->g = argb.val[1];
    dst->r = argb.val[2];
    dst->a = argb.val[3];
}

/**
 * Load 16 XRGB8888 pixels from memory to
 * A, R, G and B Helium vector registers
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pointer
 */
INLINE void aipl_mve_ldr_16px_xrgb8888_uncut(aipl_mve_rgb_x16_t* dst,
                                             const uint8_t* src)
{
    uint8x16x4_t argb = vld4q(src);

    dst->b = argb.val[0];
    dst->g = argb.val[1];
    dst->r = argb.val[2];
}

/**
 * Load 16 ARGB8888 pixels from memory to
 * A, R, G and B Helium vector registers with predicate
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pointer
 * @param pred  vector predicate
 */
INLINE void aipl_mve_ldr_16px_argb8888(aipl_mve_argb_x16_t* dst,
                                       const uint8_t* src,
                                       mve_pred16_t pred)
{
    dst->b = vldrbq_gather_offset_z(src, AIPL_4_BYTE_OFFSETS_U8, pred);
    dst->g = vldrbq_gather_offset_z(src + 1, AIPL_4_BYTE_OFFSETS_U8, pred);
    dst->r = vldrbq_gather_offset_z(src + 2, AIPL_4_BYTE_OFFSETS_U8, pred);
    dst->a = vldrbq_gather_offset_z(src + 3, AIPL_4_BYTE_OFFSETS_U8, pred);
}

/**
 * Convert 4 ARGB8888 pixels to RGBA8888
 * using Helium vector register
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pixel vector
 */
INLINE void aipl_mve_cnvt_4px_argb8888_to_rgba8888(uint32x4_t* dst,
                                                   uint32x4_t src)
{
    uint32x4_t a = vshrq(src, 24);
    *dst = vsliq(a, src, 8);
}

/**
 * Convert 8 XRGB8888 pixels to Y channel
 * using Helium vector register
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pixel vector
 */
INLINE void aipl_mve_cnvt_8px_xrgb8888_to_yuv_y(uint16x8_t* dst,
                                                aipl_mve_rgb_x8_t src)
{
    *dst = vmulq_n_u16(src.r, 66);
    *dst = vmlaq_n_u16(*dst, src.g, 129);
    *dst = vmlaq_n_u16(*dst, src.b, 25);

    *dst = vshrq(vaddq(*dst, 128), 8);
    *dst = vaddq(*dst, 16);
}

/**
 * Convert 16 XRGB8888 pixels to Y channel
 * using Helium vector register
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pixel vector
 */
INLINE void aipl_mve_cnvt_16px_xrgb8888_to_yuv_y(uint8x16_t* dst,
                                                 aipl_mve_rgb_x16_t src)
{
    aipl_mve_rgb_x8_t t, b;
    aipl_mve_convert_rgb_x16_to_x8_evn(&t, &src);
    aipl_mve_convert_rgb_x16_to_x8_odd(&b, &src);

    uint16x8_t t_r, b_r;
    aipl_mve_cnvt_8px_xrgb8888_to_yuv_y(&t_r, t);
    aipl_mve_cnvt_8px_xrgb8888_to_yuv_y(&b_r, b);

    *dst = vmovntq(vreinterpretq_u8(t_r), b_r);
}

/**
 * Convert 8 XRGB8888 pixels to U channel
 * using Helium vector register
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pixel vector
 */
INLINE void aipl_mve_cnvt_8px_xrgb8888_to_yuv_u(uint16x8_t* dst,
                                                aipl_mve_rgb_x8_t src)
{
    *dst = vmulq_n_u16(src.r, -38);
    *dst = vmlaq_n_u16(*dst, src.g, -74);
    *dst = vmlaq_n_u16(*dst, src.b, 112);

    *dst = vshrq(vaddq(*dst, 128), 8);
    *dst = vaddq(*dst, 128);
}

/**
 * Convert 16 XRGB8888 pixels to U channel
 * using Helium vector register
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pixel vector
 */
INLINE void aipl_mve_cnvt_16px_xrgb8888_to_yuv_u(uint8x16_t* dst,
                                                 aipl_mve_rgb_x16_t src)
{
    aipl_mve_rgb_x8_t t, b;
    aipl_mve_convert_rgb_x16_to_x8_evn(&t, &src);
    aipl_mve_convert_rgb_x16_to_x8_odd(&b, &src);

    uint16x8_t t_r, b_r;
    aipl_mve_cnvt_8px_xrgb8888_to_yuv_u(&t_r, t);
    aipl_mve_cnvt_8px_xrgb8888_to_yuv_u(&b_r, b);

    *dst = vmovntq(vreinterpretq_u8(t_r), b_r);
}

/**
 * Convert 8 XRGB8888 pixels to V channel
 * using Helium vector register
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pixel vector
 */
INLINE void aipl_mve_cnvt_8px_xrgb8888_to_yuv_v(uint16x8_t* dst,
                                                aipl_mve_rgb_x8_t src)
{
    *dst = vmulq_n_u16(src.r, 112);
    *dst = vmlaq_n_u16(*dst, src.g, -94);
    *dst = vmlaq_n_u16(*dst, src.b, -18);

    *dst = vshrq(vaddq(*dst, 128), 8);
    *dst = vaddq(*dst, 128);
}

/**
 * Convert 16 XRGB8888 pixels to V channel
 * using Helium vector register
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pixel vector
 */
INLINE void aipl_mve_cnvt_16px_xrgb8888_to_yuv_v(uint8x16_t* dst,
                                                 aipl_mve_rgb_x16_t src)
{
    aipl_mve_rgb_x8_t t, b;
    aipl_mve_convert_rgb_x16_to_x8_evn(&t, &src);
    aipl_mve_convert_rgb_x16_to_x8_odd(&b, &src);

    uint16x8_t t_r, b_r;
    aipl_mve_cnvt_8px_xrgb8888_to_yuv_v(&t_r, t);
    aipl_mve_cnvt_8px_xrgb8888_to_yuv_v(&b_r, b);

    *dst = vmovntq(vreinterpretq_u8(t_r), b_r);
}

/**
 * Store 4 ARGB8888 pixels to memory from
 * Helium vector register
 *
 * @param dst   destintation pointer
 * @param src   source pixel vectors
 * @param pred  vector predicate
 */
INLINE void aipl_mve_str_4px_argb8888(uint32_t* dst,
                                      uint32x4_t src,
                                      mve_pred16_t pred)
{
    vst1q_p(dst, src, pred);
}

/**
 * Store 8 ARGB8888 pixels to memory from
 * A, R, G and B Helium vector registers
 *
 * @param dst   destintation pointer
 * @param src   source pixel vectors
 * @param pred  vector predicate
 */
INLINE void aipl_mve_str_8px_argb8888(uint8_t* dst,
                                      aipl_mve_argb_x8_t src,
                                      mve_pred16_t pred)
{
    vstrbq_scatter_offset_p(dst, AIPL_4_BYTE_OFFSETS_U16, src.b, pred);
    vstrbq_scatter_offset_p(dst + 1, AIPL_4_BYTE_OFFSETS_U16, src.g, pred);
    vstrbq_scatter_offset_p(dst + 2, AIPL_4_BYTE_OFFSETS_U16, src.r, pred);
    vstrbq_scatter_offset_p(dst + 3, AIPL_4_BYTE_OFFSETS_U16, src.a, pred);
}

/**
 * Store 16 XRGB8888 pixels to memory from
 * R, G and B Helium vector registers
 *
 * @param dst   destintation pointer
 * @param src   source pixel vectors
 * @param pred  vector predicate
 */
INLINE void aipl_mve_str_16px_xrgb8888(uint8_t* dst,
                                       aipl_mve_rgb_x16_t src,
                                       mve_pred16_t pred)
{
    vstrbq_scatter_offset_p(dst, AIPL_4_BYTE_OFFSETS_U8, src.b, pred);
    vstrbq_scatter_offset_p(dst + 1, AIPL_4_BYTE_OFFSETS_U8, src.g, pred);
    vstrbq_scatter_offset_p(dst + 2, AIPL_4_BYTE_OFFSETS_U8, src.r, pred);
    vstrbq_scatter_offset_p(dst + 3, AIPL_4_BYTE_OFFSETS_U8, vdupq_n_u8(0xff), pred);
}

/**
 * Store 16 ARGB8888 pixels to memory from
 * A, R, G and B Helium vector registers
 *
 * @param dst   destintation pointer
 * @param src   source pixel vectors
 * @param pred  vector predicate
 */
INLINE void aipl_mve_str_16px_argb8888_uncut(uint8_t* dst,
                                             aipl_mve_argb_x16_t src)
{
    uint8x16x4_t argb = { { src.b, src.g, src.r, src.a } };

    vst4q(dst, argb);
}

/**
 * Store 16 XRGB8888 pixels to memory from
 * R, G and B Helium vector registers
 *
 * @param dst   destintation pointer
 * @param src   source pixel vectors
 * @param pred  vector predicate
 */
INLINE void aipl_mve_str_16px_xrgb8888_uncut(uint8_t* dst,
                                             aipl_mve_rgb_x16_t src)
{
    uint8x16x4_t argb = { { src.b, src.g, src.r, vdupq_n_u8(0xff) } };

    vst4q(dst, argb);
}

/**
 * Store 16 ARGB8888 pixels to memory from
 * A, R, G and B Helium vector registers with predicate
 *
 * @param dst   destintation pointer
 * @param src   source pixel vectors
 * @param pred  vector predicate
 */
INLINE void aipl_mve_str_16px_argb8888(uint8_t* dst,
                                       aipl_mve_argb_x16_t src,
                                       mve_pred16_t pred)
{
    vstrbq_scatter_offset_p(dst, AIPL_4_BYTE_OFFSETS_U8, src.b, pred);
    vstrbq_scatter_offset_p(dst + 1, AIPL_4_BYTE_OFFSETS_U8, src.g, pred);
    vstrbq_scatter_offset_p(dst + 2, AIPL_4_BYTE_OFFSETS_U8, src.r, pred);
    vstrbq_scatter_offset_p(dst + 3, AIPL_4_BYTE_OFFSETS_U8, src.a, pred);
}

/**
 * Load 8 ARGB4444 pixels from memory with offset to
 * Helium vector register
 *
 * @param dst       destination pixel vectors
 * @param src       source pixel vectors
 * @param offset    pixel offset
 * @param pred      vector predicate
 */
INLINE void aipl_mve_ldr_8px_offset_argb4444(uint16x8_t* dst,
                                             const uint16_t* src,
                                             uint8_t offset,
                                             mve_pred16_t pred)
{
    *dst = vldrhq_gather_offset_z(src, AIPL_OFFSETS_U16(offset, 2), pred);
}

/**
 * Load 8 ARGB4444 pixels from memory with extend to
 * A, R, G and B Helium vector registers
 *
 * @param dst       destination pixel vectors
 * @param src       source pixel vectors
 * @param pred      vector predicate
 */
INLINE void aipl_mve_ldr_8px_extend_argb4444(aipl_mve_argb_x8_t* dst,
                                             const uint16_t* src,
                                             mve_pred16_t pred)
{
    uint16x8_t gb = vldrbq_gather_offset_z((uint8_t*)src, AIPL_2_BYTE_OFFSETS_U16, pred);
    uint16x8_t ar = vldrbq_gather_offset_z((uint8_t*)src + 1, AIPL_2_BYTE_OFFSETS_U16, pred);

    dst->b = vmulq_u16(vandq(gb, vdupq_n_u16(0x000f)), vdupq_n_u16(0x0011));
    dst->g = vorrq(vandq(gb, vdupq_n_u16(0x00f0)), vshrq(gb, 4));
    dst->r = vmulq_u16(vandq(ar, vdupq_n_u16(0x000f)), vdupq_n_u16(0x0011));
    dst->a = vorrq(vandq(ar, vdupq_n_u16(0x00f0)), vshrq(ar, 4));
}

/**
 * Load 8 ARGB4444 pixels from memory to
 * Helium vector register
 *
 * @param dst   destination pixel vectors
 * @param src   source pixel vectors
 * @param pred  vector predicate
 */
INLINE void aipl_mve_ldr_8px_argb4444(uint16x8_t* dst,
                                      const uint16_t* src,
                                      mve_pred16_t pred)
{
    *dst = vldrhq_z_u16(src, pred);
}

/**
 * Load 16 XRGB4444 pixels to memory from
 * R, G and B Helium vector registers
 *
 * @param dst   destination pixel vectors
 * @param src   source pixel vectors
 * @param pred  vector predicate
 */
INLINE void aipl_mve_ldr_16px_xrgb4444(aipl_mve_rgb_x16_t* dst,
                                       const uint16_t* src,
                                       mve_pred16_t pred)
{
    uint8x16_t gb = vldrbq_gather_offset_z((uint8_t*)src, AIPL_2_BYTE_OFFSETS_U8, pred);
    uint8x16_t ar = vldrbq_gather_offset_z((uint8_t*)src + 1, AIPL_2_BYTE_OFFSETS_U8, pred);

    dst->r = vmulq_u8(vandq(ar, vdupq_n_u8(0x0f)), vdupq_n_u8(0x11));
    dst->g = vorrq(vandq(gb, vdupq_n_u8(0xf0)), vshrq(gb, 4));
    dst->b = vmulq_u8(vandq(gb, vdupq_n_u8(0x0f)), vdupq_n_u8(0x11));
}

/**
 * Load 16 ARGB4444 pixels to memory from
 * A, R, G and B Helium vector registers with vector predicate
 *
 * @param dst   destination pixel vectors
 * @param src   source pixel vectors
 * @param pred  vector predicate
 */
INLINE void aipl_mve_ldr_16px_argb4444(aipl_mve_argb_x16_t* dst,
                                       const uint16_t* src,
                                       mve_pred16_t pred)
{
    uint8x16_t gb = vldrbq_gather_offset_z((uint8_t*)src, AIPL_2_BYTE_OFFSETS_U8, pred);
    uint8x16_t ar = vldrbq_gather_offset_z((uint8_t*)src + 1, AIPL_2_BYTE_OFFSETS_U8, pred);

    dst->b = vmulq_u8(vandq(gb, vdupq_n_u8(0x0f)), vdupq_n_u8(0x11));
    dst->g = vorrq(vandq(gb, vdupq_n_u8(0xf0)), vshrq(gb, 4));
    dst->r = vmulq_u8(vandq(ar, vdupq_n_u8(0x0f)), vdupq_n_u8(0x11));
    dst->a = vorrq(vandq(ar, vdupq_n_u8(0xf0)), vshrq(ar, 4));
}

/**
 * Load 16 ARGB4444 pixels to memory from
 * A, R, G and B Helium vector registers without vector predicate
 *
 * @param dst   destination pixel vectors
 * @param src   source pixel vectors
 */
INLINE void aipl_mve_ldr_16px_argb4444_uncut(aipl_mve_argb_x16_t* dst,
                                             const uint16_t* src)
{
    uint8x16x2_t argb = vld2q((const uint8_t*)src);

    dst->b = vmulq_u8(vandq(argb.val[0], vdupq_n_u8(0x0f)), vdupq_n_u8(0x11));
    dst->g = vorrq(vandq(argb.val[0], vdupq_n_u8(0xf0)), vshrq(argb.val[0], 4));
    dst->r = vmulq_u8(vandq(argb.val[1], vdupq_n_u8(0x0f)), vdupq_n_u8(0x11));
    dst->a = vorrq(vandq(argb.val[1], vdupq_n_u8(0xf0)), vshrq(argb.val[1], 4));
}

/**
 * Load 16 XRGB4444 pixels to memory from
 * A, R, G and B Helium vector registers without vector predicate
 *
 * @param dst   destination pixel vectors
 * @param src   source pixel vectors
 */
INLINE void aipl_mve_ldr_16px_xrgb4444_uncut(aipl_mve_rgb_x16_t* dst,
                                             const uint16_t* src)
{
    uint8x16x2_t argb = vld2q((const uint8_t*)src);

    dst->b = vmulq_u8(vandq(argb.val[0], vdupq_n_u8(0x0f)), vdupq_n_u8(0x11));
    dst->g = vorrq(vandq(argb.val[0], vdupq_n_u8(0xf0)), vshrq(argb.val[0], 4));
    dst->r = vmulq_u8(vandq(argb.val[1], vdupq_n_u8(0x0f)), vdupq_n_u8(0x11));
}

/**
 * Convert 8 ARGB4444 pixels to RGBA4444
 * using Helium vector register
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pixel vector
 */
INLINE void aipl_mve_cnvt_8px_argb4444_to_rgba4444(uint16x8_t* dst,
                                                   uint16x8_t src)
{
    *dst = vshrq(src, 12);
    *dst = vsliq(*dst, src, 4);
}

/**
 * Convert 8 ARGB4444 pixels to Y channel
 * using Helium vector register
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pixel vector
 */
INLINE void aipl_mve_cnvt_8px_argb4444_to_yuv_y(uint16x8_t* dst,
                                                uint16x8_t src)
{
    uint16x8_t r = vandq(vshrq(src, 8), vdupq_n_u16(0x000f));
    uint16x8_t g = vandq(vshrq(src, 4), vdupq_n_u16(0x000f));
    uint16x8_t b = vandq(src, vdupq_n_u16(0x000f));

    *dst = vmulq_n_u16(r, 66 * 0x11);
    *dst = vmlaq_n_u16(*dst, g, 129 * 0x11);
    *dst = vmlaq_n_u16(*dst, b, 25 * 0x11);
    *dst = vshrq(vaddq(*dst, 128), 8);
    *dst = vaddq(*dst, 16);
}

/**
 * Convert 8 ARGB4444 pixels to U and V channels
 * using Helium vector register
 *
 * @param u_dst U channel destination vector pointer
 * @param v_dst V channel destination vector pointer
 * @param src   source pixel vector
 */
INLINE void aipl_mve_cnvt_8px_argb4444_to_yuv_uv(uint16x8_t* u_dst,
                                                 uint16x8_t* v_dst,
                                                 uint16x8_t src)
{
    uint16x8_t r = vandq(vshrq(src, 8), vdupq_n_u16(0x000f));
    uint16x8_t g = vandq(vshrq(src, 4), vdupq_n_u16(0x000f));
    uint16x8_t b = vandq(src, vdupq_n_u16(0x000f));

    *u_dst = vmulq_n_u16(r, -38 * 0x11);
    *u_dst = vmlaq_n_u16(*u_dst, g, -74 * 0x11);
    *u_dst = vmlaq_n_u16(*u_dst, b, 112 * 0x11);
    *u_dst = vshrq(vaddq(*u_dst, 128), 8);
    *u_dst = vaddq(*u_dst, 128);

    *v_dst = vmulq_n_u16(r, 112 * 0x11);
    *v_dst = vmlaq_n_u16(*v_dst, g, -94 * 0x11);
    *v_dst = vmlaq_n_u16(*v_dst, b, -18 * 0x11);
    *v_dst = vshrq(vaddq(*v_dst, 128), 8);
    *v_dst = vaddq(*v_dst, 128);
}

/**
 * Convert 8 ARGB4444 pixels to Y, U and V channels
 * using Helium vector register
 *
 * @param y_dst Y channel destination vector pointer
 * @param u_dst U channel destination vector pointer
 * @param v_dst V channel destination vector pointer
 * @param src   source pixel vector
 */
INLINE void aipl_mve_cnvt_8px_argb4444_to_yuv(uint16x8_t* y_dst,
                                              uint16x8_t* u_dst,
                                              uint16x8_t* v_dst,
                                              uint16x8_t src)
{
    uint16x8_t r = vandq(vshrq(src, 8), vdupq_n_u16(0x000f));
    uint16x8_t g = vandq(vshrq(src, 4), vdupq_n_u16(0x000f));
    uint16x8_t b = vandq(src, vdupq_n_u16(0x000f));

    *y_dst = vmulq_n_u16(r, 66 * 0x11);
    *y_dst = vmlaq_n_u16(*y_dst, g, 129 * 0x11);
    *y_dst = vmlaq_n_u16(*y_dst, b, 25 * 0x11);
    *y_dst = vshrq(vaddq(*y_dst, 128), 8);
    *y_dst = vaddq(*y_dst, 16);

    *u_dst = vmulq_n_u16(r, -38 * 0x11);
    *u_dst = vmlaq_n_u16(*u_dst, g, -74 * 0x11);
    *u_dst = vmlaq_n_u16(*u_dst, b, 112 * 0x11);
    *u_dst = vshrq(vaddq(*u_dst, 128), 8);
    *u_dst = vaddq(*u_dst, 128);

    *v_dst = vmulq_n_u16(r, 112 * 0x11);
    *v_dst = vmlaq_n_u16(*v_dst, g, -94 * 0x11);
    *v_dst = vmlaq_n_u16(*v_dst, b, -18 * 0x11);
    *v_dst = vshrq(vaddq(*v_dst, 128), 8);
    *v_dst = vaddq(*v_dst, 128);
}

/**
 * Store 8 ARGB4444 pixels to memory from
 * Helium vector register
 *
 * @param dst   destintation pointer
 * @param src   source pixel vectors
 * @param pred  vector predicate
 */
INLINE void aipl_mve_str_8px_argb4444(uint16_t* dst,
                                      uint16x8_t src,
                                      mve_pred16_t pred)
{
    vstrhq_p(dst, src, pred);
}

/**
 * Store 8 ARGB4444 pixels to memory from extended
 * A, R, G and B Helium vector registers
 *
 * @param dst   destintation pointer
 * @param src   source pixel vectors
 * @param pred  vector predicate
 */
INLINE void aipl_mve_str_8px_extended_argb4444(uint16_t* dst,
                                               aipl_mve_argb_x8_t src,
                                               mve_pred16_t pred)
{
    uint8x16_t ar = vreinterpretq_u8(src.a);
    ar = vsriq(ar, vreinterpretq_u8(src.r), 4);
    uint8x16_t gb = vreinterpretq_u8(src.g);
    gb = vsriq(gb, vreinterpretq_u8(src.b), 4);
    uint16x8_t pix = vreinterpretq_u16(vmovntq(gb, vreinterpretq_u16(ar)));

    vst1q_p(dst, pix, pred);
}

/**
 * Store 16 ARGB4444 pixels to memory from
 * A, R, G and B Helium vector registers with vector predicate
 *
 * @param dst   destintation pointer
 * @param src   source pixel vectors
 * @param pred  vector predicate
 */
INLINE void aipl_mve_str_16px_argb4444(uint16_t* dst,
                                       aipl_mve_argb_x16_t src,
                                       mve_pred16_t pred)
{
    uint8x16_t gb = vsriq(src.g, src.b, 4);
    uint8x16_t ar = vsriq(src.a, src.r, 4);

    vstrbq_scatter_offset_p((uint8_t*)dst, AIPL_2_BYTE_OFFSETS_U8, gb, pred);
    vstrbq_scatter_offset_p((uint8_t*)dst + 1, AIPL_2_BYTE_OFFSETS_U8, ar, pred);
}

/**
 * Store 16 ARGB4444 pixels to memory from
 * A, R, G and B Helium vector registers without vector predicate
 *
 * @param dst   destintation pointer
 * @param src   source pixel vectors
 */
INLINE void aipl_mve_str_16px_argb4444_uncut(uint16_t* dst,
                                             aipl_mve_argb_x16_t src)
{
    uint8x16x2_t argb = { vsriq(src.g, src.b, 4), vsriq(src.a, src.r, 4) };

    vst2q((uint8_t*)dst, argb);
}

/**
 * Store 16 XRGB4444 pixels to memory from
 * R, G and B Helium vector registers
 *
 * @param dst   destintation pointer
 * @param src   source pixel vectors
 * @param pred  vector predicate
 */
INLINE void aipl_mve_str_16px_xrgb4444(uint16_t* dst,
                                       aipl_mve_rgb_x16_t src,
                                       mve_pred16_t pred)
{
    uint8x16_t ar = vsriq(vdupq_n_u8(0xf0), src.r, 4);
    uint8x16_t gb = vsriq(src.g, src.b, 4);

    vstrbq_scatter_offset_p((uint8_t*)dst, AIPL_2_BYTE_OFFSETS_U8, gb, pred);
    vstrbq_scatter_offset_p((uint8_t*)dst + 1, AIPL_2_BYTE_OFFSETS_U8, ar, pred);
}

/**
 * Store 16 XRGB4444 pixels to memory from
 * R, G and B Helium vector registers without vector predicate
 *
 * @param dst   destintation pointer
 * @param src   source pixel vectors
 */
INLINE void aipl_mve_str_16px_xrgb4444_uncut(uint16_t* dst,
                                             aipl_mve_rgb_x16_t src)
{
    uint8x16x2_t argb = { vsriq(src.g, src.b, 4), vsriq(vdupq_n_u8(0xf0), src.r, 4) };

    vst2q((uint8_t*)dst, argb);
}

/**
 * Load 8 ARGB1555 pixels from memory with offset to
 * Helium vector register
 *
 * @param dst       destination pixel vectors
 * @param src       source pixel vectors
 * @param offset    pixel offset
 * @param pred      vector predicate
 */
INLINE void aipl_mve_ldr_8px_offset_argb1555(uint16x8_t* dst,
                                             const uint16_t* src,
                                             uint8_t offset,
                                             mve_pred16_t pred)
{
    *dst = vldrhq_gather_offset_z(src, AIPL_OFFSETS_U16(offset, 2), pred);
}

/**
 * Load 8 ARGB1555 pixels from memory to
 * Helium vector register
 *
 * @param dst   destination pixel vectors
 * @param src   source pixel vectors
 * @param pred  vector predicate
 */
INLINE void aipl_mve_ldr_8px_argb1555(uint16x8_t* dst,
                                      const uint16_t* src,
                                      mve_pred16_t pred)
{
    *dst = vldrhq_z_u16(src, pred);
}

/**
 * Load 16 XRGB1555 pixels from memory to
 * R, G and B Helium vector registers with vector predicate
 *
 * @param dst   destination pixel vectors
 * @param src   source pixel vectors
 * @param pred  vector predicate
 */
INLINE void aipl_mve_ldr_16px_xrgb1555(aipl_mve_rgb_x16_t* dst,
                                       const uint16_t* src,
                                       mve_pred16_t pred)
{
    uint8x16_t lower = vldrbq_gather_offset_z((uint8_t*)src, AIPL_2_BYTE_OFFSETS_U8, pred);
    uint8x16_t upper = vldrbq_gather_offset_z((uint8_t*)src + 1, AIPL_2_BYTE_OFFSETS_U8, pred);

    dst->r = vandq(vshlq_n(upper, 1), vdupq_n_u8(0xf8));
    dst->r = vorrq(dst->r, vshrq(dst->r, 5));
    dst->g = vorrq(vshlq_n(upper, 6), vshrq(vandq(lower, vdupq_n_u8(0xe0)), 2));
    dst->g = vorrq(dst->g, vshrq(dst->g, 5));
    dst->b = vshlq_n(lower, 3);
    dst->b = vorrq(dst->b, vshrq(dst->b, 5));
}

/**
 * Load 16 ARGB1555 pixels from memory to
 * A, G, R and B Helium vector registers with vector predicate
 *
 * @param dst   destination pixel vectors
 * @param src   source pixel vectors
 * @param pred  vector predicate
 */
INLINE void aipl_mve_ldr_16px_argb1555(aipl_mve_argb_x16_t* dst,
                                       const uint16_t* src,
                                       mve_pred16_t pred)
{
    uint8x16_t lower = vldrbq_gather_offset_z((uint8_t*)src, AIPL_2_BYTE_OFFSETS_U8, pred);
    uint8x16_t upper = vldrbq_gather_offset_z((uint8_t*)src + 1, AIPL_2_BYTE_OFFSETS_U8, pred);

    dst->b = vshlq_n(lower, 3);
    dst->b = vorrq(dst->b, vshrq(dst->b, 5));
    dst->g = vorrq(vshlq_n(upper, 6), vshrq(vandq(lower, vdupq_n_u8(0xe0)), 2));
    dst->g = vorrq(dst->g, vshrq(dst->g, 5));
    dst->r = vandq(vshlq_n(upper, 1), vdupq_n_u8(0xf0));
    dst->r = vorrq(dst->r, vshrq(dst->r, 5));
    dst->a = vmulq_u8(vshrq(upper, 7), vdupq_n_u8(0xff));
}

/**
 * Load 16 ARGB1555 pixels from memory to
 * A, R, G and B Helium vector registers without vector predicate
 *
 * @param dst   destination pixel vectors
 * @param src   source pixel vectors
 */
INLINE void aipl_mve_ldr_16px_argb1555_uncut(aipl_mve_argb_x16_t* dst,
                                             const uint16_t* src)
{
    uint8x16x2_t argb = vld2q((const uint8_t*)src);

    dst->b = vshlq_n(argb.val[0], 3);
    dst->b = vorrq(dst->b, vshrq(dst->b, 5));
    dst->g = vorrq(vshlq_n(argb.val[1], 6), vshrq(vandq(argb.val[0], vdupq_n_u8(0xe0)), 2));
    dst->g = vorrq(dst->g, vshrq(dst->g, 5));
    dst->r = vandq(vshlq_n(argb.val[1], 1), vdupq_n_u8(0xf0));
    dst->r = vorrq(dst->r, vshrq(dst->r, 5));
    dst->a = vmulq_u8(vshrq(argb.val[1], 7), vdupq_n_u8(0xff));
}

/**
 * Load 16 XRGB1555 pixels from memory to
 * R, G and B Helium vector registers without vector predicate
 *
 * @param dst   destination pixel vectors
 * @param src   source pixel vectors
 */
INLINE void aipl_mve_ldr_16px_xrgb1555_uncut(aipl_mve_rgb_x16_t* dst,
                                             const uint16_t* src)
{
    uint8x16x2_t argb = vld2q((const uint8_t*)src);

    dst->b = vshlq_n(argb.val[0], 3);
    dst->b = vorrq(dst->b, vshrq(dst->b, 5));
    dst->g = vorrq(vshlq_n(argb.val[1], 6), vshrq(vandq(argb.val[0], vdupq_n_u8(0xe0)), 2));
    dst->g = vorrq(dst->g, vshrq(dst->g, 5));
    dst->r = vandq(vshlq_n(argb.val[1], 1), vdupq_n_u8(0xf0));
    dst->r = vorrq(dst->r, vshrq(dst->r, 5));
}


/**
 * Convert 8 ARGB1555 pixels to Y channel
 * using Helium vector register
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pixel vector
 */
INLINE void aipl_mve_cnvt_8px_argb1555_to_yuv_y(uint16x8_t* dst,
                                                uint16x8_t src)
{
    uint16x8_t r = vandq(vshrq(src, 10), vdupq_n_u16(0x001f));
    uint16x8_t g = vandq(vshrq(src, 5), vdupq_n_u16(0x001f));
    uint16x8_t b = vandq(src, vdupq_n_u16(0x001f));

    *dst = vmulq_n_u16(r, 543);
    *dst = vmlaq_n_u16(*dst, g, 1061);
    *dst = vmlaq_n_u16(*dst, b, 205);
    *dst = vshrq(vaddq(*dst, 128), 8);
    *dst = vaddq(*dst, 16);
}

/**
 * Convert 8 ARGB1555 pixels to U and V channels
 * using Helium vector register
 *
 * @param u_dst U channel destination vector pointer
 * @param v_dst V channel destination vector pointer
 * @param src   source pixel vector
 */
INLINE void aipl_mve_cnvt_8px_argb1555_to_yuv_uv(uint16x8_t* u_dst,
                                                 uint16x8_t* v_dst,
                                                 uint16x8_t src)
{
    uint16x8_t r = vandq(vshrq(src, 10), vdupq_n_u16(0x001f));
    uint16x8_t g = vandq(vshrq(src, 5), vdupq_n_u16(0x001f));
    uint16x8_t b = vandq(src, vdupq_n_u16(0x001f));

    *u_dst = vmulq_n_u16(r, -312);
    *u_dst = vmlaq_n_u16(*u_dst, g, -608);
    *u_dst = vmlaq_n_u16(*u_dst, b, 920);
    *u_dst = vshrq(vaddq(*u_dst, 128), 8);
    *u_dst = vaddq(*u_dst, 128);

    *v_dst = vmulq_n_u16(r, 920);
    *v_dst = vmlaq_n_u16(*v_dst, g, -773);
    *v_dst = vmlaq_n_u16(*v_dst, b, -147);
    *v_dst = vshrq(vaddq(*v_dst, 128), 8);
    *v_dst = vaddq(*v_dst, 128);
}

/**
 * Convert 8 ARGB1555 pixels to Y, U and V channels
 * using Helium vector register
 *
 * @param y_dst Y channel destination vector pointer
 * @param u_dst U channel destination vector pointer
 * @param v_dst V channel destination vector pointer
 * @param src   source pixel vector
 */
INLINE void aipl_mve_cnvt_8px_argb1555_to_yuv(uint16x8_t* y_dst,
                                              uint16x8_t* u_dst,
                                              uint16x8_t* v_dst,
                                              uint16x8_t src)
{
    uint16x8_t r = vandq(vshrq(src, 10), vdupq_n_u16(0x001f));
    uint16x8_t g = vandq(vshrq(src, 5), vdupq_n_u16(0x001f));
    uint16x8_t b = vandq(src, vdupq_n_u16(0x001f));

    *y_dst = vmulq_n_u16(r, 543);
    *y_dst = vmlaq_n_u16(*y_dst, g, 1061);
    *y_dst = vmlaq_n_u16(*y_dst, b, 205);
    *y_dst = vshrq(vaddq(*y_dst, 128), 8);
    *y_dst = vaddq(*y_dst, 16);

    *u_dst = vmulq_n_u16(r, -312);
    *u_dst = vmlaq_n_u16(*u_dst, g, -608);
    *u_dst = vmlaq_n_u16(*u_dst, b, 920);
    *u_dst = vshrq(vaddq(*u_dst, 128), 8);
    *u_dst = vaddq(*u_dst, 128);

    *v_dst = vmulq_n_u16(r, 920);
    *v_dst = vmlaq_n_u16(*v_dst, g, -773);
    *v_dst = vmlaq_n_u16(*v_dst, b, -147);
    *v_dst = vshrq(vaddq(*v_dst, 128), 8);
    *v_dst = vaddq(*v_dst, 128);
}

/**
 * Store 8 ARGB1555 pixels to memory from
 * Helium vector register
 *
 * @param dst   destintation pointer
 * @param src   source pixel vectors
 * @param pred  vector predicate
 */
INLINE void aipl_mve_str_8px_argb1555(uint16_t* dst,
                                      uint16x8_t src,
                                      mve_pred16_t pred)
{
    vstrhq_p(dst, src, pred);
}

/**
 * Store 16 XRGB1555 pixels to memory from
 * R, G and B Helium vector registers with vector predicate
 *
 * @param dst   destintation pointer
 * @param src   source pixel vectors
 * @param pred  vector predicate
 */
INLINE void aipl_mve_str_16px_xrgb1555(uint16_t* dst,
                                       aipl_mve_rgb_x16_t src,
                                       mve_pred16_t pred)
{
    uint8x16_t upper = vsriq(vdupq_n_u8(0x80), src.r, 1);
    upper = vsriq(upper, src.g, 6);
    uint8x16_t lower = vshlq_n(src.g, 2);
    lower = vsriq(lower, src.b, 3);

    vstrbq_scatter_offset_p((uint8_t*)dst, AIPL_2_BYTE_OFFSETS_U8, lower, pred);
    vstrbq_scatter_offset_p((uint8_t*)dst + 1, AIPL_2_BYTE_OFFSETS_U8, upper, pred);
}

/**
 * Store 16 ARGB1555 pixels to memory from
 * A, R, G and B Helium vector registers with vector predicate
 *
 * @param dst   destintation pointer
 * @param src   source pixel vectors
 * @param pred  vector predicate
 */
INLINE void aipl_mve_str_16px_argb1555(uint16_t* dst,
                                       aipl_mve_argb_x16_t src,
                                       mve_pred16_t pred)
{
    uint8x16_t upper = vsriq(src.a, src.r, 1);
    upper = vsriq(upper, src.g, 6);
    uint8x16_t lower = vshlq_n(src.g, 2);
    lower = vsriq(lower, src.b, 3);

    vstrbq_scatter_offset_p((uint8_t*)dst, AIPL_2_BYTE_OFFSETS_U8, lower, pred);
    vstrbq_scatter_offset_p((uint8_t*)dst + 1, AIPL_2_BYTE_OFFSETS_U8, upper, pred);
}

/**
 * Store 16 XRGB1555 pixels to memory from
 * R, G and B Helium vector registers without vector predicate
 *
 * @param dst   destintation pointer
 * @param src   source pixel vectors
 */
INLINE void aipl_mve_str_16px_xrgb1555_uncut(uint16_t* dst,
                                             aipl_mve_rgb_x16_t src)
{
    uint8x16x2_t argb = { vshlq_n(src.g, 2), vsriq(vdupq_n_u8(0x80), src.r, 1) };
    argb.val[0] = vsriq(argb.val[0], src.b, 3);
    argb.val[1] = vsriq(argb.val[1], src.g, 6);

    vst2q((uint8_t*)dst, argb);
}

/**
 * Store 16 ARGB1555 pixels to memory from
 * A, R, G and B Helium vector registers without vector predicate
 *
 * @param dst   destintation pointer
 * @param src   source pixel vectors
 */
INLINE void aipl_mve_str_16px_argb1555_uncut(uint16_t* dst,
                                             aipl_mve_argb_x16_t src)
{
    uint8x16x2_t argb = { vshlq_n(src.g, 2), vsriq(src.a, src.r, 1) };
    argb.val[0] = vsriq(argb.val[0], src.b, 3);
    argb.val[1] = vsriq(argb.val[1], src.g, 6);

    vst2q((uint8_t*)dst, argb);
}


/**
 * Load 4 RGBA8888 pixels from memory to
 * Helium vector register
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pointer
 * @param pred  vector predicate
 */
INLINE void aipl_mve_ldr_4px_rgba8888(uint32x4_t* dst,
                                      const uint32_t* src,
                                      mve_pred16_t pred)
{
    *dst = vld1q_z(src, pred);
}

/**
 * Load 8 RGBA8888 pixels from memory to
 * A, R, G and B Helium vector registers
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pointer
 * @param pred  vector predicate
 */
INLINE void aipl_mve_ldr_8px_rgba8888(aipl_mve_argb_x8_t* dst,
                                      const uint8_t* src,
                                      mve_pred16_t pred)
{
    dst->a = vldrbq_gather_offset_z(src, AIPL_4_BYTE_OFFSETS_U16, pred);
    dst->b = vldrbq_gather_offset_z(src + 1, AIPL_4_BYTE_OFFSETS_U16, pred);
    dst->g = vldrbq_gather_offset_z(src + 2, AIPL_4_BYTE_OFFSETS_U16, pred);
    dst->r = vldrbq_gather_offset_z(src + 3, AIPL_4_BYTE_OFFSETS_U16, pred);
}

/**
 * Load 8 RGBX8888 pixels from memory to
 * R, G and B Helium vector registers
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pointer
 * @param pred  vector predicate
 */
INLINE void aipl_mve_ldr_8px_rgbx8888(aipl_mve_rgb_x8_t* dst,
                                      const uint8_t* src,
                                      mve_pred16_t pred)
{
    dst->b = vldrbq_gather_offset_z(src + 1, AIPL_4_BYTE_OFFSETS_U16, pred);
    dst->g = vldrbq_gather_offset_z(src + 2, AIPL_4_BYTE_OFFSETS_U16, pred);
    dst->r = vldrbq_gather_offset_z(src + 3, AIPL_4_BYTE_OFFSETS_U16, pred);
}

/**
 * Load 8 RGBX8888 pixels from memory with offset to
 * R, G and B Helium vector registers
 *
 * @param dst       destination pixel vector pointer
 * @param src       source pointer
 * @param offset    pixel offset
 * @param pred      vector predicate
 */
INLINE void aipl_mve_ldr_8px_offset_rgbx8888(aipl_mve_rgb_x8_t* dst,
                                             const uint8_t* src,
                                             uint8_t offset,
                                             mve_pred16_t pred)
{
    dst->b = vldrbq_gather_offset_z(src + 1, AIPL_OFFSETS_U16(offset, 4), pred);
    dst->g = vldrbq_gather_offset_z(src + 2, AIPL_OFFSETS_U16(offset, 4), pred);
    dst->r = vldrbq_gather_offset_z(src + 3, AIPL_OFFSETS_U16(offset, 4), pred);
}

/**
 * Load 16 RGBX8888 pixels from memory to
 * R, G and B Helium vector registers
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pointer
 * @param pred  vector predicate
 */
INLINE void aipl_mve_ldr_16px_rgbx8888(aipl_mve_rgb_x16_t* dst,
                                       const uint8_t* src,
                                       mve_pred16_t pred)
{
    dst->b = vldrbq_gather_offset_z(src + 1, AIPL_4_BYTE_OFFSETS_U8, pred);
    dst->g = vldrbq_gather_offset_z(src + 2, AIPL_4_BYTE_OFFSETS_U8, pred);
    dst->r = vldrbq_gather_offset_z(src + 3, AIPL_4_BYTE_OFFSETS_U8, pred);
}

/**
 * Load 16 RGBA8888 pixels from memory to
 * R, G and B Helium vector registers
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pointer
 * @param pred  vector predicate
 */
INLINE void aipl_mve_ldr_16px_rgba8888(aipl_mve_argb_x16_t* dst,
                                       const uint8_t* src,
                                       mve_pred16_t pred)
{
    dst->a = vldrbq_gather_offset_z(src, AIPL_4_BYTE_OFFSETS_U8, pred);
    dst->b = vldrbq_gather_offset_z(src + 1, AIPL_4_BYTE_OFFSETS_U8, pred);
    dst->g = vldrbq_gather_offset_z(src + 2, AIPL_4_BYTE_OFFSETS_U8, pred);
    dst->r = vldrbq_gather_offset_z(src + 3, AIPL_4_BYTE_OFFSETS_U8, pred);
}

/**
 * Load 16 RGBA8888 pixels from memory to
 * A, R, G and B Helium vector registers without vector predicate
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pointer
 */
INLINE void aipl_mve_ldr_16px_rgba8888_uncut(aipl_mve_argb_x16_t* dst,
                                             const uint8_t* src)
{
    uint8x16x4_t rgba = vld4q(src);

    dst->a = rgba.val[0];
    dst->b = rgba.val[1];
    dst->g = rgba.val[2];
    dst->r = rgba.val[3];
}

/**
 * Load 16 RGBX8888 pixels from memory to
 * R, G and B Helium vector registers without vector predicate
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pointer
 */
INLINE void aipl_mve_ldr_16px_rgbx8888_uncut(aipl_mve_rgb_x16_t* dst,
                                             const uint8_t* src)
{
    uint8x16x4_t rgba = vld4q(src);

    dst->b = rgba.val[1];
    dst->g = rgba.val[2];
    dst->r = rgba.val[3];
}

/**
 * Convert 4 RGBA8888 pixels to ARGB8888
 * using Helium vector register
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pixel vector
 */
INLINE void aipl_mve_cnvt_4px_rgba8888_to_argb8888(uint32x4_t* dst,
                                                   uint32x4_t src)
{
    uint32x4_t a = vshlq_n(src, 24);
    *dst = vsriq(a, *dst, 8);
}

/**
 * Convert 8 RGBX8888 pixels to Y channel
 * using Helium vector register
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pixel vector
 */
INLINE void aipl_mve_cnvt_8px_rgbx8888_to_yuv_y(uint16x8_t* dst,
                                                aipl_mve_rgb_x8_t src)
{
    aipl_mve_cnvt_8px_xrgb8888_to_yuv_y(dst, src);
}

/**
 * Convert 8 RGBX8888 pixels to U channel
 * using Helium vector register
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pixel vector
 */
INLINE void aipl_mve_cnvt_8px_rgbx8888_to_yuv_u(uint16x8_t* dst,
                                                aipl_mve_rgb_x8_t src)
{
    aipl_mve_cnvt_8px_xrgb8888_to_yuv_u(dst, src);
}

/**
 * Convert 8 RGBX8888 pixels to V channel
 * using Helium vector register
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pixel vector
 */
INLINE void aipl_mve_cnvt_8px_rgbx8888_to_yuv_v(uint16x8_t* dst,
                                                aipl_mve_rgb_x8_t src)
{
    aipl_mve_cnvt_8px_xrgb8888_to_yuv_v(dst, src);
}

/**
 * Store 4 RGBA8888 pixels to memory from
 * Helium vector register
 *
 * @param dst   destintation pointer
 * @param src   source pixel vectors
 * @param pred  vector predicate
 */
INLINE void aipl_mve_str_4px_rgba8888(uint32_t* dst,
                                      uint32x4_t src,
                                      mve_pred16_t pred)
{
    vst1q_p(dst, src, pred);
}

/**
 * Store 8 RGBA8888 pixels to memory from
 * Helium vector register
 *
 * @param dst   destintation pointer
 * @param src   source pixel vectors
 * @param pred  vector predicate
 */
INLINE void aipl_mve_str_8px_rgba8888(uint8_t* dst,
                                      aipl_mve_argb_x8_t src,
                                      mve_pred16_t pred)
{
    vstrbq_scatter_offset_p(dst, AIPL_4_BYTE_OFFSETS_U16, src.a, pred);
    vstrbq_scatter_offset_p(dst + 1, AIPL_4_BYTE_OFFSETS_U16, src.b, pred);
    vstrbq_scatter_offset_p(dst + 2, AIPL_4_BYTE_OFFSETS_U16, src.g, pred);
    vstrbq_scatter_offset_p(dst + 3, AIPL_4_BYTE_OFFSETS_U16, src.r, pred);
}

/**
 * Store 16 RGBX8888 pixels to memory from
 * Helium vector register
 *
 * @param dst   destintation pointer
 * @param src   source pixel vectors
 * @param pred  vector predicate
 */
INLINE void aipl_mve_str_16px_rgbx8888(uint8_t* dst,
                                       aipl_mve_rgb_x16_t src,
                                       mve_pred16_t pred)
{
    vstrbq_scatter_offset_p(dst, AIPL_4_BYTE_OFFSETS_U8, vdupq_n_u8(0xff), pred);
    vstrbq_scatter_offset_p(dst + 1, AIPL_4_BYTE_OFFSETS_U8, src.b, pred);
    vstrbq_scatter_offset_p(dst + 2, AIPL_4_BYTE_OFFSETS_U8, src.g, pred);
    vstrbq_scatter_offset_p(dst + 3, AIPL_4_BYTE_OFFSETS_U8, src.r, pred);
}

/**
 * Store 16 RGBA8888 pixels to memory from
 * Helium vector register with vector predicate
 *
 * @param dst   destintation pointer
 * @param src   source pixel vectors
 * @param pred  vector predicate
 */
INLINE void aipl_mve_str_16px_rgba8888(uint8_t* dst,
                                       aipl_mve_argb_x16_t src,
                                       mve_pred16_t pred)
{
    vstrbq_scatter_offset_p(dst, AIPL_4_BYTE_OFFSETS_U8, src.a, pred);
    vstrbq_scatter_offset_p(dst + 1, AIPL_4_BYTE_OFFSETS_U8, src.b, pred);
    vstrbq_scatter_offset_p(dst + 2, AIPL_4_BYTE_OFFSETS_U8, src.g, pred);
    vstrbq_scatter_offset_p(dst + 3, AIPL_4_BYTE_OFFSETS_U8, src.r, pred);
}

/**
 * Store 16 RGBA8888 pixels to memory from
 * Helium vector register without vector predicate
 *
 * @param dst   destintation pointer
 * @param src   source pixel vectors
 */
INLINE void aipl_mve_str_16px_rgba8888_uncut(uint8_t* dst,
                                             aipl_mve_argb_x16_t src)
{
    uint8x16x4_t rgba = { { src.a, src.b, src.g, src.r } };

    vst4q(dst, rgba);
}

/**
 * Store 16 RGBX8888 pixels to memory from
 * Helium vector register without vector predicate
 *
 * @param dst   destintation pointer
 * @param src   source pixel vectors
 */
INLINE void aipl_mve_str_16px_rgbx8888_uncut(uint8_t* dst,
                                             aipl_mve_rgb_x16_t src)
{
    uint8x16x4_t rgba = { { vdupq_n_u8(0xff), src.b, src.g, src.r } };

    vst4q(dst, rgba);
}

/**
 * Load 8 RGBA4444 pixels from memory with offset to
 * Helium vector register
 *
 * @param dst       destination pixel vectors
 * @param src       source pixel vectors
 * @param offset    pixel offset
 * @param pred      vector predicate
 */
INLINE void aipl_mve_ldr_8px_offset_rgba4444(uint16x8_t* dst,
                                             const uint16_t* src,
                                             uint8_t offset,
                                             mve_pred16_t pred)
{
    *dst = vldrhq_gather_offset_z(src, AIPL_OFFSETS_U16(offset, 2), pred);
}

/**
 * Load 8 RGBA4444 pixels from memory to
 * Helium vector register
 *
 * @param dst   destination pixel vectors
 * @param src   source pixel vectors
 * @param pred  vector predicate
 */
INLINE void aipl_mve_ldr_8px_rgba4444(uint16x8_t* dst,
                                      const uint16_t* src,
                                      mve_pred16_t pred)
{
    *dst = vldrhq_z_u16(src, pred);
}

/**
 * Load 8 RGBA4444 pixels from memory with offset to
 * A, R, G and B Helium vector registers
 *
 * @param dst       destination pixel vectors
 * @param src       source pixel vectors
 * @param pred      vector predicate
 */
INLINE void aipl_mve_ldr_8px_extend_rgba4444(aipl_mve_argb_x8_t* dst,
                                             const uint16_t* src,
                                             mve_pred16_t pred)
{
    uint16x8_t ba = vldrbq_gather_offset_z((uint8_t*)src, AIPL_2_BYTE_OFFSETS_U16, pred);
    uint16x8_t rg = vldrbq_gather_offset_z((uint8_t*)src + 1, AIPL_2_BYTE_OFFSETS_U16, pred);

    dst->b = vorrq(vandq(ba, vdupq_n_u16(0x00f0)), vshrq(ba, 4));
    dst->g = vmulq_u16(vandq(rg, vdupq_n_u16(0x000f)), vdupq_n_u16(0x0011));
    dst->r = vorrq(vandq(rg, vdupq_n_u16(0x00f0)), vshrq(rg, 4));
    dst->a = vmulq_u16(vandq(ba, vdupq_n_u16(0x000f)), vdupq_n_u16(0x0011));
}

/**
 * Load 16 RGBX4444 pixels to memory from
 * Helium vector register with vector predicate
 *
 * @param dst   destination pixel vectors
 * @param src   source pixel vectors
 * @param pred  vector predicate
 */
INLINE void aipl_mve_ldr_16px_rgbx4444(aipl_mve_rgb_x16_t* dst,
                                       const uint16_t* src,
                                       mve_pred16_t pred)
{
    uint8x16_t ba = vldrbq_gather_offset_z((uint8_t*)src, AIPL_2_BYTE_OFFSETS_U8, pred);
    uint8x16_t rg = vldrbq_gather_offset_z((uint8_t*)src + 1, AIPL_2_BYTE_OFFSETS_U8, pred);

    dst->r = vorrq(vandq(rg, vdupq_n_u8(0xf0)), vshrq(rg, 4));
    dst->g = vmulq_u8(vandq(rg, vdupq_n_u8(0x0f)), vdupq_n_u8(0x11));
    dst->b = vorrq(vandq(ba, vdupq_n_u8(0xf0)), vshrq(ba, 4));
}

/**
 * Load 16 RGBA4444 pixels to memory from
 * Helium vector register with vector predicate
 *
 * @param dst   destination pixel vectors
 * @param src   source pixel vectors
 * @param pred  vector predicate
 */
INLINE void aipl_mve_ldr_16px_rgba4444(aipl_mve_argb_x16_t* dst,
                                       const uint16_t* src,
                                       mve_pred16_t pred)
{
    uint8x16_t ba = vldrbq_gather_offset_z((uint8_t*)src, AIPL_2_BYTE_OFFSETS_U8, pred);
    uint8x16_t rg = vldrbq_gather_offset_z((uint8_t*)src + 1, AIPL_2_BYTE_OFFSETS_U8, pred);

    dst->b = vorrq(vandq(ba, vdupq_n_u8(0xf0)), vshrq(ba, 4));
    dst->g = vmulq_u8(vandq(rg, vdupq_n_u8(0x0f)), vdupq_n_u8(0x11));
    dst->r = vorrq(vandq(rg, vdupq_n_u8(0xf0)), vshrq(rg, 4));
    dst->a = vmulq_u8(vandq(ba, vdupq_n_u8(0x0f)), vdupq_n_u8(0x11));
}

/**
 * Load 16 RGBX4444 pixels to memory from
 * Helium vector register without vector predicate
 *
 * @param dst   destination pixel vectors
 * @param src   source pixel vectors
 */
INLINE void aipl_mve_ldr_16px_rgbx4444_uncut(aipl_mve_rgb_x16_t* dst,
                                             const uint16_t* src)
{
    uint8x16x2_t rgba = vld2q((const uint8_t*)src);

    dst->r = vorrq(vandq(rgba.val[1], vdupq_n_u8(0xf0)), vshrq(rgba.val[1], 4));
    dst->g = vmulq_u8(vandq(rgba.val[1], vdupq_n_u8(0x0f)), vdupq_n_u8(0x11));
    dst->b = vorrq(vandq(rgba.val[0], vdupq_n_u8(0xf0)), vshrq(rgba.val[0], 4));
}

/**
 * Load 16 RGBA4444 pixels to memory from
 * Helium vector register without vector predicate
 *
 * @param dst   destination pixel vectors
 * @param src   source pixel vectors
 */
INLINE void aipl_mve_ldr_16px_rgba4444_uncut(aipl_mve_argb_x16_t* dst,
                                             const uint16_t* src)
{
    uint8x16x2_t rgba = vld2q((const uint8_t*)src);

    dst->r = vorrq(vandq(rgba.val[1], vdupq_n_u8(0xf0)), vshrq(rgba.val[1], 4));
    dst->g = vmulq_u8(vandq(rgba.val[1], vdupq_n_u8(0x0f)), vdupq_n_u8(0x11));
    dst->b = vorrq(vandq(rgba.val[0], vdupq_n_u8(0xf0)), vshrq(rgba.val[0], 4));
    dst->a = vmulq_u8(vandq(rgba.val[0], vdupq_n_u8(0x0f)), vdupq_n_u8(0x11));
}

/**
 * Convert 8 RGBA4444 pixels to ARGB4444
 * using Helium vector register
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pixel vector
 */
INLINE void aipl_mve_cnvt_8px_rgba4444_to_argb4444(uint16x8_t* dst,
                                                   uint16x8_t src)
{
    *dst = vshlq_n(src, 12);
    *dst = vsriq(*dst, src, 4);
}

/**
 * Convert 8 RGBA4444 pixels to Y channel
 * using Helium vector register
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pixel vector
 */
INLINE void aipl_mve_cnvt_8px_rgba4444_to_yuv_y(uint16x8_t* dst,
                                                uint16x8_t src)
{
    uint16x8_t r = vandq(vshrq(src, 12), vdupq_n_u16(0x000f));
    uint16x8_t g = vandq(vshrq(src, 8), vdupq_n_u16(0x000f));
    uint16x8_t b = vandq(vshrq(src, 4), vdupq_n_u16(0x000f));

    *dst = vmulq_n_u16(r, 66 * 0x11);
    *dst = vmlaq_n_u16(*dst, g, 129 * 0x11);
    *dst = vmlaq_n_u16(*dst, b, 25 * 0x11);
    *dst = vshrq(vaddq(*dst, 128), 8);
    *dst = vaddq(*dst, 16);
}

/**
 * Convert 8 RGBA4444 pixels to U and V channels
 * using Helium vector register
 *
 * @param u_dst U channel destination vector pointer
 * @param v_dst V channel destination vector pointer
 * @param src   source pixel vector
 */
INLINE void aipl_mve_cnvt_8px_rgba4444_to_yuv_uv(uint16x8_t* u_dst,
                                                 uint16x8_t* v_dst,
                                                 uint16x8_t src)
{
    uint16x8_t r = vandq(vshrq(src, 12), vdupq_n_u16(0x000f));
    uint16x8_t g = vandq(vshrq(src, 8), vdupq_n_u16(0x000f));
    uint16x8_t b = vandq(vshrq(src, 4), vdupq_n_u16(0x000f));

    *u_dst = vmulq_n_u16(r, -38 * 0x11);
    *u_dst = vmlaq_n_u16(*u_dst, g, -74 * 0x11);
    *u_dst = vmlaq_n_u16(*u_dst, b, 112 * 0x11);
    *u_dst = vshrq(vaddq(*u_dst, 128), 8);
    *u_dst = vaddq(*u_dst, 128);

    *v_dst = vmulq_n_u16(r, 112 * 0x11);
    *v_dst = vmlaq_n_u16(*v_dst, g, -94 * 0x11);
    *v_dst = vmlaq_n_u16(*v_dst, b, -18 * 0x11);
    *v_dst = vshrq(vaddq(*v_dst, 128), 8);
    *v_dst = vaddq(*v_dst, 128);
}

/**
 * Convert 8 RGBA4444 pixels to Y, U and V channels
 * using Helium vector register
 *
 * @param y_dst Y channel destination vector pointer
 * @param u_dst U channel destination vector pointer
 * @param v_dst V channel destination vector pointer
 * @param src   source pixel vector
 */
INLINE void aipl_mve_cnvt_8px_rgba4444_to_yuv(uint16x8_t* y_dst,
                                              uint16x8_t* u_dst,
                                              uint16x8_t* v_dst,
                                              uint16x8_t src)
{
    uint16x8_t r = vandq(vshrq(src, 12), vdupq_n_u16(0x000f));
    uint16x8_t g = vandq(vshrq(src, 8), vdupq_n_u16(0x000f));
    uint16x8_t b = vandq(vshrq(src, 4), vdupq_n_u16(0x000f));

    *y_dst = vmulq_n_u16(r, 66 * 0x11);
    *y_dst = vmlaq_n_u16(*y_dst, g, 129 * 0x11);
    *y_dst = vmlaq_n_u16(*y_dst, b, 25 * 0x11);
    *y_dst = vshrq(vaddq(*y_dst, 128), 8);
    *y_dst = vaddq(*y_dst, 16);

    *u_dst = vmulq_n_u16(r, -38 * 0x11);
    *u_dst = vmlaq_n_u16(*u_dst, g, -74 * 0x11);
    *u_dst = vmlaq_n_u16(*u_dst, b, 112 * 0x11);
    *u_dst = vshrq(vaddq(*u_dst, 128), 8);
    *u_dst = vaddq(*u_dst, 128);

    *v_dst = vmulq_n_u16(r, 112 * 0x11);
    *v_dst = vmlaq_n_u16(*v_dst, g, -94 * 0x11);
    *v_dst = vmlaq_n_u16(*v_dst, b, -18 * 0x11);
    *v_dst = vshrq(vaddq(*v_dst, 128), 8);
    *v_dst = vaddq(*v_dst, 128);
}

/**
 * Store 8 RGBA4444 pixels to memory from
 * Helium vector register
 *
 * @param dst   destintation pointer
 * @param src   source pixel vectors
 * @param pred  vector predicate
 */
INLINE void aipl_mve_str_8px_rgba4444(uint16_t* dst,
                                      uint16x8_t src,
                                      mve_pred16_t pred)
{
    vstrhq_p(dst, src, pred);
}

/**
 * Store 8 RGBA4444 pixels to memory from
 * A, R, G and B Helium vector registers
 *
 * @param dst   destintation pointer
 * @param src   source pixel vectors
 * @param pred  vector predicate
 */
INLINE void aipl_mve_str_8px_extended_rgba4444(uint16_t* dst,
                                               aipl_mve_argb_x8_t src,
                                               mve_pred16_t pred)
{
    uint8x16_t rg = vreinterpretq_u8(src.r);
    rg = vsriq(rg, vreinterpretq_u8(src.g), 4);
    uint8x16_t ba = vreinterpretq_u8(src.b);
    ba = vsriq(ba, vreinterpretq_u8(src.a), 4);
    uint16x8_t pix = vreinterpretq_u16(vmovntq(ba, vreinterpretq_u16(rg)));

    vst1q_p(dst, pix, pred);
}

/**
 * Store 16 RGBX4444 pixels to memory from
 * Helium vector register with vector predicate
 *
 * @param dst   destintation pointer
 * @param src   source pixel vectors
 * @param pred  vector predicate
 */
INLINE void aipl_mve_str_16px_rgbx4444(uint16_t* dst,
                                       aipl_mve_rgb_x16_t src,
                                       mve_pred16_t pred)
{
    uint8x16_t rg = vsriq(src.r, src.g, 4);
    uint8x16_t ba = vsriq(src.b, vdupq_n_u8(0xff), 4);

    vstrbq_scatter_offset_p((uint8_t*)dst, AIPL_2_BYTE_OFFSETS_U8, ba, pred);
    vstrbq_scatter_offset_p((uint8_t*)dst + 1, AIPL_2_BYTE_OFFSETS_U8, rg, pred);
}

/**
 * Store 16 RGBA4444 pixels to memory from
 * Helium vector register with vector predicate
 *
 * @param dst   destintation pointer
 * @param src   source pixel vectors
 * @param pred  vector predicate
 */
INLINE void aipl_mve_str_16px_rgba4444(uint16_t* dst,
                                       aipl_mve_argb_x16_t src,
                                       mve_pred16_t pred)
{
    uint8x16_t rg = vsriq(src.r, src.g, 4);
    uint8x16_t ba = vsriq(src.b, src.a, 4);

    vstrbq_scatter_offset_p((uint8_t*)dst, AIPL_2_BYTE_OFFSETS_U8, ba, pred);
    vstrbq_scatter_offset_p((uint8_t*)dst + 1, AIPL_2_BYTE_OFFSETS_U8, rg, pred);
}

/**
 * Store 16 RGBX4444 pixels to memory from
 * Helium vector register without vector predicate
 *
 * @param dst   destintation pointer
 * @param src   source pixel vectors
 */
INLINE void aipl_mve_str_16px_rgbx4444_uncut(uint16_t* dst,
                                             aipl_mve_rgb_x16_t src)
{
    uint8x16x2_t rgba = { vsriq(src.b, vdupq_n_u8(0xff), 4), vsriq(src.r, src.g, 4) };

    vst2q((uint8_t*)dst, rgba);
}

/**
 * Store 16 RGBA4444 pixels to memory from
 * Helium vector register with vector predicate
 *
 * @param dst   destintation pointer
 * @param src   source pixel vectors
 */
INLINE void aipl_mve_str_16px_rgba4444_uncut(uint16_t* dst,
                                             aipl_mve_argb_x16_t src)
{
    uint8x16x2_t rgba = { vsriq(src.b, src.a, 4), vsriq(src.r, src.g, 4) };

    vst2q((uint8_t*)dst, rgba);
}

/**
 * Load 8 RGBA5551 pixels from memory with offset to
 * Helium vector register
 *
 * @param dst       destination pixel vectors
 * @param src       source pixel vectors
 * @param offset    pixel offset
 * @param pred      vector predicate
 */
INLINE void aipl_mve_ldr_8px_offset_rgba5551(uint16x8_t* dst,
                                             const uint16_t* src,
                                             uint8_t offset,
                                             mve_pred16_t pred)
{
    *dst = vldrhq_gather_offset_z(src, AIPL_OFFSETS_U16(offset, 2), pred);
}

/**
 * Load 8 RGBA5551 pixels from memory to
 * Helium vector register
 *
 * @param dst   destination pixel vectors
 * @param src   source pixel vectors
 * @param pred  vector predicate
 */
INLINE void aipl_mve_ldr_8px_rgba5551(uint16x8_t* dst,
                                      const uint16_t* src,
                                      mve_pred16_t pred)
{
    *dst = vldrhq_z_u16(src, pred);
}

/**
 * Load 16 RGBX5551 pixels to memory from
 * Helium vector register with vector predicate
 *
 * @param dst   destination pixel vectors
 * @param src   source pixel vectors
 * @param pred  vector predicate
 */
INLINE void aipl_mve_ldr_16px_rgbx5551(aipl_mve_rgb_x16_t* dst,
                                       const uint16_t* src,
                                       mve_pred16_t pred)
{
    uint8x16_t lower = vldrbq_gather_offset_z((uint8_t*)src, AIPL_2_BYTE_OFFSETS_U8, pred);
    uint8x16_t upper = vldrbq_gather_offset_z((uint8_t*)src + 1, AIPL_2_BYTE_OFFSETS_U8, pred);

    dst->r = vandq(upper, vdupq_n_u8(0xf8));
    dst->r = vorrq(dst->r, vshrq(dst->r, 5));
    dst->g = vorrq(vshlq_n(upper, 5), vshrq(vandq(lower, vdupq_n_u8(0xc0)), 3));
    dst->g = vorrq(dst->g, vshrq(dst->g, 5));
    dst->b = vshlq_n(vandq(lower, vdupq_n_u8(0x3e)), 2);
    dst->b = vorrq(dst->b, vshrq(dst->b, 5));
}

/**
 * Load 16 RGBA5551 pixels to memory from
 * Helium vector register with vector predicate
 *
 * @param dst   destination pixel vectors
 * @param src   source pixel vectors
 * @param pred  vector predicate
 */
INLINE void aipl_mve_ldr_16px_rgba5551(aipl_mve_argb_x16_t* dst,
                                       const uint16_t* src,
                                       mve_pred16_t pred)
{
    uint8x16_t lower = vldrbq_gather_offset_z((uint8_t*)src, AIPL_2_BYTE_OFFSETS_U8, pred);
    uint8x16_t upper = vldrbq_gather_offset_z((uint8_t*)src + 1, AIPL_2_BYTE_OFFSETS_U8, pred);

    dst->b = vshlq_n(vandq(lower, vdupq_n_u8(0x3e)), 2);
    dst->b = vorrq(dst->b, vshrq(dst->b, 5));
    dst->g = vorrq(vshlq_n(upper, 5), vshrq(vandq(lower, vdupq_n_u8(0xc0)), 3));
    dst->g = vorrq(dst->g, vshrq(dst->g, 5));
    dst->r = vandq(upper, vdupq_n_u8(0xf8));
    dst->r = vorrq(dst->r, vshrq(dst->r, 5));
    dst->a = vmulq_u8(vandq(lower, vdupq_n_u8(0x01)), vdupq_n_u8(0xff));
}

/**
 * Load 16 RGBX5551 pixels to memory from
 * Helium vector register without vector predicate
 *
 * @param dst   destination pixel vectors
 * @param src   source pixel vectors
 */
INLINE void aipl_mve_ldr_16px_rgbx5551_uncut(aipl_mve_rgb_x16_t* dst,
                                             const uint16_t* src)
{
    uint8x16x2_t rgba = vld2q((const uint8_t*)src);

    dst->r = vandq(rgba.val[1], vdupq_n_u8(0xf8));
    dst->r = vorrq(dst->r, vshrq(dst->r, 5));
    dst->g = vorrq(vshlq_n(rgba.val[1], 5), vshrq(vandq(rgba.val[0], vdupq_n_u8(0xc0)), 3));
    dst->g = vorrq(dst->g, vshrq(dst->g, 5));
    dst->b = vshlq_n(vandq(rgba.val[0], vdupq_n_u8(0x3e)), 2);
    dst->b = vorrq(dst->b, vshrq(dst->b, 5));
}

/**
 * Load 16 RGBA5551 pixels to memory from
 * Helium vector register without vector predicate
 *
 * @param dst   destination pixel vectors
 * @param src   source pixel vectors
 */
INLINE void aipl_mve_ldr_16px_rgba5551_uncut(aipl_mve_argb_x16_t* dst,
                                             const uint16_t* src)
{
    uint8x16x2_t rgba = vld2q((const uint8_t*)src);

    dst->r = vandq(rgba.val[1], vdupq_n_u8(0xf8));
    dst->r = vorrq(dst->r, vshrq(dst->r, 5));
    dst->g = vorrq(vshlq_n(rgba.val[1], 5), vshrq(vandq(rgba.val[0], vdupq_n_u8(0xc0)), 3));
    dst->g = vorrq(dst->g, vshrq(dst->g, 5));
    dst->b = vshlq_n(vandq(rgba.val[0], vdupq_n_u8(0x3e)), 2);
    dst->b = vorrq(dst->b, vshrq(dst->b, 5));
    dst->a = vmulq_u8(vandq(rgba.val[0], vdupq_n_u8(0x01)), vdupq_n_u8(0xff));
}

/**
 * Convert 8 RGBA5551 pixels to Y channel
 * using Helium vector register
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pixel vector
 */
INLINE void aipl_mve_cnvt_8px_rgba5551_to_yuv_y(uint16x8_t* dst,
                                                uint16x8_t src)
{
    uint16x8_t r = vandq(vshrq(src, 11), vdupq_n_u16(0x001f));
    uint16x8_t g = vandq(vshrq(src, 6), vdupq_n_u16(0x001f));
    uint16x8_t b = vandq(vshrq(src, 1), vdupq_n_u16(0x001f));

    *dst = vmulq_n_u16(r, 543);
    *dst = vmlaq_n_u16(*dst, g, 1061);
    *dst = vmlaq_n_u16(*dst, b, 205);
    *dst = vshrq(vaddq(*dst, 128), 8);
    *dst = vaddq(*dst, 16);
}

/**
 * Convert 8 RGBA5551 pixels to U and V channels
 * using Helium vector register
 *
 * @param u_dst U channel destination vector pointer
 * @param v_dst V channel destination vector pointer
 * @param src   source pixel vector
 */
INLINE void aipl_mve_cnvt_8px_rgba5551_to_yuv_uv(uint16x8_t* u_dst,
                                                 uint16x8_t* v_dst,
                                                 uint16x8_t src)
{
    uint16x8_t r = vandq(vshrq(src, 11), vdupq_n_u16(0x001f));
    uint16x8_t g = vandq(vshrq(src, 6), vdupq_n_u16(0x001f));
    uint16x8_t b = vandq(vshrq(src, 1), vdupq_n_u16(0x001f));

    *u_dst = vmulq_n_u16(r, -312);
    *u_dst = vmlaq_n_u16(*u_dst, g, -608);
    *u_dst = vmlaq_n_u16(*u_dst, b, 920);
    *u_dst = vshrq(vaddq(*u_dst, 128), 8);
    *u_dst = vaddq(*u_dst, 128);

    *v_dst = vmulq_n_u16(r, 920);
    *v_dst = vmlaq_n_u16(*v_dst, g, -773);
    *v_dst = vmlaq_n_u16(*v_dst, b, -147);
    *v_dst = vshrq(vaddq(*v_dst, 128), 8);
    *v_dst = vaddq(*v_dst, 128);
}

/**
 * Convert 8 RGBA5551 pixels to Y, U and V channels
 * using Helium vector register
 *
 * @param y_dst Y channel destination vector pointer
 * @param u_dst U channel destination vector pointer
 * @param v_dst V channel destination vector pointer
 * @param src   source pixel vector
 */
INLINE void aipl_mve_cnvt_8px_rgba5551_to_yuv(uint16x8_t* y_dst,
                                              uint16x8_t* u_dst,
                                              uint16x8_t* v_dst,
                                              uint16x8_t src)
{
    uint16x8_t r = vandq(vshrq(src, 11), vdupq_n_u16(0x001f));
    uint16x8_t g = vandq(vshrq(src, 6), vdupq_n_u16(0x001f));
    uint16x8_t b = vandq(vshrq(src, 1), vdupq_n_u16(0x001f));

    *y_dst = vmulq_n_u16(r, 543);
    *y_dst = vmlaq_n_u16(*y_dst, g, 1061);
    *y_dst = vmlaq_n_u16(*y_dst, b, 205);
    *y_dst = vshrq(vaddq(*y_dst, 128), 8);
    *y_dst = vaddq(*y_dst, 16);

    *u_dst = vmulq_n_u16(r, -312);
    *u_dst = vmlaq_n_u16(*u_dst, g, -608);
    *u_dst = vmlaq_n_u16(*u_dst, b, 920);
    *u_dst = vshrq(vaddq(*u_dst, 128), 8);
    *u_dst = vaddq(*u_dst, 128);

    *v_dst = vmulq_n_u16(r, 920);
    *v_dst = vmlaq_n_u16(*v_dst, g, -773);
    *v_dst = vmlaq_n_u16(*v_dst, b, -147);
    *v_dst = vshrq(vaddq(*v_dst, 128), 8);
    *v_dst = vaddq(*v_dst, 128);
}

/**
 * Store 8 RGBA5551 pixels to memory from
 * Helium vector register
 *
 * @param dst   destintation pointer
 * @param src   source pixel vectors
 * @param pred  vector predicate
 */
INLINE void aipl_mve_str_8px_rgba5551(uint16_t* dst,
                                      uint16x8_t src,
                                      mve_pred16_t pred)
{
    vstrhq_p(dst, src, pred);
}

/**
 * Store 16 RGBX5551 pixels to memory from
 * Helium vector register with vector predicate
 *
 * @param dst   destintation pointer
 * @param src   source pixel vectors
 * @param pred  vector predicate
 */
INLINE void aipl_mve_str_16px_rgbx5551(uint16_t* dst,
                                       aipl_mve_rgb_x16_t src,
                                       mve_pred16_t pred)
{
    uint8x16_t upper = vsriq(src.r, src.g, 5);
    uint8x16_t lower = vshlq_n(src.g, 3);
    lower = vsriq(lower, src.b, 2);
    lower = vorrq(lower, vdupq_n_u8(0x01));

    vstrbq_scatter_offset_p((uint8_t*)dst, AIPL_2_BYTE_OFFSETS_U8, lower, pred);
    vstrbq_scatter_offset_p((uint8_t*)dst + 1, AIPL_2_BYTE_OFFSETS_U8, upper, pred);
}

/**
 * Store 16 RGBA5551 pixels to memory from
 * Helium vector register with vector predicate
 *
 * @param dst   destintation pointer
 * @param src   source pixel vectors
 * @param pred  vector predicate
 */
INLINE void aipl_mve_str_16px_rgba5551(uint16_t* dst,
                                       aipl_mve_argb_x16_t src,
                                       mve_pred16_t pred)
{
    uint8x16_t upper = vsriq(src.r, src.g, 5);
    uint8x16_t lower = vshlq_n(src.g, 3);
    lower = vsriq(lower, src.b, 2);
    lower = vsriq(lower, src.a, 7);

    vstrbq_scatter_offset_p((uint8_t*)dst, AIPL_2_BYTE_OFFSETS_U8, lower, pred);
    vstrbq_scatter_offset_p((uint8_t*)dst + 1, AIPL_2_BYTE_OFFSETS_U8, upper, pred);
}

/**
 * Store 16 RGBX5551 pixels to memory from
 * Helium vector register without vector predicate
 *
 * @param dst   destintation pointer
 * @param src   source pixel vectors
 */
INLINE void aipl_mve_str_16px_rgbx5551_uncut(uint16_t* dst,
                                             aipl_mve_rgb_x16_t src)
{
    uint8x16x2_t rgba = { vshlq_n(src.g, 3), vsriq(src.r, src.g, 5) };
    rgba.val[0] = vsriq(rgba.val[0], src.b, 2);
    rgba.val[0] = vorrq(rgba.val[0], vdupq_n_u8(0x01));

    vst2q((uint8_t*)dst, rgba);
}

/**
 * Store 16 RGBA5551 pixels to memory from
 * Helium vector register without vector predicate
 *
 * @param dst   destintation pointer
 * @param src   source pixel vectors
 */
INLINE void aipl_mve_str_16px_rgba5551_uncut(uint16_t* dst,
                                             aipl_mve_argb_x16_t src)
{
    uint8x16x2_t rgba = { vshlq_n(src.g, 3), vsriq(src.r, src.g, 5) };
    rgba.val[0] = vsriq(rgba.val[0], src.b, 2);
    rgba.val[0] = vsriq(rgba.val[0], src.a, 7);

    vst2q((uint8_t*)dst, rgba);
}


/**
 * Load 8 RGB565 pixels from memory with offset to
 * Helium vector register
 *
 * @param dst       destination pixel vectors
 * @param src       source pixel vectors
 * @param offset    pixel offset
 * @param pred      vector predicate
 */
INLINE void aipl_mve_ldr_8px_offset_rgb565(uint16x8_t* dst,
                                           const uint16_t* src,
                                           uint8_t offset,
                                           mve_pred16_t pred)
{
    *dst = vldrhq_gather_offset_z(src, AIPL_OFFSETS_U16(offset, 2), pred);
}

/**
 * Load 8 RGB565 pixels from memory to
 * Helium vector register
 *
 * @param dst   destination pixel vectors
 * @param src   source pixel vectors
 * @param pred  vector predicate
 */
INLINE void aipl_mve_ldr_8px_rgb565(uint16x8_t* dst,
                                    const uint16_t* src,
                                    mve_pred16_t pred)
{
    *dst = vldrhq_z_u16(src, pred);
}

/**
 * Load 16 RGB565 pixels from memory to
 * Helium vector register with vector predicate
 *
 * @param dst   destination pixel vectors
 * @param src   source pixel vectors
 * @param pred  vector predicate
 */
INLINE void aipl_mve_ldr_16px_rgb565(aipl_mve_rgb_x16_t* dst,
                                     const uint16_t* src,
                                     mve_pred16_t pred)
{
    uint8x16_t lower = vldrbq_gather_offset_z((uint8_t*)src, AIPL_2_BYTE_OFFSETS_U8, pred);
    uint8x16_t upper = vldrbq_gather_offset_z((uint8_t*)src + 1, AIPL_2_BYTE_OFFSETS_U8, pred);

    dst->r = vandq(upper, vdupq_n_u8(0xf8));
    dst->r = vorrq(dst->r, vshrq(dst->r, 5));
    dst->g = vorrq(vshlq_n(upper, 5), vshrq(vandq(lower, vdupq_n_u8(0xe0)), 3));
    dst->g = vorrq(dst->g, vshrq(dst->g, 6));
    dst->b = vshlq_n(lower, 3);
    dst->b = vorrq(dst->b, vshrq(dst->b, 5));
}

/**
 * Load 16 RGB565 pixels from memory to
 * Helium vector register without vector predicate
 *
 * @param dst   destination pixel vectors
 * @param src   source pixel vectors
 */
INLINE void aipl_mve_ldr_16px_rgb565_uncut(aipl_mve_rgb_x16_t* dst,
                                          const uint16_t* src)
{
    uint8x16x2_t rgb = vld2q((const uint8_t*)src);

    dst->r = vandq(rgb.val[1], vdupq_n_u8(0xf8));
    dst->r = vorrq(dst->r, vshrq(dst->r, 5));
    dst->g = vorrq(vshlq_n(rgb.val[1], 5), vshrq(vandq(rgb.val[0], vdupq_n_u8(0xe0)), 3));
    dst->g = vorrq(dst->g, vshrq(dst->g, 6));
    dst->b = vshlq_n(rgb.val[0], 3);
    dst->b = vorrq(dst->b, vshrq(dst->b, 5));
}

/**
 * Convert 8 RGB565 pixels to Y channel
 * using Helium vector register
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pixel vector
 */
INLINE void aipl_mve_cnvt_8px_rgb565_to_yuv_y(uint16x8_t* dst,
                                              uint16x8_t src)
{
    uint16x8_t r = vandq(vshrq(src, 11), vdupq_n_u16(0x001f));
    uint16x8_t g = vandq(vshrq(src, 5), vdupq_n_u16(0x003f));
    uint16x8_t b = vandq(src, vdupq_n_u16(0x001f));

    *dst = vmulq_n_u16(r, 543);
    *dst = vmlaq_n_u16(*dst, g, 522);
    *dst = vmlaq_n_u16(*dst, b, 205);
    *dst = vshrq(vaddq(*dst, 128), 8);
    *dst = vaddq(*dst, 16);
}

/**
 * Convert 8 RGB565 pixels to U and V channels
 * using Helium vector register
 *
 * @param u_dst U channel destination vector pointer
 * @param v_dst V channel destination vector pointer
 * @param src   source pixel vector
 */
INLINE void aipl_mve_cnvt_8px_rgb565_to_yuv_uv(uint16x8_t* u_dst,
                                               uint16x8_t* v_dst,
                                               uint16x8_t src)
{
    uint16x8_t r = vandq(vshrq(src, 11), vdupq_n_u16(0x001f));
    uint16x8_t g = vandq(vshrq(src, 5), vdupq_n_u16(0x003f));
    uint16x8_t b = vandq(src, vdupq_n_u16(0x001f));

    *u_dst = vmulq_n_u16(r, -312);
    *u_dst = vmlaq_n_u16(*u_dst, g, -299);
    *u_dst = vmlaq_n_u16(*u_dst, b, 920);
    *u_dst = vshrq(vaddq(*u_dst, 128), 8);
    *u_dst = vaddq(*u_dst, 128);

    *v_dst = vmulq_n_u16(r, 920);
    *v_dst = vmlaq_n_u16(*v_dst, g, -380);
    *v_dst = vmlaq_n_u16(*v_dst, b, -147);
    *v_dst = vshrq(vaddq(*v_dst, 128), 8);
    *v_dst = vaddq(*v_dst, 128);
}

/**
 * Convert 8 RGB565 pixels to Y, U and V channels
 * using Helium vector register
 *
 * @param y_dst Y channel destination vector pointer
 * @param u_dst U channel destination vector pointer
 * @param v_dst V channel destination vector pointer
 * @param src   source pixel vector
 */
INLINE void aipl_mve_cnvt_8px_rgb565_to_yuv(uint16x8_t* y_dst,
                                            uint16x8_t* u_dst,
                                            uint16x8_t* v_dst,
                                            uint16x8_t src)
{
    uint16x8_t r = vandq(vshrq(src, 11), vdupq_n_u16(0x001f));
    uint16x8_t g = vandq(vshrq(src, 5), vdupq_n_u16(0x003f));
    uint16x8_t b = vandq(src, vdupq_n_u16(0x001f));

    *y_dst = vmulq_n_u16(r, 543);
    *y_dst = vmlaq_n_u16(*y_dst, g, 522);
    *y_dst = vmlaq_n_u16(*y_dst, b, 205);
    *y_dst = vshrq(vaddq(*y_dst, 128), 8);
    *y_dst = vaddq(*y_dst, 16);

    *u_dst = vmulq_n_u16(r, -312);
    *u_dst = vmlaq_n_u16(*u_dst, g, -299);
    *u_dst = vmlaq_n_u16(*u_dst, b, 920);
    *u_dst = vshrq(vaddq(*u_dst, 128), 8);
    *u_dst = vaddq(*u_dst, 128);

    *v_dst = vmulq_n_u16(r, 920);
    *v_dst = vmlaq_n_u16(*v_dst, g, -380);
    *v_dst = vmlaq_n_u16(*v_dst, b, -147);
    *v_dst = vshrq(vaddq(*v_dst, 128), 8);
    *v_dst = vaddq(*v_dst, 128);
}

/**
 * Store 8 RGB565 pixels to memory from
 * Helium vector register
 *
 * @param dst   destintation pointer
 * @param src   source pixel vectors
 * @param pred  vector predicate
 */
INLINE void aipl_mve_str_8px_rgb565(uint16_t* dst,
                                    uint16x8_t src,
                                    mve_pred16_t pred)
{
    vstrhq_p(dst, src, pred);
}

/**
 * Store 16 RGB565 pixels to memory from
 * Helium vector register with vector predicate
 *
 * @param dst   destintation pointer
 * @param src   source pixel vectors
 * @param pred  vector predicate
 */
INLINE void aipl_mve_str_16px_rgb565(uint16_t* dst,
                                     aipl_mve_rgb_x16_t src,
                                     mve_pred16_t pred)
{
    uint8x16_t upper = vsriq(src.r, src.g, 5);
    uint8x16_t lower = vshlq_n(src.g, 3);
    lower = vsriq(lower, src.b, 3);

    vstrbq_scatter_offset_p((uint8_t*)dst, AIPL_2_BYTE_OFFSETS_U8, lower, pred);
    vstrbq_scatter_offset_p((uint8_t*)dst + 1, AIPL_2_BYTE_OFFSETS_U8, upper, pred);
}

/**
 * Store 16 RGB565 pixels to memory from
 * Helium vector register without vector predicate
 *
 * @param dst   destintation pointer
 * @param src   source pixel vectors
 */
INLINE void aipl_mve_str_16px_rgb565_uncut(uint16_t* dst,
                                           aipl_mve_rgb_x16_t src)
{
    uint8x16x2_t rgb = { vshlq_n(src.g, 3), vsriq(src.r, src.g, 5) };
    rgb.val[0] = vsriq(rgb.val[0], src.b, 3);

    vst2q((uint8_t*)dst, rgb);
}

/**
 * Load 8 RGB pixels from memory to
 * Helium vector registers
 *
 * @param dst       destintation pointer
 * @param src       source pixel vectors
 * @param pred      vector predicate
 * @param r_offset  red channel offset
 * @param g_offset  green channel offset
 * @param b_offset  blue channel offset
 */
INLINE void aipl_mve_ldr_8px_rgb(aipl_mve_rgb_x8_t* dst,
                                 const uint8_t* src,
                                 mve_pred16_t pred,
                                 uint8_t r_offset,
                                 uint8_t g_offset,
                                 uint8_t b_offset)
{
    dst->r = vldrbq_gather_offset_z(src + r_offset, AIPL_3_BYTE_OFFSETS_U16, pred);
    dst->g = vldrbq_gather_offset_z(src + g_offset, AIPL_3_BYTE_OFFSETS_U16, pred);
    dst->b = vldrbq_gather_offset_z(src + b_offset, AIPL_3_BYTE_OFFSETS_U16, pred);
}

/**
 * Load 8 RGB pixels from memory with offset to
 * Helium vector registers
 *
 * @param dst       destintation pointer
 * @param src       source pixel vectors
 * @param offset    pixels offset
 * @param pred      vector predicate
 * @param r_offset  red channel offset
 * @param g_offset  green channel offset
 * @param b_offset  blue channel offset
 */
INLINE void aipl_mve_ldr_8px_offset_rgb(aipl_mve_rgb_x8_t* dst,
                                        const uint8_t* src,
                                        uint8_t offset,
                                        mve_pred16_t pred,
                                        uint8_t r_offset,
                                        uint8_t g_offset,
                                        uint8_t b_offset)
{
    offset *= 3;
    dst->r = vldrbq_gather_offset_z(src + r_offset, AIPL_OFFSETS_U16(offset, 1), pred);
    dst->g = vldrbq_gather_offset_z(src + g_offset, AIPL_OFFSETS_U16(offset, 1), pred);
    dst->b = vldrbq_gather_offset_z(src + b_offset, AIPL_OFFSETS_U16(offset, 1), pred);
}

/**
 * Load 16 RGB pixels from memory to
 * Helium vector registers with vector predicate
 *
 * @param dst       destintation pointer
 * @param src       source pixel vectors
 * @param pred      vector predicate
 * @param r_offset  red channel offset
 * @param g_offset  green channel offset
 * @param b_offset  blue channel offset
 */
INLINE void aipl_mve_ldr_16px_rgb(aipl_mve_rgb_x16_t* dst,
                                  const uint8_t* src,
                                  mve_pred16_t pred,
                                  uint8_t r_offset,
                                  uint8_t g_offset,
                                  uint8_t b_offset)
{
    dst->r = vldrbq_gather_offset_z(src + r_offset, AIPL_3_BYTE_OFFSETS_U8, pred);
    dst->g = vldrbq_gather_offset_z(src + g_offset, AIPL_3_BYTE_OFFSETS_U8, pred);
    dst->b = vldrbq_gather_offset_z(src + b_offset, AIPL_3_BYTE_OFFSETS_U8, pred);
}

/**
 * Load 16 RGB pixels from memory to
 * Helium vector registers without vector predicate
 *
 * @param dst       destintation pointer
 * @param src       source pixel vectors
 * @param r_offset  red channel offset
 * @param g_offset  green channel offset
 * @param b_offset  blue channel offset
 */
INLINE void aipl_mve_ldr_16px_rgb_uncut(aipl_mve_rgb_x16_t* dst,
                                        const uint8_t* src,
                                        uint8_t r_offset,
                                        uint8_t g_offset,
                                        uint8_t b_offset)
{
    dst->r = vldrbq_gather_offset(src + r_offset, AIPL_3_BYTE_OFFSETS_U8);
    dst->g = vldrbq_gather_offset(src + g_offset, AIPL_3_BYTE_OFFSETS_U8);
    dst->b = vldrbq_gather_offset(src + b_offset, AIPL_3_BYTE_OFFSETS_U8);
}

/**
 * Convert 8 RGB pixels to Y channel
 * using Helium vector register
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pixel vector
 */
INLINE void aipl_mve_cnvt_8px_rgb_to_yuv_y(uint16x8_t* dst,
                                           aipl_mve_rgb_x8_t src)
{
    aipl_mve_cnvt_8px_xrgb8888_to_yuv_y(dst, src);
}

/**
 * Convert 8 RGB pixels to U channel
 * using Helium vector register
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pixel vector
 */
INLINE void aipl_mve_cnvt_8px_rgb_to_yuv_u(uint16x8_t* dst,
                                           aipl_mve_rgb_x8_t src)
{
    aipl_mve_cnvt_8px_xrgb8888_to_yuv_u(dst, src);
}

/**
 * Convert 8 RGB pixels to V channel
 * using Helium vector register
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pixel vector
 */
INLINE void aipl_mve_cnvt_8px_rgb_to_yuv_v(uint16x8_t* dst,
                                           aipl_mve_rgb_x8_t src)
{
    aipl_mve_cnvt_8px_xrgb8888_to_yuv_v(dst, src);
}

/**
 * Store 16 RGB pixels to memory from
 * Helium vector register with vector predicate
 *
 * @param dst       destintation pointer
 * @param src       source pixel vectors
 * @param pred      vector predicate
 * @param r_offset  red channel offset
 * @param g_offset  green channel offset
 * @param b_offset  blue channel offset
 */
INLINE void aipl_mve_str_16px_rgb(uint8_t* dst,
                                  aipl_mve_rgb_x16_t src,
                                  mve_pred16_t pred,
                                  uint8_t r_offset,
                                  uint8_t g_offset,
                                  uint8_t b_offset)
{
    vstrbq_scatter_offset_p(dst + r_offset, AIPL_3_BYTE_OFFSETS_U8, src.r, pred);
    vstrbq_scatter_offset_p(dst + g_offset, AIPL_3_BYTE_OFFSETS_U8, src.g, pred);
    vstrbq_scatter_offset_p(dst + b_offset, AIPL_3_BYTE_OFFSETS_U8, src.b, pred);
}

/**
 * Store 16 RGB pixels to memory from
 * Helium vector register without vector predicate
 *
 * @param dst       destintation pointer
 * @param src       source pixel vectors
 * @param r_offset  red channel offset
 * @param g_offset  green channel offset
 * @param b_offset  blue channel offset
 */
INLINE void aipl_mve_str_16px_rgb_uncut(uint8_t* dst,
                                        aipl_mve_rgb_x16_t src,
                                        uint8_t r_offset,
                                        uint8_t g_offset,
                                        uint8_t b_offset)
{
    vstrbq_scatter_offset(dst + r_offset, AIPL_3_BYTE_OFFSETS_U8, src.r);
    vstrbq_scatter_offset(dst + g_offset, AIPL_3_BYTE_OFFSETS_U8, src.g);
    vstrbq_scatter_offset(dst + b_offset, AIPL_3_BYTE_OFFSETS_U8, src.b);
}


/**
 * Load 4 I400 pixels from memory to
 * Helium vector register
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pointer
 * @param pred  vector predicate
 */
INLINE void aipl_mve_ldr_4px_i400(uint32x4_t* dst,
                                  const uint8_t* src,
                                  mve_pred16_t pred)
{
    *dst = vldrbq_z_u32(src, pred);
}

/**
 * Load 8 I400 pixels from memory to
 * Helium vector register
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pointer
 * @param pred  vector predicate
 */
INLINE void aipl_mve_ldr_8px_i400(uint16x8_t* dst,
                                  const uint8_t* src,
                                  mve_pred16_t pred)
{
    *dst = vldrbq_z_u16(src, pred);
}

/**
 * Load 16 I400 pixels from memory to
 * Helium vector register
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pointer
 * @param pred  vector predicate
 */
INLINE void aipl_mve_ldr_16px_i400(uint8x16_t* dst,
                                   const uint8_t* src,
                                   mve_pred16_t pred)
{
    *dst = vldrbq_z_u8(src, pred);
}

/**
 * Convert 4 I400 pixels to ARGB8888
 * using Helium vector register
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pixel vector
 */
INLINE void aipl_mve_cnvt_4px_i400_to_argb8888(uint32x4_t* dst,
                                               uint32x4_t src)
{
    *dst = vmulq_u32(src, vdupq_n_u32(0x00010101));
    *dst = vorrq(*dst, vdupq_n_u32(0xff000000));
}

/**
 * Convert 8 I400 pixels to ARGB4444
 * using Helium vector register
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pixel vector
 */
INLINE void aipl_mve_cnvt_8px_i400_to_argb4444(uint16x8_t* dst,
                                               uint16x8_t src)
{
    *dst = vshrq(src, 4);
    *dst = vmulq_u16(*dst, vdupq_n_u16(0x0111));
    *dst = vorrq(*dst, vdupq_n_u16(0xf000));
}

/**
 * Convert 8 I400 pixels to ARGB1555
 * using Helium vector register
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pixel vector
 */
INLINE void aipl_mve_cnvt_8px_i400_to_argb1555(uint16x8_t* dst,
                                               uint16x8_t src)
{
    *dst = vshrq(src, 3);
    *dst = vmulq_u16(*dst, vdupq_n_u16(0x0421));
    *dst = vorrq(*dst, vdupq_n_u16(0x8000));
}

/**
 * Convert 4 I400 pixels to RGBA8888
 * using Helium vector register
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pixel vector
 */
INLINE void aipl_mve_cnvt_4px_i400_to_rgba8888(uint32x4_t* dst,
                                               uint32x4_t src)
{
    *dst = vmulq_u32(src, vdupq_n_u32(0x01010100));
    *dst = vorrq(*dst, vdupq_n_u32(0x000000ff));
}

/**
 * Convert 8 I400 pixels to RGBA4444
 * using Helium vector register
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pixel vector
 */
INLINE void aipl_mve_cnvt_8px_i400_to_rgba4444(uint16x8_t* dst,
                                               uint16x8_t src)
{
    *dst = vshrq(src, 4);
    *dst = vmulq_u16(*dst, vdupq_n_u16(0x1110));
    *dst = vorrq(*dst, vdupq_n_u16(0x000f));
}

/**
 * Convert 8 I400 pixels to RGBA5551
 * using Helium vector register
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pixel vector
 */
INLINE void aipl_mve_cnvt_8px_i400_to_rgba5551(uint16x8_t* dst,
                                               uint16x8_t src)
{
    *dst = vshrq(src, 3);
    *dst = vmulq_u16(*dst, vdupq_n_u16(0x0842));
    *dst = vorrq(*dst, vdupq_n_u16(0x0001));
}

/**
 * Convert 8 I400 pixels to RGB565
 * using Helium vector register
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pixel vector
 */
INLINE void aipl_mve_cnvt_8px_i400_to_rgb565(uint16x8_t* dst,
                                             uint16x8_t src)
{
    *dst = vshrq(src, 3);
    *dst = vmulq_u16(*dst, vdupq_n_u16(0x0841));
}

/**
 * Convert 16 I400 pixels to RGB
 * using Helium vector register
 *
 * @param dst   destination pixel vector pointer
 * @param src   source pixel vector
 */
INLINE void aipl_mve_cnvt_16px_i400_to_rgb(aipl_mve_rgb_x16_t* dst,
                                           uint8x16_t src)
{
    dst->r = src;
    dst->g = src;
    dst->b = src;
}

/**
 * Store 8 I400 pixels to memory from
 * Helium vector register
 *
 * @param dst   destintation pointer
 * @param src   source pixel vectors
 * @param pred  vector predicate
 */
INLINE void aipl_mve_str_8px_i400(uint8_t* dst,
                                  uint16x8_t src,
                                  mve_pred16_t pred)
{
    vstrbq_p(dst, src, pred);
}

/**
 * A util function for color conversions from YUV formats.
 * Precalculate the values of RGB channels (4 pixels)
 *
 * @param r_dst output red channel precalculated value
 * @param g_dst output green channel precalculated value
 * @param b_dst output blue channel precalculated value
 * @param u     U channel vector
 * @param v     V channel vector
 */
INLINE void aipl_mve_pre_cnvt_4px_yuv_to_rgb(int32x4_t* r_dst,
                                             int32x4_t* g_dst,
                                             int32x4_t* b_dst,
                                             uint32x4_t u,
                                             uint32x4_t v)
{
    int32x4_t d = vreinterpretq_s32(u);
    int32x4_t e = vreinterpretq_s32(v);

    d = vsubq_n_s32(d, 128);
    e = vsubq_n_s32(e, 128);

    *r_dst = vmlaq_n_s32(vdupq_n_s32(128), e, 409);
    *g_dst = vmlaq_n_s32(vdupq_n_s32(128), d, -100);
    *g_dst = vmlaq_n_s32(*g_dst, e, -208);
    *b_dst = vmlaq_n_s32(vdupq_n_s32(128), d, 516);
}

/**
 * A util function for color conversions from YUV formats.
 * Precalculate the values of RGB channels (8 pixels)
 *
 * @param r0_dst output red channel precalculated value
 * @param g0_dst output green channel precalculated value
 * @param b0_dst output blue channel precalculated value
 * @param r1_dst output red channel precalculated value
 * @param g1_dst output green channel precalculated value
 * @param b1_dst output blue channel precalculated value
 * @param u      U channel source vector
 * @param v      V channel source vector
 */
INLINE void aipl_mve_pre_cnvt_8px_yuv_to_rgb(int32x4_t* r0_dst,
                                             int32x4_t* g0_dst,
                                             int32x4_t* b0_dst,
                                             int32x4_t* r1_dst,
                                             int32x4_t* g1_dst,
                                             int32x4_t* b1_dst,
                                             uint16x8_t u,
                                             uint16x8_t v)
{
    int16x8_t d = vreinterpretq_s16(u);
    int16x8_t e = vreinterpretq_s16(v);

    d = vsubq_n_s16(d, 128);
    e = vsubq_n_s16(e, 128);

    int32x4_t d0 = vmovlbq(d);
    int32x4_t d1 = vmovltq(d);
    int32x4_t e0 = vmovlbq(e);
    int32x4_t e1 = vmovltq(e);

    *r0_dst = vmlaq_n_s32(vdupq_n_s32(128), e0, 409);
    *g0_dst = vmlaq_n_s32(vdupq_n_s32(128), d0, -100);
    *g0_dst = vmlaq_n_s32(*g0_dst, e0, -208);
    *b0_dst = vmlaq_n_s32(vdupq_n_s32(128), d0, 516);

    *r1_dst = vmlaq_n_s32(vdupq_n_s32(128), e1, 409);
    *g1_dst = vmlaq_n_s32(vdupq_n_s32(128), d1, -100);
    *g1_dst = vmlaq_n_s32(*g1_dst, e1, -208);
    *b1_dst = vmlaq_n_s32(vdupq_n_s32(128), d1, 516);
}

/**
 * A util function for color conversions from YUV formats.
 * Precalculate Y channel related values for RGB channels (4 pixels)
 *
 * @param c_dst output precalculated value
 * @param y     Y channel vector
 */
INLINE void aipl_mve_pre_cnvt_4px_y(int32x4_t* c_dst,
                                    uint32x4_t y)
{
    *c_dst = vreinterpretq_s32(y);
    *c_dst = vsubq_n_s32(*c_dst, 16);
    *c_dst = vmulq_n_s32(*c_dst, 298);
}

/**
 * A util function for color conversions from YUV formats.
 * Precalculate Y channel related values for RGB channels (8 pixels)
 *
 * @param c0_dst output precalculated value [0]
 * @param c1_dst output precalculated value [1]
 * @param y      Y channel vector
 */
INLINE void aipl_mve_pre_cnvt_8px_y(int32x4_t* c0_dst,
                                    int32x4_t* c1_dst,
                                    uint16x8_t y)
{
    uint16x8_t c = y;
    c = vsubq_n_u16(c, 16);

    *c0_dst = vreinterpretq_s32(vmovlbq(c));
    *c1_dst = vreinterpretq_s32(vmovltq(c));

    *c0_dst = vmulq_n_s32(*c0_dst, 298);
    *c1_dst = vmulq_n_s32(*c1_dst, 298);
}

/**
 * A util function for color conversions from YUV formats.
 * Precalculate Y channel related values for RGB channels (8 by 2 pixels)
 *
 * @param c00_dst output precalculated value [0, 0]
 * @param c01_dst output precalculated value [0, 1]
 * @param c10_dst output precalculated value [1, 0]
 * @param c11_dst output precalculated value [1, 1]
 * @param y0      Y channel vector row 0
 * @param y1      Y channel vector row 1
 */
INLINE void aipl_mve_pre_cnvt_8x2px_y(int32x4_t* c00_dst,
                                      int32x4_t* c01_dst,
                                      int32x4_t* c10_dst,
                                      int32x4_t* c11_dst,
                                      uint16x8_t y0,
                                      uint16x8_t y1)
{
    aipl_mve_pre_cnvt_8px_y(c00_dst, c01_dst, y0);
    aipl_mve_pre_cnvt_8px_y(c10_dst, c11_dst, y1);
}

/**
 * Convert YUV to R, G, and B channels using precalculated values (4 pixels)
 *
 * @param r_dst     red channels output
 * @param g_dst     green channels output
 * @param b_dst     blue channels output
 * @param c_src     precalculated luminocity portion
 * @param r_src     precalculated red portion
 * @param g_src     precalculated green portion
 * @param b_src     precalculated blue portion
 */
INLINE void aipl_mve_cnvt_4px_yuv_to_rgb(int16x8_t* r_dst,
                                         int16x8_t* g_dst,
                                         int16x8_t* b_dst,
                                         int32x4_t c_src,
                                         int32x4_t r_src,
                                         int32x4_t g_src,
                                         int32x4_t b_src)
{
    *r_dst = vreinterpretq_s16(vshrq(vaddq(c_src, r_src), 8));
    *g_dst = vreinterpretq_s16(vshrq(vaddq(c_src, g_src), 8));
    *b_dst = vreinterpretq_s16(vshrq(vaddq(c_src, b_src), 8));
}

/**
 * Convert YUV to R, G, and B channels using precalculated values (8 pixels)
 *
 * @param r_dst      red channels output
 * @param g_dst      green channels output
 * @param b_dst      blue channels output
 * @param c0_src     precalculated luminocity portion
 * @param r0_src     precalculated red portion
 * @param g0_src     precalculated green portion
 * @param b0_src     precalculated blue portion
 * @param c1_src     precalculated luminocity portion
 * @param r1_src     precalculated red portion
 * @param g1_src     precalculated green portion
 * @param b1_src     precalculated blue portion
 */
INLINE void aipl_mve_cnvt_8px_yuv_to_rgb(uint16x8_t* r_dst,
                                         uint16x8_t* g_dst,
                                         uint16x8_t* b_dst,
                                         int32x4_t c0_src,
                                         int32x4_t r0_src,
                                         int32x4_t g0_src,
                                         int32x4_t b0_src,
                                         int32x4_t c1_src,
                                         int32x4_t r1_src,
                                         int32x4_t g1_src,
                                         int32x4_t b1_src)
{
    int32x4_t r0 = vshrq(vaddq(c0_src, r0_src), 8);
    int32x4_t g0 = vshrq(vaddq(c0_src, g0_src), 8);
    int32x4_t b0 = vshrq(vaddq(c0_src, b0_src), 8);

    int32x4_t r1 = vshrq(vaddq(c1_src, r1_src), 8);
    int32x4_t g1 = vshrq(vaddq(c1_src, g1_src), 8);
    int32x4_t b1 = vshrq(vaddq(c1_src, b1_src), 8);

    *r_dst = vqmovuntq(vqmovunbq(vdupq_n_u16(0), r0), r1);
    *g_dst = vqmovuntq(vqmovunbq(vdupq_n_u16(0), g0), g1);
    *b_dst = vqmovuntq(vqmovunbq(vdupq_n_u16(0), b0), b1);
}

/**
 * Convert YUV to ARGB8888 using precalculated values (4 pixels)
 *
 * @param dst       ouput ARGB8888 pixels vector
 * @param c_src     precalculated luminocity portion
 * @param r_src     precalculated red portion
 * @param g_src     precalculated green portion
 * @param b_src     precalculated blue portion
 */
INLINE void aipl_mve_cnvt_4px_yuv_to_argb8888(uint32x4_t* dst,
                                              int32x4_t c_src,
                                              int32x4_t r_src,
                                              int32x4_t g_src,
                                              int32x4_t b_src)
{
    int16x8_t r, g, b;
    aipl_mve_cnvt_4px_yuv_to_rgb(&r, &g, &b, c_src, r_src, g_src, b_src);

    uint32x4_t ar = vreinterpretq_u32(vqmovunbq(vdupq_n_u8(0xff), r));
    uint32x4_t gb = vreinterpretq_u32(vqmovuntq(vqmovunbq(vdupq_n_u8(0), b), g));

    *dst = vorrq(vshlq_n(ar, 16), vandq(gb, vdupq_n_u32(0x0000ffff)));
}

/**
 * Convert YUV to ARGB4444 using precalculated values (8 pixels)
 *
 * @param dst       ouput ARGB4444 pixels vector
 * @param c0_src    precalculated luminocity portion [0]
 * @param c1_src    precalculated luminocity portion [1]
 * @param r_src     precalculated red portion
 * @param g_src     precalculated green portion
 * @param b_src     precalculated blue portion
 */
INLINE void aipl_mve_cnvt_8px_yuv_to_argb4444(uint16x8_t* dst,
                                              int32x4_t c0_src,
                                              int32x4_t c1_src,
                                              int32x4_t r_src,
                                              int32x4_t g_src,
                                              int32x4_t b_src)
{
    uint16x8_t r, g, b;
    aipl_mve_cnvt_8px_yuv_to_rgb(&r, &g, &b, c0_src, r_src, g_src, b_src,
                                 c1_src, r_src, g_src, b_src);

    *dst = vorrq(vorrq(vdupq_n_u16(0xf000), vandq(vshlq_n(r, 4), vdupq_n_u16(0x0f00))),
                 vorrq(vandq(g, vdupq_n_u16(0x00f0)), vshrq(b, 4)));
}

/**
 * Convert YUV to ARGB4444 using precalculated values (4+4 pixels)
 *
 * @param dst       ouput ARGB4444 pixels vector
 * @param c0_src    precalculated luminocity portion [0]
 * @param c1_src    precalculated luminocity portion [1]
 * @param r0_src    precalculated red portion   [0]
 * @param g0_src    precalculated green portion [0]
 * @param b0_src    precalculated blue portion  [0]
 * @param r1_src    precalculated red portion   [1]
 * @param g1_src    precalculated green portion [1]
 * @param b1_src    precalculated blue portion  [1]
 */
INLINE void aipl_mve_cnvt_44px_yuv_to_argb4444(uint16x8_t* dst,
                                               int32x4_t c0_src,
                                               int32x4_t c1_src,
                                               int32x4_t r0_src,
                                               int32x4_t g0_src,
                                               int32x4_t b0_src,
                                               int32x4_t r1_src,
                                               int32x4_t g1_src,
                                               int32x4_t b1_src)
{
    uint16x8_t r, g, b;
    aipl_mve_cnvt_8px_yuv_to_rgb(&r, &g, &b, c0_src, r0_src, g0_src, b0_src,
                                 c1_src, r1_src, g1_src, b1_src);

    *dst = vorrq(vorrq(vdupq_n_u16(0xf000), vandq(vshlq_n(r, 4), vdupq_n_u16(0x0f00))),
                 vorrq(vandq(g, vdupq_n_u16(0x00f0)), vshrq(b, 4)));
}

/**
 * Convert YUV to ARGB1555 using precalculated values (8 pixels)
 *
 * @param dst       output ARGB1555 pixels vector
 * @param c0_src    precalculated luminocity portion [0]
 * @param c1_src    precalculated luminocity portion [1]
 * @param r_src     precalculated red portion
 * @param g_src     precalculated green portion
 * @param b_src     precalculated blue portion
 */
INLINE void aipl_mve_cnvt_8px_yuv_to_argb1555(uint16x8_t* dst,
                                              int32x4_t c0_src,
                                              int32x4_t c1_src,
                                              int32x4_t r_src,
                                              int32x4_t g_src,
                                              int32x4_t b_src)
{
    uint16x8_t r, g, b;
    aipl_mve_cnvt_8px_yuv_to_rgb(&r, &g, &b, c0_src, r_src, g_src, b_src,
                                 c1_src, r_src, g_src, b_src);

    *dst = vorrq(vorrq(vdupq_n_u16(0x8000), vandq(vshlq_n(r, 7), vdupq_n_u16(0x7c00))),
                 vorrq(vandq(vshlq_n(g, 2), vdupq_n_u16(0x03e0)), vshrq(b, 3)));
}

/**
 * Convert YUV to ARGB1555 using precalculated values (4+4 pixels)
 *
 * @param dst       output ARGB1555 pixels vector
 * @param c0_src    precalculated luminocity portion [0]
 * @param c1_src    precalculated luminocity portion [1]
 * @param r0_src    precalculated red portion   [0]
 * @param g0_src    precalculated green portion [0]
 * @param b0_src    precalculated blue portion  [0]
 * @param r1_src    precalculated red portion   [1]
 * @param g1_src    precalculated green portion [1]
 * @param b1_src    precalculated blue portion  [1]
 */
INLINE void aipl_mve_cnvt_44px_yuv_to_argb1555(uint16x8_t* dst,
                                               int32x4_t c0_src,
                                               int32x4_t c1_src,
                                               int32x4_t r0_src,
                                               int32x4_t g0_src,
                                               int32x4_t b0_src,
                                               int32x4_t r1_src,
                                               int32x4_t g1_src,
                                               int32x4_t b1_src)
{
    uint16x8_t r, g, b;
    aipl_mve_cnvt_8px_yuv_to_rgb(&r, &g, &b, c0_src, r0_src, g0_src, b0_src,
                                 c1_src, r1_src, g1_src, b1_src);

    *dst = vorrq(vorrq(vdupq_n_u16(0x8000), vandq(vshlq_n(r, 7), vdupq_n_u16(0x7c00))),
                 vorrq(vandq(vshlq_n(g, 2), vdupq_n_u16(0x03e0)), vshrq(b, 3)));
}

/**
 * Convert YUV to RGBA8888 using precalculated values (4 pixels)
 *
 * @param dst       ouput RGBA8888 pixels vector
 * @param c_src     precalculated luminocity portion
 * @param r_src     precalculated red portion
 * @param g_src     precalculated green portion
 * @param b_src     precalculated blue portion
 */
INLINE void aipl_mve_cnvt_4px_yuv_to_rgba8888(uint32x4_t* dst,
                                              int32x4_t c_src,
                                              int32x4_t r_src,
                                              int32x4_t g_src,
                                              int32x4_t b_src)
{
    int16x8_t r, g, b;
    aipl_mve_cnvt_4px_yuv_to_rgb(&r, &g, &b, c_src, r_src, g_src, b_src);

    uint32x4_t rg = vreinterpretq_u32(vqmovuntq(vqmovunbq(vdupq_n_u8(0), g), r));
    uint32x4_t ba = vreinterpretq_u32(vqmovuntq(vdupq_n_u8(0xff), b));

    *dst = vorrq(vshlq_n(rg, 16), vandq(ba, vdupq_n_u32(0x0000ffff)));
}

/**
 * Convert YUV to RGBA4444 using precalculated values (8 pixels)
 *
 * @param dst       output RGBA4444 pixels vector
 * @param c0_src    precalculated luminocity portion [0]
 * @param c1_src    precalculated luminocity portion [1]
 * @param r_src     precalculated red portion
 * @param g_src     precalculated green portion
 * @param b_src     precalculated blue portion
 */
INLINE void aipl_mve_cnvt_8px_yuv_to_rgba4444(uint16x8_t* dst,
                                              int32x4_t c0_src,
                                              int32x4_t c1_src,
                                              int32x4_t r_src,
                                              int32x4_t g_src,
                                              int32x4_t b_src)
{
    uint16x8_t r, g, b;
    aipl_mve_cnvt_8px_yuv_to_rgb(&r, &g, &b, c0_src, r_src, g_src, b_src,
                                 c1_src, r_src, g_src, b_src);

    *dst = vorrq(vorrq(vandq(vshlq_n(r, 8), vdupq_n_u16(0xf000)), vandq(vshlq_n(g, 4), vdupq_n_u16(0x0f00))),
                 vorrq(vandq(b, vdupq_n_u16(0x00f0)), vdupq_n_u16(0x000f)));
}

/**
 * Convert YUV to RGBA4444 using precalculated values (4+4 pixels)
 *
 * @param dst       output RGBA4444 pixels vector
 * @param c0_src    precalculated luminocity portion [0]
 * @param c1_src    precalculated luminocity portion [1]
 * @param r0_src    precalculated red portion   [0]
 * @param g0_src    precalculated green portion [0]
 * @param b0_src    precalculated blue portion  [0]
 * @param r1_src    precalculated red portion   [1]
 * @param g1_src    precalculated green portion [1]
 * @param b1_src    precalculated blue portion  [1]
 */
INLINE void aipl_mve_cnvt_44px_yuv_to_rgba4444(uint16x8_t* dst,
                                               int32x4_t c0_src,
                                               int32x4_t c1_src,
                                               int32x4_t r0_src,
                                               int32x4_t g0_src,
                                               int32x4_t b0_src,
                                               int32x4_t r1_src,
                                               int32x4_t g1_src,
                                               int32x4_t b1_src)
{
    uint16x8_t r, g, b;
    aipl_mve_cnvt_8px_yuv_to_rgb(&r, &g, &b, c0_src, r0_src, g0_src, b0_src,
                                 c1_src, r1_src, g1_src, b1_src);

    *dst = vorrq(vorrq(vandq(vshlq_n(r, 8), vdupq_n_u16(0xf000)), vandq(vshlq_n(g, 4), vdupq_n_u16(0x0f00))),
                 vorrq(vandq(b, vdupq_n_u16(0x00f0)), vdupq_n_u16(0x000f)));
}

/**
 * Convert YUV to RGBA5551 using precalculated values (8 pixels)
 *
 * @param dst       output RGBA5551 pixels vector
 * @param c0_src    precalculated luminocity portion [0]
 * @param c1_src    precalculated luminocity portion [1]
 * @param r_src     precalculated red portion
 * @param g_src     precalculated green portion
 * @param b_src     precalculated blue portion
 */
INLINE void aipl_mve_cnvt_8px_yuv_to_rgba5551(uint16x8_t* dst,
                                              int32x4_t c0_src,
                                              int32x4_t c1_src,
                                              int32x4_t r_src,
                                              int32x4_t g_src,
                                              int32x4_t b_src)
{
    uint16x8_t r, g, b;
    aipl_mve_cnvt_8px_yuv_to_rgb(&r, &g, &b, c0_src, r_src, g_src, b_src,
                                 c1_src, r_src, g_src, b_src);

    *dst = vorrq(vorrq(vdupq_n_u16(0x0001), vandq(vshlq_n(r, 8), vdupq_n_u16(0xf800))),
                 vorrq(vandq(vshlq_n(g, 3), vdupq_n_u16(0x07c0)), vandq(vshrq(b, 2), vdupq_n_u16(0x03e))));
}

/**
 * Convert YUV to RGBA5551 using precalculated values (4+4 pixels)
 *
 * @param dst       output RGBA5551 pixels vector
 * @param c0_src    precalculated luminocity portion [0]
 * @param c1_src    precalculated luminocity portion [1]
 * @param r0_src    precalculated red portion   [0]
 * @param g0_src    precalculated green portion [0]
 * @param b0_src    precalculated blue portion  [0]
 * @param r1_src    precalculated red portion   [1]
 * @param g1_src    precalculated green portion [1]
 * @param b1_src    precalculated blue portion  [1]
 */
INLINE void aipl_mve_cnvt_44px_yuv_to_rgba5551(uint16x8_t* dst,
                                               int32x4_t c0_src,
                                               int32x4_t c1_src,
                                               int32x4_t r0_src,
                                               int32x4_t g0_src,
                                               int32x4_t b0_src,
                                               int32x4_t r1_src,
                                               int32x4_t g1_src,
                                               int32x4_t b1_src)
{
    uint16x8_t r, g, b;
    aipl_mve_cnvt_8px_yuv_to_rgb(&r, &g, &b, c0_src, r0_src, g0_src, b0_src,
                                 c1_src, r1_src, g1_src, b1_src);

    *dst = vorrq(vorrq(vdupq_n_u16(0x0001), vandq(vshlq_n(r, 8), vdupq_n_u16(0xf800))),
                 vorrq(vandq(vshlq_n(g, 3), vdupq_n_u16(0x07c0)), vandq(vshrq(b, 2), vdupq_n_u16(0x03e))));
}

/**
 * Convert YUV to RGB565 using precalculated values (8 pixels)
 *
 * @param dst       output RGB565 pixels vector
 * @param c0_src    precalculated luminocity portion [0]
 * @param c1_src    precalculated luminocity portion [1]
 * @param r_src     precalculated red portion
 * @param g_src     precalculated green portion
 * @param b_src     precalculated blue portion
 */
INLINE void aipl_mve_cnvt_8px_yuv_to_rgb565(uint16x8_t* dst,
                                            int32x4_t c0_src,
                                            int32x4_t c1_src,
                                            int32x4_t r_src,
                                            int32x4_t g_src,
                                            int32x4_t b_src)
{
    uint16x8_t r, g, b;
    aipl_mve_cnvt_8px_yuv_to_rgb(&r, &g, &b, c0_src, r_src, g_src, b_src,
                                 c1_src, r_src, g_src, b_src);

    *dst = vorrq(vandq(vshlq_n(r, 8), vdupq_n_u16(0xf800)),
                 vorrq(vandq(vshlq_n(g, 3), vdupq_n_u16(0x07e0)), vshrq(b, 3)));
}

/**
 * Convert YUV to RGB565 using precalculated values (4+4 pixels)
 *
 * @param dst       output RGB565 pixels vector
 * @param c0_src    precalculated luminocity portion [0]
 * @param c1_src    precalculated luminocity portion [1]
 * @param r0_src    precalculated red portion   [0]
 * @param g0_src    precalculated green portion [0]
 * @param b0_src    precalculated blue portion  [0]
 * @param r1_src    precalculated red portion   [1]
 * @param g1_src    precalculated green portion [1]
 * @param b1_src    precalculated blue portion  [1]
 */
INLINE void aipl_mve_cnvt_44px_yuv_to_rgb565(uint16x8_t* dst,
                                             int32x4_t c0_src,
                                             int32x4_t c1_src,
                                             int32x4_t r0_src,
                                             int32x4_t g0_src,
                                             int32x4_t b0_src,
                                             int32x4_t r1_src,
                                             int32x4_t g1_src,
                                             int32x4_t b1_src)
{
    uint16x8_t r, g, b;
    aipl_mve_cnvt_8px_yuv_to_rgb(&r, &g, &b, c0_src, r0_src, g0_src, b0_src,
                                 c1_src, r1_src, g1_src, b1_src);

    *dst = vorrq(vandq(vshlq_n(r, 8), vdupq_n_u16(0xf800)),
                 vorrq(vandq(vshlq_n(g, 3), vdupq_n_u16(0x07e0)), vshrq(b, 3)));
}

/**
 * Peform color correction on 8 sets of R, G and B channels
 *
 * @param r     R channel vector pointer
 * @param g     G channel vector pointer
 * @param b     B channel vector pointer
 * @param ccm   color correcion matrix
 */
INLINE void aipl_mve_color_correction_rgb_channels_x8(uint16x8_t* r,
                                                      uint16x8_t* g,
                                                      uint16x8_t* b,
                                                      const float* ccm)
{
    float16x8_t r_f = vcvtq(*r);
    float16x8_t g_f = vcvtq(*g);
    float16x8_t b_f = vcvtq(*b);

    float16x8_t r_mac = vmulq_n_f16(r_f, ccm[0]);
    r_mac = vfmaq(r_mac, g_f, ccm[1]);
    r_mac = vfmaq(r_mac, b_f, ccm[2]);

    uint16x8_t r_out = vcvtq_u16_f16(r_mac);
    *r = vreinterpretq_u16(vqmovnbq(vdupq_n_u8(0), r_out));

    float16x8_t g_mac = vmulq_n_f16(r_f, ccm[3]);
    g_mac = vfmaq(g_mac, g_f, ccm[4]);
    g_mac = vfmaq(g_mac, b_f, ccm[5]);

    uint16x8_t g_out = vcvtq_u16_f16(g_mac);
    *g = vreinterpretq_u16(vqmovnbq(vdupq_n_u8(0), g_out));

    float16x8_t b_mac = vmulq_n_f16(r_f, ccm[6]);
    b_mac = vfmaq(b_mac, g_f, ccm[7]);
    b_mac = vfmaq(b_mac, b_f, ccm[8]);

    uint16x8_t b_out = vcvtq_u16_f16(b_mac);
    *b = vreinterpretq_u16(vqmovnbq(vdupq_n_u8(0), b_out));
}

/**
 * Perform color correction on 8 ARGB pixels
 *
 * @param pix   pixels struct pointer
 * @param ccm   color correction matrix
 */
INLINE void aipl_mve_color_correction_argb_x8(aipl_mve_argb_x8_t* pix,
                                              const float* ccm)
{
    aipl_mve_color_correction_rgb_channels_x8(&pix->r, &pix->g, &pix->b, ccm);
}

/**
 * Perform color correction on 16 ARGB pixels
 *
 * @param pix   pixels struct pointer
 * @param ccm   color correction matrix
 */
INLINE void aipl_mve_color_correction_argb_x16(aipl_mve_argb_x16_t* pix,
                                                const float* ccm)
{
    aipl_mve_argb_x8_t pix_t;
    aipl_mve_argb_x8_t pix_b;
    aipl_mve_convert_argb_x16_to_x8_odd(&pix_t, pix);
    aipl_mve_convert_argb_x16_to_x8_evn(&pix_b, pix);

    aipl_mve_color_correction_argb_x8(&pix_t, ccm);
    aipl_mve_color_correction_argb_x8(&pix_b, ccm);

    aipl_mve_convert_2_argb_x8_to_x16(pix, &pix_b, &pix_t);
}

/**
 * Perform color correction on 8 RGB pixels
 *
 * @param pix   pixels struct pointer
 * @param ccm   color correction matrix
 */
INLINE void aipl_mve_color_correction_rgb_x8(aipl_mve_rgb_x8_t* pix,
                                             const float* ccm)
{
    aipl_mve_color_correction_rgb_channels_x8(&pix->r, &pix->g, &pix->b, ccm);
}

/**
 * Perform color correction on 16 RGB pixels
 *
 * @param pix   pixels struct pointer
 * @param ccm   color correction matrix
 */
INLINE void aipl_mve_color_correction_rgb_x16(aipl_mve_rgb_x16_t* pix,
                                              const float* ccm)
{
    aipl_mve_rgb_x8_t pix_t;
    aipl_mve_rgb_x8_t pix_b;
    aipl_mve_convert_rgb_x16_to_x8_odd(&pix_t, pix);
    aipl_mve_convert_rgb_x16_to_x8_evn(&pix_b, pix);

    aipl_mve_color_correction_rgb_x8(&pix_t, ccm);
    aipl_mve_color_correction_rgb_x8(&pix_b, ccm);

    aipl_mve_convert_2_rgb_x8_to_x16(pix, &pix_b, &pix_t);
}

/**
 * Perform white balance on on 8 sets of R, G and B channels
 *
 * @param r     R channel vector pointer
 * @param g     G channel vector pointer
 * @param b     B channel vector pointer
 * @param ar    red channel multiplier
 * @param ag    green channel multiplier
 * @param ab    blue channel multiplier
 */
INLINE void aipl_mve_white_balance_rgb_channels_x8(uint16x8_t* r,
                                                   uint16x8_t* g,
                                                   uint16x8_t* b,
                                                   float ar, float ag, float ab)
{
    float16x8_t r_f = vcvtq(*r);
    float16x8_t g_f = vcvtq(*g);
    float16x8_t b_f = vcvtq(*b);

    float16x8_t r_mul = vmulq_n_f16(r_f, ar);

    uint16x8_t r_out = vcvtq_u16_f16(r_mul);
    *r = vreinterpretq_u16(vqmovnbq(vdupq_n_u8(0), r_out));

    float16x8_t g_mul = vmulq_n_f16(g_f, ag);

    uint16x8_t g_out = vcvtq_u16_f16(g_mul);
    *g = vreinterpretq_u16(vqmovnbq(vdupq_n_u8(0), g_out));

    float16x8_t b_mul = vmulq_n_f16(b_f, ab);

    uint16x8_t b_out = vcvtq_u16_f16(b_mul);
    *b = vreinterpretq_u16(vqmovnbq(vdupq_n_u8(0), b_out));
}

/**
 * Perform white balance on 8 ARGB pixels
 *
 * @param pix   pixels struct pointer
 * @param ar    red channel multiplier
 * @param ag    green channel multiplier
 * @param ab    blue channel multiplier
 */
INLINE void aipl_mve_white_balance_argb_x8(aipl_mve_argb_x8_t* pix,
                                           float ar, float ag, float ab)
{
    aipl_mve_white_balance_rgb_channels_x8(&pix->r, &pix->g, &pix->b,
                                           ar, ag, ab);
}

/**
 * Perform white balance on 16 ARGB pixels
 *
 * @param pix   pixels struct pointer
 * @param ar    red channel multiplier
 * @param ag    green channel multiplier
 * @param ab    blue channel multiplier
 */
INLINE void aipl_mve_white_balance_argb_x16(aipl_mve_argb_x16_t* pix,
                                            float ar, float ag, float ab)
{
    aipl_mve_argb_x8_t pix_t;
    aipl_mve_argb_x8_t pix_b;
    aipl_mve_convert_argb_x16_to_x8_odd(&pix_t, pix);
    aipl_mve_convert_argb_x16_to_x8_evn(&pix_b, pix);

    aipl_mve_white_balance_argb_x8(&pix_t, ar, ag, ab);
    aipl_mve_white_balance_argb_x8(&pix_b, ar, ag, ab);

    aipl_mve_convert_2_argb_x8_to_x16(pix, &pix_b, &pix_t);

}

/**
 * Perform white balance on 8 RGB pixels
 *
 * @param pix   pixels struct pointer
 * @param ar    red channel multiplier
 * @param ag    green channel multiplier
 * @param ab    blue channel multiplier
 */
INLINE void aipl_mve_white_balance_rgb_x8(aipl_mve_rgb_x8_t* pix,
                                          float ar, float ag, float ab)
{
    aipl_mve_white_balance_rgb_channels_x8(&pix->r, &pix->g, &pix->b,
                                           ar, ag, ab);
}

/**
 * Perform white balance on 16 RGB pixels
 *
 * @param pix   pixels struct pointer
 * @param ar    red channel multiplier
 * @param ag    green channel multiplier
 * @param ab    blue channel multiplier
 */
INLINE void aipl_mve_white_balance_rgb_x16(aipl_mve_rgb_x16_t* pix,
                                           float ar, float ag, float ab)
{
    aipl_mve_rgb_x8_t pix_t;
    aipl_mve_rgb_x8_t pix_b;
    aipl_mve_convert_rgb_x16_to_x8_odd(&pix_t, pix);
    aipl_mve_convert_rgb_x16_to_x8_evn(&pix_b, pix);

    aipl_mve_white_balance_rgb_x8(&pix_t, ar, ag, ab);
    aipl_mve_white_balance_rgb_x8(&pix_b, ar, ag, ab);

    aipl_mve_convert_2_rgb_x8_to_x16(pix, &pix_b, &pix_t);
}

/**
 * Perform gamma correction on 16 sets of R, G and B channels
 *
 * @param r     R channel vector pointer
 * @param g     G channel vector pointer
 * @param b     B channel vector pointer
 * @param lut   gamma lookup table
 */
INLINE void aipl_mve_lut_transform_rgb_channels_x16(uint8x16_t* r,
                                                    uint8x16_t* g,
                                                    uint8x16_t* b,
                                                    uint8_t* lut)
{
    *r = vldrbq_gather_offset(lut, *r);
    *g = vldrbq_gather_offset(lut, *g);
    *b = vldrbq_gather_offset(lut, *b);
}

/**
 * Perform gamma correction on 16 ARGB pixels
 *
 * @param pix   pixels struct pointer
 * @param lut   gamma lookup table
 */
INLINE void aipl_mve_lut_transform_argb_x16(aipl_mve_argb_x16_t* pix,
                                            uint8_t* lut)
{
    aipl_mve_lut_transform_rgb_channels_x16(&pix->r, &pix->g, &pix->b, lut);
}

/**
 * Perform gamma correction on 16 RGB pixels
 *
 * @param pix   pixels struct pointer
 * @param lut   gamma lookup table
 */
INLINE void aipl_mve_lut_transform_rgb_x16(aipl_mve_rgb_x16_t* pix,
                                           uint8_t* lut)
{
    aipl_mve_lut_transform_rgb_channels_x16(&pix->r, &pix->g, &pix->b, lut);
}

/**********************
 *      MACROS
 **********************/

#endif /*AIPL_HELIUM_ACCELERATION*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*AIPL_MVE_UTILS_H*/

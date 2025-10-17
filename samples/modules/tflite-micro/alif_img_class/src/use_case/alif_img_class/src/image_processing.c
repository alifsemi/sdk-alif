/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include "image_processing.h"

/*
 * Define USE_MVE_RAW10_CONVERSION to 1 to use MVE-optimized version,
 * or 0 to use portable C version
 */
#define USE_MVE_RAW10_CONVERSION 1

#if USE_MVE_RAW10_CONVERSION
#include <arm_mve.h>
#endif
#include <math.h>

#if RTE_ARX3A0_CAMERA_SENSOR_ENABLE /* REV A RTE */

/*
 * Revised matrix (BECP-1455)
 * Manual WB, Illuminant 6021, Relative Red 1.42, Relative Blue 1.47
 */
#define C_RR (+2.2583f)
#define C_RG (-0.5501f)
#define C_RB (-0.1248f)
#define C_GR (-0.1606f)
#define C_GG (+1.4318f)
#define C_GB (-0.5268f)
#define C_BR (-0.6317f)
#define C_BG (-0.0653f)
#define C_BB (+2.3735f)

const float *camera_get_color_correction_matrix(void)
{
	static const float ccm[9] = {C_RR, C_GR, C_BR, C_RG, C_GG, C_BG, C_RB, C_GB, C_BB};

	return ccm;
}
#endif

static inline float srgb_oetf(float lum)
{
	if (lum <= 0.0031308f) {
		return 12.92f * lum;
	} else {
		return 1.055f * powf(lum, 1.0f / 2.4f) - 0.055f;
	}
}

uint8_t *camera_get_gamma_lut(void)
{
	static bool inited;
	static uint8_t lut[256];

	if (inited) {
		return lut;
	}

	for (int i = 0; i < 256; i++) {
		lut[i] = (uint8_t)(255.0f * srgb_oetf(i * (1.0f / 255.0f)) + 0.5f);
	}

	inited = true;
	return lut;
}

/*
 * Input:  buf holds N pixels, each pixel stored as 16-bit little-endian bytes:
 *           buf[2*i]   = low byte
 *           buf[2*i+1] = high byte
 *         only lower 10 bits are valid (0..1023)
 *
 * Output: buf[0..N-1] overwritten with RAW8 (0..255), same pixel order.
 *
 * Accurate mapping: out8 = round(v10 * 255 / 1023)
 * Implemented as fixed-point:
 *   K = round(255*2^16/1023) = 16336
 *   out8 = (v10*K + 2^15) >> 16
 */

#if USE_MVE_RAW10_CONVERSION

void raw10_gray16le_bytes_to_raw8_inplace_mve(uint8_t *buf, size_t n_pixels)
{
	uint8_t *dst = buf;
	const uint8_t *src = buf;

	const uint16x8_t mask10 = vdupq_n_u16(0x03FFu);

	const uint32_t K = 16336u;
	const uint32x4_t rnd = vdupq_n_u32(1u << 15);

#if CIMAGE_EXPOSURE_CALC
	/* Exposure analysis thresholds (in RAW10 space: 0-1023) */
	/* Scale thresholds from RAW8 to RAW10: multiply by 1023/255 ≈ 4.01176 */
	/* THRESH_LOW = 9 * 4.01176 ≈ 36, THRESH_HIGH = 154 * 4.01176 ≈ 618 */
	const uint16_t THRESH_LOW_10 = 36;
	const uint16_t THRESH_HIGH_10 = 618;

	uint32_t under = 0, low = 0, high = 0, over = 0;
#endif

	size_t i = 0;

	/* 16 pixels per iteration: read 32 bytes, write 16 bytes */
	for (; i + 16 <= n_pixels; i += 16) {
		const uint8_t *p = src + 2 * i;

		/* De-interleaving load: even bytes and odd bytes */
		/* For LE gray16 stream: even=low byte, odd=high byte */
		uint8x16x2_t lohi = vld2q_u8(p);
		uint8x16_t loB = lohi.val[0];
		uint8x16_t hiB = lohi.val[1];

		/* Widen to u16 for first 8 pixels and next 8 pixels */
		uint16x8_t lo0 = vmovlbq_u8(loB);
		uint16x8_t lo1 = vmovltq_u8(loB);
		uint16x8_t hi0 = vmovlbq_u8(hiB);
		uint16x8_t hi1 = vmovltq_u8(hiB);

		/* Reconstruct uint16 words: w = lo + (hi<<8), then extract 10-bit */
		uint16x8_t v10_0 = vandq_u16(vorrq_u16(lo0, vshlq_n_u16(hi0, 8)), mask10);
		uint16x8_t v10_1 = vandq_u16(vorrq_u16(lo1, vshlq_n_u16(hi1, 8)), mask10);

#if CIMAGE_EXPOSURE_CALC
		/* MVE-vectorized exposure analysis on RAW10 data */
		/* Use vector comparisons and count set predicate bits */
		/* Note: For unsigned comparisons, use vcmpcsq (carry set = >=) */
		/*       For val < thresh, check if thresh > val using vcmphiq */

		/* Broadcast threshold for < comparison (need vector-vector compare) */
		uint16x8_t thresh_low_vec = vdupq_n_u16(THRESH_LOW_10);

		/* Process first 8 pixels (v10_0) */
		mve_pred16_t p_over_0 = vcmpeqq_n_u16(v10_0, 1023);
		mve_pred16_t p_under_0 = vcmpeqq_n_u16(v10_0, 0);
		mve_pred16_t p_high_0 = vcmpcsq_n_u16(v10_0, THRESH_HIGH_10); /* val >= thresh */
		mve_pred16_t p_low_0 = vcmphiq_u16(thresh_low_vec, v10_0);    /* thresh > val means val < thresh */

		/* Process next 8 pixels (v10_1) */
		mve_pred16_t p_over_1 = vcmpeqq_n_u16(v10_1, 1023);
		mve_pred16_t p_under_1 = vcmpeqq_n_u16(v10_1, 0);
		mve_pred16_t p_high_1 = vcmpcsq_n_u16(v10_1, THRESH_HIGH_10); /* val >= thresh */
		mve_pred16_t p_low_1 = vcmphiq_u16(thresh_low_vec, v10_1);    /* thresh > val means val < thresh */

		/* Count set bits in predicates (each u16 lane uses 2 bits in predicate) */
		/* __builtin_popcount counts 1-bits; divide by 2 for u16 lane count */
		over += __builtin_popcount(p_over_0) / 2 + __builtin_popcount(p_over_1) / 2;
		under += __builtin_popcount(p_under_0) / 2 + __builtin_popcount(p_under_1) / 2;
		high += __builtin_popcount(p_high_0) / 2 + __builtin_popcount(p_high_1) / 2;
		low += __builtin_popcount(p_low_0) / 2 + __builtin_popcount(p_low_1) / 2;
#endif

		/* Accurate scale 8 pixels: v10_0 -> u8 in u16 lanes (0..255) */
		uint32x4_t a0 = vmovlbq_u16(v10_0);
		uint32x4_t a1 = vmovltq_u16(v10_0);

		a0 = vaddq_u32(vmulq_n_u32(a0, K), rnd);
		a1 = vaddq_u32(vmulq_n_u32(a1, K), rnd);

		a0 = vshrq_n_u32(a0, 16);
		a1 = vshrq_n_u32(a1, 16);

		uint16x8_t u8as16_0 = vdupq_n_u16(0);

		u8as16_0 = vmovnbq_u32(u8as16_0, a0);
		u8as16_0 = vmovntq_u32(u8as16_0, a1);

		/* Accurate scale next 8 pixels: v10_1 -> u8 in u16 lanes */
		uint32x4_t b0 = vmovlbq_u16(v10_1);
		uint32x4_t b1 = vmovltq_u16(v10_1);

		b0 = vaddq_u32(vmulq_n_u32(b0, K), rnd);
		b1 = vaddq_u32(vmulq_n_u32(b1, K), rnd);

		b0 = vshrq_n_u32(b0, 16);
		b1 = vshrq_n_u32(b1, 16);

		uint16x8_t u8as16_1 = vdupq_n_u16(0);

		u8as16_1 = vmovnbq_u32(u8as16_1, b0);
		u8as16_1 = vmovntq_u32(u8as16_1, b1);

		/* Pack two u16x8 (0..255) into one u8x16 */
		uint8x16_t out = vdupq_n_u8(0);

		out = vqmovnbq_u16(out, u8as16_0);
		out = vqmovntq_u16(out, u8as16_1);

		/* Store compacted RAW8 to the beginning of the same buffer */
		vst1q_u8(dst + i, out);
	}

	/* Tail (scalar, accurate) */
	for (; i < n_pixels; ++i) {
		uint16_t w = (uint16_t)src[2 * i] | ((uint16_t)src[2 * i + 1] << 8);
		uint16_t v10 = w & 0x03FFu;

#if CIMAGE_EXPOSURE_CALC
		/* Exposure analysis for tail pixels */
		if (v10 == 1023) {
			over++;
		}
		if (v10 == 0) {
			under++;
		}
		if (v10 >= THRESH_HIGH_10) {
			high++;
		}
		if (v10 < THRESH_LOW_10) {
			low++;
		}
#endif

		uint32_t t = (uint32_t)v10 * 16336u + (1u << 15);

		dst[i] = (uint8_t)(t >> 16);
	}

#if CIMAGE_EXPOSURE_CALC
	/* Store results in global variables */
	exposure_over_count = over;
	exposure_under_count = under;
	exposure_high_count = high;
	exposure_low_count = low;
#endif
}

#else /* !USE_MVE_RAW10_CONVERSION */

void raw10_gray16le_bytes_to_raw8_inplace_mve(uint8_t *buf, size_t n_pixels)
{
	uint8_t *dst = buf;
	const uint8_t *src = buf;

	/*
	 * Fixed-point scaling constant:
	 *   K = round(255 * 2^16 / 1023) = 16336
	 * Formula: out8 = (v10 * K + 2^15) >> 16
	 */
	const uint32_t K = 16336u;
	const uint32_t rnd = 1u << 15;

#if CIMAGE_EXPOSURE_CALC
	/* Exposure analysis thresholds (in RAW10 space: 0-1023) */
	const uint16_t THRESH_LOW_10 = 36;
	const uint16_t THRESH_HIGH_10 = 618;

	uint32_t under = 0, low = 0, high = 0, over = 0;
#endif

	for (size_t i = 0; i < n_pixels; ++i) {
		/* Read 16-bit little-endian value */
		uint16_t w = (uint16_t)src[2 * i] | ((uint16_t)src[2 * i + 1] << 8);
		/* Extract lower 10 bits */
		uint16_t v10 = w & 0x03FFu;

#if CIMAGE_EXPOSURE_CALC
		/* Exposure analysis on RAW10 data */
		if (v10 == 1023) {
			over++;
		}
		if (v10 == 0) {
			under++;
		}
		if (v10 >= THRESH_HIGH_10) {
			high++;
		}
		if (v10 < THRESH_LOW_10) {
			low++;
		}
#endif

		/* Scale to 8-bit with rounding */
		uint32_t t = (uint32_t)v10 * K + rnd;

		dst[i] = (uint8_t)(t >> 16);
	}

#if CIMAGE_EXPOSURE_CALC
	/* Store results in global variables */
	exposure_over_count = over;
	exposure_under_count = under;
	exposure_high_count = high;
	exposure_low_count = low;
#endif
}

#endif /* USE_MVE_RAW10_CONVERSION */

/* Exposure analysis counters */
uint32_t exposure_over_count, exposure_high_count, exposure_low_count, exposure_under_count;

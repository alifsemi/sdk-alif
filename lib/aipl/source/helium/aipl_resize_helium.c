/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/******************************************************************************
 * @file    aipl_resize_helium.c
 * @brief   Helium accelerated resize function implementations
 *
 ******************************************************************************/

/*********************
 *      INCLUDES
 *********************/
#include "aipl_resize_helium.h"

#include <stddef.h>

#include "aipl_mve_utils.h"

#ifdef AIPL_HELIUM_ACCELERATION

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
static aipl_error_t aipl_resize_sw_8bit_channels(const void *input, void *output, int input_pitch,
						 int input_width, int input_height,
						 int output_width, int output_height,
						 aipl_color_format_t format);
static aipl_error_t aipl_resize_sw_argb1555(const void *input, void *output, int input_pitch,
					    int input_width, int input_height, int output_width,
					    int output_height);
static aipl_error_t aipl_resize_sw_rgba5551(const void *input, void *output, int input_pitch,
					    int input_width, int input_height, int output_width,
					    int output_height);
#if !defined(AIPL_DAVE2D_ACCELERATION) || defined(AIPL_INCLUDE_ALL_HELIUM)
static aipl_error_t aipl_resize_sw_4bit_channels(const void *input, void *output, int input_pitch,
						 int input_width, int input_height,
						 int output_width, int output_height);
static aipl_error_t aipl_resize_sw_rgb565(const void *input, void *output, int input_pitch,
					  int input_width, int input_height, int output_width,
					  int output_height);
#endif

/**********************
 *      MACROS
 **********************/
#define INTERPOLATE_CHANNEL_MVE(PX0, PX1, NX_FRAC, X_FRAC)                                         \
	PX0 = vmulq(PX0, NX_FRAC);                                                                 \
	PX0 = vaddq(PX0, vmulq(PX1, X_FRAC));                                                      \
	PX0 = vrshrq(PX0, FRAC_BITS)

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
aipl_error_t aipl_resize_helium(const void *input, void *output, uint32_t pitch, uint32_t width,
				uint32_t height, aipl_color_format_t format, uint32_t output_width,
				uint32_t output_height, bool interpolate)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	switch (format) {
	/* Alpha color formats */
	case AIPL_COLOR_ALPHA8:
	/* RGB color formats */
	case AIPL_COLOR_ARGB8888:
	case AIPL_COLOR_RGBA8888:
	case AIPL_COLOR_RGB888:
	case AIPL_COLOR_BGR888:
		return aipl_resize_sw_8bit_channels(input, output, pitch, width, height,
						    output_width, output_height, format);
	case AIPL_COLOR_ARGB1555:
		return aipl_resize_sw_argb1555(input, output, pitch, width, height, output_width,
					       output_height);
	case AIPL_COLOR_RGBA5551:
		return aipl_resize_sw_rgba5551(input, output, pitch, width, height, output_width,
					       output_height);
#if !defined(AIPL_DAVE2D_ACCELERATION) || defined(AIPL_INCLUDE_ALL_HELIUM)
	case AIPL_COLOR_ARGB4444:
	case AIPL_COLOR_RGBA4444:
		return aipl_resize_sw_4bit_channels(input, output, pitch, width, height,
						    output_width, output_height);
	case AIPL_COLOR_RGB565:
		return aipl_resize_sw_rgb565(input, output, pitch, width, height, output_width,
					     output_height);
#endif

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

aipl_error_t aipl_resize_img_helium(const aipl_image_t *input, aipl_image_t *output,
				    bool interpolate)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	if (output->format != input->format) {
		return AIPL_ERR_FORMAT_MISMATCH;
	}

	return aipl_resize_helium(input->data, output->data, input->pitch, input->width,
				  input->height, input->format, output->width, output->height,
				  interpolate);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static aipl_error_t aipl_resize_sw_8bit_channels(const void *input, void *output, int input_pitch,
						 int input_width, int input_height,
						 int output_width, int output_height,
						 aipl_color_format_t format)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint32_t pixel_size_B = aipl_color_format_depth(format) / 8;

	const uint8_t *srcImage = (const uint8_t *)input;
	uint8_t *dstImage = (uint8_t *)output;

	const uint32x4_t bgr_offset = {0, 1, 2, 3};

/* Modified for BGR888 */
#define FRAC_BITS 14
	const int FRAC_VAL = (1 << FRAC_BITS);
	const int FRAC_MASK = (FRAC_VAL - 1);

	uint32_t src_x_accum, src_y_accum; /* accumulators and fractions for scaling the image */
	uint32_t x_frac, nx_frac, y_frac, ny_frac;
	int x, y, ty;

	if (input_height < 2) {
		return AIPL_ERR_FRAME_OUT_OF_RANGE;
	}

	/* start at 1/2 pixel in to account for integer downsampling which might miss pixels */
	src_y_accum = FRAC_VAL / 2;
	const uint32_t src_x_frac = (input_width * FRAC_VAL) / output_width;
	const uint32_t src_y_frac = (input_height * FRAC_VAL) / output_height;

	/* from here out, *3 b/c RGB */
	input_width *= pixel_size_B;
	input_pitch *= pixel_size_B;
	/* srcHeight not used for indexing */
	/* dstWidth still needed as is */
	/* dstHeight shouldn't be scaled */

	const uint8_t *s;
	uint8_t *d;

	for (y = 0; y < output_height; y++) {
		/* do indexing computations */
		ty = src_y_accum >> FRAC_BITS; /* src y */
		y_frac = src_y_accum & FRAC_MASK;
		src_y_accum += src_y_frac;
		ny_frac = FRAC_VAL - y_frac; /* y fraction and 1.0 - y fraction */

		s = &srcImage[ty * input_pitch];
		d = &dstImage[y * output_width * pixel_size_B]; /* not scaled above */
		/* start at 1/2 pixel in to account for integer downsampling which might miss pixels
		 */
		src_x_accum = FRAC_VAL / 2;
		for (x = 0; x < output_width; x++) {
			uint32_t tx;
			/* do indexing computations */
			tx = (src_x_accum >> FRAC_BITS) * pixel_size_B;
			x_frac = src_x_accum & FRAC_MASK;
			nx_frac = FRAC_VAL - x_frac; /* x fraction and 1.0 - x fraction */
			src_x_accum += src_x_frac;
			__builtin_prefetch(&s[tx + 64]);
			__builtin_prefetch(&s[tx + input_width + 64]);

			uint32x4_t p00 = vldrbq_u32(&s[tx]);
			uint32x4_t p10 = vldrbq_u32(&s[tx + pixel_size_B]);
			uint32x4_t p01 = vldrbq_u32(&s[tx + input_pitch]);
			uint32x4_t p11 = vldrbq_u32(&s[tx + input_pitch + pixel_size_B]);

			p00 = vmulq(p00, nx_frac);
			p00 = vmlaq_n_u32(p00, p10, x_frac);
			p00 = vrshrq(p00, FRAC_BITS);
			p01 = vmulq(p01, nx_frac);
			p01 = vmlaq_n_u32(p01, p11, x_frac);
			p01 = vrshrq(p01, FRAC_BITS);
			p00 = vmulq(p00, ny_frac);
			p00 = vmlaq_n_u32(p00, p01, y_frac);
			p00 = vrshrq(p00, FRAC_BITS);
			vstrbq_scatter_offset_p_u32(d, bgr_offset, p00, vctp32q(pixel_size_B));

			d += pixel_size_B;
		}
	}
	return AIPL_ERR_OK;
}

static aipl_error_t aipl_resize_sw_argb1555(const void *input, void *output, int input_pitch,
					    int input_width, int input_height, int output_width,
					    int output_height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint16_t *srcImage = (const uint16_t *)input;
	uint16_t *dstImage = (uint16_t *)output;

	uint32_t src_x_accum, src_y_accum; /* accumulators and fractions for scaling the image */
	int x, y, ty;

	if (input_height < 2) {
		return AIPL_ERR_FRAME_OUT_OF_RANGE;
	}

#undef FRAC_BITS
#define FRAC_BITS 10
	const int FRAC_VAL = (1 << FRAC_BITS);
	const int FRAC_MASK = (FRAC_VAL - 1);

	/* start at 1/2 pixel in to account for integer downsampling which might miss pixels */
	src_y_accum = FRAC_VAL / 2;
	const uint32_t src_x_frac = (input_width * FRAC_VAL) / output_width;
	const uint32_t src_y_frac = (input_height * FRAC_VAL) / output_height;

	const uint16_t *s;
	uint16_t *d;

	for (y = 0; y < output_height; y++) {
		/* do indexing computations */
		ty = src_y_accum >> FRAC_BITS; /* src y */
		src_y_accum += src_y_frac;

		s = &srcImage[ty * input_pitch];
		d = &dstImage[y * output_width]; /* not scaled above */
		/* start at 1/2 pixel in to account for integer downsampling which might miss pixels
		 */
		src_x_accum = FRAC_VAL / 2;

		for (x = 0; x < output_width; x += 8) {
			uint32x4_t offsets = vidupq_u32((uint32_t)0, 2);

			offsets = vmulq(offsets, src_x_frac);
			uint32x4_t tx0 = vaddq(vdupq_n_u32(src_x_accum), offsets);
			uint32x4_t tx1 = vaddq(vdupq_n_u32((src_x_accum + src_x_frac)), offsets);
			uint16x8_t x_frac = vmovntq(vreinterpretq_u16(tx0), tx1);

			x_frac = vandq(x_frac, vdupq_n_u16((uint16_t)FRAC_MASK));
			uint16x8_t nx_frac = vsubq(vdupq_n_u16((uint16_t)FRAC_VAL), x_frac);

			tx0 = vandq(vshrq(tx0, FRAC_BITS - 1), vdupq_n_u32(0xfffe));
			tx1 = vandq(vshrq(tx1, FRAC_BITS - 1), vdupq_n_u32(0xfffe));
			uint16x8_t tx00 = vmovntq(vreinterpretq_u16(tx0), tx1);
			uint16x8_t tx10 = vaddq(tx00, vdupq_n_u16(2));
			uint16x8_t tx01 = vaddq(tx00, vdupq_n_u16(input_pitch * 2));
			uint16x8_t tx11 = vaddq(tx01, vdupq_n_u16(2));

			uint16x8_t p00 = vldrhq_gather_offset_u16(s, tx00);
			uint16x8_t p10 = vldrhq_gather_offset_u16(s, tx10);
			uint16x8_t p01 = vldrhq_gather_offset_u16(s, tx01);
			uint16x8_t p11 = vldrhq_gather_offset_u16(s, tx11);

			uint16x8_t r00 = vandq(vshrq(p00, 10), vdupq_n_u16(0x1f));
			uint16x8_t r10 = vandq(vshrq(p10, 10), vdupq_n_u16(0x1f));
			uint16x8_t r01 = vandq(vshrq(p01, 10), vdupq_n_u16(0x1f));
			uint16x8_t r11 = vandq(vshrq(p11, 10), vdupq_n_u16(0x1f));
			uint16x8_t g00 = vandq(vshrq(p00, 5), vdupq_n_u16(0x1f));
			uint16x8_t g10 = vandq(vshrq(p10, 5), vdupq_n_u16(0x1f));
			uint16x8_t g01 = vandq(vshrq(p01, 5), vdupq_n_u16(0x1f));
			uint16x8_t g11 = vandq(vshrq(p11, 5), vdupq_n_u16(0x1f));
			uint16x8_t b00 = vandq(p00, vdupq_n_u16(0x1f));
			uint16x8_t b10 = vandq(p10, vdupq_n_u16(0x1f));
			uint16x8_t b01 = vandq(p01, vdupq_n_u16(0x1f));
			uint16x8_t b11 = vandq(p11, vdupq_n_u16(0x1f));

			INTERPOLATE_CHANNEL_MVE(r00, r10, nx_frac, x_frac);
			INTERPOLATE_CHANNEL_MVE(r01, r11, nx_frac, x_frac);
			INTERPOLATE_CHANNEL_MVE(r00, r01, nx_frac, x_frac);
			INTERPOLATE_CHANNEL_MVE(g00, g10, nx_frac, x_frac);
			INTERPOLATE_CHANNEL_MVE(g01, g11, nx_frac, x_frac);
			INTERPOLATE_CHANNEL_MVE(g00, g01, nx_frac, x_frac);
			INTERPOLATE_CHANNEL_MVE(b00, b10, nx_frac, x_frac);
			INTERPOLATE_CHANNEL_MVE(b01, b11, nx_frac, x_frac);
			INTERPOLATE_CHANNEL_MVE(b00, b01, nx_frac, x_frac);

			p00 = vandq(p00, vdupq_n_u16(0x8000));
			p00 = vorrq(p00, vshlq_n(r00, 10));
			p00 = vorrq(p00, vshlq_n(g00, 5));
			p00 = vorrq(p00, b00);

			vstrhq_p_u16(d, p00, vctp16q(output_width - x));

			d += 8;
			src_x_accum += src_x_frac * 8;
		}
	}
	return AIPL_ERR_OK;
}

static aipl_error_t aipl_resize_sw_rgba5551(const void *input, void *output, int input_pitch,
					    int input_width, int input_height, int output_width,
					    int output_height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint16_t *srcImage = (const uint16_t *)input;
	uint16_t *dstImage = (uint16_t *)output;

	uint32_t src_x_accum, src_y_accum; /* accumulators and fractions for scaling the image */
	int x, y, ty;

	if (input_height < 2) {
		return AIPL_ERR_FRAME_OUT_OF_RANGE;
	}

	const int FRAC_VAL = (1 << FRAC_BITS);
	const int FRAC_MASK = (FRAC_VAL - 1);

	/* start at 1/2 pixel in to account for integer downsampling which might miss pixels */
	src_y_accum = FRAC_VAL / 2;
	const uint32_t src_x_frac = (input_width * FRAC_VAL) / output_width;
	const uint32_t src_y_frac = (input_height * FRAC_VAL) / output_height;

	const uint16_t *s;
	uint16_t *d;

	for (y = 0; y < output_height; y++) {
		/* do indexing computations */
		ty = src_y_accum >> FRAC_BITS; /* src y */
		src_y_accum += src_y_frac;

		s = &srcImage[ty * input_pitch];
		d = &dstImage[y * output_width]; /* not scaled above */
		/* start at 1/2 pixel in to account for integer downsampling which might miss pixels
		 */
		src_x_accum = FRAC_VAL / 2;

		for (x = 0; x < output_width; x += 8) {
			uint32x4_t offsets = vidupq_u32((uint32_t)0, 2);

			offsets = vmulq(offsets, src_x_frac);
			uint32x4_t tx0 = vaddq(vdupq_n_u32(src_x_accum), offsets);
			uint32x4_t tx1 = vaddq(vdupq_n_u32((src_x_accum + src_x_frac)), offsets);
			uint16x8_t x_frac = vmovntq(vreinterpretq_u16(tx0), tx1);

			x_frac = vandq(x_frac, vdupq_n_u16((uint16_t)FRAC_MASK));
			uint16x8_t nx_frac = vsubq(vdupq_n_u16((uint16_t)FRAC_VAL), x_frac);

			tx0 = vandq(vshrq(tx0, FRAC_BITS - 1), vdupq_n_u32(0xfffe));
			tx1 = vandq(vshrq(tx1, FRAC_BITS - 1), vdupq_n_u32(0xfffe));
			uint16x8_t tx00 = vmovntq(vreinterpretq_u16(tx0), tx1);
			uint16x8_t tx10 = vaddq(tx00, vdupq_n_u16(2));
			uint16x8_t tx01 = vaddq(tx00, vdupq_n_u16(input_pitch * 2));
			uint16x8_t tx11 = vaddq(tx01, vdupq_n_u16(2));

			uint16x8_t p00 = vldrhq_gather_offset_u16(s, tx00);
			uint16x8_t p10 = vldrhq_gather_offset_u16(s, tx10);
			uint16x8_t p01 = vldrhq_gather_offset_u16(s, tx01);
			uint16x8_t p11 = vldrhq_gather_offset_u16(s, tx11);

			uint16x8_t r00 = vandq(vshrq(p00, 11), vdupq_n_u16(0x1f));
			uint16x8_t r10 = vandq(vshrq(p10, 11), vdupq_n_u16(0x1f));
			uint16x8_t r01 = vandq(vshrq(p01, 11), vdupq_n_u16(0x1f));
			uint16x8_t r11 = vandq(vshrq(p11, 11), vdupq_n_u16(0x1f));
			uint16x8_t g00 = vandq(vshrq(p00, 6), vdupq_n_u16(0x1f));
			uint16x8_t g10 = vandq(vshrq(p10, 6), vdupq_n_u16(0x1f));
			uint16x8_t g01 = vandq(vshrq(p01, 6), vdupq_n_u16(0x1f));
			uint16x8_t g11 = vandq(vshrq(p11, 6), vdupq_n_u16(0x1f));
			uint16x8_t b00 = vandq(vshrq(p00, 1), vdupq_n_u16(0x1f));
			uint16x8_t b10 = vandq(vshrq(p10, 1), vdupq_n_u16(0x1f));
			uint16x8_t b01 = vandq(vshrq(p01, 1), vdupq_n_u16(0x1f));
			uint16x8_t b11 = vandq(vshrq(p11, 1), vdupq_n_u16(0x1f));

			INTERPOLATE_CHANNEL_MVE(r00, r10, nx_frac, x_frac);
			INTERPOLATE_CHANNEL_MVE(r01, r11, nx_frac, x_frac);
			INTERPOLATE_CHANNEL_MVE(r00, r01, nx_frac, x_frac);
			INTERPOLATE_CHANNEL_MVE(g00, g10, nx_frac, x_frac);
			INTERPOLATE_CHANNEL_MVE(g01, g11, nx_frac, x_frac);
			INTERPOLATE_CHANNEL_MVE(g00, g01, nx_frac, x_frac);
			INTERPOLATE_CHANNEL_MVE(b00, b10, nx_frac, x_frac);
			INTERPOLATE_CHANNEL_MVE(b01, b11, nx_frac, x_frac);
			INTERPOLATE_CHANNEL_MVE(b00, b01, nx_frac, x_frac);

			p00 = vandq(p00, vdupq_n_u16(0x0001));
			p00 = vorrq(p00, vshlq_n(r00, 11));
			p00 = vorrq(p00, vshlq_n(g00, 6));
			p00 = vorrq(p00, vshlq_n(b00, 1));

			vstrhq_p_u16(d, p00, vctp16q(output_width - x));

			d += 8;
			src_x_accum += src_x_frac * 8;
		}
	}
	return AIPL_ERR_OK;
}

#if !defined(AIPL_DAVE2D_ACCELERATION) || defined(AIPL_INCLUDE_ALL_HELIUM)
static aipl_error_t aipl_resize_sw_4bit_channels(const void *input, void *output, int input_pitch,
						 int input_width, int input_height,
						 int output_width, int output_height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint16_t *srcImage = (const uint16_t *)input;
	uint16_t *dstImage = (uint16_t *)output;

	uint32_t src_x_accum, src_y_accum; /* accumulators and fractions for scaling the image */
	int x, y, ty;

	if (input_height < 2) {
		return AIPL_ERR_FRAME_OUT_OF_RANGE;
	}

	const int FRAC_VAL = (1 << FRAC_BITS);
	const int FRAC_MASK = (FRAC_VAL - 1);

	/* start at 1/2 pixel in to account for integer downsampling which might miss pixels */
	src_y_accum = FRAC_VAL / 2;
	const uint32_t src_x_frac = (input_width * FRAC_VAL) / output_width;
	const uint32_t src_y_frac = (input_height * FRAC_VAL) / output_height;

	const uint16_t *s;
	uint16_t *d;

	for (y = 0; y < output_height; y++) {
		/* do indexing computations */
		ty = src_y_accum >> FRAC_BITS; /* src y */
		src_y_accum += src_y_frac;

		s = &srcImage[ty * input_pitch];
		d = &dstImage[y * output_width]; /* not scaled above */
		/* start at 1/2 pixel in to account for integer downsampling which might miss pixels
		 */
		src_x_accum = FRAC_VAL / 2;

		for (x = 0; x < output_width; x += 8) {
			uint32x4_t offsets = vidupq_u32(0U, 2);

			offsets = vmulq(offsets, src_x_frac);
			uint32x4_t tx0 = vaddq(vdupq_n_u32(src_x_accum), offsets);
			uint32x4_t tx1 = vaddq(vdupq_n_u32((src_x_accum + src_x_frac)), offsets);
			uint16x8_t x_frac = vmovntq(vreinterpretq_u16(tx0), tx1);

			x_frac = vandq(x_frac, vdupq_n_u16((uint16_t)FRAC_MASK));
			uint16x8_t nx_frac = vsubq(vdupq_n_u16((uint16_t)FRAC_VAL), x_frac);

			tx0 = vandq(vshrq(tx0, FRAC_BITS - 1), vdupq_n_u32(0xfffe));
			tx1 = vandq(vshrq(tx1, FRAC_BITS - 1), vdupq_n_u32(0xfffe));
			uint16x8_t tx00 = vmovntq(vreinterpretq_u16(tx0), tx1);
			uint16x8_t tx10 = vaddq(tx00, vdupq_n_u16(2));
			uint16x8_t tx01 = vaddq(tx00, vdupq_n_u16(input_pitch * 2));
			uint16x8_t tx11 = vaddq(tx01, vdupq_n_u16(2));

			uint16x8_t p00 = vldrhq_gather_offset_u16(s, tx00);
			uint16x8_t p10 = vldrhq_gather_offset_u16(s, tx10);
			uint16x8_t p01 = vldrhq_gather_offset_u16(s, tx01);
			uint16x8_t p11 = vldrhq_gather_offset_u16(s, tx11);

			uint16x8_t a00 = vandq(vshrq(p00, 12), vdupq_n_u16(0x0f));
			uint16x8_t a10 = vandq(vshrq(p10, 12), vdupq_n_u16(0x0f));
			uint16x8_t a01 = vandq(vshrq(p01, 12), vdupq_n_u16(0x0f));
			uint16x8_t a11 = vandq(vshrq(p11, 12), vdupq_n_u16(0x0f));
			uint16x8_t r00 = vandq(vshrq(p00, 8), vdupq_n_u16(0x0f));
			uint16x8_t r10 = vandq(vshrq(p10, 8), vdupq_n_u16(0x0f));
			uint16x8_t r01 = vandq(vshrq(p01, 8), vdupq_n_u16(0x0f));
			uint16x8_t r11 = vandq(vshrq(p11, 8), vdupq_n_u16(0x0f));
			uint16x8_t g00 = vandq(vshrq(p00, 4), vdupq_n_u16(0x0f));
			uint16x8_t g10 = vandq(vshrq(p10, 4), vdupq_n_u16(0x0f));
			uint16x8_t g01 = vandq(vshrq(p01, 4), vdupq_n_u16(0x0f));
			uint16x8_t g11 = vandq(vshrq(p11, 4), vdupq_n_u16(0x0f));
			uint16x8_t b00 = vandq(p00, vdupq_n_u16(0x0f));
			uint16x8_t b10 = vandq(p10, vdupq_n_u16(0x0f));
			uint16x8_t b01 = vandq(p01, vdupq_n_u16(0x0f));
			uint16x8_t b11 = vandq(p11, vdupq_n_u16(0x0f));

			INTERPOLATE_CHANNEL_MVE(a00, a10, nx_frac, x_frac);
			INTERPOLATE_CHANNEL_MVE(a01, a11, nx_frac, x_frac);
			INTERPOLATE_CHANNEL_MVE(a00, a01, nx_frac, x_frac);
			INTERPOLATE_CHANNEL_MVE(r00, r10, nx_frac, x_frac);
			INTERPOLATE_CHANNEL_MVE(r01, r11, nx_frac, x_frac);
			INTERPOLATE_CHANNEL_MVE(r00, r01, nx_frac, x_frac);
			INTERPOLATE_CHANNEL_MVE(g00, g10, nx_frac, x_frac);
			INTERPOLATE_CHANNEL_MVE(g01, g11, nx_frac, x_frac);
			INTERPOLATE_CHANNEL_MVE(g00, g01, nx_frac, x_frac);
			INTERPOLATE_CHANNEL_MVE(b00, b10, nx_frac, x_frac);
			INTERPOLATE_CHANNEL_MVE(b01, b11, nx_frac, x_frac);
			INTERPOLATE_CHANNEL_MVE(b00, b01, nx_frac, x_frac);

			p00 = vshlq_n(a00, 12);
			p00 = vorrq(p00, vshlq_n(r00, 8));
			p00 = vorrq(p00, vshlq_n(g00, 4));
			p00 = vorrq(p00, b00);

			vstrhq_p_u16(d, p00, vctp16q(output_width - x));

			d += 8;
			src_x_accum += src_x_frac * 8;
		}
	}
	return AIPL_ERR_OK;
}

static aipl_error_t aipl_resize_sw_rgb565(const void *input, void *output, int input_pitch,
					  int input_width, int input_height, int output_width,
					  int output_height)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	const uint16_t *srcImage = (const uint16_t *)input;
	uint16_t *dstImage = (uint16_t *)output;

	uint32_t src_x_accum, src_y_accum; /* accumulators and fractions for scaling the image */
	int x, y, ty;

	if (input_height < 2) {
		return AIPL_ERR_FRAME_OUT_OF_RANGE;
	}

	const int FRAC_VAL = (1 << FRAC_BITS);
	const int FRAC_MASK = (FRAC_VAL - 1);

	/* start at 1/2 pixel in to account for integer downsampling which might miss pixels */
	src_y_accum = FRAC_VAL / 2;
	const uint32_t src_x_frac = (input_width * FRAC_VAL) / output_width;
	const uint32_t src_y_frac = (input_height * FRAC_VAL) / output_height;

	const uint16_t *s;
	uint16_t *d;

	for (y = 0; y < output_height; y++) {
		/* do indexing computations */
		ty = src_y_accum >> FRAC_BITS; /* src y */
		src_y_accum += src_y_frac;

		s = &srcImage[ty * input_pitch];
		d = &dstImage[y * output_width]; /* not scaled above */
		/* start at 1/2 pixel in to account for integer downsampling which might miss pixels
		 */
		src_x_accum = FRAC_VAL / 2;

		for (x = 0; x < output_width; x += 8) {
			uint32x4_t offsets = vidupq_u32(0U, 2);

			offsets = vmulq(offsets, src_x_frac);
			uint32x4_t tx0 = vaddq(vdupq_n_u32(src_x_accum), offsets);
			uint32x4_t tx1 = vaddq(vdupq_n_u32((src_x_accum + src_x_frac)), offsets);
			uint16x8_t x_frac = vmovntq(vreinterpretq_u16(tx0), tx1);

			x_frac = vandq(x_frac, vdupq_n_u16((uint16_t)FRAC_MASK));
			uint16x8_t nx_frac = vsubq(vdupq_n_u16((uint16_t)FRAC_VAL), x_frac);

			tx0 = vandq(vshrq(tx0, FRAC_BITS - 1), vdupq_n_u32(0xfffe));
			tx1 = vandq(vshrq(tx1, FRAC_BITS - 1), vdupq_n_u32(0xfffe));
			uint16x8_t tx00 = vmovntq(vreinterpretq_u16(tx0), tx1);
			uint16x8_t tx10 = vaddq(tx00, vdupq_n_u16(2));
			uint16x8_t tx01 = vaddq(tx00, vdupq_n_u16(input_pitch * 2));
			uint16x8_t tx11 = vaddq(tx01, vdupq_n_u16(2));

			uint16x8_t p00 = vldrhq_gather_offset_u16(s, tx00);
			uint16x8_t p10 = vldrhq_gather_offset_u16(s, tx10);
			uint16x8_t p01 = vldrhq_gather_offset_u16(s, tx01);
			uint16x8_t p11 = vldrhq_gather_offset_u16(s, tx11);

			uint16x8_t r00 = vandq(vshrq(p00, 11), vdupq_n_u16(0x1f));
			uint16x8_t r10 = vandq(vshrq(p10, 11), vdupq_n_u16(0x1f));
			uint16x8_t r01 = vandq(vshrq(p01, 11), vdupq_n_u16(0x1f));
			uint16x8_t r11 = vandq(vshrq(p11, 11), vdupq_n_u16(0x1f));
			uint16x8_t g00 = vandq(vshrq(p00, 5), vdupq_n_u16(0x3f));
			uint16x8_t g10 = vandq(vshrq(p10, 5), vdupq_n_u16(0x3f));
			uint16x8_t g01 = vandq(vshrq(p01, 5), vdupq_n_u16(0x3f));
			uint16x8_t g11 = vandq(vshrq(p11, 5), vdupq_n_u16(0x3f));
			uint16x8_t b00 = vandq(p00, vdupq_n_u16(0x1f));
			uint16x8_t b10 = vandq(p10, vdupq_n_u16(0x1f));
			uint16x8_t b01 = vandq(p01, vdupq_n_u16(0x1f));
			uint16x8_t b11 = vandq(p11, vdupq_n_u16(0x1f));

			INTERPOLATE_CHANNEL_MVE(r00, r10, nx_frac, x_frac);
			INTERPOLATE_CHANNEL_MVE(r01, r11, nx_frac, x_frac);
			INTERPOLATE_CHANNEL_MVE(r00, r01, nx_frac, x_frac);
			INTERPOLATE_CHANNEL_MVE(g00, g10, nx_frac, x_frac);
			INTERPOLATE_CHANNEL_MVE(g01, g11, nx_frac, x_frac);
			INTERPOLATE_CHANNEL_MVE(g00, g01, nx_frac, x_frac);
			INTERPOLATE_CHANNEL_MVE(b00, b10, nx_frac, x_frac);
			INTERPOLATE_CHANNEL_MVE(b01, b11, nx_frac, x_frac);
			INTERPOLATE_CHANNEL_MVE(b00, b01, nx_frac, x_frac);

			p00 = vshlq_n(r00, 11);
			p00 = vorrq(p00, vshlq_n(g00, 5));
			p00 = vorrq(p00, b00);

			vstrhq_p_u16(d, p00, vctp16q(output_width - x));

			d += 8;
			src_x_accum += src_x_frac * 8;
		}
	}
	return AIPL_ERR_OK;
}
#endif

#endif

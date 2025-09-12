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
 * @file    aipl_resize.c
 * @brief   Default resize function implementations
 *
 ******************************************************************************/

/*********************
 *      INCLUDES
 *********************/
#include "aipl_resize_default.h"

#include <stddef.h>

#if !defined(AIPL_HELIUM_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT)

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
#if !defined(AIPL_DAVE2D_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT)
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
#define INTERPOLATE_CHANNEL(PX0, PX1, NX_FRAC, X_FRAC)                                             \
	PX0 = ((PX0 * NX_FRAC) + (PX1 * X_FRAC) + FRAC_VAL / 2) >> FRAC_BITS

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
aipl_error_t aipl_resize_default(const void *input, void *output, uint32_t pitch, uint32_t width,
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
#if !defined(AIPL_DAVE2D_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT)
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

aipl_error_t aipl_resize_img_default(const aipl_image_t *input, aipl_image_t *output,
				     bool interpolate)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	if (output->format != input->format) {
		return AIPL_ERR_FORMAT_MISMATCH;
	}

	return aipl_resize_default(input->data, output->data, input->pitch, input->width,
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

/* Modified for BGR888 */
#define FRAC_BITS 14
	const int FRAC_VAL = (1 << FRAC_BITS);
	const int FRAC_MASK = (FRAC_VAL - 1);

	uint32_t src_x_accum, src_y_accum; /* accumulators and fractions for scaling the image */
	uint32_t x_frac, nx_frac;
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
		src_y_accum += src_y_frac;

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
			__builtin_prefetch(&s[tx + input_pitch + 64]);

			/* interpolate each color channel */
			for (int color = 0; color < pixel_size_B; color++) {
				uint32_t p00, p01, p10, p11;

				p00 = s[tx];
				p10 = s[tx + pixel_size_B];
				p01 = s[tx + input_pitch];
				p11 = s[tx + input_pitch + pixel_size_B];
				INTERPOLATE_CHANNEL(p00, p10, nx_frac, x_frac);
				INTERPOLATE_CHANNEL(p01, p11, nx_frac, x_frac);
				INTERPOLATE_CHANNEL(p00, p01, nx_frac, x_frac);
				*d++ = (uint8_t)p00; /* store new pixel */
				/* ready next loop */
				tx++;
			}
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
	uint32_t x_frac, nx_frac;
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
		for (x = 0; x < output_width; x++) {
			uint32_t tx;
			/* do indexing computations */
			tx = (src_x_accum >> FRAC_BITS);
			x_frac = src_x_accum & FRAC_MASK;
			nx_frac = FRAC_VAL - x_frac; /* x fraction and 1.0 - x fraction */
			src_x_accum += src_x_frac;
			__builtin_prefetch(&s[tx + 64]);
			__builtin_prefetch(&s[tx + input_pitch + 64]);

			uint32_t p00, p01, p10, p11;

			p00 = s[tx];
			p10 = s[tx + 1];
			p01 = s[tx + input_pitch];
			p11 = s[tx + input_pitch + 1];

			uint16_t r00 = (p00 >> 10) & 0x1f;
			uint16_t r10 = (p10 >> 10) & 0x1f;
			uint16_t r01 = (p01 >> 10) & 0x1f;
			uint16_t r11 = (p11 >> 10) & 0x1f;
			uint16_t g00 = (p00 >> 5) & 0x1f;
			uint16_t g10 = (p10 >> 5) & 0x1f;
			uint16_t g01 = (p01 >> 5) & 0x1f;
			uint16_t g11 = (p11 >> 5) & 0x1f;
			uint16_t b00 = p00 & 0x1f;
			uint16_t b10 = p10 & 0x1f;
			uint16_t b01 = p01 & 0x1f;
			uint16_t b11 = p11 & 0x1f;

			INTERPOLATE_CHANNEL(r00, r10, nx_frac, x_frac);
			INTERPOLATE_CHANNEL(r01, r11, nx_frac, x_frac);
			INTERPOLATE_CHANNEL(r00, r01, nx_frac, x_frac);
			INTERPOLATE_CHANNEL(g00, g10, nx_frac, x_frac);
			INTERPOLATE_CHANNEL(g01, g11, nx_frac, x_frac);
			INTERPOLATE_CHANNEL(g00, g01, nx_frac, x_frac);
			INTERPOLATE_CHANNEL(b00, b10, nx_frac, x_frac);
			INTERPOLATE_CHANNEL(b01, b11, nx_frac, x_frac);
			INTERPOLATE_CHANNEL(b00, b01, nx_frac, x_frac);

			p00 = (p00 & 0x8000) | (r00 << 10) | (g00 << 5) | b00;

			*d++ = (uint16_t)p00; /* store new pixel */
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
	uint32_t x_frac, nx_frac;
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

		for (x = 0; x < output_width; x++) {
			uint32_t tx;
			/* do indexing computations */
			tx = (src_x_accum >> FRAC_BITS);
			x_frac = src_x_accum & FRAC_MASK;
			nx_frac = FRAC_VAL - x_frac; /* x fraction and 1.0 - x fraction */
			src_x_accum += src_x_frac;
			__builtin_prefetch(&s[tx + 64]);
			__builtin_prefetch(&s[tx + input_pitch + 64]);

			uint32_t p00, p01, p10, p11;

			p00 = s[tx];
			p10 = s[tx + 1];
			p01 = s[tx + input_pitch];
			p11 = s[tx + input_pitch + 1];

			uint16_t r00 = (p00 >> 11) & 0x1f;
			uint16_t r10 = (p10 >> 11) & 0x1f;
			uint16_t r01 = (p01 >> 11) & 0x1f;
			uint16_t r11 = (p11 >> 11) & 0x1f;
			uint16_t g00 = (p00 >> 6) & 0x1f;
			uint16_t g10 = (p10 >> 6) & 0x1f;
			uint16_t g01 = (p01 >> 6) & 0x1f;
			uint16_t g11 = (p11 >> 6) & 0x1f;
			uint16_t b00 = (p00 >> 1) & 0x1f;
			uint16_t b10 = (p10 >> 1) & 0x1f;
			uint16_t b01 = (p01 >> 1) & 0x1f;
			uint16_t b11 = (p11 >> 1) & 0x1f;

			INTERPOLATE_CHANNEL(r00, r10, nx_frac, x_frac);
			INTERPOLATE_CHANNEL(r01, r11, nx_frac, x_frac);
			INTERPOLATE_CHANNEL(r00, r01, nx_frac, x_frac);
			INTERPOLATE_CHANNEL(g00, g10, nx_frac, x_frac);
			INTERPOLATE_CHANNEL(g01, g11, nx_frac, x_frac);
			INTERPOLATE_CHANNEL(g00, g01, nx_frac, x_frac);
			INTERPOLATE_CHANNEL(b00, b10, nx_frac, x_frac);
			INTERPOLATE_CHANNEL(b01, b11, nx_frac, x_frac);
			INTERPOLATE_CHANNEL(b00, b01, nx_frac, x_frac);

			p00 = (p00 & 0x0001) | (r00 << 11) | (g00 << 6) | (b00 << 1);

			*d++ = (uint16_t)p00; /* store new pixel */
		}
	}
	return AIPL_ERR_OK;
}

#if !defined(AIPL_DAVE2D_ACCELERATION) || defined(AIPL_INCLUDE_ALL_DEFAULT)
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
	uint32_t x_frac, nx_frac;
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

		for (x = 0; x < output_width; x++) {
			uint32_t tx;
			/* do indexing computations */
			tx = (src_x_accum >> FRAC_BITS);
			x_frac = src_x_accum & FRAC_MASK;
			nx_frac = FRAC_VAL - x_frac; /* x fraction and 1.0 - x fraction */
			src_x_accum += src_x_frac;
			__builtin_prefetch(&s[tx + 64]);
			__builtin_prefetch(&s[tx + input_pitch + 64]);

			uint32_t p00, p01, p10, p11;

			p00 = s[tx];
			p10 = s[tx + 1];
			p01 = s[tx + input_pitch];
			p11 = s[tx + input_pitch + 1];

			uint16_t a00 = (p00 >> 12) & 0x0f;
			uint16_t a10 = (p10 >> 12) & 0x0f;
			uint16_t a01 = (p01 >> 12) & 0x0f;
			uint16_t a11 = (p11 >> 12) & 0x0f;
			uint16_t r00 = (p00 >> 8) & 0x0f;
			uint16_t r10 = (p10 >> 8) & 0x0f;
			uint16_t r01 = (p01 >> 8) & 0x0f;
			uint16_t r11 = (p11 >> 8) & 0x0f;
			uint16_t g00 = (p00 >> 4) & 0x0f;
			uint16_t g10 = (p10 >> 4) & 0x0f;
			uint16_t g01 = (p01 >> 4) & 0x0f;
			uint16_t g11 = (p11 >> 4) & 0x0f;
			uint16_t b00 = p00 & 0x0f;
			uint16_t b10 = p10 & 0x0f;
			uint16_t b01 = p01 & 0x0f;
			uint16_t b11 = p11 & 0x0f;

			INTERPOLATE_CHANNEL(a00, a10, nx_frac, x_frac);
			INTERPOLATE_CHANNEL(a01, a11, nx_frac, x_frac);
			INTERPOLATE_CHANNEL(a00, a01, nx_frac, x_frac);
			INTERPOLATE_CHANNEL(r00, r10, nx_frac, x_frac);
			INTERPOLATE_CHANNEL(r01, r11, nx_frac, x_frac);
			INTERPOLATE_CHANNEL(r00, r01, nx_frac, x_frac);
			INTERPOLATE_CHANNEL(g00, g10, nx_frac, x_frac);
			INTERPOLATE_CHANNEL(g01, g11, nx_frac, x_frac);
			INTERPOLATE_CHANNEL(g00, g01, nx_frac, x_frac);
			INTERPOLATE_CHANNEL(b00, b10, nx_frac, x_frac);
			INTERPOLATE_CHANNEL(b01, b11, nx_frac, x_frac);
			INTERPOLATE_CHANNEL(b00, b01, nx_frac, x_frac);

			p00 = (a00 << 12) | (r00 << 8) | (g00 << 4) | b00;

			*d++ = (uint16_t)p00; /* store new pixel */
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
	uint32_t x_frac, nx_frac;
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

		for (x = 0; x < output_width; x++) {
			uint32_t tx;
			/* do indexing computations */
			tx = (src_x_accum >> FRAC_BITS);
			x_frac = src_x_accum & FRAC_MASK;
			nx_frac = FRAC_VAL - x_frac; /* x fraction and 1.0 - x fraction */
			src_x_accum += src_x_frac;
			__builtin_prefetch(&s[tx + 64]);
			__builtin_prefetch(&s[tx + input_pitch + 64]);

			uint32_t p00, p01, p10, p11;

			p00 = s[tx];
			p10 = s[tx + 1];
			p01 = s[tx + input_pitch];
			p11 = s[tx + input_pitch + 1];

			uint16_t r00 = (p00 >> 11) & 0x1f;
			uint16_t r10 = (p10 >> 11) & 0x1f;
			uint16_t r01 = (p01 >> 11) & 0x1f;
			uint16_t r11 = (p11 >> 11) & 0x1f;
			uint16_t g00 = (p00 >> 5) & 0x3f;
			uint16_t g10 = (p10 >> 5) & 0x3f;
			uint16_t g01 = (p01 >> 5) & 0x3f;
			uint16_t g11 = (p11 >> 5) & 0x3f;
			uint16_t b00 = p00 & 0x1f;
			uint16_t b10 = p10 & 0x1f;
			uint16_t b01 = p01 & 0x1f;
			uint16_t b11 = p11 & 0x1f;

			INTERPOLATE_CHANNEL(r00, r10, nx_frac, x_frac);
			INTERPOLATE_CHANNEL(r01, r11, nx_frac, x_frac);
			INTERPOLATE_CHANNEL(r00, r01, nx_frac, x_frac);
			INTERPOLATE_CHANNEL(g00, g10, nx_frac, x_frac);
			INTERPOLATE_CHANNEL(g01, g11, nx_frac, x_frac);
			INTERPOLATE_CHANNEL(g00, g01, nx_frac, x_frac);
			INTERPOLATE_CHANNEL(b00, b10, nx_frac, x_frac);
			INTERPOLATE_CHANNEL(b01, b11, nx_frac, x_frac);
			INTERPOLATE_CHANNEL(b00, b01, nx_frac, x_frac);

			p00 = (r00 << 11) | (g00 << 5) | b00;

			*d++ = (uint16_t)p00; /* store new pixel */
		}
	}
	return AIPL_ERR_OK;
}
#endif

#endif

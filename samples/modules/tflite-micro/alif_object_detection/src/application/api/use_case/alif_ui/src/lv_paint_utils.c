/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#if defined __clang__ || defined __GNUC__
#pragma GCC diagnostic ignored "-Wvla"
#endif

#include "lvgl.h"

#define RGB_BYTES	3
#define RGB565_BYTES	2

#if defined __ARMCC_VERSION && (__ARM_FEATURE_MVE & 1)
#define ENABLE_MVE_WRITE 1
#else
#define ENABLE_MVE_WRITE 0
#endif
#define ENABLE_WORD_WRITE 1

#if __ARM_FEATURE_MVE & 1
#include <arm_mve.h>
#endif

#if !(LV_COLOR_DEPTH == 16)
#error "Unsupported LV_COLOR_DEPTH"
#endif

#if LV_COLOR_DEPTH == 16
/* Convert RGB888 to RGB565 */
/* Width needs to be divisible by 16 */
/* src_ptr and dst_ptr are assumed 32-bit aligned */
void write_rgb888_to_rgb565_buf(int width, int height,
				const uint8_t *restrict src_ptr,
				uint16_t *restrict dst_ptr)
{
	const uint8_t (*src)[width][RGB_BYTES] = (const uint8_t (*)[width][RGB_BYTES])src_ptr;
	uint16_t (*dst)[width] = (uint16_t (*)[width])dst_ptr;

	for (int y = 0; y < height; y++) {
#if ENABLE_MVE_WRITE && __ARM_FEATURE_MVE & 1
		const uint8x16_t inc3 = vmulq_n_u8(vidupq_n_u8(0, 1), 3);
		const uint8_t *restrict srcp = src[y][0];
		uint8_t *restrict dstp = (uint8_t *)dst[y];

		for (int x = 0; x < width; x += 16) {
			uint8x16_t r = vldrbq_gather_offset(srcp + 0, inc3);
			uint8x16_t g = vldrbq_gather_offset(srcp + 1, inc3);
			uint8x16_t b = vldrbq_gather_offset(srcp + 2, inc3);

			srcp += 16 * RGB_BYTES;
			/* RGB565: RRRRRGGG GGGBBBBB (high byte first) */
			uint8x16x2_t out = { vsriq(vshlq_n_u8(g, 3), b, 3),
					    vsriq(r, g, 5) };

			vst2q(dstp, out);
			dstp += 16 * RGB565_BYTES;
		}
#elif ENABLE_WORD_WRITE
		const uint8_t *restrict srcp = src[y][0];
		uint16_t *restrict dstp = dst[y];
		const uint32_t *srcp32 = (const uint32_t *)srcp;
		uint32_t *dstp32 = (uint32_t *)dstp;

		/* Load 4 RGB888 pixels as 3 words, and convert to 2 words of RGB565 */
		for (int x = 0; x < width; x += 4) {
			uint32_t r1b0g0r0 = *srcp32++;
			uint32_t g2r2b1g1 = *srcp32++;
			uint32_t b3g3r3b2 = *srcp32++;

			*dstp32++ = (r1b0g0r0         & 0xf8000000) |
				    ((g2r2b1g1 << 19) & 0x07e00000) |
				    ((g2r2b1g1 << 5)  & 0x001f0000) |
				    ((r1b0g0r0 << 8)  & 0x0000f800) |
				    ((r1b0g0r0 >> 5)  & 0x000007e0) |
				    ((r1b0g0r0 >> 19) & 0x0000001f);
			*dstp32++ = ((b3g3r3b2 << 16) & 0xf8000000) |
				    ((b3g3r3b2 << 3)  & 0x07e00000) |
				    ((b3g3r3b2 >> 11) & 0x001f0000) |
				    ((g2r2b1g1 >> 8)  & 0x0000f800) |
				    ((g2r2b1g1 >> 21) & 0x000007e0) |
				    ((b3g3r3b2 >> 3)  & 0x0000001f);
		}
#else
		for (int x = 0; x < width; x++) {
			uint8_t r = src[y][x][0];
			uint8_t g = src[y][x][1];
			uint8_t b = src[y][x][2];
			/* RGB565: RRRRRGGG GGGBBBBB */
			dst[y][x] = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
		}
#endif
	}
}
#endif

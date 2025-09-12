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
 * @file    aipl_rotate_helium.c
 * @brief   Helium accelerated rotate function implementations
 *
 ******************************************************************************/

/*********************
 *      INCLUDES
 *********************/
#include "aipl_rotate_helium.h"

#include <stddef.h>

#include "aipl_arm_mve.h"
#include "aipl_cache.h"

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

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
aipl_error_t aipl_rotate_helium(const void *input, void *output, uint32_t pitch, uint32_t width,
				uint32_t height, aipl_color_format_t format,
				aipl_rotation_t rotation)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	if (format >= AIPL_COLOR_YV12) {
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}

	const int rgbBytes = aipl_color_format_depth(format) / 8;
	int x, y;

	if (rotation == AIPL_ROTATE_90) {
		for (y = 0; y < height; ++y) {
			const uint8_t *input_offset = (uint8_t *)input + y * pitch * rgbBytes;
			uint8_t *output_offset = (uint8_t *)output + (height - y - 1) * rgbBytes;

			for (x = 0; x < width; x += 8) {
				mve_pred16_t tail_p = vctp16q(width - x);

				uint16x8_t off_i = vidupq_n_u16(0, 1);

				off_i = vmulq_n_u16(off_i, rgbBytes);

				uint16x8_t off_o = vidupq_n_u16(0, 1);

				off_o = vmulq_n_u16(off_o, height * rgbBytes);

				for (int i = 0; i < rgbBytes; ++i) {
					uint16x8_t channel = vldrbq_gather_offset_z(
						input_offset + x * rgbBytes + i, off_i, tail_p);

					vstrbq_scatter_offset_p(output_offset +
									x * height * rgbBytes + i,
								off_o, channel, tail_p);
				}
			}
		}
	} else if (rotation == AIPL_ROTATE_180) {
		for (y = 0; y < height; ++y) {
			const uint8_t *input_offset = (uint8_t *)input + y * pitch * rgbBytes;
			uint8_t *output_offset =
				(uint8_t *)output + (height - y - 1) * width * rgbBytes;

			for (x = 0; x < width; x += 8) {
				mve_pred16_t tail_p = vctp16q(width - x);

				uint16x8_t off_i = vidupq_n_u16(0, 1);

				off_i = vmulq_n_u16(off_i, rgbBytes);

#if defined(__ARMCC_VERSION) || (GCC_VERSION >= 120300)
				uint16x8_t off_o =
					vcreateq_u16(0x0004000500060007, 0x0000000100020003);
#else
				uint16x8_t off_o =
					vcreateq_u16(0x0000000100020003, 0x0004000500060007);
#endif
				off_o = vmulq_n_u16(off_o, rgbBytes);

				for (int i = 0; i < rgbBytes; ++i) {
					uint16x8_t channel = vldrbq_gather_offset_z(
						input_offset + x * rgbBytes + i, off_i, tail_p);

					vstrbq_scatter_offset_p(
						output_offset + (width - x - 8) * rgbBytes + i,
						off_o, channel, tail_p);
				}
			}
		}
	} else if (rotation == AIPL_ROTATE_270) {
		for (y = 0; y < height; ++y) {
			const uint8_t *input_offset = (uint8_t *)input + y * pitch * rgbBytes;
			uint8_t *output_offset = (uint8_t *)output + y * rgbBytes;

			for (x = 0; x < width; x += 8) {
				mve_pred16_t tail_p = vctp16q(width - x);

				uint16x8_t off_i = vidupq_n_u16(0, 1);

				off_i = vmulq_n_u16(off_i, rgbBytes);

#if defined(__ARMCC_VERSION) || (GCC_VERSION >= 120300)
				uint16x8_t off_o =
					vcreateq_u16(0x0004000500060007, 0x0000000100020003);
#else
				uint16x8_t off_o =
					vcreateq_u16(0x0000000100020003, 0x0004000500060007);
#endif
				off_o = vmulq_n_u16(off_o, height * rgbBytes);

				for (int i = 0; i < rgbBytes; ++i) {
					uint16x8_t channel = vldrbq_gather_offset_z(
						input_offset + x * rgbBytes + i, off_i, tail_p);

					vstrbq_scatter_offset_p(
						output_offset +
							(width - x - 8) * height * rgbBytes + i,
						off_o, channel, tail_p);
				}
			}
		}
	} else {
		return AIPL_ERR_NOT_SUPPORTED;
	}

	size_t size = width * height * rgbBytes;

	aipl_cpu_cache_clean(output, size);

	return AIPL_ERR_OK;
}

aipl_error_t aipl_rotate_img_helium(const aipl_image_t *input, aipl_image_t *output,
				    aipl_rotation_t rotation)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	if (output->format != input->format) {
		return AIPL_ERR_FORMAT_MISMATCH;
	}

	if ((rotation == AIPL_ROTATE_0 || rotation == AIPL_ROTATE_180) &&
	    (input->width != output->width || input->height != output->height)) {
		return AIPL_ERR_SIZE_MISMATCH;
	}

	if ((rotation == AIPL_ROTATE_90 || rotation == AIPL_ROTATE_270) &&
	    (input->width != output->height || input->height != output->width)) {
		return AIPL_ERR_SIZE_MISMATCH;
	}

	return aipl_rotate_helium(input->data, output->data, input->pitch, input->width,
				  input->height, input->format, rotation);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif

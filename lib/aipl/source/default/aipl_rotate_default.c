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
 * @file    aipl_rotate_default.c
 * @brief   Default rotate function implementations
 *
 ******************************************************************************/

/*********************
 *      INCLUDES
 *********************/
#include "aipl_rotate_default.h"

#include <stddef.h>

#include "aipl_cache.h"

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

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
aipl_error_t aipl_rotate_default(const void *input, void *output, uint32_t pitch, uint32_t width,
				 uint32_t height, aipl_color_format_t format,
				 aipl_rotation_t rotation)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	if (format >= AIPL_COLOR_YV12) {
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}

	const uint8_t *src = input;
	uint8_t *dst = output;

	const int rgbBytes = aipl_color_format_depth(format) / 8;
	int x, y, j;

	if (rotation == AIPL_ROTATE_90) {
		for (y = 0; y < height; ++y) {
			for (x = 0; x < width; ++x) {
				int src_offset = (y * pitch + x) * rgbBytes;
				int dst_offset = (x * height + height - 1 - y) * rgbBytes;

				for (j = 0; j < rgbBytes; j++) {
					dst[dst_offset++] = src[src_offset++];
				}
			}
		}
	} else if (rotation == AIPL_ROTATE_180) {
		for (y = 0; y < height; ++y) {
			for (x = 0; x < width; ++x) {
				int src_offset = (y * pitch + x) * rgbBytes;
				int dst_offset =
					((height - 1 - y) * width + width - 1 - x) * rgbBytes;

				for (j = 0; j < rgbBytes; j++) {
					dst[dst_offset++] = src[src_offset++];
				}
			}
		}
	} else if (rotation == AIPL_ROTATE_270) {
		for (y = 0; y < height; ++y) {
			for (x = 0; x < width; ++x) {
				int src_offset = (y * pitch + x) * rgbBytes;
				int dst_offset = ((width - 1 - x) * height + y) * rgbBytes;

				for (j = 0; j < rgbBytes; j++) {
					dst[dst_offset++] = src[src_offset++];
				}
			}
		}
	} else {
		return AIPL_ERR_NOT_SUPPORTED;
	}

	size_t size = width * height * rgbBytes;

	aipl_cpu_cache_clean(dst, size);

	return AIPL_ERR_OK;
}

aipl_error_t aipl_rotate_img_default(const aipl_image_t *input, aipl_image_t *output,
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

	return aipl_rotate_default(input->data, output->data, input->pitch, input->width,
				   input->height, input->format, rotation);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif

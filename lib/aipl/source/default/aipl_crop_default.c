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
 * @file    aipl_crop_default.c
 * @brief   Default crop function implementations
 *
 ******************************************************************************/

/*********************
 *      INCLUDES
 *********************/
#include "aipl_crop_default.h"

#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "aipl_config.h"
#include "aipl_cache.h"

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
aipl_error_t aipl_crop_default(const void *input, void *output, uint32_t pitch, uint32_t width,
			       uint32_t height, aipl_color_format_t format, uint32_t left,
			       uint32_t top, uint32_t right, uint32_t bottom)
{
	/* Check pointers */
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	if (format >= AIPL_COLOR_YV12) {
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}

	/* Checking the boundary */
	if ((left > right) || (right > width) || (top > bottom) || (bottom > height)) {
		return AIPL_ERR_FRAME_OUT_OF_RANGE;
	}

	uint32_t bpp = aipl_color_format_depth(format);

	/* Check for no cropping */
	if (left == 0 && top == 0 && right == width && bottom == height && pitch == width) {
		/* No-op if cropping and in-place */
		size_t size = width * height * (bpp / 8);

		if (input != output) {
			memcpy(output, input, size);
		}
		aipl_cpu_cache_clean(output, size);
		return AIPL_ERR_OK;
	}

	/* Updating the input frame column start */
	uint8_t *ip_fb = (uint8_t *)input + (top * pitch * (bpp / 8));
	uint8_t *op_fb = (uint8_t *)output;

	uint32_t new_width = right - left;
	uint32_t new_hight = bottom - top;

	for (uint32_t i = 0; i < new_hight; ++i) {
		/* Update row address */
		const uint8_t *ip_fb_row = ip_fb + left * (bpp / 8);

		memmove(op_fb, ip_fb_row, new_width * (bpp / 8));

		/* Update fb */
		ip_fb += (pitch * (bpp / 8));
		op_fb += (new_width * (bpp / 8));
	}

	size_t size = new_width * new_hight * (bpp / 8);

	aipl_cpu_cache_clean(output, size);

	return AIPL_ERR_OK;
}

aipl_error_t aipl_crop_img_default(const aipl_image_t *input, aipl_image_t *output, uint32_t left,
				   uint32_t top, uint32_t right, uint32_t bottom)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	uint32_t new_width = right - left;
	uint32_t new_hight = bottom - top;

	if (new_width != output->width || new_hight != output->height) {
		return AIPL_ERR_SIZE_MISMATCH;
	}

	if (output->format != input->format) {
		return AIPL_ERR_FORMAT_MISMATCH;
	}

	return aipl_crop_default(input->data, output->data, input->pitch, input->width,
				 input->height, input->format, left, top, right, bottom);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

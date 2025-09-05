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
 * @file    aipl_crop_dave2d.c
 * @brief   D/AVE2D accelerated crop function implementations
 *
 ******************************************************************************/

/*********************
 *      INCLUDES
 *********************/
#include "aipl_crop_dave2d.h"

#include <stddef.h>

#include "aipl_config.h"
#include "aipl_dave2d.h"

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
aipl_error_t aipl_crop_dave2d(const void *input, void *output, uint32_t pitch, uint32_t width,
			      uint32_t height, aipl_color_format_t format, uint32_t left,
			      uint32_t top, uint32_t right, uint32_t bottom)
{
	/* Check pointers */
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	if (aipl_dave2d_check_output_format(format)) {
		uint32_t x = left;
		uint32_t y = top;
		uint32_t output_width = right - left;
		uint32_t output_height = bottom - top;

		d2_u32 ret = aipl_dave2d_texturing(
			input, output, pitch, width, height, aipl_dave2d_format_to_mode(format),
			output_width, output_height, -x, -y, 0, false, false, false, false);

		return aipl_dave2d_error_convert(ret);
	}

	return AIPL_ERR_UNSUPPORTED_FORMAT;
}

aipl_error_t aipl_crop_img_dave2d(const aipl_image_t *input, aipl_image_t *output, uint32_t left,
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

	return aipl_crop_dave2d(input->data, output->data, input->pitch, input->width,
				input->height, input->format, left, top, right, bottom);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

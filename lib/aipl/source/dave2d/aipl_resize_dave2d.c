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
 * @file    aipl_resize_dave2d.c
 * @brief   D/AVE2D accelerated resize function implementations
 *
 ******************************************************************************/

/*********************
 *      INCLUDES
 *********************/
#include "aipl_resize_dave2d.h"

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
aipl_error_t aipl_resize_dave2d(const void *input, void *output, uint32_t pitch, uint32_t width,
				uint32_t height, aipl_color_format_t format, uint32_t output_width,
				uint32_t output_height, bool interpolate)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	if (aipl_dave2d_check_output_format(format)) {
		d2_u32 ret = aipl_dave2d_texturing(
			input, output, pitch, width, height, aipl_dave2d_format_to_mode(format),
			output_width, output_height, 0, 0, 0, false, false, true, interpolate);

		return aipl_dave2d_error_convert(ret);
	}

	return AIPL_ERR_UNSUPPORTED_FORMAT;
}

aipl_error_t aipl_resize_img_dave2d(const aipl_image_t *input, aipl_image_t *output,
				    bool interpolate)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	if (output->format != input->format) {
		return AIPL_ERR_FORMAT_MISMATCH;
	}

	return aipl_resize_dave2d(input->data, output->data, input->pitch, input->width,
				  input->height, input->format, output->width, output->height,
				  interpolate);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

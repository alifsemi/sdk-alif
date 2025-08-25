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
 * @brief   Resize function implementations
 *
 ******************************************************************************/

/*********************
 *      INCLUDES
 *********************/
#include "aipl_resize.h"

#include <stddef.h>

#include "aipl_config.h"
#ifdef AIPL_DAVE2D_ACCELERATION
#include "aipl_resize_dave2d.h"
#include "aipl_dave2d.h"
#endif
#ifdef AIPL_HELIUM_ACCELERATION
#include "aipl_resize_helium.h"
#else
#include "aipl_resize_default.h"
#endif

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
aipl_error_t aipl_resize(const void *input, void *output, uint32_t pitch, uint32_t width,
			 uint32_t height, aipl_color_format_t format, uint32_t output_width,
			 uint32_t output_height, bool interpolate)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

#ifdef AIPL_DAVE2D_ACCELERATION
	if (aipl_dave2d_check_output_format(format)) {
		return aipl_resize_dave2d(input, output, pitch, width, height, format, output_width,
					  output_height, interpolate);
	}
#endif

#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_resize_helium(input, output, pitch, width, height, format, output_width,
				  output_height, interpolate);
#else
	return aipl_resize_default(input, output, pitch, width, height, format, output_width,
				   output_height, interpolate);
#endif
}

aipl_error_t aipl_resize_img(const aipl_image_t *input, aipl_image_t *output, bool interpolate)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	if (output->format != input->format) {
		return AIPL_ERR_FORMAT_MISMATCH;
	}

	return aipl_resize(input->data, output->data, input->pitch, input->width, input->height,
			   input->format, output->width, output->height, interpolate);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

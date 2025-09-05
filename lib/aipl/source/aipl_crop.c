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
 * @file    aipl_crop.c
 * @brief   Crop function implementations
 *
 ******************************************************************************/

/*********************
 *      INCLUDES
 *********************/
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "aipl_config.h"
#ifdef AIPL_DAVE2D_ACCELERATION
#include "aipl_crop_dave2d.h"
#include "aipl_dave2d.h"
#endif
#include "aipl_crop_default.h"

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
aipl_error_t aipl_crop(const void *input, void *output, uint32_t pitch, uint32_t width,
		       uint32_t height, aipl_color_format_t format, uint32_t left, uint32_t top,
		       uint32_t right, uint32_t bottom)
{
#if (defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD))
	if (aipl_dave2d_check_output_format(format)) {
		return aipl_crop_dave2d(input, output, pitch, width, height, format, left, top,
					right, bottom);
	}
#endif

	return aipl_crop_default(input, output, pitch, width, height, format, left, top, right,
				 bottom);
}

aipl_error_t aipl_crop_img(const aipl_image_t *input, aipl_image_t *output, uint32_t left,
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

	return aipl_crop(input->data, output->data, input->pitch, input->width, input->height,
			 input->format, left, top, right, bottom);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

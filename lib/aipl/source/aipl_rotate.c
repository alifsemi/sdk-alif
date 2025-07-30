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
 * @file    aipl_rotate.c
 * @brief   Rotate function implementations
 *
 ******************************************************************************/

/*********************
 *      INCLUDES
 *********************/
#include "aipl_rotate.h"

#include <stddef.h>

#include "aipl_config.h"
#ifdef AIPL_DAVE2D_ACCELERATION
#include "aipl_rotate_dave2d.h"
#include "aipl_dave2d.h"
#endif
#ifdef AIPL_HELIUM_ACCELERATION
#include "aipl_rotate_helium.h"
#else
#include "aipl_rotate_default.h"
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
aipl_error_t aipl_rotate(const void *input, void *output, uint32_t pitch, uint32_t width,
			 uint32_t height, aipl_color_format_t format, aipl_rotation_t rotation)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

#if (defined(AIPL_DAVE2D_ACCELERATION) && defined(AIPL_OPTIMIZE_CPU_LOAD))
	if (aipl_dave2d_check_output_format(format)) {
		return aipl_rotate_dave2d(input, output, pitch, width, height, format, rotation);
	}
#endif

#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_rotate_helium(input, output, pitch, width, height, format, rotation);
#else
	return aipl_rotate_default(input, output, pitch, width, height, format, rotation);
#endif
}

aipl_error_t aipl_rotate_img(const aipl_image_t *input, aipl_image_t *output,
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

	return aipl_rotate(input->data, output->data, input->pitch, input->width, input->height,
			   input->format, rotation);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

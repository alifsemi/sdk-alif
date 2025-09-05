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
 * @file    aipl_image.c
 * @brief   Image utils implementations
 *
 ******************************************************************************/

/*********************
 *      INCLUDES
 *********************/
#include "aipl_image.h"

#include <stdlib.h>

#include "aipl_video_alloc.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void *aipl_image_allocate(uint32_t pitch, uint32_t height, aipl_color_format_t format);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
aipl_error_t aipl_image_create(aipl_image_t *image, uint32_t pitch, uint32_t width, uint32_t height,
			       aipl_color_format_t format)
{
	if (image == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	image->data = aipl_image_allocate(pitch, height, format);

	if (image->data == NULL) {
		return AIPL_ERR_NO_MEM;
	}

	image->pitch = pitch;
	image->width = width;
	image->height = height;
	image->format = format;

	return AIPL_ERR_OK;
}

void aipl_image_destroy(aipl_image_t *image)
{
	aipl_video_free(image->data);
	image->data = NULL;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void *aipl_image_allocate(uint32_t pitch, uint32_t height, aipl_color_format_t format)
{
	/* YUV formats encode in 2x2 pixel so the image buffer has to have even pitch and height */
	if (aipl_color_format_space(format) == AIPL_SPACE_YUV) {
		pitch = pitch & 0x1 ? pitch + 1 : pitch;
		height = height & 0x1 ? height + 1 : height;
	}

	return aipl_video_alloc(pitch * height * aipl_color_format_depth(format) / 8);
}

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
 * @file    aipl_lut_transform.c
 * @brief   LUT transformation function implementations
 *
 ******************************************************************************/

/*********************
 *      INCLUDES
 *********************/
#include "aipl_lut_transform.h"

#include <stddef.h>

#include "aipl_config.h"
#ifdef AIPL_HELIUM_ACCELERATION
#include "aipl_lut_transform_helium.h"
#else
#include "aipl_lut_transform_default.h"
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
aipl_error_t aipl_lut_transform_rgb(const void *input, void *output, uint32_t pitch, uint32_t width,
				    uint32_t height, aipl_color_format_t format, uint8_t *lut)
{
	switch (format) {
	case AIPL_COLOR_ARGB8888:
		return aipl_lut_transform_argb8888(input, output, pitch, width, height, lut);
	case AIPL_COLOR_RGBA8888:
		return aipl_lut_transform_rgba8888(input, output, pitch, width, height, lut);
	case AIPL_COLOR_ARGB4444:
		return aipl_lut_transform_argb4444(input, output, pitch, width, height, lut);
	case AIPL_COLOR_ARGB1555:
		return aipl_lut_transform_argb1555(input, output, pitch, width, height, lut);
	case AIPL_COLOR_RGBA4444:
		return aipl_lut_transform_rgba4444(input, output, pitch, width, height, lut);
	case AIPL_COLOR_RGBA5551:
		return aipl_lut_transform_rgba5551(input, output, pitch, width, height, lut);
	case AIPL_COLOR_BGR888:
		return aipl_lut_transform_bgr888(input, output, pitch, width, height, lut);
	case AIPL_COLOR_RGB888:
		return aipl_lut_transform_rgb888(input, output, pitch, width, height, lut);
	case AIPL_COLOR_RGB565:
		return aipl_lut_transform_rgb565(input, output, pitch, width, height, lut);

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}

	return AIPL_ERR_OK;
}

aipl_error_t aipl_lut_transform_rgb_img(const aipl_image_t *input, aipl_image_t *output,
					uint8_t *lut)
{
	if (input == NULL || output == NULL || lut == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	if (input->width != output->width || input->height != output->height) {
		return AIPL_ERR_SIZE_MISMATCH;
	}

	if (input->format != output->format) {
		return AIPL_ERR_FORMAT_MISMATCH;
	}

	return aipl_lut_transform_rgb(input->data, output->data, input->pitch, input->width,
				      input->height, input->format, lut);
}

aipl_error_t aipl_lut_transform_argb8888(const void *input, void *output, uint32_t pitch,
					 uint32_t width, uint32_t height, uint8_t *lut)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_lut_transform_argb8888_helium(input, output, pitch, width, height, lut);
#else
	return aipl_lut_transform_argb8888_default(input, output, pitch, width, height, lut);
#endif
}

aipl_error_t aipl_lut_transform_argb4444(const void *input, void *output, uint32_t pitch,
					 uint32_t width, uint32_t height, uint8_t *lut)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_lut_transform_argb4444_helium(input, output, pitch, width, height, lut);
#else
	return aipl_lut_transform_argb4444_default(input, output, pitch, width, height, lut);
#endif
}

aipl_error_t aipl_lut_transform_argb1555(const void *input, void *output, uint32_t pitch,
					 uint32_t width, uint32_t height, uint8_t *lut)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_lut_transform_argb1555_helium(input, output, pitch, width, height, lut);
#else
	return aipl_lut_transform_argb1555_default(input, output, pitch, width, height, lut);
#endif
}

aipl_error_t aipl_lut_transform_rgba8888(const void *input, void *output, uint32_t pitch,
					 uint32_t width, uint32_t height, uint8_t *lut)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_lut_transform_rgba8888_helium(input, output, pitch, width, height, lut);
#else
	return aipl_lut_transform_rgba8888_default(input, output, pitch, width, height, lut);
#endif
}

aipl_error_t aipl_lut_transform_rgba4444(const void *input, void *output, uint32_t pitch,
					 uint32_t width, uint32_t height, uint8_t *lut)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_lut_transform_rgba4444_helium(input, output, pitch, width, height, lut);
#else
	return aipl_lut_transform_rgba4444_default(input, output, pitch, width, height, lut);
#endif
}

aipl_error_t aipl_lut_transform_rgba5551(const void *input, void *output, uint32_t pitch,
					 uint32_t width, uint32_t height, uint8_t *lut)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_lut_transform_rgba5551_helium(input, output, pitch, width, height, lut);
#else
	return aipl_lut_transform_rgba5551_default(input, output, pitch, width, height, lut);
#endif
}

aipl_error_t aipl_lut_transform_bgr888(const void *input, void *output, uint32_t pitch,
				       uint32_t width, uint32_t height, uint8_t *lut)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_lut_transform_bgr888_helium(input, output, pitch, width, height, lut);
#else
	return aipl_lut_transform_bgr888_default(input, output, pitch, width, height, lut);
#endif
}

aipl_error_t aipl_lut_transform_rgb888(const void *input, void *output, uint32_t pitch,
				       uint32_t width, uint32_t height, uint8_t *lut)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_lut_transform_rgb888_helium(input, output, pitch, width, height, lut);
#else
	return aipl_lut_transform_rgb888_default(input, output, pitch, width, height, lut);
#endif
}

aipl_error_t aipl_lut_transform_rgb565(const void *input, void *output, uint32_t pitch,
				       uint32_t width, uint32_t height, uint8_t *lut)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_lut_transform_rgb565_helium(input, output, pitch, width, height, lut);
#else
	return aipl_lut_transform_rgb565_default(input, output, pitch, width, height, lut);
#endif
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

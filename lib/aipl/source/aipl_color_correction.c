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
 * @file    aipl_color_correction.c
 * @brief   Color correction function implementations
 *
 ******************************************************************************/

/*********************
 *      INCLUDES
 *********************/
#include "aipl_color_correction.h"

#include <stddef.h>

#include "aipl_config.h"
#ifdef AIPL_HELIUM_ACCELERATION
#include "aipl_color_correction_helium.h"
#else
#include "aipl_color_correction_default.h"
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
aipl_error_t aipl_color_correction_rgb(const void *input, void *output, uint32_t pitch,
				       uint32_t width, uint32_t height, aipl_color_format_t format,
				       const float *ccm)
{
	switch (format) {
	case AIPL_COLOR_ARGB8888:
		return aipl_color_correction_argb8888(input, output, pitch, width, height, ccm);
	case AIPL_COLOR_RGBA8888:
		return aipl_color_correction_rgba8888(input, output, pitch, width, height, ccm);
	case AIPL_COLOR_ARGB4444:
		return aipl_color_correction_argb4444(input, output, pitch, width, height, ccm);
	case AIPL_COLOR_ARGB1555:
		return aipl_color_correction_argb1555(input, output, pitch, width, height, ccm);
	case AIPL_COLOR_RGBA4444:
		return aipl_color_correction_rgba4444(input, output, pitch, width, height, ccm);
	case AIPL_COLOR_RGBA5551:
		return aipl_color_correction_rgba5551(input, output, pitch, width, height, ccm);
	case AIPL_COLOR_BGR888:
		return aipl_color_correction_bgr888(input, output, pitch, width, height, ccm);
	case AIPL_COLOR_RGB888:
		return aipl_color_correction_rgb888(input, output, pitch, width, height, ccm);
	case AIPL_COLOR_RGB565:
		return aipl_color_correction_rgb565(input, output, pitch, width, height, ccm);

	default:
		return AIPL_ERR_UNSUPPORTED_FORMAT;
	}
}

aipl_error_t aipl_color_correction_rgb_img(const aipl_image_t *input, aipl_image_t *output,
					   const float *ccm)
{
	if (input == NULL || output == NULL) {
		return AIPL_ERR_NULL_POINTER;
	}

	if (input->width != output->width || input->height != output->height) {
		return AIPL_ERR_SIZE_MISMATCH;
	}

	if (input->format != output->format) {
		return AIPL_ERR_FORMAT_MISMATCH;
	}

	return aipl_color_correction_rgb(input->data, output->data, input->pitch, input->width,
					 input->height, input->format, ccm);
}

aipl_error_t aipl_color_correction_argb8888(const void *input, void *output, uint32_t pitch,
					    uint32_t width, uint32_t height, const float *ccm)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_correction_argb8888_helium(input, output, pitch, width, height, ccm);
#else
	return aipl_color_correction_argb8888_default(input, output, pitch, width, height, ccm);
#endif
}

aipl_error_t aipl_color_correction_argb4444(const void *input, void *output, uint32_t pitch,
					    uint32_t width, uint32_t height, const float *ccm)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_correction_argb4444_helium(input, output, pitch, width, height, ccm);
#else
	return aipl_color_correction_argb4444_default(input, output, pitch, width, height, ccm);
#endif
}

aipl_error_t aipl_color_correction_argb1555(const void *input, void *output, uint32_t pitch,
					    uint32_t width, uint32_t height, const float *ccm)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_correction_argb1555_helium(input, output, pitch, width, height, ccm);
#else
	return aipl_color_correction_argb1555_default(input, output, pitch, width, height, ccm);
#endif
}

aipl_error_t aipl_color_correction_rgba8888(const void *input, void *output, uint32_t pitch,
					    uint32_t width, uint32_t height, const float *ccm)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_correction_rgba8888_helium(input, output, pitch, width, height, ccm);
#else
	return aipl_color_correction_rgba8888_default(input, output, pitch, width, height, ccm);
#endif
}

aipl_error_t aipl_color_correction_rgba4444(const void *input, void *output, uint32_t pitch,
					    uint32_t width, uint32_t height, const float *ccm)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_correction_rgba4444_helium(input, output, pitch, width, height, ccm);
#else
	return aipl_color_correction_rgba4444_default(input, output, pitch, width, height, ccm);
#endif
}

aipl_error_t aipl_color_correction_rgba5551(const void *input, void *output, uint32_t pitch,
					    uint32_t width, uint32_t height, const float *ccm)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_correction_rgba5551_helium(input, output, pitch, width, height, ccm);
#else
	return aipl_color_correction_rgba5551_default(input, output, pitch, width, height, ccm);
#endif
}

aipl_error_t aipl_color_correction_bgr888(const void *input, void *output, uint32_t pitch,
					  uint32_t width, uint32_t height, const float *ccm)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_correction_bgr888_helium(input, output, pitch, width, height, ccm);
#else
	return aipl_color_correction_bgr888_default(input, output, pitch, width, height, ccm);
#endif
}

aipl_error_t aipl_color_correction_rgb888(const void *input, void *output, uint32_t pitch,
					  uint32_t width, uint32_t height, const float *ccm)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_correction_rgb888_helium(input, output, pitch, width, height, ccm);
#else
	return aipl_color_correction_rgb888_default(input, output, pitch, width, height, ccm);
#endif
}

aipl_error_t aipl_color_correction_rgb565(const void *input, void *output, uint32_t pitch,
					  uint32_t width, uint32_t height, const float *ccm)
{
#ifdef AIPL_HELIUM_ACCELERATION
	return aipl_color_correction_rgb565_helium(input, output, pitch, width, height, ccm);
#else
	return aipl_color_correction_rgb565_default(input, output, pitch, width, height, ccm);
#endif
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

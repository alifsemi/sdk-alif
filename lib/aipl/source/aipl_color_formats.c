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
 * @file    aipl_color_formats.c
 * @brief   Color formats utils implementations
 *
 ******************************************************************************/

/*********************
 *      INCLUDES
 *********************/
#include "aipl_color_formats.h"

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
aipl_color_space_t aipl_color_format_space(aipl_color_format_t format)
{
	switch (format) {
	/* Alpha color formats */
	case AIPL_COLOR_ALPHA8:
	/* RGB color formats */
	case AIPL_COLOR_ARGB8888:
	case AIPL_COLOR_RGBA8888:
	case AIPL_COLOR_ARGB4444:
	case AIPL_COLOR_ARGB1555:
	case AIPL_COLOR_RGBA4444:
	case AIPL_COLOR_RGBA5551:
	case AIPL_COLOR_RGB565:
	case AIPL_COLOR_RGB888:
	case AIPL_COLOR_BGR888:
		return AIPL_SPACE_RGB;

	/* YUV color formats */
	case AIPL_COLOR_YV12:
	case AIPL_COLOR_I420:
	case AIPL_COLOR_NV12:
	case AIPL_COLOR_NV21:
	case AIPL_COLOR_I422:
	case AIPL_COLOR_YUY2:
	case AIPL_COLOR_UYVY:
	case AIPL_COLOR_I444:
	case AIPL_COLOR_I400:
		return AIPL_SPACE_YUV;

	default:
		return -1;
	}
}

uint8_t aipl_color_format_depth(aipl_color_format_t format)
{
	switch (format) {
	/* Alpha color formats */
	case AIPL_COLOR_ALPHA8:
		return 8;

	/* RGB color formats */
	case AIPL_COLOR_ARGB8888:
	case AIPL_COLOR_RGBA8888:
		return 32;
	case AIPL_COLOR_ARGB4444:
	case AIPL_COLOR_ARGB1555:
	case AIPL_COLOR_RGBA4444:
	case AIPL_COLOR_RGBA5551:
	case AIPL_COLOR_RGB565:
		return 16;
	case AIPL_COLOR_BGR888:
	case AIPL_COLOR_RGB888:
		return 24;

	/* YUV color formats */
	case AIPL_COLOR_YV12:
	case AIPL_COLOR_I420:
	case AIPL_COLOR_NV12:
	case AIPL_COLOR_NV21:
		return 12;
	case AIPL_COLOR_I422:
	case AIPL_COLOR_YUY2:
	case AIPL_COLOR_UYVY:
		return 16;
	case AIPL_COLOR_I444:
		return 24;
	case AIPL_COLOR_I400:
		return 8;

	default:
		return 0;
	}
}

const char *aipl_color_format_str(aipl_color_format_t format)
{
	switch (format) {
	/* Alpha color formats */
	case AIPL_COLOR_ALPHA8:
		return "ALPHA8";
	/* RGB color formats */
	case AIPL_COLOR_ARGB8888:
		return "ARGB8888";
	case AIPL_COLOR_RGBA8888:
		return "RGBA8888";
	case AIPL_COLOR_ARGB4444:
		return "ARGB4444";
	case AIPL_COLOR_ARGB1555:
		return "ARGB1555";
	case AIPL_COLOR_RGBA4444:
		return "RGBA4444";
	case AIPL_COLOR_RGBA5551:
		return "RGBA5551";
	case AIPL_COLOR_RGB888:
		return "RGB888";
	case AIPL_COLOR_RGB565:
		return "RGB565";
	case AIPL_COLOR_BGR888:
		return "BGR888";

	/* YUV color formats */
	case AIPL_COLOR_YV12:
		return "YV12";
	case AIPL_COLOR_I420:
		return "I420";
	case AIPL_COLOR_NV12:
		return "NV12";
	case AIPL_COLOR_NV21:
		return "NV21";
	case AIPL_COLOR_I422:
		return "I422";
	case AIPL_COLOR_YUY2:
		return "YUY2";
	case AIPL_COLOR_UYVY:
		return "UYVY";
	case AIPL_COLOR_I444:
		return "I444";
	case AIPL_COLOR_I400:
		return "I400";

	default:
		return "COLOR_UNKNOWN";
	}
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

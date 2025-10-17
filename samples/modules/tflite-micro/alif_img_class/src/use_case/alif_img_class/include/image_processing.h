/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef IMAGE_PROCESSING_H_
#define IMAGE_PROCESSING_H_

#include <stdint.h>
#include <stddef.h>

#include "aipl_image.h"

#define RTE_ARX3A0_CAMERA_SENSOR_ENABLE 1

#define RGB_BYTES 		3
#define RGBA_BYTES 		4
#define RGB565_BYTES	2
#define PIXEL_BYTES 	1

// Camera dimensions
#if RTE_ARX3A0_CAMERA_SENSOR_ENABLE // REV A RTE

#define CIMAGE_X                (560)
#define CIMAGE_Y                (560)
#define CIMAGE_COLOR_CORRECTION (1)
#define CIMAGE_EXPOSURE_CALC    (1)
#define CIMAGE_RGB_WIDTH_MAX    CIMAGE_X
#define CIMAGE_RGB_HEIGHT_MAX   CIMAGE_Y
#define CAM_BAYER_FORMAT        (AIPL_BAYER_GRBG)
#else
#error "Unsupported camera"
#endif

/*error status*/
#define FRAME_FORMAT_NOT_SUPPORTED   -1
#define FRAME_OUT_OF_RANGE           -2

extern uint32_t exposure_under_count, exposure_low_count, exposure_high_count, exposure_over_count;

void raw10_gray16le_bytes_to_raw8_inplace_mve(uint8_t *buf, size_t n_pixels);
const float *camera_get_color_correction_matrix(void);
uint8_t *camera_get_gamma_lut(void);

#endif /* IMAGE_PROCESSING_H_ */

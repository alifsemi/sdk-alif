/*
 * SPDX-FileCopyrightText: Copyright Alif Semiconductor
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IMAGE_PROCESSING_H_
#define IMAGE_PROCESSING_H_

#include <stdint.h>
#include <stddef.h>

#include "aipl_image.h"

#ifdef CONFIG_DT_HAS_ONNN_ARX3A0_ENABLED
#define RTE_ARX3A0_CAMERA_SENSOR_ENABLE 1
#elif CONFIG_DT_HAS_OVTI_OV5675_ENABLED
#define RTE_OV5675_CAMERA_SENSOR_ENABLE 1
#else
#error "Unsupported camera"
#endif

#define RGB_BYTES 		3
#define RGBA_BYTES 		4
#define RGB565_BYTES	2
#define PIXEL_BYTES 	1

// Camera dimensions
#if RTE_ARX3A0_CAMERA_SENSOR_ENABLE // REV A RTE
#define CIMAGE_X                (560)
#define CIMAGE_Y                (560)
#define CIMAGE_COLOR_CORRECTION (0)
#define CIMAGE_EXPOSURE_CALC    (0)
#define CIMAGE_RGB_WIDTH_MAX    CIMAGE_X
#define CIMAGE_RGB_HEIGHT_MAX   CIMAGE_Y
#define CAM_BAYER_FORMAT        (AIPL_BAYER_GRBG)
#elif RTE_OV5675_CAMERA_SENSOR_ENABLE // OV5675 RTE
#define CIMAGE_X                (1296)
#define CIMAGE_Y                (972)
#define CIMAGE_COLOR_CORRECTION (0)
#define CIMAGE_EXPOSURE_CALC    (0)
#define CIMAGE_RGB_WIDTH_MAX    (800)
#define CIMAGE_RGB_HEIGHT_MAX   (800)
#define CAM_BAYER_FORMAT        (AIPL_BAYER_GRBG)
#endif

/*error status*/
#define FRAME_FORMAT_NOT_SUPPORTED   -1
#define FRAME_OUT_OF_RANGE           -2

extern uint32_t exposure_under_count, exposure_low_count, exposure_high_count, exposure_over_count;

void crop_bayer8_inplace_topleft(
    uint8_t* buf,
    int src_width,
    int src_height,
    int crop_x,
    int crop_y,
    int crop_width,
    int crop_height);
void raw10_gray16le_bytes_to_raw8_inplace_mve(uint8_t *buf, size_t n_pixels);
const float *camera_get_color_correction_matrix(void);
uint8_t *camera_get_gamma_lut(void);

#endif /* IMAGE_PROCESSING_H_ */

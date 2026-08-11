/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https: //alifsemi.com/license
 *
 */

#ifndef IMAGE_PROCESSING_H_
#define IMAGE_PROCESSING_H_

#include <stdint.h>
#include <stddef.h>

#include "aipl_image.h"

#define RGB_BYTES 		3
#define RGBA_BYTES 		4
#define RGB565_BYTES	2
#define PIXEL_BYTES 	1

// Camera dimensions
#if CONFIG_DT_HAS_ONNN_ARX3A0_ENABLED
#define CIMAGE_X                (560)
#define CIMAGE_Y                (560)
#define CIMAGE_COLOR_CORRECTION (0)
#define CIMAGE_EXPOSURE_CALC    (0)
#define CIMAGE_RGB_WIDTH_MAX    CIMAGE_X
#define CIMAGE_RGB_HEIGHT_MAX   CIMAGE_Y
#define CAMERA_OUTPUT_RGB565    (0)
#define CAM_BAYER_FORMAT        (AIPL_BAYER_GRBG)
#elif CONFIG_DT_HAS_OVTI_OV5675_ENABLED
#define CIMAGE_X                (1296)
#define CIMAGE_Y                (972)
#define CIMAGE_COLOR_CORRECTION (0)
#define CIMAGE_EXPOSURE_CALC    (0)
#define CIMAGE_RGB_WIDTH_MAX    (800)
#define CIMAGE_RGB_HEIGHT_MAX   (800)
#define CAMERA_OUTPUT_RGB565    (0)
#define CAM_BAYER_FORMAT        (AIPL_BAYER_GRBG)
#elif CONFIG_DT_HAS_APTINA_MT9M114_ENABLED
/*
 * With MIPI (CONFIG_MT9M114_PARALLEL_INIT=n) the largest advertised Y10P mode
 * is 1288x728, which the pipeline selects. Crop a centered square (even offsets
 * preserve the Bayer phase) before demosaicing.
 */
#define CIMAGE_X                (1288)
#define CIMAGE_Y                (728)
#define CIMAGE_COLOR_CORRECTION (0)
#define CIMAGE_EXPOSURE_CALC    (0)
#define CIMAGE_RGB_WIDTH_MAX    (560)
#define CIMAGE_RGB_HEIGHT_MAX   (560)
#define CAMERA_OUTPUT_RGB565    (0)
#define CAM_BAYER_FORMAT        (AIPL_BAYER_GRBG)
#elif CONFIG_DT_HAS_OVTI_OV5640_ENABLED
/*
 * The OV5640 has an on-chip ISP and streams processed RGB565 over the parallel
 * (LP-CPI) bus, so there is no Bayer demosaic stage. The selected sensor mode
 * on the E1C StarterKit is 320x240 RGB565; crop a centered square before
 * feeding the detector.
 */
#define CIMAGE_X                (320)
#define CIMAGE_Y                (240)
#define CIMAGE_COLOR_CORRECTION (0)
#define CIMAGE_EXPOSURE_CALC    (0)
#define CIMAGE_RGB_WIDTH_MAX    (240)
#define CIMAGE_RGB_HEIGHT_MAX   (240)
#define CAMERA_OUTPUT_RGB565    (1)
#else
#error "Unsupported camera"
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
void rgb888_planar_to_packed(const uint8_t *src, uint8_t *dst,
			     uint32_t width, uint32_t height);
const float *camera_get_color_correction_matrix(void);
uint8_t *camera_get_gamma_lut(void);

#endif /* IMAGE_PROCESSING_H_ */

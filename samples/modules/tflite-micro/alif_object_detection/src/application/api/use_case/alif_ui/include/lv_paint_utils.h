/*
 * SPDX-FileCopyrightText: Copyright Alif Semiconductor
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef LV_PAINT_UTILS_H_
#define LV_PAINT_UTILS_H_

#include <stdint.h>
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

void write_rgb888_to_rgb565_buf(int width, int height,
				const uint8_t *src,
				uint16_t *dst);

#ifdef __cplusplus
}
#endif

#endif /* LV_PAINT_UTILS_H_ */

/*
 * SPDX-FileCopyrightText: Copyright Alif Semiconductor
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IMAGE_ENSEMBLE_H
#define IMAGE_ENSEMBLE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int image_init(void);
int get_image_data(int ml_width, int ml_height, uint8_t **output_image_data);

#ifdef __cplusplus
}
#endif

#endif /* IMAGE_ENSEMBLE_H */

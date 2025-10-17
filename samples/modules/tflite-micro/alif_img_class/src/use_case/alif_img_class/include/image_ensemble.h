/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
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

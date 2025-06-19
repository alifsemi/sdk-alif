/**
 * @file image.h
 */

#ifndef IMAGE_H
#define IMAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file scenes.h
 */

/*********************
 *      INCLUDES
 *********************/
#include "aipl_image.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void aipl_dave2d_prepare(void);

void aipl_dave2d_render(void);

void aipl_image_draw(uint32_t x, uint32_t y, const aipl_image_t *image);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*IMAGE_H*/

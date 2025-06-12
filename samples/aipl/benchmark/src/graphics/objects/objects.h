/**
 * @file objects.h
 */

#ifndef OBJECTS_H
#define OBJECTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "graphics.h"

/**
 * @file scenes.h
 */

/*********************
 *      INCLUDES
 *********************/
#include "graphics.h"
#include "aipl_dave2d.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
graph_object_t graph_create_rectangle(uint32_t x, uint32_t y, uint32_t width, uint32_t height,
				      d2_color color);

graph_object_t graph_create_image(uint32_t x, uint32_t y, void *image, uint32_t pitch,
				  uint32_t width, uint32_t height, d2_u32 format);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*OBJECTS_H*/

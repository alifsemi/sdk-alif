/**
 * @file assets.h
 */

#ifndef ASSETS_H
#define ASSETS_H

#include <stdint.h>

typedef struct {
	void *data;
	uint32_t data_size;
	uint32_t pitch;
	uint32_t width;
	uint32_t height;
	uint32_t format;
} image_t;

#define ASSET_PREFIX
#define RLE_ASSET_PREFIX ASSET_PREFIX __attribute__((aligned(128)))

#ifndef ASSET_INTERNAL
extern const image_t SAMPLE_PHOTO_RGB888;
extern const image_t SAMPLE_PHOTO_ARGB8888;
extern const image_t SAMPLE_PHOTO_ARGB4444;
extern const image_t SAMPLE_PHOTO_ARGB1555;
extern const image_t SAMPLE_PHOTO_RGBA8888;
extern const image_t SAMPLE_PHOTO_RGBA4444;
extern const image_t SAMPLE_PHOTO_RGBA5551;
extern const image_t SAMPLE_PHOTO_RGB565;
extern const image_t SAMPLE_PHOTO_ALPHA8;
extern const image_t SAMPLE_PHOTO_ALPHA4;
extern const image_t SAMPLE_PHOTO_ALPHA2;
extern const image_t SAMPLE_PHOTO_ALPHA1;
#endif

#endif /* ASSETS_H */

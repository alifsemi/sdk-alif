/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef ALIF_DISPLAY_TEST_COMMON_H_
#define ALIF_DISPLAY_TEST_COMMON_H_

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/display/cdc200.h>
#include <zephyr/logging/log.h>
#include <zephyr/ztest.h>
#include <stdint.h>

#define DISPLAY_DEVICE_NODE	DT_CHOSEN(zephyr_display)

/* Framebuffer write method selection */
#define DISPLAY_FB_WRITE_DIRECT	0
#define DISPLAY_FB_WRITE_API	1

/* Default to direct framebuffer write */
#ifndef DISPLAY_FB_WRITE_METHOD
#define DISPLAY_FB_WRITE_METHOD	DISPLAY_FB_WRITE_DIRECT
#endif

/* Color definitions for different pixel formats */
#define RED_ARGB8888		0x00ff0000
#define GREEN_ARGB8888		0x0000ff00
#define BLUE_ARGB8888		0x000000ff
#define WHITE_ARGB8888		0x00ffffff
#define BLACK_ARGB8888		0x00000000
#define RED_RGB888		0x00ff0000
#define GREEN_RGB888		0x0000ff00
#define BLUE_RGB888		0x000000ff
#define WHITE_RGB888		0x00ffffff
#define BLACK_RGB888		0x00000000
#define RED_RGB565		0xf800
#define GREEN_RGB565		0x07e0
#define BLUE_RGB565		0x001f
#define WHITE_RGB565		0xffff
#define BLACK_RGB565		0x0000

/* Pixel sizes for CDC200 */
#define CDC200_PIXEL_SIZE_ARGB8888	4
#define CDC200_PIXEL_SIZE_RGB888	3
#define CDC200_PIXEL_SIZE_RGB565	2

/* Shared device handle. */
extern const struct device *display_dev;

/* Suite hook: acquires + validates the display device. */
void display_suite_before(void *fixture);

/* Get pixel size in bytes for a given pixel format. */
int display_get_pixel_size(enum display_pixel_format fmt);

/* Fill a buffer with a solid color for the given pixel format. */
void display_fill_buffer_solid(uint8_t *buf, size_t buf_size,
			       enum display_pixel_format fmt,
			       uint32_t color);

/* Fill a buffer with a color gradient pattern. */
void display_fill_buffer_gradient(uint8_t *buf, size_t buf_size,
				   enum display_pixel_format fmt,
				   size_t width, size_t height);

/* Validate that a buffer contains the expected color pattern. */
bool display_validate_buffer_color(const uint8_t *buf, size_t buf_size,
				   enum display_pixel_format fmt,
				   uint32_t expected_color);

/* Allocate a display buffer of the given size. */
uint8_t *display_alloc_buffer(size_t size);

/* Free a display buffer. */
void display_free_buffer(uint8_t *buf);

/* Convert pixel format enum to readable string. */
const char *display_pixel_format_to_string(enum display_pixel_format fmt);

/* Convert errno code to readable string. */
const char *display_errno_to_string(int err);

/* Fast word-based framebuffer fill (more efficient than per-pixel memcpy). */
void display_fb_fill_word(uint8_t *fb_addr, size_t fb_size,
			  int pixel_size, uint32_t color);

/* Per-pixel memcpy framebuffer fill (slower, used for benchmarking). */
void display_fb_fill_memcpy(uint8_t *fb_addr, size_t fb_size,
			    int pixel_size, uint32_t color);

/* Clear display to white color (cleanup function). */
void display_clear_to_white(const struct device *dev);

#endif /* ALIF_DISPLAY_TEST_COMMON_H_ */

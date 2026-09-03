/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <string.h>
#include <zephyr/cache.h>
#include "display_common.h"

const struct device *display_dev;

void display_suite_before(void *fixture)
{
	ARG_UNUSED(fixture);

	display_dev = DEVICE_DT_GET(DISPLAY_DEVICE_NODE);
	zassert_not_null(display_dev, "Display device handle is NULL");
	zassert_true(device_is_ready(display_dev),
		     "Display device %s not ready", display_dev->name);

#ifdef CONFIG_MIPI_DSI
	{
		const struct device *panel = DEVICE_DT_GET(DT_ALIAS(panel));
		const struct device *dsi = DEVICE_DT_GET(DT_ALIAS(mipi_dsi));
		int ret;

		zassert_true(device_is_ready(panel),
			     "MIPI panel %s not ready "
			     "(enable reset/backlight GPIOs in the overlay)",
			     panel->name);
		zassert_true(device_is_ready(dsi),
			     "MIPI-DSI host %s not ready", dsi->name);

		/*
		 * Panel init runs in command mode. Video mode is what
		 * streams the CDC200 framebuffer over the DSI link.
		 * Without this the panel stays black.
		 */
		ret = dsi_dw_set_mode(dsi, DSI_DW_VIDEO_MODE);
		zassert_equal(ret, 0,
			      "dsi_dw_set_mode(VIDEO) failed: %d", ret);
	}
#endif

	/*
	 * CDC200 init only programs registers; scan-out starts here.
	 * display_blanking_off() on CDC200 is a no-op (-ENOTSUP).
	 */
	cdc200_set_enable(display_dev, true);
}

void display_fb_fill_word(uint8_t *fb_addr, size_t fb_size,
			  int pixel_size, uint32_t color)
{
	if (pixel_size == 2) {
		uint16_t *p = (uint16_t *)fb_addr;
		size_t count = fb_size / 2;
		uint16_t c = (uint16_t)color;

		for (size_t i = 0; i < count; i++) {
			p[i] = c;
		}
	} else if (pixel_size == 4) {
		uint32_t *p = (uint32_t *)fb_addr;
		size_t count = fb_size / 4;

		for (size_t i = 0; i < count; i++) {
			p[i] = color;
		}
	} else {
		size_t count = fb_size / pixel_size;

		for (size_t i = 0; i < count; i++) {
			memcpy(fb_addr + (i * pixel_size), &color, pixel_size);
		}
	}

	sys_cache_data_flush_range(fb_addr, fb_size);
}

void display_fb_fill_memcpy(uint8_t *fb_addr, size_t fb_size,
			    int pixel_size, uint32_t color)
{
	size_t count = fb_size / pixel_size;

	for (size_t i = 0; i < count; i++) {
		memcpy(fb_addr + (i * pixel_size), &color, pixel_size);
	}

	sys_cache_data_flush_range(fb_addr, fb_size);
}

void display_clear_to_white(const struct device *dev)
{
	struct cdc200_fb_desc fb_desc;
	struct cdc200_display_caps caps;
	uint32_t color;
	int pixel_size;

	cdc200_get_capabilities(dev, &caps);

	/* Skip if layer 0 is disabled */
	if (!caps.layer[0].layer_en) {
		return;
	}

	/* Get framebuffer for layer 0 */
	cdc200_get_framebuffer(dev, 0, &fb_desc);

	if (fb_desc.fb_addr == NULL || fb_desc.fb_size == 0) {
		return;
	}

	/* Get pixel size */
	pixel_size = display_get_pixel_size(caps.layer[0].current_pixel_format);
	zassert_true(pixel_size > 0, "Unsupported pixel format: %d",
		     caps.layer[0].current_pixel_format);
	color = display_solid_color(caps.layer[0].current_pixel_format,
				    DISPLAY_COLOR_WHITE);

	/* Fill framebuffer with white */
	display_fb_fill_word(fb_desc.fb_addr, fb_desc.fb_size, pixel_size, color);
}

int display_get_pixel_size(enum display_pixel_format fmt)
{
	switch (fmt) {
	case PIXEL_FORMAT_ARGB_8888:
		return CDC200_PIXEL_SIZE_ARGB8888;
	case PIXEL_FORMAT_RGB_888:
		return CDC200_PIXEL_SIZE_RGB888;
	case PIXEL_FORMAT_RGB_565:
		return CDC200_PIXEL_SIZE_RGB565;
	default:
		return 0;
	}
}

uint32_t display_solid_color(enum display_pixel_format fmt,
			     enum display_test_color color)
{
	static const uint32_t colors_argb8888[DISPLAY_COLOR_COUNT] = {
		RED_ARGB8888, GREEN_ARGB8888, BLUE_ARGB8888,
		WHITE_ARGB8888, BLACK_ARGB8888,
	};
	static const uint32_t colors_rgb888[DISPLAY_COLOR_COUNT] = {
		RED_RGB888, GREEN_RGB888, BLUE_RGB888,
		WHITE_RGB888, BLACK_RGB888,
	};
	static const uint32_t colors_rgb565[DISPLAY_COLOR_COUNT] = {
		RED_RGB565, GREEN_RGB565, BLUE_RGB565,
		WHITE_RGB565, BLACK_RGB565,
	};

	if (color >= DISPLAY_COLOR_COUNT) {
		return 0;
	}

	switch (fmt) {
	case PIXEL_FORMAT_ARGB_8888:
		return colors_argb8888[color];
	case PIXEL_FORMAT_RGB_888:
		return colors_rgb888[color];
	case PIXEL_FORMAT_RGB_565:
	default:
		return colors_rgb565[color];
	}
}

void display_fill_buffer_solid(uint8_t *buf, size_t buf_size,
			       enum display_pixel_format fmt,
			       uint32_t color)
{
	int pixel_size = display_get_pixel_size(fmt);

	if (pixel_size == 0) {
		return;
	}

	switch (fmt) {
	case PIXEL_FORMAT_ARGB_8888:
		for (size_t idx = 0; idx < buf_size; idx += 4) {
			*((uint32_t *)(buf + idx)) = color;
		}
		break;
	case PIXEL_FORMAT_RGB_888:
		for (size_t idx = 0; idx < buf_size; idx += 3) {
			*(buf + idx + 2) = (color >> 16) & 0xff;
			*(buf + idx + 1) = (color >> 8) & 0xff;
			*(buf + idx + 0) = (color >> 0) & 0xff;
		}
		break;
	case PIXEL_FORMAT_RGB_565:
		for (size_t idx = 0; idx < buf_size; idx += 2) {
			*(buf + idx + 1) = (color >> 8) & 0xFFu;
			*(buf + idx + 0) = (color >> 0) & 0xFFu;
		}
		break;
	default:
		break;
	}
}

void display_fill_buffer_gradient(uint8_t *buf, size_t buf_size,
				   enum display_pixel_format fmt,
				   size_t width, size_t height)
{
	int pixel_size = display_get_pixel_size(fmt);
	size_t pixel_count = buf_size / pixel_size;

	if (pixel_size == 0) {
		return;
	}

	for (size_t idx = 0; idx < pixel_count; idx++) {
		size_t x = idx % width;
		size_t y = idx / width;
		uint32_t color;

		/* Create a gradient based on position */
		uint8_t r = (uint8_t)((x * 255) / width);
		uint8_t g = (uint8_t)((y * 255) / height);
		uint8_t b = (uint8_t)(((x + y) * 255) / (width + height));

		switch (fmt) {
		case PIXEL_FORMAT_ARGB_8888:
			color = (0xFF << 24) | (r << 16) | (g << 8) | b;
			*((uint32_t *)(buf + idx * pixel_size)) = color;
			break;
		case PIXEL_FORMAT_RGB_888:
			buf[idx * pixel_size + 0] = r;
			buf[idx * pixel_size + 1] = g;
			buf[idx * pixel_size + 2] = b;
			break;
		case PIXEL_FORMAT_RGB_565:
			color = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
			*(uint16_t *)(buf + idx * pixel_size) = (uint16_t)color;
			break;
		default:
			break;
		}
	}
}

bool display_validate_buffer_color(const uint8_t *buf, size_t buf_size,
				   enum display_pixel_format fmt,
				   uint32_t expected_color)
{
	int pixel_size = display_get_pixel_size(fmt);

	if (pixel_size == 0) {
		return false;
	}

	switch (fmt) {
	case PIXEL_FORMAT_ARGB_8888:
		for (size_t idx = 0; idx < buf_size; idx += 4) {
			if (*((uint32_t *)(buf + idx)) != expected_color) {
				return false;
			}
		}
		return true;
	case PIXEL_FORMAT_RGB_888:
		for (size_t idx = 0; idx < buf_size; idx += 3) {
			uint32_t actual = ((uint32_t)buf[idx + 2] << 16) |
					  ((uint32_t)buf[idx + 1] << 8) |
					  (uint32_t)buf[idx + 0];
			if (actual != expected_color) {
				return false;
			}
		}
		return true;
	case PIXEL_FORMAT_RGB_565:
		for (size_t idx = 0; idx < buf_size; idx += 2) {
			uint16_t actual = ((uint16_t)buf[idx + 1] << 8) |
					  (uint16_t)buf[idx + 0];
			if (actual != (uint16_t)expected_color) {
				return false;
			}
		}
		return true;
	default:
		return false;
	}
}

uint8_t *display_alloc_buffer(size_t size)
{
	uint8_t *buf = k_malloc(size);

	zassert_not_null(buf, "Failed to allocate %zu byte buffer", size);
	return buf;
}

void display_free_buffer(uint8_t *buf)
{
	if (buf) {
		k_free(buf);
	}
}

const char *display_pixel_format_to_string(enum display_pixel_format fmt)
{
	switch (fmt) {
	case PIXEL_FORMAT_RGB_888:
		return "RGB_888";
	case PIXEL_FORMAT_MONO01:
		return "MONO01";
	case PIXEL_FORMAT_MONO10:
		return "MONO10";
	case PIXEL_FORMAT_ARGB_8888:
		return "ARGB_8888";
	case PIXEL_FORMAT_RGB_565:
		return "RGB_565";
	case PIXEL_FORMAT_BGR_565:
		return "BGR_565";
	default:
		return "UNKNOWN";
	}
}

const char *display_errno_to_string(int err)
{
	switch (err) {
	case 0:
		return "0 (SUCCESS)";
	case -EINVAL:
		return "-EINVAL";
	case -ENOMEM:
		return "-ENOMEM";
	case -ENOTSUP:
		return "-ENOTSUP";
	case -ENOSYS:
		return "-ENOSYS";
	case -EIO:
		return "-EIO";
	case -EACCES:
		return "-EACCES";
	case -EBUSY:
		return "-EBUSY";
	case -ETIMEDOUT:
		return "-ETIMEDOUT";
	default:
		return "UNKNOWN";
	}
}

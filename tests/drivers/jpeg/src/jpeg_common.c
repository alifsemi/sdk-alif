/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <string.h>

#include "jpeg_common.h"

const struct device *jpeg_dev;

void jpeg_suite_before(void *fixture)
{
	ARG_UNUSED(fixture);

	jpeg_dev = DEVICE_DT_GET(JPEG_DEVICE_NODE);
	zassert_not_null(jpeg_dev, "JPEG device handle is NULL");
	zassert_true(device_is_ready(jpeg_dev),
		     "JPEG device %s not ready", jpeg_dev->name);
}

size_t jpeg_ref_image_size(void)
{
	return (size_t)(testimg_end - testimg_start);
}

void jpeg_copy_ref_image(uint8_t *dest)
{
	memcpy(dest, testimg_start, jpeg_ref_image_size());
}

void jpeg_fill_nv12_gradient(uint8_t *dest, uint32_t width, uint32_t height,
			     uint32_t pitch)
{
	uint8_t *y_plane  = dest;
	uint8_t *uv_plane = dest + (pitch * height);

	for (uint32_t row = 0; row < height; row++) {
		for (uint32_t col = 0; col < width; col++) {
			y_plane[row * pitch + col] = (uint8_t)((row + col) & 0xFF);
		}
	}
	for (uint32_t row = 0; row < (height / 2); row++) {
		for (uint32_t col = 0; col < width; col += 2) {
			uv_plane[row * pitch + col]     = (uint8_t)(0x80 + (col & 0x3F));
			uv_plane[row * pitch + col + 1] = (uint8_t)(0x80 + (row & 0x3F));
		}
	}
}

bool jpeg_validate_markers(const uint8_t *buf, size_t size)
{
	if (size < 4) {
		return false;
	}
	if (buf[0] != JPEG_MARKER_SOI_HI || buf[1] != JPEG_MARKER_SOI_LO) {
		return false;
	}
	if (buf[size - 2] != JPEG_MARKER_EOI_HI ||
	    buf[size - 1] != JPEG_MARKER_EOI_LO) {
		return false;
	}
	return true;
}

bool jpeg_validate_jfif_structure(const uint8_t *buf, size_t size)
{
	bool have_app0_jfif = false;
	bool have_dqt = false;
	bool have_sof0 = false;
	bool have_dht = false;
	bool have_sos = false;
	bool have_eoi = false;
	size_t i;

	if (size < 4 || buf[0] != 0xFF || buf[1] != 0xD8) {
		return false;
	}

	i = 2;
	while (i + 1 < size) {
		uint8_t marker;
		uint16_t seg_len;

		/* Skip 0xFF fill bytes. */
		while (i < size && buf[i] == 0xFF) {
			i++;
		}
		if (i >= size) {
			return false;
		}
		marker = buf[i++];

		if (marker == 0xD9) { /* EOI */
			have_eoi = true;
			break;
		}
		/* Standalone markers carry no length. */
		if (marker == 0x00 || marker == 0x01 ||
		    (marker >= 0xD0 && marker <= 0xD7)) {
			continue;
		}

		if (i + 2 > size) {
			return false;
		}
		seg_len = ((uint16_t)buf[i] << 8) | buf[i + 1];
		if (seg_len < 2 || i + seg_len > size) {
			return false;
		}

		switch (marker) {
		case 0xE0: /* APP0 */
			if (seg_len >= 7 &&
			    memcmp(&buf[i + 2], "JFIF\0", 5) == 0) {
				have_app0_jfif = true;
			}
			break;
		case 0xDB: /* DQT */
			have_dqt = true;
			break;
		case 0xC0: /* SOF0 (baseline) */
			have_sof0 = true;
			break;
		case 0xC4: /* DHT */
			have_dht = true;
			break;
		case 0xDA: /* SOS - entropy-coded data follows; scan for EOI */
			have_sos = true;
			i += seg_len;
			while (i + 1 < size) {
				if (buf[i] == 0xFF && buf[i + 1] != 0x00) {
					if (buf[i + 1] == 0xD9) {
						have_eoi = true;
					}
					/* Restart markers (D0-D7) keep going,
					 * but for our minimal validation it is
					 * sufficient to stop here.
					 */
					goto done;
				}
				i++;
			}
			goto done;
		default:
			break;
		}
		i += seg_len;
	}
done:
	return have_app0_jfif && have_dqt && have_sof0 &&
	       have_dht && have_sos && have_eoi;
}

bool jpeg_extract_sof0_dimensions(const uint8_t *buf, size_t size,
				  uint16_t *width, uint16_t *height)
{
	size_t i;

	if (buf == NULL || size < 4 || width == NULL || height == NULL) {
		return false;
	}
	if (buf[0] != 0xFF || buf[1] != 0xD8) {
		return false;
	}

	i = 2;
	while (i + 1 < size) {
		uint8_t marker;
		uint16_t seg_len;

		while (i < size && buf[i] == 0xFF) {
			i++;
		}
		if (i >= size) {
			return false;
		}
		marker = buf[i++];

		if (marker == 0xD9) { /* EOI */
			return false;
		}
		if (marker == 0x00 || marker == 0x01 ||
		    (marker >= 0xD0 && marker <= 0xD7)) {
			continue;
		}

		if (i + 2 > size) {
			return false;
		}
		seg_len = ((uint16_t)buf[i] << 8) | buf[i + 1];
		if (seg_len < 2 || i + seg_len > size) {
			return false;
		}

		if (marker == 0xC0) { /* SOF0 baseline */
			/* Need at least: len(2) + precision(1) + h(2) + w(2) */
			if (seg_len < 7) {
				return false;
			}
			*height = ((uint16_t)buf[i + 3] << 8) | buf[i + 4];
			*width  = ((uint16_t)buf[i + 5] << 8) | buf[i + 6];
			return true;
		}
		i += seg_len;
	}
	return false;
}

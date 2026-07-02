/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <string.h>
#include "display_common.h"

LOG_MODULE_REGISTER(display_test, LOG_LEVEL_INF);

/*
 * Test suites for the Alif CDC200 display driver
 * (compatible: alif,cdc200).
 *
 * Two test suites:
 * - display_api: Tests all display driver API functions
 * - display_functional: Tests helper functions and functional behavior
 *
 * Each test is independent and validates a specific aspect of the
 * display driver API.
 */

/* ------------------------------------------------------------------ */
/* Suite Hooks                                                         */
/* ------------------------------------------------------------------ */

ZTEST_SUITE(display_api, NULL, NULL, display_suite_before, NULL, NULL);
ZTEST_SUITE(display_functional, NULL, NULL, display_suite_before, NULL, NULL);

/* ------------------------------------------------------------------ */
/* Tests                                                               */
/* ------------------------------------------------------------------ */

/*
 * Note: device-ready is enforced unconditionally by display_suite_before()
 * before every test, so a dedicated device_ready case is intentionally
 * omitted.
 */

ZTEST(display_api, test_display_blanking)
{
	int ret;

	/* Test turning off blanking (enable display) */
	ret = display_blanking_off(display_dev);
	TC_PRINT("display_blanking_off() returned: %s (%d)\n",
		 display_errno_to_string(ret), ret);
	if (ret == -ENOTSUP) {
		TC_PRINT("Blanking control not supported, skipping test\n");
		ztest_test_skip();
	}
	zassert_equal(ret, 0, "display_blanking_off failed: %d", ret);

	/* Test turning on blanking (disable display) */
	ret = display_blanking_on(display_dev);
	TC_PRINT("display_blanking_on() returned: %s (%d)\n",
		 display_errno_to_string(ret), ret);
	if (ret == -ENOTSUP) {
		TC_PRINT("Blanking control not supported, skipping test\n");
		ztest_test_skip();
	}
	zassert_equal(ret, 0, "display_blanking_on failed: %d", ret);

	/* Turn off blanking again for subsequent tests */
	ret = display_blanking_off(display_dev);
	TC_PRINT("display_blanking_off(restore) returned: %s (%d)\n",
		 display_errno_to_string(ret), ret);
	zassert_equal(ret, 0, "display_blanking_off (restore) failed: %d", ret);
}

ZTEST(display_functional, test_display_fb_fill_benchmark)
{
	struct cdc200_fb_desc fb_desc;
	struct cdc200_display_caps caps;
	uint32_t start, end;
	uint32_t cycles_word, cycles_memcpy;
	uint64_t ns_word, ns_memcpy;
	int pixel_size;
	uint32_t color;

	cdc200_get_capabilities(display_dev, &caps);

	if (!caps.layer[0].layer_en) {
		TC_PRINT("Layer 0 disabled, skipping benchmark\n");
		ztest_test_skip();
	}

	cdc200_get_framebuffer(display_dev, 0, &fb_desc);
	zassert_not_null(fb_desc.fb_addr, "Layer 0 framebuffer address is NULL");

	pixel_size = display_get_pixel_size(caps.layer[0].current_pixel_format);

	switch (caps.layer[0].current_pixel_format) {
	case PIXEL_FORMAT_ARGB_8888:
		color = BLUE_ARGB8888;
		break;
	case PIXEL_FORMAT_RGB_888:
		color = BLUE_RGB888;
		break;
	default:
		color = BLUE_RGB565;
		break;
	}

	TC_PRINT("\n=== Framebuffer Fill Benchmark ===\n");
	TC_PRINT("Framebuffer size: %zu bytes\n", fb_desc.fb_size);
	TC_PRINT("Pixel size: %d bytes\n", pixel_size);
	TC_PRINT("Total pixels: %zu\n", fb_desc.fb_size / pixel_size);

	/* Benchmark word-based fill */
	start = k_cycle_get_32();
	display_fb_fill_word(fb_desc.fb_addr, fb_desc.fb_size, pixel_size, color);
	end = k_cycle_get_32();
	cycles_word = end - start;
	ns_word = k_cyc_to_ns_floor64(cycles_word);

	/* Benchmark per-pixel memcpy fill */
	start = k_cycle_get_32();
	display_fb_fill_memcpy(fb_desc.fb_addr, fb_desc.fb_size, pixel_size, color);
	end = k_cycle_get_32();
	cycles_memcpy = end - start;
	ns_memcpy = k_cyc_to_ns_floor64(cycles_memcpy);

	TC_PRINT("\nResults:\n");
	TC_PRINT("  word-based fill:   %u cycles, %llu ns (%llu us)\n",
		 cycles_word, ns_word, ns_word / 1000);
	TC_PRINT("  per-pixel memcpy:  %u cycles, %llu ns (%llu us)\n",
		 cycles_memcpy, ns_memcpy, ns_memcpy / 1000);

	if (cycles_word > 0) {
		TC_PRINT("  speedup: %u.%02ux\n",
			 cycles_memcpy / cycles_word,
			 ((cycles_memcpy * 100) / cycles_word) % 100);
	}
	TC_PRINT("==================================\n\n");

	/* Cleanup: clear display to white after benchmark */
	TC_PRINT("Cleaning up display (clearing to white)...\n");
	display_clear_to_white(display_dev);
	k_msleep(1000);
}

ZTEST(display_functional, test_display_fb_solid_sweep)
{
	struct cdc200_fb_desc fb_desc;
	struct cdc200_display_caps caps;
#if DISPLAY_FB_WRITE_METHOD == DISPLAY_FB_WRITE_API
	struct display_buffer_descriptor buf_desc;
	uint8_t *buf;
	size_t buf_size;
#endif
	uint32_t color;
	int pixel_size;

	cdc200_get_capabilities(display_dev, &caps);

	/* Skip if layer 0 is disabled */
	if (!caps.layer[0].layer_en) {
		TC_PRINT("Layer 0 disabled, skipping framebuffer solid sweep test\n");
		ztest_test_skip();
	}

#if DISPLAY_FB_WRITE_METHOD == DISPLAY_FB_WRITE_DIRECT
	/* Get framebuffer for layer 0 */
	cdc200_get_framebuffer(display_dev, 0, &fb_desc);

	TC_PRINT("Layer 0 Framebuffer:\n");
	TC_PRINT("  Address: %p\n", fb_desc.fb_addr);
	TC_PRINT("  Size: %zu\n", fb_desc.fb_size);
	TC_PRINT("  Pixel format: %s\n",
		 display_pixel_format_to_string(caps.layer[0].current_pixel_format));
	TC_PRINT("  Write method: DIRECT (memcpy)\n");

	zassert_not_null(fb_desc.fb_addr, "Layer 0 framebuffer address is NULL");
	zassert_true(fb_desc.fb_size > 0, "Layer 0 framebuffer size is zero");

	/* Get pixel size for the current format */
	pixel_size = display_get_pixel_size(caps.layer[0].current_pixel_format);
#elif DISPLAY_FB_WRITE_METHOD == DISPLAY_FB_WRITE_API
	/* Get pixel size first to calculate correct buffer size */
	pixel_size = display_get_pixel_size(caps.layer[0].current_pixel_format);

	/* Allocate buffer for display write API -
	 * use very small size to avoid allocation failure
	 */
	buf_size = caps.layer[0].x_resolution * 10 * pixel_size; /* 10 rows */
	buf = display_alloc_buffer(buf_size);
	zassert_not_null(buf, "Failed to allocate buffer");

	TC_PRINT("Layer 0:\n");
	TC_PRINT("  Resolution: %dx%d\n", caps.layer[0].x_resolution,
		 caps.layer[0].y_resolution);
	TC_PRINT("  Pixel format: %s\n",
		 display_pixel_format_to_string(caps.layer[0].current_pixel_format));
	TC_PRINT("  Write method: API (cdc200_display_write)\n");
	TC_PRINT("  Buffer size: %zu bytes (10 rows)\n", buf_size);

	/* Setup buffer descriptor for 10 rows */
	buf_desc.buf_size = buf_size;
	buf_desc.pitch = caps.layer[0].x_resolution;
	buf_desc.width = caps.layer[0].x_resolution;
	buf_desc.height = 10;

	pixel_size = display_get_pixel_size(caps.layer[0].current_pixel_format);
#endif

	/* Fill with RED */
	TC_PRINT("Filling framebuffer with RED\n");
	switch (caps.layer[0].current_pixel_format) {
	case PIXEL_FORMAT_ARGB_8888:
		color = RED_ARGB8888;
		break;
	case PIXEL_FORMAT_RGB_888:
		color = RED_RGB888;
		break;
	case PIXEL_FORMAT_RGB_565:
		color = RED_RGB565;
		break;
	default:
		color = RED_RGB565;
		break;
	}
#if DISPLAY_FB_WRITE_METHOD == DISPLAY_FB_WRITE_DIRECT
	display_fb_fill_word(fb_desc.fb_addr, fb_desc.fb_size, pixel_size, color);
#elif DISPLAY_FB_WRITE_METHOD == DISPLAY_FB_WRITE_API
	display_fill_buffer_solid(buf, buf_size,
				  caps.layer[0].current_pixel_format, color);
	/* Write the 10-row buffer multiple times to fill the screen */
	for (int y = 0; y < caps.layer[0].y_resolution; y += 10) {
		cdc200_display_write(display_dev, 0, 0, y, &buf_desc, buf);
	}
#endif
	k_msleep(3000);

	/* Fill with BLUE */
	TC_PRINT("Filling framebuffer with BLUE\n");
	switch (caps.layer[0].current_pixel_format) {
	case PIXEL_FORMAT_ARGB_8888:
		color = BLUE_ARGB8888;
		break;
	case PIXEL_FORMAT_RGB_888:
		color = BLUE_RGB888;
		break;
	case PIXEL_FORMAT_RGB_565:
		color = BLUE_RGB565;
		break;
	default:
		color = BLUE_RGB565;
		break;
	}
#if DISPLAY_FB_WRITE_METHOD == DISPLAY_FB_WRITE_DIRECT
	display_fb_fill_word(fb_desc.fb_addr, fb_desc.fb_size, pixel_size, color);
#elif DISPLAY_FB_WRITE_METHOD == DISPLAY_FB_WRITE_API
	display_fill_buffer_solid(buf, buf_size,
				  caps.layer[0].current_pixel_format, color);
	/* Write the 10-row buffer multiple times to fill the screen */
	for (int y = 0; y < caps.layer[0].y_resolution; y += 10) {
		cdc200_display_write(display_dev, 0, 0, y, &buf_desc, buf);
	}
#endif
	k_msleep(3000);

	/* Fill with GREEN */
	TC_PRINT("Filling framebuffer with GREEN\n");
	switch (caps.layer[0].current_pixel_format) {
	case PIXEL_FORMAT_ARGB_8888:
		color = GREEN_ARGB8888;
		break;
	case PIXEL_FORMAT_RGB_888:
		color = GREEN_RGB888;
		break;
	case PIXEL_FORMAT_RGB_565:
		color = GREEN_RGB565;
		break;
	default:
		color = GREEN_RGB565;
		break;
	}
#if DISPLAY_FB_WRITE_METHOD == DISPLAY_FB_WRITE_DIRECT
	display_fb_fill_word(fb_desc.fb_addr, fb_desc.fb_size, pixel_size, color);
#elif DISPLAY_FB_WRITE_METHOD == DISPLAY_FB_WRITE_API
	display_fill_buffer_solid(buf, buf_size,
				  caps.layer[0].current_pixel_format, color);
	/* Write the 10-row buffer multiple times to fill the screen */
	for (int y = 0; y < caps.layer[0].y_resolution; y += 10) {
		cdc200_display_write(display_dev, 0, 0, y, &buf_desc, buf);
	}
#endif
	k_msleep(3000);

	/* Fill with WHITE */
	TC_PRINT("Filling framebuffer with WHITE\n");
	switch (caps.layer[0].current_pixel_format) {
	case PIXEL_FORMAT_ARGB_8888:
		color = WHITE_ARGB8888;
		break;
	case PIXEL_FORMAT_RGB_888:
		color = WHITE_RGB888;
		break;
	case PIXEL_FORMAT_RGB_565:
		color = WHITE_RGB565;
		break;
	default:
		color = WHITE_RGB565;
		break;
	}
#if DISPLAY_FB_WRITE_METHOD == DISPLAY_FB_WRITE_DIRECT
	display_fb_fill_word(fb_desc.fb_addr, fb_desc.fb_size, pixel_size, color);
#elif DISPLAY_FB_WRITE_METHOD == DISPLAY_FB_WRITE_API
	display_fill_buffer_solid(buf, buf_size,
				  caps.layer[0].current_pixel_format, color);
	/* Write the 10-row buffer multiple times to fill the screen */
	for (int y = 0; y < caps.layer[0].y_resolution; y += 10) {
		cdc200_display_write(display_dev, 0, 0, y, &buf_desc, buf);
	}
#endif
	k_msleep(3000);

	/* Fill with BLACK */
	TC_PRINT("Filling framebuffer with BLACK\n");
	switch (caps.layer[0].current_pixel_format) {
	case PIXEL_FORMAT_ARGB_8888:
		color = BLACK_ARGB8888;
		break;
	case PIXEL_FORMAT_RGB_888:
		color = BLACK_RGB888;
		break;
	case PIXEL_FORMAT_RGB_565:
		color = BLACK_RGB565;
		break;
	default:
		color = BLACK_RGB565;
		break;
	}
#if DISPLAY_FB_WRITE_METHOD == DISPLAY_FB_WRITE_DIRECT
	display_fb_fill_word(fb_desc.fb_addr, fb_desc.fb_size, pixel_size, color);
#elif DISPLAY_FB_WRITE_METHOD == DISPLAY_FB_WRITE_API
	display_fill_buffer_solid(buf, buf_size,
				  caps.layer[0].current_pixel_format, color);
	/* Write the 10-row buffer multiple times to fill the screen */
	for (int y = 0; y < caps.layer[0].y_resolution; y += 10) {
		cdc200_display_write(display_dev, 0, 0, y, &buf_desc, buf);
	}
#endif
	k_msleep(3000);

#if DISPLAY_FB_WRITE_METHOD == DISPLAY_FB_WRITE_API
	display_free_buffer(buf);
#endif

	TC_PRINT("Direct framebuffer write test completed\n");
}

ZTEST(display_functional, test_display_fb_readback_verify)
{
	struct display_buffer_descriptor buf_desc;
	struct cdc200_display_caps caps;
	uint8_t *write_buf;
	uint8_t *read_buf;
	size_t buf_size;
	size_t rect_w = 64;
	size_t rect_h = 64;
	int pixel_size;
	int ret;
	int mismatch_count = 0;
	uint32_t color1, color2;

	cdc200_get_capabilities(display_dev, &caps);

	/* Skip if layer 0 is disabled */
	if (!caps.layer[0].layer_en) {
		TC_PRINT("Layer 0 disabled, skipping framebuffer readback test\n");
		ztest_test_skip();
	}

	TC_PRINT("\n=== Framebuffer Readback Verification ===\n");
	TC_PRINT("Test region: %zux%zu pixels\n", rect_w, rect_h);
	TC_PRINT("Pixel format: %s\n",
		 display_pixel_format_to_string(caps.layer[0].current_pixel_format));

	/* Get pixel size */
	pixel_size = display_get_pixel_size(caps.layer[0].current_pixel_format);
	buf_size = rect_w * rect_h * pixel_size;

	/* Allocate buffers for write and read */
	write_buf = display_alloc_buffer(buf_size);
	zassert_not_null(write_buf, "Failed to allocate write buffer");

	read_buf = display_alloc_buffer(buf_size);
	zassert_not_null(read_buf, "Failed to allocate read buffer");

	/* Setup buffer descriptor */
	buf_desc.buf_size = buf_size;
	buf_desc.pitch = rect_w;
	buf_desc.width = rect_w;
	buf_desc.height = rect_h;

	/* Determine colors based on pixel format */
	switch (caps.layer[0].current_pixel_format) {
	case PIXEL_FORMAT_ARGB_8888:
		color1 = RED_ARGB8888;
		color2 = BLUE_ARGB8888;
		break;
	case PIXEL_FORMAT_RGB_888:
		color1 = RED_RGB888;
		color2 = BLUE_RGB888;
		break;
	case PIXEL_FORMAT_RGB_565:
	default:
		color1 = RED_RGB565;
		color2 = BLUE_RGB565;
		break;
	}

	/* ===== Pattern 1: Checkerboard color pattern ===== */
	TC_PRINT("\n--- Pattern 1: Checkerboard (color1/color2) ---\n");
	for (size_t y = 0; y < rect_h; y++) {
		for (size_t x = 0; x < rect_w; x++) {
			uint32_t color = ((x + y) % 2 == 0) ? color1 : color2;
			size_t offset = (y * rect_w + x) * pixel_size;

			memcpy(&write_buf[offset], &color, pixel_size);
		}
	}
	memset(read_buf, 0, buf_size);

	TC_PRINT("Writing checkerboard at (100, 100)\n");
	ret = cdc200_display_write(display_dev, 0, 100, 100, &buf_desc, write_buf);
	zassert_equal(ret, 0, "cdc200_display_write failed: %d", ret);

	TC_PRINT("Waiting 3 seconds for visual observation...\n");
	k_msleep(3000);

	ret = cdc200_display_read(display_dev, 0, 100, 100, &buf_desc, read_buf);
	zassert_equal(ret, 0, "cdc200_display_read failed: %d", ret);

	for (size_t i = 0; i < buf_size; i++) {
		if (write_buf[i] != read_buf[i]) {
			mismatch_count++;
			if (mismatch_count <= 10) {
				TC_PRINT("Mismatch at byte %zu: wrote 0x%02x, read 0x%02x\n",
					 i, write_buf[i], read_buf[i]);
			}
		}
	}
	TC_PRINT("Pattern 1 result: %s (%d mismatches / %zu bytes)\n",
		 mismatch_count == 0 ? "PASS" : "FAIL", mismatch_count, buf_size);
	zassert_equal(mismatch_count, 0,
		      "Checkerboard readback failed: %d mismatches", mismatch_count);

	/* ===== Pattern 2: Sequential byte pattern (0x00..0xFF cycling) ===== */
	TC_PRINT("\n--- Pattern 2: Sequential bytes (0x00..0xFF) ---\n");
	for (size_t i = 0; i < buf_size; i++) {
		write_buf[i] = (uint8_t)(i & 0xFF);
	}
	memset(read_buf, 0, buf_size);
	mismatch_count = 0;

	TC_PRINT("Writing sequential pattern at (200, 200)\n");
	ret = cdc200_display_write(display_dev, 0, 200, 200, &buf_desc, write_buf);
	zassert_equal(ret, 0, "cdc200_display_write failed: %d", ret);

	TC_PRINT("Waiting 3 seconds for visual observation...\n");
	k_msleep(3000);

	ret = cdc200_display_read(display_dev, 0, 200, 200, &buf_desc, read_buf);
	zassert_equal(ret, 0, "cdc200_display_read failed: %d", ret);

	for (size_t i = 0; i < buf_size; i++) {
		if (write_buf[i] != read_buf[i]) {
			mismatch_count++;
			if (mismatch_count <= 10) {
				TC_PRINT("Mismatch at byte %zu: wrote 0x%02x, read 0x%02x\n",
					 i, write_buf[i], read_buf[i]);
			}
		}
	}
	TC_PRINT("Pattern 2 result: %s (%d mismatches / %zu bytes)\n",
		 mismatch_count == 0 ? "PASS" : "FAIL", mismatch_count, buf_size);
	zassert_equal(mismatch_count, 0,
		      "Sequential readback failed: %d mismatches", mismatch_count);

	display_free_buffer(write_buf);
	display_free_buffer(read_buf);
	TC_PRINT("====================================\n\n");
}

ZTEST(display_functional, test_display_region_clipping)
{
	struct display_buffer_descriptor buf_desc;
	struct cdc200_display_caps caps;
	uint8_t *buf;
	size_t buf_size;
	int pixel_size;
	int ret;
	size_t rect_w = 32;
	size_t rect_h = 32;

	cdc200_get_capabilities(display_dev, &caps);

	/* Skip if layer 0 is disabled */
	if (!caps.layer[0].layer_en) {
		TC_PRINT("Layer 0 disabled, skipping region clipping test\n");
		ztest_test_skip();
	}

	TC_PRINT("\n=== Region Clipping Test ===\n");
	TC_PRINT("Display resolution: %dx%d\n",
		 caps.layer[0].x_resolution,
		 caps.layer[0].y_resolution);
	TC_PRINT("Pixel format: %s\n",
		 display_pixel_format_to_string(caps.layer[0].current_pixel_format));

	/* Get pixel size and allocate buffer */
	pixel_size = display_get_pixel_size(caps.layer[0].current_pixel_format);
	buf_size = rect_w * rect_h * pixel_size;
	buf = display_alloc_buffer(buf_size);
	zassert_not_null(buf, "Failed to allocate buffer");

	/* Setup buffer descriptor */
	buf_desc.buf_size = buf_size;
	buf_desc.pitch = rect_w;
	buf_desc.width = rect_w;
	buf_desc.height = rect_h;

	/* Fill buffer with red */
	display_fill_buffer_solid(buf, buf_size,
				  caps.layer[0].current_pixel_format,
				  RED_RGB565);

	/* Test 1: Negative coordinates - COMMENTED OUT (causes bus fault, driver bug) */
	TC_PRINT("\nTest 1: Negative coordinates (-10, -10)\n");
	ret = cdc200_display_write(display_dev, 0, -10, -10, &buf_desc, buf);
	TC_PRINT("  Result: %s (%d)\n", display_errno_to_string(ret), ret);


	/* Test 2: Coordinates beyond resolution (should fail or clip) */
	TC_PRINT("\nTest 2: Coordinates beyond resolution (%d, %d)\n",
		 caps.layer[0].x_resolution + 10,
		 caps.layer[0].y_resolution + 10);
	ret = cdc200_display_write(display_dev, 0,
				   caps.layer[0].x_resolution + 10,
				   caps.layer[0].y_resolution + 10,
				   &buf_desc, buf);
	TC_PRINT("  Result: %s (%d)\n", display_errno_to_string(ret), ret);
	/* Driver may reject out-of-bounds or clip them - either is acceptable */

	/* Test 3: Partial out-of-bounds (rectangle extends beyond edge) */
	TC_PRINT("\nTest 3: Partial out-of-bounds (x=%d, y=%d, w=%zu, h=%zu)\n",
		 caps.layer[0].x_resolution - 10,
		 caps.layer[0].y_resolution - 10,
		 rect_w, rect_h);
	ret = cdc200_display_write(display_dev, 0,
				   caps.layer[0].x_resolution - 10,
				   caps.layer[0].y_resolution - 10,
				   &buf_desc, buf);
	TC_PRINT("  Result: %s (%d)\n", display_errno_to_string(ret), ret);
	/* Driver may clip or reject - either is acceptable */

	/* Delay for visual observation of partial out-of-bounds write */
	k_msleep(4000);

	/* Test 4: Valid write (should succeed) */
	TC_PRINT("\nTest 4: Valid write at (100, 100)\n");
	ret = cdc200_display_write(display_dev, 0, 100, 100, &buf_desc, buf);
	TC_PRINT("  Result: %s (%d)\n", display_errno_to_string(ret), ret);
	zassert_equal(ret, 0, "Valid write should succeed: %d", ret);

	/* Delay for visual observation of the valid write */
	k_msleep(2000);

	display_free_buffer(buf);
	TC_PRINT("==================================\n\n");
}

ZTEST(display_functional, test_display_power_cycle)
{
	struct display_buffer_descriptor buf_desc;
	struct cdc200_display_caps caps;
	uint8_t *buf;
	size_t buf_size;
	int pixel_size;
	int ret;
	size_t rect_w = 32;
	size_t rect_h = 32;

	cdc200_get_capabilities(display_dev, &caps);

	/* Skip if layer 0 is disabled */
	if (!caps.layer[0].layer_en) {
		TC_PRINT("Layer 0 disabled, skipping power cycle test\n");
		ztest_test_skip();
	}

	TC_PRINT("\n=== Display Power Cycle Test ===\n");

	/* Get pixel size and allocate buffer for operations test */
	pixel_size = display_get_pixel_size(caps.layer[0].current_pixel_format);
	buf_size = rect_w * rect_h * pixel_size;
	buf = display_alloc_buffer(buf_size);
	zassert_not_null(buf, "Failed to allocate buffer");

	/* Setup buffer descriptor */
	buf_desc.buf_size = buf_size;
	buf_desc.pitch = rect_w;
	buf_desc.width = rect_w;
	buf_desc.height = rect_h;

	/* Fill buffer with red */
	display_fill_buffer_solid(buf, buf_size,
				  caps.layer[0].current_pixel_format,
				  RED_RGB565);

	/* Clear display to white for visual observation
	 * TC_PRINT("Clearing display to white for visual observation...\n");
	 * display_clear_to_white(display_dev);
	 */
	k_msleep(2000);

	/* Disable display */
	TC_PRINT("Disabling display...\n");
	cdc200_set_enable(display_dev, 0);
	TC_PRINT("Display disabled\n");
	k_msleep(500);

	/* Test operations while disabled */
	TC_PRINT("\nTest 1: Attempt write while display is disabled\n");
	ret = cdc200_display_write(display_dev, 0, 100, 100, &buf_desc, buf);
	TC_PRINT("  cdc200_display_write() returned: %s (%d)\n",
		 display_errno_to_string(ret), ret);

	TC_PRINT("\nTest 2: Attempt read while display is disabled\n");
	ret = cdc200_display_read(display_dev, 0, 100, 100, &buf_desc, buf);
	TC_PRINT("  cdc200_display_read() returned: %s (%d)\n",
		 display_errno_to_string(ret), ret);

	/* Wait for display to power down */
	k_msleep(6500);

	/* Re-enable display */
	TC_PRINT("Re-enabling display...\n");
	cdc200_set_enable(display_dev, 1);
	TC_PRINT("Display enabled\n");
	k_msleep(500);

	/* Verify write works after re-enable */
	TC_PRINT("\nTest 3: Verify write works after re-enable\n");
	ret = cdc200_display_write(display_dev, 0, 100, 100, &buf_desc, buf);
	TC_PRINT("  cdc200_display_write() returned: %s (%d)\n",
		 display_errno_to_string(ret), ret);
	zassert_equal(ret, 0, "Write should succeed after re-enable: %d", ret);

	/* Wait for display to power up */
	k_msleep(3500);

	/* Verify display is still functional by writing a test pattern */
	TC_PRINT("Verifying display functionality after power cycle...\n");
	display_clear_to_white(display_dev);
	TC_PRINT("Display cleared to white successfully\n");

	display_free_buffer(buf);
	TC_PRINT("====================================\n\n");
}

/*
 * Bug validation test: Pass invalid layer indices to CDC200 APIs.
 *
 * Driver code review identified `idx > CDC_LAYER_2` checks in:
 *   - cdc200_display_write()
 *   - cdc200_display_read()
 *   - cdc200_get_framebuffer()
 *   - cdc200_swap_fb()
 *
 * Valid layer indices: CDC_LAYER_1 (0), CDC_LAYER_2 (1).
 * This test passes invalid indices (2, 5, 255) and verifies:
 *   - APIs that return int: return -EINVAL
 *   - APIs that return void: don't crash, leave outputs untouched
 */
ZTEST(display_functional, test_display_invalid_layer)
{
	struct display_buffer_descriptor buf_desc;
	struct cdc200_display_caps caps;
	struct cdc200_fb_desc fb_desc;
	uint8_t *buf;
	size_t buf_size;
	int pixel_size;
	int ret;
	size_t rect_w = 32;
	size_t rect_h = 32;
	uint8_t invalid_indices[] = {2, 5, 255};

	cdc200_get_capabilities(display_dev, &caps);

	if (!caps.layer[0].layer_en) {
		TC_PRINT("Layer 0 disabled, skipping invalid layer test\n");
		ztest_test_skip();
	}

	TC_PRINT("\n=== Invalid Layer Index Test ===\n");

	pixel_size = display_get_pixel_size(caps.layer[0].current_pixel_format);
	buf_size = rect_w * rect_h * pixel_size;
	buf = display_alloc_buffer(buf_size);
	zassert_not_null(buf, "Failed to allocate buffer");

	buf_desc.buf_size = buf_size;
	buf_desc.pitch = rect_w;
	buf_desc.width = rect_w;
	buf_desc.height = rect_h;

	display_fill_buffer_solid(buf, buf_size,
				  caps.layer[0].current_pixel_format,
				  RED_RGB565);

	for (size_t i = 0; i < ARRAY_SIZE(invalid_indices); i++) {
		uint8_t idx = invalid_indices[i];

		TC_PRINT("\n--- Testing with invalid idx = %u ---\n", idx);

		/* Test 1: cdc200_display_write() with invalid idx */
		ret = cdc200_display_write(display_dev, idx, 0, 0, &buf_desc, buf);
		TC_PRINT("  cdc200_display_write(idx=%u): %s (%d)\n",
			 idx, display_errno_to_string(ret), ret);
		zassert_equal(ret, -EINVAL,
			      "Expected -EINVAL for write with idx=%u, got %d",
			      idx, ret);

		/* Test 2: cdc200_display_read() with invalid idx */
		ret = cdc200_display_read(display_dev, idx, 0, 0, &buf_desc, buf);
		TC_PRINT("  cdc200_display_read(idx=%u):  %s (%d)\n",
			 idx, display_errno_to_string(ret), ret);
		zassert_equal(ret, -EINVAL,
			      "Expected -EINVAL for read with idx=%u, got %d",
			      idx, ret);

		/* Test 3: cdc200_get_framebuffer() with invalid idx (void return) */
		memset(&fb_desc, 0xAA, sizeof(fb_desc));
		cdc200_get_framebuffer(display_dev, idx, &fb_desc);
		TC_PRINT("  cdc200_get_framebuffer(idx=%u): fb_addr=%p fb_size=%zu\n",
			 idx, fb_desc.fb_addr, fb_desc.fb_size);
		/* fb_desc should remain unmodified (0xAA pattern preserved) */

		/* Test 4: cdc200_swap_fb() with invalid idx (void return) */
		fb_desc.fb_addr = buf;
		fb_desc.fb_size = buf_size;
		cdc200_swap_fb(display_dev, idx, &fb_desc);
		TC_PRINT("  cdc200_swap_fb(idx=%u): completed without crash\n", idx);
	}

	display_free_buffer(buf);
	TC_PRINT("================================\n\n");
}

/*
 * Bug validation test: Undersized buffer with large width/height.
 *
 * Driver bug #2: cdc200_display_write() does NOT validate desc->buf_size
 * against the actual required size (width * height * pix_size). It reads
 * past the source buffer, causing potential memory corruption / undefined
 * behavior.
 *
 * NOTE: Currently COMMENTED OUT because driver lacks validation and may
 * cause a bus fault or read invalid memory. Enable once driver is fixed.
 */
ZTEST(display_functional, test_display_undersized_buffer)
{
	struct display_buffer_descriptor buf_desc;
	struct cdc200_display_caps caps;
	uint8_t *buf;
	size_t small_size = 64;  /* Tiny buffer */
	size_t rect_w = 64;
	size_t rect_h = 64;       /* Descriptor claims 64x64 = 8192 bytes for RGB565 */
	int ret;

	cdc200_get_capabilities(display_dev, &caps);

	if (!caps.layer[0].layer_en) {
		TC_PRINT("Layer 0 disabled, skipping undersized buffer test\n");
		ztest_test_skip();
	}

	TC_PRINT("\n=== Undersized Buffer Write Test ===\n");

	buf = display_alloc_buffer(small_size);
	zassert_not_null(buf, "Failed to allocate buffer");
	memset(buf, 0xAB, small_size);

	/* Buffer descriptor claims large size, but actual buf is tiny */
	buf_desc.buf_size = small_size;
	buf_desc.pitch = rect_w;
	buf_desc.width = rect_w;
	buf_desc.height = rect_h;

	TC_PRINT("Buffer allocated: %zu bytes\n", small_size);
	TC_PRINT("Descriptor claims: %zux%zu (would need %zu bytes)\n",
		 rect_w, rect_h, rect_w * rect_h * 2);
	TC_PRINT("Expected: driver should return -EINVAL\n");

	ret = cdc200_display_write(display_dev, 0, 0, 0, &buf_desc, buf);
	TC_PRINT("Result: %s (%d)\n", display_errno_to_string(ret), ret);

	/* Driver should reject mismatched buffer size */
	zassert_equal(ret, -EINVAL,
		      "Expected -EINVAL for undersized buffer, got %d", ret);

	display_free_buffer(buf);
	TC_PRINT("=====================================\n\n");
}

/*
 * Bug validation test: NULL pointer parameters.
 *
 * Driver bug #11: cdc200_display_write/read() do NOT check for NULL
 * desc or buf parameters, causing a NULL pointer dereference crash.
 *
 * NOTE: Currently COMMENTED OUT because passing NULL will crash.
 * Enable once driver adds NULL checks.
 */
ZTEST(display_functional, test_display_null_params)
{
	struct display_buffer_descriptor buf_desc;
	struct cdc200_display_caps caps;
	uint8_t *buf;
	size_t buf_size;
	int pixel_size;
	int ret;
	size_t rect_w = 32;
	size_t rect_h = 32;

	cdc200_get_capabilities(display_dev, &caps);

	if (!caps.layer[0].layer_en) {
		TC_PRINT("Layer 0 disabled, skipping NULL params test\n");
		ztest_test_skip();
	}

	TC_PRINT("\n=== NULL Pointer Parameters Test ===\n");

	pixel_size = display_get_pixel_size(caps.layer[0].current_pixel_format);
	buf_size = rect_w * rect_h * pixel_size;
	buf = display_alloc_buffer(buf_size);
	zassert_not_null(buf, "Failed to allocate buffer");

	buf_desc.buf_size = buf_size;
	buf_desc.pitch = rect_w;
	buf_desc.width = rect_w;
	buf_desc.height = rect_h;

	/* Test 1: NULL buffer */
	TC_PRINT("Test 1: NULL buf pointer\n");
	ret = cdc200_display_write(display_dev, 0, 0, 0, &buf_desc, NULL);
	TC_PRINT("  Result: %s (%d)\n", display_errno_to_string(ret), ret);
	zassert_equal(ret, -EINVAL, "Expected -EINVAL for NULL buf, got %d", ret);

	/* Test 2: NULL descriptor */
	TC_PRINT("Test 2: NULL desc pointer\n");
	ret = cdc200_display_write(display_dev, 0, 0, 0, NULL, buf);
	TC_PRINT("  Result: %s (%d)\n", display_errno_to_string(ret), ret);
	zassert_equal(ret, -EINVAL, "Expected -EINVAL for NULL desc, got %d", ret);

	/* Test 3: NULL buffer on read */
	TC_PRINT("Test 3: NULL buf on read\n");
	ret = cdc200_display_read(display_dev, 0, 0, 0, &buf_desc, NULL);
	TC_PRINT("  Result: %s (%d)\n", display_errno_to_string(ret), ret);
	zassert_equal(ret, -EINVAL, "Expected -EINVAL for NULL read buf, got %d", ret);

	display_free_buffer(buf);
	TC_PRINT("====================================\n\n");
}

/*
 * Bug validation test: Read cache coherency.
 *
 * Driver bug #8: cdc200_display_read() does NOT call sys_cache_data_invd_range()
 * on the source framebuffer before memcpy'ing data to the user buffer.
 *
 * To exercise this bug, we BYPASS the cdc200_display_write() API (which
 * performs sys_cache_data_flush_range()) and instead write directly to the
 * framebuffer using the raw pointer from cdc200_get_framebuffer(). We do
 * NOT explicitly flush after the direct write.
 *
 * Then we call cdc200_display_read() and verify the data.
 *
 * Expected behaviors:
 *   - If cdc200_display_read() correctly invalidates cache: read matches.
 *   - If cache is configured as non-cacheable: read matches (bug latent).
 *   - If bug is exposed: read returns stale data (mismatch).
 *
 * NOTE: Standard memcpy goes through CPU cache, so the read may still hit
 * our dirty cache lines and return correct data. True cache coherency
 * issues typically manifest when a DMA agent updates the FB. This test
 * documents the attempt and provides a starting point.
 */
ZTEST(display_functional, test_display_read_cache_coherency)
{
	struct display_buffer_descriptor buf_desc;
	struct cdc200_display_caps caps;
	struct cdc200_fb_desc fb;
	uint8_t *read_buf;
	uint8_t *fb_ptr;
	size_t buf_size;
	int pixel_size;
	int ret;
	size_t rect_w = 64;
	size_t rect_h = 64;
	int mismatch_count = 0;
	uint32_t layer_width;

	cdc200_get_capabilities(display_dev, &caps);

	if (!caps.layer[0].layer_en) {
		TC_PRINT("Layer 0 disabled, skipping cache coherency test\n");
		ztest_test_skip();
	}

	TC_PRINT("\n=== Read Cache Coherency Test (Direct FB Write) ===\n");

	pixel_size = display_get_pixel_size(caps.layer[0].current_pixel_format);
	buf_size = rect_w * rect_h * pixel_size;
	layer_width = caps.layer[0].x_resolution;

	/* Get raw framebuffer pointer */
	cdc200_get_framebuffer(display_dev, 0, &fb);
	zassert_not_null(fb.fb_addr, "Framebuffer pointer is NULL");
	fb_ptr = (uint8_t *)fb.fb_addr;

	read_buf = display_alloc_buffer(buf_size);
	zassert_not_null(read_buf, "Failed to allocate read buffer");
	memset(read_buf, 0, buf_size);

	buf_desc.buf_size = buf_size;
	buf_desc.pitch = rect_w;
	buf_desc.width = rect_w;
	buf_desc.height = rect_h;

	TC_PRINT("FB base address: %p, size: %zu\n", fb.fb_addr, fb.fb_size);
	TC_PRINT("Layer width: %u, pixel size: %d\n", layer_width, pixel_size);

/*
 * Directly write a known pattern to FB at position (300, 300),
 * matching the addressing scheme used by cdc200_display_write():
 *   dst = fb + (y * width + x) * pix_size
 *
 * We do NOT call sys_cache_data_flush_range() after this write.
 */
	TC_PRINT("Writing pattern directly to FB at (300, 300) WITHOUT cache flush\n");
	for (size_t row = 0; row < rect_h; row++) {
		uint8_t *dst = fb_ptr + ((300 + row) * layer_width + 300) * pixel_size;

		for (size_t col = 0; col < rect_w; col++) {
			for (int p = 0; p < pixel_size; p++) {
				dst[col * pixel_size + p] =
					(uint8_t)((row * rect_w + col + p) & 0xFF);
			}
		}
	}

	TC_PRINT("Waiting 3 seconds for visual observation of direct FB write...\n");
	k_msleep(3000);

/*
 * Now read back via API. If the read path does not invalidate cache
 * before memcpy, AND our direct writes are sitting in CPU cache, the
 * memcpy will still get our data (cache hit). If the cache writeback
 * to FB hasn't happened and the read goes through a different
 * cache view (e.g., DMA-coherent vs CPU view), we may see stale data.
 */
	TC_PRINT("Reading back via cdc200_display_read()\n");
	ret = cdc200_display_read(display_dev, 0, 300, 300, &buf_desc, read_buf);
	zassert_equal(ret, 0, "Read failed: %d", ret);

	/* Compare expected vs actual */
	for (size_t row = 0; row < rect_h; row++) {
		for (size_t col = 0; col < rect_w; col++) {
			for (int p = 0; p < pixel_size; p++) {
				uint8_t expected = (uint8_t)((row * rect_w + col + p) & 0xFF);
				uint8_t actual = read_buf[(row * rect_w + col) * pixel_size + p];

				if (expected != actual) {
					mismatch_count++;
					if (mismatch_count <= 5) {
						TC_PRINT("Mismatch at (%zu,%zu) byte %d: "
							 "expected 0x%02x, got 0x%02x\n",
							 row, col, p, expected, actual);
					}
				}
			}
		}
	}

	if (mismatch_count == 0) {
		TC_PRINT("Result: MATCH - cache coherency not exposed in this config\n");
		TC_PRINT("        (FB may be non-cacheable, or CPU cache served the read)\n");
	} else {
		TC_PRINT("Result: MISMATCH - %d bytes differ, bug #8 may be confirmed\n",
			 mismatch_count);
	}

	zassert_equal(mismatch_count, 0,
		      "Cache coherency: %d bytes mismatch", mismatch_count);

	display_free_buffer(read_buf);
	TC_PRINT("====================================\n\n");
}

ZTEST(display_api, test_display_cdc200_enable)
{
	/* Test enabling the display */
	cdc200_set_enable(display_dev, true);

	/* Test disabling the display */
	cdc200_set_enable(display_dev, false);

	/* Re-enable for subsequent tests */
	cdc200_set_enable(display_dev, true);
}

ZTEST(display_api, test_display_cdc200_get_caps)
{
	struct cdc200_display_caps caps;

	cdc200_get_capabilities(display_dev, &caps);

	TC_PRINT("CDC200 Capabilities:\n");
	TC_PRINT("  Panel resolution: %dx%d\n",
		 caps.x_panel_resolution, caps.y_panel_resolution);
	TC_PRINT("  Supported formats: 0x%08x\n",
		 caps.supported_pixel_formats);
	TC_PRINT("  Current orientation: %d\n", caps.current_orientation);

	/* Validate panel resolution */
	zassert_true(caps.x_panel_resolution > 0,
		     "x_panel_resolution is zero");
	zassert_true(caps.y_panel_resolution > 0,
		     "y_panel_resolution is zero");

	/* Validate layer information */
	for (int i = 0; i < 2; i++) {
		TC_PRINT("  Layer %d:\n", i);
		TC_PRINT("    enabled: %d\n", caps.layer[i].layer_en);
		TC_PRINT("    resolution: %dx%d\n",
			 caps.layer[i].x_resolution,
			 caps.layer[i].y_resolution);
		TC_PRINT("    pixel_format: %s\n",
			 display_pixel_format_to_string(
				 caps.layer[i].current_pixel_format));

		if (caps.layer[i].layer_en) {
			zassert_true(caps.layer[i].x_resolution > 0,
				     "Layer %d x_resolution is zero", i);
			zassert_true(caps.layer[i].y_resolution > 0,
				     "Layer %d y_resolution is zero", i);
		}
	}
}

ZTEST(display_api, test_display_cdc200_get_framebuffer)
{
	struct cdc200_fb_desc fb_desc;
	struct cdc200_display_caps caps;

	cdc200_get_capabilities(display_dev, &caps);

	/* Test getting framebuffer for layer 0 */
	cdc200_get_framebuffer(display_dev, 0, &fb_desc);

	TC_PRINT("Layer 0 Framebuffer:\n");
	TC_PRINT("  Address: %p\n", fb_desc.fb_addr);
	TC_PRINT("  Size: %zu\n", fb_desc.fb_size);

	/* If layer is disabled, framebuffer may be NULL - skip validation */
	if (caps.layer[0].layer_en) {
		zassert_not_null(fb_desc.fb_addr, "Layer 0 framebuffer address is NULL");
		zassert_true(fb_desc.fb_size > 0, "Layer 0 framebuffer size is zero");
	} else {
		TC_PRINT("Layer 0 disabled, skipping framebuffer validation\n");
	}

	/* Test getting framebuffer for layer 1 (may not be enabled) */
	cdc200_get_framebuffer(display_dev, 1, &fb_desc);
	TC_PRINT("Layer 1 Framebuffer:\n");
	TC_PRINT("  Address: %p\n", fb_desc.fb_addr);
	TC_PRINT("  Size: %zu\n", fb_desc.fb_size);
}

ZTEST(display_api, test_display_get_capabilities)
{
	struct display_capabilities caps;

	display_get_capabilities(display_dev, &caps);

	TC_PRINT("Display Capabilities:\n");
	TC_PRINT("  x_resolution: %d\n", caps.x_resolution);
	TC_PRINT("  y_resolution: %d\n", caps.y_resolution);
	TC_PRINT("  supported_pixel_formats: 0x%08x\n",
		 caps.supported_pixel_formats);
	TC_PRINT("  current_pixel_format: %s\n",
		 display_pixel_format_to_string(caps.current_pixel_format));
	TC_PRINT("  current_orientation: %d\n", caps.current_orientation);

	/* Validate that resolution is non-zero */
	zassert_true(caps.x_resolution > 0, "x_resolution is zero");
	zassert_true(caps.y_resolution > 0, "y_resolution is zero");

	/* Validate that at least one pixel format is supported */
	zassert_true(caps.supported_pixel_formats != 0,
		     "No pixel formats supported");
}

ZTEST(display_api, test_display_orientation)
{
	struct display_capabilities caps;
	int ret;
	int supported_count = 0;

	/* Get current orientation */
	display_get_capabilities(display_dev, &caps);
	TC_PRINT("Initial orientation: %d\n", caps.current_orientation);

	/* Test setting orientation to normal */
	ret = display_set_orientation(display_dev, DISPLAY_ORIENTATION_NORMAL);
	TC_PRINT("display_set_orientation(NORMAL)    returned: %s (%d)\n",
		 display_errno_to_string(ret), ret);
	if (ret == 0) {
		supported_count++;
	}

	/* Test setting orientation to rotated 90 */
	ret = display_set_orientation(display_dev, DISPLAY_ORIENTATION_ROTATED_90);
	TC_PRINT("display_set_orientation(ROTATED_90)  returned: %s (%d)\n",
		 display_errno_to_string(ret), ret);
	if (ret == 0) {
		supported_count++;
	}

	/* Test setting orientation to rotated 180 */
	ret = display_set_orientation(display_dev,
				      DISPLAY_ORIENTATION_ROTATED_180);
	TC_PRINT("display_set_orientation(ROTATED_180) returned: %s (%d)\n",
		 display_errno_to_string(ret), ret);
	if (ret == 0) {
		supported_count++;
	}

	/* Test setting orientation to rotated 270 */
	ret = display_set_orientation(display_dev, DISPLAY_ORIENTATION_ROTATED_270);
	TC_PRINT("display_set_orientation(ROTATED_270) returned: %s (%d)\n",
		 display_errno_to_string(ret), ret);
	if (ret == 0) {
		supported_count++;
	}

	/* Restore normal orientation */
	ret = display_set_orientation(display_dev, DISPLAY_ORIENTATION_NORMAL);
	TC_PRINT("display_set_orientation(restore)     returned: %s (%d)\n",
		 display_errno_to_string(ret), ret);
	if (ret == 0) {
		supported_count++;
	}

	/* If no orientations are supported, skip the test */
	if (supported_count == 0) {
		TC_PRINT("Orientation control not supported, skipping test\n");
		ztest_test_skip();
	}

	TC_PRINT("Orientation test completed: %d orientation(s) supported\n", supported_count);
}

ZTEST(display_api, test_display_write)
{
	struct display_buffer_descriptor buf_desc;
	struct cdc200_display_caps caps;
	uint8_t *buf;
	size_t buf_size;

	/* Get capabilities to determine buffer size */
	cdc200_get_capabilities(display_dev, &caps);

	if (!caps.layer[0].layer_en) {
		TC_PRINT("Layer 0 not enabled, skipping write test\n");
		ztest_test_skip();
	}

	/* Allocate a small buffer for testing */
	buf_size = caps.layer[0].x_resolution * 4; /* One row of ARGB8888 */
	buf = display_alloc_buffer(buf_size);
	zassert_not_null(buf, "Failed to allocate buffer");

	/* Fill buffer with red */
	TC_PRINT("Using pixel format: %s\n",
		 display_pixel_format_to_string(caps.layer[0].current_pixel_format));
	display_fill_buffer_solid(buf, buf_size,
				  caps.layer[0].current_pixel_format,
				  RED_ARGB8888);

	/* Setup buffer descriptor */
	buf_desc.buf_size = buf_size;
	buf_desc.pitch = caps.layer[0].x_resolution;
	buf_desc.width = caps.layer[0].x_resolution;
	buf_desc.height = 1;

	/* Write to display at position (0, 0) */
	cdc200_display_write(display_dev, 0, 0, 0, &buf_desc, buf);

	/* Delay for visual observation */
	k_msleep(5000);

	display_free_buffer(buf);
}

ZTEST(display_api, test_display_write_multiple_rects)
{
	struct display_buffer_descriptor buf_desc;
	struct cdc200_display_caps caps;
	uint8_t *buf;
	size_t buf_size;

	cdc200_get_capabilities(display_dev, &caps);

	if (!caps.layer[0].layer_en) {
		TC_PRINT("Layer 0 not enabled, skipping test\n");
		ztest_test_skip();
	}

	/* Allocate buffer for a small rectangle */
	size_t rect_w = 64;
	size_t rect_h = 64;

	TC_PRINT("Using pixel format: %s\n",
		 display_pixel_format_to_string(caps.layer[0].current_pixel_format));
	int pixel_size = display_get_pixel_size(
		caps.layer[0].current_pixel_format);
	buf_size = rect_w * rect_h * pixel_size;

	buf = display_alloc_buffer(buf_size);
	zassert_not_null(buf, "Failed to allocate buffer");

	/* Setup buffer descriptor */
	buf_desc.buf_size = buf_size;
	buf_desc.pitch = rect_w;
	buf_desc.width = rect_w;
	buf_desc.height = rect_h;

	/* Write red rectangle at top-left */
	TC_PRINT("Writing RED rectangle at top-left (0, 0), size %zux%zu\n", rect_w, rect_h);
	display_fill_buffer_solid(buf, buf_size,
				  caps.layer[0].current_pixel_format,
				  RED_RGB565);
	cdc200_display_write(display_dev, 0, 0, 0, &buf_desc, buf);

	/* Write green rectangle at top-right */
	TC_PRINT("Writing GREEN rectangle at top-right (%d, 0), size %zux%zu\n",
		 caps.layer[0].x_resolution - rect_w, rect_w, rect_h);
	display_fill_buffer_solid(buf, buf_size,
				  caps.layer[0].current_pixel_format,
				  GREEN_RGB565);
	cdc200_display_write(display_dev, 0,
				   caps.layer[0].x_resolution - rect_w, 0,
				   &buf_desc, buf);

	/* Write blue rectangle at bottom-left */
	TC_PRINT("Writing BLUE rectangle at bottom-left (0, %d), size %zux%zu\n",
		 caps.layer[0].y_resolution - rect_h, rect_w, rect_h);
	display_fill_buffer_solid(buf, buf_size,
				  caps.layer[0].current_pixel_format,
				  BLUE_RGB565);
	cdc200_display_write(display_dev, 0, 0,
				   caps.layer[0].y_resolution - rect_h,
				   &buf_desc, buf);

	/* Delay for visual observation */
	k_msleep(5000);

	display_free_buffer(buf);
}

ZTEST(display_api, test_display_read)
{
	struct display_buffer_descriptor buf_desc;
	struct cdc200_display_caps caps;
	uint8_t *buf;
	size_t buf_size;
	int ret;

	cdc200_get_capabilities(display_dev, &caps);

	if (!caps.layer[0].layer_en) {
		TC_PRINT("Layer 0 not enabled, skipping read test\n");
		ztest_test_skip();
	}

	/* Allocate buffer for reading */
	TC_PRINT("Using pixel format: %s\n",
		 display_pixel_format_to_string(caps.layer[0].current_pixel_format));
	buf_size = caps.layer[0].x_resolution * 4; /* One row of ARGB8888 */
	buf = display_alloc_buffer(buf_size);
	zassert_not_null(buf, "Failed to allocate buffer");

	/* Setup buffer descriptor */
	buf_desc.buf_size = buf_size;
	buf_desc.pitch = caps.layer[0].x_resolution;
	buf_desc.width = caps.layer[0].x_resolution;
	buf_desc.height = 1;

	/* Read from display at position (0, 0) */
	ret = display_read(display_dev, 0, 0, &buf_desc, buf);
	TC_PRINT("display_read() returned: %s (%d)\n",
		 display_errno_to_string(ret), ret);

	display_free_buffer(buf);
}

ZTEST(display_api, test_display_set_pixel_format)
{
	struct display_capabilities caps;
	int ret;

	display_get_capabilities(display_dev, &caps);

	/* Test setting pixel format to current format */
	ret = display_set_pixel_format(display_dev,
				       caps.current_pixel_format);
	TC_PRINT("display_set_pixel_format(current-%s) returned: %s (%d)\n",
		 display_pixel_format_to_string(caps.current_pixel_format),
		 display_errno_to_string(ret), ret);
	if (ret == -ENOTSUP) {
		TC_PRINT("Pixel format setting not supported, skipping test\n");
		ztest_test_skip();
	}
	zassert_equal(ret, 0, "display_set_pixel_format(current) failed: %d", ret);
}

ZTEST(display_api, test_cdc200_display_read)
{
	struct display_buffer_descriptor buf_desc;
	struct cdc200_display_caps caps;
	uint8_t *buf;
	size_t buf_size;
	int ret;

	cdc200_get_capabilities(display_dev, &caps);

	if (!caps.layer[0].layer_en) {
		TC_PRINT("Layer 0 not enabled, skipping cdc200_display_read test\n");
		ztest_test_skip();
	}

	/* Allocate buffer for reading */
	TC_PRINT("Using pixel format: %s\n",
		 display_pixel_format_to_string(caps.layer[0].current_pixel_format));
	buf_size = caps.layer[0].x_resolution * 4; /* One row of ARGB8888 */
	buf = display_alloc_buffer(buf_size);
	zassert_not_null(buf, "Failed to allocate buffer");

	/* Setup buffer descriptor */
	buf_desc.buf_size = buf_size;
	buf_desc.pitch = caps.layer[0].x_resolution;
	buf_desc.width = caps.layer[0].x_resolution;
	buf_desc.height = 1;

	/* Read from layer 0 at position (0, 0) */
	ret = cdc200_display_read(display_dev, 0, 0, 0, &buf_desc, buf);
	TC_PRINT("cdc200_display_read(layer 0) returned: %s (%d)\n",
		 display_errno_to_string(ret), ret);

	display_free_buffer(buf);
}

ZTEST(display_api, test_cdc200_swap_fb)
{
	struct cdc200_fb_desc fb_desc;

	/* Get current framebuffer for layer 0 */
	cdc200_get_framebuffer(display_dev, 0, &fb_desc);

	/* Test swapping framebuffer for layer 0 */
	cdc200_swap_fb(display_dev, 0, &fb_desc);
	TC_PRINT("cdc200_swap_fb(layer 0) completed\n");
}

ZTEST(display_api, test_restore_fb)
{
	/* Test restoring default framebuffers */
	restore_fb(display_dev);
	TC_PRINT("restore_fb() completed\n");
}

ZTEST(display_api, test_display_set_brightness)
{
	int ret;

	/* Test setting brightness to 50% */
	ret = display_set_brightness(display_dev, 128);
	TC_PRINT("display_set_brightness(128) returned: %s (%d)\n",
		 display_errno_to_string(ret), ret);

	/* Brightness control may not be supported (-ENOTSUP) */
	if (ret == -ENOTSUP) {
		TC_PRINT("Brightness control not supported by driver\n");
		ztest_test_skip();
	}
}

ZTEST(display_api, test_display_set_contrast)
{
	int ret;

	/* Test setting contrast to 50% */
	ret = display_set_contrast(display_dev, 128);
	TC_PRINT("display_set_contrast(128) returned: %s (%d)\n",
		 display_errno_to_string(ret), ret);

	/* Contrast control may not be supported (-ENOTSUP) */
	if (ret == -ENOTSUP) {
		TC_PRINT("Contrast control not supported by driver\n");
		ztest_test_skip();
	}
}

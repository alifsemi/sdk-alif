/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/*
 * Functional tests for the D/AVE 2D GPU driver (d0/d1/d2).
 *
 * Each case renders into an SRAM0 RGB565 framebuffer and checks pixels.
 * The LVGL samples are not the unit under test.
 */

#include <errno.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/cache.h>
#include <zephyr/linker/section_tags.h>
#include <zephyr/ztest.h>

#include <dave_d0lib.h>
#include <dave_driver.h>

#define FB_W 64
#define FB_H 64

#define D2_RED   0x00FF0000u
#define D2_GREEN 0x0000FF00u
#define D2_BLUE  0x000000FFu

#define RGB565_RED   0xF800u
#define RGB565_GREEN 0x07E0u
#define RGB565_BLUE  0x001Fu
#define RGB565_BLACK 0x0000u

#define DAVE2D_HEAP_SIZE (256 * 1024)

#define PITCH_STRIDE 80
#define PITCH_H      8

static uint8_t __aligned(32) __alif_sram1_section dave2d_heap[DAVE2D_HEAP_SIZE];
static uint16_t __aligned(32) __alif_sram0_section fb[FB_W * FB_H];
static uint16_t __aligned(32) __alif_sram0_section blit_src[8 * 8];
static uint16_t __aligned(32) __alif_sram0_section pitch_fb[PITCH_STRIDE * PITCH_H];
static uint16_t __aligned(32) __alif_sram0_section copy_src[8 * 8];
static uint16_t __aligned(32) __alif_sram0_section copy_dst[8 * 8];
static uint32_t __aligned(32) __alif_sram0_section rgb888_src[8 * 8];
static uint16_t __aligned(32) cache_pat[16];

static d2_device *dev;
static int heap_ready;

static int dave2d_heap_init(void)
{
	heap_ready = d0_initheapmanager(dave2d_heap, sizeof(dave2d_heap), d0_mm_fixed_range,
					NULL, 0, 0, 0, d0_ma_unified) ? 1 : 0;
	return heap_ready ? 0 : -ENOMEM;
}
SYS_INIT(dave2d_heap_init, POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);

static uint16_t fb_px(int x, int y)
{
	return fb[(y * FB_W) + x];
}

static void assert_rgb565(uint16_t got, uint16_t exp, const char *what)
{
	zassert_equal(got, exp, "%s: got 0x%04x, expected 0x%04x", what, got, exp);
}

static int d2_check(d2_s32 ret)
{
	if (ret != D2_OK) {
		TC_PRINT("D/AVE 2D error %d (%s)\n", ret, d2_geterrorstring(dev));
	}

	return ret;
}

static int gpu_begin(void *pixels, size_t bytes, d2_s32 pitch, d2_u32 width, d2_u32 height)
{
	sys_cache_data_flush_and_invd_range(pixels, bytes);

	if (d2_check(d2_startframe(dev)) != D2_OK) {
		return -EIO;
	}
	if (d2_check(d2_framebuffer(dev, pixels, pitch, width, height, d2_mode_rgb565)) != D2_OK) {
		return -EIO;
	}
	if (d2_check(d2_setantialiasing(dev, 0)) != D2_OK ||
	    d2_check(d2_setfillmode(dev, d2_fm_color)) != D2_OK ||
	    d2_check(d2_setalphamode(dev, d2_am_constant)) != D2_OK ||
	    d2_check(d2_setalpha(dev, 255)) != D2_OK ||
	    /* Replace the destination so opaque primitives match RGB565 exactly. */
	    d2_check(d2_setblendmode(dev, d2_bm_one, d2_bm_zero)) != D2_OK) {
		return -EIO;
	}

	return 0;
}

static int gpu_submit(void *pixels, size_t bytes)
{
	/* d2_endframe() only waits for the scene kicked by the previous
	 * d2_startframe(). The commands just recorded are still in the other
	 * render buffer, so submit that buffer and wait for it before the
	 * CPU reads the framebuffer.
	 */
	if (d2_check(d2_endframe(dev)) != D2_OK ||
	    d2_check(d2_startframe(dev)) != D2_OK ||
	    d2_check(d2_flushframe(dev)) != D2_OK) {
		return -EIO;
	}

	sys_cache_data_invd_range(pixels, bytes);
	return 0;
}

static int gpu_open_frame(void)
{
	return gpu_begin(fb, sizeof(fb), FB_W, FB_W, FB_H);
}

static int gpu_close_frame(void)
{
	return gpu_submit(fb, sizeof(fb));
}

static uint16_t pitched_px(const uint16_t *buf, int pitch, int x, int y)
{
	return buf[(y * pitch) + x];
}

static void *dave2d_setup(void)
{
	d2_s32 ret;

	zassert_equal(heap_ready, 1, "D/AVE 2D heap was not initialised");

	dev = d2_opendevice(0);
	zassert_not_null(dev, "d2_opendevice failed");

	ret = d2_inithw(dev, 0);
	zassert_equal(ret, D2_OK, "d2_inithw failed: %d (%s)", ret, d2_geterrorstring(dev));

	TC_PRINT("D/AVE 2D sw %s, hw revision 0x%08x\n", d2_getversionstring(),
		 d2_getrevisionhw(dev));
	zassert_not_equal(d2_getrevisionhw(dev), 0, "GPU revision reads as zero");

	return NULL;
}

static void dave2d_teardown(void *fixture)
{
	ARG_UNUSED(fixture);

	if (dev != NULL) {
		d2_closedevice(dev);
		dev = NULL;
	}
}

ZTEST_SUITE(dave2d_api, NULL, dave2d_setup, NULL, NULL, dave2d_teardown);

ZTEST(dave2d_api, test_clear_rgb565)
{
	zassert_ok(gpu_open_frame());
	zassert_ok(d2_check(d2_clear(dev, D2_RED)));
	zassert_ok(gpu_close_frame());

	assert_rgb565(fb_px(0, 0), RGB565_RED, "origin");
	assert_rgb565(fb_px(FB_W - 1, 0), RGB565_RED, "top-right");
	assert_rgb565(fb_px(0, FB_H - 1), RGB565_RED, "bottom-left");
	assert_rgb565(fb_px(FB_W / 2, FB_H / 2), RGB565_RED, "centre");
}

ZTEST(dave2d_api, test_renderbox)
{
	zassert_ok(gpu_open_frame());
	zassert_ok(d2_check(d2_clear(dev, D2_BLUE)));
	zassert_ok(d2_check(d2_setcolor(dev, 0, D2_GREEN)));
	zassert_ok(d2_check(d2_renderbox(dev, D2_FIX4(8), D2_FIX4(8), D2_FIX4(16), D2_FIX4(16))));
	zassert_ok(gpu_close_frame());

	assert_rgb565(fb_px(12, 12), RGB565_GREEN, "inside box");
	assert_rgb565(fb_px(20, 20), RGB565_GREEN, "inside box far corner");
	assert_rgb565(fb_px(2, 2), RGB565_BLUE, "outside box");
	assert_rgb565(fb_px(40, 40), RGB565_BLUE, "outside box");
}

ZTEST(dave2d_api, test_renderline)
{
	zassert_ok(gpu_open_frame());
	zassert_ok(d2_check(d2_clear(dev, 0)));
	zassert_ok(d2_check(d2_setcolor(dev, 0, D2_RED)));
	zassert_ok(d2_check(d2_renderline(dev, D2_FIX4(4), D2_FIX4(32), D2_FIX4(60), D2_FIX4(32),
					 D2_FIX4(4), 0)));
	zassert_ok(gpu_close_frame());

	assert_rgb565(fb_px(32, 32), RGB565_RED, "line centre");
	assert_rgb565(fb_px(32, 8), RGB565_BLACK, "above line");
}

ZTEST(dave2d_api, test_rendercircle)
{
	zassert_ok(gpu_open_frame());
	zassert_ok(d2_check(d2_clear(dev, 0)));
	zassert_ok(d2_check(d2_setcolor(dev, 0, D2_BLUE)));
	/* Width 0 fills the disc. */
	zassert_ok(d2_check(d2_rendercircle(dev, D2_FIX4(32), D2_FIX4(32), D2_FIX4(12), 0)));
	zassert_ok(gpu_close_frame());

	assert_rgb565(fb_px(32, 32), RGB565_BLUE, "circle centre");
	assert_rgb565(fb_px(2, 2), RGB565_BLACK, "outside circle");
}

ZTEST(dave2d_api, test_rendertri)
{
	zassert_ok(gpu_open_frame());
	zassert_ok(d2_check(d2_clear(dev, 0)));
	zassert_ok(d2_check(d2_setcolor(dev, 0, D2_GREEN)));
	zassert_ok(d2_check(d2_rendertri(dev,
					 D2_FIX4(8), D2_FIX4(8),
					 D2_FIX4(56), D2_FIX4(8),
					 D2_FIX4(32), D2_FIX4(48),
					 0)));
	zassert_ok(gpu_close_frame());

	assert_rgb565(fb_px(32, 16), RGB565_GREEN, "inside triangle");
	assert_rgb565(fb_px(2, 60), RGB565_BLACK, "outside triangle");
}

ZTEST(dave2d_api, test_blitcopy)
{
	int i;

	for (i = 0; i < 8 * 8; i++) {
		blit_src[i] = RGB565_RED;
	}
	sys_cache_data_flush_range(blit_src, sizeof(blit_src));

	zassert_ok(gpu_open_frame());
	zassert_ok(d2_check(d2_clear(dev, 0)));
	zassert_ok(d2_check(d2_setblitsrc(dev, blit_src, 8, 8, 8, d2_mode_rgb565)));
	zassert_ok(d2_check(d2_blitcopy(dev, 8, 8, 0, 0, D2_FIX4(8), D2_FIX4(8),
					D2_FIX4(20), D2_FIX4(20), 0)));
	zassert_ok(gpu_close_frame());

	assert_rgb565(fb_px(22, 22), RGB565_RED, "blitted pixel");
	assert_rgb565(fb_px(4, 4), RGB565_BLACK, "outside blit");
}

ZTEST(dave2d_api, test_alpha_blend)
{
	uint16_t pix;
	unsigned int red;

	zassert_ok(gpu_open_frame());
	zassert_ok(d2_check(d2_clear(dev, 0)));
	zassert_ok(gpu_close_frame());

	zassert_ok(gpu_open_frame());
	zassert_ok(d2_check(d2_setblendmode(dev, d2_bm_alpha, d2_bm_one_minus_alpha)));
	zassert_ok(d2_check(d2_setalpha(dev, 128)));
	zassert_ok(d2_check(d2_setcolor(dev, 0, D2_RED)));
	zassert_ok(d2_check(d2_renderbox(dev, D2_FIX4(8), D2_FIX4(8), D2_FIX4(16), D2_FIX4(16))));
	zassert_ok(gpu_close_frame());

	pix = fb_px(16, 16);
	red = (pix >> 11) & 0x1f;
	TC_PRINT("alpha 128 over black -> 0x%04x (red channel %u)\n", pix, red);
	zassert_between_inclusive(red, 8, 24, "blended red channel %u from pixel 0x%04x", red, pix);
	zassert_true(((pix >> 5) & 0x3f) < 4, "green channel leaked: 0x%04x", pix);
	zassert_true((pix & 0x1f) < 4, "blue channel leaked: 0x%04x", pix);
	assert_rgb565(fb_px(2, 2), RGB565_BLACK, "uncovered pixel");
}

ZTEST(dave2d_api, test_cliprect)
{
	zassert_ok(gpu_open_frame());
	zassert_ok(d2_check(d2_clear(dev, 0)));
	zassert_ok(d2_check(d2_cliprect(dev, 0, 0, 15, 15)));
	zassert_ok(d2_check(d2_setcolor(dev, 0, D2_RED)));
	zassert_ok(d2_check(d2_renderbox(dev, D2_FIX4(0), D2_FIX4(0), D2_FIX4(40), D2_FIX4(40))));
	zassert_ok(gpu_close_frame());

	assert_rgb565(fb_px(8, 8), RGB565_RED, "inside clip");
	assert_rgb565(fb_px(30, 30), RGB565_BLACK, "outside clip");
}

ZTEST(dave2d_api, test_repeated_frames)
{
	int i;

	for (i = 0; i < 32; i++) {
		zassert_ok(gpu_open_frame());
		zassert_ok(d2_check(d2_clear(dev, 0)));
		zassert_ok(d2_check(d2_setcolor(dev, 0, (i & 1) ? D2_RED : D2_BLUE)));
		zassert_ok(d2_check(d2_renderbox(dev, D2_FIX4(8), D2_FIX4(8),
						 D2_FIX4(16), D2_FIX4(16))));
		zassert_ok(gpu_close_frame());
	}

	assert_rgb565(fb_px(16, 16), RGB565_RED, "last frame");
}

ZTEST(dave2d_api, test_framebuffer_pitch)
{
	memset(pitch_fb, 0, sizeof(pitch_fb));
	zassert_ok(gpu_begin(pitch_fb, sizeof(pitch_fb), PITCH_STRIDE, 64, PITCH_H));
	zassert_ok(d2_check(d2_clear(dev, D2_RED)));
	zassert_ok(gpu_submit(pitch_fb, sizeof(pitch_fb)));

	assert_rgb565(pitched_px(pitch_fb, PITCH_STRIDE, 4, 4), RGB565_RED, "pitched pixel");
	assert_rgb565(pitched_px(pitch_fb, PITCH_STRIDE, 70, 0), RGB565_BLACK, "pitch padding");
}

ZTEST(dave2d_api, test_odd_width)
{
	memset(pitch_fb, 0, sizeof(pitch_fb));
	zassert_ok(gpu_begin(pitch_fb, sizeof(pitch_fb), 64, 63, PITCH_H));
	zassert_ok(d2_check(d2_clear(dev, D2_RED)));
	zassert_ok(gpu_submit(pitch_fb, sizeof(pitch_fb)));

	assert_rgb565(pitched_px(pitch_fb, 64, 62, 2), RGB565_RED, "last odd column");
	assert_rgb565(pitched_px(pitch_fb, 64, 63, 2), RGB565_BLACK, "past odd width");
}

ZTEST(dave2d_api, test_edge_boxes)
{
	zassert_ok(gpu_open_frame());
	zassert_ok(d2_check(d2_clear(dev, D2_BLUE)));
	zassert_ok(d2_check(d2_setcolor(dev, 0, D2_GREEN)));
	zassert_ok(d2_check(d2_renderbox(dev, D2_FIX4(0), D2_FIX4(0), D2_FIX4(4), D2_FIX4(4))));
	zassert_ok(d2_check(d2_renderbox(dev, D2_FIX4(60), D2_FIX4(60), D2_FIX4(4), D2_FIX4(4))));
	zassert_ok(gpu_close_frame());

	assert_rgb565(fb_px(0, 0), RGB565_GREEN, "top-left box");
	assert_rgb565(fb_px(3, 3), RGB565_GREEN, "top-left box far corner");
	assert_rgb565(fb_px(60, 60), RGB565_GREEN, "bottom-right box");
	assert_rgb565(fb_px(63, 63), RGB565_GREEN, "bottom-right box far corner");
	assert_rgb565(fb_px(32, 32), RGB565_BLUE, "between boxes");
	assert_rgb565(fb_px(4, 4), RGB565_BLUE, "just outside top-left box");
}

ZTEST(dave2d_api, test_rgb888_blit)
{
	int i;

	for (i = 0; i < 8 * 8; i++) {
		rgb888_src[i] = D2_RED;
	}
	sys_cache_data_flush_range(rgb888_src, sizeof(rgb888_src));

	zassert_ok(gpu_open_frame());
	zassert_ok(d2_check(d2_clear(dev, 0)));
	zassert_ok(d2_check(d2_setblitsrc(dev, rgb888_src, 8, 8, 8, d2_mode_rgb888)));
	zassert_ok(d2_check(d2_blitcopy(dev, 8, 8, 0, 0, D2_FIX4(8), D2_FIX4(8),
					D2_FIX4(20), D2_FIX4(20), 0)));
	zassert_ok(gpu_close_frame());

	assert_rgb565(fb_px(22, 22), RGB565_RED, "rgb888 blit");
	assert_rgb565(fb_px(4, 4), RGB565_BLACK, "outside rgb888 blit");
}

ZTEST(dave2d_api, test_circle_ring)
{
	zassert_ok(gpu_open_frame());
	zassert_ok(d2_check(d2_clear(dev, 0)));
	zassert_ok(d2_check(d2_setcolor(dev, 0, D2_BLUE)));
	zassert_ok(d2_check(d2_rendercircle(dev, D2_FIX4(32), D2_FIX4(32), D2_FIX4(16),
					    D2_FIX4(4))));
	zassert_ok(gpu_close_frame());

	assert_rgb565(fb_px(32, 16), RGB565_BLUE, "on the ring");
	assert_rgb565(fb_px(32, 32), RGB565_BLACK, "hole in the ring");
	assert_rgb565(fb_px(2, 2), RGB565_BLACK, "outside the ring");
}

ZTEST(dave2d_api, test_antialiased_edge)
{
	uint16_t edge;

	zassert_ok(gpu_open_frame());
	zassert_ok(d2_check(d2_clear(dev, D2_BLUE)));
	zassert_ok(d2_check(d2_setantialiasing(dev, 1)));
	zassert_ok(d2_check(d2_setblendmode(dev, d2_bm_alpha, d2_bm_one_minus_alpha)));
	zassert_ok(d2_check(d2_setcolor(dev, 0, D2_GREEN)));
	/* Left edge sits on a half pixel so coverage is not 0 or 1. */
	zassert_ok(d2_check(d2_renderbox(dev, D2_FIX4(8) + 8, D2_FIX4(8), D2_FIX4(16),
					 D2_FIX4(16))));
	zassert_ok(gpu_close_frame());

	assert_rgb565(fb_px(20, 16), RGB565_GREEN, "inside antialiased box");
	assert_rgb565(fb_px(2, 2), RGB565_BLUE, "outside antialiased box");
	edge = fb_px(8, 16);
	zassert_true(edge != RGB565_GREEN && edge != RGB565_BLUE,
		     "edge pixel 0x%04x was not blended", edge);
}

ZTEST(dave2d_api, test_copytovidmem_blit)
{
	int i;
	d1_device *d1 = d2_level1interface(dev);

	memset(copy_src, 0, sizeof(copy_src));
	memset(copy_dst, 0, sizeof(copy_dst));
	sys_cache_data_flush_and_invd_range(copy_src, sizeof(copy_src));
	sys_cache_data_flush_and_invd_range(copy_dst, sizeof(copy_dst));
	for (i = 0; i < 8 * 8; i++) {
		copy_src[i] = RGB565_RED;
	}

	/* The copy itself must make the destination visible to the GPU. */
	zassert_not_equal(d1_copytovidmem(d1, copy_dst, copy_src, sizeof(copy_src), 0), 0,
			  "d1_copytovidmem failed");

	zassert_ok(gpu_open_frame());
	zassert_ok(d2_check(d2_clear(dev, 0)));
	zassert_ok(d2_check(d2_setblitsrc(dev, copy_dst, 8, 8, 8, d2_mode_rgb565)));
	zassert_ok(d2_check(d2_blitcopy(dev, 8, 8, 0, 0, D2_FIX4(8), D2_FIX4(8),
					D2_FIX4(20), D2_FIX4(20), 0)));
	zassert_ok(gpu_close_frame());

	assert_rgb565(fb_px(22, 22), RGB565_RED, "copied blit");
	assert_rgb565(fb_px(4, 4), RGB565_BLACK, "outside copied blit");
}

ZTEST(dave2d_api, test_cacheblock_flush)
{
	int i;
	d1_device *d1 = d2_level1interface(dev);

	for (i = 0; i < 16; i++) {
		cache_pat[i] = 0x1111;
	}
	sys_cache_data_flush_and_invd_range(cache_pat, sizeof(cache_pat));
	for (i = 0; i < 16; i++) {
		cache_pat[i] = 0x2222;
	}

	zassert_not_equal(d1_cacheblockflush(d1, d1_mem_dlist, cache_pat, sizeof(cache_pat)), 0,
			  "d1_cacheblockflush failed");
	zassert_equal(cache_pat[0], 0x2222, "block flush dropped the dirty pattern: 0x%04x",
		      cache_pat[0]);
}

ZTEST(dave2d_api, test_idle_irq_query)
{
	d1_device *d1 = d2_level1interface(dev);
	int ret;

	/* A finished frame can leave one completion posted. Drain it. */
	(void)d1_queryirq(d1, d1_irq_dlist, 1);
	ret = d1_queryirq(d1, d1_irq_dlist, 20);
	zassert_equal(ret, 0, "idle IRQ query returned %d", ret);
}

ZTEST(dave2d_api, test_rejected_arguments)
{
	d2_s32 ret;

	ret = d2_cliprect(dev, 30, 0, 10, 10);
	(void)d2_framebuffer(dev, fb, FB_W, FB_W, FB_H, d2_mode_rgb565);
	zassert_equal(ret, D2_INVALIDWIDTH, "inverted clip returned %d", ret);

	ret = d2_framebuffer(dev, NULL, FB_W, FB_W, FB_H, d2_mode_rgb565);
	(void)d2_framebuffer(dev, fb, FB_W, FB_W, FB_H, d2_mode_rgb565);
	zassert_equal(ret, D2_NOVIDEOMEM, "NULL framebuffer returned %d", ret);

	ret = d2_setblitsrc(dev, NULL, 8, 8, 8, d2_mode_rgb565);
	zassert_equal(ret, D2_NOVIDEOMEM, "NULL blit source returned %d", ret);
}

ZTEST(dave2d_api, test_null_handle)
{
	zassert_equal(d2_geterror(NULL), D2_INVALIDDEVICE, "NULL geterror");
	zassert_equal(d2_selectrendermode(NULL, d2_rm_solid), D2_INVALIDDEVICE,
		      "NULL rendermode");
	zassert_equal(d2_setclipgradient(NULL, 0, 0, 0, 0, 0, 0), D2_INVALIDDEVICE,
		      "NULL clip gradient");
}

ZTEST(dave2d_api, test_degenerate_polygon)
{
	d2_point pts[4] = {
		D2_FIX4(8), D2_FIX4(8),
		D2_FIX4(40), D2_FIX4(8),
	};
	d2_s32 ret;

	zassert_ok(gpu_open_frame());
	zassert_ok(d2_check(d2_clear(dev, 0)));
	ret = d2_renderpolygon(dev, pts, 2, 0);
	zassert_equal(ret, D2_VALUENEGATIVE, "two-point polygon returned %d", ret);
	zassert_ok(gpu_close_frame());

	assert_rgb565(fb_px(16, 8), RGB565_BLACK, "degenerate polygon drew");
}

ZTEST(dave2d_api, test_zero_size_box)
{
	zassert_ok(gpu_open_frame());
	zassert_ok(d2_check(d2_clear(dev, D2_BLUE)));
	zassert_ok(d2_check(d2_setcolor(dev, 0, D2_GREEN)));
	zassert_ok(d2_check(d2_renderbox(dev, D2_FIX4(16), D2_FIX4(16), 0, D2_FIX4(16))));
	zassert_ok(d2_check(d2_renderbox(dev, D2_FIX4(16), D2_FIX4(16), D2_FIX4(16), 0)));
	zassert_ok(gpu_close_frame());

	assert_rgb565(fb_px(20, 20), RGB565_BLUE, "zero-size box drew");
	assert_rgb565(fb_px(2, 2), RGB565_BLUE, "corner after zero-size box");
}

ZTEST(dave2d_api, test_offscreen_box)
{
	zassert_ok(gpu_open_frame());
	zassert_ok(d2_check(d2_clear(dev, D2_BLUE)));
	zassert_ok(d2_check(d2_setcolor(dev, 0, D2_RED)));
	zassert_ok(d2_check(d2_renderbox(dev, D2_FIX4(80), D2_FIX4(80), D2_FIX4(8), D2_FIX4(8))));
	zassert_ok(d2_check(d2_renderbox(dev, D2_FIX4(-16), D2_FIX4(10), D2_FIX4(8), D2_FIX4(8))));
	zassert_ok(gpu_close_frame());

	assert_rgb565(fb_px(0, 10), RGB565_BLUE, "box left of the framebuffer drew");
	assert_rgb565(fb_px(32, 32), RGB565_BLUE, "offscreen box drew");
	assert_rgb565(fb_px(63, 63), RGB565_BLUE, "corner after offscreen box");
}

ZTEST(dave2d_api, test_illegal_blend)
{
	d2_s32 ret;

	ret = d2_setblendmode(dev, 99, d2_bm_zero);
	zassert_equal(ret, D2_INVALIDENUM, "illegal source factor returned %d", ret);

	ret = d2_setblendmode(dev, d2_bm_one, 99);
	zassert_equal(ret, D2_INVALIDENUM, "illegal dest factor returned %d", ret);

	zassert_ok(d2_check(d2_setblendmode(dev, d2_bm_one, d2_bm_zero)));
}

ZTEST(dave2d_api, test_illegal_rendermode)
{
	d2_s32 ret;

	ret = d2_selectrendermode(dev, 0);
	(void)d2_selectrendermode(dev, d2_rm_solid);
	zassert_equal(ret, D2_ILLEGALMODE, "rendermode 0 returned %d", ret);
}

ZTEST(dave2d_api, test_tiny_framebuffer)
{
	d2_s32 ret;

	ret = d2_framebuffer(dev, fb, FB_W, 1, 1, d2_mode_rgb565);
	(void)d2_framebuffer(dev, fb, FB_W, FB_W, FB_H, d2_mode_rgb565);
	zassert_equal(ret, D2_VALUETOOSMALL, "1x1 framebuffer returned %d", ret);
}

ZTEST(dave2d_api, test_inverted_height)
{
	d2_s32 ret;

	ret = d2_cliprect(dev, 0, 30, 10, 10);
	(void)d2_framebuffer(dev, fb, FB_W, FB_W, FB_H, d2_mode_rgb565);
	zassert_equal(ret, D2_INVALIDHEIGHT, "inverted clip height returned %d", ret);
}

/* The cases below call inputs the release driver does not reject. A miss
 * can corrupt the context, queue a bad primitive, or fault the CPU. They
 * sort last so the cases above still report.
 */
ZTEST(dave2d_api, test_z_negative_radius)
{
	d2_s32 ret;

	zassert_ok(gpu_open_frame());
	zassert_ok(d2_check(d2_clear(dev, 0)));
	ret = d2_rendercircle(dev, D2_FIX4(32), D2_FIX4(32), (d2_width)(-D2_FIX4(8)), 0);
	/* Submit either way, so a command the driver accepted runs in this case. */
	(void)gpu_close_frame();
	zassert_equal(ret, D2_VALUENEGATIVE, "negative radius returned %d", ret);
	assert_rgb565(fb_px(32, 32), RGB565_BLACK, "negative radius still drew");
}

ZTEST(dave2d_api, test_zz_color_index)
{
	d2_s32 ret;

	ret = d2_setcolor(dev, 5, D2_RED);
	zassert_equal(ret, D2_INVALIDINDEX, "colour index 5 returned %d", ret);
}

ZTEST(dave2d_api, test_zzz_second_inithw)
{
	d2_device *extra;
	d2_s32 ret;

	extra = d2_opendevice(0);
	zassert_not_null(extra, "second d2_opendevice failed");

	ret = d2_inithw(extra, 0);
	/* Closing a second device that did bind calls d1_closedevice and stops
	 * the GPU clock while the suite device is still open.
	 */
	if (ret != D2_OK) {
		(void)d2_closedevice(extra);
	}
	zassert_equal(ret, D2_HWINUSE, "second d2_inithw returned %d", ret);
}

ZTEST(dave2d_api, test_zzzz_null_render)
{
	d2_s32 ret;

	ret = d2_renderbox(NULL, D2_FIX4(0), D2_FIX4(0), D2_FIX4(4), D2_FIX4(4));
	zassert_equal(ret, D2_INVALIDDEVICE, "NULL renderbox returned %d", ret);
}

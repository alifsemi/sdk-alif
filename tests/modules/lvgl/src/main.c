/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/*
 * Tests for LVGL drawing through the D/AVE 2D unit.
 *
 * Primitive cases render into a 64x64 RGB565 canvas in SRAM0 and check pixels.
 * The widget case builds the common widget set and checks that a solid object
 * survives the LVGL flush into the CDC200 framebuffer.
 */

#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/device.h>
#include <zephyr/cache.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/display/cdc200.h>
#include <zephyr/linker/section_tags.h>
#include <zephyr/ztest.h>
#if defined(CONFIG_MIPI_DSI)
#include <zephyr/drivers/mipi_dsi/dsi_dw.h>
#endif

#include <lvgl.h>
#include <dave_d0lib.h>
#include "core/lv_global.h"

#define CANVAS_W 64
#define CANVAS_H 64

#define RGB565_RED   0xF800u
#define RGB565_GREEN 0x07E0u
#define RGB565_BLACK 0x0000u

#define DAVE2D_HEAP_SIZE (512 * 1024)

static uint8_t __aligned(32) __alif_sram1_section dave2d_heap[DAVE2D_HEAP_SIZE];
static uint8_t __aligned(64) __alif_sram0_section canvas_mem[CANVAS_W * CANVAS_H * 2];
static uint16_t __aligned(32) __alif_sram0_section image_px[8 * 8];
static uint8_t __aligned(32) __alif_sram0_section rgb888_px[8 * 8 * 3];

static lv_obj_t *canvas;
static lv_image_dsc_t image_dsc;
static int heap_ready;
static int display_up;

static int dave2d_heap_init(void)
{
	heap_ready = d0_initheapmanager(dave2d_heap, sizeof(dave2d_heap), d0_mm_fixed_range,
					NULL, 0, 0, 0, d0_ma_unified) ? 1 : 0;
	return heap_ready ? 0 : -ENOMEM;
}
SYS_INIT(dave2d_heap_init, POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);

static int display_pipeline_start(void)
{
	const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
	int ret;

	if (!device_is_ready(display_dev)) {
		TC_PRINT("display device not ready\n");
		return -ENODEV;
	}

#if defined(CONFIG_MIPI_DSI)
	{
		const struct device *dsi = DEVICE_DT_GET(DT_NODELABEL(mipi_dsi));

		if (!device_is_ready(dsi)) {
			TC_PRINT("MIPI-DSI host not ready\n");
			return -ENODEV;
		}

		ret = dsi_dw_set_mode(dsi, DSI_DW_VIDEO_MODE);
		if (ret) {
			TC_PRINT("DSI video mode failed: %d\n", ret);
			return ret;
		}
	}
#endif

#if DT_NODE_HAS_STATUS(DT_ALIAS(panel), okay)
	{
		const struct device *panel = DEVICE_DT_GET(DT_ALIAS(panel));

		if (!device_is_ready(panel)) {
			TC_PRINT("panel device not ready\n");
			return -ENODEV;
		}

		ret = display_blanking_off(panel);
		if (ret) {
			TC_PRINT("panel blanking off failed: %d\n", ret);
			return ret;
		}
	}
#endif

	cdc200_set_enable(display_dev, true);
	return 0;
}

static bool dave2d_unit_registered(void)
{
	lv_draw_unit_t *unit;

	for (unit = LV_GLOBAL_DEFAULT()->draw_info.unit_head; unit != NULL; unit = unit->next) {
		if (unit->name != NULL && strcmp(unit->name, "DAVE2D") == 0) {
			return true;
		}
	}

	return false;
}

static uint16_t canvas_px(int x, int y)
{
	const lv_draw_buf_t *buf = lv_canvas_get_draw_buf(canvas);
	uint16_t pix;

	memcpy(&pix, buf->data + (y * buf->header.stride) + (x * 2), sizeof(pix));
	return pix;
}

static unsigned int canvas_ink(uint16_t background)
{
	unsigned int n = 0;
	int x;
	int y;

	for (y = 0; y < CANVAS_H; y++) {
		for (x = 0; x < CANVAS_W; x++) {
			if (canvas_px(x, y) != background) {
				n++;
			}
		}
	}

	return n;
}

static void assert_rgb565_near(uint16_t got, uint16_t exp, const char *what)
{
	int dr = (int)((got >> 11) & 0x1f) - (int)((exp >> 11) & 0x1f);
	int dg = (int)((got >> 5) & 0x3f) - (int)((exp >> 5) & 0x3f);
	int db = (int)(got & 0x1f) - (int)(exp & 0x1f);

	if (dr < 0) {
		dr = -dr;
	}
	if (dg < 0) {
		dg = -dg;
	}
	if (db < 0) {
		db = -db;
	}

	zassert_true(dr <= 1 && dg <= 2 && db <= 1,
		     "%s: got 0x%04x, expected 0x%04x", what, got, exp);
}

static void canvas_draw(void (*draw)(lv_layer_t *layer))
{
	lv_layer_t layer;

	lv_canvas_fill_bg(canvas, lv_color_black(), LV_OPA_COVER);
	sys_cache_data_flush_and_invd_range(canvas_mem, sizeof(canvas_mem));
	lv_canvas_init_layer(canvas, &layer);
	draw(&layer);
	lv_canvas_finish_layer(canvas, &layer);
	sys_cache_data_invd_range(canvas_mem, sizeof(canvas_mem));
}

static void draw_solid_rect(lv_layer_t *layer)
{
	lv_draw_rect_dsc_t dsc;
	lv_area_t area = {8, 8, 39, 39};

	lv_draw_rect_dsc_init(&dsc);
	dsc.bg_opa = LV_OPA_COVER;
	dsc.bg_color = lv_color_hex(0xFF0000);
	dsc.border_width = 0;
	dsc.radius = 0;
	lv_draw_rect(layer, &dsc, &area);
}

static void draw_blended_rect(lv_layer_t *layer)
{
	lv_draw_rect_dsc_t dsc;
	lv_area_t area = {8, 8, 39, 39};

	lv_draw_rect_dsc_init(&dsc);
	dsc.bg_opa = 128;
	dsc.bg_color = lv_color_hex(0xFF0000);
	dsc.border_width = 0;
	dsc.radius = 0;
	lv_draw_rect(layer, &dsc, &area);
}

static void draw_border(lv_layer_t *layer)
{
	lv_draw_rect_dsc_t dsc;
	lv_area_t area = {8, 8, 55, 55};

	lv_draw_rect_dsc_init(&dsc);
	dsc.bg_opa = LV_OPA_TRANSP;
	dsc.border_width = 6;
	dsc.border_opa = LV_OPA_COVER;
	dsc.border_color = lv_color_hex(0x00FF00);
	dsc.border_side = LV_BORDER_SIDE_FULL;
	dsc.radius = 0;
	lv_draw_rect(layer, &dsc, &area);
}

static void draw_image(lv_layer_t *layer)
{
	lv_draw_image_dsc_t dsc;
	lv_area_t area = {4, 4, 11, 11};

	lv_draw_image_dsc_init(&dsc);
	dsc.src = &image_dsc;
	lv_draw_image(layer, &dsc, &area);
}

static void draw_label(lv_layer_t *layer)
{
	lv_draw_label_dsc_t dsc;
	lv_area_t area = {2, 2, 62, 30};

	lv_draw_label_dsc_init(&dsc);
	dsc.text = "LVGL";
	dsc.font = LV_FONT_DEFAULT;
	dsc.color = lv_color_white();
	dsc.opa = LV_OPA_COVER;
	lv_draw_label(layer, &dsc, &area);
}

static void draw_line(lv_layer_t *layer)
{
	lv_draw_line_dsc_t dsc;

	lv_draw_line_dsc_init(&dsc);
	dsc.p1.x = 4;
	dsc.p1.y = 32;
	dsc.p2.x = 60;
	dsc.p2.y = 32;
	dsc.width = 4;
	dsc.color = lv_color_hex(0x0000FF);
	dsc.opa = LV_OPA_COVER;
	lv_draw_line(layer, &dsc);
}

static void draw_arc(lv_layer_t *layer)
{
	lv_draw_arc_dsc_t dsc;

	lv_draw_arc_dsc_init(&dsc);
	dsc.center.x = 32;
	dsc.center.y = 32;
	dsc.radius = 22;
	dsc.width = 6;
	dsc.start_angle = 0;
	dsc.end_angle = 270;
	dsc.color = lv_color_white();
	dsc.opa = LV_OPA_COVER;
	dsc.rounded = 0;
	lv_draw_arc(layer, &dsc);
}

static void draw_rounded_rect(lv_layer_t *layer)
{
	lv_draw_rect_dsc_t dsc;
	lv_area_t area = {8, 8, 39, 39};

	lv_draw_rect_dsc_init(&dsc);
	dsc.bg_opa = LV_OPA_COVER;
	dsc.bg_color = lv_color_hex(0xFF0000);
	dsc.radius = 12;
	dsc.border_width = 0;
	lv_draw_rect(layer, &dsc, &area);
}

static void draw_gradient(lv_layer_t *layer)
{
	lv_draw_rect_dsc_t dsc;
	lv_area_t area = {8, 8, 39, 39};

	lv_draw_rect_dsc_init(&dsc);
	dsc.bg_opa = LV_OPA_COVER;
	dsc.bg_grad.dir = LV_GRAD_DIR_HOR;
	dsc.bg_grad.stops_count = 2;
	dsc.bg_grad.stops[0].color = lv_color_hex(0xFF0000);
	dsc.bg_grad.stops[0].opa = LV_OPA_COVER;
	dsc.bg_grad.stops[0].frac = 0;
	dsc.bg_grad.stops[1].color = lv_color_hex(0x0000FF);
	dsc.bg_grad.stops[1].opa = LV_OPA_COVER;
	dsc.bg_grad.stops[1].frac = 255;
	lv_draw_rect(layer, &dsc, &area);
}

static void draw_shadow(lv_layer_t *layer)
{
	lv_draw_box_shadow_dsc_t dsc;
	lv_area_t area = {16, 16, 40, 40};

	lv_draw_box_shadow_dsc_init(&dsc);
	dsc.color = lv_color_white();
	dsc.opa = LV_OPA_COVER;
	dsc.width = 6;
	dsc.ofs_x = 0;
	dsc.ofs_y = 0;
	lv_draw_box_shadow(layer, &dsc, &area);
}

static void draw_scaled_image(lv_layer_t *layer)
{
	lv_draw_image_dsc_t dsc;
	lv_area_t area = {4, 4, 11, 11};

	lv_draw_image_dsc_init(&dsc);
	dsc.src = &image_dsc;
	dsc.scale_x = 512;
	dsc.scale_y = 512;
	dsc.pivot.x = 0;
	dsc.pivot.y = 0;
	lv_draw_image(layer, &dsc, &area);
}

static void draw_rgb888_image(lv_layer_t *layer)
{
	lv_draw_image_dsc_t dsc;
	lv_image_dsc_t rgb888;
	lv_area_t area = {4, 4, 11, 11};
	int i;

	for (i = 0; i < 8 * 8 * 3; i++) {
		rgb888_px[i] = 0xff;
	}
	sys_cache_data_flush_range(rgb888_px, sizeof(rgb888_px));

	memset(&rgb888, 0, sizeof(rgb888));
	rgb888.header.magic = LV_IMAGE_HEADER_MAGIC;
	rgb888.header.cf = LV_COLOR_FORMAT_RGB888;
	rgb888.header.w = 8;
	rgb888.header.h = 8;
	rgb888.header.stride = 24;
	rgb888.data_size = sizeof(rgb888_px);
	rgb888.data = rgb888_px;

	lv_draw_image_dsc_init(&dsc);
	dsc.src = &rgb888;
	lv_draw_image(layer, &dsc, &area);
}

static void draw_triangle(lv_layer_t *layer)
{
	lv_draw_triangle_dsc_t dsc;

	lv_draw_triangle_dsc_init(&dsc);
	dsc.p[0].x = 8;
	dsc.p[0].y = 8;
	dsc.p[1].x = 56;
	dsc.p[1].y = 8;
	dsc.p[2].x = 32;
	dsc.p[2].y = 48;
	dsc.color = lv_color_hex(0x00FF00);
	dsc.opa = LV_OPA_COVER;
	lv_draw_triangle(layer, &dsc);
}

static void *lvgl_setup(void)
{
	int i;

	zassert_equal(heap_ready, 1, "D/AVE 2D heap was not initialised");
	zassert_not_null(lv_display_get_default(), "LVGL display was not created");
	zassert_true(dave2d_unit_registered(), "DAVE2D draw unit is not registered");

	display_up = (display_pipeline_start() == 0);

	memset(&image_dsc, 0, sizeof(image_dsc));
	for (i = 0; i < 8 * 8; i++) {
		image_px[i] = RGB565_GREEN;
	}
	sys_cache_data_flush_range(image_px, sizeof(image_px));
	image_dsc.header.magic = LV_IMAGE_HEADER_MAGIC;
	image_dsc.header.cf = LV_COLOR_FORMAT_RGB565;
	image_dsc.header.w = 8;
	image_dsc.header.h = 8;
	image_dsc.header.stride = 16;
	image_dsc.data_size = sizeof(image_px);
	image_dsc.data = (const uint8_t *)image_px;

	canvas = lv_canvas_create(lv_screen_active());
	zassert_not_null(canvas, "canvas create failed");
	lv_canvas_set_buffer(canvas, canvas_mem, CANVAS_W, CANVAS_H, LV_COLOR_FORMAT_RGB565);
	lv_obj_set_pos(canvas, 200, 200);

	return NULL;
}

ZTEST_SUITE(lvgl_dave2d, NULL, lvgl_setup, NULL, NULL, NULL);

ZTEST(lvgl_dave2d, test_draw_unit_registered)
{
	zassert_true(dave2d_unit_registered(), "DAVE2D draw unit is not registered");
}

ZTEST(lvgl_dave2d, test_fill_rect)
{
	canvas_draw(draw_solid_rect);

	assert_rgb565_near(canvas_px(20, 20), RGB565_RED, "rect centre");
	assert_rgb565_near(canvas_px(2, 2), RGB565_BLACK, "outside rect");
}

ZTEST(lvgl_dave2d, test_fill_opacity)
{
	uint16_t pix;
	unsigned int red;

	canvas_draw(draw_blended_rect);
	pix = canvas_px(20, 20);
	red = (pix >> 11) & 0x1f;
	TC_PRINT("LVGL opa 128 red over black -> 0x%04x (red %u)\n", pix, red);
	zassert_between_inclusive(red, 8, 24, "blended red channel %u from 0x%04x", red, pix);
	assert_rgb565_near(canvas_px(2, 2), RGB565_BLACK, "outside blended rect");
}

ZTEST(lvgl_dave2d, test_border)
{
	canvas_draw(draw_border);

	assert_rgb565_near(canvas_px(10, 32), RGB565_GREEN, "border");
	assert_rgb565_near(canvas_px(32, 32), RGB565_BLACK, "border interior");
}

ZTEST(lvgl_dave2d, test_image_blit)
{
	canvas_draw(draw_image);

	assert_rgb565_near(canvas_px(6, 6), RGB565_GREEN, "image pixel");
	assert_rgb565_near(canvas_px(20, 20), RGB565_BLACK, "outside image");
}

ZTEST(lvgl_dave2d, test_label)
{
	unsigned int ink;

	canvas_draw(draw_label);
	ink = canvas_ink(RGB565_BLACK);
	TC_PRINT("label ink pixels: %u\n", ink);
	zassert_true(ink > 8, "label drew %u non-background pixels", ink);
}

ZTEST(lvgl_dave2d, test_line)
{
	uint16_t pix;

	canvas_draw(draw_line);
	pix = canvas_px(32, 32);
	TC_PRINT("line pixel 0x%04x\n", pix);
	zassert_true((pix & 0x1f) > 8, "line blue channel missing: 0x%04x", pix);
	assert_rgb565_near(canvas_px(32, 8), RGB565_BLACK, "above line");
}

ZTEST(lvgl_dave2d, test_arc)
{
	unsigned int ink;

	canvas_draw(draw_arc);
	ink = canvas_ink(RGB565_BLACK);
	TC_PRINT("arc ink pixels: %u\n", ink);
	zassert_true(ink > 20, "arc drew %u non-background pixels", ink);
}

ZTEST(lvgl_dave2d, test_triangle)
{
	canvas_draw(draw_triangle);

	assert_rgb565_near(canvas_px(32, 16), RGB565_GREEN, "inside triangle");
	assert_rgb565_near(canvas_px(4, 60), RGB565_BLACK, "outside triangle");
}

ZTEST(lvgl_dave2d, test_widgets_refresh_and_readback)
{
	const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
	static const lv_point_precise_t line_pts[] = {
		{0, 0}, {40, 16}, {80, 0},
	};
	lv_obj_t *screen = lv_screen_active();
	lv_obj_t *box;
	lv_obj_t *label;
	lv_obj_t *button;
	lv_obj_t *slider;
	lv_obj_t *bar;
	lv_obj_t *sw;
	lv_obj_t *checkbox;
	lv_obj_t *arc;
	lv_obj_t *led;
	lv_obj_t *line;
	lv_obj_t *image;
	struct display_buffer_descriptor desc = {
		.buf_size = sizeof(uint16_t),
		.width = 1,
		.height = 1,
		.pitch = 1,
	};
	uint16_t pix = 0;
	uint16_t expect;
	int ret;

	zassert_true(display_up, "display pipeline did not start");

	lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
	lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

	box = lv_obj_create(screen);
	lv_obj_remove_style_all(box);
	lv_obj_set_pos(box, 8, 8);
	lv_obj_set_size(box, 48, 48);
	lv_obj_set_style_bg_color(box, lv_color_hex(0xFF0000), 0);
	lv_obj_set_style_bg_opa(box, LV_OPA_COVER, 0);
	lv_obj_set_style_radius(box, 0, 0);
	lv_obj_set_style_border_width(box, 0, 0);

	label = lv_label_create(screen);
	lv_label_set_text(label, "UI example");
	lv_obj_set_pos(label, 8, 70);

	button = lv_button_create(screen);
	lv_obj_set_pos(button, 8, 110);
	lv_obj_set_size(button, 120, 40);
	lv_label_set_text(lv_label_create(button), "Button");

	slider = lv_slider_create(screen);
	lv_obj_set_pos(slider, 8, 170);
	lv_obj_set_width(slider, 160);
	lv_slider_set_value(slider, 40, LV_ANIM_OFF);

	bar = lv_bar_create(screen);
	lv_obj_set_pos(bar, 8, 210);
	lv_obj_set_size(bar, 160, 16);
	lv_bar_set_value(bar, 70, LV_ANIM_OFF);

	sw = lv_switch_create(screen);
	lv_obj_set_pos(sw, 8, 240);
	lv_obj_add_state(sw, LV_STATE_CHECKED);

	checkbox = lv_checkbox_create(screen);
	lv_obj_set_pos(checkbox, 8, 280);
	lv_checkbox_set_text(checkbox, "Checkbox");
	lv_obj_add_state(checkbox, LV_STATE_CHECKED);

	arc = lv_arc_create(screen);
	lv_obj_set_pos(arc, 8, 320);
	lv_obj_set_size(arc, 80, 80);
	lv_arc_set_value(arc, 60);

	led = lv_led_create(screen);
	lv_obj_set_pos(led, 8, 420);
	lv_led_set_color(led, lv_color_hex(0x00FF00));
	lv_led_on(led);

	line = lv_line_create(screen);
	lv_obj_set_pos(line, 8, 470);
	lv_line_set_points(line, line_pts, ARRAY_SIZE(line_pts));

	image = lv_image_create(screen);
	lv_obj_set_pos(image, 8, 520);
	lv_image_set_src(image, &image_dsc);

	lv_obj_invalidate(screen);
	lv_refr_now(NULL);

	ret = cdc200_display_read(display_dev, 0, 24, 24, &desc, &pix);
	zassert_equal(ret, 0, "cdc200_display_read failed: %d", ret);

	expect = lv_color_to_u16(lv_color_hex(0xFF0000));
	TC_PRINT("screen readback 0x%04x, expect 0x%04x\n", pix, expect);
	assert_rgb565_near(pix, expect, "flushed red object");

	pix = 0xffff;
	ret = cdc200_display_read(display_dev, 0, 400, 40, &desc, &pix);
	zassert_equal(ret, 0, "background read failed: %d", ret);
	assert_rgb565_near(pix, RGB565_BLACK, "panel background");

	pix = 0;
	ret = cdc200_display_read(display_dev, 0, 10, 522, &desc, &pix);
	zassert_equal(ret, 0, "image read failed: %d", ret);
	assert_rgb565_near(pix, RGB565_GREEN, "flushed green image");

	lv_obj_delete(image);
	lv_obj_delete(line);
	lv_obj_delete(led);
	lv_obj_delete(arc);
	lv_obj_delete(checkbox);
	lv_obj_delete(sw);
	lv_obj_delete(bar);
	lv_obj_delete(slider);
	lv_obj_delete(button);
	lv_obj_delete(label);
	lv_obj_delete(box);
}

ZTEST(lvgl_dave2d, test_rounded_rect)
{
	canvas_draw(draw_rounded_rect);

	assert_rgb565_near(canvas_px(24, 24), RGB565_RED, "rounded centre");
	assert_rgb565_near(canvas_px(8, 8), RGB565_BLACK, "rounded corner");
}

ZTEST(lvgl_dave2d, test_horizontal_gradient)
{
	uint16_t left;
	uint16_t right;

	canvas_draw(draw_gradient);
	left = canvas_px(10, 20);
	right = canvas_px(36, 20);
	TC_PRINT("gradient left 0x%04x right 0x%04x\n", left, right);
	zassert_true(((left >> 11) & 0x1f) > 20, "gradient start is not red: 0x%04x", left);
	zassert_true((left & 0x1f) < 8, "gradient start leaked blue: 0x%04x", left);
	zassert_true((right & 0x1f) > 20, "gradient end is not blue: 0x%04x", right);
	zassert_true(((right >> 11) & 0x1f) < 8, "gradient end leaked red: 0x%04x", right);
}

ZTEST(lvgl_dave2d, test_box_shadow)
{
	unsigned int ink;

	canvas_draw(draw_shadow);
	ink = canvas_ink(RGB565_BLACK);
	TC_PRINT("shadow ink pixels: %u\n", ink);
	zassert_true(ink > 10, "shadow drew %u non-background pixels", ink);
	assert_rgb565_near(canvas_px(2, 2), RGB565_BLACK, "far from shadow");
}

ZTEST(lvgl_dave2d, test_scaled_image)
{
	canvas_draw(draw_scaled_image);

	assert_rgb565_near(canvas_px(6, 6), RGB565_GREEN, "scaled image origin");
	assert_rgb565_near(canvas_px(16, 16), RGB565_GREEN, "scaled image extent");
	assert_rgb565_near(canvas_px(40, 40), RGB565_BLACK, "outside scaled image");
}

ZTEST(lvgl_dave2d, test_rgb888_image)
{
	uint16_t pix;

	canvas_draw(draw_rgb888_image);
	pix = canvas_px(6, 6);
	TC_PRINT("rgb888 image pixel 0x%04x\n", pix);
	zassert_true(((pix >> 11) & 0x1f) > 20 && ((pix >> 5) & 0x3f) > 40 && (pix & 0x1f) > 20,
		     "rgb888 image pixel 0x%04x is not white", pix);
	assert_rgb565_near(canvas_px(20, 20), RGB565_BLACK, "outside rgb888 image");
}

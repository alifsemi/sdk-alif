/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/device.h>
#include <zephyr/drivers/clock_control.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/display/cdc200.h>
#if defined(CONFIG_MIPI_DSI)
#include <zephyr/drivers/mipi_dsi/dsi_dw.h>
#endif
#include <zephyr/cache.h>
#include <zephyr/linker/section_tags.h>
#include <zephyr/logging/log.h>
#include <se_service.h>
#include <soc_common.h>

#include <lvgl.h>
#include <lv_demos.h>

#include <dave_d0lib.h>

#if defined(CONFIG_APP_LVGL_HEAP_STATS)
#include <lvgl_mem.h>
#endif

LOG_MODULE_REGISTER(app, CONFIG_LOG_DEFAULT_LEVEL);

#if DT_NODE_HAS_STATUS(DT_ALIAS(panel), okay)
static int display_panel_on(void)
{
	const struct device *panel_dev = DEVICE_DT_GET(DT_ALIAS(panel));
	int ret;

	if (!device_is_ready(panel_dev)) {
		LOG_ERR("Display panel device not ready");
		return -ENODEV;
	}

	ret = display_blanking_off(panel_dev);
	if (ret) {
		LOG_ERR("Failed to turn display panel on (%d)", ret);
	}

	return ret;
}
#endif

/*
 * D/AVE 2D working memory (display lists, render buffers, ...). The LVGL D/AVE 2D
 * draw unit allocates from this heap inside lv_draw_dave2d_init(), which runs as
 * part of lv_init(). Zephyr's LVGL glue calls lv_init() automatically from a
 * SYS_INIT() hook at the APPLICATION init level, so the heap must already be
 * initialised by then - hence the POST_KERNEL SYS_INIT() below, which always
 * runs before any APPLICATION-level hook.
 *
 * The heap lives in SRAM1, and the LVGL rendering buffers + memory pool live in
 * SRAM0 (see lvgl_sram.ld) - both globally addressable, so the GPU can reach
 * them. In DIRECT mode the full-screen VDBs in SRAM0 double as the CDC200
 * scan-out framebuffers (zero-copy page-flip via cdc200_swap_fb).
 */
#define DAVE2D_HEAP_SIZE CONFIG_APP_DAVE2D_HEAP_SIZE
BUILD_ASSERT(DAVE2D_HEAP_SIZE % CONFIG_D0_FIXED_RANGE_ALIGNMENT == 0,
	     "D/AVE 2D heap size must be a multiple of the d0 allocator alignment");
static uint8_t __alif_sram1_section
	__aligned(CONFIG_D0_FIXED_RANGE_ALIGNMENT) dave2d_heap[DAVE2D_HEAP_SIZE];

#if defined(CONFIG_APP_DAVE2D_HEAP_HWM)
/*
 * The D/AVE 2D fixed-range allocator is first-fit from the bottom of the heap
 * and writes a block header right after every allocation it splits off, so the
 * highest byte that no longer holds the fill pattern marks the peak extent.
 */
#define DAVE2D_HEAP_FILL 0xA5A5A5A5U

static uint32_t dave2d_heap_hwm(void)
{
	const uint32_t *words = (const uint32_t *)dave2d_heap;
	size_t i = DAVE2D_HEAP_SIZE / sizeof(uint32_t);

	while (i > 0 && words[i - 1] == DAVE2D_HEAP_FILL) {
		i--;
	}

	return i * sizeof(uint32_t);
}
#endif

#if defined(CONFIG_APP_DAVE2D_HEAP_HWM) || defined(CONFIG_APP_LVGL_HEAP_STATS)
static void benchmark_end_cb(const lv_demo_benchmark_summary_t *summary)
{
#if defined(CONFIG_APP_DAVE2D_HEAP_HWM)
	uint32_t hwm = dave2d_heap_hwm();
#endif
#if defined(CONFIG_APP_LVGL_HEAP_STATS)
	struct sys_memory_stats stats;
	size_t stack_size = k_current_get()->stack_info.size;
	size_t stack_unused = 0;

	lvgl_heap_stats(&stats);
	(void)k_thread_stack_space_get(k_current_get(), &stack_unused);
#endif

	lv_demo_benchmark_summary_display(summary);

#if defined(CONFIG_APP_DAVE2D_HEAP_HWM)
	LOG_INF("D/AVE 2D heap high-water mark: %u of %u bytes (%u%%)", (unsigned int)hwm,
		(unsigned int)DAVE2D_HEAP_SIZE, (unsigned int)((hwm * 100U) / DAVE2D_HEAP_SIZE));
#endif
#if defined(CONFIG_APP_LVGL_HEAP_STATS)
	LOG_INF("LVGL pool: %u allocated, %u peak of %u bytes (peak %u%%)",
		(unsigned int)stats.allocated_bytes, (unsigned int)stats.max_allocated_bytes,
		(unsigned int)CONFIG_LV_Z_MEM_POOL_SIZE,
		(unsigned int)((stats.max_allocated_bytes * 100U) / CONFIG_LV_Z_MEM_POOL_SIZE));
	LOG_INF("Main stack: %u peak of %u bytes (%u%%)", (unsigned int)(stack_size - stack_unused),
		(unsigned int)stack_size,
		(unsigned int)(((stack_size - stack_unused) * 100U) / stack_size));
#endif
}
#endif

static int dave2d_heap_init(void)
{
#if defined(CONFIG_APP_DAVE2D_HEAP_HWM)
	for (size_t i = 0; i < DAVE2D_HEAP_SIZE / sizeof(uint32_t); i++) {
		((uint32_t *)dave2d_heap)[i] = DAVE2D_HEAP_FILL;
	}
#endif

	if (!d0_initheapmanager(dave2d_heap, sizeof(dave2d_heap), d0_mm_fixed_range,
				NULL, 0, 0, 0, d0_ma_unified)) {
		LOG_ERR("Failed to initialise the D/AVE 2D heap manager");
		return -ENOMEM;
	}

	return 0;
}
SYS_INIT(dave2d_heap_init, POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);

/*
 * DIRECT-mode flush callback with vsync synchronization. In DIRECT mode the
 * two full-screen LVGL VDBs ARE the scan-out framebuffers (both in SRAM0).
 * The GPU renders directly into the back buffer, and this flush callback
 * page-flips via cdc200_swap_fb() then waits for the CDC200 LINE IRQ to
 * actually process the swap. This ensures:
 *  - No tearing (buffer swap happens at vblank)
 *  - Smooth frame pacing (naturally locked to display refresh)
 *  - LVGL doesn't touch the new back buffer until the old front is released
 */
static void direct_flush_cb(lv_display_t *disp, const lv_area_t *area,
			     uint8_t *px_map)
{
	const struct device *dev = lv_display_get_user_data(disp);

	if (lv_display_flush_is_last(disp)) {
		uint32_t h = lv_display_get_vertical_resolution(disp);
		uint32_t w = lv_display_get_horizontal_resolution(disp);
		uint32_t stride = w * (LV_COLOR_DEPTH / 8);

		sys_cache_data_flush_range(px_map, stride * h);

		struct cdc200_fb_desc fb = {
			.fb_addr = px_map,
			.fb_size = stride * h,
		};

		cdc200_swap_fb(dev, 0, &fb);

		/* Block until the CDC200 ISR processes the swap at vblank. */
		cdc200_wait_vsync(dev, K_MSEC(50));
	}

	lv_display_flush_ready(disp);
}

/*
 * Set up DIRECT-mode rendering. Override the LVGL glue's default PARTIAL mode
 * flush with a zero-copy page-flip callback. The CDC200 must be pointed at one
 * of the VDBs initially; the first swap_fb call inside the flush callback
 * handles the transition.
 */
static void setup_direct_mode(const struct device *display_dev,
			       lv_display_t *disp)
{
	lv_display_set_render_mode(disp, LV_DISPLAY_RENDER_MODE_DIRECT);
	lv_display_set_flush_cb(disp, direct_flush_cb);
	lv_display_set_user_data(disp, (void *)display_dev);

	/*
	 * Point CDC200 at the first VDB so it has a valid scan-out address
	 * before LVGL renders its first frame.
	 */
	lv_draw_buf_t *buf0 = lv_display_get_buf_active(disp);

	if (buf0) {
		uint32_t h = lv_display_get_vertical_resolution(disp);
		uint32_t w = lv_display_get_horizontal_resolution(disp);
		struct cdc200_fb_desc fb = {
			.fb_addr = buf0->data,
			.fb_size = w * (LV_COLOR_DEPTH / 8) * h,
		};

		cdc200_swap_fb(display_dev, 0, &fb);
	}
}

int main(void)
{
	const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));

	if (!device_is_ready(display_dev)) {
		LOG_ERR("Display device not ready");
		return 0;
	}

#if defined(CONFIG_MIPI_DSI)
	{
		const struct device *dsi = DEVICE_DT_GET(DT_NODELABEL(mipi_dsi));
		int ret;

		if (!device_is_ready(dsi)) {
			LOG_ERR("MIPI-DSI host not ready");
			return 0;
		}

		/*
		 * The panel is initialised over the DSI link in command mode. Switch
		 * the host to video mode so the CDC200 framebuffer is actually
		 * streamed to the panel; without this the screen stays black even
		 * though the panel configured successfully.
		 */
		ret = dsi_dw_set_mode(dsi, DSI_DW_VIDEO_MODE);
		if (ret) {
			LOG_ERR("Failed to set DSI video mode (%d)", ret);
			return 0;
		}
	}
#endif

#if DT_NODE_HAS_STATUS(DT_ALIAS(panel), okay)
	if (display_panel_on()) {
		return 0;
	}
#endif

	/*
	 * The CDC200 controller is not started by its own driver init, and the
	 * generic display_blanking_off() API the LVGL glue calls is a no-op for
	 * it (returns -ENOTSUP). Start the scan-out engine explicitly here, or the
	 * panel stays dark even though the MIPI-DSI link and backlight are up.
	 */
	cdc200_set_enable(display_dev, true);

	/*
	 * Switch to DIRECT-mode: GPU renders into full-screen VDBs in SRAM0,
	 * flush callback page-flips via cdc200_swap_fb() at each vblank.
	 */
	lv_display_t *disp = lv_display_get_default();

	setup_direct_mode(display_dev, disp);

#if defined(CONFIG_APP_DAVE2D_HEAP_HWM) || defined(CONFIG_APP_LVGL_HEAP_STATS)
	lv_demo_benchmark_set_end_cb(benchmark_end_cb);
#endif
	lv_demo_benchmark();

	while (1) {
		uint32_t sleep_ms = lv_timer_handler();

		k_msleep(MIN(sleep_ms, INT32_MAX));
	}

	return 0;
}

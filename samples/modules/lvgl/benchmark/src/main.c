/*
 * SPDX-FileCopyrightText: Copyright Alif Semiconductor
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/display/cdc200.h>
#include <zephyr/cache.h>
#include <zephyr/linker/section_tags.h>

#include <lvgl.h>
#include <lv_demos.h>

#include <dave_d0lib.h>

#include <se_service.h>
#include <soc_common.h>
#if defined(CONFIG_MIPI_DSI)
#include <zephyr/drivers/mipi_dsi/dsi_dw.h>
#endif
#if defined(CONFIG_ENSEMBLE_GEN2) && defined(CONFIG_MIPI_DSI)
#include <zephyr/drivers/gpio.h>
#endif

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app, CONFIG_LOG_DEFAULT_LEVEL);

/*
 * Bring up the power and clocks the display pipeline needs before any peripheral
 * initialises. This must run early (PRE_KERNEL_1, priority 46: after the SE
 * services at 45, before the power-domain and peripheral drivers) because the
 * CDC200, MIPI-DSI and D/AVE 2D drivers all touch registers that are otherwise
 * power- or clock-gated.
 *
 *  - HFOSC (38.4 MHz) is the MIPI D-PHY PLL reference clock (the dphy node's
 *    ref-frequency); without it the D-PHY PLL cannot lock and the panel fails
 *    to attach to the DSI host.
 *  - The MIPI TX/RX/PLL D-PHYs and the PHY LDO must be powered.
 *  - The CDC200, MIPI-DSI and GPU2D IP clocks must be ungated.
 *  - SRAM0 (LVGL render buffers + memory pool) and SRAM1 (D/AVE 2D heap) must
 *    stay powered.
 *  - On Gen2 (E8) the MIPI lanes are routed through a camera/display mux that
 *    the application drives to the display position.
 */
static int display_pipeline_power_init(void)
{
	run_profile_t runp = {0};
	int ret;

#if defined(CONFIG_ENSEMBLE_GEN2) && defined(CONFIG_MIPI_DSI)
	const struct gpio_dt_spec cam_disp_mux_gpio =
		GPIO_DT_SPEC_GET(DT_NODELABEL(mipi_dsi), cam_disp_mux_gpios);

	gpio_pin_configure_dt(&cam_disp_mux_gpio, GPIO_OUTPUT_ACTIVE);
#endif

	/* Enable HFOSC (38.4 MHz, the D-PHY PLL reference) and the CFG clock. */
	sys_set_bits(CGU_CLK_ENA, BIT(21) | BIT(23));

	runp.power_domains = PD_SYST_MASK | PD_SSE700_AON_MASK;
	runp.dcdc_voltage = 825;
	runp.dcdc_mode = DCDC_MODE_PWM;
	runp.aon_clk_src = CLK_SRC_LFXO;
	runp.run_clk_src = CLK_SRC_PLL;
	runp.vdd_ioflex_3V3 = IOFLEX_LEVEL_1V8;
#if defined(CONFIG_RTSS_HP)
	runp.cpu_clk_freq = CLOCK_FREQUENCY_400MHZ;
#else
	runp.cpu_clk_freq = CLOCK_FREQUENCY_160MHZ;
#endif
	runp.memory_blocks = MRAM_MASK | SRAM0_MASK | SRAM1_MASK;
	runp.phy_pwr_gating = MIPI_TX_DPHY_MASK | MIPI_RX_DPHY_MASK |
			      MIPI_PLL_DPHY_MASK | LDO_PHY_MASK;
	runp.ip_clock_gating = CDC200_MASK | MIPI_DSI_MASK | GPU_MASK;

	ret = se_service_set_run_cfg(&runp);
	if (ret) {
		LOG_ERR("SE: set_run_cfg failed (%d)", ret);
	}

	return ret;
}
SYS_INIT(display_pipeline_power_init, PRE_KERNEL_1, 46);

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
#define DAVE2D_HEAP_SIZE (512 * 1024)
static uint8_t __alif_sram1_section dave2d_heap[DAVE2D_HEAP_SIZE];

static int dave2d_heap_init(void)
{
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
		uint32_t stride = w * sizeof(uint16_t); /* RGB565 */

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
			.fb_size = w * sizeof(uint16_t) * h,
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

	lv_demo_benchmark();

	while (1) {
		uint32_t sleep_ms = lv_timer_handler();

		k_msleep(MIN(sleep_ms, INT32_MAX));
	}

	return 0;
}

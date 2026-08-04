/*
 * Copyright (C) 2026 Alif Semiconductor.
 * SPDX-License-Identifier: Apache-2.0
 *
 * SoC bring-up for the UVC webcam sample: the SE-service run profile plus the
 * camera pixel clock (and, on the parallel-camera boards, the OV5640 enable
 * buffer). This is board/SoC plumbing that runs once at PRE_KERNEL_1 and is
 * not part of the UVC sample logic, so it lives here rather than in main.c.
 */

#include "pipeline.h"

#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <soc_common.h>
#include <se_service.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(uvc_platform, LOG_LEVEL_INF);

#if UVC_OUTPUT_MJPEG
/*
 * Weak hook the Alif ISP library links against to gate its own log output.
 * 0 = silent; raise for ISP-lib debugging.
 */
int log_level(void)
{
	return 0;
}
#endif

/*
 * Program the SE-service run profile and the camera pixel clock so the camera
 * pipeline and USB PHY are powered before the drivers probe. The run profile is
 * common to both pipelines; only the CPU clock, the pixel-clock register and
 * (RGB565 only) the OV5640 enable-buffer GPIO differ.
 */
static int app_set_parameters(void)
{
	run_profile_t runp = { 0 };
	int ret;

#if UVC_OUTPUT_MJPEG
	/* E8: enable HFOSC (38.4 MHz) and CFG (100 MHz) clocks. */
	sys_set_bits(CGU_CLK_ENA, BIT(23) | BIT(7));
#endif

	runp.power_domains  = PD_SYST_MASK | PD_SSE700_AON_MASK | PD_DBSS_MASK;
	runp.dcdc_voltage   = 825;
	runp.dcdc_mode      = DCDC_MODE_PWM;
	runp.aon_clk_src    = CLK_SRC_LFXO;
	runp.run_clk_src    = CLK_SRC_PLL;
	runp.vdd_ioflex_3V3 = IOFLEX_LEVEL_1V8;
#if UVC_OUTPUT_MJPEG && defined(CONFIG_RTSS_HP)
	runp.cpu_clk_freq   = CLOCK_FREQUENCY_400MHZ;
#else
	runp.cpu_clk_freq   = CLOCK_FREQUENCY_160MHZ;
#endif

	runp.memory_blocks = MRAM_MASK;
#if DT_NODE_EXISTS(DT_NODELABEL(sram0))
	runp.memory_blocks |= SRAM0_MASK;
#endif

	runp.phy_pwr_gating  = MIPI_TX_DPHY_MASK | MIPI_RX_DPHY_MASK |
			       MIPI_PLL_DPHY_MASK | LDO_PHY_MASK | USB_PHY_MASK;
	runp.ip_clock_gating = CAMERA_MASK | MIPI_CSI_MASK | MIPI_DSI_MASK |
			       USB_MASK;

	ret = se_service_set_run_cfg(&runp);
	if (ret) {
		__ASSERT(false, "SE: set_run_cfg failed = %d", ret);
		return ret;
	}

#if UVC_OUTPUT_MJPEG
	/* CPI/CSI pixel clock (XVCLK) used by the CSI-2 sensor. */
	sys_write32(0x140001, CLKCTRL_PER_MST_CAMERA_PIXCLK_CTRL);
#else
	/* LP-CAM pixel clock and OV5640 enable-buffer GPIO. */
	sys_write32(0x080001, M55HE_CFG_HE_CAMERA_PIXCLK);

	const struct gpio_dt_spec cam_enbuf =
		GPIO_DT_SPEC_GET(DT_NODELABEL(cam_enbuf), enable_gpios);

	gpio_pin_configure_dt(&cam_enbuf, GPIO_OUTPUT_ACTIVE);
#endif

	return 0;
}

SYS_INIT(app_set_parameters, PRE_KERNEL_1, 46);

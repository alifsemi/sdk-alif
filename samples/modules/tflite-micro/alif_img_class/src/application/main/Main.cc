/* This file was ported to work on Alif Semiconductor devices. */

/*
 * SPDX-FileCopyrightText: Copyright Alif Semiconductor
 */

/*
 * SPDX-FileCopyrightText: Copyright 2021-2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include "TensorFlowLiteMicro.hpp" /* our inference logic api */

#include <cstdio>
#include <new>
#include <exception>

#include <zephyr/console/console.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>
#include <soc_common.h>
#include <se_service.h>

#define DISPLAY_NODE          DT_CHOSEN(zephyr_display)

#if IS_ENABLED(CONFIG_CDC200)
#define IS_ENABLED_CDC200 DT_NODE_HAS_COMPAT(DISPLAY_NODE, tes_cdc_2_1)
#else
#define IS_ENABLED_CDC200 0
#endif

#if IS_ENABLED(CONFIG_MIPI_DSI)
#define IS_ENABLED_MIPI_DSI DT_HAS_ALIAS(mipi_dsi)
#else
#define IS_ENABLED_MIPI_DSI 0
#endif

#if IS_ENABLED(CONFIG_MIPI_DSI)
#include <zephyr/drivers/mipi_dsi/dsi_dw.h>
#endif
#if IS_ENABLED(CONFIG_CDC200)
#include <zephyr/drivers/display/cdc200.h>
#endif

LOG_MODULE_REGISTER(Main);

extern void main_loop();

#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
__ASM(" .global __ARM_use_no_argv\n");
#endif

/* Print application information. */
static void print_application_intro()
{
	LOG_INF("Build date: " __DATE__ " @ " __TIME__ "");
	LOG_INF("Copyright 2021-2024 Arm Limited and/or its affiliates "
	     "<open-source-office@arm.com>");
}

static void out_of_heap()
{
	LOG_WRN("Out of heap");
	std::terminate();
}

static int app_set_dsi_cdc()
{
#if IS_ENABLED_MIPI_DSI
	const struct device *dsi = DEVICE_DT_GET(DT_ALIAS(mipi_dsi));

	if (!device_is_ready(dsi)) {
		LOG_ERR("DSI device not ready.");
		return -ENODEV;
	}

	int err = dsi_dw_set_mode(dsi, DSI_DW_VIDEO_MODE);
	if (err) {
		LOG_ERR("Could not set DSI Host controller to video mode: %d.", err);
		return err;
	}
#endif

#if IS_ENABLED_CDC200
	const struct device *display_dev = DEVICE_DT_GET(DISPLAY_NODE);
	cdc200_set_enable(display_dev, true);
	return 0;
#endif

	return -1;
}

int main()
{
	print_application_intro();

	std::set_new_handler(out_of_heap);

	LOG_INF("Starting application...");

	// /* Run the application. */
	main_loop();

	/* This is unreachable without errors. */
	LOG_INF("program terminating...");

	return 0;
}

/*
 * Do application configurations.
 */
static int app_set_parameters(void)
{
#if (DT_NODE_HAS_STATUS(DT_NODELABEL(cam), okay))
	int ret;
	run_profile_t runp = (run_profile_t){0};

#if (DT_NODE_HAS_STATUS(DT_NODELABEL(camera_select), okay))
	const struct gpio_dt_spec sel =
		GPIO_DT_SPEC_GET(DT_NODELABEL(camera_select), select_gpios);

	gpio_pin_configure_dt(&sel, GPIO_OUTPUT);
	gpio_pin_set_dt(&sel, 1);
#endif /* (DT_NODE_HAS_STATUS(DT_NODELABEL(camera_sensor), okay)) */

#if (defined(CONFIG_ENSEMBLE_GEN2) && defined(CONFIG_MIPI_DSI))
	const struct gpio_dt_spec cam_disp_mux_gpio =
		GPIO_DT_SPEC_GET(DT_NODELABEL(mipi_dsi), cam_disp_mux_gpios);
	gpio_pin_configure_dt(&cam_disp_mux_gpio, GPIO_OUTPUT_ACTIVE);
#endif

	runp.power_domains = PD_SYST_MASK | PD_SSE700_AON_MASK | PD_DBSS_MASK;
	runp.dcdc_voltage  = 825;
	runp.dcdc_mode     = DCDC_MODE_PWM;
	runp.aon_clk_src   = CLK_SRC_LFXO;
	runp.run_clk_src   = CLK_SRC_PLL;
	runp.vdd_ioflex_3V3 = IOFLEX_LEVEL_1V8;
#if defined(CONFIG_RTSS_HP)
	runp.cpu_clk_freq  = CLOCK_FREQUENCY_400MHZ;
#else
	runp.cpu_clk_freq  = CLOCK_FREQUENCY_160MHZ;
#endif

	runp.memory_blocks = MRAM_MASK;
#if DT_NODE_EXISTS(DT_NODELABEL(sram0))
	runp.memory_blocks |= SRAM0_MASK;
#endif
#if DT_NODE_EXISTS(DT_NODELABEL(sram1))
	runp.memory_blocks |= SRAM1_MASK;
#endif

	runp.phy_pwr_gating |= MIPI_TX_DPHY_MASK | MIPI_RX_DPHY_MASK |
		MIPI_PLL_DPHY_MASK | LDO_PHY_MASK;
	runp.ip_clock_gating = CDC200_MASK | CAMERA_MASK | MIPI_CSI_MASK | MIPI_DSI_MASK;

	ret = se_service_set_run_cfg(&runp);
	__ASSERT(ret == 0, "SE: set_run_cfg failed = %d", ret);

	/*
	 * CPI Pixel clock - Generate XVCLK. Used by ARX3A0 & OV5675 sensors.
	 */
	sys_write32(0x140001, CLKCTRL_PER_MST_CAMERA_PIXCLK_CTRL);
#endif

	return 0;
}

SYS_INIT(app_set_parameters, PRE_KERNEL_1, 46);
SYS_INIT(app_set_dsi_cdc, APPLICATION, APP_SET_DSI_CDC_PRIORITY);

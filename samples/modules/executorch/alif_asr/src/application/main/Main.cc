/* This file was ported to work on Alif Semiconductor devices. */

/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/*
 * SPDX-FileCopyrightText: Copyright 2021-2026 Arm Limited and/or its
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

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <new>
#include <exception>

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

extern void MainLoop();

#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
__ASM(" .global __ARM_use_no_argv\n");
#endif

/* Print application information. */
static void PrintApplicationIntro()
{
    LOG_INF("%s\n", PRJ_DES_STR);
    LOG_INF("Version %s Build date: " __DATE__ " @ " __TIME__ "\n", PRJ_VER_STR);
    LOG_INF("Compiler: %s\n", PRJ_COMPILER);
    LOG_INF("Copyright 2021-2026 Arm Limited and/or "
         "its affiliates <open-source-office@arm.com>\n\n");
}

static void out_of_heap()
{
    LOG_WRN("Out of heap\n");
    std::terminate();
}

#ifdef CONFIG_ENABLE_DISPLAY
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

	/* The LVGL VDBs double as the CDC200 scan-out framebuffers (DIRECT mode) and
	 * live in a NOLOAD SRAM section, so they hold random data at boot. The
	 * display is enabled here, long before LVGL's first flush, so clear them to
	 * black first to avoid scanning out garbage until the UI is drawn. */
	extern uint8_t __lvgl_buf_start[];
	extern uint8_t __lvgl_buf_end[];
	memset(__lvgl_buf_start, 0, (size_t)(__lvgl_buf_end - __lvgl_buf_start));

	cdc200_set_enable(display_dev, true);
	return 0;
#endif

	return -1;
}
#endif /* CONFIG_ENABLE_DISPLAY */

int main ()
{
	/* Application information, UART should have been initialised. */
	PrintApplicationIntro();

	std::set_new_handler(out_of_heap);

	/* Run the application. */
	MainLoop();

    /* This is unreachable without errors. */
    LOG_INF("program terminating...\n");

    return 0;
}

#ifdef CONFIG_ENABLE_DISPLAY
SYS_INIT(app_set_dsi_cdc, APPLICATION, APP_SET_DSI_CDC_PRIORITY);
#endif /* CONFIG_ENABLE_DISPLAY */

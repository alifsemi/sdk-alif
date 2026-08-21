/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <string.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/display/cdc200.h>
#include <soc_common.h>
#include <se_service.h>
#include <zephyr/pm/device.h>
#include <zephyr/pm/policy.h>
#include <zephyr/logging/log.h>
#ifdef CONFIG_MIPI_DSI
#include <zephyr/drivers/mipi_dsi/dsi_dw.h>
#endif /* CONFIG_MIPI_DSI */
#include <zephyr/cache.h>
#include "alif_logo.h"

#include "display_pm_test.h"

LOG_MODULE_DECLARE(disp_pm, LOG_LEVEL_DBG);

/* size of stack area used by each thread */
#define STACKSIZE 4096

/* Thread Priority */
#define DISPLAY_PRIORITY 7

#define RED_ARGB8888	0x00ff0000
#define GREEN_ARGB8888	0x0000ff00
#define BLUE_ARGB8888	0x000000ff
#define RED_RGB888	    0x00ff0000
#define GREEN_RGB888	0x0000ff00
#define BLUE_RGB888	    0x000000ff
#define RED_RGB565	    0xf800
#define GREEN_RGB565	0x07e0
#define BLUE_RGB565	    0x001f

#define CDC200_PIXEL_SIZE_ARGB8888	4
#define CDC200_PIXEL_SIZE_RGB888	3
#define CDC200_PIXEL_SIZE_RGB565	2

K_THREAD_STACK_DEFINE(DisplayT_stack, STACKSIZE);
static struct k_thread thread_display;

/* Semaphore for PM suspend/resume synchronization */
K_SEM_DEFINE(display_pm_resume_sem, 0, 1);

/* Semaphore to signal that display thread has suspended */
K_SEM_DEFINE(display_pm_suspended_sem, 0, 1);

/* Semaphore to signal that a streaming has completed */
K_SEM_DEFINE(display_streaming_done_sem, 0, 1);

/* Thread control flags */
static volatile uint8_t THREAD_TO_BE_SUSPEND;
static volatile uint8_t THREAD_SUSPENDED;
static volatile uint8_t THREAD_TO_BE_STOPPED;

/* display device */
static const struct device *display_dev;

#if defined(CONFIG_MIPI_DSI)
/* Panel device for PM actions */
static const struct device *panel_dev;

/* DSI device for PM actions */
static const struct device *dsi_dev;

static const struct device *dphy_dev;
#endif /* defined(CONFIG_MIPI_DSI) */

/* Tracks whether streaming was active when the PM suspend was requested,
 * so we only auto-restart stream on resume if it was streaming before.
 */
static bool was_streaming;

static bool streaming_active;

enum corner {
	TOP_LEFT,
	TOP_RIGHT,
	BOTTOM_RIGHT,
	BOTTOM_LEFT
};

typedef void (*fill_buffer)(enum corner corner, uint8_t grey, uint8_t *buf,
			    size_t buf_size);

struct display_capabilities panel_caps;

struct display_buffer_descriptor buf_desc;

struct cdc200_display_caps capabilities;

struct cdc200_fb_desc fb_l2 = { 0 };

fill_buffer fill_buffer_fnc;

size_t pixel_size;
size_t buf_size;
size_t rect_w = 2;
size_t rect_h = 1;
uint8_t *buf;
size_t scale;
size_t x;
size_t y;

#if (!defined(CONFIG_MIPI_DSI) || \
	!DT_NODE_HAS_PROP(DT_ALIAS(mipi_dsi), dpi_video_pattern_gen))
static void fill_buffer_argb8888(enum corner corner, uint8_t grey, uint8_t *buf,
				 size_t buf_size)
{
	uint32_t color = 0;

	switch (corner) {
	case TOP_LEFT:
		color = RED_ARGB8888;
		break;
	case TOP_RIGHT:
		color = GREEN_ARGB8888;
		break;
	case BOTTOM_RIGHT:
		color = BLUE_ARGB8888;
		break;
	case BOTTOM_LEFT:
		color = grey << 16 | grey << 8 | grey;
		break;
	}

	for (size_t idx = 0; idx < buf_size; idx += 4) {
		*((uint32_t *)(buf + idx)) = color;
	}
}

static void fill_buffer_rgb888(enum corner corner, uint8_t grey, uint8_t *buf,
			       size_t buf_size)
{
	uint32_t color = 0;

	switch (corner) {
	case TOP_LEFT:
		color = RED_RGB888;
		break;
	case TOP_RIGHT:
		color = GREEN_RGB888;
		break;
	case BOTTOM_RIGHT:
		color = BLUE_RGB888;
		break;
	case BOTTOM_LEFT:
		color = grey << 16 | grey << 8 | grey;
		break;
	}

	for (size_t idx = 0; idx < buf_size; idx += 3) {
		*(buf + idx + 2) = (color >> 16) & 0xff;
		*(buf + idx + 1) = (color >> 8) & 0xff;
		*(buf + idx + 0) = (color >> 0) & 0xff;
	}
}

static uint16_t get_rgb565_color(enum corner corner, uint8_t grey)
{
	uint16_t color = 0;

	switch (corner) {
	case TOP_LEFT:
		color = RED_RGB565;
		break;
	case TOP_RIGHT:
		color = GREEN_RGB565;
		break;
	case BOTTOM_RIGHT:
		color = BLUE_RGB565;
		break;
	case BOTTOM_LEFT:
		color = (grey & 0x1f) << 11 |
			(grey & 0x3f) << 5 | (grey & 0x1F);
		break;
	}
	return color;
}

static void fill_buffer_rgb565(enum corner corner, uint8_t grey, uint8_t *buf,
			       size_t buf_size)
{
	uint16_t color = get_rgb565_color(corner, grey);

	for (size_t idx = 0; idx < buf_size; idx += 2) {
		*(buf + idx + 1) = (color >> 8) & 0xFFu;
		*(buf + idx + 0) = (color >> 0) & 0xFFu;
	}
}

int get_pixel_size(enum display_pixel_format fmt)
{
	if (fmt == PIXEL_FORMAT_RGB_888)
		return CDC200_PIXEL_SIZE_RGB888;
	else if (fmt == PIXEL_FORMAT_ARGB_8888)
		return CDC200_PIXEL_SIZE_ARGB8888;
	else if (fmt == PIXEL_FORMAT_RGB_565)
		return CDC200_PIXEL_SIZE_RGB565;
	else
		return 0;
}
#endif /* (!defined(CONFIG_MIPI_DSI) || \
	* !DT_NODE_HAS_PROP(DT_ALIAS(mipi_dsi), dpi_video_pattern_gen))
	*/

/*
 * Initialize display (called once at startup)
 */
static int display_init(void)
{
#if defined(CONFIG_MIPI_DSI)
	int ret;

	panel_dev = DEVICE_DT_GET(DT_ALIAS(panel));
	if (!device_is_ready(panel_dev)) {
		LOG_WRN("Panel device not ready for PM actions");
		panel_dev = NULL;
	}

	dsi_dev = DEVICE_DT_GET(DT_ALIAS(mipi_dsi));
	if (!device_is_ready(dsi_dev)) {
		LOG_WRN("DSI device not ready for PM actions");
		dsi_dev = NULL;
	}

	dphy_dev = DEVICE_DT_GET_ONE(snps_designware_dphy);
	if (!device_is_ready(dphy_dev)) {
		LOG_WRN("dphy device not ready for PM actions");
		dphy_dev = NULL;
	}

	ret = display_set_orientation(panel_dev, DISPLAY_ORIENTATION_ROTATED_180);
	if (ret == -ENOTSUP)
		LOG_DBG("Un-supported Display Rotation.");

	ret = dsi_dw_set_mode(dsi_dev, DSI_DW_VIDEO_MODE);
	if (ret) {
		LOG_ERR("DSI Host controller set to video mode.");
		return -1;
	}

	display_get_capabilities(panel_dev, &panel_caps);
	display_blanking_off(panel_dev);
#endif /* defined(CONFIG_MIPI_DSI) */

	display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
	if (!device_is_ready(display_dev)) {
		LOG_WRN("CDC200 device not ready for PM actions");
		display_dev = NULL;
	}

	LOG_INF("Enabling CDC200 Device.");
	cdc200_set_enable(display_dev, true);
	cdc200_get_capabilities(display_dev, &capabilities);

	LOG_INF("Display init: %s, panel res (%d, %d), fmt %d", display_dev->name,
		capabilities.x_panel_resolution, capabilities.y_panel_resolution,
		capabilities.supported_pixel_formats);

	for (int i = 0; i <= 1; i++) {
		LOG_DBG("Layer %d: en=%d res=(%d,%d) fmt=%d", i + 1,
			capabilities.layer[i].layer_en,
			capabilities.layer[i].x_resolution,
			capabilities.layer[i].y_resolution,
			capabilities.layer[i].current_pixel_format);
	}

	scale = (capabilities.layer[0].x_resolution / 8);
	rect_w *= scale;
	rect_h *= scale;
	buf_size = rect_w * rect_h;

	if (buf_size < (capabilities.layer[0].x_resolution)) {
		buf_size = capabilities.layer[0].x_resolution;
	}

	pixel_size =
		MAX(get_pixel_size(capabilities.layer[0].current_pixel_format),
		get_pixel_size(capabilities.layer[1].current_pixel_format));
	buf_size *= pixel_size;

	switch (capabilities.layer[0].current_pixel_format) {
	case PIXEL_FORMAT_ARGB_8888:
		fill_buffer_fnc = fill_buffer_argb8888;
		break;
	case PIXEL_FORMAT_RGB_888:
		fill_buffer_fnc = fill_buffer_rgb888;
		break;
	case PIXEL_FORMAT_RGB_565:
		fill_buffer_fnc = fill_buffer_rgb565;
		break;
	default:
		LOG_ERR("Unsupported pixel format. Aborting sample.");
		return -1;
	}

	buf = k_malloc(buf_size);
	if (buf == NULL) {
		LOG_ERR("Could not allocate memory."
			"Aborting sample. Required Heap Size - %d", buf_size);
		return -1;
	}

	return 0;
}

/*
 * Display streaming thread function
 */
static void display_streaming_thread(void *p1, void *p2, void *p3)
{
	int ret;
	bool pm_locks_held = false;

	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	if (!was_streaming) {
		LOG_INF("skip starting a new stream cycle this round");
	}

	/* Initialize camera */
	ret = display_init();
	if (ret) {
		LOG_ERR("Display initialization failed: %d", ret);
		return;
	}

	while (1) {
		/* Check if thread should be stopped (PM sequence done) */
		if (THREAD_TO_BE_STOPPED) {
			LOG_INF("Display: thread stopping (PM sequence completed)");
			return;
		}

		if (!streaming_active) {
			/*
			 * Check if thread should be suspended (PM cycle).
			 * Only suspend when streaming is not active.
			 */
			if (THREAD_TO_BE_SUSPEND) {
				THREAD_SUSPENDED = 1;
				LOG_INF("Display: suspending for PM cycle");

#if defined(CONFIG_MIPI_DSI)
				if (panel_dev) {
					ret = pm_device_action_run(panel_dev,
							PM_DEVICE_ACTION_SUSPEND);
					if (ret && ret != -EALREADY && ret != -ENOSYS) {
						LOG_ERR("Panel PM suspend failed: %d", ret);
					}
				}

				if (dsi_dev) {
					ret = pm_device_action_run(dsi_dev,
						PM_DEVICE_ACTION_SUSPEND);
					if (ret && ret != -EALREADY && ret != -ENOSYS) {
						LOG_ERR("DSI PM suspend failed: %d", ret);
					}
				}

				if (dphy_dev) {
					ret = pm_device_action_run(dphy_dev,
						PM_DEVICE_ACTION_SUSPEND);
					if (ret && ret != -EALREADY && ret != -ENOSYS) {
						LOG_ERR("DPHY PM suspend failed: %d", ret);
					}
				}
#endif /* defined(CONFIG_MIPI_DSI) */

				if (display_dev) {
					ret = pm_device_action_run(display_dev,
						PM_DEVICE_ACTION_SUSPEND);
					if (ret && ret != -EALREADY && ret != -ENOSYS) {
						LOG_ERR("CDC200 PM suspend failed: %d", ret);
					}
				}

				/* Signal that we are now suspended */
				k_sem_give(&display_pm_suspended_sem);

				/* Block on semaphore - does not wake system like k_msleep */
				k_sem_take(&display_pm_resume_sem, K_FOREVER);

				LOG_INF("Display: resuming after PM wake");
				THREAD_SUSPENDED = 0;

				/*
				 * Lock deeper PM states during display resume
				 * sequence to prevent S2RAM from trashing
				 * hardware state mid-reinit.
				 */
				pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_RAM,
							PM_ALL_SUBSTATES);
				pm_policy_state_lock_get(PM_STATE_SOFT_OFF,
							PM_ALL_SUBSTATES);
				pm_locks_held = true;

				if (display_dev) {
					ret = pm_device_action_run(display_dev,
						PM_DEVICE_ACTION_RESUME);
					if (ret && ret != -EALREADY && ret != -ENOSYS) {
						LOG_ERR("CDC200 PM resume failed: %d", ret);
					}
				}

#if defined(CONFIG_MIPI_DSI)
				if (dphy_dev) {
					ret = pm_device_action_run(dphy_dev,
						PM_DEVICE_ACTION_RESUME);
					if (ret && ret != -EALREADY && ret != -ENOSYS) {
						LOG_ERR("DPHY PM resume failed: %d", ret);
					}
				}

				if (dsi_dev) {
					ret = pm_device_action_run(dsi_dev,
						PM_DEVICE_ACTION_RESUME);
					if (ret && ret != -EALREADY && ret != -ENOSYS) {
						LOG_ERR("DSI PM resume failed: %d", ret);
					}
				}

				if (panel_dev) {
					ret = pm_device_action_run(panel_dev,
							PM_DEVICE_ACTION_RESUME);
					if (ret && ret != -EALREADY && ret != -ENOSYS) {
						LOG_ERR("Panel PM resume failed: %d", ret);
					}
				}
#endif /* defined(CONFIG_MIPI_DSI) */

				/* After STOP, DBSS was fully powered off. Give DPHY analog
				 * (PLL, LDO, lane receivers) time to stabilize after power
				 * domain re-enable before attempting MIPI link setup.
				 */
				k_msleep(50);

#if defined(CONFIG_MIPI_DSI)
				display_set_orientation(panel_dev, DISPLAY_ORIENTATION_ROTATED_180);
				dsi_dw_set_mode(dsi_dev, DSI_DW_VIDEO_MODE);
				display_blanking_off(panel_dev);
#endif /* defined(CONFIG_MIPI_DSI) */

				cdc200_set_enable(display_dev, true);
				LOG_DBG("Display: reinit complete, resuming streaming");
			}

			streaming_active = true;
		}

		if (capabilities.layer[1].layer_en) {
			cdc200_get_framebuffer(display_dev, 1, &fb_l2);
			memset((uint8_t *) fb_l2.fb_addr, 0, fb_l2.fb_size);
			memcpy((uint8_t *) fb_l2.fb_addr, logo, sizeof(logo));
			sys_cache_data_flush_range((void *)fb_l2.fb_addr, sizeof(logo));
		}

		if (capabilities.layer[0].layer_en) {
			cdc200_get_framebuffer(display_dev, 0, &fb_l2);
			LOG_INF("FB0 - 0x%08x, size - %d",
				(uint32_t)fb_l2.fb_addr, fb_l2.fb_size);
			(void)memset(buf, 0xFFu, buf_size);

			buf_desc.buf_size = buf_size;
			buf_desc.pitch = capabilities.layer[0].x_resolution;
			buf_desc.width = capabilities.layer[0].x_resolution;
			buf_desc.height = 1;

			int row_errs = 0;

			for (int idx = 0; idx < capabilities.layer[0].y_resolution; idx += 1) {
				ret = cdc200_display_write(display_dev, 0, 0, idx, &buf_desc, buf);
				if (ret) {
					row_errs++;
				}
			}
			if (row_errs) {
				LOG_ERR("cdc200_display_write failed on %d row(s)", row_errs);
			}

			buf_desc.pitch = rect_w;
			buf_desc.width = rect_w;
			buf_desc.height = rect_h;

			fill_buffer_fnc(TOP_LEFT, 0, buf, buf_size);
			x = 0;
			y = 0;

			ret = cdc200_display_write(display_dev, 0, x, y, &buf_desc, buf);
			if (ret) {
				LOG_ERR("cdc200_display_write failed (TOP_LEFT)");
			}

			fill_buffer_fnc(TOP_RIGHT, 0, buf, buf_size);
			x = capabilities.layer[0].x_resolution - rect_w;
			y = 0;

			ret = cdc200_display_write(display_dev, 0, x, y, &buf_desc, buf);
			if (ret) {
				LOG_ERR("cdc200_display_write failed (TOP_RIGHT)");
			}

			fill_buffer_fnc(BOTTOM_RIGHT, 0, buf, buf_size);
			x = capabilities.layer[0].x_resolution - rect_w;
			y = capabilities.layer[0].y_resolution - rect_h;

			ret = cdc200_display_write(display_dev, 0, x, y, &buf_desc, buf);
			if (ret) {
				LOG_ERR("cdc200_display_write failed (BOTTOM_RIGHT)");
			}

			display_blanking_off(display_dev);
			k_msleep(10000);
		}

		/* Release PM state locks after streaming is fully done */
		if (pm_locks_held) {
			pm_policy_state_lock_put(PM_STATE_SOFT_OFF, PM_ALL_SUBSTATES);
			pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_RAM, PM_ALL_SUBSTATES);
			pm_locks_held = false;
		}

		/* Wait for hardware to fully reset */
		k_msleep(100);

		streaming_active = false;
		LOG_INF("Display: streaming cycle complete, waiting for next PM cycle");

		/* Signal main thread that streaming cycle is done */
		k_sem_give(&display_streaming_done_sem);

		/* Wait for next PM suspend/resume cycle instead of free-running.
		 * Without this, the thread starts unsolicited streaming without
		 * PM lock protection, which can corrupt hardware state if the
		 * system enters deep sleep mid-streaming.
		 */
		THREAD_TO_BE_SUSPEND = 1;
	}
}

int display_pm_thread_init(void)
{
	k_tid_t tid = k_thread_create(&thread_display, DisplayT_stack, STACKSIZE,
			&display_streaming_thread, NULL, NULL, NULL,
			DISPLAY_PRIORITY, 0, K_FOREVER);
	if (tid == NULL) {
		LOG_ERR("Error creating Display Thread");
		return -1;
	}

	return 0;
}

int display_pm_thread_start(void)
{
	THREAD_TO_BE_SUSPEND = 0;
	THREAD_SUSPENDED = 0;

	k_thread_start(&thread_display);
	return 0;
}

int display_pm_thread_suspend(void)
{
	was_streaming = streaming_active;
	THREAD_TO_BE_SUSPEND = 1;

	/* Wait for thread to acknowledge suspend request */
	k_sem_take(&display_pm_suspended_sem, K_FOREVER);

	LOG_INF("Display thread is now suspended (polling for resume)");

	return 0;
}

int display_pm_thread_resume(void)
{
	LOG_DBG("Display: Try to Resume...");

	/* Reset stale signal from interrupted capture cycles */
	k_sem_reset(&display_streaming_done_sem);

	/* Clear the suspend flag and signal semaphore to wake thread */
	THREAD_TO_BE_SUSPEND = 0;
	k_sem_give(&display_pm_resume_sem);

	LOG_INF("Display: resume signal sent");
	return 0;
}

int display_pm_wait_streaming_done(void)
{
	return k_sem_take(&display_streaming_done_sem, K_FOREVER);
}

/**
 * Set the early init configuration for this application.
 */
static int app_board_early_init(void)
{
#if (defined(CONFIG_ENSEMBLE_GEN2) && defined(CONFIG_MIPI_DSI))
	const struct gpio_dt_spec cam_disp_mux_gpio =
		GPIO_DT_SPEC_GET(DT_NODELABEL(mipi_dsi), cam_disp_mux_gpios);
	gpio_pin_configure_dt(&cam_disp_mux_gpio, GPIO_OUTPUT_ACTIVE);
#endif

	return 0;
}
/*
 * CRITICAL: Must run at PRE_KERNEL_1 to restore SYSTOP before peripherals initialize.
 *
 * Priority 46 ensures this runs:
 *   - AFTER SE Services (priority 45) - SE must be ready for set_run_cfg()
 *   - BEFORE Power Domain (priority 47) - Power domain needs SYSTOP enabled
 *   - BEFORE UART and peripherals (priority 50+) - Peripherals need SYSTOP ON
 *
 * On cold boot: SYSTOP is already ON by default, safe to call.
 * On SOFT_OFF wakeup: SYSTOP is OFF, must restore BEFORE peripherals access registers.
 */
SYS_INIT(app_board_early_init, PRE_KERNEL_1, 46);

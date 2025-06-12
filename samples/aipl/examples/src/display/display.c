#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/display/cdc200.h>
#include <zephyr/drivers/mipi_dsi/dsi_dw.h>
#include <zephyr/logging/log.h>
#include "display.h"

LOG_MODULE_REGISTER(display_app, LOG_LEVEL_DBG);

#define DISPLAY_NODE DT_CHOSEN(zephyr_display)

#define DISPLAY_WIDTH  DT_PROP(DISPLAY_NODE, width)
#define DISPLAY_HEIGHT DT_PROP(DISPLAY_NODE, height)

#define BUFFER_SIZE (DISPLAY_WIDTH * DISPLAY_HEIGHT * 3)

uint8_t __attribute__((section(".alif_sram0"))) buf0[BUFFER_SIZE];
uint8_t __attribute__((section(".alif_sram0"))) buf1[BUFFER_SIZE];

enum {
	BUFFER_1 = 0,
	BUFFER_2 = 1,
	NUM_BUFFERS
};

static uint8_t *buffers[NUM_BUFFERS] = {buf0, buf1};

static uint8_t current_buffer = BUFFER_1;

static const struct device *display_dev = DEVICE_DT_GET(DISPLAY_NODE);

/* Main Display Initialization Function */
int display_init(void)
{
	if (!device_is_ready(display_dev)) {
		LOG_ERR("Device %s not found. Aborting sample.", display_dev->name);
		return -ENODEV;
	}

	const struct device *panel = DEVICE_DT_GET(DT_ALIAS(panel));
	const struct device *dsi = DEVICE_DT_GET(DT_ALIAS(mipi_dsi));
	int ret;

	if (!device_is_ready(panel)) {
		LOG_ERR("Device %s not found. Aborting sample.", panel->name);
		return -ENODEV;
	}

	if (!device_is_ready(dsi)) {
		LOG_ERR("Device %s not found. Aborting sample.", dsi->name);
		return -ENODEV;
	}

	LOG_INF("Enabling Ensemble-DSI Device video mode.");
	ret = dsi_dw_set_mode(dsi, DSI_DW_VIDEO_MODE);
	if (ret) {
		LOG_ERR("Failed to set DSI Host controller to video mode.");
		return ret;
	}

	cdc200_set_enable(display_dev, true);

	return 0;
}

void display_next_frame(void)
{
	current_buffer = (current_buffer + 1) % NUM_BUFFERS;

	struct display_buffer_descriptor desc = {.buf_size = BUFFER_SIZE,
						 .width = DISPLAY_WIDTH,
						 .height = DISPLAY_HEIGHT,
						 .pitch = DISPLAY_WIDTH};

	display_write(display_dev, 0, 0, &desc, buffers[current_buffer]);
}

void *display_active_buffer(void)
{
	return buffers[current_buffer];
}

void *display_inactive_buffer(void)
{
	return buffers[(current_buffer + 1) % NUM_BUFFERS];
}

uint32_t display_width(void)
{
	return DISPLAY_WIDTH;
}

uint32_t display_height(void)
{
	return DISPLAY_HEIGHT;
}

/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https: //alifsemi.com/license
 *
 */

#include "uvc_display.h"

#include <sample_usbd.h>
#include <aipl_color_conversion.h>

#include <errno.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/cache.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/video.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/usb/class/usbd_uvc.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(uvc_display, LOG_LEVEL_INF);

#define UVC_LCD_ENABLED \
	(DT_HAS_COMPAT_STATUS_OKAY(alif_uvc_display) && IS_ENABLED(CONFIG_DISPLAY))

#if defined(CONFIG_LVGL) && !UVC_LCD_ENABLED
#error "LVGL on the UVC display needs zephyr,display set to an alif,uvc-display node"
#endif

#define YUY2_BYTES 2
#define BOX_THICKNESS 2

/* Black in YUY2 (Y0 U Y1 V = 16 128 16 128), as a little-endian word. */
#define YUY2_BLACK_PAIR 0x80108010U

/*
 * How long a display write waits for the previous frame to finish streaming.
 * On timeout the frame is updated anyway: the host may see a torn frame, but
 * LVGL never stalls on a host that stopped reading.
 */
#define FRAME_IDLE_TIMEOUT_MS 100

static const struct device *const uvc_dev = DEVICE_DT_GET(DT_NODELABEL(uvc));

/*
 * The UVC class builds its USB descriptors from the caps of the video device
 * it is bound to. Binding it to the camera would advertise every sensor mode,
 * while the application only produces frames of one size, so bind it to this
 * format-only device that advertises exactly that size.
 */
static struct video_format_cap src_caps[2];
static struct video_format src_fmt;

static int src_set_format(const struct device *dev, enum video_endpoint_id ep,
			  struct video_format *fmt)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(ep);

	if (fmt->pixelformat != src_fmt.pixelformat || fmt->width != src_fmt.width ||
	    fmt->height != src_fmt.height) {
		return -ENOTSUP;
	}

	return 0;
}

static int src_get_format(const struct device *dev, enum video_endpoint_id ep,
			  struct video_format *fmt)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(ep);

	*fmt = src_fmt;

	return 0;
}

static int src_set_stream(const struct device *dev, bool enable)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(enable);

	return 0;
}

static int src_get_caps(const struct device *dev, enum video_endpoint_id ep,
			struct video_caps *caps)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(ep);

	caps->format_caps = src_caps;
	caps->min_vbuf_count = 1;

	return 0;
}

static DEVICE_API(video, src_api) = {
	.set_format = src_set_format,
	.get_format = src_get_format,
	.set_stream = src_set_stream,
	.get_caps = src_get_caps,
};

DEVICE_DEFINE(uvc_display_src, "uvc_display_src", NULL, NULL, NULL, NULL,
	      POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEVICE, &src_api);

/*
 * The single YUY2 frame streamed to the host. It persists between sends, so a
 * display client (LVGL) only needs to redraw the areas that changed.
 *
 * The UVC class sends it zero-copy, so it must be USB DMA-able. Use SRAM0
 * where available (as the in-tree UVC sample does for its video buffers);
 * devices without SRAM0 keep it in NON_SECURE0. It is sized for the
 * alif,uvc-display node when there is one, which also covers the smaller
 * direct (non-LVGL) frame; otherwise for the 192x192 model input.
 */
#if DT_HAS_COMPAT_STATUS_OKAY(alif_uvc_display)
#define FRAME_BUF_SIZE                                                                     \
	(DT_PROP(DT_INST(0, alif_uvc_display), width) *                                    \
	 DT_PROP(DT_INST(0, alif_uvc_display), height) * YUY2_BYTES)
#else
#define FRAME_BUF_SIZE (192 * 192 * YUY2_BYTES)
#endif

#if DT_NODE_HAS_STATUS(DT_NODELABEL(sram0), okay)
#define UVC_FRAME_SECTION __section(".alif_sram0.uvc_frame")
#else
#define UVC_FRAME_SECTION __section(".alif_ns.uvc_frame")
#endif

static uint8_t frame_buf[FRAME_BUF_SIZE] __aligned(CONFIG_VIDEO_BUFFER_POOL_ALIGN)
	UVC_FRAME_SECTION;
static struct video_buffer frame_vbuf;
static struct video_buffer *frame;
static size_t frame_bytes;
static bool frame_pending;
static bool frame_in_flight;
static bool usb_started;

/*
 * Cleared while the cable is unplugged or the bus is suspended. The UVC class
 * only learns that the stream ended at the next bus reset, so check this too.
 */
static atomic_t usb_bus_active = ATOMIC_INIT(1);

static void usbd_msg_cb(struct usbd_context *const ctx, const struct usbd_msg *const msg)
{
	ARG_UNUSED(ctx);

	switch (msg->type) {
	case USBD_MSG_VBUS_REMOVED:
	case USBD_MSG_SUSPEND:
		atomic_set(&usb_bus_active, 0);
		break;
	case USBD_MSG_VBUS_READY:
	case USBD_MSG_RESUME:
		atomic_set(&usb_bus_active, 1);
		break;
	default:
		break;
	}
}

/* True while the host has committed a stream it reads frames from. */
static bool stream_open(void)
{
	struct video_format fmt;

	return usb_started && atomic_get(&usb_bus_active) != 0 &&
	       video_get_format(uvc_dev, VIDEO_EP_OUT, &fmt) == 0;
}

/* Get the frame buffer back from the UVC class. Returns true once it is free. */
static bool frame_reclaim(void)
{
	struct video_buffer *vbuf;

	if (!frame_in_flight) {
		return true;
	}

	if (video_dequeue(uvc_dev, VIDEO_EP_OUT, &vbuf, K_NO_WAIT) == 0) {
		frame_in_flight = false;
		return true;
	}

	if (stream_open()) {
		/* Still being pulled by the host. */
		return false;
	}

	/*
	 * The host closed the stream with the frame still queued. Cancel the
	 * transfer; the buffer comes back asynchronously from the aborted
	 * bulk IN request, hence the short wait.
	 */
	if (video_flush(uvc_dev, VIDEO_EP_IN, true) != 0) {
		return false;
	}

	if (video_dequeue(uvc_dev, VIDEO_EP_OUT, &vbuf, K_MSEC(50)) == 0) {
		frame_in_flight = false;
		return true;
	}

	return false;
}

/* Send the frame if the host is streaming and the previous send completed. */
static void frame_send(void)
{
	int ret;

	if (frame_in_flight || !stream_open()) {
		return;
	}

	sys_cache_data_flush_range(frame->buffer, frame_bytes);

	frame->bytesused = frame_bytes;
	frame->line_offset = 0;
	frame->timestamp = k_uptime_get_32();

	ret = video_enqueue(uvc_dev, VIDEO_EP_IN, frame);
	if (ret) {
		LOG_ERR("UVC enqueue failed (%d)", ret);
		return;
	}
	frame_in_flight = true;
}

int uvc_display_init(int width, int height)
{
	uint32_t *pairs;

	if (frame != NULL) {
		return (width == src_fmt.width && height == src_fmt.height) ? 0 : -EALREADY;
	}

	if ((width % 2) != 0) {
		LOG_ERR("YUY2 needs an even width, got %d", width);
		return -EINVAL;
	}

	src_fmt.pixelformat = VIDEO_PIX_FMT_YUYV;
	src_fmt.width = width;
	src_fmt.height = height;
	src_fmt.pitch = width * YUY2_BYTES;

	src_caps[0] = (struct video_format_cap){
		.pixelformat = VIDEO_PIX_FMT_YUYV,
		.width_min = width,
		.width_max = width,
		.height_min = height,
		.height_max = height,
	};

	frame_bytes = (size_t)src_fmt.pitch * height;
	if (frame_bytes > sizeof(frame_buf)) {
		LOG_ERR("%dx%d frame does not fit the %u byte UVC frame buffer", width, height,
			(unsigned int)sizeof(frame_buf));
		return -ENOMEM;
	}

	frame_vbuf.buffer = frame_buf;
	frame_vbuf.size = frame_bytes;
	frame = &frame_vbuf;

	pairs = (uint32_t *)frame->buffer;
	for (size_t i = 0; i < frame_bytes / sizeof(*pairs); i++) {
		pairs[i] = YUY2_BLACK_PAIR;
	}

	return 0;
}

int uvc_display_start(void)
{
	struct usbd_context *usbd;
	int ret;

	if (usb_started) {
		return 0;
	}

	if (frame == NULL) {
		return -ENODEV;
	}

	if (!device_is_ready(uvc_dev)) {
		LOG_ERR("UVC device not ready");
		return -ENODEV;
	}

	uvc_set_video_dev(uvc_dev, DEVICE_GET(uvc_display_src));

	usbd = sample_usbd_init_device(usbd_msg_cb);
	if (usbd == NULL) {
		LOG_ERR("Failed to initialize USB device");
		return -EIO;
	}

	ret = usbd_enable(usbd);
	if (ret) {
		LOG_ERR("Failed to enable USB: %d", ret);
		return ret;
	}
	usb_started = true;

	LOG_INF("UVC display enabled: YUY2 %ux%u", src_fmt.width, src_fmt.height);

	return 0;
}

int uvc_display_capture(const uint8_t *rgb888)
{
	aipl_error_t ret;

	if (frame == NULL) {
		return -ENODEV;
	}

	/* A frame left pending by a failed iteration is still ours to reuse. */
	if (!frame_pending && (!frame_reclaim() || !stream_open())) {
		return -EAGAIN;
	}

	ret = aipl_color_convert(rgb888, frame->buffer, src_fmt.width, src_fmt.width,
				 src_fmt.height, AIPL_COLOR_RGB888, AIPL_COLOR_YUY2);
	if (ret != AIPL_ERR_OK) {
		LOG_ERR("RGB888->YUY2 conversion failed (%d)", ret);
		return -EIO;
	}
	frame_pending = true;

	return 0;
}

struct yuv {
	uint8_t y, u, v;
};

/* BT.601 limited range, as used by UVC YUY2. */
static struct yuv rgb565_to_yuv(uint16_t color)
{
	int r = ((color >> 11) & 0x1f) << 3;
	int g = ((color >> 5) & 0x3f) << 2;
	int b = (color & 0x1f) << 3;

	return (struct yuv){
		.y = (uint8_t)(((66 * r + 129 * g + 25 * b + 128) >> 8) + 16),
		.u = (uint8_t)(((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128),
		.v = (uint8_t)(((112 * r - 94 * g - 18 * b + 128) >> 8) + 128),
	};
}

/*
 * YUY2 packs two pixels as Y0 U Y1 V, so the chroma of a pixel is shared with
 * its horizontal neighbour.
 */
static void fill_rect(int x0, int y0, int x1, int y1, struct yuv c)
{
	x0 = CLAMP(x0, 0, (int)src_fmt.width);
	x1 = CLAMP(x1, 0, (int)src_fmt.width);
	y0 = CLAMP(y0, 0, (int)src_fmt.height);
	y1 = CLAMP(y1, 0, (int)src_fmt.height);

	for (int y = y0; y < y1; y++) {
		uint8_t *row = &frame->buffer[y * src_fmt.pitch];

		for (int x = x0; x < x1; x++) {
			uint8_t *pair = &row[(x & ~1) * 2];

			row[x * 2] = c.y;
			pair[1] = c.u;
			pair[3] = c.v;
		}
	}
}

void uvc_display_draw_rect(int x, int y, int w, int h, uint16_t color)
{
	struct yuv c;

	if (!frame_pending || w <= 0 || h <= 0) {
		return;
	}

	c = rgb565_to_yuv(color);

	fill_rect(x, y, x + w, y + BOX_THICKNESS, c);
	fill_rect(x, y + h - BOX_THICKNESS, x + w, y + h, c);
	fill_rect(x, y, x + BOX_THICKNESS, y + h, c);
	fill_rect(x + w - BOX_THICKNESS, y, x + w, y + h, c);
}

void uvc_display_submit(void)
{
	if (!frame_pending) {
		return;
	}
	frame_pending = false;

	frame_send();
}

#if UVC_LCD_ENABLED
/*
 * Zephyr display driver on top of the UVC frame, so that LVGL (or any display
 * API client) can render to the webcam. Written areas are converted from
 * RGB565 into the persistent YUY2 frame, which is sent after the last area of
 * each refresh.
 */
#define DT_DRV_COMPAT alif_uvc_display

BUILD_ASSERT(DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT) == 1,
	     "Only one alif,uvc-display instance is supported");

#if defined(CONFIG_LVGL)
BUILD_ASSERT((CONFIG_LV_Z_AREA_X_ALIGNMENT_WIDTH % 2) == 0,
	     "YUY2 pixel pairs need LVGL areas rounded to an even x");
#endif

static int uvc_lcd_write(const struct device *dev, const uint16_t x, const uint16_t y,
			 const struct display_buffer_descriptor *desc, const void *buf)
{
	const uint16_t *src = buf;

	ARG_UNUSED(dev);

	if (((x | desc->width) & 1) != 0 ||
	    x + desc->width > src_fmt.width || y + desc->height > src_fmt.height) {
		return -EINVAL;
	}

	for (int timeout = FRAME_IDLE_TIMEOUT_MS; !frame_reclaim() && timeout > 0; timeout--) {
		k_msleep(1);
	}

	for (uint16_t row = 0; row < desc->height; row++) {
		aipl_error_t ret = aipl_color_convert(
			&src[row * desc->pitch],
			&frame->buffer[(y + row) * src_fmt.pitch + x * YUY2_BYTES],
			desc->width, desc->width, 1, AIPL_COLOR_RGB565, AIPL_COLOR_YUY2);

		if (ret != AIPL_ERR_OK) {
			LOG_ERR("RGB565->YUY2 conversion failed (%d)", ret);
			return -EIO;
		}
	}

	if (!desc->frame_incomplete) {
		frame_send();
	}

	return 0;
}

static int uvc_lcd_blanking(const struct device *dev)
{
	ARG_UNUSED(dev);

	return 0;
}

static void uvc_lcd_get_capabilities(const struct device *dev,
				     struct display_capabilities *caps)
{
	ARG_UNUSED(dev);

	memset(caps, 0, sizeof(*caps));
	caps->x_resolution = src_fmt.width;
	caps->y_resolution = src_fmt.height;
	caps->supported_pixel_formats = PIXEL_FORMAT_RGB_565;
	caps->current_pixel_format = PIXEL_FORMAT_RGB_565;
	caps->screen_info = SCREEN_INFO_X_ALIGNMENT_WIDTH;
	caps->current_orientation = DISPLAY_ORIENTATION_NORMAL;
}

static int uvc_lcd_set_pixel_format(const struct device *dev,
				    const enum display_pixel_format pixel_format)
{
	ARG_UNUSED(dev);

	return pixel_format == PIXEL_FORMAT_RGB_565 ? 0 : -ENOTSUP;
}

static DEVICE_API(display, uvc_lcd_api) = {
	.blanking_on = uvc_lcd_blanking,
	.blanking_off = uvc_lcd_blanking,
	.write = uvc_lcd_write,
	.get_capabilities = uvc_lcd_get_capabilities,
	.set_pixel_format = uvc_lcd_set_pixel_format,
};

/* The frame must exist before LVGL starts drawing; USB is started later. */
static int uvc_lcd_init(const struct device *dev)
{
	ARG_UNUSED(dev);

	return uvc_display_init(DT_INST_PROP(0, width), DT_INST_PROP(0, height));
}

DEVICE_DT_INST_DEFINE(0, uvc_lcd_init, NULL, NULL, NULL, POST_KERNEL,
		      CONFIG_DISPLAY_INIT_PRIORITY, &uvc_lcd_api);
#endif /* UVC_LCD_ENABLED */

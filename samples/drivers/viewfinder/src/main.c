/*
 * Copyright (C) Alif Semiconductor.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <string.h>

#include <zephyr/drivers/video.h>
#include <soc_common.h>
#include <se_service.h>
#include <zephyr/drivers/gpio.h>

#include <zephyr/drivers/video/video_alif.h>
#include <zephyr/drivers/video/isp-vsi.h>

#include <zephyr/logging/log.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/display/cdc200.h>
#ifdef CONFIG_MIPI_DSI
#include <zephyr/drivers/mipi_dsi/dsi_dw.h>
#endif /* CONFIG_MIPI_DSI */
#include <zephyr/cache.h>

#include <aipl_error.h>

#define ISP_ENABLED DT_NODE_HAS_STATUS_OKAY(DT_NODELABEL(isp))

#if ISP_ENABLED
#include <aipl_color_conversion.h>
#else
#include <aipl_demosaic.h>
#endif

#include "alif_logo.h"

LOG_MODULE_REGISTER(video_app, LOG_LEVEL_INF);

#define N_FRAMES		2
#define N_VID_BUFF              MIN(CONFIG_VIDEO_BUFFER_POOL_NUM_MAX, N_FRAMES)

#ifdef CONFIG_DT_HAS_HIMAX_HM0360_ENABLED
#define PIPELINE_FORMAT	VIDEO_PIX_FMT_BGGR8
#define SENSOR_WIDTH	320
#define SENSOR_HEIGHT	240
#define PIPELINE_IS_RGB565	0
#elif CONFIG_DT_HAS_OVTI_OV5640_ENABLED
#define PIPELINE_FORMAT	VIDEO_PIX_FMT_RGB565
#define SENSOR_WIDTH	160
#define SENSOR_HEIGHT	120
#define PIPELINE_IS_RGB565	1
#elif CONFIG_DT_HAS_APTINA_MT9M114_ENABLED
#if ISP_ENABLED
#define PIPELINE_FORMAT	VIDEO_PIX_FMT_Y10P
#define SENSOR_WIDTH	648
#define SENSOR_HEIGHT	488
#define PIPELINE_IS_RGB565	0
#else
#define PIPELINE_FORMAT	VIDEO_PIX_FMT_RGB565
#define SENSOR_WIDTH	648
#define SENSOR_HEIGHT	488
#define PIPELINE_IS_RGB565	1
#endif
#else
#define PIPELINE_FORMAT	VIDEO_PIX_FMT_Y10P
#define SENSOR_WIDTH	0
#define SENSOR_HEIGHT	0
#define PIPELINE_IS_RGB565	0
#endif

#if ISP_ENABLED
#define OUTPUT_FORMAT	VIDEO_PIX_FMT_YUV420
#endif

#if (CONFIG_VIDEO_ALIF_CAM_EXTENDED && CONFIG_VIDEO_MIPI_CSI2_DW)
#define NUM_CAMS DT_PROP_LEN(DT_NODELABEL(csi), phy_if)
#else
#define NUM_CAMS 1
#endif /* CONFIG_VIDEO_ALIF_CAM_EXTENDED */

#if ISP_ENABLED
enum logging_level {
	LOGGING_LEVEL_NONE = 0,
	LOGGING_LEVEL_ERR,
	LOGGING_LEVEL_WARN,
	LOGGING_LEVEL_INFO,
	LOGGING_LEVEL_DEBUG,
	LOGGING_LEVEL_VERBOSE,
};

#define LIB_LOG_LEVEL LOGGING_LEVEL_NONE

int log_level(void)
{
	return LIB_LOG_LEVEL;
}
#endif

static int fourcc_to_pitch(uint32_t fourcc, uint32_t width)
{
	int pitch;

	switch (fourcc) {
	case VIDEO_PIX_FMT_RGB888_PLANAR_PRIVATE:
	case VIDEO_PIX_FMT_NV24:
	case VIDEO_PIX_FMT_NV42:
		pitch = width * 3;
		break;
	case VIDEO_PIX_FMT_RGB565:
	case VIDEO_PIX_FMT_Y10P:
	case VIDEO_PIX_FMT_BGGR10:
	case VIDEO_PIX_FMT_GBRG10:
	case VIDEO_PIX_FMT_GRBG10:
	case VIDEO_PIX_FMT_RGGB10:
	case VIDEO_PIX_FMT_BGGR12:
	case VIDEO_PIX_FMT_GBRG12:
	case VIDEO_PIX_FMT_GRBG12:
	case VIDEO_PIX_FMT_RGGB12:
	case VIDEO_PIX_FMT_BGGR14:
	case VIDEO_PIX_FMT_GBRG14:
	case VIDEO_PIX_FMT_GRBG14:
	case VIDEO_PIX_FMT_RGGB14:
	case VIDEO_PIX_FMT_BGGR16:
	case VIDEO_PIX_FMT_GBRG16:
	case VIDEO_PIX_FMT_GRBG16:
	case VIDEO_PIX_FMT_RGGB16:
	case VIDEO_PIX_FMT_Y10:
	case VIDEO_PIX_FMT_Y12:
	case VIDEO_PIX_FMT_Y14:
	case VIDEO_PIX_FMT_YUYV:
	case VIDEO_PIX_FMT_YVYU:
	case VIDEO_PIX_FMT_VYUY:
	case VIDEO_PIX_FMT_UYVY:
	case VIDEO_PIX_FMT_NV16:
	case VIDEO_PIX_FMT_NV61:
	case VIDEO_PIX_FMT_YUV422P:
		pitch = width << 1;
		break;
	case VIDEO_PIX_FMT_NV12:
	case VIDEO_PIX_FMT_NV21:
	case VIDEO_PIX_FMT_YUV420:
	case VIDEO_PIX_FMT_YVU420:
		pitch = (width * 3) >> 1;
		break;
	case VIDEO_PIX_FMT_BGGR8:
	case VIDEO_PIX_FMT_GBRG8:
	case VIDEO_PIX_FMT_GRBG8:
	case VIDEO_PIX_FMT_RGGB8:
	case VIDEO_PIX_FMT_GREY:
	default:
		pitch = width;
		break;
	}

	return pitch;
}

#if ISP_ENABLED && defined(CONFIG_ISP_LIB_AE_MODULE)
static void ae_status(const struct device *dev, uint8_t ae_stable,
		       void *user_data)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(user_data);
	LOG_INF("AE Stabilization status: %d", ae_stable);
}
#endif /* ISP_ENABLED && defined(CONFIG_ISP_LIB_AE_MODULE) */

/*
 * Blit the Alif logo (indexed ALPHA8 with RGBA LUT) to the
 * framebuffer in RGB565.
 * Placed at the bottom-center of the display, below the camera image area.
 */
static void blit_logo_to_fb(uint16_t *fb, uint32_t fb_stride_px,
			     uint32_t cam_height, uint32_t panel_height)
{
	const aipl_image_t *logo = get_alif_logo();
	const uint8_t *lut = get_alif_lut();
	const uint8_t *src = (const uint8_t *)logo->data;
	uint32_t logo_w = logo->width;
	uint32_t logo_h = logo->height;
	uint32_t x_off = (fb_stride_px - logo_w) / 2 - 10;
	uint32_t y_off = cam_height + (panel_height - cam_height - logo_h) / 2;

	for (uint32_t y = 0; y < logo_h; y++) {
		for (uint32_t x = 0; x < logo_w; x++) {
			uint8_t idx = src[y * logo_w + x];
			uint8_t b = lut[idx * 4 + 0];
			uint8_t g = lut[idx * 4 + 1];
			uint8_t r = lut[idx * 4 + 2];
			uint16_t rgb565 = ((uint16_t)(r >> 3) << 11) |
					  ((uint16_t)(g >> 2) << 5) |
					  (uint16_t)(b >> 3);
			fb[(y_off + y) * fb_stride_px + (x_off + x)] = rgb565;
		}
	}
}

int main(void)
{
	struct video_buffer *buffers[N_VID_BUFF], *vbuf;
	struct video_format fmt = { 0 };
	struct video_caps caps;
	const struct device *video;
	enum video_endpoint_id ep;
	unsigned int frame = 0;
	size_t bsize;
	int i = 0;
	int ret;
	struct video_buffer *conv_buf = NULL;

	struct cdc200_display_caps capabilities;
	struct cdc200_fb_desc fb_l2 = { 0 };
	const struct device *display_dev;
	uint16_t preview_size;
	size_t panel_fb_bytes;
	size_t conv_bytes;
#if defined(CONFIG_MIPI_DSI)
	struct display_capabilities panel_caps;
	const struct device *panel;
	const struct device *dsi;

	panel = DEVICE_DT_GET(DT_ALIAS(panel));
	if (!device_is_ready(panel)) {
		LOG_ERR("Device %s not found. Aborting sample.",
			panel->name);
		return -1;
	}

	dsi = DEVICE_DT_GET(DT_ALIAS(mipi_dsi));
	if (!device_is_ready(dsi)) {
		LOG_ERR("Device %s not found. Aborting sample.",
			dsi->name);
		return -1;
	}

	LOG_DBG("Rotating the display by 180 degrees");
	ret = display_set_orientation(panel, DISPLAY_ORIENTATION_ROTATED_180);
	if (ret == -ENOTSUP) {
		LOG_INF("Un-supported Display Rotation.");
	}

	LOG_INF("Enable Ensemble-DSI Device video mode.");
	ret = dsi_dw_set_mode(dsi, DSI_DW_VIDEO_MODE);
	if (ret) {
		LOG_ERR("DSI Host controller set to video mode.");
		return -1;
	}

	display_get_capabilities(panel, &panel_caps);
	LOG_DBG("Panel Orientation - %d", panel_caps.current_orientation);

	display_blanking_off(panel);
#endif /* defined(CONFIG_MIPI_DSI) */

#ifdef CONFIG_DT_HAS_HIMAX_HM0360_ENABLED
	uint32_t num_frames;
#endif /* CONFIG_DT_HAS_HIMAX_HM0360_ENABLED */

#if (CONFIG_VIDEO_ALIF_CAM_EXTENDED && CONFIG_VIDEO_MIPI_CSI2_DW)
	uint8_t current_sensor;
#endif /* CONFIG_VIDEO_ALIF_CAM_EXTENDED */
	int loop_ctr;

	uint32_t last_timestamp = 0;
	uint32_t frame_time = 0;

	/* Per-stage timing (in microseconds) */
	uint32_t t_conv_us = 0, t_disp_us = 0, t_total_us = 0;
	uint32_t t_conv_sum = 0, t_disp_sum = 0, t_total_sum = 0;
	uint32_t t_begin, t_conv_start, t_disp_start;

#if ISP_ENABLED
	video = DEVICE_DT_GET_ONE(vsi_isp_pico);
#else
	video = DEVICE_DT_GET_ONE(alif_cam);
#endif /* ISP_ENABLED */

	if (!device_is_ready(video)) {
		LOG_ERR("%s: device not ready.", video->name);
		return -1;
	}
	LOG_INF("Video device: %s", video->name);

	/* Query the panel early so the square preview size can drive ISP
	 * output, conversion, logo placement, and cache flushes.
	 */
	display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
	if (!device_is_ready(display_dev)) {
		LOG_ERR("Display device %s not ready", display_dev->name);
		return -1;
	}
	cdc200_get_capabilities(display_dev, &capabilities);
	preview_size = MIN(capabilities.x_panel_resolution,
			   capabilities.y_panel_resolution);
	/* Conversion writes a tightly packed RGB565 square into the FB.
	 * White-fill and the logo use the full panel surface.
	 */
	conv_bytes = (size_t)preview_size * preview_size * 2;
	panel_fb_bytes = (size_t)capabilities.x_panel_resolution *
			 capabilities.y_panel_resolution * 2;
	LOG_INF("Display %s: %dx%d (preview %ux%u)",
		display_dev->name,
		capabilities.x_panel_resolution,
		capabilities.y_panel_resolution,
		preview_size, preview_size);

#ifdef CONFIG_DT_HAS_OVTI_OV5640_ENABLED
	const struct device *cam_enbuf =
		DEVICE_DT_GET(DT_NODELABEL(cam_enbuf));

	if (!device_is_ready(cam_enbuf)) {
		LOG_ERR("cam_enbuf regulator not ready");
		return -1;
	}
#endif

#if ISP_ENABLED && defined(CONFIG_ISP_LIB_AE_MODULE)
	ret = isp_vsi_register_ae_status_callback(video, ae_status, NULL);
	if (ret) {
		LOG_ERR("Failed to register AE callback!");
		return ret;
	}
#endif /* ISP_ENABLED && defined(CONFIG_ISP_LIB_AE_MODULE) */

	for (loop_ctr = NUM_CAMS - 1; loop_ctr >= 0; loop_ctr--) {
		i = 0;
		memset(&fmt, 0, sizeof(fmt));
#if (CONFIG_VIDEO_ALIF_CAM_EXTENDED && CONFIG_VIDEO_MIPI_CSI2_DW)
		ret = video_get_ctrl(video,
				VIDEO_CID_ALIF_CSI_CURR_CAM,
				&current_sensor);
		if (ret) {
			LOG_ERR("Failed to get current camera!");
			return ret;
		}
		LOG_INF("Selected camera: %s",
			(current_sensor) ? "Standard" : "Selfie");
#endif /* CONFIG_VIDEO_ALIF_CAM_EXTENDED */

		if (ISP_ENABLED) {
			ep = VIDEO_EP_IN;
		} else {
			ep = VIDEO_EP_OUT;
		}

		/* Get capabilities */
		if (video_get_caps(video, ep, &caps)) {
			LOG_ERR("Unable to retrieve video capabilities");
			return -1;
		}

		LOG_INF("Sensor Capabilities:");
		while (caps.format_caps[i].pixelformat) {
			const struct video_format_cap *fcap =
				&caps.format_caps[i];
			/* fourcc to string */
			LOG_INF("  %c%c%c%c %ux%u (wxh)",
			       (char)fcap->pixelformat,
			       (char)(fcap->pixelformat >> 8),
			       (char)(fcap->pixelformat >> 16),
			       (char)(fcap->pixelformat >> 24),
			       fcap->width_min, fcap->height_min);
			if (fcap->pixelformat == PIPELINE_FORMAT) {
				fmt.pixelformat = PIPELINE_FORMAT;
				if (SENSOR_WIDTH && SENSOR_HEIGHT) {
					fmt.width = SENSOR_WIDTH;
					fmt.height = SENSOR_HEIGHT;
				} else {
					fmt.width = fcap->width_min;
					fmt.height = fcap->height_min;
				}
			}
			i++;
		}

		if (fmt.pixelformat == 0) {
			LOG_ERR("Desired Pixel format is not supported.");
			return -1;
		}

		fmt.pitch = fourcc_to_pitch(fmt.pixelformat, fmt.width);

		LOG_INF("Setting %c%c%c%c %ux%u",
			(char)fmt.pixelformat,
			(char)(fmt.pixelformat >> 8),
			(char)(fmt.pixelformat >> 16),
			(char)(fmt.pixelformat >> 24),
			fmt.width, fmt.height);

		ret = video_set_format(video, ep, &fmt);
		if (ret) {
			LOG_ERR("Failed to set video format. ret - %d", ret);
			return -1;
		}

#if (CONFIG_VIDEO_ALIF_CAM_EXTENDED && CONFIG_VIDEO_MIPI_CSI2_DW)
		if (NUM_CAMS > 1) {
			current_sensor ^= 1;
			ret = video_set_ctrl(video, VIDEO_CID_ALIF_CSI_CURR_CAM,
					&current_sensor);
			if (ret) {
				LOG_ERR("Unable to switch camera!");
			}
		}
#endif /* CONFIG_VIDEO_ALIF_CAM_EXTENDED */
	}

#if ISP_ENABLED
	/*
	 * Set Output Endpoint format. Ensure that ISP EP-out
	 * format is set while allocating the buffers used to
	 * capture images. CSM must be on so this YU12 is real
	 * video-range YUV (MT9 is black/green without it).
	 */
	fmt.pixelformat = OUTPUT_FORMAT;
	fmt.width = preview_size;
	fmt.height = preview_size;
	fmt.pitch = fourcc_to_pitch(fmt.pixelformat, fmt.width);

	ret = video_set_format(video, VIDEO_EP_OUT, &fmt);
	if (ret) {
		LOG_ERR("Failed to set video format for ISP EP-out. ret - %d", ret);
		return -1;
	}
#endif /* ISP_ENABLED */

	LOG_INF("Format: %c%c%c%c %ux%u", (char)fmt.pixelformat,
	       (char)(fmt.pixelformat >> 8),
	       (char)(fmt.pixelformat >> 16),
	       (char)(fmt.pixelformat >> 24),
	       fmt.width, fmt.height);

	/* Size to allocate for each buffer */
	bsize = fmt.pitch * fmt.height;

	LOG_INF("Width - %u, Pitch - %u, Height - %u, Buff size - %zu",
			fmt.width, fmt.pitch, fmt.height, bsize);

#if (CONFIG_VIDEO_ALIF_CAM_EXTENDED && CONFIG_VIDEO_MIPI_CSI2_DW)
		if (NUM_CAMS > 1) {
			current_sensor = 0;
			ret = video_set_ctrl(video, VIDEO_CID_ALIF_CSI_CURR_CAM,
					&current_sensor);
			if (ret) {
				LOG_ERR("Unable to switch camera!");
			}
		}
#endif /* CONFIG_VIDEO_ALIF_CAM_EXTENDED */

	/* Alloc video buffers and enqueue for capture */
	for (i = 0; i < ARRAY_SIZE(buffers); i++) {
		buffers[i] = video_buffer_alloc(bsize, K_NO_WAIT);
		if (buffers[i] == NULL) {
			LOG_ERR("Unable to alloc video buffer");
			return -1;
		}

		/* Allocated Buffer Information */
		LOG_INF("Buffer %d: addr 0x%x, size %d, %ux%u",
			i, (uint32_t)buffers[i]->buffer,
			bsize,
			fmt.width, fmt.height);

		memset(buffers[i]->buffer, 0, bsize);
		video_enqueue(video, VIDEO_EP_OUT, buffers[i]);

		LOG_DBG("capture buffer[%d]", i);
		LOG_DBG("dump: \"~/capture_%d.bin\" 0x%08x 0x%08x -r",
			i, (uint32_t)buffers[i]->buffer,
			(uint32_t)buffers[i]->buffer + bsize - 1);
	}

	/* Use display framebuffer (in SDRAM) as conversion output buffer
	 * to avoid consuming SRAM (video pool / heap are too small).
	 */
	{
		static struct video_buffer fb_conv_buf;

		cdc200_get_framebuffer(display_dev, 0, &fb_l2);
		if (fb_l2.fb_addr == 0 || fb_l2.fb_size < panel_fb_bytes) {
			LOG_ERR("FB too small: addr 0x%x size %u (need %zu)",
				(uint32_t)fb_l2.fb_addr, fb_l2.fb_size,
				panel_fb_bytes);
			return -1;
		}
		fb_conv_buf.buffer = (uint8_t *)fb_l2.fb_addr;
		fb_conv_buf.size = fb_l2.fb_size;
		fb_conv_buf.bytesused = 0;
		conv_buf = &fb_conv_buf;
	}
	LOG_INF("Conversion buffer (framebuffer): addr - 0x%x, size - %zu",
		(uint32_t)conv_buf->buffer, conv_bytes);

	/*
	 * TODO: Need to fix this delay.
	 * As per our observation, if we are not giving this much delay
	 * then the camera sensor is not setup properly and images
	 * sent out are not clear.
	 */
	k_msleep(1000);

#if CONFIG_DT_HAS_HIMAX_HM0360_ENABLED
	/* Video test SNAPSHOT capture. */
	num_frames = N_FRAMES;
	ret = video_set_ctrl(video, VIDEO_CID_SNAPSHOT_CAPTURE, &num_frames);
	if (ret) {
		LOG_INF("Snapshot mode not-supported by CMOS sensor.");
	}
#endif

#ifdef CONFIG_DT_HAS_OVTI_OV5640_ENABLED
	regulator_enable(cam_enbuf);
#endif

	/* --- Initialize display once, before capture starts --- */
	cdc200_set_enable(display_dev, true);

	/* Fill the entire panel white once (uses conv_buf as 1-row scratch) */
	{
		uint16_t panel_w = capabilities.x_panel_resolution;
		uint16_t panel_h = capabilities.y_panel_resolution;
		uint16_t *white_row = (uint16_t *)conv_buf->buffer;
		size_t white_bytes  = (size_t)panel_w * 2;
		struct display_buffer_descriptor wd = {
			.buf_size = white_bytes,
			.width    = panel_w,
			.height   = 1,
			.pitch    = panel_w,
		};

		for (size_t k = 0; k < panel_w; k++) {
			white_row[k] = 0xFFFF; /* RGB565 white */
		}
		sys_cache_data_flush_range(white_row, white_bytes);
		for (uint16_t row = 0; row < panel_h; row++) {
			ret = display_write(display_dev, 0, row, &wd,
					(uint8_t *)white_row);
			if (ret) {
				LOG_ERR("White fill failed at row %u: %d",
					row, ret);
				break;
			}
		}
	}

	/* Blit Alif logo below the square camera preview */
	blit_logo_to_fb((uint16_t *)conv_buf->buffer,
			capabilities.x_panel_resolution,
			preview_size,
			capabilities.y_panel_resolution);
	sys_cache_data_flush_range(conv_buf->buffer, panel_fb_bytes);
	LOG_DBG("Logo blitted below %ux%u image area",
		preview_size, preview_size);

	/* Start video capture */
	ret = video_stream_start(video);
	if (ret) {
		LOG_ERR("Unable to start capture. ret - %d", ret);
		return -1;
	}
	LOG_INF("Capture started");

	/* Continuous capture loop */
	while (1) {
		ret = video_dequeue(video, VIDEO_EP_OUT, &vbuf, K_FOREVER);
		if (ret) {
			LOG_ERR("Unable to dequeue video buf");
			goto stop_capture;
		}

		frame++;
		if (last_timestamp != 0) {
			frame_time = vbuf->timestamp - last_timestamp;
		}
		last_timestamp = vbuf->timestamp;

		t_begin = k_cycle_get_32();

#if !ISP_ENABLED
#if PIPELINE_IS_RGB565
		/* Processed RGB565 sensor: center-crop into the square
		 * preview. No demosaic or YUV convert.
		 */
		t_conv_start = k_cycle_get_32();
		{
			uint32_t crop_w = MIN(preview_size, fmt.width);
			uint32_t crop_h = MIN(preview_size, fmt.height);
			uint32_t crop_x = (fmt.width - crop_w) / 2;
			uint32_t crop_y = (fmt.height - crop_h) / 2;
			uint16_t *src = (uint16_t *)vbuf->buffer +
					crop_y * fmt.width + crop_x;
			uint16_t *dst = (uint16_t *)conv_buf->buffer;
			uint32_t row;

			sys_cache_data_invd_range(vbuf->buffer,
				vbuf->bytesused ? vbuf->bytesused : bsize);

			for (row = 0; row < crop_h; row++) {
				memcpy(dst, src, crop_w * sizeof(uint16_t));
				src += fmt.width;
				dst += preview_size;
			}

			t_conv_us = k_cyc_to_us_floor32(
				k_cycle_get_32() - t_conv_start);
			t_disp_start = k_cycle_get_32();
			sys_cache_data_flush_range(conv_buf->buffer, conv_bytes);
			t_disp_us = k_cyc_to_us_floor32(
				k_cycle_get_32() - t_disp_start);
		}
#else
		/* Bayer Y10P: unpack to 8-bit, then AIPL demosaics the
		 * square preview directly to the framebuffer.
		 */
		t_conv_start = k_cycle_get_32();
		{
			uint16_t *src16 = (uint16_t *)vbuf->buffer;
			uint8_t *raw8 = (uint8_t *)vbuf->buffer;
			uint32_t total_px = fmt.width * fmt.height;
			aipl_error_t aipl_ret;

			for (uint32_t i = 0; i < total_px; i++) {
				raw8[i] = (src16[i] & 0x03FFu) >> 2;
			}

			aipl_ret = aipl_demosaic_rgb565(
					vbuf->buffer,
					conv_buf->buffer,
					fmt.width,
					preview_size, preview_size,
					AIPL_BAYER_GRBG);
			if (aipl_ret != AIPL_ERR_OK) {
				LOG_ERR("AIPL demosaic failed: %d", aipl_ret);
			}

			t_conv_us = k_cyc_to_us_floor32(
				k_cycle_get_32() - t_conv_start);
			t_disp_start = k_cycle_get_32();
			/* Only the square preview is rewritten each frame;
			 * the logo below it is static.
			 */
			sys_cache_data_flush_range(conv_buf->buffer, conv_bytes);
			t_disp_us = k_cyc_to_us_floor32(
				k_cycle_get_32() - t_disp_start);
		}
#endif /* PIPELINE_FORMAT == RGB565 */
#else
		/* ISP path: AIPL converts I420 -> RGB565 directly to
		 * framebuffer. conv_buf IS the framebuffer, and AIPL writes
		 * tightly packed output (pitch=width=preview_size),
		 * so no display_write() needed.
		 */
		t_conv_start = k_cycle_get_32();
		{
			aipl_error_t aipl_ret;

			aipl_ret = aipl_color_convert_i420_to_rgb565(
					vbuf->buffer,
					conv_buf->buffer,
					fmt.width,
					fmt.width, fmt.height);
			if (aipl_ret != AIPL_ERR_OK) {
				LOG_ERR("AIPL I420->RGB565 failed: %d",
					aipl_ret);
			}

			t_conv_us = k_cyc_to_us_floor32(
				k_cycle_get_32() - t_conv_start);
			t_disp_start = k_cycle_get_32();

			/* Only the square preview is rewritten each frame;
			 * the logo below it is static.
			 */
			sys_cache_data_flush_range(conv_buf->buffer, conv_bytes);
		}
		t_disp_us = k_cyc_to_us_floor32(
				k_cycle_get_32() - t_disp_start);
#endif /* !ISP_ENABLED */

		t_total_us = k_cyc_to_us_floor32(k_cycle_get_32() - t_begin);
		t_conv_sum += t_conv_us;
		t_disp_sum += t_disp_us;
		t_total_sum += t_total_us;

		/* Periodic metrics log every 30 frames */
		if ((frame % 30) == 0 && frame_time) {
			LOG_INF("Frame %u | FPS ~%d", frame,
				1000 / (int)frame_time);
			LOG_DBG("conv %lu/%lu | disp %lu/%lu | total %lu/%lu",
				(unsigned long)t_conv_us,
				(unsigned long)(t_conv_sum / 30),
				(unsigned long)t_disp_us,
				(unsigned long)(t_disp_sum / 30),
				(unsigned long)t_total_us,
				(unsigned long)(t_total_sum / 30));
			t_conv_sum = 0;
			t_disp_sum = 0;
			t_total_sum = 0;
		}

		/* Requeue the video buffer for continuous capture */
		ret = video_enqueue(video, VIDEO_EP_OUT, vbuf);
		if (ret) {
			LOG_ERR("Unable to requeue video buf");
			goto stop_capture;
		}
		/* Driver auto-stops when the IN-FIFO drains; restart it.
		 * -EBUSY means streaming is still active, which is fine.
		 */
		ret = video_stream_start(video);
		if (ret && ret != -EBUSY) {
			LOG_ERR("Unable to restart capture. ret - %d", ret);
			goto stop_capture;
		}
	}

stop_capture:
	/* Only reached on error from the capture loop above. */
	LOG_INF("Stopping capture...");
	video_flush(video, VIDEO_EP_OUT, false);
	if (video_stream_stop(video)) {
		LOG_ERR("Unable to stop capture");
	}

	/* conv_buf uses display framebuffer; nothing to free */
	for (i = 0; i < ARRAY_SIZE(buffers); i++) {
		if (buffers[i]) {
			video_buffer_release(buffers[i]);
		}
	}
	return ret;
}

/*
 * Do application configurations.
 */
static int app_set_parameters(void)
{
	run_profile_t runp;
	int ret;

#if defined(CONFIG_VIDEO_MIPI_CSI2_DW)
#if (DT_NODE_HAS_STATUS(DT_NODELABEL(camera_select), okay))
	const struct gpio_dt_spec sel =
		GPIO_DT_SPEC_GET(DT_NODELABEL(camera_select), select_gpios);

	gpio_pin_configure_dt(&sel, GPIO_OUTPUT);
	gpio_pin_set_dt(&sel, 1);
#endif /* (DT_NODE_HAS_STATUS(DT_NODELABEL(camera_select), okay)) */
#endif

#if (defined(CONFIG_ENSEMBLE_GEN2) && defined(CONFIG_MIPI_DSI))
	const struct gpio_dt_spec cam_disp_mux_gpio =
		GPIO_DT_SPEC_GET(DT_NODELABEL(mipi_dsi), cam_disp_mux_gpios);
	gpio_pin_configure_dt(&cam_disp_mux_gpio, GPIO_OUTPUT_ACTIVE);
#endif

	/* Enable HFOSC (38.4 MHz) and CFG (100 MHz) clock. */
#if defined(CONFIG_SOC_SERIES_E8)
	sys_set_bits(CGU_CLK_ENA, BIT(23) | BIT(7));
#else
	sys_set_bits(CGU_CLK_ENA, BIT(23) | BIT(21));
#endif /* defined (CONFIG_SOC_SERIES_E7) */

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

	runp.phy_pwr_gating |= MIPI_TX_DPHY_MASK | MIPI_RX_DPHY_MASK |
		MIPI_PLL_DPHY_MASK | LDO_PHY_MASK;
	runp.ip_clock_gating = CAMERA_MASK | MIPI_CSI_MASK |
		MIPI_DSI_MASK | CDC200_MASK | GPU_MASK;

	ret = se_service_set_run_cfg(&runp);
	__ASSERT(ret == 0, "SE: set_run_cfg failed = %d", ret);

	/*
	 * CPI Pixel clock - Generate XVCLK. Used by ARX3A0
	 * TODO: parse this clock from DTS and set on board from camera
	 * controller driver.
	 */
#if defined(CONFIG_VIDEO_MIPI_CSI2_DW)
	sys_write32(0x140001, CLKCTRL_PER_MST_CAMERA_PIXCLK_CTRL);
#endif

#if (DT_NODE_HAS_STATUS(DT_NODELABEL(lpcam), okay))
	/* Enable LPCAM controller Pixel Clock (XVCLK). */
	/*
	 * Not needed for the time being as LP-CAM supports only
	 * parallel data-mode of capture and only MT9M114 sensor is
	 * tested with parallel data capture which generates clock
	 * internally. But can be used to generate XVCLK from LP CAM
	 * controller.
	 * sys_write32(0x140001, M55HE_CFG_HE_CAMERA_PIXCLK);
	 */
#endif
	return 0;
}

SYS_INIT(app_set_parameters, PRE_KERNEL_1, 46);

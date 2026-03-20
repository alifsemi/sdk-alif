/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https: //alifsemi.com/license
 *
 */

#include <zephyr/drivers/video/video_alif.h>

#include "image_ensemble.h"
#include "image_processing.h"

#include <aipl_demosaic.h>
#include <aipl_resize.h>
#include <aipl_color_correction.h>
#include <aipl_white_balance.h>
#include <aipl_lut_transform.h>
#include <aipl_crop.h>
#include "aipl_cache.h"
#include <zephyr/cache.h>

#include <tgmath.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>
#include <stddef.h>

#include <zephyr/drivers/video.h>
#include <zephyr/drivers/video-controls.h>
#include <zephyr/logging/log.h>

#include <arm_mve.h>

LOG_MODULE_REGISTER(image_ensemble, LOG_LEVEL_INF);

#define ISP_ENABLED DT_NODE_HAS_STATUS_OKAY(DT_NODELABEL(isp))
#if ISP_ENABLED
#define OUTPUT_FORMAT	VIDEO_PIX_FMT_RGB888_PLANAR_PRIVATE
#endif

#if defined(CONFIG_SOC_SERIES_E8)
#if ISP_ENABLED
#define VIDEO_BUFFER_COUNT CONFIG_VIDEO_BUFFER_POOL_NUM_MAX
#else
#define VIDEO_BUFFER_COUNT 1
#endif
#else /* for E7 & ARX3A0 2 buffers are needed, otherwise image pipeline will stall */
#if DT_HAS_COMPAT_STATUS_OKAY(onnn_arx3a0)
#define VIDEO_BUFFER_COUNT 2
#else
#define VIDEO_BUFFER_COUNT 1
#endif
#endif

static const struct device *video_dev;
struct video_buffer *buffers[VIDEO_BUFFER_COUNT], *vbuf;

/* Output dimensions, set during image_init() */
static int output_width;
static int output_height;

/*
 * Camera fills the raw_image buffer.
 * Bayer->RGB conversion transfers into the rgb_image buffer.
 */
static uint8_t image_data[CIMAGE_RGB_WIDTH_MAX * CIMAGE_RGB_HEIGHT_MAX * RGB_BYTES]
#if defined(CONFIG_SOC_SERIES_E8)
	__section("SRAM1.camera_frame_bayer_to_rgb_buf");
#else
	__section("SRAM0.camera_frame_bayer_to_rgb_buf");
#endif

void aipl_cpu_cache_clean(const void *ptr, uint32_t size)
{
	sys_cache_data_flush_range((void *)ptr, size);
}

void aipl_cpu_cache_invalidate(const void *ptr, uint32_t size)
{
	sys_cache_data_invd_range((void *)ptr, size);
}

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

int image_init(int req_output_width, int req_output_height)
{
	struct video_format fmt = { 0 };
	struct video_caps caps;
	size_t bsize;
	int i = 0;
	int ret;

	output_width = req_output_width;
	output_height = req_output_height;

	enum video_endpoint_id ep = VIDEO_EP_OUT;

	#if ISP_ENABLED
	video_dev = DEVICE_DT_GET_ONE(vsi_isp_pico);
	ep = VIDEO_EP_IN;
	#else
	video_dev = DEVICE_DT_GET_ONE(alif_cam);
	#endif
	if (!device_is_ready(video_dev)) {
		LOG_ERR("%s: device not ready.", video_dev->name);
		return -1;
	}
	LOG_INF("- Device name: %s\n", video_dev->name);

	/* Get capabilities */
	if (video_get_caps(video_dev, ep, &caps)) {
		LOG_ERR("Unable to retrieve video capabilities");
		return -1;
	}

	LOG_INF("- Capabilities:\n");
	while (caps.format_caps[i].pixelformat) {
		const struct video_format_cap *fcap = &caps.format_caps[i];
		/* fourcc to string */
		LOG_INF("  %c%c%c%c width (min, max, step)[%u; %u; %u] "
			"height (min, max, step)[%u; %u; %u]\n",
		       (char)fcap->pixelformat,
		       (char)(fcap->pixelformat >> 8),
		       (char)(fcap->pixelformat >> 16),
		       (char)(fcap->pixelformat >> 24),
		       fcap->width_min, fcap->width_max, fcap->width_step,
		       fcap->height_min, fcap->height_max, fcap->height_step);

		if (fcap->pixelformat == VIDEO_PIX_FMT_Y10P) {
			fmt.pixelformat = VIDEO_PIX_FMT_Y10P;
			fmt.width = fcap->width_min;
			fmt.height = fcap->height_min;
		}
		i++;
	}

	if (fmt.pixelformat == 0) {
		LOG_ERR("Desired Pixel format is not supported.");
		return -1;
	}

    fmt.pitch = fourcc_to_pitch(fmt.pixelformat, fmt.width);

	ret = video_set_format(video_dev, ep, &fmt);
	if (ret) {
		LOG_ERR("Failed to set video format. ret - %d", ret);
		return -1;
	}

	LOG_INF("- pipeline format: %c%c%c%c %ux%u\n", (char)fmt.pixelformat,
	       (char)(fmt.pixelformat >> 8),
	       (char)(fmt.pixelformat >> 16),
	       (char)(fmt.pixelformat >> 24),
	       fmt.width, fmt.height);

	#if (ISP_ENABLED)
		/*
		 * Set Output Endpoint format. ISP scales from sensor
		 * resolution to the requested capture size.
		 */
		fmt.width = output_width;
		fmt.height = output_height;
		fmt.pixelformat = OUTPUT_FORMAT;
		fmt.pitch = fourcc_to_pitch(fmt.pixelformat, fmt.width);

		LOG_INF("- output format: %c%c%c%c %ux%u\n", (char)fmt.pixelformat,
		(char)(fmt.pixelformat >> 8),
		(char)(fmt.pixelformat >> 16),
		(char)(fmt.pixelformat >> 24),
		fmt.width, fmt.height);

		ret = video_set_format(video_dev, VIDEO_EP_OUT, &fmt);
		if (ret) {
			LOG_ERR("Failed to set video output format. ret - %d", ret);
			return -1;
		}
	#endif

	/* Size to allocate for each buffer */
	bsize = fmt.pitch * fmt.height;

	LOG_INF("Width - %d, Pitch - %d, Height - %d, Buff size - %d\n",
		fmt.width, fmt.pitch, fmt.height, bsize);

	for (int j = 0; j < ARRAY_SIZE(buffers); j++) {
		buffers[j] = video_buffer_alloc(bsize, K_NO_WAIT);
		if (buffers[j] == NULL) {
			LOG_ERR("Unable to alloc video buffer");
			return -1;
		}

		/* Allocated Buffer Information */
		LOG_INF("- addr - 0x%x, size - %d, bytesused - %d\n",
			(uint32_t)buffers[j]->buffer,
			bsize,
			buffers[j]->bytesused);

		video_enqueue(video_dev, VIDEO_EP_OUT, buffers[j]);
	}

	/* Start streaming */
	ret = video_stream_start(video_dev);
	if (ret) {
		LOG_ERR("Unable to start capture (interface). ret - %d", ret);
		return -1;
	}

	LOG_INF("Capture started\n");

	return 0;
}

int get_image_data(uint8_t **output_image_data)
{
	int ret;
	aipl_error_t aipl_ret;

	ret = video_dequeue(video_dev, VIDEO_EP_OUT, &vbuf, K_FOREVER);
	if (ret) {
		LOG_ERR("Unable to dequeue video buf");
		return -1;
	}

	/*
	 * Drain any stale frames that accumulated while the previous frame was
	 * being processed (inference + display).  Re-enqueue them immediately so
	 * the ISP has empty buffers to write into and does not stall.  Keep only
	 * the newest (last) dequeued buffer for actual processing.
	 */
	struct video_buffer *newer;
	while (video_dequeue(video_dev, VIDEO_EP_OUT, &newer, K_NO_WAIT) == 0) {
		video_enqueue(video_dev, VIDEO_EP_OUT, vbuf);
		vbuf = newer;
	}

	uint8_t *raw_image = vbuf->buffer;

	#if !ISP_ENABLED
	/* in place conversion of RAW10 to RAW8 (scaling) */
	/* When CIMAGE_EXPOSURE_CALC is enabled, exposure statistics are computed during conversion */
	raw10_gray16le_bytes_to_raw8_inplace_mve(raw_image, (CIMAGE_X * CIMAGE_Y));

	/* Crop image */
#if CIMAGE_X > CIMAGE_RGB_WIDTH_MAX || CIMAGE_Y > CIMAGE_RGB_HEIGHT_MAX
	uint32_t x0 = (CIMAGE_X - CIMAGE_RGB_WIDTH_MAX) / 2;
	uint32_t y0 = (CIMAGE_Y - CIMAGE_RGB_HEIGHT_MAX) / 2;
	uint32_t y1 = y0 + CIMAGE_RGB_HEIGHT_MAX;
	uint32_t x1 = x0 + CIMAGE_RGB_WIDTH_MAX;

	crop_bayer8_inplace_topleft(
    raw_image,
    CIMAGE_X,
    CIMAGE_Y,
    x0,
    y0,
    CIMAGE_RGB_WIDTH_MAX,
    CIMAGE_RGB_HEIGHT_MAX);

	LOG_DBG("Image cropped to region x:[%u, %u], y:[%u, %u]\n", x0, x1, y0, y1);
#endif

	/* RGB conversion from RAW8 Bayer pattern */
	aipl_ret = aipl_demosaic(raw_image, image_data,
				 CIMAGE_RGB_WIDTH_MAX, CIMAGE_RGB_WIDTH_MAX,
				 CIMAGE_RGB_HEIGHT_MAX, CAM_BAYER_FORMAT,
				 AIPL_COLOR_RGB888);

	if (aipl_ret != AIPL_ERR_OK) {
		LOG_ERR("Demosaic failed with error code %d", aipl_ret);
		return -1;
	}
	#else
	/* Planar RGB888 to packed RGB888 conversion.
	 * ISP outputs R, G, B as separate planes: [R plane][G plane][B plane]
	 * ML model expects packed: RGBRGBRGB...
	 */
	rgb888_planar_to_packed(raw_image, image_data,
				output_width, output_height);
	#endif

	ret = video_enqueue(video_dev, VIDEO_EP_OUT, vbuf);
	if (ret) {
		LOG_ERR("Unable to requeue video buf");
		return -1;
	}

	ret = video_stream_start(video_dev);
	if (ret && ret != -EBUSY) {
		LOG_ERR("Unable to start capture (interface). ret - %d", ret);
		return -1;
	}

#if !ISP_ENABLED
	/* Image resizing from sensor resolution to requested output size */
	if (output_width > CIMAGE_RGB_WIDTH_MAX ||
	    output_height > CIMAGE_RGB_HEIGHT_MAX) {
		LOG_ERR("Requested image can't be processed in place");
		return -1;
	}

	aipl_ret = aipl_resize(image_data, image_data,
			       CIMAGE_RGB_WIDTH_MAX, CIMAGE_RGB_WIDTH_MAX,
			       CIMAGE_RGB_HEIGHT_MAX, AIPL_COLOR_RGB888,
			       output_width, output_height, true);
	if (aipl_ret != AIPL_ERR_OK) {
		LOG_ERR("Resize failed with error code %d", aipl_ret);
		return -1;
	}

#if CIMAGE_COLOR_CORRECTION
	/* Color correction */
	aipl_ret = aipl_color_correction_rgb(image_data, image_data,
					     output_width, output_width,
					     output_height,
					     AIPL_COLOR_RGB888,
					     camera_get_color_correction_matrix());
	if (aipl_ret != AIPL_ERR_OK) {
		LOG_ERR("Color correction failed with error code %d", aipl_ret);
		return -1;
	}

	/* LUT transform (gamma correction) */
	aipl_ret = aipl_lut_transform_rgb(image_data, image_data,
					  output_width, output_width,
					  output_height,
					  AIPL_COLOR_RGB888, camera_get_gamma_lut());
	if (aipl_ret != AIPL_ERR_OK) {
		LOG_ERR("LUT transform failed with error code %d", aipl_ret);
		return -1;
	}
#endif
#endif

	*output_image_data = image_data;

	return 0;
}

/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/drivers/video/video_alif.h>

#include "image_ensemble.h"
#include "image_processing.h"

#include <aipl_demosaic.h>
#include <aipl_resize.h>
#include <aipl_color_correction.h>
#include <aipl_white_balance.h>
#include <aipl_lut_transform.h>

#include <tgmath.h>
#include <string.h>
#include <inttypes.h>

#include <zephyr/drivers/video.h>
#include <zephyr/drivers/video-controls.h>
#include <zephyr/logging/log.h>

#include <arm_mve.h>

LOG_MODULE_REGISTER(image_ensemble, LOG_LEVEL_INF);

#define VIDEO_CTRL_CLASS_CAMERA	0x00010000	/* Camera class controls */

#define VIDEO_BUFFER_COUNT 2

static const struct device *video_dev;
struct video_buffer *buffers[VIDEO_BUFFER_COUNT], *vbuf;

/*
 * Camera fills the raw_image buffer.
 * Bayer->RGB conversion transfers into the rgb_image buffer.
 * With MT9M114 camera this can be a RGB565 to RGB conversion.
 * Following steps (crop, interpolate, colour correct) all occur in the rgb_image buffer in-place.
 */
static uint8_t image_data[CIMAGE_RGB_WIDTH_MAX * CIMAGE_RGB_HEIGHT_MAX * RGB_BYTES]
	__section("SRAM0.camera_frame_bayer_to_rgb_buf");

#include <stdint.h>
#include <stddef.h>

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

int image_init(void)
{
	struct video_format fmt = { 0 };
	struct video_caps caps;
	size_t bsize;
	int i = 0;
	int ret;

	video_dev = DEVICE_DT_GET_ONE(alif_cam);
	if (!device_is_ready(video_dev)) {
		LOG_ERR("%s: device not ready.", video_dev->name);
		return -1;
	}
	LOG_INF("- Device name: %s\n", video_dev->name);

	/* Get capabilities */
	if (video_get_caps(video_dev, VIDEO_EP_OUT, &caps)) {
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

	ret = video_set_format(video_dev, VIDEO_EP_OUT, &fmt);
	if (ret) {
		LOG_ERR("Failed to set video format. ret - %d", ret);
		return -1;
	}

	LOG_INF("- format: %c%c%c%c %ux%u\n", (char)fmt.pixelformat,
	       (char)(fmt.pixelformat >> 8),
	       (char)(fmt.pixelformat >> 16),
	       (char)(fmt.pixelformat >> 24),
	       fmt.width, fmt.height);

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

int get_image_data(int ml_width, int ml_height, uint8_t **output_image_data)
{
	int ret;

	ret = video_dequeue(video_dev, VIDEO_EP_OUT, &vbuf, K_FOREVER);
	if (ret) {
		LOG_ERR("Unable to dequeue video buf");
		return -1;
	}

	uint8_t *raw_image = vbuf->buffer;

	SCB_CleanInvalidateDCache();

	/* in place conversion of RAW10 to RAW8 (scaling) */
	/* When CIMAGE_EXPOSURE_CALC is enabled, exposure statistics are computed during conversion */
	raw10_gray16le_bytes_to_raw8_inplace_mve(raw_image, (CIMAGE_X * CIMAGE_Y));

	/* RGB conversion from RAW8 Bayer pattern */
	aipl_error_t aipl_ret = aipl_demosaic(raw_image, image_data,
					      CIMAGE_X, CIMAGE_X,
					      CIMAGE_Y, CAM_BAYER_FORMAT,
					      AIPL_COLOR_RGB888);

	if (aipl_ret != AIPL_ERR_OK) {
		LOG_ERR("Demosaic failed with error code %d", aipl_ret);
		return -1;
	}

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

	/* Image resizing */
	if (ml_width > CIMAGE_X || ml_height > CIMAGE_Y) {
		LOG_ERR("Requested image can't be processed in place");
		return -1;
	}

	aipl_ret = aipl_resize(image_data, image_data,
			       CIMAGE_X, CIMAGE_X, CIMAGE_Y, AIPL_COLOR_RGB888,
			       ml_width, ml_height, true);
	if (aipl_ret != AIPL_ERR_OK) {
		LOG_ERR("Resize failed with error code %d", aipl_ret);
		return -1;
	}

#if CIMAGE_COLOR_CORRECTION
	/* Color correction */
	aipl_ret = aipl_color_correction_rgb(image_data, image_data,
					     ml_width, ml_width, ml_height,
					     AIPL_COLOR_RGB888,
					     camera_get_color_correction_matrix());
	if (aipl_ret != AIPL_ERR_OK) {
		LOG_ERR("Color correction failed with error code %d", aipl_ret);
		return -1;
	}

	/* LUT transform (gamma correction) */
	aipl_ret = aipl_lut_transform_rgb(image_data, image_data,
					  ml_width, ml_width, ml_height,
					  AIPL_COLOR_RGB888, camera_get_gamma_lut());
	if (aipl_ret != AIPL_ERR_OK) {
		LOG_ERR("LUT transform failed with error code %d", aipl_ret);
		return -1;
	}
#endif

	*output_image_data = image_data;

	return 0;
}

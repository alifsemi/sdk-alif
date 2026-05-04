/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include "video_common.h"

LOG_MODULE_DECLARE(video_app);

int fourcc_to_pitch(uint32_t fourcc, uint32_t width)
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

void manual_suite_before(void *fixture)
{
	ARG_UNUSED(fixture);
	uintptr_t regs;

#if ISP_ENABLED
	video = DEVICE_DT_GET_ONE(vsi_isp_pico);
#else
	video = DEVICE_DT_GET_ONE(alif_cam);
#endif
	zassert_true(device_is_ready(video), "%s: device not ready.", video->name);

	regs = DEVICE_MMIO_GET(video);
	if (sys_read32(regs + CAM_CTRL) & CAM_CTRL_BUSY) {
		LOG_ERR("Can't start stream. Already Capturing!");
		video_stream_stop(video);
		k_msleep(20);
	}
}

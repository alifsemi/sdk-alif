/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include "video_common.h"

#include <errno.h>
#include <string.h>

#include <zephyr/drivers/video-controls.h>

/*
 * OV5640-only suite. Compiled only when CONFIG_DT_HAS_OVTI_OV5640_ENABLED
 * (see CMakeLists.txt). Other sensor builds never register these tests.
 *
 * Controls are issued on the sensor node (ov5640) so errno matches the
 * driver (alif,cam maps every set_ctrl failure to -ENOTSUP). Format is
 * programmed through the CAM pipeline so CPI FCFG stays in sync.
 *
 * set_ctrl takes the integer in the pointer bits, not a pointer-to-int:
 *     video_set_ctrl(dev, CID, (void *)value)
 */

static const struct device *ov5640_dev;

static int ov5640_restore_default_fmt(void)
{
	struct video_format fmt = {
		.pixelformat = PIPELINE_FORMAT,
		.width = SENSOR_WIDTH,
		.height = SENSOR_HEIGHT,
		.pitch = fourcc_to_pitch(PIPELINE_FORMAT, SENSOR_WIDTH),
	};

	return video_set_capture_format(video, &fmt);
}

static void ov5640_restore_ctrls(void)
{
	(void)video_set_ctrl(ov5640_dev, VIDEO_CID_HFLIP, (void *)0);
	(void)video_set_ctrl(ov5640_dev, VIDEO_CID_VFLIP, (void *)0);
	(void)video_set_ctrl(ov5640_dev, VIDEO_CID_TEST_PATTERN, (void *)0);
	(void)video_set_ctrl(ov5640_dev, VIDEO_CID_BRIGHTNESS, (void *)0);
	(void)video_set_ctrl(ov5640_dev, VIDEO_CID_CONTRAST, (void *)0);
	(void)video_set_ctrl(ov5640_dev, VIDEO_CID_SATURATION, (void *)64);
	(void)video_set_ctrl(ov5640_dev, VIDEO_CID_GAIN, (void *)0);
	(void)video_set_ctrl(ov5640_dev, VIDEO_CID_POWER_LINE_FREQUENCY,
			     (void *)VIDEO_CID_POWER_LINE_FREQUENCY_AUTO);
}

void *ov5640_suite_setup(void)
{
	ov5640_dev = DEVICE_DT_GET(DT_NODELABEL(ov5640));
	zassert_true(device_is_ready(ov5640_dev),
		"%s: device not ready", ov5640_dev->name);
	return NULL;
}

void ov5640_suite_teardown(void *data)
{
	struct video_buffer *buf = NULL;

	ARG_UNUSED(data);

	video_stream_stop(video);
	k_msleep(20);
	video_flush(video, VIDEO_EP_OUT, true);
	while (video_dequeue(video, VIDEO_EP_OUT, &buf, K_NO_WAIT) == 0) {
		video_buffer_release(buf);
		buf = NULL;
	}

	ov5640_restore_ctrls();
	(void)ov5640_restore_default_fmt();
}

/* -------------------------------------------------------------------------
 * set_format + get_format for every DVP RGB565 mode
 * -------------------------------------------------------------------------
 */
ZTEST(ov5640_testcase, ov5640_set_format_all_dvp_modes)
{
	struct video_caps caps;
	struct video_format fmt = { 0 };
	struct video_format got = { 0 };
	int i = 0;
	int ret;
	int n_modes = 0;

	ret = video_get_caps(video, CAPTURE_EP, &caps);
	zassert_equal(ret, 0, "video_get_caps failed: %d", ret);

	while (caps.format_caps[i].pixelformat) {
		const struct video_format_cap *fcap = &caps.format_caps[i];

		if (fcap->pixelformat != PIPELINE_FORMAT) {
			i++;
			continue;
		}

		fmt.pixelformat = fcap->pixelformat;
		fmt.width = fcap->width_min;
		fmt.height = fcap->height_min;
		fmt.pitch = fourcc_to_pitch(fmt.pixelformat, fmt.width);

		ret = video_set_capture_format(video, &fmt);
		zassert_equal(ret, 0,
			"set_format failed for %ux%u: %d",
			fmt.width, fmt.height, ret);

		memset(&got, 0, sizeof(got));
		ret = video_get_format(video, VIDEO_EP_OUT, &got);
		zassert_equal(ret, 0,
			"get_format failed after %ux%u: %d",
			fmt.width, fmt.height, ret);
		zassert_equal(got.pixelformat, fmt.pixelformat,
			"pixelformat mismatch at %ux%u", fmt.width, fmt.height);
		zassert_equal(got.width, fmt.width,
			"width mismatch: got %u want %u", got.width, fmt.width);
		zassert_equal(got.height, fmt.height,
			"height mismatch: got %u want %u",
			got.height, fmt.height);
		zassert_equal(got.pitch, fmt.pitch,
			"pitch mismatch: got %u want %u", got.pitch, fmt.pitch);

		TC_PRINT("ov5640_set_format_all_dvp_modes: %ux%u pitch=%u OK\n",
			got.width, got.height, got.pitch);
		n_modes++;
		i++;
	}

	zassert_true(n_modes >= 1, "No DVP RGB565 modes found in caps");
	ret = ov5640_restore_default_fmt();
	zassert_equal(ret, 0, "restore default %ux%u failed: %d",
		SENSOR_WIDTH, SENSOR_HEIGHT, ret);
}

/* -------------------------------------------------------------------------
 * HFLIP / VFLIP — leave both off so later capture tests are not mirrored
 * -------------------------------------------------------------------------
 */
ZTEST(ov5640_testcase, ov5640_ctrl_hflip_vflip)
{
	int ret;

	ret = video_set_ctrl(ov5640_dev, VIDEO_CID_HFLIP, (void *)1);
	zassert_equal(ret, 0, "HFLIP on failed: %d", ret);

	ret = video_set_ctrl(ov5640_dev, VIDEO_CID_VFLIP, (void *)1);
	zassert_equal(ret, 0, "VFLIP on failed: %d", ret);

	ret = video_set_ctrl(ov5640_dev, VIDEO_CID_HFLIP, (void *)0);
	zassert_equal(ret, 0, "HFLIP off failed: %d", ret);

	ret = video_set_ctrl(ov5640_dev, VIDEO_CID_VFLIP, (void *)0);
	zassert_equal(ret, 0, "VFLIP off failed: %d", ret);

	TC_PRINT("ov5640_ctrl_hflip_vflip: on/off OK\n");
}

/* -------------------------------------------------------------------------
 * TEST_PATTERN 0..4 accepted, 5 rejected; one color-bar frame at QQVGA
 * -------------------------------------------------------------------------
 */
ZTEST(ov5640_testcase, ov5640_ctrl_test_pattern)
{
	struct video_format fmt = { 0 };
	struct video_buffer *vbuf = NULL;
	struct video_buffer *captured = NULL;
	size_t bsize;
	int ret;
	int pat;

	for (pat = 0; pat <= 4; pat++) {
		ret = video_set_ctrl(ov5640_dev, VIDEO_CID_TEST_PATTERN,
				     (void *)(uintptr_t)pat);
		zassert_equal(ret, 0, "TEST_PATTERN %d failed: %d", pat, ret);
	}

	ret = video_set_ctrl(ov5640_dev, VIDEO_CID_TEST_PATTERN, (void *)5);
	zassert_equal(ret, -EINVAL,
		"TEST_PATTERN 5 expected -EINVAL, got %d", ret);

	ret = ov5640_restore_default_fmt();
	zassert_equal(ret, 0, "restore default fmt failed: %d", ret);

	ret = video_set_ctrl(ov5640_dev, VIDEO_CID_TEST_PATTERN, (void *)1);
	zassert_equal(ret, 0, "TEST_PATTERN color-bar failed: %d", ret);

	fmt.pixelformat = PIPELINE_FORMAT;
	fmt.width = SENSOR_WIDTH;
	fmt.height = SENSOR_HEIGHT;
	fmt.pitch = fourcc_to_pitch(fmt.pixelformat, fmt.width);
	bsize = fmt.pitch * fmt.height;

	vbuf = video_buffer_aligned_alloc(bsize, CONFIG_VIDEO_BUFFER_POOL_ALIGN,
					  K_NO_WAIT);
	zassert_not_null(vbuf, "alloc failed for test-pattern frame");
	memset(vbuf->buffer, 0, bsize);

	ret = video_enqueue(video, VIDEO_EP_OUT, vbuf);
	zassert_equal(ret, 0, "enqueue failed: %d", ret);

	ret = video_stream_start(video);
	zassert_equal(ret, 0, "stream_start failed: %d", ret);

	ret = video_dequeue(video, VIDEO_EP_OUT, &captured, K_MSEC(2000));
	zassert_equal(ret, 0, "dequeue test-pattern frame failed: %d", ret);
	zassert_not_null(captured, "captured buffer is NULL");
	zassert_equal(captured->bytesused, bsize, "bytesused %u != bsize %u",
		captured->bytesused, (uint32_t)bsize);
	TC_PRINT("ov5640_ctrl_test_pattern: color-bar frame bytesused=%u\n",
		captured->bytesused);

	ret = video_stream_stop(video);
	zassert_equal(ret, 0, "stream_stop failed: %d", ret);
	k_msleep(20);

	video_flush(video, VIDEO_EP_OUT, true);
	if (captured) {
		video_buffer_release(captured);
		captured = NULL;
	}
	while (video_dequeue(video, VIDEO_EP_OUT, &captured, K_NO_WAIT) == 0) {
		video_buffer_release(captured);
		captured = NULL;
	}

	ret = video_set_ctrl(ov5640_dev, VIDEO_CID_TEST_PATTERN, (void *)0);
	zassert_equal(ret, 0, "TEST_PATTERN off failed: %d", ret);
}

/* -------------------------------------------------------------------------
 * Out-of-range / unknown CIDs — issued on the sensor so errno is real
 * -------------------------------------------------------------------------
 */
ZTEST(ov5640_testcase, ov5640_ctrl_range_reject)
{
	int ret;

	ret = video_set_ctrl(ov5640_dev, VIDEO_CID_HUE, (void *)361);
	zassert_equal(ret, -EINVAL, "HUE 361 expected -EINVAL, got %d", ret);

	ret = video_set_ctrl(ov5640_dev, VIDEO_CID_SATURATION, (void *)256);
	zassert_equal(ret, -EINVAL,
		"SATURATION 256 expected -EINVAL, got %d", ret);

	ret = video_set_ctrl(ov5640_dev, VIDEO_CID_BRIGHTNESS, (void *)256);
	zassert_equal(ret, -EINVAL,
		"BRIGHTNESS 256 expected -EINVAL, got %d", ret);

	ret = video_set_ctrl(ov5640_dev, VIDEO_CID_EXPOSURE, (void *)1);
	zassert_equal(ret, -ENOTSUP,
		"unknown CID expected -ENOTSUP, got %d", ret);

	ret = video_set_ctrl(ov5640_dev, VIDEO_CID_POWER_LINE_FREQUENCY,
			     (void *)VIDEO_CID_POWER_LINE_FREQUENCY_DISABLED);
	zassert_equal(ret, -EINVAL,
		"POWER_LINE DISABLED expected -EINVAL, got %d", ret);

	ret = video_set_ctrl(ov5640_dev, VIDEO_CID_POWER_LINE_FREQUENCY,
			(void *)99);
	zassert_equal(ret, -EINVAL,
		"POWER_LINE 99 expected -EINVAL, got %d", ret);

	TC_PRINT("ov5640_ctrl_range_reject: OK\n");
}

/* -------------------------------------------------------------------------
 * In-range image-adjust CIDs (I2C happy path). Hue is skipped: it uses
 * libm/double. Restore AEC auto (gain 0) and AUTO flicker at the end.
 * -------------------------------------------------------------------------
 */
ZTEST(ov5640_testcase, ov5640_ctrl_image_adjust)
{
	int ret;

	ret = video_set_ctrl(ov5640_dev, VIDEO_CID_BRIGHTNESS, (void *)16);
	zassert_equal(ret, 0, "BRIGHTNESS 16 failed: %d", ret);

	ret = video_set_ctrl(ov5640_dev, VIDEO_CID_CONTRAST, (void *)16);
	zassert_equal(ret, 0, "CONTRAST 16 failed: %d", ret);

	ret = video_set_ctrl(ov5640_dev, VIDEO_CID_SATURATION, (void *)64);
	zassert_equal(ret, 0, "SATURATION 64 failed: %d", ret);

	ret = video_set_ctrl(ov5640_dev, VIDEO_CID_GAIN, (void *)0);
	zassert_equal(ret, 0, "GAIN 0 (AEC auto) failed: %d", ret);

	ret = video_set_ctrl(ov5640_dev, VIDEO_CID_POWER_LINE_FREQUENCY,
			     (void *)VIDEO_CID_POWER_LINE_FREQUENCY_50HZ);
	zassert_equal(ret, 0, "POWER_LINE 50Hz failed: %d", ret);

	ret = video_set_ctrl(ov5640_dev, VIDEO_CID_POWER_LINE_FREQUENCY,
			     (void *)VIDEO_CID_POWER_LINE_FREQUENCY_60HZ);
	zassert_equal(ret, 0, "POWER_LINE 60Hz failed: %d", ret);

	ret = video_set_ctrl(ov5640_dev, VIDEO_CID_POWER_LINE_FREQUENCY,
			     (void *)VIDEO_CID_POWER_LINE_FREQUENCY_AUTO);
	zassert_equal(ret, 0, "POWER_LINE AUTO failed: %d", ret);

	ov5640_restore_ctrls();
	TC_PRINT("ov5640_ctrl_image_adjust: OK\n");
}

/* -------------------------------------------------------------------------
 * DVP does not advertise YUYV or 640x480 — sensor must reject them.
 * Issued on ov5640 so a failed set_fmt cannot pollute alif,cam's cached fmt.
 * -------------------------------------------------------------------------
 */
ZTEST(ov5640_testcase, ov5640_set_format_unsupported)
{
	struct video_format fmt = {
		.pixelformat = VIDEO_PIX_FMT_YUYV,
		.width = SENSOR_WIDTH,
		.height = SENSOR_HEIGHT,
		.pitch = SENSOR_WIDTH * 2,
	};
	int ret;

	ret = video_set_format(ov5640_dev, VIDEO_EP_OUT, &fmt);
	zassert_equal(ret, -ENOTSUP,
		"YUYV on DVP expected -ENOTSUP, got %d", ret);

	fmt.pixelformat = VIDEO_PIX_FMT_RGB565;
	fmt.width = 640;
	fmt.height = 480;
	fmt.pitch = 1280;
	ret = video_set_format(ov5640_dev, VIDEO_EP_OUT, &fmt);
	zassert_equal(ret, -ENOTSUP,
		"640x480 RGB565 on DVP expected -ENOTSUP, got %d", ret);

	TC_PRINT("ov5640_set_format_unsupported: YUYV and 640x480 rejected\n");
}

/* -------------------------------------------------------------------------
 * Frame-interval API is CSI-2 only; DVP returns -ENOTSUP on the sensor.
 * alif,cam does not implement frmival, so the CAM node returns -ENOSYS.
 * -------------------------------------------------------------------------
 */
ZTEST(ov5640_testcase, ov5640_frmival_dvp_enotsup)
{
	struct video_frmival ival = {
		.numerator = 1,
		.denominator = 30,
	};
	struct video_format fmt = {
		.pixelformat = PIPELINE_FORMAT,
		.width = SENSOR_WIDTH,
		.height = SENSOR_HEIGHT,
		.pitch = fourcc_to_pitch(PIPELINE_FORMAT, SENSOR_WIDTH),
	};
	struct video_frmival_enum fie = {
		.index = 0,
		.format = &fmt,
	};
	int ret;

	ret = video_get_frmival(ov5640_dev, VIDEO_EP_OUT, &ival);
	zassert_equal(ret, -ENOTSUP,
		"get_frmival on DVP OV5640 expected -ENOTSUP, got %d", ret);

	ret = video_set_frmival(ov5640_dev, VIDEO_EP_OUT, &ival);
	zassert_equal(ret, -ENOTSUP,
		"set_frmival on DVP OV5640 expected -ENOTSUP, got %d", ret);

	ret = video_enum_frmival(ov5640_dev, VIDEO_EP_OUT, &fie);
	zassert_equal(ret, -ENOTSUP,
		"enum_frmival on DVP OV5640 expected -ENOTSUP, got %d", ret);

	ret = video_get_frmival(video, VIDEO_EP_OUT, &ival);
	zassert_equal(ret, -ENOTSUP,
		"get_frmival on alif,cam expected -ENOTSUP, got %d", ret);

	TC_PRINT("ov5640_frmival_dvp_enotsup: sensor -ENOTSUP, CAM -ENOSYS\n");
}

ZTEST_SUITE(ov5640_testcase, NULL, ov5640_suite_setup, manual_suite_before,
	    NULL, ov5640_suite_teardown);

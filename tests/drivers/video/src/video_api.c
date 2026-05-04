/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include "video_common.h"
#include <string.h>

/* =========================================================================
 * API Test Suite — cpi_api_testcase
 *
 * Each test targets a single video API function in isolation.
 * All tests are self-contained: they acquire and release their own
 * buffers and leave the device in a stopped, unflushed-free state.
 *
 * Suite setup acquires the device handle once. No per-test setup/teardown
 * is needed because every test cleans up after itself.
 *
 * =========================================================================
 */

static const struct device *api_video;

/*
 * Probe VIDEO_EP_IN and VIDEO_EP_OUT at runtime to find an endpoint that the
 * driver does not support (video_get_caps returns -EINVAL). This keeps the
 * "invalid endpoint" tests portable across direct-CAM (output-only) and
 * ISP (both EPs valid) builds — no compile-time #if needed.
 *
 * Returns VIDEO_EP_NONE if every endpoint is valid (caller should skip).
 */
static enum video_endpoint_id api_find_unsupported_ep(void)
{
	struct video_caps caps;

	if (video_get_caps(api_video, VIDEO_EP_IN, &caps) == -EINVAL) {
		return VIDEO_EP_IN;
	}
	if (video_get_caps(api_video, VIDEO_EP_OUT, &caps) == -EINVAL) {
		return VIDEO_EP_OUT;
	}
	return VIDEO_EP_NONE;
}

/*
 * Query caps and fill fmt with the minimum supported size for PIPELINE_FORMAT.
 * Only PIPELINE_FORMAT is used — it is the format the sensor set_format accepts.
 * Returns 0 on success, -ENOENT if PIPELINE_FORMAT is not found in caps.
 */
static int api_get_default_fmt(struct video_format *fmt)
{
	struct video_caps caps;
	int i = 0;
	int ret;

	ret = video_get_caps(api_video, CAPTURE_EP, &caps);
	if (ret) {
		return ret;
	}

	if (!caps.format_caps || caps.format_caps[0].pixelformat == 0) {
		return -ENOENT;
	}

	/* Search caps for PIPELINE_FORMAT — do not fall back to other formats. */
	while (caps.format_caps[i].pixelformat) {
		if (caps.format_caps[i].pixelformat == PIPELINE_FORMAT) {
			fmt->pixelformat = PIPELINE_FORMAT;
			fmt->width       = caps.format_caps[i].width_min;
			fmt->height      = caps.format_caps[i].height_min;
			fmt->pitch       = fourcc_to_pitch(fmt->pixelformat, fmt->width);
			return 0;
		}
		i++;
	}

	return -ENOENT;
}

void *api_suite_setup(void)
{
#if ISP_ENABLED
	api_video = DEVICE_DT_GET_ONE(vsi_isp_pico);
#else
	api_video = DEVICE_DT_GET_ONE(alif_cam);
#endif
	zassert_true(device_is_ready(api_video),
		"%s: device not ready", api_video->name);
	return NULL;
}

void api_suite_teardown(void *data)
{
	ARG_UNUSED(data);
	/* Guarantee the stream is stopped and all buffers returned to the pool
	 * before cpi_manual_testcase starts. Idempotent if already stopped.
	 * Sleep after stop to allow CAM_CTRL_BUSY hardware bit to de-assert.
	 */
	video_stream_stop(api_video);
	k_msleep(100);
	video_flush(api_video, VIDEO_EP_OUT, true);

	struct video_buffer *buf = NULL;

	while (video_dequeue(api_video, VIDEO_EP_OUT, &buf, K_NO_WAIT) == 0) {
		video_buffer_release(buf);
		buf = NULL;
	}
}

/* -------------------------------------------------------------------------
 * video_get_caps — valid endpoint returns at least one format
 * -------------------------------------------------------------------------
 */
ZTEST(cpi_api_testcase, api_get_caps_valid_ep)
{
	struct video_caps caps;
	int ret;

	ret = video_get_caps(api_video, VIDEO_EP_OUT, &caps);
	zassert_equal(ret, 0, "video_get_caps(EP_OUT) failed: %d", ret);
	zassert_not_null(caps.format_caps,
		"format_caps pointer is NULL");
	zassert_not_equal(caps.format_caps[0].pixelformat, 0,
		"No format caps returned — format_caps[0].pixelformat is 0");
	TC_PRINT("api_get_caps_valid_ep: first fmt=0x%08x w=[%u..%u] h=[%u..%u]\n",
		caps.format_caps[0].pixelformat,
		caps.format_caps[0].width_min, caps.format_caps[0].width_max,
		caps.format_caps[0].height_min, caps.format_caps[0].height_max);
}

/* -------------------------------------------------------------------------
 * video_get_caps — passing an endpoint the driver does not support must
 * return -EINVAL. The "unsupported" endpoint is discovered at runtime via
 * api_find_unsupported_ep() so this test works for both direct-CAM
 * (output-only) and ISP (both EPs valid) builds.
 * -------------------------------------------------------------------------
 */
ZTEST(cpi_api_testcase, api_get_caps_invalid_ep)
{
	enum video_endpoint_id bad_ep = api_find_unsupported_ep();
	struct video_caps caps;
	int ret;

	if (bad_ep == VIDEO_EP_NONE) {
		TC_PRINT("api_get_caps_invalid_ep: all endpoints valid, skipping\n");
		ztest_test_skip();
		return;
	}

	ret = video_get_caps(api_video, bad_ep, &caps);
	TC_PRINT("api_get_caps_invalid_ep: ep=%d ret=%d (expected -EINVAL=%d)\n",
		bad_ep, ret, -EINVAL);
	zassert_equal(ret, -EINVAL,
		"Expected -EINVAL for unsupported EP %d, got %d", bad_ep, ret);
}

/* -------------------------------------------------------------------------
 * video_set_format — valid format is accepted
 * -------------------------------------------------------------------------
 */
ZTEST(cpi_api_testcase, api_set_format_valid)
{
	struct video_format fmt = { 0 };
	int ret;

	ret = api_get_default_fmt(&fmt);
	zassert_equal(ret, 0, "api_get_default_fmt failed: %d", ret);

	ret = video_set_format(api_video, CAPTURE_EP, &fmt);
	zassert_equal(ret, 0, "video_set_format on CAPTURE_EP with valid fmt failed: %d", ret);
	TC_PRINT("api_set_format_valid: fmt=%c%c%c%c %ux%u pitch=%u ret=%d\n",
		(char)fmt.pixelformat, (char)(fmt.pixelformat >> 8),
		(char)(fmt.pixelformat >> 16), (char)(fmt.pixelformat >> 24),
		fmt.width, fmt.height, fmt.pitch, ret);

#if ISP_ENABLED
	/* ISP also requires an output-side format (OUTPUT_FORMAT on EP_OUT). */
	struct video_format out_fmt = {
		.pixelformat = OUTPUT_FORMAT,
		.width       = 480,
		.height      = 480,
		.pitch       = fourcc_to_pitch(OUTPUT_FORMAT, 480),
	};
	ret = video_set_format(api_video, VIDEO_EP_OUT, &out_fmt);
	zassert_equal(ret, 0, "ISP EP_OUT set_format failed: %d", ret);
#endif
}

/* -------------------------------------------------------------------------
 * video_set_format — invalid pixelformat must be rejected
 * -------------------------------------------------------------------------
 */
ZTEST(cpi_api_testcase, api_set_format_invalid_pixelformat)
{
	struct video_format fmt = {
		.pixelformat = 0xDEADBEEF,
		.width       = 560,
		.height      = 560,
		.pitch       = 560,
	};
	uintptr_t regs = DEVICE_MMIO_GET(api_video);
	int ret;

	ret = video_set_format(api_video, VIDEO_EP_OUT, &fmt);
	TC_PRINT("api_set_format_invalid_pixelformat: ret=%d\n", ret);
	zassert_true(ret == -EINVAL || ret == -ENOTSUP,
		"Expected -EINVAL or -ENOTSUP for bad pixelformat, got %d", ret);
	zassert_false(sys_read32(regs + CAM_CTRL) & CAM_CTRL_BUSY,
		"Can't set format. Already Capturing!");
}

/* -------------------------------------------------------------------------
 * video_set_format — passing an endpoint the driver does not support must
 * be rejected with -EINVAL. Unsupported EP is discovered at runtime so the
 * test is portable across direct-CAM and ISP builds.
 * -------------------------------------------------------------------------
 */
ZTEST(cpi_api_testcase, api_set_format_invalid_ep)
{
	enum video_endpoint_id bad_ep = api_find_unsupported_ep();
	struct video_format fmt = { 0 };
	int ret;

	if (bad_ep == VIDEO_EP_NONE) {
		TC_PRINT("api_set_format_invalid_ep: all endpoints valid, skipping\n");
		ztest_test_skip();
		return;
	}

	ret = api_get_default_fmt(&fmt);
	zassert_equal(ret, 0, "api_get_default_fmt failed: %d", ret);

	ret = video_set_format(api_video, bad_ep, &fmt);
	TC_PRINT("api_set_format_invalid_ep: ep=%d ret=%d (expected -EINVAL=%d)\n",
		bad_ep, ret, -EINVAL);
	zassert_equal(ret, -EINVAL,
		"Expected -EINVAL for unsupported EP %d, got %d", bad_ep, ret);
}

/* -------------------------------------------------------------------------
 * video_get_format — returns the format previously set
 * -------------------------------------------------------------------------
 */
ZTEST(cpi_api_testcase, api_get_format_after_set)
{
	struct video_format set_fmt = { 0 };
	struct video_format get_fmt = { 0 };
	int ret;

	ret = api_get_default_fmt(&set_fmt);
	zassert_equal(ret, 0, "api_get_default_fmt failed: %d", ret);

	ret = video_set_format(api_video, CAPTURE_EP, &set_fmt);
	zassert_equal(ret, 0, "set_format failed: %d", ret);

	ret = video_get_format(api_video, CAPTURE_EP, &get_fmt);
	zassert_equal(ret, 0, "get_format failed: %d", ret);

	zassert_equal(get_fmt.pixelformat, set_fmt.pixelformat,
		"pixelformat mismatch: got 0x%08x, expected 0x%08x",
		get_fmt.pixelformat, set_fmt.pixelformat);
	zassert_equal(get_fmt.width, set_fmt.width,
		"width mismatch: got %u, expected %u",
		get_fmt.width, set_fmt.width);
	zassert_equal(get_fmt.height, set_fmt.height,
		"height mismatch: got %u, expected %u",
		get_fmt.height, set_fmt.height);
	TC_PRINT("api_get_format_after_set: OK fmt=%c%c%c%c %ux%u\n",
		(char)get_fmt.pixelformat, (char)(get_fmt.pixelformat >> 8),
		(char)(get_fmt.pixelformat >> 16), (char)(get_fmt.pixelformat >> 24),
		get_fmt.width, get_fmt.height);
}

/* -------------------------------------------------------------------------
 * video_buffer_alloc / video_buffer_release — basic lifecycle
 * -------------------------------------------------------------------------
 */
ZTEST(cpi_api_testcase, api_buffer_alloc_release)
{
	struct video_format fmt = { 0 };
	struct video_buffer *vbuf;
	size_t bsize;
	int ret;

	ret = api_get_default_fmt(&fmt);
	zassert_equal(ret, 0, "api_get_default_fmt failed: %d", ret);
	bsize = fmt.pitch * fmt.height;

	vbuf = video_buffer_alloc(bsize, K_NO_WAIT);
	zassert_not_null(vbuf, "video_buffer_alloc returned NULL");
	zassert_not_null(vbuf->buffer, "vbuf->buffer pointer is NULL");
	zassert_true(vbuf->size >= bsize,
		"vbuf->size %u < requested bsize %u", vbuf->size, (uint32_t)bsize);
	TC_PRINT("api_buffer_alloc_release: addr=0x%08x size=%u\n",
		(uint32_t)vbuf->buffer, vbuf->size);

	video_buffer_release(vbuf);
}

/* -------------------------------------------------------------------------
 * video_enqueue — valid aligned buffer is accepted
 * -------------------------------------------------------------------------
 */
ZTEST(cpi_api_testcase, api_enqueue_valid)
{
	struct video_format fmt = { 0 };
	struct video_buffer *vbuf;
	size_t bsize;
	int ret;

	ret = api_get_default_fmt(&fmt);
	zassert_equal(ret, 0, "api_get_default_fmt failed: %d", ret);
	bsize = fmt.pitch * fmt.height;

	ret = video_set_format(api_video, VIDEO_EP_OUT, &fmt);
	zassert_equal(ret, 0, "set_format failed: %d", ret);

	vbuf = video_buffer_alloc(bsize, K_NO_WAIT);
	zassert_not_null(vbuf, "video_buffer_alloc returned NULL");

	ret = video_enqueue(api_video, VIDEO_EP_OUT, vbuf);
	TC_PRINT("api_enqueue_valid: ret=%d\n", ret);
	zassert_equal(ret, 0, "video_enqueue failed: %d", ret);

	/*
	 * flush(cancel=true) moves the buffer from fifo_in to fifo_out inside
	 * the driver. We must dequeue it from fifo_out before releasing it,
	 * otherwise the driver retains a dangling pointer to freed memory.
	 */
	video_flush(api_video, VIDEO_EP_OUT, true);

	struct video_buffer *drained = NULL;

	ret = video_dequeue(api_video, VIDEO_EP_OUT, &drained, K_NO_WAIT);
	TC_PRINT("api_enqueue_valid: post-flush dequeue ret=%d\n", ret);
	zassert_equal(ret, 0,
		"Buffer not in fifo_out after cancel-flush: %d", ret);

	video_buffer_release(drained);
}

/* -------------------------------------------------------------------------
 * video_enqueue — enqueuing on an endpoint the driver does not support must
 * return -EINVAL. Unsupported EP discovered at runtime so the test works
 * for both direct-CAM and ISP builds.
 * -------------------------------------------------------------------------
 */
ZTEST(cpi_api_testcase, api_enqueue_invalid_ep)
{
	enum video_endpoint_id bad_ep = api_find_unsupported_ep();
	struct video_format fmt = { 0 };
	struct video_buffer *vbuf;
	size_t bsize;
	int ret;

	if (bad_ep == VIDEO_EP_NONE) {
		TC_PRINT("api_enqueue_invalid_ep: all endpoints valid, skipping\n");
		ztest_test_skip();
		return;
	}

	ret = api_get_default_fmt(&fmt);
	zassert_equal(ret, 0, "api_get_default_fmt failed: %d", ret);
	bsize = fmt.pitch * fmt.height;

	vbuf = video_buffer_alloc(bsize, K_NO_WAIT);
	zassert_not_null(vbuf, "video_buffer_alloc returned NULL");

	ret = video_enqueue(api_video, bad_ep, vbuf);
	TC_PRINT("api_enqueue_invalid_ep: ep=%d ret=%d (expected -EINVAL=%d)\n",
		bad_ep, ret, -EINVAL);
	zassert_equal(ret, -EINVAL,
		"Expected -EINVAL for unsupported EP %d enqueue, got %d", bad_ep, ret);

	video_buffer_release(vbuf);
}

/* -------------------------------------------------------------------------
 * video_dequeue — timeout when no buffer is ready returns
 *              -EAGAIN (K_NO_WAIT) or blocks briefly
 * -------------------------------------------------------------------------
 */
ZTEST(cpi_api_testcase, api_dequeue_timeout)
{
	struct video_buffer *vbuf = NULL;
	int ret;

	/* No buffers enqueued, no stream running — must time out immediately. */
	ret = video_dequeue(api_video, VIDEO_EP_OUT, &vbuf, K_NO_WAIT);
	TC_PRINT("api_dequeue_timeout: ret=%d (expected -EAGAIN=%d)\n",
		ret, -EAGAIN);
	zassert_equal(ret, -EAGAIN,
		"Expected -EAGAIN on empty dequeue with K_NO_WAIT, got %d", ret);
	zassert_is_null(vbuf, "vbuf should be NULL on failed dequeue");
}

/* -------------------------------------------------------------------------
 * video_stream_start — without buffers must fail gracefully
 * -------------------------------------------------------------------------
 */
ZTEST(cpi_api_testcase, api_stream_start_no_buffers)
{
	int ret;

	ret = video_stream_start(api_video);
	TC_PRINT("api_stream_start_no_buffers: ret=%d\n", ret);
	/* Driver should refuse or at least not crash — -ENOBUFS or -EIO */
	zassert_true(ret != 0,
		"stream_start with no buffers unexpectedly returned 0");

	/* Ensure device is stopped regardless of above result. */
	ret = video_stream_stop(api_video);
	TC_PRINT("api_stream_start_no_buffers: stop ret=%d\n", ret);
	zassert_true(ret == 0 || ret == -EALREADY,
		"stream_stop after failed start returned %d (expected 0 or -EALREADY)", ret);
}

/* -------------------------------------------------------------------------
 * video_stream_stop — calling stop when already stopped must
 *              return 0 (idempotent)
 * -------------------------------------------------------------------------
 */
ZTEST(cpi_api_testcase, api_stream_stop_idempotent)
{
	int ret;

	/* First stop — device is already stopped at test entry. */
	ret = video_stream_stop(api_video);
	TC_PRINT("api_stream_stop_idempotent: first stop ret=%d\n", ret);
	zassert_true(ret == 0 || ret == -EALREADY,
		"First stop on already-stopped device returned %d (expected 0 or -EALREADY)",
		ret);

	/* Second stop — must still be benign. */
	ret = video_stream_stop(api_video);
	TC_PRINT("api_stream_stop_idempotent: second stop ret=%d\n", ret);
	zassert_true(ret == 0 || ret == -EALREADY,
		"Second stop returned %d (expected 0 or -EALREADY)", ret);
}

/* -------------------------------------------------------------------------
 * video_flush with cancel=true drains all queued buffers
 * -------------------------------------------------------------------------
 */
ZTEST(cpi_api_testcase, api_flush_cancel)
{
	struct video_format fmt = { 0 };
	struct video_buffer *bufs[2];
	struct video_buffer *vbuf;
	size_t bsize;
	int i, ret;

	ret = api_get_default_fmt(&fmt);
	zassert_equal(ret, 0, "api_get_default_fmt failed: %d", ret);
	bsize = fmt.pitch * fmt.height;

	ret = video_set_format(api_video, VIDEO_EP_OUT, &fmt);
	zassert_equal(ret, 0, "set_format failed: %d", ret);

	for (i = 0; i < 2; i++) {
		bufs[i] = video_buffer_alloc(bsize, K_NO_WAIT);
		zassert_not_null(bufs[i], "alloc failed for buf %d", i);
		ret = video_enqueue(api_video, VIDEO_EP_OUT, bufs[i]);
		zassert_equal(ret, 0, "enqueue failed for buf %d: %d", i, ret);
	}

	/* Cancel flush — all buffers must move to OUT-FIFO. */
	ret = video_flush(api_video, VIDEO_EP_OUT, true);
	zassert_equal(ret, 0, "video_flush(cancel=true) failed: %d", ret);
	TC_PRINT("api_flush_cancel: flush ret=%d\n", ret);

	/* Dequeue both — they must be available immediately. */
	for (i = 0; i < 2; i++) {
		ret = video_dequeue(api_video, VIDEO_EP_OUT, &vbuf, K_NO_WAIT);
		zassert_equal(ret, 0,
			"dequeue after cancel-flush failed at buf %d: %d", i, ret);
		video_buffer_release(vbuf);
	}
}

/* -------------------------------------------------------------------------
 * video_flush with cancel=false (non-cancel) when stream is
 *              stopped — buffers must still be drained to OUT-FIFO
 * -------------------------------------------------------------------------
 */
ZTEST(cpi_api_testcase, api_flush_no_cancel_stopped)
{
	struct video_format fmt = { 0 };
	struct video_buffer *vbuf;
	size_t bsize;
	int ret;

	ret = api_get_default_fmt(&fmt);
	zassert_equal(ret, 0, "api_get_default_fmt failed: %d", ret);
	bsize = fmt.pitch * fmt.height;

	ret = video_set_format(api_video, VIDEO_EP_OUT, &fmt);
	zassert_equal(ret, 0, "set_format failed: %d", ret);

	vbuf = video_buffer_alloc(bsize, K_NO_WAIT);
	zassert_not_null(vbuf, "video_buffer_alloc returned NULL");

	ret = video_enqueue(api_video, VIDEO_EP_OUT, vbuf);
	zassert_equal(ret, 0, "enqueue failed: %d", ret);

	/*
	 * Stream is stopped (curr_vid_buf == 0), so non-cancel flush
	 * must drain fifo_in to fifo_out immediately per driver logic.
	 */
	ret = video_flush(api_video, VIDEO_EP_OUT, false);
	TC_PRINT("api_flush_no_cancel_stopped: flush ret=%d\n", ret);
	zassert_equal(ret, 0, "video_flush(cancel=false) failed: %d", ret);

	/* Buffer must now be dequeue-able. */
	struct video_buffer *out = NULL;

	ret = video_dequeue(api_video, VIDEO_EP_OUT, &out, K_NO_WAIT);
	zassert_equal(ret, 0,
		"Buffer not available after non-cancel flush: %d", ret);
	video_buffer_release(out);
}

/* -------------------------------------------------------------------------
 * video_set_signal — set and clear signal without error
 * -------------------------------------------------------------------------
 */
#ifdef CONFIG_POLL
ZTEST(cpi_api_testcase, api_set_signal)
{
	struct k_poll_signal sig;
	int ret;

	k_poll_signal_init(&sig);

	ret = video_set_signal(api_video, VIDEO_EP_OUT, &sig);
	TC_PRINT("api_set_signal: set ret=%d\n", ret);
	zassert_equal(ret, 0, "video_set_signal failed: %d", ret);

	/* Setting a second signal when one is active must return -EALREADY. */
	ret = video_set_signal(api_video, VIDEO_EP_OUT, &sig);
	TC_PRINT("api_set_signal: duplicate set ret=%d (expected -EALREADY=%d)\n",
		ret, -EALREADY);
	zassert_equal(ret, -EALREADY,
		"Expected -EALREADY on duplicate set_signal, got %d", ret);

	/* Clear the signal by passing NULL. */
	ret = video_set_signal(api_video, VIDEO_EP_OUT, NULL);
	TC_PRINT("api_set_signal: clear ret=%d\n", ret);
	zassert_equal(ret, 0, "Clearing signal with NULL failed: %d", ret);
}
#endif /* CONFIG_POLL */

/* -------------------------------------------------------------------------
 * video_stream_start + stop — round-trip with buffers enqueued
 * -------------------------------------------------------------------------
 */
ZTEST(cpi_api_testcase, api_stream_start_stop_roundtrip)
{
	struct video_format fmt = { 0 };
	struct video_buffer *vbuf;
	size_t bsize;
	int ret;

	ret = api_get_default_fmt(&fmt);
	zassert_equal(ret, 0, "api_get_default_fmt failed: %d", ret);
	bsize = fmt.pitch * fmt.height;

	ret = video_set_format(api_video, VIDEO_EP_OUT, &fmt);
	zassert_equal(ret, 0, "set_format failed: %d", ret);

	vbuf = video_buffer_alloc(bsize, K_NO_WAIT);
	zassert_not_null(vbuf, "video_buffer_alloc returned NULL");
	memset(vbuf->buffer, 0, bsize);

	ret = video_enqueue(api_video, VIDEO_EP_OUT, vbuf);
	zassert_equal(ret, 0, "enqueue failed: %d", ret);

	ret = video_stream_start(api_video);
	zassert_equal(ret, 0, "stream_start failed: %d", ret);
	TC_PRINT("api_stream_start_stop_roundtrip: stream started\n");

	/*
	 * Bounded wait for one frame (up to 500 ms). The test is about the
	 * start/stop round-trip, not successful capture, so a dequeue
	 * timeout is acceptable here.
	 */
	struct video_buffer *captured = NULL;

	ret = video_dequeue(api_video, VIDEO_EP_OUT, &captured, K_MSEC(500));
	TC_PRINT("api_stream_start_stop_roundtrip: frame dequeue ret=%d\n", ret);
	if (ret == 0 && captured) {
		video_enqueue(api_video, VIDEO_EP_OUT, captured);
	}

	ret = video_stream_stop(api_video);
	zassert_equal(ret, 0, "stream_stop failed: %d", ret);
	TC_PRINT("api_stream_start_stop_roundtrip: stream stopped\n");

	/* Drain any remaining buffers before release. */
	video_flush(api_video, VIDEO_EP_OUT, true);

	struct video_buffer *out = NULL;

	while (video_dequeue(api_video, VIDEO_EP_OUT, &out, K_NO_WAIT) == 0) {
		video_buffer_release(out);
		out = NULL;
	}
	/* vbuf ownership transferred to driver; released above via dequeue loop. */
}

/*
 * Regression test: video_get_format called BEFORE video_set_format must return
 * a valid (non-zero) default format on ARX3A0.
 *
 * Known bug: arx3a0_init() does not initialize data->fmt. So get_format
 * called before set_format returns pixelformat=0, width=0, height=0.
 *
 * ORDERING NOTE: Zephyr runs tests in alphabetical order by test name.
 * "api_a_get_format_before_set_format" runs before all other "api_*" tests
 * — so no set_format has been called yet when this test executes.
 * DO NOT rename this test to lose the "api_a_" prefix, as that would let
 * another test run first and populate data->fmt, causing a buggy driver
 * to pass this test incorrectly.
 *
 */
ZTEST(cpi_api_testcase, api_a_get_format_before_set_format)
{
	struct video_format fmt = { 0 };
	const struct device *dev;
	int ret;

	dev = DEVICE_DT_GET_ONE(alif_cam);
	zassert_true(device_is_ready(dev), "%s: device not ready.", dev->name);

	/* Call get_format WITHOUT any prior set_format call.
	 * On a buggy driver (arx3a0_init does not init data->fmt),
	 * fmt.pixelformat will be 0 and the assertions below will fail.
	 */
	ret = video_get_format(dev, VIDEO_EP_OUT, &fmt);
	zassert_equal(ret, 0,
		"video_get_format before set_format returned error: %d", ret);

	TC_PRINT("get_format before set_format: fmt=%c%c%c%c %ux%u pitch=%u\n",
		(char)fmt.pixelformat, (char)(fmt.pixelformat >> 8),
		(char)(fmt.pixelformat >> 16), (char)(fmt.pixelformat >> 24),
		fmt.width, fmt.height, fmt.pitch);

	zassert_not_equal(fmt.pixelformat, 0,
		"BUG: pixelformat=0 — arx3a0_init does not set default fmt");
	zassert_not_equal(fmt.width, 0,
		"BUG: width=0 — arx3a0_init does not set default fmt");
	zassert_not_equal(fmt.height, 0,
		"BUG: height=0 — arx3a0_init does not set default fmt");
}

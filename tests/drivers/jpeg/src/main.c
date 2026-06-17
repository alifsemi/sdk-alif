/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <string.h>

#include "jpeg_common.h"

LOG_MODULE_REGISTER(jpeg_test, LOG_LEVEL_INF);

/*
 * Test suite for the VeriSilicon Hantro VC9000E JPEG hardware encoder
 * driver (compatible: verisilicon,hantro-vc9000e-jpeg).
 *
 * Each test is independent. A single buffer pair (input/output) is
 * allocated/released per test to avoid cross-test state leaks.
 */

/* ------------------------------------------------------------------ */
/* Helpers                                                             */
/* ------------------------------------------------------------------ */

/* Acquire one input + one output buffer; copy reference image to input.
 * On any allocation failure, every successfully-allocated buffer is
 * released before the zassert fires so a single failure does not leak
 * pool slots and cascade into later tests.
 */
void alloc_ref_buffers(struct video_buffer **in, struct video_buffer **out)
{
	*in  = video_buffer_alloc(REF_IMG_NV12_SIZE, K_NO_WAIT);
	*out = video_buffer_alloc(DEFAULT_OUT_BUF_SIZE, K_NO_WAIT);

	if (*in == NULL || *out == NULL) {
		if (*in) {
			video_buffer_release(*in);
			*in = NULL;
		}
		if (*out) {
			video_buffer_release(*out);
			*out = NULL;
		}
	}

	zassert_not_null(*in,  "Failed to alloc input buffer");
	zassert_not_null(*out, "Failed to alloc output buffer");

	jpeg_copy_ref_image((uint8_t *)(*in)->buffer);
	(*out)->bytesused = (*out)->size;
}

void release_buffers(struct video_buffer *in, struct video_buffer *out)
{
	if (in) {
		video_buffer_release(in);
	}
	if (out) {
		video_buffer_release(out);
	}
}

/* Configure encoder for a single-shot encode of the reference image and
 * dequeue the result. Returns the dequeued buffer pointer via *out_buf.
 */
int do_single_encode(struct video_buffer *in, struct video_buffer *out,
			    uint16_t quality, uint32_t width, uint32_t height,
			    uint32_t pitch, struct video_buffer **dequeued)
{
	struct video_format fmt = {
		.pixelformat = VIDEO_PIX_FMT_NV12,
		.width = width,
		.height = height,
		.pitch = pitch,
	};
	int ret;

	ret = video_set_format(jpeg_dev, VIDEO_EP_OUT, &fmt);
	if (ret) {
		return ret;
	}

	ret = video_set_ctrl(jpeg_dev, VIDEO_CID_JPEG_COMPRESSION_QUALITY, &quality);
	if (ret) {
		return ret;
	}

	ret = video_set_ctrl(jpeg_dev, VIDEO_CID_JPEG_INPUT_BUFFER, in->buffer);
	if (ret) {
		return ret;
	}

	out->bytesused = out->size;
	ret = video_enqueue(jpeg_dev, VIDEO_EP_OUT, out);
	if (ret) {
		return ret;
	}

	ret = video_stream_start(jpeg_dev);
	if (ret) {
		return ret;
	}

	ret = video_dequeue(jpeg_dev, VIDEO_EP_OUT, dequeued, K_SECONDS(5));
	(void)video_stream_stop(jpeg_dev);
	return ret;
}

/* ------------------------------------------------------------------ */
/* Tests                                                               */
/* ------------------------------------------------------------------ */

/*
 * Note: device-ready is enforced unconditionally by jpeg_suite_before()
 * before every test, so a dedicated device_ready case is intentionally
 * omitted.
 */

ZTEST(jpeg_hantro, test_jpeg_get_caps)
{
	struct video_caps caps = { 0 };
	bool nv12_seen = false;
	bool nv21_seen = false;
	int ret;

	ret = video_get_caps(jpeg_dev, VIDEO_EP_OUT, &caps);
	zassert_equal(ret, 0, "video_get_caps failed: %d", ret);
	zassert_not_null(caps.format_caps, "format_caps is NULL");

	for (int i = 0; caps.format_caps[i].pixelformat != 0; i++) {
		const struct video_format_cap *c = &caps.format_caps[i];

		TC_PRINT("Cap[%d]: 0x%08x %ux%u..%ux%u step=%ux%u\n", i,
			 c->pixelformat,
			 c->width_min, c->height_min,
			 c->width_max, c->height_max,
			 c->width_step, c->height_step);

		zassert_equal(c->width_step, 16, "width_step != 16");
		zassert_equal(c->height_step, 16, "height_step != 16");
		zassert_true(c->width_min  >= CONFIG_VIDEO_JPEG_HANTRO_VC9000E_MIN_SIZE,
			     "width_min %u < MIN_SIZE", c->width_min);
		zassert_true(c->width_max  <= CONFIG_VIDEO_JPEG_HANTRO_VC9000E_MAX_WIDTH,
			     "width_max %u > MAX_WIDTH", c->width_max);
		zassert_true(c->height_max <= CONFIG_VIDEO_JPEG_HANTRO_VC9000E_MAX_HEIGHT,
			     "height_max %u > MAX_HEIGHT", c->height_max);

		if (c->pixelformat == VIDEO_PIX_FMT_NV12) {
			nv12_seen = true;
		} else if (c->pixelformat == VIDEO_PIX_FMT_NV21) {
			nv21_seen = true;
		}
	}

	zassert_true(nv12_seen, "NV12 missing from capabilities");
	zassert_true(nv21_seen, "NV21 missing from capabilities");

	/* Wrong endpoint */
	ret = video_get_caps(jpeg_dev, VIDEO_EP_IN, &caps);
	zassert_equal(ret, -EINVAL,
		      "get_caps on EP_IN expected -EINVAL got %d", ret);
}

ZTEST(jpeg_hantro, test_jpeg_set_format_valid)
{
	const struct {
		uint32_t pixfmt;
		uint32_t w, h, pitch;
		const char *name;
	} cases[] = {
		{ VIDEO_PIX_FMT_NV12, 640, 480, 640, "NV12" },
		{ VIDEO_PIX_FMT_NV21, 320, 240, 320, "NV21" },
	};

	for (int i = 0; i < (int)ARRAY_SIZE(cases); i++) {
		struct video_format set = {
			.pixelformat = cases[i].pixfmt,
			.width  = cases[i].w,
			.height = cases[i].h,
			.pitch  = cases[i].pitch,
		};
		struct video_format got = { 0 };
		int ret;

		ret = video_set_format(jpeg_dev, VIDEO_EP_OUT, &set);
		zassert_equal(ret, 0,
			      "set_format %s failed: %d", cases[i].name, ret);

		ret = video_get_format(jpeg_dev, VIDEO_EP_OUT, &got);
		zassert_equal(ret, 0,
			      "get_format %s failed: %d", cases[i].name, ret);
		zassert_equal(got.pixelformat, cases[i].pixfmt,
			      "%s pixfmt mismatch", cases[i].name);
		zassert_equal(got.width,  cases[i].w,
			      "%s width mismatch",  cases[i].name);
		zassert_equal(got.height, cases[i].h,
			      "%s height mismatch", cases[i].name);
		zassert_equal(got.pitch,  cases[i].pitch,
			      "%s pitch mismatch",  cases[i].name);
	}
}

ZTEST(jpeg_hantro, test_jpeg_set_format_invalid)
{
	struct video_format f;
	int ret;

	/* Too small */
	f = (struct video_format){
		.pixelformat = VIDEO_PIX_FMT_NV12,
		.width = 16, .height = 16, .pitch = 16,
	};
	ret = video_set_format(jpeg_dev, VIDEO_EP_OUT, &f);
	zassert_equal(ret, -EINVAL, "tiny dims expected -EINVAL got %d", ret);

	/* Too large */
	f = (struct video_format){
		.pixelformat = VIDEO_PIX_FMT_NV12,
		.width  = CONFIG_VIDEO_JPEG_HANTRO_VC9000E_MAX_WIDTH + 16,
		.height = CONFIG_VIDEO_JPEG_HANTRO_VC9000E_MAX_HEIGHT + 16,
		.pitch  = CONFIG_VIDEO_JPEG_HANTRO_VC9000E_MAX_WIDTH + 16,
	};
	ret = video_set_format(jpeg_dev, VIDEO_EP_OUT, &f);
	zassert_equal(ret, -EINVAL, "huge dims expected -EINVAL got %d", ret);

	/* Unsupported pixel format */
	f = (struct video_format){
		.pixelformat = VIDEO_PIX_FMT_RGB565,
		.width = 320, .height = 240, .pitch = 640,
	};
	ret = video_set_format(jpeg_dev, VIDEO_EP_OUT, &f);
	zassert_equal(ret, -ENOTSUP,
		      "RGB565 expected -ENOTSUP got %d", ret);

	/* Wrong endpoint */
	f = (struct video_format){
		.pixelformat = VIDEO_PIX_FMT_NV12,
		.width = 320, .height = 240, .pitch = 320,
	};
	ret = video_set_format(jpeg_dev, VIDEO_EP_IN, &f);
	zassert_equal(ret, -EINVAL,
		      "EP_IN set_format expected -EINVAL got %d", ret);
}

ZTEST(jpeg_hantro, test_jpeg_set_get_ctrl_quality)
{
	uint16_t q;
	int ret;

	const uint16_t levels[] = { 1, 50, 100 };

	for (int i = 0; i < (int)ARRAY_SIZE(levels); i++) {
		q = levels[i];
		ret = video_set_ctrl(jpeg_dev,
				     VIDEO_CID_JPEG_COMPRESSION_QUALITY, &q);
		zassert_equal(ret, 0,
			      "set quality %u failed: %d", levels[i], ret);

		q = 0;
		ret = video_get_ctrl(jpeg_dev,
				     VIDEO_CID_JPEG_COMPRESSION_QUALITY, &q);
		zassert_equal(ret, 0, "get quality failed: %d", ret);
		zassert_equal(q, levels[i],
			      "quality readback %u != %u", q, levels[i]);
	}

	/* Unsupported CID */
	uint32_t dummy = 0;

	ret = video_set_ctrl(jpeg_dev, 0xDEADBEEF, &dummy);
	zassert_equal(ret, -ENOTSUP, "unknown CID expected -ENOTSUP got %d", ret);
}

/*
 * Single-frame encode validation done at two quality levels: covers
 * the basic-encode sanity (markers + > header size + < input size +
 * compression ratio) and additionally checks that the quality knob
 * has the documented monotonic effect on output size (higher Q =>
 * larger compressed stream).
 */
ZTEST(jpeg_hantro, test_jpeg_encode_quality_levels)
{
	struct video_buffer *in = NULL, *out = NULL, *deq = NULL;
	uint32_t size_q10, size_q90;
	int ret;

	alloc_ref_buffers(&in, &out);

	ret = do_single_encode(in, out, 10, REF_IMG_WIDTH, REF_IMG_HEIGHT,
			       REF_IMG_PITCH, &deq);
	zassert_equal(ret, 0, "encode q=10 failed: %d", ret);
	zassert_not_null(deq, "q=10 dequeued buffer NULL");
	zassert_true(deq->bytesused > JPEG_HEADER_SIZE,
		     "q=10 bytesused %u <= header size", deq->bytesused);
	zassert_true(deq->bytesused < REF_IMG_NV12_SIZE,
		     "q=10 no compression: out %u >= in %u",
		     deq->bytesused, REF_IMG_NV12_SIZE);
	zassert_true(jpeg_validate_markers((uint8_t *)deq->buffer,
					   deq->bytesused),
		     "q=10 markers missing");
	size_q10 = deq->bytesused;

	ret = do_single_encode(in, out, 90, REF_IMG_WIDTH, REF_IMG_HEIGHT,
			       REF_IMG_PITCH, &deq);
	zassert_equal(ret, 0, "encode q=90 failed: %d", ret);
	zassert_not_null(deq, "q=90 dequeued buffer NULL");
	zassert_true(jpeg_validate_markers((uint8_t *)deq->buffer,
					   deq->bytesused),
		     "q=90 markers missing");
	size_q90 = deq->bytesused;

	TC_PRINT("Encoded %ux%u: in=%u  q10=%u (%.2fx)  q90=%u (%.2fx)\n",
		 REF_IMG_WIDTH, REF_IMG_HEIGHT, REF_IMG_NV12_SIZE,
		 size_q10, (double)REF_IMG_NV12_SIZE / (double)size_q10,
		 size_q90, (double)REF_IMG_NV12_SIZE / (double)size_q90);

	/* Export hints (only the q=90 buffer survives in `out` at this point). */
	TC_PRINT("EXPORT[q=90] gdb   : dump binary memory \"q90.jpg\" "
		 "0x%08x 0x%08x\n",
		 (uint32_t)(uintptr_t)deq->buffer,
		 (uint32_t)(uintptr_t)deq->buffer + size_q90);
	TC_PRINT("EXPORT[q=90] J-Link: savebin q90.jpg 0x%08x 0x%x\n",
		 (uint32_t)(uintptr_t)deq->buffer, size_q90);

	zassert_true(size_q90 > size_q10,
		     "expected q90 (%u) > q10 (%u)", size_q90, size_q10);

	release_buffers(in, out);
}

ZTEST(jpeg_hantro, test_jpeg_encode_multi_resolutions)
{
	const struct {
		uint32_t w, h;
	} cases[] = {
		{ 32,  32  },
		{ 320, 240 },
		{ 640, 480 },
	};
	int ret;

	for (int i = 0; i < (int)ARRAY_SIZE(cases); i++) {
		uint32_t w = cases[i].w;
		uint32_t h = cases[i].h;
		size_t   nv12_size = (size_t)w * h * 3 / 2;
		struct video_buffer *in = NULL, *out = NULL, *deq = NULL;

		in  = video_buffer_alloc(nv12_size, K_NO_WAIT);
		zassert_not_null(in,  "alloc in (%ux%u) failed", w, h);
		out = video_buffer_alloc(nv12_size + JPEG_HEADER_SIZE, K_NO_WAIT);
		zassert_not_null(out, "alloc out (%ux%u) failed", w, h);

		jpeg_fill_nv12_gradient((uint8_t *)in->buffer, w, h, w);

		ret = do_single_encode(in, out, 50, w, h, w, &deq);
		zassert_equal(ret, 0, "encode %ux%u failed: %d", w, h, ret);
		zassert_true(jpeg_validate_markers((uint8_t *)deq->buffer,
						   deq->bytesused),
			     "%ux%u markers missing", w, h);

		TC_PRINT("Encoded %ux%u -> %u bytes\n", w, h, deq->bytesused);

		release_buffers(in, out);
	}
}

ZTEST(jpeg_hantro, test_jpeg_stream_already_started_stopped)
{
	struct video_format fmt = {
		.pixelformat = VIDEO_PIX_FMT_NV12,
		.width = 320, .height = 240, .pitch = 320,
	};
	int ret;

	/* Need a valid format before stream_start. */
	ret = video_set_format(jpeg_dev, VIDEO_EP_OUT, &fmt);
	zassert_equal(ret, 0, "set_format failed: %d", ret);

	ret = video_stream_start(jpeg_dev);
	zassert_equal(ret, 0, "stream_start failed: %d", ret);

	ret = video_stream_start(jpeg_dev);
	zassert_equal(ret, -EALREADY,
		      "duplicate stream_start expected -EALREADY got %d", ret);

	ret = video_stream_stop(jpeg_dev);
	zassert_equal(ret, 0, "stream_stop failed: %d", ret);

	ret = video_stream_stop(jpeg_dev);
	zassert_equal(ret, -EALREADY,
		      "duplicate stream_stop expected -EALREADY got %d", ret);
}

ZTEST(jpeg_hantro, test_jpeg_enqueue_busy)
{
	struct video_buffer *in = NULL, *out = NULL, *deq = NULL;
	int ret;

	alloc_ref_buffers(&in, &out);

	ret = video_set_ctrl(jpeg_dev, VIDEO_CID_JPEG_INPUT_BUFFER, in->buffer);
	zassert_equal(ret, 0, "set input buffer failed: %d", ret);

	struct video_format fmt = {
		.pixelformat = VIDEO_PIX_FMT_NV12,
		.width  = REF_IMG_WIDTH,
		.height = REF_IMG_HEIGHT,
		.pitch  = REF_IMG_PITCH,
	};

	ret = video_set_format(jpeg_dev, VIDEO_EP_OUT, &fmt);
	zassert_equal(ret, 0, "set_format failed: %d", ret);

	out->bytesused = DEFAULT_OUT_BUF_SIZE;
	ret = video_enqueue(jpeg_dev, VIDEO_EP_OUT, out);
	zassert_equal(ret, 0, "first enqueue failed: %d", ret);

	/* Second enqueue without dequeue — driver allows only 1 in flight. */
	ret = video_enqueue(jpeg_dev, VIDEO_EP_OUT, out);
	zassert_equal(ret, -EBUSY,
		      "second enqueue expected -EBUSY got %d", ret);

	/* Drain so the test does not leak state. */
	ret = video_stream_start(jpeg_dev);
	zassert_equal(ret, 0, "stream_start failed: %d", ret);
	ret = video_dequeue(jpeg_dev, VIDEO_EP_OUT, &deq, K_SECONDS(5));
	zassert_equal(ret, 0, "drain dequeue failed: %d", ret);
	(void)video_stream_stop(jpeg_dev);

	release_buffers(in, out);
}

ZTEST(jpeg_hantro, test_jpeg_dequeue_timeout)
{
	struct video_buffer *deq = NULL;
	int ret;

	/* No enqueue performed — dequeue must time out. */
	ret = video_dequeue(jpeg_dev, VIDEO_EP_OUT, &deq, K_MSEC(50));
	zassert_equal(ret, -EAGAIN,
		      "dequeue with no buffer expected -EAGAIN got %d", ret);
}

/*
 * Repeated encode of the same input + same parameters must produce the
 * same output size. This helps catch driver state leaks across encodes
 * even when the test does not compare the full byte stream.
 */
ZTEST(jpeg_hantro, test_jpeg_back_to_back_encodes)
{
	struct video_buffer *in = NULL, *out = NULL, *deq = NULL;
	const int iters = 5;
	uint32_t first_size = 0;
	int ret;

	alloc_ref_buffers(&in, &out);

	for (int i = 0; i < iters; i++) {
		ret = do_single_encode(in, out, 50, REF_IMG_WIDTH,
				       REF_IMG_HEIGHT, REF_IMG_PITCH, &deq);
		zassert_equal(ret, 0, "encode iter %d failed: %d", i, ret);
		zassert_true(jpeg_validate_markers((uint8_t *)deq->buffer,
						   deq->bytesused),
			     "iter %d markers missing", i);

		if (i == 0) {
			first_size = deq->bytesused;
		} else {
			zassert_equal(deq->bytesused, first_size,
				      "iter %d size %u != iter0 size %u "
				      "(non-deterministic re-encode)",
				      i, deq->bytesused, first_size);
		}
		TC_PRINT("iter %d: %u bytes\n", i, deq->bytesused);

		if (i == iters - 1) {
			TC_PRINT("EXPORT[b2b q=50 iter %d] gdb   : "
				 "dump binary memory \"b2b_q50.jpg\" "
				 "0x%08x 0x%08x\n",
				 i,
				 (uint32_t)(uintptr_t)deq->buffer,
				 (uint32_t)(uintptr_t)deq->buffer +
					deq->bytesused);
			TC_PRINT("EXPORT[b2b q=50 iter %d] J-Link: "
				 "savebin b2b_q50.jpg 0x%08x 0x%x\n",
				 i,
				 (uint32_t)(uintptr_t)deq->buffer,
				 deq->bytesused);
		}
	}

	release_buffers(in, out);
}

/* ------------------------------------------------------------------ */
/* Pre-init / zero-state test.                                         */
/*                                                                     */
/* Name prefixed with "aa_" so it sorts alphabetically BEFORE every    */
/* other test_jpeg_* and therefore executes first, while data->fmt    */
/* is still zero-initialized (no prior set_format anywhere in the      */
/* suite). The suite-before hook only does device_is_ready and does    */
/* NOT touch driver state, so the fresh-state condition is preserved.  */
/* ------------------------------------------------------------------ */

/*
 * stream_start() must validate that a format was ever set.
 * After fresh init data->fmt is zero (width=0, height=0), which is
 * below the documented minimum (32). Per the video API contract
 * stream_start on an unconfigured device must fail.
 */
ZTEST(jpeg_hantro, test_jpeg_aa_initial_state_rejects_stream_start)
{
	struct video_format fmt = { 0 };
	int ret;

	ret = video_get_format(jpeg_dev, VIDEO_EP_OUT, &fmt);
	zassert_equal(ret, 0, "get_format on init failed: %d", ret);

	TC_PRINT("Initial fmt: pix=0x%08x %ux%u pitch=%u\n",
		 fmt.pixelformat, fmt.width, fmt.height, fmt.pitch);

	/* If the driver pre-initialized fmt to a valid default, the
	 * "no set_format ever called" scenario is unreachable. Skip.
	 */
	if (fmt.width >= 32 && fmt.height >= 32) {
		TC_PRINT("Driver provides default fmt; skipping check\n");
		ztest_test_skip();
		return;
	}

	ret = video_stream_start(jpeg_dev);
	/* Best-effort cleanup so subsequent tests are unaffected. */
	if (ret == 0) {
		(void)video_stream_stop(jpeg_dev);
	}
	zassert_not_equal(ret, 0,
			  "stream_start without set_format must fail,%d", ret);
}

/* ------------------------------------------------------------------ */
/* API contract validation tests.                                       */
/* ------------------------------------------------------------------ */

/*
 * Verify that get_ctrl(VIDEO_CID_JPEG_INPUT_BUFFER) returns the
 * value previously set via set_ctrl. After set/get round-trip the
 * caller's variable must equal what was set.
 */
ZTEST(jpeg_hantro, test_jpeg_get_ctrl_input_buffer_roundtrip)
{
	uint8_t dummy_buf[16] = { 0 };
	void *readback = NULL;
	int ret;

	ret = video_set_ctrl(jpeg_dev, VIDEO_CID_JPEG_INPUT_BUFFER, dummy_buf);
	zassert_equal(ret, 0, "set_ctrl(INPUT_BUFFER) failed: %d", ret);

	ret = video_get_ctrl(jpeg_dev, VIDEO_CID_JPEG_INPUT_BUFFER, &readback);
	zassert_equal(ret, 0, "get_ctrl(INPUT_BUFFER) failed: %d", ret);
	zassert_equal_ptr(readback, dummy_buf,
			  "INPUT_BUFFER get_ctrl did not return the value "
			  "previously set: got=%p expected=%p",
			  readback, dummy_buf);

	/* Reset to NULL so it doesn't leak into other tests. */
	(void)video_set_ctrl(jpeg_dev, VIDEO_CID_JPEG_INPUT_BUFFER, NULL);
}

/*
 * Verify that set_ctrl(VIDEO_CID_JPEG_COMPRESSION_QUALITY) validates
 * range. Per JPEG conventions and the driver's own Kconfig/devicetree
 * comments (1..100), values of 0 and >100 must be rejected with
 * -EINVAL.
 */
ZTEST(jpeg_hantro, test_jpeg_quality_range_validation)
{
	uint16_t q;
	int ret;

	q = 0;
	ret = video_set_ctrl(jpeg_dev, VIDEO_CID_JPEG_COMPRESSION_QUALITY, &q);
	zassert_equal(ret, -EINVAL,
		      "quality=0 should be rejected, got %d", ret);

	q = 200;
	ret = video_set_ctrl(jpeg_dev, VIDEO_CID_JPEG_COMPRESSION_QUALITY, &q);
	zassert_equal(ret, -EINVAL,
		      "quality=200 should be rejected, got %d", ret);

	/* Sanity: legal extremes (1 and 100) must continue to succeed. */
	q = 1;
	ret = video_set_ctrl(jpeg_dev, VIDEO_CID_JPEG_COMPRESSION_QUALITY, &q);
	zassert_equal(ret, 0, "quality=1 unexpectedly rejected: %d", ret);

	q = 100;
	ret = video_set_ctrl(jpeg_dev, VIDEO_CID_JPEG_COMPRESSION_QUALITY, &q);
	zassert_equal(ret, 0, "quality=100 unexpectedly rejected: %d", ret);
}

/* ------------------------------------------------------------------ */
/* Unique-scenario tests: stride / alignment edge cases.               */
/* ------------------------------------------------------------------ */

/*
 * NV12 with pitch > width (line padding). The encoder must walk the
 * source using `pitch` bytes per row but emit only `width` columns.
 * Validates that the driver's ROWLENGTH / LUMA_STRIDE / CHROMA_STRIDE
 * register programming follows fmt.pitch rather than fmt.width.
 */
ZTEST(jpeg_hantro, test_jpeg_pitch_with_padding)
{
	const uint32_t width  = 320;
	const uint32_t height = 240;
	const uint32_t pitch  = 384; /* 64 bytes of right-side padding */
	const size_t   nv12_sz = (size_t)pitch * height * 3 / 2;

	struct video_buffer *in = NULL, *out = NULL, *deq = NULL;
	int ret;

	if (!alloc_pair_or_skip(nv12_sz, nv12_sz + JPEG_HEADER_SIZE,
				&in, &out, "pitch_with_padding")) {
		return;
	}

	jpeg_fill_nv12_gradient((uint8_t *)in->buffer, width, height, pitch);

	ret = do_single_encode(in, out, 50, width, height, pitch, &deq);
	zassert_equal(ret, 0, "padded-pitch encode failed: %d", ret);
	zassert_not_null(deq, "padded-pitch dequeued buffer NULL");
	zassert_true(jpeg_validate_markers((uint8_t *)deq->buffer,
					   deq->bytesused),
		     "padded-pitch SOI/EOI missing");

	TC_PRINT("Encoded %ux%u (pitch=%u) -> %u bytes\n",
		 width, height, pitch, deq->bytesused);

	release_buffers(in, out);
}

/*
 * Non-16-aligned dimensions exercise the xfill / yfill paths in
 * set_format(). The JPEG header still reports the user-supplied
 * width/height; the encoder pads internally to the 16-px MCU grid.
 * Validates encode succeeds and produces a structurally-valid stream.
 */
ZTEST(jpeg_hantro, test_jpeg_non_aligned_dimensions)
{
	const uint32_t width  = 300;  /* 300 % 16 = 12  -> xfill = 2 */
	const uint32_t height = 200;  /* 200 % 16 =  8  -> yfill = 8 */
	const uint32_t pitch  = width;
	const size_t   nv12_sz = (size_t)pitch * height * 3 / 2;

	struct video_buffer *in = NULL, *out = NULL, *deq = NULL;
	int ret;

	if (!alloc_pair_or_skip(nv12_sz, nv12_sz + JPEG_HEADER_SIZE,
				&in, &out, "non_aligned_dimensions")) {
		return;
	}

	jpeg_fill_nv12_gradient((uint8_t *)in->buffer, width, height, pitch);

	ret = do_single_encode(in, out, 50, width, height, pitch, &deq);
	zassert_equal(ret, 0, "non-aligned encode failed: %d", ret);
	zassert_not_null(deq, "non-aligned dequeued buffer NULL");
	zassert_true(jpeg_validate_markers((uint8_t *)deq->buffer,
					   deq->bytesused),
		     "non-aligned SOI/EOI missing");

	TC_PRINT("Encoded %ux%u (xfill=2, yfill=8) -> %u bytes\n",
		 width, height, deq->bytesused);

	release_buffers(in, out);
}

/* ------------------------------------------------------------------ */
/* Extended coverage tests.                                            */
/* ------------------------------------------------------------------ */

/*
 * NV21 end-to-end encode. The set_format-only NV21 case in
 * test_jpeg_set_format_valid does NOT run the actual encoder path
 * (chroma-swap register programming, jpeg_start_encode with NV21,
 * etc). This test closes that gap by performing a full encode using
 * the same reference YUV bytes interpreted as NV21.
 */
ZTEST(jpeg_hantro, test_jpeg_encode_nv21_end_to_end)
{
	struct video_buffer *in = NULL, *out = NULL, *deq = NULL;
	struct video_format fmt = {
		.pixelformat = VIDEO_PIX_FMT_NV21,
		.width  = REF_IMG_WIDTH,
		.height = REF_IMG_HEIGHT,
		.pitch  = REF_IMG_PITCH,
	};
	uint16_t quality = 50;
	int ret;

	alloc_ref_buffers(&in, &out);

	ret = video_set_format(jpeg_dev, VIDEO_EP_OUT, &fmt);
	zassert_equal(ret, 0, "set_format(NV21) failed: %d", ret);

	ret = video_set_ctrl(jpeg_dev, VIDEO_CID_JPEG_COMPRESSION_QUALITY,
			     &quality);
	zassert_equal(ret, 0, "set_ctrl(quality) failed: %d", ret);

	ret = video_set_ctrl(jpeg_dev, VIDEO_CID_JPEG_INPUT_BUFFER, in->buffer);
	zassert_equal(ret, 0, "set_ctrl(input) failed: %d", ret);

	out->bytesused = DEFAULT_OUT_BUF_SIZE;
	ret = video_enqueue(jpeg_dev, VIDEO_EP_OUT, out);
	zassert_equal(ret, 0, "NV21 enqueue failed: %d", ret);

	ret = video_stream_start(jpeg_dev);
	zassert_equal(ret, 0, "NV21 stream_start failed: %d", ret);

	ret = video_dequeue(jpeg_dev, VIDEO_EP_OUT, &deq, K_SECONDS(5));
	(void)video_stream_stop(jpeg_dev);

	zassert_equal(ret, 0, "NV21 dequeue failed: %d", ret);
	zassert_not_null(deq, "NV21 dequeued buffer NULL");
	zassert_true(jpeg_validate_markers((uint8_t *)deq->buffer,
					   deq->bytesused),
		     "NV21 SOI/EOI missing");

	TC_PRINT("NV21 encode %ux%u -> %u bytes\n",
		 REF_IMG_WIDTH, REF_IMG_HEIGHT, deq->bytesused);

	release_buffers(in, out);
}

/*
 * Format-switch between encodes. Two encodes back-to-back at
 * DIFFERENT resolutions on the same device. Validates that
 * set_format() correctly re-programs all dimension-dependent HW
 * registers (encoding_width/height, xfill/yfill, stride) and that no
 * state leaks from the first encode into the second.
 */
ZTEST(jpeg_hantro, test_jpeg_encode_format_switch)
{
	struct video_buffer *in = NULL, *out = NULL, *deq = NULL;
	const uint32_t w1 = 640, h1 = 480;
	const uint32_t w2 = 320, h2 = 240;
	const size_t   sz1 = (size_t)w1 * h1 * 3 / 2;
	const size_t   sz2 = (size_t)w2 * h2 * 3 / 2;
	uint32_t size_at_640, size_at_320;
	int ret;

	/* The video buffer pool has NUM_MAX=2 slots, so we can only
	 * keep two buffers alive at once. Allocate the output buffer
	 * once (reused across both encodes), and recycle the input
	 * slot between the two resolutions.
	 */
	out = video_buffer_alloc(sz1 + JPEG_HEADER_SIZE, K_NO_WAIT);
	zassert_not_null(out, "alloc out failed");

	/* First encode at 640x480. */
	in = video_buffer_alloc(sz1, K_NO_WAIT);
	zassert_not_null(in, "alloc in (640x480) failed");
	jpeg_fill_nv12_gradient((uint8_t *)in->buffer, w1, h1, w1);

	ret = do_single_encode(in, out, 50, w1, h1, w1, &deq);
	zassert_equal(ret, 0, "640x480 encode failed: %d", ret);
	zassert_true(jpeg_validate_markers((uint8_t *)deq->buffer,
					   deq->bytesused),
		     "640x480 markers missing");
	size_at_640 = deq->bytesused;

	video_buffer_release(in);
	in = NULL;

	/* Second encode at 320x240 (different resolution => set_format
	 * must reprogram all dimension-dependent HW registers).
	 */
	in = video_buffer_alloc(sz2, K_NO_WAIT);
	zassert_not_null(in, "alloc in (320x240) failed");
	jpeg_fill_nv12_gradient((uint8_t *)in->buffer, w2, h2, w2);

	ret = do_single_encode(in, out, 50, w2, h2, w2, &deq);
	zassert_equal(ret, 0, "320x240 encode (after 640x480) failed: %d",
		      ret);
	zassert_true(jpeg_validate_markers((uint8_t *)deq->buffer,
					   deq->bytesused),
		     "320x240 markers missing");
	size_at_320 = deq->bytesused;

	zassert_true(size_at_640 > size_at_320,
		     "640x480 size %u should exceed 320x240 size %u",
		     size_at_640, size_at_320);

	TC_PRINT("Format-switch: 640x480=%u  320x240=%u\n",
		 size_at_640, size_at_320);

	video_buffer_release(in);
	video_buffer_release(out);
}

/*
 * Negative path: enqueue with NULL input buffer must be rejected.
 * The driver checks data->input_buffer == NULL before queueing and
 * returns -EINVAL. Resets state on entry/exit so it does not depend
 * on test ordering.
 */
ZTEST(jpeg_hantro, test_jpeg_enqueue_without_input_buffer)
{
	struct video_buffer *out = NULL;
	struct video_format fmt = {
		.pixelformat = VIDEO_PIX_FMT_NV12,
		.width = 320, .height = 240, .pitch = 320,
	};
	int ret;

	/* Clear any input buffer left over from previous tests. */
	ret = video_set_ctrl(jpeg_dev, VIDEO_CID_JPEG_INPUT_BUFFER, NULL);
	zassert_equal(ret, 0, "clear input ctrl failed: %d", ret);

	ret = video_set_format(jpeg_dev, VIDEO_EP_OUT, &fmt);
	zassert_equal(ret, 0, "set_format failed: %d", ret);

	out = video_buffer_alloc(64 * 1024, K_NO_WAIT);
	zassert_not_null(out, "alloc out failed");
	out->bytesused = 64 * 1024;

	ret = video_enqueue(jpeg_dev, VIDEO_EP_OUT, out);
	zassert_equal(ret, -EINVAL,
		      "enqueue with NULL input expected -EINVAL got %d", ret);

	video_buffer_release(out);
}

/*
 * Deep structural validation of the produced JPEG. SOI/EOI-only
 * checks accept many malformed streams; this test additionally
 * asserts presence of APP0/JFIF identifier, at least one DQT, SOF0,
 * at least one DHT, SOS and a terminating EOI -- enough to guarantee
 * the stream is decodable by any baseline JPEG decoder.
 */
ZTEST(jpeg_hantro, test_jpeg_jfif_structural_validation)
{
	struct video_buffer *in = NULL, *out = NULL, *deq = NULL;
	int ret;

	alloc_ref_buffers(&in, &out);

	ret = do_single_encode(in, out, 75, REF_IMG_WIDTH, REF_IMG_HEIGHT,
			       REF_IMG_PITCH, &deq);
	zassert_equal(ret, 0, "encode failed: %d", ret);
	zassert_true(jpeg_validate_markers((uint8_t *)deq->buffer,
					   deq->bytesused),
		     "SOI/EOI missing");
	zassert_true(jpeg_validate_jfif_structure((uint8_t *)deq->buffer,
						  deq->bytesused),
		     "JFIF structure incomplete (missing APP0/JFIF, DQT, "
		     "SOF0, DHT, SOS or EOI in proper order)");

	/* Verify that SOF0 reports the same dimensions we encoded. Catches
	 * driver bugs where the JPEG header carries stale/swapped W/H.
	 */
	uint16_t enc_w = 0, enc_h = 0;

	zassert_true(jpeg_extract_sof0_dimensions((uint8_t *)deq->buffer,
						  deq->bytesused,
						  &enc_w, &enc_h),
		     "SOF0 marker not found / unparseable");
	zassert_equal(enc_w, REF_IMG_WIDTH,
		      "SOF0 width mismatch: got %u, expected %u",
		      enc_w, REF_IMG_WIDTH);
	zassert_equal(enc_h, REF_IMG_HEIGHT,
		      "SOF0 height mismatch: got %u, expected %u",
		      enc_h, REF_IMG_HEIGHT);

	TC_PRINT("JFIF structure OK: %u bytes, SOF0=%ux%u\n",
		 deq->bytesused, enc_w, enc_h);

	release_buffers(in, out);
}

/*
 * Large-resolution stress: 1920x1080 NV12 encode. Validates that the
 * driver handles address calculations and HW programming for FHD
 * input. Skipped automatically if the buffer pool is too small for
 * this resolution (e.g. when CONFIG_VIDEO_BUFFER_POOL_SZ_MAX has been
 * lowered).
 */
ZTEST(jpeg_hantro, test_jpeg_encode_large_1920x1080)
{
	const uint32_t width  = 1920;
	const uint32_t height = 1080;
	const size_t   nv12_sz = (size_t)width * height * 3 / 2;

	struct video_buffer *in = NULL, *out = NULL, *deq = NULL;
	int ret;

	if (!alloc_pair_or_skip(nv12_sz, nv12_sz + JPEG_HEADER_SIZE,
				&in, &out, "1920x1080")) {
		return;
	}

	jpeg_fill_nv12_gradient((uint8_t *)in->buffer, width, height, width);

	ret = do_single_encode(in, out, 50, width, height, width, &deq);
	zassert_equal(ret, 0, "1920x1080 encode failed: %d", ret);
	zassert_not_null(deq, "1920x1080 dequeued NULL");
	zassert_true(jpeg_validate_jfif_structure((uint8_t *)deq->buffer,
						  deq->bytesused),
		     "1920x1080 JFIF structure incomplete");

	TC_PRINT("Encoded 1920x1080 -> %u bytes (%.2fx compression)\n",
		 deq->bytesused, (double)nv12_sz / (double)deq->bytesused);
	TC_PRINT("EXPORT[1920x1080 q=50] gdb   : "
		 "dump binary memory \"fhd_q50.jpg\" 0x%08x 0x%08x\n",
		 (uint32_t)(uintptr_t)deq->buffer,
		 (uint32_t)(uintptr_t)deq->buffer + deq->bytesused);
	TC_PRINT("EXPORT[1920x1080 q=50] J-Link: "
		 "savebin fhd_q50.jpg 0x%08x 0x%x\n",
		 (uint32_t)(uintptr_t)deq->buffer, deq->bytesused);

	release_buffers(in, out);
}

/*
 * HRM negative contract: the IP supports many input formats (YUV400,
 * YUV422, RGB, tiled, etc.) but the Zephyr driver only exposes NV12
 * and NV21. set_format MUST reject every other documented HRM input
 * format with a non-zero error. Locks in the negative API contract
 * so a future driver expansion cannot silently change behavior
 * without updating this test.
 */
ZTEST(jpeg_hantro, test_jpeg_set_format_unsupported_hrm_formats)
{
	static const struct {
		uint32_t fourcc;
		const char *name;
	} cases[] = {
		{ VIDEO_PIX_FMT_YUYV, "YUYV (4:2:2)" },
		{ VIDEO_PIX_FMT_GREY, "GREY/Y8 (4:0:0)" },
		{ VIDEO_PIX_FMT_RGB565, "RGB565" },
		{ VIDEO_PIX_FMT_XRGB32, "XRGB32" },
	};
	int ret;

	for (int i = 0; i < (int)ARRAY_SIZE(cases); i++) {
		struct video_format fmt = {
			.pixelformat = cases[i].fourcc,
			.width = 320, .height = 240, .pitch = 320,
		};
		ret = video_set_format(jpeg_dev, VIDEO_EP_OUT, &fmt);
		zassert_not_equal(ret, 0,
				  "%s (0x%08x) should be rejected, got 0",
				  cases[i].name, cases[i].fourcc);
		TC_PRINT("rejected %-20s (0x%08x): ret=%d\n",
			 cases[i].name, cases[i].fourcc, ret);
	}
}

/*
 * Wide-image / max-width path. The IP and driver advertise a max
 * width of 16384 px. Encoding at the full max would consume an
 * impractical pool slot (16384 * H * 1.5); instead we encode at
 * 8192x32 NV12 which still exercises the wide-stride address math
 * end-to-end (~393 KB input). If your pool is sized smaller this
 * test will auto-skip.
 */
ZTEST(jpeg_hantro, test_jpeg_encode_max_width)
{
	const uint32_t width  = 8192;
	const uint32_t height = 32;
	const size_t   nv12_sz = (size_t)width * height * 3 / 2;

	struct video_buffer *in = NULL, *out = NULL, *deq = NULL;
	int ret;

	if (!alloc_pair_or_skip(nv12_sz, nv12_sz + JPEG_HEADER_SIZE,
				&in, &out, "8192x32")) {
		return;
	}

	jpeg_fill_nv12_gradient((uint8_t *)in->buffer, width, height, width);

	ret = do_single_encode(in, out, 50, width, height, width, &deq);
	zassert_equal(ret, 0, "8192x32 encode failed: %d", ret);
	zassert_true(jpeg_validate_jfif_structure((uint8_t *)deq->buffer,
						  deq->bytesused),
		     "8192x32 JFIF structure incomplete");

	TC_PRINT("Encoded 8192x32 -> %u bytes\n", deq->bytesused);

	release_buffers(in, out);
}

ZTEST_SUITE(jpeg_hantro, NULL, NULL, jpeg_suite_before, NULL, NULL);

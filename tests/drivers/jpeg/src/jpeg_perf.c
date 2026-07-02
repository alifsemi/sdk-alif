/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement.
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/*
 * JPEG encoder performance benchmark suite.
 *
 * This suite measures encode latency and throughput at various resolutions.
 * Tests are informational (no hard pass/fail thresholds) but will fail if
 * encode time goes to zero (clock not running) or any encode errors out.
 */

#include "jpeg_common.h"
#include <zephyr/ztest.h>
#include <zephyr/kernel.h>

static void jpeg_perf_suite_before(void *fixture)
{
	ARG_UNUSED(fixture);

	jpeg_dev = DEVICE_DT_GET(DT_NODELABEL(jpeg0));
	zassert_not_null(jpeg_dev, "jpeg device not found");
	zassert_true(device_is_ready(jpeg_dev),
		"jpeg device not ready: %s", jpeg_dev->name);
}

/*
 * Performance benchmark: measures encode latency and throughput at the
 * reference 1280x720 resolution. Runs N iterations of a single encode
 * (set_format -> set_ctrl -> enqueue -> stream_start -> dequeue ->
 * stream_stop) and reports per-frame mean/min/max in microseconds plus
 * the equivalent input-pixel throughput in MB/s. This is informational
 * (no hard pass/fail threshold) but will fail if encode time goes to
 * zero (clock not running) or any single iteration errors out.
 */
ZTEST(jpeg_perf, test_jpeg_encode_performance_1280x720)
{
	struct video_buffer *in = NULL, *out = NULL, *deq = NULL;
	const int iterations = 10;
	uint32_t cyc_min = UINT32_MAX, cyc_max = 0;
	uint64_t cyc_sum = 0;
	int i, ret;

	alloc_ref_buffers(&in, &out);

	TC_PRINT("Performance test: %ux%u NV12, quality=75, iterations=%d\n",
		 REF_IMG_WIDTH, REF_IMG_HEIGHT, iterations);

	/* Warm-up encode (not measured) to exclude one-time HW init costs. */
	TC_PRINT("Step 1: Warm-up encode (excludes HW init costs)\n");
	ret = do_single_encode(in, out, 75, REF_IMG_WIDTH, REF_IMG_HEIGHT,
			       REF_IMG_PITCH, &deq);
	zassert_equal(ret, 0, "warm-up encode failed: %d", ret);
	TC_PRINT("Warm-up complete: %u bytes encoded\n", deq->bytesused);

	TC_PRINT("Step 2: Timed iterations (n=%d)\n", iterations);
	for (i = 0; i < iterations; i++) {
		uint32_t t0 = k_cycle_get_32();

		ret = do_single_encode(in, out, 75, REF_IMG_WIDTH,
				       REF_IMG_HEIGHT, REF_IMG_PITCH, &deq);
		uint32_t dt = k_cycle_get_32() - t0;

		zassert_equal(ret, 0, "iter %d encode failed: %d", i, ret);
		zassert_true(jpeg_validate_markers((uint8_t *)deq->buffer,
						   deq->bytesused),
			     "iter %d: SOI/EOI missing", i);

		cyc_sum += dt;
		if (dt < cyc_min) {
			cyc_min = dt;
		}
		if (dt > cyc_max) {
			cyc_max = dt;
		}

		uint32_t hz = sys_clock_hw_cycles_per_sec();
		uint32_t us = (uint32_t)((uint64_t)dt * 1000000U / hz);

		TC_PRINT("  Iter %2d: %u cycles (%u us), %u bytes\n",
			 i, dt, us, deq->bytesused);
	}

	zassert_true(cyc_sum > 0, "cycle counter not running");

	TC_PRINT("Step 3: Metrics calculation\n");
	uint32_t cyc_mean = (uint32_t)(cyc_sum / iterations);
	uint32_t hz       = sys_clock_hw_cycles_per_sec();
	uint32_t us_mean  = (uint32_t)((uint64_t)cyc_mean * 1000000U / hz);
	uint32_t us_min   = (uint32_t)((uint64_t)cyc_min  * 1000000U / hz);
	uint32_t us_max   = (uint32_t)((uint64_t)cyc_max  * 1000000U / hz);
	/* NV12 input bytes per frame = w*h*3/2 */
	const uint64_t in_bytes = (uint64_t)REF_IMG_WIDTH * REF_IMG_HEIGHT
				  * 3U / 2U;
	/* MB/s = bytes / sec / 1e6 = bytes * hz / cycles / 1e6 */
	uint32_t mb_per_s = (uint32_t)((in_bytes * hz)
				       / ((uint64_t)cyc_mean * 1000000U));

	TC_PRINT("Step 4: Results\n");
	TC_PRINT("  Clock frequency: %u Hz\n", hz);
	TC_PRINT("  Input size: %llu bytes (%.2f MB)\n",
		 (unsigned long long)in_bytes, (double)in_bytes / (1024.0 * 1024.0));
	TC_PRINT("  Cycle statistics: mean=%u, min=%u, max=%u\n",
		 cyc_mean, cyc_min, cyc_max);
	TC_PRINT("  Latency statistics: mean=%u us, min=%u us, max=%u us\n",
		 us_mean, us_min, us_max);
	TC_PRINT("  Throughput: %u MB/s, %u FPS\n", mb_per_s,
		 (us_mean > 0) ? (1000000U / us_mean) : 0);
	TC_PRINT("Perf 1280x720 NV12 (q=75, n=%d): "
		 "mean=%u us, min=%u us, max=%u us, throughput=%u MB/s, %u FPS\n",
		 iterations, us_mean, us_min, us_max, mb_per_s,
		 (us_mean > 0) ? (1000000U / us_mean) : 0);

	release_buffers(in, out);
}

/*
 * Performance benchmark at 2MP (1920x1080 FHD) resolution. Measures
 * encode latency and throughput similar to the 1280x720 test but at
 * a higher resolution to assess performance scaling. Auto-skipped if
 * the buffer pool cannot accommodate the larger buffers.
 */
ZTEST(jpeg_perf, test_jpeg_encode_performance_1920x1080)
{
	const uint32_t width  = 1920;
	const uint32_t height = 1080;
	const size_t   nv12_sz = (size_t)width * height * 3 / 2;
	struct video_buffer *in = NULL, *out = NULL, *deq = NULL;
	const int iterations = 10;
	uint32_t cyc_min = UINT32_MAX, cyc_max = 0;
	uint64_t cyc_sum = 0;
	int i, ret;

	in  = video_buffer_alloc(nv12_sz, K_NO_WAIT);
	out = video_buffer_alloc(nv12_sz + JPEG_HEADER_SIZE, K_NO_WAIT);
	if (!in || !out) {
		if (in) {
			video_buffer_release(in);
		}
		if (out) {
			video_buffer_release(out);
		}
		TC_PRINT("Skipping 1920x1080 perf: buffer pool too small\n");
		ztest_test_skip();
		return;
	}

	jpeg_fill_nv12_gradient((uint8_t *)in->buffer, width, height, width);
	out->bytesused = nv12_sz + JPEG_HEADER_SIZE;

	TC_PRINT("Performance test: %ux%u NV12 (2MP), quality=75, iterations=%d\n",
		 width, height, iterations);

	/* Warm-up encode (not measured) to exclude one-time HW init costs. */
	TC_PRINT("Step 1: Warm-up encode (excludes HW init costs)\n");
	ret = do_single_encode(in, out, 75, width, height, width, &deq);
	zassert_equal(ret, 0, "warm-up encode failed: %d", ret);
	TC_PRINT("Warm-up complete: %u bytes encoded\n", deq->bytesused);

	TC_PRINT("Step 2: Timed iterations (n=%d)\n", iterations);
	for (i = 0; i < iterations; i++) {
		uint32_t t0 = k_cycle_get_32();

		ret = do_single_encode(in, out, 75, width, height, width, &deq);
		uint32_t dt = k_cycle_get_32() - t0;

		zassert_equal(ret, 0, "iter %d encode failed: %d", i, ret);
		zassert_true(jpeg_validate_markers((uint8_t *)deq->buffer,
						   deq->bytesused),
			     "iter %d: SOI/EOI missing", i);

		cyc_sum += dt;
		if (dt < cyc_min) {
			cyc_min = dt;
		}
		if (dt > cyc_max) {
			cyc_max = dt;
		}

		uint32_t hz = sys_clock_hw_cycles_per_sec();
		uint32_t us = (uint32_t)((uint64_t)dt * 1000000U / hz);

		TC_PRINT("  Iter %2d: %u cycles (%u us), %u bytes\n",
			 i, dt, us, deq->bytesused);
	}

	zassert_true(cyc_sum > 0, "cycle counter not running");

	TC_PRINT("Step 3: Metrics calculation\n");
	uint32_t cyc_mean = (uint32_t)(cyc_sum / iterations);
	uint32_t hz       = sys_clock_hw_cycles_per_sec();
	uint32_t us_mean  = (uint32_t)((uint64_t)cyc_mean * 1000000U / hz);
	uint32_t us_min   = (uint32_t)((uint64_t)cyc_min  * 1000000U / hz);
	uint32_t us_max   = (uint32_t)((uint64_t)cyc_max  * 1000000U / hz);
	const uint64_t in_bytes = (uint64_t)width * height * 3U / 2U;
	uint32_t mb_per_s = (uint32_t)((in_bytes * hz)
				       / ((uint64_t)cyc_mean * 1000000U));

	TC_PRINT("Step 4: Results\n");
	TC_PRINT("  Clock frequency: %u Hz\n", hz);
	TC_PRINT("  Input size: %llu bytes (%.2f MB)\n",
		 (unsigned long long)in_bytes, (double)in_bytes / (1024.0 * 1024.0));
	TC_PRINT("  Cycle statistics: mean=%u, min=%u, max=%u\n",
		 cyc_mean, cyc_min, cyc_max);
	TC_PRINT("  Latency statistics: mean=%u us, min=%u us, max=%u us\n",
		 us_mean, us_min, us_max);
	TC_PRINT("  Throughput: %u MB/s, %u FPS\n", mb_per_s,
		 (us_mean > 0) ? (1000000U / us_mean) : 0);
	TC_PRINT("Perf 1920x1080 NV12 (q=75, n=%d): "
		 "mean=%u us, min=%u us, max=%u us, throughput=%u MB/s, %u FPS\n",
		 iterations, us_mean, us_min, us_max, mb_per_s,
		 (us_mean > 0) ? (1000000U / us_mean) : 0);

	release_buffers(in, out);
}

ZTEST_SUITE(jpeg_perf, NULL, NULL, jpeg_perf_suite_before, NULL, NULL);

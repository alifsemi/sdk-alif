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

/*
 * Performance benchmark helper: runs warm-up encode, timed iterations,
 * metrics calculation, and prints results. Takes pre-allocated buffers
 * and dimensions as parameters, and a label for output identification.
 */
static void run_perf(uint32_t width, uint32_t height, uint32_t pitch,
		     struct video_buffer *in, struct video_buffer *out,
		     const char *label)
{
	struct video_buffer *deq = NULL;
	const int iterations = 10;
	uint32_t cyc_min = UINT32_MAX, cyc_max = 0;
	uint64_t cyc_sum = 0;
	int i, ret;

	TC_PRINT("Performance test: %ux%u NV12, quality=75, iterations=%d\n",
		 width, height, iterations);

	/* Warm-up encode (not measured) to exclude one-time HW init costs. */
	TC_PRINT("Step 1: Warm-up encode (excludes HW init costs)\n");
	ret = do_single_encode(in, out, 75, width, height, pitch, &deq);
	zassert_equal(ret, 0, "warm-up encode failed: %d", ret);
	TC_PRINT("Warm-up complete: %u bytes encoded\n", deq->bytesused);

	TC_PRINT("Step 2: Timed iterations (n=%d)\n", iterations);
	for (i = 0; i < iterations; i++) {
		uint32_t t0 = k_cycle_get_32();

		ret = do_single_encode(in, out, 75, width, height, pitch, &deq);
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
	const uint64_t in_bytes = (uint64_t)width * height * 3U / 2U;
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
	TC_PRINT("Perf %s NV12 (q=75, n=%d): "
		 "mean=%u us, min=%u us, max=%u us, throughput=%u MB/s, %u FPS\n",
		 label, iterations, us_mean, us_min, us_max, mb_per_s,
		 (us_mean > 0) ? (1000000U / us_mean) : 0);
}

/*
 * Performance benchmark: measures encode latency and throughput at the
 * reference 1280x720 resolution. Uses the shared run_perf helper.
 */
ZTEST(jpeg_perf, test_jpeg_encode_performance_1280x720)
{
	struct video_buffer *in = NULL, *out = NULL;

	alloc_ref_buffers(&in, &out);
	run_perf(REF_IMG_WIDTH, REF_IMG_HEIGHT, REF_IMG_PITCH, in, out,
		 "1280x720");
	release_buffers(in, out);
}

/*
 * Performance benchmark at 2MP (1920x1080 FHD) resolution. Measures
 * encode latency and throughput similar to the 1280x720 test but at
 * a higher resolution to assess performance scaling. Auto-skipped if
 * the buffer pool cannot accommodate the larger buffers. Uses the
 * shared run_perf helper.
 */
ZTEST(jpeg_perf, test_jpeg_encode_performance_1920x1080)
{
	const uint32_t width  = 1920;
	const uint32_t height = 1080;
	const size_t   nv12_sz = (size_t)width * height * 3 / 2;
	struct video_buffer *in = NULL, *out = NULL;

	if (!alloc_pair_or_skip(nv12_sz, nv12_sz + JPEG_HEADER_SIZE,
				&in, &out, "1920x1080_perf")) {
		return;
	}

	jpeg_fill_nv12_gradient((uint8_t *)in->buffer, width, height, width);
	out->bytesused = nv12_sz + JPEG_HEADER_SIZE;

	run_perf(width, height, width, in, out, "1920x1080");
	release_buffers(in, out);
}

ZTEST_SUITE(jpeg_perf, NULL, NULL, jpeg_suite_before, NULL, NULL);

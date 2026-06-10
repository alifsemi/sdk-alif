/*
 * Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * test_i2s_tx_golden.c - Golden-vector TX master loopback test cases.
 *
 * Test suite: i2s_golden_tx
 *
 * Constraints:
 *   - TX master-only (I2S_OPT_BIT_CLK_MASTER | I2S_OPT_FRAME_CLK_MASTER)
 *   - No full-duplex (I2S_DIR_BOTH never used)
 *   - No slave mode
 *   - 5 ZTESTs (one per bit-depth) × 8 sample rates = 40 combinations,
 *     identical coverage to the previous 40-ZTEST design.
 *
 * Each ZTEST:
 *   1. Gets the I2S device from DT alias i2s_node0.
 *   2. Iterates over all 8 sample rates in gv_rates[].
 *   3. Calls i2s_golden_run() for each rate.
 *   4. Skips unsupported rates (-ENOTSUP / -ENOSYS) without failing.
 *   5. Counts genuine failures; the ZTEST fails only if n_fail > 0.
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>
#include <zephyr/device.h>
#include "i2s_golden_test.h"

LOG_MODULE_REGISTER(i2s_golden_tx_test, LOG_LEVEL_INF);

/* -------------------------------------------------------------------------
 * Rate table — iterated in every ZTEST (mirrors test_i2s_loopback.c)
 * -------------------------------------------------------------------------
 */
static const uint32_t gv_rates[] = {
	8000U, 16000U, 32000U, 44100U,
	48000U, 88200U, 96000U, 192000U
};

/* -------------------------------------------------------------------------
 * Suite setup: verify the I2S device is present and accessible.
 * -------------------------------------------------------------------------
 */
static void *i2s_golden_setup(void)
{
	const struct device *dev = DEVICE_DT_GET(I2S_GOLDEN_NODE);

	zassert_not_null(dev, "I2S device handle is NULL");
	zassert_true(device_is_ready(dev),
		     "I2S device '%s' is not ready", dev->name);

	LOG_INF("I2S golden-vector suite: device '%s' ready\n", dev->name);
	LOG_INF("  Block size : %u bytes\n", (unsigned int)I2S_GOLDEN_BLOCK_SIZE);
	LOG_INF("  Frames/blk : %u\n",      (unsigned int)I2S_GOLDEN_FRAMES);
	LOG_INF("  Channels   : %u\n",      (unsigned int)I2S_GOLDEN_CHANNELS);
	LOG_INF("  Rates      : %u\n",      (unsigned int)ARRAY_SIZE(gv_rates));

	return NULL;
}

/* -------------------------------------------------------------------------
 * Suite teardown: nothing to free (static slabs).
 * -------------------------------------------------------------------------
 */
static void i2s_golden_teardown(void *fixture)
{
	ARG_UNUSED(fixture);

	const struct device *dev = DEVICE_DT_GET(I2S_GOLDEN_NODE);
	struct i2s_config zero = { 0 };

	(void)i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_DROP);
	(void)i2s_trigger(dev, I2S_DIR_RX, I2S_TRIGGER_DROP);
	(void)i2s_configure(dev, I2S_DIR_TX, &zero);
	(void)i2s_configure(dev, I2S_DIR_RX, &zero);
}

/* -------------------------------------------------------------------------
 * Suite: i2s_golden_tx
 *   5 ZTESTs — one per bit-depth, looping over all 8 sample rates.
 * -------------------------------------------------------------------------
 */
ZTEST_SUITE(i2s_golden_tx, NULL, i2s_golden_setup, NULL, NULL,
	    i2s_golden_teardown);

/* =========================================================================
 * 5 ZTESTs — one per bit-depth, iterating over gv_rates[].
 *
 * Per-rate -ENOTSUP / -ENOSYS is a skip (continue); any other non-zero
 * return is a genuine failure counted in n_fail.  The ZTEST fails only
 * when n_fail > 0, so the overall pass/fail still reflects all 40
 * rate×depth combinations.
 * =========================================================================
 */

ZTEST(i2s_golden_tx, test_tx_12bit)
{
	const struct device *dev = DEVICE_DT_GET(I2S_GOLDEN_NODE);
	uint32_t n_fail = 0U;

	zassert_true(device_is_ready(dev), "I2S device not ready");

	for (uint32_t i = 0U; i < ARRAY_SIZE(gv_rates); i++) {
		int rc = i2s_golden_run(dev, gv_rates[i], 12U,
					i2s_golden_12bit_L, i2s_golden_12bit_R);

		if (rc == -ENOTSUP || rc == -ENOSYS) {
			LOG_INF("[gv-12] SKIP rate=%u — not supported\n",
				 gv_rates[i]);
			continue;
		}
		if (rc != 0) {
			LOG_INF("[gv-12] FAIL rate=%u rc=%d\n",
				 gv_rates[i], rc);
			n_fail++;
		}
	}
	zassert_equal(n_fail, 0U, "[gv-12] %u rate(s) failed", n_fail);
}

ZTEST(i2s_golden_tx, test_tx_16bit)
{
	const struct device *dev = DEVICE_DT_GET(I2S_GOLDEN_NODE);
	uint32_t n_fail = 0U;

	zassert_true(device_is_ready(dev), "I2S device not ready");

	for (uint32_t i = 0U; i < ARRAY_SIZE(gv_rates); i++) {
		int rc = i2s_golden_run(dev, gv_rates[i], 16U,
					i2s_golden_16bit_L, i2s_golden_16bit_R);

		if (rc == -ENOTSUP || rc == -ENOSYS) {
			LOG_INF("[gv-16] SKIP rate=%u — not supported\n",
				 gv_rates[i]);
			continue;
		}
		if (rc != 0) {
			LOG_INF("[gv-16] FAIL rate=%u rc=%d\n",
				 gv_rates[i], rc);
			n_fail++;
		}
	}
	zassert_equal(n_fail, 0U, "[gv-16] %u rate(s) failed", n_fail);
}

ZTEST(i2s_golden_tx, test_tx_20bit)
{
	const struct device *dev = DEVICE_DT_GET(I2S_GOLDEN_NODE);
	uint32_t n_fail = 0U;

	zassert_true(device_is_ready(dev), "I2S device not ready");

	for (uint32_t i = 0U; i < ARRAY_SIZE(gv_rates); i++) {
		int rc = i2s_golden_run(dev, gv_rates[i], 20U,
					i2s_golden_20bit_L, i2s_golden_20bit_R);

		if (rc == -ENOTSUP || rc == -ENOSYS) {
			LOG_INF("[gv-20] SKIP rate=%u — not supported\n",
				 gv_rates[i]);
			continue;
		}
		if (rc != 0) {
			LOG_INF("[gv-20] FAIL rate=%u rc=%d\n",
				 gv_rates[i], rc);
			n_fail++;
		}
	}
	zassert_equal(n_fail, 0U, "[gv-20] %u rate(s) failed", n_fail);
}

ZTEST(i2s_golden_tx, test_tx_24bit)
{
	const struct device *dev = DEVICE_DT_GET(I2S_GOLDEN_NODE);
	uint32_t n_fail = 0U;

	zassert_true(device_is_ready(dev), "I2S device not ready");

	for (uint32_t i = 0U; i < ARRAY_SIZE(gv_rates); i++) {
		int rc = i2s_golden_run(dev, gv_rates[i], 24U,
					i2s_golden_24bit_L, i2s_golden_24bit_R);

		if (rc == -ENOTSUP || rc == -ENOSYS) {
			LOG_INF("[gv-24] SKIP rate=%u — not supported\n",
				 gv_rates[i]);
			continue;
		}
		if (rc != 0) {
			LOG_INF("[gv-24] FAIL rate=%u rc=%d\n",
				 gv_rates[i], rc);
			n_fail++;
		}
	}
	zassert_equal(n_fail, 0U, "[gv-24] %u rate(s) failed", n_fail);
}

ZTEST(i2s_golden_tx, test_tx_32bit)
{
	const struct device *dev = DEVICE_DT_GET(I2S_GOLDEN_NODE);
	uint32_t n_fail = 0U;

	zassert_true(device_is_ready(dev), "I2S device not ready");

	for (uint32_t i = 0U; i < ARRAY_SIZE(gv_rates); i++) {
		int rc = i2s_golden_run(dev, gv_rates[i], 32U,
					i2s_golden_32bit_L, i2s_golden_32bit_R);

		if (rc == -ENOTSUP || rc == -ENOSYS) {
			LOG_INF("[gv-32] SKIP rate=%u — not supported\n",
				 gv_rates[i]);
			continue;
		}
		if (rc != 0) {
			LOG_INF("[gv-32] FAIL rate=%u rc=%d\n",
				 gv_rates[i], rc);
			n_fail++;
		}
	}
	zassert_equal(n_fail, 0U, "[gv-32] %u rate(s) failed", n_fail);
}

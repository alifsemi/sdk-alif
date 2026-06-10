/*
 * Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * test_i2s_negative.c - I2S negative state-machine test suite.
 *
 * Suite: i2s_negative
 *
 * Test cases (9 total):
 *
 *   test_neg_not_ready_triggers         - all triggers rejected in NOT_READY
 *   test_neg_ready_invalid_triggers     - DRAIN/STOP/PREPARE rejected in READY
 *   test_neg_tx_start_empty_queue       - START with empty TX queue fails
 *   test_neg_tx_double_start            - second START while RUNNING fails
 *   test_neg_tx_drop_in_not_ready       - DROP in NOT_READY fails
 *   test_neg_tx_zero_freq_not_ready     - zero-freq configure → NOT_READY
 *   test_neg_tx_stop_from_ready         - STOP from READY fails
 *   test_neg_rx_configure_ready_drop    - RX configure→READY then DROP succeeds
 *   test_neg_tx_prepare_from_error      - PREPARE acceptable after DROP
 */

#include <zephyr/drivers/i2s.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/ztest.h>
#include "i2s_test.h"

LOG_MODULE_REGISTER(i2s_negative_test, LOG_LEVEL_INF);

/* -------------------------------------------------------------------------
 * Helper: reset TX (and RX) to NOT_READY state
 * -------------------------------------------------------------------------
 */
static void neg_reset_streams(const struct device *dev)
{
	struct i2s_config zero = { 0 };

	(void)i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_DROP);
	(void)i2s_trigger(dev, I2S_DIR_RX, I2S_TRIGGER_DROP);
	(void)i2s_configure(dev, I2S_DIR_TX, &zero);
	(void)i2s_configure(dev, I2S_DIR_RX, &zero);
	k_sleep(K_MSEC(2));
}

/* -------------------------------------------------------------------------
 * All triggers rejected in NOT_READY state
 * Tests both TX and RX directions.
 * -------------------------------------------------------------------------
 */
ZTEST(i2s_negative, test_neg_not_ready_triggers)
{
	const struct device *const dev = DEVICE_DT_GET(I2S_GOLDEN_NODE);

	zassert_true(device_is_ready(dev), "I2S device not ready");
	neg_reset_streams(dev);

	/* RX in NOT_READY */
	zassert_equal(i2s_trigger(dev, I2S_DIR_RX, I2S_TRIGGER_START),  -EIO,
		      "RX START in NOT_READY should return -EIO");
	zassert_equal(i2s_trigger(dev, I2S_DIR_RX, I2S_TRIGGER_DRAIN),  -EIO,
		      "RX DRAIN in NOT_READY should return -EIO");
	zassert_equal(i2s_trigger(dev, I2S_DIR_RX, I2S_TRIGGER_STOP),   -EIO,
		      "RX STOP in NOT_READY should return -EIO");
	zassert_equal(i2s_trigger(dev, I2S_DIR_RX, I2S_TRIGGER_DROP),   -EIO,
		      "RX DROP in NOT_READY should return -EIO");
	zassert_equal(i2s_trigger(dev, I2S_DIR_RX, I2S_TRIGGER_PREPARE), -EIO,
		      "RX PREPARE in NOT_READY should return -EIO");

	/* TX in NOT_READY */
	zassert_equal(i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_START),  -EIO,
		      "TX START in NOT_READY should return -EIO");
	zassert_equal(i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_DRAIN),  -EIO,
		      "TX DRAIN in NOT_READY should return -EIO");
	zassert_equal(i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_STOP),   -EIO,
		      "TX STOP in NOT_READY should return -EIO");
	zassert_equal(i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_DROP),   -EIO,
		      "TX DROP in NOT_READY should return -EIO");
	zassert_equal(i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_PREPARE), -EIO,
		      "TX PREPARE in NOT_READY should return -EIO");

	LOG_INF("PASS: all NOT_READY triggers correctly rejected\n");
}

/* -------------------------------------------------------------------------
 * DRAIN, STOP, PREPARE rejected in READY state
 * -------------------------------------------------------------------------
 */
ZTEST(i2s_negative, test_neg_ready_invalid_triggers)
{
	const struct device *const dev = DEVICE_DT_GET(I2S_GOLDEN_NODE);
	int ret;

	zassert_true(device_is_ready(dev), "I2S device not ready");
	neg_reset_streams(dev);

	/* Configure RX → READY */
	ret = i2s_golden_configure(dev, I2S_DIR_RX, 48000U, 16U);
	if (ret == -ENOTSUP || ret == -ENOSYS) {
		ztest_test_skip();
		return;
	}
	zassert_equal(ret, 0, "RX configure failed: %d", ret);

	zassert_equal(i2s_trigger(dev, I2S_DIR_RX, I2S_TRIGGER_DRAIN),   -EIO,
		      "RX DRAIN in READY should return -EIO");
	zassert_equal(i2s_trigger(dev, I2S_DIR_RX, I2S_TRIGGER_STOP),    -EIO,
		      "RX STOP in READY should return -EIO");
	zassert_equal(i2s_trigger(dev, I2S_DIR_RX, I2S_TRIGGER_PREPARE), -EIO,
		      "RX PREPARE in READY should return -EIO");

	/* Configure TX → READY */
	ret = i2s_golden_configure(dev, I2S_DIR_TX, 48000U, 16U);
	zassert_equal(ret, 0, "TX configure failed: %d", ret);

	zassert_equal(i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_DRAIN),   -EIO,
		      "TX DRAIN in READY should return -EIO");
	zassert_equal(i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_STOP),    -EIO,
		      "TX STOP in READY should return -EIO");
	zassert_equal(i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_PREPARE), -EIO,
		      "TX PREPARE in READY should return -EIO");

	LOG_INF("PASS: READY-state invalid triggers correctly rejected\n");
	neg_reset_streams(dev);
}

/* -------------------------------------------------------------------------
 * START with empty TX queue fails
 * -------------------------------------------------------------------------
 */
ZTEST(i2s_negative, test_neg_tx_start_empty_queue)
{
	const struct device *const dev = DEVICE_DT_GET(I2S_GOLDEN_NODE);

	zassert_true(device_is_ready(dev), "I2S device not ready");
	neg_reset_streams(dev);

	int ret = i2s_golden_configure(dev, I2S_DIR_TX, 48000U, 16U);

	if (ret == -ENOTSUP || ret == -ENOSYS) {
		ztest_test_skip();
		return;
	}
	zassert_equal(ret, 0, "TX configure failed: %d", ret);

	ret = i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_START);
	zassert_not_equal(ret, 0,
			  "TX START with empty queue should fail (got 0)");

	LOG_INF("PASS: TX START with empty queue returned %d (expected non-zero)\n",
		 ret);
	neg_reset_streams(dev);
}

/* -------------------------------------------------------------------------
 * Double START while RUNNING fails
 * -------------------------------------------------------------------------
 */
ZTEST(i2s_negative, test_neg_tx_double_start)
{
	const struct device *const dev = DEVICE_DT_GET(I2S_GOLDEN_NODE);

	zassert_true(device_is_ready(dev), "I2S device not ready");
	neg_reset_streams(dev);

	int ret = i2s_golden_configure(dev, I2S_DIR_TX, 48000U, 16U);

	if (ret == -ENOTSUP || ret == -ENOSYS) {
		ztest_test_skip();
		return;
	}
	zassert_equal(ret, 0, "TX configure failed: %d", ret);

	for (int i = 0; i < 2; i++) {
		ret = i2s_golden_tx_write(dev, 16U,
					  i2s_golden_16bit_L, i2s_golden_16bit_R);
		zassert_equal(ret, 0, "TX write[%d] failed: %d", i, ret);
	}

	ret = i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_START);
	if (ret != 0) {
		LOG_INF("SKIP: first TX START failed (%d)\n", ret);
		neg_reset_streams(dev);
		ztest_test_skip();
		return;
	}

	ret = i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_START);
	zassert_not_equal(ret, 0,
			  "Second TX START while RUNNING should fail (got 0)");

	LOG_INF("PASS: second START while RUNNING returned %d (expected non-zero)\n",
		 ret);
	neg_reset_streams(dev);
}

/* -------------------------------------------------------------------------
 * DROP in NOT_READY state fails
 * -------------------------------------------------------------------------
 */
ZTEST(i2s_negative, test_neg_tx_drop_in_not_ready)
{
	const struct device *const dev = DEVICE_DT_GET(I2S_GOLDEN_NODE);

	zassert_true(device_is_ready(dev), "I2S device not ready");
	neg_reset_streams(dev);

	int ret = i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_DROP);

	zassert_not_equal(ret, 0,
			  "DROP in NOT_READY should fail (got 0)");

	LOG_INF("PASS: DROP in NOT_READY returned %d (expected non-zero)\n", ret);
}

/* -------------------------------------------------------------------------
 * Configure with frame_clk_freq=0 transitions stream to NOT_READY
 * -------------------------------------------------------------------------
 */
ZTEST(i2s_negative, test_neg_tx_zero_freq_not_ready)
{
	const struct device *const dev = DEVICE_DT_GET(I2S_GOLDEN_NODE);

	zassert_true(device_is_ready(dev), "I2S device not ready");
	neg_reset_streams(dev);

	int ret = i2s_golden_configure(dev, I2S_DIR_TX, 48000U, 16U);

	if (ret == -ENOTSUP || ret == -ENOSYS) {
		ztest_test_skip();
		return;
	}
	zassert_equal(ret, 0, "TX configure (48kHz/16b) failed: %d", ret);

	struct i2s_config zero_freq_cfg = {
		.word_size      = 16U,
		.channels       = I2S_GOLDEN_CHANNELS,
		.format         = I2S_FMT_DATA_FORMAT_I2S,
		.frame_clk_freq = 0U,
		.block_size     = I2S_GOLDEN_BLOCK_SIZE,
		.timeout        = I2S_GOLDEN_TIMEOUT_MS,
		.options        = I2S_OPT_BIT_CLK_MASTER | I2S_OPT_FRAME_CLK_MASTER,
		.mem_slab       = &g_tx_slab,
	};

	ret = i2s_configure(dev, I2S_DIR_TX, &zero_freq_cfg);
	zassert_equal(ret, 0, "zero-freq configure failed: %d", ret);

	ret = i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_DROP);
	zassert_not_equal(ret, 0,
			  "DROP after zero-freq configure should fail (NOT_READY)");

	LOG_INF("PASS: DROP after zero-freq configure returned %d (expected non-zero)\n",
		 ret);
}

/* -------------------------------------------------------------------------
 * STOP from READY state fails
 * -------------------------------------------------------------------------
 */
ZTEST(i2s_negative, test_neg_tx_stop_from_ready)
{
	const struct device *const dev = DEVICE_DT_GET(I2S_GOLDEN_NODE);

	zassert_true(device_is_ready(dev), "I2S device not ready");
	neg_reset_streams(dev);

	int ret = i2s_golden_configure(dev, I2S_DIR_TX, 48000U, 16U);

	if (ret == -ENOTSUP || ret == -ENOSYS) {
		ztest_test_skip();
		return;
	}
	zassert_equal(ret, 0, "TX configure failed: %d", ret);

	ret = i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_STOP);
	zassert_not_equal(ret, 0,
			  "STOP from READY should fail (got 0)");

	LOG_INF("PASS: STOP from READY returned %d (expected non-zero)\n", ret);
	neg_reset_streams(dev);
}

/* -------------------------------------------------------------------------
 * RX configure → READY, then DROP succeeds (valid no-op)
 * -------------------------------------------------------------------------
 */
ZTEST(i2s_negative, test_neg_rx_configure_ready_drop)
{
	const struct device *const dev = DEVICE_DT_GET(I2S_GOLDEN_NODE);

	zassert_true(device_is_ready(dev), "I2S device not ready");
	neg_reset_streams(dev);

	int ret = i2s_golden_configure(dev, I2S_DIR_RX, 48000U, 16U);

	if (ret == -ENOTSUP || ret == -ENOSYS) {
		ztest_test_skip();
		return;
	}
	zassert_equal(ret, 0, "RX configure failed: %d", ret);

	ret = i2s_trigger(dev, I2S_DIR_RX, I2S_TRIGGER_DROP);
	zassert_equal(ret, 0, "DROP on RX in READY failed: %d", ret);

	LOG_INF("PASS: RX configure→READY then DROP succeeded\n");
	neg_reset_streams(dev);
}

/* -------------------------------------------------------------------------
 * PREPARE trigger behavior after forced DROP
 * Writes one block, starts, lets it run briefly, then DROPs.
 * PREPARE is then issued — either 0 (was in ERROR) or -EIO (was in READY)
 * are both acceptable; the test verifies the driver does not crash.
 * -------------------------------------------------------------------------
 */
ZTEST(i2s_negative, test_neg_tx_prepare_from_error)
{
	const struct device *const dev = DEVICE_DT_GET(I2S_GOLDEN_NODE);

	zassert_true(device_is_ready(dev), "I2S device not ready");
	neg_reset_streams(dev);

	int ret = i2s_golden_configure(dev, I2S_DIR_TX, 48000U, 16U);

	if (ret == -ENOTSUP || ret == -ENOSYS) {
		ztest_test_skip();
		return;
	}
	zassert_equal(ret, 0, "TX configure failed: %d", ret);

	ret = i2s_golden_tx_write(dev, 16U, i2s_golden_16bit_L, i2s_golden_16bit_R);
	zassert_equal(ret, 0, "TX write failed: %d", ret);

	ret = i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_START);
	if (ret != 0) {
		LOG_INF("SKIP: TX START failed (%d)\n", ret);
		neg_reset_streams(dev);
		ztest_test_skip();
		return;
	}

	k_sleep(K_MSEC(2));
	(void)i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_DROP);

	ret = i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_PREPARE);
	LOG_INF("PASS: PREPARE after DROP returned %d "
		 "(0=was in ERROR, -EIO=was in READY — both acceptable)\n", ret);

	neg_reset_streams(dev);
}

/* =========================================================================
 * Suite registration
 * =========================================================================
 */
ZTEST_SUITE(i2s_negative, NULL, NULL, NULL, NULL, NULL);

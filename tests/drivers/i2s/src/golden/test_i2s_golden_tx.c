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
 *   - 8 sample rates × 5 bit depths = 40 test cases
 *
 * Each ZTEST case:
 *   1. Gets the I2S device from DT alias i2s_node0.
 *   2. Calls i2s_golden_run() which configures TX+RX separately, pumps the
 *      golden sine vector through hardware loopback, and verifies bit-exact
 *      match on the RX side.
 *   3. Skips (ztest_test_skip) if the driver reports -ENOTSUP / -ENOSYS for
 *      the requested rate or bit-depth combination.
 *   4. Fails with a detailed mismatch print on any data error.
 */

#include <zephyr/kernel.h>
#include <zephyr/ztest.h>
#include <zephyr/device.h>
#include "i2s_golden_test.h"

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

	TC_PRINT("I2S golden-vector suite: device '%s' ready\n", dev->name);
	TC_PRINT("  Block size : %u bytes\n", (unsigned int)I2S_GOLDEN_BLOCK_SIZE);
	TC_PRINT("  Frames/blk : %u\n",      (unsigned int)I2S_GOLDEN_FRAMES);
	TC_PRINT("  Channels   : %u\n",      (unsigned int)I2S_GOLDEN_CHANNELS);
	TC_PRINT("  Test matrix: %u cases\n", (unsigned int)i2s_golden_matrix_len);

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
 *   Golden-vector TX master loopback (40 test cases)
 * -------------------------------------------------------------------------
 */
ZTEST_SUITE(i2s_golden_tx, NULL, i2s_golden_setup, NULL, NULL,
	    i2s_golden_teardown);

/* Convenience macro — avoids repeating boilerplate in every test body */
#define I2S_GOLDEN_RUN_TC(_rate, _bits, _vecL, _vecR)                        \
	do {                                                                  \
		const struct device *dev =                                    \
			DEVICE_DT_GET(I2S_GOLDEN_NODE);                       \
		zassert_true(device_is_ready(dev), "I2S device not ready");   \
		int _rc = i2s_golden_run(dev, (_rate), (_bits),               \
					 (_vecL), (_vecR));               \
		if (_rc == -ENOTSUP) {                                        \
			TC_PRINT("SKIP rate=%u bits=%u — not supported\n",    \
				 (unsigned int)(_rate), (unsigned int)(_bits));       \
			ztest_test_skip();                                    \
		}                                                             \
		zassert_equal(_rc, 0,                                         \
			      "golden TX/RX failed rate=%u bits=%u rc=%d",   \
			      (unsigned int)(_rate), (unsigned int)(_bits), _rc);    \
	} while (0)

/* =========================================================================
 * 8 kHz
 * =========================================================================
 */
ZTEST(i2s_golden_tx, test_tx_8kHz_12bit)
{
	I2S_GOLDEN_RUN_TC(8000U, 12U, i2s_golden_12bit_L, i2s_golden_12bit_R);
}

ZTEST(i2s_golden_tx, test_tx_8kHz_16bit)
{
	I2S_GOLDEN_RUN_TC(8000U, 16U, i2s_golden_16bit_L, i2s_golden_16bit_R);
}

ZTEST(i2s_golden_tx, test_tx_8kHz_20bit)
{
	I2S_GOLDEN_RUN_TC(8000U, 20U, i2s_golden_20bit_L, i2s_golden_20bit_R);
}

ZTEST(i2s_golden_tx, test_tx_8kHz_24bit)
{
	I2S_GOLDEN_RUN_TC(8000U, 24U, i2s_golden_24bit_L, i2s_golden_24bit_R);
}

ZTEST(i2s_golden_tx, test_tx_8kHz_32bit)
{
	I2S_GOLDEN_RUN_TC(8000U, 32U, i2s_golden_32bit_L, i2s_golden_32bit_R);
}

/* =========================================================================
 * 16 kHz
 * =========================================================================
 */
ZTEST(i2s_golden_tx, test_tx_16kHz_12bit)
{
	I2S_GOLDEN_RUN_TC(16000U, 12U, i2s_golden_12bit_L, i2s_golden_12bit_R);
}

ZTEST(i2s_golden_tx, test_tx_16kHz_16bit)
{
	I2S_GOLDEN_RUN_TC(16000U, 16U, i2s_golden_16bit_L, i2s_golden_16bit_R);
}

ZTEST(i2s_golden_tx, test_tx_16kHz_20bit)
{
	I2S_GOLDEN_RUN_TC(16000U, 20U, i2s_golden_20bit_L, i2s_golden_20bit_R);
}

ZTEST(i2s_golden_tx, test_tx_16kHz_24bit)
{
	I2S_GOLDEN_RUN_TC(16000U, 24U, i2s_golden_24bit_L, i2s_golden_24bit_R);
}

ZTEST(i2s_golden_tx, test_tx_16kHz_32bit)
{
	I2S_GOLDEN_RUN_TC(16000U, 32U, i2s_golden_32bit_L, i2s_golden_32bit_R);
}

/* =========================================================================
 * 32 kHz
 * =========================================================================
 */
ZTEST(i2s_golden_tx, test_tx_32kHz_12bit)
{
	I2S_GOLDEN_RUN_TC(32000U, 12U, i2s_golden_12bit_L, i2s_golden_12bit_R);
}

ZTEST(i2s_golden_tx, test_tx_32kHz_16bit)
{
	I2S_GOLDEN_RUN_TC(32000U, 16U, i2s_golden_16bit_L, i2s_golden_16bit_R);
}

ZTEST(i2s_golden_tx, test_tx_32kHz_20bit)
{
	I2S_GOLDEN_RUN_TC(32000U, 20U, i2s_golden_20bit_L, i2s_golden_20bit_R);
}

ZTEST(i2s_golden_tx, test_tx_32kHz_24bit)
{
	I2S_GOLDEN_RUN_TC(32000U, 24U, i2s_golden_24bit_L, i2s_golden_24bit_R);
}

ZTEST(i2s_golden_tx, test_tx_32kHz_32bit)
{
	I2S_GOLDEN_RUN_TC(32000U, 32U, i2s_golden_32bit_L, i2s_golden_32bit_R);
}

/* =========================================================================
 * 44.1 kHz
 * =========================================================================
 */
ZTEST(i2s_golden_tx, test_tx_44k1Hz_12bit)
{
	I2S_GOLDEN_RUN_TC(44100U, 12U, i2s_golden_12bit_L, i2s_golden_12bit_R);
}

ZTEST(i2s_golden_tx, test_tx_44k1Hz_16bit)
{
	I2S_GOLDEN_RUN_TC(44100U, 16U, i2s_golden_16bit_L, i2s_golden_16bit_R);
}

ZTEST(i2s_golden_tx, test_tx_44k1Hz_20bit)
{
	I2S_GOLDEN_RUN_TC(44100U, 20U, i2s_golden_20bit_L, i2s_golden_20bit_R);
}

ZTEST(i2s_golden_tx, test_tx_44k1Hz_24bit)
{
	I2S_GOLDEN_RUN_TC(44100U, 24U, i2s_golden_24bit_L, i2s_golden_24bit_R);
}

ZTEST(i2s_golden_tx, test_tx_44k1Hz_32bit)
{
	I2S_GOLDEN_RUN_TC(44100U, 32U, i2s_golden_32bit_L, i2s_golden_32bit_R);
}

/* =========================================================================
 * 48 kHz
 * =========================================================================
 */
ZTEST(i2s_golden_tx, test_tx_48kHz_12bit)
{
	I2S_GOLDEN_RUN_TC(48000U, 12U, i2s_golden_12bit_L, i2s_golden_12bit_R);
}

ZTEST(i2s_golden_tx, test_tx_48kHz_16bit)
{
	I2S_GOLDEN_RUN_TC(48000U, 16U, i2s_golden_16bit_L, i2s_golden_16bit_R);
}

ZTEST(i2s_golden_tx, test_tx_48kHz_20bit)
{
	I2S_GOLDEN_RUN_TC(48000U, 20U, i2s_golden_20bit_L, i2s_golden_20bit_R);
}

ZTEST(i2s_golden_tx, test_tx_48kHz_24bit)
{
	I2S_GOLDEN_RUN_TC(48000U, 24U, i2s_golden_24bit_L, i2s_golden_24bit_R);
}

ZTEST(i2s_golden_tx, test_tx_48kHz_32bit)
{
	I2S_GOLDEN_RUN_TC(48000U, 32U, i2s_golden_32bit_L, i2s_golden_32bit_R);
}

/* =========================================================================
 * 88.2 kHz
 * =========================================================================
 */
ZTEST(i2s_golden_tx, test_tx_88k2Hz_12bit)
{
	I2S_GOLDEN_RUN_TC(88200U, 12U, i2s_golden_12bit_L, i2s_golden_12bit_R);
}

ZTEST(i2s_golden_tx, test_tx_88k2Hz_16bit)
{
	I2S_GOLDEN_RUN_TC(88200U, 16U, i2s_golden_16bit_L, i2s_golden_16bit_R);
}

ZTEST(i2s_golden_tx, test_tx_88k2Hz_20bit)
{
	I2S_GOLDEN_RUN_TC(88200U, 20U, i2s_golden_20bit_L, i2s_golden_20bit_R);
}

ZTEST(i2s_golden_tx, test_tx_88k2Hz_24bit)
{
	I2S_GOLDEN_RUN_TC(88200U, 24U, i2s_golden_24bit_L, i2s_golden_24bit_R);
}

ZTEST(i2s_golden_tx, test_tx_88k2Hz_32bit)
{
	I2S_GOLDEN_RUN_TC(88200U, 32U, i2s_golden_32bit_L, i2s_golden_32bit_R);
}

/* =========================================================================
 * 96 kHz
 * =========================================================================
 */
ZTEST(i2s_golden_tx, test_tx_96kHz_12bit)
{
	I2S_GOLDEN_RUN_TC(96000U, 12U, i2s_golden_12bit_L, i2s_golden_12bit_R);
}

ZTEST(i2s_golden_tx, test_tx_96kHz_16bit)
{
	I2S_GOLDEN_RUN_TC(96000U, 16U, i2s_golden_16bit_L, i2s_golden_16bit_R);
}

ZTEST(i2s_golden_tx, test_tx_96kHz_20bit)
{
	I2S_GOLDEN_RUN_TC(96000U, 20U, i2s_golden_20bit_L, i2s_golden_20bit_R);
}

ZTEST(i2s_golden_tx, test_tx_96kHz_24bit)
{
	I2S_GOLDEN_RUN_TC(96000U, 24U, i2s_golden_24bit_L, i2s_golden_24bit_R);
}

ZTEST(i2s_golden_tx, test_tx_96kHz_32bit)
{
	I2S_GOLDEN_RUN_TC(96000U, 32U, i2s_golden_32bit_L, i2s_golden_32bit_R);
}

/* =========================================================================
 * 192 kHz
 * =========================================================================
 */
ZTEST(i2s_golden_tx, test_tx_192kHz_12bit)
{
	I2S_GOLDEN_RUN_TC(192000U, 12U, i2s_golden_12bit_L, i2s_golden_12bit_R);
}

ZTEST(i2s_golden_tx, test_tx_192kHz_16bit)
{
	I2S_GOLDEN_RUN_TC(192000U, 16U, i2s_golden_16bit_L, i2s_golden_16bit_R);
}

ZTEST(i2s_golden_tx, test_tx_192kHz_20bit)
{
	I2S_GOLDEN_RUN_TC(192000U, 20U, i2s_golden_20bit_L, i2s_golden_20bit_R);
}

ZTEST(i2s_golden_tx, test_tx_192kHz_24bit)
{
	I2S_GOLDEN_RUN_TC(192000U, 24U, i2s_golden_24bit_L, i2s_golden_24bit_R);
}

ZTEST(i2s_golden_tx, test_tx_192kHz_32bit)
{
	I2S_GOLDEN_RUN_TC(192000U, 32U, i2s_golden_32bit_L, i2s_golden_32bit_R);
}

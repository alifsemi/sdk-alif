/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/sys/util_macro.h>

#if IS_ENABLED(CONFIG_TEST_SPI_NEGATIVE)

#include "test_spi_common.h"

LOG_MODULE_REGISTER(spi_test_neg, LOG_LEVEL_INF);

static const struct device *const controller_dev =
	DEVICE_DT_GET(SPI_CONTROLLER_NODE);

/* ------------------------------------------------------------------
 * Helpers
 * ------------------------------------------------------------------
 */
static struct spi_config neg_test_cfg(uint32_t extra_flags,
				      uint32_t word_size,
				      uint32_t freq_hz)
{
	struct spi_config cfg = {
		.frequency = freq_hz,
		.operation = (spi_operation_t)(
			SPI_OP_MODE_MASTER |
			extra_flags | SPI_WORD_SET(word_size)),
		.slave = 0,
	};

	return cfg;
}

static int do_loopback_xfer(const struct spi_config *cfg,
			    void *tx, size_t tx_len,
			    void *rx, size_t rx_len)
{
	struct spi_buf tb = { .buf = tx, .len = tx_len };
	struct spi_buf rb = { .buf = rx, .len = rx_len };
	struct spi_buf_set ts = { .buffers = &tb, .count = 1 };
	struct spi_buf_set rs = { .buffers = &rb, .count = 1 };

	struct spi_buf_set *txs = tx_len ? &ts : NULL;
	struct spi_buf_set *rxs = rx_len ? &rs : NULL;

	return spi_test_transceive(controller_dev, cfg, txs, rxs, false);
}

/* ------------------------------------------------------------------
 * Suite-level before function: shared setup + device-ready check
 * ------------------------------------------------------------------
 */
static void test_spi_negative_before(void *fixture)
{
	static struct spi_config cfg;

	cfg = neg_test_cfg(SPI_TRANSFER_LSB,
					     8U, SPI_FREQ_MHZ);
	uint8_t tx[4] = {0x11, 0x22, 0x33, 0x44};
	uint8_t rx[4];
	struct spi_config cfg = neg_test_cfg(SPI_TRANSFER_LSB, 8U, SPI_FREQ_MHZ);
	int ret = do_loopback_xfer(&cfg, tx, sizeof(tx), rx, sizeof(rx));

	zassert_equal(ret, -EINVAL,
		      "LSB-first: expected -EINVAL, got %d", ret);
}

ZTEST(test_spi_negative, test_release_wrong_cfg)
{
	static struct spi_config cfg1;

	cfg1 = neg_test_cfg(0, 8U, SPI_FREQ_MHZ);
	uint8_t tx[4] = {0xDE, 0xAD, 0xBE, 0xEF};
	uint8_t rx[4];
	int ret;

	ret = do_loopback_xfer(&cfg1, tx, sizeof(tx), rx, sizeof(rx));
	zassert_true(ret >= 0,
		     "release_wrong: initial xfer failed %d", ret);

	static struct spi_config cfg2;

	cfg2 = neg_test_cfg(0, 16U, 2 * SPI_FREQ_MHZ);

	ret = spi_release(controller_dev, &cfg2);
	zassert_equal(ret, -EINVAL,
		      "release_wrong: expected -EINVAL, got %d", ret);
}

ZTEST(test_spi_negative, test_freq_exceed)
{
	struct spi_config cfg = neg_test_cfg(0, 8U, UINT32_MAX);
	uint8_t tx[4] = {0x11, 0x22, 0x33, 0x44};
	uint8_t rx[4];
	struct spi_config cfg = neg_test_cfg(0, 8U, UINT32_MAX);
	int ret = do_loopback_xfer(&cfg, tx, sizeof(tx), rx, sizeof(rx));

	zassert_equal(ret, -EINVAL,
		      "freq_exceed: expected -EINVAL, got %d", ret);
}

static int subtest_null_buf(void)
{
	static struct spi_config cfg;

	cfg = neg_test_cfg(SPI_OP_MODE_SLAVE,
					     8U, SPI_FREQ_MHZ);
	uint8_t tx[4] = {0x11, 0x22, 0x33, 0x44};
	uint8_t rx[4];
	int ret = do_loopback_xfer(&cfg, tx, sizeof(tx),
				   rx, sizeof(rx));

	if (ret != -EINVAL) {
		LOG_ERR("  LSB-first: expected -EINVAL, got %d", ret);
		return 1;
	}
	return 0;
}

static int subtest_release_wrong_cfg(void)
{
	static struct spi_config cfg1;

	cfg1 = neg_test_cfg(0, 8U, SPI_FREQ_MHZ);
	uint8_t tx[4] = {0xDE, 0xAD, 0xBE, 0xEF};
	uint8_t rx[4];
	struct spi_buf tb = { .buf = tx, .len = sizeof(tx) };
	struct spi_buf rb = { .buf = rx, .len = sizeof(rx) };
	struct spi_buf_set ts = { .buffers = &tb, .count = 1 };
	struct spi_buf_set rs = { .buffers = &rb, .count = 1 };
	int ret;

	ret = do_loopback_xfer(&cfg1, tx, sizeof(tx),
			       rx, sizeof(rx));
	if (ret < 0) {
		LOG_ERR("  release_wrong: initial xfer failed %d", ret);
		return 1;
	}

	static struct spi_config cfg2;

	cfg2 = neg_test_cfg(0, 16U, 2 * SPI_FREQ_MHZ);

	ret = spi_release(controller_dev, &cfg2);
	if (ret != -EINVAL) {
		LOG_ERR("  release_wrong: expected -EINVAL, got %d", ret);
		return 1;
	}
	return 0;
}

static int subtest_freq_exceed(void)
{
	static struct spi_config cfg;

	cfg = neg_test_cfg(0, 8U, UINT32_MAX);
	uint8_t tx[4] = {0x11, 0x22, 0x33, 0x44};
	uint8_t rx[4];
	int ret = do_loopback_xfer(&cfg, tx, sizeof(tx),
				   rx, sizeof(rx));

	if (ret != -EINVAL) {
		LOG_ERR("  freq_exceed: expected -EINVAL, got %d", ret);
		return 1;
	}
	return 0;
}

static int subtest_word_zero_invalid(void)
{
	static struct spi_config cfg;

	cfg = neg_test_cfg(0, 0U, SPI_FREQ_MHZ);
	int ret = spi_test_transceive(controller_dev, &cfg, NULL, NULL, false);

	if (ret != -EINVAL) {
		LOG_ERR("  word_zero: expected -EINVAL, got %d", ret);
		return 1;
	}
	return 0;
}

static int subtest_half_duplex(void)
{
	static struct spi_config cfg;

	cfg = neg_test_cfg(SPI_HALF_DUPLEX,
					     8U, SPI_FREQ_MHZ);
	uint8_t tx[4] = {0x11, 0x22, 0x33, 0x44};
	uint8_t rx[4];
	int ret = do_loopback_xfer(&cfg, tx, sizeof(tx),
				   rx, sizeof(rx));

	if (ret != -ENOTSUP) {
		LOG_ERR("  half_duplex: expected -ENOTSUP, got %d", ret);
		return 1;
	}
	return 0;
}

static int subtest_op_mode_slave(void)
{
	static struct spi_config cfg;

	cfg = neg_test_cfg(SPI_OP_MODE_SLAVE,
					     8U, SPI_FREQ_MHZ);
	uint8_t tx[4] = {0x11, 0x22, 0x33, 0x44};
	uint8_t rx[4];
	int ret = do_loopback_xfer(&cfg, tx, sizeof(tx),
				   rx, sizeof(rx));

	if (ret != -ENOTSUP) {
		LOG_ERR("  op_mode_slave: expected -ENOTSUP, got %d", ret);
		return 1;
	}
	return 0;
}

static int subtest_op_mode_master_on_slave(void)
{
	static const struct device *const target_dev =
		DEVICE_DT_GET(SPI_TARGET_NODE);
	static struct spi_config cfg;

	cfg = neg_test_cfg(0, 8U, SPI_FREQ_MHZ);
	uint8_t tx[4] = {0x11, 0x22, 0x33, 0x44};
	uint8_t rx[4];
	struct spi_buf tb = { .buf = tx, .len = sizeof(tx) };
	struct spi_buf rb = { .buf = rx, .len = sizeof(rx) };
	struct spi_buf_set ts = { .buffers = &tb, .count = 1 };
	struct spi_buf_set rs = { .buffers = &rb, .count = 1 };
	int ret;

	ret = spi_test_transceive(target_dev, &cfg, &ts, &rs, false);
	if (ret != -ENOTSUP) {
		LOG_ERR("  op_mode_master_on_slave: expected -ENOTSUP, got %d", ret);
		return 1;
	}
	return 0;
}

#if IS_ENABLED(CONFIG_SPI_EXTENDED_MODES)
static int subtest_extended_lines(void)
{
	static struct spi_config cfg;

	cfg = neg_test_cfg(SPI_LINES_DUAL,
					     8U, SPI_FREQ_MHZ);
	uint8_t tx[4] = {0x11, 0x22, 0x33, 0x44};
	uint8_t rx[4];
	int ret = do_loopback_xfer(&cfg, tx, sizeof(tx),
				   rx, sizeof(rx));

	if (ret != -EINVAL) {
		LOG_ERR("  extended_lines: expected -EINVAL, got %d", ret);
		return 1;
	}
	return 0;
}
#endif

static int subtest_ti_loopback(void)
{
	static struct spi_config cfg;

	cfg = neg_test_cfg(SPI_FRAME_FORMAT_TI | SPI_MODE_LOOP,
					     8U, SPI_FREQ_MHZ);
	uint8_t tx[4] = {0x11, 0x22, 0x33, 0x44};
	uint8_t rx[4];
	int ret = do_loopback_xfer(&cfg, tx, sizeof(tx),
				   rx, sizeof(rx));

	if (ret != -ENOTSUP) {
		LOG_ERR("  ti_loopback: expected -ENOTSUP, got %d", ret);
		return 1;
	}
	return 0;
}

ZTEST(test_spi_negative, test_word_exceed)
{
	static struct spi_config cfg;

	cfg = neg_test_cfg(0, 33U, SPI_FREQ_MHZ);
	uint8_t tx[4] = {0xAA, 0xBB, 0xCC, 0xDD};
	uint8_t rx[4];
	struct spi_config cfg = neg_test_cfg(0, 33U, SPI_FREQ_MHZ);
	int ret = do_loopback_xfer(&cfg, tx, sizeof(tx), rx, sizeof(rx));

	zassert_equal(ret, -ENOTSUP,
		      "word_exceed: expected -ENOTSUP, got %d", ret);
}

ZTEST(test_spi_negative, test_lsb_first)
{
	zassert_true(spi_test_device_ready(controller_dev, "Controller"),
		     "SPI device not ready");
	zassert_equal(subtest_lsb_first(), 0, "LSB-first negative test failed");
}

ZTEST(test_spi_negative, test_release_wrong_cfg)
{
	zassert_true(spi_test_device_ready(controller_dev, "Controller"),
		     "SPI device not ready");
	zassert_equal(subtest_release_wrong_cfg(), 0,
		      "Release wrong config negative test failed");
}

ZTEST(test_spi_negative, test_freq_exceed)
{
	zassert_true(spi_test_device_ready(controller_dev, "Controller"),
		     "SPI device not ready");
	zassert_equal(subtest_freq_exceed(), 0,
		      "Frequency exceed negative test failed");
}

ZTEST(test_spi_negative, test_word_zero)
{
	zassert_true(spi_test_device_ready(controller_dev, "Controller"),
		     "SPI device not ready");
	zassert_equal(subtest_word_zero_invalid(), 0,
		      "Word zero negative test failed");
}

ZTEST(test_spi_negative, test_word_exceed)
{
	zassert_true(spi_test_device_ready(controller_dev, "Controller"),
		     "SPI device not ready");
	zassert_equal(subtest_word_exceed(), 0,
		      "Word exceed negative test failed");
}

ZTEST(test_spi_negative, test_half_duplex)
{
	zassert_true(spi_test_device_ready(controller_dev, "Controller"),
		     "SPI device not ready");
	zassert_equal(subtest_half_duplex(), 0,
		      "Half-duplex negative test failed");
}

ZTEST(test_spi_negative, test_op_mode_slave)
{
	zassert_true(spi_test_device_ready(controller_dev, "Controller"),
		     "SPI device not ready");
	zassert_equal(subtest_op_mode_slave(), 0,
		      "Operation mode slave negative test failed");
}

ZTEST(test_spi_negative, test_op_mode_master_on_slave)
{
	static const struct device *const target_dev =
		DEVICE_DT_GET(SPI_TARGET_NODE);

	zassert_true(spi_test_device_ready(target_dev, "Target"),
		     "SPI target device not ready");
	zassert_equal(subtest_op_mode_master_on_slave(), 0,
		      "Operation mode master on slave negative test failed");
}

#if IS_ENABLED(CONFIG_SPI_EXTENDED_MODES)
ZTEST(test_spi_negative, test_extended_lines)
{
	zassert_true(spi_test_device_ready(controller_dev, "Controller"),
		     "SPI device not ready");
	zassert_equal(subtest_extended_lines(), 0,
		      "Extended lines negative test failed");
}
#endif

ZTEST(test_spi_negative, test_ti_loopback)
{
	zassert_true(spi_test_device_ready(controller_dev, "Controller"),
		     "SPI device not ready");
	zassert_equal(subtest_ti_loopback(), 0,
		      "TI loopback negative test failed");
}

/* ------------------------------------------------------------------
 * Suite registration
 * ------------------------------------------------------------------
 */
ZTEST_SUITE(test_spi_negative, NULL, NULL, test_spi_negative_before,
	    NULL, NULL);

#endif /* CONFIG_TEST_SPI_NEGATIVE */

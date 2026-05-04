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

LOG_MODULE_REGISTER(alif_spi_neg, LOG_LEVEL_INF);

static const struct device *const controller_dev =
	DEVICE_DT_GET(SPI_CONTROLLER_NODE);

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


static int subtest_lsb_first(void)
{
	struct spi_config cfg = neg_test_cfg(SPI_TRANSFER_LSB,
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
	struct spi_config cfg1 = neg_test_cfg(0, 8U, SPI_FREQ_MHZ);
	uint8_t tx[4] = {0xDE, 0xAD, 0xBE, 0xEF};
	uint8_t rx[4];
	int ret;

	ret = do_loopback_xfer(&cfg1, tx, sizeof(tx),
			       rx, sizeof(rx));
	if (ret < 0) {
		LOG_ERR("  release_wrong: initial xfer failed %d", ret);
		return 1;
	}

	struct spi_config cfg2 = neg_test_cfg(0, 16U, 2 * SPI_FREQ_MHZ);

	ret = spi_release(controller_dev, &cfg2);
	if (ret != -EINVAL) {
		LOG_ERR("  release_wrong: expected -EINVAL, got %d", ret);
		return 1;
	}
	return 0;
}

static int subtest_freq_exceed(void)
{
	struct spi_config cfg = neg_test_cfg(0, 8U, UINT32_MAX);
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

static int subtest_null_buf(void)
{
	struct spi_config cfg = neg_test_cfg(0, 8U, SPI_FREQ_MHZ);
	struct spi_buf tb = { .buf = NULL, .len = 16 };
	struct spi_buf_set ts = { .buffers = &tb, .count = 1 };
	int ret = spi_test_transceive(controller_dev, &cfg, &ts, NULL, false);

	if (ret != -EINVAL) {
		LOG_ERR("  null_buf: expected -EINVAL, got %d", ret);
		return 1;
	}
	return 0;
}

static int subtest_word_exceed(void)
{
	struct spi_config cfg = neg_test_cfg(0, 33U, SPI_FREQ_MHZ);
	uint8_t tx[4] = {0xAA, 0xBB, 0xCC, 0xDD};
	uint8_t rx[4];
	int ret = do_loopback_xfer(&cfg, tx, sizeof(tx),
				   rx, sizeof(rx));

	if (ret != -ENOTSUP) {
		LOG_ERR("  word_exceed: expected -ENOTSUP, got %d", ret);
		return 1;
	}
	return 0;
}

static int subtest_word_zero_invalid(void)
{
	struct spi_config cfg = neg_test_cfg(0, 0U, SPI_FREQ_MHZ);
	int ret = spi_test_transceive(controller_dev, &cfg, NULL, NULL, false);

	if (ret != -EINVAL) {
		LOG_ERR("  word_zero: expected -EINVAL, got %d", ret);
		return 1;
	}
	return 0;
}

static int subtest_word_len_unaligned(void)
{
	struct spi_config cfg = neg_test_cfg(0, 16U, SPI_FREQ_MHZ);
	uint8_t tx[3] = {0xAA, 0xBB, 0xCC};
	uint8_t rx[3];
	int ret = do_loopback_xfer(&cfg, tx, sizeof(tx),
				   rx, sizeof(rx));

	if (ret != -EINVAL) {
		LOG_ERR("  word_len_unaligned: expected -EINVAL, got %d", ret);
		return 1;
	}
	return 0;
}


ZTEST(test_spi_negative, test_comprehensive_negative)
{
	zassert_true(spi_test_device_ready(controller_dev, "Controller"),
		     "SPI device not ready");

	int fail = 0;
	int f;

	TC_PRINT("=== Comprehensive Negative Validation ===\n");

	f = subtest_lsb_first();
	fail += f;
	TC_PRINT("  lsb_first        : %s\n", f ? "FAIL" : "OK");

	f = subtest_release_wrong_cfg();
	fail += f;
	TC_PRINT("  release_wrong    : %s\n", f ? "FAIL" : "OK");

	f = subtest_freq_exceed();
	fail += f;
	TC_PRINT("  freq_exceed      : %s\n", f ? "FAIL" : "OK");

	f = subtest_null_buf();
	fail += f;
	TC_PRINT("  null_buf         : %s\n", f ? "FAIL" : "OK");

	f = subtest_word_exceed();
	fail += f;
	TC_PRINT("  word_exceed      : %s\n", f ? "FAIL" : "OK");

	f = subtest_word_zero_invalid();
	fail += f;
	TC_PRINT("  word_zero        : %s\n", f ? "FAIL" : "OK");

	f = subtest_word_len_unaligned();
	fail += f;
	TC_PRINT("  word_len_unalign : %s\n", f ? "FAIL" : "OK");

	TC_PRINT("=== Result: %d failures ===\n", fail);
	zassert_equal(fail, 0, "Comprehensive negative: %d failures", fail);
}

ZTEST_SUITE(test_spi_negative, NULL, NULL, test_before_func,
	    NULL, NULL);

#endif /* CONFIG_TEST_SPI_NEGATIVE */

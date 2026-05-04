/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/sys/util_macro.h>

#if IS_ENABLED(CONFIG_TEST_SPI_PERFORMANCE)

#include "test_spi_threads.h"

LOG_MODULE_REGISTER(alif_spi_perf, LOG_LEVEL_INF);

static void assert_mosi_perf_clean(const char *tag)
{
	zassert_equal(ctrl_ret_tx, 0U,
		      "%s: controller TX failures=%u", tag, ctrl_ret_tx);
	zassert_equal(tgt_ret_rx, 0U,
		      "%s: target RX failures=%u", tag, tgt_ret_rx);
	TC_PRINT("%s: OK\n", tag);
}

static void assert_miso_perf_clean(const char *tag)
{
	zassert_equal(tgt_ret_tx, 0U,
		      "%s: target TX failures=%u", tag, tgt_ret_tx);
	zassert_equal(ctrl_ret_rx, 0U,
		      "%s: controller RX failures=%u", tag, ctrl_ret_rx);
	TC_PRINT("%s: OK\n", tag);
}

static void assert_xcv_perf_clean(const char *tag)
{
	zassert_equal(ctrl_ret_tx, 0U,
		      "%s: controller TX failures=%u", tag, ctrl_ret_tx);
	zassert_equal(tgt_ret_tx, 0U,
		      "%s: target TX failures=%u", tag, tgt_ret_tx);
	zassert_equal(ctrl_ret_rx, 0U,
		      "%s: controller RX failures=%u", tag, ctrl_ret_rx);
	zassert_equal(tgt_ret_rx, 0U,
		      "%s: target RX failures=%u", tag, tgt_ret_rx);
	TC_PRINT("%s: OK\n", tag);
}

/* Controller TX -> Target RX */

ZTEST(test_spi_perf_mosi, test_perf_mosi)
{
	zassert_ok(spi_test_prepare_data(ctrl_txdata,
					 spi_test_pattern(0xBEEF),
					 SPI_TEST_BUFF_SIZE_PERF),
		   "Failed to prepare controller TX data");
	run_spi_test_threads(controller_spi_transmit, target_spi_receive);
	assert_mosi_perf_clean("MOSI perf");
}

ZTEST_SUITE(test_spi_perf_mosi, NULL, NULL, test_before_func, NULL, NULL);

/* Target TX -> Controller RX */

ZTEST(test_spi_perf_miso, test_perf_miso)
{
	zassert_ok(spi_test_prepare_data(tgt_txdata,
					 spi_test_pattern(0xCAFE),
					 SPI_TEST_BUFF_SIZE_PERF),
		   "Failed to prepare target TX data");
	run_spi_test_threads(controller_receive, target_send);
	assert_miso_perf_clean("MISO perf");
}

ZTEST_SUITE(test_spi_perf_miso, NULL, NULL, test_before_func, NULL, NULL);

/* Full-duplex Controller <-> Target */

ZTEST(test_spi_perf_transceive, test_perf_xcv)
{
	zassert_ok(spi_test_prepare_data(ctrl_txdata,
					 spi_test_pattern(0xBEEF),
					 SPI_TEST_BUFF_SIZE_PERF),
		   "Failed to prepare controller TX data");
	zassert_ok(spi_test_prepare_data(tgt_txdata,
					 spi_test_pattern(0xCAFE),
					 SPI_TEST_BUFF_SIZE_PERF),
		   "Failed to prepare target TX data");
	run_spi_test_threads(controller_spi, target_spi);
	assert_xcv_perf_clean("XCV perf");
}

ZTEST_SUITE(test_spi_perf_transceive, NULL, NULL, test_before_func, NULL, NULL);

#endif /* CONFIG_TEST_SPI_PERFORMANCE */

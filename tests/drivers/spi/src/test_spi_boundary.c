/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/*
 * Alif SPI boundary / edge-case validation suite.
 *
 * Two ZTESTs only:
 *   - test_boundary_sync  : runs the boundary scenarios using
 *                           synchronous spi_transceive
 *   - test_boundary_async : same scenarios driven via
 *                           spi_transceive_signal + k_poll
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/ztest.h>
#include <zephyr/logging/log.h>

#if IS_ENABLED(CONFIG_TEST_SPI_BOUNDARY)

#include "test_spi_common.h"

LOG_MODULE_REGISTER(alif_spi_bnd, LOG_LEVEL_INF);

#define BND_MAX_BUF      1024
#define BND_DEFAULT_FREQ SPI_FREQ_MHZ

static const struct device *const controller_dev =
	DEVICE_DT_GET(SPI_CONTROLLER_NODE);

static struct k_poll_signal bnd_async_sig;
static struct k_poll_event  bnd_async_evt;

static int bnd_xfer(const struct spi_config *cfg,
		    const struct spi_buf_set *tx,
		    const struct spi_buf_set *rx,
		    bool async)
{
	int ret;

	if (!async) {
		return spi_transceive(controller_dev, cfg, tx, rx);
	}

	k_poll_signal_reset(&bnd_async_sig);
	bnd_async_evt.signal->signaled = 0U;
	bnd_async_evt.state = K_POLL_STATE_NOT_READY;

	ret = spi_transceive_signal(controller_dev, cfg, tx, rx, &bnd_async_sig);
	if (ret != 0) {
		return ret;
	}
	ret = k_poll(&bnd_async_evt, 1, K_MSEC(500));
	if (ret != 0) {
		LOG_ERR("boundary async poll timeout");
		return -ETIMEDOUT;
	}
	return bnd_async_sig.result;
}

static int bnd_xfer_compare(const struct spi_config *cfg,
			    void *tx, void *rx, size_t len, bool async)
{
	struct spi_buf tb = { .buf = tx, .len = len };
	struct spi_buf rb = { .buf = rx, .len = len };
	struct spi_buf_set ts = { .buffers = &tb, .count = 1 };
	struct spi_buf_set rs = { .buffers = &rb, .count = 1 };
	int ret;

	memset(rx, 0, len);
	ret = bnd_xfer(cfg, &ts, &rs, async);
	if (ret < 0) {
		return ret;
	}
	return memcmp(tx, rx, len) == 0 ? 0 : -EIO;
}

static struct spi_config bnd_cfg(uint32_t word, uint32_t freq_hz)
{
	uint32_t operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(word);

	if (IS_ENABLED(CONFIG_TEST_SPI_INTERNAL_LOOPBACK)) {
		operation |= SPI_MODE_LOOP;
	}
	return spi_test_config(operation, freq_hz);
}

static int sc_single_byte(bool async)
{
	static const uint8_t patterns[] = {
		0x00, 0xFF, 0xA5, 0x5A, 0x01, 0xFE, 0x55, 0xAA
	};
	struct spi_config cfg = bnd_cfg(8U, BND_DEFAULT_FREQ);
	int fail = 0;

	for (size_t p = 0; p < ARRAY_SIZE(patterns); p++) {
		uint8_t tx = patterns[p];
		uint8_t rx = 0U;
		int ret = bnd_xfer_compare(&cfg, &tx, &rx, 1U, async);

		if (ret != 0) {
			LOG_ERR("single_byte 0x%02X ret=%d", patterns[p], ret);
			fail++;
		}
	}
	return fail;
}

static int sc_fifo_sizes(uint32_t word, const size_t *sizes,
			 size_t n_sizes, bool async)
{
	static uint32_t tx[64];
	static uint32_t rx[64];
	struct spi_config cfg = bnd_cfg(word, BND_DEFAULT_FREQ);
	int fail = 0;
	size_t bytes_per_frame = word / 8U;

	for (size_t s = 0; s < n_sizes; s++) {
		size_t frames = sizes[s];
		size_t len = frames * bytes_per_frame;

		for (size_t i = 0; i < ARRAY_SIZE(tx); i++) {
			tx[i] = 0xA5000000U | (uint32_t)(i + frames);
		}
		int ret = bnd_xfer_compare(&cfg, tx, rx, len, async);

		if (ret != 0) {
			LOG_ERR("fifo_%u_bit frames=%zu ret=%d", word,
				frames, ret);
			fail++;
		}
	}
	return fail;
}

static int sc_fifo_8bit(bool async)
{
	static const size_t sizes[] = {15, 16, 17, 31, 32, 33, 63, 64, 65};

	return sc_fifo_sizes(8U, sizes, ARRAY_SIZE(sizes), async);
}

static int sc_fifo_16bit(bool async)
{
	static const size_t sizes[] = {15, 16, 17, 31, 32, 33};

	return sc_fifo_sizes(16U, sizes, ARRAY_SIZE(sizes), async);
}

static int sc_fifo_32bit(bool async)
{
	static const size_t sizes[] = {15, 16, 17, 31, 32, 33};

	return sc_fifo_sizes(32U, sizes, ARRAY_SIZE(sizes), async);
}

static int sc_fifo_flush(bool async)
{
	struct spi_config cfg = bnd_cfg(8U, BND_DEFAULT_FREQ);
	uint8_t tx_a[17], rx_a[17];
	uint8_t tx_b[17], rx_b[17];
	int ret;

	memset(tx_a, 0xA5, sizeof(tx_a));
	for (size_t i = 0; i < sizeof(tx_b); i++) {
		tx_b[i] = (uint8_t)(i & 0xFFU);
	}

	ret = bnd_xfer_compare(&cfg, tx_a, rx_a, sizeof(tx_a), async);
	if (ret != 0) {
		LOG_ERR("fifo_flush A ret=%d", ret);
		return 1;
	}
	ret = bnd_xfer_compare(&cfg, tx_b, rx_b, sizeof(tx_b), async);
	if (ret != 0) {
		LOG_ERR("fifo_flush B (stale data?) ret=%d", ret);
		return 1;
	}
	return 0;
}

static int sc_xfer_sweep(bool async)
{
	static const size_t sizes[] = {1, 2, 4, 8, 16, 32, 64, 128, 256};
	static const uint8_t pat[] = {0xDE, 0xAD, 0xBE, 0xEF};
	static uint8_t tx[256], rx[256];
	struct spi_config cfg = bnd_cfg(8U, BND_DEFAULT_FREQ);
	int fail = 0;

	for (size_t s = 0; s < ARRAY_SIZE(sizes); s++) {
		size_t len = sizes[s];

		for (size_t i = 0; i < len; i++) {
			tx[i] = pat[i & 3U];
		}
		int ret = bnd_xfer_compare(&cfg, tx, rx, len, async);

		if (ret != 0) {
			LOG_ERR("xfer_sweep len=%zu ret=%d", len, ret);
			fail++;
		}
	}
	return fail;
}

static int sc_fifo_plus_one(bool async)
{
	struct spi_config cfg = bnd_cfg(8U, BND_DEFAULT_FREQ);
	uint8_t tx[33], rx[33];

	for (size_t i = 0; i < sizeof(tx); i++) {
		tx[i] = (uint8_t)(i + 1U);
	}
	int ret = bnd_xfer_compare(&cfg, tx, rx, sizeof(tx), async);

	if (ret != 0) {
		LOG_ERR("fifo_plus_one ret=%d", ret);
		return 1;
	}
	return 0;
}

static int sc_large_xfer(bool async)
{
	struct spi_config cfg = bnd_cfg(8U, BND_DEFAULT_FREQ);
	static uint8_t tx[BND_MAX_BUF], rx[BND_MAX_BUF];

	for (size_t i = 0; i < sizeof(tx); i++) {
		tx[i] = (uint8_t)(i & 0xFFU);
	}
	int ret = bnd_xfer_compare(&cfg, tx, rx, sizeof(tx), async);

	if (ret != 0) {
		LOG_ERR("large_xfer 1024 ret=%d", ret);
		return 1;
	}
	return 0;
}

static int sc_unaligned(bool async)
{
	struct spi_config cfg = bnd_cfg(8U, BND_DEFAULT_FREQ);
	static uint8_t tx[16], rx[16];
	int fail = 0;

	for (size_t len = 1U; len < 16U; len++) {
		for (size_t i = 0; i < len; i++) {
			tx[i] = (uint8_t)((i + 1U) & 0xFFU);
		}
		int ret = bnd_xfer_compare(&cfg, tx, rx, len, async);

		if (ret != 0) {
			LOG_ERR("unaligned len=%zu ret=%d", len, ret);
			fail++;
		}
	}
	return fail;
}

static int sc_cs_toggle(bool async)
{
	struct spi_config cfg = bnd_cfg(8U, BND_DEFAULT_FREQ);
	static uint8_t tx[256], rx[256];
	static const uint8_t pat[] = {0xDE, 0xAD, 0xBE, 0xEF};
	static const size_t sizes[] = {1, 2, 4, 8, 16, 32, 64, 128, 256};
	int fail = 0;
	int ret;

	for (size_t i = 0; i < 16U; i++) {
		tx[i] = pat[i & 3U];
	}
	ret = bnd_xfer_compare(&cfg, tx, rx, 16U, async);
	if (ret != 0) {
		LOG_ERR("cs_toggle basic ret=%d", ret);
		return 1;
	}

	memset(tx, 0xA5, 8U);
	for (int i = 0; i < 50; i++) {
		ret = bnd_xfer_compare(&cfg, tx, rx, 8U, async);
		if (ret != 0) {
			fail++;
			if (fail <= 3) {
				LOG_ERR("cs_toggle iter %d ret=%d", i, ret);
			}
		}
	}

	for (size_t s = 0; s < ARRAY_SIZE(sizes); s++) {
		size_t len = sizes[s];

		for (size_t i = 0; i < len; i++) {
			tx[i] = (uint8_t)((i + s) & 0xFFU);
		}
		ret = bnd_xfer_compare(&cfg, tx, rx, len, async);
		if (ret != 0) {
			LOG_ERR("cs_toggle multi-size %zu ret=%d", len, ret);
			fail++;
		}
	}
	return fail;
}

static int sc_max_freq(bool async)
{
	struct spi_config cfg = bnd_cfg(8U,
					SPI_TEST_MAX_FREQUENCY * SPI_FREQ_MHZ);
	uint8_t tx[32], rx[32];

	for (size_t i = 0; i < sizeof(tx); i++) {
		tx[i] = (uint8_t)(0xFFU - i);
	}
	int ret = bnd_xfer_compare(&cfg, tx, rx, sizeof(tx), async);

	if (ret != 0) {
		LOG_ERR("max_freq ret=%d", ret);
		return 1;
	}
	return 0;
}

struct bnd_scenario {
	const char *name;
	int (*fn)(bool async);
};

static const struct bnd_scenario scenarios[] = {
	{ "single_byte",   sc_single_byte   },
	{ "fifo_8bit",     sc_fifo_8bit     },
	{ "fifo_16bit",    sc_fifo_16bit    },
	{ "fifo_32bit",    sc_fifo_32bit    },
	{ "fifo_flush",    sc_fifo_flush    },
	{ "xfer_sweep",    sc_xfer_sweep    },
	{ "fifo_plus_one", sc_fifo_plus_one },
	{ "large_xfer",    sc_large_xfer    },
	{ "unaligned",     sc_unaligned     },
	{ "cs_toggle",     sc_cs_toggle     },
	{ "max_freq",      sc_max_freq      },
};

static int run_boundary(bool async)
{
	int total = 0;

	for (size_t i = 0; i < ARRAY_SIZE(scenarios); i++) {
		int f = scenarios[i].fn(async);

		TC_PRINT("  %-14s : %s (%d)\n", scenarios[i].name,
			f ? "FAIL" : "OK", f);
		total += f;
	}
	return total;
}

ZTEST(test_spi_boundary, test_boundary_sync)
{
	zassert_true(spi_test_device_ready(controller_dev, "Controller"),
		     "SPI controller not ready");

	TC_PRINT("=== SPI boundary (SYNC) ===\n");
	int fail = run_boundary(false);

	zassert_equal(fail, 0, "SYNC boundary: %d scenario failure(s)", fail);
}

#if IS_ENABLED(CONFIG_SPI_ASYNC)
ZTEST(test_spi_boundary, test_boundary_async)
{
	zassert_true(spi_test_device_ready(controller_dev, "Controller"),
		     "SPI controller not ready");

	TC_PRINT("=== SPI boundary (ASYNC) ===\n");
	int fail = run_boundary(true);

	zassert_equal(fail, 0, "ASYNC boundary: %d scenario failure(s)", fail);
}
#endif /* CONFIG_SPI_ASYNC */

static void boundary_suite_before(void *fixture)
{
	ARG_UNUSED(fixture);

	k_poll_signal_init(&bnd_async_sig);
	k_poll_event_init(&bnd_async_evt, K_POLL_TYPE_SIGNAL,
			  K_POLL_MODE_NOTIFY_ONLY, &bnd_async_sig);
}

ZTEST_SUITE(test_spi_boundary, NULL, NULL, boundary_suite_before, NULL, NULL);

#endif /* CONFIG_TEST_SPI_BOUNDARY */

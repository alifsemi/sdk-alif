/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/ztest.h>
#include <zephyr/logging/log.h>

#if IS_ENABLED(CONFIG_TEST_SPI_LOOPBACK)

#include "test_spi_common.h"

LOG_MODULE_REGISTER(alif_spi_loop, LOG_LEVEL_INF);

#define LB_DEFAULT_FREQ_HZ  SPI_FREQ_MHZ
#define LB_BUF_MAX          64

/* Word-size selection: when CONFIG_TEST_SPI_WORD_SIZE is non-zero, the
 * scenarios pin to that single width; otherwise they sweep 8/16/32.
 */
#if !IS_ENABLED(CONFIG_TEST_SPI_WORD_SIZE_SWEEP)
#define LB_PINNED_WORD       CONFIG_TEST_SPI_WORD_SIZE
#define LB_WORDS             ((const uint32_t[]){LB_PINNED_WORD})
#else
#define LB_PINNED_WORD       8U
#define LB_WORDS             ((const uint32_t[]){8U, 16U, 32U})
#endif

static const struct device *const controller_dev =
	DEVICE_DT_GET(SPI_CONTROLLER_NODE);

static struct k_poll_signal async_sig;
static struct k_poll_event  async_evt;

static int do_xfer(const struct spi_config *cfg,
		   const struct spi_buf_set *tx,
		   const struct spi_buf_set *rx,
		   bool async)
{
	int ret;

	if (!async) {
		return spi_transceive(controller_dev, cfg, tx, rx);
	}

	k_poll_signal_reset(&async_sig);
	async_evt.signal->signaled = 0U;
	async_evt.state = K_POLL_STATE_NOT_READY;

	ret = spi_transceive_signal(controller_dev, cfg, tx, rx, &async_sig);
	if (ret != 0) {
		return ret;
	}

	ret = k_poll(&async_evt, 1, K_MSEC(500));
	if (ret != 0) {
		LOG_ERR("async poll timeout");
		return -ETIMEDOUT;
	}
	return async_sig.result;
}

static int xfer_compare(const struct spi_config *cfg,
			void *tx, void *rx, size_t len, bool async)
{
	struct spi_buf tb = { .buf = tx, .len = len };
	struct spi_buf rb = { .buf = rx, .len = len };
	struct spi_buf_set ts = { .buffers = &tb, .count = 1 };
	struct spi_buf_set rs = { .buffers = &rb, .count = 1 };
	int ret;

	memset(rx, 0, len);
	ret = do_xfer(cfg, &ts, &rs, async);
	if (ret < 0) {
		return ret;
	}
	return memcmp(tx, rx, len) == 0 ? 0 : -EIO;
}

static inline struct spi_config lb_cfg(uint32_t op_extra,
					     uint32_t word, uint32_t freq)
{
	uint32_t operation = SPI_OP_MODE_MASTER |
			     SPI_WORD_SET(word) | op_extra;

	if (IS_ENABLED(CONFIG_TEST_SPI_INTERNAL_LOOPBACK)) {
		operation |= SPI_MODE_LOOP;
	}
	return spi_test_config(operation, freq);
}


static int sc_word_sweep(bool async)
{
	static const uint8_t pat[] = {0xDE, 0xAD, 0xBE, 0xEF};
	static uint8_t tx[32], rx[32];
	const uint32_t *words = LB_WORDS;
	const size_t n_words = ARRAY_SIZE(LB_WORDS);
	int fail = 0;

	for (size_t i = 0; i < sizeof(tx); i++) {
		tx[i] = pat[i & 3U];
	}
	for (size_t w = 0; w < n_words; w++) {
		struct spi_config cfg = lb_cfg(0, words[w], LB_DEFAULT_FREQ_HZ);
		int ret = xfer_compare(&cfg, tx, rx, sizeof(tx), async);

		if (ret) {
			LOG_ERR("word_sweep w=%u ret=%d", words[w], ret);
			fail++;
		}
	}
	return fail;
}

static int sc_walking_bit(bool async)
{
	static const size_t sizes[] = {1, 4, 16, 64};
	static uint8_t tx[LB_BUF_MAX], rx[LB_BUF_MAX];
	struct spi_config cfg = lb_cfg(0, 8, LB_DEFAULT_FREQ_HZ);
	int fail = 0;

	for (size_t s = 0; s < ARRAY_SIZE(sizes); s++) {
		size_t len = sizes[s];

		for (size_t i = 0; i < len; i++) {
			tx[i] = (uint8_t)(1U << (i & 7U));
		}
		int ret = xfer_compare(&cfg, tx, rx, len, async);

		if (ret) {
			LOG_ERR("walking_bit len=%zu ret=%d", len, ret);
			fail++;
		}
	}
	return fail;
}

static int sc_cpol_cpha(bool async)
{
	static const uint32_t modes[] = {
		0,
		SPI_MODE_CPHA,
		SPI_MODE_CPOL,
		SPI_MODE_CPOL | SPI_MODE_CPHA,
	};
	static uint8_t tx[16], rx[16];
	int fail = 0;

	for (size_t i = 0; i < sizeof(tx); i++) {
		tx[i] = (uint8_t)(0x5A ^ i);
	}
	for (size_t m = 0; m < ARRAY_SIZE(modes); m++) {
		struct spi_config cfg = lb_cfg(modes[m], 8, LB_DEFAULT_FREQ_HZ);
		int ret = xfer_compare(&cfg, tx, rx, sizeof(tx), async);

		if (ret) {
			LOG_ERR("cpol_cpha mode=0x%02x ret=%d", modes[m], ret);
			fail++;
		}
	}
	return fail;
}

static int sc_tmod_tx(bool async)
{
	uint8_t tx[16];
	struct spi_buf tb = { .buf = tx, .len = sizeof(tx) };
	struct spi_buf_set ts = { .buffers = &tb, .count = 1 };
	struct spi_config cfg = lb_cfg(0, 8, LB_DEFAULT_FREQ_HZ);
	int ret;

	memset(tx, 0xA5, sizeof(tx));
	ret = do_xfer(&cfg, &ts, NULL, async);
	if (ret < 0) {
		LOG_ERR("tmod_tx ret=%d", ret);
		return 1;
	}
	return 0;
}

static int sc_config_reuse(bool async)
{
	static struct spi_config cfg;
	uint8_t tx[16], rx[16];
	struct spi_buf tb = { .buf = tx, .len = sizeof(tx) };
	struct spi_buf rb = { .buf = rx, .len = sizeof(rx) };
	struct spi_buf_set ts = { .buffers = &tb, .count = 1 };
	struct spi_buf_set rs = { .buffers = &rb, .count = 1 };
	int ret;

	cfg = lb_cfg(0, 8, LB_DEFAULT_FREQ_HZ);
	memset(tx, 0x5A, sizeof(tx));

	for (int pass = 0; pass < 2; pass++) {
		memset(rx, 0, sizeof(rx));
		ret = do_xfer(&cfg, &ts, &rs, async);
		if (ret < 0 || memcmp(tx, rx, sizeof(tx)) != 0) {
			LOG_ERR("config_reuse pass=%d ret=%d", pass, ret);
			return 1;
		}
	}
	return 0;
}

static int sc_miso_not_stuck(void)
{
	uint8_t tx[32], rx[32];
	struct spi_config cfg = lb_cfg(0, 8, LB_DEFAULT_FREQ_HZ);
	bool all_zero = true, all_ff = true;

	for (size_t i = 0; i < sizeof(tx); i++) {
		tx[i] = (uint8_t)i;
	}
	memset(rx, 0xCC, sizeof(rx));

	int ret = xfer_compare(&cfg, tx, rx, sizeof(tx), false);

	if (ret < 0) {
		LOG_ERR("miso_not_stuck xfer ret=%d", ret);
		return 1;
	}
	for (size_t i = 0; i < sizeof(rx); i++) {
		if (rx[i] != 0x00U) {
			all_zero = false;
		}
		if (rx[i] != 0xFFU) {
			all_ff = false;
		}
	}
	if (all_zero || all_ff) {
		LOG_ERR("MISO stuck %s", all_zero ? "LOW" : "HIGH");
		return 1;
	}
	return 0;
}

static int sc_freq_sweep(bool async)
{
	static const uint32_t freqs_hz[] = {
		1U  * SPI_FREQ_MHZ,
		5U  * SPI_FREQ_MHZ,
		12U * SPI_FREQ_MHZ,
		25U * SPI_FREQ_MHZ,
	};
	static uint8_t tx[32], rx[32];
	int fail = 0;

	for (size_t i = 0; i < sizeof(tx); i++) {
		tx[i] = (uint8_t)(0xC3 ^ i);
	}
	for (size_t f = 0; f < ARRAY_SIZE(freqs_hz); f++) {
		struct spi_config cfg = lb_cfg(0, 8, freqs_hz[f]);
		int ret = xfer_compare(&cfg, tx, rx, sizeof(tx), async);

		if (ret) {
			LOG_ERR("freq_sweep %u Hz ret=%d", freqs_hz[f], ret);
			fail++;
		}
	}
	return fail;
}

struct scenario {
	const char *name;
	int (*fn)(bool async);
};

static const struct scenario scenarios[] = {
	{ "word_sweep",   sc_word_sweep    },
	{ "walking_bit",  sc_walking_bit   },
	{ "cpol_cpha",    sc_cpol_cpha     },
	{ "tmod_tx",      sc_tmod_tx       },
	{ "config_reuse", sc_config_reuse  },
	{ "freq_sweep",   sc_freq_sweep    },
};

static int run_loopback(bool async)
{
	int total = 0;

	for (size_t i = 0; i < ARRAY_SIZE(scenarios); i++) {
		int f = scenarios[i].fn(async);

		TC_PRINT("  %-12s : %s (%d)\n", scenarios[i].name,
			f ? "FAIL" : "OK", f);
		total += f;
	}
	return total;
}


ZTEST(test_spi_loopback, test_loopback_sync)
{
	zassert_true(spi_test_device_ready(controller_dev, "Controller"),
		     "SPI controller not ready");

	TC_PRINT("=== SPI loopback (SYNC) ===\n");
	int fail = run_loopback(false);

	int wf = sc_miso_not_stuck();

	TC_PRINT("  %-12s : %s (%d)\n", "miso_wiring", wf ? "FAIL" : "OK", wf);
	fail += wf;

	zassert_equal(fail, 0, "SYNC loopback: %d scenario failure(s)", fail);
}

#if IS_ENABLED(CONFIG_SPI_ASYNC)
ZTEST(test_spi_loopback, test_loopback_async)
{
	zassert_true(spi_test_device_ready(controller_dev, "Controller"),
		     "SPI controller not ready");

	TC_PRINT("=== SPI loopback (ASYNC) ===\n");
	int fail = run_loopback(true);

	zassert_equal(fail, 0, "ASYNC loopback: %d scenario failure(s)", fail);
}
#endif /* CONFIG_SPI_ASYNC */

static void loopback_suite_before(void *fixture)
{
	ARG_UNUSED(fixture);

	k_poll_signal_init(&async_sig);
	k_poll_event_init(&async_evt, K_POLL_TYPE_SIGNAL,
			  K_POLL_MODE_NOTIFY_ONLY, &async_sig);
}

ZTEST_SUITE(test_spi_loopback, NULL, NULL, loopback_suite_before, NULL, NULL);

#endif /* CONFIG_TEST_SPI_LOOPBACK */

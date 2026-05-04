/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/*
 * SPI stress validation suite.
 *
 * Goal: exercise long-running, back-to-back SPI activity to surface
 *       bus-stability issues that a single transfer cannot reveal:
 *         - FIFO leakage between transfers
 *         - DMA channel reuse / state retention
 *         - Interrupt and polling-path stability under churn
 *
 * Design (one-transfer-per-iteration):
 *   Each iteration is a single full-duplex transceive between the
 *   controller (SPI1) and target (SPI0), driven by the proven
 *   `run_spi_test_threads(controller_spi, target_spi)` helper used by
 *   the performance/TI/NI suites. Spawning fresh threads per iteration
 *   keeps the slave from getting stuck if the bus desyncs - thread
 *   join naturally bounds each iteration.
 *
 * Variation is done by mutating the shared ctrl_txdata / tgt_txdata
 * buffers between iterations.
 *
 * Length variation is intentionally NOT done here; the boundary suite
 * covers FIFO-edge transfer sizes. Stress focuses on volume.
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/ztest.h>
#include <zephyr/logging/log.h>

#if IS_ENABLED(CONFIG_TEST_SPI_STRESS)

#include "test_spi_threads.h"

LOG_MODULE_REGISTER(alif_spi_stress, LOG_LEVEL_INF);

#define STRESS_DEFAULT_ITERS    100
#define STRESS_PATTERN_ITERS    50

static void fill_tx_pattern(uint32_t seed_ctrl, uint32_t seed_tgt)
{
	for (size_t i = 0; i < SPI_TEST_BUFF_SIZE; i++) {
		ctrl_txdata[i] = seed_ctrl ^ (uint32_t)i;
		tgt_txdata[i]  = seed_tgt  ^ (uint32_t)i;
	}
}

static uint32_t stress_seed(uint16_t idx)
{
	static const uint16_t bases[] = {
		0xA5A5, 0x5A5A, 0xDEAD, 0xCAFE,
		0x1234, 0x8765, 0x0000, 0xFFFF,
	};

	return spi_test_seed32(bases[idx % ARRAY_SIZE(bases)]);
}

/*
 * Scenario 1: fixed pattern, many iterations.
 * Detects FIFO leakage and state retention across repeated transfers.
 */
static int sc_repeat(bool async)
{
	atomic_set(&spi_test_async_mode, async ? 1 : 0);

	for (int i = 0; i < STRESS_DEFAULT_ITERS; i++) {
		fill_tx_pattern(stress_seed(0), stress_seed(1));
		atomic_clear(&err_count);

		run_spi_test_threads(controller_spi, target_spi);

		if (atomic_get(&err_count) != 0) {
			atomic_clear(&spi_test_async_mode);
			LOG_ERR("repeat: failed at iteration %d", i);
			return 1;
		}
	}
	atomic_clear(&spi_test_async_mode);
	return 0;
}

/*
 * Scenario 2: rotating patterns.
 * Detects pattern-dependent bugs (e.g. all-zero or all-one sensitive
 * paths in the IP, DMA word alignment).
 */
static int sc_pattern_rotate(bool async)
{
	atomic_set(&spi_test_async_mode, async ? 1 : 0);

	for (int i = 0; i < STRESS_PATTERN_ITERS; i++) {
		uint32_t s_ctrl = stress_seed((uint16_t)i);
		uint32_t s_tgt  = stress_seed((uint16_t)(i + 3));

		fill_tx_pattern(s_ctrl, s_tgt);
		atomic_clear(&err_count);

		run_spi_test_threads(controller_spi, target_spi);

		if (atomic_get(&err_count) != 0) {
			atomic_clear(&spi_test_async_mode);
			LOG_ERR("pattern_rotate: failed at iter %d (ctrl=0x%08x tgt=0x%08x)",
				i, s_ctrl, s_tgt);
			return 1;
		}
	}
	atomic_clear(&spi_test_async_mode);
	return 0;
}

struct stress_scenario {
	const char *name;
	int (*fn)(bool async);
};

static const struct stress_scenario scenarios[] = {
	{ "repeat",          sc_repeat         },
	{ "pattern_rotate",  sc_pattern_rotate },
};

static int run_stress(bool async)
{
	int total = 0;
	uint32_t t_start = k_uptime_get_32();

	for (size_t i = 0; i < ARRAY_SIZE(scenarios); i++) {
		int f = scenarios[i].fn(async);

		TC_PRINT("  %-16s : %s\n", scenarios[i].name,
			 f ? "FAIL" : "OK");
		total += f;
	}
	TC_PRINT("Stress run total: %u ms\n",
		 (unsigned int)(k_uptime_get_32() - t_start));
	return total;
}

ZTEST(test_spi_stress, test_stress_sync)
{
	zassert_true(spi_test_device_ready(DEVICE_DT_GET(SPI_CONTROLLER_NODE),
					   "Controller"),
		     "SPI controller not ready");
	zassert_true(spi_test_device_ready(DEVICE_DT_GET(SPI_TARGET_NODE),
					   "Target"),
		     "SPI target not ready");

	TC_PRINT("=== SPI stress (SYNC) ===\n");
	int fail = run_stress(false);

	zassert_equal(fail, 0, "SYNC stress: %d scenario failure(s)", fail);
}

#if IS_ENABLED(CONFIG_SPI_ASYNC)
ZTEST(test_spi_stress, test_stress_async)
{
	zassert_true(spi_test_device_ready(DEVICE_DT_GET(SPI_CONTROLLER_NODE),
					   "Controller"),
		     "SPI controller not ready");
	zassert_true(spi_test_device_ready(DEVICE_DT_GET(SPI_TARGET_NODE),
					   "Target"),
		     "SPI target not ready");

	TC_PRINT("=== SPI stress (ASYNC) ===\n");
	int fail = run_stress(true);

	zassert_equal(fail, 0, "ASYNC stress: %d scenario failure(s)", fail);
}
#endif /* CONFIG_SPI_ASYNC */

ZTEST_SUITE(test_spi_stress, NULL, NULL, test_before_func, NULL, NULL);

#endif /* CONFIG_TEST_SPI_STRESS */

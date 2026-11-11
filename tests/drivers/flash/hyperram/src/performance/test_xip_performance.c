/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 * Bandwidth and latency cases. Built only when CONFIG_TEST_XIP_PERFORMANCE=y.
 * The window stays non-cacheable. A full-chip image does not register these.
 */

#include <stdbool.h>
#include <stdint.h>
#include <cmsis_core.h>
#include <zephyr/arch/arm/mpu/arm_mpu.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/barrier.h>
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

#if !DT_HAS_ALIAS(spi_psram)
#error "Devicetree alias spi-psram is required"
#endif

LOG_MODULE_REGISTER(xip);

#if IS_ENABLED(CONFIG_TEST_ISSI_HYPERRAM)
#define XIP_DEV_NAME "ISSI HyperRAM"
#elif IS_ENABLED(CONFIG_TEST_HYPERRAM)
#define XIP_DEV_NAME "HyperRAM"
#elif IS_ENABLED(CONFIG_TEST_PSRAM)
#define XIP_DEV_NAME "PSRAM"
#else
#define XIP_DEV_NAME "XIP"
#endif
#define PSRAM_NODE                   DT_ALIAS(spi_psram)
#define PSRAM_CTRL_NODE              DT_PARENT(PSRAM_NODE)
#define PSRAM_TEST_SIZE              ((uint32_t)DT_PROP(PSRAM_NODE, size))
#define PSRAM_XIP_BASE_ADDR          ((uint32_t)DT_PROP_BY_IDX(PSRAM_CTRL_NODE, \
							       xip_base_address, 0))
#define PSRAM_XIP_WIN_SIZE           ((uint32_t)DT_PROP_BY_IDX(PSRAM_CTRL_NODE, \
							       xip_base_address, 1))
#define PSRAM_BUS_SPEED              ((uint32_t)DT_PROP(PSRAM_CTRL_NODE, bus_speed))

#if DT_NODE_HAS_COMPAT(PSRAM_NODE, alif_infineon_s80ks2564) || \
	DT_NODE_HAS_PROP(PSRAM_NODE, x16_data_transfer_mode)
#define PSRAM_DATA_BYTES             2U
#else
#define PSRAM_DATA_BYTES             1U
#endif
#define PSRAM_DDR_EDGES              2U
#define PSRAM_PERF_BYTES             65536U
#define PSRAM_CACHE_BYTES            4096U

static const struct device *const psram_dev = DEVICE_DT_GET(PSRAM_NODE);

static uint8_t *get_xip_ptr(uint32_t offset)
{
	zassert_true(offset < PSRAM_TEST_SIZE,
		     "XIP offset 0x%x outside size %u", offset, PSRAM_TEST_SIZE);
	return (uint8_t *)(PSRAM_XIP_BASE_ADDR + offset);
}


#if !IS_ENABLED(CONFIG_TEST_PSRAM_FULL_CHIP) && \
	!IS_ENABLED(CONFIG_TEST_HYPERRAM_FULL_CHIP) && \
	!IS_ENABLED(CONFIG_TEST_ISSI_HYPERRAM_FULL_CHIP)

static uint32_t perf_sink;

static uint64_t perf_bytes_per_sec(uint32_t bytes, uint32_t cycles)
{
	if (cycles == 0U) {
		return 0U;
	}

	return ((uint64_t)bytes * sys_clock_hw_cycles_per_sec()) / cycles;
}

static uint64_t perf_peak_bytes_per_sec(void)
{
	return (uint64_t)PSRAM_BUS_SPEED * PSRAM_DATA_BYTES * PSRAM_DDR_EDGES;
}

static void perf_log_rate(const char *tag, uint32_t bytes, uint32_t cycles)
{
	uint64_t bps = perf_bytes_per_sec(bytes, cycles);
	uint64_t peak = perf_peak_bytes_per_sec();
	uint32_t pct = (peak == 0U) ? 0U : (uint32_t)((bps * 100U) / peak);

	LOG_INF("%s: %u bytes, %u cycles, %u.%02u MB/s, %u%% of peak %u.%02u MB/s",
		tag, bytes, cycles,
		(uint32_t)(bps / 1000000U),
		(uint32_t)((bps % 1000000U) / 10000U),
		pct,
		(uint32_t)(peak / 1000000U),
		(uint32_t)((peak % 1000000U) / 10000U));
}

static uint32_t perf_time_read(volatile uint32_t *ptr, uint32_t words)
{
	uint32_t start;
	uint32_t end;

	barrier_dsync_fence_full();
	start = k_cycle_get_32();
	for (uint32_t i = 0; i < words; i++) {
		perf_sink = ptr[i];
	}
	barrier_dsync_fence_full();
	end = k_cycle_get_32();

	return end - start;
}

static uint32_t perf_time_write(volatile uint32_t *ptr, uint32_t words, uint32_t seed)
{
	uint32_t start;
	uint32_t end;

	barrier_dsync_fence_full();
	start = k_cycle_get_32();
	for (uint32_t i = 0; i < words; i++) {
		ptr[i] = seed + i;
	}
	barrier_dsync_fence_full();
	end = k_cycle_get_32();

	return end - start;
}

static int perf_check(volatile uint32_t *ptr, uint32_t words, uint32_t seed)
{
	int errors = 0;

	for (uint32_t i = 0; i < words; i++) {
		if (ptr[i] != (seed + i)) {
			errors++;
			if (errors <= 5) {
				LOG_INF("perf mismatch word %u exp 0x%08x got 0x%08x",
					i, seed + i, ptr[i]);
			}
		}
	}

	return errors;
}



static uint32_t xip_xorshift32(uint32_t *state)
{
	uint32_t x = *state;

	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	*state = x;

	return x;
}

static uint32_t xip_time_read_reps(volatile uint32_t *ptr, uint32_t words, uint32_t reps)
{
	uint32_t start;
	uint32_t end;

	barrier_dsync_fence_full();
	start = k_cycle_get_32();
	for (uint32_t r = 0; r < reps; r++) {
		for (uint32_t i = 0; i < words; i++) {
			perf_sink = ptr[i];
		}
	}
	barrier_dsync_fence_full();
	end = k_cycle_get_32();

	return end - start;
}

static uint32_t xip_time_write_reps(volatile uint32_t *ptr, uint32_t words, uint32_t reps,
				    uint32_t seed)
{
	uint32_t start;
	uint32_t end;

	barrier_dsync_fence_full();
	start = k_cycle_get_32();
	for (uint32_t r = 0; r < reps; r++) {
		for (uint32_t i = 0; i < words; i++) {
			ptr[i] = seed + i;
		}
	}
	barrier_dsync_fence_full();
	end = k_cycle_get_32();

	return end - start;
}


/* Sustained sequential read. Timed section is loads only. */
ZTEST(xip_tests, test_read_bandwidth)
{
	volatile uint32_t *ptr = (volatile uint32_t *)PSRAM_XIP_BASE_ADDR;
	const uint32_t bytes = PSRAM_PERF_BYTES;
	const uint32_t words = bytes / sizeof(uint32_t);
	const uint32_t seed = 0x6EAD0000U;
	uint32_t cycles;
	int errors;

	if (bytes > PSRAM_TEST_SIZE) {
		ztest_test_skip();
	}

	LOG_INF("=== Read bandwidth (%u bytes, cycle counter) ===", bytes);
	for (uint32_t i = 0; i < words; i++) {
		ptr[i] = seed + i;
	}

	cycles = perf_time_read(ptr, words);
	perf_log_rate("read", bytes, cycles);
	errors = perf_check(ptr, words, seed);
	zassert_equal(errors, 0, "read bandwidth verify failed: %d errors", errors);
	zassert_true(cycles > 0U, "read bandwidth cycle count is 0");
}

/* Sustained sequential write. Verify is outside the timed section. */
ZTEST(xip_tests, test_write_bandwidth)
{
	volatile uint32_t *ptr = (volatile uint32_t *)PSRAM_XIP_BASE_ADDR;
	const uint32_t bytes = PSRAM_PERF_BYTES;
	const uint32_t words = bytes / sizeof(uint32_t);
	const uint32_t seed = 0x5717E000U;
	uint32_t cycles;
	int errors;

	if (bytes > PSRAM_TEST_SIZE) {
		ztest_test_skip();
	}

	LOG_INF("=== Write bandwidth (%u bytes, cycle counter) ===", bytes);
	cycles = perf_time_write(ptr, words, seed);
	perf_log_rate("write", bytes, cycles);
	errors = perf_check(ptr, words, seed);
	zassert_equal(errors, 0, "write bandwidth verify failed: %d errors", errors);
	zassert_true(cycles > 0U, "write bandwidth cycle count is 0");
}

/*
 * One 32-bit load. The SoC MPU maps this window non-cacheable (Ensemble
 * SRAM_NOCACHE, Balletto Device), so the access is not a cache hit.
 */
ZTEST(xip_tests, test_word_access_latency)
{
	volatile uint32_t *ptr = (volatile uint32_t *)PSRAM_XIP_BASE_ADDR;
	const uint32_t pattern = 0x1A7E0001U;
	uint32_t overhead;
	uint32_t start;
	uint32_t raw;
	uint32_t cycles;
	uint32_t got;

	*ptr = pattern;
	barrier_dsync_fence_full();

	barrier_dsync_fence_full();
	start = k_cycle_get_32();
	barrier_dsync_fence_full();
	overhead = k_cycle_get_32() - start;

	barrier_dsync_fence_full();
	start = k_cycle_get_32();
	got = *ptr;
	barrier_dsync_fence_full();
	raw = k_cycle_get_32() - start;
	cycles = (raw > overhead) ? (raw - overhead) : raw;

	LOG_INF("=== 1-word read latency (cache not used on this window) ===");
	LOG_INF("latency %u cycles (%llu ns), timer overhead %u, raw %u",
		cycles, k_cyc_to_ns_floor64(cycles), overhead, raw);
	zassert_equal(got, pattern, "latency read 0x%08x, expected 0x%08x", got, pattern);
	zassert_true(raw > 0U, "latency measurement returned 0 cycles");
}

/* Measured sequential rate divided by clock x width x DDR edges. */
ZTEST(xip_tests, test_bandwidth_efficiency)
{
	volatile uint32_t *ptr = (volatile uint32_t *)PSRAM_XIP_BASE_ADDR;
	const uint32_t bytes = PSRAM_PERF_BYTES;
	const uint32_t words = bytes / sizeof(uint32_t);
	const uint32_t seed = 0xEFF10000U;
	uint64_t peak = perf_peak_bytes_per_sec();
	uint64_t read_bps;
	uint64_t write_bps;
	int errors;

	if (bytes > PSRAM_TEST_SIZE) {
		ztest_test_skip();
	}

	LOG_INF("=== Bandwidth efficiency ===");
	LOG_INF("peak = %u Hz x %u-bit x %u DDR edges = %u.%02u MB/s",
		PSRAM_BUS_SPEED, PSRAM_DATA_BYTES * 8U, PSRAM_DDR_EDGES,
		(uint32_t)(peak / 1000000U),
		(uint32_t)((peak % 1000000U) / 10000U));
	LOG_INF("peak ignores command, address, and latency cycles");

	(void)perf_time_write(ptr, words, seed);
	write_bps = perf_bytes_per_sec(bytes, perf_time_write(ptr, words, seed));
	read_bps = perf_bytes_per_sec(bytes, perf_time_read(ptr, words));

	LOG_INF("read  %u.%02u MB/s (%u%%)",
		(uint32_t)(read_bps / 1000000U),
		(uint32_t)((read_bps % 1000000U) / 10000U),
		(peak == 0U) ? 0U : (uint32_t)((read_bps * 100U) / peak));
	LOG_INF("write %u.%02u MB/s (%u%%)",
		(uint32_t)(write_bps / 1000000U),
		(uint32_t)((write_bps % 1000000U) / 10000U),
		(peak == 0U) ? 0U : (uint32_t)((write_bps * 100U) / peak));

	errors = perf_check(ptr, words, seed);
	zassert_equal(errors, 0, "efficiency verify failed: %d errors", errors);
	zassert_true(peak > 0U, "theoretical peak is 0");
	zassert_true(read_bps > 0U && write_bps > 0U, "measured bandwidth is 0");
}


/*
 * 4 KB read on the normal non-cacheable window. A temporary cacheable MPU
 * region over this window faults, so the cached column is not measured.
 */
ZTEST(xip_tests, test_cached_vs_uncached_bandwidth)
{
	volatile uint32_t *ptr = (volatile uint32_t *)PSRAM_XIP_BASE_ADDR;
	const uint32_t bytes = PSRAM_CACHE_BYTES;
	const uint32_t words = bytes / sizeof(uint32_t);
	const uint32_t seed = 0xCAC4E000U;
	uint32_t uncached_cyc;
	int errors;

	if (bytes > PSRAM_TEST_SIZE) {
		ztest_test_skip();
	}

	LOG_INF("=== Uncached read (%u bytes) ===", bytes);
	LOG_INF("D-cache %s; this window stays non-cacheable",
		(SCB->CCR & SCB_CCR_DC_Msk) ? "enabled" : "disabled in SCB.CCR");

	(void)perf_time_write(ptr, words, seed);
	uncached_cyc = perf_time_read(ptr, words);
	perf_log_rate("uncached", bytes, uncached_cyc);
	LOG_INF("cached run not started: a cacheable MPU overlay faults this window");

	errors = perf_check(ptr, words, seed);
	zassert_equal(errors, 0, "uncached verify failed: %d errors", errors);
	zassert_true(uncached_cyc > 0U, "uncached cycle count is 0");
}
ZTEST(xip_tests, test_cached_vs_uncached_write_bandwidth)
{
	volatile uint32_t *ptr = (volatile uint32_t *)PSRAM_XIP_BASE_ADDR;
	const uint32_t bytes = PSRAM_CACHE_BYTES;
	const uint32_t words = bytes / sizeof(uint32_t);
	const uint32_t seed = 0xCA5E0000U;
	uint32_t uncached_cyc;
	int errors;

	if (bytes > PSRAM_TEST_SIZE) {
		ztest_test_skip();
	}

	LOG_INF("=== Uncached write (%u bytes) ===", bytes);
	LOG_INF("cached run not started: a cacheable MPU overlay faults this window");

	uncached_cyc = perf_time_write(ptr, words, seed);
	perf_log_rate("uncached write", bytes, uncached_cyc);

	errors = perf_check(ptr, words, seed);
	zassert_equal(errors, 0, "uncached write verify failed: %d errors", errors);
	zassert_true(uncached_cyc > 0U, "uncached write cycle count is 0");
}
ZTEST(xip_tests, test_burst_size_bandwidth)
{
	volatile uint32_t *ptr = (volatile uint32_t *)PSRAM_XIP_BASE_ADDR;
	static const uint32_t lengths[] = {16U, 32U, 64U, 128U};

	LOG_INF("=== Burst size bandwidth ===");

	for (uint32_t n = 0; n < ARRAY_SIZE(lengths); n++) {
		uint32_t bytes = lengths[n];
		uint32_t words = bytes / sizeof(uint32_t);
		uint32_t reps = 65536U / bytes;
		uint32_t total = bytes * reps;
		uint32_t cycles;
		uint32_t errors;

		cycles = xip_time_write_reps(ptr, words, reps, 0xB5720000U + n);
		perf_log_rate("burst write", total, cycles);
		LOG_INF("burst write length %u bytes", bytes);

		cycles = xip_time_read_reps(ptr, words, reps);
		perf_log_rate("burst read", total, cycles);
		LOG_INF("burst read length %u bytes", bytes);
		(void)perf_sink;

		errors = perf_check(ptr, words, 0xB5720000U + n);
		zassert_equal(errors, 0, "burst %u verify failed: %d", bytes, errors);
		zassert_true(cycles > 0U, "burst cycle count is 0");
	}
}

static uint32_t xip_copy_words(volatile uint32_t *dst, volatile uint32_t *src, uint32_t words)
{
	uint32_t start;
	uint32_t end;

	barrier_dsync_fence_full();
	start = k_cycle_get_32();
	for (uint32_t i = 0; i < words; i++) {
		dst[i] = src[i];
	}
	barrier_dsync_fence_full();
	end = k_cycle_get_32();

	return end - start;
}

ZTEST(xip_tests, test_block_copy_bandwidth)
{
	static const uint32_t lengths[] = {4096U, 65536U, 1048576U};

	LOG_INF("=== Block copy bandwidth ===");

	for (uint32_t n = 0; n < ARRAY_SIZE(lengths); n++) {
		uint32_t bytes = lengths[n];
		uint32_t words = bytes / sizeof(uint32_t);
		volatile uint32_t *src;
		volatile uint32_t *dst;
		uint32_t cycles;
		uint32_t errors = 0;

		if ((bytes * 2U) > PSRAM_TEST_SIZE) {
			LOG_INF("skip %u byte copy, device is %u bytes", bytes, PSRAM_TEST_SIZE);
			continue;
		}

		src = (volatile uint32_t *)PSRAM_XIP_BASE_ADDR;
		dst = (volatile uint32_t *)get_xip_ptr(bytes);
		for (uint32_t i = 0; i < words; i++) {
			src[i] = 0xC0790000U + i;
		}

		cycles = xip_copy_words(dst, src, words);
		perf_log_rate("block copy", bytes, cycles);
		LOG_INF("block copy length %u bytes", bytes);

		for (uint32_t i = 0; i < words; i++) {
			if (dst[i] != (0xC0790000U + i)) {
				errors++;
				if (errors <= 5U) {
					LOG_INF("copy word %u exp 0x%08x got 0x%08x",
						i, 0xC0790000U + i, dst[i]);
				}
			}
		}
		zassert_equal(errors, 0, "block copy %u failed: %u errors", bytes, errors);
		zassert_true(cycles > 0U, "block copy cycle count is 0");
	}
}


ZTEST(xip_tests, test_random_read_latency)
{
	volatile uint32_t *base = (volatile uint32_t *)PSRAM_XIP_BASE_ADDR;
	uint32_t span = PSRAM_TEST_SIZE / sizeof(uint32_t);
	uint32_t state = 0xA5A51234U;
	uint32_t min_cyc = UINT32_MAX;
	uint32_t max_cyc = 0U;
	uint64_t sum = 0U;
	const uint32_t samples = 10000U;
	uint32_t errors;

	LOG_INF("=== Random read latency (%u samples) ===", samples);

	for (uint32_t n = 0; n < samples; n++) {
		uint32_t index = xip_xorshift32(&state) % span;
		uint32_t start;
		uint32_t cycles;
		uint32_t got;

		barrier_dsync_fence_full();
		start = k_cycle_get_32();
		got = base[index];
		barrier_dsync_fence_full();
		cycles = k_cycle_get_32() - start;
		perf_sink = got;

		if (cycles < min_cyc) {
			min_cyc = cycles;
		}
		if (cycles > max_cyc) {
			max_cyc = cycles;
		}
		sum += cycles;
	}

	LOG_INF("random read min %u cycles, max %u cycles, avg %u cycles",
		min_cyc, max_cyc, (uint32_t)(sum / samples));

	state = 0x51A70001U;
	for (uint32_t i = 0; i < 256U; i++) {
		uint32_t index = xip_xorshift32(&state) % span;

		base[index] = 0x51A70000U + i;
	}

	state = 0x51A70001U;
	errors = 0U;
	for (uint32_t i = 0; i < 256U; i++) {
		uint32_t index = xip_xorshift32(&state) % span;
		uint32_t expected = 0x51A70000U + i;
		uint32_t got = base[index];

		if (got != expected) {
			bool overwritten = false;
			uint32_t replay = 0x51A70001U;

			for (uint32_t j = 0; j < 256U; j++) {
				uint32_t later = xip_xorshift32(&replay) % span;

				if (j > i && later == index) {
					overwritten = true;
					break;
				}
				if (j == i && later != index) {
					break;
				}
			}
			if (!overwritten) {
				errors++;
			}
		}
	}
	(void)perf_sink;
	zassert_equal(errors, 0, "random write check failed: %u errors", errors);
	zassert_true(max_cyc > 0U, "latency samples are 0");
}


ZTEST(xip_tests, test_frequency_sweep_record)
{
	volatile uint32_t *ptr = (volatile uint32_t *)PSRAM_XIP_BASE_ADDR;
	const uint32_t bytes = PSRAM_PERF_BYTES;
	const uint32_t words = bytes / sizeof(uint32_t);
	const uint32_t seed = 0xF4E90000U;
	uint32_t read_cyc;
	uint32_t write_cyc;
	uint64_t read_bps;
	uint64_t write_bps;
	int errors;

	if (bytes > PSRAM_TEST_SIZE) {
		ztest_test_skip();
	}

	LOG_INF("=== Frequency row for this image ===");
	LOG_INF("bus-speed %u Hz. One image, one clock. Latency was set at boot.", PSRAM_BUS_SPEED);

	for (uint32_t i = 0; i < 256U; i++) {
		ptr[i] = (i & 1U) ? 0x55555555U : 0xAAAAAAAAU;
	}
	errors = 0;
	for (uint32_t i = 0; i < 256U; i++) {
		uint32_t expected = (i & 1U) ? 0x55555555U : 0xAAAAAAAAU;

		if (ptr[i] != expected) {
			errors++;
		}
	}

	write_cyc = perf_time_write(ptr, words, seed);
	read_cyc = perf_time_read(ptr, words);
	errors += perf_check(ptr, words, seed);
	read_bps = perf_bytes_per_sec(bytes, read_cyc);
	write_bps = perf_bytes_per_sec(bytes, write_cyc);

	LOG_INF("FREQ_ROW Hz %u functional %s read %u.%02u MB/s write %u.%02u MB/s errors %d",
		PSRAM_BUS_SPEED, errors == 0 ? "PASS" : "FAIL",
		(uint32_t)(read_bps / 1000000U),
		(uint32_t)((read_bps % 1000000U) / 10000U),
		(uint32_t)(write_bps / 1000000U),
		(uint32_t)((write_bps % 1000000U) / 10000U),
		errors);
	zassert_equal(errors, 0, "frequency row compare failed: %d errors", errors);
}


/* Timed XIP R/W at the boot-time latency-code (no runtime reconfig API). */
ZTEST(xip_tests, test_xip_rw_timing)
{
	volatile uint32_t *ptr = (volatile uint32_t *)PSRAM_XIP_BASE_ADDR;
	uint32_t test_pattern = 0xCAFEBABE;
	int64_t elapsed_ms;

	LOG_INF("=== XIP R/W timing (DT latency-code, not reconfigured) ===");

	elapsed_ms = k_uptime_get();
	for (int i = 0; i < 1000; i++) {
		*ptr = test_pattern;
		volatile uint32_t dummy = *ptr;

		(void)dummy;
	}
	elapsed_ms = k_uptime_get() - elapsed_ms;

	LOG_INF("1000 R/W operations took %lld ms", elapsed_ms);

	*ptr = test_pattern;
	zassert_equal(*ptr, test_pattern, "XIP R/W timing data check failed");
}

#endif /* !FULL_CHIP */

/*
 * Ensemble OSPI1 XIP is read-only. A higher region number wins on overlap,
 * so map the spi-psram parent's xip-base-address as privileged read/write,
 * non-cacheable, above whichever SoC region covers that base (OSPI0 or OSPI1).
 */
static void xip_mpu_map_dt_window(void)
{
	static bool mapped;
	const uint32_t base = PSRAM_XIP_BASE_ADDR;
	const uint32_t size = PSRAM_XIP_WIN_SIZE;
	const uint32_t nregions =
		(MPU->TYPE & MPU_TYPE_DREGION_Msk) >> MPU_TYPE_DREGION_Pos;
	int cover = -1;
	int highest = -1;
	int slot = -1;
	int above;

	if (mapped) {
		return;
	}

	zassert_true(size >= 32U && (base % 32U) == 0U && (size % 32U) == 0U,
		     "DT XIP window 0x%08x size %u is not MPU-aligned", base, size);

	for (uint32_t i = 0; i < nregions; i++) {
		uint32_t rbar;
		uint32_t rlar;
		uint32_t rbase;
		uint32_t rlast;

		MPU->RNR = i;
		rbar = MPU->RBAR;
		rlar = MPU->RLAR;
		if ((rlar & MPU_RLAR_EN_Msk) == 0U) {
			continue;
		}

		highest = (int)i;
		rbase = rbar & MPU_RBAR_BASE_Msk;
		rlast = (rlar & MPU_RLAR_LIMIT_Msk) | ~MPU_RLAR_LIMIT_Msk;
		if (base >= rbase && base <= rlast) {
			cover = (int)i;
		}
	}

	/* No SoC region (E8 OSPI0): still sit above the last fixed region. */
	above = (cover >= 0) ? cover : highest;
	for (uint32_t i = (uint32_t)above + 1U; i < nregions; i++) {
		MPU->RNR = i;
		if ((MPU->RLAR & MPU_RLAR_EN_Msk) == 0U) {
			slot = (int)i;
			break;
		}
	}

	zassert_true(slot > above,
		     "no free MPU region above SoC region %d for XIP 0x%08x",
		     above, base);

	MPU->RNR = (uint32_t)slot;
	MPU->RBAR = (base & MPU_RBAR_BASE_Msk) |
		    (NOT_EXEC | P_RW_U_NA_Msk | NON_SHAREABLE_Msk);
	MPU->RLAR = (REGION_LIMIT_ADDR(base, size) & MPU_RLAR_LIMIT_Msk) |
		    ((MPU_MAIR_INDEX_SRAM_NOCACHE << MPU_RLAR_AttrIndx_Pos) &
		     MPU_RLAR_AttrIndx_Msk) |
		    MPU_RLAR_EN_Msk;
	barrier_dsync_fence_full();
	barrier_isync_fence_full();
	mapped = true;

	LOG_INF("MPU[%d] XIP 0x%08x size 0x%08x privileged RW, non-cacheable (above %d)",
		slot, base, size, above);
}

static void *psram_test_suite_setup(void)
{
	LOG_INF("=============================================");
	LOG_INF("%s XIP Test Suite Started", XIP_DEV_NAME);
	LOG_INF("=============================================");
	xip_mpu_map_dt_window();
	return NULL;
}

static void psram_test_suite_teardown(void *fixture)
{
	ARG_UNUSED(fixture);
	LOG_INF("=============================================");
	LOG_INF("%s XIP Test Suite Completed", XIP_DEV_NAME);
	LOG_INF("=============================================");
}

static void psram_test_before(void *fixture)
{
	ARG_UNUSED(fixture);
	zassert_true(device_is_ready(psram_dev),
		     "%s device not ready: %s", XIP_DEV_NAME, psram_dev->name);
	LOG_INF("--- Test starting ---");
}

static void psram_test_after(void *fixture)
{
	ARG_UNUSED(fixture);
	LOG_INF("--- Test completed ---");
}

ZTEST_SUITE(xip_tests, NULL, psram_test_suite_setup,
	    psram_test_before, psram_test_after, psram_test_suite_teardown);

/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#include <string.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

#if !DT_HAS_ALIAS(spi_psram)
#error "Devicetree alias spi-psram is required (boards/alif_psram.overlay)"
#endif

LOG_MODULE_REGISTER(psram);

/* Same DT usage as samples/drivers/spi_psram */
#define PSRAM_NODE                   DT_ALIAS(spi_psram)
#define PSRAM_CTRL_NODE              DT_PARENT(PSRAM_NODE)
#define PSRAM_TEST_SIZE              DT_PROP(PSRAM_NODE, size)
#define PSRAM_XIP_BASE_ADDR          DT_PROP_BY_IDX(PSRAM_CTRL_NODE, \
						   xip_base_address, 0)
#define PSRAM_XIP_WIN_SIZE           DT_PROP_BY_IDX(PSRAM_CTRL_NODE, \
						   xip_base_address, 1)
#define PSRAM_BUS_SPEED              DT_PROP(PSRAM_CTRL_NODE, bus_speed)
#define PSRAM_TEST_CHUNK_SIZE        1024
#define PSRAM_PERF_ITERATIONS        100
#define PSRAM_REGION2_OFF            32768
#define PSRAM_REFRESH_LOAD_MS        2000

static const struct device *const psram_dev = DEVICE_DT_GET(PSRAM_NODE);

/* Verify buffer against expected */
static int verify_data(const uint8_t *expected, const uint8_t *actual,
		       size_t len, const char *tag)
{
	int e_count = 0;

	for (size_t i = 0; i < len; i++) {
		if (actual[i] != expected[i]) {
			e_count++;
			if (e_count <= 5) {
				LOG_INF("%s mismatch at [%zu] w[0x%02x] r[0x%02x]",
					tag, i, expected[i], actual[i]);
			}
		}
	}
	return e_count;
}

static uint8_t *get_xip_ptr(uint32_t offset)
{
	zassert_true(offset < PSRAM_TEST_SIZE,
		     "XIP offset 0x%x outside size %u", offset, PSRAM_TEST_SIZE);
	return (uint8_t *)(PSRAM_XIP_BASE_ADDR + offset);
}

static void log_throughput_kibs(const char *tag, int64_t elapsed_ms, uint32_t bytes)
{
	if (elapsed_ms <= 0) {
		LOG_INF("%s: <1 ms for %u bytes", tag, bytes);
		return;
	}

	uint32_t kib_s = (uint32_t)(((uint64_t)bytes * 1000U) /
				    (uint64_t)elapsed_ms / 1024U);

	LOG_INF("%s: %lld ms, %u KiB/s", tag, elapsed_ms, kib_s);
}

/* XIP base address accessibility */
ZTEST(psram_tests, test_xip_base_address_test)
{
	volatile uint32_t *xip_ptr = (volatile uint32_t *)PSRAM_XIP_BASE_ADDR;
	uint32_t test_pattern = 0x12345678;
	uint32_t readback;

	LOG_INF("=== XIP Base Address Test ===");
	LOG_INF("XIP Base Address: 0x%08X", PSRAM_XIP_BASE_ADDR);
	LOG_INF("PSRAM Size: %u bytes (%u MB)",
		PSRAM_TEST_SIZE, PSRAM_TEST_SIZE / (1024 * 1024));

	/* Test write to XIP region */
	*xip_ptr = test_pattern;

	/* Test read from XIP region */
	readback = *xip_ptr;

	zassert_equal(readback, test_pattern,
		      "XIP read/write failed: wrote 0x%08X, read 0x%08X",
		      test_pattern, readback);

	LOG_INF("XIP base address test PASSED");
}

/* -------- Test: basic word read/write -------- */
ZTEST(psram_tests, test_basic_word_rw_test)
{
	volatile uint32_t *ptr = (volatile uint32_t *)PSRAM_XIP_BASE_ADDR;
	uint32_t patterns[] = {0x00000000, 0xFFFFFFFF, 0xAAAAAAAA, 0x55555555,
			       0x12345678, 0x9ABCDEF0, 0x55AA33CC, 0x11223344};

	LOG_INF("=== Basic Word R/W Test ===");

	for (int i = 0; i < ARRAY_SIZE(patterns); i++) {
		ptr[i] = patterns[i];
	}

	for (int i = 0; i < ARRAY_SIZE(patterns); i++) {
		uint32_t readback = ptr[i];

		zassert_equal(readback, patterns[i],
			      "Word[%d] mismatch: wrote 0x%08X, read 0x%08X",
			      i, patterns[i], readback);
	}

	LOG_INF("Basic word R/W test PASSED");
}

/* -------- Test: byte access -------- */
/* Note: In x16 (dual octal) mode, minimum HW access is 16-bit.
 * ARM bus interface handles byte access via read-modify-write.
 * This test validates that byte-level access works correctly.
 */
ZTEST(psram_tests, test_byte_access_test)
{
	volatile uint8_t *ptr = (volatile uint8_t *)PSRAM_XIP_BASE_ADDR;
	uint8_t pattern[8] = {0x00, 0xFF, 0x55, 0xAA, 0x12, 0x34, 0x78, 0x9A};

	LOG_INF("=== Byte Access Test ===");

	/* Write bytes individually */
	for (int i = 0; i < 8; i++) {
		ptr[i] = pattern[i];
	}

	/* Read back and verify */
	for (int i = 0; i < 8; i++) {
		uint8_t readback = ptr[i];

		zassert_equal(readback, pattern[i],
		      "Byte[%d] mismatch: wrote 0x%02X, read 0x%02X",
		      i, pattern[i], readback);
	}

	LOG_INF("Byte access test PASSED");
}

/* -------- Test: halfword (16-bit) access -------- */
ZTEST(psram_tests, test_halfword_access_test)
{
	volatile uint16_t *ptr = (volatile uint16_t *)PSRAM_XIP_BASE_ADDR;
	uint16_t patterns[] = {0x0000, 0xFFFF, 0xAAAA, 0x5555, 0x1234, 0x5678, 0x9ABC, 0xDEF0};

	LOG_INF("=== Halfword Access Test ===");

	for (int i = 0; i < ARRAY_SIZE(patterns); i++) {
		ptr[i] = patterns[i];
	}

	for (int i = 0; i < ARRAY_SIZE(patterns); i++) {
		uint16_t readback = ptr[i];

		zassert_equal(readback, patterns[i],
		      "Halfword[%d] mismatch: wrote 0x%04X, read 0x%04X",
		      i, patterns[i], readback);
	}

	LOG_INF("Halfword access test PASSED");
}

/* -------- Test: sequential write/read -------- */
ZTEST(psram_tests, test_sequential_rw_test)
{
	volatile uint32_t *ptr = (volatile uint32_t *)PSRAM_XIP_BASE_ADDR;
	uint32_t num_words = PSRAM_TEST_CHUNK_SIZE / sizeof(uint32_t);
	int e_count = 0;

	LOG_INF("=== Sequential R/W Test (%u bytes) ===", PSRAM_TEST_CHUNK_SIZE);

	/* Sequential write */
	for (uint32_t i = 0; i < num_words; i++) {
		ptr[i] = i;
	}

	/* Sequential read and verify */
	for (uint32_t i = 0; i < num_words; i++) {
		uint32_t readback = ptr[i];

		if (readback != i) {
			e_count++;
			if (e_count <= 5) {
				LOG_INF("Mismatch at [%u]: expected %u, got %u",
					i, i, readback);
			}
		}
	}

	zassert_equal(e_count, 0, "Sequential R/W failed: %d mismatches", e_count);
	LOG_INF("Sequential R/W test PASSED");
}

/* -------- Test: data pattern robustness -------- */
ZTEST(psram_tests, test_data_pattern_test)
{
	uint8_t *xip_ptr = get_xip_ptr(0);
	uint8_t w_buf[PSRAM_TEST_CHUNK_SIZE];
	uint8_t r_buf[PSRAM_TEST_CHUNK_SIZE];
	int e_count;

	LOG_INF("=== Data Pattern Robustness Test ===");

	/* Test 1: All zeros */
	LOG_INF("Test all-zeros pattern");
	memset(w_buf, 0x00, PSRAM_TEST_CHUNK_SIZE);
	memcpy(xip_ptr, w_buf, PSRAM_TEST_CHUNK_SIZE);
	memcpy(r_buf, xip_ptr, PSRAM_TEST_CHUNK_SIZE);
	e_count = verify_data(w_buf, r_buf, PSRAM_TEST_CHUNK_SIZE, "all_zeros");
	zassert_equal(e_count, 0, "All-zeros pattern failed");

	/* Test 2: All ones */
	LOG_INF("Test all-ones pattern");
	memset(w_buf, 0xFF, PSRAM_TEST_CHUNK_SIZE);
	memcpy(xip_ptr, w_buf, PSRAM_TEST_CHUNK_SIZE);
	memcpy(r_buf, xip_ptr, PSRAM_TEST_CHUNK_SIZE);
	e_count = verify_data(w_buf, r_buf, PSRAM_TEST_CHUNK_SIZE, "all_ones");
	zassert_equal(e_count, 0, "All-ones pattern failed");

	/* Test 3: Alternating 0xAA */
	LOG_INF("Test 0xAA pattern");
	memset(w_buf, 0xAA, PSRAM_TEST_CHUNK_SIZE);
	memcpy(xip_ptr, w_buf, PSRAM_TEST_CHUNK_SIZE);
	memcpy(r_buf, xip_ptr, PSRAM_TEST_CHUNK_SIZE);
	e_count = verify_data(w_buf, r_buf, PSRAM_TEST_CHUNK_SIZE, "0xAA");
	zassert_equal(e_count, 0, "0xAA pattern failed");

	/* Test 4: Alternating 0x55 */
	LOG_INF("Test 0x55 pattern");
	memset(w_buf, 0x55, PSRAM_TEST_CHUNK_SIZE);
	memcpy(xip_ptr, w_buf, PSRAM_TEST_CHUNK_SIZE);
	memcpy(r_buf, xip_ptr, PSRAM_TEST_CHUNK_SIZE);
	e_count = verify_data(w_buf, r_buf, PSRAM_TEST_CHUNK_SIZE, "0x55");
	zassert_equal(e_count, 0, "0x55 pattern failed");

	/* Test 5: Walking ones */
	LOG_INF("Test walking bit pattern");
	for (int i = 0; i < PSRAM_TEST_CHUNK_SIZE; i++) {
		w_buf[i] = (1 << (i % 8));
	}
	memcpy(xip_ptr, w_buf, PSRAM_TEST_CHUNK_SIZE);
	memcpy(r_buf, xip_ptr, PSRAM_TEST_CHUNK_SIZE);
	e_count = verify_data(w_buf, r_buf, PSRAM_TEST_CHUNK_SIZE, "walking");
	zassert_equal(e_count, 0, "Walking bit pattern failed");

	LOG_INF("Data pattern robustness test PASSED");
}

/* -------- Test: address boundary edge cases -------- */
ZTEST(psram_tests, test_address_boundary_test)
{
	volatile uint32_t *ptr;
	uint32_t test_pattern = 0xDEADBEEF;

	LOG_INF("=== Address Boundary Test ===");

	/* Test at offset 0 */
	ptr = (volatile uint32_t *)get_xip_ptr(0);
	*ptr = test_pattern;
	zassert_equal(*ptr, test_pattern, "Address 0 test failed");
	LOG_INF("Address 0x0 boundary test PASSED");

	if (PSRAM_TEST_SIZE > 1024) {
		ptr = (volatile uint32_t *)get_xip_ptr(1024);
		*ptr = test_pattern;
		zassert_equal(*ptr, test_pattern, "1KB boundary test failed");
		LOG_INF("1KB boundary test PASSED");
	}

	if (PSRAM_TEST_SIZE > 4096) {
		ptr = (volatile uint32_t *)get_xip_ptr(4096);
		*ptr = test_pattern;
		zassert_equal(*ptr, test_pattern, "4KB boundary test failed");
		LOG_INF("4KB boundary test PASSED");
	}

	if (PSRAM_TEST_SIZE > 65536) {
		ptr = (volatile uint32_t *)get_xip_ptr(65536);
		*ptr = test_pattern;
		zassert_equal(*ptr, test_pattern, "64KB boundary test failed");
		LOG_INF("64KB boundary test PASSED");
	}

	/* Test near end of PSRAM */
	if (PSRAM_TEST_SIZE > 65536 + 1024) {
		ptr = (volatile uint32_t *)get_xip_ptr(PSRAM_TEST_SIZE - 1024);
		*ptr = test_pattern;
		zassert_equal(*ptr, test_pattern, "Near-end boundary test failed");
		LOG_INF("Near-end boundary test PASSED");
	}

	LOG_INF("Address boundary test PASSED");
}

#if IS_ENABLED(CONFIG_TEST_PSRAM_FULL_CHIP)
/* Full array word R/W — same coverage as samples/drivers/spi_psram */
ZTEST(psram_tests, test_full_memory_rw_test)
{
	volatile uint32_t *ptr;
	uint32_t test_chunk = 4096;
	uint32_t chunks = PSRAM_TEST_SIZE / test_chunk;
	uint32_t rem = PSRAM_TEST_SIZE % test_chunk;
	uint32_t errors = 0;
	int64_t start_time, end_time;

	BUILD_ASSERT(PSRAM_TEST_SIZE / 4096 <= 0xFFFF,
		     "Pattern encoding overflows for this PSRAM size");

	LOG_INF("=== Full Memory R/W Test (%u bytes) ===", PSRAM_TEST_SIZE);

	start_time = k_uptime_get();

	for (uint32_t chunk = 0; chunk < chunks; chunk++) {
		uint32_t chunk_offset = chunk * test_chunk;
		uint32_t words_in_chunk = test_chunk / sizeof(uint32_t);

		ptr = (volatile uint32_t *)get_xip_ptr(chunk_offset);

		for (uint32_t i = 0; i < words_in_chunk; i++) {
			ptr[i] = (chunk << 16) | i;
		}

		for (uint32_t i = 0; i < words_in_chunk; i++) {
			uint32_t expected = (chunk << 16) | i;

			if (ptr[i] != expected) {
				errors++;
			}
		}

		if (chunk % 16 == 0) {
			LOG_INF("Processed chunk %u/%u", chunk, chunks);
		}
	}

	if (rem >= sizeof(uint32_t)) {
		uint32_t words = rem / sizeof(uint32_t);

		ptr = (volatile uint32_t *)get_xip_ptr(chunks * test_chunk);
		for (uint32_t i = 0; i < words; i++) {
			ptr[i] = 0xFFFF0000U | i;
		}
		for (uint32_t i = 0; i < words; i++) {
			if (ptr[i] != (0xFFFF0000U | i)) {
				errors++;
			}
		}
	}

	end_time = k_uptime_get();

	LOG_INF("Full memory test completed in %lld ms", end_time - start_time);
	zassert_equal(errors, 0, "Full memory test failed: %u errors", errors);
	LOG_INF("Full memory R/W test PASSED");
}
#endif

/* -------- Test: read/write performance -------- */
ZTEST(psram_tests, test_performance_test)
{
	uint8_t *xip_ptr;
	uint8_t buf[PSRAM_TEST_CHUNK_SIZE];
	int64_t write_ms, read_ms;
	uint32_t total_bytes = PSRAM_PERF_ITERATIONS * PSRAM_TEST_CHUNK_SIZE;

	if (PSRAM_TEST_SIZE < 16U * PSRAM_TEST_CHUNK_SIZE) {
		ztest_test_skip();
	}

	xip_ptr = get_xip_ptr(0);

	LOG_INF("=== Performance Test (%d iterations) ===", PSRAM_PERF_ITERATIONS);

	for (int i = 0; i < PSRAM_TEST_CHUNK_SIZE; i++) {
		buf[i] = i % 256;
	}

	memcpy(xip_ptr, buf, PSRAM_TEST_CHUNK_SIZE);
	memcpy(buf, xip_ptr, PSRAM_TEST_CHUNK_SIZE);

	write_ms = k_uptime_get();
	for (int i = 0; i < PSRAM_PERF_ITERATIONS; i++) {
		memcpy(xip_ptr + (i % 16) * PSRAM_TEST_CHUNK_SIZE, buf,
		       PSRAM_TEST_CHUNK_SIZE);
	}
	write_ms = k_uptime_get() - write_ms;

	read_ms = k_uptime_get();
	for (int i = 0; i < PSRAM_PERF_ITERATIONS; i++) {
		memcpy(buf, xip_ptr + (i % 16) * PSRAM_TEST_CHUNK_SIZE,
		       PSRAM_TEST_CHUNK_SIZE);
	}
	read_ms = k_uptime_get() - read_ms;

	log_throughput_kibs("Write", write_ms, total_bytes);
	log_throughput_kibs("Read", read_ms, total_bytes);
	LOG_INF("Bus speed configured: %u Hz", PSRAM_BUS_SPEED);
}

/* -------- Test: unaligned access -------- */
/* Note: In x16 mode, PSRAM native access is 16-bit word aligned.
 * This test validates that the AHB bus and address shim handle
 * unaligned byte offsets correctly via memcpy.
 */
ZTEST(psram_tests, test_unaligned_access_test)
{
	uint8_t test_data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
	uint8_t read_buf[8];
	int e_count;

	LOG_INF("=== Unaligned Access Test ===");

	for (int offset = 1; offset < 8; offset++) {
		uint8_t *unaligned_ptr = get_xip_ptr(offset);

		/* Use byte-wise copy to avoid unaligned word access faults on ARM Cortex-M */
		for (int i = 0; i < 8; i++) {
			unaligned_ptr[i] = test_data[i];
		}
		for (int i = 0; i < 8; i++) {
			read_buf[i] = unaligned_ptr[i];
		}

		e_count = verify_data(test_data, read_buf, 8, "unaligned");
		zassert_equal(e_count, 0, "Unaligned access at offset %d failed", offset);
	}

	LOG_INF("Unaligned access test PASSED");
}

/* -------- Test: memory stress (alternating patterns) -------- */
ZTEST(psram_tests, test_memory_stress_test)
{
	volatile uint32_t *xip_ptr = (volatile uint32_t *)get_xip_ptr(0);
	uint32_t num_words = PSRAM_TEST_CHUNK_SIZE / sizeof(uint32_t);
	uint32_t patterns[] = {0xFFFFFFFF, 0x00000000, 0xAAAAAAAA, 0x55555555};

	LOG_INF("=== Memory Stress Test ===");

	for (int iter = 0; iter < 10; iter++) {
		for (int p = 0; p < ARRAY_SIZE(patterns); p++) {
			/* Fill with pattern */
			for (uint32_t i = 0; i < num_words; i++) {
				xip_ptr[i] = patterns[p];
			}

			/* Verify */
			for (uint32_t i = 0; i < num_words; i++) {
				zassert_equal(xip_ptr[i], patterns[p],
				      "Stress test failed at iter %d, pattern %d, word %u",
				      iter, p, i);
			}
		}
	}

	LOG_INF("Memory stress test PASSED");
}

/* -------- Test: concurrent region access -------- */
ZTEST(psram_tests, test_concurrent_region_test)
{
	uint32_t *region1;
	uint32_t *region2;
	uint32_t pattern1 = 0x11111111;
	uint32_t pattern2 = 0x22222222;

	if (PSRAM_TEST_SIZE <= PSRAM_REGION2_OFF + sizeof(uint32_t)) {
		ztest_test_skip();
	}

	region1 = (uint32_t *)get_xip_ptr(0);
	region2 = (uint32_t *)get_xip_ptr(PSRAM_REGION2_OFF);

	LOG_INF("=== Concurrent Region Access Test ===");

	/* Write to both regions */
	*region1 = pattern1;
	*region2 = pattern2;

	/* Read from both regions */
	zassert_equal(*region1, pattern1, "Region 1 data corrupted");
	zassert_equal(*region2, pattern2, "Region 2 data corrupted");

	LOG_INF("Concurrent region test PASSED");
}

/* -------- Test: large block transfer -------- */
ZTEST(psram_tests, test_large_block_transfer_test)
{
	uint8_t *src_buf;
	uint8_t *verify_buf;
	uint8_t *xip_ptr;
	int e_count;

	if (PSRAM_TEST_SIZE < 16384) {
		ztest_test_skip();
	}

	src_buf = k_malloc(16384);
	verify_buf = k_malloc(16384);
	xip_ptr = get_xip_ptr(0);

	LOG_INF("=== Large Block Transfer Test (16KB) ===");
	zassert_not_null(src_buf, "Failed to allocate source buffer");
	if (verify_buf == NULL) {
		k_free(src_buf);
		zassert_not_null(verify_buf, "Failed to allocate verify buffer");
	}

	/* Fill source with pattern */
	for (int i = 0; i < 16384; i++) {
		src_buf[i] = (i * 7 + 13) % 256;
	}

	/* Write to PSRAM */
	memcpy(xip_ptr, src_buf, 16384);

	/* Read back */
	memcpy(verify_buf, xip_ptr, 16384);

	/* Verify */
	e_count = verify_data(src_buf, verify_buf, 16384, "large_block");
	zassert_equal(e_count, 0, "Large block transfer failed: %d errors", e_count);

	k_free(src_buf);
	k_free(verify_buf);

	LOG_INF("Large block transfer test PASSED");
}

ZTEST(psram_tests, test_memset_memcmp_test)
{
	uint8_t *xip_ptr = get_xip_ptr(0);
	uint8_t expected[PSRAM_TEST_CHUNK_SIZE];
	uint8_t verify_buf[PSRAM_TEST_CHUNK_SIZE];

	LOG_INF("=== Memset/Memcmp Test ===");

	memset(expected, 0xAB, PSRAM_TEST_CHUNK_SIZE);
	memset(xip_ptr, 0xAB, PSRAM_TEST_CHUNK_SIZE);
	memcpy(verify_buf, xip_ptr, PSRAM_TEST_CHUNK_SIZE);
	zassert_mem_equal(verify_buf, expected, PSRAM_TEST_CHUNK_SIZE,
			  "memset/memcmp of XIP region failed");

	LOG_INF("Memset/memcmp test PASSED");
}

/* -------- Test: DMA-like sequential burst -------- */
ZTEST(psram_tests, test_sequential_burst_test)
{
	volatile uint32_t *ptr = (volatile uint32_t *)PSRAM_XIP_BASE_ADDR;
	uint32_t burst_sizes[] = {16, 64, 256, 1024, 4096};

	LOG_INF("=== Sequential Burst Test ===");

	for (int b = 0; b < ARRAY_SIZE(burst_sizes); b++) {
		uint32_t words = burst_sizes[b] / sizeof(uint32_t);

		/* Burst write */
		for (uint32_t i = 0; i < words; i++) {
			ptr[i] = (i << 16) | i;
		}

		/* Burst read and verify */
		for (uint32_t i = 0; i < words; i++) {
			uint32_t expected = (i << 16) | i;
			uint32_t readback = ptr[i];

			zassert_equal(readback, expected,
			      "Burst %u bytes failed at word %u: expected 0x%08X, got 0x%08X",
			      burst_sizes[b], i, expected, readback);
		}

		LOG_INF("Burst %u bytes PASSED", burst_sizes[b]);
	}

	LOG_INF("Sequential burst test PASSED");
}

ZTEST(psram_tests, test_xip_config_test)
{
	LOG_INF("PSRAM device: %s", psram_dev->name);
	LOG_INF("XIP base: 0x%08x window %u bytes", PSRAM_XIP_BASE_ADDR,
		PSRAM_XIP_WIN_SIZE);
	LOG_INF("PSRAM size: %u bytes, bus-speed: %u Hz",
		PSRAM_TEST_SIZE, PSRAM_BUS_SPEED);

	zassert_true(PSRAM_TEST_SIZE > 0, "DT size must be non-zero");
	zassert_true(PSRAM_XIP_BASE_ADDR != 0, "DT xip-base-address[0] is 0");
	zassert_true(PSRAM_TEST_SIZE <= PSRAM_XIP_WIN_SIZE,
		     "PSRAM size %u exceeds XIP window %u",
		     PSRAM_TEST_SIZE, PSRAM_XIP_WIN_SIZE);
	zassert_true(PSRAM_BUS_SPEED > 0, "DT bus-speed is 0");
}

/* Timed XIP R/W at the boot-time latency-code (no runtime reconfig API). */
ZTEST(psram_tests, test_xip_rw_timing)
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

/* -------- Test: DDR data integrity -------- */
/* DDR transfers data on both clock edges. This test uses patterns
 * that stress the DDR transition boundaries (0x5555/0xAAAA alternating)
 * to detect any setup/hold timing issues.
 */
ZTEST(psram_tests, test_ddr_data_integrity)
{
	volatile uint32_t *ptr = (volatile uint32_t *)PSRAM_XIP_BASE_ADDR;
	uint32_t ddr_patterns[] = {
		0x5555AAAA, 0xAAAA5555, /* Max transitions on both edges */
		0x33333333, 0xCCCCCCCC, /* 2-bit alternating */
		0x0F0F0F0F, 0xF0F0F0F0, /* Nibble alternating */
		0x00FF00FF, 0xFF00FF00, /* Byte alternating */
	};
	uint32_t num_patterns = ARRAY_SIZE(ddr_patterns);
	uint32_t num_words = 256; /* Test 1KB region */

	LOG_INF("=== DDR Data Integrity Test ===");

	for (uint32_t p = 0; p < num_patterns; p++) {
		/* Fill region with DDR-stressing pattern */
		for (uint32_t i = 0; i < num_words; i++) {
			ptr[i] = ddr_patterns[p];
		}

		/* Verify all words */
		for (uint32_t i = 0; i < num_words; i++) {
			zassert_equal(ptr[i], ddr_patterns[p],
				"DDR pattern 0x%08X failed at word %u: got 0x%08X",
				ddr_patterns[p], i, ptr[i]);
		}
		LOG_INF("DDR pattern 0x%08X PASSED", ddr_patterns[p]);
	}

	/* Interleaved pattern: alternate between two DDR-stressing values */
	for (uint32_t i = 0; i < num_words; i++) {
		ptr[i] = (i & 1) ? 0x5555AAAA : 0xAAAA5555;
	}
	for (uint32_t i = 0; i < num_words; i++) {
		uint32_t expected = (i & 1) ? 0x5555AAAA : 0xAAAA5555;

		zassert_equal(ptr[i], expected,
			"DDR interleaved pattern failed at word %u", i);
	}

	LOG_INF("DDR data integrity test PASSED");
}

/* -------- Test: 16-bit data bus validation -------- */
/* APS512XXN uses 16-bit data bus DQ[15:0]. This test exercises
 * all 16 data lines individually using walking-1 and walking-0
 * patterns at halfword (native bus width) granularity.
 */
ZTEST(psram_tests, test_16bit_databus_validation)
{
	volatile uint16_t *ptr = (volatile uint16_t *)PSRAM_XIP_BASE_ADDR;
	uint16_t readback;

	LOG_INF("=== 16-bit Data Bus Validation Test ===");

	/* Walking-1 on all 16 data lines */
	LOG_INF("Walking-1 pattern on DQ[15:0]");
	for (int bit = 0; bit < 16; bit++) {
		uint16_t pattern = (1U << bit);

		ptr[bit] = pattern;
	}
	for (int bit = 0; bit < 16; bit++) {
		uint16_t expected = (1U << bit);

		readback = ptr[bit];
		zassert_equal(readback, expected,
			"DQ[%d] walking-1 failed: wrote 0x%04X, read 0x%04X",
			bit, expected, readback);
	}

	/* Walking-0 on all 16 data lines */
	LOG_INF("Walking-0 pattern on DQ[15:0]");
	for (int bit = 0; bit < 16; bit++) {
		uint16_t pattern = ~(1U << bit) & 0xFFFF;

		ptr[bit] = pattern;
	}
	for (int bit = 0; bit < 16; bit++) {
		uint16_t expected = ~(1U << bit) & 0xFFFF;

		readback = ptr[bit];
		zassert_equal(readback, expected,
			"DQ[%d] walking-0 failed: wrote 0x%04X, read 0x%04X",
			bit, expected, readback);
	}

	/* All-lines-high / all-lines-low */
	ptr[0] = 0xFFFF;
	zassert_equal(ptr[0], 0xFFFF, "All DQ lines high failed");
	ptr[0] = 0x0000;
	zassert_equal(ptr[0], 0x0000, "All DQ lines low failed");

	LOG_INF("16-bit data bus validation test PASSED");
}

/* Sequential access across 16/32/64/128-word offsets (burst length is
 * programmed at driver init; this does not change wrap mode).
 */
ZTEST(psram_tests, test_burst_length_boundaries)
{
	volatile uint32_t *ptr = (volatile uint32_t *)PSRAM_XIP_BASE_ADDR;
	uint32_t burst_words[] = {16, 32, 64, 128};

	LOG_INF("=== Burst-length offset test ===");

	for (int b = 0; b < ARRAY_SIZE(burst_words); b++) {
		uint32_t blen = burst_words[b];
		uint32_t boundary_offset = blen; /* Start at burst boundary */

		/* Write across the burst boundary: blen words before and after */
		for (uint32_t i = 0; i < blen * 2; i++) {
			ptr[boundary_offset - blen + i] = 0xB0000000 | (b << 16) | i;
		}

		/* Verify data across boundary */
		int errors = 0;

		for (uint32_t i = 0; i < blen * 2; i++) {
			uint32_t expected = 0xB0000000 | (b << 16) | i;

			if (ptr[boundary_offset - blen + i] != expected) {
				errors++;
				if (errors <= 3) {
					LOG_INF("Burst[%u] boundary err at word %u: "
						"exp 0x%08X got 0x%08X",
						blen, i, expected,
						ptr[boundary_offset - blen + i]);
				}
			}
		}
		zassert_equal(errors, 0,
			"Burst-length offset %u words failed: %d errors",
			blen, errors);
		LOG_INF("Burst-length offset %u words PASSED", blen);
	}

	LOG_INF("Burst-length offset test PASSED");
}

/* -------- Test: linear burst access -------- */
/* APS512XXN supports linear burst mode for sequential access.
 * This test performs a long sequential write/read that exercises
 * linear burst across multiple row boundaries.
 */
ZTEST(psram_tests, test_linear_burst_access)
{
	volatile uint32_t *ptr = (volatile uint32_t *)PSRAM_XIP_BASE_ADDR;
	/* Test 32KB region - large enough to cross multiple row boundaries.
	 * APS512XXN has 1KB column address space (CA0-CA9), so rows are 2KB
	 * in x16 mode. 32KB = 16 row crossings.
	 */
	uint32_t test_size = 32768;
	uint32_t num_words;
	int errors = 0;

	if (PSRAM_TEST_SIZE < test_size) {
		ztest_test_skip();
	}

	num_words = test_size / sizeof(uint32_t);

	LOG_INF("=== Linear Burst Access Test (%u bytes, crossing row boundaries) ===",
		test_size);

	/* Sequential linear write */
	for (uint32_t i = 0; i < num_words; i++) {
		ptr[i] = 0x1B000000 | i;
	}

	/* Sequential linear read and verify */
	for (uint32_t i = 0; i < num_words; i++) {
		uint32_t expected = 0x1B000000 | i;

		if (ptr[i] != expected) {
			errors++;
			if (errors <= 5) {
				LOG_INF("Linear burst err at word %u (byte offset 0x%X): "
					"exp 0x%08X got 0x%08X",
					i, i * 4, expected, ptr[i]);
			}
		}
	}

	zassert_equal(errors, 0, "Linear burst access failed: %d errors", errors);
	LOG_INF("Linear burst access test PASSED");
}

ZTEST(psram_tests, test_max_throughput_validation)
{
	uint8_t *xip_ptr;
	uint8_t buf[4096];
	int64_t elapsed_ms;
	int iterations = 200;
	uint32_t total_bytes = iterations * 4096;
	uint32_t theo_kib_s;

	if (PSRAM_TEST_SIZE < 8U * 4096U) {
		ztest_test_skip();
	}

	xip_ptr = get_xip_ptr(0);

	LOG_INF("=== Throughput (informational, no pass threshold) ===");

	for (int i = 0; i < 4096; i++) {
		buf[i] = i & 0xFF;
	}

	elapsed_ms = k_uptime_get();
	for (int i = 0; i < iterations; i++) {
		memcpy(xip_ptr + (i % 8) * 4096, buf, 4096);
	}
	elapsed_ms = k_uptime_get() - elapsed_ms;
	log_throughput_kibs("Write", elapsed_ms, total_bytes);

	elapsed_ms = k_uptime_get();
	for (int i = 0; i < iterations; i++) {
		memcpy(buf, xip_ptr + (i % 8) * 4096, 4096);
	}
	elapsed_ms = k_uptime_get() - elapsed_ms;
	log_throughput_kibs("Read", elapsed_ms, total_bytes);

	/* DDR x16: bus_speed * 2 (DDR) * 2 bytes = bus_speed * 4 */
	theo_kib_s = (uint32_t)(((uint64_t)PSRAM_BUS_SPEED * 4U) / 1024U);
	LOG_INF("Configured bus speed: %u Hz", PSRAM_BUS_SPEED);
	LOG_INF("Theoretical max (DDR x16): %u KiB/s", theo_kib_s);
}

/* -------- Test: RWDS interleaved read/write -------- */
/* RWDS (Read-Write Data Strobe) is output during reads as data strobe
 * and input during writes as data mask. This test rapidly interleaves
 * read and write operations to exercise RWDS signal toggling.
 */
ZTEST(psram_tests, test_rwds_interleaved_rw)
{
	volatile uint32_t *ptr = (volatile uint32_t *)PSRAM_XIP_BASE_ADDR;
	uint32_t num_words = 256;

	LOG_INF("=== RWDS Interleaved R/W Test ===");

	/* Phase 1: Write-Read-Write-Read interleaved per word */
	for (uint32_t i = 0; i < num_words; i++) {
		ptr[i] = 0x2C000000 | i;
		volatile uint32_t readback = ptr[i];

		zassert_equal(readback, 0x2C000000 | i,
			"RWDS interleaved phase 1 failed at word %u", i);
	}

	/* Phase 2: Write all, then read-write-read pattern */
	for (uint32_t i = 0; i < num_words; i++) {
		ptr[i] = 0xAA000000 | i;
	}
	for (uint32_t i = 0; i < num_words; i++) {
		/* Read original */
		volatile uint32_t val1 = ptr[i];
		/* Overwrite */
		ptr[i] = 0xBB000000 | i;
		/* Read new value */
		volatile uint32_t val2 = ptr[i];

		zassert_equal(val1, 0xAA000000 | i,
			"RWDS phase 2 first read failed at word %u", i);
		zassert_equal(val2, 0xBB000000 | i,
			"RWDS phase 2 second read failed at word %u", i);
	}

	LOG_INF("RWDS interleaved R/W test PASSED");
}

/* Alternating patterns at the DT/init drive strength (no runtime API). */
ZTEST(psram_tests, test_alternating_pattern_integrity)
{
	volatile uint32_t *ptr = (volatile uint32_t *)PSRAM_XIP_BASE_ADDR;
	uint32_t num_words = 1024;
	int errors = 0;

	LOG_INF("=== Alternating pattern integrity ===");

	/* Worst-case signal integrity patterns: max simultaneous
	 * switching on all 16 data lines
	 */
	uint32_t si_patterns[] = {
		0xFFFF0000, 0x0000FFFF, /* All lines switch simultaneously */
		0xAAAAAAAA, 0x55555555, /* Alternating - crosstalk stress */
		0xFF00FF00, 0x00FF00FF, /* Byte-lane switching */
		0xF0F0F0F0, 0x0F0F0F0F, /* Nibble-lane switching */
	};

	for (int p = 0; p < ARRAY_SIZE(si_patterns); p += 2) {
		/* Write alternating pattern pairs across region */
		for (uint32_t i = 0; i < num_words; i++) {
			ptr[i] = si_patterns[p + (i & 1)];
		}

		/* Verify */
		for (uint32_t i = 0; i < num_words; i++) {
			uint32_t expected = si_patterns[p + (i & 1)];

			if (ptr[i] != expected) {
				errors++;
				if (errors <= 3) {
					LOG_INF("SI pattern 0x%08X/0x%08X err at %u",
						si_patterns[p], si_patterns[p + 1], i);
				}
			}
		}
	}

	zassert_equal(errors, 0,
		      "Alternating pattern integrity failed: %d errors", errors);
	LOG_INF("Alternating pattern integrity PASSED");
}

/* -------- Test: data retention (refresh validation) -------- */
/* APS512XXN requires periodic refresh of its memory array.
 * This test writes data, waits for a delay (allowing refresh
 * cycles to occur), then verifies data is still intact.
 */
ZTEST(psram_tests, test_data_retention)
{
	volatile uint32_t *ptr = (volatile uint32_t *)PSRAM_XIP_BASE_ADDR;
	uint32_t num_words = 1024;
	uint32_t delay_ms_values[] = {100, 500, 1000};
	int errors;

	LOG_INF("=== Data Retention (Refresh) Test ===");

	for (int d = 0; d < ARRAY_SIZE(delay_ms_values); d++) {
		/* Write known pattern */
		for (uint32_t i = 0; i < num_words; i++) {
			ptr[i] = 0xDA7A0000 | i;
		}

		/* Wait for refresh cycles to occur */
		LOG_INF("Waiting %u ms for refresh...", delay_ms_values[d]);
		k_msleep(delay_ms_values[d]);

		/* Verify data retained */
		errors = 0;
		for (uint32_t i = 0; i < num_words; i++) {
			uint32_t expected = 0xDA7A0000 | i;

			if (ptr[i] != expected) {
				errors++;
				if (errors <= 3) {
					LOG_INF("Retention err after %u ms at word %u: "
						"exp 0x%08X got 0x%08X",
						delay_ms_values[d], i,
						expected, ptr[i]);
				}
			}
		}
		zassert_equal(errors, 0,
			"Data retention failed after %u ms: %d errors",
			delay_ms_values[d], errors);
		LOG_INF("Data retention after %u ms PASSED", delay_ms_values[d]);
	}

	LOG_INF("Data retention test PASSED");
}

/* Touch 1/8, 1/4, 1/2, and full-array offsets. Does not program PASR
 * (driver leaves PASR at 0; there is no public reconfig API).
 */
ZTEST(psram_tests, test_region_fraction_access)
{
	uint32_t array_fractions[] = {8, 4, 2, 1};
	uint32_t test_pattern = 0x3A000000;
	int total_errors = 0;

	LOG_INF("=== Region fraction access ===");
	LOG_INF("Total PSRAM size: %u bytes", PSRAM_TEST_SIZE);

	for (int f = 0; f < ARRAY_SIZE(array_fractions); f++) {
		uint32_t region_size = PSRAM_TEST_SIZE / array_fractions[f];
		uint32_t region_start = (array_fractions[f] == 1) ? 0 :
					PSRAM_TEST_SIZE - region_size;
		volatile uint32_t *ptr = (volatile uint32_t *)
					 (PSRAM_XIP_BASE_ADDR + region_start);
		uint32_t test_words = 64; /* Test 256 bytes at start of each region */
		int errors = 0;

		LOG_INF("Testing 1/%u array region at offset 0x%08X (%u bytes)",
			array_fractions[f], region_start, region_size);

		/* Write pattern to region start */
		for (uint32_t i = 0; i < test_words; i++) {
			ptr[i] = test_pattern | (f << 16) | i;
		}

		/* Verify */
		for (uint32_t i = 0; i < test_words; i++) {
			uint32_t expected = test_pattern | (f << 16) | i;

			if (ptr[i] != expected) {
				errors++;
				if (errors <= 3) {
					LOG_INF("Array 1/%u err at word %u: "
						"exp 0x%08X got 0x%08X",
						array_fractions[f], i,
						expected, ptr[i]);
				}
			}
		}
		total_errors += errors;
		LOG_INF("1/%u array region: %s",
			array_fractions[f], errors ? "FAILED" : "PASSED");
	}

	zassert_equal(total_errors, 0,
		      "Region fraction access failed: %d total errors",
		      total_errors);
	LOG_INF("Region fraction access PASSED");
}

/* -------- Test setup/teardown -------- */
static void *psram_test_suite_setup(void)
{
	LOG_INF("=============================================");
	LOG_INF("PSRAM XIP Test Suite Started");
	LOG_INF("=============================================");
	return NULL;
}

static void psram_test_suite_teardown(void *fixture)
{
	ARG_UNUSED(fixture);
	LOG_INF("=============================================");
	LOG_INF("PSRAM XIP Test Suite Completed");
	LOG_INF("=============================================");
}

/* -------- Test before/after -------- */
static void psram_test_before(void *fixture)
{
	ARG_UNUSED(fixture);
	zassert_true(device_is_ready(psram_dev),
		     "PSRAM device not ready: %s", psram_dev->name);
	LOG_INF("--- Test starting ---");
}

static void psram_test_after(void *fixture)
{
	ARG_UNUSED(fixture);
	LOG_INF("--- Test completed ---");
}

ZTEST_SUITE(psram_tests, NULL, psram_test_suite_setup,
	    psram_test_before, psram_test_after, psram_test_suite_teardown);

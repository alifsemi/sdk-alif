/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 * Stress cases for the xip_tests suite. Built only when
 * CONFIG_TEST_XIP_STRESS=y. A full-chip image does not register these.
 */

#include <stdbool.h>
#include <string.h>
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

#if IS_ENABLED(CONFIG_TEST_XIP_PERFORMANCE)
LOG_MODULE_DECLARE(xip);
#else
LOG_MODULE_REGISTER(xip);
#endif

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
#define PSRAM_TEST_CHUNK_SIZE        1024
#define PSRAM_REGION2_OFF            32768

#if IS_ENABLED(CONFIG_TEST_ISSI_HYPERRAM)
#define SKIP_ISSI_BYTE_STORE()                                                         \
	do {                                                                           \
		LOG_INF("SKIP: ISSI HyperRAM XiP write is 16-bit; 8-bit store is not used"); \
		ztest_test_skip();                                                     \
	} while (0)
#else
#define SKIP_ISSI_BYTE_STORE() do { } while (0)
#endif

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
/* -------- Test: memory stress (alternating patterns) -------- */
ZTEST(xip_tests, test_memory_stress_test)
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
ZTEST(xip_tests, test_concurrent_region_test)
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
ZTEST(xip_tests, test_large_block_transfer_test)
{
	SKIP_ISSI_BYTE_STORE();

	uint8_t *src_buf;
	uint8_t *verify_buf;
	uint8_t *xip_ptr;
	int e_count;

	if (PSRAM_TEST_SIZE < 16384) {
		ztest_test_skip();
	}

	src_buf = k_malloc(16384);
	verify_buf = k_malloc(16384);
	if (src_buf == NULL || verify_buf == NULL) {
		k_free(src_buf);
		k_free(verify_buf);
		zassert_not_null(src_buf, "Failed to allocate source buffer");
		zassert_not_null(verify_buf, "Failed to allocate verify buffer");
	}

	xip_ptr = get_xip_ptr(0);

	LOG_INF("=== Large Block Transfer Test (16KB) ===");

	/* Fill source with pattern */
	for (int i = 0; i < 16384; i++) {
		src_buf[i] = (i * 7 + 13) % 256;
	}

#if IS_ENABLED(CONFIG_TEST_HYPERRAM)
	/* S80KS tCSM: 64-word chunks, then idle so CS# can rise. */
	for (size_t off = 0; off < 16384U; off += 64U * sizeof(uint32_t)) {
		memcpy(xip_ptr + off, src_buf + off, 64U * sizeof(uint32_t));
		k_busy_wait(5);
	}
	for (size_t off = 0; off < 16384U; off += 64U * sizeof(uint32_t)) {
		memcpy(verify_buf + off, xip_ptr + off, 64U * sizeof(uint32_t));
		k_busy_wait(5);
	}
#else
	/* Write to PSRAM */
	memcpy(xip_ptr, src_buf, 16384);

	/* Read back */
	memcpy(verify_buf, xip_ptr, 16384);
#endif

	/* Verify */
	e_count = verify_data(src_buf, verify_buf, 16384, "large_block");
	zassert_equal(e_count, 0, "Large block transfer failed: %d errors", e_count);

	k_free(src_buf);
	k_free(verify_buf);

	LOG_INF("Large block transfer test PASSED");
}

/* -------- Test: DDR data integrity -------- */
/* DDR transfers data on both clock edges. This test uses patterns
 * that stress the DDR transition boundaries (0x5555/0xAAAA alternating)
 * to detect any setup/hold timing issues.
 */
ZTEST(xip_tests, test_ddr_data_integrity)
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

/* Alternating patterns at the DT/init drive strength (no runtime API). */
ZTEST(xip_tests, test_alternating_pattern_integrity)
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
ZTEST(xip_tests, test_data_retention)
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
ZTEST(xip_tests, test_region_fraction_access)
{
	uint32_t array_fractions[] = {8, 4, 2, 1};
	uint32_t test_pattern = 0x3A000000;
	int total_errors = 0;

	LOG_INF("=== Region fraction access ===");
	LOG_INF("Total %s size: %u bytes", XIP_DEV_NAME, PSRAM_TEST_SIZE);

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

#endif /* !FULL_CHIP */

#if !IS_ENABLED(CONFIG_TEST_XIP_PERFORMANCE)
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
#endif

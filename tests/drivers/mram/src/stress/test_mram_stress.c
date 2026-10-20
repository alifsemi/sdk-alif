/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/* MRAM Flash Driver — Stress Test Suite */

#include <zephyr/kernel.h>
#include <zephyr/ztest.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/devicetree.h>
#include <zephyr/storage/flash_map.h>
#include <string.h>


/* ---------------------------------------------------------------------------
 * Test area definitions
 * -------------------------------------------------------------------------
 */
#if !FIXED_PARTITION_EXISTS(storage_partition)
#error "MRAM stress tests require a storage_partition in the board overlay"
#endif
#define TEST_AREA          storage_partition
#define TEST_AREA_OFFSET   FIXED_PARTITION_OFFSET(TEST_AREA)
#define TEST_AREA_SIZE     FIXED_PARTITION_SIZE(TEST_AREA)
#define TEST_AREA_DEVICE   FIXED_PARTITION_DEVICE(TEST_AREA)

/* Stress test tunables */
#define STRESS_CYCLE_COUNT         100
#define STRESS_BLOCK_SIZE          1024U
#define STRESS_BOUNDARY_ITERATIONS 50
#define STRESS_RANDOM_ITERATIONS   200

static const struct device *const flash_dev = TEST_AREA_DEVICE;
static struct flash_pages_info page_info;
static const struct flash_parameters *flash_params;
static size_t write_block_size;
static uint8_t erase_value;

/* Global buffers */

static uint8_t write_buf[STRESS_BLOCK_SIZE];
static uint8_t read_buf[STRESS_BLOCK_SIZE];

/* Simple pseudo-random number generator (xorshift32) for deterministic tests */
static uint32_t prng_state = 0xDEADBEEF;

static uint32_t prng_next(void)
{
	prng_state ^= prng_state << 13;
	prng_state ^= prng_state >> 17;
	prng_state ^= prng_state << 5;
	return prng_state;
}

static size_t erase_size_for(size_t n)
{
	size_t pages;

	zassert_true(page_info.size > 0, "page_info.size is 0");
	pages = (n + page_info.size - 1U) / page_info.size;
	return pages * page_info.size;
}

/* setup */
static void *stress_test_setup(void)
{
	int rc;

	zassert_true(device_is_ready(flash_dev),
		     "MRAM flash device is not ready");

	flash_params = flash_get_parameters(flash_dev);
	zassert_not_null(flash_params, "flash_get_parameters returned NULL");
	erase_value = flash_params->erase_value;
	write_block_size = flash_params->write_block_size;
	zassert_true(write_block_size > 0, "write_block_size is 0");
	zassert_true((STRESS_BLOCK_SIZE % write_block_size) == 0,
		     "STRESS_BLOCK_SIZE %zu not a multiple of write_block_size %zu",
		     (size_t)STRESS_BLOCK_SIZE, write_block_size);
	zassert_true(TEST_AREA_SIZE >= STRESS_BLOCK_SIZE,
		     "storage_partition too small: %u < %u",
		     (uint32_t)TEST_AREA_SIZE, STRESS_BLOCK_SIZE);

	rc = flash_get_page_info_by_offs(flash_dev, TEST_AREA_OFFSET,
					 &page_info);
	zassert_equal(rc, 0, "flash_get_page_info_by_offs failed: %d", rc);

	return NULL;
}

/* Per-test: reset PRNG and erase one block */
static void stress_test_before(void *fixture)
{
	ARG_UNUSED(fixture);

	prng_state = 0xDEADBEEF;

	size_t erase_len = erase_size_for(STRESS_BLOCK_SIZE);

	zassert_true(erase_len <= TEST_AREA_SIZE,
		     "Rounded erase %zu exceeds partition %u",
		     erase_len, (uint32_t)TEST_AREA_SIZE);

	int rc = flash_erase(flash_dev, TEST_AREA_OFFSET, erase_len);

	zassert_equal(rc, 0, "Per-test erase failed: %d", rc);
}

/* --------------------------------------------------------------------------
 * Validates that the same MRAM region can withstand STRESS_CYCLE_COUNT
 * consecutive erase → write → read → verify cycles without data corruption.
 * -------------------------------------------------------------------------
 */
ZTEST(mram_stress, test_repeated_erase_write_read_cycles)
{
	int rc;
	const size_t len = STRESS_BLOCK_SIZE;

	printk("Stress: %d erase-write-read cycles on %zu bytes\n",
	       STRESS_CYCLE_COUNT, len);

	for (int cycle = 0; cycle < STRESS_CYCLE_COUNT; cycle++) {
		size_t erase_len = erase_size_for(len);

		zassert_true(erase_len <= TEST_AREA_SIZE,
			     "Cycle %d: erase %zu exceeds partition",
			     cycle, erase_len);
		rc = flash_erase(flash_dev, TEST_AREA_OFFSET, erase_len);
		zassert_equal(rc, 0,
			      "Erase failed at cycle %d: %d", cycle, rc);

		/* Verify erased */
		rc = flash_read(flash_dev, TEST_AREA_OFFSET, read_buf, len);
		zassert_equal(rc, 0,
			      "Read after erase failed at cycle %d", cycle);

		memset(write_buf, erase_value, len);
		zassert_mem_equal(read_buf, write_buf, len,
				  "Cycle %d: region not erased", cycle);

		/* Write with cycle-dependent pattern */
		uint8_t pattern = (uint8_t)((cycle * 7U + 0x11U) & 0xFFU);

		memset(write_buf, pattern, len);

		rc = flash_write(flash_dev, TEST_AREA_OFFSET, write_buf, len);
		zassert_equal(rc, 0,
			      "Write failed at cycle %d: %d", cycle, rc);

		/* Read back and verify */
		memset(read_buf, 0, len);
		rc = flash_read(flash_dev, TEST_AREA_OFFSET, read_buf, len);
		zassert_equal(rc, 0,
			      "Read failed at cycle %d: %d", cycle, rc);

		zassert_mem_equal(read_buf, write_buf, len,
				  "Data mismatch at cycle %d (pattern 0x%02x)",
				  cycle, pattern);

		if ((cycle + 1) % 25 == 0) {
			printk("  Cycle %d/%d passed\n",
			       cycle + 1, STRESS_CYCLE_COUNT);
		}
	}

	printk("Stress: All %d cycles passed\n", STRESS_CYCLE_COUNT);
}

/* ---------------------------------------------------------------------------
 * MRAM does not require erase before write (unlike NOR flash). This test
 * writes different patterns to the same region back-to-back and verifies
 * that each overwrite takes effect correctly.
 * -------------------------------------------------------------------------
 */
ZTEST(mram_stress, test_rapid_sequential_writes)
{
	int rc;
	const size_t len = STRESS_BLOCK_SIZE;
	const int iterations = STRESS_CYCLE_COUNT;

	printk("Stress: %d rapid overwrites on %zu bytes (no erase)\n",
	       iterations, len);

	for (int i = 0; i < iterations; i++) {
		uint8_t pattern = (uint8_t)((i * 13 + 0x37) & 0xFF);

		memset(write_buf, pattern, len);

		rc = flash_write(flash_dev, TEST_AREA_OFFSET, write_buf, len);
		zassert_equal(rc, 0,
			      "Overwrite %d failed: %d", i, rc);

		memset(read_buf, 0, len);
		rc = flash_read(flash_dev, TEST_AREA_OFFSET, read_buf, len);
		zassert_equal(rc, 0,
			      "Read after overwrite %d failed: %d", i, rc);

		zassert_mem_equal(read_buf, write_buf, len,
				  "Mismatch at overwrite %d (pattern 0x%02x)",
				  i, pattern);
	}

	printk("Stress: All %d rapid overwrites passed\n", iterations);
}

/* ---------------------------------------------------------------------------
 * Writes alternating 0xAA / 0x55 patterns across multiple blocks of the
 * storage partition, then reads the entire region and verifies every byte.
 * This catches bit-stuck or coupling faults.
 * -------------------------------------------------------------------------
 */
ZTEST(mram_stress, test_alternating_pattern_stress)
{
	int rc;
	const size_t block_sz = STRESS_BLOCK_SIZE;
	/* Use up to 8 blocks or whatever fits in the partition */
	uint32_t num_blocks = TEST_AREA_SIZE / block_sz;

	zassert_true(num_blocks > 0,
				"Partition %u too small for block size %zu",
				(uint32_t)TEST_AREA_SIZE, block_sz);

	if (num_blocks > 8) {
		num_blocks = 8;
	}

	size_t total = (size_t)num_blocks * block_sz;
	size_t erase_len = erase_size_for(total);

	printk("Stress: alternating patterns over %u blocks (%zu bytes)\n",
	       num_blocks, total);

	zassert_true(erase_len <= TEST_AREA_SIZE,
		     "Rounded erase %zu exceeds partition %u",
		     erase_len, (uint32_t)TEST_AREA_SIZE);
	rc = flash_erase(flash_dev, TEST_AREA_OFFSET, erase_len);
	zassert_equal(rc, 0, "Erase for alternating test failed: %d", rc);

	/* Pass 1: write 0xAA to all blocks */
	memset(write_buf, 0xAA, block_sz);
	for (uint32_t b = 0; b < num_blocks; b++) {
		rc = flash_write(flash_dev,
				 TEST_AREA_OFFSET + b * block_sz,
				 write_buf, block_sz);
		zassert_equal(rc, 0,
			      "Write 0xAA block %u failed: %d", b, rc);
	}

	/* Verify 0xAA */
	for (uint32_t b = 0; b < num_blocks; b++) {
		memset(read_buf, 0, block_sz);
		rc = flash_read(flash_dev,
				TEST_AREA_OFFSET + b * block_sz,
				read_buf, block_sz);
		zassert_equal(rc, 0,
			      "Read 0xAA block %u failed: %d", b, rc);

		zassert_mem_equal(read_buf, write_buf, block_sz,
				  "Block %u 0xAA mismatch", b);
	}

	/* Pass 2: overwrite with 0x55 (no erase — MRAM supports this) */
	memset(write_buf, 0x55, block_sz);
	for (uint32_t b = 0; b < num_blocks; b++) {
		rc = flash_write(flash_dev,
				 TEST_AREA_OFFSET + b * block_sz,
				 write_buf, block_sz);
		zassert_equal(rc, 0,
			      "Write 0x55 block %u failed: %d", b, rc);
	}

	/* Verify 0x55 */
	for (uint32_t b = 0; b < num_blocks; b++) {
		memset(read_buf, 0, block_sz);
		rc = flash_read(flash_dev,
				TEST_AREA_OFFSET + b * block_sz,
				read_buf, block_sz);
		zassert_equal(rc, 0,
			      "Read 0x55 block %u failed: %d", b, rc);

		zassert_mem_equal(read_buf, write_buf, block_sz,
				  "Block %u 0x55 mismatch", b);
	}

	printk("Stress: alternating pattern test passed\n");
}

/* ---------------------------------------------------------------------------
 * The MRAM sector is 16 bytes. This test repeatedly writes across 16-byte
 * boundaries with offsets 1..15 to stress the read-modify-write path in
 * the driver. Verifies both the written data and that neighboring bytes
 * outside the write region remain intact.
 * -------------------------------------------------------------------------
 */
ZTEST(mram_stress, test_boundary_sector_stress)
{
	int rc;
	const size_t region = 64; /* work within 64 bytes */
	static uint8_t ref_buf[64];
	static uint8_t verify_buf[64];

	printk("Stress: %d boundary-crossing iterations\n",
	       STRESS_BOUNDARY_ITERATIONS);

	for (int iter = 0; iter < STRESS_BOUNDARY_ITERATIONS; iter++) {
		/* Erase and fill with a known background */
		rc = flash_erase(flash_dev, TEST_AREA_OFFSET,
				 erase_size_for(page_info.size));
		zassert_equal(rc, 0,
			      "Erase at iter %d failed: %d", iter, rc);

		uint8_t bg = (uint8_t)(0xB0 + iter);

		memset(ref_buf, bg, region);
		rc = flash_write(flash_dev, TEST_AREA_OFFSET, ref_buf, region);
		zassert_equal(rc, 0,
			      "Background write at iter %d failed: %d",
			      iter, rc);

		/* Write across a 16-byte boundary with unaligned offset */
		off_t unaligned_off = (off_t)(1 + (iter % 15)); /* 1..15 */
		size_t write_len = 20; /* crosses at least one 16B boundary */
		uint8_t wr_pattern = (uint8_t)(0xD0 + iter);

		uint8_t cross_data[20];

		memset(cross_data, wr_pattern, write_len);

		rc = flash_write(flash_dev,
				 TEST_AREA_OFFSET + unaligned_off,
				 cross_data, write_len);
		zassert_equal(rc, 0,
			      "Boundary write at iter %d off %ld failed: %d",
			      iter, (long)unaligned_off, rc);

		/* Update reference buffer to match expected state */
		memcpy(ref_buf + unaligned_off, cross_data, write_len);

		/* Read back full region and verify */
		memset(verify_buf, 0, region);
		rc = flash_read(flash_dev, TEST_AREA_OFFSET,
				verify_buf, region);
		zassert_equal(rc, 0,
			      "Verify read at iter %d failed: %d", iter, rc);

		zassert_mem_equal(verify_buf, ref_buf, region,
				  "Iter %d mismatch (off=%ld)",
				  iter, (long)unaligned_off);
	}

	printk("Stress: all %d boundary iterations passed\n",
	       STRESS_BOUNDARY_ITERATIONS);
}

/* ---------------------------------------------------------------------------
 * Fills the entire storage partition with incremental data one block at a
 * time, then reads the whole partition back and verifies every byte.
 * -------------------------------------------------------------------------
 */
ZTEST(mram_stress, test_full_partition_fill_and_verify)
{
	int rc;
	const size_t block_sz = STRESS_BLOCK_SIZE;
	uint32_t num_blocks = TEST_AREA_SIZE / block_sz;

	zassert_true(num_blocks > 0,
		     "Partition %u smaller than block %zu",
		     (uint32_t)TEST_AREA_SIZE, block_sz);

	size_t total = (size_t)num_blocks * block_sz;
	size_t erase_len = erase_size_for(total);

	printk("Stress: fill & verify entire partition: %u blocks, "
	       "%zu bytes\n", num_blocks, total);

	zassert_true(erase_len <= TEST_AREA_SIZE,
		     "Rounded erase %zu exceeds partition %u",
		     erase_len, (uint32_t)TEST_AREA_SIZE);
	rc = flash_erase(flash_dev, TEST_AREA_OFFSET, erase_len);
	zassert_equal(rc, 0, "Full partition erase failed: %d", rc);

	/* Write block by block with incremental pattern */
	for (uint32_t b = 0; b < num_blocks; b++) {
		for (size_t i = 0; i < block_sz; i++) {
			write_buf[i] = (uint8_t)((b + i + 1) & 0xFF);
		}

		rc = flash_write(flash_dev,
				 TEST_AREA_OFFSET + b * block_sz,
				 write_buf, block_sz);
		zassert_equal(rc, 0,
			      "Write block %u failed: %d", b, rc);

		if ((b + 1) % 4 == 0) {
			printk("  Written %u/%u blocks\n", b + 1, num_blocks);
		}
	}

	/* Read back and verify block by block */
	for (uint32_t b = 0; b < num_blocks; b++) {
		/* Regenerate expected pattern */
		for (size_t i = 0; i < block_sz; i++) {
			write_buf[i] = (uint8_t)((b + i + 1) & 0xFF);
		}

		memset(read_buf, 0, block_sz);
		rc = flash_read(flash_dev,
				TEST_AREA_OFFSET + b * block_sz,
				read_buf, block_sz);
		zassert_equal(rc, 0,
			      "Read block %u failed: %d", b, rc);

		zassert_mem_equal(read_buf, write_buf, block_sz,
				  "Full partition verify: block %u mismatch",
				  b);
	}

	printk("Stress: full partition fill & verify passed\n");
}

/* ---------------------------------------------------------------------------
 * Uses a deterministic PRNG to generate random (offset, length) pairs
 * within the storage partition, writes patterned data, and verifies the
 * whole 4 KB window against a gold image. This catches RMW corruption of
 * neighboring bytes that a last-span-only check would miss.
 * -------------------------------------------------------------------------
 */
ZTEST(mram_stress, test_random_offset_write_read)
{
	int rc;
	const size_t region = 4096;
	const size_t max_len = 128;
	static uint8_t gold[4096];
	static uint8_t wr[128];
	static uint8_t rd[4096];

	printk("Stress: %d random offset write-read iterations\n",
	       STRESS_RANDOM_ITERATIONS);

	zassert_true(region <= TEST_AREA_SIZE,
		     "Random-test region %zu exceeds partition %u",
		     region, (uint32_t)TEST_AREA_SIZE);

	rc = flash_erase(flash_dev, TEST_AREA_OFFSET, erase_size_for(region));
	zassert_equal(rc, 0, "Erase for random test failed: %d", rc);

	memset(gold, erase_value, region);

	for (int i = 0; i < STRESS_RANDOM_ITERATIONS; i++) {
		uint32_t rand_off = prng_next() % (region - max_len);
		uint32_t rand_len = 1 + (prng_next() % max_len);

		if (rand_off + rand_len > region) {
			rand_len = region - rand_off;
		}

		uint8_t pat = (uint8_t)(prng_next() & 0xFF);

		for (uint32_t j = 0; j < rand_len; j++) {
			wr[j] = (uint8_t)(pat + j);
		}

		rc = flash_write(flash_dev,
				 TEST_AREA_OFFSET + rand_off,
				 wr, rand_len);
		zassert_equal(rc, 0,
			      "Random write iter %d off=%u len=%u failed: %d",
			      i, rand_off, rand_len, rc);

		memcpy(gold + rand_off, wr, rand_len);

		memset(rd, 0, region);
		rc = flash_read(flash_dev, TEST_AREA_OFFSET, rd, region);
		zassert_equal(rc, 0,
			      "Random region read iter %d failed: %d", i, rc);

		zassert_mem_equal(rd, gold, region,
				  "Random iter %d neighbor mismatch: off=%u len=%u pat=0x%02x",
				  i, rand_off, rand_len, pat);

		if ((i + 1) % 50 == 0) {
			printk("  Iteration %d/%d passed\n",
			       i + 1, STRESS_RANDOM_ITERATIONS);
		}
	}

	printk("Stress: all %d random iterations passed\n",
	       STRESS_RANDOM_ITERATIONS);
}

ZTEST_SUITE(mram_stress, NULL, stress_test_setup,
	    stress_test_before, NULL, NULL);

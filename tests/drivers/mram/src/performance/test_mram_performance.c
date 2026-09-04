/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/* MRAM Flash Driver — Performance  Test Suite*/

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
#define TEST_AREA          storage_partition
#define TEST_AREA_OFFSET   FIXED_PARTITION_OFFSET(TEST_AREA)
#define TEST_AREA_SIZE     FIXED_PARTITION_SIZE(TEST_AREA)
#define TEST_AREA_DEVICE   FIXED_PARTITION_DEVICE(TEST_AREA)

/* Performance test parameters */
#define PERF_BLOCK_SIZE    1024
#define PERF_ITERATIONS    5
#define PERF_NUM_BLOCKS    (TEST_AREA_SIZE / PERF_BLOCK_SIZE)

static const struct device *const flash_dev = TEST_AREA_DEVICE;

static uint8_t w_buf[PERF_BLOCK_SIZE];
static uint8_t r_buf[PERF_BLOCK_SIZE];

/* KB/s = (bytes * 1000000) / (us * 1024) */
static uint32_t throughput_kbps(uint32_t bytes, uint64_t us)
{
	if (us == 0U) {
		return 0;
	}

	return (uint32_t)((bytes * 1000000ULL) / (us * 1024ULL));
}

static void *perf_test_setup(void)
{
	zassert_true(device_is_ready(flash_dev),
		     "MRAM flash device is not ready");

	zassert_true(PERF_NUM_BLOCKS > 0,
		     "storage_partition smaller than PERF_BLOCK_SIZE");

	return NULL;
}

/* ---------------------------------------------------------------------------
 * Measures erase, write, and read throughput over the full storage partition,
 * repeated PERF_ITERATIONS times. Results are averaged and printed in KB/s.
 * -------------------------------------------------------------------------
 */
ZTEST(mram_performance, test_mram_throughput)
{
	int rc;
	uint32_t start_cyc, end_cyc;
	uint64_t tot_erase_us = 0;
	uint64_t tot_write_us = 0;
	uint64_t tot_read_us = 0;
	uint32_t num_blocks = PERF_NUM_BLOCKS;
	uint32_t total_bytes = num_blocks * PERF_BLOCK_SIZE;

	memset(w_buf, 0xAB, PERF_BLOCK_SIZE);

	printk("\n=== MRAM Performance Test ===\n");
	printk("Block size: %d bytes, Blocks: %u, Total: %u bytes\n",
	       PERF_BLOCK_SIZE, num_blocks, total_bytes);
	printk("Iterations: %d\n\n", PERF_ITERATIONS);

	for (int iter = 1; iter <= PERF_ITERATIONS; iter++) {
		uint64_t e_us, w_us, r_us;

		/* --- Erase --- */
		start_cyc = k_cycle_get_32();
		rc = flash_erase(flash_dev, TEST_AREA_OFFSET, total_bytes);
		end_cyc = k_cycle_get_32();
		zassert_equal(rc, 0, "Erase failed in iteration %d: %d",
			      iter, rc);
		e_us = k_cyc_to_us_floor64(end_cyc - start_cyc);

		/* --- Write --- */
		w_us = 0;
		for (uint32_t b = 0; b < num_blocks; b++) {
			start_cyc = k_cycle_get_32();
			rc = flash_write(flash_dev,
					 TEST_AREA_OFFSET +
					 (b * PERF_BLOCK_SIZE),
					 w_buf, PERF_BLOCK_SIZE);
			end_cyc = k_cycle_get_32();
			zassert_equal(rc, 0,
				      "Write block %u failed in iteration %d",
				      b, iter);
			w_us += k_cyc_to_us_floor64(end_cyc - start_cyc);
		}

		/* --- Read + Verify --- */
		r_us = 0;
		for (uint32_t b = 0; b < num_blocks; b++) {
			memset(r_buf, 0, PERF_BLOCK_SIZE);

			start_cyc = k_cycle_get_32();
			rc = flash_read(flash_dev,
					TEST_AREA_OFFSET +
					(b * PERF_BLOCK_SIZE),
					r_buf, PERF_BLOCK_SIZE);
			end_cyc = k_cycle_get_32();
			zassert_equal(rc, 0,
				      "Read block %u failed in iteration %d",
				      b, iter);
			r_us += k_cyc_to_us_floor64(end_cyc - start_cyc);

			zassert_mem_equal(r_buf, w_buf, PERF_BLOCK_SIZE,
					  "Data mismatch block %u iteration %d",
					  b, iter);
		}

		printk("Iteration %d: erase=%llu us, write=%llu us, "
		       "read=%llu us\n", iter,
		       (unsigned long long)e_us,
		       (unsigned long long)w_us,
		       (unsigned long long)r_us);

		tot_erase_us += e_us;
		tot_write_us += w_us;
		tot_read_us += r_us;
	}

	/* Compute averages */
	uint64_t avg_e = tot_erase_us / PERF_ITERATIONS;
	uint64_t avg_w = tot_write_us / PERF_ITERATIONS;
	uint64_t avg_r = tot_read_us / PERF_ITERATIONS;

	uint32_t e_kbps = throughput_kbps(total_bytes, avg_e);
	uint32_t w_kbps = throughput_kbps(total_bytes, avg_w);
	uint32_t r_kbps = throughput_kbps(total_bytes, avg_r);

	printk("\n--- Average over %d iterations ---\n", PERF_ITERATIONS);
	printk("Erase : %llu us  (%u KB/s)\n",
	       (unsigned long long)avg_e, e_kbps);
	printk("Write : %llu us  (%u KB/s)\n",
	       (unsigned long long)avg_w, w_kbps);
	printk("Read  : %llu us  (%u KB/s)\n",
	       (unsigned long long)avg_r, r_kbps);
	printk("=== Performance Test Complete ===\n");
}

ZTEST_SUITE(mram_performance, NULL, perf_test_setup, NULL, NULL, NULL);

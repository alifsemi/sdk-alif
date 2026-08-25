/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <string.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/logging/log.h>
#include <zephyr/ztest.h>

#include "test_ospi_flash_test.h"

LOG_MODULE_REGISTER(ospi_perf, LOG_LEVEL_INF);

/* -------- Test: performance read/write across all sectors -------- */
static void ospi_itr_all_sector_read_write_test(void)
{
	int ret, i, j, e_count = 0;
	uint32_t e_start, e_end, w_start, w_end, r_start, r_end;
	uint32_t tot_erase_time = 0, tot_read_time = 0, tot_write_time = 0;
	double w_time_ms, r_time_ms, e_time_ms;
	double e_avg_ms, r_avg_ms, w_avg_ms;
	uint16_t w_buf[BUFF_SIZE] = {0};
	uint16_t r_buf[BUFF_SIZE] = {0};
	const size_t len = sizeof(w_buf);
	const size_t transfer_count = ITER - 1;
	const size_t write_read_bytes = transfer_count * len;
	const size_t erase_bytes = SPI_FLASH_SECTOR_SIZE * ITER;

	for (j = 0; j < BUFF_SIZE; j++) {
		w_buf[j] = (512 + j) % 65536;
	}

	for (int l = 1; l <= 10; l++) {
		w_time_ms = 0;
		r_time_ms = 0;

		LOG_INF("Perf Iter %d: Erasing %u bytes at 0x%x",
			l, (uint32_t)erase_bytes,
			SPI_FLASH_TEST_REGION_OFFSET);
		e_start = k_uptime_get();
		ret = flash_erase(flash_dev,
				  SPI_FLASH_TEST_REGION_OFFSET,
				  erase_bytes);
		e_end = k_uptime_get();
		e_time_ms = e_end - e_start;
		LOG_INF("Perf test: Iter %d - Erase took %u ms",
			l, (unsigned int)e_time_ms);
		zassert_equal(ret, 0, "Flash erase failed [%d]", ret);

		for (i = 1; i < ITER; i++) {
			/* Write/read in safe region (sector 9+) */
			uint32_t sect_off;

			sect_off = SPI_FLASH_TEST_REGION_OFFSET +
				   SPI_FLASH_SECTOR_SIZE * i;

			w_start = k_uptime_get();
			ret = flash_write(flash_dev, sect_off,
					  w_buf, len);
			w_end = k_uptime_get();
			w_time_ms += (w_end - w_start);
			zassert_equal(ret, 0,
				      "Flash write failed Sec %d [%d]",
				      i, ret);

			e_count = 0;
			memset(r_buf, 0, sizeof(r_buf));
			r_start = k_uptime_get();
			ret = flash_read(flash_dev, sect_off,
					 r_buf, len);
			r_end = k_uptime_get();
			r_time_ms += (r_end - r_start);
			zassert_equal(ret, 0,
				      "Flash read failed Sec %d [%d]",
				      i, ret);

			for (int k = 0; k < BUFF_SIZE; k++) {
				if (r_buf[k] != w_buf[k]) {
					e_count++;
				}
			}
			zassert_equal(e_count, 0,
				      "Iter %d Sec %d: %d mismatches",
				      l, i, e_count);
		}

		LOG_INF("Iter %d: erase=%f write=%f read=%f ms",
			l, (double)e_time_ms,
			(double)w_time_ms, (double)r_time_ms);

		tot_erase_time += e_time_ms;
		tot_read_time += r_time_ms;
		tot_write_time += w_time_ms;
	}

	e_avg_ms = (double)tot_erase_time / 10.0;
	w_avg_ms = (double)tot_write_time / 10.0;
	r_avg_ms = (double)tot_read_time / 10.0;

	LOG_INF("Avg erase: %f ms", e_avg_ms);
	LOG_INF("Avg write: %f ms", w_avg_ms);
	LOG_INF("Avg read : %f ms", r_avg_ms);

	if (e_avg_ms > 0.0) {
		LOG_INF("Erase throughput: %f KiB/s",
			((double)erase_bytes / 1024.0) / (e_avg_ms / 1000.0));
	}
	if (w_avg_ms > 0.0) {
		LOG_INF("Write throughput: %f KiB/s",
			((double)write_read_bytes / 1024.0) / (w_avg_ms / 1000.0));
	}
	if (r_avg_ms > 0.0) {
		LOG_INF("Read  throughput: %f KiB/s",
			((double)write_read_bytes / 1024.0) / (r_avg_ms / 1000.0));
	}
}

#if defined(CONFIG_TEST_OSPI_FLASH_FULL_CHIP)

#define PERF_FULL_CHUNK     1024
#define PERF_FULL_LOG_BYTES (1024 * 1024)

static void fill_addr_pattern(uint8_t *buf, size_t len, uint32_t offset)
{
	for (size_t i = 0; i < len; i++) {
		buf[i] = (uint8_t)((offset + i) ^ (offset >> 8));
	}
}

static void assert_erased_chunk(uint32_t offset, size_t len, const char *tag)
{
	uint8_t r_buf[PERF_FULL_CHUNK];
	int ret;
	int bad = 0;

	zassert_true(len <= sizeof(r_buf), "%s: chunk too large", tag);

	memset(r_buf, 0, len);
	ret = flash_read(flash_dev, offset, r_buf, len);
	zassert_equal(ret, 0, "%s: erase-check read at 0x%x failed [%d]", tag, offset, ret);

	for (size_t i = 0; i < len; i++) {
		if (r_buf[i] != flash_param->erase_value) {
			bad++;
		}
	}

	zassert_equal(bad, 0, "%s: %d bytes not erased at 0x%x", tag, bad, offset);
}

/*
 * Full-device erase / write / read. On E7 that is 16384 * 4 KB = 64 MB,
 * on E8 it is 128 MB. flash_erase(0, total) uses the driver's chip-erase
 * path. This WIPES offset 0 (including any boot image), so it is compiled
 * only when CONFIG_TEST_OSPI_FLASH_FULL_CHIP is set and must never run on
 * a board that boots from this device.
 */
static void ospi_full_chip_erase_write_read_test(void)
{
	uint8_t w_buf[PERF_FULL_CHUNK];
	uint8_t r_buf[PERF_FULL_CHUNK];
	const size_t sector = flash_param->sector_size;
	const uint32_t total = flash_param->num_of_sector * sector;
	const uint32_t mib = total / (1024U * 1024U);
	int64_t e_ms, w_ms, r_ms, start;
	uint32_t next_log;
	int ret;
	int mismatches = 0;

	zassert_true(sector > 0 && flash_param->num_of_sector > 0,
		     "flash geometry is zero");
	zassert_equal(total % PERF_FULL_CHUNK, 0,
		      "device size %u is not a multiple of the %u-byte chunk",
		      total, PERF_FULL_CHUNK);
	zassert_equal(PERF_FULL_CHUNK % flash_param->write_block_size, 0,
		      "chunk is not write-block aligned");

	LOG_INF("Full-chip test: %u bytes (%u MiB), %zu sectors x %zu B",
		total, mib, flash_param->num_of_sector, sector);
	LOG_WRN("This erases the entire device including offset 0");

	start = k_uptime_get();
	ret = flash_erase(flash_dev, 0, total);
	e_ms = k_uptime_get() - start;
	zassert_equal(ret, 0, "Full-chip erase failed [%d]", ret);
	LOG_INF("Full-chip erase took %lld ms", e_ms);

	assert_erased_chunk(0, PERF_FULL_CHUNK, "chip-start");
	assert_erased_chunk(total / 2, PERF_FULL_CHUNK, "chip-mid");
	assert_erased_chunk(total - PERF_FULL_CHUNK, PERF_FULL_CHUNK, "chip-end");

	start = k_uptime_get();
	next_log = PERF_FULL_LOG_BYTES;
	for (uint32_t off = 0; off < total; off += PERF_FULL_CHUNK) {
		fill_addr_pattern(w_buf, PERF_FULL_CHUNK, off);
		ret = flash_write(flash_dev, off, w_buf, PERF_FULL_CHUNK);
		zassert_equal(ret, 0, "Write failed at 0x%x [%d]", off, ret);

		if (off + PERF_FULL_CHUNK >= next_log || off + PERF_FULL_CHUNK == total) {
			LOG_INF("Full-chip write %u / %u MiB",
				(off + PERF_FULL_CHUNK) / (1024U * 1024U), mib);
			next_log += PERF_FULL_LOG_BYTES;
		}
	}
	w_ms = k_uptime_get() - start;
	LOG_INF("Full-chip write took %lld ms", w_ms);

	start = k_uptime_get();
	next_log = PERF_FULL_LOG_BYTES;
	for (uint32_t off = 0; off < total; off += PERF_FULL_CHUNK) {
		fill_addr_pattern(w_buf, PERF_FULL_CHUNK, off);
		memset(r_buf, 0, sizeof(r_buf));
		ret = flash_read(flash_dev, off, r_buf, PERF_FULL_CHUNK);
		zassert_equal(ret, 0, "Read failed at 0x%x [%d]", off, ret);

		for (size_t i = 0; i < PERF_FULL_CHUNK; i++) {
			if (r_buf[i] != w_buf[i]) {
				mismatches++;
				if (mismatches <= 5) {
					LOG_INF("Mismatch 0x%x: w=0x%02x r=0x%02x",
						off + (uint32_t)i, w_buf[i], r_buf[i]);
				}
			}
		}

		if (off + PERF_FULL_CHUNK >= next_log || off + PERF_FULL_CHUNK == total) {
			LOG_INF("Full-chip read %u / %u MiB",
				(off + PERF_FULL_CHUNK) / (1024U * 1024U), mib);
			next_log += PERF_FULL_LOG_BYTES;
		}
	}
	r_ms = k_uptime_get() - start;
	LOG_INF("Full-chip read took %lld ms", r_ms);

	zassert_equal(mismatches, 0, "Full-chip verify failed: %d mismatches", mismatches);

	if (e_ms > 0) {
		LOG_INF("Full-chip erase throughput: %f KiB/s",
			((double)total / 1024.0) / ((double)e_ms / 1000.0));
	}
	if (w_ms > 0) {
		LOG_INF("Full-chip write throughput: %f KiB/s",
			((double)total / 1024.0) / ((double)w_ms / 1000.0));
	}
	if (r_ms > 0) {
		LOG_INF("Full-chip read  throughput: %f KiB/s",
			((double)total / 1024.0) / ((double)r_ms / 1000.0));
	}

	LOG_INF("Full-chip erase/write/read of %u MiB PASSED", mib);
}

#endif /* CONFIG_TEST_OSPI_FLASH_FULL_CHIP */

/* ======== Performance Test Suite ======== */

ZTEST(test_ospi_perf, test_ospi_perf_read_write)
{
	ospi_itr_all_sector_read_write_test();
}

#if defined(CONFIG_TEST_OSPI_FLASH_FULL_CHIP)
ZTEST(test_ospi_perf, test_ospi_perf_full_chip)
{
	ospi_full_chip_erase_write_read_test();
}
#endif

/* ======== Test Suite Lifecycle Functions ======== */

static void *test_ospi_perf_setup(void)
{
	LOG_INF("=== OSPI Performance Test Suite Setup ===");
	ospi_flash_setup_device();
	return NULL;
}

static void test_ospi_perf_before(void *fixture)
{
	(void)fixture;
	ospi_flash_setup_device();
}

ZTEST_SUITE(test_ospi_perf, NULL, test_ospi_perf_setup,
	    test_ospi_perf_before, NULL, NULL);

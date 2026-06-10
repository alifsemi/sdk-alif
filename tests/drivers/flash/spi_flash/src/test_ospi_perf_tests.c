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
	float w_time_ms, r_time_ms, e_time_ms;
	float e_avg_ms, r_avg_ms, w_avg_ms;
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
			if (e_count) {
				LOG_INF("Iter %d Sec %d: %d mismatches",
					l, i, e_count);
			}
		}

		LOG_INF("Iter %d: erase=%f write=%f read=%f ms",
			l, (double)e_time_ms,
			(double)w_time_ms, (double)r_time_ms);

		tot_erase_time += e_time_ms;
		tot_read_time += r_time_ms;
		tot_write_time += w_time_ms;
	}

	e_avg_ms = (float)tot_erase_time / 10.0f;
	w_avg_ms = (float)tot_write_time / 10.0f;
	r_avg_ms = (float)tot_read_time / 10.0f;

	LOG_INF("Avg erase: %f ms", (double)e_avg_ms);
	LOG_INF("Avg write: %f ms", (double)w_avg_ms);
	LOG_INF("Avg read : %f ms", (double)r_avg_ms);

	if (e_avg_ms > 0) {
		LOG_INF("Erase throughput: %f KiB/s",
			((double)erase_bytes / 1024.0) /
			(e_avg_ms / 1000.0));
	}
	if (w_avg_ms > 0) {
		LOG_INF("Write throughput: %f KiB/s",
			((double)write_read_bytes / 1024.0) /
			(w_avg_ms / 1000.0));
	}
	if (r_avg_ms > 0) {
		LOG_INF("Read  throughput: %f KiB/s",
			((double)write_read_bytes / 1024.0) /
			(r_avg_ms / 1000.0));
	}
}

/* ======== Performance Test Suite ======== */

ZTEST(test_ospi_perf, test_ospi_perf_read_write)
{
	ospi_itr_all_sector_read_write_test();
}

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

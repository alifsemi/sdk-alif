/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https: //alifsemi.com/license
 *
 */


#include <stdio.h>
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/ztest.h>

#include "test_ospi_flash_test.h"

LOG_MODULE_REGISTER(test_ospi_flash_app, LOG_LEVEL_INF);

/* Global variables - shared across test files via ospi_flash_test.h */
const struct flash_parameters *flash_param;
const struct device *flash_dev;

/* ======== Helper: validate flash device and parameters ======== */
void ospi_flash_setup_device(void)
{
	LOG_DBG("%s: Getting device handle", __func__);
	flash_dev = DEVICE_DT_GET(SPI_FLASH_DT_NODE);
	zassert_not_null(flash_dev, "Flash device handle is NULL");
	LOG_DBG("%s: Checking device ready", __func__);
	zassert_true(device_is_ready(flash_dev),
		     "Flash device is not ready: %s", flash_dev->name);
	LOG_DBG("%s: Getting flash parameters", __func__);
	flash_param = flash_get_parameters(flash_dev);
	zassert_not_null(flash_param, "Failed to get flash parameters");
	LOG_DBG("%s: Complete", __func__);
}

/* -------- Helper: verify buffer against expected -------- */
int verify_data(const uint8_t *expected, const uint8_t *actual,
		size_t len, const char *tag)
{
	int e_count = 0;

	for (size_t i = 0; i < len; i++) {
		if (actual[i] != expected[i]) {
			e_count++;
			if (e_count <= 5) {
				LOG_INF("%s mismatch [%zu] w[0x%02x] r[0x%02x]",
					tag, i, expected[i], actual[i]);
			}
		}
	}
	return e_count;
}

/* -------- Test: device status -------- */
static void get_ospi_device_status(void)
{
	zassert_true(device_is_ready(flash_dev),
		     "%s: device not ready.", flash_dev->name);
}

/* -------- Test: configure and setup OSPI flash using DTS -------- */
static void configure_setup_ospi_flash_test(void)
{
	int ret;
	uint32_t total_flash_size;

	LOG_INF("=== Configure & Setup OSPI Flash using DTS ===");

	/* Step 1-2: Verify device is ready */
	LOG_INF("Step 1-2: Verify device is ready");
	zassert_true(device_is_ready(flash_dev),
		     "Device is not ready: %s", flash_dev->name);
	LOG_INF("Device is ready and accessible");

	/* Step 3-4: Get flash parameters */
	LOG_INF("Step 3-4: Get flash parameters");
	flash_param = flash_get_parameters(flash_dev);
	zassert_not_null(flash_param, "Failed to get flash parameters");
	LOG_INF("Flash parameters structure returned");
	LOG_INF("Parameters valid");

	/* Step 5: Log device name */
	LOG_INF("Step 5: Log device name");
	LOG_INF("Flash device: %s", flash_dev->name);
	LOG_INF("Device name logged");

	/* Step 6: Confirm device obtained from device tree alias */
	LOG_INF("Step 6: Device obtained from DT alias");
	LOG_INF("Device handle obtained from DEVICE_DT_GET(SPI_FLASH_DT_NODE)");
	zassert_not_null(flash_dev, "Flash device handle is NULL");

	/* Step 7-8: Calculate and log total flash size */
	LOG_INF("Step 7-8: Calculate total flash size");
	total_flash_size = flash_param->num_of_sector *
			   flash_param->sector_size;
	LOG_INF("Total flash size: %u bytes (%u MB)",
		total_flash_size, total_flash_size / (1024 * 1024));
	LOG_INF("Total size calculated and logged");

	/* Log detailed flash configuration from DTS */
	LOG_INF("=== Flash Configuration from DTS ===");
	LOG_INF("Bus Speed         : %u Hz",
		DT_PROP(DT_PARENT(DT_ALIAS(spi_flash0)), bus_speed));
	LOG_INF("Number of Sectors : %zu", flash_param->num_of_sector);
	LOG_INF("Sector Size       : %zu bytes", flash_param->sector_size);
	LOG_INF("Page Size         : %zu bytes", flash_param->page_size);
	LOG_INF("Write Block Size  : %zu bytes", flash_param->write_block_size);
	LOG_INF("Erase Value       : 0x%02x", flash_param->erase_value);

	/* Step 9-10: Erase test region */
	LOG_INF("Step 9-10: Erase test region");
	ret = flash_erase(flash_dev, SPI_FLASH_TEST_REGION_OFFSET,
			  SPI_FLASH_SECTOR_SIZE);
	zassert_equal(ret, 0, "Test region erase failed [%d]", ret);
	LOG_INF("Test region erased successfully (return code = 0)");

	/* Step 11: Log setup completion */
	LOG_INF("Step 11: Setup completion");
	LOG_INF("=== OSPI Flash Setup Completed Successfully ===");
	LOG_INF("Device configured and ready for testing");

	/* Additional validation: Verify erased region */
	uint8_t verify_buf[256];

	ret = flash_read(flash_dev, SPI_FLASH_TEST_REGION_OFFSET,
			 verify_buf, sizeof(verify_buf));
	zassert_equal(ret, 0, "Verification read failed [%d]", ret);

	int erase_check = 0;

	for (int i = 0; i < sizeof(verify_buf); i++) {
		if (verify_buf[i] != flash_param->erase_value) {
			erase_check++;
		}
	}
	zassert_equal(erase_check, 0,
		      "Erase verify failed: %d bytes not erased",
		      erase_check);
	LOG_INF("Erase verification passed");

	LOG_INF("Configure & Setup OSPI Flash test PASSED");
}

static void single_sector_test(void)
{
	const uint8_t expected[] = { 0x55, 0xaa, 0x66, 0x99 };
	const size_t len = ARRAY_SIZE(expected);
	uint8_t buf[len];
	int ret;
	int i, e_count = 0;
	int64_t start_time, end_time;
	uint32_t start_cycle, end_cycle;

	LOG_INF("Test 1: Flash erase");

	/*
	 * Full flash erase if SPI_FLASH_TEST_REGION_OFFSET = 0 and
	 * SPI_FLASH_SECTOR_SIZE = flash size
	 */
	ret = flash_erase(flash_dev, SPI_FLASH_TEST_REGION_OFFSET,
			  SPI_FLASH_SECTOR_SIZE);
	zassert_equal(ret, 0, "Flash erase failed!");

	LOG_INF("Test 1: Flash write");
	LOG_INF("Attempting to write %zu bytes", len);

	start_time = k_uptime_get();
	start_cycle = k_cycle_get_32();
	ret = flash_write(flash_dev, SPI_FLASH_TEST_REGION_OFFSET,
			  expected, len);
	end_cycle = k_cycle_get_32();
	end_time = k_uptime_get();

	zassert_equal(ret, 0, "Flash write failed!");

	int64_t time_taken_ms = end_time - start_time;
	uint32_t cycle_taken = end_cycle - start_cycle;

	LOG_INF("Write completed in %lld ms (%u cycles)",
		time_taken_ms, cycle_taken);

	LOG_INF("Test 1: Flash read");

	memset(buf, 0, len);
	ret = flash_read(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, buf, len);
	zassert_equal(ret, 0, "Flash Read failed!");

	for (i = 0; i < len; i++) {
		if (buf[i] != expected[i]) {
			e_count++;
			LOG_INF("Not matched at [%d] _w[%4x] _r[%4x]\n", i, expected[i], buf[i]);
		}
	}

	if (e_count) {
		LOG_INF("Error: Data read NOT matches data written\n");
	} else {
		LOG_INF("Data read matches data written. Good!!\n");
	}
}

static void erase_test(const struct device *dev, uint32_t offset, uint32_t len)
{
	int ret = 0, i = 0, count = 0;
	uint8_t r_buf[BUFF_SIZE] = { 0 };

	LOG_INF("Test 2: Flash Erase");
	LOG_INF("Erase offset: 0x%x, size: %u bytes", offset, len);

	ret = flash_erase(dev, offset, len);
	zassert_equal(ret, 0, "Error: Bulk Erase Failed [%d]", ret);

	ret = flash_read(dev, offset, r_buf, BUFF_SIZE);
	zassert_equal(ret, 0, "Error: [RetVal :%d] Reading Erased value", ret);

	for (i = 0; i < BUFF_SIZE; i++) {
		if (r_buf[i] != flash_param->erase_value) {
			count++;
		}
	}

	LOG_INF("Total errors after reading erased chip = %d", count);
}

static void multi_page_test(const struct device *dev)
{
	int ret, e_count = 0;
	int64_t start_time, end_time;
	uint8_t w_buf[SPI_FLASH_SECTOR_SIZE];
	uint8_t r_buf[SPI_FLASH_SECTOR_SIZE];
	size_t page_size;
	int64_t tm_taken_ms;
	const size_t num_pages = 4;

	flash_param = flash_get_parameters(dev);
	zassert_not_null(flash_param, "Failed to get flash parameters");
	page_size = flash_param->page_size;
	zassert_true(page_size > 0 && page_size <= sizeof(w_buf),
		     "Unsupported page size: %zu", page_size);

	/* Erase enough sectors to cover num_pages pages */
	size_t total_size = num_pages * page_size;
	size_t erase_size = ((total_size + SPI_FLASH_SECTOR_SIZE - 1) /
			     SPI_FLASH_SECTOR_SIZE) * SPI_FLASH_SECTOR_SIZE;

	LOG_INF("Test 3: Flash erase");
	LOG_INF("Erasing %zu bytes for %zu pages of %zu bytes",
		erase_size, num_pages, page_size);

	ret = flash_erase(dev, SPI_FLASH_TEST_REGION_OFFSET, erase_size);
	zassert_equal(ret, 0, "Error: Flash erase Failed [%d]", ret);

	LOG_INF("Test 3: Flash write (%zu pages)", num_pages);

	start_time = k_uptime_get();
	for (size_t p = 0; p < num_pages; p++) {
		for (size_t i = 0; i < page_size; i++) {
			w_buf[i] = (i + p * 37) % 256;
		}

		ret = flash_write(dev,
				  SPI_FLASH_TEST_REGION_OFFSET + p * page_size,
				  w_buf, page_size);
		zassert_equal(ret, 0,
			      "Error: page %zu write Failed [%d]", p, ret);
	}
	end_time = k_uptime_get();

	tm_taken_ms = end_time - start_time;
	LOG_INF("Write completed in %lld ms", tm_taken_ms);

	LOG_INF("Test 3: Flash read and verify (%zu pages)", num_pages);

	for (size_t p = 0; p < num_pages; p++) {
		/* Regenerate expected pattern for this page */
		for (size_t i = 0; i < page_size; i++) {
			w_buf[i] = (i + p * 37) % 256;
		}

		memset(r_buf, 0, page_size);
		ret = flash_read(dev,
				 SPI_FLASH_TEST_REGION_OFFSET + p * page_size,
				 r_buf, page_size);
		zassert_equal(ret, 0,
			      "Error: page %zu read Failed [%d]", p, ret);

		for (size_t i = 0; i < page_size; i++) {
			if (r_buf[i] != w_buf[i]) {
				e_count++;
				if (e_count <= 5) {
					LOG_INF("Page %zu [%zu] "
						"w[0x%02x] r[0x%02x]",
						p, i, w_buf[i], r_buf[i]);
				}
			}
		}
	}

	if (e_count) {
		LOG_INF("Error: Data read NOT matches data written");
		LOG_INF(" -- number of Unmatched data [%d]", e_count);
	} else {
		LOG_INF("Data read matches data written. Good!!");
	}
	zassert_equal(e_count, 0,
		      "Multi-page test failed: %d mismatches", e_count);
}

static void multi_sector_test(void)
{
	int ret, i, e_count = 0;
	uint8_t w_buf[BUFF_SIZE] = { 0 };
	uint8_t r_buf[BUFF_SIZE] = { 0 };
	const size_t len = ARRAY_SIZE(w_buf);

	flash_param = flash_get_parameters(flash_dev);
	zassert_not_null(flash_param, "Failed to get flash parameters");

	/* Verify device readiness before proceeding */
	zassert_true(device_is_ready(flash_dev), "%s: device not ready.", flash_dev->name);

	for (i = 0; i < BUFF_SIZE; i++) {
		w_buf[i] = i % 256;
	}

	/* Prepare sectors 4 and 5 by erasing before writing */
	LOG_INF("\nTest 4: Prepare sectors 4 and 5 (erase before write)\n");
	ret = flash_erase(flash_dev, SPI_FLASH_SECTOR_4_OFFSET, SPI_FLASH_SECTOR_SIZE * 2);
	zassert_equal(ret, 0, "Error: Preparation erase failed! [%d]\n", ret);

	LOG_INF("Test 4: write sector %d", SPI_FLASH_SECTOR_4_OFFSET);

	/* Write into Sector 4 */
	ret = flash_write(flash_dev, SPI_FLASH_SECTOR_4_OFFSET, w_buf, len);
	zassert_equal(ret, 0, "Error: Flash write failed at Sec 4! [%d]\n", ret);

	LOG_INF("Test 4: write sector %d", SPI_FLASH_SECTOR_5_OFFSET);

	/* Write into Sector 5 */
	ret = flash_write(flash_dev, SPI_FLASH_SECTOR_5_OFFSET, w_buf, len);
	zassert_equal(ret, 0,
		     "Error: Flash write failed at Sec 5! [%d]",
		     ret);

	/* Read from Sector 4 */
	LOG_INF("Sec4: Read and Verify written data");
	e_count = 0;
	memset(r_buf, 0, len);

	LOG_INF("Test 4: read sector %d", SPI_FLASH_SECTOR_4_OFFSET);

	ret = flash_read(flash_dev, SPI_FLASH_SECTOR_4_OFFSET, r_buf, len);
	zassert_equal(ret, 0,
		     "Error: Flash read Failed Sec 4! [%d]", ret);

	for (i = 0; i < BUFF_SIZE; i++) {
		if (r_buf[i] != w_buf[i]) {
			e_count++;
			LOG_INF("Not matched [%d] w[%4x] r[%4x]",
				i, w_buf[i], r_buf[i]);
		}
	}

	if (e_count) {
		LOG_INF("Error: Data read NOT matches data written");
		LOG_INF(" -- number of Unmatched data [%d]", e_count);
	} else {
		LOG_INF("Data read matches data written. Good!!");
	}

	/* Read from Sector 5 */
	LOG_INF("Sec5: Read and Verify written data");
	e_count = 0;
	memset(r_buf, 0, len);

	LOG_INF("Test 4: read sector %d", SPI_FLASH_SECTOR_5_OFFSET);

	ret = flash_read(flash_dev, SPI_FLASH_SECTOR_5_OFFSET, r_buf, len);
	zassert_equal(ret, 0,
		     "Error: Flash read Failed Sec 5! [%d]", ret);

	for (i = 0; i < BUFF_SIZE; i++) {
		if (r_buf[i] != w_buf[i]) {
			e_count++;
			LOG_INF("Not matched [%d] w[%4x] r[%4x]",
				i, w_buf[i], r_buf[i]);
		}
	}

	if (e_count) {
		LOG_INF("Error: Data read NOT matches data written");
		LOG_INF(" -- number of Unmatched data [%d]", e_count);
	} else {
		LOG_INF("Data read matches data written. Good!!");
	}

	/* Erase multiple Sector Sec 4+5 */
	LOG_INF("Test 4: Erase Sector 4 and 5");
	zassert_equal(e_count, 0,
		      "Sec 4 verify failed - %d mismatches",
		      e_count);
	LOG_INF("Flash Erase from Sector %d Size to Erase %d",
		SPI_FLASH_SECTOR_4_OFFSET, SPI_FLASH_SECTOR_SIZE * 2);

	ret = flash_erase(flash_dev, SPI_FLASH_SECTOR_4_OFFSET,
			  SPI_FLASH_SECTOR_SIZE * 2);
	zassert_equal(ret, 0,
		     "Error: Multi-Sector erase failed! [%d]", ret);

	int count_1 = 0;

	memset(r_buf, 0, len);
	LOG_INF("Test 4: read sector %d", SPI_FLASH_SECTOR_4_OFFSET);

	/* Read Erased value and compare */
	ret = flash_read(flash_dev, SPI_FLASH_SECTOR_4_OFFSET, r_buf, BUFF_SIZE);
	zassert_equal(ret, 0, "Error: Reading Erased value [%d]", ret);

	/* Verify the read data */
	for (i = 0; i < BUFF_SIZE; i++) {
		if (r_buf[i] != flash_param->erase_value) {
			count_1++;
		}
	}

	LOG_INF("Total errors reading erased Sec 4 = %d",
		count_1);

	int count_2 = 0;

	memset(r_buf, 0, len);
	LOG_INF("Test 4: read sector %d", SPI_FLASH_SECTOR_5_OFFSET);

	/* Read Erased value and compare */
	ret = flash_read(flash_dev, SPI_FLASH_SECTOR_5_OFFSET, r_buf, BUFF_SIZE);
	zassert_equal(ret, 0, "Error: Reading Erased value [%d]", ret);

	/* Verify the read data */
	for (i = 0; i < BUFF_SIZE; i++) {
		if (r_buf[i] != flash_param->erase_value) {
			count_2++;
		}
	}

	LOG_INF("Total errors reading erased Sec 5 = %d", count_2);

	if (count_1 == 0 && count_2 == 0) {
		LOG_INF("Multi-Sector Erase Test Succeeded !");
	} else {
		LOG_INF("Multi-Sector Erase Failed");
	}
	zassert_equal(count_1, 0,
		"Sector 4 erase verification failed - %d non-erased bytes found", count_1);
	zassert_equal(count_2, 0,
		"Sector 5 erase verification failed - %d non-erased bytes found", count_2);
	zassert_equal(count_1 + count_2, 0,
		"Multi-Sector Erase Test Failed - Total errors: %d",
		count_1 + count_2);
}

/* -------- Test: write data to flash -------- */
static void write_data_to_flash_test(void)
{
	int ret, e_count;
	int64_t start_time, end_time;
	uint8_t w_buf[BUFF_SIZE];
	uint8_t r_buf[BUFF_SIZE];
	const size_t len = sizeof(w_buf);

	for (int i = 0; i < BUFF_SIZE; i++) {
		w_buf[i] = i % 256;
	}

	LOG_INF("Write data: erase test region");
	ret = flash_erase(flash_dev, SPI_FLASH_TEST_REGION_OFFSET,
			  SPI_FLASH_SECTOR_SIZE);
	zassert_equal(ret, 0, "Flash erase failed [%d]", ret);

	LOG_INF("Write data: writing %zu bytes", len);
	start_time = k_uptime_get();
	ret = flash_write(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, w_buf, len);
	end_time = k_uptime_get();
	zassert_equal(ret, 0, "Flash write failed!");
	LOG_INF("Write completed in %lld ms", end_time - start_time);

	LOG_INF("Write data: read and verify");
	memset(r_buf, 0, len);
	ret = flash_read(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, r_buf, len);
	zassert_equal(ret, 0, "Flash read failed!");

	e_count = verify_data(w_buf, r_buf, len, "write_data");
	zassert_equal(e_count, 0,
		     "Write data test failed: %d mismatches",
		     e_count);
	LOG_INF("Write data to flash test PASSED");
}

/* -------- Test: data pattern robustness -------- */
static void data_pattern_test(void)
{
	int ret, e_count;
	uint8_t w_buf[BUFF_SIZE];
	uint8_t r_buf[BUFF_SIZE];

	LOG_INF("=== Data Pattern Robustness Test ===");

	/* Test 1: All zeros pattern */
	LOG_INF("Test all-zeros pattern");
	memset(w_buf, 0x00, BUFF_SIZE);

	ret = flash_erase(flash_dev,
			  SPI_FLASH_TEST_REGION_OFFSET,
			  SPI_FLASH_SECTOR_SIZE);
	zassert_equal(ret, 0, "Erase failed [%d]", ret);

	ret = flash_fill(flash_dev, 0x00,
			 SPI_FLASH_TEST_REGION_OFFSET,
			 BUFF_SIZE);
	zassert_equal(ret, 0, "All-zeros fill failed [%d]", ret);

	memset(r_buf, 0xFF, BUFF_SIZE);
	ret = flash_read(flash_dev,
			 SPI_FLASH_TEST_REGION_OFFSET,
			 r_buf, BUFF_SIZE);
	zassert_equal(ret, 0, "All-zeros read failed [%d]", ret);

	e_count = verify_data(w_buf, r_buf, BUFF_SIZE, "all_zeros");
	zassert_equal(e_count, 0,
		     "All-zeros verify failed: %d mismatches",
		     e_count);
	LOG_INF("All-zeros pattern PASSED");

	/* Test 2: All ones pattern */
	LOG_INF("Test all-ones pattern");
	memset(w_buf, 0xFF, BUFF_SIZE);

	ret = flash_erase(flash_dev,
			  SPI_FLASH_TEST_REGION_OFFSET,
			  SPI_FLASH_SECTOR_SIZE);
	zassert_equal(ret, 0, "Erase failed [%d]", ret);

	ret = flash_fill(flash_dev, 0xFF,
			 SPI_FLASH_TEST_REGION_OFFSET,
			 BUFF_SIZE);
	zassert_equal(ret, 0, "All-ones fill failed [%d]", ret);

	memset(r_buf, 0x00, BUFF_SIZE);
	ret = flash_read(flash_dev,
			 SPI_FLASH_TEST_REGION_OFFSET,
			 r_buf, BUFF_SIZE);
	zassert_equal(ret, 0, "All-ones read failed [%d]", ret);

	e_count = verify_data(w_buf, r_buf, BUFF_SIZE, "all_ones");
	zassert_equal(e_count, 0,
		     "All-ones verify failed: %d mismatches",
		     e_count);
	LOG_INF("All-ones pattern PASSED");

	/* Test 3: Alternating pattern (0xAA) */
	LOG_INF("Test alternating pattern 0xAA");
	memset(w_buf, 0xAA, BUFF_SIZE);

	ret = flash_erase(flash_dev,
			  SPI_FLASH_TEST_REGION_OFFSET,
			  SPI_FLASH_SECTOR_SIZE);
	zassert_equal(ret, 0, "Erase failed [%d]", ret);

	ret = flash_fill(flash_dev, 0xAA,
			 SPI_FLASH_TEST_REGION_OFFSET,
			 BUFF_SIZE);
	zassert_equal(ret, 0, "0xAA pattern fill failed [%d]", ret);

	memset(r_buf, 0x00, BUFF_SIZE);
	ret = flash_read(flash_dev,
			 SPI_FLASH_TEST_REGION_OFFSET,
			 r_buf, BUFF_SIZE);
	zassert_equal(ret, 0, "0xAA pattern read failed [%d]", ret);

	e_count = verify_data(w_buf, r_buf, BUFF_SIZE, "pattern_0xAA");
	zassert_equal(e_count, 0,
		     "0xAA pattern verify failed: %d mismatches",
		     e_count);
	LOG_INF("0xAA pattern PASSED");

	/* Test 4: Alternating pattern (0x55) */
	LOG_INF("Test alternating pattern 0x55");
	memset(w_buf, 0x55, BUFF_SIZE);

	ret = flash_erase(flash_dev,
			  SPI_FLASH_TEST_REGION_OFFSET,
			  SPI_FLASH_SECTOR_SIZE);
	zassert_equal(ret, 0, "Erase failed [%d]", ret);

	ret = flash_fill(flash_dev, 0x55,
			 SPI_FLASH_TEST_REGION_OFFSET,
			 BUFF_SIZE);
	zassert_equal(ret, 0, "0x55 pattern fill failed [%d]", ret);

	memset(r_buf, 0x00, BUFF_SIZE);
	ret = flash_read(flash_dev,
			 SPI_FLASH_TEST_REGION_OFFSET,
			 r_buf, BUFF_SIZE);
	zassert_equal(ret, 0, "0x55 pattern read failed [%d]", ret);

	e_count = verify_data(w_buf, r_buf, BUFF_SIZE, "pattern_0x55");
	zassert_equal(e_count, 0,
		     "0x55 pattern verify failed: %d mismatches",
		     e_count);
	LOG_INF("0x55 pattern PASSED");

	/* Test 5: Walking bit pattern */
	LOG_INF("Test walking bit pattern");
	for (int i = 0; i < BUFF_SIZE; i++) {
		w_buf[i] = (1 << (i % 8));
	}

	ret = flash_erase(flash_dev,
			  SPI_FLASH_TEST_REGION_OFFSET,
			  SPI_FLASH_SECTOR_SIZE);
	zassert_equal(ret, 0, "Erase failed [%d]", ret);

	ret = flash_write(flash_dev,
			  SPI_FLASH_TEST_REGION_OFFSET,
			  w_buf, BUFF_SIZE);
	zassert_equal(ret, 0, "Walking bit write failed [%d]", ret);

	memset(r_buf, 0x00, BUFF_SIZE);
	ret = flash_read(flash_dev,
			 SPI_FLASH_TEST_REGION_OFFSET,
			 r_buf, BUFF_SIZE);
	zassert_equal(ret, 0, "Walking bit read failed [%d]", ret);

	e_count = verify_data(w_buf, r_buf, BUFF_SIZE, "walking_bit");
	zassert_equal(e_count, 0,
		     "Walking bit verify failed: %d mismatches",
		     e_count);
	LOG_INF("Walking bit pattern PASSED");

	LOG_INF("Data pattern robustness test PASSED");
}

/* ======== ZTEST wrappers ======== */

/* TESTLINK TEST CASE - ZTC-1096 */
ZTEST(test_ospi_flash, test_ospi_configure_setup)
{
	configure_setup_ospi_flash_test();
}

/* TESTLINK TEST CASE - ZTC-1097 */
ZTEST(test_ospi_flash, test_ospi_device_status)
{
	get_ospi_device_status();
}

ZTEST(test_ospi_flash, test_ospi_flash_parameters)
{
	LOG_INF("%s OSPI flash testing", flash_dev->name);
	LOG_INF("========================================");

	flash_param = flash_get_parameters(flash_dev);
	zassert_not_null(flash_param, "Failed to get flash parameters");

	LOG_INF("****Flash Configured Parameters******");
	LOG_INF("* Num Of Sectors : %d", flash_param->num_of_sector);
	LOG_INF("* Sector Size : %d", flash_param->sector_size);
	LOG_INF("* Page Size : %d", flash_param->page_size);
	LOG_INF("* Erase value : %d", flash_param->erase_value);
	LOG_INF("* Write Blk Size: %d", flash_param->write_block_size);
	LOG_INF("* Total Size in Bytes: %d",
		flash_param->num_of_sector * flash_param->sector_size);
	LOG_INF("* Test Region Offset: 0x%x",
		(uint32_t)SPI_FLASH_TEST_REGION_OFFSET);
}

/* TESTLINK TEST CASE - ZTC-1099 */
ZTEST(test_ospi_flash, test_ospi_single_sector)
{
	single_sector_test();
}

/* TESTLINK TEST CASE - ZTC-1100 */
ZTEST(test_ospi_flash, test_ospi_full_erase)
{
	flash_param = flash_get_parameters(flash_dev);
	/* Erase limited sectors - full 64MB chip erase is too slow.
	 * Avoids boot image at offset 0x0 (XIP addr 0xC0000000).
	 */
	uint32_t test_erase_size = SPI_FLASH_SECTOR_SIZE * 4;

	erase_test(flash_dev, SPI_FLASH_TEST_REGION_OFFSET,
		   test_erase_size);
}

/* TESTLINK TEST CASE - ZTC-1101 */
ZTEST(test_ospi_flash, test_ospi_multi_page)
{
	multi_page_test(flash_dev);
}

/* TESTLINK TEST CASE - ZTC-1102 */
ZTEST(test_ospi_flash, test_ospi_multi_sector)
{
	multi_sector_test();
}

/* TESTLINK TEST CASE - ZTC-1104 */
ZTEST(test_ospi_flash, test_ospi_write_data_to_flash)
{
	write_data_to_flash_test();
}

/* TESTLINK TEST CASE - ZTC-2021 */
ZTEST(test_ospi_flash, test_ospi_data_patterns)
{
	data_pattern_test();
}

/* ======== Test Suite Lifecycle Functions ======== */

static void *test_ospi_flash_setup(void)
{
	LOG_INF("=== OSPI Flash Test Suite Setup ===");
	ospi_flash_setup_device();
	LOG_INF("Flash device ready: %s", flash_dev->name);

	LOG_INF("Test suite setup complete");
	return NULL;
}

static void test_ospi_flash_before(void *fixture)
{
	(void)fixture;
	ospi_flash_setup_device();
	LOG_INF("--- Before test ---");
}

ZTEST_SUITE(test_ospi_flash, NULL, test_ospi_flash_setup,
	    test_ospi_flash_before, NULL, NULL);

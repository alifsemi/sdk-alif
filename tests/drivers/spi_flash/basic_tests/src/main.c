/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
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
#include <zephyr/random/random.h>
#include <zephyr/logging/log.h>
#include <zephyr/ztest.h>

LOG_MODULE_REGISTER(test_ospi_flash_app, LOG_LEVEL_INF);

#define SPI_FLASH_TEST_REGION_OFFSET 0x0
#define SPI_FLASH_SECTOR_SIZE        4096
#define BUFF_SIZE                    1024
#define SPI_FLASH_TEST_ITERATIONS    10
#define SPI_FLASH_SECTOR_4_OFFSET    (4 * 1024 * 4)
#define SPI_FLASH_SECTOR_5_OFFSET    (5 * 1024 * 4)

const struct flash_parameters *flash_param;
const struct device *flash_dev = DEVICE_DT_GET(DT_ALIAS(spi_flash0));

static void generate_random_data(uint8_t *data, size_t size)
{
	for (size_t i = 0; i < size; i++) {
		/* Generate random byte */
		data[i] = sys_rand32_get() % 256;
	}
}

static void get_ospi_device_status(void)
{
	zassert_true(device_is_ready(flash_dev), "%s: device not ready.", flash_dev->name);
}

static void single_sector_test(void)
{
	const uint8_t expected[] = { 0x55, 0xaa, 0x66, 0x99 };
	const size_t len = ARRAY_SIZE(expected);
	uint8_t buf[len];
	int ret;
	int i, e_count = 0;
	uint32_t start_time, end_time, start_cycle, end_cycle;

	LOG_INF("\nTest 1: Flash erase\n");

	/*
	 * Full flash erase if SPI_FLASH_TEST_REGION_OFFSET = 0 and
	 * SPI_FLASH_SECTOR_SIZE = flash size
	 */
	ret = flash_erase(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, SPI_FLASH_SECTOR_SIZE);
	zassert_equal(ret, 0, "Flash write failed!");

	LOG_INF("\nTest 1: Flash write\n");
	LOG_INF("Attempting to write %zu bytes\n", len);

	start_time = k_uptime_get();
	start_cycle = k_cycle_get_32();
	ret = flash_write(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, expected, len);
	end_cycle = k_cycle_get_32();
	end_time = k_uptime_get();

	zassert_equal(ret, 0, "Flash write failed!");

	float time_taken_ms = end_time - start_time;
	float cycle_taken = end_cycle - start_cycle;

	ARG_UNUSED(cycle_taken);
	LOG_INF("Write operation completed in %f microseconds\n", (double)time_taken_ms);

	LOG_INF("\nTest 1: Flash read\n");

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

static void erase_test(const struct device *dev, uint32_t len)
{
	int ret = 0, i = 0, count = 0;
	uint8_t r_buf[BUFF_SIZE] = { 0 };

	LOG_INF("\nTest 2: Flash Full Erase\n");
	LOG_INF("total sector's size: %d\n", len);

	ret = flash_erase(dev, SPI_FLASH_TEST_REGION_OFFSET, len);
	zassert_equal(ret, 0, "Error: Bulk Erase Failed [%d]\n", ret);

	ret = flash_read(dev, SPI_FLASH_TEST_REGION_OFFSET, r_buf, BUFF_SIZE);
	zassert_equal(ret, 0, "Error: [RetVal :%d] Reading Erased value\n", ret);

	for (i = 0; i < BUFF_SIZE; i++) {
		if (r_buf[i] != flash_param->erase_value) {
			count++;
		}
	}

	LOG_INF("Total errors after reading erased chip = %d\n", count);
}

static void multi_page_test(const struct device *dev)
{
	int ret, i, e_count = 0;
	uint32_t start_time, end_time;
	uint8_t w_buf[BUFF_SIZE] = { 0 };
	uint8_t r_buf[BUFF_SIZE] = { 0 };
	const size_t len = ARRAY_SIZE(w_buf);
	float tm_taken_ms;

	LOG_INF("\nTest 3: Flash erase\n");

	ret = flash_erase(dev, SPI_FLASH_TEST_REGION_OFFSET, SPI_FLASH_SECTOR_SIZE);
	zassert_equal(ret, 0, "Error: Flash erase Failed [%d]\n", ret);

	for (i = 0; i < BUFF_SIZE; i++) {
		w_buf[i] = i % 256;
	}

	LOG_INF("\nTest 3: Flash write\n");
	LOG_INF("Attempting to write %zu bytes\n", len);

	start_time = k_uptime_get();
	ret = flash_write(dev, SPI_FLASH_TEST_REGION_OFFSET, w_buf, len);
	end_time = k_uptime_get();
	zassert_equal(ret, 0, "Error: Flash write Failed [%d]\n", ret);

	tm_taken_ms = end_time - start_time;
	LOG_INF("Write operation test 3: completed in %f microseconds\n", (double)tm_taken_ms);

	LOG_INF("\nTest 3: Flash read\n");

	memset(r_buf, 0, len);
	ret = flash_read(dev, SPI_FLASH_TEST_REGION_OFFSET, r_buf, len);
	zassert_equal(ret, 0, "Error: Flash read Failed [%d]\n", ret);

	for (i = 0; i < BUFF_SIZE; i++) {
		if (r_buf[i] != w_buf[i]) {
			e_count++;
			LOG_INF("Not matched at [%d] _w[%4x] _r[%4x]\n", i, w_buf[i], r_buf[i]);
		}
	}

	if (e_count) {
		LOG_INF("Error: Data read NOT matches data written\n");
		LOG_INF(" -- number of Unmatched data [%d]\n", e_count);
	} else {
		LOG_INF("Data read matches data written. Good!!\n");
	}
}

static void multi_sector_test(void)
{
	int ret, i, e_count = 0;
	uint8_t w_buf[BUFF_SIZE] = { 0 };
	uint8_t r_buf[BUFF_SIZE] = { 0 };
	const size_t len = ARRAY_SIZE(w_buf);

	for (i = 0; i < BUFF_SIZE; i++) {
		w_buf[i] = i % 256;
	}

	LOG_INF("\nTest 4: write sector %d\n", SPI_FLASH_SECTOR_4_OFFSET);

	/* Write into Sector 4 */
	ret = flash_write(flash_dev, SPI_FLASH_SECTOR_4_OFFSET, w_buf, len);
	zassert_equal(ret, 0, "Error: Flash write failed at Sec 4! [%d]\n", ret);

	LOG_INF("\nTest 4: write sector %d\n", SPI_FLASH_SECTOR_5_OFFSET);

	/* Write into Sector 5 */
	ret = flash_write(flash_dev, SPI_FLASH_SECTOR_5_OFFSET, w_buf, len);
	zassert_equal(ret, 0, "Error: Flash write failed at Sec 5! [%d]\n", ret);

	/* Read from Sector 4 */
	LOG_INF("Sec4: Read and Verify written data\n");
	e_count = 0;
	memset(r_buf, 0, len);

	LOG_INF("\nTest 4: read sector %d\n", SPI_FLASH_SECTOR_4_OFFSET);

	ret = flash_read(flash_dev, SPI_FLASH_SECTOR_4_OFFSET, r_buf, len);
	zassert_equal(ret, 0, "Error: Flash read Failed at Sector 4! [%d]\n", ret);

	for (i = 0; i < BUFF_SIZE; i++) {
		if (r_buf[i] != w_buf[i]) {
			e_count++;
			LOG_INF("Not matched at [%d] _w[%4x] _r[%4x]\n", i, w_buf[i], r_buf[i]);
		}
	}

	if (e_count) {
		LOG_INF("\nError: Data read NOT matches data written\n");
		LOG_INF(" -- number of Unmatched data [%d]\n", e_count);
	} else {
		LOG_INF("\nData read matches data written. Good!!\n");
	}

	/* Read from Sector 5 */
	LOG_INF("Sec5: Read and Verify written data\n");
	e_count = 0;
	memset(r_buf, 0, len);

	LOG_INF("\nTest 4: read sector %d\n", SPI_FLASH_SECTOR_5_OFFSET);

	ret = flash_read(flash_dev, SPI_FLASH_SECTOR_5_OFFSET, r_buf, len);
	zassert_equal(ret, 0, "Error: Flash read Failed at Sector 5! [%d]\n", ret);

	for (i = 0; i < BUFF_SIZE; i++) {
		if (r_buf[i] != w_buf[i]) {
			e_count++;
			LOG_INF("Not matched at [%d] _w[%4x] _r[%4x]\n", i, w_buf[i], r_buf[i]);
		}
	}

	if (e_count) {
		LOG_INF("Error: Data read NOT matches data written\n");
		LOG_INF(" -- number of Unmatched data [%d]\n", e_count);
	} else {
		LOG_INF("Data read matches data written. Good!!\n");
	}

	/* Erase multiple Sector Sec 4+5 */
	LOG_INF("\nTest 4: Erase Sector 4 and 5\n");
	LOG_INF("Flash Erase from Sector %d Size to Erase %d\n",
		SPI_FLASH_SECTOR_4_OFFSET, SPI_FLASH_SECTOR_SIZE * 2);

	ret = flash_erase(flash_dev, SPI_FLASH_SECTOR_4_OFFSET, SPI_FLASH_SECTOR_SIZE * 2);
	zassert_equal(ret, 0, "Error: Multi-Sector erase failed! [%d]\n", ret);

	int count_1 = 0;

	memset(r_buf, 0, len);
	LOG_INF("\nTest 4: read sector %d\n", SPI_FLASH_SECTOR_4_OFFSET);

	/* Read Erased value and compare */
	ret = flash_read(flash_dev, SPI_FLASH_SECTOR_4_OFFSET, r_buf, BUFF_SIZE);
	zassert_equal(ret, 0, "Error: Reading Erased value [%d]\n", ret);

	/* Verify the read data */
	for (i = 0; i < BUFF_SIZE; i++) {
		if (r_buf[i] != flash_param->erase_value) {
			count_1++;
		}
	}

	LOG_INF("Total errors after reading erased Sector 4 = %d\n", count_1);

	int count_2 = 0;

	memset(r_buf, 0, len);
	LOG_INF("\nTest 4: read sector %d\n", SPI_FLASH_SECTOR_5_OFFSET);

	/* Read Erased value and compare */
	ret = flash_read(flash_dev, SPI_FLASH_SECTOR_5_OFFSET, r_buf, BUFF_SIZE);
	zassert_equal(ret, 0, "Error: Reading Erased value [%d]\n", ret);

	/* Verify the read data */
	for (i = 0; i < BUFF_SIZE; i++) {
		if (r_buf[i] != flash_param->erase_value) {
			count_2++;
		}
	}

	LOG_INF("Total errors after reading erased Sector 5 = %d\n", count_2);

	if (count_1 == 0 && count_2 == 0) {
		LOG_INF("\nMulti-Sector Erase Test Succeeded !\n");
	} else {
		LOG_INF("\nMulti-Sector Erase Failed\n");
	}
}

static void xip_test(const struct device *dev)
{
	uint8_t i;
	uint32_t xip_r[64] = { 0 }, fls_r[64] = { 0 }, cnt;
	uint32_t *ptr = (uint32_t *)DT_PROP_BY_IDX(DT_PARENT(DT_ALIAS(spi_flash0)),
						   xip_base_address, 0);
	int32_t rc, e_count = 0;

	LOG_INF("\n XiP Read\n");

	memcpy(xip_r, ptr, sizeof(xip_r));
	LOG_INF("Content Read from OSPI Flash in XiP Mode successfully\n\n");

	cnt = ARRAY_SIZE(xip_r);
	LOG_INF("Read from Flash cmd while XiP Mode turned on\n\n");

	rc = flash_read(dev, SPI_FLASH_TEST_REGION_OFFSET, fls_r, cnt * sizeof(uint32_t));
	zassert_equal(rc, 0, "Flash read failed! %d\n", rc);

	for (i = 0; i < cnt; i++) {
		if (fls_r[i] != xip_r[i]) {
			e_count++;
		}
	}

	zassert_true(!e_count,
		     "XiP Test Failed ! contents are NOT Matching : Err Count [%d]!!!\n",
		     e_count);

	LOG_INF("XiP Read Test Succeeded !!\n\n");
}

/* -------- Tests -------- */

ZTEST(test_ospi_flash, test_ospi_device_status)
{
	get_ospi_device_status();
}

ZTEST(test_ospi_flash, test_test_ospi_flash_parameters)
{
	LOG_INF("\n%s OSPI flash testing\n", flash_dev->name);
	LOG_INF("========================================\n");

	flash_param = flash_get_parameters(flash_dev);

	LOG_INF("****Flash Configured Parameters******\n");
	LOG_INF("* Num Of Sectors : %d\n", flash_param->num_of_sector);
	LOG_INF("* Sector Size : %d\n", flash_param->sector_size);
	LOG_INF("* Page Size : %d\n", flash_param->page_size);
	LOG_INF("* Erase value : %d\n", flash_param->erase_value);
	LOG_INF("* Write Blk Size: %d\n", flash_param->write_block_size);
	LOG_INF("* Total Size in Bytes: %d\n",
		flash_param->num_of_sector * flash_param->sector_size);
}

/* Test: Single sector erase / write / read verification */

ZTEST(test_ospi_flash, test_ospi_single_sector)
{
	single_sector_test();
}

/* Test: Full flash erase and verify erased pattern over entire device */
ZTEST(test_ospi_flash, test_ospi_full_erase)
{
	flash_param = flash_get_parameters(flash_dev);
	erase_test(flash_dev, flash_param->num_of_sector * flash_param->sector_size);
}

/* Test: Multi-page write/read performance and data integrity on one sector */
ZTEST(test_ospi_flash, test_ospi_multi_page)
{
	multi_page_test(flash_dev);
}

/* Test: Multi-sector write/read and erase verification (e.g. sectors 4 & 5) */
ZTEST(test_ospi_flash, test_ospi_multi_sector)
{
	multi_sector_test();
}

/* Test: XiP read path – compare XiP-mapped reads with normal flash reads */
ZTEST(test_ospi_xip, test_ospi_xip_mode)
{
	xip_test(flash_dev);
}

/* -------- Suites -------- */

ZTEST_SUITE(test_ospi_flash, NULL, NULL, NULL, NULL, NULL);

#if CONFIG_TEST_XIP_MODE
ZTEST_SUITE(test_ospi_xip, NULL, NULL, NULL, NULL, NULL);
#endif

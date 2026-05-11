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

LOG_MODULE_REGISTER(ospi_boundary, LOG_LEVEL_INF);

/* -------- Test: cross-page boundary read/write -------- */
static void cross_page_boundary_test(void)
{
	int ret, e_count = 0;
	uint8_t w_buf[BUFF_SIZE] = {0};
	uint8_t r_buf[BUFF_SIZE] = {0};
	const size_t len = ARRAY_SIZE(w_buf);
	uint32_t cross_offset;

	LOG_INF("=== Cross-Page Boundary Test ===");

	/* Prepare test data */
	for (int i = 0; i < BUFF_SIZE; i++) {
		w_buf[i] = (i + 100) % 256;
	}

	/* Use an offset that crosses a page boundary (4KB page size) */
	cross_offset = SPI_FLASH_TEST_REGION_OFFSET +
		       SPI_FLASH_SECTOR_SIZE - (BUFF_SIZE / 2);

	/* Erase both sectors that the cross-page write will span */
	ret = flash_erase(flash_dev, SPI_FLASH_TEST_REGION_OFFSET,
			  SPI_FLASH_SECTOR_SIZE * 2);
	zassert_equal(ret, 0, "Flash erase failed [%d]", ret);

	/* Write data crossing page boundary */
	ret = flash_write(flash_dev, cross_offset, w_buf, len);
	zassert_equal(ret, 0, "Cross-page write failed [%d]", ret);

	/* Read back across page boundary */
	memset(r_buf, 0, len);
	ret = flash_read(flash_dev, cross_offset, r_buf, len);
	zassert_equal(ret, 0, "Cross-page read failed [%d]", ret);

	e_count = verify_data(w_buf, r_buf, len, "cross_page");
	zassert_equal(e_count, 0,
		     "Cross-page verify failed: %d mismatches",
		     e_count);

	LOG_INF("Cross-page boundary test PASSED");
}

/* -------- Test: address boundaries -------- */
static void address_boundary_test(void)
{
	int ret;
	uint8_t w_buf[256] = {0};
	uint8_t r_buf[256] = {0};
	const size_t len = ARRAY_SIZE(w_buf);
	uint32_t test_offsets[] = {
		SPI_FLASH_TEST_REGION_OFFSET, /* Safe region */
		SPI_FLASH_TEST_REGION_OFFSET + 0x1000,  /* 4KB offset */
		SPI_FLASH_TEST_REGION_OFFSET + 0x10000, /* 64KB offset */
		SPI_FLASH_TEST_REGION_OFFSET + 0x100000 /* 1MB offset */
	};
	size_t num_offsets = ARRAY_SIZE(test_offsets);

	LOG_INF("=== Address Boundary Test ===");

	/* Prepare test data */
	for (int i = 0; i < len; i++) {
		w_buf[i] = (i + 50) % 256;
	}

	/* Erase test region */
	ret = flash_erase(flash_dev, SPI_FLASH_TEST_REGION_OFFSET,
			  SPI_FLASH_SECTOR_SIZE * 2);
	zassert_equal(ret, 0, "Flash erase failed [%d]", ret);

	/* Test write/read at various address boundaries */
	for (size_t i = 0; i < num_offsets; i++) {
		uint32_t offset = test_offsets[i];
		int e_count;

		LOG_INF("Testing at offset 0x%x", offset);

		/* Ensure target sector is erased before write */
		ret = flash_erase(flash_dev, offset,
				  SPI_FLASH_SECTOR_SIZE);
		zassert_equal(ret, 0,
			     "Sector erase failed at 0x%x [%d]",
			     offset, ret);

		/* Write */
		ret = flash_write(flash_dev, offset,
				  w_buf, len);
		zassert_equal(ret, 0,
			     "Write failed at 0x%x [%d]",
			     offset, ret);

		/* Read */
		memset(r_buf, 0, len);
		ret = flash_read(flash_dev, offset,
				 r_buf, len);
		zassert_equal(ret, 0,
			     "Read failed at 0x%x [%d]",
			     offset, ret);

		/* Verify */
		e_count = verify_data(w_buf, r_buf, len,
				      "address_boundary");
		zassert_equal(e_count, 0,
			     "Verify failed at 0x%x: %d mismatches",
			     offset, e_count);
	}

	LOG_INF("Address boundary test PASSED");
}

/* -------- Test: boundary size operations -------- */
static void boundary_size_operations_test(void)
{
	int ret, e_count;
	uint8_t min_buf[256];
	uint8_t page_buf[4096];  /* Page size buffer */
	uint8_t r_buf[4096];
	const struct flash_parameters *params;
	size_t min_len;
	size_t page_len;
	uint32_t page_offset;

	LOG_INF("=== Boundary Size Operations Test ===");

	params = flash_get_parameters(flash_dev);
	zassert_not_null(params, "flash parameters are NULL");

	min_len = params->write_block_size;
	page_len = params->page_size;

	zassert_true(min_len > 0 && min_len <= sizeof(min_buf),
		     "Invalid/min unsupported write_block_size: %zu", min_len);
	zassert_true(page_len > 0 && page_len <= sizeof(page_buf),
		     "Invalid/unsupported page_size: %zu", page_len);
	zassert_true(page_len <= SPI_FLASH_SECTOR_SIZE,
		     "page_size (%zu) larger than sector (%d)",
		     page_len, SPI_FLASH_SECTOR_SIZE);

	/* Erase test region */
	ret = flash_erase(flash_dev, SPI_FLASH_TEST_REGION_OFFSET,
			  SPI_FLASH_SECTOR_SIZE);
	zassert_equal(ret, 0, "Erase failed [%d]", ret);

	/* Test exact minimum programmable size (write_block_size) */
	memset(min_buf, 0xAA, min_len);
	ret = flash_fill(flash_dev, 0xAA,
			 SPI_FLASH_TEST_REGION_OFFSET,
			 min_len);
	zassert_equal(ret, 0, "Minimum size fill failed [%d]", ret);

	memset(r_buf, 0, min_len);
	ret = flash_read(flash_dev, SPI_FLASH_TEST_REGION_OFFSET,
			 r_buf, min_len);
	zassert_equal(ret, 0, "Minimum size read failed [%d]", ret);

	e_count = verify_data(min_buf, r_buf, min_len,
			      "min_size");
	zassert_equal(e_count, 0,
		     "Min size verify failed: %d mismatches",
		     e_count);

	LOG_INF("Minimum write-block boundary (%zu bytes) PASSED", min_len);

	/* Test exact page-size operation near boundary */
	page_offset = SPI_FLASH_TEST_REGION_OFFSET +
		      (SPI_FLASH_SECTOR_SIZE - page_len);

	ret = flash_erase(flash_dev, SPI_FLASH_TEST_REGION_OFFSET,
			  SPI_FLASH_SECTOR_SIZE);
	zassert_equal(ret, 0, "Page sector erase failed [%d]", ret);

	memset(page_buf, 0x55, page_len);
	ret = flash_fill(flash_dev, 0x55,
			 page_offset,
			 page_len);
	zassert_equal(ret, 0, "Page size fill failed [%d]", ret);

	memset(r_buf, 0, page_len);
	ret = flash_read(flash_dev, page_offset,
			r_buf, page_len);
	zassert_equal(ret, 0, "Page size read failed [%d]", ret);

	e_count = verify_data(page_buf, r_buf, page_len,
			      "page_size");
	zassert_equal(e_count, 0,
		     "Page size verify failed: %d mismatches",
		     e_count);

	LOG_INF("Page boundary operation (%zu bytes @0x%x) PASSED",
		page_len, page_offset);
}

/* ======== Boundary Test Suite ======== */

/* TESTLINK TEST CASE - ZTC-2001 */
ZTEST(test_ospi_boundary, test_ospi_address_boundaries)
{
	address_boundary_test();
}

/* TESTLINK TEST CASE - ZTC-2002 */
ZTEST(test_ospi_boundary, test_ospi_cross_page_boundary)
{
	cross_page_boundary_test();
}

/* TESTLINK TEST CASE - ZTC-2004 */
ZTEST(test_ospi_boundary, test_ospi_boundary_size_operations)
{
	boundary_size_operations_test();
}

/* ======== Test Suite Lifecycle Functions ======== */

static void *test_ospi_boundary_setup(void)
{
	LOG_INF("=== OSPI Boundary Test Suite Setup ===");
	ospi_flash_setup_device();
	return NULL;
}

static void test_ospi_boundary_before(void *fixture)
{
	(void)fixture;
	ospi_flash_setup_device();
}

ZTEST_SUITE(test_ospi_boundary, NULL, test_ospi_boundary_setup,
	    test_ospi_boundary_before, NULL,
	    NULL);

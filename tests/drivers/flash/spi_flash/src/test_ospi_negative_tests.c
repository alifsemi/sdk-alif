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

LOG_MODULE_REGISTER(ospi_negative, LOG_LEVEL_INF);

/* -------- Test: write without erase (negative scenario) -------- */
static void write_without_erase_test(void)
{
	int ret;
	uint8_t w_buf[BUFF_SIZE] = {0};
	uint8_t before_buf[BUFF_SIZE] = {0};
	uint8_t r_buf[BUFF_SIZE] = {0};
	const size_t len = ARRAY_SIZE(w_buf);
	int e_count = 0;
	int changed_bytes = 0;
	int programmable_bytes = 0;

	LOG_INF("=== Write Without Erase Test ===");
	LOG_INF("Read-before, write-without-erase, read-after validation");

	/* Prepare test data */
	for (int i = 0; i < BUFF_SIZE; i++) {
		w_buf[i] = (i + 200) % 256;
	}

	/* Read current content before writing */
	ret = flash_read(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, before_buf, len);
	zassert_equal(ret, 0, "Initial read failed [%d]", ret);

	/* Write to flash without erasing first */
	ret = flash_write(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, w_buf, len);
	zassert_equal(ret, 0, "Write failed [%d]", ret);

	/* Read back */
	memset(r_buf, 0, len);
	ret = flash_read(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, r_buf, len);
	zassert_equal(ret, 0, "Read failed [%d]", ret);

	/* NOR behavior check without erase:
	 * resulting byte must be (before & requested).
	 */
	for (size_t i = 0; i < len; i++) {
		uint8_t expected = before_buf[i] & w_buf[i];

		if (expected != before_buf[i]) {
			programmable_bytes++;
		}

		if (r_buf[i] != before_buf[i]) {
			changed_bytes++;
		}

		if (r_buf[i] != expected) {
			e_count++;
			if (e_count <= 5) {
				LOG_INF("Mismatch[%zu] before=0x%02x req=0x%02x "
					"exp=0x%02x got=0x%02x",
					i, before_buf[i], w_buf[i], expected, r_buf[i]);
			}
		}
	}

	zassert_equal(e_count, 0,
		     "Write-without-erase violated NOR behavior: %d mismatches",
		     e_count);

	/* If there were bits available to be programmed 1->0, data should change. */
	if (programmable_bytes > 0) {
		zassert_true(changed_bytes > 0,
			     "Expected changed data after write without erase");
	}

	LOG_INF("Before/After changed bytes: %d", changed_bytes);
	LOG_INF("Programmable bytes in request: %d", programmable_bytes);

	LOG_INF("Write without erase test completed");
}

/* -------- Test: invalid parameters -------- */
static void invalid_parameter_test(void)
{
	int ret;
	uint8_t buf[BUFF_SIZE] = {0};

	LOG_INF("=== Invalid Parameter Test ===");

	/* Validate setup */
	zassert_not_null(flash_param, "flash_param is NULL");
	zassert_not_null(flash_dev, "flash_dev is NULL");

	/* Test 1: Invalid offset (beyond flash size) */
	ret = flash_read(flash_dev,
			 flash_param->num_of_sector *
			 flash_param->sector_size + 1,
			 buf, BUFF_SIZE);
	zassert_not_equal(ret, 0, "Should fail with invalid offset");

	LOG_INF("Invalid offset test passed");

	/* Test 2: Zero length */
	ret = flash_read(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, buf, 0);
	/* Zero length may or may not be valid depending on implementation */
	LOG_INF("Zero length test completed (ret=%d)", ret);

	/* Test 3: Length exceeds flash size */
	ret = flash_read(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, buf,
			flash_param->num_of_sector * flash_param->sector_size);
	zassert_not_equal(ret, 0,
			  "Should fail with length exceeding size");

	LOG_INF("Invalid length test passed");

	LOG_INF("Invalid parameter test PASSED");
}

/* -------- Helper: verify unaligned read buffer -------- */
static int verify_unaligned_read(const uint8_t *r_buf,
				 const uint8_t *w_buf,
				 uint32_t offset_delta,
				 uint32_t unaligned_offset,
				 size_t read_len)
{
	int e_count = 0;

	for (size_t j = 0; j < read_len; j++) {
		uint8_t expected = w_buf[j + offset_delta];

		if (r_buf[j] == expected) {
			continue;
		}
		e_count++;
		if (e_count <= 5) {
			LOG_INF("Mismatch 0x%x: exp 0x%02x got 0x%02x",
				(unsigned int)(unaligned_offset + j),
				expected, r_buf[j]);
		}
	}
	return e_count;
}

/* -------- Test: unaligned address read/write -------- */
static void unaligned_address_test(void)
{
	int ret, i, e_count = 0;
	uint8_t w_buf[BUFF_SIZE] = {0};
	uint8_t r_buf[BUFF_SIZE] = {0};
	const size_t len = ARRAY_SIZE(w_buf);
	uint32_t aligned_offset;
	uint32_t unaligned_offsets[] = {
		1, 3, 7, 15, 31, 63, 127, 255
	};
	size_t num_unaligned_tests = ARRAY_SIZE(unaligned_offsets);
	const struct flash_parameters *params;

	LOG_INF("=== OSPI Unaligned Address Read/Write ===");

	/* Validate setup */
	zassert_not_null(flash_dev, "flash_dev is NULL");

	params = flash_get_parameters(flash_dev);
	zassert_not_null(params, "flash parameters are NULL");

	/* Prepare test data */
	for (i = 0; i < BUFF_SIZE; i++) {
		w_buf[i] = (i + 100) % 256;
	}

	/* Use an aligned base offset */
	aligned_offset = SPI_FLASH_SECTOR_SIZE * 10; /* Use sector 10 as base */

	/* Erase the test region */
	ret = flash_erase(flash_dev, aligned_offset, SPI_FLASH_SECTOR_SIZE);
	zassert_equal(ret, 0,
		     "Flash erase failed for unaligned [%d]", ret);

	/* Write aligned data first */
	ret = flash_write(flash_dev, aligned_offset, w_buf, len);
	zassert_equal(ret, 0,
		     "Flash write failed for unaligned [%d]", ret);

	/* Test reading from various unaligned offsets */
	for (i = 0; i < num_unaligned_tests; i++) {
		uint32_t unaligned_offset;
		size_t read_len = len - unaligned_offsets[i];

		unaligned_offset = aligned_offset + unaligned_offsets[i];

		if (read_len > 0) {
			memset(r_buf, 0, len);

			LOG_INF("Testing unaligned read 0x%x (+%d)",
				unaligned_offset, unaligned_offsets[i]);

			ret = flash_read(flash_dev, unaligned_offset,
					 r_buf, read_len);

			/* Alif OSPI driver does not support unaligned
			 * addresses. Should fail with -EINVAL.
			 */
			if (ret == -EINVAL) {
				LOG_INF("Unaligned read rejected at 0x%x",
					unaligned_offset);
				continue;
			}

			/* If unaligned read succeeded, verify data */
			if (ret == 0) {
				LOG_INF("Unaligned read OK at 0x%x",
					unaligned_offset);

				/* Verify data matches expected pattern */
				e_count = verify_unaligned_read(
					r_buf, w_buf,
					unaligned_offsets[i],
					unaligned_offset,
					read_len);

				if (e_count == 0) {
					LOG_INF("Unaligned read verified +%d",
						unaligned_offsets[i]);
				}
			} else {
				/* Unexpected error */
				zassert_equal(ret, 0,
					     "Unexpected err at 0x%x [%d]",
					     unaligned_offset, ret);
			}
		}
	}

	/* Test writing to unaligned addresses if supported */
	LOG_INF("Testing unaligned write operations");

	/* Erase a sector for unaligned write tests */
	ret = flash_erase(flash_dev, aligned_offset + SPI_FLASH_SECTOR_SIZE,
			  SPI_FLASH_SECTOR_SIZE);
	zassert_equal(ret, 0,
		     "Erase for unaligned write failed [%d]",
		     ret);

	for (i = 0; i < 3; i++) { /* Test a few unaligned write offsets */
		uint32_t unaligned_write_offset;

		unaligned_write_offset = aligned_offset +
			SPI_FLASH_SECTOR_SIZE + (i + 1) * 3;
		uint8_t test_pattern[] = {0xAA, 0x55, 0xFF, 0x00, 0x33, 0xCC};
		size_t pattern_len = ARRAY_SIZE(test_pattern);
		uint8_t read_back[ARRAY_SIZE(test_pattern)];

		/* Align pattern_len to write_block_size */
		flash_param = flash_get_parameters(flash_dev);
		if (flash_param->write_block_size > 1) {
			size_t wb = flash_param->write_block_size;

			pattern_len = (pattern_len / wb) * wb;
		}

		memset(read_back, 0, pattern_len);

		LOG_INF("Testing unaligned write to offset 0x%x (%zu bytes)",
			unaligned_write_offset, pattern_len);

		ret = flash_write(flash_dev, unaligned_write_offset,
				  test_pattern, pattern_len);

		/* Alif OSPI driver does not support unaligned
		 * addresses. Should fail with -EINVAL.
		 */
		if (ret == -EINVAL) {
			LOG_INF("Unaligned write rejected at 0x%x",
				unaligned_write_offset);
		} else if (ret == 0) {
			/* If unaligned write succeeded, verify it */
			LOG_INF("Unaligned write OK at 0x%x",
				unaligned_write_offset);

			/* Read back and verify */
			ret = flash_read(flash_dev,
					 unaligned_write_offset,
					 read_back, pattern_len);
			zassert_equal(ret, 0,
				     "Read failed after unaligned write [%d]",
				     ret);

			e_count = 0;
			for (size_t j = 0; j < pattern_len; j++) {
				if (read_back[j] != test_pattern[j]) {
					e_count++;
				}
			}

			if (e_count == 0) {
				LOG_INF("Unaligned write passed at 0x%x",
					unaligned_write_offset);
			} else {
				LOG_INF("Unaligned write fail 0x%x: %d err",
					unaligned_write_offset, e_count);
			}
		} else {
			/* Unexpected error */
			zassert_equal(ret, 0,
				     "Unexpected err at write 0x%x [%d]",
				     unaligned_write_offset, ret);
		}
	}

	LOG_INF("Unaligned address test PASSED");
}

/* ======== Negative Test Suite ======== */

/* TESTLINK TEST CASE - ZTC-2008 */
ZTEST(test_ospi_negative, test_ospi_write_without_erase)
{
	write_without_erase_test();
}

/* TESTLINK TEST CASE - ZTC-2007 */
ZTEST(test_ospi_negative, test_ospi_invalid_parameters)
{
	invalid_parameter_test();
}

/* TESTLINK TEST CASE - ZTC-1547 */
ZTEST(test_ospi_negative, test_ospi_unaligned_address)
{
	unaligned_address_test();
}

/* ======== Test Suite Lifecycle Functions ======== */

static void *test_ospi_negative_setup(void)
{
	LOG_INF("=== OSPI Negative Test Suite Setup ===");
	ospi_flash_setup_device();
	return NULL;
}

static void test_ospi_negative_before(void *fixture)
{
	(void)fixture;
	ospi_flash_setup_device();
}

ZTEST_SUITE(test_ospi_negative, NULL, test_ospi_negative_setup,
	    test_ospi_negative_before, NULL,
	    NULL);

/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/* MRAM Flash Driver Functional Test Suite */

#include <zephyr/kernel.h>
#include <zephyr/ztest.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/devicetree.h>
#include <zephyr/storage/flash_map.h>
#include <string.h>

/* Test area definitions derived from devicetree storage_partition*/
#if !FIXED_PARTITION_EXISTS(storage_partition)
#error "MRAM functional tests require a storage_partition in the board overlay"
#endif
#define TEST_AREA          storage_partition
#define TEST_AREA_OFFSET   FIXED_PARTITION_OFFSET(TEST_AREA)
#define TEST_AREA_SIZE     FIXED_PARTITION_SIZE(TEST_AREA)
#define TEST_AREA_MAX      (TEST_AREA_OFFSET + TEST_AREA_SIZE)
#define TEST_AREA_DEVICE   FIXED_PARTITION_DEVICE(TEST_AREA)

/* Maximum buffer size used across tests */
#define EXPECTED_SIZE      10240
#define SMALL_BUF_SIZE     256
#define CHUNK_SIZE         1024

static const struct device *flash_dev;
static struct flash_pages_info page_info;
static const struct flash_parameters *flash_params;
static uint8_t erase_value;

/* Shared buffers */
static uint8_t write_buf[CHUNK_SIZE];
static uint8_t read_buf[CHUNK_SIZE];

/* setup: validate device, get parameters, check test region size */
static void *flash_functional_setup(void)
{
	int rc;

	flash_dev = TEST_AREA_DEVICE;

	zassert_true(device_is_ready(flash_dev),
		     "MRAM flash device is not ready");

	flash_params = flash_get_parameters(flash_dev);
	zassert_not_null(flash_params, "flash_get_parameters returned NULL");

	erase_value = flash_params->erase_value;

	rc = flash_get_page_info_by_offs(flash_dev, TEST_AREA_OFFSET,
					 &page_info);
	zassert_equal(rc, 0,
		      "flash_get_page_info_by_offs failed: %d", rc);

	zassert_true(EXPECTED_SIZE <= TEST_AREA_SIZE,
			"EXPECTED_SIZE %u exceeds partition size %u",
			EXPECTED_SIZE, (uint32_t)TEST_AREA_SIZE);

	return NULL;
}

static void flash_functional_before(void *fixture)
{
	ARG_UNUSED(fixture);

	size_t erase_len = page_info.size *
			    ((EXPECTED_SIZE + page_info.size - 1) /
			     page_info.size);

	zassert_true((TEST_AREA_OFFSET + erase_len) <= TEST_AREA_MAX,
		     "Erase length %zu overruns partition", erase_len);

	int rc = flash_erase(flash_dev, TEST_AREA_OFFSET, erase_len);

	zassert_equal(rc, 0, "Per-test erase failed: %d", rc);
}


ZTEST(mram_functional, test_flash_parameters)
{
	const struct flash_parameters *params;

	params = flash_get_parameters(flash_dev);
	zassert_not_null(params, "flash_get_parameters returned NULL");

	zassert_equal(params->write_block_size, 16,
		      "write_block_size expected 16, got %zu",
		      params->write_block_size);

	zassert_equal(params->erase_value, 0x00,
		      "erase_value expected 0x00, got 0x%02x",
		      params->erase_value);
}

ZTEST(mram_functional, test_flash_page_layout)
{
	struct flash_pages_info info;
	int rc;

	rc = flash_get_page_info_by_offs(flash_dev, TEST_AREA_OFFSET, &info);
	zassert_equal(rc, 0, "flash_get_page_info_by_offs failed: %d", rc);

	/* erase-block-size from DTS is 1024 */
	zassert_equal(info.size, 1024,
		      "Page size expected 1024, got %zu", info.size);

	/* Page index should be >= 0 */
	zassert_true(info.index >= 0,
		     "Page index should be non-negative, got %d", info.index);
}

ZTEST(mram_functional, test_erase_and_verify)
{
	int rc;
	size_t erase_sz = page_info.size; /* 1024 bytes */

	/* Write non-zero data first */
	memset(write_buf, 0xAB, erase_sz);
	rc = flash_write(flash_dev, TEST_AREA_OFFSET, write_buf, erase_sz);
	zassert_equal(rc, 0, "Write before erase failed: %d", rc);

	/* Erase */
	rc = flash_erase(flash_dev, TEST_AREA_OFFSET, erase_sz);
	zassert_equal(rc, 0, "Erase failed: %d", rc);

	/* Verify all bytes are erase_value */
	memset(read_buf, 0xFF, erase_sz);
	rc = flash_read(flash_dev, TEST_AREA_OFFSET, read_buf, erase_sz);
	zassert_equal(rc, 0, "Read after erase failed: %d", rc);

	for (size_t i = 0; i < erase_sz; i++) {
		zassert_equal(read_buf[i], erase_value,
			      "Byte at offset %zu: expected 0x%02x, got 0x%02x",
			      i, erase_value, read_buf[i]);
	}
}

ZTEST(mram_functional, test_erase_unaligned_offset)
{
	int rc;

	/* Offset not multiple of erase-block-size → should fail */
	rc = flash_erase(flash_dev, TEST_AREA_OFFSET + 1, page_info.size);
	zassert_equal(rc, -EINVAL,
		      "Erase with unaligned offset should return -EINVAL, got %d",
		      rc);
}

ZTEST(mram_functional, test_erase_unaligned_length)
{
	int rc;

	/* Length not multiple of erase-block-size → should fail */
	rc = flash_erase(flash_dev, TEST_AREA_OFFSET, page_info.size + 1);
	zassert_equal(rc, -EINVAL,
		      "Erase with unaligned length should return -EINVAL, got %d",
		      rc);
}

ZTEST(mram_functional, test_write_aligned)
{
	int rc;
	const size_t len = 64; /* multiple of 16 */

	for (size_t i = 0; i < len; i++) {
		write_buf[i] = (uint8_t)(i & 0xFF);
	}

	rc = flash_write(flash_dev, TEST_AREA_OFFSET, write_buf, len);
	zassert_equal(rc, 0, "Aligned write failed: %d", rc);

	memset(read_buf, 0, len);
	rc = flash_read(flash_dev, TEST_AREA_OFFSET, read_buf, len);
	zassert_equal(rc, 0, "Read after aligned write failed: %d", rc);

	zassert_mem_equal(read_buf, write_buf, len,
		"Data mismatch after aligned write");
}


ZTEST(mram_functional, test_write_unaligned_offset)
{
	int rc;
	const size_t region = 64;
	const off_t rel = 5; /* not 16-aligned */
	const size_t len = 32;
	uint8_t expected[64];
	uint8_t unaligned_data[32];

	memset(expected, 0xCC, region);
	rc = flash_write(flash_dev, TEST_AREA_OFFSET, expected, region);
	zassert_equal(rc, 0, "Base write failed: %d", rc);

	for (size_t i = 0; i < len; i++) {
		unaligned_data[i] = (uint8_t)(0x10 + i);
	}

	rc = flash_write(flash_dev, TEST_AREA_OFFSET + rel,
			 unaligned_data, len);
	zassert_equal(rc, 0, "Unaligned offset write failed: %d", rc);

	memcpy(expected + rel, unaligned_data, len);

	memset(read_buf, 0, region);
	rc = flash_read(flash_dev, TEST_AREA_OFFSET, read_buf, region);
	zassert_equal(rc, 0, "Read after unaligned write failed: %d", rc);

	zassert_mem_equal(read_buf, expected, region,
			  "Unaligned offset write corrupted neighbors");
}

ZTEST(mram_functional, test_write_unaligned_length)
{
	int rc;
	const size_t region = 64;
	const size_t len = 25; /* not a multiple of 16 */
	uint8_t expected[64];

	memset(expected, erase_value, region);
	for (size_t i = 0; i < len; i++) {
		write_buf[i] = (uint8_t)(0x30 + i);
		expected[i] = write_buf[i];
	}

	rc = flash_write(flash_dev, TEST_AREA_OFFSET, write_buf, len);
	zassert_equal(rc, 0, "Unaligned length write failed: %d", rc);

	memset(read_buf, 0, region);
	rc = flash_read(flash_dev, TEST_AREA_OFFSET, read_buf, region);
	zassert_equal(rc, 0, "Read after unaligned length write failed: %d", rc);

	zassert_mem_equal(read_buf, expected, region,
			  "Unaligned length write corrupted sector remainder");
}

ZTEST(mram_functional, test_read_basic)
{
	int rc;
	const size_t len = 48;

	/* Write known data */
	for (size_t i = 0; i < len; i++) {
		write_buf[i] = (uint8_t)(0x40 + i);
	}
	rc = flash_write(flash_dev, TEST_AREA_OFFSET, write_buf, len);
	zassert_equal(rc, 0, "Write failed: %d", rc);

	/* Read from different offsets within the written region */
	for (off_t off = 0; off < 16; off++) {
		size_t rlen = len - off;

		memset(read_buf, 0, rlen);
		rc = flash_read(flash_dev, TEST_AREA_OFFSET + off,
				read_buf, rlen);
		zassert_equal(rc, 0,
			      "Read at offset %ld failed: %d", (long)off, rc);
		zassert_mem_equal(read_buf, write_buf + off, rlen,
			      "Data mismatch reading from offset %ld",
			      (long)off);
	}
}

ZTEST(mram_functional, test_read_unaligned_address)
{
	int rc;
	uint8_t buf[SMALL_BUF_SIZE + 8];
	const uint8_t canary = erase_value;

	/* Fill test area with known data */
	for (size_t i = 0; i < SMALL_BUF_SIZE; i++) {
		write_buf[i] = (uint8_t)((i + 1) & 0xFF);
		if (write_buf[i] == erase_value) {
			write_buf[i]++;
		}
	}
	rc = flash_write(flash_dev, TEST_AREA_OFFSET,
			 write_buf, SMALL_BUF_SIZE);
	zassert_equal(rc, 0, "Cannot write to flash");

	/* Test various combinations of read length, address offset, buffer offset */
	for (off_t len = 0; len < 25; len++) {
		for (off_t ad_o = 0; ad_o < 4; ad_o++) {
			for (off_t buf_o = 1; buf_o < 5; buf_o++) {
				buf[buf_o - 1] = canary;
				buf[buf_o + len] = canary;
				memset(buf + buf_o, 0, len);

				rc = flash_read(flash_dev,
						TEST_AREA_OFFSET + ad_o,
						buf + buf_o, len);
				zassert_equal(rc, 0, "Cannot read flash");

				zassert_mem_equal(buf + buf_o,
						  write_buf + ad_o, len,
						  "Flash read mismatch at len=%d, "
						  "ad_o=%d, buf_o=%d",
						  len, ad_o, buf_o);

				/* Check buffer guards */
				zassert_equal(buf[buf_o - 1], canary,
					      "Buffer underflow at len=%d, "
					      "ad_o=%d, buf_o=%d",
					      len, ad_o, buf_o);
				zassert_equal(buf[buf_o + len], canary,
					      "Buffer overflow at len=%d, "
					      "ad_o=%d, buf_o=%d",
					      len, ad_o, buf_o);
			}
		}
	}
}


ZTEST(mram_functional, test_write_read_patterns)
{
	int rc;
	const size_t len = 64;
	const uint8_t patterns[] = {0x00, 0xFF, 0xAA, 0x55};

	for (int p = 0; p < (int)ARRAY_SIZE(patterns); p++) {
		/* Erase before each pattern test */
		rc = flash_erase(flash_dev, TEST_AREA_OFFSET, page_info.size);
		zassert_equal(rc, 0, "Erase before pattern 0x%02x failed",
			      patterns[p]);

		memset(write_buf, patterns[p], len);
		rc = flash_write(flash_dev, TEST_AREA_OFFSET, write_buf, len);
		zassert_equal(rc, 0, "Write pattern 0x%02x failed: %d",
			      patterns[p], rc);

		memset(read_buf, ~patterns[p], len);
		rc = flash_read(flash_dev, TEST_AREA_OFFSET, read_buf, len);
		zassert_equal(rc, 0, "Read pattern 0x%02x failed: %d",
			      patterns[p], rc);

		zassert_mem_equal(read_buf, write_buf, len,
			      "Pattern 0x%02x mismatch", patterns[p]);
	}

	/* Incremental pattern */
	rc = flash_erase(flash_dev, TEST_AREA_OFFSET, page_info.size);
	zassert_equal(rc, 0, "Erase before incremental pattern failed");

	for (size_t i = 0; i < len; i++) {
		write_buf[i] = (uint8_t)(i & 0xFF);
	}
	rc = flash_write(flash_dev, TEST_AREA_OFFSET, write_buf, len);
	zassert_equal(rc, 0, "Incremental write failed: %d", rc);

	memset(read_buf, 0, len);
	rc = flash_read(flash_dev, TEST_AREA_OFFSET, read_buf, len);
	zassert_equal(rc, 0, "Incremental read failed: %d", rc);

	zassert_mem_equal(read_buf, write_buf, len,
		      "Incremental pattern mismatch");
}


ZTEST(mram_functional, test_erase_write_read_cycle)
{
	int rc;
	const size_t len = 128;

	/* Step 1: Erase */
	rc = flash_erase(flash_dev, TEST_AREA_OFFSET, page_info.size);
	zassert_equal(rc, 0, "Erase failed: %d", rc);

	/* Step 2: Verify erased to 0x00 */
	rc = flash_read(flash_dev, TEST_AREA_OFFSET, read_buf, len);
	zassert_equal(rc, 0, "Read after erase failed: %d", rc);

	for (size_t i = 0; i < len; i++) {
		zassert_equal(read_buf[i], erase_value,
			      "Byte %zu not erased: 0x%02x", i, read_buf[i]);
	}

	/* Step 3: Write */
	for (size_t i = 0; i < len; i++) {
		write_buf[i] = (uint8_t)((i * 3 + 7) & 0xFF);
	}
	rc = flash_write(flash_dev, TEST_AREA_OFFSET, write_buf, len);
	zassert_equal(rc, 0, "Write after erase failed: %d", rc);

	/* Step 4: Read and verify */
	memset(read_buf, 0, len);
	rc = flash_read(flash_dev, TEST_AREA_OFFSET, read_buf, len);
	zassert_equal(rc, 0, "Final read failed: %d", rc);

	zassert_mem_equal(read_buf, write_buf, len,
		      "Data mismatch in erase-write-read cycle");
}

ZTEST(mram_functional, test_large_data_transfer)
{
	int rc;
	const size_t len = EXPECTED_SIZE;
	size_t erase_len = page_info.size *
			   ((len + page_info.size - 1) / page_info.size);

	rc = flash_erase(flash_dev, TEST_AREA_OFFSET, erase_len);
	zassert_equal(rc, 0, "Large erase failed: %d", rc);

	for (size_t off = 0; off < len; off += CHUNK_SIZE) {
		size_t chunk = MIN(CHUNK_SIZE, len - off);

		for (size_t i = 0; i < chunk; i++) {
			write_buf[i] = (uint8_t)((off + i + 1) & 0xFF);
			if (write_buf[i] == erase_value) {
				write_buf[i]++;
			}
		}

		rc = flash_write(flash_dev, TEST_AREA_OFFSET + off,
				 write_buf, chunk);
		zassert_equal(rc, 0,
			      "Large write at offset %zu failed: %d", off, rc);
	}

	for (size_t off = 0; off < len; off += CHUNK_SIZE) {
		size_t chunk = MIN(CHUNK_SIZE, len - off);

		for (size_t i = 0; i < chunk; i++) {
			write_buf[i] = (uint8_t)((off + i + 1) & 0xFF);
			if (write_buf[i] == erase_value) {
				write_buf[i]++;
			}
		}

		memset(read_buf, 0, chunk);
		rc = flash_read(flash_dev, TEST_AREA_OFFSET + off,
				read_buf, chunk);
		zassert_equal(rc, 0,
			      "Large read at offset %zu failed: %d", off, rc);
		zassert_mem_equal(read_buf, write_buf, chunk,
				  "Large transfer mismatch at offset %zu", off);
	}
}

ZTEST(mram_functional, test_boundary_offset_overflow)
{
	int rc;
	uint8_t tmp[16];
	size_t page_count;
	off_t flash_size;

	/* Alif MRAM has no get_size(); device size is uniform pages. */
	page_count = flash_get_page_count(flash_dev);
	zassert_true(page_count > 0, "flash_get_page_count returned 0");

	flash_size = (off_t)page_count * (off_t)page_info.size;

	rc = flash_read(flash_dev, flash_size, tmp, 1);
	zassert_equal(rc, -EINVAL,
		      "Read past device end should return -EINVAL, got %d", rc);

	memset(tmp, 0xAA, sizeof(tmp));
	rc = flash_write(flash_dev, flash_size, tmp, 1);
	zassert_equal(rc, -EINVAL,
		      "Write past device end should return -EINVAL, got %d", rc);

	rc = flash_erase(flash_dev, flash_size, page_info.size);
	zassert_equal(rc, -EINVAL,
		      "Erase past device end should return -EINVAL, got %d", rc);
}

ZTEST(mram_functional, test_zero_length_operations)
{
	int rc;
	uint8_t tmp[1] = {0xDE};

	/* Zero-length read should succeed without modifying buffer */
	rc = flash_read(flash_dev, TEST_AREA_OFFSET, tmp, 0);
	zassert_equal(rc, 0,
		      "Zero-length read should succeed, got %d", rc);
	zassert_equal(tmp[0], 0xDE,
		      "Buffer modified by zero-length read");

	/* Zero-length write should succeed */
	rc = flash_write(flash_dev, TEST_AREA_OFFSET, tmp, 0);
	zassert_equal(rc, 0,
		      "Zero-length write should succeed, got %d", rc);
}

ZTEST_SUITE(mram_functional, NULL, flash_functional_setup,
	    flash_functional_before, NULL, NULL);

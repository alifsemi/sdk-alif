/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/* MRAM Flash Driver — LittleFS Filesystem Integration Test Suite */

#include <zephyr/kernel.h>
#include <zephyr/ztest.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/devicetree.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/fs/fs.h>
#include <zephyr/fs/littlefs.h>
#include <zephyr/logging/log.h>
#include <string.h>

LOG_MODULE_REGISTER(mram_lfs_test, LOG_LEVEL_INF);

/* ---------------------------------------------------------------------------
 * LittleFS mount configuration over MRAM storage_partition
 * -------------------------------------------------------------------------
 */
FS_LITTLEFS_DECLARE_DEFAULT_CONFIG(storage);

static struct fs_mount_t lfs_mnt = {
	.type = FS_LITTLEFS,
	.fs_data = &storage,
	.storage_dev = (void *)FIXED_PARTITION_ID(storage_partition),
	.mnt_point = "/lfs",
};

#define TEST_FILE      "/lfs/test_data.bin"
#define TEST_BOOT_FILE "/lfs/boot_count"
#define TEST_FILE_SIZE 128

static uint8_t file_write_buf[TEST_FILE_SIZE];
static uint8_t file_read_buf[TEST_FILE_SIZE];

/* ---------------------------------------------------------------------------
 * Helper: erase the flash area backing the filesystem
 * -------------------------------------------------------------------------
 */
static int erase_flash_area(void)
{
	const struct flash_area *pfa;
	int rc;

	rc = flash_area_open(FIXED_PARTITION_ID(storage_partition), &pfa);
	if (rc < 0) {
		return rc;
	}

	rc = flash_area_flatten(pfa, 0, pfa->fa_size);
	flash_area_close(pfa);
	return rc;
}

ZTEST(mram_filesystem, test_littlefs_mount)
{
	int rc;

	rc = fs_mount(&lfs_mnt);
	zassert_equal(rc, 0, "LittleFS mount failed: %d", rc);

	/* Unmount for the next test */
	rc = fs_unmount(&lfs_mnt);
	zassert_equal(rc, 0, "LittleFS unmount failed: %d", rc);
}

ZTEST(mram_filesystem, test_littlefs_file_create_write_read)
{
	struct fs_file_t file;
	int rc;
	ssize_t bytes;

	/* Mount */
	rc = fs_mount(&lfs_mnt);
	zassert_equal(rc, 0, "Mount failed: %d", rc);

	/* Prepare test pattern */
	for (int i = 0; i < TEST_FILE_SIZE; i++) {
		file_write_buf[i] = (uint8_t)((i * 7 + 3) & 0xFF);
	}

	/* Create and write file */
	fs_file_t_init(&file);
	rc = fs_open(&file, TEST_FILE, FS_O_CREATE | FS_O_WRITE);
	zassert_equal(rc, 0, "File open for write failed: %d", rc);

	bytes = fs_write(&file, file_write_buf, TEST_FILE_SIZE);
	zassert_equal(bytes, TEST_FILE_SIZE,
		      "File write returned %d, expected %d", bytes,
		      TEST_FILE_SIZE);

	rc = fs_close(&file);
	zassert_equal(rc, 0, "File close after write failed: %d", rc);

	/* Reopen and read back */
	fs_file_t_init(&file);
	rc = fs_open(&file, TEST_FILE, FS_O_READ);
	zassert_equal(rc, 0, "File open for read failed: %d", rc);

	memset(file_read_buf, 0, TEST_FILE_SIZE);
	bytes = fs_read(&file, file_read_buf, TEST_FILE_SIZE);
	zassert_equal(bytes, TEST_FILE_SIZE,
		      "File read returned %d, expected %d", bytes,
		      TEST_FILE_SIZE);

	rc = fs_close(&file);
	zassert_equal(rc, 0, "File close after read failed: %d", rc);

	/* Verify data integrity */
	zassert_equal(memcmp(file_write_buf, file_read_buf, TEST_FILE_SIZE), 0,
		      "File data mismatch after write/read");

	/* Cleanup */
	rc = fs_unmount(&lfs_mnt);
	zassert_equal(rc, 0, "Unmount failed: %d", rc);
}

ZTEST(mram_filesystem, test_littlefs_file_persistence)
{
	struct fs_file_t file;
	struct fs_dirent dirent;
	int rc;
	ssize_t bytes;
	uint8_t boot_count = 0;

	/* Mount */
	rc = fs_mount(&lfs_mnt);
	zassert_equal(rc, 0, "Mount failed: %d", rc);

	/* Write a boot count file */
	fs_file_t_init(&file);
	rc = fs_open(&file, TEST_BOOT_FILE, FS_O_CREATE | FS_O_RDWR);
	zassert_equal(rc, 0, "Open boot_count for write failed: %d", rc);

	boot_count = 42;
	bytes = fs_write(&file, &boot_count, sizeof(boot_count));
	zassert_equal(bytes, sizeof(boot_count), "Write boot_count failed");

	rc = fs_close(&file);
	zassert_equal(rc, 0, "Close boot_count failed: %d", rc);

	/* Unmount */
	rc = fs_unmount(&lfs_mnt);
	zassert_equal(rc, 0, "Unmount failed: %d", rc);

	/* Remount */
	rc = fs_mount(&lfs_mnt);
	zassert_equal(rc, 0, "Remount failed: %d", rc);

	/* Verify file still exists */
	rc = fs_stat(TEST_BOOT_FILE, &dirent);
	zassert_equal(rc, 0, "File stat after remount failed: %d", rc);
	zassert_equal(dirent.type, FS_DIR_ENTRY_FILE,
		      "boot_count is not a file after remount");
	zassert_equal(dirent.size, sizeof(boot_count),
		      "boot_count file size mismatch: %zu", dirent.size);

	/* Read back and verify value */
	fs_file_t_init(&file);
	rc = fs_open(&file, TEST_BOOT_FILE, FS_O_READ);
	zassert_equal(rc, 0, "Open boot_count for read failed: %d", rc);

	boot_count = 0;
	bytes = fs_read(&file, &boot_count, sizeof(boot_count));
	zassert_equal(bytes, sizeof(boot_count), "Read boot_count failed");
	zassert_equal(boot_count, 42,
		      "boot_count value mismatch: expected 42, got %u",
		      boot_count);

	rc = fs_close(&file);
	zassert_equal(rc, 0, "Close boot_count after read failed: %d", rc);

	/* Cleanup */
	rc = fs_unmount(&lfs_mnt);
	zassert_equal(rc, 0, "Final unmount failed: %d", rc);
}

ZTEST(mram_filesystem, test_littlefs_statvfs)
{
	struct fs_statvfs sbuf;
	int rc;

	/* Mount */
	rc = fs_mount(&lfs_mnt);
	zassert_equal(rc, 0, "Mount failed: %d", rc);

	rc = fs_statvfs(lfs_mnt.mnt_point, &sbuf);
	zassert_equal(rc, 0, "statvfs failed: %d", rc);

	/* LittleFS block size should be > 0 */
	zassert_true(sbuf.f_bsize > 0,
		     "Block size should be > 0, got %lu", sbuf.f_bsize);

	/* Total blocks should be > 0 */
	zassert_true(sbuf.f_blocks > 0,
		     "Total blocks should be > 0, got %lu", sbuf.f_blocks);

	LOG_INF("statvfs: bsize=%lu frsize=%lu blocks=%lu bfree=%lu",
		sbuf.f_bsize, sbuf.f_frsize, sbuf.f_blocks, sbuf.f_bfree);

	/* Cleanup */
	rc = fs_unmount(&lfs_mnt);
	zassert_equal(rc, 0, "Unmount failed: %d", rc);
}

ZTEST(mram_filesystem, test_littlefs_unmount)
{
	int rc;

	/* Mount first */
	rc = fs_mount(&lfs_mnt);
	zassert_equal(rc, 0, "Mount failed: %d", rc);

	/* Unmount should return 0 */
	rc = fs_unmount(&lfs_mnt);
	zassert_equal(rc, 0,
		      "LittleFS unmount should return 0, got %d", rc);
}

static void lfs_test_before(void *fixture)
{
	ARG_UNUSED(fixture);

	/* Best-effort unmount (ignore errors if not mounted) */
	(void)fs_unmount(&lfs_mnt);

	/* Erase storage area for clean state */
	int rc = erase_flash_area();

	zassert_equal(rc, 0, "Failed to erase flash area: %d", rc);
}

/* ---------------------------------------------------------------------------
 * Per-test teardown: ensure the filesystem is unmounted even if a test fails
 * mid-way, so subsequent tests don't get -EBUSY.
 * -------------------------------------------------------------------------
 */
static void lfs_test_after(void *fixture)
{
	ARG_UNUSED(fixture);

	/* Best-effort unmount — ignore errors (e.g. not mounted) */
	(void)fs_unmount(&lfs_mnt);
}

ZTEST_SUITE(mram_filesystem, NULL, NULL, lfs_test_before, lfs_test_after, NULL);

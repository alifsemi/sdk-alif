/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/ztest.h>
#include <ff.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/storage/disk_access.h>
#include <zephyr/drivers/disk.h>
#include <zephyr/logging/log.h>
#include <zephyr/fs/fs.h>
#include <string.h>
#include <errno.h>

#if defined(CONFIG_SDHC)
#include <zephyr/drivers/sdhc.h>
#endif

LOG_MODULE_REGISTER(test_sd, LOG_LEVEL_INF);

/* ─── Configuration ─── */
#define SD_DISK_NAME		"SD"
#define SD_MOUNT_POINT		"/SD:"
#define SD_TEST_FILE		"/SD:/test.txt"
#define SD_STRESS_FILE		"/SD:/stress.txt"
#define SD_HEALTH_FILE		"/SD:/HMON.TXT"
#define SD_LARGE_FILE		"/SD:/large.bin"
#define SD_DIR_PATH			"/SD:/testdir"
#define SD_DIR_FILE			"/SD:/testdir/sub.txt"

#define TEST_CONTENT		"Hello SD card!\n"
#define APPEND_CONTENT		"Appended line\n"
#define SECTOR_SIZE				512
#define DISK_INIT_RETRIES		3
#define RETRY_DELAY_MS			50
#define STRESS_ITERATIONS		25
#define MOUNT_STRESS_ITER		10
#define HEALTH_MONITOR_ITER		50
#define RAW_STRESS_ITER			100
#define FILE_CREATE_DEL_ITER	20
#define DEINIT_REINIT_ITER		5
#define LARGE_BUF_SIZE			2048

/* Raw sector test offset from end of disk to avoid FAT corruption */
#define RAW_TEST_SECTOR_OFFSET  64

/* ─── Shared Static Buffers (reused across tests; tests run sequentially) ─── */
static uint8_t sector_wr[SECTOR_SIZE] __aligned(4);
static uint8_t sector_rd[SECTOR_SIZE] __aligned(4);
static uint8_t sector_save[SECTOR_SIZE] __aligned(4);
static uint8_t multi_wr[SECTOR_SIZE * 4] __aligned(4);
static uint8_t multi_rd[SECTOR_SIZE * 4] __aligned(4);
static uint8_t multi_save[SECTOR_SIZE * 4] __aligned(4);
static uint8_t large_wr[LARGE_BUF_SIZE] __aligned(4);
static uint8_t large_rd[LARGE_BUF_SIZE] __aligned(4);

/* ─── State ─── */
static FATFS sd_fat_fs;
static bool sd_disk_inited;
static bool sd_fs_mounted;
static uint32_t sd_sector_count;

static struct fs_mount_t sd_mp = {
	.type = FS_FATFS,
	.fs_data = &sd_fat_fs,
	.storage_dev = (void *)SD_DISK_NAME,
	.mnt_point = SD_MOUNT_POINT,
};

#if defined(CONFIG_SDHC)
static const struct device *sdhc_dev;
static bool sdhc_dev_ready;
#endif

/* ─── Helpers ─── */
static int sd_ensure_disk_ready(void)
{
	int ret = -EIO;
	int i;

	if (!sd_disk_inited) {
		for (i = 0; i < DISK_INIT_RETRIES; i++) {
			ret = disk_access_ioctl(SD_DISK_NAME,
						DISK_IOCTL_CTRL_INIT, NULL);
			if (ret == 0) {
				break;
			}
			k_msleep(RETRY_DELAY_MS);
		}
		if (ret != 0) {
			return ret;
		}

		/* Cache sector count once */
		ret = disk_access_ioctl(SD_DISK_NAME, DISK_IOCTL_GET_SECTOR_COUNT,
					&sd_sector_count);
		if (ret != 0 || sd_sector_count == 0) {
			return ret ? ret : -ENODEV;
		}
		sd_disk_inited = true;
	}
	return 0;
}

static int sd_ensure_mounted(void)
{
	int ret;

	if (sd_fs_mounted) {
		return 0;
	}

	ret = sd_ensure_disk_ready();
	if (ret != 0) {
		return ret;
	}

	ret = fs_mount(&sd_mp);
	if (ret == 0 || ret == -EBUSY || ret == -EALREADY) {
		sd_fs_mounted = true;
		return 0;
	}

	/* Recovery path: force re-init and remount once */
	sd_disk_inited = false;
	ret = sd_ensure_disk_ready();
	if (ret != 0) {
		return ret;
	}
	ret = fs_mount(&sd_mp);
	if (ret == 0 || ret == -EBUSY || ret == -EALREADY) {
		sd_fs_mounted = true;
		return 0;
	}
	return ret;
}

static int sd_ensure_unmounted(void)
{
	if (sd_fs_mounted) {
		int ret = fs_unmount(&sd_mp);

		if (ret == 0 || ret == -ENOENT) {
			sd_fs_mounted = false;
			return 0;
		}
		return ret;
	}
	return 0;
}

/**
 * @brief Remove all test files and directories created during tests.
 *
 * Checks if each file exists before unlinking to avoid error messages.
 */
static void sd_cleanup_test_files(void)
{
	static const char * const cleanup_paths[] = {
		SD_TEST_FILE,
		SD_STRESS_FILE,
		SD_HEALTH_FILE,
		SD_LARGE_FILE,
		"/SD:/del.txt",
		"/SD:/f1.txt",
		"/SD:/f2.txt",
		"/SD:/1byte.txt",
		"/SD:/empty.txt",
		SD_DIR_FILE,
		SD_DIR_PATH,
		NULL
	};
	struct fs_dirent stat;
	int i;

	for (i = 0; cleanup_paths[i] != NULL; i++) {
		if (fs_stat(cleanup_paths[i], &stat) == 0) {
			fs_unlink(cleanup_paths[i]);
		}
	}
}

/**
 * @brief Suite setup: one-time initialization before any test runs.
 *
 * Validates the SDHC device readiness (if CONFIG_SDHC is enabled) so that
 * individual SDHC tests can skip cleanly without repeating the check.
 */
static void *sd_suite_setup(void)
{
#if defined(CONFIG_SDHC)
	sdhc_dev = DEVICE_DT_GET(DT_NODELABEL(sdhc));
	sdhc_dev_ready = device_is_ready(sdhc_dev);
#endif
	return NULL;
}

/**
 * @brief Before-each hook: ensure the SD filesystem is mounted.
 *
 * Called automatically before every test. Initializes the disk and mounts
 * the FATFS partition so individual tests do not need to call
 * sd_ensure_mounted() themselves.
 */
static void sd_test_before(void *fixture)
{
	ARG_UNUSED(fixture);

	int ret = sd_ensure_mounted();

	zassert_equal(ret, 0, "SD filesystem failed to mount (%d)", ret);
}

/**
 * @brief After-each hook: clean up test files created during the test.
 *
 * Called automatically after every test to remove leftover files so each
 * test starts with a clean slate. Only runs cleanup if the filesystem is
 * mounted (some tests end in unmounted state for raw I/O).
 */
static void sd_test_after(void *fixture)
{
	ARG_UNUSED(fixture);

	if (sd_fs_mounted) {
		sd_cleanup_test_files();
	}
}

/**
 * @brief Suite teardown: final cleanup after all tests complete.
 *
 * Remounts the filesystem if needed and cleans up any remaining test files.
 * Ensures the SD card is left in a clean state after the test suite runs.
 */
static void sd_suite_teardown(void *fixture)
{
	ARG_UNUSED(fixture);

	int ret = sd_ensure_mounted();

	if (ret == 0) {
		sd_cleanup_test_files();
		sd_ensure_unmounted();
	}
}

static uint32_t sd_raw_test_sector(void)
{
	zassert_true(sd_sector_count > RAW_TEST_SECTOR_OFFSET,
			"SD card too small for raw sector tests");
	return sd_sector_count - RAW_TEST_SECTOR_OFFSET;
}

/********************************
 *  E2E TESTS — SDHC Driver Level
 ********************************/

#if defined(CONFIG_SDHC)

/**
 * @brief Verify that the SDHC controller detects an inserted SD card.
 *
 * Skips if the SDHC device is not ready (checked once in the before-each hook).
 * Calls sdhc_card_present() and asserts it returns 1 (card detected).
 */
ZTEST(sd_card, test_sd_card_present)
{
	int ret;

	if (!sdhc_dev_ready) {
		ztest_test_skip();
		return;
	}

	ret = sdhc_card_present(sdhc_dev);
	zassert_equal(ret, 1, "SD card not present (%d)", ret);
}

/**
 * @brief Query and validate the SDHC host controller properties.
 *
 * Skips if the SDHC device is not ready (checked once in the before-each hook).
 * Retrieves host properties via
 * sdhc_get_host_props() and verifies that minimum/maximum clock frequencies
 * are non-zero, f_max >= f_min, and 4-bit bus width is supported.
 */
ZTEST(sd_card, test_sd_host_props)
{
	struct sdhc_host_props props;
	int ret;

	if (!sdhc_dev_ready) {
		ztest_test_skip();
		return;
	}

	ret = sdhc_get_host_props(sdhc_dev, &props);
	zassert_equal(ret, 0, "get_host_props failed (%d)", ret);
	zassert_true(props.f_max > 0, "f_max is 0");
	zassert_true(props.f_min > 0, "f_min is 0");
	zassert_true(props.f_max >= props.f_min, "f_max < f_min");
	zassert_true(props.host_caps.bus_4_bit_support,
		     "4-bit bus not supported");
	LOG_INF("SD host: f_min=%u Hz, f_max=%u Hz, 4bit=%d, 8bit=%d",
		props.f_min, props.f_max,
		props.host_caps.bus_4_bit_support,
		props.host_caps.bus_8_bit_support);
}

#endif /* CONFIG_SDHC && DT_HAS_NODELABEL(sdhc) */

/********************************
 *  E2E TESTS — Disk Access Level
 *******************************/

/**
 * @brief Initialize the SD disk via the disk access API.
 *
 * Exercises DISK_IOCTL_CTRL_INIT with retry logic and caches the total
 * sector count. Asserts that initialization succeeds within the retry limit.
 */
ZTEST(sd_card, test_sd_disk_init)
{
	zassert_true(sd_disk_inited, "SD disk not initialized");
}

/**
 * @brief Verify the SD disk reports DISK_STATUS_OK after initialization.
 *
 * Ensures the disk is initialized, then calls disk_access_status() and
 * asserts the returned status equals DISK_STATUS_OK.
 */
ZTEST(sd_card, test_sd_disk_status)
{
	int ret;

	ret = disk_access_status(SD_DISK_NAME);
	zassert_equal(ret, DISK_STATUS_OK, "Disk status not OK (%d)", ret);
}

/**
 * @brief Read and validate the SD card's total sector count.
 *
 * Issues DISK_IOCTL_GET_SECTOR_COUNT and asserts the count is non-zero.
 * Logs the total capacity in MB for informational purposes.
 */
ZTEST(sd_card, test_sd_get_sector_count)
{
	uint32_t count = 0;
	int ret;

	ret = disk_access_ioctl(SD_DISK_NAME, DISK_IOCTL_GET_SECTOR_COUNT,
				&count);
	zassert_equal(ret, 0, "GET_SECTOR_COUNT failed (%d)", ret);
	zassert_true(count > 0, "Sector count is 0");
	LOG_INF("SD sector count: %u (%u MB)", count, count / 2048);
}

/**
 * @brief Confirm the SD card's sector size matches the expected 512 bytes.
 *
 * Issues DISK_IOCTL_GET_SECTOR_SIZE and asserts the returned size equals
 * the SECTOR_SIZE constant (512).
 */
ZTEST(sd_card, test_sd_get_sector_size)
{
	uint32_t size = 0;
	int ret;

	ret = disk_access_ioctl(SD_DISK_NAME, DISK_IOCTL_GET_SECTOR_SIZE,
				&size);
	zassert_equal(ret, 0, "GET_SECTOR_SIZE failed (%d)", ret);
	zassert_equal(size, (uint32_t)SECTOR_SIZE,
			"Unexpected sector size %u", size);
}

/**
 * @brief Retrieve and validate the SD card's erase block size.
 *
 * Issues DISK_IOCTL_GET_ERASE_BLOCK_SZ and asserts the returned value
 * is greater than zero. Logs the erase block size in sectors.
 */
ZTEST(sd_card, test_sd_get_erase_block_size)
{
	uint32_t erase_sz = 0;
	int ret;

	ret = disk_access_ioctl(SD_DISK_NAME, DISK_IOCTL_GET_ERASE_BLOCK_SZ,
				&erase_sz);
	zassert_equal(ret, 0, "GET_ERASE_BLOCK_SZ failed (%d)", ret);
	zassert_true(erase_sz > 0, "Erase block size is 0");
	LOG_INF("SD erase block size: %u sectors", erase_sz);
}

/**
 * @brief Write and read back a single raw sector to verify data integrity.
 *
 * Saves the original content of a test sector near the end of the disk,
 * writes a known byte pattern, reads it back and compares, then restores
 * the original data. Validates basic single-sector read/write functionality.
 */
ZTEST(sd_card, test_sd_raw_single_sector_rw)
{
	uint32_t sector;
	int ret;

	ret = sd_ensure_unmounted();
	zassert_equal(ret, 0, "Unmount failed (%d)", ret);
	sector = sd_raw_test_sector();

	/* Save original */
	ret = disk_access_read(SD_DISK_NAME, sector_save, sector, 1);
	zassert_equal(ret, 0, "Save read failed (%d)", ret);

	/* Write test pattern */
	for (int i = 0; i < SECTOR_SIZE; i++) {
		sector_wr[i] = (uint8_t)(i & 0xFF);
	}
	ret = disk_access_write(SD_DISK_NAME, sector_wr, sector, 1);
	zassert_equal(ret, 0, "Write failed (%d)", ret);

	/* Read back and verify */
	memset(sector_rd, 0, SECTOR_SIZE);
	ret = disk_access_read(SD_DISK_NAME, sector_rd, sector, 1);
	zassert_equal(ret, 0, "Read back failed (%d)", ret);
	zassert_mem_equal(sector_wr, sector_rd, SECTOR_SIZE,
			"Single sector data mismatch");

	/* Restore */
	ret = disk_access_write(SD_DISK_NAME, sector_save, sector, 1);
	zassert_equal(ret, 0, "Restore failed (%d)", ret);

	/* Remount for cleanup */
	ret = sd_ensure_mounted();
	zassert_equal(ret, 0, "Remount failed (%d)", ret);
}

/**
 * @brief Write and read back 4 contiguous raw sectors to verify multi-sector
 *        transfer integrity.
 *
 * Saves 4 sectors near the end of the disk, writes a computed pattern across
 * all 4 sectors in a single operation, reads them back and compares, then
 * restores the originals. Tests the driver's multi-block transfer path.
 */
ZTEST(sd_card, test_sd_raw_multi_sector_rw)
{
	uint32_t sector;
	int ret;

	ret = sd_ensure_unmounted();
	zassert_equal(ret, 0, "Unmount failed (%d)", ret);
	zassert_true(sd_sector_count > RAW_TEST_SECTOR_OFFSET + 4,
			"Disk too small for multi-sector test");
	sector = sd_raw_test_sector();

	/* Save original 4 sectors */
	ret = disk_access_read(SD_DISK_NAME, multi_save, sector, 4);
	zassert_equal(ret, 0, "Multi save read failed (%d)", ret);

	/* Write pattern */
	for (int i = 0; i < (int)sizeof(multi_wr); i++) {
		multi_wr[i] = (uint8_t)((i * 7 + 3) & 0xFF);
	}
	ret = disk_access_write(SD_DISK_NAME, multi_wr, sector, 4);
	zassert_equal(ret, 0, "Multi write failed (%d)", ret);

	/* Read back and verify */
	memset(multi_rd, 0, sizeof(multi_rd));
	ret = disk_access_read(SD_DISK_NAME, multi_rd, sector, 4);
	zassert_equal(ret, 0, "Multi read failed (%d)", ret);
	zassert_mem_equal(multi_wr, multi_rd, sizeof(multi_wr),
			"Multi-sector data mismatch");

	/* Restore */
	ret = disk_access_write(SD_DISK_NAME, multi_save, sector, 4);
	zassert_equal(ret, 0, "Multi restore failed (%d)", ret);

	/* Remount for cleanup */
	ret = sd_ensure_mounted();
	zassert_equal(ret, 0, "Remount failed (%d)", ret);
}

/**
 * @brief Flush any pending writes by issuing a DISK_IOCTL_CTRL_SYNC.
 *
 * Ensures the disk is initialized, then invokes CTRL_SYNC and asserts it
 * completes successfully. Verifies the driver supports explicit cache flush.
 */
ZTEST(sd_card, test_sd_disk_sync)
{
	int ret;

	ret = disk_access_ioctl(SD_DISK_NAME, DISK_IOCTL_CTRL_SYNC, NULL);
	zassert_equal(ret, 0, "CTRL_SYNC failed (%d)", ret);
}

/**
 * @brief De-initialize and re-initialize the SD disk to verify lifecycle
 *        management.
 *
 * Performs a forced de-init via DISK_IOCTL_CTRL_DEINIT, confirms the disk
 * transitions to DISK_STATUS_UNINIT, then re-initializes and verifies it
 * returns to DISK_STATUS_OK. Unmounts the filesystem first to avoid
 * conflicts.
 */
ZTEST(sd_card, test_sd_disk_deinit_reinit)
{
	bool force = true;
	int ret;

	ret = sd_ensure_unmounted();
	zassert_equal(ret, 0, "Unmount failed (%d)", ret);

	/* Forced deinit (ignores refcount from double_init etc.) */
	ret = disk_access_ioctl(SD_DISK_NAME, DISK_IOCTL_CTRL_DEINIT, &force);
	zassert_equal(ret, 0, "CTRL_DEINIT failed (%d)", ret);
	sd_disk_inited = false;

	/* Verify uninit state */
	ret = disk_access_status(SD_DISK_NAME);
	zassert_equal(ret, DISK_STATUS_UNINIT,
			"Not UNINIT after deinit (%d)", ret);

	/* Re-init */
	ret = sd_ensure_disk_ready();
	zassert_equal(ret, 0, "Re-init failed (%d)", ret);
	ret = disk_access_status(SD_DISK_NAME);
	zassert_equal(ret, DISK_STATUS_OK, "Not OK after re-init (%d)", ret);

	/* Remount for cleanup */
	ret = sd_ensure_mounted();
	zassert_equal(ret, 0, "Remount failed (%d)", ret);
}

/*****************************
 *E2E TESTS — Filesystem Level
 *****************************/

/**
 * @brief Mount the FAT filesystem on the SD card.
 *
 * Initializes the disk if needed and mounts the FATFS partition at /SD:.
 * Includes a recovery path that re-initializes the disk on mount failure.
 * Asserts that the mount operation succeeds.
 */
ZTEST(sd_card, test_sd_mount)
{
	zassert_true(sd_fs_mounted, "SD filesystem not mounted");
}

/**
 * @brief Create a file on the SD card and write known content to it.
 *
 * Opens SD_TEST_FILE with CREATE|WRITE|TRUNC flags, writes TEST_CONTENT,
 * and asserts the number of bytes written matches the content length.
 * Verifies basic file creation and write functionality.
 */
ZTEST(sd_card, test_sd_file_write)
{
	struct fs_file_t file;
	ssize_t written;
	int ret;

	fs_file_t_init(&file);

	ret = fs_open(&file, SD_TEST_FILE,
			FS_O_CREATE | FS_O_WRITE | FS_O_TRUNC);
	zassert_equal(ret, 0, "Create/open failed (%d)", ret);

	written = fs_write(&file, TEST_CONTENT, strlen(TEST_CONTENT));
	zassert_equal(written, (ssize_t)strlen(TEST_CONTENT),
			"Write failed (%d)", (int)written);
	fs_close(&file);
}

/**
 * @brief Write to a file, then read back and verify the content.
 *
 * Creates SD_TEST_FILE with TEST_CONTENT, reopens it for reading, reads the
 * data into a buffer, and asserts the content matches. Validates the basic
 * file write and read path with data integrity verification.
 */
ZTEST(sd_card, test_sd_file_read_verify)
{
	struct fs_file_t file;
	char buf[64] = {0};
	ssize_t len;
	int ret;

	/* Write test content */
	fs_file_t_init(&file);
	ret = fs_open(&file, SD_TEST_FILE,
			FS_O_CREATE | FS_O_WRITE | FS_O_TRUNC);
	zassert_equal(ret, 0, "Create for write failed (%d)", ret);
	ret = fs_write(&file, TEST_CONTENT, strlen(TEST_CONTENT));
	zassert_equal(ret, (int)strlen(TEST_CONTENT),
			"Write failed (%d)", ret);
	fs_close(&file);

	/* Read back and verify */
	fs_file_t_init(&file);
	ret = fs_open(&file, SD_TEST_FILE, FS_O_READ);
	zassert_equal(ret, 0, "Open for read failed (%d)", ret);

	len = fs_read(&file, buf, sizeof(buf) - 1);
	zassert_true(len >= 0, "Read failed (%d)", (int)len);
	buf[len] = '\0';

	zassert_true(strncmp(buf, TEST_CONTENT, strlen(TEST_CONTENT)) == 0,
			"Content mismatch");
	fs_close(&file);
}

/**
 * @brief Verify that appending to an existing file preserves original content.
 *
 * Writes base content to SD_TEST_FILE, reopens it in append mode and writes
 * additional data, then reads back the entire file. Asserts that both the
 * original and appended content are present in the correct order.
 */
ZTEST(sd_card, test_sd_file_append_verify)
{
	struct fs_file_t file;
	char buf[128] = {0};
	ssize_t len;
	int ret;

	/* Write base content */
	fs_file_t_init(&file);
	ret = fs_open(&file, SD_TEST_FILE,
			FS_O_CREATE | FS_O_WRITE | FS_O_TRUNC);
	zassert_equal(ret, 0, "Base write open failed (%d)", ret);
	ret = fs_write(&file, TEST_CONTENT, strlen(TEST_CONTENT));
	zassert_equal(ret, (int)strlen(TEST_CONTENT),
			"Base write failed (%d)", ret);
	fs_close(&file);

	/* Append */
	fs_file_t_init(&file);
	ret = fs_open(&file, SD_TEST_FILE,
			FS_O_CREATE | FS_O_WRITE | FS_O_APPEND);
	zassert_equal(ret, 0, "Append open failed (%d)", ret);
	ret = fs_write(&file, APPEND_CONTENT, strlen(APPEND_CONTENT));
	zassert_equal(ret, (int)strlen(APPEND_CONTENT),
			"Append write failed (%d)", ret);
	fs_close(&file);

	/* Verify both parts present */
	fs_file_t_init(&file);
	ret = fs_open(&file, SD_TEST_FILE, FS_O_READ);
	zassert_equal(ret, 0, "Verify open failed (%d)", ret);
	len = fs_read(&file, buf, sizeof(buf) - 1);
	zassert_true(len >= 0, "Verify read failed (%d)", (int)len);
	buf[len] = '\0';
	zassert_not_null(strstr(buf, TEST_CONTENT), "Base content missing");
	zassert_not_null(strstr(buf, APPEND_CONTENT), "Appended content missing");
	fs_close(&file);
}

/**
 * @brief Create a file, delete it, and confirm it no longer exists.
 *
 * Creates a temporary file, writes data to it, then removes it with
 * fs_unlink(). Attempts to reopen the file for reading and asserts the
 * open fails, confirming the file was successfully deleted.
 */
ZTEST(sd_card, test_sd_file_delete)
{
	struct fs_file_t file;
	int ret;
	const char *path = "/SD:/del.txt";

	/* Create file */
	fs_file_t_init(&file);
	ret = fs_open(&file, path, FS_O_CREATE | FS_O_WRITE);
	zassert_equal(ret, 0, "Create failed (%d)", ret);
	ret = fs_write(&file, "delete_me", 9);
	zassert_equal(ret, 9, "Write failed (%d)", ret);
	fs_close(&file);

	/* Delete */
	ret = fs_unlink(path);
	zassert_equal(ret, 0, "Unlink failed (%d)", ret);

	/* Verify gone */
	fs_file_t_init(&file);
	ret = fs_open(&file, path, FS_O_READ);
	zassert_true(ret != 0, "Deleted file still exists");
}

/**
 * @brief Write two separate files and verify each retains its own content.
 *
 * Creates f1.txt with "FILE_ONE" and f2.txt with "FILE_TWO", then reads
 * each file back independently and asserts the content matches. Ensures
 * that writing one file does not corrupt another file's data.
 */
ZTEST(sd_card, test_sd_multifile_integrity)
{
	const char *f1 = "/SD:/f1.txt";
	const char *f2 = "/SD:/f2.txt";
	const char *c1 = "FILE_ONE";
	const char *c2 = "FILE_TWO";
	struct fs_file_t file;
	char buf[32] = {0};
	int ret;

	/* Write file 1 */
	fs_file_t_init(&file);
	ret = fs_open(&file, f1, FS_O_CREATE | FS_O_WRITE | FS_O_TRUNC);
	zassert_equal(ret, 0, "Open f1 failed (%d)", ret);
	ret = fs_write(&file, c1, strlen(c1));
	zassert_equal(ret, (int)strlen(c1), "Write f1 failed (%d)", ret);
	fs_close(&file);

	/* Write file 2 */
	fs_file_t_init(&file);
	ret = fs_open(&file, f2, FS_O_CREATE | FS_O_WRITE | FS_O_TRUNC);
	zassert_equal(ret, 0, "Open f2 failed (%d)", ret);
	ret = fs_write(&file, c2, strlen(c2));
	zassert_equal(ret, (int)strlen(c2), "Write f2 failed (%d)", ret);
	fs_close(&file);

	/* Verify f1 */
	memset(buf, 0, sizeof(buf));
	fs_file_t_init(&file);
	ret = fs_open(&file, f1, FS_O_READ);
	zassert_equal(ret, 0, "Read f1 failed (%d)", ret);
	ret = fs_read(&file, buf, sizeof(buf) - 1);
	zassert_true(ret >= 0, "Read f1 data failed (%d)", ret);
	buf[ret] = '\0';
	zassert_true(strcmp(buf, c1) == 0, "f1 data mismatch");
	fs_close(&file);

	/* Verify f2 */
	memset(buf, 0, sizeof(buf));
	fs_file_t_init(&file);
	ret = fs_open(&file, f2, FS_O_READ);
	zassert_equal(ret, 0, "Read f2 failed (%d)", ret);
	ret = fs_read(&file, buf, sizeof(buf) - 1);
	zassert_true(ret >= 0, "Read f2 data failed (%d)", ret);
	buf[ret] = '\0';
	zassert_true(strcmp(buf, c2) == 0, "f2 data mismatch");
	fs_close(&file);
}

/**
 * @brief Create a subdirectory, place a file inside it, and list its contents.
 *
 * Creates SD_DIR_PATH, writes a file (sub.txt) into it, then opens the
 * directory and iterates through its entries. Asserts that sub.txt appears
 * in the directory listing. Validates mkdir, nested file creation, and
 * directory enumeration.
 */
ZTEST(sd_card, test_sd_dir_create_list)
{
	struct fs_dir_t dir;
	struct fs_dirent entry;
	struct fs_file_t file;
	int ret;
	bool found = false;

	/* Create directory */
	ret = fs_mkdir(SD_DIR_PATH);
	zassert_true(ret == 0 || ret == -EEXIST,
			"mkdir failed (%d)", ret);

	/* Create file inside directory */
	fs_file_t_init(&file);
	ret = fs_open(&file, SD_DIR_FILE,
			FS_O_CREATE | FS_O_WRITE | FS_O_TRUNC);
	zassert_equal(ret, 0, "Create in dir failed (%d)", ret);
	ret = fs_write(&file, "sub", 3);
	zassert_equal(ret, 3, "Write in dir failed (%d)", ret);
	fs_close(&file);

	/* List directory contents */
	fs_dir_t_init(&dir);
	ret = fs_opendir(&dir, SD_DIR_PATH);
	zassert_equal(ret, 0, "opendir failed (%d)", ret);

	while (fs_readdir(&dir, &entry) == 0 && entry.name[0] != '\0') {
		if (strcmp(entry.name, "sub.txt") == 0 ||
				strcmp(entry.name, "SUB.TXT") == 0) {
			found = true;
		}
	}
	fs_closedir(&dir);
	zassert_true(found, "File not found in directory listing");
}

/**
 * @brief Verify that file data persists across an unmount/remount cycle.
 *
 * Writes known content to SD_TEST_FILE, unmounts the filesystem, remounts
 * it, and reads the file back. Asserts the content survived the remount,
 * confirming data is flushed to the SD card before unmount.
 */
ZTEST(sd_card, test_sd_remount_persistence)
{
	struct fs_file_t file;
	char buf[64] = {0};
	int ret;

	/* Write known data */
	fs_file_t_init(&file);
	ret = fs_open(&file, SD_TEST_FILE,
			FS_O_CREATE | FS_O_WRITE | FS_O_TRUNC);
	zassert_equal(ret, 0, "Pre-write failed (%d)", ret);
	ret = fs_write(&file, TEST_CONTENT, strlen(TEST_CONTENT));
	zassert_equal(ret, (int)strlen(TEST_CONTENT),
			"Pre-write content failed (%d)", ret);
	fs_close(&file);

	/* Unmount and remount */
	ret = sd_ensure_unmounted();
	zassert_equal(ret, 0, "Unmount failed (%d)", ret);
	ret = sd_ensure_mounted();
	zassert_equal(ret, 0, "Remount failed (%d)", ret);

	/* Verify data persisted */
	fs_file_t_init(&file);
	ret = fs_open(&file, SD_TEST_FILE, FS_O_READ);
	zassert_equal(ret, 0, "Open after remount failed (%d)", ret);
	ret = fs_read(&file, buf, sizeof(buf) - 1);
	zassert_true(ret > 0, "Read after remount failed (%d)", ret);
	buf[ret] = '\0';
	zassert_not_null(strstr(buf, TEST_CONTENT), "Data not persisted");
	fs_close(&file);
}

/******************
 * BOUNDARY TESTS
 *****************/

/**
 * @brief Write and read back a single-byte file to test minimum-size I/O.
 *
 * Creates a file containing exactly one byte ('X'), then reads it back
 * and asserts both the size (1 byte) and content match. Tests the edge
 * case of the smallest possible file write.
 */
ZTEST(sd_card, test_sd_write_single_byte)
{
	struct fs_file_t file;
	char buf[4] = {0};
	int ret;
	const char *path = "/SD:/1byte.txt";

	fs_file_t_init(&file);
	ret = fs_open(&file, path, FS_O_CREATE | FS_O_WRITE | FS_O_TRUNC);
	zassert_equal(ret, 0, "Open failed (%d)", ret);
	ret = fs_write(&file, "X", 1);
	zassert_equal(ret, 1, "Write 1 byte failed (%d)", ret);
	fs_close(&file);

	fs_file_t_init(&file);
	ret = fs_open(&file, path, FS_O_READ);
	zassert_equal(ret, 0, "Read open failed (%d)", ret);
	ret = fs_read(&file, buf, sizeof(buf));
	zassert_equal(ret, 1, "Read back size wrong (%d)", ret);
	zassert_equal(buf[0], 'X', "Read back content wrong");
	fs_close(&file);
}

/**
 * @brief Create an empty file and verify that reading it returns zero bytes.
 *
 * Opens a file with CREATE|WRITE|TRUNC but writes nothing, then reopens
 * it for reading. Asserts that fs_read() returns 0, confirming correct
 * handling of zero-length files.
 */
ZTEST(sd_card, test_sd_read_empty_file)
{
	struct fs_file_t file;
	char buf[16];
	int ret;
	const char *path = "/SD:/empty.txt";

	/* Create empty file */
	fs_file_t_init(&file);
	ret = fs_open(&file, path, FS_O_CREATE | FS_O_WRITE | FS_O_TRUNC);
	zassert_equal(ret, 0, "Create empty failed (%d)", ret);
	fs_close(&file);

	/* Read should return 0 bytes */
	fs_file_t_init(&file);
	ret = fs_open(&file, path, FS_O_READ);
	zassert_equal(ret, 0, "Open empty failed (%d)", ret);
	ret = fs_read(&file, buf, sizeof(buf));
	zassert_equal(ret, 0, "Empty file read non-zero (%d)", ret);
	fs_close(&file);
}

/**
 * @brief Seek past the end of a file and verify that reading returns no data.
 *
 * Creates a 5-byte file, seeks to offset 1000 (well beyond EOF), and
 * attempts a read. If the seek succeeds, asserts the read returns 0 bytes.
 * Validates graceful handling of out-of-bounds seek positions.
 */
ZTEST(sd_card, test_sd_seek_beyond_eof)
{
	struct fs_file_t file;
	char buf[16];
	int ret;

	/* Create small file */
	fs_file_t_init(&file);
	ret = fs_open(&file, SD_TEST_FILE,
		      FS_O_CREATE | FS_O_WRITE | FS_O_TRUNC);
	zassert_equal(ret, 0, "Create failed (%d)", ret);
	ret = fs_write(&file, "short", 5);
	zassert_equal(ret, 5, "Write failed (%d)", ret);
	fs_close(&file);

	/* Open for read, seek beyond end */
	fs_file_t_init(&file);
	ret = fs_open(&file, SD_TEST_FILE, FS_O_READ);
	zassert_equal(ret, 0, "Open read failed (%d)", ret);
	ret = fs_seek(&file, 1000, FS_SEEK_SET);
	/* Seek beyond EOF may succeed; subsequent read should return 0 */
	if (ret == 0) {
		ret = fs_read(&file, buf, sizeof(buf));
		zassert_equal(ret, 0,
				"Read beyond EOF returned data (%d)", ret);
	}
	fs_close(&file);
}

/**
 * @brief Attempt to open a file that does not exist and verify failure.
 *
 * Tries to open a non-existent path for reading without the CREATE flag.
 * Asserts that fs_open() returns a non-zero error code, confirming the
 * filesystem correctly rejects access to missing files.
 */
ZTEST(sd_card, test_sd_open_nonexistent)
{
	struct fs_file_t file;
	int ret;

	fs_file_t_init(&file);
	ret = fs_open(&file, "/SD:/no_such_file.txt", FS_O_READ);
	zassert_true(ret != 0, "Non-existent file opened successfully");
}

/**
 * @brief Mount an already-mounted filesystem and verify it does not crash.
 *
 * With the filesystem already mounted, calls fs_mount() again and asserts
 * the return is -EBUSY, -EALREADY, or 0. Ensures the driver handles
 * redundant mount requests gracefully without data corruption.
 */
ZTEST(sd_card, test_sd_double_mount)
{
	int ret;
	/* Second mount should return -EBUSY or similar, not crash */
	ret = fs_mount(&sd_mp);
	zassert_true(ret == -EBUSY || ret == -EALREADY || ret == 0,
			"Double mount unexpected error (%d)", ret);
}

/**
 * @brief Initialize the SD disk a second time and verify it succeeds.
 *
 * With the disk already initialized, issues another DISK_IOCTL_CTRL_INIT.
 * Asserts success (return 0), confirming the driver's reference-counted
 * initialization handles duplicate init calls correctly.
 */
ZTEST(sd_card, test_sd_double_init)
{
	int ret;
	/* Double init should succeed (reference counted) */
	ret = disk_access_ioctl(SD_DISK_NAME, DISK_IOCTL_CTRL_INIT, NULL);
	zassert_equal(ret, 0, "Double init failed (%d)", ret);
}

/**
 * @brief Destructive test: write and verify raw sector 0 (FAT boot sector).
 *
 * Gated behind CONFIG_SDMMC_TEST_DESTRUCTIVE_SECTOR_ZERO (default off).
 * Saves sector 0, overwrites it with a test pattern, reads back and
 * verifies, then restores the original. WARNING: if the test aborts
 * mid-way, the SD card's FAT boot sector will be corrupted and the card
 * will require reformatting.
 */
ZTEST(sd_card, test_sd_raw_sector_zero)
{
	int ret;

	if (!IS_ENABLED(CONFIG_SDMMC_TEST_DESTRUCTIVE_SECTOR_ZERO)) {
		ztest_test_skip();
		return;
	}

	ret = sd_ensure_unmounted();
	zassert_equal(ret, 0, "Unmount failed (%d)", ret);

	/* Save sector 0 */
	ret = disk_access_read(SD_DISK_NAME, sector_save, 0, 1);
	zassert_equal(ret, 0, "Read sector 0 failed (%d)", ret);

	/* Write pattern */
	memset(sector_wr, 0xA5, SECTOR_SIZE);
	ret = disk_access_write(SD_DISK_NAME, sector_wr, 0, 1);
	zassert_equal(ret, 0, "Write sector 0 failed (%d)", ret);

	/* Verify */
	memset(sector_rd, 0, SECTOR_SIZE);
	ret = disk_access_read(SD_DISK_NAME, sector_rd, 0, 1);
	zassert_equal(ret, 0, "Re-read sector 0 failed (%d)", ret);
	zassert_mem_equal(sector_wr, sector_rd, SECTOR_SIZE,
			"Sector 0 data mismatch");

	/* Restore original (critical for FAT boot sector) */
	ret = disk_access_write(SD_DISK_NAME, sector_save, 0, 1);
	zassert_equal(ret, 0, "Restore sector 0 failed (%d)", ret);

	/* Remount for cleanup */
	ret = sd_ensure_mounted();
	zassert_equal(ret, 0, "Remount failed (%d)", ret);
}

/**
 * @brief Write and verify the last sector on the SD card.
 *
 * Targets the highest valid sector address to test boundary addressing.
 * Saves the original content, writes a 0x5A fill pattern, reads back
 * and compares, then restores. Ensures the driver correctly handles
 * I/O at the maximum LBA.
 */
ZTEST(sd_card, test_sd_raw_last_sector)
{
	uint32_t last;
	int ret;

	ret = sd_ensure_unmounted();
	zassert_equal(ret, 0, "Unmount failed (%d)", ret);
	zassert_true(sd_sector_count > 0, "No sectors available");
	last = sd_sector_count - 1;

	/* Save */
	ret = disk_access_read(SD_DISK_NAME, sector_save, last, 1);
	zassert_equal(ret, 0, "Read last sector failed (%d)", ret);

	/* Write */
	memset(sector_wr, 0x5A, SECTOR_SIZE);
	ret = disk_access_write(SD_DISK_NAME, sector_wr, last, 1);
	zassert_equal(ret, 0, "Write last sector failed (%d)", ret);

	/* Verify */
	memset(sector_rd, 0, SECTOR_SIZE);
	ret = disk_access_read(SD_DISK_NAME, sector_rd, last, 1);
	zassert_equal(ret, 0, "Re-read last sector failed (%d)", ret);
	zassert_mem_equal(sector_wr, sector_rd, SECTOR_SIZE,
			"Last sector data mismatch");

	/* Restore */
	ret = disk_access_write(SD_DISK_NAME, sector_save, last, 1);
	zassert_equal(ret, 0, "Restore last sector failed (%d)", ret);

	/* Remount for cleanup */
	ret = sd_ensure_mounted();
	zassert_equal(ret, 0, "Remount failed (%d)", ret);
}

/**
 * @brief Write and read back a 2 KB file to verify large-buffer I/O.
 *
 * Fills a LARGE_BUF_SIZE (2048) byte buffer with a repeating pattern,
 * writes it to SD_LARGE_FILE, then reads it back using a loop to handle
 * potential partial reads. Asserts the total bytes read match and the
 * data is identical. Tests multi-cluster file I/O.
 */
ZTEST(sd_card, test_sd_large_file_write)
{
	struct fs_file_t file;
	ssize_t len;
	int ret;

	/* Fill write buffer with pattern */
	for (int i = 0; i < LARGE_BUF_SIZE; i++) {
		large_wr[i] = (uint8_t)(i & 0xFF);
	}

	/* Write */
	fs_file_t_init(&file);
	ret = fs_open(&file, SD_LARGE_FILE,
			FS_O_CREATE | FS_O_WRITE | FS_O_TRUNC);
	zassert_equal(ret, 0, "Large open failed (%d)", ret);
	len = fs_write(&file, large_wr, LARGE_BUF_SIZE);
	zassert_equal(len, LARGE_BUF_SIZE,
			"Large write failed (%d)", (int)len);
	fs_close(&file);

	/* Read back and verify */
	memset(large_rd, 0, LARGE_BUF_SIZE);
	fs_file_t_init(&file);
	ret = fs_open(&file, SD_LARGE_FILE, FS_O_READ);
	zassert_equal(ret, 0, "Large read open failed (%d)", ret);

	ssize_t total_read = 0;

	while (total_read < LARGE_BUF_SIZE) {
		len = fs_read(&file, large_rd + total_read,
			      LARGE_BUF_SIZE - total_read);
		zassert_true(len >= 0, "Large read failed (%d)", (int)len);
		if (len == 0) {
			break;
		}
		total_read += len;
	}
	zassert_equal(total_read, LARGE_BUF_SIZE,
			"Large read size wrong (%d)", (int)total_read);
	zassert_mem_equal(large_wr, large_rd, LARGE_BUF_SIZE,
			"Large file data mismatch");
	fs_close(&file);
}

/******************
 **STRESS TESTS
 *****************/

/**
 * @brief Repeatedly write and read back unique strings to stress filesystem I/O.
 *
 * Runs STRESS_ITERATIONS (25) cycles of: write a unique iteration-tagged
 * string to SD_STRESS_FILE, read it back, and compare. Detects data
 * corruption, file handle leaks, or driver instability under sustained
 * sequential file operations.
 */
ZTEST(sd_card, test_sd_stress_file_write_read)
{
	struct fs_file_t file;
	char wbuf[64];
	char rbuf[64];
	int ret;

	for (int i = 0; i < STRESS_ITERATIONS; i++) {
		snprintk(wbuf, sizeof(wbuf), "stress-iter-%d", i);
		memset(rbuf, 0, sizeof(rbuf));

		fs_file_t_init(&file);
		ret = fs_open(&file, SD_STRESS_FILE,
				FS_O_CREATE | FS_O_WRITE | FS_O_TRUNC);
		zassert_equal(ret, 0,
				"Stress write open iter %d (%d)", i, ret);
		ret = fs_write(&file, wbuf, strlen(wbuf));
		zassert_equal(ret, (int)strlen(wbuf),
				"Stress write iter %d (%d)", i, ret);
		fs_close(&file);

		fs_file_t_init(&file);
		ret = fs_open(&file, SD_STRESS_FILE, FS_O_READ);
		zassert_equal(ret, 0,
				"Stress read open iter %d (%d)", i, ret);
		ret = fs_read(&file, rbuf, sizeof(rbuf) - 1);
		zassert_true(ret >= 0,
				"Stress read iter %d (%d)", i, ret);
		rbuf[ret] = '\0';
		zassert_true(strcmp(wbuf, rbuf) == 0,
				"Stress mismatch iter %d", i);
		fs_close(&file);
	}
}

/**
 * @brief Repeatedly mount and unmount the filesystem to stress the mount path.
 *
 * Cycles through MOUNT_STRESS_ITER (10) mount/unmount pairs and verifies
 * each succeeds. Detects resource leaks or state corruption in the
 * filesystem mount/unmount logic. Leaves the filesystem mounted on exit.
 */
ZTEST(sd_card, test_sd_stress_mount_unmount)
{
	int ret;

	for (int i = 0; i < MOUNT_STRESS_ITER; i++) {
		ret = sd_ensure_unmounted();
		zassert_equal(ret, 0, "Unmount iter %d failed (%d)", i, ret);
		ret = sd_ensure_mounted();
		zassert_equal(ret, 0, "Mount iter %d failed (%d)", i, ret);
	}
}

/**
 * @brief Simulate a health-monitoring workload: append data and verify tail reads.
 *
 * Over HEALTH_MONITOR_ITER (50) iterations, appends a unique line to
 * SD_HEALTH_FILE, then seeks to the end minus the line length and reads
 * back just the appended portion. Asserts each read matches the written
 * line. Validates append-mode writes, seek-from-end, and incremental
 * file growth under sustained I/O.
 */
ZTEST(sd_card, test_sd_stress_health_monitor)
{
	struct fs_file_t file;
	char expected[64];
	char actual[64];
	ssize_t read_len;
	int ret;

	/* Pre-create/truncate file */
	fs_file_t_init(&file);
	ret = fs_open(&file, SD_HEALTH_FILE,
			FS_O_CREATE | FS_O_WRITE | FS_O_TRUNC);
	zassert_equal(ret, 0, "Health pre-create failed (%d)", ret);
	fs_close(&file);

	for (int i = 0; i < HEALTH_MONITOR_ITER; i++) {
		snprintk(expected, sizeof(expected), "health-iter-%d\n", i);
		memset(actual, 0, sizeof(actual));

		/* Append */
		fs_file_t_init(&file);
		ret = fs_open(&file, SD_HEALTH_FILE,
				FS_O_WRITE | FS_O_APPEND);
		zassert_equal(ret, 0,
				"Health append open iter %d (%d)", i, ret);
		ret = fs_write(&file, expected, strlen(expected));
		zassert_equal(ret, (int)strlen(expected),
				"Health write iter %d (%d)", i, ret);
		fs_close(&file);

		/* Seek-from-end and read */
		fs_file_t_init(&file);
		ret = fs_open(&file, SD_HEALTH_FILE, FS_O_READ);
		zassert_equal(ret, 0,
				"Health read open iter %d (%d)", i, ret);
		ret = fs_seek(&file, -(off_t)strlen(expected), FS_SEEK_END);
		zassert_equal(ret, 0,
				"Health seek iter %d (%d)", i, ret);
		read_len = fs_read(&file, actual, sizeof(actual) - 1);
		zassert_equal(read_len, (ssize_t)strlen(expected),
				"Health read size iter %d", i);
		actual[read_len] = '\0';
		zassert_true(strcmp(expected, actual) == 0,
				"Health mismatch iter %d", i);
		fs_close(&file);
	}
}

/**
 * @brief Stress-test raw single-sector write/read with varying data patterns.
 *
 * Runs RAW_STRESS_ITER (100) iterations of: fill a sector with a pattern
 * derived from the iteration index, write it to a safe test sector near
 * the end of the disk, read it back, and compare. Restores the original
 * sector content on completion. Detects intermittent transfer errors or
 * driver wear under heavy raw I/O.
 */
ZTEST(sd_card, test_sd_stress_raw_sector_rw)
{
	uint32_t sector;
	int ret;

	ret = sd_ensure_unmounted();
	zassert_equal(ret, 0, "Unmount failed (%d)", ret);
	sector = sd_raw_test_sector();

	/* Save original */
	ret = disk_access_read(SD_DISK_NAME, sector_save, sector, 1);
	zassert_equal(ret, 0, "Raw stress save failed (%d)", ret);

	for (int i = 0; i < RAW_STRESS_ITER; i++) {
		memset(sector_wr, (uint8_t)(i & 0xFF), SECTOR_SIZE);

		ret = disk_access_write(SD_DISK_NAME, sector_wr, sector, 1);
		zassert_equal(ret, 0,
				"Raw stress write iter %d (%d)", i, ret);

		memset(sector_rd, 0, SECTOR_SIZE);
		ret = disk_access_read(SD_DISK_NAME, sector_rd, sector, 1);
		zassert_equal(ret, 0,
				"Raw stress read iter %d (%d)", i, ret);
		zassert_mem_equal(sector_wr, sector_rd, SECTOR_SIZE,
				"Raw stress mismatch iter %d", i);
	}

	/* Restore */
	ret = disk_access_write(SD_DISK_NAME, sector_save, sector, 1);
	zassert_equal(ret, 0, "Raw stress restore failed (%d)", ret);

	/* Remount for cleanup */
	ret = sd_ensure_mounted();
	zassert_equal(ret, 0, "Remount failed (%d)", ret);
}

/**
 * @brief Rapidly create and delete temporary files to stress FAT metadata updates.
 *
 * Runs FILE_CREATE_DEL_ITER (20) cycles of: create a uniquely named file,
 * write a small payload, close it, then delete it. Asserts every create,
 * write, and unlink succeeds. Targets the FAT directory entry and cluster
 * allocation/free paths under rapid churn.
 */
ZTEST(sd_card, test_sd_stress_file_create_delete)
{
	struct fs_file_t file;
	char path[32];
	int ret;

	for (int i = 0; i < FILE_CREATE_DEL_ITER; i++) {
		snprintk(path, sizeof(path), "/SD:/tmp%02d.txt", i);

		fs_file_t_init(&file);
		ret = fs_open(&file, path, FS_O_CREATE | FS_O_WRITE);
		zassert_equal(ret, 0,
				"Create iter %d (%d)", i, ret);
		ret = fs_write(&file, "tmp", 3);
		zassert_equal(ret, 3,
				"Write iter %d (%d)", i, ret);
		fs_close(&file);

		ret = fs_unlink(path);
		zassert_equal(ret, 0,
				"Delete iter %d (%d)", i, ret);
	}
}

/**
 * @brief Stress the disk de-init/re-init lifecycle across multiple iterations.
 *
 * Unmounts the filesystem first, then runs DEINIT_REINIT_ITER (5) cycles
 * of: initialize the disk, force a de-init, and verify the status
 * transitions to DISK_STATUS_UNINIT. After all cycles, re-initializes
 * the disk and remounts the filesystem. Validates the driver's ability
 * to cleanly tear down and re-establish the SD card connection.
 */
ZTEST(sd_card, test_sd_stress_deinit_reinit)
{
	int ret;

	ret = sd_ensure_unmounted();
	zassert_equal(ret, 0, "Unmount failed (%d)", ret);

	for (int i = 0; i < DEINIT_REINIT_ITER; i++) {
		bool force = true;

		ret = sd_ensure_disk_ready();
		zassert_equal(ret, 0, "Init iter %d failed (%d)", i, ret);

		ret = disk_access_ioctl(SD_DISK_NAME,
					DISK_IOCTL_CTRL_DEINIT, &force);
		zassert_equal(ret, 0, "Deinit iter %d (%d)", i, ret);
		sd_disk_inited = false;

		ret = disk_access_status(SD_DISK_NAME);
		zassert_equal(ret, DISK_STATUS_UNINIT,
				"Not UNINIT iter %d (%d)", i, ret);
	}

	/* Leave disk ready for any subsequent tests */
	ret = sd_ensure_disk_ready();
	zassert_equal(ret, 0, "Final re-init failed (%d)", ret);

	/* Remount for cleanup */
	ret = sd_ensure_mounted();
	zassert_equal(ret, 0, "Remount failed (%d)", ret);
}

ZTEST_SUITE(sd_card, NULL, sd_suite_setup, sd_test_before, sd_test_after, sd_suite_teardown);

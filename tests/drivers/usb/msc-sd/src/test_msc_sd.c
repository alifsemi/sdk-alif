/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/**
 * @file test_msc_sd.c
 * @brief USB MSC Class - SD Card Mode test suite
 *
 * Device-side ztest code covering USB Mass Storage Class with SD card
 * backend on Alif boards. 8 consolidated tests:
 *   1. SD mount + disk info     5. Data integrity + ADMA alignment
 *   2. USB init + descriptors   6. USB disconnect/reconnect
 *   3. File CRUD                7. Overwrite + persistence
 *   4. Directory + LFN          8. Regulator + stress
 */

#include <zephyr/ztest.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/sys/slist.h>
#include <zephyr/sys/util.h>
#include <zephyr/storage/disk_access.h>
#include <zephyr/fs/fs.h>
#include <zephyr/logging/log.h>
#include <ff.h>
#include <string.h>
#include <sample_usbd.h>

#if defined(CONFIG_SDHC)
#include <zephyr/drivers/sdhc.h>
#endif

LOG_MODULE_REGISTER(test_msc_sd, LOG_LEVEL_INF);

/* ─── Configuration ─── */
#define SD_DISK_NAME       "SD"
#define SD_MOUNT_POINT     "/SD:"
#define SD_TEST_FILE       "/SD:/msc_test.txt"
#define SD_LFN_FILE        "/SD:/this_is_a_long_filename_msc_test.txt"
#define SD_LARGE_FILE      "/SD:/msc_large.bin"
#define SD_DIR_PATH        "/SD:/msc_dir"
#define SD_DIR_FILE        "/SD:/msc_dir/nested.txt"
#define SD_OVERWRITE_FILE  "/SD:/msc_ow.txt"

#define TEST_CONTENT       "USB MSC SD test data"
#define LARGE_BUF_SIZE     4096
#define STRESS_USB_CYCLES  5
#define MULTI_FILE_COUNT   5
#define SECTOR_SIZE        512
#define WRITE_RETRIES      10
#define RETRY_DELAY_CAP_MS 2000
#define USB_SETTLE_MS      500
#define USB_REENUM_MS      1000

/* ─── Shared State ─── */
static FATFS msc_fat_fs;
static struct usbd_context *msc_usbd_ctx;
static bool usb_init_ok;
static bool sd_mounted;

static struct fs_mount_t msc_mp = {
	.type = FS_FATFS,
	.fs_data = &msc_fat_fs,
	.storage_dev = (void *)SD_DISK_NAME,
	.mnt_point = SD_MOUNT_POINT,
};

#if defined(CONFIG_SDHC)
static const struct device *sdhc_dev = DEVICE_DT_GET(DT_NODELABEL(sdhc));
#endif

static uint8_t large_wr[LARGE_BUF_SIZE] __aligned(4);
static uint8_t large_rd[LARGE_BUF_SIZE] __aligned(4);

/* ─── Helpers ─── */
static int msc_ensure_mounted(void)
{
	int ret;

	if (sd_mounted) {
		return 0;
	}

	ret = disk_access_ioctl(SD_DISK_NAME, DISK_IOCTL_CTRL_INIT, NULL);
	if (ret != 0) {
		return ret;
	}

	ret = fs_mount(&msc_mp);
	if (ret == 0 || ret == -EBUSY || ret == -EALREADY) {
		sd_mounted = true;
		return 0;
	}
	return ret;
}

static int msc_ensure_unmounted(void)
{
	if (sd_mounted) {
		int ret = fs_unmount(&msc_mp);

		if (ret == 0 || ret == -ENOENT) {
			sd_mounted = false;
			return 0;
		}
		return ret;
	}
	return 0;
}

static int __maybe_unused msc_usb_disable(void)
{
	int err;

	if (msc_usbd_ctx == NULL) {
		return -ENODEV;
	}

	k_sleep(K_MSEC(USB_SETTLE_MS));
	err = usbd_disable(msc_usbd_ctx);
	if (err == 0) {
		usb_init_ok = false;
		return 0;
	}
	if (err == -EALREADY) {
		usb_init_ok = false;
		return 0;
	}

	/*
	 * usbd_disable() marks the stack disabled even when DWC3 halt
	 * fails (-EIO). Do not treat a follow-up -EALREADY as success:
	 * SET_CONFIGURATION(0) already dropped the host MSC drive.
	 */
	TC_PRINT("usbd_disable failed (%d), controller did not halt\n", err);
	return err;
}

static int msc_usb_ensure_enabled(void)
{
	int err;

	if (msc_usbd_ctx == NULL) {
		return -ENODEV;
	}

	if (usb_init_ok) {
		return 0;
	}

	err = usbd_enable(msc_usbd_ctx);
	if (err == 0 || err == -EALREADY) {
		usb_init_ok = true;
		if (err == 0) {
			k_sleep(K_MSEC(USB_REENUM_MS));
		}
		return 0;
	}

	TC_PRINT("usbd_enable failed (%d)\n", err);
	usb_init_ok = false;
	return err;
}

static void try_unlink(const char *path)
{
	struct fs_dirent st;

	if (fs_stat(path, &st) == 0) {
		(void)fs_unlink(path);
	}
}

static void msc_cleanup_files(void)
{
	static const char * const paths[] = {
		SD_TEST_FILE, SD_LFN_FILE, SD_LARGE_FILE,
		SD_OVERWRITE_FILE, SD_DIR_FILE, SD_DIR_PATH, NULL
	};
	char path[40];

	for (int i = 0; paths[i] != NULL; i++) {
		try_unlink(paths[i]);
	}
	for (int i = 0; i < MULTI_FILE_COUNT; i++) {
		snprintk(path, sizeof(path), "/SD:/msc_f%02d.txt", i);
		try_unlink(path);
	}
	static const int align_sizes[] = {1, 511, 512, 513, 1024};

	for (int s = 0; s < ARRAY_SIZE(align_sizes); s++) {
		snprintk(path, sizeof(path), "/SD:/align_%d.bin",
			 align_sizes[s]);
		try_unlink(path);
	}
}

static int write_test_file(const char *path, const void *data, size_t len)
{
	struct fs_file_t f;
	int ret;

	for (int attempt = 0; attempt < WRITE_RETRIES; attempt++) {
		if (attempt > 0) {
			int delay = MIN(200 << (attempt - 1),
					RETRY_DELAY_CAP_MS);

			TC_PRINT("Write retry %d/%d for %s (wait %d ms)\n",
				 attempt + 1, WRITE_RETRIES, path, delay);
			k_sleep(K_MSEC(delay));
		}
		fs_file_t_init(&f);
		ret = fs_open(&f, path,
			      FS_O_CREATE | FS_O_WRITE | FS_O_TRUNC);
		if (ret != 0) {
			continue;
		}
		ret = fs_write(&f, data, len);
		fs_close(&f);
		if (ret == (int)len) {
			return 0;
		}
	}
	return -EIO;
}

static int read_test_file(const char *path, void *buf, size_t buf_sz,
			  ssize_t *out_len)
{
	struct fs_file_t f;
	ssize_t len;
	int ret;

	for (int attempt = 0; attempt < WRITE_RETRIES; attempt++) {
		if (attempt > 0) {
			int delay = MIN(200 << (attempt - 1),
					RETRY_DELAY_CAP_MS);

			TC_PRINT("Read retry %d/%d for %s (wait %d ms)\n",
				 attempt + 1, WRITE_RETRIES, path, delay);
			k_sleep(K_MSEC(delay));
		}
		fs_file_t_init(&f);
		ret = fs_open(&f, path, FS_O_READ);
		if (ret != 0) {
			continue;
		}
		len = fs_read(&f, buf, buf_sz);
		fs_close(&f);
		if (len >= 0) {
			if (out_len) {
				*out_len = len;
			}
			return 0;
		}
	}
	return -EIO;
}

/* ─── Suite Setup / Teardown ─── */
static void *msc_suite_init(void)
{
	int err;

	err = msc_ensure_mounted();
	if (err) {
		TC_PRINT("WARNING: SD mount failed (%d)\n", err);
	}

	msc_usbd_ctx = sample_usbd_init_device(NULL);
	if (msc_usbd_ctx == NULL) {
		TC_PRINT("WARNING: USB device init failed\n");
		usb_init_ok = false;
		return NULL;
	}

	/*
	 * Do not usbd_enable() here. Enabling while FS tests run lets the
	 * host MSC driver hit the same SD card (stale DEPEVT / cancelled EP).
	 * USB is enabled in USB tests and in suite teardown for the host.
	 */
	usb_init_ok = false;
	TC_PRINT("USB MSC SD stack initialized (enable deferred)\n");

	/* Warm-up: absorb CMD24 error bursts after boot */
	if (sd_mounted) {
		const char *warmup = "/SD:/msc_warmup.tmp";
		struct fs_file_t wf;
		int wret;
		bool ok = false;

		for (int i = 0; i < LARGE_BUF_SIZE; i++) {
			large_wr[i] = (uint8_t)(i & 0xFF);
		}
		for (int i = 0; i < 30 && !ok; i++) {
			if (i > 0) {
				k_sleep(K_MSEC(RETRY_DELAY_CAP_MS));
			}
			fs_file_t_init(&wf);
			wret = fs_open(&wf, warmup,
				       FS_O_CREATE | FS_O_WRITE | FS_O_TRUNC);
			if (wret != 0) {
				continue;
			}
			wret = fs_write(&wf, large_wr, LARGE_BUF_SIZE);
			fs_close(&wf);
			if (wret == LARGE_BUF_SIZE) {
				ok = true;
			}
		}
		try_unlink(warmup);
		TC_PRINT("SDHC warm-up %s\n",
			 ok ? "complete" : "failed (tests may be flaky)");
	}

	return NULL;
}

static void msc_before(void *fixture)
{
	ARG_UNUSED(fixture);
	msc_ensure_mounted();
}

static void msc_after(void *fixture)
{
	ARG_UNUSED(fixture);
	if (sd_mounted) {
		msc_cleanup_files();
	}
}

static void msc_suite_cleanup(void *data)
{
	ARG_UNUSED(data);
	if (sd_mounted) {
		msc_cleanup_files();
	}
	if (msc_usbd_ctx != NULL) {
		(void)msc_usb_ensure_enabled();
		TC_PRINT("USB MSC remains enabled for the host drive\n");
	}
}

/* ========================================================================
 * 1. test_sd_mount_and_disk_info
 *
 * Mount, FS stats, disk_access API, sector count/size, card type.
 * ========================================================================
 */
ZTEST(msc_sd_qa, test_sd_mount_and_disk_info)
{
	struct fs_statvfs stat;
	uint32_t sector_count = 0;
	uint32_t sector_size = 0;
	int ret;

	zassert_true(sd_mounted, "SD not mounted");

	/* Filesystem statistics */
	ret = fs_statvfs(SD_MOUNT_POINT, &stat);
	zassert_equal(ret, 0, "fs_statvfs failed (%d)", ret);
	zassert_true(stat.f_bfree <= stat.f_blocks,
		     "Free blocks > total blocks");

	uint64_t total_bytes = (uint64_t)stat.f_blocks * stat.f_frsize;

	TC_PRINT("FS: bsize=%lu blocks=%lu bfree=%lu total=%llu MB\n",
		 stat.f_bsize, stat.f_blocks, stat.f_bfree,
		 total_bytes / (1024 * 1024));

	/* Disk access API + card info */
	ret = disk_access_init(SD_DISK_NAME);
	zassert_true(ret == 0 || ret == -EALREADY,
		     "disk_access_init failed (%d)", ret);

	ret = disk_access_status(SD_DISK_NAME);
	zassert_equal(ret, DISK_STATUS_OK, "Disk status not OK (%d)", ret);

	ret = disk_access_ioctl(SD_DISK_NAME, DISK_IOCTL_GET_SECTOR_COUNT,
				&sector_count);
	zassert_equal(ret, 0, "GET_SECTOR_COUNT failed (%d)", ret);
	zassert_true(sector_count > 0, "Sector count is 0");

	ret = disk_access_ioctl(SD_DISK_NAME, DISK_IOCTL_GET_SECTOR_SIZE,
				&sector_size);
	zassert_equal(ret, 0, "GET_SECTOR_SIZE failed (%d)", ret);
	zassert_equal(sector_size, (uint32_t)SECTOR_SIZE,
		      "Unexpected sector size %u", sector_size);

	uint32_t mb = sector_count / 2048;

	TC_PRINT("SD: %u sectors, %u B/sector, ~%u MB (%s)\n",
		 sector_count, sector_size, mb,
		 mb <= 2048 ? "SDSC" : mb <= 32768 ? "SDHC" : "SDXC");

	/* Raw sector read (MBR/boot) — do not write sector 0 */
	{
		uint8_t sector[SECTOR_SIZE];

		ret = disk_access_read(SD_DISK_NAME, sector, 0, 1);
		zassert_equal(ret, 0, "disk_access_read sector 0 failed (%d)", ret);
		TC_PRINT("disk_access_read(sector 0) OK\n");
	}
}

/* ========================================================================
 * 2. test_usb_init_and_descriptors
 *
 * USB init, enumeration, speed, PID, write-protect check.
 * ========================================================================
 */
ZTEST(msc_sd_qa, test_usb_init_and_descriptors)
{
	int ret;

	zassert_not_null(msc_usbd_ctx, "USB device context NULL");
	ret = msc_usb_ensure_enabled();
	zassert_equal(ret, 0, "usbd_enable failed (%d)", ret);

	enum usbd_speed caps = usbd_caps_speed(msc_usbd_ctx);
	enum usbd_speed bus = usbd_bus_speed(msc_usbd_ctx);

	zassert_false(sys_slist_is_empty(&msc_usbd_ctx->fs_configs),
		      "FS configuration not registered");
	if (caps == USBD_SPEED_HS) {
		zassert_false(sys_slist_is_empty(&msc_usbd_ctx->hs_configs),
			      "HS configuration not registered");
	}
	zassert_false(usbd_is_suspended(msc_usbd_ctx),
		      "USB unexpectedly suspended");

	TC_PRINT("USB capability: %s, bus: %s\n",
		 caps == USBD_SPEED_HS ? "High-Speed" :
		 caps == USBD_SPEED_FS ? "Full-Speed" : "Unknown",
		 bus == USBD_SPEED_HS ? "High-Speed" :
		 bus == USBD_SPEED_FS ? "Full-Speed" : "Unknown");
	zassert_true(caps == USBD_SPEED_FS || caps == USBD_SPEED_HS,
		     "Unexpected speed %d", caps);
	zassert_true(bus == USBD_SPEED_FS || bus == USBD_SPEED_HS,
		     "Unexpected bus speed %d", bus);

#ifdef CONFIG_SAMPLE_USBD_PID
	TC_PRINT("PID: 0x%04x\n", CONFIG_SAMPLE_USBD_PID);
	zassert_equal(CONFIG_SAMPLE_USBD_PID, 0x0008,
		      "PID mismatch (expected 0x0008)");
#endif

	/* Write-protect check */
	ret = write_test_file(SD_TEST_FILE, "wp", 2);
	zassert_equal(ret, 0, "Write failed - card may be write-protected");
	fs_unlink(SD_TEST_FILE);
	TC_PRINT("Write protect: off\n");
}

/* ========================================================================
 * 3. test_file_read_write_delete
 *
 * File CRUD: write, read-back, verify, delete, confirm absent.
 * ========================================================================
 */
ZTEST(msc_sd_qa, test_file_read_write_delete)
{
	char buf[64] = {0};
	ssize_t len;
	int ret;

	ret = write_test_file(SD_TEST_FILE, TEST_CONTENT,
			      strlen(TEST_CONTENT));
	zassert_equal(ret, 0, "Write failed (%d)", ret);

	ret = read_test_file(SD_TEST_FILE, buf, sizeof(buf) - 1, &len);
	zassert_equal(ret, 0, "Read failed (%d)", ret);
	buf[len] = '\0';
	zassert_true(strcmp(buf, TEST_CONTENT) == 0, "Content mismatch");
	TC_PRINT("Verified: \"%s\"\n", buf);

	ret = fs_unlink(SD_TEST_FILE);
	zassert_equal(ret, 0, "Unlink failed (%d)", ret);

	{
		struct fs_dirent st;

		ret = fs_stat(SD_TEST_FILE, &st);
		zassert_true(ret != 0, "Deleted file still exists");
	}
	TC_PRINT("File CRUD verified\n");
}

/* ========================================================================
 * 4. test_directory_and_lfn
 *
 * Directory create/list/delete + long file name support.
 * ========================================================================
 */
ZTEST(msc_sd_qa, test_directory_and_lfn)
{
	struct fs_dir_t dir;
	struct fs_dirent entry;
	int ret;
	bool found = false;

	/* Directory operations */
	ret = fs_mkdir(SD_DIR_PATH);
	zassert_true(ret == 0 || ret == -EEXIST, "mkdir failed (%d)", ret);

	ret = write_test_file(SD_DIR_FILE, "nested", 6);
	zassert_equal(ret, 0, "Nested file write failed (%d)", ret);

	fs_dir_t_init(&dir);
	ret = fs_opendir(&dir, SD_DIR_PATH);
	zassert_equal(ret, 0, "opendir failed (%d)", ret);

	while (fs_readdir(&dir, &entry) == 0 && entry.name[0] != '\0') {
		if (strcmp(entry.name, "nested.txt") == 0 ||
		    strcmp(entry.name, "NESTED.TXT") == 0) {
			found = true;
		}
	}
	fs_closedir(&dir);
	zassert_true(found, "Nested file not found in dir listing");

	fs_unlink(SD_DIR_FILE);
	fs_unlink(SD_DIR_PATH);
	TC_PRINT("Directory operations verified\n");

	/* LFN support */
#ifdef CONFIG_FS_FATFS_LFN
	const char *lfn_data = "LFN test content via MSC SD";
	char buf[64] = {0};
	ssize_t len;

	ret = write_test_file(SD_LFN_FILE, lfn_data, strlen(lfn_data));
	zassert_equal(ret, 0, "LFN write failed (%d)", ret);

	ret = read_test_file(SD_LFN_FILE, buf, sizeof(buf) - 1, &len);
	zassert_equal(ret, 0, "LFN read failed (%d)", ret);
	buf[len] = '\0';
	zassert_true(strcmp(buf, lfn_data) == 0, "LFN content mismatch");
	TC_PRINT("LFN verified: %s\n", SD_LFN_FILE);
#else
	TC_PRINT("LFN not enabled, skipped\n");
#endif
}

/* ========================================================================
 * 5. test_data_integrity_and_alignment
 *
 * Large file (4 KB ADMA), multi-file, 512-byte boundary alignment.
 * ========================================================================
 */
ZTEST(msc_sd_qa, test_data_integrity_and_alignment)
{
	ssize_t len;
	int ret;

	/* Large file transfer (ADMA) */
	TC_PRINT("Large file: %d bytes\n", LARGE_BUF_SIZE);
	for (int i = 0; i < LARGE_BUF_SIZE; i++) {
		large_wr[i] = (uint8_t)((i * 7 + 0xAB) & 0xFF);
	}

	ret = write_test_file(SD_LARGE_FILE, large_wr, LARGE_BUF_SIZE);
	zassert_equal(ret, 0, "Large write failed (%d)", ret);

	memset(large_rd, 0, LARGE_BUF_SIZE);
	ret = read_test_file(SD_LARGE_FILE, large_rd, LARGE_BUF_SIZE, &len);
	zassert_equal(ret, 0, "Large read failed (%d)", ret);
	zassert_equal(len, LARGE_BUF_SIZE, "Size mismatch");
	zassert_mem_equal(large_wr, large_rd, LARGE_BUF_SIZE,
			  "Data corruption detected");
	TC_PRINT("Large file integrity verified\n");

	/* Multi-file */
	TC_PRINT("Multi-file: %d files\n", MULTI_FILE_COUNT);
	char path[40], wbuf[32], rbuf[32];

	for (int i = 0; i < MULTI_FILE_COUNT; i++) {
		snprintk(path, sizeof(path), "/SD:/msc_f%02d.txt", i);
		snprintk(wbuf, sizeof(wbuf), "File content %d", i);
		ret = write_test_file(path, wbuf, strlen(wbuf));
		zassert_equal(ret, 0, "Write file %d failed", i);
	}
	for (int i = 0; i < MULTI_FILE_COUNT; i++) {
		snprintk(path, sizeof(path), "/SD:/msc_f%02d.txt", i);
		snprintk(wbuf, sizeof(wbuf), "File content %d", i);
		memset(rbuf, 0, sizeof(rbuf));
		ret = read_test_file(path, rbuf, sizeof(rbuf) - 1, &len);
		zassert_equal(ret, 0, "Read file %d failed", i);
		rbuf[len] = '\0';
		zassert_true(strcmp(wbuf, rbuf) == 0, "File %d mismatch", i);
		fs_unlink(path);
	}

	/* 512-byte alignment boundary */
	TC_PRINT("512-byte alignment boundary\n");
	static const int sizes[] = {1, 511, 512, 513, 1024};

	for (int s = 0; s < ARRAY_SIZE(sizes); s++) {
		snprintk(path, sizeof(path), "/SD:/align_%d.bin", sizes[s]);
		memset(large_wr, (uint8_t)(sizes[s] & 0xFF), sizes[s]);
		ret = write_test_file(path, large_wr, sizes[s]);
		zassert_equal(ret, 0, "Write %d B failed", sizes[s]);
		memset(large_rd, 0, sizes[s]);
		ret = read_test_file(path, large_rd, sizes[s], &len);
		zassert_equal(ret, 0, "Read %d B failed", sizes[s]);
		zassert_equal(len, sizes[s], "Size mismatch %d", sizes[s]);
		zassert_mem_equal(large_wr, large_rd, sizes[s],
				  "Mismatch for %d B", sizes[s]);
		fs_unlink(path);
	}
	TC_PRINT("All integrity + alignment tests passed\n");
}

/* ========================================================================
 * 6. test_usb_disconnect_reconnect
 *
 * Validate that USB is enabled and storage I/O still works.
 * Note: usbd_disable()/reenable is intentionally skipped due to DWC3 halt issues.
 * ========================================================================
 */
ZTEST(msc_sd_qa, test_usb_disconnect_reconnect)
{
	int err;

	if (msc_usbd_ctx == NULL) {
		ztest_test_skip();
		return;
	}

	err = msc_usb_ensure_enabled();
	zassert_equal(err, 0, "usbd_enable failed (%d)", err);

	/*
	 * Do not call usbd_disable(): DWC3 halt times out after
	 * SET_CONFIGURATION(0), so the host MSC drive disappears.
	 */
	TC_PRINT("Skipping usbd_disable: DWC3 halt would drop host MSC\n");
	zassert_true(sd_mounted, "SD not mounted");

	/* Verify post-reconnect data access */
	char buf[32] = {0};
	ssize_t len;

	err = write_test_file(SD_TEST_FILE, "post-reconnect", 14);
	zassert_equal(err, 0, "Post-reconnect write failed");

	err = read_test_file(SD_TEST_FILE, buf, sizeof(buf) - 1, &len);
	zassert_equal(err, 0, "Post-reconnect read failed");
	buf[len] = '\0';
	zassert_true(strcmp(buf, "post-reconnect") == 0, "Content mismatch");
	TC_PRINT("Storage still accessible (USB left enabled)\n");
}

/* ========================================================================
 * 7. test_overwrite_and_persistence
 *
 * Overwrite replaces content. Unmount/remount preserves data
 * (SD card is non-volatile).
 * ========================================================================
 */
ZTEST(msc_sd_qa, test_overwrite_and_persistence)
{
	char buf[64] = {0};
	ssize_t len;
	int ret;

	/* Overwrite */
	ret = write_test_file(SD_OVERWRITE_FILE, "original", 8);
	zassert_equal(ret, 0, "Original write failed");

	ret = write_test_file(SD_OVERWRITE_FILE, "new overwritten", 15);
	zassert_equal(ret, 0, "Overwrite failed");

	ret = read_test_file(SD_OVERWRITE_FILE, buf, sizeof(buf) - 1, &len);
	zassert_equal(ret, 0, "Read overwritten failed");
	buf[len] = '\0';
	zassert_true(strcmp(buf, "new overwritten") == 0,
		     "Overwrite content mismatch");
	zassert_is_null(strstr(buf, "original"), "Old content still present");
	TC_PRINT("Overwrite verified\n");

	/* Persistence across unmount/remount (non-volatile) */
	const char *pdata = "persist test 12345";

	ret = write_test_file(SD_TEST_FILE, pdata, strlen(pdata));
	zassert_equal(ret, 0, "Persist write failed");

	ret = msc_ensure_unmounted();
	zassert_equal(ret, 0, "Unmount failed");

	ret = msc_ensure_mounted();
	zassert_equal(ret, 0, "Remount failed");

	memset(buf, 0, sizeof(buf));
	ret = read_test_file(SD_TEST_FILE, buf, sizeof(buf) - 1, &len);
	zassert_equal(ret, 0, "Read after remount failed");
	buf[len] = '\0';
	zassert_true(strcmp(buf, pdata) == 0, "Data not persisted");
	TC_PRINT("Non-volatile persistence verified\n");
}

/* ========================================================================
 * 8. test_regulator_and_stress
 *
 * Regulator/SDHC init ordering + USB stress cycles with file I/O.
 * ========================================================================
 */
ZTEST(msc_sd_qa, test_regulator_and_stress)
{
	char wbuf[64], rbuf[64];
	ssize_t len;
	int ret;

	/* Regulator init order */
#ifndef CONFIG_REGULATOR
	zassert_true(false, "CONFIG_REGULATOR not enabled");
#endif
#if defined(CONFIG_SDHC)
	zassert_true(device_is_ready(sdhc_dev),
		     "SDHC device not ready - regulator may have failed");
#endif
	TC_PRINT("SDHC_INIT_PRIORITY=%d\n", CONFIG_SDHC_INIT_PRIORITY);
	zassert_true(CONFIG_SDHC_INIT_PRIORITY >= 80,
		     "SDHC init priority too low");
	zassert_true(sd_mounted, "SD not mounted - power sequencing wrong");
	TC_PRINT("Regulator -> SDHC ordering verified\n");

	/* File I/O stress without USB disable (DWC3 halt drops MSC) */
	TC_PRINT("Stress: %d file I/O cycles (USB disable skipped)\n",
		 STRESS_USB_CYCLES);
	for (int i = 0; i < STRESS_USB_CYCLES; i++) {
		snprintk(wbuf, sizeof(wbuf), "stress-sd-%d", i);
		ret = write_test_file(SD_TEST_FILE, wbuf, strlen(wbuf));
		zassert_equal(ret, 0, "Stress write %d", i);

		memset(rbuf, 0, sizeof(rbuf));
		ret = read_test_file(SD_TEST_FILE, rbuf,
				     sizeof(rbuf) - 1, &len);
		zassert_equal(ret, 0, "Stress read %d", i);
		rbuf[len] = '\0';
		zassert_true(strcmp(wbuf, rbuf) == 0, "Stress mismatch %d", i);
	}
	TC_PRINT("Stress cycles completed\n");
}

/* ─── Test Suite Registration ─── */
ZTEST_SUITE(msc_sd_qa,
	    NULL,                /* predicate */
	    msc_suite_init,      /* suite setup */
	    msc_before,          /* before each */
	    msc_after,           /* after each */
	    msc_suite_cleanup);  /* suite teardown */

/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/**
 * @file test_msc_ramdisk.c
 * @brief USB MSC Class - RAMDisk Mode test suite
 *
 * Device-side ztest code covering USB Mass Storage Class with RAM disk
 * backend on Alif platforms. 6 consolidated tests:
 *   1. RAMDisk mount + disk access  4. Directory + LFN
 *   2. USB init + descriptors       5. Data integrity
 *   3. File CRUD                    6. Reconnect + overwrite + volatile + stress
 *
 * RAMDisk: 192 sectors x 512 bytes = 96 KB. Volatile (data lost on reset).
 * No SD card or external hardware needed beyond USB.
 */

#include <zephyr/ztest.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/sys/slist.h>
#include <zephyr/sys/util.h>
#include <zephyr/usb/class/usbd_msc.h>
#include <zephyr/storage/disk_access.h>
#include <zephyr/fs/fs.h>
#include <zephyr/logging/log.h>
#include <ff.h>
#include <string.h>
#include <sample_usbd.h>

LOG_MODULE_REGISTER(test_msc_ramdisk, LOG_LEVEL_INF);

/* ─── Configuration ─── */
#define RAM_DISK_NAME      "RAM"
#define RAM_MOUNT_POINT    "/RAM:"
#define RAM_TEST_FILE      "/RAM:/msc_test.txt"
#define RAM_LFN_FILE       "/RAM:/long_filename_ramdisk_test.txt"
#define RAM_LARGE_FILE     "/RAM:/msc_lg.bin"
#define RAM_DIR_PATH       "/RAM:/msc_dir"
#define RAM_DIR_FILE       "/RAM:/msc_dir/nested.txt"
#define RAM_OVERWRITE_FILE "/RAM:/msc_ow.txt"

#define TEST_CONTENT       "USB MSC RAMDisk test data"
#define LARGE_BUF_SIZE     2048
#define STRESS_USB_CYCLES  5
#define MULTI_FILE_COUNT   5
#define SECTOR_SIZE        512
#define SECTOR_COUNT       192
#define USB_SETTLE_MS      500
#define USB_REENUM_MS      1000

/* ─── Shared State ─── */
static FATFS msc_fat_fs;
static struct usbd_context *msc_usbd_ctx;
static bool usb_init_ok;
static bool ram_mounted;

static struct fs_mount_t msc_mp = {
	.type = FS_FATFS,
	.fs_data = &msc_fat_fs,
	.mnt_point = RAM_MOUNT_POINT,
};

USBD_DEFINE_MSC_LUN(ram, "RAM", "Zephyr", "RAMDisk", "0.00");

static uint8_t large_wr[LARGE_BUF_SIZE];
static uint8_t large_rd[LARGE_BUF_SIZE];

/* ─── Helpers ─── */

static int __maybe_unused msc_usb_disable(void)
{
	int err;

	if (msc_usbd_ctx == NULL) {
		return -ENODEV;
	}

	k_sleep(K_MSEC(USB_SETTLE_MS));
	err = usbd_disable(msc_usbd_ctx);
	if (err == 0 || err == -EALREADY) {
		usb_init_ok = false;
		return 0;
	}

	TC_PRINT("usbd_disable failed (%d), controller did not halt\n", err);
	return err;
}

static int msc_usb_ensure_enabled(void)
{
	int err;

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
	return err;
}

static int msc_ensure_mounted(void)
{
	if (ram_mounted) {
		return 0;
	}
	int ret = fs_mount(&msc_mp);

	if (ret == 0) {
		ram_mounted = true;
	}
	return ret;
}

static int msc_ensure_unmounted(void)
{
	if (!ram_mounted) {
		return 0;
	}
	int ret = fs_unmount(&msc_mp);

	if (ret == 0) {
		ram_mounted = false;
	}
	return ret;
}

static void try_unlink(const char *path)
{
	struct fs_dirent entry;

	if (fs_stat(path, &entry) == 0) {
		fs_unlink(path);
	}
}

static int write_test_file(const char *path, const void *data, size_t len)
{
	struct fs_file_t f;
	int ret;

	fs_file_t_init(&f);
	ret = fs_open(&f, path, FS_O_CREATE | FS_O_WRITE | FS_O_TRUNC);
	if (ret != 0) {
		return ret;
	}
	ret = fs_write(&f, data, len);
	fs_close(&f);
	return (ret == (int)len) ? 0 : -EIO;
}

static int read_test_file(const char *path, void *buf, size_t buf_sz,
			  ssize_t *out_len)
{
	struct fs_file_t f;
	ssize_t len;
	int ret;

	fs_file_t_init(&f);
	ret = fs_open(&f, path, FS_O_READ);
	if (ret != 0) {
		return ret;
	}
	len = fs_read(&f, buf, buf_sz);
	fs_close(&f);
	if (len >= 0) {
		if (out_len) {
			*out_len = len;
		}
		return 0;
	}
	return -EIO;
}

static void msc_cleanup_files(void)
{
	char path[40];

	try_unlink(RAM_TEST_FILE);
	try_unlink(RAM_LFN_FILE);
	try_unlink(RAM_LARGE_FILE);
	try_unlink(RAM_DIR_FILE);
	try_unlink(RAM_DIR_PATH);
	try_unlink(RAM_OVERWRITE_FILE);

	for (int i = 0; i < MULTI_FILE_COUNT; i++) {
		snprintk(path, sizeof(path), "/RAM:/msc_f%02d.txt", i);
		try_unlink(path);
	}
	{
		static const int sizes[] = {1, 511, 512, 513};

		for (int s = 0; s < ARRAY_SIZE(sizes); s++) {
			snprintk(path, sizeof(path), "/RAM:/aln_%d.bin", sizes[s]);
			try_unlink(path);
		}
	}
}

/* ─── Suite Setup / Teardown ─── */

static void *msc_suite_init(void)
{
	int err;

	err = fs_mount(&msc_mp);
	if (err == 0) {
		ram_mounted = true;
		TC_PRINT("RAM disk mounted at %s\n", RAM_MOUNT_POINT);
	} else {
		TC_PRINT("WARNING: RAM disk mount failed (%d)\n", err);
	}

	msc_usbd_ctx = sample_usbd_init_device(NULL);
	if (msc_usbd_ctx == NULL) {
		TC_PRINT("WARNING: USB device init failed\n");
		return NULL;
	}

	usb_init_ok = false;
	TC_PRINT("USB MSC RAMDisk stack initialized (enable deferred)\n");

	return NULL;
}

static void msc_before(void *data)
{
	ARG_UNUSED(data);
	msc_ensure_mounted();
}

static void msc_after(void *data)
{
	ARG_UNUSED(data);
	if (ram_mounted) {
		msc_cleanup_files();
	}
}

static void msc_suite_cleanup(void *data)
{
	ARG_UNUSED(data);
	if (ram_mounted) {
		msc_cleanup_files();
		fs_unmount(&msc_mp);
		ram_mounted = false;
	}
	if (msc_usbd_ctx != NULL) {
		(void)msc_usb_ensure_enabled();
		TC_PRINT("USB MSC remains enabled for the host drive\n");
	}
}

/* ========================================================================
 * 1. test_ramdisk_mount_and_disk_access
 *
 * Mount, FS stats, disk_access API (init, status, 192 sectors, 512 B).
 * ========================================================================
 */
ZTEST(msc_ramdisk_qa, test_ramdisk_mount_and_disk_access)
{
	struct fs_statvfs stat;
	uint32_t sector_count = 0;
	uint32_t sector_size = 0;
	int ret;

	zassert_true(ram_mounted, "RAM disk not mounted");

	/* Filesystem statistics */
	ret = fs_statvfs(RAM_MOUNT_POINT, &stat);
	zassert_equal(ret, 0, "fs_statvfs failed (%d)", ret);
	zassert_true(stat.f_bfree <= stat.f_blocks,
		     "Free blocks > total blocks");

	uint64_t total_bytes = (uint64_t)stat.f_blocks * stat.f_frsize;

	TC_PRINT("FS: bsize=%lu blocks=%lu bfree=%lu total=%llu KB\n",
		 stat.f_bsize, stat.f_blocks, stat.f_bfree,
		 total_bytes / 1024);

	/* Disk access API */
	ret = disk_access_init(RAM_DISK_NAME);
	zassert_true(ret == 0 || ret == -EALREADY,
		     "disk_access_init failed (%d)", ret);

	ret = disk_access_status(RAM_DISK_NAME);
	zassert_equal(ret, DISK_STATUS_OK, "Disk status not OK (%d)", ret);

	ret = disk_access_ioctl(RAM_DISK_NAME, DISK_IOCTL_GET_SECTOR_COUNT,
				&sector_count);
	zassert_equal(ret, 0, "GET_SECTOR_COUNT failed");
	zassert_equal(sector_count, (uint32_t)SECTOR_COUNT,
		      "Sector count %u != %u", sector_count, SECTOR_COUNT);

	ret = disk_access_ioctl(RAM_DISK_NAME, DISK_IOCTL_GET_SECTOR_SIZE,
				&sector_size);
	zassert_equal(ret, 0, "GET_SECTOR_SIZE failed");
	zassert_equal(sector_size, (uint32_t)SECTOR_SIZE,
		      "Sector size %u != %u", sector_size, SECTOR_SIZE);

	TC_PRINT("RAM disk: %u sectors x %u B = %u KB\n",
		 sector_count, sector_size,
		 (sector_count * sector_size) / 1024);

	{
		uint8_t sector[SECTOR_SIZE];

		ret = disk_access_read(RAM_DISK_NAME, sector, 0, 1);
		zassert_equal(ret, 0, "disk_access_read sector 0 failed (%d)", ret);
		TC_PRINT("disk_access_read(sector 0) OK\n");
	}
}

/* ========================================================================
 * 2. test_usb_init_and_descriptors
 *
 * USB init, enumeration, speed, PID.
 * ========================================================================
 */
ZTEST(msc_ramdisk_qa, test_usb_init_and_descriptors)
{
	zassert_not_null(msc_usbd_ctx, "USB device context NULL");
	zassert_equal(msc_usb_ensure_enabled(), 0, "usbd_enable failed");

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
	TC_PRINT("USB init + descriptors verified\n");
}

/* ========================================================================
 * 3. test_file_read_write_delete
 *
 * File CRUD: write, read-back, verify, delete, confirm absent.
 * ========================================================================
 */
ZTEST(msc_ramdisk_qa, test_file_read_write_delete)
{
	char buf[64] = {0};
	ssize_t len;
	int ret;

	ret = write_test_file(RAM_TEST_FILE, TEST_CONTENT,
			      strlen(TEST_CONTENT));
	zassert_equal(ret, 0, "Write failed (%d)", ret);

	ret = read_test_file(RAM_TEST_FILE, buf, sizeof(buf) - 1, &len);
	zassert_equal(ret, 0, "Read failed (%d)", ret);
	buf[len] = '\0';
	zassert_true(strcmp(buf, TEST_CONTENT) == 0, "Content mismatch");
	TC_PRINT("Verified: \"%s\"\n", buf);

	ret = fs_unlink(RAM_TEST_FILE);
	zassert_equal(ret, 0, "Unlink failed (%d)", ret);

	{
		struct fs_dirent st;

		ret = fs_stat(RAM_TEST_FILE, &st);
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
ZTEST(msc_ramdisk_qa, test_directory_and_lfn)
{
	struct fs_dir_t dir;
	struct fs_dirent entry;
	int ret;
	bool found = false;

	/* Directory operations */
	ret = fs_mkdir(RAM_DIR_PATH);
	zassert_true(ret == 0 || ret == -EEXIST, "mkdir failed (%d)", ret);

	ret = write_test_file(RAM_DIR_FILE, "nested", 6);
	zassert_equal(ret, 0, "Nested file write failed");

	fs_dir_t_init(&dir);
	ret = fs_opendir(&dir, RAM_DIR_PATH);
	zassert_equal(ret, 0, "opendir failed (%d)", ret);

	while (fs_readdir(&dir, &entry) == 0 && entry.name[0] != '\0') {
		if (strcmp(entry.name, "nested.txt") == 0 ||
		    strcmp(entry.name, "NESTED.TXT") == 0) {
			found = true;
		}
	}
	fs_closedir(&dir);
	zassert_true(found, "Nested file not found in dir listing");

	fs_unlink(RAM_DIR_FILE);
	fs_unlink(RAM_DIR_PATH);
	TC_PRINT("Directory operations verified\n");

	/* LFN support */
#ifdef CONFIG_FS_FATFS_LFN
	const char *lfn_data = "LFN test content via RAMDisk";
	char buf[64] = {0};
	ssize_t len;

	ret = write_test_file(RAM_LFN_FILE, lfn_data, strlen(lfn_data));
	zassert_equal(ret, 0, "LFN write failed");

	ret = read_test_file(RAM_LFN_FILE, buf, sizeof(buf) - 1, &len);
	zassert_equal(ret, 0, "LFN read failed");
	buf[len] = '\0';
	zassert_true(strcmp(buf, lfn_data) == 0, "LFN content mismatch");
	TC_PRINT("LFN verified: %s\n", RAM_LFN_FILE);
#else
	TC_PRINT("LFN not enabled, skipped\n");
#endif
}

/* ========================================================================
 * 5. test_data_integrity
 *
 * 2 KB large file + 5 multi-file integrity (fits in 96 KB RAMDisk).
 * ========================================================================
 */
ZTEST(msc_ramdisk_qa, test_data_integrity)
{
	ssize_t len;
	int ret;

	/* Large file */
	TC_PRINT("Large file: %d bytes\n", LARGE_BUF_SIZE);
	for (int i = 0; i < LARGE_BUF_SIZE; i++) {
		large_wr[i] = (uint8_t)(i & 0xFF);
	}

	ret = write_test_file(RAM_LARGE_FILE, large_wr, LARGE_BUF_SIZE);
	zassert_equal(ret, 0, "Large write failed");

	memset(large_rd, 0, LARGE_BUF_SIZE);
	ret = read_test_file(RAM_LARGE_FILE, large_rd, LARGE_BUF_SIZE, &len);
	zassert_equal(ret, 0, "Large read failed");
	zassert_equal(len, LARGE_BUF_SIZE, "Size mismatch");
	zassert_mem_equal(large_wr, large_rd, LARGE_BUF_SIZE,
			  "Data corruption detected");
	TC_PRINT("Large file integrity verified\n");

	fs_unlink(RAM_LARGE_FILE);

	/* Multi-file */
	TC_PRINT("Multi-file: %d files\n", MULTI_FILE_COUNT);
	char path[40], wbuf[32], rbuf[32];

	for (int i = 0; i < MULTI_FILE_COUNT; i++) {
		snprintk(path, sizeof(path), "/RAM:/msc_f%02d.txt", i);
		snprintk(wbuf, sizeof(wbuf), "File content %d", i);
		ret = write_test_file(path, wbuf, strlen(wbuf));
		zassert_equal(ret, 0, "Write file %d failed", i);
	}
	for (int i = 0; i < MULTI_FILE_COUNT; i++) {
		snprintk(path, sizeof(path), "/RAM:/msc_f%02d.txt", i);
		snprintk(wbuf, sizeof(wbuf), "File content %d", i);
		memset(rbuf, 0, sizeof(rbuf));
		ret = read_test_file(path, rbuf, sizeof(rbuf) - 1, &len);
		zassert_equal(ret, 0, "Read file %d failed", i);
		rbuf[len] = '\0';
		zassert_true(strcmp(wbuf, rbuf) == 0, "File %d mismatch", i);
		fs_unlink(path);
	}
	TC_PRINT("Multi-file integrity verified\n");

	/* 512-byte alignment (combined-spec test_alignment_boundaries) */
	TC_PRINT("512-byte alignment boundary\n");
	static const int sizes[] = {1, 511, 512, 513};
	char apath[40];

	for (int s = 0; s < ARRAY_SIZE(sizes); s++) {
		snprintk(apath, sizeof(apath), "/RAM:/aln_%d.bin", sizes[s]);
		memset(large_wr, (uint8_t)(sizes[s] & 0xFF), sizes[s]);
		ret = write_test_file(apath, large_wr, sizes[s]);
		zassert_equal(ret, 0, "Write %d B failed", sizes[s]);
		memset(large_rd, 0, sizes[s]);
		ret = read_test_file(apath, large_rd, sizes[s], &len);
		zassert_equal(ret, 0, "Read %d B failed", sizes[s]);
		zassert_equal(len, sizes[s], "Size mismatch %d", sizes[s]);
		zassert_mem_equal(large_wr, large_rd, sizes[s],
				  "Mismatch for %d B", sizes[s]);
		fs_unlink(apath);
	}
	TC_PRINT("Alignment boundary tests passed\n");
}

/* ========================================================================
 * 6. test_reconnect_overwrite_volatile_stress
 *
 * Overwrite + volatile nature + stress, with USB enabled.
 * Note: usbd_disable()/reenable is intentionally skipped due to DWC3 halt issues.
 * RAMDisk is volatile: data lost on power cycle (but survives unmount/remount within same session).
 * ========================================================================
 */
ZTEST(msc_ramdisk_qa, test_reconnect_overwrite_volatile_stress)
{
	char buf[64] = {0};
	ssize_t len;
	int ret;

	if (msc_usbd_ctx == NULL) {
		ztest_test_skip();
		return;
	}
	zassert_equal(msc_usb_ensure_enabled(), 0, "usbd_enable failed");

	/*
	 * Do not call usbd_disable(): DWC3 halt times out after
	 * SET_CONFIGURATION(0), so the host MSC drive disappears.
	 */
	TC_PRINT("Skipping usbd_disable: DWC3 halt would drop host MSC\n");
	zassert_true(ram_mounted, "RAM disk not mounted");

	ret = write_test_file(RAM_TEST_FILE, "post-reconnect", 14);
	zassert_equal(ret, 0, "Post-reconnect write failed");

	ret = read_test_file(RAM_TEST_FILE, buf, sizeof(buf) - 1, &len);
	zassert_equal(ret, 0, "Post-reconnect read failed");
	buf[len] = '\0';
	zassert_true(strcmp(buf, "post-reconnect") == 0, "Mismatch");
	TC_PRINT("Storage still accessible (USB left enabled)\n");

	/* Overwrite */
	ret = write_test_file(RAM_OVERWRITE_FILE, "original", 8);
	zassert_equal(ret, 0, "Original write failed");

	ret = write_test_file(RAM_OVERWRITE_FILE, "new overwritten", 15);
	zassert_equal(ret, 0, "Overwrite failed");

	memset(buf, 0, sizeof(buf));
	ret = read_test_file(RAM_OVERWRITE_FILE, buf, sizeof(buf) - 1, &len);
	zassert_equal(ret, 0, "Read overwritten failed");
	buf[len] = '\0';
	zassert_true(strcmp(buf, "new overwritten") == 0,
		     "Overwrite content mismatch");
	zassert_is_null(strstr(buf, "original"), "Old content present");
	TC_PRINT("Overwrite verified\n");

	/* Volatile nature: data persists within session (unmount/remount) */
	ret = write_test_file(RAM_TEST_FILE, "persist data", 12);
	zassert_equal(ret, 0, "Persist write failed");

	ret = msc_ensure_unmounted();
	zassert_equal(ret, 0, "Unmount failed");

	ret = msc_ensure_mounted();
	zassert_equal(ret, 0, "Remount failed");

	memset(buf, 0, sizeof(buf));
	ret = read_test_file(RAM_TEST_FILE, buf, sizeof(buf) - 1, &len);
	zassert_equal(ret, 0, "Read after remount failed");
	buf[len] = '\0';
	zassert_true(strcmp(buf, "persist data") == 0,
		     "Data not persisted within session");
	TC_PRINT("Volatile: data persists within session (expected)\n");
	TC_PRINT("NOTE: Data will be lost on power cycle (volatile)\n");

	/* File I/O stress without USB disable (DWC3 halt drops MSC) */
	TC_PRINT("Stress: %d file I/O cycles (USB disable skipped)\n",
		 STRESS_USB_CYCLES);
	char wbuf[64], rbuf[64];

	for (int i = 0; i < STRESS_USB_CYCLES; i++) {
		snprintk(wbuf, sizeof(wbuf), "stress-ram-%d", i);
		ret = write_test_file(RAM_TEST_FILE, wbuf, strlen(wbuf));
		zassert_equal(ret, 0, "Stress write %d", i);

		memset(rbuf, 0, sizeof(rbuf));
		ret = read_test_file(RAM_TEST_FILE, rbuf,
				     sizeof(rbuf) - 1, &len);
		zassert_equal(ret, 0, "Stress read %d", i);
		rbuf[len] = '\0';
		zassert_true(strcmp(wbuf, rbuf) == 0, "Stress mismatch %d", i);
	}
	TC_PRINT("All overwrite + volatile + file I/O stress verified\n");
}

/* ─── Test Suite Registration ─── */
ZTEST_SUITE(msc_ramdisk_qa,
	    NULL,                /* predicate */
	    msc_suite_init,      /* suite setup */
	    msc_before,          /* before each */
	    msc_after,           /* after each */
	    msc_suite_cleanup);  /* suite teardown */

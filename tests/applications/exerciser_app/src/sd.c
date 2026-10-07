/*
 * Copyright (c) 2019 Tavish Naruka <tavishnaruka@gmail.com>
 * Copyright (c) 2023 Nordic Semiconductor ASA
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/storage/disk_access.h>
#include <zephyr/logging/log.h>
#include <zephyr/fs/fs.h>
#include <ff.h>
#include <errno.h>

LOG_MODULE_REGISTER(sd_write, LOG_LEVEL_INF);

#define SD_NODE_OKAY	DT_NODE_HAS_STATUS_OKAY(DT_NODELABEL(sdhc))

#if SD_NODE_OKAY
/*
 * FatFs can only mount volume strings listed in _VOLUME_STRS in ffconf.h.
 * Overlay sets mmc disk-name = "SD".
 */
#define DISK_DRIVE_NAME "SD"
#define DISK_MOUNT_PT   "/" DISK_DRIVE_NAME ":"
#define FS_RET_OK       FR_OK
#define MAX_PATH        128
#define WRITE_CHUNK     4096
#define SD_STRESS_BYTES 1024

static FATFS Z_GENERIC_SECTION(CONFIG_SD_BUFFER_SECTION) fat_fs;
static struct fs_mount_t mp = {
	.type = FS_FATFS,
	.fs_data = &fat_fs,
};

static const char *disk_mount_pt = DISK_MOUNT_PT;

/* sd.c — write rgb, do not delete it */
int sd_write_rgb(const void *data, size_t size, unsigned int index)
{
	ARG_UNUSED(index);
	char path[MAX_PATH];
	struct fs_file_t file;
	const uint8_t *p = data;
	size_t remaining = size;
	int ret;

	if (!data || size == 0) {
		return -EINVAL;
	}
	/* 8.3 name is safe even without CONFIG_FS_FATFS_LFN */
	snprintf(path, sizeof(path), "%s/FRAME.data", disk_mount_pt);
	fs_file_t_init(&file);
	ret = fs_open(&file, path, FS_O_CREATE | FS_O_WRITE | FS_O_TRUNC);
	if (ret) {
		LOG_ERR("Failed to create %s (%d)", path, ret);
		return ret;
	}

	while (remaining > 0) {
		size_t chunk = MIN(remaining, WRITE_CHUNK);
		ssize_t wr = fs_write(&file, p, chunk);

		if (wr <= 0) {
			LOG_ERR("fs_write failed (%d)", (int)wr);
			fs_close(&file);
			return (wr < 0) ? wr : -ENOSPC;
		}
		p += wr;
		remaining -= wr;
	}

	ret = fs_sync(&file);
	fs_close(&file);
	if (ret) {
		LOG_ERR("fs_sync failed (%d)", ret);
		return ret;
	}
	LOG_INF("Wrote %zu bytes to %s", size, path);
	return 0;
}

void sd_thread(void)
{
	static uint8_t scratch[SD_STRESS_BYTES];
	static const char *disk_pdrv = DISK_DRIVE_NAME;
	int res;
	int i;

	for (i = 0; i < SD_STRESS_BYTES; i++) {
		scratch[i] = (uint8_t)i;
	}

	if (disk_access_ioctl(disk_pdrv, DISK_IOCTL_CTRL_INIT, NULL) != 0) {
		LOG_ERR("Storage init ERROR!");
		return;
	}

	mp.mnt_point = disk_mount_pt;
	res = fs_mount(&mp);
	if (res != FS_RET_OK) {
		LOG_ERR("Error mounting disk: %d", res);
		return;
	}
	LOG_INF("Disk mounted successfully.");

	while (1) {
		res = sd_write_rgb(scratch, SD_STRESS_BYTES, 0);
		if (res) {
			LOG_ERR("SD write failed (%d)", res);
		}
		k_msleep(2000);
	}
}

#endif /* SD_NODE_OKAY */

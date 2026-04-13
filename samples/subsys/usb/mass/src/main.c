/*
 * SPDX-FileCopyrightText: Copyright Alif Semiconductor
 * SPDX-License-Identifier: Apache-2.0
 */

#include <sample_usbd.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/usb/class/usbd_msc.h>
#include <zephyr/fs/fs.h>

LOG_MODULE_REGISTER(main);

#if CONFIG_DISK_DRIVER_FLASH
#include <zephyr/storage/flash_map.h>
#endif

#if CONFIG_FAT_FILESYSTEM_ELM
#include <ff.h>
#endif

#if CONFIG_FILE_SYSTEM_LITTLEFS
#include <zephyr/fs/littlefs.h>
FS_LITTLEFS_DECLARE_DEFAULT_CONFIG(storage);
#endif

#if !defined(CONFIG_DISK_DRIVER_FLASH) && \
	!defined(CONFIG_DISK_DRIVER_RAM) && \
	!defined(CONFIG_DISK_DRIVER_SDMMC)
#error No supported disk driver enabled
#endif

#define STORAGE_PARTITION			ospi_storage_partition
#define STORAGE_PARTITION_ID		FIXED_PARTITION_ID(STORAGE_PARTITION)

#if defined(CONFIG_USB_DEVICE_STACK_NEXT)
static struct usbd_context *sample_usbd;

#if CONFIG_DISK_DRIVER_RAM
USBD_DEFINE_MSC_LUN(ram, "RAM", "Zephyr", "RAMDisk", "0.00");
#endif

#if CONFIG_DISK_DRIVER_FLASH
USBD_DEFINE_MSC_LUN(nand, "NAND", "Zephyr", "FlashDisk", "0.00");
#endif

#if CONFIG_DISK_DRIVER_SDMMC
USBD_DEFINE_MSC_LUN(sd, "SD", "Zephyr", "SD", "0.00");
#endif

static int enable_usb_device_next(void)
{
	int err;

	sample_usbd = sample_usbd_init_device(NULL);
	if (sample_usbd == NULL) {
		LOG_ERR("Failed to initialize USB device");
		return -ENODEV;
	}

	err = usbd_enable(sample_usbd);
	if (err) {
		LOG_ERR("Failed to enable device support");
		return err;
	}

	LOG_DBG("USB device support enabled");

	return 0;
}
#endif /* defined(CONFIG_USB_DEVICE_STACK_NEXT) */

static int setup_flash(struct fs_mount_t *mnt)
{
	int rc = 0;
#if CONFIG_DISK_DRIVER_FLASH
	unsigned int id;
	const struct flash_area *pfa;

	mnt->storage_dev = (void *)STORAGE_PARTITION_ID;
	id = STORAGE_PARTITION_ID;

	rc = flash_area_open(id, &pfa);
	if (rc < 0) {
		LOG_ERR("Failed to open flash area %u: %d", id, rc);
		return rc;
	}

	LOG_INF("Area %u at 0x%x on %s for %u bytes",
		id, (unsigned int)pfa->fa_off, pfa->fa_dev->name,
		(unsigned int)pfa->fa_size);

	if (IS_ENABLED(CONFIG_APP_WIPE_STORAGE)) {
		LOG_INF("Erasing flash area ...");
		rc = flash_area_flatten(pfa, 0, pfa->fa_size);
		LOG_INF("Erase result: %d", rc);
	}

	if (rc < 0) {
		flash_area_close(pfa);
	}
#endif
	return rc;
}

struct disk_mount_info {
	const char *mnt_point;
	struct fs_mount_t mnt;
	union {
#if CONFIG_FAT_FILESYSTEM_ELM
		FATFS fat_fs;
#endif
#if CONFIG_FILE_SYSTEM_LITTLEFS
		struct fs_littlefs lfs_data;
#endif
	} fs_data;
};

static struct disk_mount_info disk_mounts[] = {
#if CONFIG_DISK_DRIVER_RAM
	{ .mnt_point = "/RAM:" },
#endif
#if CONFIG_DISK_DRIVER_FLASH
	{ .mnt_point = "/NAND:" },
#endif
#if CONFIG_DISK_DRIVER_SDMMC
	{ .mnt_point = "/SD:" },
#endif
};

static void mount_and_list(struct disk_mount_info *info)
{
	struct fs_dir_t dir;
	struct fs_statvfs sbuf;
	int rc;

	fs_dir_t_init(&dir);

#if CONFIG_FAT_FILESYSTEM_ELM
	info->mnt.type = FS_FATFS;
	info->mnt.fs_data = &info->fs_data.fat_fs;
#elif CONFIG_FILE_SYSTEM_LITTLEFS
	info->mnt.type = FS_LITTLEFS;
	info->mnt.fs_data = &info->fs_data.lfs_data;
#endif
	info->mnt.mnt_point = info->mnt_point;

	rc = fs_mount(&info->mnt);
	if (rc < 0) {
		LOG_ERR("Failed to mount %s: %d", info->mnt_point, rc);
		return;
	}

	LOG_INF("Mount %s: %d", info->mnt_point, rc);

	rc = fs_statvfs(info->mnt_point, &sbuf);
	if (rc < 0) {
		LOG_ERR("statvfs %s: %d", info->mnt_point, rc);
	} else {
		LOG_INF("%s: bsize = %lu ; frsize = %lu ;"
			" blocks = %lu ; bfree = %lu",
			info->mnt_point,
			sbuf.f_bsize, sbuf.f_frsize,
			sbuf.f_blocks, sbuf.f_bfree);
	}

	rc = fs_opendir(&dir, info->mnt_point);
	LOG_INF("%s opendir: %d", info->mnt_point, rc);

	if (rc < 0) {
		LOG_ERR("Failed to open directory");
		return;
	}

	while (rc >= 0) {
		struct fs_dirent ent = { 0 };

		rc = fs_readdir(&dir, &ent);
		if (rc < 0) {
			LOG_ERR("Failed to read directory entries");
			break;
		}
		if (ent.name[0] == 0) {
			LOG_INF("End of files");
			break;
		}
		LOG_INF("  %c %u %s",
			(ent.type == FS_DIR_ENTRY_FILE) ? 'F' : 'D',
			ent.size,
			ent.name);
	}

	(void)fs_closedir(&dir);
}

static void setup_disk(void)
{
	if (IS_ENABLED(CONFIG_DISK_DRIVER_FLASH)) {
		struct fs_mount_t flash_mnt;
		int rc;

		rc = setup_flash(&flash_mnt);
		if (rc < 0) {
			LOG_ERR("Failed to setup flash area");
			return;
		}
	}

	if (!IS_ENABLED(CONFIG_FILE_SYSTEM_LITTLEFS) &&
	    !IS_ENABLED(CONFIG_FAT_FILESYSTEM_ELM)) {
		LOG_INF("No file system selected");
		return;
	}

	for (int i = 0; i < ARRAY_SIZE(disk_mounts); i++) {
		mount_and_list(&disk_mounts[i]);
	}
}

int main(void)
{
	int ret;

	setup_disk();

#if defined(CONFIG_USB_DEVICE_STACK_NEXT)
	ret = enable_usb_device_next();
#else
	ret = usb_enable(NULL);
#endif
	if (ret != 0) {
		LOG_ERR("Failed to enable USB");
		return 0;
	}

	LOG_INF("The device is put in USB mass storage mode.");
	return 0;
}

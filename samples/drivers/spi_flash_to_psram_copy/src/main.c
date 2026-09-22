/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#include <stdio.h>
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/kernel.h>

#include "flash_to_psram.h"

#define SPI_FLASH_TEST_REGION_OFFSET 0x0
#define SPI_FLASH_SECTOR_SIZE        4096
#define BUFF_SIZE                    1024

#define FLASH_ALIAS_NODE DT_ALIAS(spi_flash0)

const struct flash_parameters *flash_param;

int main(void)
{
	printk("%s: start\n",  __func__);

#if DT_NODE_EXISTS(FLASH_ALIAS_NODE)
	const struct device *flash_dev = DEVICE_DT_GET(FLASH_ALIAS_NODE);

	if (!device_is_ready(flash_dev)) {
		printk("%s: device not ready.\n", flash_dev->name);
		return -1;
	}
#else
	printk("spi_flash0 alias not defined; sample requires the board overlay.\n");
	return -ENOTSUP;
#endif

	/*
	 * Copy the first 4MB of the flash into the PSRAM XiP window and verify
	 * it. The copy alternates OSPI0 between the flash and the PSRAM one chunk
	 * at a time and leaves the bus in PSRAM XiP mode -- return once done.
	 */
	int err = flash_to_psram_copy(0, 0, 4 * 1024 * 1024);

	if (err != 0) {
		printk("flash_to_psram_copy failed: %d\n", err);
		return err;
	}

	err = flash_to_psram_verify(0, 4 * 1024 * 1024);
	if (err != 0) {
		printk("flash_to_psram_verify failed: %d\n", err);
		return err;
	}

	return 0;
}

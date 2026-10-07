/*
 * Copyright (c) 2024 Alif Semiconductor
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "exerciser_app.h"

LOG_MODULE_REGISTER(exerciser_ospi, LOG_LEVEL_INF);

#if OSPI_NODE_OKAY
#define SPI_FLASH_SECTOR_SIZE        4096

static const struct flash_parameters *flash_param;

#define SPI_FLASH_SECTOR_4_OFFSET (4 * 1024 * 4)

void single_sector_test(const struct device *flash_dev)
{
	uint8_t expected[512];
	const size_t len = ARRAY_SIZE(expected);
	uint8_t buf[len];
	int rc;
	int i, e_count = 0;

	LOG_INF("Test 1: Flash erase");
	for (i = 0; i < 512; i++) {
		expected[i] =  i % 256;
	}

	/* Full flash erase if SPI_FLASH_TEST_REGION_OFFSET = 0 and
	 * SPI_FLASH_SECTOR_SIZE = flash size
	 */
	rc = flash_erase(flash_dev, SPI_FLASH_SECTOR_4_OFFSET, SPI_FLASH_SECTOR_SIZE);
	if (rc != 0) {
		LOG_ERR("Flash erase failed! %d", rc);
	} else {
		LOG_INF("Flash erase succeeded!");
	}

	LOG_INF("Test 1: Flash write");

	LOG_INF("Attempting to write %zu bytes", len);
	rc = flash_write(flash_dev, SPI_FLASH_SECTOR_4_OFFSET, expected, len);
	if (rc != 0) {
		LOG_ERR("Flash write failed! %d", rc);
		return;
	}
	k_msleep(1000);
	LOG_INF("Test 1: Flash read");

	memset(buf, 0, len);
	rc = flash_read(flash_dev, SPI_FLASH_SECTOR_4_OFFSET, buf, len);
	if (rc != 0) {
		LOG_ERR("Flash read failed! %d", rc);
		return;
	}

	for (i = 0; i < len; i++) {
		if (buf[i] != expected[i]) {
			e_count++;
			LOG_ERR("Not matched at [%d] _w[%4x] _r[%4x]", i, expected[i], buf[i]);
		}
	}

	if (e_count) {
		LOG_ERR("Error: Data read NOT matches data written");
	} else {
		LOG_INF("Data read matches data written. Success!!!");
	}
	k_msleep(1000);
}

void ospi_thread(void)
{
	const struct device *flash_dev = DEVICE_DT_GET(DT_ALIAS(spi_flash0));

	if (!device_is_ready(flash_dev)) {
		LOG_ERR("%s: device not ready.", flash_dev->name);
		return;
	}
	while (1) {
		LOG_INF("%s OSPI flash testing", flash_dev->name);
		LOG_INF("========================================");
		flash_param = flash_get_parameters(flash_dev);

		LOG_INF("****Flash Configured Parameters******");
		LOG_INF("* Num Of Sectors : %d", flash_param->num_of_sector);
		LOG_INF("* Sector Size : %d", flash_param->sector_size);
		LOG_INF("* Page Size : %d", flash_param->page_size);
		LOG_INF("* Erase value : %d", flash_param->erase_value);
		LOG_INF("* Write Blk Size: %d", flash_param->write_block_size);
		LOG_INF("* Total Size in MB: %d",
			(flash_param->num_of_sector * flash_param->sector_size) / (1024 * 1024));

		/* ---- RUN ALL OSPI TESTS HERE ---- */
		single_sector_test(flash_dev);
		LOG_INF("OSPI flash test thread completed.");
		k_msleep(8000);
	}
}
#endif /* OSPI_NODE_OKAY */

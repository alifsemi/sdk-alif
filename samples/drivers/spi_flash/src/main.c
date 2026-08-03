/*
 * Copyright (c) 2024 Alif Semiconductor
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/kernel.h>

#define SPI_FLASH_TEST_REGION_OFFSET 0x0
#define SPI_FLASH_SECTOR_SIZE        4096
#define BUFF_SIZE                    1024

const struct flash_parameters *flash_param;

void single_sector_test(const struct device *flash_dev)
{
	int rc;
	int i, e_count = 0;

	uint8_t w_buf[BUFF_SIZE] = {0};
	uint8_t r_buf[BUFF_SIZE] = {0};

	const size_t len = ARRAY_SIZE(w_buf);

	for (i = 0; i < BUFF_SIZE; i++) {
		w_buf[i] =  i % 256;
	}

	printf("\nTest 1: Flash erase\n");

	/* Full flash erase if SPI_FLASH_TEST_REGION_OFFSET = 0 and
	 * SPI_FLASH_SECTOR_SIZE = flash size
	 */
	rc = flash_erase(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, SPI_FLASH_SECTOR_SIZE);
	if (rc != 0) {
		printf("Flash erase failed! %d\n", rc);
		return;
	} else {
		printf("Flash erase succeeded!\n");
	}

	printf("\nTest 1: Flash write\n");

	printf("Attempting to write %zu bytes\n", len);
	rc = flash_write(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, w_buf, len);
	if (rc != 0) {
		printf("Flash write failed! %d\n", rc);
		return;
	}

	printf("\nTest 1: Flash read\n");

	memset(r_buf, 0, len);
	rc = flash_read(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, r_buf, len);
	if (rc != 0) {
		printf("Flash read failed! %d\n", rc);
		return;
	}

	for (i = 0; i < len; i++) {
		if (r_buf[i] != w_buf[i]) {
			e_count++;
			printf("Not matched at [%d] _w[%4d] _r[%4d]\n", i, w_buf[i], r_buf[i]);
		}
	}

	if (e_count) {
		printf("Error:Data read NOT matches data written\n");
	} else {
		printf("Data read matches data written. Good!!\n");
	}
}

void erase_test(const struct device *dev, size_t len)
{
	int ret = 0, i = 0, count = 0;
	uint8_t r_buf[BUFF_SIZE] = {0};

	printf("\nTest 2: Flash Full Erase\n");

	ret = flash_erase(dev, SPI_FLASH_TEST_REGION_OFFSET, len);
	if (ret == 0) {
		printf("Successfully Erased whole Flash Memory\n");
	} else {
		printf("Error: Bulk Erase Failed [%d]\n", ret);
	}

	/* Read data Cleared Buffer */
	ret = flash_read(dev, SPI_FLASH_TEST_REGION_OFFSET, r_buf, BUFF_SIZE);
	if (ret != 0) {
		printf("Error: [RetVal :%d] Reading Erased value\n", ret);
		return;
	}

	/* Verify the read data */
	for (i = 0; i < BUFF_SIZE; i++) {
		if (r_buf[i] != flash_param->erase_value) {
			count++;
		}
	}

	printf("Total errors after reading erased chip = %d\n", count);
}

void multi_page_test(const struct device *flash_dev)
{
	int rc, i, e_count = 0;

	uint8_t w_buf[BUFF_SIZE] = {0};
	uint8_t r_buf[BUFF_SIZE] = {0};

	const size_t len = ARRAY_SIZE(w_buf);

	printf("\nTest 3: Flash erase\n");

	/* Full flash erase if SPI_FLASH_TEST_REGION_OFFSET = 0 and
	 * SPI_FLASH_SECTOR_SIZE = flash size
	 */
	rc = flash_erase(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, SPI_FLASH_SECTOR_SIZE);
	if (rc != 0) {
		printf("Flash erase failed! %d\n", rc);
	} else {
		printf("Flash erase succeeded!\n");
	}

	for (i = 0; i < BUFF_SIZE; i++) {
		w_buf[i] = i % 256;
	}

	printf("\nTest 3: Flash write\n");

	printf("Attempting to write %zu bytes\n", len);
	rc = flash_write(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, w_buf, len);
	if (rc != 0) {
		printf("Flash write failed! %d\n", rc);
		return;
	}

	printf("\nTest 3: Flash read\n");

	memset(r_buf, 0, len);
	rc = flash_read(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, r_buf, len);
	if (rc != 0) {
		printf("Flash read failed! %d\n", rc);
		return;
	}

	for (i = 0; i < BUFF_SIZE; i++) {
		if (r_buf[i] != w_buf[i]) {
			e_count++;
			printf("Not matched at [%d] _w[%4x] _r[%4x]\n", i, w_buf[i], r_buf[i]);
		}
	}

	if (e_count) {
		printf("Error:Data read NOT matches data written\n");
		printf(" -- number of Unmatched data [%d]\n", e_count);
	} else {
		printf("Data read matches data written. Good!!\n");
	}
}

#define SPI_FLASH_SECTOR_4_OFFSET (4 * 1024 * 4)
#define SPI_FLASH_SECTOR_5_OFFSET (5 * 1024 * 4)

void multi_sector_test(const struct device *flash_dev)
{
	int rc, i, e_count = 0;

	uint8_t w_buf[BUFF_SIZE] = {0};
	uint8_t r_buf[BUFF_SIZE] = {0};

	const size_t len = ARRAY_SIZE(w_buf);

	for (i = 0; i < BUFF_SIZE; i++) {
		w_buf[i] =  i % 256;
	}

	printf("\nTest 4: write sector %d\n", SPI_FLASH_SECTOR_4_OFFSET);
	/* Write into Sector 4 */
	rc = flash_write(flash_dev, SPI_FLASH_SECTOR_4_OFFSET, w_buf, len);
	if (rc != 0) {
		printf("\nFlash write failed at Sec 4! %d\n", rc);
		return;
	}

	printf("\nTest 4: write sector %d\n", SPI_FLASH_SECTOR_5_OFFSET);
	/* Write into Sector 5 */
	rc = flash_write(flash_dev, SPI_FLASH_SECTOR_5_OFFSET, w_buf, len);
	if (rc != 0) {
		printf("\nFlash write failed at Sec 5! %d\n", rc);
		return;
	}

	/* Read from Sector 4 */
	printf("Sec4: Read and Verify written data\n");

	e_count = 0;
	memset(r_buf, 0, len);

	printf("\nTest 4: read sector %d\n", SPI_FLASH_SECTOR_4_OFFSET);

	rc = flash_read(flash_dev, SPI_FLASH_SECTOR_4_OFFSET, r_buf, len);
	if (rc != 0) {
		printf("Flash read failed at Sector 4! %d\n", rc);
		return;
	}

	for (i = 0; i < BUFF_SIZE; i++) {
		if (r_buf[i] != w_buf[i]) {
			e_count++;
			printf("Not matched at [%d] _w[%4x] _r[%4x]\n", i, w_buf[i], r_buf[i]);
		}
	}

	if (e_count) {
		printf("\nError:Data read NOT matches data written\n");
		printf(" -- number of Unmatched data [%d]\n", e_count);
	} else {
		printf("\nData read matches data written. Good!!\n");
	}

	/* Read from Sector 5 */
	printf("Sec5: Read and Verify written data\n");

	e_count = 0;
	memset(r_buf, 0, len);

	printf("\nTest 4: read sector %d\n", SPI_FLASH_SECTOR_5_OFFSET);

	rc = flash_read(flash_dev, SPI_FLASH_SECTOR_5_OFFSET, r_buf, len);
	if (rc != 0) {
		printf("Flash read failed at Sector 5! %d\n", rc);
		return;
	}

	for (i = 0; i < BUFF_SIZE; i++) {
		if (r_buf[i] != w_buf[i]) {
			e_count++;
			printf("Not matched at [%d] _w[%4x] _r[%4x]\n", i, w_buf[i], r_buf[i]);
		}
	}

	if (e_count) {
		printf("Error:Data read NOT matches data written\n");
		printf(" -- number of Unmatched data [%d]\n", e_count);
	} else {
		printf("Data read matches data written. Good!!\n");
	}

	/* Erase multiple Sector Sec 4+5 */
	printf("\nTest 4: Erase Sector 4 and 5\n");
	printf("Flash Erase from Sector %d Size to Erase %d\n", SPI_FLASH_SECTOR_4_OFFSET,
	       SPI_FLASH_SECTOR_SIZE * 2);

	rc = flash_erase(flash_dev, SPI_FLASH_SECTOR_4_OFFSET, SPI_FLASH_SECTOR_SIZE * 2);
	if (rc != 0) {
		printf("\nMulti-Sector erase failed! %d\n", rc);
	} else {
		printf("\nMulti-Sector erase succeeded!\n");
	}

	int count_1 = 0;

	memset(r_buf, 0, len);

	printf("\nTest 4: read sector %d\n", SPI_FLASH_SECTOR_4_OFFSET);
	/* Read Erased value and compare */
	rc = flash_read(flash_dev, SPI_FLASH_SECTOR_4_OFFSET, r_buf, BUFF_SIZE);
	if (rc != 0) {
		printf("Error: [RetVal :%d] Reading Erased value\n", rc);
		return;
	}

	/* Verify the read data */
	for (i = 0; i < BUFF_SIZE; i++) {
		if (r_buf[i] != flash_param->erase_value) {
			count_1++;
		}
	}

	printf("Total errors after reading erased Sector 4 = %d\n", count_1);

	int count_2 = 0;

	memset(r_buf, 0, len);
	printf("\nTest 4: read sector %d\n", SPI_FLASH_SECTOR_5_OFFSET);

	/* Read Erased value and compare */
	rc = flash_read(flash_dev, SPI_FLASH_SECTOR_5_OFFSET, r_buf, BUFF_SIZE);
	if (rc != 0) {
		printf("Error: [RetVal :%d] Reading Erased value\n", rc);
		return;
	}

	/* Verify the read data */
	for (i = 0; i < BUFF_SIZE; i++) {
		if (r_buf[i] != flash_param->erase_value) {
			count_2++;
		}
	}

	printf("Total errors after reading erased Sector 5 = %d\n", count_2);

	if (count_1 == 0 && count_2 == 0) {
		printf("\nMulti-Sector Erase Test Succeeded !\n");
	} else {
		printf("\nMulti-Sector Erase Failed\n");
	}
}

void xip_test(void)
{
	const uintptr_t xip_addr = DT_REG_ADDR_BY_NAME(
		DT_PARENT(DT_ALIAS(spi_flash0)), xip);
	const uint8_t *xip = (const uint8_t *)xip_addr;
	uint8_t data[16];

	printf("\nXiP Read Test\n");

	memcpy(data, xip, sizeof(data));

	printf("XiP data at 0x%08" PRIxPTR ":\n", xip_addr);
	for (size_t i = 0; i < sizeof(data); i++) {
		printf("%02x%c", data[i], (i == sizeof(data) - 1U) ? '\n' : ' ');
	}
}


int main(void)
{
	const struct device *flash_dev = DEVICE_DT_GET(DT_ALIAS(spi_flash0));
	uint64_t flash_size;
	int rc;

	if (!device_is_ready(flash_dev)) {
		printk("%s: device not ready.\n", flash_dev->name);
		return -1;
	}

	printf("\n%s OSPI flash testing\n", flash_dev->name);
	printf("========================================\n");

	flash_param = flash_get_parameters(flash_dev);
	rc = flash_get_size(flash_dev, &flash_size);
	if (rc != 0) {
		printf("Flash size read failed! %d\n", rc);
		return rc;
	}
	if (flash_size > SIZE_MAX) {
		printf("Flash size too large for this platform: %" PRIu64 "\n", flash_size);
		return -ERANGE;
	}

	printf("****Flash Configured Parameters******\n");
	printf("* Erase value : %d\n", flash_param->erase_value);
	printf("* Write Blk Size: %zu\n", flash_param->write_block_size);
	printf("* Total Size in MB: %" PRIu64 "\n", flash_size / (1024U * 1024U));
#ifdef CONFIG_MSPI_XIP
	printf("\nRunning XiP mode test\n");
	xip_test();
#else
	printf("\nRunning indirect command-mode test cases\n");

	/*Current RW support only on 16 DFS */
	single_sector_test(flash_dev);

	/* Erase Full and verify the content */
	erase_test(flash_dev, (size_t)flash_size);

	/* Multipage test */
	multi_page_test(flash_dev);

	/* Multi-Secot R/W and Erase test*/
	multi_sector_test(flash_dev);
#endif
	return 0;
}

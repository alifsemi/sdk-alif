/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */


#ifndef OSPI_FLASH_TEST_H
#define OSPI_FLASH_TEST_H

#include <stddef.h>
#include <stdint.h>
#include <zephyr/device.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/kernel.h>

/* Test Configuration */
#define SPI_FLASH_SECTOR_SIZE        4096
#define SPI_FLASH_TEST_REGION_OFFSET (8  * SPI_FLASH_SECTOR_SIZE)  /* 0x8000 */
#define SPI_FLASH_SECTOR_4_OFFSET    (12 * SPI_FLASH_SECTOR_SIZE)  /* 0xC000 */
#define SPI_FLASH_SECTOR_5_OFFSET    (13 * SPI_FLASH_SECTOR_SIZE)  /* 0xD000 */
#define BUFF_SIZE                    1024
#define ITER                         100

/* Device Tree Node for SPI Flash */
#define SPI_FLASH_DT_NODE            DT_ALIAS(spi_flash0)

/* Global Variables */
extern const struct flash_parameters *flash_param;
extern const struct device *flash_dev;

/* Helper Functions */
void ospi_flash_setup_device(void);
int verify_data(const uint8_t *expected, const uint8_t *actual,
		size_t len, const char *tag);

#endif /* OSPI_FLASH_TEST_H */

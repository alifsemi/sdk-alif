/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <string.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/logging/log.h>
#include <zephyr/ztest.h>
#include <zephyr/kernel.h>
#include <flash_ospi_is25wx.h>

#include "test_ospi_flash_test.h"

LOG_MODULE_REGISTER(ospi_flashdev, LOG_LEVEL_INF);

#define FLASH_DEV_BUF 512
#define OSPI_DT_BUS_SPEED DT_PROP(DT_PARENT(SPI_FLASH_DT_NODE), bus_speed)

static void *ospi_flashdev_setup(void)
{
	ospi_flash_setup_device();
	return NULL;
}

static void ospi_flashdev_before(void *fixture)
{
	(void)fixture;

	zassert_not_null(flash_dev, "Flash device handle is NULL");
	zassert_true(device_is_ready(flash_dev), "Flash device is not ready: %s",
		     flash_dev->name);
	zassert_not_null(flash_param, "flash_get_parameters returned NULL");
}


/* -------- Test: OSPI_Flash_GetInfo_Validation -------- */
ZTEST(test_ospi_flashdev, test_ospi_flash_get_info_validation)
{
	const struct flash_parameters *param = flash_get_parameters(flash_dev);
	struct flash_pages_info page_info;
	uint32_t total;
	int ret;

	zassert_not_null(param, "flash_get_parameters returned NULL");

	zassert_equal(param->sector_size, DT_PROP(SPI_FLASH_DT_NODE, sector_size),
		      "sector_size %zu disagrees with DT", param->sector_size);
	zassert_equal(param->page_size, DT_PROP(SPI_FLASH_DT_NODE, page_size),
		      "page_size %zu disagrees with DT", param->page_size);
	zassert_equal(param->num_of_sector, DT_PROP(SPI_FLASH_DT_NODE, num_of_sector),
		      "num_of_sector %zu disagrees with DT", param->num_of_sector);
	zassert_equal(param->write_block_size, DT_PROP(SPI_FLASH_DT_NODE, write_block_size),
		      "write_block_size %zu disagrees with DT", param->write_block_size);
	zassert_equal(param->erase_value, DT_PROP(SPI_FLASH_DT_NODE, erase_value),
		      "erase_value 0x%02x disagrees with DT", param->erase_value);

	zassert_true(param->sector_size && !(param->sector_size & (param->sector_size - 1)),
		     "sector_size %zu is not a power of two", param->sector_size);
	zassert_true(param->write_block_size > 0, "write_block_size is zero");

	total = param->num_of_sector * param->sector_size;
	zassert_true(total > 0, "Total flash size computed as zero");

	ret = flash_get_page_info_by_offs(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, &page_info);
	zassert_equal(ret, 0, "flash_get_page_info_by_offs failed [%d]", ret);
	zassert_equal(page_info.size, param->page_size,
		      "Page size %zu disagrees with DT page_size %zu", page_info.size,
		      param->page_size);
	zassert_equal(page_info.start_offset % param->page_size, 0,
		      "Page start 0x%lx is not page aligned",
		      (unsigned long)page_info.start_offset);

	LOG_INF("geometry: %zu sectors x %zu B = %u B, page %zu B, wbs %zu B, erase 0x%02x",
		param->num_of_sector, param->sector_size, total, param->page_size,
		param->write_block_size, param->erase_value);
}

/* -------- Test: OSPI_Flash_CrossPage_Program -------- */
ZTEST(test_ospi_flashdev, test_ospi_flash_cross_page_program)
{
	uint8_t w_buf[FLASH_DEV_BUF];
	uint8_t r_buf[FLASH_DEV_BUF];
	size_t page = flash_param->page_size;
	off_t offset;
	int ret, mismatches;

	offset = SPI_FLASH_TEST_REGION_OFFSET + page - (FLASH_DEV_BUF / 2);

	for (int i = 0; i < FLASH_DEV_BUF; i++) {
		w_buf[i] = (uint8_t)(0x40 + (i % 0x60));
	}

	ret = flash_erase(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, page * 2);
	zassert_equal(ret, 0, "Erase of the two straddled pages failed [%d]", ret);

	ret = flash_write(flash_dev, offset, w_buf, FLASH_DEV_BUF);
	zassert_equal(ret, 0, "Cross-page program failed [%d]", ret);

	memset(r_buf, 0, sizeof(r_buf));
	ret = flash_read(flash_dev, offset, r_buf, FLASH_DEV_BUF);
	zassert_equal(ret, 0, "Cross-page read-back failed [%d]", ret);

	mismatches = verify_data(w_buf, r_buf, FLASH_DEV_BUF, "cross_page");
	zassert_equal(mismatches, 0, "Cross-page program lost %d bytes at the boundary",
		      mismatches);

	LOG_INF("cross-page program at 0x%lx spanning page boundary 0x%zx verified",
		(unsigned long)offset, page);
}

/* -------- Test: OSPI_Flash_Boundary_Address -------- */
ZTEST(test_ospi_flashdev, test_ospi_flash_boundary_address)
{
	uint8_t w_buf[64];
	uint8_t r_buf[64];
	uint32_t total = flash_param->num_of_sector * flash_param->sector_size;
	off_t last_sector = total - flash_param->sector_size;
	int ret, mismatches;

	for (int i = 0; i < sizeof(w_buf); i++) {
		w_buf[i] = (uint8_t)(0xC3 ^ i);
	}

	ret = flash_erase(flash_dev, last_sector, flash_param->sector_size);
	zassert_equal(ret, 0, "Erase of last sector 0x%lx failed [%d]", (unsigned long)last_sector,
		      ret);

	ret = flash_write(flash_dev, last_sector, w_buf, sizeof(w_buf));
	zassert_equal(ret, 0, "Write to last sector failed [%d]", ret);

	memset(r_buf, 0, sizeof(r_buf));
	ret = flash_read(flash_dev, last_sector, r_buf, sizeof(r_buf));
	zassert_equal(ret, 0, "Read from last sector failed [%d]", ret);

	mismatches = verify_data(w_buf, r_buf, sizeof(w_buf), "last_sector");
	zassert_equal(mismatches, 0, "Last-sector round trip lost %d bytes", mismatches);

	ret = flash_read(flash_dev, total, r_buf, sizeof(r_buf));
	zassert_not_equal(ret, 0, "Read past the end of the device unexpectedly succeeded");

	LOG_INF("last sector 0x%lx round trips; reads past 0x%x are rejected",
		(unsigned long)last_sector, total);
}

/* -------- Test: OSPI_Flash_BusyStatus_Poll -------- */
ZTEST(test_ospi_flashdev, test_ospi_flash_busy_status_poll)
{
	uint8_t buf[64];
	int64_t start, elapsed;
	int ret;

	/*
	 * flash_erase returns after the driver has polled flag-status. A
	 * successful erase that takes measurable time, then a flash_read of
	 * erase_value, is the API stand-in for SR.BUSY / RXFLR.
	 */
	start = k_uptime_get();
	ret = flash_erase(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, SPI_FLASH_SECTOR_SIZE);
	elapsed = k_uptime_get() - start;

	zassert_equal(ret, 0, "Erase failed [%d]", ret);
	zassert_true(elapsed > 0, "Erase returned in under a millisecond - busy poll skipped?");


	ret = flash_read(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, buf, sizeof(buf));
	zassert_equal(ret, 0, "Read straight after erase failed [%d]", ret);

	for (int i = 0; i < sizeof(buf); i++) {
		zassert_equal(buf[i], flash_param->erase_value,
			      "Byte %d not erased - busy poll returned early", i);
	}

	LOG_INF("erase took %lld ms and left the flash readable", elapsed);
}

/* -------- Test: OSPI_Flash_WriteEnableLatch -------- */
ZTEST(test_ospi_flashdev, test_ospi_flash_write_enable_latch)
{
	uint8_t pattern[32];
	uint8_t r_buf[32];
	int ret, mismatches;

	ret = flash_erase(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, SPI_FLASH_SECTOR_SIZE);
	zassert_equal(ret, 0, "Erase failed [%d]", ret);

	memset(pattern, 0xF0, sizeof(pattern));
	ret = flash_write(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, pattern, sizeof(pattern));
	zassert_equal(ret, 0, "First program failed - WEL not armed? [%d]", ret);

	ret = flash_read(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, r_buf, sizeof(r_buf));
	zassert_equal(ret, 0, "Read-back failed [%d]", ret);
	mismatches = verify_data(pattern, r_buf, sizeof(pattern), "wel_first");
	zassert_equal(mismatches, 0, "First program did not commit (%d bytes wrong)", mismatches);

	memset(pattern, 0x0F, sizeof(pattern));
	ret = flash_write(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, pattern, sizeof(pattern));
	zassert_equal(ret, 0, "Second program failed [%d]", ret);

	ret = flash_read(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, r_buf, sizeof(r_buf));
	zassert_equal(ret, 0, "Second read-back failed [%d]", ret);

	for (int i = 0; i < sizeof(r_buf); i++) {
		zassert_equal(r_buf[i], 0x00,
			      "Byte %d = 0x%02x; expected 0x00 from 0xF0 AND 0x0F", i, r_buf[i]);
	}

	LOG_INF("write-enable latch armed per command; AND-only programming confirmed");
}

/* -------- Test: OSPI_Flash_OctalDDR_ReadWrite -------- */
ZTEST(test_ospi_flashdev, test_ospi_flash_octal_ddr_read_write)
{
	const struct alif_flash_ospi_dev_data *data = flash_dev->data;
	uint8_t w_buf[FLASH_DEV_BUF];
	uint8_t r_buf[FLASH_DEV_BUF];
	int ret, mismatches;

	zassert_equal(data->trans_conf.frame_format, OSPI_FRF_OCTAL,
		      "Driver transfer config is not octal (%u)", data->trans_conf.frame_format);
	zassert_true(data->trans_conf.ddr_enable, "Driver transfer config has DDR disabled");

	for (int i = 0; i < FLASH_DEV_BUF; i++) {
		w_buf[i] = (uint8_t)(i * 7 + 3);
	}

	ret = flash_erase(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, SPI_FLASH_SECTOR_SIZE);
	zassert_equal(ret, 0, "Erase failed [%d]", ret);

	ret = flash_write(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, w_buf, FLASH_DEV_BUF);
	zassert_equal(ret, 0, "Octal DDR write failed [%d]", ret);

	memset(r_buf, 0, sizeof(r_buf));
	ret = flash_read(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, r_buf, FLASH_DEV_BUF);
	zassert_equal(ret, 0, "Octal DDR read failed [%d]", ret);

	mismatches = verify_data(w_buf, r_buf, FLASH_DEV_BUF, "octal_ddr");
	zassert_equal(mismatches, 0, "Octal DDR round trip lost %d bytes", mismatches);

	LOG_INF("octal DDR round trip of %d bytes verified at %u Hz", FLASH_DEV_BUF,
		OSPI_DT_BUS_SPEED);
}

ZTEST_SUITE(test_ospi_flashdev, NULL, ospi_flashdev_setup, ospi_flashdev_before, NULL, NULL);

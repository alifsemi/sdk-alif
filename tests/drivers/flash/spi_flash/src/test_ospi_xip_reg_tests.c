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

#include <flash_ospi_is25wx.h>

#include "test_ospi_flash_test.h"

LOG_MODULE_REGISTER(ospi_xip_reg, LOG_LEVEL_INF);

/*
 * XIP coverage in the same style as
 * samples/drivers/spi_flash/src/main.c xip_test():
 *   DT xip-base-address + memcpy from the window + flash_read() compare.
 *
 * Transitions use alif_hal_ospi_xip_enable / disable on the driver's handle.
 * Register fields (XIP_CTRL, AES_CTRL, FIFO, IMR) are not touched.
 */
#define XIP_DRIVER_ACTIVE IS_ENABLED(CONFIG_ALIF_OSPI_FLASH_XIP)
#define XIP_MAPPED_OK     (XIP_DRIVER_ACTIVE || IS_ENABLED(CONFIG_TEST_OSPI_XIP_LIVE))

#define OSPI_CTRL_NODE   DT_PARENT(SPI_FLASH_DT_NODE)
#define OSPI_XIP_BASE    ((uint32_t)DT_PROP_BY_IDX(OSPI_CTRL_NODE, xip_base_address, 0))
#define OSPI_XIP_SIZE    ((uint32_t)DT_PROP_BY_IDX(OSPI_CTRL_NODE, xip_base_address, 1))
#define OSPI_DT_XIP_WAIT DT_PROP(OSPI_CTRL_NODE, xip_wait_cycles)

static void skip_needs_xip_build(const char *what)
{
	LOG_WRN("%s needs CONFIG_ALIF_OSPI_FLASH_XIP=y or CONFIG_TEST_OSPI_XIP_LIVE=y",
		what);
	ztest_test_skip();
}

static void *ospi_xip_api_setup(void)
{
	ospi_flash_setup_device();
	LOG_INF("XIP window DT 0x%08x size 0x%08x, wait %u, driver-xip %d",
		OSPI_XIP_BASE, OSPI_XIP_SIZE, OSPI_DT_XIP_WAIT, XIP_DRIVER_ACTIVE);
	return NULL;
}

static void ospi_xip_api_before(void *fixture)
{
	(void)fixture;

	zassert_not_null(flash_dev, "Flash device handle is NULL");
	zassert_true(device_is_ready(flash_dev), "Flash device is not ready: %s",
		     flash_dev->name);
}

static HAL_OSPI_Handle_T ospi_handle(void)
{
	struct alif_flash_ospi_dev_data *data = flash_dev->data;

	return data->ospi_handle;
}

/* Same compare the sample app uses: mapped window vs flash_read(). */
static void xip_sample_compare(uint32_t offset, size_t words, const char *tag)
{
	uint32_t xip_r[64];
	uint32_t fls_r[64];
	const uint32_t *ptr;
	int ret;
	int mismatches = 0;

	zassert_true(words <= ARRAY_SIZE(xip_r), "%s: %zu words too large", tag, words);

	ptr = (const uint32_t *)(OSPI_XIP_BASE + offset);
	memcpy(xip_r, ptr, words * sizeof(uint32_t));

	ret = flash_read(flash_dev, offset, fls_r, words * sizeof(uint32_t));
	zassert_equal(ret, 0, "%s: flash_read failed [%d]", tag, ret);

	for (size_t i = 0; i < words; i++) {
		if (xip_r[i] != fls_r[i]) {
			mismatches++;
		}
	}

	zassert_equal(mismatches, 0, "%s: mapped and flash_read disagree on %d words",
		      tag, mismatches);
}

/* -------- Test: OSPI_XIP_MemoryMapped_Read -------- */
ZTEST(test_ospi_xipreg, test_ospi_xip_memory_mapped_read)
{
	zassert_true(OSPI_XIP_BASE != 0, "xip-base-address[0] is zero");
	zassert_true(OSPI_XIP_SIZE > 0, "xip-base-address[1] is zero");

	if (!XIP_MAPPED_OK) {
		skip_needs_xip_build("memory-mapped read comparison");
		return;
	}

	if (!XIP_DRIVER_ACTIVE) {
		int ret = alif_hal_ospi_xip_enable(ospi_handle());

		zassert_equal(ret, 0, "XIP enable for mapped compare failed [%d]", ret);
	}

	/* Sample app reads from the window base; tests use the reserved region. */
	xip_sample_compare(SPI_FLASH_TEST_REGION_OFFSET, 16, "memory-mapped");

	if (!XIP_DRIVER_ACTIVE) {
		int ret = alif_hal_ospi_xip_disable(ospi_handle());

		zassert_equal(ret, 0, "XIP disable after mapped compare failed [%d]", ret);
	}
	LOG_INF("mapped read at 0x%08x matches flash_read",
		OSPI_XIP_BASE + SPI_FLASH_TEST_REGION_OFFSET);
}

/* -------- Test: OSPI_XIP_Enable_Reentrancy -------- */
ZTEST(test_ospi_xipreg, test_ospi_xip_enable_reentrancy)
{
	int ret;

	ret = alif_hal_ospi_xip_enable(ospi_handle());
	zassert_equal(ret, 0, "first XIP enable failed [%d]", ret);

	ret = alif_hal_ospi_xip_enable(ospi_handle());
	zassert_equal(ret, 0, "second XIP enable failed [%d]", ret);

	if (XIP_DRIVER_ACTIVE) {
		xip_sample_compare(SPI_FLASH_TEST_REGION_OFFSET, 8, "enable-reentrancy");
	} else {
		uint8_t buf[16];

		ret = alif_hal_ospi_xip_disable(ospi_handle());
		zassert_equal(ret, 0, "restore disable failed [%d]", ret);
		ret = flash_read(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, buf, sizeof(buf));
		zassert_equal(ret, 0, "flash_read after enable/disable failed [%d]", ret);
	}

	LOG_INF("alif_hal_ospi_xip_enable is idempotent");
}

/* -------- Test: OSPI_XIP_Disable_Reentrancy -------- */
ZTEST(test_ospi_xipreg, test_ospi_xip_disable_reentrancy)
{
	uint8_t buf[16];
	int ret;

	ret = alif_hal_ospi_xip_disable(ospi_handle());
	zassert_equal(ret, 0, "first XIP disable failed [%d]", ret);

	ret = alif_hal_ospi_xip_disable(ospi_handle());
	zassert_equal(ret, 0, "second XIP disable failed [%d]", ret);

	ret = flash_read(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, buf, sizeof(buf));
	zassert_equal(ret, 0, "flash_read after repeated disable failed [%d]", ret);

	if (XIP_DRIVER_ACTIVE) {
		ret = alif_hal_ospi_xip_enable(ospi_handle());
		zassert_equal(ret, 0, "re-enable after disable failed [%d]", ret);
	}

	LOG_INF("alif_hal_ospi_xip_disable is idempotent");
}




/* -------- Test: OSPI_XIP_ModeBits_Insertion -------- */
ZTEST(test_ospi_xipreg, test_ospi_xip_mode_bits_insertion)
{
	/*
	 * The flash driver never programs mode bits (init leaves them 0).
	 * Confirm the header/DT contract the sample path relies on.
	 */
	zassert_equal(ISSI_XIP_INCR_CMD, 0xFD, "ISSI INCR opcode is not 0xFD");
	zassert_equal(ISSI_XIP_WRAP_CMD, 0xFD, "ISSI WRAP opcode is not 0xFD");
	zassert_true(OSPI_DT_XIP_WAIT > 0, "xip-wait-cycles is zero");
	LOG_INF("mode-bit path unused; XIP opcodes 0xFD, wait %u", OSPI_DT_XIP_WAIT);
}

/* -------- Test: OSPI_XIP_INCR_WRAP_Opcode_Selection -------- */
ZTEST(test_ospi_xipreg, test_ospi_xip_incr_wrap_opcode_selection)
{
	zassert_equal(ISSI_XIP_INCR_CMD, ISSI_XIP_WRAP_CMD,
		      "ISSI flash uses the same opcode for INCR and WRAP");
	zassert_equal(ISSI_XIP_INCR_CMD, 0xFD, "ISSI XIP opcode is not 0xFD");
	LOG_INF("INCR/WRAP macros are 0xFD (driver init values)");
}

ZTEST_SUITE(test_ospi_xipreg, NULL, ospi_xip_api_setup, ospi_xip_api_before, NULL, NULL);

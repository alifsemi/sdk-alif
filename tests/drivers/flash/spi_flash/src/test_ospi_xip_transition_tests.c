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

LOG_MODULE_REGISTER(ospi_xip_trans, LOG_LEVEL_INF);

/*
 * XIP entry/exit using the same HAL APIs the flash driver uses
 * (alif_hal_ospi_xip_enable / disable) plus Zephyr flash_* and DT macros.
 * OSPI/AES registers are not read or written from this file.
 *
 * Supported path: program with flash_* (XIP off), then XIP on for mapped
 * reads. XIP-on write / XIP-off read is not supported and is not tested.
 * Erase/write coverage lives in test_ospi_main.c.
 *
 * Mapped window reads need CONFIG_TEST_OSPI_XIP_LIVE (default y). A bad
 * descriptor is a bus fault, so disable that option on an unproven board.
 */
#define XIP_LIVE_ENABLED IS_ENABLED(CONFIG_TEST_OSPI_XIP_LIVE)

#define OSPI_CTRL_NODE   DT_PARENT(SPI_FLASH_DT_NODE)
#define OSPI_XIP_BASE    ((uint32_t)DT_PROP_BY_IDX(OSPI_CTRL_NODE, xip_base_address, 0))
#define OSPI_XIP_SIZE    ((uint32_t)DT_PROP_BY_IDX(OSPI_CTRL_NODE, xip_base_address, 1))
#define OSPI_DT_XIP_WAIT DT_PROP(OSPI_CTRL_NODE, xip_wait_cycles)

#define XIP_SCRATCH_OFFSET SPI_FLASH_SECTOR_12_OFFSET
#define OSPI_FLASH_SIZE                                                                    \
	((uint32_t)DT_PROP_OR(SPI_FLASH_DT_NODE, num_of_sector, 16384) *                   \
	 SPI_FLASH_SECTOR_SIZE)

static void *ospi_xiptrans_setup(void)
{
	ospi_flash_setup_device();
	LOG_INF("XIP window DT 0x%08x size 0x%08x wait %u live %d", OSPI_XIP_BASE,
		OSPI_XIP_SIZE, OSPI_DT_XIP_WAIT, XIP_LIVE_ENABLED);
	return NULL;
}

static void ospi_xiptrans_before(void *fixture)
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

static int xip_enter(void)
{
	return alif_hal_ospi_xip_enable(ospi_handle());
}

static int xip_exit(void)
{
	return alif_hal_ospi_xip_disable(ospi_handle());
}

static bool skip_unless_live(const char *what)
{
	if (XIP_LIVE_ENABLED) {
		return false;
	}

	LOG_WRN("%s needs mapped access; rebuild with CONFIG_TEST_OSPI_XIP_LIVE=y", what);
	ztest_test_skip();
	return true;
}

static void xip_window_read(uint32_t offset, uint32_t *dst, size_t words)
{
	const volatile uint32_t *src = (const volatile uint32_t *)(OSPI_XIP_BASE + offset);

	for (size_t i = 0; i < words; i++) {
		dst[i] = src[i];
	}
}

/* Compare the window (XIP on) with flash_read (XIP still on), same as the sample. */
static void assert_window_matches_flash(uint32_t offset, size_t words, const char *tag)
{
	uint32_t mapped[32];
	uint32_t indirect[32];
	int ret;

	zassert_true(words <= ARRAY_SIZE(mapped), "%s: %zu words exceeds compare buffer", tag,
		     words);

	xip_window_read(offset, mapped, words);

	ret = flash_read(flash_dev, offset, indirect, words * sizeof(uint32_t));
	zassert_equal(ret, 0, "%s: flash_read at 0x%08x failed [%d]", tag, offset, ret);

	for (size_t i = 0; i < words; i++) {
		zassert_equal(mapped[i], indirect[i],
			      "%s: word %zu at 0x%08x differs (mapped 0x%08x, flash 0x%08x)",
			      tag, i, offset, mapped[i], indirect[i]);
	}
}

/* -------- Test: OSPI_XIP_Random_Read -------- */
ZTEST(test_ospi_xiptrans, test_ospi_xip_random_read)
{
	static const uint32_t offsets[] = {
		SPI_FLASH_TEST_REGION_OFFSET + 0x004,
		SPI_FLASH_TEST_REGION_OFFSET + 0x3F0,
		SPI_FLASH_TEST_REGION_OFFSET + 0x104,
		SPI_FLASH_TEST_REGION_OFFSET + 0xAB0,
		SPI_FLASH_TEST_REGION_OFFSET + 0x058,
	};
	int ret;

	if (skip_unless_live("random mapped reads")) {
		return;
	}

	ret = xip_enter();
	zassert_equal(ret, 0, "XIP enable failed [%d]", ret);

	for (int i = 0; i < ARRAY_SIZE(offsets); i++) {
		assert_window_matches_flash(offsets[i], 4, "random read");
	}

	ret = xip_exit();
	zassert_equal(ret, 0, "XIP disable failed [%d]", ret);

	LOG_INF("%d scattered mapped reads matched flash_read", (int)ARRAY_SIZE(offsets));
}

/* -------- Test: OSPI_XIP_Boundary_Read -------- */
ZTEST(test_ospi_xiptrans, test_ospi_xip_boundary_read)
{
	const uint32_t last_word = OSPI_FLASH_SIZE - sizeof(uint32_t);
	const uint32_t sector_edge = SPI_FLASH_TEST_REGION_OFFSET + SPI_FLASH_SECTOR_SIZE -
				     sizeof(uint32_t) * 2;
	int ret;

	zassert_true(OSPI_XIP_SIZE >= OSPI_FLASH_SIZE,
		     "XIP window 0x%08x is smaller than flash 0x%08x", OSPI_XIP_SIZE,
		     OSPI_FLASH_SIZE);

	if (skip_unless_live("boundary mapped reads")) {
		return;
	}

	ret = xip_enter();
	zassert_equal(ret, 0, "XIP enable failed [%d]", ret);

	assert_window_matches_flash(0, 4, "window base");
	assert_window_matches_flash(sector_edge, 4, "sector boundary");
	assert_window_matches_flash(last_word, 1, "last word");

	ret = xip_exit();
	zassert_equal(ret, 0, "XIP disable failed [%d]", ret);

	LOG_INF("mapped reads correct at 0, 0x%08x and 0x%08x", sector_edge, last_word);
}

/* -------- Test: OSPI_XIP_DummyCycle -------- */
ZTEST(test_ospi_xiptrans, test_ospi_xip_dummy_cycle)
{
	int ret;

	zassert_true(OSPI_DT_XIP_WAIT > 0, "xip-wait-cycles in DT is zero");
	zassert_true(OSPI_DT_XIP_WAIT <= 31, "xip-wait-cycles %u exceeds 5-bit field",
		     OSPI_DT_XIP_WAIT);

	if (skip_unless_live("mapped read with DT dummy cycles")) {
		return;
	}

	ret = xip_enter();
	zassert_equal(ret, 0, "XIP enable failed [%d]", ret);
	assert_window_matches_flash(SPI_FLASH_TEST_REGION_OFFSET, 16, "dt wait cycles");
	ret = xip_exit();
	zassert_equal(ret, 0, "XIP disable failed [%d]", ret);

	LOG_INF("XIP entry uses DT xip-wait-cycles %u; mapped data matches flash_read",
		OSPI_DT_XIP_WAIT);
}

/* -------- Test: OSPI_XIP_OFF_to_ON_DataIntegrity -------- */
ZTEST(test_ospi_xiptrans, test_ospi_xip_off_to_on_data_integrity)
{
	uint8_t pattern[128];
	uint32_t mapped[ARRAY_SIZE(pattern) / sizeof(uint32_t)];
	int ret;

	for (int i = 0; i < ARRAY_SIZE(pattern); i++) {
		pattern[i] = (uint8_t)(i * 7 + 1);
	}

	ret = xip_exit();
	zassert_equal(ret, 0, "XIP disable failed [%d]", ret);

	ret = flash_erase(flash_dev, XIP_SCRATCH_OFFSET, SPI_FLASH_SECTOR_SIZE);
	zassert_equal(ret, 0, "Erase failed [%d]", ret);

	ret = flash_write(flash_dev, XIP_SCRATCH_OFFSET, pattern, sizeof(pattern));
	zassert_equal(ret, 0, "Write failed [%d]", ret);

	if (skip_unless_live("reading the written pattern back through the window")) {
		return;
	}

	ret = xip_enter();
	zassert_equal(ret, 0, "XIP enable failed [%d]", ret);
	xip_window_read(XIP_SCRATCH_OFFSET, mapped, ARRAY_SIZE(mapped));
	ret = xip_exit();
	zassert_equal(ret, 0, "XIP disable failed [%d]", ret);

	zassert_mem_equal(mapped, pattern, sizeof(pattern),
			  "Pattern written with XIP off is not intact when read through XIP");

	LOG_INF("%zu bytes written indirectly read back through the XIP window",
		sizeof(pattern));
}

ZTEST_SUITE(test_ospi_xiptrans, NULL, ospi_xiptrans_setup, ospi_xiptrans_before, NULL, NULL);

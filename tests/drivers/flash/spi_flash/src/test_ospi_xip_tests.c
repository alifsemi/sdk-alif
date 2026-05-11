/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */


#include <string.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/logging/log.h>
#include <zephyr/ztest.h>

#include "test_ospi_flash_test.h"

LOG_MODULE_REGISTER(ospi_xip, LOG_LEVEL_INF);

const struct flash_parameters *flash_param;
const struct device *flash_dev;

void ospi_flash_setup_device(void)
{
	flash_dev = DEVICE_DT_GET(SPI_FLASH_DT_NODE);
	zassert_not_null(flash_dev, "Flash device handle is NULL");
	zassert_true(device_is_ready(flash_dev),
		     "Flash device is not ready: %s", flash_dev->name);
	flash_param = flash_get_parameters(flash_dev);
	zassert_not_null(flash_param, "Failed to get flash parameters");
}

/* -------- Test: XIP mode -------- */
static void xip_test(const struct device *dev)
{
	uint32_t i;
	uint32_t xip_r[64] = { 0 }, fls_r[64] = { 0 }, cnt;
	uint32_t *ptr = (uint32_t *)(DT_PROP_BY_IDX(
			DT_PARENT(DT_ALIAS(spi_flash0)),
			xip_base_address, 0) +
			SPI_FLASH_TEST_REGION_OFFSET);
	int32_t rc, e_count = 0;

	LOG_INF("XiP Read");

	memcpy(xip_r, ptr, sizeof(xip_r));
	LOG_INF("Content Read from OSPI Flash in XiP Mode");

	cnt = ARRAY_SIZE(xip_r);
	LOG_INF("Read from Flash cmd while XiP Mode turned on");

	rc = flash_read(dev, SPI_FLASH_TEST_REGION_OFFSET, fls_r, cnt * sizeof(uint32_t));
	zassert_equal(rc, 0, "Flash read failed! %d\n", rc);

	for (i = 0; i < cnt; i++) {
		if (fls_r[i] != xip_r[i]) {
			e_count++;
		}
	}

	zassert_true(!e_count,
		     "XiP Test Failed! Err Count [%d]",
		     e_count);
	LOG_INF("XiP read test PASSED");
}

/* ======== XIP Test Suite ======== */

/* TESTLINK TEST CASE - ZTC-1395 */
ZTEST(test_ospi_xip, test_ospi_xip_mode)
{
	xip_test(flash_dev);
}

/* ======== Test Suite Lifecycle Functions ======== */

static void *test_ospi_xip_setup(void)
{
	LOG_INF("=== OSPI XIP Test Suite Setup ===");
	ospi_flash_setup_device();
	return NULL;
}

static void test_ospi_xip_before(void *fixture)
{
	(void)fixture;
	zassert_not_null(flash_dev, "Flash device handle is NULL");
	zassert_true(device_is_ready(flash_dev),
		     "Flash device is not ready: %s", flash_dev->name);
}

ZTEST_SUITE(test_ospi_xip, NULL, test_ospi_xip_setup, test_ospi_xip_before,
	    NULL, NULL);

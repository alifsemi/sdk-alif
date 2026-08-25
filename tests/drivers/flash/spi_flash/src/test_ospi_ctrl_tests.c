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
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/ztest.h>

#include "test_ospi_flash_test.h"

LOG_MODULE_REGISTER(ospi_ctrl, LOG_LEVEL_INF);

/*
 * Controller configuration is applied at driver init from the OSPI parent
 * node (ospi0 or ospi1). Change those values with an overlay
 * (boards/ospi1_50Mhz.overlay, boards/ospi1_100Mhz.overlay, boards/ospi1_200Mhz.overlay,
 *  boards/ospi0_80Mhz.overlay) and rebuild. This
 * suite reads the compiled DT and proves the driver still serves flash_read /
 * flash_write / flash_erase. It does not poke OSPI registers.
 */
#define OSPI_CTRL_NODE      DT_PARENT(SPI_FLASH_DT_NODE)
#define OSPI_DT_BUS_SPEED   DT_PROP(OSPI_CTRL_NODE, bus_speed)
#define OSPI_DT_CORE_CLK    DT_PROP(OSPI_CTRL_NODE, clock_frequency)
#define OSPI_DT_CS_PIN      DT_PROP(OSPI_CTRL_NODE, cs_pin)
#define OSPI_DT_DDR_EDGE    DT_PROP(OSPI_CTRL_NODE, ddr_drive_edge)
#define OSPI_DT_RX_DS_DELAY DT_PROP(OSPI_CTRL_NODE, rx_ds_delay)
#define OSPI_DT_XIP_WAIT    DT_PROP(OSPI_CTRL_NODE, xip_wait_cycles)
#define OSPI_DT_BAUD2_DLY   DT_PROP(OSPI_CTRL_NODE, baud2_delay)
#define OSPI_DT_WBS         DT_PROP(SPI_FLASH_DT_NODE, write_block_size)

#define OSPI_CTRL_SCRATCH SPI_FLASH_SECTOR_12_OFFSET

static void *ospi_ctrl_setup(void)
{
	ospi_flash_setup_device();
	LOG_INF("OSPI parent %s: bus-speed %u Hz, clock-frequency %u Hz, cs-pin %u",
		flash_dev->name, OSPI_DT_BUS_SPEED, OSPI_DT_CORE_CLK, OSPI_DT_CS_PIN);
	return NULL;
}

static void ospi_ctrl_before(void *fixture)
{
	(void)fixture;

	zassert_not_null(flash_dev, "Flash device handle is NULL");
	zassert_true(device_is_ready(flash_dev), "Flash device is not ready: %s",
		     flash_dev->name);
}

static void assert_flash_readable(const char *tag)
{
	uint8_t buf[16];
	int ret;

	ret = flash_read(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, buf, sizeof(buf));
	zassert_equal(ret, 0, "%s: flash_read failed [%d]", tag, ret);
}

static void assert_flash_programmable(const char *tag)
{
	uint8_t pattern[16];
	uint8_t readback[16];
	int ret;

	for (int i = 0; i < ARRAY_SIZE(pattern); i++) {
		pattern[i] = (uint8_t)(0xA0 + i);
	}

	ret = flash_erase(flash_dev, OSPI_CTRL_SCRATCH, SPI_FLASH_SECTOR_SIZE);
	zassert_equal(ret, 0, "%s: flash_erase failed [%d]", tag, ret);

	ret = flash_write(flash_dev, OSPI_CTRL_SCRATCH, pattern, sizeof(pattern));
	zassert_equal(ret, 0, "%s: flash_write failed [%d]", tag, ret);

	ret = flash_read(flash_dev, OSPI_CTRL_SCRATCH, readback, sizeof(readback));
	zassert_equal(ret, 0, "%s: flash_read failed [%d]", tag, ret);

	zassert_mem_equal(readback, pattern, sizeof(pattern),
			  "%s: programmed data does not match", tag);
}

/* -------- Test: OSPI_BusSpeed_Config -------- */
ZTEST(test_ospi_ctrl, test_ospi_bus_speed_config)
{
	/*
	 * Overlay sets bus-speed; the driver programs BAUDR = clock-frequency /
	 * bus-speed at init. Prove the compiled value is a legal even divisor
	 * and that the resulting bus still talks to the flash.
	 */
	zassert_true(OSPI_DT_BUS_SPEED > 0, "bus-speed in DT is zero");
	zassert_true(OSPI_DT_CORE_CLK >= OSPI_DT_BUS_SPEED,
		     "clock-frequency %u < bus-speed %u", OSPI_DT_CORE_CLK,
		     OSPI_DT_BUS_SPEED);
	zassert_equal(OSPI_DT_CORE_CLK % OSPI_DT_BUS_SPEED, 0,
		      "clock-frequency %u is not an integer multiple of bus-speed %u",
		      OSPI_DT_CORE_CLK, OSPI_DT_BUS_SPEED);

	assert_flash_readable("bus-speed");
	LOG_INF("overlay/DT bus-speed %u Hz (div %u) serves flash_read",
		OSPI_DT_BUS_SPEED, OSPI_DT_CORE_CLK / OSPI_DT_BUS_SPEED);
}

/* -------- Test: OSPI_DataBits_Config -------- */
ZTEST(test_ospi_ctrl, test_ospi_databits_config)
{
	const struct flash_parameters *param = flash_get_parameters(flash_dev);

	/*
	 * Frame size is not a DT property on the controller. The driver derives
	 * DFS from the flash node's write-block-size (1 -> 8, 2 -> 16, 4 -> 32).
	 * Overlay that property to change the programmed DFS.
	 */
	zassert_not_null(param, "flash_get_parameters returned NULL");
	zassert_equal(param->write_block_size, OSPI_DT_WBS,
		      "write-block-size %zu disagrees with DT %u",
		      param->write_block_size, OSPI_DT_WBS);
	zassert_true(OSPI_DT_WBS == 1 || OSPI_DT_WBS == 2 || OSPI_DT_WBS == 4,
		     "write-block-size %u is not a legal DFS source", OSPI_DT_WBS);

	assert_flash_programmable("data-bits");
	LOG_INF("write-block-size %u -> %u-bit frames, program path works",
		OSPI_DT_WBS, OSPI_DT_WBS * 8);
}

/* -------- Test: OSPI_DDR_Config -------- */
ZTEST(test_ospi_ctrl, test_ospi_ddr_config)
{
	/*
	 * Data/instruction DDR bits are not overlay properties. The board's
	 * ddr-drive-edge is, and the ISSI path always uses data-phase DDR after
	 * init. Prove the DT edge is in range and the flash still programs.
	 */
	zassert_true(OSPI_DT_DDR_EDGE <= 1, "ddr-drive-edge %u is not 0 or 1",
		     OSPI_DT_DDR_EDGE);
	assert_flash_programmable("ddr-drive-edge");
	LOG_INF("ddr-drive-edge %u from DT; flash program path works",
		OSPI_DT_DDR_EDGE);
}

/* -------- Test: OSPI_WaitCycles_Config -------- */
ZTEST(test_ospi_ctrl, test_ospi_wait_cycles_config)
{
	/* Indirect reads hard-code 16 dummy cycles; XIP uses xip-wait-cycles. */
	zassert_true(OSPI_DT_XIP_WAIT <= 31, "xip-wait-cycles %u exceeds 5-bit field",
		     OSPI_DT_XIP_WAIT);
	assert_flash_readable("xip-wait-cycles");
	LOG_INF("xip-wait-cycles %u from DT; indirect flash_read still works",
		OSPI_DT_XIP_WAIT);
}

/* -------- Test: OSPI_SlaveSelect_Config -------- */
ZTEST(test_ospi_ctrl, test_ospi_slave_select_config)
{
	zassert_true(OSPI_DT_CS_PIN <= 3, "cs-pin %u is out of SER range", OSPI_DT_CS_PIN);
	assert_flash_readable("cs-pin");
	LOG_INF("cs-pin %u from DT; flash_read asserts the select path", OSPI_DT_CS_PIN);
}

/* -------- Test: OSPI_RXSampleDelay_Config -------- */
ZTEST(test_ospi_ctrl, test_ospi_rx_sample_delay_config)
{
	/*
	 * Overlay property is rx-ds-delay (AES RXDS), not the OSPI
	 * RX_SAMPLE_DELAY register. The driver copies it at init.
	 */
	zassert_true(OSPI_DT_RX_DS_DELAY <= 255, "rx-ds-delay %u exceeds 8 bits",
		     OSPI_DT_RX_DS_DELAY);
	assert_flash_readable("rx-ds-delay");
	LOG_INF("rx-ds-delay %u from DT; flash_read works", OSPI_DT_RX_DS_DELAY);
}

/* -------- Test: OSPI_DDRDriveEdge_Config -------- */
ZTEST(test_ospi_ctrl, test_ospi_ddr_drive_edge_config)
{
	zassert_true(OSPI_DT_DDR_EDGE <= 1, "ddr-drive-edge %u is not 0 or 1",
		     OSPI_DT_DDR_EDGE);
	assert_flash_readable("ddr-drive-edge");
	LOG_INF("ddr-drive-edge %u from DT", OSPI_DT_DDR_EDGE);
}

/* -------- Test: OSPI_GetStatus_Check -------- */
ZTEST(test_ospi_ctrl, test_ospi_get_status_check)
{
	/* Idle is implied when the flash API returns without EBUSY. */
	assert_flash_readable("idle-status");
	LOG_INF("controller is idle enough for flash_read to complete");
}


/* -------- Test: OSPI_GetStatus_BusyTiming -------- */
ZTEST(test_ospi_ctrl, test_ospi_get_status_busy_timing)
{
	int64_t start, elapsed;
	int ret;

	/*
	 * flash_erase returns only after the driver has waited for the part.
	 * That is the public stand-in for polling SR.BUSY in the testcase.
	 */
	start = k_uptime_get();
	ret = flash_erase(flash_dev, OSPI_CTRL_SCRATCH, SPI_FLASH_SECTOR_SIZE);
	elapsed = k_uptime_get() - start;

	zassert_equal(ret, 0, "flash_erase failed [%d]", ret);
	zassert_true(elapsed > 0, "erase returned in under 1 ms; busy wait skipped?");
}

/* -------- Test: OSPI_CS_MinHigh_Config -------- */
ZTEST(test_ospi_ctrl, test_ospi_cs_min_high_config)
{
	/* baud2-delay is the overlay knob (0 disable, 1 enable, 2 auto). */
	zassert_true(OSPI_DT_BAUD2_DLY <= 2, "baud2-delay %u is not 0/1/2",
		     OSPI_DT_BAUD2_DLY);
	assert_flash_readable("baud2-delay");
	LOG_INF("baud2-delay %u from DT; flash_read works", OSPI_DT_BAUD2_DLY);
}

/* -------- Flash must survive all of the above -------- */
ZTEST(test_ospi_ctrl, test_ospi_ctrl_flash_still_healthy)
{
	assert_flash_readable("ctrl-suite");
	LOG_INF("flash still responds after the DT/overlay controller sweep");
}

ZTEST_SUITE(test_ospi_ctrl, NULL, ospi_ctrl_setup, ospi_ctrl_before, NULL, NULL);

/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/devicetree.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/irq.h>
#include <zephyr/logging/log.h>
#include <zephyr/ztest.h>

#include "test_ospi_flash_test.h"

LOG_MODULE_REGISTER(ospi_irq, LOG_LEVEL_INF);

#define OSPI_CTRL_NODE DT_PARENT(SPI_FLASH_DT_NODE)

static void *ospi_irq_setup(void)
{
	ospi_flash_setup_device();
	return NULL;
}

static void ospi_irq_before(void *fixture)
{
	(void)fixture;

	zassert_not_null(flash_dev, "Flash device handle is NULL");
	zassert_true(device_is_ready(flash_dev), "Flash device is not ready: %s",
		     flash_dev->name);
}

/* -------- Test: OSPI_IRQ_Routing_PerInstance -------- */
ZTEST(test_ospi_irq, test_ospi_irq_routing_per_instance)
{
	zassert_equal(DT_NUM_IRQS(OSPI_CTRL_NODE), 1,
		      "Expected exactly one interrupt for the OSPI instance, found %d",
		      DT_NUM_IRQS(OSPI_CTRL_NODE));

	const unsigned int irqn = DT_IRQN(OSPI_CTRL_NODE);

	zassert_true(irq_is_enabled(irqn), "OSPI interrupt %u is not enabled", irqn);

	LOG_INF("OSPI instance routed to NVIC line %u at priority %u", irqn,
		DT_IRQ(OSPI_CTRL_NODE, priority));
}

/* -------- Flash must survive all of the above -------- */
ZTEST(test_ospi_irq, test_ospi_irq_flash_still_healthy)
{
	uint8_t buf[16];
	int ret;

	ret = flash_read(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, buf, sizeof(buf));
	zassert_equal(ret, 0, "flash unusable after IRQ suite [%d]", ret);
	LOG_INF("flash still responds after the interrupt DT sweep");
}

ZTEST_SUITE(test_ospi_irq, NULL, ospi_irq_setup, ospi_irq_before, NULL, NULL);

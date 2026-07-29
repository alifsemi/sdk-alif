/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <stdint.h>
#include <string.h>
#include <zephyr/drivers/spi.h>
#include <asm-generic/errno-base.h>
#include <soc_common.h>
#include <zephyr/logging/log.h>
#include <stdbool.h>

#include "spi_test.h"

LOG_MODULE_DECLARE(spi_pm, LOG_LEVEL_DBG);

#define Mhz 1000000
#define Khz 1000

/* SPI Nodes
 */

#define SPIDW_NODE   DT_ALIAS(master_spi)
#define S_SPIDW_NODE DT_ALIAS(slave_spi)

/* size of stack area used by each thread */
#define STACKSIZE 1024

/* default SPI master SS(slave select) is H/W controlled,
 * enable this to use as S/W controlled using gpio.
 */
#define SPI_MASTER_SS_SW_CONTROLLED_GPIO 0

/* Priorities.
 */
#define MASTER_PRIORITY 7
#define SLAVE_PRIORITY  6 /* High Priority */

/* delay between greetings (in ms) */
#define SLEEPTIME (1 * 1000)

K_THREAD_STACK_DEFINE(MasterT_stack, STACKSIZE);
static struct k_thread thread_m;

K_THREAD_STACK_DEFINE(SlaveT_stack, STACKSIZE);
static struct k_thread thread_s;

/* Master and Slave buffer size */
#define BUFF_SIZE 200

/* Master and Slave buffer word size */
#define SPI_WORD_SIZE 8

/* Master and Slave buffer frequency */
#define SPI_FREQUENCY (1 * Mhz)

/* Master and Slave buffer transfers */
#define SPI_NUM_TRANSFERS 50

/*  */
K_SEM_DEFINE(s_sem, 0, 1);
K_SEM_DEFINE(m_sem, 0, 1);

/* */
static volatile uint8_t THREAD_TO_BE_SUSPEND;
static volatile uint8_t THREAD_SUSPENDED;
static volatile bool spi_threads_started;
static volatile bool spi_threads_suspended;

/* Master and Slave buffers */
static uint32_t master_txdata[BUFF_SIZE] __aligned(4);
static uint32_t master_rxdata[BUFF_SIZE] __aligned(4);
static uint32_t slave_txdata[BUFF_SIZE] __aligned(4);
static uint32_t slave_rxdata[BUFF_SIZE] __aligned(4);

/*
 * Send/Receive data through slave spi
 */
int slave_spi_transceive(const struct device *dev)
{
	struct spi_config cnfg = {0};
	int ret;

	cnfg.frequency = SPI_FREQUENCY;
	cnfg.operation = SPI_OP_MODE_SLAVE | SPI_WORD_SET(SPI_WORD_SIZE);
	cnfg.slave = 0;

	int length = (BUFF_SIZE) * sizeof(slave_rxdata[0]);

	struct spi_buf rx_buf = {.buf = slave_rxdata, .len = length};
	struct spi_buf_set rx_bufset = {.buffers = &rx_buf, .count = 1};
	struct spi_buf tx_buf = {.buf = slave_txdata, .len = length};
	struct spi_buf_set tx_bufset = {.buffers = &tx_buf, .count = 1};

	LOG_DBG("slave Ry");

	ret = spi_transceive(dev, &cnfg, &tx_bufset, &rx_bufset);
	if (ret < 0) {
		LOG_ERR("ERROR: Slave SPI Transceive: %d", ret);
		return ret;
	}
	ret = memcmp(master_txdata, slave_rxdata, length);
	if (ret) {
		LOG_ERR("ERROR: SPI Master TX & Slave RX DATA NOT MATCHING: %d", ret);
	} else {
		LOG_DBG("SUCCESS: SPI Master TX & Slave RX DATA IS MATCHING: %d", ret);
	}

	return ret;
}

/*
 * Send/Receive data through master spi
 */
int master_spi_transceive(const struct device *dev, struct spi_cs_control *cs)
{
	struct spi_config cnfg = {0};
	int ret;

	cnfg.frequency = SPI_FREQUENCY;
	cnfg.operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(SPI_WORD_SIZE);
	cnfg.slave = 0;
	cnfg.cs = *cs;

	int length = (BUFF_SIZE) * sizeof(master_txdata[0]);

	struct spi_buf tx_buf = {.buf = master_txdata, .len = length};

	struct spi_buf_set tx_bufset = {.buffers = &tx_buf, .count = 1};

	struct spi_buf rx_buf = {.buf = master_rxdata, .len = length};
	struct spi_buf_set rx_bufset = {.buffers = &rx_buf, .count = 1};

	ret = spi_transceive(dev, &cnfg, &tx_bufset, &rx_bufset);
	if (ret) {
		LOG_ERR("ERROR: SPI=%p transceive: %d", dev, ret);
		return ret;
	}
	ret = memcmp(master_rxdata, slave_txdata, length);
	if (ret) {
		LOG_ERR("ERROR: SPI Master RX & Slave TX DATA NOT MATCHING: %d", ret);
		return ret;
	}
	LOG_DBG("SUCCESS: SPI Master RX & Slave TX DATA IS MATCHING: %d", ret);

	return ret;
}

static void master_spi(void *p1, void *p2, void *p3)
{
	const struct device *const dev = DEVICE_DT_GET(SPIDW_NODE);
	int ret;

	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	if (!device_is_ready(dev)) {
		LOG_DBG("%s: Master device not ready.", dev->name);
		return;
	}

#if SPI_MASTER_SS_SW_CONTROLLED_GPIO /* SPI master SS as S/W controlled using gpio */
	struct spi_cs_control cs_ctrl = (struct spi_cs_control){
		.gpio = GPIO_DT_SPEC_GET(SPIDW_NODE, cs_gpios),
		.delay = 100u, /* k_busy_wait(uint32_t usec_to_wait) */
	};
#else  /* SPI master SS as H/W controlled */
	struct spi_cs_control cs_ctrl = {0};
#endif /* SPI_MASTER_SS_SW_CONTROLLED_GPIO */

	while (1) {
		LOG_DBG("M: waiting for Slv");
		k_sem_take(&s_sem, K_FOREVER); /* wait for slave */
		k_msleep(100);
		LOG_DBG("M: got Slv");
		ret = master_spi_transceive(dev, &cs_ctrl);
		if (ret < 0) {
			LOG_ERR("Stopping the Master Thread due to error");
			k_msleep(100);
			continue;
		}
		LOG_DBG("M: Transffered");
		k_msleep(SLEEPTIME);
	}
	LOG_DBG("Master Transfer Successfully Completed");
}

static void slave_spi(void *p1, void *p2, void *p3)
{
	const struct device *const slave_dev = DEVICE_DT_GET(S_SPIDW_NODE);
	int ret;

	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	if (!device_is_ready(slave_dev)) {
		LOG_DBG("%s: Slave device not ready", slave_dev->name);
		return;
	}

	while (1) {
		if (THREAD_TO_BE_SUSPEND) {
			THREAD_SUSPENDED = 1;
			LOG_DBG("S: putting thread into wait for event");
			k_sem_take(&m_sem, K_FOREVER);
			LOG_DBG("S: Wokeup by application");
			THREAD_TO_BE_SUSPEND = 0;
			THREAD_SUSPENDED = 0;
		}

		k_sem_give(&s_sem); /* signal master */
		LOG_DBG("S: wokeup Mstr");
		ret = slave_spi_transceive(slave_dev);
		if (ret < 0) {
			LOG_ERR("Stopping the Slave Thread due to error");
			k_msleep(100); /* small delay before retry */
			continue;
		}
	}
	LOG_DBG("Slave Transfer Successfully Completed");
}

static void prepare_data(uint32_t *data, uint16_t def_mask)
{
	for (uint32_t cnt = 0; cnt < BUFF_SIZE; cnt++) {
		data[cnt] = (def_mask << 16) | cnt;
	}
}

int spi_pm_thread_init(void)
{
	prepare_data(master_txdata, 0xA5A5);
	prepare_data(slave_txdata, 0x5A5A);

	k_tid_t tids = k_thread_create(&thread_s, SlaveT_stack, STACKSIZE, &slave_spi, NULL, NULL,
				       NULL, SLAVE_PRIORITY, 0, K_FOREVER);
	if (tids == NULL) {
		LOG_ERR("Error creating Slave Thread");
		return -ENOMEM;
	}

	k_tid_t tidm = k_thread_create(&thread_m, MasterT_stack, STACKSIZE, &master_spi, NULL, NULL,
				       NULL, MASTER_PRIORITY, 0, K_FOREVER);
	if (tidm == NULL) {
		LOG_ERR("Error creating Master Thread");
		return -ENOMEM;
	}

	return 0;
};

int spi_pm_thread_start(void)
{
	if (spi_threads_started) {
		if (spi_threads_suspended) {
			return spi_pm_thread_resume();
		}
		LOG_INF("SPI threads already started");
		return 0;
	}

	THREAD_TO_BE_SUSPEND = 0;
	THREAD_SUSPENDED = 0;

	prepare_data(master_txdata, 0xA5A5);
	prepare_data(slave_txdata, 0x5A5A);
	memset(master_rxdata, 0, sizeof(master_rxdata));
	memset(slave_rxdata, 0, sizeof(slave_rxdata));

	k_sem_reset(&s_sem);
	k_sem_reset(&m_sem);

	k_thread_start(&thread_s);
	k_usleep(10 * 1000);
	k_thread_start(&thread_m);

	spi_threads_started = true;
	spi_threads_suspended = false;

	LOG_INF("SPI threads started");
	return 0;
}

int spi_pm_thread_suspend(void)
{
	if (!spi_threads_started) {
		return -EINVAL;
	}

	if (spi_threads_suspended) {
		LOG_DBG("SPI threads already suspended");
		return 0;
	}

	THREAD_TO_BE_SUSPEND = 1;
	THREAD_SUSPENDED = 0;

	do {
		k_usleep(100);
	} while (!THREAD_SUSPENDED);

	k_msleep(100);

	k_thread_suspend(&thread_s);
	k_thread_suspend(&thread_m);

	spi_threads_suspended = true;
	return 0;
}

int spi_pm_thread_resume(void)
{
	if (!spi_threads_started) {
		return -EINVAL;
	}

	if (!spi_threads_suspended) {
		LOG_DBG("SPI threads are not suspended, resume skipped");
		return 0;
	}

	THREAD_TO_BE_SUSPEND = 0;

	LOG_INF("Try to Resume...");
	k_sem_give(&m_sem);

	LOG_INF("wokeup main sem...");
	k_thread_resume(&thread_s);
	k_msleep(500);
	k_thread_resume(&thread_m);

	THREAD_SUSPENDED = 0;
	spi_threads_suspended = false;
	return 0;
}

int spi_pm_thread_stop(void)
{
	if (!spi_threads_started) {
		LOG_DBG("SPI threads not started, stop skipped");
		return 0;
	}

	if (!spi_threads_suspended) {
		spi_pm_thread_suspend();
	}

	k_thread_abort(&thread_s);
	k_thread_abort(&thread_m);

	THREAD_TO_BE_SUSPEND = 0;
	THREAD_SUSPENDED = 0;
	spi_threads_started = false;
	spi_threads_suspended = false;

	LOG_INF("SPI threads aborted");
	return 0;
}

bool spi_pm_thread_is_started(void)
{
	return spi_threads_started;
}

bool spi_pm_thread_is_suspended(void)
{
	return spi_threads_suspended;
}

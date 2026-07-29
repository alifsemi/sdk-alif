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

/* SPI Nodes */
#define SPIDW_NODE   DT_ALIAS(controller_spi)
#define S_SPIDW_NODE DT_ALIAS(peripheral_spi)

/* size of stack area used by each thread */
#define STACKSIZE 1024

/* default SPI controller CS is H/W controlled,
 * enable this to use as S/W controlled using gpio.
 */
#define SPI_CONTROLLER_CS_SW_CONTROLLED_GPIO 0

/* Priorities */
#define CONTROLLER_PRIORITY 7
#define PERIPHERAL_PRIORITY 6 /* High Priority */

/* delay between greetings (in ms) */
#define SLEEPTIME (1 * 1000)

K_THREAD_STACK_DEFINE(ControllerT_stack, STACKSIZE);
static struct k_thread thread_m;

K_THREAD_STACK_DEFINE(PeripheralT_stack, STACKSIZE);
static struct k_thread thread_s;

/* Controller and Peripheral buffer size */
#define BUFF_SIZE 200

/* Controller and Peripheral buffer word size */
#define SPI_WORD_SIZE 8

/* Controller and Peripheral buffer frequency */
#define SPI_FREQUENCY (1 * Mhz)

K_SEM_DEFINE(s_sem, 0, 1);
K_SEM_DEFINE(m_sem, 0, 1);

static volatile uint8_t THREAD_TO_BE_SUSPEND;
static volatile uint8_t THREAD_SUSPENDED;
static volatile bool spi_threads_started;
static volatile bool spi_threads_suspended;

/* Controller and Peripheral buffers */
static uint32_t controller_txdata[BUFF_SIZE] __aligned(4);
static uint32_t controller_rxdata[BUFF_SIZE] __aligned(4);
static uint32_t peripheral_txdata[BUFF_SIZE] __aligned(4);
static uint32_t peripheral_rxdata[BUFF_SIZE] __aligned(4);

/*
 * Send/Receive data through peripheral SPI
 */
static int peripheral_spi_transceive(const struct device *dev)
{
	struct spi_config cnfg = {0};
	int ret;

	cnfg.frequency = SPI_FREQUENCY;
	cnfg.operation = SPI_OP_MODE_SLAVE | SPI_WORD_SET(SPI_WORD_SIZE);
	cnfg.slave = 0;

	int length = (BUFF_SIZE) * sizeof(peripheral_rxdata[0]);

	struct spi_buf rx_buf = {.buf = peripheral_rxdata, .len = length};
	struct spi_buf_set rx_bufset = {.buffers = &rx_buf, .count = 1};
	struct spi_buf tx_buf = {.buf = peripheral_txdata, .len = length};
	struct spi_buf_set tx_bufset = {.buffers = &tx_buf, .count = 1};

	LOG_DBG("peripheral ready");

	ret = spi_transceive(dev, &cnfg, &tx_bufset, &rx_bufset);
	if (ret < 0) {
		LOG_ERR("ERROR: Peripheral SPI Transceive: %d", ret);
		return ret;
	}
	ret = memcmp(controller_txdata, peripheral_rxdata, length);
	if (ret) {
		LOG_ERR("ERROR: SPI Controller TX & Peripheral RX DATA NOT MATCHING: %d",
			ret);
	} else {
		LOG_DBG("SUCCESS: SPI Controller TX & Peripheral RX DATA IS MATCHING: %d",
			ret);
	}

	return ret;
}

/*
 * Send/Receive data through controller SPI
 */
static int controller_spi_transceive(const struct device *dev, struct spi_cs_control *cs)
{
	struct spi_config cnfg = {0};
	int ret;

	cnfg.frequency = SPI_FREQUENCY;
	cnfg.operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(SPI_WORD_SIZE);
	cnfg.slave = 0;
	cnfg.cs = *cs;

	int length = (BUFF_SIZE) * sizeof(controller_txdata[0]);

	struct spi_buf tx_buf = {.buf = controller_txdata, .len = length};
	struct spi_buf_set tx_bufset = {.buffers = &tx_buf, .count = 1};
	struct spi_buf rx_buf = {.buf = controller_rxdata, .len = length};
	struct spi_buf_set rx_bufset = {.buffers = &rx_buf, .count = 1};

	ret = spi_transceive(dev, &cnfg, &tx_bufset, &rx_bufset);
	if (ret) {
		LOG_ERR("ERROR: SPI=%p transceive: %d", dev, ret);
		return ret;
	}
	ret = memcmp(controller_rxdata, peripheral_txdata, length);
	if (ret) {
		LOG_ERR("ERROR: SPI Controller RX & Peripheral TX DATA NOT MATCHING: %d",
			ret);
		return ret;
	}
	LOG_DBG("SUCCESS: SPI Controller RX & Peripheral TX DATA IS MATCHING: %d",
		ret);

	return ret;
}

static void controller_spi(void *p1, void *p2, void *p3)
{
	const struct device *const dev = DEVICE_DT_GET(SPIDW_NODE);
	int ret;

	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	if (!device_is_ready(dev)) {
		LOG_DBG("%s: Controller device not ready.", dev->name);
		return;
	}

#if SPI_CONTROLLER_CS_SW_CONTROLLED_GPIO /* SPI controller CS as S/W controlled using gpio */
	struct spi_cs_control cs_ctrl = (struct spi_cs_control){
		.gpio = GPIO_DT_SPEC_GET(SPIDW_NODE, cs_gpios),
		.delay = 100u, /* k_busy_wait(uint32_t usec_to_wait) */
	};
#else  /* SPI controller CS as H/W controlled */
	struct spi_cs_control cs_ctrl = {0};
#endif /* SPI_CONTROLLER_CS_SW_CONTROLLED_GPIO */

	while (1) {
		LOG_DBG("C: waiting for Periph");
		k_sem_take(&s_sem, K_FOREVER); /* wait for peripheral */
		k_msleep(100);
		LOG_DBG("C: got Periph");
		ret = controller_spi_transceive(dev, &cs_ctrl);
		if (ret < 0) {
			LOG_ERR("Stopping the Controller Thread due to error");
			k_msleep(100);
			continue;
		}
		LOG_DBG("C: Transferred");
		k_msleep(SLEEPTIME);
	}
	LOG_DBG("Controller Transfer Successfully Completed");
}

static void peripheral_spi(void *p1, void *p2, void *p3)
{
	const struct device *const peripheral_dev = DEVICE_DT_GET(S_SPIDW_NODE);
	int ret;

	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	if (!device_is_ready(peripheral_dev)) {
		LOG_DBG("%s: Peripheral device not ready", peripheral_dev->name);
		return;
	}

	while (1) {
		if (THREAD_TO_BE_SUSPEND) {
			THREAD_SUSPENDED = 1;
			LOG_DBG("P: putting thread into wait for event");
			k_sem_take(&m_sem, K_FOREVER);
			LOG_DBG("P: Wokeup by application");
			THREAD_TO_BE_SUSPEND = 0;
			THREAD_SUSPENDED = 0;
		}

		k_sem_give(&s_sem); /* signal controller */
		LOG_DBG("P: wokeup Ctrl");
		ret = peripheral_spi_transceive(peripheral_dev);
		if (ret < 0) {
			LOG_ERR("Stopping the Peripheral Thread due to error");
			k_msleep(100); /* small delay before retry */
			continue;
		}
	}
	LOG_DBG("Peripheral Transfer Successfully Completed");
}

static void prepare_data(uint32_t *data, uint16_t def_mask)
{
	for (uint32_t cnt = 0; cnt < BUFF_SIZE; cnt++) {
		data[cnt] = (def_mask << 16) | cnt;
	}
}

int spi_pm_thread_init(void)
{
	k_tid_t tids = k_thread_create(&thread_s, PeripheralT_stack, STACKSIZE, &peripheral_spi,
				       NULL, NULL, NULL, PERIPHERAL_PRIORITY, 0, K_FOREVER);
	if (tids == NULL) {
		LOG_ERR("Error creating Peripheral Thread");
		return -ENOMEM;
	}

	k_tid_t tidm = k_thread_create(&thread_m, ControllerT_stack, STACKSIZE, &controller_spi,
				       NULL, NULL, NULL, CONTROLLER_PRIORITY, 0, K_FOREVER);
	if (tidm == NULL) {
		LOG_ERR("Error creating Controller Thread");
		return -ENOMEM;
	}

	return 0;
}

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

	prepare_data(controller_txdata, 0xA5A5);
	prepare_data(peripheral_txdata, 0x5A5A);
	memset(controller_rxdata, 0, sizeof(controller_rxdata));
	memset(peripheral_rxdata, 0, sizeof(peripheral_rxdata));

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

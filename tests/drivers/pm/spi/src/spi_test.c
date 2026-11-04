/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#include <string.h>
#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/pm/policy.h>
#include <zephyr/sys/util.h>
#include <zephyr/logging/log.h>
#include <zephyr/cache.h>

#include "spi_test.h"

LOG_MODULE_DECLARE(spi_pm, LOG_LEVEL_INF);

/* controller-spi / peripheral-spi aliases come from the snippet overlays. */
#define SPIDW_NODE   DT_ALIAS(controller_spi)
#define S_SPIDW_NODE DT_ALIAS(peripheral_spi)

#if !DT_NODE_HAS_STATUS(SPIDW_NODE, okay) || \
	!DT_NODE_HAS_STATUS(S_SPIDW_NODE, okay)
#error "controller-spi / peripheral-spi aliases missing"
#endif

#define STACKSIZE 1024

/* Default SPI controller chip-select is hardware controlled.
 * Enable this to drive chip-select from a GPIO.
 */
#define SPI_CONTROLLER_CS_SW_CONTROLLED_GPIO 0

/* Controller must outrank the peripheral for on-board loopback. */
#define CONTROLLER_PRIORITY 6
#define PERIPHERAL_PRIORITY 7

#define BUFF_SIZE     200
#define SPI_WORD_SIZE 8
#define Mhz           1000000
#define SPI_FREQUENCY (1 * Mhz)

/* Transfers per round and controller gap between transfers. */
#define SPI_NUM_TRANSFERS   5
#define SPI_TRANSFER_GAP_MS 200

/* Peripheral is armed this long before the controller starts clocking. */
#define SPI_PERIPHERAL_ARM_MS 100

#define SPI_ROUND_TIMEOUT \
	K_MSEC(SPI_PERIPHERAL_ARM_MS + \
	       SPI_NUM_TRANSFERS * (SPI_TRANSFER_GAP_MS + 500) + 1000)

static K_SEM_DEFINE(peripheral_start_sem, 0, 1);
static K_SEM_DEFINE(controller_start_sem, 0, 1);
static K_SEM_DEFINE(peripheral_done_sem, 0, 1);
static K_SEM_DEFINE(controller_done_sem, 0, 1);

static int controller_result;
static int peripheral_result;
static bool spi_threads_started;
static bool spi_xfer_in_flight;

static uint32_t controller_txdata[BUFF_SIZE];
static uint32_t controller_rxdata[BUFF_SIZE];
static uint32_t peripheral_txdata[BUFF_SIZE];
static uint32_t peripheral_rxdata[BUFF_SIZE];

K_THREAD_STACK_DEFINE(controller_stack, STACKSIZE);
static struct k_thread controller_thread;

K_THREAD_STACK_DEFINE(peripheral_stack, STACKSIZE);
static struct k_thread peripheral_thread;

/*
 * SPI/DMA IRQs are outside the IWIC wakeup range, so every state deeper
 * than RUNTIME_IDLE must be blocked while a transfer is in flight.
 */
static void spi_pm_lock_sleep_states(bool lock)
{
	if (lock) {
		pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE,
					 PM_ALL_SUBSTATES);
		pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_RAM,
					 PM_ALL_SUBSTATES);
		pm_policy_state_lock_get(PM_STATE_SOFT_OFF,
					 PM_ALL_SUBSTATES);
	} else {
		pm_policy_state_lock_put(PM_STATE_SOFT_OFF,
					 PM_ALL_SUBSTATES);
		pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_RAM,
					 PM_ALL_SUBSTATES);
		pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_IDLE,
					 PM_ALL_SUBSTATES);
	}
}

static int peripheral_spi_transceive(const struct device *dev)
{
	struct spi_config cnfg = {0};
	int ret;

	cnfg.frequency = SPI_FREQUENCY;
	cnfg.operation = SPI_OP_MODE_SLAVE | SPI_WORD_SET(SPI_WORD_SIZE);
	cnfg.slave = 0;

	int length = (BUFF_SIZE) * sizeof(peripheral_rxdata[0]);

	struct spi_buf rx_buf = {
		.buf = peripheral_rxdata,
		.len = length
	};
	struct spi_buf_set rx_bufset = {
		.buffers = &rx_buf,
		.count = 1
	};
	struct spi_buf tx_buf = {
		.buf = peripheral_txdata,
		.len = length
	};
	struct spi_buf_set tx_bufset = {
		.buffers = &tx_buf,
		.count = 1
	};

	sys_cache_data_flush_range(peripheral_txdata, length);
	sys_cache_data_flush_and_invd_range(peripheral_rxdata, length);

	ret = spi_transceive(dev, &cnfg, &tx_bufset, &rx_bufset);
	if (ret < 0) {
		LOG_ERR("Peripheral SPI transceive error: %d", ret);
	}

	sys_cache_data_invd_range(peripheral_rxdata, length);

	LOG_INF("peripheral wrote: %08x %08x %08x %08x %08x",
		peripheral_txdata[0], peripheral_txdata[1],
		peripheral_txdata[2], peripheral_txdata[3],
		peripheral_txdata[4]);

	LOG_INF("peripheral read:  %08x %08x %08x %08x %08x",
		peripheral_rxdata[0], peripheral_rxdata[1],
		peripheral_rxdata[2], peripheral_rxdata[3],
		peripheral_rxdata[4]);

	/* Check the transceive result before comparing RX data. */
	if (ret < 0) {
		return ret;
	}

	if (memcmp(controller_txdata, peripheral_rxdata, length) != 0) {
		LOG_ERR("SPI controller TX & peripheral RX mismatch");
		return -EIO;
	}

	LOG_INF("SUCCESS: SPI controller TX & peripheral RX match");
	return 0;
}

static int controller_spi_transceive(const struct device *dev,
				     struct spi_cs_control *cs)
{
	struct spi_config cnfg = {0};
	int ret;

	cnfg.frequency = SPI_FREQUENCY;
	cnfg.operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(SPI_WORD_SIZE);
	cnfg.slave = 0;
	cnfg.cs = *cs;

	int length = (BUFF_SIZE) * sizeof(controller_txdata[0]);

	struct spi_buf tx_buf = {
		.buf = controller_txdata,
		.len = length
	};
	struct spi_buf_set tx_bufset = {
		.buffers = &tx_buf,
		.count = 1
	};
	struct spi_buf rx_buf = {
		.buf = controller_rxdata,
		.len = length
	};
	struct spi_buf_set rx_bufset = {
		.buffers = &rx_buf,
		.count = 1
	};

	sys_cache_data_flush_range(controller_txdata, length);
	sys_cache_data_flush_and_invd_range(controller_rxdata, length);

	ret = spi_transceive(dev, &cnfg, &tx_bufset, &rx_bufset);
	if (ret < 0) {
		LOG_ERR("SPI=%s transceive error: %d", dev->name, ret);
	}

	sys_cache_data_invd_range(controller_rxdata, length);

	LOG_INF("controller wrote:   %08x %08x %08x %08x %08x",
		controller_txdata[0], controller_txdata[1],
		controller_txdata[2], controller_txdata[3],
		controller_txdata[4]);
	LOG_INF("controller receive: %08x %08x %08x %08x %08x",
		controller_rxdata[0], controller_rxdata[1],
		controller_rxdata[2], controller_rxdata[3],
		controller_rxdata[4]);

	/* Check the transceive result before comparing RX data. */
	if (ret < 0) {
		return ret;
	}

	if (memcmp(controller_rxdata, peripheral_txdata, length) != 0) {
		LOG_ERR("SPI controller RX & peripheral TX mismatch");
		return -EIO;
	}

	LOG_INF("SUCCESS: SPI controller RX & peripheral TX match");
	return 0;
}

static void prepare_data(uint32_t *data, uint16_t def_mask)
{
	for (uint32_t cnt = 0; cnt < BUFF_SIZE; cnt++) {
		data[cnt] = ((uint32_t)def_mask << 16) | cnt;
	}
}

static void controller_spi_thread(void *p1, void *p2, void *p3)
{
	const struct device *const dev = DEVICE_DT_GET(SPIDW_NODE);

	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

#if SPI_CONTROLLER_CS_SW_CONTROLLED_GPIO
	struct spi_cs_control cs_ctrl = (struct spi_cs_control){
		.gpio  = GPIO_DT_SPEC_GET(SPIDW_NODE, cs_gpios),
		.delay = 100u,
	};
#else
	struct spi_cs_control cs_ctrl = {0};
#endif

	while (true) {
		int result = 0;

		k_sem_take(&controller_start_sem, K_FOREVER);
		for (uint32_t i = 0; i < SPI_NUM_TRANSFERS; i++) {
			LOG_INF("Controller transceive iter= %u", i + 1);
			result = controller_spi_transceive(dev, &cs_ctrl);
			if (result < 0) {
				LOG_ERR("Stopping controller round, error");
				break;
			}
			k_msleep(SPI_TRANSFER_GAP_MS);
		}
		if (result == 0) {
			LOG_INF("Controller transfer completed");
		}
		controller_result = result;
		k_sem_give(&controller_done_sem);
	}
}

static void peripheral_spi_thread(void *p1, void *p2, void *p3)
{
	const struct device *const peripheral_dev =
		DEVICE_DT_GET(S_SPIDW_NODE);

	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	while (true) {
		int result = 0;

		k_sem_take(&peripheral_start_sem, K_FOREVER);
		for (uint32_t i = 0; i < SPI_NUM_TRANSFERS; i++) {
			LOG_INF("Peripheral transceive iter= %u", i + 1);
			result = peripheral_spi_transceive(peripheral_dev);
			if (result < 0) {
				LOG_ERR("Stopping peripheral round, error");
				break;
			}
		}
		if (result == 0) {
			LOG_INF("Peripheral transfer completed");
		}
		peripheral_result = result;
		k_sem_give(&peripheral_done_sem);
	}
}

int spi_test_init(void)
{
	const struct device *const dev = DEVICE_DT_GET(SPIDW_NODE);
	const struct device *const peripheral_dev =
		DEVICE_DT_GET(S_SPIDW_NODE);

	if (spi_threads_started) {
		return 0;
	}

	if (!device_is_ready(dev)) {
		LOG_ERR("%s: Controller device not ready", dev->name);
		return -ENODEV;
	}

	if (!device_is_ready(peripheral_dev)) {
		LOG_ERR("%s: Peripheral not ready", peripheral_dev->name);
		return -ENODEV;
	}

	k_thread_create(&peripheral_thread, peripheral_stack, STACKSIZE,
			peripheral_spi_thread, NULL, NULL, NULL,
			PERIPHERAL_PRIORITY, 0, K_NO_WAIT);
	k_thread_name_set(&peripheral_thread, "spi_peripheral");

	k_thread_create(&controller_thread, controller_stack, STACKSIZE,
			controller_spi_thread, NULL, NULL, NULL,
			CONTROLLER_PRIORITY, 0, K_NO_WAIT);
	k_thread_name_set(&controller_thread, "spi_controller");

	spi_threads_started = true;
	LOG_INF("SPI threads started (controller %s, peripheral %s)",
		dev->name, peripheral_dev->name);

	return 0;
}

int spi_loopback_run(const char *phase_label)
{
	int ret = 0;

	if (!spi_threads_started) {
		return -ENODEV;
	}
	if (spi_xfer_in_flight) {
		LOG_ERR("[SPI Test] %s: skipped, prior transfer still "
			"in flight after timeout",
			phase_label);
		return -EBUSY;
	}

	spi_pm_lock_sleep_states(true);
	LOG_INF("[SPI Test] %s", phase_label);

	k_sem_reset(&peripheral_done_sem);
	k_sem_reset(&controller_done_sem);
	controller_result = -EINPROGRESS;
	peripheral_result = -EINPROGRESS;

	prepare_data(controller_txdata, 0xA5A5);
	prepare_data(peripheral_txdata, 0x5A5A);
	memset(controller_rxdata, 0, sizeof(controller_rxdata));
	memset(peripheral_rxdata, 0, sizeof(peripheral_rxdata));

	k_sem_give(&peripheral_start_sem);
	k_msleep(SPI_PERIPHERAL_ARM_MS);
	k_sem_give(&controller_start_sem);

	if (k_sem_take(&peripheral_done_sem, SPI_ROUND_TIMEOUT) != 0) {
		LOG_ERR("[SPI Test] %s: peripheral timeout", phase_label);
		ret = -ETIMEDOUT;
	}
	if (k_sem_take(&controller_done_sem, SPI_ROUND_TIMEOUT) != 0) {
		LOG_ERR("[SPI Test] %s: controller timeout", phase_label);
		ret = -ETIMEDOUT;
	}

	if (ret == -ETIMEDOUT) {
		/*
		 * DMA/SPI IRQ may still be pending. Leave PM locks held
		 * so a late completion cannot wake from S2RAM/SOFT_OFF.
		 */
		spi_xfer_in_flight = true;
		LOG_ERR("[SPI Test] %s: keeping sleep-state locks; "
			"later SPI rounds will be skipped",
			phase_label);
		return ret;
	}

	spi_xfer_in_flight = false;
	spi_pm_lock_sleep_states(false);

	if (ret == 0 && (controller_result != 0 || peripheral_result != 0)) {
		LOG_ERR("[SPI Test] %s: FAILED (ctrl %d, peri %d)",
			phase_label, controller_result, peripheral_result);
		ret = -EIO;
	}

	if (ret == 0) {
		LOG_INF("[SPI Test] %s: PASS", phase_label);
	}

	return ret;
}

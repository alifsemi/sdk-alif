/*
 * Copyright (c) 2023 Alif Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "hci_uart.h"

#include "ble_api.h"
#include <stdlib.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/sys/ring_buffer.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/logging/log.h>
#include <zephyr/pm/pm.h>
#include <zephyr/pm/policy.h>
#include <zephyr/sys/poweroff.h>

/* Register HCI UART log module with standard UART log level */
LOG_MODULE_REGISTER(hci_uart, CONFIG_UART_LOG_LEVEL);

#include "es0_power_manager.h"

/* change this to any other UART peripheral if desired */
#define UART_DEVICE_NODE DT_CHOSEN(zephyr_hci_uart)
static const struct device *uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);

/*
 * STRUCT DEFINITIONS
 *****************************************************************************************
 */
/* TX and RX channel class holding data used for asynchronous read and write
 * data transactions
 */
/* UART TX RX Channel */
struct uart_txrxchannel {
	/* call back function pointer */
	void (*callback)(void *a, uint8_t b);
	/* Dummy data pointer returned to callback when operation is over. */
	void *dummy;
};

/* UART environment structure */
struct uart_env_tag {
	/* tx channel */
	struct uart_txrxchannel tx;
	/* rx channel */
	struct uart_txrxchannel rx;
	/* error detect */
	uint8_t errordetect;
	/* external wakeup */
	bool ext_wakeup;
	/* Flag to track if RX is active */
	bool rx_enabled;
};

/* receive buffer used in UART callback */
static uint8_t *rx_buf_ptr __noinit;
static uint32_t rx_buf_size __noinit;
static uint32_t rx_buf_len __noinit;

/* uart environment structure */
static struct uart_env_tag uart_env __noinit;

void hci_uart_callback(const struct device *dev, void *user_data)
{
	if (!uart_irq_update(uart_dev)) {
		return;
	}

	void (*callback)(void *, uint8_t) = NULL;
	void *data = NULL;

	while (uart_irq_rx_ready(uart_dev) && rx_buf_len < rx_buf_size) {
		int read_bytes = uart_fifo_read(uart_dev, rx_buf_ptr + rx_buf_len, 1);

		rx_buf_len += read_bytes;
	}

	if (rx_buf_len == rx_buf_size) {
		uart_irq_rx_disable(uart_dev);
		/* Retrieve callback pointer */
		callback = uart_env.rx.callback;
		data = uart_env.rx.dummy;

		if (callback != NULL) {
			/* Clear callback pointer */
			uart_env.rx.callback = NULL;
			uart_env.rx.dummy = NULL;

			/* Call handler */
			callback(data, ITF_STATUS_OK);
		}
	}

	/* TODO error/overflow handling */
}

/**
 * @brief Initialize the HCI UART interface
 *
 * @return 0 on success, negative error code otherwise
 */
int32_t hci_uart_init(void)
{
	/* Get the UART device */
	uart_dev = DEVICE_DT_GET_OR_NULL(DT_ALIAS(uart_hci));

	if (!uart_dev) {
		/* Try to get the device by nodelabel as fallback */
		uart_dev = DEVICE_DT_GET_OR_NULL(DT_NODELABEL(uart_hci));
	}

	if (!device_is_ready(uart_dev)) {
		LOG_ERR("UART device not found or not ready!");
		return -ENODEV;
	}

	uart_irq_rx_enable(uart_dev);
	uart_irq_callback_user_data_set(uart_dev, hci_uart_callback, NULL);

	/* We cannot initialize RX transfer callback here
	 * as that might be kept in retention and also when
	 * read operation is started a (new) callback is always set.
	 */
	uart_env.tx.callback = NULL;
	return 0;
}

void hci_uart_read(uint8_t *bufptr, uint32_t size, void (*callback)(void *, uint8_t), void *dummy)
{
	LOG_DBG("HCI UART read request: %d bytes", size);

	__ASSERT(bufptr != NULL, "Invalid buffer pointer");
	__ASSERT(size != 0, "Invalid size");
	__ASSERT(callback != NULL, "Invalid callback");

	/* Store callback and user data */
	uart_env.rx.callback = callback;
	uart_env.rx.dummy = dummy;

	rx_buf_ptr = bufptr;
	rx_buf_size = size;
	rx_buf_len = 0;
	uart_irq_rx_enable(uart_dev);
}

void hci_uart_write(uint8_t *bufptr, uint32_t size, void (*callback)(void *, uint8_t), void *dummy)
{
#if CONFIG_ALIF_BLE_ALLOW_SLEEP_RUNTIME
	/* Disable ES0 sleep while writing commands */
	uart_line_ctrl_set(uart_dev, UART_LINE_CTRL_BRK, 0);
#endif

	LOG_DBG("HCI UART write request: %d bytes", size);

	__ASSERT(bufptr != NULL, "Invalid buffer pointer");
	__ASSERT(size != 0, "Invalid size");
	__ASSERT(callback != NULL, "Invalid callback");
	enum itf_status if_status = ITF_STATUS_OK;

	/* Deassert&assert rts_n, falling edge triggers wake up the RF core */
	wake_es0(uart_dev);

	uart_env.tx.callback = callback;
	uart_env.tx.dummy = dummy;

	if (IS_ENABLED(CONFIG_PM)) {
		pm_policy_state_lock_get(PM_STATE_SOFT_OFF, PM_ALL_SUBSTATES);
	}

	for (int i = 0; i < size; i++) {
		uart_poll_out(uart_dev, bufptr[i]);
	}

	if (IS_ENABLED(CONFIG_PM)) {
		pm_policy_state_lock_put(PM_STATE_SOFT_OFF, PM_ALL_SUBSTATES);
	}

	if (callback != NULL) {
		/* Clear callback pointer */
		uart_env.tx.callback = NULL;
		uart_env.tx.dummy = NULL;
		/* Call handler */
		callback(dummy, if_status);
	}

#if CONFIG_ALIF_BLE_ALLOW_SLEEP_RUNTIME
	/* Allow ES0 to sleep */
	uart_line_ctrl_set(uart_dev, UART_LINE_CTRL_BRK, 1);
#endif
}

void hci_uart_flow_on(void)
{
	/* Not supported */
	/* TODO */
}

bool hci_uart_flow_off(void)
{
	/* Not supported */
	/* TODO */
	return true;
}

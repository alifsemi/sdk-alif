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
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>
#include <string.h>

LOG_MODULE_REGISTER(echo_dma, LOG_LEVEL_INF);

#define UART_NODE DT_CHOSEN(zephyr_shell_uart)

#define RX_TIMEOUT       1000
#define RX_DMA_BUF_SIZE  64
#define TX_DMA_BUF_SIZE  64

static const struct device *uart_dev = DEVICE_DT_GET(UART_NODE);
static const struct device *console_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

static uint8_t rx_dma_buf_a[RX_DMA_BUF_SIZE];
static uint8_t rx_dma_buf_b[RX_DMA_BUF_SIZE];
static uint8_t *next_rx_buf = rx_dma_buf_b;
static uint8_t tx_dma_buf[TX_DMA_BUF_SIZE];

static size_t tx_len;
static volatile bool truncated;
static bool prev_was_cr;

static struct k_sem line_ready;
static struct k_sem tx_done;

/*
 * Print character by character to the UART interface
 */
static void print_uart(const char *buf)
{
	while (*buf) {
		uart_poll_out(console_dev, *buf++);
	}
}

/*
 * The application receives data over Shell UART using DMA and
 * echoes the received line back after the user presses the Enter key.
 */
static void uart_cb(const struct device *dev,
					struct uart_event *evt,
					void *user_data)
{
	ARG_UNUSED(user_data);

	switch (evt->type) {

	case UART_RX_RDY: {
		uint8_t *data = &evt->data.rx.buf[evt->data.rx.offset];
		size_t len = evt->data.rx.len;

		for (size_t i = 0; i < len; i++) {
			uint8_t c = data[i];

			/* Collapse \r\n, \n\r into single line-end */
			if (c == '\n' && prev_was_cr) {
				prev_was_cr = false;
				continue;
			}
			prev_was_cr = (c == '\r');

			/* Store everything (including \r or \n) */
			if (tx_len < TX_DMA_BUF_SIZE) {
				tx_dma_buf[tx_len++] = c;
			} else {
				truncated = true;
			}

			/* Detect line-end / ENTER */
			if (c == '\r' || c == '\n') {
				k_sem_give(&line_ready);
			}
		}
		break;
	}

	case UART_TX_DONE:
		LOG_DBG("TX_DONE");
		k_sem_give(&tx_done);
		break;

	case UART_TX_ABORTED:
		LOG_WRN("TX_ABORTED");
		k_sem_give(&tx_done);
		break;

	case UART_RX_BUF_REQUEST:
		LOG_DBG("BUF_REQ");
		uart_rx_buf_rsp(dev, next_rx_buf, RX_DMA_BUF_SIZE);
		next_rx_buf = (next_rx_buf == rx_dma_buf_a)
			    ? rx_dma_buf_b : rx_dma_buf_a;
		break;

	case UART_RX_DISABLED:
		LOG_WRN("RX_DISABLED -> re-enabling");
		uart_rx_enable(dev, next_rx_buf, RX_DMA_BUF_SIZE, RX_TIMEOUT);
		next_rx_buf = (next_rx_buf == rx_dma_buf_a)
			    ? rx_dma_buf_b : rx_dma_buf_a;
		break;

	default:
		break;
	}
}

/* small helper API for DMA TX */
static int dma_uart_send(const uint8_t *data, size_t len)
{
	int ret;

	ret = uart_tx(uart_dev, data, len, SYS_FOREVER_US);
	if (ret < 0) {
		return ret;
	}

	/* wait until DMA TX finishes */
	k_sem_take(&tx_done, K_FOREVER);

	return 0;
}

int main(void)
{
	int ret = 0;

	if (!device_is_ready(uart_dev)) {
		print_uart("UART device not ready\n");
		return 0;
	}

	print_uart("\r\nUART DMA Echo Test\r\n");

	k_sem_init(&line_ready, 0, 10);
	k_sem_init(&tx_done, 0, 1);

	ret = uart_callback_set(uart_dev, uart_cb, NULL);
	if (ret < 0) {
		print_uart("Error setting UART callback\r\n");
		return ret;
	}

	print_uart("Enabling UART RX DMA\r\n");

	ret = uart_rx_enable(uart_dev, rx_dma_buf_a, RX_DMA_BUF_SIZE, RX_TIMEOUT);
	if (ret < 0) {
		print_uart("Error enabling UART RX DMA\r\n");
		return ret;
	}

	print_uart("Type anything on Shell UART and press ENTER!\r\n");

	while (1) {
		/* indefinitely wait for the user input */
		k_sem_take(&line_ready, K_FOREVER);

		if (tx_len > 0) {
			uint8_t echo_buf[TX_DMA_BUF_SIZE + 10];
			size_t echo_len;
			size_t pos = 0;
			unsigned int key;

			/* snapshot the RX buffer under IRQ lock */
			key = irq_lock();
			echo_len = (tx_len > TX_DMA_BUF_SIZE)
				 ? TX_DMA_BUF_SIZE : tx_len;

			/* Build single contiguous TX: "Echo:\r\n" + received data + "\r\n" */
			memcpy(&echo_buf[pos], "Echo:\r\n", 7);
			pos += 7;
			memcpy(&echo_buf[pos], tx_dma_buf, echo_len);
			pos += echo_len;
			echo_buf[pos++] = '\r';
			echo_buf[pos++] = '\n';
			tx_len = 0;
			irq_unlock(key);

			/* Single DMA send for the entire echo response */
			ret = dma_uart_send(echo_buf, pos);
			if (ret < 0) {
				LOG_ERR("DMA TX err: %d", ret);
			}

			if (truncated) {
				truncated = false;
				LOG_WRN("input truncated to 64 bytes!");
			}
		}
	}
	return 0;
}

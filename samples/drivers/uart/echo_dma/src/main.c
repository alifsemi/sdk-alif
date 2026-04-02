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
#include <string.h>

#define UART_NODE DT_CHOSEN(zephyr_shell_uart)

#define RX_TIMEOUT       50
#define RX_DMA_BUF_SIZE  64
#define TX_DMA_BUF_SIZE  64

static const struct device *uart_dev = DEVICE_DT_GET(UART_NODE);

static uint8_t rx_dma_buf[RX_DMA_BUF_SIZE];
static uint8_t tx_dma_buf[TX_DMA_BUF_SIZE];

static size_t tx_len;

static struct k_sem line_ready;
static struct k_sem tx_done;

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

			if (tx_len < TX_DMA_BUF_SIZE) {
				tx_dma_buf[tx_len++] = c;
			}

			/* detect ENTER */
			if (c == '\r' || c == '\n') {
				k_sem_give(&line_ready);
			}
		}
		break;
	}

	case UART_TX_DONE:
		printk("DMA TX done\n");
		k_sem_give(&tx_done);
		break;

	case UART_TX_ABORTED:
		printk("DMA TX aborted\n");
		k_sem_give(&tx_done);
		break;

	case UART_RX_BUF_REQUEST:
		printk("RX buffer request\n");
		uart_rx_buf_rsp(dev, rx_dma_buf, sizeof(rx_dma_buf));
		break;

	case UART_RX_DISABLED:
		printk("RX disabled -> re-enabling DMA RX\n");
		uart_rx_enable(dev, rx_dma_buf, sizeof(rx_dma_buf), RX_TIMEOUT);
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
		printk("UART device not ready\n");
		return 0;
	}

	printk("\nUART DMA Echo Test\n");

	k_sem_init(&line_ready, 0, 1);
	k_sem_init(&tx_done, 0, 1);

	ret = uart_callback_set(uart_dev, uart_cb, NULL);
	if (ret < 0) {
		printk("Error setting UART callback: %d\n", ret);
		return ret;
	}

	printk("Enabling UART RX DMA\n");

	ret = uart_rx_enable(uart_dev, rx_dma_buf, sizeof(rx_dma_buf), RX_TIMEOUT);
	if (ret < 0) {
		printk("Error enabling UART RX DMA: %d\n", ret);
		return ret;
	}

	printk("Type anything on Shell UART and press ENTER! \r\n");

	while (1) {
		/* indefinitely wait for the user input */
		k_sem_take(&line_ready, K_FOREVER);

		if (tx_len > 0) {
			uint8_t echo_buf[TX_DMA_BUF_SIZE];
			size_t echo_len;
			unsigned int key;

			/* snapshot the RX buffer under IRQ lock */
			key = irq_lock();
			echo_len = tx_len;
			memcpy(echo_buf, tx_dma_buf, echo_len);
			tx_len = 0;
			irq_unlock(key);

			/* print "Echo" message via DMA to shell UART */
			ret = dma_uart_send((const uint8_t *)"Echo:\r\n", sizeof("Echo:\r\n") - 1);
			if (ret < 0) {
				printk("Error sending UART TX DMA: %d\n", ret);
			}

			/* echo back user input via DMA to shell UART */
			ret = dma_uart_send(echo_buf, echo_len);
			if (ret < 0) {
				printk("Error sending UART TX DMA: %d\n", ret);
			}

			/* send newline via DMA to shell UART */
			ret = dma_uart_send((const uint8_t *)"\r\n", sizeof("\r\n") - 1);
			if (ret < 0) {
				printk("Error sending UART TX DMA: %d\n", ret);
			}
		}
	}
	return 0;
}

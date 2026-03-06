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
#include <soc_common.h>

#define UART_NODE DT_CHOSEN(zephyr_shell_uart)

#define RX_TIMEOUT       50
#define RX_DMA_BUF_SIZE  64
#define TX_DMA_BUF_SIZE  64

static const struct device *uart_dev = DEVICE_DT_GET(UART_NODE);

static uint8_t rx_dma_buf[RX_DMA_BUF_SIZE];
static uint8_t tx_dma_buf[TX_DMA_BUF_SIZE];

volatile static bool tx_busy;
static size_t tx_len;

static struct k_sem line_ready;

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
		tx_busy = false;
		break;

	case UART_TX_ABORTED:
		printk("DMA TX aborted\n");
		tx_busy = false;
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

/* UART DMA configurations */
#define DMA_CTRL_ACK_TYPE_Pos          (16U)
#define DMA_CTRL_ENA                   (1U << 4)

static void configure_uart2_for_dma0(void)
{
	/* Enable UART2 dma0 EVTRTR channel */
#define UART2_DMA_RX_PERIPH_REQ         10
#define UART2_DMA_TX_PERIPH_REQ         18
#define UART2_DMA_GROUP                  0

	uint32_t regdata;

	printk("\n configure uart2 for dma0\n");

	/* channel enable UART2-RX */
	sys_write32(DMA_CTRL_ENA |
			(0 << DMA_CTRL_ACK_TYPE_Pos)|
			(UART2_DMA_GROUP),
			EVTRTR0_DMA_CTRL0 + (UART2_DMA_RX_PERIPH_REQ * 4));

	/* DMA Handshake enable UART2-RX */
	regdata = sys_read32(EVTRTR0_DMA_ACK_TYPE0 + (UART2_DMA_GROUP * 4));
	regdata |= (1 << UART2_DMA_RX_PERIPH_REQ);
	sys_write32(regdata, EVTRTR0_DMA_ACK_TYPE0 + (UART2_DMA_GROUP * 4));

	/* channel enable UART2-TX */
	sys_write32(DMA_CTRL_ENA |
			(0 << DMA_CTRL_ACK_TYPE_Pos)|
			(UART2_DMA_GROUP),
			EVTRTR0_DMA_CTRL0 + (UART2_DMA_TX_PERIPH_REQ * 4));

	/* DMA Handshake enable UART2-TX */
	regdata = sys_read32(EVTRTR0_DMA_ACK_TYPE0 + (UART2_DMA_GROUP * 4));
	regdata |= (1 << UART2_DMA_TX_PERIPH_REQ);
	sys_write32(regdata, EVTRTR0_DMA_ACK_TYPE0 + (UART2_DMA_GROUP * 4));
}

/* small helper API for DMA TX */
static int dma_uart_send(const uint8_t *data, size_t len)
{
	int ret = 0;

	tx_busy = true;

	ret = uart_tx(uart_dev, data, len, SYS_FOREVER_US);
	if (ret < 0) {
		return ret;
	}

	/* wait until DMA TX finishes */
	while (tx_busy) {
		k_sleep(K_MSEC(1));
	}

	return ret;
}

int main(void)
{
	int ret = 0;

	if (!device_is_ready(uart_dev)) {
		printk("UART device not ready\n");
		return 0;
	}

	/* UART2 DMA0 configuration */
	configure_uart2_for_dma0();

	printk("\nUART DMA Echo Test\n");

	k_sem_init(&line_ready, 0, 1);

	ret = uart_callback_set(uart_dev, uart_cb, NULL);
	if (ret < 0) {
		printk("Error setting UART callback: %d\n", ret);
	}

	printk("Enabling UART RX DMA\n");

	ret = uart_rx_enable(uart_dev, rx_dma_buf, sizeof(rx_dma_buf), RX_TIMEOUT);
	if (ret < 0) {
		printk("Error enabling UART RX DMA: %d\n", ret);
	}

	printk("Type anything on Shell UART and press ENTER! \r\n");

	while (1) {
		/* indefinitely wait for the user input */
		k_sem_take(&line_ready, K_FOREVER);

		if (!tx_busy && tx_len > 0) {

			/* print "Echo" message via DMA to shell UART */
			ret = dma_uart_send((const uint8_t *)"Echo:\r\n", sizeof("Echo:\r\n") - 1);
			if (ret < 0) {
				printk("Error sending UART TX DMA: %d\n", ret);
			}

			/* echo back user input via DMA to shell UART */
			ret = dma_uart_send(tx_dma_buf, tx_len);
			if (ret < 0) {
				printk("Error sending UART TX DMA: %d\n", ret);
			}

			/* send newline via DMA to shell UART */
			ret = dma_uart_send((const uint8_t *)"\r\n", sizeof("\r\n") - 1);
			if (ret < 0) {
				printk("Error sending UART TX DMA: %d\n", ret);
			}

			tx_len = 0;
		}
	}
	return 0;
}

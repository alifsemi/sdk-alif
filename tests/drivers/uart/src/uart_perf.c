/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <string.h>
#include "test_uart.h"

#define UART_PERF_1KB_BYTES    1024
#define UART_PERF_SPEED_BYTES  (1024 * 25)

K_EVENT_DEFINE(uart_perf_events);

static uint8_t perf_rx_buf[FIFO_DEPTH];

static const uint32_t perf_bauds[] = {
	115200,
	921600,
	2500000,
	5000000,
};

/* Spark UART max is 2.5 Mbps. uart_configure() still accepts 5 Mbps,
 * so skip that rate here instead of timing out on transfer.
 * Ensemble/Eagle keep 5 Mbps.
 */
static bool perf_baud_supported_on_board(uint32_t baud)
{
	if (baud <= 2500000U) {
		return true;
	}

	return strstr(CONFIG_BOARD, "spark") == NULL;
}

static void uart_perf_tx_cb(const struct device *dev, void *user_data)
{
	struct uart_cb_ctx *ctx = user_data;
	int ret;

	ret = uart_irq_update(dev);
	if (ret <= 0) {
		return;
	}

	if (!uart_irq_tx_ready(dev)) {
		return;
	}

	if (tx_data_idx < ctx->size) {
		int remaining = ctx->size - tx_data_idx;

		ret = uart_fifo_fill(dev, &ctx->buf[tx_data_idx], remaining);
		if (ret > 0) {
			tx_data_idx += ret;
		}
	}

	if (tx_data_idx >= ctx->size) {
		data_transmitted = true;
		char_sent = ctx->size;
		k_event_set(&uart_perf_events, TX_DONE_EVENT);
		uart_irq_tx_disable(dev);
	}
}

static void uart_perf_rx_cb(const struct device *dev, void *user_data)
{
	struct uart_cb_ctx *ctx = user_data;
	int ret;

	ret = uart_irq_update(dev);
	if (ret <= 0) {
		return;
	}

	if (!uart_irq_rx_ready(dev)) {
		return;
	}

	while (rec_bytes < ctx->size && uart_irq_rx_ready(dev)) {
		int rd = uart_fifo_read(dev, &perf_rx_buf[rec_bytes],
					ctx->size - rec_bytes);

		if (rd > 0) {
			rec_bytes += rd;
		} else {
			break;
		}
	}

	if (rec_bytes >= ctx->size) {
		data_received = true;
		uart_irq_rx_disable(dev);
		k_event_set(&uart_perf_events, RX_DONE_EVENT);
	}
}

static void uart_perf_drain(const struct device *dev)
{
	uint8_t dump[FIFO_DEPTH];

	uart_irq_rx_disable(dev);
	uart_irq_tx_disable(dev);

	while (uart_irq_update(dev) > 0 && uart_irq_rx_ready(dev)) {
		if (uart_fifo_read(dev, dump, sizeof(dump)) <= 0) {
			break;
		}
	}
}

static void perf_set_8n1(uint32_t baudrate)
{
	uart_cfg_tx.baudrate = baudrate;
	uart_cfg_tx.parity = UART_CFG_PARITY_NONE;
	uart_cfg_tx.stop_bits = UART_CFG_STOP_BITS_1;
	uart_cfg_tx.data_bits = UART_CFG_DATA_BITS_8;
	uart_cfg_tx.flow_ctrl = UART_CFG_FLOW_CTRL_NONE;

	uart_cfg_rx.baudrate = baudrate;
	uart_cfg_rx.parity = UART_CFG_PARITY_NONE;
	uart_cfg_rx.stop_bits = UART_CFG_STOP_BITS_1;
	uart_cfg_rx.data_bits = UART_CFG_DATA_BITS_8;
	uart_cfg_rx.flow_ctrl = UART_CFG_FLOW_CTRL_NONE;
}

static void perf_print_metrics(uint32_t baudrate, uint32_t total_bytes,
			      int64_t duration_ms)
{
	uint32_t bitrate = 0;

	if (duration_ms > 0) {
		bitrate = (uint32_t)(((uint64_t)total_bytes * 10U * 1000U) /
				     (uint64_t)duration_ms);
	}

	TC_PRINT("====== UART perf metrics ======\n");
	TC_PRINT("Baud rate (bps):         %u\n", baudrate);
	TC_PRINT("Total data transferred:  %u bytes\n", total_bytes);
	TC_PRINT("Transfer time:           %lld ms\n", duration_ms);
	TC_PRINT("Approx. line bit rate:   %u bps\n", bitrate);
	TC_PRINT("================================\n");
}

static int perf_one_transfer(const struct device *tx_dev,
			     const struct device *rx_dev,
			     struct uart_cb_ctx *tx_ctx,
			     struct uart_cb_ctx *rx_ctx,
			     uint32_t baudrate)
{
	int64_t deadline;

	char_sent = 0;
	tx_data_idx = 0;
	rec_bytes = 0;
	data_transmitted = false;
	data_received = false;
	memset(perf_rx_buf, 0, sizeof(perf_rx_buf));
	k_event_clear(&uart_perf_events, TX_DONE_EVENT | RX_DONE_EVENT);

	uart_perf_drain(tx_dev);
	uart_perf_drain(rx_dev);

	uart_irq_rx_enable(rx_dev);
	uart_irq_tx_enable(tx_dev);

	deadline = k_uptime_get() + uart_test_timeout_ms(baudrate, tx_ctx->size);

	while (!data_transmitted || !data_received) {
		(void)k_event_wait(&uart_perf_events,
				   TX_DONE_EVENT | RX_DONE_EVENT,
				   false, K_MSEC(50));

		if (k_uptime_get() > deadline) {
			TC_PRINT("TIMEOUT at %u baud (sent=%d/%d, recv=%d/%d)\n",
				 baudrate, char_sent, tx_ctx->size,
				 rec_bytes, rx_ctx->size);
			break;
		}
	}

	uart_irq_tx_disable(tx_dev);
	uart_irq_rx_disable(rx_dev);

	if (!data_transmitted || !data_received ||
	    char_sent != tx_ctx->size || rec_bytes != rx_ctx->size ||
	    memcmp(tx_ctx->buf, perf_rx_buf, tx_ctx->size) != 0) {
		return -1;
	}

	return 0;
}

static int perf_run(uint32_t total_bytes, const char *label)
{
	const struct device *tx_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_devnode1));
	const struct device *rx_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_devnode2));
	uint8_t tx_buf[FIFO_DEPTH];
	struct uart_cb_ctx tx_ctx;
	struct uart_cb_ctx rx_ctx;
	uint32_t loops = total_bytes / FIFO_DEPTH;
	int failed = 0;
	int tested = 0;
	int ret;

	zassert_true(loops > 0, "total_bytes must be a multiple of FIFO_DEPTH");
	zassert_true((total_bytes % FIFO_DEPTH) == 0,
		     "total_bytes must be a multiple of FIFO_DEPTH");
	zassert_true(device_is_ready(tx_dev), "TX UART device not ready");
	zassert_true(device_is_ready(rx_dev), "RX UART device not ready");

	memset(tx_buf, 'A', sizeof(tx_buf));
	tx_ctx.buf = tx_buf;
	tx_ctx.size = FIFO_DEPTH;
	rx_ctx.buf = NULL;
	rx_ctx.size = FIFO_DEPTH;

	ret = uart_irq_callback_user_data_set(tx_dev, uart_perf_tx_cb, &tx_ctx);
	if (uart_irq_cb(ret) != 0) {
		zassert_true(false, "TX callback set failed");
	}

	ret = uart_irq_callback_user_data_set(rx_dev, uart_perf_rx_cb, &rx_ctx);
	if (uart_irq_cb(ret) != 0) {
		zassert_true(false, "RX callback set failed");
	}

	for (int b = 0; b < ARRAY_SIZE(perf_bauds); b++) {
		uint32_t baud = perf_bauds[b];
		int64_t duration_ms = 0;
		uint32_t sent_bytes = 0;
		int baud_fail = 0;

		if (!perf_baud_supported_on_board(baud)) {
			TC_PRINT("Skipping baud %u (Spark max is 2.5 Mbps)\n",
				 baud);
			continue;
		}

		perf_set_8n1(baud);

		ret = uart_configure(tx_dev, &uart_cfg_tx);
		if (check_configure_result_loopback(ret) != 0) {
			TC_PRINT("Skipping unsupported TX baud %u\n", baud);
			continue;
		}
		ret = uart_configure(rx_dev, &uart_cfg_rx);
		if (check_configure_result_loopback(ret) != 0) {
			TC_PRINT("Skipping unsupported RX baud %u\n", baud);
			continue;
		}

		TC_PRINT("[ STARTED ] %s at baudrate %u (%u x %d bytes)\n",
			 label, baud, loops, FIFO_DEPTH);

		for (uint32_t i = 0; i < loops; i++) {
			int64_t start = k_uptime_get();

			if (perf_one_transfer(tx_dev, rx_dev, &tx_ctx, &rx_ctx,
					      baud) != 0) {
				baud_fail++;
				break;
			}

			duration_ms += k_uptime_get() - start;
			sent_bytes += FIFO_DEPTH;
		}

		perf_print_metrics(baud, sent_bytes, duration_ms);

		if (baud_fail != 0 || sent_bytes != total_bytes) {
			failed++;
			TC_PRINT("[ FAILED ] %s at baudrate %u\n", label, baud);
		} else {
			tested++;
			TC_PRINT("[ PASSED ] %s at baudrate %u\n", label, baud);
		}
	}

	uart_irq_tx_disable(tx_dev);
	uart_irq_rx_disable(rx_dev);

	if (tested == 0) {
		return -1;
	}

	return (failed == 0) ? 0 : -1;
}

/**
 * @brief External loopback of 1 KiB (32 x 32-byte bursts). Legacy ZTC-162.
 */
ZTEST(uart_performance, test_external_loopback_1kb)
{
	int ret = perf_run(UART_PERF_1KB_BYTES, "Perf-1KB");

	zassert_equal(ret, 0, "External loopback 1 KiB performance test failed");
}

/**
 * @brief External loopback of 25 KiB (800 x 32-byte bursts). Legacy ZTC-1507.
 */
ZTEST(uart_performance, test_external_loopback_speed)
{
	int ret = perf_run(UART_PERF_SPEED_BYTES, "Perf-25KB");

	zassert_equal(ret, 0, "External loopback speed performance test failed");
}

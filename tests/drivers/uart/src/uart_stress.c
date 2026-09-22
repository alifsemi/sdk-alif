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

/*
 * Legacy payload sizes from the original Zephyr_tests stress cases.
 * 300 KiB is exact. The "1 Mb" case sent 1 250 000 bytes (not 1 Mbit).
 */
#define UART_STRESS_300KB_BYTES  (300 * 1024)
#define UART_STRESS_1MB_BYTES    1250000

K_EVENT_DEFINE(uart_stress_events);

static uint8_t stress_tx_buf[FIFO_DEPTH];

static void uart_stress_tx_cb(const struct device *dev, void *user_data)
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
			char_sent += ret;
		}
	}

	if (tx_data_idx >= ctx->size) {
		data_transmitted = true;
		k_event_set(&uart_stress_events, TX_DONE_EVENT);
		uart_irq_tx_disable(dev);
	}
}

static void stress_print_metrics(uint32_t baudrate, uint32_t total_bytes,
				 int64_t duration_ms)
{
	uint32_t bitrate = 0;

	if (duration_ms > 0) {
		bitrate = (uint32_t)(((uint64_t)total_bytes * 10U * 1000U) /
				     (uint64_t)duration_ms);
	}

	TC_PRINT("====== UART TX stress metrics ======\n");
	TC_PRINT("Baud rate (bps):         %u\n", baudrate);
	TC_PRINT("Total data transferred:  %u bytes (%u KiB)\n",
		 total_bytes, total_bytes / 1024U);
	TC_PRINT("Transfer time:           %lld ms\n", duration_ms);
	TC_PRINT("Approx. line bit rate:   %u bps\n", bitrate);
	TC_PRINT("====================================\n");
}

static int stress_configure_tx(const struct device *dev, uint32_t baudrate)
{
	int ret;

	uart_cfg.baudrate = baudrate;
	uart_cfg.parity = UART_CFG_PARITY_NONE;
	uart_cfg.stop_bits = UART_CFG_STOP_BITS_1;
	uart_cfg.data_bits = UART_CFG_DATA_BITS_8;
	uart_cfg.flow_ctrl = UART_CFG_FLOW_CTRL_NONE;

	ret = uart_configure(dev, &uart_cfg);
	return check_configure_result(ret);
}

static int stress_tx_burst(const struct device *dev, struct uart_cb_ctx *ctx,
			   uint32_t baudrate)
{
	uint32_t ev;
	int64_t timeout_ms;

	data_transmitted = false;
	char_sent = 0;
	tx_data_idx = 0;
	k_event_clear(&uart_stress_events, TX_DONE_EVENT);

	uart_irq_tx_enable(dev);

	timeout_ms = uart_test_timeout_ms(baudrate, ctx->size);
	ev = k_event_wait(&uart_stress_events, TX_DONE_EVENT, false,
			  K_MSEC(timeout_ms));

	uart_irq_tx_disable(dev);

	if ((ev & TX_DONE_EVENT) == 0 || !data_transmitted ||
	    char_sent != ctx->size) {
		TC_PRINT("TX burst timeout or short write (sent=%d/%d)\n",
			 char_sent, ctx->size);
		return -1;
	}

	return 0;
}

static void stress_run_tx(uint32_t target_bytes)
{
	const struct device *dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_devnode1));
	struct uart_cb_ctx ctx;
	uint32_t baudrate;
	uint32_t burst_size = sizeof(stress_tx_buf);
	uint32_t loop_count;
	uint32_t failed = 0;
	uint32_t sent_bytes = 0;
	int64_t duration_ms = 0;
	int ret;

	zassert_true(device_is_ready(dev), "%s UART device not ready",
		     dev->name);
	TC_PRINT("%s UART device ready\n", dev->name);

	baudrate = get_uart_baudrate_from_dt();
	stress_configure_tx(dev, baudrate);
	TC_PRINT("Configured baudrate %u, target %u bytes\n",
		 baudrate, target_bytes);

	memset(stress_tx_buf, 'A', burst_size);
	ctx.buf = stress_tx_buf;
	ctx.size = (int)burst_size;

	ret = uart_irq_callback_user_data_set(dev, uart_stress_tx_cb, &ctx);
	zassert_equal(uart_irq_cb(ret), 0, "TX callback set failed");

	loop_count = (target_bytes + burst_size - 1U) / burst_size;

	for (uint32_t i = 0; i < loop_count; i++) {
		int64_t start = k_uptime_get();

		if (stress_tx_burst(dev, &ctx, baudrate) != 0) {
			failed++;
			break;
		}

		duration_ms += k_uptime_get() - start;
		sent_bytes += burst_size;
	}

	uart_irq_tx_disable(dev);
	uart_irq_rx_disable(dev);
	k_event_clear(&uart_stress_events, TX_DONE_EVENT);

	stress_print_metrics(baudrate, sent_bytes, duration_ms);

	zassert_equal(failed, 0, "TX stress failed after %u of %u bursts",
		      sent_bytes / burst_size, loop_count);
	zassert_true(sent_bytes >= target_bytes,
		     "Transferred %u bytes, expected at least %u",
		     sent_bytes, target_bytes);
}

/**
 * @brief Interrupt-driven TX of 300 KiB (legacy ZTC-25 / 300 KB case).
 */
ZTEST(uart_stress, test_transmit_300kb)
{
	stress_run_tx(UART_STRESS_300KB_BYTES);
}

/**
 * @brief Interrupt-driven TX of 1 250 000 bytes (legacy test_send_1Mb).
 */
ZTEST(uart_stress, test_transmit_1mb)
{
	stress_run_tx(UART_STRESS_1MB_BYTES);
}

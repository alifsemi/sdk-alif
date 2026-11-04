/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/**
 * @file
 * @brief FIFO depth / 33rd-byte boundary tests.
 *
 * Alif UART FIFO is 32 bytes. After the FIFO (and optional holding
 * register) is full, the next uart_fifo_fill() must return 0 — that is
 * the TX "33rd byte" case. RX: 32+1 can sit without OE; the 34th byte
 * should set UART_ERROR_OVERRUN, then the driver must recover.
 */

#include <string.h>
#include "test_uart.h"

struct fifo_tx_result {
	volatile int accepted;
	volatile int boundary;
	volatile bool done;
};

static struct k_sem fifo_tx_done;
static uint8_t fifo_pattern[FIFO_DEPTH];
static uint8_t fifo_extra = 0x5A;
static struct fifo_tx_result fifo_tx;

/*
 * Fill only while the driver reports TX ready. A 0 return from
 * uart_fifo_fill(), or TX-ready dropping once FIFO_DEPTH bytes are in,
 * is the full-FIFO boundary. Disable the IRQ before leaving so an
 * unserviced TX-ready cannot spin.
 */
static void fifo_tx_isr(const struct device *dev, void *user_data)
{
	struct fifo_tx_result *res = user_data;

	if (res->done) {
		uart_irq_tx_disable(dev);
		return;
	}

	if (uart_irq_update(dev) <= 0) {
		return;
	}

	while (uart_irq_tx_ready(dev)) {
		int n;

		if (res->accepted < FIFO_DEPTH) {
			n = uart_fifo_fill(dev, &fifo_pattern[res->accepted],
					   FIFO_DEPTH - res->accepted);
		} else {
			n = uart_fifo_fill(dev, &fifo_extra, 1);
		}

		if (n <= 0) {
			res->boundary = n;
			res->done = true;
			break;
		}

		res->accepted += n;
		if (res->accepted > (FIFO_DEPTH + 1)) {
			res->boundary = n;
			res->done = true;
			break;
		}
	}

	if (!res->done && res->accepted >= FIFO_DEPTH &&
	    !uart_irq_tx_ready(dev)) {
		res->boundary = 0;
		res->done = true;
	}

	if (res->done) {
		uart_irq_tx_disable(dev);
		k_sem_give(&fifo_tx_done);
	}
}

static int fifo_cfg_8n1(const struct device *dev)
{
	struct uart_config cfg = {
		.baudrate = get_uart_baudrate_from_dt(),
		.parity = UART_CFG_PARITY_NONE,
		.stop_bits = UART_CFG_STOP_BITS_1,
		.data_bits = UART_CFG_DATA_BITS_8,
		.flow_ctrl = UART_CFG_FLOW_CTRL_NONE,
	};

	return uart_configure(dev, &cfg);
}

/**
 * @brief TX: accept 32 (or 32+THR) bytes, refuse the next (33rd+) byte.
 *
 * The fill runs in the TX ISR, which is the only context where
 * uart_fifo_fill() is supported. The thread waits on fifo_tx_done.
 */
ZTEST(uart_fifo, test_tx_fifo_33rd_byte)
{
	const struct device *dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_devnode1));
	int ret;

	zassert_true(device_is_ready(dev), "%s not ready", dev->name);
	zassert_equal(fifo_cfg_8n1(dev), 0, "uart_configure failed");

	memset(fifo_pattern, 0xA5, sizeof(fifo_pattern));
	fifo_tx.accepted = 0;
	fifo_tx.boundary = -1;
	fifo_tx.done = false;
	k_sem_init(&fifo_tx_done, 0, 1);

	uart_irq_rx_disable(dev);
	uart_irq_tx_disable(dev);

	ret = uart_irq_callback_user_data_set(dev, fifo_tx_isr, &fifo_tx);
	zassert_equal(uart_irq_cb(ret), 0, "TX IRQ callback setup failed");

	uart_irq_tx_enable(dev);

	ret = k_sem_take(&fifo_tx_done, K_MSEC(200));
	uart_irq_tx_disable(dev);
	zassert_equal(ret, 0, "TX FIFO probe timed out");
	zassert_true(fifo_tx.done, "TX ISR did not report a FIFO boundary");

	TC_PRINT("TX FIFO: accepted %d bytes (depth=%d), boundary fill=%d\n",
		 fifo_tx.accepted, FIFO_DEPTH, fifo_tx.boundary);

	zassert_true(fifo_tx.accepted >= FIFO_DEPTH,
		     "Filled only %d bytes, expected at least FIFO_DEPTH %d",
		     fifo_tx.accepted, FIFO_DEPTH);
	zassert_true(fifo_tx.accepted <= (FIFO_DEPTH + 1),
		     "Accepted %d bytes; HW FIFO should be %d (+1 THR max)",
		     fifo_tx.accepted, FIFO_DEPTH);
	zassert_equal(fifo_tx.boundary, 0,
		      "Driver still accepted data (%d) after the FIFO filled",
		      fifo_tx.boundary);
}

#if DT_HAS_CHOSEN(zephyr_devnode2)
/**
 * @brief RX: overflow the 32+1 deep FIFO, then recover.
 *
 * 32 FIFO + RBR can hold 33 bytes without OE. The 34th received byte
 * is the overrun. Wait one char-time after the last poll_out so the
 * last stop bit has reached RX before uart_err_check().
 */
ZTEST(uart_fifo, test_rx_fifo_33rd_byte)
{
	const struct device *tx = DEVICE_DT_GET(DT_CHOSEN(zephyr_devnode1));
	const struct device *rx = DEVICE_DT_GET(DT_CHOSEN(zephyr_devnode2));
	const uint32_t baud = get_uart_baudrate_from_dt();
	const int flood = FIFO_DEPTH + 2;
	int err;
	uint8_t c;
	int64_t deadline;
	uint32_t char_us;

	if (!IS_ENABLED(CONFIG_TEST_EXTERNAL_LB) &&
	    !IS_ENABLED(CONFIG_TEST_UART_FIFO)) {
		TC_PRINT("RX overflow test needs a TX-RX wire "
			 "(CONFIG_TEST_EXTERNAL_LB or CONFIG_TEST_UART_FIFO)\n");
		ztest_test_skip();
	}

	zassert_true(device_is_ready(tx), "TX not ready");
	zassert_true(device_is_ready(rx), "RX not ready");
	zassert_equal(fifo_cfg_8n1(tx), 0, "TX configure");
	zassert_equal(fifo_cfg_8n1(rx), 0, "RX configure");

	uart_irq_rx_disable(rx);
	uart_irq_tx_disable(tx);
	(void)uart_err_check(rx);
	while (uart_poll_in(rx, &c) == 0) {
	}

	/*
	 * 32 FIFO + 1 RBR = 33 without OE. Send 34 so the last completed
	 * character overruns.
	 */
	for (int i = 0; i < flood; i++) {
		uart_poll_out(tx, (uint8_t)i);
	}

	char_us = (10U * 1000000U) / (baud ? baud : 115200U);
	k_busy_wait(char_us * 2U);
	k_msleep(2);

	(void)uart_irq_update(rx);

	err = uart_err_check(rx);
	TC_PRINT("RX after %d bytes (no drain): uart_err_check()=0x%x "
		 "(OVERRUN=0x%x)\n", flood, err, UART_ERROR_OVERRUN);
	zassert_true((err & UART_ERROR_OVERRUN) != 0,
		     "Expected UART_ERROR_OVERRUN after %d-byte flood, got 0x%x",
		     flood, err);

	while (uart_poll_in(rx, &c) == 0) {
	}
	(void)uart_err_check(rx);

	uart_poll_out(tx, 0xA5);
	k_busy_wait(char_us * 2U);

	deadline = k_uptime_get() + 200;
	while (k_uptime_get() < deadline) {
		if (uart_poll_in(rx, &c) == 0) {
			if (c == 0xA5) {
				TC_PRINT("RX recovered with 0xA5\n");
				return;
			}
			TC_PRINT("Ignoring leftover flood byte 0x%02x\n", c);
		}
	}

	zassert_true(false, "RX did not recover after FIFO overflow flood");
}
#else
ZTEST(uart_fifo, test_rx_fifo_33rd_byte)
{
	TC_PRINT("Skipped: zephyr,devnode2 not in DT\n");
	ztest_test_skip();
}
#endif


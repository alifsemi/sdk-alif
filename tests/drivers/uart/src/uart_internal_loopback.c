/*
 * Copyright (c) 2025 Alif Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include "test_uart.h"

static void uart_TxRx_cb(const struct device *dev, void *user_data)
{
	struct uart_cb_ctx *ctx = (struct uart_cb_ctx *)user_data;
	int ret;

	ret = uart_irq_update(dev);
	if (ret <= 0) {
		return;
	}

	if (uart_irq_tx_ready(dev) && tx_data_idx1 < ctx->size) {
		int tx_len = uart_fifo_fill(dev,
					    &ctx->buf[tx_data_idx1], 1);
		if (tx_len > 0) {
			tx_data_idx1++;
			char_sent1++;
		}

		if (tx_data_idx1 == ctx->size) {
			data_transmitted_auto = true;
			uart_irq_tx_disable(dev);
		}
	}

	if (uart_irq_rx_ready(dev)) {
		uint8_t recv_byte;
		int rd = uart_fifo_read(dev, &recv_byte, 1);

		if (rd > 0 && rec_count_auto < DATASIZE) {
			rx_verify_buf_auto[rec_count_auto] = recv_byte;
			rec_count_auto++;
		}
		if (rec_count_auto == ctx->size) {
			data_received_auto = true;
		}
	}
}

static int test_InternalLoopback_withInterrupt(const uint8_t *tx_buf, int size,
					       const char *str)
{
	int i, count = 0;
	int ret;
	struct uart_cb_ctx ctx;

	zassert_true(size > 0, "TX buffer size must be > 0");
	zassert_true(size <= DATASIZE, "TX buffer size exceeds DATASIZE");

	const struct device *dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_devnode2));

	zassert_true(device_is_ready(dev), "%s UART device not ready", dev->name);
	TC_PRINT("%s UART device is ready\n", dev->name);

	ctx.buf = tx_buf;
	ctx.size = size;

	for (i = B9600; i <= B2500000; i++) {
		char_sent1 = 0;
		tx_data_idx1 = 0;
		data_transmitted_auto = false;
		data_received_auto = false;
		rec_count_auto = 0;
		memset(rx_verify_buf_auto, 0, DATASIZE);

		int baudrate = checkBaud(i);

		if (baudrate == TC_SKIP) {
			continue;
		}
		uart_cfg.baudrate = baudrate;

		ret = uart_configure(dev, &uart_cfg);
		if (Check_configure_result_loopback(ret) == -1) {
			continue;
		}

		int ret_irq = uart_irq_callback_user_data_set(dev, uart_TxRx_cb, &ctx);

		if (Uart_irq_cb(ret_irq) == -1) {
			continue;
		}

		uart_irq_tx_enable(dev);
		uart_irq_rx_enable(dev);

		int64_t deadline = k_uptime_get() +
				    uart_test_timeout_ms(uart_cfg.baudrate, size);

		while (!data_transmitted_auto || !data_received_auto) {
			if (k_uptime_get() > deadline) {
				TC_PRINT("TIMEOUT: %s at baudrate %d "
					 "(sent=%d/%d, recv=%d/%d)\n",
					 str, uart_cfg.baudrate,
					 char_sent1, size,
					 rec_count_auto, size);
				break;
			}
			k_sleep(K_MSEC(100));
		}

		uart_irq_tx_disable(dev);
		uart_irq_rx_disable(dev);

		if (data_transmitted_auto && data_received_auto &&
		    char_sent1 == size && rec_count_auto == size &&
		    memcmp(tx_buf, rx_verify_buf_auto, size) == 0) {
			TC_PRINT("Internal Loop back test for %s with baudrate %d "
				 "has PASSED\n", str, uart_cfg.baudrate);
		} else {
			count++;
			TC_PRINT("Internal Loop back test for %s with baudrate %d "
				 "has FAILED\n", str, uart_cfg.baudrate);
		}
	}

	return (count == 0) ? 0 : -1;
}

#if CONFIG_TEST_UART_RTSCTS
/* ZTC-184: Test RTS and CTS flow control */
ZTEST(uart_Internalloopback, test_Interrupt_flowctrl_RTS_CTS)
{
	int ret;
	unsigned char tx_rtsCTS[] = "TESTING OF RTS/CTS!!!";
	int size = strlen((const char *)tx_rtsCTS);
	struct uart_cb_ctx ctx = { .buf = tx_rtsCTS, .size = size };

	char_sent1 = 0;
	tx_data_idx1 = 0;
	data_transmitted_auto = false;
	data_received_auto = false;
	rec_count_auto = 0;
	memset(rx_verify_buf_auto, 0, DATASIZE);

	uart_cfg.flow_ctrl = UART_CFG_FLOW_CTRL_RTS_CTS;
	uart_cfg.parity    = UART_CFG_PARITY_NONE;
	uart_cfg.stop_bits = UART_CFG_STOP_BITS_1;
	uart_cfg.data_bits = UART_CFG_DATA_BITS_8;
	uart_cfg.baudrate  = get_uart_baudrate_from_dt();
	TC_PRINT("Current Baudrate: %d\n", uart_cfg.baudrate);

	const struct device *dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_devnode2));

	zassert_true(device_is_ready(dev), "%s UART device not ready", dev->name);
	TC_PRINT("%s UART device ready\n", dev->name);

	ret = uart_configure(dev, &uart_cfg);
	Check_configure_result(ret);

	int ret_irq = uart_irq_callback_user_data_set(dev, uart_TxRx_cb, &ctx);

	zassert_equal(Uart_irq_cb(ret_irq), 0, "Failed to set IRQ callback");

	uart_irq_tx_enable(dev);
	uart_irq_rx_enable(dev);

	int64_t deadline = k_uptime_get() +
			    uart_test_timeout_ms(uart_cfg.baudrate, size);

	while (!data_transmitted_auto || !data_received_auto) {
		if (k_uptime_get() > deadline) {
			TC_PRINT("TIMEOUT: RTS/CTS at baudrate %d\n",
				 uart_cfg.baudrate);
			break;
		}
		k_sleep(K_MSEC(100));
	}

	uart_irq_tx_disable(dev);
	uart_irq_rx_disable(dev);

	zassert_true(data_transmitted_auto, "Data was not transmitted");
	zassert_equal(char_sent1, size, "Not all characters were sent");
	zassert_true(data_received_auto, "Data was not received");
	zassert_equal(memcmp(tx_rtsCTS, rx_verify_buf_auto, size), 0,
		      "RX data mismatch");
}
#endif

/* ZTC-18: Configure stop bits and transmit data */
ZTEST(uart_Internalloopback, test_Interrupt_configure_stopBits)
{
	int i, count = 0;
	int ret;
	uint8_t tx_stopBits[] = "TESTING STOP BITS DATA!!!";
	int size = strlen((const char *)tx_stopBits);
	struct uart_cb_ctx ctx = { .buf = tx_stopBits, .size = size };

	uart_cfg.flow_ctrl = UART_CFG_FLOW_CTRL_NONE;
	uart_cfg.parity    = UART_CFG_PARITY_NONE;
	uart_cfg.data_bits = UART_CFG_DATA_BITS_8;
	uart_cfg.baudrate  = get_uart_baudrate_from_dt();
	TC_PRINT("Current Baudrate: %d\n", uart_cfg.baudrate);

	const struct device *dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_devnode2));

	zassert_true(device_is_ready(dev), "%s UART device not ready", dev->name);
	TC_PRINT("%s UART device ready\n", dev->name);

	for (i = UART_CFG_STOP_BITS_0_5; i <= UART_CFG_STOP_BITS_2; i++) {
		char_sent1 = 0;
		tx_data_idx1 = 0;
		data_transmitted_auto = false;
		data_received_auto = false;
		rec_count_auto = 0;

		uart_cfg.stop_bits = checkstopBits(i);
		ret = uart_configure(dev, &uart_cfg);
		if (Check_configure_result_loopback(ret) == -1) {
			TC_PRINT("For stop_bits %d -> SKIPPED (unsupported)\n", i);
			continue;
		}

		int ret_irq = uart_irq_callback_user_data_set(dev, uart_TxRx_cb,
							       &ctx);
		if (Uart_irq_cb(ret_irq) == -1) {
			count++;
			TC_PRINT("For stop_bits %d -> FAILED (callback)\n", i);
			continue;
		}

		uart_irq_tx_enable(dev);
		uart_irq_rx_enable(dev);

		int64_t deadline = k_uptime_get() +
				    uart_test_timeout_ms(uart_cfg.baudrate, size);

		while (!data_transmitted_auto || !data_received_auto) {
			if (k_uptime_get() > deadline) {
				TC_PRINT("TIMEOUT: stop_bits %d\n", i);
				break;
			}
			k_sleep(K_MSEC(100));
		}

		uart_irq_tx_disable(dev);
		uart_irq_rx_disable(dev);

		if (data_transmitted_auto && data_received_auto &&
		    char_sent1 == size) {
			TC_PRINT("For stop_bits %d -> PASSED\n", i);
		} else {
			count++;
			TC_PRINT("For stop_bits %d -> FAILED\n", i);
		}
	}
	zassert_equal(count, 0, "Stop bits test failed for some configurations");
}

/* ZTC-17: Configure parity bits and transmit data */
ZTEST(uart_Internalloopback, test_Interrupt_configure_parityBits)
{
	int ret, i, count = 0;
	unsigned char tx_parity[] = "TESTING PARITY BITS DATA!!!";
	int size = strlen((const char *)tx_parity);
	struct uart_cb_ctx ctx = { .buf = tx_parity, .size = size };

	uart_cfg.flow_ctrl = UART_CFG_FLOW_CTRL_NONE;
	uart_cfg.stop_bits = UART_CFG_STOP_BITS_1;
	uart_cfg.data_bits = UART_CFG_DATA_BITS_8;
	uart_cfg.baudrate  = get_uart_baudrate_from_dt();
	TC_PRINT("Current Baudrate: %d\n", uart_cfg.baudrate);

	const struct device *dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_devnode2));

	zassert_true(device_is_ready(dev), "%s UART device not ready", dev->name);
	TC_PRINT("%s UART device ready\n", dev->name);

	for (i = UART_CFG_PARITY_NONE; i <= UART_CFG_PARITY_SPACE; i++) {
		char_sent1 = 0;
		tx_data_idx1 = 0;
		data_transmitted_auto = false;
		data_received_auto = false;
		rec_count_auto = 0;

		uart_cfg.parity = checkparityBits(i);

		ret = uart_configure(dev, &uart_cfg);
		if (Check_configure_result_loopback(ret) == -1) {
			TC_PRINT("For parity %d -> SKIPPED (unsupported)\n", i);
			continue;
		}

		int ret_irq = uart_irq_callback_user_data_set(dev, uart_TxRx_cb,
							       &ctx);
		if (Uart_irq_cb(ret_irq) == -1) {
			count++;
			TC_PRINT("For parity %d -> FAILED (callback)\n", i);
			continue;
		}

		uart_irq_tx_enable(dev);
		uart_irq_rx_enable(dev);

		int64_t deadline = k_uptime_get() +
				    uart_test_timeout_ms(uart_cfg.baudrate, size);

		while (!data_transmitted_auto || !data_received_auto) {
			if (k_uptime_get() > deadline) {
				TC_PRINT("TIMEOUT: parity %d\n", i);
				break;
			}
			k_sleep(K_MSEC(100));
		}

		uart_irq_tx_disable(dev);
		uart_irq_rx_disable(dev);

		if (data_transmitted_auto && data_received_auto &&
		    char_sent1 == size) {
			TC_PRINT("For parity %d -> PASSED\n", i);
		} else {
			count++;
			TC_PRINT("For parity %d -> FAILED\n", i);
		}
	}
	zassert_equal(count, 0, "Parity bits test failed for some configurations");
}

/* ZTC-20: Internal loopback with interrupts for transmitting strings */
ZTEST(uart_Internalloopback, test_Interrupt_InternalloopbackString)
{
	unsigned char tx_string[] = "Loopback test for Strings !!!";
	int size = strlen((const char *)tx_string);

	int ret = test_InternalLoopback_withInterrupt(tx_string, size, "String");

	zassert_equal(ret, 0, "Internal loopback string test failed");
}

/* ZTC-21: Internal loopback with interrupts for transmitting numbers */
ZTEST(uart_Internalloopback, test_Interrupt_InternalloopbackNumbers)
{
	unsigned char tx_bufNum[] = "123456789";
	int size = strlen((const char *)tx_bufNum);

	int ret = test_InternalLoopback_withInterrupt(tx_bufNum, size, "Numbers");

	zassert_equal(ret, 0,
		      "Internal loopback test for numbers failed");
}

/* ZTC-22: Internal loopback with interrupts for transmitting binary data */
ZTEST(uart_Internalloopback, test_Interrupt_InternalloopbackBinaryData)
{
	/* Binary payload — no NUL dependency, size passed explicitly */
	uint8_t tx_binary[] = {0xF7, 0xA3, 0x00, 0x5C};
	int size = sizeof(tx_binary);

	TC_PRINT("Binary payload: ");
	for (int i = 0; i < size; i++) {
		TC_PRINT("0x%02X ", tx_binary[i]);
	}
	TC_PRINT("\n");

	int ret = test_InternalLoopback_withInterrupt(tx_binary, size, "Binary");

	zassert_equal(ret, 0, "Internal loopback binary test failed");
}

/* ZTC-182: Internal loopback with interrupts for transmitting ASCII data */
ZTEST(uart_Internalloopback, test_Interrupt_InternalloopbackAsciiData)
{
	unsigned char tx_buf[DATASIZE];
	uint8_t tx_val = 11;

	int len = snprintf((char *)tx_buf, sizeof(tx_buf), "%u", tx_val);

	zassert_true(len > 0, "Failed to convert data to ASCII");

	int ret = test_InternalLoopback_withInterrupt(tx_buf, len, "Ascii");

	zassert_equal(ret, 0, "Internal loopback ASCII test failed");
}

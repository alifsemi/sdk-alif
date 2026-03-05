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

static struct k_event uart_events;
static unsigned char global_rx_buffer[DATASIZE];

static void print_buffer_hex_ascii(const char *label, const unsigned char *buf,
				   int len)
{
	TC_PRINT("%s (%d bytes)\n", label, len);
	TC_PRINT("HEX: ");
	for (int i = 0; i < len; i++) {
		TC_PRINT("%02X ", buf[i]);
	}
	TC_PRINT("\nASCII: ");
	for (int i = 0; i < len; i++) {
		unsigned char c = buf[i];

		TC_PRINT("%c", (c >= 32 && c <= 126) ? c : '.');
	}
	TC_PRINT("\n");
}

static void print_buffer_hex_only(const char *label, const unsigned char *buf,
				  int len)
{
	TC_PRINT("%s (%d bytes)\n", label, len);
	for (int i = 0; i < len; i++) {
		TC_PRINT("%02X ", buf[i]);
	}
	TC_PRINT("\n");
}

/*
 * TX-only callback for external loopback.
 * Uses struct uart_cb_ctx to know the buffer size without strlen().
 */
static void uart_external_loopback_tx_cb(const struct device *dev,
					 void *user_data)
{
	struct uart_cb_ctx *ctx = (struct uart_cb_ctx *)user_data;
	int ret;

	ret = uart_irq_update(dev);
	if (ret <= 0) {
		return;
	}

	if (uart_irq_tx_ready(dev)) {
		int remaining = ctx->size - tx_data_idx;

		if (remaining > 0) {
			ret = uart_fifo_fill(dev,
					     &ctx->buf[tx_data_idx],
					     remaining);
			if (ret > 0) {
				tx_data_idx += ret;
			}
		}

		if (tx_data_idx >= ctx->size) {
			data_transmitted = true;
			char_sent = ctx->size;
			k_event_set(&uart_events, TX_DONE_EVENT);
			uart_irq_tx_disable(dev);
		}
	}
}

/*
 * RX-only callback for external loopback.
 * Reads into global_rx_buffer; signals RX_DONE when expected bytes arrive.
 */
static void uart_external_loopback_rx_cb(const struct device *dev,
					 void *user_data)
{
	struct uart_cb_ctx *ctx = (struct uart_cb_ctx *)user_data;
	int ret;

	ret = uart_irq_update(dev);
	if (ret <= 0) {
		return;
	}

	if (uart_irq_rx_ready(dev)) {
		int read_limit = (ctx->size > DATASIZE)
				 ? DATASIZE : ctx->size;

		while (rec_bytes < read_limit && uart_irq_rx_ready(dev)) {
			int rd = uart_fifo_read(dev,
					       &global_rx_buffer[rec_bytes],
					       read_limit - rec_bytes);
			if (rd > 0) {
				rec_bytes += rd;
			}
		}

		if (rec_bytes >= read_limit) {
			data_received = true;
			uart_irq_rx_disable(dev);
			k_event_set(&uart_events, RX_DONE_EVENT);
		}
	}
}

static int test_external_loopback_interrupt(const uint8_t *tx_buf, int size,
					  const char *str)
{
	int ret;
	struct uart_cb_ctx tx_ctx = { .buf = tx_buf, .size = size };
	struct uart_cb_ctx rx_ctx = { .buf = NULL, .size = size };

	zassert_true(size > 0, "TX buffer size must be > 0");
	zassert_true(size <= DATASIZE, "TX buffer exceeds DATASIZE");

	const struct device *tx_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_devnode1));
	const struct device *rx_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_devnode2));

	zassert_true(device_is_ready(tx_dev), "TX UART device not ready");
	zassert_true(device_is_ready(rx_dev), "RX UART device not ready");

	char_sent = 0;
	tx_data_idx = 0;
	data_transmitted = false;
	data_received = false;
	rec_bytes = 0;
	memset(global_rx_buffer, 0, sizeof(global_rx_buffer));

	k_event_init(&uart_events);

	ret = uart_configure(tx_dev, &uart_cfg_tx);
	if (ret != 0) {
		TC_PRINT("TX uart_configure failed (ret=%d)\n", ret);
		return -1;
	}
	ret = uart_configure(rx_dev, &uart_cfg_rx);
	if (ret != 0) {
		TC_PRINT("RX uart_configure failed (ret=%d)\n", ret);
		return -1;
	}

	TC_PRINT("[ STARTED ] External loopback %s at baudrate %d\n",
		 str, uart_cfg_tx.baudrate);

	int ret_rx = uart_irq_callback_user_data_set(rx_dev,
						     uart_external_loopback_rx_cb,
						     &rx_ctx);
	if (uart_irq_cb(ret_rx) == -1) {
		TC_PRINT("[ FAILED ] RX callback set failed for %s\n", str);
		return -1;
	}
	uart_irq_rx_enable(rx_dev);

	int ret_tx = uart_irq_callback_user_data_set(tx_dev,
						     uart_external_loopback_tx_cb,
						     &tx_ctx);
	if (uart_irq_cb(ret_tx) == -1) {
		TC_PRINT("[ FAILED ] TX callback set failed for %s\n", str);
		uart_irq_rx_disable(rx_dev);
		return -1;
	}
	uart_irq_tx_enable(tx_dev);

	int64_t deadline = k_uptime_get() +
			    uart_test_timeout_ms(uart_cfg_tx.baudrate, size);

	while (!data_transmitted || !data_received) {
		(void)k_event_wait(&uart_events, TX_DONE_EVENT | RX_DONE_EVENT,
				   false, K_MSEC(50));

		if (k_uptime_get() > deadline) {
			TC_PRINT("TIMEOUT: %s (sent=%d/%d, recv=%d/%d)\n",
				 str, char_sent, size, rec_bytes, size);
			break;
		}
	}

	uart_irq_tx_disable(tx_dev);
	uart_irq_rx_disable(rx_dev);

	if (data_transmitted && data_received &&
	    char_sent == size && rec_bytes == size &&
	    memcmp(tx_buf, global_rx_buffer, size) == 0) {
		TC_PRINT("[ PASSED ] %s at baudrate %d (sent=%d, recv=%d)\n",
			 str, uart_cfg_tx.baudrate, char_sent, rec_bytes);
		return 0;
	}

	TC_PRINT("[ FAILED ] %s (sent=%d/%d, recv=%d/%d)\n",
		 str, char_sent, size, rec_bytes, size);
	return -1;
}

/*
 * Helper: reset uart_cfg_tx and uart_cfg_rx to default 8N1/no-flow
 * at the given baud rate.
 */
static void ext_lb_set_config(uint32_t baudrate)
{
	uart_cfg_tx.baudrate  = baudrate;
	uart_cfg_tx.parity    = UART_CFG_PARITY_NONE;
	uart_cfg_tx.stop_bits = UART_CFG_STOP_BITS_1;
	uart_cfg_tx.data_bits = UART_CFG_DATA_BITS_8;
	uart_cfg_tx.flow_ctrl = UART_CFG_FLOW_CTRL_NONE;

	uart_cfg_rx.baudrate  = baudrate;
	uart_cfg_rx.parity    = UART_CFG_PARITY_NONE;
	uart_cfg_rx.stop_bits = UART_CFG_STOP_BITS_1;
	uart_cfg_rx.data_bits = UART_CFG_DATA_BITS_8;
	uart_cfg_rx.flow_ctrl = UART_CFG_FLOW_CTRL_NONE;
}

/**
 * @brief Test external loopback with interrupt for transmitting strings.
 *
 * Generates a 32-byte string and transmits it via UART using interrupt
 * mode, then verifies the received data matches the transmitted data.
 */
ZTEST(uart_external_loopback, test_interrupt_external_loopback_string)
{
	unsigned char tx_string[33];

	for (int i = 0; i < 32; i++) {
		tx_string[i] = (unsigned char)('A' + (i % 26));
	}
	tx_string[32] = '\0';

	ext_lb_set_config(get_uart_baudrate_from_dt());

	int size = strlen((const char *)tx_string);
	int ret = test_external_loopback_interrupt(tx_string, size, "String-32B");

	print_buffer_hex_ascii("Sent", tx_string, size);
	print_buffer_hex_ascii("Received", global_rx_buffer, rec_bytes);
	zassert_equal(ret, 0, "External loopback string test failed");
}

/**
 * @brief Test external loopback with interrupt for transmitting numbers.
 *
 * Transmits numeric string data via UART using interrupt mode and
 * verifies the received data matches the transmitted data.
 */
ZTEST(uart_external_loopback, test_interrupt_external_loopback_numbers)
{
	unsigned char tx_num[] = "44332232232";
	int size = strlen((const char *)tx_num);

	ext_lb_set_config(get_uart_baudrate_from_dt());

	int ret = test_external_loopback_interrupt(tx_num, size, "Numbers");

	print_buffer_hex_ascii("Sent", tx_num, size);
	print_buffer_hex_ascii("Received", global_rx_buffer, rec_bytes);
	zassert_equal(ret, 0, "External loopback numbers test failed");
}

/**
 * @brief Test external loopback with interrupt for transmitting binary data (32 bytes).
 *
 * Generates 32 bytes of sequential binary data and transmits it via UART
 * using interrupt mode, then verifies the received data matches.
 */
ZTEST(uart_external_loopback, test_interrupt_external_loopback_binary_data)
{
	uint8_t tx_binary[FIFO_DEPTH];

	for (int i = 0; i < FIFO_DEPTH; i++) {
		tx_binary[i] = (uint8_t)i;
	}

	ext_lb_set_config(get_uart_baudrate_from_dt());

	int size = FIFO_DEPTH;
	int ret = test_external_loopback_interrupt(tx_binary, size, "Binary-32B");

	print_buffer_hex_only("Sent", tx_binary, size);
	print_buffer_hex_only("Received", global_rx_buffer, rec_bytes);
	zassert_equal(ret, 0, "External loopback binary test failed");
}

/**
 * @brief Test external loopback with interrupt for transmitting ASCII data.
 *
 * Transmits 32 bytes of ASCII characters via UART using interrupt mode
 * and verifies the received data matches the transmitted data.
 */
ZTEST(uart_external_loopback, test_interrupt_external_loopback_ascii_data)
{
	char tx_ascii[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345";
	int len = strlen(tx_ascii);

	ext_lb_set_config(get_uart_baudrate_from_dt());

	int ret = test_external_loopback_interrupt((unsigned char *)tx_ascii,
						 len, "ASCII-32B");

	print_buffer_hex_ascii("Sent", (unsigned char *)tx_ascii, len);
	print_buffer_hex_ascii("Received", global_rx_buffer, rec_bytes);
	zassert_equal(ret, 0, "External loopback ASCII test failed");
}

/**
 * @brief Test external loopback with interrupt for transmitting hex data.
 *
 * Transmits 32 bytes of hexadecimal values via UART using interrupt mode
 * and verifies the received data matches the transmitted data.
 */
ZTEST(uart_external_loopback, test_interrupt_external_loopback_hex_data)
{
	unsigned char tx_hex[FIFO_DEPTH] = {
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
		0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x10, 0x21, 0x32, 0x43, 0x54, 0x65, 0x76, 0x87,
		0x98, 0xA9, 0xBA, 0xCB, 0xDC, 0xED, 0xFE, 0x0F
	};
	int size = sizeof(tx_hex);

	ext_lb_set_config(get_uart_baudrate_from_dt());

	int ret = test_external_loopback_interrupt(tx_hex, size, "HEX-32B");

	print_buffer_hex_only("HEX Sent", tx_hex, size);
	print_buffer_hex_only("HEX Received", global_rx_buffer, rec_bytes);
	zassert_equal(ret, 0, "External loopback HEX test failed");
}

/**
 * @brief Test external loopback across multiple baud rates.
 *
 * Iterates through standard baud rates (9600, 19200, 38400, 57600,
 * 115200, 921600) and verifies successful transmission at each rate.
 */
ZTEST(uart_external_loopback, test_interrupt_external_loopback_multi_baud)
{
	static const uint32_t bauds[] = {
		9600, 19200, 38400, 57600, 115200, 921600
	};
	unsigned char tx_buf[] = "BaudSweep!";
	int size = strlen((const char *)tx_buf);
	int count = 0;

	for (int i = 0; i < ARRAY_SIZE(bauds); i++) {
		ext_lb_set_config(bauds[i]);

		int ret = test_external_loopback_interrupt(tx_buf, size,
							  "MultiBaud");
		if (ret != 0) {
			count++;
			TC_PRINT("External loopback FAILED at baud %d\n",
				 bauds[i]);
		}
	}

	zassert_equal(count, 0,
		      "External loopback multi-baud sweep had failures");
}

/**
 * @brief Test external loopback with different stop bit configurations.
 *
 * Tests all supported stop bit settings (0.5, 1, 1.5, 2) and verifies
 * successful transmission with each configuration.
 */
ZTEST(uart_external_loopback, test_interrupt_external_loopback_stop_bits)
{
	unsigned char tx_buf[] = "StopBitTest!";
	int size = strlen((const char *)tx_buf);
	int tested = 0;
	int failed = 0;

	const struct device *tx_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_devnode1));
	const struct device *rx_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_devnode2));

	for (int i = UART_CFG_STOP_BITS_0_5; i <= UART_CFG_STOP_BITS_2; i++) {
		ext_lb_set_config(get_uart_baudrate_from_dt());
		uart_cfg_tx.stop_bits = i;
		uart_cfg_rx.stop_bits = i;

		/* Probe uart_configure to check if this config is supported */
		int cfg_ret = uart_configure(tx_dev, &uart_cfg_tx);

		if (cfg_ret != 0) {
			TC_PRINT("External loopback stop_bits %d -> SKIPPED "
				 "(unsupported, ret=%d)\n", i, cfg_ret);
			continue;
		}
		cfg_ret = uart_configure(rx_dev, &uart_cfg_rx);
		if (cfg_ret != 0) {
			TC_PRINT("External loopback stop_bits %d -> SKIPPED "
				 "(RX unsupported, ret=%d)\n", i, cfg_ret);
			continue;
		}

		int ret = test_external_loopback_interrupt(tx_buf, size,
							  "StopBits");
		if (ret != 0) {
			failed++;
			TC_PRINT("External loopback stop_bits %d -> FAILED\n",
				 i);
		} else {
			tested++;
			TC_PRINT("External loopback stop_bits %d -> PASSED\n",
				 i);
		}
	}

	TC_PRINT("Stop-bits: %d passed, %d failed\n", tested, failed);
	zassert_equal(failed, 0, "Stop bits test failed for %d configuration(s)", failed);
	zassert_true(tested > 0, "No stop-bit configurations were tested successfully");
}

/**
 * @brief Test external loopback with different parity configurations.
 *
 * Tests all supported parity settings (none, odd, even, mark, space)
 * and verifies successful transmission with each configuration.
 */
ZTEST(uart_external_loopback, test_interrupt_external_loopback_parity)
{
	unsigned char tx_buf[] = "ParityTest!";
	int size = strlen((const char *)tx_buf);
	int tested = 0;
	int failed = 0;

	const struct device *tx_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_devnode1));
	const struct device *rx_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_devnode2));

	for (int i = UART_CFG_PARITY_NONE; i <= UART_CFG_PARITY_SPACE; i++) {
		ext_lb_set_config(get_uart_baudrate_from_dt());
		uart_cfg_tx.parity = i;
		uart_cfg_rx.parity = i;

		/* Probe uart_configure to check if this config is supported */
		int cfg_ret = uart_configure(tx_dev, &uart_cfg_tx);

		if (cfg_ret != 0) {
			TC_PRINT("External loopback parity %d -> SKIPPED "
				 "(unsupported, ret=%d)\n", i, cfg_ret);
			continue;
		}
		cfg_ret = uart_configure(rx_dev, &uart_cfg_rx);
		if (cfg_ret != 0) {
			TC_PRINT("External loopback parity %d -> SKIPPED "
				 "(RX unsupported, ret=%d)\n", i, cfg_ret);
			continue;
		}

		int ret = test_external_loopback_interrupt(tx_buf, size,
							  "Parity");
		if (ret != 0) {
			failed++;
			TC_PRINT("External loopback parity %d -> FAILED\n", i);
		} else {
			tested++;
			TC_PRINT("External loopback parity %d -> PASSED\n", i);
		}
	}

	TC_PRINT("Parity: %d passed, %d failed\n", tested, failed);
	zassert_equal(failed, 0, "Parity test failed for %d configuration(s)", failed);
	zassert_true(tested > 0, "No parity configurations were tested successfully");
}

/**
 * @brief Test back-to-back external loopback transfers.
 *
 * Performs two consecutive transfers (31 bytes and 32 bytes) to verify
 * the UART can handle multiple transfers without issues.
 */
ZTEST(uart_external_loopback, test_interrupt_external_loopback_back_to_back)
{
	unsigned char tx_a[] = "BackToBack_Test_1_Below_FIFO_XX";
	unsigned char tx_b[] = "BackToBack_Test_2_Exact_FIFO_XX!";
	int size_a = 31;
	int size_b = 32;


	ext_lb_set_config(get_uart_baudrate_from_dt());
	int ret = test_external_loopback_interrupt(tx_a, size_a, "BackToBack-31B");

	zassert_equal(ret, 0, "Back-to-back transfer 1 (31B) failed");

	ext_lb_set_config(get_uart_baudrate_from_dt());
	ret = test_external_loopback_interrupt(tx_b, size_b, "BackToBack-32B");
	zassert_equal(ret, 0, "Back-to-back transfer 2 (32B) failed");
}

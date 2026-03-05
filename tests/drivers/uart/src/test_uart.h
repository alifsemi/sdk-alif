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
 * @brief UART test cases header file
 *
 * Common declarations for UART test suite covering internal/external
 * loopback and RTS/CTS flow-control tests on Alif platforms.
 */

#ifndef TEST_UART_H_
#define TEST_UART_H_

#include <zephyr/drivers/uart.h>
#include <zephyr/ztest.h>
#include <zephyr/sys/util.h>

#if !DT_HAS_CHOSEN(zephyr_devnode1) || !DT_HAS_CHOSEN(zephyr_devnode2)
#error "UART test requires DT chosen nodes zephyr,devnode1 and zephyr,devnode2. " \
	"Apply boards/alif_uart.overlay (or pass -DDTC_OVERLAY_FILE=boards/alif_uart.overlay)."
#endif

/** Maximum receive-buffer size shared across test modules. */
#define DATASIZE        128

/** Hardware FIFO depth (bytes) for a single uart_fifo_fill / uart_fifo_read. */
#define FIFO_DEPTH      32

/** k_event bit signalling TX completion inside an ISR callback. */
#define TX_DONE_EVENT   BIT(0)

/** k_event bit signalling RX completion inside an ISR callback. */
#define RX_DONE_EVENT   BIT(1)

/** Floor value (ms) returned by uart_test_timeout_ms() for fast baud rates. */
#define UART_TEST_TIMEOUT_MS  3000

/**
 * @brief Compute a safe per-transfer timeout in milliseconds.
 *
 * Uses 8N1 framing (10 bits per byte) to estimate the raw transfer time,
 * then applies a 3x multiplier plus a 2 s margin.  Never returns less than
 * @ref UART_TEST_TIMEOUT_MS so that high-baud-rate transfers still get a
 * reasonable deadline.
 *
 * @param baudrate  Configured baud rate (bps).
 * @param size      Number of bytes to transfer.
 * @return Timeout in milliseconds.
 */
static inline int64_t uart_test_timeout_ms(uint32_t baudrate, int size)
{
	int64_t transfer_ms = ((int64_t)size * 10 * 1000) / baudrate;
	int64_t timeout = transfer_ms * 3 + 2000; /* 3x transfer time + 2s margin */

	return (timeout > UART_TEST_TIMEOUT_MS) ? timeout : UART_TEST_TIMEOUT_MS;
}

/*
 * Context structure passed as user_data to ISR callbacks.
 * Avoids calling strlen() in ISR context and supports binary data.
 */
struct uart_cb_ctx {
	const uint8_t *buf;  /* TX data buffer */
	int size;            /* TX data length (no NUL dependency) */
};

typedef enum {
	UART_BAUD_9600,
	UART_BAUD_300,
	UART_BAUD_19200,
	UART_BAUD_38400,
	UART_BAUD_57600,
	UART_BAUD_115200,
	UART_BAUD_921600,
	UART_BAUD_2500000,
} uart_config_baudrates;

/* Automation (internal loopback) state */
extern volatile bool data_transmitted_auto;
extern volatile bool data_received_auto;
extern volatile int tx_data_idx1;
extern volatile int rec_count_auto;
extern volatile int char_sent1;
extern uint8_t rx_verify_buf_auto[];

/* Manual (external loopback / stress) state */
extern volatile bool data_transmitted;
extern volatile bool data_received;
extern volatile int tx_data_idx;
extern volatile int char_sent;
extern volatile int rec_bytes;

extern struct uart_config uart_cfg;
extern struct uart_config uart_cfg_rx;
extern struct uart_config uart_cfg_tx;

/* Utility functions (main.c) */
extern int set_baudrate(const struct device *uart_dev, uint32_t test_baudrate);
extern int check_baud(int num);
extern int check_stop_bits(int num);
extern int check_parity_bits(int num);
extern int check_configure_result(int ret);
extern int check_configure_result_loopback(int ret);
extern int uart_irq_cb(int ret);

/* Helper macro to get current-speed from devicetree */
#define UART_GET_BAUDRATE(node) DT_PROP_OR(node, current_speed, 115200)

static inline uint32_t get_uart_baudrate_from_dt(void)
{
	return UART_GET_BAUDRATE(DT_CHOSEN(zephyr_devnode1));
}

#endif /* __TEST_UART_H__ */

/**
 * @file
 * @brief UART test cases header file
 *
 * Common declarations for UART test suite covering internal/external
 * loopback and RTS/CTS flow-control tests on Alif platforms.
 */

#ifndef __TEST_UART_H__
#define __TEST_UART_H__

#include <zephyr/drivers/uart.h>
#include <zephyr/ztest.h>
#include <zephyr/sys/util.h>

#if !DT_HAS_CHOSEN(zephyr_devnode1) || !DT_HAS_CHOSEN(zephyr_devnode2)
#error "UART test requires DT chosen nodes zephyr,devnode1 and zephyr,devnode2. " \
	"Apply boards/alif_uart.overlay (or pass -DDTC_OVERLAY_FILE=boards/alif_uart.overlay)."
#endif

#define DATASIZE        128
#define FIFO_DEPTH      32

#define TX_DONE_EVENT   BIT(0)
#define RX_DONE_EVENT   BIT(1)

/* Default timeout for UART transfer completion (per baud-rate iteration) */
#define UART_TEST_TIMEOUT_MS  3000

/*
 * Compute a safe timeout (ms) based on baud rate and data size.
 * Formula: (size_bytes * 10 bits_per_byte * 1000 ms/s) / baudrate + margin.
 * Minimum UART_TEST_TIMEOUT_MS to handle high baud rates.
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
extern int checkBaud(int num);
extern int checkstopBits(int num);
extern int checkparityBits(int num);
extern int Check_configure_result(int ret);
extern int Check_configure_result_loopback(int ret);
extern int Uart_irq_cb(int ret);

/* Helper macro to get current-speed from devicetree */
#define UART_GET_BAUDRATE(node) DT_PROP_OR(node, current_speed, 115200)

static inline uint32_t get_uart_baudrate_from_dt(void)
{
	return UART_GET_BAUDRATE(DT_CHOSEN(zephyr_devnode1));
}

#endif /* __TEST_UART_H__ */

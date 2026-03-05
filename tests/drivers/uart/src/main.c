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
 * @brief UART test shared state, utility helpers, and test-suite registration.
 *
 * This file owns the global UART configuration structs and ISR-shared
 * variables consumed by uart_internal_loopback.c and
 * uart_external_loopback.c.  It also provides small helper functions
 * (baud-rate look-up, config-result checking, IRQ-callback validation)
 * and the ZTEST_SUITE() declarations gated by Kconfig flags.
 */

#include "test_uart.h"

/*
 * ---------------------------------------------------------------------------
 *  UART configuration structs — one per logical role.
 *  All default to 115200 / 8N1 / no flow-control.
 * ---------------------------------------------------------------------------
 */
struct uart_config uart_cfg = {
	.baudrate  = 115200,
	.parity    = UART_CFG_PARITY_NONE,
	.stop_bits = UART_CFG_STOP_BITS_1,
	.data_bits = UART_CFG_DATA_BITS_8,
	.flow_ctrl = UART_CFG_FLOW_CTRL_NONE,
};

struct uart_config uart_cfg_rx = {
	.baudrate  = 115200,
	.parity    = UART_CFG_PARITY_NONE,
	.stop_bits = UART_CFG_STOP_BITS_1,
	.data_bits = UART_CFG_DATA_BITS_8,
	.flow_ctrl = UART_CFG_FLOW_CTRL_NONE,
};

struct uart_config uart_cfg_tx = {
	.baudrate  = 115200,
	.parity    = UART_CFG_PARITY_NONE,
	.stop_bits = UART_CFG_STOP_BITS_1,
	.data_bits = UART_CFG_DATA_BITS_8,
	.flow_ctrl = UART_CFG_FLOW_CTRL_NONE,
};

/*
 * ---------------------------------------------------------------------------
 *  ISR-shared state: internal loopback ("auto") path
 * ---------------------------------------------------------------------------
 */
volatile bool data_transmitted_auto;
volatile bool data_received_auto;
volatile int tx_data_idx1;
volatile int rec_count_auto;
volatile int char_sent1;
uint8_t rx_verify_buf_auto[DATASIZE];

/*
 * ---------------------------------------------------------------------------
 *  ISR-shared state: external loopback path
 * ---------------------------------------------------------------------------
 */
volatile bool data_transmitted;
volatile bool data_received;
volatile int tx_data_idx;
volatile int char_sent;
volatile int rec_bytes;

/*
 * ---------------------------------------------------------------------------
 *  Utility helpers
 * ---------------------------------------------------------------------------
 */

/**
 * @brief Set the baud rate of a UART device at runtime.
 *
 * Reads the current configuration via uart_config_get(), updates only
 * the baud-rate field, then applies it with uart_configure().  This
 * lets a single compiled binary test every supported rate without
 * per-baud overlay recompilation.
 *
 * @param uart_dev       UART device to reconfigure.
 * @param test_baudrate  Desired baud rate in bps.
 * @return 0 on success, negative errno on failure.
 */
int set_baudrate(const struct device *uart_dev, uint32_t test_baudrate)
{
	struct uart_config current_cfg;
	int err;

	err = uart_config_get(uart_dev, &current_cfg);
	if (err != 0) {
		TC_PRINT("uart_config_get failed (ret=%d)\n", err);
		return err;
	}

	current_cfg.baudrate = test_baudrate;
	err = uart_configure(uart_dev, &current_cfg);
	if (err != 0) {
		TC_PRINT("uart_configure failed for baudrate %u (ret=%d)\n",
			 test_baudrate, err);
	}

	return err;
}

int check_baud(int num)
{
	switch (num) {
	case UART_BAUD_9600:
		return 9600;
	case UART_BAUD_300:
		return 300;
	case UART_BAUD_19200:
		return 19200;
	case UART_BAUD_38400:
		return 38400;
	case UART_BAUD_57600:
		return 57600;
	case UART_BAUD_115200:
		return 115200;
	case UART_BAUD_921600:
		return 921600;
	case UART_BAUD_2500000:
		return 2500000;
	default:
		TC_PRINT("INVALID BAUDRATE\t");
		return TC_SKIP;
	}
}

int check_stop_bits(int num)
{
	switch (num) {
	case UART_CFG_STOP_BITS_0_5:
		TC_PRINT("\nUART_CFG_STOP_BITS_0_5\n");
		return UART_CFG_STOP_BITS_0_5;
	case UART_CFG_STOP_BITS_1:
		TC_PRINT("\nUART_CFG_STOP_BITS_1\n");
		return UART_CFG_STOP_BITS_1;
	case UART_CFG_STOP_BITS_1_5:
		TC_PRINT("\nUART_CFG_STOP_BITS_1_5\n");
		return UART_CFG_STOP_BITS_1_5;
	case UART_CFG_STOP_BITS_2:
		TC_PRINT("\nUART_CFG_STOP_BITS_2\n");
		return UART_CFG_STOP_BITS_2;
	default:
		TC_PRINT("\nINVALID STOP BITS\n");
		return TC_SKIP;
	}
}

/**
 * @brief Map a parity index to its Zephyr UART_CFG_PARITY_* constant.
 *
 * @param num  Parity selector (0 = none … 4 = space).
 * @return The corresponding UART_CFG_PARITY_* value, or TC_SKIP.
 */
int check_parity_bits(int num)
{
	switch (num) {
	case UART_CFG_PARITY_NONE:
		TC_PRINT("\nUART_CFG_PARITY_NONE\n");
		return UART_CFG_PARITY_NONE;
	case UART_CFG_PARITY_ODD:
		TC_PRINT("\nUART_CFG_PARITY_ODD\n");
		return UART_CFG_PARITY_ODD;
	case UART_CFG_PARITY_EVEN:
		TC_PRINT("\nUART_CFG_PARITY_EVEN\n");
		return UART_CFG_PARITY_EVEN;
	case UART_CFG_PARITY_MARK:
		TC_PRINT("\nUART_CFG_PARITY_MARK\n");
		return UART_CFG_PARITY_MARK;
	case UART_CFG_PARITY_SPACE:
		TC_PRINT("\nUART_CFG_PARITY_SPACE\n");
		return UART_CFG_PARITY_SPACE;
	default:
		TC_PRINT("\nINVALID PARITY\n");
		return TC_SKIP;
	}
}

/**
 * @brief Non-fatal check of uart_configure() return value.
 *
 * Prints a diagnostic and returns -1 on any error so that the caller
 * can skip the current baud-rate / config iteration without aborting
 * the entire test.
 *
 * @param ret  Return value from uart_configure().
 * @return 0 on success, -1 on failure.
 */
int check_configure_result_loopback(int ret)
{
	switch (ret) {
	case -ENOSYS:
		TC_PRINT("Configuration not supported by device or driver\n");
		return -1;
	case -ENOTSUP:
		TC_PRINT("API is not enabled\n");
		return -1;
	default:
		if (ret < 0) {
			TC_PRINT("Negative errno %d\n", ret);
			return -1;
		} else if (ret != 0) {
			TC_PRINT("Configuration failed\n");
			return -1;
		}
		TC_PRINT("Configuration passed\n");
		return 0;
	}
}

/**
 * @brief Fatal check of uart_configure() return value.
 *
 * Calls zassert_equal(); the test is aborted if @p ret != 0.
 *
 * @param ret  Return value from uart_configure().
 * @return 0 (only reached on success).
 */
int check_configure_result(int ret)
{
	zassert_equal(ret, 0, "UART configuration failed (ret=%d)", ret);
	return 0;
}

int uart_irq_cb(int ret)
{
	switch (ret) {
	case 0:
		return 0;
	case -ENOSYS:
		TC_PRINT("Function is not supported\n");
		return -1;
	case -ENOTSUP:
		TC_PRINT("API is not enabled\n");
		return -1;
	default:
		if (ret < 0) {
			TC_PRINT("Negative errno %d from uart_irq_callback_user_data_set\n", ret);
			return -1;
		} else if (ret != 0) {
			TC_PRINT("Unexpected non-zero return value %d from "
			 "uart_irq_callback_user_data_set\n", ret);
			return -1;
		}
	}
}

/*
 * ---------------------------------------------------------------------------
 *  Test-suite registration (gated by Kconfig)
 * ---------------------------------------------------------------------------
 */

#if CONFIG_TEST_EXTERNAL_LB
/** External-loopback suite — requires a physical TX→RX wire between devnode1
 *  and devnode2.  Enabled with CONFIG_TEST_EXTERNAL_LB=y.
 */
ZTEST_SUITE(uart_external_loopback, NULL, NULL, NULL, NULL, NULL);
#endif

#if CONFIG_TEST_INTERNAL_LB
/** Internal-loopback suite — uses the UART's built-in loopback mode on
 *  devnode1.  Enabled with CONFIG_TEST_INTERNAL_LB=y.
 */
ZTEST_SUITE(uart_internal_loopback, NULL, NULL, NULL, NULL, NULL);
#endif

#if CONFIG_TEST_UART_RTSCTS
/** RTS/CTS flow control suite — tests hardware flow control on
 *  devnode1.  Enabled with CONFIG_TEST_UART_RTSCTS=y.
 */
ZTEST_SUITE(uart_RTSCTS_suite, NULL, NULL, NULL, NULL, NULL);
#endif

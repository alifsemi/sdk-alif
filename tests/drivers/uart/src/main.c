/*
 * Copyright (c) 2025 Alif Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "test_uart.h"

/* UART configuration structs used by different test modules */
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

/* Automation (internal loopback) state */
volatile bool data_transmitted_auto;
volatile bool data_received_auto;
volatile int tx_data_idx1;
volatile int rec_count_auto;
volatile int char_sent1;
uint8_t rx_verify_buf_auto[DATASIZE];

/* Manual (external loopback / stress) state */
volatile bool data_transmitted;
volatile bool data_received;
volatile int tx_data_idx;
volatile int char_sent;
volatile int rec_bytes;

int checkBaud(int num)
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

int checkstopBits(int num)
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

int checkparityBits(int num)
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

int Check_configure_result_loopback(int ret)
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

int Check_configure_result(int ret)
{
	zassert_equal(ret, 0, "UART configuration failed (ret=%d)", ret);
	return 0;
}

int Uart_irq_cb(int ret)
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
			return -1;
		}
		return 0;
	}
}

#if CONFIG_TEST_EXTERNAL_LB
ZTEST_SUITE(uart_Externalloopback, NULL, NULL, NULL, NULL, NULL);
#endif

#if CONFIG_TEST_INTERNAL_LB
ZTEST_SUITE(uart_Internalloopback, NULL, NULL, NULL, NULL, NULL);
#endif

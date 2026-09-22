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
 * @brief Line-break and false start-bit tests (not in Zephyr uart tests).
 *
 * Needs a physical TX(devnode1) → RX(devnode2) wire.
 *
 * Break: uart_line_ctrl_set(UART_LINE_CTRL_BRK) holds TX low longer than
 * one 8N1 frame. The receiver should report UART_BREAK and/or
 * UART_ERROR_FRAMING via uart_err_check().
 *
 * False start: a TX-low pulse shorter than one RX bit (short BRK).
 * Hardware should reject it as a false start / framing error, not
 * deliver a valid character. Skipped if UART_LINE_CTRL_BRK is not
 * implemented.
 */

#include <string.h>
#include "test_uart.h"

#define LINE_ERR_BAUD            115200U
/* Mid-start sample is ~208 us at 2400; keep the BRK pulse well under that. */
#define FALSE_START_RX_BAUD      2400U
#define FALSE_START_PULSE_US     20U

static const struct device *tx_dev;
static const struct device *rx_dev;

static int line_cfg_8n1(const struct device *dev, uint32_t baud)
{
	struct uart_config cfg = {
		.baudrate = baud,
		.parity = UART_CFG_PARITY_NONE,
		.stop_bits = UART_CFG_STOP_BITS_1,
		.data_bits = UART_CFG_DATA_BITS_8,
		.flow_ctrl = UART_CFG_FLOW_CTRL_NONE,
	};

	return uart_configure(dev, &cfg);
}

static void line_drain(const struct device *dev)
{
	uint8_t c;
	int spins = 0;

	while (spins++ < 64 && uart_poll_in(dev, &c) == 0) {
	}

	if (uart_irq_update(dev) > 0 && uart_irq_rx_ready(dev)) {
		uint8_t dump[FIFO_DEPTH];

		(void)uart_fifo_read(dev, dump, sizeof(dump));
	}

	(void)uart_err_check(dev);
}

static int line_poll_expect(const struct device *rx, uint8_t expect,
			    int timeout_ms)
{
	uint8_t c;
	int64_t deadline = k_uptime_get() + timeout_ms;

	while (k_uptime_get() < deadline) {
		if (uart_poll_in(rx, &c) == 0) {
			return (c == expect) ? 0 : -EIO;
		}
	}

	return -EAGAIN;
}

static int line_roundtrip(uint8_t v)
{
	line_drain(rx_dev);
	uart_poll_out(tx_dev, v);
	return line_poll_expect(rx_dev, v, 200);
}

static void line_errors_setup(void)
{
	int link;

	tx_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_devnode1));
	rx_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_devnode2));

	zassert_true(device_is_ready(tx_dev), "TX UART not ready");
	zassert_true(device_is_ready(rx_dev), "RX UART not ready");
	zassert_equal(line_cfg_8n1(tx_dev, LINE_ERR_BAUD), 0, "TX configure");
	zassert_equal(line_cfg_8n1(rx_dev, LINE_ERR_BAUD), 0, "RX configure");
	line_drain(rx_dev);

	/*
	 * These cases need a physical uart0 TX → uart1 RX wire.
	 * Without it, uart_err_check() stays 0 and recover round-trips
	 * time out. test_false_start_bit_detect can false-pass (no byte).
	 */
	link = line_roundtrip(0x3C);
	if (link != 0) {
		TC_PRINT("No TX→RX path (roundtrip 0x3C ret=%d). "
			 "Wire %s TX to %s RX (E7/E8: P0_1 → P0_4)\n",
			 link, tx_dev->name, rx_dev->name);
		ztest_test_skip();
	}
}

static int line_set_break(uint32_t on)
{
	int ret = uart_line_ctrl_set(tx_dev, UART_LINE_CTRL_BRK, on);

	if (ret == -ENOSYS || ret == -ENOTSUP) {
		TC_PRINT("uart_line_ctrl_set(UART_LINE_CTRL_BRK) not "
			 "implemented (ret=%d)\n", ret);
		TC_PRINT("Need CONFIG_UART_NS16550_LINE_CTRL=y so ns16550 "
			 "registers line_ctrl_set (CONFIG_UART_LINE_CTRL=y "
			 "alone is not enough)\n");
	}

	return ret;
}

static int line_false_start_pulse(void)
{
	int ret;

	ret = line_cfg_8n1(rx_dev, FALSE_START_RX_BAUD);
	if (ret != 0) {
		return ret;
	}
	line_drain(rx_dev);

	ret = line_set_break(1);
	if (ret != 0) {
		return ret;
	}
	k_busy_wait(FALSE_START_PULSE_US);
	ret = line_set_break(0);
	if (ret == 0) {
		TC_PRINT("False start via short BRK pulse (~%u us at RX %u baud)\n",
			 FALSE_START_PULSE_US, FALSE_START_RX_BAUD);
	}

	return ret;
}

static bool err_is_break_or_framing(int err)
{
	return (err & UART_BREAK) || (err & UART_ERROR_FRAMING);
}

/**
 * @brief Generate a line break and detect it on the paired RX UART.
 *
 * Holds TX low for several character times (a legal break), then
 * checks uart_err_check() for UART_BREAK and/or UART_ERROR_FRAMING.
 */
ZTEST(uart_line_errors, test_line_break_generate_detect)
{
	int ret;
	int err;
	uint32_t char_us;

	line_errors_setup();

	ret = line_set_break(1);
	if (ret == -ENOSYS || ret == -ENOTSUP) {
		ztest_test_skip();
	}
	zassert_equal(ret, 0, "failed to assert break (ret=%d)", ret);

	/* 8N1 frame is 10 bits; hold > 3 frames so LSR BI is unambiguous. */
	char_us = (10U * 1000000U) / LINE_ERR_BAUD;
	k_busy_wait(char_us * 4U);
	k_msleep(2);

	/* LSR BI/FE are live while SIN is still held low; reading LSR clears them. */
	err = uart_err_check(rx_dev);
	TC_PRINT("During break: uart_err_check()=0x%x (BREAK=0x%x FRAMING=0x%x)\n",
		 err, UART_BREAK, UART_ERROR_FRAMING);

	ret = line_set_break(0);
	zassert_equal(ret, 0, "failed to release break (ret=%d)", ret);
	k_msleep(2);

	if (!err_is_break_or_framing(err)) {
		int err_after = uart_err_check(rx_dev);

		TC_PRINT("After break: uart_err_check()=0x%x\n", err_after);
		err |= err_after;
	}

	zassert_true(err_is_break_or_framing(err),
		     "Expected UART_BREAK and/or UART_ERROR_FRAMING, got 0x%x",
		     err);
}

/**
 * @brief After a detected break, a normal byte must still transfer.
 */
ZTEST(uart_line_errors, test_line_break_recover)
{
	int ret;

	line_errors_setup();

	ret = line_set_break(1);
	if (ret == -ENOSYS || ret == -ENOTSUP) {
		ztest_test_skip();
	}
	zassert_equal(ret, 0, "failed to assert break");
	k_msleep(2);
	zassert_equal(line_set_break(0), 0, "failed to release break");
	k_msleep(2);
	(void)uart_err_check(rx_dev);
	line_drain(rx_dev);
	zassert_equal(line_cfg_8n1(tx_dev, LINE_ERR_BAUD), 0, "TX reconfigure");
	zassert_equal(line_cfg_8n1(rx_dev, LINE_ERR_BAUD), 0, "RX reconfigure");
	line_drain(rx_dev);

	zassert_equal(line_roundtrip(0xA5), 0,
		      "UART did not recover after line break");
	TC_PRINT("Post-break 0xA5 round-trip OK\n");
}

/**
 * @brief Inject a false (too-short) start bit and confirm no valid RX byte.
 *
 * RX is dropped to 2400 baud so a ~20 us BRK is < 0.5 bit (mid-start
 * sample ~208 us). Skipped when UART_LINE_CTRL_BRK is not implemented.
 */
ZTEST(uart_line_errors, test_false_start_bit_detect)
{
	int ret;
	int err;
	uint8_t c;

	line_errors_setup();

	ret = line_false_start_pulse();
	if (ret == -ENOSYS || ret == -ENOTSUP) {
		ztest_test_skip();
	}
	zassert_equal(ret, 0, "failed to inject short break (ret=%d)", ret);

	k_msleep(2);
	(void)uart_irq_update(rx_dev);
	err = uart_err_check(rx_dev);
	ret = uart_poll_in(rx_dev, &c);

	TC_PRINT("False start: poll_in=%d byte=0x%02x err=0x%x\n",
		 ret, (ret == 0) ? c : 0, err);

	/*
	 * Hardware may silently drop a false start (poll_in == -1, err==0)
	 * or latch FRAMING. A clean data byte without an error is a fail.
	 */
	if (ret == 0 && !err_is_break_or_framing(err)) {
		zassert_true(false,
			     "False start produced data 0x%02x with no "
			     "framing/break error", c);
	}

	zassert_true(ret != 0 || err_is_break_or_framing(err),
		     "False start was accepted as a valid character");
}

/**
 * @brief After a false start, matching-baud traffic must succeed.
 */
ZTEST(uart_line_errors, test_false_start_bit_recover)
{
	int ret;

	line_errors_setup();

	ret = line_false_start_pulse();
	if (ret == -ENOSYS || ret == -ENOTSUP) {
		ztest_test_skip();
	}
	zassert_equal(ret, 0, "failed to inject short break");

	k_msleep(2);
	(void)uart_err_check(rx_dev);
	line_drain(rx_dev);
	zassert_equal(line_cfg_8n1(tx_dev, LINE_ERR_BAUD), 0, "TX reconfigure");
	zassert_equal(line_cfg_8n1(rx_dev, LINE_ERR_BAUD), 0, "RX reconfigure");
	line_drain(rx_dev);

	zassert_equal(line_roundtrip(0x5A), 0,
		      "UART did not recover after false start bit");
	TC_PRINT("Post-false-start 0x5A round-trip OK\n");
}


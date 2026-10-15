/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/**
 * @file
 * @brief UART hard-plug: unmount mid-transfer, detect stall, measure loss.
 *
 * Needs a physical TX(devnode1) → RX(devnode2) wire.
 *
 * Unplug is applied in-process by unmounting the TX UART (IRQ off, then
 * PM suspend when CONFIG_PM_DEVICE is on). The RX side must notice the
 * stream stop immediately. Remount (resume + uart_configure) is the
 * plug-back; a short transfer must succeed again.
 */

#include <string.h>
#include "test_uart.h"

#ifdef CONFIG_PM_DEVICE
#include <zephyr/pm/device.h>
#endif

#define HOTPLUG_BAUD           115200U
#define HOTPLUG_LEN            128
#define HOTPLUG_UNPLUG_AFTER   40
#define HOTPLUG_STALL_MS       8
#define HOTPLUG_DETECT_LIMIT_MS 50

static const struct device *tx_dev;
static const struct device *rx_dev;

static volatile int hp_tx_idx;
static volatile int hp_rx_idx;
static uint8_t hp_tx_buf[HOTPLUG_LEN];
static uint8_t hp_rx_buf[HOTPLUG_LEN];

static void hotplug_tx_cb(const struct device *dev, void *user_data)
{
	struct uart_cb_ctx *ctx = user_data;
	int ret;

	ret = uart_irq_update(dev);
	if (ret <= 0) {
		return;
	}

	if (!uart_irq_tx_ready(dev) || hp_tx_idx >= ctx->size) {
		return;
	}

	ret = uart_fifo_fill(dev, &ctx->buf[hp_tx_idx],
			     ctx->size - hp_tx_idx);
	if (ret > 0) {
		hp_tx_idx += ret;
	}
	if (hp_tx_idx >= ctx->size) {
		uart_irq_tx_disable(dev);
	}
}

static void hotplug_rx_cb(const struct device *dev, void *user_data)
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

	while (hp_rx_idx < ctx->size && uart_irq_rx_ready(dev)) {
		int rd = uart_fifo_read(dev, &hp_rx_buf[hp_rx_idx],
					ctx->size - hp_rx_idx);

		if (rd > 0) {
			hp_rx_idx += rd;
		} else {
			break;
		}
	}
}

static int hotplug_cfg(const struct device *dev)
{
	struct uart_config cfg = {
		.baudrate = HOTPLUG_BAUD,
		.parity = UART_CFG_PARITY_NONE,
		.stop_bits = UART_CFG_STOP_BITS_1,
		.data_bits = UART_CFG_DATA_BITS_8,
		.flow_ctrl = UART_CFG_FLOW_CTRL_NONE,
	};

	return uart_configure(dev, &cfg);
}

static void hotplug_drain(const struct device *dev)
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

static int hotplug_unmount_tx(void)
{
	int ret = 0;

	uart_irq_tx_disable(tx_dev);
	uart_irq_rx_disable(tx_dev);

#ifdef CONFIG_PM_DEVICE
	ret = pm_device_action_run(tx_dev, PM_DEVICE_ACTION_SUSPEND);
	if (ret == -ENOSYS || ret == -ENOTSUP || ret == -EALREADY) {
		TC_PRINT("PM suspend unavailable (%d); IRQ unmount only\n",
			 ret);
		ret = 0;
	}
#endif
	return ret;
}

static int hotplug_mount_tx(void)
{
	int ret = 0;

#ifdef CONFIG_PM_DEVICE
	ret = pm_device_action_run(tx_dev, PM_DEVICE_ACTION_RESUME);
	if (ret == -ENOSYS || ret == -ENOTSUP || ret == -EALREADY) {
		ret = 0;
	}
#endif
	if (ret != 0) {
		return ret;
	}

	return hotplug_cfg(tx_dev);
}

static void hotplug_setup(void)
{
	int i;

	tx_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_devnode1));
	rx_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_devnode2));

	zassert_true(device_is_ready(tx_dev), "TX UART not ready");
	zassert_true(device_is_ready(rx_dev), "RX UART not ready");
	zassert_equal(hotplug_cfg(tx_dev), 0, "TX configure");
	zassert_equal(hotplug_cfg(rx_dev), 0, "RX configure");

	for (i = 0; i < HOTPLUG_LEN; i++) {
		hp_tx_buf[i] = (uint8_t)(i + 1);
	}
	memset(hp_rx_buf, 0, sizeof(hp_rx_buf));
	hp_tx_idx = 0;
	hp_rx_idx = 0;
	hotplug_drain(rx_dev);
}

static int hotplug_start_stream(struct uart_cb_ctx *tx_ctx,
				struct uart_cb_ctx *rx_ctx)
{
	int ret;

	ret = uart_irq_callback_user_data_set(rx_dev, hotplug_rx_cb, rx_ctx);
	if (uart_irq_cb(ret) != 0) {
		return -1;
	}
	ret = uart_irq_callback_user_data_set(tx_dev, hotplug_tx_cb, tx_ctx);
	if (uart_irq_cb(ret) != 0) {
		return -1;
	}

	uart_irq_rx_enable(rx_dev);
	uart_irq_tx_enable(tx_dev);
	return 0;
}

static void hotplug_stop_irqs(void)
{
	uart_irq_tx_disable(tx_dev);
	uart_irq_rx_disable(rx_dev);
}

static int hotplug_roundtrip(uint8_t v)
{
	uint8_t c;
	int64_t deadline;

	hotplug_drain(rx_dev);
	uart_poll_out(tx_dev, v);

	deadline = k_uptime_get() + 200;
	while (k_uptime_get() < deadline) {
		if (uart_poll_in(rx_dev, &c) == 0) {
			return (c == v) ? 0 : -EIO;
		}
	}

	return -EAGAIN;
}

/**
 * @brief Unmount TX mid-stream; RX must stall quickly; count lost bytes.
 */
ZTEST(uart_hotplug, test_unplug_detect_data_loss)
{
	struct uart_cb_ctx tx_ctx = { .buf = hp_tx_buf, .size = HOTPLUG_LEN };
	struct uart_cb_ctx rx_ctx = { .buf = NULL, .size = HOTPLUG_LEN };
	int64_t wait_deadline;
	int64_t t_unplug;
	int64_t t_detect = -1;
	int last_rx;
	int64_t last_change;
	int tx_at_unplug;
	int rx_at_unplug;
	int lost;
	int err;
	int detect_ms;

	hotplug_setup();
	zassert_equal(hotplug_start_stream(&tx_ctx, &rx_ctx), 0,
		      "failed to start stream");

	wait_deadline = k_uptime_get() + 500;
	while (hp_rx_idx < HOTPLUG_UNPLUG_AFTER) {
		if (k_uptime_get() > wait_deadline) {
			hotplug_stop_irqs();
			TC_PRINT("No RX after 500 ms — need uart0 TX → uart1 RX\n");
			ztest_test_skip();
		}
		k_sleep(K_MSEC(1));
	}

	tx_at_unplug = hp_tx_idx;
	rx_at_unplug = hp_rx_idx;
	t_unplug = k_uptime_get();

	zassert_equal(hotplug_unmount_tx(), 0, "TX unmount failed");
	TC_PRINT("Unmounted %s at tx=%d rx=%d\n",
		 tx_dev->name, tx_at_unplug, rx_at_unplug);

	last_rx = hp_rx_idx;
	last_change = k_uptime_get();
	wait_deadline = t_unplug + HOTPLUG_DETECT_LIMIT_MS + 20;

	while (k_uptime_get() <= wait_deadline) {
		if (hp_rx_idx != last_rx) {
			last_rx = hp_rx_idx;
			last_change = k_uptime_get();
		} else if ((k_uptime_get() - last_change) >= HOTPLUG_STALL_MS) {
			t_detect = last_change;
			break;
		}
		k_sleep(K_MSEC(1));
	}

	hotplug_stop_irqs();
	(void)uart_irq_update(rx_dev);
	err = uart_err_check(rx_dev);

	zassert_true(t_detect >= 0,
		     "RX stream did not stall within %d ms of unplug",
		     HOTPLUG_DETECT_LIMIT_MS + 20);

	detect_ms = (int)(t_detect - t_unplug);
	if (detect_ms < 0) {
		detect_ms = 0;
	}

	lost = tx_at_unplug - hp_rx_idx;
	if (lost < 0) {
		lost = 0;
	}

	TC_PRINT("Unplug detected in %d ms (limit %d ms)\n",
		 detect_ms, HOTPLUG_DETECT_LIMIT_MS);
	TC_PRINT("At detect: tx=%d rx=%d lost=%d err=0x%x ready=%d\n",
		 hp_tx_idx, hp_rx_idx, lost, err,
		 device_is_ready(tx_dev));

	zassert_true(detect_ms <= HOTPLUG_DETECT_LIMIT_MS,
		     "Detection took %d ms (limit %d)",
		     detect_ms, HOTPLUG_DETECT_LIMIT_MS);
	zassert_true(hp_rx_idx < HOTPLUG_LEN,
		     "RX completed the full %d-byte payload after unplug",
		     HOTPLUG_LEN);
	zassert_true((tx_at_unplug - rx_at_unplug) > 0 || lost > 0,
		     "Expected in-flight / lost bytes after unplug");
}

/**
 * @brief Remount TX after unplug; auto-detect resume and transfer again.
 */
ZTEST(uart_hotplug, test_replug_recover)
{
	struct uart_cb_ctx tx_ctx = { .buf = hp_tx_buf, .size = HOTPLUG_LEN };
	struct uart_cb_ctx rx_ctx = { .buf = NULL, .size = HOTPLUG_LEN };
	int64_t wait_deadline;
	int64_t t_plug;
	int64_t t_rx = -1;
	int ret;
	int plug_ms;

	hotplug_setup();
	zassert_equal(hotplug_start_stream(&tx_ctx, &rx_ctx), 0,
		      "failed to start stream");

	wait_deadline = k_uptime_get() + 500;
	while (hp_rx_idx < HOTPLUG_UNPLUG_AFTER) {
		if (k_uptime_get() > wait_deadline) {
			hotplug_stop_irqs();
			TC_PRINT("No RX after 500 ms — need uart0 TX → uart1 RX\n");
			ztest_test_skip();
		}
		k_sleep(K_MSEC(1));
	}

	zassert_equal(hotplug_unmount_tx(), 0, "TX unmount failed");
	k_sleep(K_MSEC(HOTPLUG_STALL_MS + 2));
	hotplug_stop_irqs();
	(void)uart_err_check(rx_dev);
	hotplug_drain(rx_dev);

	hp_tx_idx = 0;
	hp_rx_idx = 0;
	memset(hp_rx_buf, 0, sizeof(hp_rx_buf));

	ret = hotplug_mount_tx();
	zassert_equal(ret, 0, "TX remount failed (%d)", ret);
	zassert_true(device_is_ready(tx_dev), "TX not ready after remount");
	zassert_equal(hotplug_cfg(rx_dev), 0, "RX reconfigure");
	hotplug_drain(rx_dev);

	t_plug = k_uptime_get();
	uart_poll_out(tx_dev, 0xA5);

	wait_deadline = t_plug + 200;
	while (k_uptime_get() < wait_deadline) {
		uint8_t c;

		if (uart_poll_in(rx_dev, &c) == 0) {
			t_rx = k_uptime_get();
			zassert_equal(c, 0xA5,
				      "First byte after remount was 0x%02x", c);
			break;
		}
		k_sleep(K_MSEC(1));
	}

	zassert_true(t_rx >= 0, "No RX after remount (plug not detected)");
	plug_ms = (int)(t_rx - t_plug);
	TC_PRINT("Plug-back detected in %d ms (first RX 0xA5)\n", plug_ms);

	zassert_equal(hotplug_roundtrip(0x5A), 0,
		      "UART did not recover after remount");
	TC_PRINT("Post-replug 0x5A round-trip OK\n");
}


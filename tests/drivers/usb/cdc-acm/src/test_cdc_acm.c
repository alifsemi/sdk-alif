/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/ztest.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/ring_buffer.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/kernel.h>
#include <string.h>
#include <sample_usbd.h>

#define TEST_BUF_SIZE       64
#define STRESS_ITERATIONS   50
#define RING_BUF_SIZE       256

/*
 * ZTEST fixture: holds all shared test state.
 * Allocated once by cdc_suite_setup(), passed to every ZTEST_F test.
 */
struct cdc_acm_qa_fixture {
	const struct device *cdc_dev;
	struct usbd_context *usbd_ctx;
};

/*
 * IRQ callback state — must be file-scope because the ISR
 * context cannot receive the fixture pointer.
 */
static volatile bool irq_cb_called;
static volatile bool irq_tx_ready;
static volatile bool irq_rx_ready;
static const struct device *irq_cb_dev;

static void cdc_irq_cb(const struct device *dev, void *user_data)
{
	ARG_UNUSED(user_data);

	irq_cb_called = true;
	irq_cb_dev = dev;

	if (uart_irq_tx_ready(dev)) {
		irq_tx_ready = true;
		uart_irq_tx_disable(dev);
	}

	if (uart_irq_rx_ready(dev)) {
		irq_rx_ready = true;
	}
}

/*
 * Minimal IRQ callback for the stress test.
 * Only records invocation; does NOT disable TX IRQ so that
 * the test loop retains full control over enable/disable.
 */
static void cdc_irq_stress_cb(const struct device *dev, void *user_data)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(user_data);
}

static void *cdc_suite_setup(void)
{
	static struct cdc_acm_qa_fixture fixture;
	int err;

	/* Device lookup — validates DT node and driver readiness */
	fixture.cdc_dev = DEVICE_DT_GET_ONE(zephyr_cdc_acm_uart);
	zassert_not_null(fixture.cdc_dev, "CDC ACM device not found in DT");
	zassert_true(device_is_ready(fixture.cdc_dev), "CDC ACM device not ready");

	/* Initialize and enable USB device stack (next) */
	fixture.usbd_ctx = sample_usbd_init_device(NULL);
	zassert_not_null(fixture.usbd_ctx,
			 "USB device init failed (check HW connection)");

	err = usbd_enable(fixture.usbd_ctx);
	zassert_equal(err, 0, "usbd_enable failed (%d)", err);

	TC_PRINT("USB device stack initialized and enabled\n");
	/* Allow USB enumeration time */
	k_sleep(K_MSEC(500));

	return &fixture;
}

static void cdc_suite_before(void *f)
{
	ARG_UNUSED(f);

	/* Reset IRQ tracking flags before each test */
	irq_cb_called = false;
	irq_tx_ready = false;
	irq_rx_ready = false;
	irq_cb_dev = NULL;
}

static void cdc_suite_teardown(void *f)
{
	ARG_UNUSED(f);

	/*
	 * Keep USB enabled after tests so the host can enumerate the
	 * CDC-ACM device and /dev/ttyACMx appears on the host PC.
	 */
	TC_PRINT("USB device stack remains enabled for host enumeration\n");
}

/*
 * USB Disable and Re-enable
 *
 * Purpose : Ensure the USB device stack can be cleanly disabled and then
 *           re-enabled without errors, simulating a cable unplug/replug.
 * Method  : Call usbd_disable(), sleep 100 ms, then usbd_enable() and
 *           sleep 500 ms to allow re-enumeration.
 * Pass    : Both usbd_disable() and usbd_enable() return 0.
 * Fail    : Either call returns a non-zero error code.
 */
ZTEST_F(cdc_acm_qa, test_usb_disable_reenable)
{
	int err;

	err = usbd_disable(fixture->usbd_ctx);
	zassert_equal(err, 0, "usbd_disable failed (%d)", err);
	TC_PRINT("USB disabled\n");

	k_sleep(K_MSEC(100));

	err = usbd_enable(fixture->usbd_ctx);
	zassert_equal(err, 0, "usbd_enable (re-enable) failed (%d)", err);
	TC_PRINT("USB re-enabled\n");

	k_sleep(K_MSEC(500));
}

/*
 * USB Speed Capability
 *
 * Purpose : Query and validate the speed capability reported by the USB
 *           device controller (DWC3 on Alif platforms).
 * Method  : Call usbd_caps_speed() and verify the result is either
 *           USBD_SPEED_FS (Full-Speed, 12 Mbps) or USBD_SPEED_HS
 *           (High-Speed, 480 Mbps).
 * Pass    : Reported speed is Full-Speed or High-Speed.
 * Fail    : Reported speed is an unexpected/unknown value.
 */
ZTEST_F(cdc_acm_qa, test_usb_speed_capability)
{
	enum usbd_speed speed;

	speed = usbd_caps_speed(fixture->usbd_ctx);
	TC_PRINT("USB controller speed capability: %s\n",
		 speed == USBD_SPEED_HS ? "High-Speed" :
		 speed == USBD_SPEED_FS ? "Full-Speed" : "Unknown");

	/* DWC3 on Alif should support at least FS */
	zassert_true(speed == USBD_SPEED_FS || speed == USBD_SPEED_HS,
			"Unexpected speed capability %d", speed);
}

/*
 * Line Control
 *
 * Purpose : Exercise all UART line control enums in a single test:
 *           read DTR, baudrate, and RTS; assert DCD and DSR towards
 *           the host when a host terminal is connected.
 * Method  :
 *   1. DTR  — uart_line_ctrl_get(DTR): must succeed.
 *   2. Baudrate — uart_line_ctrl_get(BAUD_RATE): logged; -ENOTSUP is
 *                 acceptable (driver may not track host line coding).
 *   3. RTS  — uart_line_ctrl_get(RTS): logged; -ENOTSUP is acceptable.
 *   4. DCD/DSR set — uart_line_ctrl_set(DCD/DSR): only attempted when
 *                    DTR is asserted (host terminal open). These calls
 *                    send CDC SERIAL_STATE notifications on the interrupt
 *                    endpoint; attempting them without a host blocks
 *                    forever. -ENOTSUP and -EACCES are acceptable.
 * Pass    : DTR read returns 0; all other operations either succeed or
 *           return an expected -ENOTSUP / -EACCES.
 * Fail    : Any call returns an unexpected error code.
 */
ZTEST_F(cdc_acm_qa, test_line_ctrl)
{
	uint32_t val = 0;
	int ret;

	/* DTR — always expected to succeed */
	ret = uart_line_ctrl_get(fixture->cdc_dev, UART_LINE_CTRL_DTR, &val);
	zassert_equal(ret, 0, "Failed to get DTR (%d)", ret);
	TC_PRINT("DTR: %u\n", val);

	/* Baudrate — driver may not expose host line coding */
	val = 0;
	ret = uart_line_ctrl_get(fixture->cdc_dev, UART_LINE_CTRL_BAUD_RATE, &val);
	if (ret == -ENOTSUP) {
		TC_PRINT("Baudrate: not supported by driver\n");
	} else {
		zassert_equal(ret, 0, "Failed to get baudrate (%d)", ret);
		zassert_not_equal(val, 0, "Baudrate should not be zero on success");
		TC_PRINT("Baudrate: %u\n", val);
	}

	/* RTS — driver may not expose this signal */
	val = 0;
	ret = uart_line_ctrl_get(fixture->cdc_dev, UART_LINE_CTRL_RTS, &val);
	if (ret == -ENOTSUP) {
		TC_PRINT("RTS: not supported by driver\n");
	} else {
		zassert_equal(ret, 0, "Failed to get RTS (%d)", ret);
		TC_PRINT("RTS: %u\n", val);
	}

	/*
	 * DCD and DSR set send CDC SERIAL_STATE notifications via the
	 * interrupt endpoint. If no host is polling that endpoint the
	 * transfer blocks forever — guard on DTR before attempting them.
	 */
	val = 0;
	ret = uart_line_ctrl_get(fixture->cdc_dev, UART_LINE_CTRL_DTR, &val);
	if (ret != 0 || val == 0) {
		TC_PRINT("No host connected (DTR=0), skipping DCD/DSR set\n");
		return;
	}

	/* DCD set */
	ret = uart_line_ctrl_set(fixture->cdc_dev, UART_LINE_CTRL_DCD, 1);
	if (ret == -ENOTSUP || ret == -EACCES) {
		TC_PRINT("DCD set not available (ret=%d), skipping\n", ret);
	} else {
		zassert_equal(ret, 0, "Failed to set DCD (%d)", ret);
		TC_PRINT("DCD set successfully\n");
	}

	/* DSR set */
	ret = uart_line_ctrl_set(fixture->cdc_dev, UART_LINE_CTRL_DSR, 1);
	if (ret == -ENOTSUP || ret == -EACCES) {
		TC_PRINT("DSR set not available (ret=%d), skipping\n", ret);
	} else {
		zassert_equal(ret, 0, "Failed to set DSR (%d)", ret);
		TC_PRINT("DSR set successfully\n");
	}
}

/*
 * UART Config Get
 *
 * Purpose : Retrieve the current virtual UART configuration (baud rate,
 *           parity, stop bits, data bits, flow control) from the
 *           CDC-ACM driver.
 * Method  : Call uart_config_get() and print the returned values.
 *           The call may return -ENOTSUP if the driver does not cache
 *           the host-supplied line coding.
 * Pass    : The API returns 0 and the config values are printed.
 * Skip    : The API returns -ENOTSUP (driver does not cache line coding).
 * Fail    : The API returns any other error code.
 */
ZTEST_F(cdc_acm_qa, test_uart_config_get)
{
	struct uart_config cfg;
	int ret;

	ret = uart_config_get(fixture->cdc_dev, &cfg);
	if (ret == -ENOTSUP) {
		TC_PRINT("uart_config_get not supported by driver, skipping\n");
		ztest_test_skip();
		return;
	}
	zassert_equal(ret, 0, "uart_config_get failed (%d)", ret);
	TC_PRINT("UART config: baudrate=%u, parity=%u, stop_bits=%u, "
		 "data_bits=%u, flow_ctrl=%u\n",
		 cfg.baudrate, cfg.parity, cfg.stop_bits,
		 cfg.data_bits, cfg.flow_ctrl);
}

/*
 * UART Config Set
 *
 * Purpose : Apply a standard 115200/8N1 UART configuration to the
 *           CDC-ACM device and verify the driver accepts it.
 * Method  : Populate a uart_config struct with 115200 baud, no parity,
 *           1 stop bit, 8 data bits, no flow control, then call
 *           uart_configure().
 * Pass    : The API returns 0 (configuration accepted by driver).
 * Skip    : The API returns -ENOTSUP (driver does not support configure).
 * Fail    : The API returns any other error code.
 */
ZTEST_F(cdc_acm_qa, test_uart_config_set)
{
	struct uart_config cfg = {
		.baudrate = 115200,
		.parity = UART_CFG_PARITY_NONE,
		.stop_bits = UART_CFG_STOP_BITS_1,
		.data_bits = UART_CFG_DATA_BITS_8,
		.flow_ctrl = UART_CFG_FLOW_CTRL_NONE,
	};
	int ret;

	ret = uart_configure(fixture->cdc_dev, &cfg);
	if (ret == -ENOTSUP) {
		TC_PRINT("uart_configure not supported by driver, skipping\n");
		ztest_test_skip();
		return;
	}
	zassert_equal(ret, 0, "uart_configure failed (%d)", ret);
	TC_PRINT("uart_configure(115200,8N1): success\n");
}

/*
 * IRQ Callback Set
 *
 * Purpose : Register an interrupt-driven callback function with the
 *           CDC-ACM UART driver and verify the callback is actually
 *           invoked when a TX interrupt is triggered.
 * Method  : Call uart_irq_callback_set() with cdc_irq_cb, then enable
 *           TX IRQ (TX-ready fires immediately when the buffer is empty),
 *           sleep briefly, and verify irq_cb_called is true and that the
 *           callback received the correct device pointer.
 * Pass    : The API returns 0, the callback fires, and the device pointer
 *           inside the callback matches cdc_dev.
 * Fail    : Callback not invoked, or device pointer mismatch.
 * Skip    : The driver does not support IRQ callbacks (-ENOTSUP).
 */
ZTEST_F(cdc_acm_qa, test_uart_irq_callback_set)
{
	int ret;

	ret = uart_irq_callback_set(fixture->cdc_dev, cdc_irq_cb);
	if (ret == -ENOTSUP) {
		TC_PRINT("IRQ callback not supported, skipping\n");
		ztest_test_skip();
		return;
	}
	zassert_equal(ret, 0, "irq callback set failed (%d)", ret);

	/* Trigger a TX-ready IRQ to verify the callback fires */
	uart_irq_tx_enable(fixture->cdc_dev);
	k_sleep(K_MSEC(50));

	zassert_true(irq_cb_called,
		     "IRQ callback was not invoked after TX enable");
	zassert_equal(irq_cb_dev, fixture->cdc_dev,
		      "IRQ callback received wrong device pointer");
	TC_PRINT("IRQ callback set and verified successfully\n");
}

/*
 * IRQ RX Enable/Disable
 *
 * Purpose : Verify that the RX interrupt can be enabled and subsequently
 *           disabled without causing a crash or assertion failure.
 *           Also resets IRQ tracking flags to confirm no spurious RX-ready
 *           event is reported when no host data has been sent.
 * Method  : Register the IRQ callback (skip if unsupported), reset IRQ
 *           flags, enable RX IRQ, wait 50 ms, then disable RX IRQ.
 *           Log whether an RX-ready event was reported (informational,
 *           since actual RX data depends on the host).
 * Pass    : Enable and disable complete without crash or assert.
 * Fail    : Assertion or crash during enable/disable.
 * Skip    : IRQ callback registration not supported.
 */
ZTEST_F(cdc_acm_qa, test_uart_irq_rx_enable_disable)
{
	int ret;

	ret = uart_irq_callback_set(fixture->cdc_dev, cdc_irq_cb);
	if (ret == -ENOTSUP) {
		ztest_test_skip();
		return;
	}
	zassert_equal(ret, 0, "uart_irq_callback_set failed (%d)", ret);

	uart_irq_rx_enable(fixture->cdc_dev);
	TC_PRINT("RX IRQ enabled\n");
	k_sleep(K_MSEC(50));

	uart_irq_rx_disable(fixture->cdc_dev);
	TC_PRINT("RX IRQ disabled (callback_invoked=%s, rx_ready=%s)\n",
		 irq_cb_called ? "yes" : "no",
		 irq_rx_ready ? "yes" : "no");
}

/*
 * IRQ TX Enable/Disable
 *
 * Purpose : Verify that the TX interrupt fires and reports TX-ready when
 *           enabled, and can be subsequently disabled without crash.
 *           TX-ready is expected to fire immediately because the transmit
 *           buffer starts empty.
 * Method  : Register the IRQ callback (skip if unsupported), reset IRQ
 *           flags, enable TX IRQ, wait 50 ms, then verify irq_cb_called
 *           and irq_tx_ready are both true. The callback itself disables
 *           TX IRQ after detecting TX-ready to prevent repeated firing.
 * Pass    : Callback invoked, TX-ready detected, no crash.
 * Fail    : Callback not invoked or TX-ready not reported.
 * Skip    : IRQ callback registration not supported.
 */
ZTEST_F(cdc_acm_qa, test_uart_irq_tx_enable_disable)
{
	int ret;

	ret = uart_irq_callback_set(fixture->cdc_dev, cdc_irq_cb);
	if (ret == -ENOTSUP) {
		ztest_test_skip();
		return;
	}
	zassert_equal(ret, 0, "uart_irq_callback_set failed (%d)", ret);

	uart_irq_tx_enable(fixture->cdc_dev);
	TC_PRINT("TX IRQ enabled\n");
	k_sleep(K_MSEC(50));

	zassert_true(irq_cb_called,
		     "IRQ callback was not invoked after TX enable");
	zassert_true(irq_tx_ready,
		     "TX ready was not reported in IRQ callback");
	TC_PRINT("TX IRQ verified (callback_invoked=yes, tx_ready=yes)\n");
}

/*
 * UART Poll Out (single byte)
 *
 * Purpose : Transmit a single byte ('T') through the CDC-ACM bulk IN
 *           endpoint using the polling API and confirm no crash occurs.
 * Precond : USB stack must be initialized and enabled.
 * Method  : Call uart_poll_out() with the character 'T'.
 * Pass    : The call completes without crash or hang.
 * Fail    : The call triggers an assertion, crash, or hang.
 */
ZTEST_F(cdc_acm_qa, test_uart_poll_out)
{
	/* Send a single character via poll out - should not crash */
	uart_poll_out(fixture->cdc_dev, 'T');
	TC_PRINT("uart_poll_out('T') executed\n");
}

/*
 * UART Bulk Write via poll_out
 *
 * Purpose : Transmit a multi-byte string ("USB-CDC-TEST\r\n") through
 *           the CDC-ACM bulk IN endpoint using uart_poll_out().
 * Note    : uart_fifo_fill() must be called from the CDC-ACM work-queue
 *           context (inside the IRQ callback). Calling it directly from
 *           a test thread triggers an assertion in usbd_cdc_acm.c.
 *           Therefore this test uses uart_poll_out() in a loop as a
 *           thread-safe alternative to exercise the bulk data path.
 * Precond : USB stack must be initialized and enabled.
 * Method  : Loop over each byte of the test string and call
 *           uart_poll_out() for each byte.
 * Pass    : All bytes are queued without crash.
 * Fail    : Crash, assertion, or hang during transmission.
 */
ZTEST_F(cdc_acm_qa, test_uart_bulk_write_poll_out)
{
	/*
	 * uart_fifo_fill() must be called from the CDC-ACM work queue
	 * context (i.e., inside the IRQ callback). Calling it directly
	 * from a test thread triggers an assertion in usbd_cdc_acm.c.
	 * Instead, verify that poll_out (which is thread-safe) can
	 * queue multiple bytes for the bulk IN endpoint.
	 */
	uint8_t test_data[] = "USB-CDC-TEST\r\n";

	for (int i = 0; i < (int)(sizeof(test_data) - 1); i++) {
		uart_poll_out(fixture->cdc_dev, test_data[i]);
	}
	TC_PRINT("uart_fifo_fill (via poll_out): %d bytes queued\n",
		 (int)(sizeof(test_data) - 1));
}

/*
 * Ring Buffer Echo Logic
 *
 * Purpose : Validate the ring-buffer-based echo path that a real CDC-ACM
 *           echo application would use: data written in equals data read
 *           out, byte-for-byte.
 * Method  : Initialize a 64-byte ring buffer, put the string
 *           "cdc-acm-test" into it, get the same number of bytes out,
 *           and compare with zassert_mem_equal().
 * Pass    : All bytes match between put and get.
 * Fail    : Byte count mismatch or data content mismatch.
 */
ZTEST(cdc_acm_basic, test_ring_buffer_echo_logic)
{
	uint8_t rb_buf[TEST_BUF_SIZE];
	struct ring_buf rb;
	uint8_t tx_data[] = "cdc-acm-test";
	uint8_t rx_data[sizeof(tx_data)];

	ring_buf_init(&rb, TEST_BUF_SIZE, rb_buf);

	int put = ring_buf_put(&rb, tx_data, sizeof(tx_data));

	zassert_equal(put, sizeof(tx_data), "Ring buf put failed");

	int get = ring_buf_get(&rb, rx_data, sizeof(rx_data));

	zassert_equal(get, sizeof(rx_data), "Ring buf get failed");

	zassert_mem_equal(tx_data, rx_data, sizeof(tx_data),
			"Echo data mismatch");
	TC_PRINT("Ring buffer echo: %d bytes matched\n", get);
}

/*
 * Ring Buffer Full Capacity
 *
 * Purpose : Stress the ring buffer by filling it to its maximum capacity
 *           (256 bytes) with a known incrementing pattern and verifying
 *           all data is read back without corruption.
 * Method  : Write a 256-byte pattern (0x00..0xFF) into the ring buffer,
 *           read it back, and compare with zassert_mem_equal(). Note:
 *           the ring buffer may not accept all 256 bytes due to internal
 *           overhead, so the actual accepted count is checked.
 * Pass    : All accepted bytes are read back and match the pattern.
 * Fail    : Put returns 0, get count differs from put, or data mismatch.
 */
ZTEST(cdc_acm_basic, test_ring_buffer_full_capacity)
{
	uint8_t rb_buf[RING_BUF_SIZE];
	struct ring_buf rb;
	uint8_t write_buf[RING_BUF_SIZE];
	uint8_t read_buf[RING_BUF_SIZE];
	int put, get;

	ring_buf_init(&rb, RING_BUF_SIZE, rb_buf);

	/* Fill with known pattern */
	for (int i = 0; i < RING_BUF_SIZE; i++) {
		write_buf[i] = (uint8_t)(i & 0xFF);
	}

	put = ring_buf_put(&rb, write_buf, RING_BUF_SIZE);
	/* Ring buf may not accept all bytes (internal overhead) */
	zassert_true(put > 0, "Ring buf put failed (%d)", put);

	get = ring_buf_get(&rb, read_buf, put);
	zassert_equal(get, put, "Ring buf get mismatch: put=%d get=%d", put, get);
	zassert_mem_equal(write_buf, read_buf, get,
			"Data corruption in full capacity test");
	TC_PRINT("Ring buffer full capacity: %d/%d bytes verified\n",
			get, RING_BUF_SIZE);
}

/*
 * Ring Buffer Wrap-around
 *
 * Purpose : Verify correct ring buffer behavior when internal read/write
 *           pointers wrap past the end of the backing array.
 * Method  : Use a 32-byte ring buffer. First write 20 bytes (0xAA) and
 *           read them back to advance the pointers past the midpoint.
 *           Then write another 20 bytes (0xBB) — this second write
 *           forces a wrap-around. Read back and confirm every byte
 *           equals 0xBB.
 * Pass    : All bytes in the second read are 0xBB (no corruption).
 * Fail    : Any byte differs from 0xBB, or put/get counts mismatch.
 */
ZTEST(cdc_acm_basic, test_ring_buffer_wraparound)
{
	uint8_t rb_buf[32];
	struct ring_buf rb;
	uint8_t data[20];
	uint8_t out[20];
	int put, get;

	ring_buf_init(&rb, sizeof(rb_buf), rb_buf);

	memset(data, 0xAA, sizeof(data));

	/* First write + read to advance pointers */
	put = ring_buf_put(&rb, data, 20);
	zassert_true(put > 0, "First put failed");
	get = ring_buf_get(&rb, out, put);
	zassert_equal(get, put, "First get failed");

	/* Second write wraps around */
	memset(data, 0xBB, sizeof(data));
	put = ring_buf_put(&rb, data, 20);
	zassert_true(put > 0, "Wraparound put failed");
	get = ring_buf_get(&rb, out, put);
	zassert_equal(get, put, "Wraparound get failed");

	for (int i = 0; i < get; i++) {
		zassert_equal(out[i], 0xBB,
				"Wraparound data corruption at byte %d", i);
	}
	TC_PRINT("Ring buffer wraparound verified (%d bytes)\n", get);
}

/*
 * UART Poll Out Stress
 *
 * Purpose : Stress the CDC-ACM bulk IN data path by rapidly sending
 *           50 characters ('A'–'Z' cycling) via uart_poll_out().
 * Precond : USB stack must be initialized and enabled.
 * Method  : Loop STRESS_ITERATIONS (50) times, calling uart_poll_out()
 *           with successive alphabetic characters.
 * Pass    : All 50 calls complete without crash, hang, or data loss.
 * Fail    : Crash, assertion, or hang during the loop.
 */
ZTEST_F(cdc_acm_qa, test_uart_poll_out_stress)
{
	for (int i = 0; i < STRESS_ITERATIONS; i++) {
		uart_poll_out(fixture->cdc_dev, (uint8_t)('A' + (i % 26)));
	}
	TC_PRINT("Poll out stress: %d chars sent\n", STRESS_ITERATIONS);
}

/*
 * Line Control DTR Repeated Read
 *
 * Purpose : Stress-test the DTR line-control read path by performing
 *           50 consecutive reads to ensure the API is stable and
 *           returns consistent results under rapid repeated access.
 * Method  : Loop STRESS_ITERATIONS (50) times, each time calling
 *           uart_line_ctrl_get(UART_LINE_CTRL_DTR) and asserting
 *           a return value of 0.
 * Pass    : All 50 reads return 0 (success).
 * Fail    : Any read returns a non-zero error code.
 */
ZTEST_F(cdc_acm_qa, test_line_ctrl_dtr_stress)
{
	uint32_t dtr;
	int ret;

	for (int i = 0; i < STRESS_ITERATIONS; i++) {
		dtr = 0;
		ret = uart_line_ctrl_get(fixture->cdc_dev, UART_LINE_CTRL_DTR, &dtr);
		zassert_equal(ret, 0,
				"DTR read failed at iteration %d (%d)", i, ret);
	}
	TC_PRINT("DTR stress: %d reads OK\n", STRESS_ITERATIONS);
}

/*
 * USB Disable/Enable Stress
 *
 * Purpose : Verify the USB device stack can survive multiple rapid
 *           disable/enable cycles (simulating repeated cable
 *           unplug/replug) without errors or resource leaks.
 * Precond : USB stack must be initialized and enabled.
 * Method  : Perform 5 cycles of usbd_disable() (100 ms wait) followed
 *           by usbd_enable() (500 ms wait for enumeration). Assert
 *           both calls return 0 in every cycle. A final 500 ms sleep
 *           allows the controller to stabilize.
 * Pass    : All 5 disable/enable cycles complete with return code 0.
 * Fail    : Any call returns a non-zero error.
 */
ZTEST_F(cdc_acm_qa, test_usb_disable_enable_stress)
{
	int err;
	int cycles = 5;

	for (int i = 0; i < cycles; i++) {
		err = usbd_disable(fixture->usbd_ctx);
		zassert_equal(err, 0,
				"Disable failed at cycle %d (%d)", i, err);

		k_sleep(K_MSEC(100));

		err = usbd_enable(fixture->usbd_ctx);
		zassert_equal(err, 0,
				"Enable failed at cycle %d (%d)", i, err);

		k_sleep(K_MSEC(500));
	}
	/* Allow controller to fully stabilize after last enable */
	k_sleep(K_MSEC(500));
	TC_PRINT("USB disable/enable stress: %d cycles completed\n", cycles);
}

/*
 * VBUS Detection Capability Query
 *
 * Purpose : Query whether the USB device controller supports hardware
 *           VBUS detection (i.e., can detect whether a USB cable is
 *           physically connected to a host).
 * Precond : USB stack must be initialized.
 * Method  : Call usbd_can_detect_vbus() and print the result. This is
 *           an informational test — both true and false are valid.
 * Pass    : The API call completes without crash; result is logged.
 */
ZTEST_F(cdc_acm_qa, test_usb_vbus_detection)
{
	bool can_detect;

	can_detect = usbd_can_detect_vbus(fixture->usbd_ctx);
	TC_PRINT("VBUS detection supported: %s\n",
		 can_detect ? "yes" : "no");
}

/*
 * IRQ Enable/Disable Stress
 *
 * Purpose : Stress-test the UART IRQ enable/disable paths by rapidly
 *           toggling both RX and TX interrupts 50 times to ensure no
 *           race conditions, resource leaks, or assertion failures.
 * Method  : Register a minimal no-op IRQ callback (skip if unsupported),
 *           then loop STRESS_ITERATIONS (50) times. Each iteration
 *           enables RX, enables TX, disables TX, and disables RX in
 *           sequence. A dedicated stress callback is used instead of
 *           cdc_irq_cb to avoid conflicting TX-disable logic.
 * Pass    : All 50 iterations complete without crash or assertion.
 * Fail    : Crash, assertion, or hang during any iteration.
 * Skip    : IRQ callback registration not supported (-ENOTSUP).
 */
ZTEST_F(cdc_acm_qa, test_uart_irq_stress)
{
	int ret;

	ret = uart_irq_callback_set(fixture->cdc_dev, cdc_irq_stress_cb);
	if (ret == -ENOTSUP) {
		ztest_test_skip();
		return;
	}
	zassert_equal(ret, 0, "uart_irq_callback_set failed (%d)", ret);

	for (int i = 0; i < STRESS_ITERATIONS; i++) {
		uart_irq_rx_enable(fixture->cdc_dev);
		uart_irq_tx_enable(fixture->cdc_dev);
		uart_irq_tx_disable(fixture->cdc_dev);
		uart_irq_rx_disable(fixture->cdc_dev);
	}
	TC_PRINT("IRQ enable/disable stress: %d iterations OK\n",
		 STRESS_ITERATIONS);
}

/* ---------- Test Suites ---------- */

/* Software-only tests (ring buffer logic) — no USB hardware required */
ZTEST_SUITE(cdc_acm_basic,
		NULL,					/* predicate */
		NULL,					/* suite setup */
		NULL,					/* before each */
		NULL,					/* after each */
		NULL);					/* suite teardown */

/* USB/CDC-ACM hardware tests — require USB device connected */
ZTEST_SUITE(cdc_acm_qa,
		NULL,					/* predicate */
		cdc_suite_setup,		/* suite setup (returns fixture) */
		cdc_suite_before,		/* before each test */
		NULL,					/* after each test */
		cdc_suite_teardown);	/* suite teardown */

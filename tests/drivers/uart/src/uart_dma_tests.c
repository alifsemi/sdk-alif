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
 * @brief UART DMA (async API) test cases.
 *
 * Tests the UART asynchronous DMA transfer path using the Zephyr
 * async UART API: uart_callback_set(), uart_rx_enable(), uart_tx(),
 * uart_rx_buf_rsp(), and uart_tx_abort().
 *
 * Common per-test setup (device-readiness check, primitive reset and
 * callback installation) is performed by the ztest before hook, and the
 * after hook releases the DMA buffers so every test starts and ends from
 * a clean, known state regardless of pass/fail.
 */

#include <string.h>
#include "test_uart.h"

#if CONFIG_TEST_UART_DMA

#define RX_BUF_SIZE        64
#define TX_BUF_SIZE        64
#define TX_ABORT_BUF_SIZE  2048

/*
 * RX may be fragmented across multiple UART_RX_RDY events. Each fragment
 * is pushed onto a message queue so that events arriving before the test
 * thread consumes them are preserved. A single semaphore would coalesce
 * (max-count) the events and the shared globals would be overwritten,
 * dropping earlier fragments.
 */
#define RX_MSGQ_DEPTH      16

struct rx_chunk {
	uint8_t *buf;
	size_t   offset;
	size_t   len;
};

static uint8_t rx_dma_buf_a[RX_BUF_SIZE];
static uint8_t rx_dma_buf_b[RX_BUF_SIZE];
static uint8_t tx_dma_buf[TX_BUF_SIZE];
static uint8_t tx_abort_buf[TX_ABORT_BUF_SIZE];

K_MSGQ_DEFINE(rx_msgq, sizeof(struct rx_chunk), RX_MSGQ_DEPTH, 4);

static struct k_sem tx_done_sem;
static struct k_sem rx_disabled_sem;
static struct k_sem buf_req_sem;
static struct k_sem buf_released_sem;

static volatile int last_tx_evt;

/* DMA-capable UART device under test (devnode1), resolved in the before hook. */
static const struct device *dma_uart_dev;

static void uart_dma_cb(const struct device *dev, struct uart_event *evt,
			void *user_data)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(user_data);

	switch (evt->type) {
	case UART_RX_RDY: {
		struct rx_chunk chunk = {
			.buf    = evt->data.rx.buf,
			.offset = evt->data.rx.offset,
			.len    = evt->data.rx.len,
		};

		(void)k_msgq_put(&rx_msgq, &chunk, K_NO_WAIT);
		break;
	}
	case UART_TX_DONE:
		last_tx_evt = evt->type;
		TC_PRINT("UART_TX_DONE\n");
		k_sem_give(&tx_done_sem);
		break;
	case UART_TX_ABORTED:
		last_tx_evt = evt->type;
		TC_PRINT("UART_TX_ABORTED\n");
		k_sem_give(&tx_done_sem);
		break;
	case UART_RX_BUF_REQUEST:
		TC_PRINT("UART_RX_BUF_REQUEST\n");
		k_sem_give(&buf_req_sem);
		break;
	case UART_RX_BUF_RELEASED:
		TC_PRINT("UART_RX_BUF_RELEASED\n");
		k_sem_give(&buf_released_sem);
		break;
	case UART_RX_DISABLED:
		TC_PRINT("UART_RX_DISABLED\n");
		k_sem_give(&rx_disabled_sem);
		break;
	default:
		break;
	}
}

static int dma_uart_send(const struct device *dev, const uint8_t *data,
			 size_t len)
{
	int ret;

	ret = uart_tx(dev, data, len, SYS_FOREVER_US);
	if (ret < 0) {
		return ret;
	}

	k_sem_take(&tx_done_sem, K_FOREVER);
	return 0;
}

/*
 * Per-test setup, invoked by the ztest framework before every test in
 * the uart_dma suite. Validates device readiness, resets all
 * synchronization primitives and buffers, and (re)installs the async
 * callback so each test starts from a clean, known state.
 */
static void uart_dma_before(void *fixture)
{
	ARG_UNUSED(fixture);
	int ret;

	dma_uart_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_devnode1));
	zassert_true(device_is_ready(dma_uart_dev),
		     "%s UART device not ready", dma_uart_dev->name);
	TC_PRINT("%s UART device is ready\n", dma_uart_dev->name);

	k_sem_init(&tx_done_sem, 0, 1);
	k_sem_init(&rx_disabled_sem, 0, 1);
	k_sem_init(&buf_req_sem, 0, 1);
	k_sem_init(&buf_released_sem, 0, 1);
	k_msgq_purge(&rx_msgq);

	last_tx_evt = -1;

	memset(rx_dma_buf_a, 0, RX_BUF_SIZE);
	memset(rx_dma_buf_b, 0, RX_BUF_SIZE);
	memset(tx_dma_buf, 0, TX_BUF_SIZE);

	ret = uart_callback_set(dma_uart_dev, uart_dma_cb, NULL);
	zassert_equal(ret, 0, "uart_callback_set failed (ret=%d)", ret);
}

/*
 * Per-test teardown: abort any in-flight TX and disable RX so the DMA
 * buffers are released and re-acquired cleanly between test cases,
 * regardless of whether the test passed or failed.
 */
static void uart_dma_after(void *fixture)
{
	ARG_UNUSED(fixture);

	if (dma_uart_dev == NULL) {
		return;
	}

	(void)uart_tx_abort(dma_uart_dev);
	(void)uart_rx_disable(dma_uart_dev);
}

/**
 * @brief Basic DMA TX/RX loopback test.
 *
 * Transmits a short string via DMA and verifies the identical
 * payload is received back via DMA RX.
 */
ZTEST(uart_dma, test_dma_basic_tx_rx)
{
	int ret;
	uint8_t tx_payload[] = "DMA loopback test!";
	size_t tx_len = strlen((char *)tx_payload);

	ret = uart_rx_enable(dma_uart_dev, rx_dma_buf_a, RX_BUF_SIZE, 1000);
	zassert_equal(ret, 0, "uart_rx_enable failed (ret=%d)", ret);

	memcpy(tx_dma_buf, tx_payload, tx_len);
	ret = dma_uart_send(dma_uart_dev, tx_dma_buf, tx_len);
	zassert_equal(ret, 0, "dma_uart_send failed (ret=%d)", ret);

	/*
	 * RX may be fragmented across multiple UART_RX_RDY events, so
	 * accumulate and verify chunks from the queue until the full
	 * payload arrives.
	 */
	size_t received = 0;

	while (received < tx_len) {
		struct rx_chunk chunk;

		ret = k_msgq_get(&rx_msgq, &chunk, K_MSEC(5000));
		zassert_equal(ret, 0, "RX timeout waiting for data");
		zassert_not_null(chunk.buf, "RX buffer is NULL");
		zassert_true(chunk.offset + chunk.len <= RX_BUF_SIZE,
			     "RX chunk out of bounds (offset=%zu len=%zu)",
			     chunk.offset, chunk.len);
		zassert_true(received + chunk.len <= tx_len,
			     "RX longer than expected (received=%zu chunk=%zu expected=%zu)",
			     received, chunk.len, tx_len);
		zassert_equal(memcmp(chunk.buf + chunk.offset,
				     tx_payload + received, chunk.len), 0,
			      "RX data mismatch");
		received += chunk.len;
	}

	zassert_equal(received, tx_len,
		      "RX length mismatch: expected %zu, got %zu",
		      tx_len, received);

	ret = uart_rx_disable(dma_uart_dev);
	zassert_equal(ret, 0, "uart_rx_disable failed (ret=%d)", ret);

	ret = k_sem_take(&rx_disabled_sem, K_MSEC(1000));
	zassert_equal(ret, 0, "RX disabled event timeout");

	TC_PRINT("DMA basic TX/RX test PASSED\n");
}

/**
 * @brief DMA RX double-buffer request/release test.
 *
 * Enables double-buffered DMA RX, fills both buffers, and verifies
 * that UART_RX_BUF_REQUEST and UART_RX_BUF_RELEASED events fire.
 */
ZTEST(uart_dma, test_dma_rx_buf_request_release)
{
	int ret;
	uint8_t tx_payload[RX_BUF_SIZE * 2];
	size_t tx_len = sizeof(tx_payload);
	struct rx_chunk chunk;

	for (size_t i = 0; i < tx_len; i++) {
		tx_payload[i] = (uint8_t)i;
	}

	ret = uart_rx_enable(dma_uart_dev, rx_dma_buf_a, RX_BUF_SIZE, 1000);
	zassert_equal(ret, 0, "uart_rx_enable failed (ret=%d)", ret);

	/* Start TX without waiting so we can respond to RX buffer requests. */
	ret = uart_tx(dma_uart_dev, tx_payload, tx_len, SYS_FOREVER_US);
	zassert_equal(ret, 0, "uart_tx failed (ret=%d)", ret);

	ret = k_sem_take(&buf_req_sem, K_MSEC(5000));
	zassert_equal(ret, 0, "UART_RX_BUF_REQUEST event timeout");

	ret = uart_rx_buf_rsp(dma_uart_dev, rx_dma_buf_b, RX_BUF_SIZE);
	zassert_equal(ret, 0, "uart_rx_buf_rsp failed (ret=%d)", ret);

	ret = k_sem_take(&tx_done_sem, K_MSEC(5000));
	zassert_equal(ret, 0, "TX timeout waiting for completion");

	ret = k_msgq_get(&rx_msgq, &chunk, K_MSEC(5000));
	zassert_equal(ret, 0, "RX timeout waiting for data");

	ret = k_sem_take(&buf_released_sem, K_MSEC(1000));
	zassert_equal(ret, 0, "UART_RX_BUF_RELEASED event timeout");

	ret = uart_rx_disable(dma_uart_dev);
	zassert_equal(ret, 0, "uart_rx_disable failed (ret=%d)", ret);

	ret = k_sem_take(&rx_disabled_sem, K_MSEC(1000));
	zassert_equal(ret, 0, "RX disabled event timeout");

	TC_PRINT("DMA RX buffer request/release test PASSED\n");
}

/**
 * @brief DMA TX abort test.
 *
 * Starts a large DMA transfer and immediately aborts it,
 * verifying that the UART_TX_ABORTED event is delivered.
 */
ZTEST(uart_dma, test_dma_tx_abort)
{
	int ret;

	for (size_t i = 0; i < sizeof(tx_abort_buf); i++) {
		tx_abort_buf[i] = (uint8_t)i;
	}

	ret = uart_tx(dma_uart_dev, tx_abort_buf, sizeof(tx_abort_buf), SYS_FOREVER_US);
	zassert_equal(ret, 0, "uart_tx failed (ret=%d)", ret);

	/* Let the PL330 DMA channel reach a stable running state before
	 * aborting. Issuing DMAKILL on a channel that has not yet started
	 * (especially with peripheral handshake flow control) can leave the
	 * controller inconsistent and hang the subsequent abort handling.
	 */
	k_msleep(1);

	ret = uart_tx_abort(dma_uart_dev);
	zassert_equal(ret, 0, "uart_tx_abort failed (ret=%d)", ret);

	ret = k_sem_take(&tx_done_sem, K_MSEC(5000));
	zassert_equal(ret, 0, "TX abort event timeout");

	zassert_equal(last_tx_evt, UART_TX_ABORTED,
		      "Expected UART_TX_ABORTED, got %d", last_tx_evt);

	TC_PRINT("DMA TX abort test PASSED\n");
}

/**
 * @brief DMA RX timeout test.
 *
 * Sends a payload smaller than the RX buffer over the loopback and
 * verifies the partial data is reported via UART_RX_RDY because of the
 * RX idle timeout. Without the timeout, a buffer that never fills would
 * not produce an RX_RDY event. Per the Zephyr async API, an RX timeout
 * delivers UART_RX_RDY (not UART_RX_DISABLED), and this driver only arms
 * the idle-timeout timer once data arrives (via the character-timeout
 * interrupt), so a small transfer is required to exercise the feature.
 */
ZTEST(uart_dma, test_dma_rx_timeout)
{
	int ret;
	uint8_t tx_payload[] = "RXto";
	size_t tx_len = strlen((char *)tx_payload);
	struct rx_chunk chunk;

	/* Buffer is intentionally larger than the payload so the transfer
	 * completes only via the RX idle timeout, not a full-buffer event.
	 */
	ret = uart_rx_enable(dma_uart_dev, rx_dma_buf_a, RX_BUF_SIZE, 100);
	zassert_equal(ret, 0, "uart_rx_enable failed (ret=%d)", ret);

	memcpy(tx_dma_buf, tx_payload, tx_len);
	ret = dma_uart_send(dma_uart_dev, tx_dma_buf, tx_len);
	zassert_equal(ret, 0, "dma_uart_send failed (ret=%d)", ret);

	/* RX_RDY must arrive via the idle timeout for the partial payload. */
	ret = k_msgq_get(&rx_msgq, &chunk, K_MSEC(5000));
	zassert_equal(ret, 0, "RX timeout did not deliver data");
	zassert_not_null(chunk.buf, "RX buffer is NULL");
	zassert_true(chunk.len > 0 && chunk.len <= tx_len,
		     "Unexpected RX chunk length %zu", chunk.len);
	zassert_equal(memcmp(chunk.buf + chunk.offset, tx_payload, chunk.len),
		      0, "RX data mismatch");

	ret = uart_rx_disable(dma_uart_dev);
	zassert_equal(ret, 0, "uart_rx_disable failed (ret=%d)", ret);

	ret = k_sem_take(&rx_disabled_sem, K_MSEC(1000));
	zassert_equal(ret, 0, "RX disabled event timeout");

	TC_PRINT("DMA RX timeout test PASSED\n");
}

/*
 * DMA async-API suite. The before hook validates device readiness and
 * resets state before every test; the after hook releases the DMA
 * buffers (abort TX, disable RX) so buffers are cleanly re-acquired.
 */
ZTEST_SUITE(uart_dma, NULL, NULL, uart_dma_before, uart_dma_after, NULL);

#endif /* CONFIG_TEST_UART_DMA */

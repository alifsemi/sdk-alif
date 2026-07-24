/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 * MHU Doorbell + Shared SRAM1 heap test: M55-HE (initiator) <-> A32 (responder)
 *
 * Protocol:
 *   - A struct sys_heap and the memory it manages both live in shared
 *     SRAM1. HE operates on the heap; A32 only reads/writes the data block
 *     whose physical address HE passes over the MHU doorbell (A32 cannot
 *     operate on the Zephyr heap, so HE owns both the alloc and the free).
 *   - HE (initiator) dynamically allocates a message block from the shared
 *     heap, fills it with a payload, and rings the MHU doorbell passing the
 *     physical address of the allocated block.
 *   - A32 (responder) maps the shared SRAM via /dev/mem, reads/validates the
 *     block, writes its response into the same block, and rings the doorbell
 *     back with the block address as an acknowledgment.
 *   - HE invalidates its D-cache, validates A32's response, and frees the
 *     block back to the shared heap.
 *
 * IMPORTANT: D-cache coherency
 *   A32 accesses SRAM via a non-cacheable /dev/mem mapping, but M55-HE has
 *   D-cache enabled. After writing the shared block M55-HE cleans (flushes)
 *   its D-cache; before reading A32's response it invalidates its D-cache.
 *
 * Shared memory layout (struct shared_msg):
 *   [0x00] magic      - 0xCAFE0E00 when written by HE, 0xA32F055E by A32
 *   [0x04] msg_id     - incrementing message counter
 *   [0x08] data_len   - number of payload bytes (max 240)
 *   [0x0C] data[240]  - payload
 *   [0xFC] checksum   - sum of payload bytes
 *
 * Test flow (per iteration):
 *   1. HE allocates a block from the shared heap, writes test data, flushes
 *   2. HE rings doorbell to A32 with the allocated block address
 *   3. HE waits for A32's acknowledgment doorbell
 *   4. HE invalidates D-cache, validates A32's response, frees the block
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/sys_heap.h>
#include <zephyr/drivers/ipm.h>
#include <string.h>

#include "mhu_doorbell_msg.h"
#include "mhu_doorbell_heap.h"

#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
#define CACHE_INVALIDATE(addr, size) SCB_InvalidateDCache_by_Addr((void *)(addr), (int32_t)(size))
#define CACHE_CLEAN(addr, size)      SCB_CleanDCache_by_Addr((void *)(addr), (int32_t)(size))
#else
#define CACHE_INVALIDATE(addr, size) __DSB()
#define CACHE_CLEAN(addr, size)      __DSB()
#endif

#define ITERATIONS       25
#define MAX_ERRORS       10
#define RX_TIMEOUT_MS    5000
#define TX_TIMEOUT_MS    1000

/* Shared SRAM1 cross-core heap for HE <-> A32 communication.
 * The struct sys_heap control block and the managed memory both live in
 * shared SRAM1 so HE can allocate a block whose physical address is passed
 * to A32 over the doorbell. A32 only reads/writes the data block (it cannot
 * operate on the Zephyr heap), so HE owns both the alloc and the free.
 *
 * The heap base is page-aligned and the whole span (0x800) fits in one 4 KB
 * page so the A32 side can reach every block through a single /dev/mem mmap.
 * Must match the Linux side (a32_to_m55he_doorbell_responder).
 *
 *   MHU0 heap: ctrl @ 0x027DD000, mem @ 0x027DD100 (size 0x700)
 *   MHU1 heap: ctrl @ 0x027DE000, mem @ 0x027DE100 (size 0x700)
 *
 * Addresses come from mhu_doorbell_heap.h.
 */
#if IS_ENABLED(CONFIG_USE_MHU1)
#define SHARED_HEAP_CTRL  SHARED_HEAP_HEA32_MHU1_CTRL
#define SHARED_HEAP_MEM   SHARED_HEAP_HEA32_MHU1_MEM
#else
#define SHARED_HEAP_CTRL  SHARED_HEAP_HEA32_MHU0_CTRL
#define SHARED_HEAP_MEM   SHARED_HEAP_HEA32_MHU0_MEM
#endif

#if IS_ENABLED(CONFIG_USE_MHU1)
#define MHU_NAME "MHU1"
#define MHU_R_ALIAS apssmhu1r
#define MHU_S_ALIAS apssmhu1s
#else
#define MHU_NAME "MHU0"
#define MHU_R_ALIAS apssmhu0r
#define MHU_S_ALIAS apssmhu0s
#endif

const struct device *mhu_rx_dev;
const struct device *mhu_tx_dev;

static volatile bool doorbell_tx_done;
static volatile bool doorbell_rx_received;
static volatile uint32_t rx_doorbell_addr;

static void mhu_rx_callback(const struct device *dev, void *user_data,
			    uint32_t id, volatile void *data)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(user_data);
	ARG_UNUSED(id);

	/* Keep the ISR minimal: no printk here (blocking/latency in interrupt
	 * context). The main loop logs the doorbell in thread context.
	 */
	rx_doorbell_addr = *((uint32_t *)data);
	doorbell_rx_received = true;
}

static void mhu_tx_callback(const struct device *dev, void *user_data,
			    uint32_t id, volatile void *data)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(user_data);
	ARG_UNUSED(id);
	ARG_UNUSED(data);

	/* Keep the ISR minimal: no printk here. */
	doorbell_tx_done = true;
}

static uint32_t calc_checksum(const uint8_t *buf, uint32_t len)
{
	uint32_t sum = 0;

	for (uint32_t i = 0; i < len; i++) {
		sum += buf[i];
	}
	return sum;
}

int main(void)
{
	int iter;
	struct sys_heap *heap = (struct sys_heap *)SHARED_HEAP_CTRL;

	printk("\n==========================================\n");
	printk("M55-HE -> A32 : %s Doorbell + Shared SRAM1 heap (initiator)\n", MHU_NAME);
	printk("  Shared heap ctrl: 0x%08x\n", SHARED_HEAP_CTRL);
	printk("  Shared heap mem : 0x%08x (size 0x%x)\n",
	       SHARED_HEAP_MEM, SHARED_HEAP_SIZE);
	printk("==========================================\n");

	mhu_rx_dev = DEVICE_DT_GET(DT_ALIAS(MHU_R_ALIAS));
	mhu_tx_dev = DEVICE_DT_GET(DT_ALIAS(MHU_S_ALIAS));

	if (!device_is_ready(mhu_rx_dev) || !device_is_ready(mhu_tx_dev)) {
		printk("ERROR: %s devices not ready!\n", MHU_NAME);
		return -1;
	}
	printk("%s devices ready\n", MHU_NAME);

	ipm_register_callback(mhu_rx_dev, mhu_rx_callback, NULL);
	ipm_register_callback(mhu_tx_dev, mhu_tx_callback, NULL);
	ipm_set_enabled(mhu_rx_dev, true);

	/* Initialise the cross-core heap in shared SRAM. Both the control
	 * block and the managed memory live in shared SRAM so the allocated
	 * block address can be shared with A32. Flush so the initialised heap
	 * is visible to the (non-cacheable) A32 mapping.
	 */
	sys_heap_init(heap, (void *)SHARED_HEAP_MEM, SHARED_HEAP_SIZE);
	CACHE_CLEAN(SHARED_HEAP_CTRL, SHARED_HEAP_SPAN);
	__DSB();

	/* Give A32 (Linux) time to be ready */
	printk("Waiting for A32 Linux responder to be ready...\n");
	printk("Please start the A32 responder application\n");
	k_sleep(K_SECONDS(20));

	int completed = 0;
	int errors = 0;

	for (iter = 0; iter < ITERATIONS; iter++) {
		/* --- Pick up the heap state freed in the previous iteration --- */
		CACHE_INVALIDATE(SHARED_HEAP_CTRL, SHARED_HEAP_SPAN);
		__DSB();

		/* --- Dynamically allocate a message block from the heap --- */
		volatile struct shared_msg *tx_msg =
				sys_heap_alloc(heap, sizeof(struct shared_msg));

		if (tx_msg == NULL) {
			printk("ERROR: sys_heap_alloc failed (iter=%d)\n", iter);
			errors++;
			if (errors >= MAX_ERRORS)
				break;
			continue;
		}

		uint32_t block_addr = (uint32_t)tx_msg;

		/* --- Write test data into the allocated block --- */
		tx_msg->magic    = MAGIC_HE;
		tx_msg->msg_id   = iter;
		tx_msg->data_len = 16;

		/* Fill payload with test pattern */
		for (uint32_t j = 0; j < tx_msg->data_len; j++) {
			tx_msg->data[j] = (uint8_t)(iter + j);
		}
		/* Zero-pad unused data[] bytes */
		memset((void *)&tx_msg->data[tx_msg->data_len], 0,
		       MAX_PAYLOAD - tx_msg->data_len);
		tx_msg->checksum = calc_checksum((const uint8_t *)tx_msg->data,
						 tx_msg->data_len);

		/* --- Flush D-cache so A32 sees the allocation and payload --- */
		CACHE_CLEAN(SHARED_HEAP_CTRL, SHARED_HEAP_SPAN);
		__DSB();

		printk("[%d] HE->A32: alloc block @ 0x%08x msg_id=%u len=%u "
		       "cksum=0x%x\n", iter, block_addr, tx_msg->msg_id,
		       tx_msg->data_len, tx_msg->checksum);

		/* --- Ring doorbell to A32 with the allocated block address --- */
		doorbell_tx_done = false;
		doorbell_rx_received = false;
		uint32_t doorbell_val = block_addr;

		int send_ret = ipm_send(mhu_tx_dev, 0, 0, &doorbell_val, 4);

		if (send_ret != 0) {
			printk("ERROR: ipm_send failed: %d\n", send_ret);
			errors++;
			if (errors >= MAX_ERRORS)
				break;
			continue;
		}

		int waited = 0;

		while (!doorbell_tx_done && waited < TX_TIMEOUT_MS) {
			k_sleep(K_MSEC(1));
			waited++;
		}
		if (!doorbell_tx_done) {
			printk("ERROR: TX timeout (%d ms)\n", TX_TIMEOUT_MS);
			errors++;
			if (errors >= MAX_ERRORS)
				break;
			continue;
		}
		printk("M55-HE: Doorbell TX done\n");

		/* --- Wait for A32's acknowledgment doorbell --- */
		waited = 0;
		while (!doorbell_rx_received && waited < RX_TIMEOUT_MS) {
			k_sleep(K_MSEC(1));
			waited++;
		}
		if (!doorbell_rx_received) {
			printk("ERROR: Timeout waiting for A32 response "
			       "(%d ms)\n", RX_TIMEOUT_MS);
			errors++;
			if (errors >= MAX_ERRORS)
				break;
			continue;
		}
		printk("M55-HE: Doorbell RX addr=0x%08x\n", rx_doorbell_addr);

		/* A32 writes its response into the same block and rings back
		 * with the block address as an acknowledgment.
		 */
		if (rx_doorbell_addr != block_addr) {
			printk("[%d] A32->HE: FAIL ack addr 0x%08x != "
			       "block 0x%08x\n", iter, rx_doorbell_addr,
			       block_addr);
			sys_heap_free(heap, (void *)block_addr);
			CACHE_CLEAN(SHARED_HEAP_CTRL, SHARED_HEAP_SPAN);
			__DSB();
			errors++;
			if (errors >= MAX_ERRORS)
				break;
			continue;
		}

		/* --- Invalidate D-cache before reading A32's response --- */
		CACHE_INVALIDATE(SHARED_HEAP_CTRL, SHARED_HEAP_SPAN);
		__DSB();

		volatile struct shared_msg *incoming =
				(volatile struct shared_msg *)block_addr;
		bool ok = true;

		if (incoming->magic != MAGIC_A32) {
			printk("ERROR: Bad magic 0x%08x (expected 0x%08x)\n",
			       incoming->magic, MAGIC_A32);
			ok = false;
		} else if (incoming->msg_id != (uint32_t)iter) {
			printk("ERROR: msg_id mismatch: got %u, expected %d\n",
			       incoming->msg_id, iter);
			ok = false;
		} else if (incoming->data_len > MAX_PAYLOAD) {
			printk("ERROR: data_len %u exceeds MAX_PAYLOAD\n",
			       incoming->data_len);
			ok = false;
		} else {
			uint32_t rx_len = incoming->data_len;
			uint32_t computed_cksum = calc_checksum(
					(const uint8_t *)incoming->data, rx_len);

			printk("[%d] A32->HE: block @ 0x%08x msg_id=%u len=%u "
			       "cksum=0x%x/0x%x %s\n\n", iter, block_addr,
			       incoming->msg_id, rx_len, computed_cksum,
			       incoming->checksum,
			       (computed_cksum == incoming->checksum) ?
			       "PASS" : "FAIL");
			ok = (computed_cksum == incoming->checksum);
		}

		/* --- Free the block back to the shared heap --- */
		sys_heap_free(heap, (void *)block_addr);
		CACHE_CLEAN(SHARED_HEAP_CTRL, SHARED_HEAP_SPAN);
		__DSB();

		if (ok) {
			completed++;
		} else {
			errors++;
		}

		if (errors >= MAX_ERRORS) {
			printk("ERROR: Too many errors (%d), aborting\n",
			       errors);
			break;
		}

		/* Small delay between iterations */
		k_sleep(K_MSEC(500));
	}

	ipm_set_enabled(mhu_rx_dev, false);
	printk("Done: %d/%d passed, %d failed\n",
	       completed, ITERATIONS, errors);
	return (completed + errors == ITERATIONS) ? 0 : -1;
}

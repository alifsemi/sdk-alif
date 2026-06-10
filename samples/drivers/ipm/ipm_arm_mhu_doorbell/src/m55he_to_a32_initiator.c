/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 * MHU Doorbell + Shared SRAM1 test: M55-HE (initiator) <-> A32 (responder)
 *
 * Protocol:
 *   - SRAM1 region is used as shared memory.
 *   - MHU channel 0 is used as a doorbell (the 32-bit value sent via
 *     MHU carries the physical address of the data in shared SRAM1).
 *   - HE writes its payload into shared SRAM, then rings the MHU doorbell
 *     with the physical address so A32 knows where to read.
 *   - A32 reads, processes, writes a response, and rings back.
 *
 * IMPORTANT: D-cache coherency
 *   A32 accesses SRAM via non-cacheable /dev/mem mapping, but M55-HE
 *   has D-cache enabled. Before reading shared SRAM written by A32,
 *   M55-HE must invalidate its D-cache. After writing shared SRAM,
 *   M55-HE must clean (flush) its D-cache.
 *
 * Shared memory layout (struct shared_msg):
 *   [0x00] magic      - 0xCAFE0E00 when written by HE, 0xA32F055E by A32
 *   [0x04] msg_id     - incrementing message counter
 *   [0x08] data_len   - number of payload bytes (max 240)
 *   [0x0C] data[240]  - payload
 *   [0xFC] checksum   - sum of payload bytes
 *
 * Test flow (per iteration):
 *   1. HE writes test data into shared SRAM, flushes D-cache
 *   2. HE rings doorbell to A32 with the data address
 *   3. HE waits for A32's response doorbell
 *   4. HE invalidates D-cache, reads and validates A32's response
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/ipm.h>
#include <string.h>

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

/* Shared SRAM1 addresses for HE <-> A32 communication
 * Must match the Linux side (a32_to_m55he_responder)
 */
#if IS_ENABLED(CONFIG_USE_MHU1)
#define SHARED_MEM_BASE  0x027DDE00
#else
#define SHARED_MEM_BASE  0x027DDC00
#endif
#define HE_TO_A32_ADDR   (SHARED_MEM_BASE + 0x0000)  /* HE writes here  */
#define A32_TO_HE_ADDR   (SHARED_MEM_BASE + 0x0100)  /* A32 writes here */

#define MAGIC_HE         0xCAFE0E00   /* Written by HE  */
#define MAGIC_A32        0xA32F055E   /* Written by A32 */
#define MAX_PAYLOAD      240

struct shared_msg {
	uint32_t magic;
	uint32_t msg_id;
	uint32_t data_len;
	uint8_t  data[MAX_PAYLOAD];
	uint32_t checksum;
};

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

	rx_doorbell_addr = *((uint32_t *)data);
	printk("M55-HE: Doorbell RX Ch%d, addr=0x%08x\n", id, rx_doorbell_addr);
	doorbell_rx_received = true;
}

static void mhu_tx_callback(const struct device *dev, void *user_data,
			    uint32_t id, volatile void *data)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(user_data);
	ARG_UNUSED(data);

	printk("M55-HE: Doorbell TX Ch%d done\n", id);
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
	volatile struct shared_msg *tx_msg =
			(volatile struct shared_msg *)HE_TO_A32_ADDR;

	printk("\n==========================================\n");
	printk("M55-HE -> A32 : %s Doorbell + Shared SRAM1 (initiator)\n", MHU_NAME);
	printk("  Shared TX (HE->A32): 0x%08x\n", HE_TO_A32_ADDR);
	printk("  Shared RX (A32->HE): 0x%08x\n", A32_TO_HE_ADDR);
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

	/* Clear our TX region */
	memset((void *)HE_TO_A32_ADDR, 0, sizeof(struct shared_msg));
	CACHE_CLEAN(HE_TO_A32_ADDR, sizeof(struct shared_msg));

	/* Give A32 (Linux) time to be ready */
	printk("Waiting for A32 Linux responder to be ready...\n");
	printk("Please start the A32 responder application\n");
	k_sleep(K_SECONDS(20));

	int completed = 0;
	int errors = 0;

	for (iter = 0; iter < ITERATIONS; iter++) {
		/* --- Invalidate D-cache for TX region before writing --- */
		CACHE_INVALIDATE(HE_TO_A32_ADDR, sizeof(struct shared_msg));
		__DSB();

		/* --- Write test data into shared SRAM --- */
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

		/* --- Flush D-cache so A32 sees our writes --- */
		CACHE_CLEAN(HE_TO_A32_ADDR, sizeof(struct shared_msg));
		__DSB();

		printk("[%d] HE->A32: Sending msg_id=%u len=%u cksum=0x%x\n",
		       iter, tx_msg->msg_id, tx_msg->data_len, tx_msg->checksum);

		/* --- Ring doorbell to A32 --- */
		doorbell_tx_done = false;
		doorbell_rx_received = false;
		uint32_t doorbell_val = HE_TO_A32_ADDR;

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

		/* --- Wait for A32's response doorbell --- */
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

		/* --- Invalidate D-cache before reading A32's response --- */
		CACHE_INVALIDATE(rx_doorbell_addr, sizeof(struct shared_msg));
		__DSB();

		volatile struct shared_msg *incoming =
				(volatile struct shared_msg *)rx_doorbell_addr;

		if (incoming->magic != MAGIC_A32) {
			printk("ERROR: Bad magic 0x%08x (expected 0x%08x)\n",
			       incoming->magic, MAGIC_A32);
			errors++;
			continue;
		}

		if (incoming->msg_id != (uint32_t)iter) {
			printk("ERROR: msg_id mismatch: got %u, expected %d\n",
			       incoming->msg_id, iter);
			errors++;
			continue;
		}

		uint32_t rx_len = incoming->data_len;

		if (rx_len > MAX_PAYLOAD) {
			printk("ERROR: data_len %u exceeds MAX_PAYLOAD\n",
			       rx_len);
			errors++;
			continue;
		}

		uint32_t computed_cksum = calc_checksum(
				(const uint8_t *)incoming->data, rx_len);
		printk("[%d] A32->HE: msg_id=%u len=%u cksum=0x%x/0x%x %s\n\n",
		       iter, incoming->msg_id, rx_len,
		       computed_cksum, incoming->checksum,
		       (computed_cksum == incoming->checksum) ? "PASS" : "FAIL");

		if (computed_cksum == incoming->checksum) {
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

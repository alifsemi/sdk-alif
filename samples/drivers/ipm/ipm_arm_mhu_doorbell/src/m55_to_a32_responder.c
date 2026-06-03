/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 * MHU Doorbell + Shared SRAM1 test: A32 (initiator) <-> M55-HE/M55-HP
 * (responder)
 *
 * Counterpart to the A32 Linux corner-case initiators:
 *   pthread_m55_he_mhu0_shared_mem  (HE, MHU0)
 *   pthread_m55_he_mhu1_shared_mem  (HE, MHU1)
 *   pthread_m55_hp_mhu0_shared_mem  (HP, MHU0)
 *   pthread_m55_hp_mhu1_shared_mem  (HP, MHU1)
 *
 * Protocol:
 *   - SRAM1 is used as shared memory, split into two regions:
 *       [base + 0x0000] A32 -> M55 messages (A32 writes, M55 reads)
 *       [base + 0x0100] M55 -> A32 responses (M55 writes, A32 reads)
 *   - The MHU channel is used as a doorbell; the 32-bit value it carries
 *     is the physical address of the data region.
 *   - A32 writes a payload, rings the doorbell with the A32->M55 region
 *     address. M55 reads/validates it, echoes each byte XOR'd with
 *     (msg_id + 0x42) into the M55->A32 region, and rings the doorbell
 *     back with the M55->A32 region address.
 *
 * D-cache coherency:
 *   A32 accesses SRAM via a non-cacheable /dev/mem mapping, but M55 has
 *   D-cache enabled. M55 invalidates its D-cache before reading the data
 *   A32 wrote, and cleans (flushes) its D-cache after writing a response.
 *
 * Shared memory layout (struct shared_msg):
 *   [0x00] magic      - 0xA32F055E from A32, 0xF055EA32 from M55
 *   [0x04] msg_id     - incrementing message counter
 *   [0x08] data_len   - number of payload bytes (max 240)
 *   [0x0C] data[240]  - payload
 *   [0xFC] checksum   - sum of payload bytes
 *
 * Test flow (per iteration):
 *   1. M55 waits for doorbell from A32 (MHU RX interrupt)
 *   2. M55 invalidates D-cache, reads and validates A32's message
 *   3. M55 writes XOR'd response, cleans D-cache
 *   4. M55 rings doorbell back to A32 with the response region address
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
#define RX_TIMEOUT_MS    5000  /* Timeout waiting for A32 doorbell */
#define TX_TIMEOUT_MS    1000  /* Timeout waiting for TX completion */

/* Shared SRAM1 base, selected per core / MHU instance.
 * Must match the Linux side (pthread_m55_*_shared_mem).
 */
#if IS_ENABLED(CONFIG_RTSS_HE)
  #if IS_ENABLED(CONFIG_USE_MHU1)
    #define SHARED_MEM_BASE  0x027DDE00   /* HE MHU1 */
  #else
    #define SHARED_MEM_BASE  0x027DDC00   /* HE MHU0 */
  #endif
#else
  #if IS_ENABLED(CONFIG_USE_MHU1)
    #define SHARED_MEM_BASE  0x027DDA00   /* HP MHU1 */
  #else
    #define SHARED_MEM_BASE  0x027DD800   /* HP MHU0 */
  #endif
#endif

#define A32_TO_M55_ADDR  (SHARED_MEM_BASE + 0x0000)  /* A32 writes here */
#define M55_TO_A32_ADDR  (SHARED_MEM_BASE + 0x0100)  /* M55 writes here */

#define MAGIC_A32        0xA32F055E   /* Written by A32 */
#define MAGIC_M55        0xF055EA32   /* Written by M55 */
#define MAX_PAYLOAD      240

struct shared_msg {
	uint32_t magic;
	uint32_t msg_id;
	uint32_t data_len;
	uint8_t  data[MAX_PAYLOAD];
	uint32_t checksum;
};

const struct device *mhu_rx_dev;  /* MHU receiver from A32 */
const struct device *mhu_tx_dev;  /* MHU sender to A32     */

static volatile bool doorbell_tx_done;
static volatile bool doorbell_rx_received;
static volatile uint32_t rx_doorbell_addr;

/* Callback: doorbell received from A32 */
static void mhu_rx_callback(const struct device *dev, void *user_data,
			    uint32_t id, volatile void *data)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(user_data);

	rx_doorbell_addr = *((uint32_t *)data);
	printk("M55: Doorbell RX Ch%d, addr=0x%08x\n", id, rx_doorbell_addr);
	doorbell_rx_received = true;
}

/* Callback: doorbell sent to A32 */
static void mhu_tx_callback(const struct device *dev, void *user_data,
			    uint32_t id, volatile void *data)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(user_data);
	ARG_UNUSED(data);

	printk("M55: Doorbell TX Ch%d done\n", id);
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
	volatile struct shared_msg *rx_msg =
			(volatile struct shared_msg *)A32_TO_M55_ADDR;
	volatile struct shared_msg *tx_msg =
			(volatile struct shared_msg *)M55_TO_A32_ADDR;

	printk("\n==========================================\n");
#if IS_ENABLED(CONFIG_RTSS_HE)
  #if IS_ENABLED(CONFIG_USE_MHU1)
	printk("M55-HE <-> A32 : MHU1 Doorbell + Shared SRAM1 (responder)\n");
  #else
	printk("M55-HE <-> A32 : MHU0 Doorbell + Shared SRAM1 (responder)\n");
  #endif
#else
  #if IS_ENABLED(CONFIG_USE_MHU1)
	printk("M55-HP <-> A32 : MHU1 Doorbell + Shared SRAM1 (responder)\n");
  #else
	printk("M55-HP <-> A32 : MHU0 Doorbell + Shared SRAM1 (responder)\n");
  #endif
#endif
	printk("  Shared RX (A32->M55): 0x%08x\n", A32_TO_M55_ADDR);
	printk("  Shared TX (M55->A32): 0x%08x\n", M55_TO_A32_ADDR);
	printk("==========================================\n");

	/* Get device handles from DTS aliases */
#if IS_ENABLED(CONFIG_USE_MHU1)
	mhu_rx_dev = DEVICE_DT_GET(DT_ALIAS(apssmhu1r));
	mhu_tx_dev = DEVICE_DT_GET(DT_ALIAS(apssmhu1s));
#else
	mhu_rx_dev = DEVICE_DT_GET(DT_ALIAS(apssmhu0r));
	mhu_tx_dev = DEVICE_DT_GET(DT_ALIAS(apssmhu0s));
#endif

	if (!device_is_ready(mhu_rx_dev) || !device_is_ready(mhu_tx_dev)) {
		printk("ERROR: MHU devices not ready!\n");
		return -1;
	}
	printk("MHU devices ready\n");

	/* Register callbacks */
	ipm_register_callback(mhu_rx_dev, mhu_rx_callback, NULL);
	ipm_register_callback(mhu_tx_dev, mhu_tx_callback, NULL);

	/* Enable receiver interrupt */
	ipm_set_enabled(mhu_rx_dev, true);

	/* Clear our TX region */
	memset((void *)M55_TO_A32_ADDR, 0, sizeof(struct shared_msg));
	CACHE_CLEAN(M55_TO_A32_ADDR, sizeof(struct shared_msg));

	printk("Waiting for A32 doorbell...\n\n");

	int timeouts = 0;
	int completed = 0;
	int errors = 0;

	for (iter = 0; iter < ITERATIONS; iter++) {
		/* --- Wait for doorbell from A32 --- */
		doorbell_rx_received = false;

		if (iter == 0) {
			/* First iteration: wait indefinitely for Linux to boot */
			while (!doorbell_rx_received) {
				k_sleep(K_MSEC(1));
			}
		} else {
			/* Subsequent iterations: apply timeout */
			int rx_waited = 0;

			while (!doorbell_rx_received &&
			       rx_waited < RX_TIMEOUT_MS) {
				k_sleep(K_MSEC(1));
				rx_waited++;
			}
		}

		if (!doorbell_rx_received) {
			printk("WARN: Timeout waiting for A32 doorbell "
			       "(%d ms), retry %d\n",
			       RX_TIMEOUT_MS, timeouts + 1);
			if (++timeouts > 5) {
				printk("ERROR: Too many timeouts, aborting\n");
				break;
			}
			iter--; /* Don't consume this iteration */
			continue;
		}
		timeouts = 0;

		/* --- Invalidate D-cache before reading A32's message --- */
		CACHE_INVALIDATE(A32_TO_M55_ADDR, sizeof(struct shared_msg));
		__DSB();

		if (rx_msg->magic != MAGIC_A32) {
			printk("[%d] ERROR: Bad magic 0x%08x (expected 0x%08x)\n",
			       iter, rx_msg->magic, MAGIC_A32);
			errors++;
			if (errors >= MAX_ERRORS)
				break;
			continue;
		}

		uint32_t msg_id = rx_msg->msg_id;
		uint32_t rx_len = rx_msg->data_len;

		if (rx_len > MAX_PAYLOAD) {
			printk("[%d] ERROR: data_len %u exceeds MAX_PAYLOAD\n",
			       iter, rx_len);
			errors++;
			if (errors >= MAX_ERRORS)
				break;
			continue;
		}

		uint32_t rx_cksum = calc_checksum((const uint8_t *)rx_msg->data,
						  rx_len);
		printk("[%d] A32->M55: msg_id=%u len=%u cksum=0x%x/0x%x %s\n",
		       iter, msg_id, rx_len, rx_cksum, rx_msg->checksum,
		       (rx_cksum == rx_msg->checksum) ? "PASS" : "FAIL");

		if (rx_cksum != rx_msg->checksum) {
			errors++;
			if (errors >= MAX_ERRORS)
				break;
			continue;
		}

		/* --- Build response: echo each byte XOR'd with key --- */
		uint8_t xor_key = (uint8_t)(msg_id + 0x42);

		tx_msg->magic    = MAGIC_M55;
		tx_msg->msg_id   = msg_id;
		tx_msg->data_len = rx_len;
		for (uint32_t j = 0; j < rx_len; j++) {
			tx_msg->data[j] = rx_msg->data[j] ^ xor_key;
		}
		memset((void *)&tx_msg->data[rx_len], 0, MAX_PAYLOAD - rx_len);
		tx_msg->checksum = calc_checksum((const uint8_t *)tx_msg->data,
						 rx_len);

		/* --- Flush D-cache so A32 sees our writes --- */
		CACHE_CLEAN(M55_TO_A32_ADDR, sizeof(struct shared_msg));
		__DSB();

		/* Small delay to let A32 prepare for RX */
		k_sleep(K_MSEC(10));

		/* --- Ring doorbell back to A32 with response address --- */
		doorbell_tx_done = false;
		uint32_t doorbell_val = M55_TO_A32_ADDR;

		int send_ret = ipm_send(mhu_tx_dev, 0, 0, &doorbell_val, 4);

		if (send_ret != 0) {
			printk("ERROR: ipm_send failed: %d\n", send_ret);
			errors++;
			if (errors >= MAX_ERRORS)
				break;
			continue;
		}

		int tx_waited = 0;

		while (!doorbell_tx_done && tx_waited < TX_TIMEOUT_MS) {
			k_sleep(K_MSEC(1));
			tx_waited++;
		}
		if (!doorbell_tx_done) {
			printk("ERROR: Timeout waiting for TX completion "
			       "(%d ms)\n", TX_TIMEOUT_MS);
			break;
		}

		printk("[%d] M55->A32: msg_id=%u len=%u cksum=0x%x Complete\n\n",
		       iter, msg_id, rx_len, tx_msg->checksum);
		completed++;
	}

	ipm_set_enabled(mhu_rx_dev, false);
	printk("Done: %d/%d passed, %d failed\n",
	       completed, ITERATIONS, errors);
	return (completed + errors == ITERATIONS) ? 0 : -1;
}

/*
 * SPDX-FileCopyrightText: Copyright Alif Semiconductor
 * SPDX-License-Identifier: Apache-2.0
 *
 * MHU Doorbell + Shared SRAM1 heap test: A32 (responder) <-> M55-HE/HP (initiator)
 *
 * Zephyr-on-A32 counterpart to src/m55_to_a32_initiator.c, using the Zephyr
 * IPM/MHU driver. The A32 has a separate MHU to each M55 core, so the responder
 * listens on both the HE and HP channels and replies on whichever rang; the
 * initiator can therefore run on either M55 core.
 *
 * Protocol:
 *   - A struct sys_heap and the memory it manages both live in shared SRAM1.
 *     The M55 initiator owns the heap (it does the alloc and the free); A32
 *     only reads/writes the data block whose physical address the M55 passes
 *     over the MHU doorbell.
 *   - The M55 initiator allocates a block, writes a payload (magic MAGIC_HE),
 *     and rings the doorbell with the block address.
 *   - A32 (responder) reads/validates the block, writes its response into the
 *     same block (magic MAGIC_A32, payload echoed back), and rings the
 *     doorbell back with the block address as an acknowledgment.
 *
 * IMPORTANT: D-cache coherency
 *   The A32 runs under an MMU, so the shared SRAM1 heap page is mapped
 *   non-cacheable (K_MEM_CACHE_NONE), so A32 accesses go straight to memory and
 *   need no cache maintenance; the M55 initiator (D-cache enabled) cleans after
 *   writing and invalidates before reading. The doorbell carries a physical
 *   address, which A32 translates into its non-cacheable mapping.
 *
 * Shared memory layout (struct shared_msg):
 *   [0x00] magic      - 0xCAFE0E00 from the M55 initiator, 0xA32F055E by A32
 *   [0x04] msg_id     - incrementing message counter
 *   [0x08] data_len   - number of payload bytes (max 240)
 *   [0x0C] data[240]  - payload
 *   [0xFC] checksum   - sum of payload bytes
 *
 * Test flow (per iteration):
 *   1. A32 waits for a doorbell from either M55 core (MHU RX interrupt)
 *   2. A32 reads and validates the block via the non-cacheable mapping
 *   3. A32 writes its response into the same block
 *   4. A32 rings the doorbell back to that core with the block address
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/ipm.h>
#include <zephyr/kernel/mm.h>
#include <string.h>

#include "mhu_doorbell_msg.h"
#include "mhu_doorbell_heap.h"

#define ITERATIONS       25
#define MAX_ERRORS       10
#define RX_TIMEOUT_MS    5000  /* Timeout waiting for a doorbell     */
#define TX_TIMEOUT_MS    1000  /* Timeout waiting for TX completion  */

/* Shared SRAM1 cross-core heap for M55 <-> A32 communication. A32 does not
 * operate the Zephyr heap; it only reads/writes the block the M55 passes over
 * the doorbell, and range-checks that address against this heap's span.
 * Addresses come from mhu_doorbell_heap.h and must match the M55 initiator.
 * The HE and HP initiators use the same HEA32 heap region (one peer at a time).
 */
#if IS_ENABLED(CONFIG_USE_MHU1)
#define SHARED_HEAP_CTRL  SHARED_HEAP_HEA32_MHU1_CTRL
#else
#define SHARED_HEAP_CTRL  SHARED_HEAP_HEA32_MHU0_CTRL
#endif

/* MHU aliases as seen from the A32 (APSS) side. The A32 has a separate MHU to
 * each M55 core, so the responder listens on both the HE and the HP channel
 * (for the selected MHU instance) and replies on whichever one rang -- the
 * initiator can therefore run on either M55 core with no rebuild here. The A32
 * board overlay provides both alias sets.
 */
#if IS_ENABLED(CONFIG_USE_MHU1)
#define MHU_NAME    "MHU1"
#define HE_R_ALIAS  apssmhu1r
#define HE_S_ALIAS  apssmhu1s
#define HP_R_ALIAS  apsshpmhu1r
#define HP_S_ALIAS  apsshpmhu1s
#else
#define MHU_NAME    "MHU0"
#define HE_R_ALIAS  apssmhu0r
#define HE_S_ALIAS  apssmhu0s
#define HP_R_ALIAS  apsshpmhu0r
#define HP_S_ALIAS  apsshpmhu0s
#endif

const struct device *mhu_he_rx_dev;  /* receive from M55-HE */
const struct device *mhu_he_tx_dev;  /* send to M55-HE      */
const struct device *mhu_hp_rx_dev;  /* receive from M55-HP */
const struct device *mhu_hp_tx_dev;  /* send to M55-HP      */

static volatile bool doorbell_tx_done;
static volatile uint32_t rx_doorbell_addr;
static const struct device *volatile active_tx_dev;
static const char *volatile active_peer;

/* Signalled from the MHU RX interrupt so the main loop can block until a
 * doorbell actually arrives instead of polling.
 */
static K_SEM_DEFINE(doorbell_rx_sem, 0, 1);

static void mhu_rx_callback(const struct device *dev, void *user_data,
			    uint32_t id, volatile void *data)
{
	ARG_UNUSED(user_data);
	ARG_UNUSED(id);

	/* Keep the ISR minimal: no printk here (blocking/latency in interrupt
	 * context). Record which peer rang so the reply goes back on the same
	 * channel; the main loop logs in thread context.
	 */
	rx_doorbell_addr = *((uint32_t *)data);
	if (dev == mhu_he_rx_dev) {
		active_tx_dev = mhu_he_tx_dev;
		active_peer = "M55-HE";
	} else {
		active_tx_dev = mhu_hp_tx_dev;
		active_peer = "M55-HP";
	}
	k_sem_give(&doorbell_rx_sem);
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
	uint8_t *heap_virt;

	printk("\n==========================================\n");
	printk("A32 <-> M55-HE/HP : %s Doorbell + Shared SRAM1 heap (responder)\n",
	       MHU_NAME);
	printk("  Shared heap ctrl: 0x%08x (span 0x%x)\n",
	       SHARED_HEAP_CTRL, SHARED_HEAP_SPAN);
	printk("==========================================\n");

	/* Map the shared SRAM1 heap page non-cacheable so A32 sees the data the
	 * M55 initiator flushes (and vice versa) without any cache maintenance.
	 * The doorbell carries a physical block address; translate it into this
	 * mapping via (heap_virt + (phys - SHARED_HEAP_CTRL)).
	 */
	k_mem_map_phys_bare(&heap_virt, SHARED_HEAP_CTRL, SHARED_HEAP_SPAN,
			    K_MEM_CACHE_NONE | K_MEM_PERM_RW);

	mhu_he_rx_dev = DEVICE_DT_GET(DT_ALIAS(HE_R_ALIAS));
	mhu_he_tx_dev = DEVICE_DT_GET(DT_ALIAS(HE_S_ALIAS));
	mhu_hp_rx_dev = DEVICE_DT_GET(DT_ALIAS(HP_R_ALIAS));
	mhu_hp_tx_dev = DEVICE_DT_GET(DT_ALIAS(HP_S_ALIAS));

	if (!device_is_ready(mhu_he_rx_dev) || !device_is_ready(mhu_he_tx_dev) ||
	    !device_is_ready(mhu_hp_rx_dev) || !device_is_ready(mhu_hp_tx_dev)) {
		printk("ERROR: %s devices not ready!\n", MHU_NAME);
		return -1;
	}
	printk("%s devices ready\n", MHU_NAME);

	ipm_register_callback(mhu_he_rx_dev, mhu_rx_callback, NULL);
	ipm_register_callback(mhu_he_tx_dev, mhu_tx_callback, NULL);
	ipm_register_callback(mhu_hp_rx_dev, mhu_rx_callback, NULL);
	ipm_register_callback(mhu_hp_tx_dev, mhu_tx_callback, NULL);
	ipm_set_enabled(mhu_he_rx_dev, true);
	ipm_set_enabled(mhu_hp_rx_dev, true);

	printk("Waiting for M55-HE/HP doorbells...\n\n");

	int completed = 0;
	int errors = 0;

	for (iter = 0; iter < ITERATIONS; iter++) {
		/* --- Wait for a doorbell from either M55 core --- */
		if (iter == 0) {
			/* First iteration: wait indefinitely for the M55 to boot */
			k_sem_take(&doorbell_rx_sem, K_FOREVER);
		} else if (k_sem_take(&doorbell_rx_sem,
				      K_MSEC(RX_TIMEOUT_MS)) != 0) {
			printk("ERROR: Timeout waiting for doorbell "
			       "(%d ms)\n", RX_TIMEOUT_MS);
			break;
		}

		unsigned int key = irq_lock();
		uint32_t rx_addr = rx_doorbell_addr;
		const struct device *tx_dev = active_tx_dev;
		const char *peer = active_peer;

		irq_unlock(key);

		printk("A32: Doorbell RX from %s addr=0x%08x\n", peer, rx_addr);

		/* --- Range-check the untrusted doorbell address --- */
		if (rx_addr < SHARED_HEAP_CTRL ||
		    rx_addr + sizeof(struct shared_msg) >
			    SHARED_HEAP_CTRL + SHARED_HEAP_SPAN) {
			printk("[%d] ERROR: doorbell addr 0x%08x outside heap "
			       "[0x%08x, 0x%08x)\n", iter, rx_addr,
			       SHARED_HEAP_CTRL,
			       SHARED_HEAP_CTRL + SHARED_HEAP_SPAN);
			errors++;
			if (errors >= MAX_ERRORS)
				break;
			continue;
		}

		/* Translate the physical block address into the non-cacheable
		 * mapping (no cache maintenance needed on the A32 side).
		 */
		volatile struct shared_msg *msg = (volatile struct shared_msg *)
				(heap_virt + (rx_addr - SHARED_HEAP_CTRL));

		if (msg->magic != MAGIC_HE) {
			printk("[%d] ERROR: Bad magic 0x%08x (expected 0x%08x)\n",
			       iter, msg->magic, MAGIC_HE);
			errors++;
			if (errors >= MAX_ERRORS)
				break;
			continue;
		}

		uint32_t rx_len = msg->data_len;

		if (rx_len > MAX_PAYLOAD) {
			printk("[%d] ERROR: data_len %u exceeds MAX_PAYLOAD\n",
			       iter, rx_len);
			errors++;
			if (errors >= MAX_ERRORS)
				break;
			continue;
		}

		uint32_t computed = calc_checksum((const uint8_t *)msg->data,
						  rx_len);
		printk("[%d] %s->A32: block @ 0x%08x msg_id=%u len=%u "
		       "cksum=0x%x/0x%x %s\n", iter, peer, rx_addr, msg->msg_id,
		       rx_len, computed, msg->checksum,
		       (computed == msg->checksum) ? "PASS" : "FAIL");

		if (computed != msg->checksum) {
			errors++;
			if (errors >= MAX_ERRORS)
				break;
			continue;
		}

		/* --- Write the response into the same block (echo payload) --- */
		msg->magic = MAGIC_A32;
		/* msg_id, data[] and data_len are the M55 payload echoed back. */
		msg->checksum = calc_checksum((const uint8_t *)msg->data, rx_len);

		/* --- Ring doorbell back to the initiator with the block address --- */
		doorbell_tx_done = false;
		uint32_t doorbell_val = rx_addr;

		int send_ret = ipm_send(tx_dev, 0, 0, &doorbell_val, 4);

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

		printk("[%d] A32->%s: block @ 0x%08x msg_id=%u len=%u "
		       "cksum=0x%x Complete\n\n", iter, peer, rx_addr, msg->msg_id,
		       rx_len, msg->checksum);
		completed++;
	}

	ipm_set_enabled(mhu_he_rx_dev, false);
	ipm_set_enabled(mhu_hp_rx_dev, false);
	printk("Done: %d/%d passed, %d failed\n",
	       completed, ITERATIONS, errors);
	return (completed + errors == ITERATIONS) ? 0 : -1;
}

/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 * MHU Doorbell + Shared SRAM1 heap test: M55-HP (responder) <-> M55-HE (initiator)
 *
 * Protocol:
 *   - A struct sys_heap and the memory it manages both live in shared
 *     SRAM1, so both cores operate on the same heap instance.
 *   - HE (initiator) dynamically allocates a message block from the
 *     shared heap and rings the MHU doorbell passing the block address.
 *   - HP (responder) reads/validates the block, frees it back to the
 *     shared heap, and rings back the freed address as an acknowledgment.
 *
 * IMPORTANT: D-cache coherency
 *   Both M55-HE and M55-HP have D-cache enabled. Before reading shared
 *   SRAM written by the other side, the reader must invalidate its D-cache.
 *   After writing shared SRAM, the writer must clean (flush) its D-cache.
 *   The strict doorbell ping-pong guarantees only one core touches the
 *   shared heap at a time, so full-region maintenance is sufficient.
 *
 * Shared memory layout (struct shared_msg):
 *   [0x00] magic      - 0xCAFE0E00 when written by HE, 0xCAFE0F00 by HP
 *   [0x04] msg_id     - incrementing message counter
 *   [0x08] data_len   - number of payload bytes (max 240)
 *   [0x0C] data[240]  - payload
 *   [0xFC] checksum   - sum of payload bytes
 *
 * Test flow (per iteration):
 *   1. HP waits for doorbell from HE (MHU RX interrupt)
 *   2. HP invalidates D-cache, reads and validates HE's block
 *   3. HP frees the block back to the shared heap, flushes D-cache
 *   4. HP rings doorbell to HE (MHU TX) with the freed block address
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/sys_heap.h>
#include <zephyr/drivers/ipm.h>
#include <string.h>
#include <zephyr/pm/pm.h>
#include <zephyr/pm/policy.h>

#include "mhu_doorbell_msg.h"
#include "mhu_doorbell_heap.h"

#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
#define CACHE_INVALIDATE(addr, size) SCB_InvalidateDCache_by_Addr((void *)(addr), (int32_t)(size))
#define CACHE_CLEAN(addr, size)      SCB_CleanDCache_by_Addr((void *)(addr), (int32_t)(size))
#else
#define CACHE_INVALIDATE(addr, size) __DSB()
#define CACHE_CLEAN(addr, size)      __DSB()
#endif

#define TX_TIMEOUT_MS    1000

/* Without CONFIG_PM (no pm-system-off-hp snippet) the core never resets, so
 * handle a fixed number of doorbells and then stop with a summary.
 */
#define NUM_EXCHANGES    10

/* Shared SRAM1 cross-core heaps for HE <-> HP inter-core communication.
 * Each MHU channel has its own heap whose struct sys_heap control block
 * and managed memory both live in shared SRAM. HE allocates blocks; HP
 * frees them on the same heap instance. Addresses come from
 * mhu_doorbell_heap.h.
 */
#define SHARED_HEAP_MHU0_CTRL  SHARED_HEAP_HEHP_MHU0_CTRL
#define SHARED_HEAP_MHU1_CTRL  SHARED_HEAP_HEHP_MHU1_CTRL

const struct device *mhu0_rx_dev;
const struct device *mhu0_tx_dev;
const struct device *mhu1_rx_dev;
const struct device *mhu1_tx_dev;

static volatile bool doorbell_tx_done;
static volatile uint32_t rx_doorbell_addr;
static const struct device *volatile active_tx_dev;

/* Signalled from the MHU RX interrupt callback so the main loop can sleep
 * (in low power, with CONFIG_PM) until a doorbell actually arrives instead
 * of polling.
 */
static K_SEM_DEFINE(doorbell_rx_sem, 0, 1);

static void mhu_rx_callback(const struct device *dev, void *user_data,
			    uint32_t id, volatile void *data)
{
	ARG_UNUSED(user_data);
	ARG_UNUSED(id);

	/* Keep the ISR minimal: no printk here (blocking/latency in interrupt
	 * context). handle_doorbell() logs the doorbell in thread context.
	 */
	rx_doorbell_addr = *((uint32_t *)data);
	active_tx_dev = (dev == mhu0_rx_dev) ? mhu0_tx_dev : mhu1_tx_dev;
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

/*
 * Power management is enabled only when this sample is built with the
 * pm-system-off-hp snippet. The snippet configures the SE run/off power
 * profiles in devicetree (applied automatically by se_service) and the deep
 * off-state nodes.
 *
 * HP-goes-SOFT_OFF model: HP makes PM_STATE_SOFT_OFF the only available deep
 * state and blocks waiting for a doorbell, so the idle thread powers the
 * subsystem off. SOFT_OFF uses the M55 EWIC, which automatically monitors the
 * core's enabled NVIC interrupt lines 0-63. The MHU RX IRQs (MHU0 = 41,
 * MHU1 = 43) are within that range and are enabled by ipm_set_enabled(), so
 * when HE rings the doorbell the EWIC wakes the subsystem. On this MRAM-boot
 * image the wake re-enters through the reset vector, so HP REBOOTS on every
 * doorbell; main() re-runs and the pending doorbell is delivered as soon as
 * ipm_set_enabled() re-arms the RX IRQ.
 *
 * Each doorbell wake is a full reset, so no state survives between exchanges:
 * main() simply re-arms the receivers and handles the one pending doorbell.
 *
 * Without CONFIG_PM (default build, no snippet) none of this applies: HP just
 * blocks on the doorbell semaphore and handles each exchange in place.
 */
#if defined(CONFIG_PM)
/* Make SOFT_OFF the only available deep state: lock the shallower states so the
 * idle thread powers the subsystem off while HP blocks on the doorbell. The MHU
 * RX IRQ then wakes it via the EWIC -> reset.
 */
static void pm_lock_for_soft_off(void)
{
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_RAM, PM_ALL_SUBSTATES);
	/* PM_STATE_SOFT_OFF left unlocked. */
}
#endif /* CONFIG_PM */

/* Handle one incoming doorbell: validate HE's block, free it back to the shared
 * heap, and ring the acknowledgment doorbell. Returns true on a clean PASS.
 */
static bool handle_doorbell(void)
{
	/* Capture ISR-written values under interrupt lock to prevent a second
	 * doorbell from clobbering them before we use them.
	 */
	unsigned int key = irq_lock();
	uint32_t local_rx_addr = rx_doorbell_addr;
	const struct device *local_tx_dev = active_tx_dev;

	irq_unlock(key);

	/* Select the shared heap for the MHU channel that rang. */
	struct sys_heap *heap = (struct sys_heap *)
		((local_tx_dev == mhu0_tx_dev) ?
		 SHARED_HEAP_MHU0_CTRL : SHARED_HEAP_MHU1_CTRL);
	uint32_t heap_ctrl = (uint32_t)heap;

	printk("M55-HP: Doorbell RX (%s) addr=0x%08x\n",
	       (local_tx_dev == mhu0_tx_dev) ? "MHU0" : "MHU1", local_rx_addr);

	/* --- Invalidate D-cache before reading HE's block --- */
	CACHE_INVALIDATE(heap_ctrl, SHARED_HEAP_SPAN);
	__DSB();

	volatile struct shared_msg *incoming =
			(volatile struct shared_msg *)local_rx_addr;

	if (incoming->magic != MAGIC_HE) {
		printk("ERROR: Bad magic 0x%08x (expected 0x%08x)\n",
		       incoming->magic, MAGIC_HE);
		return false;
	}

	if (incoming->msg_id != 0) {
		printk("WARN: msg_id mismatch: got %u, expected 0\n",
		       incoming->msg_id);
	}

	uint32_t rx_len = incoming->data_len;

	if (rx_len > MAX_PAYLOAD) {
		printk("ERROR: data_len %u exceeds MAX_PAYLOAD\n", rx_len);
		return false;
	}

	uint32_t computed_cksum = calc_checksum((const uint8_t *)incoming->data,
						rx_len);
	bool ok = (computed_cksum == incoming->checksum);

	printk("HE->HP: block @ 0x%08x msg_id=%u len=%u cksum=0x%x/0x%x %s\n",
	       local_rx_addr, incoming->msg_id, rx_len, computed_cksum,
	       incoming->checksum, ok ? "PASS" : "FAIL");

	/* --- Free the block back to the shared heap --- */
	sys_heap_free(heap, (void *)local_rx_addr);

	/* --- Flush so HE sees the updated (freed) heap state --- */
	CACHE_CLEAN(heap_ctrl, SHARED_HEAP_SPAN);
	__DSB();

	printk("HP: freed block @ 0x%08x\n", local_rx_addr);

	/* --- Ring doorbell to HE acknowledging the free --- */
	doorbell_tx_done = false;
	uint32_t doorbell_val = local_rx_addr;

	int send_ret = ipm_send(local_tx_dev, 0, 0, &doorbell_val, 4);

	if (send_ret != 0) {
		printk("ERROR: ipm_send failed: %d\n", send_ret);
		return false;
	}

	int tx_waited = 0;

	while (!doorbell_tx_done && tx_waited < TX_TIMEOUT_MS) {
		k_sleep(K_MSEC(1));
		tx_waited++;
	}
	if (!doorbell_tx_done) {
		printk("ERROR: TX timeout (%d ms)\n", TX_TIMEOUT_MS);
		return false;
	}
	printk("M55-HP: Doorbell TX (%s) done\n",
	       (local_tx_dev == mhu0_tx_dev) ? "MHU0" : "MHU1");

	return ok;
}

int main(void)
{
	printk("\n==========================================\n");
	printk("M55-HP <-> M55-HE : MHU Doorbell + Shared SRAM1 heap (responder)\n");
	printk("  MHU0 heap ctrl: 0x%08x\n", SHARED_HEAP_MHU0_CTRL);
	printk("  MHU1 heap ctrl: 0x%08x\n", SHARED_HEAP_MHU1_CTRL);
#if defined(CONFIG_PM)
	printk("  PM: SOFT_OFF while waiting; MHU doorbell wakes via EWIC reset\n");
#else
	printk("  PM: disabled -- exchanges run back-to-back "
	       "(build with -S pm-system-off-hp for SOFT_OFF)\n");
#endif
	printk("==========================================\n");

	mhu0_rx_dev = DEVICE_DT_GET(DT_ALIAS(rtssmhu0r));
	mhu0_tx_dev = DEVICE_DT_GET(DT_ALIAS(rtssmhu0s));
	mhu1_rx_dev = DEVICE_DT_GET(DT_ALIAS(rtssmhu1r));
	mhu1_tx_dev = DEVICE_DT_GET(DT_ALIAS(rtssmhu1s));

	if (!device_is_ready(mhu0_rx_dev) || !device_is_ready(mhu0_tx_dev) ||
	    !device_is_ready(mhu1_rx_dev) || !device_is_ready(mhu1_tx_dev)) {
		printk("ERROR: MHU devices not ready!\n");
		return -1;
	}
	printk("MHU devices ready\n");

	ipm_register_callback(mhu0_rx_dev, mhu_rx_callback, NULL);
	ipm_register_callback(mhu0_tx_dev, mhu_tx_callback, NULL);
	ipm_register_callback(mhu1_rx_dev, mhu_rx_callback, NULL);
	ipm_register_callback(mhu1_tx_dev, mhu_tx_callback, NULL);

	/* Re-arming the RX channels delivers any doorbell that woke us from
	 * SOFT_OFF (the EWIC transfers the pending IRQ into the NVIC on reset).
	 */
	ipm_set_enabled(mhu0_rx_dev, true);
	ipm_set_enabled(mhu1_rx_dev, true);

#if defined(CONFIG_PM)
	/* SOFT_OFF while waiting. The enabled MHU RX IRQs (MHU0=41, MHU1=43) are
	 * within the EWIC's 0-63 range, so HE ringing the doorbell wakes HP --
	 * via reset on this MRAM-boot image.
	 */
	pm_lock_for_soft_off();
#endif /* CONFIG_PM */

	/* HE owns and initialises the shared heaps; HP only frees blocks
	 * from them, so nothing to clear here.
	 */
	printk("Waiting for HE doorbell...\n\n");

#if defined(CONFIG_PM)
	/* Reset-based responder: between doorbells the idle thread enters
	 * SOFT_OFF; the next doorbell wakes HP via an EWIC reset, so on hardware
	 * this loop body runs once per boot (the pending doorbell is delivered
	 * right after ipm_set_enabled() above re-arms the RX IRQ).
	 */
	for (;;) {
		k_sem_take(&doorbell_rx_sem, K_FOREVER);

		if (handle_doorbell()) {
			printk("Complete\n\n");
		}
	}
#else
	/* No power management: the core never resets, so handle NUM_EXCHANGES
	 * doorbells back-to-back, then report the result and stop.
	 */
	unsigned int passed = 0;

	for (unsigned int i = 0; i < NUM_EXCHANGES; i++) {
		k_sem_take(&doorbell_rx_sem, K_FOREVER);

		if (handle_doorbell()) {
			passed++;
			printk("Complete\n\n");
		}
	}

	printk("\n==========================================\n");
	printk("M55-HP: done -- %u/%u exchanges PASSED\n", passed, NUM_EXCHANGES);
	printk("==========================================\n");

	return 0;
#endif /* CONFIG_PM */
}

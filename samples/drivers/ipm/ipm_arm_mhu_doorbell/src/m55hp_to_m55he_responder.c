/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
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
 *     shared heap, and rings back the freed address as an acknowledgement.
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

#if defined(CONFIG_PM)
#include <zephyr/pm/pm.h>
#include <zephyr/pm/policy.h>
#endif

#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
#define CACHE_INVALIDATE(addr, size) SCB_InvalidateDCache_by_Addr((void *)(addr), (int32_t)(size))
#define CACHE_CLEAN(addr, size)      SCB_CleanDCache_by_Addr((void *)(addr), (int32_t)(size))
#else
#define CACHE_INVALIDATE(addr, size) __DSB()
#define CACHE_CLEAN(addr, size)      __DSB()
#endif

#define ITERATIONS       25
#define MAX_ERRORS       10
#define TX_TIMEOUT_MS    1000

/* Shared SRAM1 cross-core heaps for HE <-> HP inter-core communication.
 * Each MHU channel has its own heap whose struct sys_heap control block
 * and managed memory both live in shared SRAM. HE allocates blocks; HP
 * frees them on the same heap instance.
 *
 *   MHU0 heap: ctrl @ 0x027DC000, mem @ 0x027DC100 (size 0x700)
 *   MHU1 heap: ctrl @ 0x027DC800, mem @ 0x027DC900 (size 0x700)
 */
#define SHARED_HEAP_MHU0_CTRL  0x027DC000
#define SHARED_HEAP_MHU1_CTRL  0x027DC800
#define SHARED_HEAP_SIZE       0x700
#define SHARED_HEAP_SPAN       0x800   /* ctrl block + managed memory */

#define MAGIC_HE         0xCAFE0E00   /* Written by HE */
#define MAGIC_HP         0xCAFE0F00   /* Written by HP */
#define MAX_PAYLOAD      240

struct shared_msg {
	uint32_t magic;
	uint32_t msg_id;
	uint32_t data_len;
	uint8_t  data[MAX_PAYLOAD];
	uint32_t checksum;
};

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

	rx_doorbell_addr = *((uint32_t *)data);
	active_tx_dev = (dev == mhu0_rx_dev) ? mhu0_tx_dev : mhu1_tx_dev;
	printk("M55-HP: Doorbell RX (%s) Ch%d, addr=0x%08x\n",
	       (dev == mhu0_rx_dev) ? "MHU0" : "MHU1", id, rx_doorbell_addr);
	k_sem_give(&doorbell_rx_sem);
}

static void mhu_tx_callback(const struct device *dev, void *user_data,
			    uint32_t id, volatile void *data)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(user_data);
	ARG_UNUSED(data);

	printk("M55-HP: Doorbell TX Ch%d done\n", id);
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

#if defined(CONFIG_PM)
/*
 * Power management is enabled only when this sample is built with the
 * pm-system-off-hp snippet. The snippet configures the SE run/off power
 * profiles in devicetree (applied automatically by se_service) and the deep
 * off-state nodes. During the doorbell protocol the core idles into
 * PM_STATE_SUSPEND_TO_IDLE (see pm_lock_low_power_states()); the deep sleep is
 * entered once, at the very end.
 */

/* While the doorbell protocol runs, lock the deep states (SUSPEND_TO_RAM /
 * SOFT_OFF) and leave PM_STATE_SUSPEND_TO_IDLE unlocked, so the idle thread
 * always enters suspend-to-idle while it waits on the doorbell semaphore.
 * Suspend-to-idle uses the IWIC, which wakes on any NVIC interrupt 0-63 -- in
 * particular the MHU doorbell RX IRQ (numbers 41/43, both < 64) -- so the
 * responder reliably wakes when HE rings the bell. No idle timer is needed
 * because the responder waits with k_sem_take(K_FOREVER) (no timed wake), so
 * the suspend_idle node works even with CORTEX_M_SYSTICK_LPM_TIMER_NONE.
 *
 * The deep states must stay locked while waiting: SUSPEND_TO_RAM has no HP
 * retention and SOFT_OFF's SE off-profile wake-events are AON timers
 * (LPTIMER/VBAT) -- NOT the MHU -- so neither can wake on a doorbell. Only
 * release them (lock=false) for the final deep sleep, once the protocol has
 * completed.
 */
static void pm_lock_low_power_states(bool lock)
{
	const enum pm_state states[] = {
		PM_STATE_SUSPEND_TO_RAM,
		PM_STATE_SOFT_OFF,
	};

	for (int i = 0; i < (int)ARRAY_SIZE(states); i++) {
		if (lock) {
			pm_policy_state_lock_get(states[i], PM_ALL_SUBSTATES);
		} else {
			pm_policy_state_lock_put(states[i], PM_ALL_SUBSTATES);
		}
	}
}
#endif /* CONFIG_PM */

int main(void)
{
	int iter;

	printk("\n==========================================\n");
	printk("M55-HP <-> M55-HE : MHU Doorbell + Shared SRAM1 heap (responder)\n");
	printk("  MHU0 heap ctrl: 0x%08x\n", SHARED_HEAP_MHU0_CTRL);
	printk("  MHU1 heap ctrl: 0x%08x\n", SHARED_HEAP_MHU1_CTRL);
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

	ipm_set_enabled(mhu0_rx_dev, true);
	ipm_set_enabled(mhu1_rx_dev, true);

#if defined(CONFIG_PM)
	/* Lock the deep states for the duration of the protocol so the idle
	 * thread always enters suspend-to-idle while waiting on the doorbell.
	 * Suspend-to-idle wakes on the MHU RX IRQ via the IWIC, so the responder
	 * reliably wakes when HE rings the doorbell. The deep states are released
	 * for the final deep sleep.
	 */
	pm_lock_low_power_states(true);
#endif

	/* HE owns and initialises the shared heaps; HP only frees blocks
	 * from them, so nothing to clear here.
	 */
	printk("Waiting for HE doorbell...\n\n");

	int completed = 0;
	int errors = 0;

	for (iter = 0; iter < ITERATIONS; iter++) {
		/* Sleep until HE rings the doorbell. The MHU RX interrupt wakes
		 * the core (a plain WFI in the idle thread) and the callback
		 * gives the semaphore; until then this thread is suspended.
		 * After responding the loop returns here and sleeps again.
		 */
		k_sem_take(&doorbell_rx_sem, K_FOREVER);

		/* Capture ISR-written values under interrupt lock to prevent
		 * a second doorbell from clobbering them before we use them.
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

		/* --- Invalidate D-cache before reading HE's block --- */
		CACHE_INVALIDATE(heap_ctrl, SHARED_HEAP_SPAN);
		__DSB();

		volatile struct shared_msg *incoming =
				(volatile struct shared_msg *)local_rx_addr;

		if (incoming->magic != MAGIC_HE) {
			printk("ERROR: Bad magic 0x%08x (expected 0x%08x)\n",
			       incoming->magic, MAGIC_HE);
			errors++;
			if (errors >= MAX_ERRORS)
				break;
			continue;
		}

		if (incoming->msg_id != (uint32_t)iter) {
			printk("WARN: msg_id mismatch: got %u, expected %d\n",
			       incoming->msg_id, iter);
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
		bool ok = (computed_cksum == incoming->checksum);

		printk("[%d] HE->HP: block @ 0x%08x msg_id=%u len=%u "
		       "cksum=0x%x/0x%x %s\n", iter, local_rx_addr,
		       incoming->msg_id, rx_len, computed_cksum,
		       incoming->checksum, ok ? "PASS" : "FAIL");

		/* --- Free the block back to the shared heap --- */
		sys_heap_free(heap, (void *)local_rx_addr);

		/* --- Flush so HE sees the updated (freed) heap state --- */
		CACHE_CLEAN(heap_ctrl, SHARED_HEAP_SPAN);
		__DSB();

		printk("[%d] HP: freed block @ 0x%08x\n",
		       iter, local_rx_addr);

		/* --- Ring doorbell to HE acknowledging the free --- */
		doorbell_tx_done = false;
		uint32_t doorbell_val = local_rx_addr;

		int send_ret = ipm_send(local_tx_dev, 0, 0,
					&doorbell_val, 4);
		if (send_ret != 0) {
			printk("ERROR: ipm_send failed: %d\n", send_ret);
			errors++;
			continue;
		}

		int tx_waited = 0;

		while (!doorbell_tx_done && tx_waited < TX_TIMEOUT_MS) {
			k_sleep(K_MSEC(1));
			tx_waited++;
		}
		if (!doorbell_tx_done) {
			printk("ERROR: TX timeout (%d ms)\n", TX_TIMEOUT_MS);
			break;
		}

		if (ok) {
			printk("[%d] Complete\n\n", iter);
			completed++;
		} else {
			printk("[%d] CKSUM FAIL\n\n", iter);
			errors++;
		}

		if (errors >= MAX_ERRORS) {
			printk("ERROR: Too many errors (%d), aborting\n",
			       errors);
			break;
		}
	}

	ipm_set_enabled(mhu0_rx_dev, false);
	ipm_set_enabled(mhu1_rx_dev, false);
	printk("Done: %d/%d passed, %d failed\n",
	       completed, ITERATIONS, errors);

	return (completed + errors == ITERATIONS) ? 0 : -1;
}

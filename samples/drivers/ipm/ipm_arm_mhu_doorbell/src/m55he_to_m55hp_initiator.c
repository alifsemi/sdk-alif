/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 * MHU Doorbell + Shared SRAM1 heap test: M55-HE (initiator) <-> M55-HP (responder)
 *
 * Protocol:
 *   - A struct sys_heap and the memory it manages both live in shared
 *     SRAM1, so both cores operate on the same heap instance.
 *   - HE (initiator) dynamically allocates a message block from the
 *     shared heap, fills it with a payload, and rings the MHU doorbell
 *     passing the physical address of the allocated block.
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
 *   1. HE allocates a block from the shared heap, writes test data,
 *      flushes D-cache
 *   2. HE rings doorbell to HP with the allocated block address
 *   3. HE waits for HP's acknowledgement doorbell (the freed address)
 *   4. HE verifies the acknowledged address matches the block it sent
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/sys_heap.h>
#include <zephyr/drivers/ipm.h>
#include <string.h>

#if defined(CONFIG_PM)
#include <errno.h>
#include <zephyr/pm/pm.h>
#include <zephyr/pm/policy.h>
#if defined(CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER)
#include <zephyr/drivers/counter.h>
#define IDLE_TIMER_NODE DT_CHOSEN(zephyr_cortex_m_idle_timer)
#endif
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
#define RX_TIMEOUT_MS    5000
#define TX_TIMEOUT_MS    1000

/* Shared SRAM1 cross-core heap for HE <-> HP inter-core communication.
 * The struct sys_heap control block and the memory it manages both live
 * in shared SRAM so the initiator (HE) and responder (HP) operate on the
 * same heap instance.
 *
 *   MHU0 heap: ctrl @ 0x027DC000, mem @ 0x027DC100 (size 0x700)
 *   MHU1 heap: ctrl @ 0x027DC800, mem @ 0x027DC900 (size 0x700)
 */
#if IS_ENABLED(CONFIG_USE_MHU1)
#define SHARED_HEAP_CTRL  0x027DC800
#define SHARED_HEAP_MEM   0x027DC900
#else
#define SHARED_HEAP_CTRL  0x027DC000
#define SHARED_HEAP_MEM   0x027DC100
#endif
#define SHARED_HEAP_SIZE  0x700
#define SHARED_HEAP_SPAN  0x800   /* ctrl block + managed memory */

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

#if IS_ENABLED(CONFIG_USE_MHU1)
#define MHU_NAME "MHU1"
#define MHU_R_ALIAS rtssmhu1r
#define MHU_S_ALIAS rtssmhu1s
#else
#define MHU_NAME "MHU0"
#define MHU_R_ALIAS rtssmhu0r
#define MHU_S_ALIAS rtssmhu0s
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

#if defined(CONFIG_PM)
/*
 * Power management is enabled only when this sample is built with the
 * pm-system-off-he snippet. The snippet applies the SE run/off power profiles
 * in devicetree (applied automatically by se_service) and the deep off-state
 * nodes.
 *
 * Under the SE run profile a plain WFI is a real low-power state in which the
 * Cortex-M SysTick is NOT clocked (see the comment in
 * drivers/timer/cortex_m_systick.c). So we MUST let the kernel idle path enter
 * PM_STATE_SUSPEND_TO_IDLE: that path arms the rtc0 LPM idle-timer
 * (zephyr,cortex-m-idle-timer) as the wake source for k_sleep() timeouts, and
 * because suspend-to-idle uses the IWIC it ALSO wakes on the MHU doorbell IRQ.
 * We only lock the deep states (SUSPEND_TO_RAM / SOFT_OFF), whose multi-second
 * min-residency and power-gating would otherwise tear down the MHU / SRAM mid
 * protocol, so the core always idles into SUSPEND_TO_IDLE during the protocol.
 * The deepest state is entered once, at the very end.
 *
 * NOTE: the systick driver only *arms* an alarm on the idle-timer, it never
 * starts the counter -- the application must counter_start() it before the
 * first k_sleep(), otherwise the alarm never fires and k_sleep() hangs forever
 * (see pm_start_idle_timer()).
 */
static void pm_lock_low_power_states(bool lock)
{
	/* Leave PM_STATE_SUSPEND_TO_IDLE unlocked: it is the only state whose
	 * wake source (rtc0 idle-timer + IWIC MHU IRQ) works during the
	 * protocol. Lock the deep states so the core always idles into
	 * suspend-to-idle and never tears down the MHU / SRAM mid protocol.
	 */
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

#if defined(CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER)
/* Start the rtc0 LPM idle-timer. The cortex_m systick driver programs wake
 * alarms on this counter while idle but never starts it, so it must be running
 * before the first k_sleep() or the suspend-to-idle wake alarm never fires.
 */
static void pm_start_idle_timer(void)
{
	const struct device *const idle_timer = DEVICE_DT_GET(IDLE_TIMER_NODE);
	int ret;

	if (!device_is_ready(idle_timer)) {
		printk("PM: idle timer not ready\n");
		return;
	}

	ret = counter_start(idle_timer);
	if (ret && ret != -EALREADY) {
		printk("PM: counter_start failed (%d)\n", ret);
	}
}
#endif /* CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER */
#endif /* CONFIG_PM */

int main(void)
{
	int iter;
	struct sys_heap *heap = (struct sys_heap *)SHARED_HEAP_CTRL;

	printk("\n==========================================\n");
	printk("M55-HE -> M55-HP : %s Doorbell + Shared SRAM1 heap (initiator)\n", MHU_NAME);
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

#if defined(CONFIG_PM)
	/* Lock the deep low-power states for the duration of the protocol so the
	 * idle thread only ever enters suspend-to-idle. That keeps the MHU and
	 * shared SRAM powered while still letting the rtc0 idle-timer wake each
	 * k_sleep() and the MHU IRQ wake the doorbell waits. The deep states are
	 * released for the final deep sleep.
	 */
	pm_lock_low_power_states(true);
#if defined(CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER)
	/* Must run before the first k_sleep() below (see pm_start_idle_timer). */
	pm_start_idle_timer();
#endif
#endif

	/* Initialise the cross-core heap in shared SRAM. Both the control
	 * block and the managed memory live in shared SRAM so HP can free
	 * blocks allocated here. Flush so HP observes the initialised heap.
	 */
	sys_heap_init(heap, (void *)SHARED_HEAP_MEM, SHARED_HEAP_SIZE);
	CACHE_CLEAN(SHARED_HEAP_CTRL, SHARED_HEAP_SPAN);
	__DSB();

	/* Give HP time to be ready */
	printk("Waiting for HP to be ready...\n");
	k_sleep(K_SECONDS(1));

	int completed = 0;
	int errors = 0;

	for (iter = 0; iter < ITERATIONS; iter++) {
		/* --- Pick up the freed heap state from HP's last iteration --- */
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

		/* --- Flush so HP sees the allocation metadata and payload --- */
		CACHE_CLEAN(SHARED_HEAP_CTRL, SHARED_HEAP_SPAN);
		__DSB();

		printk("[%d] HE->HP: alloc block @ 0x%08x msg_id=%u len=%u "
		       "cksum=0x%x\n", iter, block_addr, tx_msg->msg_id,
		       tx_msg->data_len, tx_msg->checksum);

		/* --- Ring doorbell to HP with the allocated block address --- */
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

		/* --- Wait for HP's acknowledgement doorbell --- */
		waited = 0;
		while (!doorbell_rx_received && waited < RX_TIMEOUT_MS) {
			k_sleep(K_MSEC(1));
			waited++;
		}
		if (!doorbell_rx_received) {
			printk("ERROR: Timeout waiting for HP ack "
			       "(%d ms)\n", RX_TIMEOUT_MS);
			errors++;
			if (errors >= MAX_ERRORS)
				break;
			continue;
		}

		/* HP acknowledges the free by returning the block address --- */
		if (rx_doorbell_addr != block_addr) {
			printk("[%d] HP->HE: FAIL ack addr 0x%08x != "
			       "block 0x%08x\n", iter, rx_doorbell_addr,
			       block_addr);
			errors++;
			if (errors >= MAX_ERRORS) {
				printk("ERROR: Too many errors (%d), aborting\n",
				       errors);
				break;
			}
			continue;
		}

		/* HP read/validated the block and freed it back to the shared
		 * heap, then rang back the freed address as an acknowledgement.
		 */
		printk("[%d] HP->HE: ack free of block @ 0x%08x PASS\n\n",
		       iter, block_addr);
		completed++;

		/* Delay between iterations. With CONFIG_PM this k_sleep also
		 * enters low power after each send: the deep states are locked so
		 * the PM policy selects SUSPEND_TO_IDLE (the MHU/SRAM survive) and
		 * the chosen idle timer (rtc0) wakes the core for the next
		 * iteration.
		 */
		k_sleep(K_MSEC(500));
	}

	ipm_set_enabled(mhu_rx_dev, false);
	printk("Done: %d/%d passed, %d failed\n",
	       completed, ITERATIONS, errors);

	return (completed + errors == ITERATIONS) ? 0 : -1;
}

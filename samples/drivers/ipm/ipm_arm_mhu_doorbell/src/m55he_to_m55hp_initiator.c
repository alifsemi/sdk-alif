/* Copyright (C) Alif Semiconductor - All Rights Reserved.
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
 * Test flow (one exchange per boot):
 *   1. HE allocates a block from the shared heap, writes test data,
 *      flushes D-cache
 *   2. HE rings doorbell to HP with the allocated block address
 *   3. HE waits for HP's acknowledgment doorbell (the freed address)
 *   4. HE verifies the acknowledged address matches the block it sent
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/sys_heap.h>
#include <zephyr/drivers/ipm.h>
#include <string.h>
#include <errno.h>
#include <zephyr/pm/pm.h>
#include <zephyr/pm/policy.h>
#include <zephyr/drivers/counter.h>

#include "mhu_doorbell_msg.h"
#include "mhu_doorbell_heap.h"

#if defined(CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER)
#define IDLE_TIMER_NODE DT_CHOSEN(zephyr_cortex_m_idle_timer)
#endif
/* LPTIMER0 (timer0) is the SOFT_OFF wake source (off_profile_soft_off ->
 * ALIF_WE_LPTIMER0). It must be enabled in the build overlay (lpuart.overlay).
 */
#define SOFT_OFF_WAKE_TIMER_NODE DT_NODELABEL(timer0)
#define SOFT_OFF_PERIOD_USEC     (2 * 1000 * 1000)   /* 2 s LPTIMER0 wake */

#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
#define CACHE_INVALIDATE(addr, size) SCB_InvalidateDCache_by_Addr((void *)(addr), (int32_t)(size))
#define CACHE_CLEAN(addr, size)      SCB_CleanDCache_by_Addr((void *)(addr), (int32_t)(size))
#else
#define CACHE_INVALIDATE(addr, size) __DSB()
#define CACHE_CLEAN(addr, size)      __DSB()
#endif

#define RX_TIMEOUT_MS    5000
#define TX_TIMEOUT_MS    1000

/* Without CONFIG_PM (no pm-system-off-he snippet) the core never resets, so run
 * a fixed number of exchanges back-to-back and then stop with a summary.
 */
#define NUM_EXCHANGES    10

/* Shared SRAM1 cross-core heap for HE <-> HP inter-core communication (one
 * heap per MHU channel). The struct sys_heap control block and the memory it
 * manages both live in shared SRAM so the initiator (HE) and responder (HP)
 * operate on the same heap instance. Addresses come from mhu_doorbell_heap.h.
 */
#if IS_ENABLED(CONFIG_USE_MHU1)
#define SHARED_HEAP_CTRL  SHARED_HEAP_HEHP_MHU1_CTRL
#define SHARED_HEAP_MEM   SHARED_HEAP_HEHP_MHU1_MEM
#else
#define SHARED_HEAP_CTRL  SHARED_HEAP_HEHP_MHU0_CTRL
#define SHARED_HEAP_MEM   SHARED_HEAP_HEHP_MHU0_MEM
#endif

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
	ARG_UNUSED(id);

	/* Keep the ISR minimal: no printk here (blocking/latency in interrupt
	 * context). The main loop logs the doorbell once it observes the flag.
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

#if defined(CONFIG_PM)
/*
 * Low-power model: HE deep-sleeps to SOFT_OFF between exchanges, and HP runs
 * its own reset-based SOFT_OFF responder (m55hp_to_m55he_responder.c).
 *
 * Built with the pm-system-off-he snippet, which applies the SE run/off power
 * profiles in devicetree. Per cycle:
 *   1. HE wakes -- on this MRAM-boot image SOFT_OFF resets the core, so the
 *      LPTIMER0 wake re-runs main() from the reset vector.
 *   2. HE rings the MHU doorbell to HP (which is itself in SOFT_OFF and wakes
 *      via its EWIC -- see m55hp_to_m55he_responder.c).
 *   3. HE waits for HP's ack in SUSPEND_TO_IDLE: that state uses the IWIC and
 *      wakes on the MHU doorbell IRQ, and the rtc0 LPM idle-timer
 *      (zephyr,cortex-m-idle-timer) backs the k_sleep() poll timeouts.
 *   4. HE re-enters SOFT_OFF and arms LPTIMER0 for a 2 s wake.
 *
 * Why SOFT_OFF resets instead of resuming: SOFT_OFF wakes only on the EWIC AON
 * events (here LPTIMER0, via off_profile_soft_off) and NEVER on the MHU. On an
 * MRAM boot (VTOR >= 0x80000000) the off profile re-enters through the reset
 * vector, so the LPTIMER0 wake reboots HE.
 *
 * No state survives a cycle: a real dual-core SOFT_OFF does not retain SRAM1
 * content (ALIF_SRAM1_MASK keeps SRAM1 powered while RUNNING, but retention in
 * the OFF state would need ALIF_SRAM1_RET_MASK, which keeps a rail up and
 * defeats the deep power-off this sample demonstrates). So each boot re-creates
 * the cross-core heap and performs exactly one HE->HP exchange.
 *
 * NOTE: the systick driver only *arms* alarms on the idle-timer, it never
 * starts the counter, so pm_start_idle_timer() must run before the first
 * k_sleep() or the SUSPEND_TO_IDLE wake alarm never fires.
 */
static void pm_state_lock(enum pm_state state, bool lock)
{
	if (lock) {
		pm_policy_state_lock_get(state, PM_ALL_SUBSTATES);
	} else {
		pm_policy_state_lock_put(state, PM_ALL_SUBSTATES);
	}
}

/* Keep only SUSPEND_TO_IDLE available (MHU-wakeable) for the doorbell protocol;
 * lock SUSPEND_TO_RAM and SOFT_OFF.
 */
static void pm_lock_for_protocol(void)
{
	pm_state_lock(PM_STATE_SUSPEND_TO_RAM, true);
	pm_state_lock(PM_STATE_SOFT_OFF, true);
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

#if DT_NODE_HAS_STATUS(DT_NODELABEL(timer0), okay)
/* Mandatory (no-op) alarm callback for LPTIMER0 -- the dw-timer counter driver
 * rejects a NULL callback. SOFT_OFF resets the core before it would run.
 */
static void softoff_alarm_cb(const struct device *dev, uint8_t chan,
			     uint32_t ticks, void *user_data)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(chan);
	ARG_UNUSED(ticks);
	ARG_UNUSED(user_data);
}

/* LPTIMER0 TimerEOI register (DW APB timer): reading it clears the latched
 * interrupt flag. Offset 0xC from the timer base.
 */
#define LPTIMER0_EOI_ADDR (DT_REG_ADDR(SOFT_OFF_WAKE_TIMER_NODE) + 0xCu)
#define LPTIMER0_IRQ_NUM  DT_IRQN(SOFT_OFF_WAKE_TIMER_NODE)

/* Clear any stale LPTIMER0 interrupt left over from the previous SOFT_OFF wake.
 * LPTIMER0 is in the always-on VBAT domain, so it is NOT reset across the
 * subsystem reset, and it fired with the core powered off -- the dw-timer ISR
 * never ran, so its EOI was never read and the latched IRQ persists (the timer
 * is also periodic and keeps reloading). Without clearing it, the next
 * counter_set_channel_alarm() unmasks an already-pending IRQ that fires
 * immediately, consuming that cycle's alarm -- so every other SOFT_OFF wakes
 * instantly instead of staying off ~2 s. Read EOI to clear the peripheral
 * latch, then drop any NVIC pending bit.
 */
static void lptimer0_clear_stale_irq(void)
{
	(void)sys_read32(LPTIMER0_EOI_ADDR);
	NVIC_ClearPendingIRQ(LPTIMER0_IRQ_NUM);
}
#endif /* timer0 okay */

/* Enter PM_STATE_SOFT_OFF until the LPTIMER0 wake. Make SOFT_OFF the only
 * available deep state, arm LPTIMER0 (the off_profile_soft_off wake source --
 * the rtc0 idle-timer is powered down in that profile), then sleep longer than
 * the subsys_off min-residency so the idle policy commits to SOFT_OFF. On MRAM
 * boot the LPTIMER0 wake resets the core.
 */
static void pm_enter_soft_off(void)
{
	pm_state_lock(PM_STATE_SUSPEND_TO_IDLE, true);
	pm_state_lock(PM_STATE_SUSPEND_TO_RAM, true);
	pm_state_lock(PM_STATE_SOFT_OFF, false);

#if DT_NODE_HAS_STATUS(DT_NODELABEL(timer0), okay)
	const struct device *const lptimer =
		DEVICE_DT_GET(SOFT_OFF_WAKE_TIMER_NODE);
	struct counter_alarm_cfg alarm_cfg = {
		.flags = 0,
		.ticks = counter_us_to_ticks(lptimer, SOFT_OFF_PERIOD_USEC),
		.callback = softoff_alarm_cb,
		.user_data = NULL,
	};

	if (device_is_ready(lptimer)) {
		int ret;

		(void)counter_start(lptimer);
		/* Drop the previous cycle's stale latched IRQ before arming, or
		 * every other SOFT_OFF would wake instantly (see helper).
		 */
		lptimer0_clear_stale_irq();
		ret = counter_set_channel_alarm(lptimer, 0, &alarm_cfg);
		if (ret) {
			printk("PM: failed to arm LPTIMER0 for SOFT_OFF (%d)\n",
			       ret);
		}
	} else {
		printk("PM: LPTIMER0 (timer0) not ready -- SOFT_OFF has no wake\n");
	}
#else
	printk("PM: timer0 disabled -- SOFT_OFF has no LPTIMER0 wake "
	       "(enable &timer0 in the build overlay)\n");
#endif

	printk("M55-HE: entering SOFT_OFF; LPTIMER0 wake in %u ms (core resets)\n",
	       SOFT_OFF_PERIOD_USEC / 1000U);

	/* Sleep longer than subsys_off min-residency so the policy enters
	 * SOFT_OFF; LPTIMER0 fires first and resets the core.
	 */
	k_sleep(K_USEC(SOFT_OFF_PERIOD_USEC));
}
#endif /* CONFIG_PM */

/* Perform one HE->HP allocate/send/ack exchange. Returns true on a fully
 * successful round-trip, false on any error (each failure is logged). The
 * doorbell waits run in PM_STATE_SUSPEND_TO_IDLE so the MHU IRQ can wake the
 * core.
 */
static bool do_one_exchange(struct sys_heap *heap)
{
	/* Read the freshly initialised heap state from memory. */
	CACHE_INVALIDATE(SHARED_HEAP_CTRL, SHARED_HEAP_SPAN);
	__DSB();

	volatile struct shared_msg *tx_msg =
			sys_heap_alloc(heap, sizeof(struct shared_msg));

	if (tx_msg == NULL) {
		printk("ERROR: sys_heap_alloc failed\n");
		return false;
	}

	uint32_t block_addr = (uint32_t)tx_msg;

	/* Write test data into the allocated block. */
	tx_msg->magic    = MAGIC_HE;
	tx_msg->msg_id   = 0;
	tx_msg->data_len = 16;
	for (uint32_t j = 0; j < tx_msg->data_len; j++) {
		tx_msg->data[j] = (uint8_t)j;
	}
	memset((void *)&tx_msg->data[tx_msg->data_len], 0,
	       MAX_PAYLOAD - tx_msg->data_len);
	tx_msg->checksum = calc_checksum((const uint8_t *)tx_msg->data,
					 tx_msg->data_len);

	/* Flush so HP sees the allocation metadata and payload. */
	CACHE_CLEAN(SHARED_HEAP_CTRL, SHARED_HEAP_SPAN);
	__DSB();

	printk("HE->HP: alloc block @ 0x%08x msg_id=%u len=%u cksum=0x%x\n",
	       block_addr, tx_msg->msg_id, tx_msg->data_len,
	       tx_msg->checksum);

	/* Ring doorbell to HP with the allocated block address. */
	doorbell_tx_done = false;
	doorbell_rx_received = false;
	uint32_t doorbell_val = block_addr;

	int send_ret = ipm_send(mhu_tx_dev, 0, 0, &doorbell_val, 4);

	if (send_ret != 0) {
		printk("ERROR: ipm_send failed: %d\n", send_ret);
		return false;
	}

	int waited = 0;

	while (!doorbell_tx_done && waited < TX_TIMEOUT_MS) {
		k_sleep(K_MSEC(1));
		waited++;
	}
	if (!doorbell_tx_done) {
		printk("ERROR: TX timeout (%d ms)\n", TX_TIMEOUT_MS);
		return false;
	}
	printk("M55-HE: Doorbell TX (%s) done\n", MHU_NAME);

	/* Wait for HP's acknowledgment doorbell. */
	waited = 0;
	while (!doorbell_rx_received && waited < RX_TIMEOUT_MS) {
		k_sleep(K_MSEC(1));
		waited++;
	}
	if (!doorbell_rx_received) {
		printk("ERROR: Timeout waiting for HP ack (%d ms)\n",
		       RX_TIMEOUT_MS);
		return false;
	}
	printk("M55-HE: Doorbell RX (%s) addr=0x%08x\n", MHU_NAME,
	       rx_doorbell_addr);

	/* HP acknowledges the free by returning the block address. */
	if (rx_doorbell_addr != block_addr) {
		printk("HP->HE: FAIL ack addr 0x%08x != block 0x%08x\n",
		       rx_doorbell_addr, block_addr);
		return false;
	}

	printk("HP->HE: ack free of block @ 0x%08x PASS\n\n", block_addr);
	return true;
}

int main(void)
{
	struct sys_heap *heap = (struct sys_heap *)SHARED_HEAP_CTRL;

	mhu_rx_dev = DEVICE_DT_GET(DT_ALIAS(MHU_R_ALIAS));
	mhu_tx_dev = DEVICE_DT_GET(DT_ALIAS(MHU_S_ALIAS));

	if (!device_is_ready(mhu_rx_dev) || !device_is_ready(mhu_tx_dev)) {
		printk("ERROR: %s devices not ready!\n", MHU_NAME);
		return -1;
	}

	ipm_register_callback(mhu_rx_dev, mhu_rx_callback, NULL);
	ipm_register_callback(mhu_tx_dev, mhu_tx_callback, NULL);
	ipm_set_enabled(mhu_rx_dev, true);

#if defined(CONFIG_PM)
	/* Lock the deep states (S2RAM/SOFT_OFF) and start the rtc0 idle-timer
	 * BEFORE any k_sleep(). Otherwise the "wait for HP" sleep in
	 * do_one_exchange() would run with SOFT_OFF still unlocked (min-residency
	 * 0.5 s < 2 s) and no LPTIMER0 armed, so HE would power off with no wake
	 * source and hang. With only SUSPEND_TO_IDLE available and the idle timer
	 * running, that sleep is woken by rtc0 as intended.
	 */
	pm_lock_for_protocol();
#if defined(CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER)
	pm_start_idle_timer();
#endif
#endif /* CONFIG_PM */

	/* Each LPTIMER0 wake is a full reset and SRAM1 content is not retained
	 * across the dual-core SOFT_OFF, so every boot starts fresh: print the
	 * banner, (re)create the cross-core heap, and run one exchange.
	 */
	printk("\n==========================================\n");
	printk("M55-HE -> M55-HP : %s Doorbell + Shared SRAM1 heap (initiator)\n", MHU_NAME);
	printk("  Shared heap ctrl: 0x%08x\n", SHARED_HEAP_CTRL);
	printk("  Shared heap mem : 0x%08x (size 0x%x)\n", SHARED_HEAP_MEM, SHARED_HEAP_SIZE);
#if defined(CONFIG_PM)
	printk("  PM: SOFT_OFF between exchanges, %u ms LPTIMER0 wake "
	       "(each wake resets HE)\n", SOFT_OFF_PERIOD_USEC / 1000U);
#else
	printk("  PM: disabled -- exchanges run back-to-back "
	       "(build with -S pm-system-off-he for SOFT_OFF)\n");
#endif
	printk("==========================================\n");

	/* HE owns the cross-core heap; initialise it before the first alloc. The
	 * doorbell can be sent right away: the MHU channel bit latches in hardware
	 * until HP acknowledges it, and HP is woken by that very doorbell
	 * (EWIC -> reset), so no readiness wait is needed.
	 */
	sys_heap_init(heap, (void *)SHARED_HEAP_MEM, SHARED_HEAP_SIZE);
	CACHE_CLEAN(SHARED_HEAP_CTRL, SHARED_HEAP_SPAN);
	__DSB();

#if defined(CONFIG_PM)
	(void)do_one_exchange(heap);

	/* Back to SOFT_OFF until the LPTIMER0 wake resets HE for the next
	 * exchange. The test runs indefinitely (one exchange per boot).
	 */
	pm_enter_soft_off();

	/* Not reached on MRAM boot (SOFT_OFF resets the core). */
	return 0;
#else
	/* No power management: SRAM1 stays powered and the core never resets, so
	 * run NUM_EXCHANGES exchanges back-to-back on the same heap (HP frees each
	 * block before the next alloc), then report the result and stop.
	 */
	unsigned int passed = 0;

	for (unsigned int i = 0; i < NUM_EXCHANGES; i++) {
		if (do_one_exchange(heap)) {
			passed++;
		}
		if (i + 1 < NUM_EXCHANGES) {
			k_sleep(K_MSEC(SOFT_OFF_PERIOD_USEC / 1000U));
		}
	}

	printk("\n==========================================\n");
	printk("M55-HE: done -- %u/%u exchanges PASSED\n", passed, NUM_EXCHANGES);
	printk("==========================================\n");

	return 0;
#endif /* CONFIG_PM */
}

/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/init.h>
#include <zephyr/pm/pm.h>
#include <zephyr/pm/policy.h>
#include <zephyr/drivers/counter.h>
#include <zephyr/drivers/spi.h>
#include <string.h>
#include <zephyr/sys/util.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(pm_spi_dw, LOG_LEVEL_INF);

/* Set to 1 to dump full NVIC ISPR state on wakeup */
#define APP_PM_WAKEUP_DEBUG 0

#if DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(rtc0), snps_dw_apb_rtc, okay)
	#define WAKEUP_SOURCE DT_NODELABEL(rtc0)
#elif DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(timer0), snps_dw_timers, okay)
	#define WAKEUP_SOURCE DT_NODELABEL(timer0)
#else
#error "Wakeup Device not enabled in the dts"
#endif

/*
 * Sleep duration constants for each PM state.
 *
 * Upper bound: at 400 MHz, ticks to cycles calculation in PM driver
 * overflows uint32 max for values > 10.7s (2^32 / 400). All deep-sleep
 * durations and overlay min-residency-us values must stay below this ceiling.
 */
/* Sleep duration for PM_STATE_RUNTIME_IDLE */
#define RUNTIME_IDLE_SLEEP_USEC (18 * 1000 * 1000)
/* Sleep duration for PM_STATE_SUSPEND_TO_IDLE */
#define SUSPEND_IDLE_SLEEP_USEC (10 * 1000)
/* Sleep duration for PM_STATE_SUSPEND_TO_RAM substate 0 (STANDBY) */
#define S2RAM_STANDBY_SLEEP_USEC (6 * 1000 * 1000)
/* Sleep duration for PM_STATE_SUSPEND_TO_RAM substate 1 (STOP) */
#define S2RAM_STOP_SLEEP_USEC (9 * 1000 * 1000)
/* Sleep duration for PM_STATE_SOFT_OFF */
#define SOFT_OFF_SLEEP_USEC (10 * 1000 * 1000)

/*
 * MRAM base address - used to determine boot location
 * TCM boot: VTOR = 0x0
 * MRAM boot: VTOR >= 0x80000000
 */
#define MRAM_BASE_ADDRESS 0x80000000

#define IS_BOOTING_FROM_MRAM() (SCB->VTOR >= MRAM_BASE_ADDRESS)

/*
 * True when the DTS chosen zephyr,sram points at sram0. The snippet overlay
 * is responsible for ensuring SRAM0 has retention support on the target board.
 */
#if DT_NODE_EXISTS(DT_NODELABEL(sram0))
#define IS_SRAM0_CONFIGURED_AS_RAM() \
	DT_SAME_NODE(DT_CHOSEN(zephyr_sram), DT_NODELABEL(sram0))
#else
#define IS_SRAM0_CONFIGURED_AS_RAM() 0
#endif

/*
 * S2RAM_SUPPORTED — retention-capable sleep is possible when:
 *   - SRAM0 is the configured data RAM (HE or HP, E8 only), OR
 *   - HE core booting from TCM (TCM has hardware retention)
 *
 * SOFT_OFF_SUPPORTED — mutually exclusive with S2RAM: used when no
 * retained RAM is available (HP-TCM, HE-MRAM boot without SRAM0).
 */
#define S2RAM_SUPPORTED \
	(IS_SRAM0_CONFIGURED_AS_RAM() || \
	 (IS_ENABLED(CONFIG_RTSS_HE) && !IS_BOOTING_FROM_MRAM()))

#define SOFT_OFF_SUPPORTED (!S2RAM_SUPPORTED)

/* Validate ordering of deep-sleep durations at compile time */
BUILD_ASSERT(S2RAM_STOP_SLEEP_USEC > S2RAM_STANDBY_SLEEP_USEC,
	"STOP sleep duration must be greater than STANDBY sleep duration");
BUILD_ASSERT(SOFT_OFF_SLEEP_USEC > S2RAM_STOP_SLEEP_USEC,
	"SOFT_OFF sleep duration must be greater than STOP sleep duration");

/**
 * Helper function to lock/unlock deeper power states.
 * @param lock true → lock all deep states (allow RUNTIME_IDLE only)
 *             false → unlock the applicable state; keep the other locked
 */
static void app_pm_lock_deeper_states(bool lock)
{
	if (lock) {
		/*
		 * Lock both deep states unconditionally so PM policy cannot
		 * accidentally enter them during the RUNTIME_IDLE phase, where
		 * the 18 s sleep exceeds the reduced min-residency values.
		 */
		pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_RAM, PM_ALL_SUBSTATES);
		pm_policy_state_lock_get(PM_STATE_SOFT_OFF,       PM_ALL_SUBSTATES);
	} else if (S2RAM_SUPPORTED) {
		pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_RAM, PM_ALL_SUBSTATES);
		/* SOFT_OFF stays locked for the entire demo */
	} else {
		pm_policy_state_lock_put(PM_STATE_SOFT_OFF, PM_ALL_SUBSTATES);
		/* S2RAM stays locked for the entire demo */
	}
}

/*
 * This function will be invoked in the PRE_KERNEL_2 phase of the init routine.
 */
static int app_pre_kernel_init(void)
{
	/* Lock deeper power states to allow only RUNTIME_IDLE */
	app_pm_lock_deeper_states(true);

	return 0;
}
SYS_INIT(app_pre_kernel_init, PRE_KERNEL_2, 0);

#if !defined(CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER)
static volatile uint32_t alarm_cb_status;
static void alarm_callback_fn(const struct device *wakeup_dev,
				uint8_t chan_id, uint32_t ticks,
				void *user_data)
{
	LOG_DBG("%s: Alarm triggered", wakeup_dev->name);
	alarm_cb_status = 1;
}
#endif

static int app_enter_normal_sleep(uint32_t sleep_usec)
{
#if defined(CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER)
	k_sleep(K_USEC(sleep_usec));
#else
	const struct device *const wakeup_dev = DEVICE_DT_GET(WAKEUP_SOURCE);
	struct counter_alarm_cfg alarm_cfg;
	int ret;

	alarm_cfg.flags = 0;
	alarm_cfg.ticks = counter_us_to_ticks(wakeup_dev, sleep_usec);
	alarm_cfg.callback = alarm_callback_fn;
	alarm_cfg.user_data = &alarm_cfg;

	ret = counter_set_channel_alarm(wakeup_dev, 0, &alarm_cfg);
	if (ret) {
		LOG_ERR("Could not set the alarm");
		return ret;
	}
	LOG_DBG("Set alarm for %u microseconds", sleep_usec);

	k_sleep(K_USEC(sleep_usec));

	if (!alarm_cb_status) {
		return -1;
	}
	alarm_cb_status = 0;

#endif
	return 0;
}

static int app_enter_deep_sleep(uint32_t sleep_usec)
{
#if defined(CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER)
	/*
	 * Set a delay more than the min-residency-us configured so that
	 * the sub-system will go to OFF state.
	 */
	k_sleep(K_USEC(sleep_usec));
#else
	const struct device *const wakeup_dev = DEVICE_DT_GET(WAKEUP_SOURCE);
	struct counter_alarm_cfg alarm_cfg;
	int ret;
	/*
	 * Set the alarm and delay so that idle thread can run
	 */
	alarm_cfg.ticks = counter_us_to_ticks(wakeup_dev, sleep_usec);
	ret = counter_set_channel_alarm(wakeup_dev, 0, &alarm_cfg);
	if (ret) {
		LOG_ERR("Failed to set the alarm (err %d)", ret);
		return ret;
	}

	LOG_DBG("Set alarm for %u microseconds", sleep_usec);
	/*
	 * Wait for the alarm to trigger. The idle thread will
	 * take care of entering the deep sleep state via PM framework.
	 */
	k_sleep(K_USEC(sleep_usec));
#endif

	return 0;
}

/* master_spi and slave_spi aliases are defined in
 * overlay files to use different SPI instance if needed.
 */
#define SPIDW_NODE   DT_ALIAS(master_spi)
#define S_SPIDW_NODE DT_ALIAS(slave_spi)

/* size of stack area used by each thread */
#define STACKSIZE 1024

/* default SPI master SS(slave select) is H/W controlled,
 * enable this to use as S/W controlled using gpio.
 */
#define SPI_MASTER_SS_SW_CONTROLLED_GPIO   0

/* scheduling priority used by each thread,
 * as we are testing Loopback on the same board,
 * make sure master priority is higher than slave priority.
 */
#define MASTER_PRIORITY 6
#define SLAVE_PRIORITY  7

/* Master and Slave buffer size */
#define BUFF_SIZE  200

/* Master and Slave buffer word size */
#define SPI_WORD_SIZE 8

/* Master and Slave buffer frequency */
#define Mhz 1000000
#define SPI_FREQUENCY (1 * Mhz)

/* Master and Slave buffer transfers */
#define SPI_NUM_TRANSFERS  10

/*
 * Signaling semaphores: main gives start sems to kick off a transfer round;
 * threads give done sems when the round finishes. Both sets survive S2RAM in
 * retained RAM so threads resume from k_sem_take() without re-creation.
 */
static struct k_sem slave_start_sem;
static struct k_sem master_start_sem;
static struct k_sem slave_done_sem;
static struct k_sem master_done_sem;

/* Master and Slave buffers */
static uint32_t master_txdata[BUFF_SIZE];
static uint32_t master_rxdata[BUFF_SIZE];
static uint32_t slave_txdata[BUFF_SIZE];
static uint32_t slave_rxdata[BUFF_SIZE];

K_THREAD_STACK_DEFINE(MasterT_stack, STACKSIZE);
static struct k_thread MasterT_data;

K_THREAD_STACK_DEFINE(SlaveT_stack, STACKSIZE);
static struct k_thread SlaveT_data;

/*
 * Send/Receive data through slave spi
 */
int slave_spi_transceive(const struct device *dev)
{
	struct spi_config cnfg;
	int ret;

	cnfg.frequency = SPI_FREQUENCY;
	cnfg.operation = SPI_OP_MODE_SLAVE | SPI_WORD_SET(SPI_WORD_SIZE);
	cnfg.slave = 0;

	int length = (BUFF_SIZE) * sizeof(slave_rxdata[0]);

	struct spi_buf rx_buf = {
		.buf = slave_rxdata,
		.len = length
	};
	struct spi_buf_set rx_bufset = {
		.buffers = &rx_buf,
		.count = 1
	};
	struct spi_buf tx_buf = {
		.buf = slave_txdata,
		.len = length
	};
	struct spi_buf_set tx_bufset = {
		.buffers = &tx_buf,
		.count = 1
	};

	ret = spi_transceive(dev, &cnfg, &tx_bufset, &rx_bufset);
	if (ret < 0) {
		LOG_ERR("Slave SPI transceive error: %d", ret);
	}

	LOG_INF("slave wrote: %08x %08x %08x %08x %08x",
		slave_txdata[0], slave_txdata[1], slave_txdata[2],
		slave_txdata[3], slave_txdata[4]);

	LOG_INF("slave read:  %08x %08x %08x %08x %08x",
		slave_rxdata[0], slave_rxdata[1], slave_rxdata[2],
		slave_rxdata[3], slave_rxdata[4]);

	ret = memcmp(master_txdata, slave_rxdata, length);
	if (ret) {
		LOG_ERR("SPI master TX & slave RX data mismatch: %d", ret);
	} else {
		LOG_INF("SUCCESS: SPI Master TX & Slave RX DATA IS MATCHING: 0");
	}

	return ret;
}

/*
 * Send/Receive data through master spi
 */
int master_spi_transceive(const struct device *dev, struct spi_cs_control *cs)
{
	struct spi_config cnfg;
	int ret;

	cnfg.frequency = SPI_FREQUENCY;
	cnfg.operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(SPI_WORD_SIZE);
	cnfg.slave = 0;
	cnfg.cs = *cs;

	int length = (BUFF_SIZE) * sizeof(master_txdata[0]);

	struct spi_buf tx_buf = {
		.buf = master_txdata,
		.len = length
	};

	struct spi_buf_set tx_bufset = {
		.buffers = &tx_buf,
		.count = 1
	};

	struct spi_buf rx_buf = {
		.buf = master_rxdata,
		.len = length
	};
	struct spi_buf_set rx_bufset = {
		.buffers = &rx_buf,
		.count = 1
	};

	ret = spi_transceive(dev, &cnfg, &tx_bufset, &rx_bufset);
	if (ret) {
		LOG_ERR("SPI=%s transceive error: %d", dev->name, ret);
	}
	LOG_INF("Master wrote:   %08x %08x %08x %08x %08x",
		master_txdata[0], master_txdata[1], master_txdata[2],
		master_txdata[3], master_txdata[4]);
	LOG_INF("Master receive: %08x %08x %08x %08x %08x",
		master_rxdata[0], master_rxdata[1], master_rxdata[2],
		master_rxdata[3], master_rxdata[4]);

	ret = memcmp(master_rxdata, slave_txdata, length);
	if (ret) {
		LOG_ERR("SPI master RX & slave TX data mismatch: %d", ret);
	} else {
		LOG_INF("SUCCESS: SPI Master RX & Slave TX DATA IS MATCHING: 0");
	}

	return ret;
}

static void prepare_data(uint32_t *data, uint16_t def_mask)
{
	for (uint32_t cnt = 0; cnt < BUFF_SIZE; cnt++) {
		data[cnt] = (def_mask << 16) | cnt;
	}
}

/*
 * Master SPI thread. Blocks on master_start_sem between rounds.
 * After S2RAM wakeup, retained RAM keeps the thread context intact and it
 * resumes from k_sem_take() — no re-creation required.
 */
static void master_spi_thread(void *p1, void *p2, void *p3)
{
	const struct device *const dev = DEVICE_DT_GET(SPIDW_NODE);

	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	if (!device_is_ready(dev)) {
		LOG_ERR("%s: Master device not ready", dev->name);
		return;
	}

#if SPI_MASTER_SS_SW_CONTROLLED_GPIO
	struct spi_cs_control cs_ctrl = (struct spi_cs_control){
		.gpio  = GPIO_DT_SPEC_GET(SPIDW_NODE, cs_gpios),
		.delay = 100u,
	};
#else
	struct spi_cs_control cs_ctrl = {0};
#endif

	while (true) {
		k_sem_take(&master_start_sem, K_FOREVER);
		for (uint32_t i = 0; i < SPI_NUM_TRANSFERS; i++) {
			LOG_INF("Master Transceive Iter= %u", i + 1);
			if (master_spi_transceive(dev, &cs_ctrl) < 0) {
				LOG_ERR("Stopping the Master Thread due to error");
				break;
			}
			k_msleep(1000);
		}
		LOG_INF("Master Transfer Successfully Completed");
		k_sem_give(&master_done_sem);
	}
}

/*
 * Slave SPI thread. Blocks on slave_start_sem between rounds.
 * After S2RAM wakeup, retained RAM keeps the thread context intact and it
 * resumes from k_sem_take() — no re-creation required.
 */
static void slave_spi_thread(void *p1, void *p2, void *p3)
{
	const struct device *const slave_dev = DEVICE_DT_GET(S_SPIDW_NODE);

	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	if (!device_is_ready(slave_dev)) {
		LOG_ERR("%s: Slave device not ready", slave_dev->name);
		return;
	}

	while (true) {
		k_sem_take(&slave_start_sem, K_FOREVER);
		for (uint32_t i = 0; i < SPI_NUM_TRANSFERS; i++) {
			LOG_INF("Slave Transceive Iter= %u", i + 1);
			if (slave_spi_transceive(slave_dev) < 0) {
				LOG_ERR("Stopping the Slave Thread due to error");
				break;
			}
		}
		LOG_INF("Slave Transfer Successfully Completed");
		k_sem_give(&slave_done_sem);
	}
}

/*
 * Signal both SPI threads to run one round of transfers and wait for
 * completion. Slave is armed 100 ms before master to ensure the slave SPI
 * peripheral is ready to receive before the master starts the transaction.
 */
static void spi_loopback_run(const char *phase_label)
{
	LOG_INF("[SPI Demo] %s", phase_label);
	prepare_data(master_txdata, 0xA5A5);
	prepare_data(slave_txdata, 0x5A5A);
	k_sem_give(&slave_start_sem);
	k_msleep(100);
	k_sem_give(&master_start_sem);
	k_sem_take(&slave_done_sem, K_FOREVER);
	k_sem_take(&master_done_sem, K_FOREVER);
}

/* PM state name lookup for notifier logging */
static const char * const pm_state_names[] = {
	[PM_STATE_ACTIVE]          = "ACTIVE",
	[PM_STATE_RUNTIME_IDLE]    = "RUNTIME_IDLE",
	[PM_STATE_SUSPEND_TO_IDLE] = "SUSPEND_TO_IDLE",
	[PM_STATE_STANDBY]         = "STANDBY",
	[PM_STATE_SUSPEND_TO_RAM]  = "SUSPEND_TO_RAM",
	[PM_STATE_SUSPEND_TO_DISK] = "SUSPEND_TO_DISK",
	[PM_STATE_SOFT_OFF]        = "SOFT_OFF",
};

#define PM_STATE_STR(s) \
	((s) < ARRAY_SIZE(pm_state_names) ? pm_state_names[(s)] : "UNKNOWN")

static void pm_notify_entry(enum pm_state state)
{
	const struct pm_state_info *info = pm_state_next_get(0);

	LOG_INF("PM enter: %s (substate %u)", PM_STATE_STR(state), info->substate_id);
}

/*
 * With CONFIG_LOG_MODE_DEFERRED, LOG_INF here only writes to the ring buffer
 * — no UART access — so this is safe even before devices are resumed.
 */
static void pm_notify_pre_resume(enum pm_state state)
{
	const struct pm_state_info *info = pm_state_next_get(0);
	uint32_t active_exc = SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk;

	if (active_exc >= 16U) {
		LOG_INF("PM wakeup: %s (substate %u) IRQ %u",
			PM_STATE_STR(state), info->substate_id, active_exc - 16U);
	} else if (active_exc != 0U) {
		LOG_INF("PM wakeup: %s (substate %u) exception %u",
			PM_STATE_STR(state), info->substate_id, active_exc);
	} else {
		LOG_INF("PM wakeup: %s (substate %u)",
			PM_STATE_STR(state), info->substate_id);
	}
#if APP_PM_WAKEUP_DEBUG
	for (int i = 0; i < 16; i++) {
		if (NVIC->ISPR[i]) {
			LOG_INF("PM wakeup: NVIC ISPR[%d] = 0x%08x", i, NVIC->ISPR[i]);
		}
	}
#endif
}

static void pm_notify_exit(enum pm_state state)
{
	const struct pm_state_info *info = pm_state_next_get(0);

	LOG_INF("PM exit:  %s (substate %u)", PM_STATE_STR(state), info->substate_id);
}

static struct pm_notifier app_pm_notifier = {
	.state_entry       = pm_notify_entry,
	.pre_device_resume = pm_notify_pre_resume,
	.state_exit        = pm_notify_exit,
};

int main(void)
{
	const struct device *const cons = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
	const struct device *const wakeup_dev = DEVICE_DT_GET(WAKEUP_SOURCE);
	int ret;

	__ASSERT(device_is_ready(cons), "%s: device not ready", cons->name);
	__ASSERT(device_is_ready(wakeup_dev), "%s: device not ready", wakeup_dev->name);

	pm_notifier_register(&app_pm_notifier);

	if (S2RAM_SUPPORTED) {
		LOG_INF("%s (S2RAM): SPI DW PM demo "
			"(RUNTIME_IDLE, SUSPEND_TO_IDLE, S2RAM STANDBY, S2RAM STOP)",
			CONFIG_BOARD);
	} else {
		LOG_INF("%s (SOFT_OFF): SPI DW PM demo "
			"(RUNTIME_IDLE, SUSPEND_TO_IDLE, SOFT_OFF)",
			CONFIG_BOARD);
	}

	ret = counter_start(wakeup_dev);
	__ASSERT(!ret || ret == -EALREADY, "Failed to start counter (err %d)", ret);

	k_sem_init(&slave_start_sem, 0, 1);
	k_sem_init(&master_start_sem, 0, 1);
	k_sem_init(&slave_done_sem, 0, 1);
	k_sem_init(&master_done_sem, 0, 1);

	/*
	 * Create SPI threads once. They loop on their start semaphores so after
	 * S2RAM wakeup they resume from k_sem_take() without re-creation.
	 */
	k_thread_create(&SlaveT_data, SlaveT_stack, STACKSIZE,
			slave_spi_thread, NULL, NULL, NULL,
			SLAVE_PRIORITY, 0, K_NO_WAIT);
	k_thread_create(&MasterT_data, MasterT_stack, STACKSIZE,
			master_spi_thread, NULL, NULL, NULL,
			MASTER_PRIORITY, 0, K_NO_WAIT);

	LOG_INF("POWER STATE SEQUENCE:");
	LOG_INF("  1. PM_STATE_RUNTIME_IDLE");
	LOG_INF("  2. PM_STATE_SUSPEND_TO_IDLE");
	if (S2RAM_SUPPORTED) {
		LOG_INF("  3. PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY)");
		LOG_INF("  4. PM_STATE_SUSPEND_TO_RAM (substate 1: STOP)");
	} else {
		LOG_INF("  3. PM_STATE_SOFT_OFF");
	}

	/* Lock SUSPEND_TO_IDLE during SPI and RUNTIME_IDLE to prevent deeper sleep */
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
	spi_loopback_run("before RUNTIME_IDLE");

	LOG_INF("Enter RUNTIME_IDLE sleep for (%d microseconds)", RUNTIME_IDLE_SLEEP_USEC);
	ret = app_enter_normal_sleep(RUNTIME_IDLE_SLEEP_USEC);
	__ASSERT(ret == 0, "Could not enter RUNTIME_IDLE sleep (err %d)", ret);

	LOG_INF("Exited from RUNTIME_IDLE sleep");
	pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);

#if defined(CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER)
	LOG_INF("Enter PM_STATE_SUSPEND_TO_IDLE for (%d microseconds)",
		SUSPEND_IDLE_SLEEP_USEC);
	k_sleep(K_USEC(SUSPEND_IDLE_SLEEP_USEC));
	LOG_INF("Exited from PM_STATE_SUSPEND_TO_IDLE");
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
	spi_loopback_run("after SUSPEND_TO_IDLE");
	pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
#else
	/* Lock SUSPEND_TO_IDLE when LPM timer support is not configured */
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
	LOG_INF("PM_STATE_SUSPEND_TO_IDLE (skipped - LPM timer not enabled)");
#endif

	/* Unlock deeper power states to allow S2RAM and/or SOFT_OFF */
	app_pm_lock_deeper_states(false);

	if (S2RAM_SUPPORTED) {
		LOG_INF("Enter PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) for (%d microseconds)",
			S2RAM_STANDBY_SLEEP_USEC);
		ret = app_enter_deep_sleep(S2RAM_STANDBY_SLEEP_USEC);
		__ASSERT(ret == 0, "Could not enter PM_STATE_SUSPEND_TO_RAM (err %d)", ret);

		LOG_INF("=== Resumed from PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) ===");
		pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
		spi_loopback_run("after S2RAM STANDBY");
		pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);

		LOG_INF("Enter PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) for (%d microseconds)",
			S2RAM_STOP_SLEEP_USEC);
		ret = app_enter_deep_sleep(S2RAM_STOP_SLEEP_USEC);
		__ASSERT(ret == 0, "Could not enter PM_STATE_SUSPEND_TO_RAM (err %d)", ret);

		LOG_INF("=== Resumed from PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) ===");
		pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
		spi_loopback_run("after S2RAM STOP");
		pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
	}

	if (SOFT_OFF_SUPPORTED) {
		LOG_INF("Enter PM_STATE_SOFT_OFF for (%d microseconds)", SOFT_OFF_SLEEP_USEC);
		LOG_INF("Note: SOFT_OFF has no retention - system will reset on wakeup");
		ret = app_enter_deep_sleep(SOFT_OFF_SLEEP_USEC);
		__ASSERT(ret == 0, "Could not enter PM_STATE_SOFT_OFF (err %d)", ret);

		/* Should never reach here - SOFT_OFF causes full reset on wakeup */
		LOG_ERR("ERROR: Resumed after PM_STATE_SOFT_OFF - this should not happen!");
		__ASSERT(false, "PM_STATE_SOFT_OFF should have caused a reset");
	}

	LOG_INF("=== SPI DW PM SEQUENCE COMPLETED ===");

	app_pm_lock_deeper_states(true);
	/* Demo complete — prevent SUSPEND_TO_IDLE so the idle spin stays in RUNTIME_IDLE */
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);

	while (true) {
		/* spin here */
		k_sleep(K_SECONDS(1));
	}

	return 0;
}

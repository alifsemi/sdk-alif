/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https: //alifsemi.com/license
 *
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/pm/pm.h>
#include <zephyr/pm/policy.h>
#include <zephyr/drivers/counter.h>
#include <cmsis_core.h>
#include <soc.h>
#include <se_service.h>

/**
 * As per the application requirements, it can remove the memory blocks which are not in use.
 */
#if defined(CONFIG_SOC_SERIES_ENSEMBLE_E1C) || defined(CONFIG_SOC_SERIES_BALLETTO_B1)
	#define APP_RET_MEM_BLOCKS SRAM4_1_MASK | SRAM4_2_MASK | SRAM4_3_MASK | SRAM4_4_MASK | \
					SRAM5_1_MASK | SRAM5_2_MASK | SRAM5_3_MASK | SRAM5_4_MASK |\
					SRAM5_5_MASK
	#define SERAM_MEMORY_BLOCKS_IN_USE SERAM_1_MASK | SERAM_2_MASK | SERAM_3_MASK | SERAM_4_MASK
#else
	#define APP_RET_MEM_BLOCKS SRAM4_1_MASK | SRAM4_2_MASK | SRAM5_1_MASK | SRAM5_2_MASK
	#define SERAM_MEMORY_BLOCKS_IN_USE SERAM_MASK
#endif

#if DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(rtc0), snps_dw_apb_rtc, okay)
	#define WAKEUP_SOURCE DT_NODELABEL(rtc0)
	#define SE_OFFP_EWIC_CFG EWIC_RTC_A
	#define SE_OFFP_WAKEUP_EVENTS WE_LPRTC
#elif DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(timer0), snps_dw_timers, okay)
	#define WAKEUP_SOURCE DT_NODELABEL(timer0)
	#define SE_OFFP_EWIC_CFG EWIC_VBAT_TIMER
	#define SE_OFFP_WAKEUP_EVENTS WE_LPTIMER0
#else
#error "Wakeup Device not enabled in the dts"
#endif

#define WAKEUP_SOURCE_IRQ DT_IRQ_BY_IDX(WAKEUP_SOURCE, 0, irq)

#define SLEEP_IN_SEC (20)
#define SLEEP_IN_MICROSECS (SLEEP_IN_SEC * 1000 * 1000)

#define SOC_STANDBY_MODE_PD PD_SSE700_AON_MASK
#define SOC_STOP_MODE_PD PD_VBAT_AON_MASK

/**
 * By default STOP mode is requested.
 * For Standby, set the SOC_REQUESTED_POWER_MODE to SOC_STANDBY_MODE_PD
 */
#define SOC_REQUESTED_POWER_MODE SOC_STOP_MODE_PD

static uint32_t wakeup_reason;

static inline uint32_t get_wakeup_irq_status(void)
{
	return NVIC_GetPendingIRQ(WAKEUP_SOURCE_IRQ);
}

/*
 * This function will be invoked in the PRE_KERNEL_2 phase of the init routine.
 * We can read the wakeup reason from reading the RESET STATUS register
 * and from the pending IRQ.
 */
static int get_core_wakeup_reason(void)
{
	wakeup_reason = get_wakeup_irq_status();

	pm_policy_state_lock_get(PM_STATE_SOFT_OFF, PM_ALL_SUBSTATES);

	return 0;
}
SYS_INIT(get_core_wakeup_reason, PRE_KERNEL_2, 0);

/*
 * This function will be invoked in the POST_KERNEL phase of the init routine.
 * In this example we are using the UART as console and the banner string will
 * be pushed before the APPLICATION phase of the init routine.
 *
 * We have to make sure the SYSTOP is ON before UART is initialized.
 * Set the RUN profile parameters for this application accordingly.
 */
static int app_set_run_params(void)
{
	run_profile_t runp;
	int ret;

	ret = se_service_sync();
	if (ret) {
		printk("SE: not responding to service calls %d\n", ret);
		return 0;
	}

	ret = se_service_get_run_cfg(&runp);
	if (ret) {
		printk("SE: get_run_cfg failed = %d.\n", ret);
		return 0;
	}

	runp.power_domains = PD_SYST_MASK | PD_SSE700_AON_MASK;
	runp.dcdc_voltage  = 825;
	runp.dcdc_mode     = DCDC_MODE_PWM;
	runp.aon_clk_src   = CLK_SRC_LFXO;
	runp.run_clk_src   = CLK_SRC_PLL;
#if defined(CONFIG_RTSS_HP)
	runp.cpu_clk_freq  = CLOCK_FREQUENCY_400MHZ;
#else
	runp.cpu_clk_freq  = CLOCK_FREQUENCY_160MHZ;
#endif
	if (SCB->VTOR) {
		runp.memory_blocks |= MRAM_MASK;
	}

	ret = se_service_set_run_cfg(&runp);
	if (ret) {
		printk("SE: set_run_cfg failed = %d.\n", ret);
		return 0;
	}

	return 0;
}
SYS_INIT(app_set_run_params, POST_KERNEL, 50);

static void alarm_callback_fn(const struct device *wakeup_dev,
				      uint8_t chan_id, uint32_t ticks,
				      void *user_data)
{
	printk("%s: !!! Alarm !!!\n", wakeup_dev->name);
}

int main(void)
{
	const struct device *const cons = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
	const struct device *const wakeup_dev = DEVICE_DT_GET(WAKEUP_SOURCE);
	struct counter_alarm_cfg alarm_cfg;
	off_profile_t offp;
	uint32_t now_ticks;
	int ret;

	if (!device_is_ready(cons)) {
		printk("%s: device not ready.\n", cons->name);
		printk("ERROR: app exiting..\n");
		return 0;
	}

	if (!device_is_ready(wakeup_dev)) {
		printk("%s: device not ready.\n", wakeup_dev->name);
		printk("ERROR: app exiting..\n");
		return 0;
	}

	printk("\n%s System Off Demo\n", CONFIG_BOARD);

	if (wakeup_reason) {
		printk("\r\nWakeup Interrupt Reason : %s\n", wakeup_dev->name);
	}

	ret = se_service_get_off_cfg(&offp);
	if (ret) {
		printk("SE: get_off_cfg failed = %d.\n", ret);
		printk("ERROR: Can't establish SE connection, app exiting..\n");
		return 0;
	}

	offp.power_domains = SOC_REQUESTED_POWER_MODE;
	offp.aon_clk_src   = CLK_SRC_LFXO;
	offp.stby_clk_src  = CLK_SRC_HFXO;
	offp.ewic_cfg      = SE_OFFP_EWIC_CFG;
	offp.wakeup_events = SE_OFFP_WAKEUP_EVENTS;
	offp.vtor_address  = SCB->VTOR;
	offp.memory_blocks = MRAM_MASK;

#if defined(CONFIG_RTSS_HE)
	/*
	 * Enable the HE TCM retention only if the VTOR is present.
	 * This is just for this test application.
	 */
	if (!SCB->VTOR) {
		offp.memory_blocks = APP_RET_MEM_BLOCKS | SERAM_MEMORY_BLOCKS_IN_USE;
	} else {
		offp.memory_blocks |= SERAM_MEMORY_BLOCKS_IN_USE;
	}
#else
	/*
	 * Retention is not possible with HP-TCM
	 */
	if (SCB->VTOR) {
		printf("\r\nHP TCM Retention is not possible\n");
		printk("ERROR: VTOR is set to TCM, app exiting..\n");
		return 0;
	}

	offp.memory_blocks = MRAM_MASK;
#endif

	printk("SE: VTOR = %x\n", offp.vtor_address);
	printk("SE: MEMBLOCKS = %x\n", offp.memory_blocks);

	ret = se_service_set_off_cfg(&offp);
	if (ret) {
		printk("SE: set_off_cfg failed = %d.\n", ret);
		printk("ERROR: Can't establish SE connection, app exiting..\n");
		return 0;
	}

	ret = counter_start(wakeup_dev);
	if (ret) {
		printk("Failed to start counter (err %d)", ret);
		printk("ERROR: app exiting..\n");
		return 0;
	}

	alarm_cfg.flags = 0;
	alarm_cfg.ticks = counter_us_to_ticks(wakeup_dev, SLEEP_IN_MICROSECS);
	alarm_cfg.callback = alarm_callback_fn;
	alarm_cfg.user_data = &alarm_cfg;

	printk("Set Alarm and enter Normal Sleep\n");

	ret = counter_set_channel_alarm(wakeup_dev, 0,
					&alarm_cfg);
	if (ret) {
		printk("Couldnt set the alarm\n");
		printk("ERROR: app exiting..\n");
		return 0;
	}
	printk("Set alarm in %u sec (%u ticks)\n",
	       SLEEP_IN_SEC,
	       alarm_cfg.ticks);

	k_sleep(K_SECONDS(SLEEP_IN_SEC + 1));

	printk("Set Alarm and enter Subsystem OFF & then STANDBY/STOP mode\n");

	/*
	 * Set the alarm and delay so that idle thread can run
	 */
#if DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(rtc0), snps_dw_apb_rtc, okay)
	ret = counter_get_value(wakeup_dev, &now_ticks);
	if (ret) {
		printk("Failed to read counter value (err %d)", ret);
		printk("ERROR: app exiting..\n");
		return 0;
	}
#else
	now_ticks = 0;
#endif
	alarm_cfg.ticks = now_ticks + counter_us_to_ticks(wakeup_dev, SLEEP_IN_MICROSECS);
	ret = counter_set_channel_alarm(wakeup_dev, 0,
					&alarm_cfg);
	if (ret) {
		printk("Failed to set the alarm (err %d)", ret);
		printk("ERROR: app exiting..\n");
		return 0;
	}

	printk("Set alarm in %u sec (%u ticks)\n",
	       SLEEP_IN_SEC,
	       alarm_cfg.ticks);

	if (ret) {
		printk("Couldnt set the alarm\n");
		printk("ERROR: app exiting..\n");
		return 0;
	}

	pm_policy_state_lock_put(PM_STATE_SOFT_OFF, PM_ALL_SUBSTATES);

	/*
	 * Force Subsytem OFF on any delay.
	 */
	pm_state_force(0u, &(struct pm_state_info){PM_STATE_SOFT_OFF, 0, 0});

	k_sleep(K_SECONDS(1));

	printk("ERROR: Failed to enter Subsystem OFF\n");
	while (true) {
		/* spin here */
		k_sleep(K_SECONDS(1));
	}

	return 0;
}

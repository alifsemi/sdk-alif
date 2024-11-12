/*
 * Copyright (c) 2024 Alif Semiconductor
 * SPDX-License-Identifier: Apache-2.0
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

#define RTC DT_NODELABEL(rtc0)
#define RTC_SLEEP_S (20)
#define RTC_SLEEP_US (RTC_SLEEP_S * 1000 * 1000)

#define LPRTC_IRQ_IRQn 58

static bool is_rtc_wakeup;

/*
 * This function will be invoked in the PRE_KERNEL_2 phase of the init routine.
 * We can read the wakeup reason from reading the RESET STATUS register
 * and from the pending IRQ.
 */
static int get_core_wakeup_reason(void)
{
	if (NVIC_GetPendingIRQ(LPRTC_IRQ_IRQn)) {
		is_rtc_wakeup = true;
	}

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
	runp.memory_blocks = SRAM2_MASK | SRAM3_MASK | MRAM_MASK;
#else
	runp.cpu_clk_freq  = CLOCK_FREQUENCY_160MHZ;
	runp.memory_blocks = SRAM4_1_MASK | SRAM4_2_MASK
				| SRAM5_1_MASK | SRAM5_2_MASK;
	if (SCB->VTOR) {
		runp.memory_blocks |= MRAM_MASK;
	}
#endif

	ret = se_service_set_run_cfg(&runp);
	if (ret) {
		printk("SE: set_run_cfg failed = %d.\n", ret);
		return 0;
	}

	return 0;
}
SYS_INIT(app_set_run_params, POST_KERNEL, 50);

static void alarm_callback_fn(const struct device *rtc_dev,
				      uint8_t chan_id, uint32_t ticks,
				      void *user_data)
{
	uint32_t now_ticks;
	int ret;

	ret = counter_get_value(rtc_dev, &now_ticks);
	if (ret) {
		printk("Failed to read counter value (err %d)", ret);
		return;
	}
	printk("!!! Alarm !!! at %u ticks\n", now_ticks);
}

int main(void)
{
	const struct device *const cons = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
	const struct device *const rtc_dev = DEVICE_DT_GET(RTC);
	struct counter_alarm_cfg alarm_cfg;
	off_profile_t offp;
	uint32_t now_ticks;
	int ret;

	if (!device_is_ready(cons)) {
		printk("%s: device not ready.\n", cons->name);
		printk("ERROR: app exiting..\n");
		return 0;
	}

	if (!device_is_ready(rtc_dev)) {
		printk("%s: device not ready.\n", rtc_dev->name);
		printk("ERROR: app exiting..\n");
		return 0;
	}

		printk("\n%s System Off Demo\n", CONFIG_BOARD);

	if (is_rtc_wakeup) {
		printk("\r\nWakeup Interrupt Reason : RTC\n");
	}

	ret = se_service_get_off_cfg(&offp);
	if (ret) {
		printk("SE: get_off_cfg failed = %d.\n", ret);
		printk("ERROR: Can't establish SE connection, app exiting..\n");
		return 0;
	}

	offp.power_domains = PD_VBAT_AON_MASK;
	offp.aon_clk_src   = CLK_SRC_LFXO;
	offp.stby_clk_src  = CLK_SRC_HFXO;
	offp.ewic_cfg      = EWIC_RTC_A;
	offp.wakeup_events = WE_LPRTC;
	offp.vtor_address  = SCB->VTOR;
	offp.memory_blocks = MRAM_MASK;

#if defined(CONFIG_RTSS_HE)
	/*
	 * Enable the HE TCM retention only if the VTOR is present.
	 * This is just for this test application.
	 */
	if (!SCB->VTOR) {
		offp.memory_blocks = SRAM4_1_MASK | SRAM4_2_MASK
					| SRAM5_1_MASK | SRAM5_2_MASK | SERAM_MASK;
	} else {
		offp.memory_blocks |= SERAM_MASK;
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

	ret = counter_start(rtc_dev);
	if (ret) {
		printk("Failed to start counter (err %d)", ret);
		printk("ERROR: app exiting..\n");
		return 0;
	}

	ret = counter_get_value(rtc_dev, &now_ticks);
	if (ret) {
		printk("Failed to read counter value (err %d)", ret);
		printk("ERROR: app exiting..\n");
		return 0;
	}
	alarm_cfg.flags = 0;
	alarm_cfg.ticks = counter_us_to_ticks(rtc_dev, RTC_SLEEP_US);
	alarm_cfg.callback = alarm_callback_fn;
	alarm_cfg.user_data = &alarm_cfg;

	printk("Set Alarm and enter Normal Sleep\n");

	ret = counter_set_channel_alarm(rtc_dev, 0,
					&alarm_cfg);
	if (ret) {
		printk("Couldnt set the alarm\n");
		printk("ERROR: app exiting..\n");
		return 0;
	}
	printk("Set alarm in %u sec (%u ticks)\n",
	       RTC_SLEEP_S,
	       alarm_cfg.ticks);

	k_sleep(K_SECONDS(RTC_SLEEP_S + 1));

	printk("Set Alarm and enter Subsystem OFF & then STOP mode\n");

	/*
	 * Set the alarm and delay so that idle thread can run
	 */
	ret = counter_get_value(rtc_dev, &now_ticks);
	if (ret) {
		printk("Failed to read counter value (err %d)", ret);
		printk("ERROR: app exiting..\n");
		return 0;
	}
	alarm_cfg.ticks = now_ticks + counter_us_to_ticks(rtc_dev, RTC_SLEEP_US);
	ret = counter_set_channel_alarm(rtc_dev, 0,
					&alarm_cfg);
	if (ret) {
		printk("Failed to set the alarm (err %d)", ret);
		printk("ERROR: app exiting..\n");
		return 0;
	}

	printk("Set alarm in %u sec (%u ticks)\n",
	       RTC_SLEEP_S,
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

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
 #include <zephyr/drivers/adc.h>
 #include <zephyr/drivers/counter.h>
 #include <zephyr/sys/util.h>
 #include <zephyr/logging/log.h>
 #include "temperature.h"

 LOG_MODULE_REGISTER(pm_adc, LOG_LEVEL_INF);

 /* Set to 1 to dump full NVIC ISPR state on wakeup */
 #define APP_PM_WAKEUP_DEBUG 0

 #define ADC_DEV_NODE DT_NODELABEL(adc0)

 #define ADC_CHANNEL_6			(0x06)
 #define ADC_UNMASK_CHANNEL_6		(1 << 6)

 /* Number of ADC conversions per PM phase */
 #define ADC_NUM_READS 3

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
  * Upper bound: at 400 MHz, ticks-to-cycles calculation in the PM driver
  * overflows uint32 max for values > 10.7s (2^32 / 400). Deep-sleep durations
  * (S2RAM, SOFT_OFF) and overlay min-residency-us values must stay below this
  * ceiling. RUNTIME_IDLE may exceed it because deeper states are locked out
  * during that sleep (see app_pm_lock_sleep_states()).
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
 #if DT_NODE_EXISTS(DT_NODELABEL(sram0)) && DT_HAS_CHOSEN(zephyr_sram)
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
		 pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_RAM, PM_ALL_SUBSTATES);
		 pm_policy_state_lock_get(PM_STATE_SOFT_OFF,       PM_ALL_SUBSTATES);
	 } else {
		 pm_policy_state_lock_put(PM_STATE_SOFT_OFF,       PM_ALL_SUBSTATES);
		 pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_RAM, PM_ALL_SUBSTATES);
	 }
 }

 /*
  * Lock every PM sleep state deeper than RUNTIME_IDLE. Intended for use around
  * ADC conversions and around any sleep call the app makes where an accidental
  * upgrade to a deeper state must be prevented.
  *
  * ADC IRQs sit outside the IWIC wakeup range, so SUSPEND_TO_IDLE must not be
  * entered while a conversion is in progress.
  */
 static void app_pm_lock_sleep_states(bool lock)
 {
	 if (lock) {
		 pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
		 pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_RAM,  PM_ALL_SUBSTATES);
		 pm_policy_state_lock_get(PM_STATE_SOFT_OFF,        PM_ALL_SUBSTATES);
	 } else {
		 pm_policy_state_lock_put(PM_STATE_SOFT_OFF,        PM_ALL_SUBSTATES);
		 pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_RAM,  PM_ALL_SUBSTATES);
		 pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
	 }
 }

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
	 struct counter_alarm_cfg alarm_cfg = {0};
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
	 struct counter_alarm_cfg alarm_cfg = {0};
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

 static uint32_t adc_buffer[1];
 static uint32_t m_samplings_done;
 static uint8_t comparator;

 enum adc_action adc_call_back(const struct device *dev,
				 const struct adc_sequence *sequence,
				 uint16_t sampling_index)
 {
	 ARG_UNUSED(dev);
	 ARG_UNUSED(sequence);
	 ARG_UNUSED(sampling_index);

	 ++m_samplings_done;

	 if (m_samplings_done < 2) {
		 return ADC_ACTION_REPEAT;
	 }

	 return ADC_ACTION_FINISH;
 }

 /*
  * Re-setup the TSENS channel and take one conversion. Channel setup is
  * repeated after each resume because adc_hw_init() rebuilds analog state.
  */
 static int adc_read_once(const struct device *adc_dev)
 {
	 struct adc_sequence_options adc_seq_options = {
		 .callback = adc_call_back,
		 .user_data = &comparator,
	 };
	 struct adc_channel_cfg channel_cfg = {
		 .differential = 0,
		 .channel_id = ADC_CHANNEL_6,
	 };
	 struct adc_sequence sequence = {
		 .options = &adc_seq_options,
		 .buffer = adc_buffer,
		 .buffer_size = sizeof(adc_buffer),
		 .channels = ADC_UNMASK_CHANNEL_6,
	 };
	 float temp;
	 int ret;

	 m_samplings_done = 0;

	 ret = adc_channel_setup(adc_dev, &channel_cfg);
	 if (ret) {
		 LOG_ERR("Unable to set up ADC channel %u (err %d)",
			 channel_cfg.channel_id, ret);
		 return ret;
	 }

	 ret = adc_read(adc_dev, &sequence);
	 if (ret) {
		 LOG_ERR("ADC read failed (err %d)", ret);
		 return ret;
	 }

	 temp = get_temperature(adc_buffer[0]);
	 if (temp == -1) {
		 LOG_INF("ADC raw=0x%X (temperature outside range)", adc_buffer[0]);
	 } else {
		 LOG_INF("ADC raw=0x%X temp=%0.1f C", adc_buffer[0], (double)temp);
	 }

	 return 0;
 }

 /*
  * Run a short ADC conversion burst around one PM phase. Sleep states deeper
  * than RUNTIME_IDLE are locked while ADC is active so ADC IRQs outside the
  * IWIC wakeup range cannot be lost in SUSPEND_TO_IDLE.
  */
 static void adc_demo_run(const struct device *adc_dev, const char *phase_label)
 {
	 bool success = true;

	 app_pm_lock_sleep_states(true);
	 LOG_INF("[ADC Demo] %s", phase_label);

	 for (uint32_t i = 0; i < ADC_NUM_READS; i++) {
		 LOG_INF("ADC read iter= %u", i + 1);
		 if (adc_read_once(adc_dev) < 0) {
			 LOG_ERR("Stopping ADC conversions due to error");
			 success = false;
			 break;
		 }
		 k_msleep(100);
	 }

	 if (success) {
		 LOG_INF("ADC conversion Successfully Completed");
	 }

	 app_pm_lock_sleep_states(false);
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
	 uint8_t substate_id = info ? info->substate_id : 0U;

	 LOG_INF("PM enter: %s (substate %u)", PM_STATE_STR(state), substate_id);
 }

 /*
  * With CONFIG_LOG_MODE_DEFERRED, LOG_INF here only writes to the ring buffer
  * — no UART access — so this is safe even before devices are resumed.
  */
 static void pm_notify_pre_resume(enum pm_state state)
 {
	 const struct pm_state_info *info = pm_state_next_get(0);
	 uint8_t substate_id = info ? info->substate_id : 0U;
	 uint32_t active_exc = SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk;

	 if (active_exc >= 16U) {
		 LOG_INF("PM wakeup: %s (substate %u) IRQ %u",
			 PM_STATE_STR(state), substate_id, active_exc - 16U);
	 } else if (active_exc != 0U) {
		 LOG_INF("PM wakeup: %s (substate %u) exception %u",
			 PM_STATE_STR(state), substate_id, active_exc);
	 } else {
		 LOG_INF("PM wakeup: %s (substate %u)",
			 PM_STATE_STR(state), substate_id);
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
	 uint8_t substate_id = info ? info->substate_id : 0U;

	 LOG_INF("PM exit:  %s (substate %u)", PM_STATE_STR(state), substate_id);
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
	 const struct device *const adc_dev = DEVICE_DT_GET(ADC_DEV_NODE);
	 int ret;

	 __ASSERT(device_is_ready(cons), "%s: device not ready", cons->name);
	 __ASSERT(device_is_ready(wakeup_dev), "%s: device not ready", wakeup_dev->name);
	 __ASSERT(device_is_ready(adc_dev), "%s: device not ready", adc_dev->name);

	 pm_notifier_register(&app_pm_notifier);

	 if (S2RAM_SUPPORTED) {
		 LOG_INF("%s (S2RAM): ADC PM demo "
			 "(RUNTIME_IDLE, SUSPEND_TO_IDLE, S2RAM STANDBY, S2RAM STOP)",
			 CONFIG_BOARD);
	 } else {
		 LOG_INF("%s (SOFT_OFF): ADC PM demo "
			 "(RUNTIME_IDLE, SUSPEND_TO_IDLE, SOFT_OFF)",
			 CONFIG_BOARD);
	 }

	 ret = counter_start(wakeup_dev);
	 __ASSERT(!ret || ret == -EALREADY, "Failed to start counter (err %d)", ret);

	 LOG_INF("POWER STATE SEQUENCE:");
	 LOG_INF("  1. PM_STATE_RUNTIME_IDLE");
	 LOG_INF("  2. PM_STATE_SUSPEND_TO_IDLE");
	 if (S2RAM_SUPPORTED) {
		 LOG_INF("  3. PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY)");
		 LOG_INF("  4. PM_STATE_SUSPEND_TO_RAM (substate 1: STOP)");
	 } else {
		 LOG_INF("  3. PM_STATE_SOFT_OFF");
	 }

	 adc_demo_run(adc_dev, "before RUNTIME_IDLE");

	 LOG_INF("Enter RUNTIME_IDLE sleep for (%d microseconds)", RUNTIME_IDLE_SLEEP_USEC);
	 app_pm_lock_sleep_states(true);
	 ret = app_enter_normal_sleep(RUNTIME_IDLE_SLEEP_USEC);
	 app_pm_lock_sleep_states(false);
	 __ASSERT(ret == 0, "Could not enter RUNTIME_IDLE sleep (err %d)", ret);

	 LOG_INF("Exited from RUNTIME_IDLE sleep");

 #if defined(CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER)
	 LOG_INF("Request SUSPEND_TO_IDLE for %d us", SUSPEND_IDLE_SLEEP_USEC);
	 app_pm_lock_deeper_states(true);
	 k_sleep(K_USEC(SUSPEND_IDLE_SLEEP_USEC));
	 app_pm_lock_deeper_states(false);
	 adc_demo_run(adc_dev, "after SUSPEND_TO_IDLE");
 #else
	 /* Lock SUSPEND_TO_IDLE when LPM timer support is not configured */
	 pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
	 LOG_INF("PM_STATE_SUSPEND_TO_IDLE (skipped - LPM timer not enabled)");
 #endif

	 if (S2RAM_SUPPORTED) {
		 LOG_INF("Request S2RAM STANDBY for %d us", S2RAM_STANDBY_SLEEP_USEC);
		 ret = app_enter_deep_sleep(S2RAM_STANDBY_SLEEP_USEC);
		 __ASSERT(ret == 0, "Could not enter PM_STATE_SUSPEND_TO_RAM (err %d)", ret);

		 adc_demo_run(adc_dev, "after S2RAM STANDBY");

		 LOG_INF("Request S2RAM STOP for %d us", S2RAM_STOP_SLEEP_USEC);
		 ret = app_enter_deep_sleep(S2RAM_STOP_SLEEP_USEC);
		 __ASSERT(ret == 0, "Could not enter PM_STATE_SUSPEND_TO_RAM (err %d)", ret);

		 adc_demo_run(adc_dev, "after S2RAM STOP");
	 }

	 if (SOFT_OFF_SUPPORTED) {
		 LOG_INF("Request SOFT_OFF for %d us (no retention - system resets on wakeup)",
			 SOFT_OFF_SLEEP_USEC);
		 ret = app_enter_deep_sleep(SOFT_OFF_SLEEP_USEC);
		 __ASSERT(ret == 0, "Could not enter PM_STATE_SOFT_OFF (err %d)", ret);

		 /* Should never reach here - SOFT_OFF causes full reset on wakeup */
		 LOG_ERR("ERROR: Resumed after PM_STATE_SOFT_OFF - this should not happen!");
		 __ASSERT(false, "PM_STATE_SOFT_OFF should have caused a reset");
	 }

	 LOG_INF("=== ADC PM SEQUENCE COMPLETED ===");

	 app_pm_lock_deeper_states(true);
	 /* Demo complete — prevent SUSPEND_TO_IDLE so the idle spin stays in RUNTIME_IDLE */
	 pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);

	 while (true) {
		 /* spin here */
		 k_sleep(K_SECONDS(1));
	 }

	 return 0;
 }

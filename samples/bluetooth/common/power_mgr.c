/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/drivers/counter.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/pm/pm.h>
#include <zephyr/pm/policy.h>
#include <zephyr/sys/poweroff.h>
#include <zephyr/sys/__assert.h>
#include <se_service.h>
#include <soc_common.h>
#include "power_mgr.h"

#if !DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(rtc0), snps_dw_apb_rtc, okay)
#error "RTC device not enabled in the dts. It is mandatory for wakeup from low power modes."
#endif

#if !DT_NODE_HAS_STATUS_OKAY(DT_NODELABEL(stop_s2ram))
#error "stop_s2ram node not enabled in the dts. It is mandatory for low power mode."
#endif

LOG_MODULE_REGISTER(common_ble_power_mgr, LOG_LEVEL_INF);

/**
 * SRAM4 == SRAM0
 * SRAM5 == SRAM1
 *
 * See HWRM Figure 9-1 M55-HE TCM Retention
 *
 * MB_SRAM4_1 : M55-HE ITCM RET1 itcm 64kb
 * MB_SRAM4_2 : M55-HE ITCM RET2 itcm 64kb
 * MB_SRAM4_3 : M55-HE ITCM RET3 itcm 128kb
 * MB_SRAM4_4 : M55-HE ITCM RET4 itcm 256kb
 * MB_SRAM5_1 : M55-HE DTCM RET1 dtcm 64kb
 * MB_SRAM5_2 : M55-HE DTCM RET2 dtcm 64kb
 * MB_SRAM5_3 : M55-HE DTCM RET3 dtcm 128kb
 * MB_SRAM5_4 : M55-HE DTCM RET4 dtcm 256kb
 * MB_SRAM5_5 : M55-HE DTCM RET5 dtcm 1024kb
 */

#define RET_A1 SRAM4_1_MASK
#define RET_A2 SRAM5_1_MASK
#define RET_B  (SRAM4_2_MASK | SRAM5_2_MASK)
#define RET_C  (SRAM4_3_MASK | SRAM5_3_MASK)
#define RET_D  (SRAM4_4_MASK | SRAM5_4_MASK)
#define RET_E  (SRAM5_5_MASK)

#define MEMORY_RETENTION_CONFIG                                                                    \
	(((CONFIG_FLASH_BASE_ADDRESS == 0) ? 0 : MRAM_MASK) |                                      \
	 (RET_A1 | RET_A2 | RET_B | RET_C | RET_D | RET_E) | SERAM_MASK)

#define DEFAULT_DCDC_VOLTAGE 775

enum off_state {
	OFF_STATE_IDLE = 0,
	OFF_STATE_STANDBY,
	OFF_STATE_STOP,
	OFF_STATE_OFF,
};

static int se_service_status;

void power_mgr_disable_sleep(void)
{
	pm_policy_state_lock_get(PM_STATE_SOFT_OFF, PM_ALL_SUBSTATES);
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_RAM, PM_ALL_SUBSTATES);
}

void power_mgr_allow_sleep(void)
{
	pm_policy_state_lock_put(PM_STATE_SOFT_OFF, PM_ALL_SUBSTATES);
	pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_RAM, PM_ALL_SUBSTATES);
}

static int set_off_profile(enum off_state const mode)
{
	int ret;
	off_profile_t offp = {
		.power_domains = PD_VBAT_AON_MASK,
		.memory_blocks = MEMORY_RETENTION_CONFIG,
		.dcdc_voltage = DEFAULT_DCDC_VOLTAGE,
		.dcdc_mode = DCDC_MODE_OFF,
		.stby_clk_freq = SCALED_FREQ_RC_STDBY_0_075_MHZ,
		.aon_clk_src = CLK_SRC_LFXO,
		.stby_clk_src = CLK_SRC_HFRC,
		.ip_clock_gating = 0,
		.phy_pwr_gating = 0,
		.ewic_cfg = EWIC_RTC_A,
		.wakeup_events = WE_LPRTC,
		.vtor_address = SCB->VTOR,
		.vtor_address_ns = SCB->VTOR,
	};

	switch (mode) {
	case OFF_STATE_IDLE:
		return 0;
	case OFF_STATE_STANDBY:
		offp.power_domains = PD_SSE700_AON_MASK;
		offp.stby_clk_freq = SCALED_FREQ_RC_STDBY_76_8_MHZ;
		break;
	case OFF_STATE_STOP:
		break;
	case OFF_STATE_OFF:
		break;
	}

	ret = se_service_set_off_cfg(&offp);
	if (ret) {
		LOG_ERR("SE: set_off_cfg failed = %d", ret);
	}

	return ret;
}

static int set_run_params(void)
{
	run_profile_t runp = {
		.power_domains =
			(PD_VBAT_AON_MASK | PD_SYST_MASK | PD_SSE700_AON_MASK | PD_SESS_MASK
#if CONFIG_DEBUG
			 | PD_DBSS_MASK
#endif
			 ),
		.dcdc_voltage = DEFAULT_DCDC_VOLTAGE,
		.dcdc_mode = DCDC_MODE_PFM_FORCED,
		.aon_clk_src = CLK_SRC_LFXO,
		.run_clk_src = CLK_SRC_PLL,
		.cpu_clk_freq = CLOCK_FREQUENCY_160MHZ,
		.phy_pwr_gating = LDO_PHY_MASK,
		.ip_clock_gating = LP_PERIPH_MASK,
		.vdd_ioflex_3V3 = IOFLEX_LEVEL_1V8,
		.scaled_clk_freq = SCALED_FREQ_XO_HIGH_DIV_38_4_MHZ,
		.memory_blocks = MEMORY_RETENTION_CONFIG,
	};

	se_service_status = se_service_set_run_cfg(&runp);

	if (se_service_status) {
		return -ENOEXEC;
	}

	return 0;
}

static int pre_configure_profiles(void)
{
	return set_run_params();
}

SYS_INIT(pre_configure_profiles, PRE_KERNEL_1, 3);

/**
 * PM Notifier callback called BEFORE devices are resumed
 *
 * This restores SE run configuration when resuming from S2RAM states.
 * Note: For SOFT_OFF, the system resets completely and set_run_params()
 * runs during normal PRE_KERNEL_1 initialization, so this callback is not needed.
 */
static void pm_notify_pre_device_resume(const enum pm_state state)
{
	switch (state) {
	case PM_STATE_SUSPEND_TO_IDLE:
		/* No action needed */
		break;
	case PM_STATE_SOFT_OFF:
	case PM_STATE_SUSPEND_TO_RAM: {
		set_run_params();
		break;
	}
	default: {
		__ASSERT(false, "Pre-resume for unknown power state %d", state);
		LOG_ERR("Pre-resume for unknown power state %d", state);
		break;
	}
	}
}

static struct pm_notifier notifier = {
	.pre_device_resume = pm_notify_pre_device_resume,
};

/*
 * This function will be invoked in the PRE_KERNEL_1 phase of the init
 * routine to prevent sleep during startup.
 */
static int app_pre_kernel_init(void)
{
	pm_notifier_register(&notifier);

#if PREKERNEL_DISABLE_SLEEP
	power_mgr_disable_sleep();
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
#endif

	return 0;
}
SYS_INIT(app_pre_kernel_init, PRE_KERNEL_1, 39);

/*
 * Prepare configuration at application initialization
 */
static int prepare_application_config(void)
{
	if (se_service_status) {
		LOG_ERR("SE service failed to initialize earlier. Error: %d", se_service_status);
		return se_service_status;
	}

	const struct device *const wakeup_dev = DEVICE_DT_GET(DT_NODELABEL(rtc0));

	if (!device_is_ready(wakeup_dev)) {
		LOG_ERR("device '%s' not ready", wakeup_dev->name);
		return -1;
	}

	int ret = counter_start(wakeup_dev);

	if (ret && ret != -EALREADY) {
		LOG_ERR("Counter '%s' start failed!", wakeup_dev->name);
		return -1;
	}

	ret = set_off_profile(OFF_STATE_STOP);

	pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);

	return ret;
}
SYS_INIT(prepare_application_config, APPLICATION, 0);

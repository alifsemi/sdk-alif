/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <zephyr/device.h>
#include <zephyr/pm/pm.h>
#include <zephyr/pm/policy.h>
#include <zephyr/drivers/counter.h>
#include <zephyr/drivers/pinctrl.h>
#include <zephyr/dt-bindings/pinctrl/balletto-pinctrl.h>
#include <se_service.h>
#include <stdlib.h>
#include <inttypes.h>
#include <cmsis_core.h>
#include <soc.h>
#include <pm_rtss.h>
#include <power_mgr.h>

#define LOG_MODULE_NAME alif_power_mgr_lib
#define LOG_LEVEL       LOG_LEVEL_INFO

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(LOG_MODULE_NAME);


#if DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(rtc0), snps_dw_apb_rtc, okay)
#define WAKEUP_SOURCE DT_NODELABEL(rtc0)
#define WAKEUP_SOURCE_IRQ DT_IRQ_BY_IDX(WAKEUP_SOURCE, 0, irq)
#define WAKEUP_EVENT WE_LPRTC
#define WAKEUP_EWIC_CFG EWIC_RTC_A
#elif DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(timer0), snps_dw_timer, okay)
#define WAKEUP_SOURCE DT_NODELABEL(timer0)
#define WAKEUP_SOURCE_IRQ DT_IRQ_BY_IDX(WAKEUP_SOURCE, 0, irq)
#define WAKEUP_EVENT WE_LPRTC
#define WAKEUP_EWIC_CFG EWIC_RTC_A
#else
#error "RTC0 or Timer 0 not available"
#endif

static uint32_t wakeup_reason;
static bool cold_boot;

#define VBAT_RESUME_ENABLED 0xcafecafe

uint32_t vbat_resume __attribute__((noinit));

static void balletto_vbat_resume_enable(void)
{
	vbat_resume = VBAT_RESUME_ENABLED;
}

static bool balletto_vbat_resume_enabled(void)
{
	if (vbat_resume == VBAT_RESUME_ENABLED) {
		return true;
	}
	return false;
}

/**
 * Use the HFOSC clock for the UART console
 */
#if DT_SAME_NODE(DT_NODELABEL(uart4), DT_CHOSEN(zephyr_console))
#define CONSOLE_UART_NUM 4
#define EARLY_BOOT_CONSOLE_INIT 1
#elif DT_SAME_NODE(DT_NODELABEL(uart2), DT_CHOSEN(zephyr_console))
#define CONSOLE_UART_NUM 2
#define EARLY_BOOT_CONSOLE_INIT 1
#elif DT_SAME_NODE(DT_NODELABEL(uart1), DT_CHOSEN(zephyr_console))
#define CONSOLE_UART_NUM 1
#define EARLY_BOOT_CONSOLE_INIT 1
#else
#error "Specify the uart console number"
#define EARLY_BOOT_CONSOLE_INIT 0
#endif

#if EARLY_BOOT_CONSOLE_INIT
#define UART_CTRL_CLK_SEL_POS 8
static int app_pre_console_init(void)
{
	/* Enable HFOSC in CGU */
	sys_set_bits(CGU_CLK_ENA, BIT(23));

	/* Enable HFOSC for the UART console */
	sys_clear_bits(EXPSLV_UART_CTRL, BIT((CONSOLE_UART_NUM + UART_CTRL_CLK_SEL_POS)));
	return 0;
}
SYS_INIT(app_pre_console_init, PRE_KERNEL_1, 50);
#endif

static inline uint32_t get_wakeup_irq_status(void)
{
	return NVIC_GetPendingIRQ(WAKEUP_SOURCE_IRQ);
}

/*
 * This function will be invoked in the PRE_KERNEL_1 phase of the init
 * routine to prevent sleep during startup.
 */
static int app_pre_kernel_init(void)
{
	wakeup_reason = get_wakeup_irq_status();
	pm_policy_state_lock_get(PM_STATE_SOFT_OFF, PM_ALL_SUBSTATES);

	return 0;
}
SYS_INIT(app_pre_kernel_init, PRE_KERNEL_1, 39);

static int pm_application_init(void)
{
	if (!balletto_vbat_resume_enabled()) {
		/* Mark a cold boot */
		cold_boot = true;
	}

	return 0;
}
SYS_INIT(pm_application_init, PRE_KERNEL_1, 3); /*CONFIG_SE_SERVICE_INIT_PRIORITY + 3 */

bool power_mgr_cold_boot(void)
{
	return cold_boot;
}

uint32_t power_mgr_get_wakeup_reason(void)
{
	return wakeup_reason;
}

int power_mgr_set_offprofile(pm_state_mode_type_e pm_mode)
{
	int ret;
	off_profile_t offp;

	if (!balletto_vbat_resume_enabled()) {

		const struct device *const wakeup_dev = DEVICE_DT_GET(WAKEUP_SOURCE);

		balletto_vbat_resume_enable();

		if (!device_is_ready(wakeup_dev)) {
			LOG_ERR("%s: device not ready", wakeup_dev->name);
			return -1;
		}

		ret = counter_start(wakeup_dev);
		if (ret) {
			LOG_ERR("Failed to start counter (err %d)", ret);
			return -1;
		}
	}

	/* Set default for stop mode with RTC wakeup support */
	offp.power_domains = PD_VBAT_AON_MASK;
	offp.memory_blocks = MRAM_MASK;
	offp.memory_blocks |= SERAM_1_MASK | SERAM_2_MASK | SERAM_3_MASK | SERAM_4_MASK;
	offp.memory_blocks |= SRAM5_1_MASK | SRAM5_2_MASK;
	offp.dcdc_voltage = DCDC_VOUT_0825;

	switch (pm_mode) {
	case PM_STATE_MODE_IDLE:
		offp.power_domains |= PD_SYST_MASK | PD_SSE700_AON_MASK;
		offp.memory_blocks |= SRAM5_3_MASK;
		offp.ip_clock_gating = LDO_PHY_MASK;
		offp.phy_pwr_gating = LDO_PHY_MASK;
		offp.dcdc_mode = DCDC_MODE_PFM_FORCED;
		break;
	case PM_STATE_MODE_STANDBY:
		offp.power_domains |= PD_SSE700_AON_MASK;
		offp.memory_blocks |= SRAM5_3_MASK;
		offp.ip_clock_gating = 0;
		offp.phy_pwr_gating = 0;
		offp.dcdc_mode = DCDC_MODE_OFF;
		break;
	case PM_STATE_MODE_STOP:
		offp.ip_clock_gating = 0;
		offp.phy_pwr_gating = 0;
		offp.dcdc_mode = DCDC_MODE_OFF;
		break;
	}

	offp.aon_clk_src = CLK_SRC_LFXO;
	offp.stby_clk_src = CLK_SRC_HFRC;
	offp.stby_clk_freq = SCALED_FREQ_RC_STDBY_76_8_MHZ;
	offp.ewic_cfg = WAKEUP_EWIC_CFG;
	offp.wakeup_events = WAKEUP_EVENT;
	offp.vtor_address = SCB->VTOR;
	offp.vtor_address_ns = SCB->VTOR;

	ret = se_service_set_off_cfg(&offp);
	if (ret) {
		LOG_ERR("SE: set_off_cfg failed = %d", ret);
	} else {
		if (IS_ENABLED(CONFIG_SOC_B1_DK_RTSS_HE)) {
			/* Set DCDC */
			sys_write32(0x0a004411, 0x1a60a034);
			sys_write32(0x1e11e701, 0x1a60a030);
		}
	}

	return ret;
}

void power_mgr_ready_for_sleep(void)
{
	pm_policy_state_lock_put(PM_STATE_SOFT_OFF, PM_ALL_SUBSTATES);
}

void power_mgr_set_subsys_off_period(uint32_t period_ms)
{
	k_sleep(K_MSEC(period_ms));
	pm_policy_state_lock_get(PM_STATE_SOFT_OFF, PM_ALL_SUBSTATES);
}

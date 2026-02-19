/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <zephyr/shell/shell_uart.h>
#include <zephyr/logging/log.h>
#include <stdlib.h>
#include <se_service.h>

#include "appl_shell.h"
#include "power_mgr.h"
#include "power_defines.h"
#include "power_shell.h"

extern run_profile_t current_runp;
extern off_profile_t current_offp;
extern int16_t power_profile;

LOG_MODULE_REGISTER(power_shell, CONFIG_MAIN_LOG_LEVEL);

static int16_t get_power_profile(const char *power_profile)
{
	if (!strcmp(power_profile, "GO_1")) {
		/* GO_1: M55-HE running CoreMark at 160 MHz.
		 * NPU running convolution MAC workload.
		 */
		current_runp.power_domains = PD_VBAT_AON_MASK | PD_SSE700_AON_MASK | PD_SYST_MASK;
		current_runp.power_domains |= PD_SESS_MASK;
		current_runp.dcdc_voltage = DCDC_VOUT_0825;
		current_runp.dcdc_mode = DCDC_MODE_PFM_FORCED;
		current_runp.memory_blocks = MRAM_MASK | SERAM_MASK | BACKUP4K_MASK;
		current_runp.memory_blocks |= APP_RET_MEM_BLOCKS;
		current_runp.aon_clk_src = CLK_SRC_LFXO;
		current_runp.run_clk_src = CLK_SRC_PLL;
		current_runp.cpu_clk_freq = CLOCK_FREQUENCY_160MHZ;
		current_runp.scaled_clk_freq = SCALED_FREQ_RC_ACTIVE_76_8_MHZ;
		current_runp.phy_pwr_gating = 0;
		current_runp.vdd_ioflex_3V3 = IOFLEX_LEVEL_1V8;
		return PM_STATE_MODE_GO_1;
	} else if (!strcmp(power_profile, "GO_2")) {
		/* GO_2: M55-HE running CoreMark at 160 MHz. No NPU is enabled. */
		current_runp.power_domains = PD_VBAT_AON_MASK | PD_SSE700_AON_MASK | PD_SYST_MASK;
		current_runp.power_domains |= PD_SESS_MASK;
		current_runp.dcdc_voltage = DCDC_VOUT_0825;
		current_runp.dcdc_mode = DCDC_MODE_PFM_FORCED;
		current_runp.memory_blocks = MRAM_MASK | SERAM_MASK | BACKUP4K_MASK;
		current_runp.memory_blocks |= APP_RET_MEM_BLOCKS;
		current_runp.aon_clk_src = CLK_SRC_LFXO;
		current_runp.run_clk_src = CLK_SRC_PLL;
		current_runp.cpu_clk_freq = CLOCK_FREQUENCY_160MHZ;
		current_runp.scaled_clk_freq = SCALED_FREQ_RC_ACTIVE_76_8_MHZ;
		current_runp.phy_pwr_gating = 0;
		current_runp.vdd_ioflex_3V3 = IOFLEX_LEVEL_1V8;
		return PM_STATE_MODE_GO_2;
	} else if (!strcmp(power_profile, "GO_3")) {
		/** GO_3: M55-HE running CoreMark at 160 MHz. No NPU is enabled.
		 * TCM not retained, MRAM off, SYSTOP OFF, SE OFF, SE in retention.
		 * NOTE! No MRAM, must use TCM build
		 * NOTE! No SYSTOP, must use LPUART
		 */
		current_runp.power_domains = PD_VBAT_AON_MASK | PD_SSE700_AON_MASK;
		current_runp.dcdc_voltage = DCDC_VOUT_0825;
		current_runp.dcdc_mode = DCDC_MODE_PFM_FORCED;
		current_runp.memory_blocks = SERAM_MASK | BACKUP4K_MASK | APP_RET_MEM_BLOCKS;
		current_runp.aon_clk_src = CLK_SRC_LFXO;
		current_runp.run_clk_src = CLK_SRC_PLL;
		current_runp.cpu_clk_freq = CLOCK_FREQUENCY_160MHZ;
		current_runp.scaled_clk_freq = SCALED_FREQ_RC_ACTIVE_76_8_MHZ;
		current_runp.phy_pwr_gating = 0;
		current_runp.vdd_ioflex_3V3 = IOFLEX_LEVEL_1V8;
		return PM_STATE_MODE_GO_3;
	} else if (!strcmp(power_profile, "GO_4")) {
		/* GO_4: M55-HE running CoreMark at 76.8 MHz. No NPU is enabled.
		 * TCM not retained, MRAM off, SYSTOP OFF, SE OFF, SE is NOT in retention.
		 * NOTE! No MRAM, must use TCM build
		 * NOTE! No SYSTOP, must use LPUART
		 */
		current_runp.power_domains = PD_VBAT_AON_MASK | PD_SSE700_AON_MASK;
		current_runp.dcdc_voltage = 825;
		current_runp.dcdc_mode = DCDC_MODE_PFM_FORCED;
		current_runp.memory_blocks = BACKUP4K_MASK | APP_RET_MEM_BLOCKS;
		current_runp.aon_clk_src = CLK_SRC_LFXO;
		current_runp.run_clk_src = CLK_SRC_HFRC;
		current_runp.cpu_clk_freq = CLOCK_FREQUENCY_76_8_RC_MHZ;
		/* Set RC divider to max 76.8Mhz so CPU is run at requested 76.8Mhz */
		current_runp.scaled_clk_freq = SCALED_FREQ_RC_ACTIVE_76_8_MHZ;
		current_runp.phy_pwr_gating = 0;
		current_runp.vdd_ioflex_3V3 = IOFLEX_LEVEL_1V8;
		return PM_STATE_MODE_GO_4;
	} else if (!strcmp(power_profile, "GO_5")) {
		/* GO_5: M55-HE running CoreMark at 19.2 MHz. No NPU is enabled.
		 * TCM not retained, MRAM off, SYSTOP OFF, SE OFF, SE is NOT in retention.
		 * NOTE! No MRAM, must use TCM build
		 * NOTE! No SYSTOP, must use LPUART
		 */
		current_runp.power_domains = PD_VBAT_AON_MASK | PD_SSE700_AON_MASK;
		current_runp.dcdc_voltage = 825;
		current_runp.dcdc_mode = DCDC_MODE_PFM_FORCED;
		current_runp.memory_blocks = BACKUP4K_MASK | APP_RET_MEM_BLOCKS;
		current_runp.aon_clk_src = CLK_SRC_LFXO;
		current_runp.run_clk_src = CLK_SRC_HFRC;
		current_runp.cpu_clk_freq = CLOCK_FREQUENCY_76_8_RC_MHZ;
		/* Set divider to divide max RC 76.8Mhz to 19.2 Mhz */
		current_runp.scaled_clk_freq = SCALED_FREQ_RC_ACTIVE_19_2_MHZ;
		current_runp.phy_pwr_gating = 0;
		current_runp.vdd_ioflex_3V3 = IOFLEX_LEVEL_1V8;
		return PM_STATE_MODE_GO_5;
	} else if (!strcmp(power_profile, "READY_1")) {
		/* READY_1: M55-HE WFI at 160 MHz from PLL.
		 * NOTE! No MRAM, must use TCM build
		 */
		current_runp.power_domains = PD_VBAT_AON_MASK | PD_SSE700_AON_MASK | PD_SYST_MASK;
		current_runp.dcdc_voltage = DCDC_VOUT_0825;
		current_runp.dcdc_mode = DCDC_MODE_PFM_FORCED;
		current_runp.memory_blocks = BACKUP4K_MASK | APP_RET_MEM_BLOCKS;
		current_runp.aon_clk_src = CLK_SRC_LFXO;
		current_runp.run_clk_src = CLK_SRC_PLL;
		current_runp.cpu_clk_freq = CLOCK_FREQUENCY_160MHZ;
		current_runp.scaled_clk_freq = SCALED_FREQ_RC_ACTIVE_76_8_MHZ;
		current_runp.phy_pwr_gating = 0;
		current_runp.vdd_ioflex_3V3 = IOFLEX_LEVEL_1V8;
	} else if (!strcmp(power_profile, "READY_2")) {
		/* READY_2: M55-HE WFI at 76.8 MHz from HFRC.
		 * NOTE! No MRAM, must use TCM build
		 * NOTE! No SYSTOP, must use LPUART
		 */
		current_runp.power_domains = PD_VBAT_AON_MASK | PD_SSE700_AON_MASK;
		current_runp.dcdc_voltage = 825;
		current_runp.dcdc_mode = DCDC_MODE_PFM_FORCED;
		current_runp.memory_blocks = BACKUP4K_MASK | APP_RET_MEM_BLOCKS;
		current_runp.aon_clk_src = CLK_SRC_LFXO;
		current_runp.run_clk_src = CLK_SRC_HFRC;
		current_runp.cpu_clk_freq = CLOCK_FREQUENCY_76_8_RC_MHZ;
		/* Set RC divider to max 76.8Mhz so CPU is run at requested 76.8Mhz */
		current_runp.scaled_clk_freq = SCALED_FREQ_RC_ACTIVE_76_8_MHZ;
		current_runp.phy_pwr_gating = 0;
		current_runp.vdd_ioflex_3V3 = IOFLEX_LEVEL_1V8;
		return PM_STATE_MODE_READY_2;
	/* Rest power states are for Off profile only */
	} else if (!strcmp(power_profile, "IDLE_1")) {
		/* IDLE_1: All CPU cores powered off. 38.4MHz clock from HFXO. */
		current_offp.power_domains   = PD_VBAT_AON_MASK | PD_SSE700_AON_MASK | PD_SYST_MASK;
		current_offp.dcdc_voltage    = 825;
		current_offp.dcdc_mode       = DCDC_MODE_PFM_FORCED;
		current_offp.memory_blocks   = BACKUP4K_MASK | APP_RET_MEM_BLOCKS;
		current_offp.aon_clk_src     = CLK_SRC_LFXO;
		current_offp.stby_clk_src    = CLK_SRC_HFXO;
		current_offp.stby_clk_freq   = SCALED_FREQ_XO_HIGH_DIV_38_4_MHZ;
		current_offp.ewic_cfg        = EWIC_RTC_A;
		current_offp.phy_pwr_gating  = 0;
		current_offp.vdd_ioflex_3V3  = IOFLEX_LEVEL_1V8;
		current_offp.wakeup_events   = WE_LPRTC;
		current_offp.vtor_address    = SCB->VTOR;
		current_offp.vtor_address_ns = SCB->VTOR;
		return PM_STATE_MODE_IDLE_1;
	} else if (!strcmp(power_profile, "IDLE_2")) {
		/* IDLE_2: All CPU cores powered off. 600kHz clock from HFRC. */
		current_offp.power_domains   = PD_VBAT_AON_MASK | PD_SSE700_AON_MASK | PD_SYST_MASK;
		current_offp.dcdc_voltage    = 825;
		current_offp.dcdc_mode       = DCDC_MODE_PFM_FORCED;
		current_offp.memory_blocks   = BACKUP4K_MASK | APP_RET_MEM_BLOCKS;
		current_offp.aon_clk_src     = CLK_SRC_LFXO;
		current_offp.stby_clk_src    = CLK_SRC_HFRC;
		current_offp.stby_clk_freq   = SCALED_FREQ_RC_STDBY_0_6_MHZ;
		current_offp.ewic_cfg        = EWIC_RTC_A;
		current_offp.phy_pwr_gating  = 0;
		current_offp.vdd_ioflex_3V3  = IOFLEX_LEVEL_1V8;
		current_offp.wakeup_events   = WE_LPRTC;
		current_offp.vtor_address    = SCB->VTOR;
		current_offp.vtor_address_ns = SCB->VTOR;
		return PM_STATE_MODE_IDLE_2;
	} else if (!strcmp(power_profile, "STANDBY_1")) {
		/* STANDBY_1: All CPU cores powered off. 600kHz clock from HFRC.
		 * No SYSTOP.
		 */
		current_offp.power_domains   = PD_VBAT_AON_MASK | PD_SSE700_AON_MASK;
		current_offp.dcdc_voltage    = 825;
		current_offp.dcdc_mode       = DCDC_MODE_PFM_FORCED;
		current_offp.memory_blocks   = BACKUP4K_MASK | APP_RET_MEM_BLOCKS;
		current_offp.aon_clk_src     = CLK_SRC_LFXO;
		current_offp.stby_clk_src    = CLK_SRC_HFRC;
		current_offp.stby_clk_freq   = SCALED_FREQ_RC_STDBY_0_6_MHZ;
		current_offp.ewic_cfg        = EWIC_RTC_A;
		current_offp.phy_pwr_gating  = 0;
		current_offp.vdd_ioflex_3V3  = IOFLEX_LEVEL_1V8;
		current_offp.wakeup_events   = WE_LPRTC;
		current_offp.vtor_address    = SCB->VTOR;
		current_offp.vtor_address_ns = SCB->VTOR;
		return PM_STATE_MODE_STANDBY_1;
	} else if (!strcmp(power_profile, "STOP_1")) {
		/* STOP_2 plus 256KB of M55-HE TCM SRAM retained. */
		current_offp.power_domains   = 0;
		current_offp.dcdc_voltage    = 825;
		current_offp.dcdc_mode       = DCDC_MODE_PFM_FORCED;
		current_offp.memory_blocks   = BACKUP4K_MASK | SRAM4_1_MASK | SRAM4_2_MASK;
		current_offp.memory_blocks   |= SRAM5_1_MASK | SRAM5_2_MASK;
		current_offp.aon_clk_src     = CLK_SRC_LFXO;
		current_offp.stby_clk_src    = CLK_SRC_HFRC;
		current_offp.stby_clk_freq   = SCALED_FREQ_RC_STDBY_0_6_MHZ;
		current_offp.ewic_cfg        = EWIC_RTC_A;
		current_offp.phy_pwr_gating  = 0;
		current_offp.vdd_ioflex_3V3  = IOFLEX_LEVEL_1V8;
		current_offp.wakeup_events   = WE_LPRTC;
		current_offp.vtor_address    = SCB->VTOR;
		current_offp.vtor_address_ns = SCB->VTOR;
		return PM_STATE_MODE_STOP_1;
	} else if (!strcmp(power_profile, "STOP_2")) {
		/* STOP_3 plus 4KB Utility SRAM retained. */
		current_offp.power_domains   = 0;
		current_offp.dcdc_voltage    = 825;
		current_offp.dcdc_mode       = DCDC_MODE_PFM_FORCED;
		current_offp.memory_blocks   = BACKUP4K_MASK;
		current_offp.aon_clk_src     = CLK_SRC_LFXO;
		current_offp.stby_clk_src    = CLK_SRC_HFRC;
		current_offp.stby_clk_freq   = SCALED_FREQ_RC_STDBY_0_6_MHZ;
		current_offp.ewic_cfg        = EWIC_RTC_A;
		current_offp.phy_pwr_gating  = 0;
		current_offp.vdd_ioflex_3V3  = IOFLEX_LEVEL_1V8;
		current_offp.wakeup_events   = WE_LPRTC;
		current_offp.vtor_address    = SCB->VTOR;
		current_offp.vtor_address_ns = SCB->VTOR;
		return PM_STATE_MODE_STOP_2;
	} else if (!strcmp(power_profile, "STOP_3")) {
		/* STOP_4 plus LPTIMER, BOD, LPCMP, and LPGPIO active. */
		current_offp.power_domains   = 0;
		current_offp.dcdc_voltage    = 825;
		current_offp.dcdc_mode       = DCDC_MODE_PFM_FORCED;
		current_offp.memory_blocks   = 0;
		current_offp.aon_clk_src     = CLK_SRC_LFXO;
		current_offp.stby_clk_src    = CLK_SRC_HFRC;
		current_offp.stby_clk_freq   = SCALED_FREQ_RC_STDBY_0_6_MHZ;
		current_offp.ewic_cfg        = EWIC_RTC_A;
		current_offp.phy_pwr_gating  = 0;
		current_offp.vdd_ioflex_3V3  = IOFLEX_LEVEL_1V8;
		current_offp.wakeup_events   = WE_LPRTC;
		current_offp.vtor_address    = SCB->VTOR;
		current_offp.vtor_address_ns = SCB->VTOR;
		return PM_STATE_MODE_STOP_3;
	} else if (!strcmp(power_profile, "STOP_4")) {
		/* STOP_5 plus LPRTC running from 32.768 kHz LFXO. */
		current_offp.power_domains   = 0;
		current_offp.dcdc_voltage    = 825;
		current_offp.dcdc_mode       = DCDC_MODE_PFM_FORCED;
		current_offp.memory_blocks   = 0;
		current_offp.aon_clk_src     = CLK_SRC_LFXO;
		current_offp.stby_clk_src    = CLK_SRC_HFRC;
		current_offp.stby_clk_freq   = SCALED_FREQ_RC_STDBY_0_6_MHZ;
		current_offp.ewic_cfg        = EWIC_RTC_A;
		current_offp.phy_pwr_gating  = 0;
		current_offp.vdd_ioflex_3V3  = IOFLEX_LEVEL_1V8;
		current_offp.wakeup_events   = WE_LPRTC;
		current_offp.vtor_address    = SCB->VTOR;
		current_offp.vtor_address_ns = SCB->VTOR;
		return PM_STATE_MODE_STOP_4;
	} else if (!strcmp(power_profile, "STOP_5")) {
		/* STOP_5: 32.7 kHz LFRC */
		current_offp.power_domains   = 0;
		current_offp.dcdc_voltage    = 825;
		current_offp.dcdc_mode       = DCDC_MODE_PFM_FORCED;
		current_offp.memory_blocks   = 0;
		current_offp.aon_clk_src     = CLK_SRC_LFRC;
		current_offp.stby_clk_src    = CLK_SRC_HFRC;
		current_offp.stby_clk_freq   = SCALED_FREQ_RC_STDBY_0_6_MHZ;
		current_offp.ewic_cfg        = EWIC_RTC_A;
		current_offp.phy_pwr_gating  = 0;
		current_offp.vdd_ioflex_3V3  = IOFLEX_LEVEL_1V8;
		current_offp.wakeup_events   = WE_LPRTC;
		current_offp.vtor_address    = SCB->VTOR;
		current_offp.vtor_address_ns = SCB->VTOR;
		return PM_STATE_MODE_STOP_5;
	}

	return -1;
}

static void print_run_cfg(const struct shell *shell, run_profile_t *runp, uint32_t err)
{
	shell_print(shell, "run_cfg: power_domains = 0x%08" PRIX32 ", dcdc_voltage = %" PRIu32
		", aon_clk_src = %" PRIu32 ", run_clk_src = %" PRIu32 ", cpu_clk_freq = %" PRIu32
		", scaled_clk_freq = %" PRIu32 "\n"
		"         memory_blocks = 0x%08" PRIX32 ", dcdc_mode = %" PRIu32
		", phy_pwr_gating = %" PRIu32 ", vdd_ioflex_3V3 = %" PRIu32 "\n"
		"         error_code = %" PRIu32 ", ret = %" PRIu32 "\n",
		runp->power_domains, runp->dcdc_voltage, runp->aon_clk_src,
		runp->run_clk_src, runp->cpu_clk_freq, runp->scaled_clk_freq,
		runp->memory_blocks, runp->dcdc_mode, runp->phy_pwr_gating,
		runp->vdd_ioflex_3V3, err, err);
}

static void print_off_cfg(const struct shell *shell, off_profile_t *offp, uint32_t err)
{
	shell_print(shell, "off_cfg: power_domains = 0x%08" PRIX32 ", dcdc_voltage = %" PRIu32
		", dcdc_mode = %" PRIu32 ", aon_clk_src = %" PRIu32 ", stby_clk_src = %" PRIu32
		", stby_clk_freq = %" PRIu32 "\n         memory_blocks = 0x%08" PRIX32
		", phy_pwr_gating = %" PRIu32 ", vdd_ioflex_3V3 = %" PRIu32
		", wakeup_events = 0x%08" PRIX32 ", ewic_cfg = 0x%08" PRIX32
		", vtor_address = 0x%08" PRIX32 "\n         vtor_address_ns = 0x%08" PRIX32
		", error_code = %" PRIu32 ", ret = %" PRIu32 "\n",
		offp->power_domains, offp->dcdc_voltage, offp->dcdc_mode,
		offp->aon_clk_src, offp->stby_clk_src, offp->stby_clk_freq,
		offp->memory_blocks, offp->phy_pwr_gating,
		offp->vdd_ioflex_3V3, offp->wakeup_events, offp->ewic_cfg,
		offp->vtor_address, offp->vtor_address_ns, err, err);
}

/**
 * Application command functions
 **/
static int cmd_set_run_cfg(const struct shell *shell, size_t argc, char **argv)
{
	/**
	 * If power profile was defined, set correct params to current_runp
	 * before getting the rest of the params
	 */
	char *pwr_profile = param_get_char(argc, argv, "--power_profile", NULL);

	if (pwr_profile) {
		shell_print(shell, "Using power profile %s\n", pwr_profile);
		power_profile = get_power_profile(pwr_profile);
	}

	current_runp.power_domains = param_get_int(argc, argv, "--power_domains",
		current_runp.power_domains);
	current_runp.dcdc_mode = param_get_int(argc, argv, "--dcdc_mode", current_runp.dcdc_mode);
	current_runp.dcdc_voltage = param_get_int(argc, argv, "--dcdc_voltage",
		current_runp.dcdc_voltage);
	current_runp.aon_clk_src = param_get_int(argc, argv, "--aon_clk_src",
		current_runp.aon_clk_src);
	current_runp.run_clk_src = param_get_int(argc, argv, "--run_clk_src",
		current_runp.run_clk_src);
	current_runp.cpu_clk_freq = param_get_int(argc, argv, "--cpu_clk_freq",
		current_runp.cpu_clk_freq);
	current_runp.scaled_clk_freq = param_get_int(argc, argv, "--scaled_clk_freq",
		current_runp.scaled_clk_freq);
	current_runp.memory_blocks = param_get_int(argc, argv, "--memory_blocks",
		current_runp.memory_blocks);
	current_runp.phy_pwr_gating = param_get_int(argc, argv, "--phy_pwr_gating",
		current_runp.phy_pwr_gating);
	current_runp.vdd_ioflex_3V3 = param_get_int(argc, argv, "--vdd_ioflex_3V3",
		current_runp.vdd_ioflex_3V3);

	int ret = se_service_set_run_cfg(&current_runp);

	if (ret) {
		LOG_ERR("SE: set_run_cfg failed = %d", ret);
		return -ENOEXEC;
	}

	print_run_cfg(shell, &current_runp, ret);

	return ret;

}

static int cmd_set_off_cfg(const struct shell *shell, size_t argc, char **argv)
{
	/**
	 * If power profile was defined, set correct params to offp
	 * before getting the rest of the params
	 */
	char *pwr_profile = param_get_char(argc, argv, "--power_profile", NULL);

	if (pwr_profile) {
		shell_print(shell, "Using power profile %s\n", pwr_profile);
		power_profile = get_power_profile(pwr_profile);
	}

	current_offp.power_domains = param_get_int(argc, argv, "--power_domains",
		current_offp.power_domains);
	current_offp.dcdc_mode = param_get_int(argc, argv, "--dcdc_mode",
		current_offp.dcdc_mode);
	current_offp.dcdc_voltage = param_get_int(argc, argv, "--dcdc_voltage",
		current_offp.dcdc_voltage);
	current_offp.aon_clk_src = param_get_int(argc, argv, "--aon_clk_src",
		current_offp.aon_clk_src);
	current_offp.stby_clk_src = param_get_int(argc, argv, "--stby_clk_src",
		current_offp.stby_clk_src);
	current_offp.stby_clk_freq = param_get_int(argc, argv, "--stby_clk_freq",
		current_offp.stby_clk_freq);
	current_offp.memory_blocks = param_get_int(argc, argv, "--memory_blocks",
		current_offp.memory_blocks);
	current_offp.phy_pwr_gating = param_get_int(argc, argv, "--phy_pwr_gating",
		current_offp.phy_pwr_gating);
	current_offp.vdd_ioflex_3V3 = param_get_int(argc, argv, "--vdd_ioflex_3V3",
		current_offp.vdd_ioflex_3V3);

	current_offp.wakeup_events = param_get_int(argc, argv, "--wakeup_events",
		current_offp.wakeup_events);
	current_offp.ewic_cfg = param_get_int(argc, argv, "--ewic_cfg", current_offp.ewic_cfg);
	current_offp.vtor_address = param_get_int(argc, argv, "--vtor_address",
		current_offp.vtor_address);
	current_offp.vtor_address_ns = param_get_int(argc, argv, "--vtor_address_ns",
		current_offp.vtor_address_ns);

	int ret = set_current_off_profile();

	if (ret) {
		LOG_ERR("SE: set_off_cfg failed = %d", ret);
		return -ENOEXEC;
	}

	print_off_cfg(shell, &current_offp, ret);

	return 0;
}

static int cmd_set_clk_divider(const struct shell *shell, size_t argc, char **argv)
{
	int32_t divider = 0;
	uint32_t value = 0;

	divider = param_get_int(argc, argv, "--divider", divider);
	value = param_get_int(argc, argv, "--value", value);

	int ret = se_service_clock_set_divider((clock_divider_t)divider, value);

	if (ret) {
		LOG_ERR("SE: set_clock_set_divider failed = %d", ret);
	}

	shell_print(shell, "SERVICES_clocks_set_divider: divider = %" PRId32
		", value = %" PRId32 ", error_code = 0, ret = 0", divider, value);

	return ret;
}

static int cmd_set_default_run_cfg(const struct shell *shell, size_t argc, char **argv)
{
	get_default_run_cfg(&current_runp);
	int ret = se_service_set_run_cfg(&current_runp);

	if (ret) {
		LOG_ERR("SE: set_run_cfg failed = %d", ret);
	}

	print_run_cfg(shell, &current_runp, ret);

	return ret;
}

static int cmd_set_default_off_cfg(const struct shell *shell, size_t argc, char **argv)
{
	get_default_off_cfg(&current_offp);
	int ret = se_service_set_off_cfg(&current_offp);

	if (ret) {
		LOG_ERR("SE: set_off_cfg failed = %d", ret);
	}

	print_off_cfg(shell, &current_offp, ret);

	return ret;
}

static int cmd_get_default_run_cfg(const struct shell *shell, size_t argc, char **argv)
{
	run_profile_t runp;

	get_default_run_cfg(&runp);
	print_run_cfg(shell, &runp, 0);

	return 0;
}

static int cmd_get_default_off_cfg(const struct shell *shell, size_t argc, char **argv)
{
	off_profile_t offp;

	get_default_off_cfg(&offp);
	print_off_cfg(shell, &offp, 0);

	return 0;
}

#define SET_RUN_HELP "set_run_cfg --power_profile <optional profile> --power_domains" \
" <pwr domains in hex> --dcdc_voltage <750 - 850>\n --dcdc_mode <...> --aon_clk_src " \
"<LFRC = 0, LFXO = 1> --run_clk_src <HFRC = 0, HFXO = 1, PLL = 2>\n" \
"--cpu_clk_freq <core freq, see clock_frequency_t> --scaled_clk_freq <scaled_clk_freq_t>\n" \
"--memory_blocks <memory blocks in hex> --phy_pwr_gating <...> --vdd_ioflex_3V3 " \
"<3V3 = 0, 1V8 0 1 (ioflex_mode_t)>\n" \
"See more details from file aipm.h"

#define SET_OFF_HELP "set_off_cfg --power_profile <optional profile> --power_domains " \
"<pwr domains in hex> --dcdc_voltage <750 - 850>\n --dcdc_mode <...>  --aon_clk_src " \
"<LFRC = 0, LFXO = 1> --stby_clk_src <HFRC = 0, HFXO = 1, PLL = 2>\n" \
"--stby_clk_freq < core freq, see clock_frequency_t> --memory_blocks < memory blocks in hex>\n" \
"--phy_pwr_gating <...> --vdd_ioflex_3V3 <3V3 = 0, 1V8 0 1 (ioflex_mode_t)> --wakeup_events" \
"<hex value>\n --ewic_cfg <hex value> --vtor_address <vtor addr in hex> --vtor_address_ns" \
"<vtor addr_ns in hex>\n" \
"See more details from aipm.h"

#define SET_CLK_HELP ""

SHELL_STATIC_SUBCMD_SET_CREATE(
	power_sub_cmds,
	SHELL_CMD_ARG(set_default_run_cfg, NULL, "Set the default run aipm profile",
		cmd_set_default_run_cfg, 0, 0),
	SHELL_CMD_ARG(set_default_off_cfg, NULL, "Set the default off aipm profile",
		cmd_set_default_off_cfg, 0, 0),
	SHELL_CMD_ARG(get_default_run_cfg, NULL, "Get the default run aipm profile",
		cmd_get_default_run_cfg, 0, 0),
	SHELL_CMD_ARG(get_default_off_cfg, NULL, "Get the default off aipm profile",
		cmd_get_default_off_cfg, 0, 0),
	SHELL_CMD_ARG(set_run_cfg, NULL, SET_RUN_HELP, cmd_set_run_cfg, 0, 0),
	SHELL_CMD_ARG(set_off_cfg, NULL, SET_OFF_HELP, cmd_set_off_cfg, 0, 0),
	SHELL_CMD_ARG(clocks_set_divider, NULL, SET_CLK_HELP, cmd_set_clk_divider, 0, 0),
	SHELL_SUBCMD_SET_END);
SHELL_CMD_REGISTER(senc, &power_sub_cmds, "Secure Enclave configuration commands", NULL);

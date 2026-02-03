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
#include "power_shell.h"

extern run_profile_t current_runp;
extern off_profile_t current_offp;
extern int16_t power_profile;

LOG_MODULE_REGISTER(power_shell, CONFIG_MAIN_LOG_LEVEL);

#define APP_RET_MEM_BLOCKS \
	SRAM4_1_MASK | SRAM4_2_MASK | SRAM4_3_MASK | SRAM4_4_MASK | SRAM5_1_MASK | SRAM5_2_MASK |  \
	SRAM5_3_MASK | SRAM5_4_MASK | SRAM5_5_MASK

#if DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(rtc0), snps_dw_apb_rtc, okay)
#define WAKEUP_SOURCE         DT_NODELABEL(rtc0)
#define SE_OFFP_EWIC_CFG      EWIC_RTC_A
#define SE_OFFP_WAKEUP_EVENTS WE_LPRTC
#elif DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(timer0), snps_dw_timers, okay)
#define WAKEUP_SOURCE         DT_NODELABEL(timer0)
#define SE_OFFP_EWIC_CFG      EWIC_VBAT_TIMER
#define SE_OFFP_WAKEUP_EVENTS WE_LPTIMER0
#else
#error "Wakeup Device not enabled in the dts"
#endif

static int16_t get_power_profile(const char *power_profile)
{
	if (!strcmp(power_profile, "GO_4")) {
		/* GO_4: Only M55-HE running WHILE(1) at 76.8 MHz from HFRC. No NPU is enabled. */
		/* NOTE! No MRAM, must use TCM build */
		/* NOTE! No SYSTOP, must use LPUART */
		current_runp.power_domains = PD_VBAT_AON_MASK | PD_SSE700_AON_MASK;
		current_runp.aon_clk_src = CLK_SRC_LFXO;
		current_runp.run_clk_src = CLK_SRC_HFRC;
		current_runp.cpu_clk_freq = CLOCK_FREQUENCY_76_8_RC_MHZ;
		/* Set RC divider to max 76.8Mhz so CPU is run at requested 76.8Mhz */
		current_runp.scaled_clk_freq = SCALED_FREQ_RC_ACTIVE_76_8_MHZ;
		current_runp.memory_blocks = SERAM_MASK | BACKUP4K_MASK;
		current_runp.phy_pwr_gating = 0;
		current_runp.vdd_ioflex_3V3 = IOFLEX_LEVEL_1V8;
		current_runp.dcdc_voltage = 825;
		current_runp.dcdc_mode = DCDC_MODE_PFM_FORCED;
		return PM_STATE_MODE_GO_4;
	} else if (!strcmp(power_profile, "GO_5")) {
		/* GO_5: Only M55-HE running WHILE(1) at 19.2 MHz from HFRC. No NPU is enabled. */
		/* NOTE! No MRAM, must use TCM build */
		/* NOTE! No SYSTOP, must use LPUART */
		current_runp.power_domains = PD_VBAT_AON_MASK | PD_SSE700_AON_MASK;
		current_runp.aon_clk_src = CLK_SRC_LFXO;
		current_runp.run_clk_src = CLK_SRC_HFRC;
		current_runp.cpu_clk_freq = CLOCK_FREQUENCY_76_8_RC_MHZ;
		/* Set divider to divide max RC 76.8Mhz to 19.2 Mhz */
		current_runp.scaled_clk_freq = SCALED_FREQ_RC_ACTIVE_19_2_MHZ;
		current_runp.memory_blocks = SERAM_MASK | BACKUP4K_MASK;
		current_runp.phy_pwr_gating = 0;
		current_runp.vdd_ioflex_3V3 = IOFLEX_LEVEL_1V8;
		current_runp.dcdc_voltage = 825;
		current_runp.dcdc_mode = DCDC_MODE_PFM_FORCED;
		return PM_STATE_MODE_GO_5;
	} else if (!strcmp(power_profile, "READY_2")) {
		/* READY_2: M55-HE WFI at 76.8 MHz from HFRC. */
		/* NOTE! No MRAM, must use TCM build */
		/* NOTE! No SYSTOP, must use LPUART */
		current_runp.power_domains = PD_VBAT_AON_MASK | PD_SSE700_AON_MASK;
		current_runp.aon_clk_src = CLK_SRC_LFXO;
		current_runp.run_clk_src = CLK_SRC_HFRC;
		current_runp.cpu_clk_freq = CLOCK_FREQUENCY_76_8_RC_MHZ;
		/* Set RC divider to max 76.8Mhz so CPU is run at requested 76.8Mhz */
		current_runp.scaled_clk_freq = SCALED_FREQ_RC_ACTIVE_76_8_MHZ;
		current_runp.memory_blocks = BACKUP4K_MASK;
		current_runp.phy_pwr_gating = 0;
		current_runp.vdd_ioflex_3V3 = IOFLEX_LEVEL_1V8;
		current_runp.dcdc_voltage = 825;
		current_runp.dcdc_mode = DCDC_MODE_PFM_FORCED;
		return PM_STATE_MODE_READY_2;
	/* Rest power states are for Off profile only */
	} else if (!strcmp(power_profile, "IDLE_1")) {
		/* IDLE_1: All CPU cores powered off. 38.4MHz clock from HFXO. */
		current_offp.power_domains   = PD_VBAT_AON_MASK | PD_SSE700_AON_MASK | PD_SYST_MASK;
		current_offp.dcdc_voltage    = 825;
		current_offp.dcdc_mode       = DCDC_MODE_PFM_FORCED;
		current_offp.aon_clk_src     = CLK_SRC_LFXO;
		current_offp.stby_clk_src    = CLK_SRC_HFXO;
		current_offp.stby_clk_freq   = SCALED_FREQ_XO_HIGH_DIV_38_4_MHZ;
		current_offp.memory_blocks   = APP_RET_MEM_BLOCKS | BACKUP4K_MASK;
		current_offp.ewic_cfg        = SE_OFFP_EWIC_CFG;
		/** If CONFIG_FLASH_BASE_ADDRESS is zero application run
		 * from itcm and no MRAM needed
		 */
#if (CONFIG_FLASH_BASE_ADDRESS != 0)
		current_offp.memory_blocks  |= MRAM_MASK;
#endif
		current_offp.phy_pwr_gating  = 0;
		current_offp.vdd_ioflex_3V3  = IOFLEX_LEVEL_1V8;
		current_offp.wakeup_events   = SE_OFFP_WAKEUP_EVENTS;
		current_offp.vtor_address    = SCB->VTOR;
		current_offp.vtor_address_ns = SCB->VTOR;
		return PM_STATE_MODE_IDLE_1;
	} else if (!strcmp(power_profile, "IDLE_2")) {
		return PM_STATE_MODE_IDLE_2;
	} else if (!strcmp(power_profile, "STANDBY_1")) {
		return PM_STATE_MODE_STANDBY_1;
	} else if (!strcmp(power_profile, "STOP_1")) {
		return PM_STATE_MODE_STOP_1;
	} else if (!strcmp(power_profile, "STOP_2")) {
		return PM_STATE_MODE_STOP_2;
	} else if (!strcmp(power_profile, "STOP_3")) {
		return PM_STATE_MODE_STOP_3;
	} else if (!strcmp(power_profile, "STOP_4")) {
		return PM_STATE_MODE_STOP_4;
	} else if (!strcmp(power_profile, "STOP_5")) {
		return PM_STATE_MODE_STOP_5;
	}

	return -1;
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

	shell_print(shell, "run_cfg: power_domains = 0x%08" PRIX32 ", dcdc_voltage = %" PRIu32
		", aon_clk_src = %" PRIu32 ", run_clk_src = %" PRIu32 ", cpu_clk_freq = %" PRIu32
		", scaled_clk_freq = %" PRIu32 "\n"
		"         memory_blocks = 0x%08" PRIX32 ", dcdc_mode = %" PRIu32
		", phy_pwr_gating = %" PRIu32 ", vdd_ioflex_3V3 = %" PRIu32 "\n"
		"         error_code = %" PRIu32 ", ret = %" PRIu32 "\n",
		current_runp.power_domains, current_runp.dcdc_voltage, current_runp.aon_clk_src,
		current_runp.run_clk_src, current_runp.cpu_clk_freq, current_runp.scaled_clk_freq,
		current_runp.memory_blocks, current_runp.dcdc_mode, current_runp.phy_pwr_gating,
		current_runp.vdd_ioflex_3V3, ret, ret);
	return 0;
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

	shell_print(shell, "off_cfg: power_domains = 0x%08" PRIX32 ", dcdc_voltage = %" PRIu32
		", dcdc_mode = %" PRIu32 ", aon_clk_src = %" PRIu32 ", stby_clk_src = %" PRIu32
		", stby_clk_freq = %" PRIu32 "\n         memory_blocks = 0x%08" PRIX32
		", phy_pwr_gating = %" PRIu32 ", vdd_ioflex_3V3 = %" PRIu32
		", wakeup_events = 0x%08" PRIX32 ", ewic_cfg = 0x%08" PRIX32
		", vtor_address = 0x%08" PRIX32 "\n         vtor_address_ns = 0x%08" PRIX32
		", error_code = %" PRIu32 ", ret = %" PRIu32 "\n",
		current_offp.power_domains, current_offp.dcdc_voltage, current_offp.dcdc_mode,
		current_offp.aon_clk_src, current_offp.stby_clk_src, current_offp.stby_clk_freq,
		current_offp.memory_blocks, current_offp.phy_pwr_gating,
		current_offp.vdd_ioflex_3V3, current_offp.wakeup_events, current_offp.ewic_cfg,
		current_offp.vtor_address, current_offp.vtor_address_ns, ret, ret);

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

SHELL_STATIC_SUBCMD_SET_CREATE(
	power_sub_cmds,
	SHELL_CMD_ARG(set_run_cfg, NULL, SET_RUN_HELP, cmd_set_run_cfg, 0, 0),
	SHELL_CMD_ARG(set_off_cfg, NULL, SET_OFF_HELP, cmd_set_off_cfg, 0, 0),
	SHELL_SUBCMD_SET_END);
SHELL_CMD_REGISTER(senc, &power_sub_cmds, "Ble power configuration commands", NULL);

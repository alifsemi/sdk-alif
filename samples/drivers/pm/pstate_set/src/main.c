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
#include <string.h>
#include <zephyr/cpu_freq/cpu_freq.h>
#include <zephyr/cpu_freq/pstate.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/counter.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/time_units.h>
#include <zephyr/sys/util.h>
#include <se_service.h>

LOG_MODULE_REGISTER(pstate_set, CONFIG_PSTATE_SET_LOG_LEVEL);

#define SLEEP_MS 1000U
#define UPTIME_TOL_MS 100U
#define HZ_TOL_PERCENT 2U

#if defined(CONFIG_RTSS_HP)
#define EXTSYS_SETTING CLOCK_SETTING_EXTSYS0_FREQ
#define EXTSYS_NAME "EXTSYS0"
#else
#define EXTSYS_SETTING CLOCK_SETTING_EXTSYS1_FREQ
#define EXTSYS_NAME "EXTSYS1"
#endif

#if DT_NODE_EXISTS(DT_NODELABEL(pstate_pll_400))
#define BOOT_PSTATE_NODE DT_NODELABEL(pstate_pll_400)
#elif DT_NODE_EXISTS(DT_NODELABEL(pstate_pll_160))
#define BOOT_PSTATE_NODE DT_NODELABEL(pstate_pll_160)
#else
#error "SoC dtsi must provide pstate_pll_400 (HP) or pstate_pll_160 (HE)"
#endif

#if !DT_NODE_HAS_STATUS(BOOT_PSTATE_NODE, okay)
#error "Boot PLL P-state must be okay"
#endif

#if !DT_NODE_HAS_STATUS(DT_NODELABEL(rtc0), okay)
#error "Enable rtc0 in the pstate snippet for a SysTick wall-clock check"
#endif
#define REF_COUNTER_NODE DT_NODELABEL(rtc0)

static const struct device *const ref_ctr = DEVICE_DT_GET(REF_COUNTER_NODE);

static const struct pstate *const pstates[] = {
	DT_FOREACH_CHILD_STATUS_OKAY_SEP(DT_PATH(performance_states),
					 PSTATE_DT_GET, (,))
};

static const char *const pstate_names[] = {
	DT_FOREACH_CHILD_STATUS_OKAY_SEP(DT_PATH(performance_states),
					 DT_NODE_FULL_NAME, (,))
};

BUILD_ASSERT(ARRAY_SIZE(pstates) == ARRAY_SIZE(pstate_names));

struct pstate_result {
	int apply_err;
	int verify_err;
	bool profile_mismatch;
	hfclock_t sent_clk_src;
	hfclock_t live_clk_src;
	clock_frequency_t sent_cpu_clk;
	clock_frequency_t live_cpu_clk;
	scaled_clk_freq_t sent_scaled;
	scaled_clk_freq_t live_scaled;
	uint32_t extsys_hz;
	uint32_t axi_hz;
	uint32_t ahb_hz;
	uint32_t apb_hz;
	uint32_t hfosc_hz;
	uint32_t sysref_hz;
	uint32_t tick_hz;
	int64_t sleep_ms;
	uint64_t ref_ms;
	uint32_t dcyc;
};

static struct pstate_result results[ARRAY_SIZE(pstates)];

static const char *pstate_short_name(const char *full)
{
	const char *slash = strrchr(full, '/');

	return (slash != NULL) ? (slash + 1) : full;
}

static bool hz_close(uint32_t a, uint32_t b)
{
	uint32_t lo = MIN(a, b);
	uint32_t hi = MAX(a, b);
	uint32_t tol;

	if (lo == 0U) {
		return hi == 0U;
	}

	tol = (lo * HZ_TOL_PERCENT) / 100U;
	if (tol == 0U) {
		tol = 1U;
	}

	return (hi - lo) <= tol;
}

#if CONFIG_PSTATE_SET_LOG_LEVEL >= LOG_LEVEL_DBG
static const char *hfclk_name(hfclock_t src)
{
	switch (src) {
	case CLK_SRC_HFRC:
		return "HFRC";
	case CLK_SRC_HFXO:
		return "HFXO";
	case CLK_SRC_PLL:
		return "PLL";
	default:
		return "?";
	}
}

static const char *cpu_clk_name(clock_frequency_t freq)
{
	static const char *const names[] = {
		"800MHz",  "400MHz",  "300MHz",  "200MHz",
		"160MHz",  "120MHz",  "80MHz",   "60MHz",
		"100MHz",  "50MHz",   "20MHz",   "10MHz",
		"76.8RC",  "38.4RC",  "76.8XO",  "38.4XO",
		"DISABLED",
	};

	if ((unsigned int)freq < ARRAY_SIZE(names)) {
		return names[freq];
	}

	return "?";
}

static const char *scaled_clk_name(scaled_clk_freq_t freq)
{
	static const char *const names[] = {
		"RC_A_76.8", "RC_A_38.4", "RC_A_19.2", "RC_A_9.6",
		"RC_A_4.8",  "RC_A_2.4",  "RC_A_1.2",  "RC_A_0.6",
		"RC_S_76.8", "RC_S_38.4", "RC_S_19.2", "RC_S_4.8",
		"RC_S_1.2",  "RC_S_0.6",  "RC_S_0.3",  "RC_S_0.075",
		"XO_L_38.4", "XO_L_19.2", "XO_L_9.6",  "XO_L_4.8",
		"XO_L_2.4",  "XO_L_1.2",  "XO_L_0.6",  "XO_L_0.3",
		"XO_H_38.4", "XO_H_19.2", "XO_H_9.6",  "XO_H_2.4",
		"XO_H_0.6",  "XO_H_0.3",  "XO_H_0.15", "XO_H_0.0375",
		"NONE",
	};

	if ((unsigned int)freq < ARRAY_SIZE(names)) {
		return names[freq];
	}

	return "?";
}

static void print_vote_field(const char *name, const char *sent_name,
			     unsigned int sent, const char *live_name,
			     unsigned int live)
{
	if (sent != live) {
		LOG_DBG("%s sent=%s(%u) live=%s(%u) !=", name, sent_name, sent,
			live_name, live);
	} else {
		LOG_DBG("%s sent=%s(%u) live=%s(%u)", name, sent_name, sent,
			live_name, live);
	}
}

static void print_profile_mismatch(const struct pstate_result *r)
{
	if (!r->profile_mismatch) {
		return;
	}

	LOG_DBG("SE profile vote mismatch");
	print_vote_field("clk_src", hfclk_name(r->sent_clk_src),
			 (unsigned int)r->sent_clk_src,
			 hfclk_name(r->live_clk_src),
			 (unsigned int)r->live_clk_src);
	print_vote_field("cpu_clk_freq", cpu_clk_name(r->sent_cpu_clk),
			 (unsigned int)r->sent_cpu_clk,
			 cpu_clk_name(r->live_cpu_clk),
			 (unsigned int)r->live_cpu_clk);
	print_vote_field("scaled_clk_freq", scaled_clk_name(r->sent_scaled),
			 (unsigned int)r->sent_scaled,
			 scaled_clk_name(r->live_scaled),
			 (unsigned int)r->live_scaled);
}

static void report_row_debug(const struct pstate_result *r)
{
	if (r->apply_err != 0) {
		print_profile_mismatch(r);
		return;
	}

	LOG_DBG("%s=%u.%u AXI=%u.%u AHB=%u.%u APB=%u.%u HFOSC=%u.%u "
		"SYSREF=%u.%u SysTick=%u.%u MHz",
		EXTSYS_NAME,
		r->extsys_hz / 1000000U, (r->extsys_hz / 100000U) % 10U,
		r->axi_hz / 1000000U, (r->axi_hz / 100000U) % 10U,
		r->ahb_hz / 1000000U, (r->ahb_hz / 100000U) % 10U,
		r->apb_hz / 1000000U, (r->apb_hz / 100000U) % 10U,
		r->hfosc_hz / 1000000U, (r->hfosc_hz / 100000U) % 10U,
		r->sysref_hz / 1000000U, (r->sysref_hz / 100000U) % 10U,
		r->tick_hz / 1000000U, (r->tick_hz / 100000U) % 10U);
	LOG_DBG("sleep %lld ms RTC %llu ms cycles %u",
		(long long)r->sleep_ms, (unsigned long long)r->ref_ms,
		r->dcyc);
	print_profile_mismatch(r);
}
#endif /* CONFIG_PSTATE_SET_LOG_LEVEL >= LOG_LEVEL_DBG */

static void console_idle(void)
{
	const struct device *uart = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
	uint8_t discard;
	int64_t t0;

	if (device_is_ready(uart)) {
		t0 = k_uptime_get();
		while (uart_irq_tx_complete(uart) == 0) {
			if ((k_uptime_get() - t0) > 50) {
				break;
			}
		}

		t0 = k_uptime_get();
		while (uart_poll_in(uart, &discard) == 0) {
			if ((k_uptime_get() - t0) > 50) {
				break;
			}
		}
	}

	k_msleep(20);
}

static int apply_pstate(const struct pstate *state)
{
	return cpu_freq_pstate_set(state);
}

static int se_read_clocks(struct pstate_result *r)
{
	int err;

	err = se_service_clock_setting_get(EXTSYS_SETTING, &r->extsys_hz);
	if (err) {
		return err;
	}
	err = se_service_clock_setting_get(CLOCK_SETTING_AXI_FREQ, &r->axi_hz);
	if (err) {
		return err;
	}
	err = se_service_clock_setting_get(CLOCK_SETTING_AHB_FREQ, &r->ahb_hz);
	if (err) {
		return err;
	}
	err = se_service_clock_setting_get(CLOCK_SETTING_APB_FREQ, &r->apb_hz);
	if (err) {
		return err;
	}
	err = se_service_clock_setting_get(CLOCK_SETTING_HFOSC_FREQ, &r->hfosc_hz);
	if (err) {
		return err;
	}
	err = se_service_clock_setting_get(CLOCK_SETTING_SYSREF_FREQ, &r->sysref_hz);
	if (err) {
		return err;
	}

	r->tick_hz = sys_clock_hw_cycles_per_sec();
	return 0;
}

static int measure_pstate(const struct pstate *state, struct pstate_result *r)
{
	run_profile_t sent_cfg;
	run_profile_t run_cfg;
	int err;
	int64_t t0;
	uint32_t ref0, ref1;
	uint32_t c0, c1;

	r->apply_err = 0;
	r->verify_err = 0;
	r->profile_mismatch = false;
	r->sent_clk_src = CLK_SRC_PLL;
	r->live_clk_src = CLK_SRC_PLL;
	r->sent_cpu_clk = CLOCK_FREQUENCY_DISABLED;
	r->live_cpu_clk = CLOCK_FREQUENCY_DISABLED;
	r->sent_scaled = SCALED_FREQ_NONE;
	r->live_scaled = SCALED_FREQ_NONE;

	console_idle();
	r->apply_err = apply_pstate(state);
	if (r->apply_err != 0) {
		return r->apply_err;
	}

	err = se_service_get_last_set_run_cfg(&sent_cfg);
	if (err) {
		r->verify_err = err;
		return err;
	}

	err = se_service_get_run_cfg(&run_cfg);
	if (err) {
		r->verify_err = err;
		return err;
	}

	r->sent_clk_src = sent_cfg.run_clk_src;
	r->live_clk_src = run_cfg.run_clk_src;
	r->sent_cpu_clk = sent_cfg.cpu_clk_freq;
	r->live_cpu_clk = run_cfg.cpu_clk_freq;
	r->sent_scaled = sent_cfg.scaled_clk_freq;
	r->live_scaled = run_cfg.scaled_clk_freq;
	r->profile_mismatch =
		(r->sent_clk_src != r->live_clk_src) ||
		(r->sent_cpu_clk != r->live_cpu_clk) ||
		(r->sent_scaled != r->live_scaled);

	err = se_read_clocks(r);
	if (err) {
		r->verify_err = err;
		return err;
	}

	if (!hz_close(r->tick_hz, r->extsys_hz)) {
		r->verify_err = -EIO;
		return -EIO;
	}

	err = counter_get_value(ref_ctr, &ref0);
	if (err) {
		r->verify_err = err;
		return err;
	}

	c0 = k_cycle_get_32();
	t0 = k_uptime_get();
	k_msleep(SLEEP_MS);
	r->sleep_ms = k_uptime_get() - t0;
	c1 = k_cycle_get_32();
	r->dcyc = c1 - c0;

	err = counter_get_value(ref_ctr, &ref1);
	if (err) {
		r->verify_err = err;
		return err;
	}

	r->ref_ms = counter_ticks_to_us(ref_ctr, ref1 - ref0) / 1000U;

	if ((r->sleep_ms < (int64_t)(SLEEP_MS - UPTIME_TOL_MS)) ||
	    (r->sleep_ms > (int64_t)(SLEEP_MS + UPTIME_TOL_MS))) {
		r->verify_err = -EIO;
		return -EIO;
	}

	if ((r->ref_ms < (SLEEP_MS - UPTIME_TOL_MS)) ||
	    (r->ref_ms > (SLEEP_MS + UPTIME_TOL_MS))) {
		r->verify_err = -EIO;
		return -EIO;
	}

	if (!hz_close(r->dcyc, r->extsys_hz)) {
		r->verify_err = -EIO;
		return -EIO;
	}

	return 0;
}

static bool row_failed(const struct pstate_result *r)
{
	return (r->apply_err != 0) || (r->verify_err != 0);
}

static void report_row_walk(unsigned int step, unsigned int n, const char *name,
			    const struct pstate_result *r)
{
	const char *short_name = pstate_short_name(name);

	if (r->apply_err != 0) {
		LOG_INF("[%u/%u] %s: APPLY_FAIL (err=%d)", step, n, short_name,
			r->apply_err);
	} else if (r->verify_err != 0) {
		LOG_INF("[%u/%u] %s: VERIFY_FAIL (err=%d)", step, n, short_name,
			r->verify_err);
	} else {
		LOG_INF("[%u/%u] %s: PASS", step, n, short_name);
	}

#if CONFIG_PSTATE_SET_LOG_LEVEL >= LOG_LEVEL_DBG
	report_row_debug(r);
#endif
}

static void report_pass_list(unsigned int n)
{
	unsigned int i;
	bool any = false;

	for (i = 0; i < n; i++) {
		if (row_failed(&results[i])) {
			continue;
		}
		if (!any) {
			printf("\nPASS\n");
			any = true;
		}
		printf("  %s\n", pstate_short_name(pstate_names[i]));
	}
}

static void report_fail_list(unsigned int n)
{
	unsigned int i;
	const char *verdict;
	int err;
	bool any = false;

	for (i = 0; i < n; i++) {
		if (!row_failed(&results[i])) {
			continue;
		}
		if (!any) {
			printf("\nFAIL\n");
			any = true;
		}
		if (results[i].apply_err != 0) {
			verdict = "APPLY_FAIL";
			err = results[i].apply_err;
		} else {
			verdict = "VERIFY_FAIL";
			err = results[i].verify_err;
		}
		printf("  %s  %s (err=%d)\n",
		       pstate_short_name(pstate_names[i]), verdict, err);
	}
}

static void report_all(unsigned int n, int restore_err)
{
	unsigned int i;
	unsigned int pass = 0;
	unsigned int fail = 0;

	printf("pstate_set: walked %u states (%s, rtc0)\n\n", n, EXTSYS_NAME);

	for (i = 0; i < n; i++) {
		if (row_failed(&results[i])) {
			fail++;
		} else {
			pass++;
		}
	}

	printf("Summary\n");
	printf("  PASS  %u\n", pass);
	printf("  FAIL  %u\n", fail);
	if (restore_err != 0) {
		printf("  boot restore FAIL (err=%d)\n", restore_err);
	} else {
		printf("  boot restore ok\n");
	}

	report_pass_list(n);
	report_fail_list(n);

	if ((fail != 0U) || (restore_err != 0)) {
		printf("\npstate_set: FAILED\n");
	} else {
		printf("\npstate_set: PASSED\n");
	}
}

int main(void)
{
	const unsigned int n = (unsigned int)ARRAY_SIZE(pstates);
	unsigned int i;
	int err;
	int restore_err;

	if (!device_is_ready(ref_ctr)) {
		printf("pstate_set: reference clock %s is not ready\n",
		       ref_ctr->name);
		return 0;
	}

	err = counter_start(ref_ctr);
	if (err != 0 && err != -EALREADY) {
		printf("pstate_set: could not start reference clock %s (err=%d)\n",
		       ref_ctr->name, err);
		return 0;
	}

	printf("pstate_set: walking %u states (%s, rtc0)\n", n, EXTSYS_NAME);

	for (i = 0; i < n; i++) {
		int boot_err;

		(void)measure_pstate(pstates[i], &results[i]);
		console_idle();
		boot_err = apply_pstate(PSTATE_DT_GET(BOOT_PSTATE_NODE));
		console_idle();
		report_row_walk(i + 1U, n, pstate_names[i], &results[i]);
#if CONFIG_PSTATE_SET_LOG_LEVEL >= LOG_LEVEL_DBG
		LOG_DBG("boot restore apply_err=%d", boot_err);
#else
		ARG_UNUSED(boot_err);
#endif
	}

	console_idle();
	restore_err = apply_pstate(PSTATE_DT_GET(BOOT_PSTATE_NODE));
	console_idle();

	report_all(n, restore_err);
	return 0;
}

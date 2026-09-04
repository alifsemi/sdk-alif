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

LOG_MODULE_REGISTER(pstate_set, LOG_LEVEL_INF);

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

/* Boot PLL by DT label. An app that wants one destination does the same:
 *   cpu_freq_pstate_set(PSTATE_DT_GET(DT_NODELABEL(pstate_pll_160)));
 */
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

static void print_mhz(uint32_t hz)
{
	printf("%u.%u MHz", hz / 1000000U, (hz / 100000U) % 10U);
}

/* UART PM suspend returns -EBUSY if RX still has a byte, and TX
 * may still be draining after printf. Idle the console first.
 */
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

static int verify_pstate(int step, unsigned int n, const char *name)
{
	int err;
	uint32_t extsys_hz = 0;
	uint32_t axi_hz = 0;
	uint32_t ahb_hz = 0;
	uint32_t apb_hz = 0;
	uint32_t tick_hz;
	int64_t t0, elapsed;
	uint32_t c0, c1, dcyc;
	uint32_t ref0, ref1;
	uint64_t ref_ms;
	run_profile_t sent_cfg;
	run_profile_t run_cfg;

	err = se_service_get_last_set_run_cfg(&sent_cfg);
	if (err) {
		printf("[%d/%u] %s: FAIL — last_set_run_cfg (err=%d)\n",
		       step, n, name, err);
		return err;
	}

	err = se_service_get_run_cfg(&run_cfg);
	if (err) {
		printf("[%d/%u] %s: FAIL — get_run_cfg (err=%d)\n",
		       step, n, name, err);
		return err;
	}

	if ((sent_cfg.run_clk_src != run_cfg.run_clk_src) ||
	    (sent_cfg.cpu_clk_freq != run_cfg.cpu_clk_freq) ||
	    ((sent_cfg.run_clk_src != CLK_SRC_PLL) &&
	     (sent_cfg.scaled_clk_freq != run_cfg.scaled_clk_freq))) {
		printf("[%d/%u] %s: MISMATCH — SE run profile differs from this core's "
		       "request (system clocks use a vote of all cores)\n",
		       step, n, name);
		printf("[%d/%u] %s:   requested   run_clk_src=%u  "
		       "cpu_clk_freq=%u  scaled_clk_freq=%u\n",
		       step, n, name,
		       (unsigned int)sent_cfg.run_clk_src,
		       (unsigned int)sent_cfg.cpu_clk_freq,
		       (unsigned int)sent_cfg.scaled_clk_freq);
		printf("[%d/%u] %s:   get_run_cfg run_clk_src=%u  "
		       "cpu_clk_freq=%u  scaled_clk_freq=%u\n",
		       step, n, name,
		       (unsigned int)run_cfg.run_clk_src,
		       (unsigned int)run_cfg.cpu_clk_freq,
		       (unsigned int)run_cfg.scaled_clk_freq);
	}

	err = se_service_clock_setting_get(EXTSYS_SETTING, &extsys_hz);
	if (err) {
		printf("[%d/%u] %s: FAIL — could not read %s (err=%d)\n",
		       step, n, name, EXTSYS_NAME, err);
		return err;
	}

	err = se_service_clock_setting_get(CLOCK_SETTING_AXI_FREQ, &axi_hz);
	if (err) {
		printf("[%d/%u] %s: FAIL — could not read AXI (err=%d)\n",
		       step, n, name, err);
		return err;
	}

	err = se_service_clock_setting_get(CLOCK_SETTING_AHB_FREQ, &ahb_hz);
	if (err) {
		printf("[%d/%u] %s: FAIL — could not read AHB (err=%d)\n",
		       step, n, name, err);
		return err;
	}

	err = se_service_clock_setting_get(CLOCK_SETTING_APB_FREQ, &apb_hz);
	if (err) {
		printf("[%d/%u] %s: FAIL — could not read APB (err=%d)\n",
		       step, n, name, err);
		return err;
	}

	tick_hz = sys_clock_hw_cycles_per_sec();
	printf("[%d/%u] %s: current clocks after apply  CPU(%s)=",
	       step, n, name, EXTSYS_NAME);
	print_mhz(extsys_hz);
	printf("  AXI=");
	print_mhz(axi_hz);
	printf("  AHB=");
	print_mhz(ahb_hz);
	printf("  APB=");
	print_mhz(apb_hz);
	printf("  SysTick=");
	print_mhz(tick_hz);
	printf("\n");

	if (!hz_close(tick_hz, extsys_hz)) {
		printf("[%d/%u] %s: FAIL — SysTick ", step, n, name);
		print_mhz(tick_hz);
		printf(" does not match %s ", EXTSYS_NAME);
		print_mhz(extsys_hz);
		printf("\n");
		return -EIO;
	}

	err = counter_get_value(ref_ctr, &ref0);
	if (err) {
		printf("[%d/%u] %s: FAIL — could not read reference clock (err=%d)\n",
		       step, n, name, err);
		return err;
	}

	c0 = k_cycle_get_32();
	t0 = k_uptime_get();
	k_msleep(SLEEP_MS);
	elapsed = k_uptime_get() - t0;
	c1 = k_cycle_get_32();
	dcyc = c1 - c0;

	err = counter_get_value(ref_ctr, &ref1);
	if (err) {
		printf("[%d/%u] %s: FAIL — could not read reference clock (err=%d)\n",
		       step, n, name, err);
		return err;
	}

	ref_ms = counter_ticks_to_us(ref_ctr, ref1 - ref0) / 1000U;

	printf("[%d/%u] %s: slept 1 s to check SysTick against the RTC wall clock\n",
	       step, n, name);
	printf("[%d/%u] %s:   kernel uptime %lld ms (from SysTick)\n",
	       step, n, name, elapsed);
	printf("[%d/%u] %s:   RTC wall clock %llu ms (independent of CPU clock)\n",
	       step, n, name, ref_ms);
	printf("[%d/%u] %s:   CPU cycles in that second %u (expect about ",
	       step, n, name, dcyc);
	print_mhz(extsys_hz);
	printf(")\n");

	if ((elapsed < (int64_t)(SLEEP_MS - UPTIME_TOL_MS)) ||
	    (elapsed > (int64_t)(SLEEP_MS + UPTIME_TOL_MS))) {
		printf("[%d/%u] %s: FAIL — kernel sleep was %lld ms, expected ~%u ms\n",
		       step, n, name, elapsed, SLEEP_MS);
		return -EIO;
	}

	if ((ref_ms < (SLEEP_MS - UPTIME_TOL_MS)) ||
	    (ref_ms > (SLEEP_MS + UPTIME_TOL_MS))) {
		printf("[%d/%u] %s: FAIL — RTC measured %llu ms, expected ~%u ms\n",
		       step, n, name, ref_ms, SLEEP_MS);
		return -EIO;
	}

	if (!hz_close(dcyc, extsys_hz)) {
		printf("[%d/%u] %s: FAIL — CPU cycle count %u does not match %s ",
		       step, n, name, dcyc, EXTSYS_NAME);
		print_mhz(extsys_hz);
		printf("\n");
		return -EIO;
	}

	printf("[%d/%u] %s: PASS : 1 sec sleep matches RTC and %s\n",
	       step, n, name, EXTSYS_NAME);
	return 0;
}

static int apply_and_check(int idx, const struct pstate *state, const char *name)
{
	const unsigned int n = (unsigned int)ARRAY_SIZE(pstates);
	const int step = idx + 1;
	int err;

	printf("\n[%d/%u] %s: applying\n", step, n, name);
	console_idle();

	err = apply_pstate(state);
	if (err) {
		printf("[%d/%u] %s: FAIL — could not apply P-state (err=%d)\n",
		       step, n, name, err);
		return err;
	}

	return verify_pstate(step, n, name);
}

int main(void)
{
	int i, err, fails = 0;

	if (!device_is_ready(ref_ctr)) {
		printf("pstate_set: reference clock %s is not ready\n",
		       ref_ctr->name);
		return 0;
	}

	err = counter_start(ref_ctr);
	if (err && err != -EALREADY) {
		printf("pstate_set: could not start reference clock %s (err=%d)\n",
		       ref_ctr->name, err);
		return 0;
	}

	printf("pstate_set: Demo for %s using reference clock %s, %u states\n",
	       EXTSYS_NAME, ref_ctr->name, (unsigned int)ARRAY_SIZE(pstates));

	for (i = 0; i < (int)ARRAY_SIZE(pstates); i++) {
		if (apply_and_check(i, pstates[i], pstate_names[i])) {
			fails++;
		}
	}

	/* Restore boot PLL by node label, not pstates[0]. */
	console_idle();
	err = apply_pstate(PSTATE_DT_GET(BOOT_PSTATE_NODE));
	if (err) {
		fails++;
		printf("pstate_set: FAIL — could not restore boot clocks (err=%d)\n",
		       err);
	}

	if (fails) {
		printf("pstate_set: Demo FAILED (%d of %u states)\n",
		       fails, (unsigned int)ARRAY_SIZE(pstates));
	} else {
		printf("pstate_set: Demo PASSED (%u states)\n",
		       (unsigned int)ARRAY_SIZE(pstates));
	}
	return 0;
}

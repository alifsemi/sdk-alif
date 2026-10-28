/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/cpu_freq/cpu_freq.h>
#include <zephyr/cpu_freq/pstate.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/time_units.h>
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>
#include <se_service.h>

#if defined(CONFIG_RTSS_HP)
#define EXTSYS_SETTING CLOCK_SETTING_EXTSYS0_FREQ
#define EXTSYS_NAME "EXTSYS0"
#else
#define EXTSYS_SETTING CLOCK_SETTING_EXTSYS1_FREQ
#define EXTSYS_NAME "EXTSYS1"
#endif

#ifdef CONFIG_ZTEST_SHUFFLE
#ifndef CONFIG_ZTEST_REPEAT
#define NUM_ITER_PER_SUITE CONFIG_ZTEST_SHUFFLE_SUITE_REPEAT_COUNT
#define NUM_ITER_PER_TEST  CONFIG_ZTEST_SHUFFLE_TEST_REPEAT_COUNT
#endif
#endif

#ifdef CONFIG_ZTEST_REPEAT
#define NUM_ITER_PER_SUITE CONFIG_ZTEST_SUITE_REPEAT_COUNT
#define NUM_ITER_PER_TEST  CONFIG_ZTEST_TEST_REPEAT_COUNT
#else
#ifndef CONFIG_ZTEST_SHUFFLE
#define NUM_ITER_PER_SUITE 1
#define NUM_ITER_PER_TEST  1
#endif
#endif

static const struct pstate *const pstates[] = {
	DT_FOREACH_CHILD_STATUS_OKAY_SEP(DT_PATH(performance_states),
					 PSTATE_DT_GET, (,))
};

static const char *const pstate_names[] = {
	DT_FOREACH_CHILD_STATUS_OKAY_SEP(DT_PATH(performance_states),
					 DT_NODE_FULL_NAME, (,))
};

BUILD_ASSERT(ARRAY_SIZE(pstates) == ARRAY_SIZE(pstate_names));
BUILD_ASSERT(ARRAY_SIZE(pstates) > 0);

static void console_idle(void)
{
	const struct device *uart = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
	uint8_t discard;
	int64_t t0;

	if (!device_is_ready(uart)) {
		k_msleep(20);
		return;
	}

#if defined(CONFIG_UART_INTERRUPT_DRIVEN)
	t0 = k_uptime_get();
	while (uart_irq_tx_complete(uart) == 0) {
		if ((k_uptime_get() - t0) > 50) {
			break;
		}
	}
#endif

	t0 = k_uptime_get();
	while (uart_poll_in(uart, &discard) == 0) {
		if ((k_uptime_get() - t0) > 50) {
			break;
		}
	}

	k_msleep(20);
}

static int apply_pstate(const struct pstate *state)
{
	int err;

	console_idle();
	err = cpu_freq_pstate_set(state);
	if (err == -EBUSY) {
		console_idle();
		err = cpu_freq_pstate_set(state);
	}

	return err;
}

/* Read the clocks the next test pass will run on. EXTSYS is this core.
 * I2C takes APB. CRC follows the CPU clock.
 */
static int read_clocks_after_set(const char *name)
{
	uint32_t extsys_hz = 0;
	uint32_t ahb_hz = 0;
	uint32_t apb_hz = 0;
	int err;

	err = se_service_clock_setting_get(EXTSYS_SETTING, &extsys_hz);
	if (err) {
		TC_PRINT("P-state %s: read %s failed: %d\n", name, EXTSYS_NAME, err);
		return err;
	}

	err = se_service_clock_setting_get(CLOCK_SETTING_AHB_FREQ, &ahb_hz);
	if (err) {
		TC_PRINT("P-state %s: read AHB failed: %d\n", name, err);
		return err;
	}

	err = se_service_clock_setting_get(CLOCK_SETTING_APB_FREQ, &apb_hz);
	if (err) {
		TC_PRINT("P-state %s: read APB failed: %d\n", name, err);
		return err;
	}

	TC_PRINT("P-state %s clocks: %s=%u Hz AHB=%u Hz APB=%u Hz SysTick=%u Hz\n",
		 name, EXTSYS_NAME, extsys_hz, ahb_hz, apb_hz,
		 sys_clock_hw_cycles_per_sec());
	return 0;
}

/* Each frequency is its own ztest pass. Counts from the previous pass
 * would make a later PASS look like a failure of the same case.
 */
static void pstate_reset_ztest_stats(void)
{
	struct ztest_suite_node *suite;
	struct ztest_unit_test *test;

	for (suite = _ztest_suite_node_list_start; suite < _ztest_suite_node_list_end;
	     suite++) {
		suite->stats->run_count = 0;
		suite->stats->skip_count = 0;
		suite->stats->fail_count = 0;
	}

	for (test = _ztest_unit_test_list_start; test < _ztest_unit_test_list_end; test++) {
		test->stats->run_count = 0;
		test->stats->skip_count = 0;
		test->stats->fail_count = 0;
		test->stats->pass_count = 0;
	}
}

static void run_suites_once(void)
{
	pstate_reset_ztest_stats();
#if CONFIG_ZTEST_SHUFFLE
	ztest_run_all(NULL, true, NUM_ITER_PER_SUITE, NUM_ITER_PER_TEST);
#else
	ztest_run_all(NULL, false, NUM_ITER_PER_SUITE, NUM_ITER_PER_TEST);
#endif
}

void test_main(void)
{
	bool apply_failed = false;

	for (size_t i = 0; i < ARRAY_SIZE(pstates); i++) {
		int err = apply_pstate(pstates[i]);

		if (err != 0) {
			TC_PRINT("P-state %s failed: %d\n", pstate_names[i], err);
			apply_failed = true;
			continue;
		}

		err = read_clocks_after_set(pstate_names[i]);
		if (err != 0) {
			apply_failed = true;
			continue;
		}

		TC_PRINT("=== %s (%u/%u) ===\n", pstate_names[i],
			 (unsigned int)(i + 1U),
			 (unsigned int)ARRAY_SIZE(pstates));
		run_suites_once();
	}

	(void)apply_pstate(pstates[0]);
	ztest_verify_all_test_suites_ran();

	if (apply_failed) {
		k_oops();
	}
}

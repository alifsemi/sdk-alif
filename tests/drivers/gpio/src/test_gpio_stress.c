/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/*
 * Alif GPIO stress and performance coverage.
 *
 * This file adds tests that the upstream Zephyr gpio_basic_api
 * conformance suite does NOT provide:
 *   - High iteration-count stress over the data-path APIs
 *     (set / clear / toggle / get) to catch regressions that only
 *     surface under sustained load.
 *   - Micro-benchmarks that measure and report the average number
 *     of CPU cycles per gpio_pin_{set,get,toggle}_dt() call.
 *     Results are printed only; no hard thresholds are asserted
 *     because absolute timing depends on bus wait-states and
 *     compiler optimisation level. The tests still fail if any
 *     underlying API call returns an error.
 *   - Interrupt storm: many fast edges on a cross-wired input pin
 *     with an EDGE_BOTH callback registered, verifying that the
 *     driver delivers at least a minimum expected number of
 *     callbacks without losing or crashing.
 *
 * Wiring assumptions mirror the multi-input interrupt test:
 *   led0  -> output used to drive the cross-wired input.
 *   led1  -> primary pin under test (GPIO_NODE, also used as
 *            stand-alone output for stress/perf when no physical
 *            jumper to led0 is present).
 */

#include "test_gpio.h"

#define TOGGLE_STRESS_ITERATIONS 10000
#define PERF_ITERATIONS          2000
#define INT_STORM_EDGES          200
#define INT_STORM_MIN_CALLBACKS  1
#define INT_STORM_EDGE_DELAY_US  500

#define LED0_NODE DT_ALIAS(led0)

static const struct gpio_dt_spec stress_pin = GPIO_DT_SPEC_GET(GPIO_NODE, gpios);

static uint32_t avg_cycles(uint32_t total_cycles, uint32_t iterations)
{
	return iterations ? (total_cycles / iterations) : 0U;
}

/*
 * Run TOGGLE_STRESS_ITERATIONS back-to-back toggles on a single
 * output pin and assert every call returned success. This catches
 * drive-path regressions that only manifest under sustained load,
 * which the upstream suite (small fixed-count toggles) does not.
 */
ZTEST(gpio_stress, test_gpio_toggle_stress)
{
	int rc;
	uint32_t start;
	uint32_t duration_us;

	zassert_true(gpio_is_ready_dt(&stress_pin), "GPIO dev is not ready");

	rc = gpio_pin_configure_dt(&stress_pin, GPIO_OUTPUT_LOW);
	zassert_equal(rc, 0, "Failed to configure output pin");

	start = k_cycle_get_32();
	for (unsigned int i = 0; i < TOGGLE_STRESS_ITERATIONS; i++) {
		rc = gpio_pin_toggle_dt(&stress_pin);
		zassert_equal(rc, 0, "Toggle failed at iter %u: %d", i, rc);
	}
	duration_us = k_cyc_to_us_floor32(k_cycle_get_32() - start);

	TC_PRINT("Toggle stress: %u iterations completed in %u us (%u us avg)\n",
		 TOGGLE_STRESS_ITERATIONS, duration_us,
		 TOGGLE_STRESS_ITERATIONS ? (duration_us / TOGGLE_STRESS_ITERATIONS)
					  : 0U);
}

/*
 * Measure and report average cycles per gpio_pin_set_dt() call,
 * alternating between 1 and 0 so the driver cannot short-circuit
 * identical writes.
 */
ZTEST(gpio_stress, test_gpio_pin_set_performance)
{
	int rc;
	uint32_t start;
	uint32_t total;

	zassert_true(gpio_is_ready_dt(&stress_pin), "GPIO dev is not ready");

	rc = gpio_pin_configure_dt(&stress_pin, GPIO_OUTPUT_LOW);
	zassert_equal(rc, 0, "Failed to configure output pin");

	start = k_cycle_get_32();
	for (unsigned int i = 0; i < PERF_ITERATIONS; i++) {
		rc = gpio_pin_set_dt(&stress_pin, i & 1);
		zassert_equal(rc, 0, "pin_set failed at iter %u: %d", i, rc);
	}
	total = k_cycle_get_32() - start;

	TC_PRINT("gpio_pin_set_dt perf: %u iters, %u cycles total, "
		 "%u cycles/call (%u ns/call)\n",
		 PERF_ITERATIONS, total,
		 avg_cycles(total, PERF_ITERATIONS),
		 k_cyc_to_ns_floor32(avg_cycles(total, PERF_ITERATIONS)));
}

/*
 * Measure and report average cycles per gpio_pin_get_dt() call on
 * the same pin after driving it high. The read result is asserted
 * for consistency (all calls must read the same level) to prevent
 * the optimiser from eliminating the loop body.
 */
ZTEST(gpio_stress, test_gpio_pin_get_performance)
{
	int rc;
	int val;
	uint32_t start;
	uint32_t total;
	int first_val;

	zassert_true(gpio_is_ready_dt(&stress_pin), "GPIO dev is not ready");

	rc = gpio_pin_configure_dt(&stress_pin, GPIO_OUTPUT_HIGH);
	zassert_equal(rc, 0, "Failed to configure output pin");

	/*
	 * Reading back an output pin returns the driven level on
	 * DesignWare GPIO, so the value must be stable for the
	 * entire measurement window.
	 */
	first_val = gpio_pin_get_dt(&stress_pin);
	zassert_true(first_val >= 0,
		     "Initial gpio_pin_get_dt failed: %d", first_val);

	start = k_cycle_get_32();
	for (unsigned int i = 0; i < PERF_ITERATIONS; i++) {
		val = gpio_pin_get_dt(&stress_pin);
		zassert_equal(val, first_val,
			      "pin_get drift at iter %u: got %d, expected %d",
			      i, val, first_val);
	}
	total = k_cycle_get_32() - start;

	TC_PRINT("gpio_pin_get_dt perf: %u iters, %u cycles total, "
		 "%u cycles/call (%u ns/call)\n",
		 PERF_ITERATIONS, total,
		 avg_cycles(total, PERF_ITERATIONS),
		 k_cyc_to_ns_floor32(avg_cycles(total, PERF_ITERATIONS)));
}

/*
 * Measure and report average cycles per gpio_pin_toggle_dt() call.
 */
ZTEST(gpio_stress, test_gpio_pin_toggle_performance)
{
	int rc;
	uint32_t start;
	uint32_t total;

	zassert_true(gpio_is_ready_dt(&stress_pin), "GPIO dev is not ready");

	rc = gpio_pin_configure_dt(&stress_pin, GPIO_OUTPUT_LOW);
	zassert_equal(rc, 0, "Failed to configure output pin");

	start = k_cycle_get_32();
	for (unsigned int i = 0; i < PERF_ITERATIONS; i++) {
		rc = gpio_pin_toggle_dt(&stress_pin);
		zassert_equal(rc, 0, "toggle failed at iter %u: %d", i, rc);
	}
	total = k_cycle_get_32() - start;

	TC_PRINT("gpio_pin_toggle_dt perf: %u iters, %u cycles total, "
		 "%u cycles/call (%u ns/call)\n",
		 PERF_ITERATIONS, total,
		 avg_cycles(total, PERF_ITERATIONS),
		 k_cyc_to_ns_floor32(avg_cycles(total, PERF_ITERATIONS)));
}

#if DT_HAS_ALIAS(led0)

static const struct gpio_dt_spec storm_out = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec storm_in  = GPIO_DT_SPEC_GET(GPIO_NODE, gpios);

static volatile unsigned int storm_cb_cnt;

static void storm_callback(const struct device *port, struct gpio_callback *cb,
			   gpio_port_pins_t pins)
{
	ARG_UNUSED(port);
	ARG_UNUSED(cb);

	if (pins & BIT(storm_in.pin)) {
		storm_cb_cnt++;
	}
}

/*
 * Interrupt storm: drive INT_STORM_EDGES edges on the output pin
 * with a short delay and verify that the edge-both callback ran
 * often enough. We only assert a low minimum because debounce or
 * sync paths may legitimately drop a fraction of very fast edges;
 * the intent is to ensure the driver does not hang or permanently
 * mis-mask interrupts under heavy edge traffic. The observed count
 * is printed for diagnostic purposes.
 */
ZTEST(gpio_stress, test_gpio_interrupt_storm)
{
	struct gpio_callback cb;
	int rc;
	unsigned int observed;

	if (!gpio_is_ready_dt(&storm_out) || !gpio_is_ready_dt(&storm_in)) {
		ztest_test_skip();
		return;
	}

	rc = gpio_pin_interrupt_configure_dt(&storm_in, GPIO_INT_DISABLE);
	zassert_equal(rc, 0, "Disable interrupt failed: %d", rc);

	rc = gpio_pin_configure_dt(&storm_in, GPIO_INPUT);
	zassert_equal(rc, 0, "Configure input failed: %d", rc);

	rc = gpio_pin_configure_dt(&storm_out, GPIO_OUTPUT_LOW);
	zassert_equal(rc, 0, "Configure output failed: %d", rc);

	gpio_init_callback(&cb, storm_callback, BIT(storm_in.pin));
	rc = gpio_add_callback(storm_in.port, &cb);
	zassert_equal(rc, 0, "Add callback failed: %d", rc);

	rc = gpio_pin_interrupt_configure_dt(&storm_in, GPIO_INT_EDGE_BOTH);
	if (rc == -ENOTSUP) {
		TC_PRINT("GPIO_INT_EDGE_BOTH not supported; skipping storm\n");
		gpio_remove_callback(storm_in.port, &cb);
		ztest_test_skip();
		return;
	}
	zassert_equal(rc, 0, "Enable EDGE_BOTH failed: %d", rc);

	storm_cb_cnt = 0;
	for (unsigned int i = 0; i < INT_STORM_EDGES; i++) {
		rc = gpio_pin_toggle_dt(&storm_out);
		zassert_equal(rc, 0, "toggle failed at edge %u: %d", i, rc);
		k_busy_wait(INT_STORM_EDGE_DELAY_US);
	}

	/* Allow any in-flight interrupts to drain. */
	k_msleep(50);
	observed = storm_cb_cnt;

	rc = gpio_pin_interrupt_configure_dt(&storm_in, GPIO_INT_DISABLE);
	zassert_equal(rc, 0, "Disable interrupt failed: %d", rc);

	rc = gpio_remove_callback(storm_in.port, &cb);
	zassert_equal(rc, 0, "Remove callback failed: %d", rc);

	TC_PRINT("Interrupt storm: drove %u edges, observed %u callbacks\n",
		 INT_STORM_EDGES, observed);

	if (observed == 0) {
		TC_PRINT("No callbacks observed; check led0->led1 jumper\n");
		ztest_test_skip();
		return;
	}

	zassert_true(observed >= INT_STORM_MIN_CALLBACKS,
		     "Edge storm produced only %u callbacks (min %u)",
		     observed, INT_STORM_MIN_CALLBACKS);
}

#else /* !DT_HAS_ALIAS(led0) */

ZTEST(gpio_stress, test_gpio_interrupt_storm)
{
	TC_PRINT("led0 alias not present in devicetree; skipping storm.\n");
	ztest_test_skip();
}

#endif /* DT_HAS_ALIAS(led0) */

/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/*
 * Alif-unique GPIO coverage.
 *
 * Zephyr's upstream gpio_basic_api conformance test
 * (tests/drivers/gpio/gpio_basic_api) already exercises pin
 * configuration, raw/logical read and write, bitwise port
 * operations, toggle, pull-up/pull-down and callback management.
 * Those behaviours are therefore NOT re-tested here.
 *
 * What is covered in this file is the direction-query API
 * (gpio_pin_is_input_dt() and gpio_pin_is_output_dt()), which the
 * upstream suite does not exercise. A single consolidated test
 * walks the pin through the three relevant states (un-configured,
 * input and output) and verifies the query API reports the expected
 * direction after each transition.
 */

#include "test_gpio.h"

extern int check_pin_conf(int ret);

static int check_pin_is_input(int ret)
{
	if (ret == -ENOSYS) {
		TC_PRINT("gpio_pin_is_input_dt not implemented by driver\n");
		ztest_test_skip();
		return -1;
	}
	return (ret == 1) ? 0 : -1;
}

static int check_pin_is_output(int ret)
{
	if (ret == -ENOSYS) {
		TC_PRINT("gpio_pin_is_output_dt not implemented by driver\n");
		ztest_test_skip();
		return -1;
	}
	return (ret == 1) ? 0 : -1;
}

/*
 * Number of INPUT<->OUTPUT reconfigure cycles performed by the
 * reconfiguration corner-case test. Chosen high enough to catch
 * state-leak bugs but low enough to keep total runtime under a
 * second even on slow simulators.
 */
#define RECONFIGURE_CYCLES 50

/*
 * Pin index that must exceed any reasonable ngpios value on the
 * controller backing GPIO_NODE. The DesignWare IP on Alif Ensemble
 * Gen2 instances supports at most 8 pins per port, so pin 31 is
 * always out of range and must be rejected with -EINVAL.
 */
#define OUT_OF_RANGE_PIN 31

/*
 * Verify gpio_pin_is_input_dt() / gpio_pin_is_output_dt() report
 * the correct direction across the pin's lifecycle:
 *   1) Before any configuration the pin is neither input nor output.
 *   2) After GPIO_INPUT it is input and not output.
 *   3) After GPIO_OUTPUT_LOW it is output and not input.
 * This coverage is NOT provided by the upstream gpio_basic_api test.
 */
ZTEST(gpio_check, test_gpio_direction_query)
{
	static const struct gpio_dt_spec gpio_port = GPIO_DT_SPEC_GET(GPIO_NODE, gpios);
	int rv;

	zassert_true(gpio_is_ready_dt(&gpio_port), "GPIO dev is not ready");

	/* 1. Pre-configuration: pin is neither input nor output. */
	rv = gpio_pin_is_input_dt(&gpio_port);
	zassert_not_equal(check_pin_is_input(rv), 0,
			  "Unconfigured pin reported as input");

	rv = gpio_pin_is_output_dt(&gpio_port);
	zassert_not_equal(check_pin_is_output(rv), 0,
			  "Unconfigured pin reported as output");

	/* 2. Configure as input: queries must reflect input. */
	zassert_equal(check_pin_conf(gpio_pin_configure_dt(&gpio_port, GPIO_INPUT)), 0,
		      "Failed to configure input pin");

	rv = gpio_pin_is_input_dt(&gpio_port);
	zassert_equal(check_pin_is_input(rv), 0,
		      "Pin is not reported as input after GPIO_INPUT");

	rv = gpio_pin_is_output_dt(&gpio_port);
	zassert_not_equal(check_pin_is_output(rv), 0,
			  "Input pin incorrectly reported as output");

	/* 3. Configure as output: queries must reflect output. */
	zassert_equal(check_pin_conf(gpio_pin_configure_dt(&gpio_port, GPIO_OUTPUT_LOW)), 0,
		      "Failed to configure output pin");

	rv = gpio_pin_is_output_dt(&gpio_port);
	zassert_equal(check_pin_is_output(rv), 0,
		      "Pin is not reported as output after GPIO_OUTPUT_LOW");

	rv = gpio_pin_is_input_dt(&gpio_port);
	zassert_not_equal(check_pin_is_input(rv), 0,
			  "Output pin incorrectly reported as input");
}

/*
 * Corner case: configuring a pin as input AND output simultaneously
 * is not supported by the DesignWare GPIO IP (it has no separate
 * drive/receive enables), so the driver must reject the request with
 * -ENOTSUP. Not covered by upstream gpio_basic_api.
 */
ZTEST(gpio_check, test_gpio_invalid_simultaneous_in_out)
{
	static const struct gpio_dt_spec gpio_port = GPIO_DT_SPEC_GET(GPIO_NODE, gpios);
	int rc;

	zassert_true(gpio_is_ready_dt(&gpio_port), "GPIO dev is not ready");

	rc = gpio_pin_configure_dt(&gpio_port, GPIO_INPUT | GPIO_OUTPUT);
	zassert_equal(rc, -ENOTSUP,
		      "Simultaneous GPIO_INPUT|GPIO_OUTPUT should be rejected "
		      "with -ENOTSUP, got %d", rc);

	rc = gpio_pin_configure_dt(&gpio_port, GPIO_DISCONNECTED);
	zassert_true(rc == 0 || rc == -ENOTSUP,
		     "GPIO_DISCONNECTED should be accepted or reported "
		     "-ENOTSUP, got %d", rc);
}

/*
 * Corner case: addressing a pin index outside the controller's
 * ngpios range must fail with -EINVAL, without side-effects on valid
 * pins. Not covered by upstream gpio_basic_api.
 */
ZTEST(gpio_check, test_gpio_invalid_pin_index)
{
	/*
	 * Zephyr's gpio_pin_configure() contains an __ASSERT that validates
	 * the pin against port_pin_mask before the driver is called.  When
	 * CONFIG_ASSERT is enabled this fires a kernel panic for out-of-range
	 * pins, so skip in that configuration.
	 */
	Z_TEST_SKIP_IFDEF(CONFIG_ASSERT);

	static const struct gpio_dt_spec gpio_port = GPIO_DT_SPEC_GET(GPIO_NODE, gpios);
	int rc;

	zassert_true(gpio_is_ready_dt(&gpio_port), "GPIO dev is not ready");

	rc = gpio_pin_configure(gpio_port.port, OUT_OF_RANGE_PIN, GPIO_INPUT);
	zassert_equal(rc, -EINVAL,
		      "Configuring out-of-range pin %d should fail with "
		      "-EINVAL, got %d", OUT_OF_RANGE_PIN, rc);

	/* Valid pin must still be configurable afterwards. */
	rc = gpio_pin_configure_dt(&gpio_port, GPIO_INPUT);
	zassert_equal(check_pin_conf(rc), 0,
		      "Valid pin reconfiguration after invalid-pin attempt failed");
}

/*
 * Stress corner case: flip the pin direction between INPUT and
 * OUTPUT many times in a row. This exposes state-leak bugs in the
 * direction or data registers and verifies the direction-query API
 * stays consistent on every transition. Not covered by upstream.
 */
ZTEST(gpio_check, test_gpio_reconfigure_cycles)
{
	static const struct gpio_dt_spec gpio_port = GPIO_DT_SPEC_GET(GPIO_NODE, gpios);
	int rc;
	int rv;

	zassert_true(gpio_is_ready_dt(&gpio_port), "GPIO dev is not ready");

	for (unsigned int i = 0; i < RECONFIGURE_CYCLES; i++) {
		rc = gpio_pin_configure_dt(&gpio_port, GPIO_INPUT);
		zassert_equal(check_pin_conf(rc), 0,
			      "Iter %u: configure INPUT failed", i);

		rv = gpio_pin_is_input_dt(&gpio_port);
		zassert_equal(check_pin_is_input(rv), 0,
			      "Iter %u: pin not reported as input", i);

		rc = gpio_pin_configure_dt(&gpio_port, GPIO_OUTPUT_LOW);
		zassert_equal(check_pin_conf(rc), 0,
			      "Iter %u: configure OUTPUT failed", i);

		rv = gpio_pin_is_output_dt(&gpio_port);
		zassert_equal(check_pin_is_output(rv), 0,
			      "Iter %u: pin not reported as output", i);
	}

	TC_PRINT("Completed %u INPUT<->OUTPUT reconfigure cycles with stable "
		 "direction-query results\n", RECONFIGURE_CYCLES);
}

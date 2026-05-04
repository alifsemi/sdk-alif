/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include "test_gpio.h"

/*
 * Test suites exposed by this test application. Each suite holds only
 * Alif-unique coverage that is not already provided by Zephyr's upstream
 * tests/drivers/gpio/gpio_basic_api conformance test:
 *   - gpio_check            : direction-query API (gpio_pin_is_input_dt /
 *                             gpio_pin_is_output_dt) across pin lifecycle
 *                             plus Alif-specific corner cases.
 *   - gpio_test_interrupts  : multi-input single-output interrupt topology.
 *   - gpio_stress           : high-iteration stress and per-call
 *                             performance micro-benchmarks for the
 *                             GPIO data-path APIs, plus an edge-storm
 *                             regression for the interrupt path.
 */
ZTEST_SUITE(gpio_check, NULL, NULL, NULL, NULL, NULL);
ZTEST_SUITE(gpio_test_interrupts, NULL, NULL, NULL, NULL, NULL);
ZTEST_SUITE(gpio_stress, NULL, NULL, NULL, NULL, NULL);


/*
 * Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * main.c - I2S Alif consolidated test suite entry point.
 *
 * Suites registered here (each compiled conditionally via CMakeLists.txt):
 *   i2s_alif_functional - Loopback matrix, features, and playback tests
 *   i2s_alif_states    - State machine and transition tests
 *   i2s_alif_golden    - Golden vector tests across rates/bit depths
 *   i2s_alif_negative  - Error condition and negative tests
 *   i2s_alif_config    - Configuration and parameter tests
 */

#include <zephyr/ztest.h>

/* Suite declarations — defined in their respective translation units.
 * ztest_run_test_suite() is called automatically by the framework when
 * ZTEST_SUITE() is used; no explicit registration is needed here.
 */

int main(void)
{
	ztest_run_all(NULL, false, 1, 1);
	return 0;
}

/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef ALIF_GPIO_TEST_GPIO_H_
#define ALIF_GPIO_TEST_GPIO_H_

#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/ztest.h>

/*
 * Devicetree aliases used by the Alif-specific GPIO tests.
 *   led0  - primary output (driven high/low to stimulate inputs).
 *   led1  - primary input under test (GPIO_NODE).
 *   led2  - optional second input for the multi-input interrupt test.
 */
#define GPIO_NODE   DT_ALIAS(led1)

#endif /* ALIF_GPIO_TEST_GPIO_H_ */

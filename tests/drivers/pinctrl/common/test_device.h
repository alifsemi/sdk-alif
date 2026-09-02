/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef ZEPHYR_TESTS_DRIVERS_PINCTRL_COMMON_TEST_DEVICE_H_
#define ZEPHYR_TESTS_DRIVERS_PINCTRL_COMMON_TEST_DEVICE_H_

#include <zephyr/drivers/pinctrl.h>

/** Custom pinctrl state "mystate". */
#ifndef PINCTRL_STATE_MYSTATE
#define PINCTRL_STATE_MYSTATE PINCTRL_STATE_PRIV_START
#endif

/*
 * Alif pinmux encoding: bits 0:2 function, bits 3:9 port.
 * Matches GET_PINMUX_PORT() in the Alif pinctrl driver.
 */
#define TEST_PIN_POS 3U
#define TEST_PIN_MSK 0x7FU
#define TEST_GET_PIN(pincfg) (((pincfg) >> TEST_PIN_POS) & TEST_PIN_MSK)

#endif /* ZEPHYR_TESTS_DRIVERS_PINCTRL_COMMON_TEST_DEVICE_H_ */

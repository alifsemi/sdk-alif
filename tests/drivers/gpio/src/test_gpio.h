/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/ztest.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

/* The devicetree node identifier for the "led0" alias */
#define LED0_NODE       DT_ALIAS(led0)
#define GPIO_NODE       DT_ALIAS(led1)

/* TEST_NODE */
#define GPIO_LED_NODE          DT_GPIO_CTLR(DT_NODELABEL(aled0), gpios)
#define GPIO_LED_PIN           DT_GPIO_PIN(DT_NODELABEL(aled0), gpios)
#define GPIO_LED_PIN_DTS_FLAGS DT_GPIO_FLAGS(DT_NODELABEL(aled0), gpios)
#define GPIO_PIN               DT_GPIO_PIN(DT_NODELABEL(aled1), gpios)


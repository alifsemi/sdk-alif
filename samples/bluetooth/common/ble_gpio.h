/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef BLE_GPIO_H
#define BLE_GPIO_H

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

/* Button Generic event handler */
typedef void (*button_handler_cb)(uint32_t button_state, uint32_t has_changed);

/* Init Supported Buttons */
int ble_gpio_buttons_init(button_handler_cb buttonHandler);

/* Init Supported Leds */
int ble_gpio_led_init(void);

/* Set or clear led */
void ble_gpio_led_set(const struct gpio_dt_spec *led_dev, bool enable);

/* Toggle led state */
void ble_gpio_led_toggle(const struct gpio_dt_spec *led_dev);

#endif /* BLE_GPIO_H */

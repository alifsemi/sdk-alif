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

#include <stdint.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

/* Button Generic event handler */
typedef void (*button_handler_cb)(uint32_t button_state, uint32_t has_changed);

/* Init Supported Buttons */
int ble_gpio_buttons_init(button_handler_cb buttonHandler);

/* Same mask for an enabled button selected by node or alias, or 0 if it is absent.
 * The gpio-keys label property is optional, so aliases such as sw0 stay valid
 * when that property is omitted.
 */
uint32_t ble_gpio_button_cb_mask_dt(struct gpio_dt_spec button);

/* Paste both status tokens before any GPIO spec is formed. GPIO_DT_SPEC_GET
 * exists only in the both-okay helper, so a disabled /buttons node or alias
 * expands to 0U and does not emit DEVICE_DT_GET.
 * Use only BLE_GPIO_BUTTON_MASK(). The helpers below are internal.
 */
#define BLE_GPIO_BUTTON_MASK(alias)                                                                \
	BLE_GPIO_BUTTON_MASK_BY_STATUS(DT_NODE_HAS_STATUS(DT_PATH(buttons), okay),                 \
				       DT_NODE_HAS_STATUS(DT_ALIAS(alias), okay), alias)

#define BLE_GPIO_BUTTON_MASK_BY_STATUS(buttons_ok, node_ok, alias)                                 \
	BLE_GPIO_BUTTON_MASK_BY_STATUS_I(buttons_ok, node_ok, alias)

#define BLE_GPIO_BUTTON_MASK_BY_STATUS_I(buttons_ok, node_ok, alias)                               \
	BLE_GPIO_BUTTON_MASK_##buttons_ok##_##node_ok(alias)

#define BLE_GPIO_BUTTON_MASK_1_1(alias)                                                            \
	ble_gpio_button_cb_mask_dt((struct gpio_dt_spec)GPIO_DT_SPEC_GET(DT_ALIAS(alias), gpios))

#define BLE_GPIO_BUTTON_MASK_1_0(alias) 0U
#define BLE_GPIO_BUTTON_MASK_0_1(alias) 0U
#define BLE_GPIO_BUTTON_MASK_0_0(alias) 0U

/* Paste both status tokens before any GPIO spec is formed. GPIO_DT_SPEC_GET
 * exists only in the both-okay helper. A disabled /leds node or alias expands
 * to an empty spec and does not emit DEVICE_DT_GET.
 * Use only BLE_GPIO_LED_SPEC(). The helpers below are internal.
 */
#define BLE_GPIO_LED_SPEC(alias)                                                                   \
	BLE_GPIO_LED_SPEC_BY_STATUS(DT_NODE_HAS_STATUS(DT_PATH(leds), okay),                       \
				    DT_NODE_HAS_STATUS(DT_ALIAS(alias), okay), alias)

#define BLE_GPIO_LED_SPEC_BY_STATUS(leds_ok, node_ok, alias)                                       \
	BLE_GPIO_LED_SPEC_BY_STATUS_I(leds_ok, node_ok, alias)

#define BLE_GPIO_LED_SPEC_BY_STATUS_I(leds_ok, node_ok, alias)                                     \
	BLE_GPIO_LED_SPEC_##leds_ok##_##node_ok(alias)

#define BLE_GPIO_LED_SPEC_1_1(alias) GPIO_DT_SPEC_GET(DT_ALIAS(alias), gpios)

#define BLE_GPIO_LED_SPEC_1_0(alias) {0}
#define BLE_GPIO_LED_SPEC_0_1(alias) {0}
#define BLE_GPIO_LED_SPEC_0_0(alias) {0}

/* Init Supported Leds */
int ble_gpio_led_init(void);

/* Set or clear led */
void ble_gpio_led_set(const struct gpio_dt_spec *led_dev, bool enable);

/* Toggle led state */
void ble_gpio_led_toggle(const struct gpio_dt_spec *led_dev);

/* Get led state */
int ble_gpio_led_get(const struct gpio_dt_spec *led_dev);

#endif /* BLE_GPIO_H */

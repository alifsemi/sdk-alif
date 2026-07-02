/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef TEST_CMP_H
#define TEST_CMP_H

#include <stdint.h>
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/comparator.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/ztest.h>

#if DT_HAS_ALIAS(cmp)
#define CMP_NODE           DT_ALIAS(cmp)
#elif DT_NODE_HAS_STATUS(DT_NODELABEL(cmp1), okay)
#define CMP_NODE           DT_NODELABEL(cmp1)
#elif DT_NODE_HAS_STATUS(DT_NODELABEL(cmp0), okay)
#define CMP_NODE           DT_NODELABEL(cmp0)
#else
#error "No enabled comparator node found. Set alias 'cmp' or enable cmp0/cmp1."
#endif

#define NODE_LABEL         CMP_NODE
#define CMP_FILTER_TAPS    DT_PROP(CMP_NODE, filter_taps)
#define CMP_HYSTERESIS     DT_PROP(CMP_NODE, hysteresis_level)
#define CMP_NEGATIVE_INPUT DT_PROP(CMP_NODE, negative_input)
#define CMP_PRESCALER      DT_PROP_OR(CMP_NODE, prescaler, 0)
#define CMP_POLARITY_EN    DT_PROP_OR(CMP_NODE, polarity_en, 0)
#define CMP_POSITIVE_INPUT DT_PROP(CMP_NODE, positive_input)

#define CMP_CALLBACK_TIMEOUT_MS 5000

extern volatile uint8_t call_back_event;
extern volatile uint8_t cmp_status;
extern uint8_t value;

void cmp_callback(const struct device *dev, void *status);
void Comp_func(uint32_t loop);
void Comp_func_ref_ext(uint32_t loop);

#endif /* TEST_CMP_H */

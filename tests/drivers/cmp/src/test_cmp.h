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

/*
 * Same node selection as samples/drivers/cmp: any enabled alif,cmp instance.
 * LPCMP overlay enables &lpcmp and disables high-speed CMP; CMP overlay does
 * the reverse. driver_instance enum index 0 is CMP_INSTANCE_LP.
 */
#if !DT_HAS_COMPAT_STATUS_OKAY(alif_cmp)
#error "No enabled alif,cmp node. Enable cmp0/cmp1 or lpcmp (see boards/*.overlay)."
#endif

#define NODE_LABEL DT_COMPAT_GET_ANY_STATUS_OKAY(alif_cmp)
#define CMP_NODE   NODE_LABEL

#define CMP_DRV_INSTANCE DT_ENUM_IDX(CMP_NODE, driver_instance)
#define CMP_IS_LPCMP     (CMP_DRV_INSTANCE == 0)

/*
 * LPCMP: PD0 rail-to-rail comparator. No on-chip negative (no AON/DAC6).
 * Both terminals are analog pins from pinctrl_lpcmp:
 *   +  CMP_POS_IN0..3 = P2_4..P2_7 (ANA_S20..23)
 *   -  CMP_NEG_IN1 = P2_0 (ANA_S16), IN2 = P2_1, IN3 = P2_2
 * Default test: POS P2_4 vs NEG P2_0 (CMP_NEG_IN1).
 * Not applicable: filter_taps, prescaler, polarity_en, cmp-gpios, DAC6.
 *
 * Optional properties must be gated with #if DT_NODE_HAS_PROP. A C-level
 * if () does not stop DT_PROP() from expanding, which is the LPCMP build error.
 */
#if DT_NODE_HAS_PROP(CMP_NODE, filter_taps)
#define CMP_HAS_FILTER_TAPS 1
#define CMP_FILTER_TAPS     DT_PROP(CMP_NODE, filter_taps)
#else
#define CMP_HAS_FILTER_TAPS 0
#define CMP_FILTER_TAPS     0
#endif

#if DT_NODE_HAS_PROP(CMP_NODE, prescaler)
#define CMP_HAS_PRESCALER 1
#define CMP_PRESCALER     DT_PROP(CMP_NODE, prescaler)
#else
#define CMP_HAS_PRESCALER 0
#define CMP_PRESCALER     0
#endif

/*
 * Boolean DT properties still "exist" when unset (value 0). Use the value,
 * not DT_NODE_HAS_PROP, to decide if polarity inversion is enabled.
 */
#define CMP_POLARITY_EN DT_PROP_OR(CMP_NODE, polarity_en, 0)
#if CMP_POLARITY_EN
#define CMP_HAS_POLARITY 1
#else
#define CMP_HAS_POLARITY 0
#endif

#define CMP_HAS_CMP_GPIOS  DT_NODE_HAS_PROP(CMP_NODE, cmp_gpios)
#define CMP_HYSTERESIS     DT_PROP(CMP_NODE, hysteresis_level)
#define CMP_NEGATIVE_INPUT DT_PROP(CMP_NODE, negative_input)
#define CMP_POSITIVE_INPUT DT_PROP(CMP_NODE, positive_input)

#define CMP_CALLBACK_TIMEOUT_MS 5000
#if CMP_IS_LPCMP
/* User analog on P2_4 / P2_0; LED is IRQ indicator only. */
#define LPCMP_ANALOG_WAIT_MS 20000
#define LPCMP_LED_GLOW_MS 300
#endif

extern volatile uint8_t call_back_event;
extern volatile uint8_t cmp_status;
extern uint8_t value;

void cmp_callback(const struct device *dev, void *status);
void Comp_func(uint32_t loop);
void Comp_func_ref_ext(uint32_t loop);

#endif /* TEST_CMP_H */

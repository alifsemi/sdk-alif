/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#ifndef TEST_DAC_H
#define TEST_DAC_H

#include <stddef.h>
#include <stdint.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/dac.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/ztest.h>

/* Device Tree nodes */
#define DAC_NODE		DT_ALIAS(dac)
#define DAC0_NODE		DT_NODELABEL(dac0)
#define DAC1_NODE		DT_NODELABEL(dac1)
#define DAC_TWOSCOMP_ENABLED	DT_PROP_OR(DAC_NODE, twoscomp_enabled, 0)

/* DAC configuration */
#define DAC_RESOLUTION		12
#define DAC_CHANNEL_ID		0
#define DAC_MAX_INPUT		(0xFFFU)  /* 12-bit max: 4095 */
#define DAC_MIN_INPUT		(0x0U)
#define DAC_MID_INPUT		(0x800U)  /* Mid-scale: 2048 */
#define DAC_QUARTER_INPUT	(0x400U)  /* Quarter-scale: 1024 */

/* Voltage reference (internal 1.8V) */
#define DAC_VREF_MV		1800

/* Conversion rate */
#define DAC_MAX_CONV_RATE_HZ	1000  /* Up to 1 kHz at 12-bit */

/* Continuous test parameters */
#define DAC_RAMP_STEP		100   /* Step size for ramp tests */
#define DAC_CONT_ITERATIONS	100   /* Iterations for continuous tests */

/* Global device pointer */
extern const struct device *dac_dev;

/* Default channel configuration */
extern const struct dac_channel_cfg dac_ch_cfg;

struct dac_dual_devs {
	const struct device *dac0;
	const struct device *dac1;
};

/**
 * @brief Initialize and return DAC device, asserting readiness and channel setup.
 * @return Fixture pointer to initialized DAC device.
 */
void *init_dac_device(void);

#if DT_NODE_HAS_STATUS(DAC0_NODE, okay) && DT_NODE_HAS_STATUS(DAC1_NODE, okay)
struct dac_dual_devs init_dac_dual_devices(void);
#endif

#endif /* TEST_DAC_H */

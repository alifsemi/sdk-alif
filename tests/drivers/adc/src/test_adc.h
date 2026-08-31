
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include "temperature.h"
#include <zephyr/ztest.h>
#include <zephyr/logging/log.h>


extern uint32_t buffer[8];
extern uint32_t All_channel;
#define ADC_CHANNEL_0				(0x00)
#define ADC_CHANNEL_1				(0x01)
#define ADC_CHANNEL_2				(0x02)
#define ADC_CHANNEL_3				(0x03)
#define ADC_CHANNEL_4				(0x04)
#define ADC_CHANNEL_5				(0x05)
#define ADC_CHANNEL_6				(0x06)
#define ADC_CHANNEL_7				(0x07)
#define ADC_CHANNEL_8				(0x08)

/****ADC MASK CHANNEL****/
#define ADC_UNMASK_CHANNEL_0			(1 << 0)
#define ADC_UNMASK_CHANNEL_1			(1 << 1)
#define ADC_UNMASK_CHANNEL_2			(1 << 2)
#define ADC_UNMASK_CHANNEL_3			(1 << 3)
#define ADC_UNMASK_CHANNEL_4			(1 << 4)
#define ADC_UNMASK_CHANNEL_5			(1 << 5)
#define ADC_UNMASK_CHANNEL_6			(1 << 6)
#define ADC_UNMASK_CHANNEL_7			(1 << 7)
#define ADC_UNMASK_CHANNEL_8			(1 << 8)

#define ADC_COMPARATOR_THRESHOLD_ABOVE_A	(1 << 0)
#define ADC_COMPARATOR_THRESHOLD_ABOVE_B	(1 << 1)
#define ADC_COMPARATOR_THRESHOLD_BELOW_A	(1 << 2)
#define ADC_COMPARATOR_THRESHOLD_BELOW_B	(1 << 3)
#define ADC_COMPARATOR_THRESHOLD_BETWEEN_A_B	(1 << 4)
#define ADC_COMPARATOR_THRESHOLD_OUTSIDE_A_B	(1 << 5)

#define TEMPERATURE_SENSOR			ADC_CHANNEL_6
#define MAX_NUM_THRESHOLD			(6)

extern uint32_t m_samplings_done;
extern uint8_t comparator;

extern uint32_t comp_value[MAX_NUM_THRESHOLD];

#define ADC	DT_ALIAS(test_adc)

void adc_test_reset(void);

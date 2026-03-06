/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef TEST_LPTIMER_H_
#define TEST_LPTIMER_H_

#include <zephyr/drivers/counter.h>
#include <zephyr/ztest.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#ifdef CONFIG_PM
#include <zephyr/pm/pm.h>
#include <zephyr/pm/state.h>
#endif

/* External declarations for variables defined in test_counter.c */
extern struct k_sem top_cnt_sem;
extern volatile uint32_t top_cnt;
extern struct k_sem alarm_cnt_sem;
extern volatile uint32_t alarm_cnt;
extern struct counter_alarm_cfg alarm_cfg;
extern struct counter_alarm_cfg alarm_cfg2;
extern void *exp_user_data;

#define DOWN_COUNTER 0

/*Delay in microseconds i.e 2 sec*/
#define DELAY 2000000

/*Delay in microseconds i.e 5 sec*/
#define DELAY1 5000000
#define DELAY_EXTERNAL 4000000
/*Delay is 0.0005*/
#define EXTERNALDELAY 10000000
#define LPTIMER_MAX_CHANNELS 4

#define TICKS_PER_SEC 32768
#define ALARM_CHANNEL_ID 0
#define LED0_NODE DT_ALIAS(led0)

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

#define GPIO_NODE DT_ALIAS(gpioport)

#define INT_ITER 2
#define STRESS_TEST_COUNT 3

#define DEVICE_DT_GET_AND_COMMA(node_id) DEVICE_DT_GET(node_id),
/* Generate a list of devices for all instances of the "compat" */
#define DEVS_FOR_DT_COMPAT(compat) \
	DT_FOREACH_STATUS_OKAY(compat, DEVICE_DT_GET_AND_COMMA)

#define TIMER0 DT_NODELABEL(timer0)
#define TIMER1 DT_NODELABEL(timer1)
#define TIMER2 DT_NODELABEL(timer2)
#define TIMER3 DT_NODELABEL(timer3)

#endif /* TEST_LPTIMER_H_ */

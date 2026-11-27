/*
 * Copyright (c) 2023 Alif Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "timer.h"

LOG_MODULE_REGISTER(host_timer_kernel);

#define MS_TO_US(ms) ((ms) * 1000)

static timer_cb cb_func;

static void on_timeout(struct k_timer *timer_id)
{
	(void)timer_id;
	if (cb_func) {
		cb_func();
		cb_func = NULL; /* All timeouts are one-shot */
	}
}

K_TIMER_DEFINE(alif_bt_host_timer, on_timeout, NULL);

void timer_init(void)
{
	/* The kernel timer is initialised statically, so there is nothing to do here */
}

void timer_enable(bool enable)
{
	/* The system timer is always enabled so there is nothing to do here */
	(void)enable;
}

void timer_set_timeout(uint32_t to, timer_cb cb)
{
	/* First stop any timeout that is already in progress before replacing the callback function
	 */
	k_timer_stop(&alif_bt_host_timer);

	/* If there is no callback, return from here */
	if (cb == NULL) {
		return;
	}

	cb_func = cb;

	/* The 'to' parameter is an absolute timeout, convert this to a time relative to current
	 * time
	 */
	uint32_t now = timer_get_time();
	uint32_t relative_timeout = to - now;

	LOG_DBG("ABS timeout %u us, NOW: %u us, REL timeout %u, cb %p", to, now, relative_timeout,
		cb);

	/* Start a one-shot timer for the relative timeout duration */
	k_timer_start(&alif_bt_host_timer, K_USEC(relative_timeout), K_FOREVER);
}

uint32_t timer_get_time(void)
{
	return MS_TO_US(k_uptime_get_32());
}

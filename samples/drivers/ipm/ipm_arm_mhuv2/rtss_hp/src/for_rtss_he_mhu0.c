/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/ipm.h>

#define ITERATIONS 10
const struct device *mhu0_r;
const struct device *mhu0_s;

static volatile bool msg_sent;
static volatile bool msg_received;
static uint32_t tx_msg;

static void recv_cb(const struct device *mhuv2_ipmdev, void *user_data,
		uint32_t id, volatile void *data)
{
	ARG_UNUSED(mhuv2_ipmdev);
	ARG_UNUSED(data);

	printk("RTSS-HP: MSG rcvd on ch:%d is 0x%x\n", id, *((uint32_t *)user_data));
	msg_received = true;
}

static void send_cb(const struct device *mhuv2_ipmdev, void *user_data,
		uint32_t id, volatile void *data)
{
	ARG_UNUSED(mhuv2_ipmdev);
	ARG_UNUSED(user_data);
	ARG_UNUSED(data);

	printk("RTSS-HP: MSG sent on Ch:%d is 0x%x\n", id, tx_msg);
	msg_sent = true;
}

static void send(void)
{
	int wait = 0;
	int size = 4;

	msg_received = false;
	msg_sent     = false;
	tx_msg	     = 0x12345678;

	ipm_send(mhu0_s, wait, 0, &tx_msg, size);
	while (!msg_sent)
		;
	msg_sent     = false;
	tx_msg	     = 0xA5A5FAFA;
	ipm_send(mhu0_s, wait, 1, &tx_msg, size);
	while (!msg_sent)
		;
	msg_sent = false;
}

int main(void)
{
	uint32_t recv_data;
	int i = 0;

	msg_received = false;
	msg_sent = false;
	printk("RTSS-HP RTSS-HE MHU 0 example on %s\n", CONFIG_BOARD);
	mhu0_r = DEVICE_DT_GET(DT_ALIAS(rtsshemhu0r));
	mhu0_s = DEVICE_DT_GET(DT_ALIAS(rtsshemhu0s));
	if (!device_is_ready(mhu0_r) || !device_is_ready(mhu0_s)) {
		printk("MHU devices not ready\n");
		return -1;
	}

	ipm_register_callback(mhu0_r, recv_cb, &recv_data);
	ipm_register_callback(mhu0_s, send_cb, NULL);

	/* Enable Rx MHU interrupt */
	ipm_set_enabled(mhu0_r, true);

	/* wait until receiver gets ready */
	k_sleep(K_MSEC(500));
	while (i < ITERATIONS) {
		/* Let RTSS_HP initiate data transfer */
		send();
		while (!msg_received)
			;
		++i;
	}
	/* Disable Rx MHU interrupt */
	ipm_set_enabled(mhu0_r, false);

	return 0;
}

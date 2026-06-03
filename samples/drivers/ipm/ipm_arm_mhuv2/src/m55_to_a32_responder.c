/* Copyright (C) Alif Semiconductor - All Rights Reserved.
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

#include "mhuv2_common.h"

#if defined(CONFIG_RTSS_HE)
#define M55 "M55-HE"
#else
#define M55 "M55-HP"    /* RTSS-HP specific code */
#endif

/*
 * M55 <-> A32 Communication Test
 *
 * Aliases must be defined in DTS:
 * apssmhu0r -> MHU0 Receiver (from A32)
 * apssmhu0s -> MHU0 Sender (to A32)
 * apssmhu1r -> MHU1 Receiver (from A32)
 * apssmhu1s -> MHU1 Sender (to A32)
 */

const struct device *mhu0_r;  /* Receiver from A32 */
const struct device *mhu1_r;  /* Receiver from A32 */
const struct device *volatile mhu_s;   /* Sender to A32 */
const struct device *mhu0_s;  /* Sender to A32 */
const struct device *mhu1_s;  /* Sender to A32 */

static volatile bool msg_sent;
static volatile int msg_rx_count;
static uint32_t tx_msg;

#define RX_PER_ITERATION 2

/* Callback when data received from A32 */
static void recv_cb(const struct device *dev, void *user_data,
uint32_t id, volatile void *data)
{
	ARG_UNUSED(user_data);

	if (dev == mhu0_r) {
		mhu_s = mhu0_s;
	} else if (dev == mhu1_r) {
		mhu_s = mhu1_s;
	} else {
		return;
	}

	/* Print received data */
	printk("%s: RX %s Ch%d = 0x%x\n", M55,
	       (dev == mhu0_r) ? "MHU0" : "MHU1", id, *((uint32_t *)data));
	msg_rx_count++;
}

/* Callback when data sent to A32 */
static void send_cb(const struct device *dev, void *user_data,
uint32_t id, volatile void *data)
{
	ARG_UNUSED(user_data);
	ARG_UNUSED(data);

	printk("%s: TX %s Ch%d Done\n", M55,
	       (dev == mhu0_s) ? "MHU0" : "MHU1", id);
	msg_sent = true;
}

/* Send reply to A32 */
static void send_reply(void)
{
	const struct device *dev = mhu_s;
	const char *mhu_name = (dev == mhu0_s) ? "MHU0" : "MHU1";

	/* Send 1st Message */
	msg_sent = false;
	tx_msg = 0xff23fe32;
	printk("%s: Sending 0x%x on %s Ch0...\n", M55, tx_msg, mhu_name);
	ipm_send(dev, 0, 0, &tx_msg, 4);
	while (!msg_sent) {
		k_sleep(K_MSEC(1));
	}
	/* Send 2nd Message */
	msg_sent = false;
	tx_msg = 0xff23fe33;
	printk("%s: Sending 0x%x on %s Ch0...\n", M55, tx_msg, mhu_name);
	ipm_send(dev, 0, 0, &tx_msg, 4);
	while (!msg_sent) {
		k_sleep(K_MSEC(1));
	}
}

int main(void)
{
	int i = 0;

	printk("\n====================================================================\n");
	printk("This application demonstrates M55 <-> A32 communication using MHU.\n");
	printk("M55 waits for messages from A32, then replies back.\n");
	printk("If M55 received msg from MHU0 it will reply on MHU0\n");
	printk("If M55 received msg from MHU1 it will reply on MHU1\n");
	printk("====================================================================\n");

	/* Get device handles from DTS aliases */
	if (MHU_GET_READY(mhu0_r, apssmhu0r) ||
	    MHU_GET_READY(mhu0_s, apssmhu0s) ||
	    MHU_GET_READY(mhu1_r, apssmhu1r) ||
	    MHU_GET_READY(mhu1_s, apssmhu1s)) {
		return -1;
	}

	printk("%s devices ready\n", M55);

	/* Register callbacks */
	ipm_register_callback(mhu0_r, recv_cb, NULL);
	ipm_register_callback(mhu1_r, recv_cb, NULL);
	ipm_register_callback(mhu0_s, send_cb, NULL);
	ipm_register_callback(mhu1_s, send_cb, NULL);

	/* Enable receiver interrupt */
	ipm_set_enabled(mhu0_r, true);
	ipm_set_enabled(mhu1_r, true);

	printk("Waiting for A32 messages\n\n");

	/* Main loop: wait for A32, then reply */
	while (i < ITERATIONS) {
		msg_rx_count = 0;

		/* Wait for A32 to send data */
		while (msg_rx_count < RX_PER_ITERATION) {
			k_sleep(K_MSEC(1));
		}

		/* Give a small delay to ensure A32 is ready to receive */
		k_sleep(K_MSEC(10));

		/* Send reply */
		send_reply();

		printk("Iteration %d complete\n\n", i + 1);
		++i;
	}

	/* Disable receiver */
	ipm_set_enabled(mhu0_r, false);
	ipm_set_enabled(mhu1_r, false);

	printk("Test completed successfully!\n");
	return 0;
}

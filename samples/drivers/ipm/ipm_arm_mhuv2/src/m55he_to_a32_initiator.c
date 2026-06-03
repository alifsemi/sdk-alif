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

#define M55 "M55-HE"
#define REQUEST_MSG 0xBAADF00D

const struct device *mhu0_r;
const struct device *mhu0_s;

static K_SEM_DEFINE(sem_sent, 0, 1);
static K_SEM_DEFINE(sem_received, 0, 1);
static uint32_t tx_msg;
static uint32_t rx_msg;
static const char *mhu = "MHU0";

static void recv_cb(const struct device *dev, void *user_data,
		uint32_t id, volatile void *data)
{
	ARG_UNUSED(user_data);
	ARG_UNUSED(dev);
	rx_msg = *((uint32_t *)data);

	printk("%s: RESP received on %s Ch%d = 0x%x\n\n", M55, mhu, id, rx_msg);
	k_sem_give(&sem_received);
}

static void send_cb(const struct device *dev, void *user_data,
		uint32_t id, volatile void *data)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(user_data);
	ARG_UNUSED(data);

	printk("%s: MSG sent on %s Ch:%d = 0x%x\n", M55, mhu, id, tx_msg);
	k_sem_give(&sem_sent);
}

static void send(const struct device *dev)
{
	int wait = 0;

	tx_msg = REQUEST_MSG;
	int ret = ipm_send(dev, wait, 0, &tx_msg, sizeof(tx_msg));

	if (ret != 0) {
		printk("%s: ipm_send failed on %s: %d\n", M55, mhu, ret);
		return;
	}
	k_sem_take(&sem_sent, K_FOREVER);
}

int main(void)
{
	int i = 0;

	printk("\nM55-HE initiator <-> A32 Linux responder MHU example on %s\n", CONFIG_BOARD);
	if (MHU_GET_READY(mhu0_r, apssmhu0r) ||
	    MHU_GET_READY(mhu0_s, apssmhu0s)) {
		return -1;
	}

	ipm_register_callback(mhu0_r, recv_cb, NULL);
	ipm_register_callback(mhu0_s, send_cb, NULL);

	/* Enable Rx MHU interrupt */
	ipm_set_enabled(mhu0_r, true);

	printk("Wait for A32 side to get ready...\n");
	printk("Please start the A32 responder application\n");
	k_sleep(K_SECONDS(20));  /* Let A32 side get ready */

	printk("Initiating messages to A32, then waiting for response\n");

	while (i < ITERATIONS) {
		k_sleep(K_SECONDS(1));
		printk("%s: REQ[%d] sending 0x%x on %s Ch0\n", M55, i + 1, REQUEST_MSG, mhu);
		send(mhu0_s);
		k_sem_take(&sem_received, K_FOREVER);
		++i;
	}
	/* Disable Rx MHU interrupt */
	ipm_set_enabled(mhu0_r, false);

	printk("%s Test completed successfully!\n", M55);
	return 0;
}

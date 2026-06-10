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
#include <string.h>

#define ITERATIONS 10

#if defined(CONFIG_USE_MHU1)
#define MHU_NAME "MHU1"
#define MHU_R_ALIAS rtssmhu1r
#define MHU_S_ALIAS rtssmhu1s
#else
#define MHU_NAME "MHU0"
#define MHU_R_ALIAS rtssmhu0r
#define MHU_S_ALIAS rtssmhu0s
#endif

const struct device *mhu_r;
const struct device *mhu_s;

static K_SEM_DEFINE(sem_sent, 0, 1);
static K_SEM_DEFINE(sem_received, 0, 1);
static uint32_t tx_msg;

static void recv_cb(const struct device *dev, void *user_data,
		uint32_t id, volatile void *data)
{
	ARG_UNUSED(user_data);
	ARG_UNUSED(dev);

	printk("M55-HE: MSG received on %s Ch%d = 0x%x\n", MHU_NAME, id,
	       *((uint32_t *)data));
	k_sem_give(&sem_received);
}

static void send_cb(const struct device *dev, void *user_data,
		uint32_t id, volatile void *data)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(user_data);
	ARG_UNUSED(data);

	printk("M55-HE: MSG sent on %s Ch:%d = 0x%x\n", MHU_NAME, id, tx_msg);
	k_sem_give(&sem_sent);
}

static void send(void)
{
	tx_msg = 0x12345678;
	int ret = ipm_send(mhu_s, 0, 0, &tx_msg, sizeof(tx_msg));

	if (ret != 0) {
		printk("M55-HE: ipm_send failed on %s: %d\n", MHU_NAME, ret);
		return;
	}
	k_sem_take(&sem_sent, K_FOREVER);
}

int main(void)
{
	int i = 0;

	printk("\nM55-HE <-> M55-HP %s example on %s\n", MHU_NAME, CONFIG_BOARD);
	mhu_r = DEVICE_DT_GET(DT_ALIAS(MHU_R_ALIAS));
	mhu_s = DEVICE_DT_GET(DT_ALIAS(MHU_S_ALIAS));
	if (!device_is_ready(mhu_r) || !device_is_ready(mhu_s)) {
		printk("%s devices not ready\n", MHU_NAME);
		return -1;
	}

	ipm_register_callback(mhu_r, recv_cb, NULL);
	ipm_register_callback(mhu_s, send_cb, NULL);

	/* Enable Rx MHU interrupt */
	ipm_set_enabled(mhu_r, true);

	while (i < ITERATIONS) {
		k_sleep(K_SECONDS(1));
		send();
		k_sem_take(&sem_received, K_FOREVER);
		++i;
	}
	/* Disable Rx MHU interrupt */
	ipm_set_enabled(mhu_r, false);

	printk("M55-HE: Test completed successfully!\n");
	return 0;
}

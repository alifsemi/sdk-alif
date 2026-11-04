/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/counter.h>
#include <zephyr/drivers/dma.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/sys_io.h>

#define DELAY_US		2000000
#define ALARM_CHANNEL_ID	0

#define UTIMER_CHAN_INTERRUPT_OFF	0x118U

#if !DT_NODE_HAS_STATUS(DT_NODELABEL(utimer0), okay) || \
	!DT_PROP_OR(DT_NODELABEL(utimer0), dma_trig, 0)
#error "Overlay must enable utimer0 with dma-trig"
#endif

#if !IS_ENABLED(CONFIG_COUNTER_ALIF_UTIMER_DMA)
#error "Needs CONFIG_COUNTER_ALIF_UTIMER_DMA=y in prj.conf"
#endif

#define DMA_NODE DT_NODELABEL(utimer0)
#define COUNTER_NODE DT_CHILD(DT_NODELABEL(utimer0), counter)

#if !DT_DMAS_HAS_IDX(DMA_NODE, 1)
#error "Overlay needs a second dmas entry (wrong DMA_REQ)"
#endif

BUILD_ASSERT(DT_DMAS_CELL_BY_IDX(DMA_NODE, 0, periph) !=
	     DT_DMAS_CELL_BY_IDX(DMA_NODE, 1, periph),
	     "negative dmas entry must not use the same DMA_REQ as positive");

static const struct device *counter_dev = DEVICE_DT_GET(COUNTER_NODE);
static const struct device *dma_dev = DEVICE_DT_GET(DT_DMAS_CTLR(DMA_NODE));
static const uint32_t dma_channel = DT_DMAS_CELL_BY_IDX(DMA_NODE, 0, channel);
static const uint32_t dma_slot = DT_DMAS_CELL_BY_IDX(DMA_NODE, 0, periph);
static const uint32_t dma_slot_neg = DT_DMAS_CELL_BY_IDX(DMA_NODE, 1, periph);
static const uint32_t timer_base =
	DT_REG_ADDR_BY_NAME(DT_NODELABEL(utimer0), timer);

static K_SEM_DEFINE(dma_done, 0, 1);
static volatile int dma_status;
static volatile uint32_t dma_dst;
static volatile uint32_t dma_src = 0xA5A5A5A5U;
static struct dma_block_config block;
static struct dma_config dma_cfg;
static struct counter_alarm_cfg alarm_cfg;

static void timer_eoi(void)
{
	sys_write32(BIT(ALARM_CHANNEL_ID), timer_base + UTIMER_CHAN_INTERRUPT_OFF);
}

static uint32_t ticks_to_ms(uint32_t ticks)
{
	uint32_t ms = (uint32_t)(counter_ticks_to_us(counter_dev, ticks) /
				 USEC_PER_MSEC);

	return (ms != 0U) ? ms : (DELAY_US / USEC_PER_MSEC);
}

static void dma_cb(const struct device *dev, void *user_data,
		   uint32_t channel, int status)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(user_data);
	ARG_UNUSED(channel);

	dma_status = status;
	k_sem_give(&dma_done);
}

/*
 * cancel (re-arm REQ low) → set_channel_alarm(NULL) → dma_start.
 * After DMA completes, cancel again before the next period.
 */
static int arm_dma_alarm(uint32_t slot, uint32_t ticks)
{
	int ret;

	if (ticks == 0U) {
		ticks = counter_us_to_ticks(counter_dev, DELAY_US);
		if (ticks == 0U) {
			ticks = 1U;
		}
	}

	dma_dst = 0;
	dma_status = -1;
	dma_cfg.dma_slot = slot;
	k_sem_reset(&dma_done);

	ret = dma_config(dma_dev, dma_channel, &dma_cfg);
	if (ret != 0) {
		return ret;
	}

	(void)counter_cancel_channel_alarm(counter_dev, ALARM_CHANNEL_ID);
	timer_eoi();

	alarm_cfg.flags = 0;
	alarm_cfg.ticks = ticks;
	alarm_cfg.callback = NULL;
	alarm_cfg.user_data = NULL;

	ret = counter_set_channel_alarm(counter_dev, ALARM_CHANNEL_ID, &alarm_cfg);
	if (ret != 0) {
		return ret;
	}

	return dma_start(dma_dev, dma_channel);
}

static int wait_dma_req(uint32_t slot, uint32_t ticks, int32_t wait_ms,
			int64_t *elapsed, uint32_t *status_reg)
{
	int ret;
	int64_t t0;

	ret = arm_dma_alarm(slot, ticks);
	if (ret != 0) {
		return ret;
	}

	t0 = k_uptime_get();
	ret = k_sem_take(&dma_done, K_MSEC(wait_ms));
	*elapsed = k_uptime_get() - t0;

	*status_reg = sys_read32(timer_base + UTIMER_CHAN_INTERRUPT_OFF);
	(void)dma_stop(dma_dev, dma_channel);
	(void)counter_cancel_channel_alarm(counter_dev, ALARM_CHANNEL_ID);

	return ret;
}

static bool timer_status_pending(uint32_t status_reg)
{
	return (status_reg & BIT(ALARM_CHANNEL_ID)) != 0U;
}

static bool pos_dma_ok(int err, int64_t elapsed, uint32_t alarm_ms,
		       uint32_t status_reg)
{
	return (err == 0) && (dma_status >= 0) &&
	       (elapsed >= (int64_t)(alarm_ms / 2U)) &&
	       (dma_dst == dma_src) && timer_status_pending(status_reg);
}

int main(void)
{
	int err;
	int64_t elapsed;
	uint32_t status_reg;
	uint32_t ticks;
	uint32_t base_ticks;
	uint32_t alarm_ms;
	uint32_t now_sec = 0;

	printk("UTIMER counter DMA trig\n\n");

	if (!device_is_ready(counter_dev) || !device_is_ready(dma_dev)) {
		printk("device not ready.\n");
		return 0;
	}

	counter_start(counter_dev);

	ticks = counter_us_to_ticks(counter_dev, DELAY_US);
	if (ticks == 0U) {
		ticks = 1U;
	}
	base_ticks = ticks;

	block.source_address = (uint32_t)&dma_src;
	block.dest_address = (uint32_t)&dma_dst;
	block.block_size = sizeof(dma_dst);
	block.source_addr_adj = DMA_ADDR_ADJ_NO_CHANGE;
	block.dest_addr_adj = DMA_ADDR_ADJ_NO_CHANGE;
	dma_cfg.channel_direction = PERIPHERAL_TO_MEMORY;
	dma_cfg.source_data_size = sizeof(dma_dst);
	dma_cfg.dest_data_size = sizeof(dma_dst);
	dma_cfg.source_burst_length = 1;
	dma_cfg.dest_burst_length = 1;
	dma_cfg.complete_callback_en = 1;
	dma_cfg.block_count = 1;
	dma_cfg.head_block = &block;
	dma_cfg.dma_callback = dma_cb;

	alarm_ms = ticks_to_ms(ticks);
	err = wait_dma_req(dma_slot_neg, ticks,
			   (int32_t)(alarm_ms + (2U * MSEC_PER_SEC)),
			   &elapsed, &status_reg);
	timer_eoi();
	if (err == 0) {
		printk("FAIL: DMA completed on wrong request\n");
		return 0;
	}
	if (!timer_status_pending(status_reg)) {
		printk("FAIL: timeout with no compare pending\n");
		return 0;
	}

	printk("Set alarm in %u sec (%u ticks)\n",
	       (uint32_t)(counter_ticks_to_us(counter_dev, ticks) / USEC_PER_SEC),
	       ticks);

	while (1) {
		alarm_ms = ticks_to_ms(ticks);
		err = wait_dma_req(dma_slot, ticks,
				   (int32_t)(alarm_ms + (8U * MSEC_PER_SEC)),
				   &elapsed, &status_reg);
		timer_eoi();

		if (!pos_dma_ok(err, elapsed, alarm_ms, status_reg)) {
			printk("FAIL: DMA alarm (status=0x%x err=%d)\n",
			       status_reg, err);
			return 0;
		}

		now_sec += (uint32_t)(elapsed / MSEC_PER_SEC);
		printk("!!! Alarm !!!\n");
		printk("Now: %u\n", now_sec);
		printk("UTIMER DMA trig: PASS\n");

		ticks = ticks * 2U;
		if (ticks == 0U) {
			ticks = base_ticks;
		}
		printk("Set alarm in %u sec (%u ticks)\n",
		       (uint32_t)(counter_ticks_to_us(counter_dev, ticks) /
				  USEC_PER_SEC),
		       ticks);
	}
}

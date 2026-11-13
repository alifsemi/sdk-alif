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
#define TIMER			DT_NODELABEL(timer0)
#define LPTIMER_EOI		0xc
#define LPTIMER_INTSTAT		0x10

#if !DT_NODE_HAS_STATUS_OKAY(TIMER)
#error "Overlay must enable timer0"
#endif
#if !DT_NODE_HAS_PROP(TIMER, dma_trig)
#error "Overlay must set dma-trig on timer0"
#endif
#if !DT_DMAS_HAS_IDX(TIMER, 1)
#error "Overlay needs a second dmas entry (wrong DMA_REQ)"
#endif
#if !IS_ENABLED(CONFIG_COUNTER_DW_DMA)
#error "Enable CONFIG_COUNTER_DW_DMA"
#endif

BUILD_ASSERT(DT_DMAS_CELL_BY_IDX(TIMER, 0, periph) !=
	     DT_DMAS_CELL_BY_IDX(TIMER, 1, periph),
	     "negative dmas entry must not be LPTIMER0_DMA_REQ");

static const struct device *counter_dev = DEVICE_DT_GET(TIMER);
static const struct device *dma_dev = DEVICE_DT_GET(DT_DMAS_CTLR(TIMER));
static const uint32_t dma_channel = DT_DMAS_CELL_BY_IDX(TIMER, 0, channel);
static const uint32_t dma_slot = DT_DMAS_CELL_BY_IDX(TIMER, 0, periph);
static const uint32_t dma_slot_neg = DT_DMAS_CELL_BY_IDX(TIMER, 1, periph);
static const uint32_t timer_base = DT_REG_ADDR(TIMER);

static K_SEM_DEFINE(dma_done, 0, 1);
static volatile int dma_status;
static volatile uint32_t dma_dst;
static volatile uint32_t dma_src = 0xA5A5A5A5U;
static struct dma_block_config block;
static struct dma_config dma_cfg;
static struct counter_top_cfg top_cfg;

static void timer_eoi(void)
{
	(void)sys_read32(timer_base + LPTIMER_EOI);
}

static uint32_t ticks_to_ms(uint32_t ticks)
{
	uint32_t ms = (uint32_t)(counter_ticks_to_us(counter_dev, ticks) /
				 USEC_PER_MSEC);

	return (ms != 0U) ? ms : (DELAY_US / USEC_PER_MSEC);
}

static void print_alarm(uint32_t ticks)
{
	printk("Set alarm in %u sec (%u ticks)\n",
	       (uint32_t)(counter_ticks_to_us(counter_dev, ticks) / USEC_PER_SEC),
	       ticks);
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

static int arm_dma_alarm(uint32_t slot, uint32_t ticks)
{
	int ret;

	dma_dst = 0;
	dma_status = -1;
	dma_cfg.dma_slot = slot;
	k_sem_reset(&dma_done);

	counter_stop(counter_dev);
	timer_eoi();

	ret = dma_config(dma_dev, dma_channel, &dma_cfg);
	if (ret != 0) {
		return ret;
	}

	ret = dma_start(dma_dev, dma_channel);
	if (ret != 0) {
		return ret;
	}

	top_cfg.ticks = ticks;
	return counter_set_top_value(counter_dev, &top_cfg);
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
	*status_reg = sys_read32(timer_base + LPTIMER_INTSTAT);
	(void)dma_stop(dma_dev, dma_channel);
	counter_stop(counter_dev);
	return ret;
}

static bool pos_dma_ok(int err, int64_t elapsed, uint32_t alarm_ms,
		       uint32_t status_reg)
{
	return (err == 0) && (dma_status >= 0) &&
	       (elapsed >= (int64_t)(alarm_ms / 2U)) &&
	       (dma_dst == dma_src) && (status_reg != 0U);
}

int main(void)
{
	int err;
	int64_t elapsed;
	uint32_t status_reg;
	uint32_t ticks;
	uint32_t alarm_ms;
	uint32_t now_sec = 0;

	printk("Counter DMA trig (LPTIMER)\n\n");

	if (!device_is_ready(counter_dev) || !device_is_ready(dma_dev)) {
		printk("device not ready.\n");
		return 0;
	}

	ticks = counter_us_to_ticks(counter_dev, DELAY_US);
	if (ticks == 0U) {
		ticks = 1U;
	}

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
	if (status_reg == 0U) {
		printk("FAIL: timeout with no timer pending\n");
		return 0;
	}

	print_alarm(ticks);

	while (1) {
		alarm_ms = ticks_to_ms(ticks);
		err = wait_dma_req(dma_slot, ticks,
				   (int32_t)(alarm_ms + (8U * MSEC_PER_SEC)),
				   &elapsed, &status_reg);
		timer_eoi();

		if (!pos_dma_ok(err, elapsed, alarm_ms, status_reg)) {
			printk("FAIL: DMA alarm\n");
			return 0;
		}

		now_sec += (uint32_t)(elapsed / MSEC_PER_SEC);
		printk("!!! Alarm !!!\n");
		printk("Now: %u\n", now_sec);
		printk("LPTIMER DMA trig: PASS\n");

		ticks = ticks * 2U;
		print_alarm(ticks);
	}
}

/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pinctrl.h>
#include <drivers/i2s_sync.h>
#include "i2s_sync_int.h"

LOG_MODULE_REGISTER(i2s_sync, CONFIG_I2S_SYNC_LOG_LEVEL);

#define DT_DRV_COMPAT alif_i2s_sync

#define I2S_CLK_SRC_HZ 76800000

#define I2S_CLK_DIVISOR_MAX 0x3FF
#define I2S_CLK_DIVISOR_MIN 2

struct i2s_sync_channel {
	i2s_sync_cb_t cb;
	void *buf;
	size_t samples;
	size_t count;
	size_t idx;
	bool overrun;
	bool running;
};

struct i2s_sync_data {
	struct i2s_sync_channel tx;
	struct i2s_sync_channel rx;
};

struct i2s_sync_config_priv {
	struct i2s_t *paddr;
	void (*irq_config)(const struct device *dev);
#ifdef CONFIG_PINCTRL
	const struct pinctrl_dev_config *pincfg;
#endif
	uint32_t sample_rate;
	uint32_t bit_depth;
	uint8_t channel_count;
};

static int i2s_register_cb(const struct device *dev, enum i2s_dir dir, i2s_sync_cb_t cb)
{
	struct i2s_sync_data *dev_data = (struct i2s_sync_data *)dev->data;

	if (dir == I2S_DIR_TX) {
		dev_data->tx.cb = cb;
	} else if (dir == I2S_DIR_RX) {
		dev_data->rx.cb = cb;
	} else {
		/* Not possible to register the same callback for both directions, as it would be
		 * impossible to determine within the callback which direction is triggered
		 */
		return -EINVAL;
	}

	LOG_DBG("Registered I2S callback %p for direction %d", cb, dir);

	return 0;
}

__ramfunc static void i2s_transmitter_start(const struct i2s_sync_config_priv *dev_cfg)
{
	struct i2s_t *i2s = dev_cfg->paddr;

	i2s_tx_channel_enable(i2s);
	i2s_tx_interrupt_enable(i2s);
	i2s_tx_block_enable(i2s);
	/* Should immediately get interrupt during which FIFO is filled */
}

__ramfunc static int i2s_send(const struct device *dev, void *buf, size_t len)
{
	if ((buf == NULL) || (len == 0)) {
		return -EINVAL;
	}

	const struct i2s_sync_config_priv *dev_cfg =
		(const struct i2s_sync_config_priv *)dev->config;
	struct i2s_sync_data *dev_data = (struct i2s_sync_data *)dev->data;
	struct i2s_t *i2s = dev_cfg->paddr;

	if (dev_data->tx.buf) {
		return -EINPROGRESS;
	}

	size_t bytes_per_sample = dev_cfg->bit_depth / 8U;

	if ((len % (dev_cfg->channel_count * bytes_per_sample)) != 0) {
		LOG_ERR("Invalid buffer size");
		return -EINVAL;
	}

	dev_data->tx.buf = buf;
	dev_data->tx.samples = len / bytes_per_sample;
	dev_data->tx.count = 0;
	dev_data->tx.idx = 0;

	if (!dev_data->tx.running) {
		i2s_transmitter_start(dev_cfg);
		dev_data->tx.running = true;
	} else {
		i2s_tx_interrupt_enable(i2s);
	}

	return 0;
}

__ramfunc static void i2s_receiver_start(const struct i2s_sync_config_priv *dev_cfg)
{
	struct i2s_t *i2s = dev_cfg->paddr;

	i2s_rx_channel_enable(i2s);
	i2s_rx_interrupt_enable(i2s);
	i2s_rx_block_enable(i2s);
}

__ramfunc static int i2s_recv(const struct device *dev, void *buf, size_t len)
{
	if ((buf == NULL) || (len == 0)) {
		return -EINVAL;
	}

	const struct i2s_sync_config_priv *dev_cfg =
		(const struct i2s_sync_config_priv *)dev->config;
	struct i2s_sync_data *dev_data = (struct i2s_sync_data *)dev->data;
	struct i2s_t *i2s = dev_cfg->paddr;

	if (dev_data->rx.buf) {
		return -EINPROGRESS;
	}

	size_t bytes_per_sample = dev_cfg->bit_depth / 8U;

	if ((len % (dev_cfg->channel_count * bytes_per_sample)) != 0) {
		LOG_ERR("Invalid buffer size");
		return -EINVAL;
	}

	dev_data->rx.buf = buf;
	dev_data->rx.samples = len / bytes_per_sample;
	dev_data->rx.count = 0;
	dev_data->rx.idx = 0;

	if (!dev_data->rx.running) {
		i2s_receiver_start(dev_cfg);
		dev_data->rx.running = true;
	} else {
		i2s_rx_interrupt_enable(i2s);
	}

	return 0;
}

static void channel_reset(struct i2s_sync_channel *chn)
{
	chn->buf = NULL;
	chn->samples = 0;
	chn->count = 0;
	chn->idx = 0;
}

static void channel_disable(struct i2s_sync_channel *chn)
{
	chn->running = false;
	chn->overrun = false;
	channel_reset(chn);
}

static void i2s_disable_tx(const struct device *dev)
{
	const struct i2s_sync_config_priv *dev_cfg =
		(const struct i2s_sync_config_priv *)dev->config;
	struct i2s_sync_data *dev_data = (struct i2s_sync_data *)dev->data;
	struct i2s_t *i2s = dev_cfg->paddr;

	if (dev_data->tx.running) {
		i2s_tx_channel_disable(i2s);
		i2s_tx_block_disable(i2s);
		i2s_tx_fifo_interrupt_disable(i2s);
		i2s_tx_overrun_interrupt_disable(i2s);
		i2s_tx_fifo_clear(i2s);
		channel_disable(&dev_data->tx);
	}
}

static void i2s_disable_rx(const struct device *dev)
{
	const struct i2s_sync_config_priv *dev_cfg =
		(const struct i2s_sync_config_priv *)dev->config;
	struct i2s_sync_data *dev_data = (struct i2s_sync_data *)dev->data;
	struct i2s_t *i2s = dev_cfg->paddr;

	if (dev_data->rx.running) {
		i2s_rx_channel_disable(i2s);
		i2s_rx_block_disable(i2s);
		i2s_rx_fifo_interrupt_disable(i2s);
		i2s_rx_overrun_interrupt_disable(i2s);
		i2s_rx_fifo_clear(i2s);
		channel_disable(&dev_data->rx);
	}
}

static int i2s_disable(const struct device *dev, enum i2s_dir dir)
{
	switch (dir) {
	case I2S_DIR_TX:
		i2s_disable_tx(dev);
		break;
	case I2S_DIR_RX:
		i2s_disable_rx(dev);
		break;
	case I2S_DIR_BOTH:
		i2s_disable_rx(dev);
		i2s_disable_tx(dev);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int i2s_get_config(const struct device *dev, struct i2s_sync_config *cfg)
{
	if (!dev || !cfg) {
		return -EINVAL;
	}

	const struct i2s_sync_config_priv *dev_cfg =
		(const struct i2s_sync_config_priv *)dev->config;

	cfg->sample_rate = dev_cfg->sample_rate;
	cfg->bit_depth = dev_cfg->bit_depth;
	cfg->channel_count = dev_cfg->channel_count;

	return 0;
}

static int enable_clock(const struct device *dev)
{
	const struct i2s_sync_config_priv *dev_cfg =
		(const struct i2s_sync_config_priv *)dev->config;

	i2s_select_clock_source(dev_cfg->paddr);
	i2s_enable_sclk_aon(dev_cfg->paddr);
	i2s_enable_module_clk(dev_cfg->paddr);
	i2s_global_enable(dev_cfg->paddr);
	i2s_configure_clk(dev_cfg->paddr);
	i2s_clken(dev_cfg->paddr);

	return 0;
}

static int configure_clock_source(const struct i2s_sync_config_priv *dev_cfg)
{
	/* Bit clock should be equal to channel_count * bit_depth * sample_rate */
	uint32_t bclk = 2U * dev_cfg->bit_depth * dev_cfg->sample_rate;

	uint32_t div = I2S_CLK_SRC_HZ / bclk;

	if ((div > I2S_CLK_DIVISOR_MAX) || (div < I2S_CLK_DIVISOR_MIN)) {
		LOG_ERR("Selected I2S sample rate cannot be acheieved, divisor out of range");
		return -EINVAL;
	}

	i2s_set_clock_divisor(dev_cfg->paddr, div);

	uint32_t bclk_real = I2S_CLK_SRC_HZ / div;

	if (bclk_real != bclk) {
		LOG_WRN("Selected I2S sample rate cannot be achieved, actual BCLK %u, selected %u",
			bclk_real, bclk);
	}

	return 0;
}

static int i2s_sync_init(const struct device *dev)
{
	int ret;
	const struct i2s_sync_config_priv *dev_cfg =
		(const struct i2s_sync_config_priv *)dev->config;
	struct i2s_t *i2s = dev_cfg->paddr;

	__ASSERT(dev_cfg->bit_depth == 16, "Bit depth other than 16 is not yet supported");

	ret = enable_clock(dev);
	if (ret) {
		LOG_ERR("Failed to enable clock, err %d", ret);
		return ret;
	}

#ifdef CONFIG_PINCTRL
	/* Set up pincfg if present */
	if (dev_cfg->pincfg != NULL) {
		ret = pinctrl_apply_state(dev_cfg->pincfg, PINCTRL_STATE_DEFAULT);
		if (ret) {
			LOG_ERR("I2S pinctrl failed, err %d", ret);
			return ret;
		}
	}
#endif

	/* Initialise IRQ for this instance */
	dev_cfg->irq_config(dev);

	/* Configure I2S peripheral clock */
	ret = configure_clock_source(dev_cfg);
	if (ret) {
		return ret;
	}

	/* Clear both FIFOs */
	i2s_tx_fifo_clear(i2s);
	i2s_rx_fifo_clear(i2s);

	/* Set FIFO trigger level for TX and RX */
	i2s_set_tx_trigger_level(i2s);
	i2s_set_rx_trigger_level(i2s);

	/* Set word length */
	i2s_set_rx_wlen(i2s, dev_cfg->bit_depth);
	i2s_set_tx_wlen(i2s, dev_cfg->bit_depth);

	/* Disable RX and TX channels */
	i2s_rx_channel_disable(i2s);
	i2s_tx_channel_disable(i2s);

	/* Mask all interrupts */
	i2s_interrupt_disable_all(i2s);

	return 0;
}

__ramfunc static void i2s_sync_tx_isr_handler(const struct device *dev)
{
	const struct i2s_sync_config_priv *dev_cfg = (struct i2s_sync_config_priv *)dev->config;
	struct i2s_sync_data *dev_data = (struct i2s_sync_data *)dev->data;
	struct i2s_t *i2s = dev_cfg->paddr;
	int16_t *buf = (int16_t *)dev_data->tx.buf;
	uint32_t tx_free = I2S_FIFO_TRG_LEVEL;

	while (buf && tx_free && (dev_data->tx.count < dev_data->tx.samples)) {
		/* Left channel is always output from first buffer position */
		i2s_write_left_tx(i2s, (uint32_t)buf[dev_data->tx.idx]);

		if (dev_cfg->channel_count == 1U) {
			/* In mono mode, right channel is duplicated left channel data */
			i2s_write_right_tx(i2s, (uint32_t)buf[dev_data->tx.idx]);
		} else {
#ifdef CONFIG_I2S_SYNC_BUFFER_FORMAT_SEQUENTIAL
			/* For sequential buffer format, right channel comes from the second half of
			 * buffer
			 */
			i2s_write_right_tx(
				i2s, (uint32_t)buf[dev_data->tx.idx + (dev_data->tx.samples / 2)]);
#else
			/* For interleaved buffer format, right channel comes from next sample of
			 * buffer. Buffer index must be incremented.
			 */
			i2s_write_right_tx(i2s, (uint32_t)buf[++dev_data->tx.idx]);
#endif
		}

		dev_data->tx.idx++;
		dev_data->tx.count += dev_cfg->channel_count;
		tx_free--;
	}

	if (i2s_interrupt_status_tx_overrun(i2s)) {
		/* Clear the interrupt and disable it to avoid triggering again for the same error
		 * condition. Interrupt will be re-enabled on the next call to i2s_sync_send
		 */
		i2s_tx_overrun_interrupt_disable(i2s);
		i2s_interrupt_clear_tx_overrun(i2s);
		dev_data->tx.overrun = true;
	}

	if (dev_data->tx.count == dev_data->tx.samples) {
		i2s_tx_interrupt_disable(i2s);
		dev_data->tx.buf = NULL;
		dev_data->tx.samples = 0;
		dev_data->tx.idx = 0;

		if (dev_data->tx.cb) {
			enum i2s_sync_status status =
				dev_data->tx.overrun ? I2S_SYNC_STATUS_OVERRUN : I2S_SYNC_STATUS_OK;

			dev_data->tx.cb(dev, status);
		}

		dev_data->tx.overrun = false;
	}
}

__ramfunc static void i2s_sync_rx_isr_handler(const struct device *dev)
{
	const struct i2s_sync_config_priv *dev_cfg = (struct i2s_sync_config_priv *)dev->config;
	struct i2s_sync_data *dev_data = (struct i2s_sync_data *)dev->data;
	struct i2s_t *i2s = dev_cfg->paddr;
	int16_t *buf = (int16_t *)dev_data->rx.buf;
	uint32_t rx_avail = I2S_FIFO_TRG_LEVEL;

	while (buf && rx_avail && (dev_data->rx.count < dev_data->rx.samples)) {
		/* Left channel is always placed in first buffer position */
		buf[dev_data->rx.idx] = (int16_t)i2s_read_left_rx(i2s);

		if (dev_cfg->channel_count == 1U) {
			/* In mono mode, right channel should be read and then discarded */
			(void)i2s_read_right_rx(i2s);
		} else {
#ifdef CONFIG_I2S_SYNC_BUFFER_FORMAT_SEQUENTIAL
			/* For sequential buffer format, right channel is placed in second half of
			 * buffer
			 */
			buf[dev_data->rx.idx + (dev_data->rx.samples / 2)] =
				(int16_t)i2s_read_right_rx(i2s);
#else
			/* For interleaved buffer format, right channel is placed in the next sample
			 * of buffer. Buffer index must be incremented.
			 */
			buf[++dev_data->rx.idx] = (int16_t)i2s_read_right_rx(i2s);
#endif
		}

		dev_data->rx.idx++;
		dev_data->rx.count += dev_cfg->channel_count;
		rx_avail--;
	}

	if (i2s_interrupt_status_rx_overrun(i2s)) {
		/* Clear the interrupt and disable it to avoid triggering again for the same error
		 * condition. Interrupt will be re-enabled on the next call to i2s_sync_recv
		 */
		i2s_rx_overrun_interrupt_disable(i2s);
		i2s_interrupt_clear_rx_overrun(i2s);
		dev_data->rx.overrun = true;
	}

	if (dev_data->rx.count == dev_data->rx.samples) {
		i2s_rx_interrupt_disable(i2s);
		dev_data->rx.buf = NULL;
		dev_data->rx.samples = 0;
		dev_data->rx.idx = 0;

		if (dev_data->rx.cb) {
			enum i2s_sync_status status =
				dev_data->rx.overrun ? I2S_SYNC_STATUS_OVERRUN : I2S_SYNC_STATUS_OK;

			dev_data->rx.cb(dev, status);
		}

		dev_data->rx.overrun = false;
	}
}

__ramfunc static void i2s_sync_isr(const struct device *dev)
{
	const struct i2s_sync_config_priv *dev_cfg =
		(const struct i2s_sync_config_priv *)dev->config;
	struct i2s_sync_data *dev_data = (struct i2s_sync_data *)dev->data;
	struct i2s_t *i2s = dev_cfg->paddr;

	if ((i2s_interrupt_status_tx_fifo(i2s) || i2s_interrupt_status_tx_overrun(i2s)) &&
	    dev_data->tx.running) {
		i2s_sync_tx_isr_handler(dev);
	}

	if ((i2s_interrupt_status_rx_fifo(i2s) || i2s_interrupt_status_rx_overrun(i2s)) &&
	    dev_data->rx.running) {
		i2s_sync_rx_isr_handler(dev);
	}
}

static const struct i2s_sync_driver_api i2s_sync_api = {.register_cb = i2s_register_cb,
							.send = i2s_send,
							.recv = i2s_recv,
							.disable = i2s_disable,
							.get_config = i2s_get_config};

#define I2S_SYNC_DEFINE(inst)                                                                      \
	static void i2s_sync_irq_config_func_##inst(const struct device *dev)                      \
	{                                                                                          \
		IRQ_CONNECT(DT_INST_IRQN(inst), DT_INST_IRQ(inst, priority), i2s_sync_isr,         \
			    DEVICE_DT_INST_GET(inst), 0);                                          \
		irq_enable(DT_INST_IRQN(inst));                                                    \
	}                                                                                          \
	IF_ENABLED(DT_INST_NODE_HAS_PROP(inst, pinctrl_0), (PINCTRL_DT_INST_DEFINE(inst)));        \
	static struct i2s_sync_data i2s_sync_data_##inst;                                          \
	static const struct i2s_sync_config_priv i2s_sync_config_##inst = {                        \
		.paddr = (struct i2s_t *)DT_INST_REG_ADDR(inst),                                   \
		.irq_config = i2s_sync_irq_config_func_##inst,                                     \
		IF_ENABLED(DT_INST_NODE_HAS_PROP(inst, pinctrl_0),                                 \
			   (.pincfg = PINCTRL_DT_INST_DEV_CONFIG_GET(inst),))                      \
			.sample_rate = DT_INST_PROP(inst, sample_rate),                            \
		.bit_depth = DT_INST_PROP(inst, bit_depth),                                        \
		.channel_count = DT_INST_PROP(inst, mono_mode) ? 1 : 2,                            \
	};                                                                                         \
	DEVICE_DT_INST_DEFINE(inst, i2s_sync_init, NULL, &i2s_sync_data_##inst,                    \
			      &i2s_sync_config_##inst, POST_KERNEL, CONFIG_I2S_INIT_PRIORITY,      \
			      &i2s_sync_api);

DT_INST_FOREACH_STATUS_OKAY(I2S_SYNC_DEFINE)

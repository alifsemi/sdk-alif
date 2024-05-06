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
#include <zephyr/drivers/i2s.h>
#include <zephyr/drivers/gpio.h>
#include <string.h>
#include "hello.h"

#if DT_NODE_EXISTS(DT_NODELABEL(i2s_rxtx))
#define I2S_RX_NODE  DT_NODELABEL(i2s_rxtx)
#define I2S_TX_NODE  I2S_RX_NODE
#else
#define I2S_RX_NODE  DT_NODELABEL(i2s_rx)
#define I2S_TX_NODE  DT_NODELABEL(i2s_tx)
#endif

#define SAMPLE_FREQUENCY    48000
#define SAMPLE_BIT_WIDTH    24
#define BYTES_PER_SAMPLE    sizeof(uint32_t)
#define NUMBER_OF_CHANNELS  2
/* Such block length provides an echo with the delay of 100 ms. */
#define SAMPLES_PER_BLOCK   ((SAMPLE_FREQUENCY / 10) * NUMBER_OF_CHANNELS)
#define INITIAL_BLOCKS      4
#define TIMEOUT             1000
#define BLOCK_SIZE  (BYTES_PER_SAMPLE * SAMPLES_PER_BLOCK)
#define BLOCK_COUNT (10)
K_MEM_SLAB_DEFINE_STATIC(mem_slab, BLOCK_SIZE, BLOCK_COUNT, 4);

static bool config_streams(const struct device *i2s_dev_rx,
			      const struct device *i2s_dev_tx,
			      const struct i2s_config *config)
{
	int ret;

	if (i2s_dev_rx == i2s_dev_tx) {
		ret = i2s_configure(i2s_dev_rx, I2S_DIR_BOTH, config);
		if (ret == 0) {
			return true;
		}
		/* -ENOSYS means that the RX and TX streams need to be
		 * configured separately.
		 */
		if (ret != -ENOSYS) {
			printk("Failed to configure streams: %d\n", ret);
			return false;
		}
	}

	ret = i2s_configure(i2s_dev_rx, I2S_DIR_RX, config);
	if (ret < 0) {
		printk("Failed to configure RX stream: %d\n", ret);
		return false;
	}

	ret = i2s_configure(i2s_dev_tx, I2S_DIR_TX, config);
	if (ret < 0) {
		printk("Failed to configure TX stream: %d\n", ret);
		return false;
	}
	return true;
}
static bool tx_init(const struct device *i2s_dev_rx,
			     const struct device *i2s_dev_tx)
{
	int ret;

	char *buf = (char *) &hello_samples_24bit_48khz;
	uint32_t size = sizeof(hello_samples_24bit_48khz)/
			sizeof(hello_samples_24bit_48khz[0]);
	int blocks = (size * 4) / BLOCK_SIZE;
	int offset = 0;
	void *mem_block;

	for (int i = 0; i < blocks; ++i) {

		ret = k_mem_slab_alloc(&mem_slab, &mem_block, K_NO_WAIT);
		if (ret < 0) {
			printk("Failed to allocate TX block %d: %d\n", i, ret);
			return false;
		}
		memcpy(mem_block, &buf[offset], BLOCK_SIZE);
		offset += BLOCK_SIZE;
		ret = i2s_write(i2s_dev_tx, mem_block, BLOCK_SIZE);
		if (ret < 0) {
			printk("Failed to write block %d: block_count %d %d\n",
							 i, BLOCK_COUNT, ret);
			return false;
		}
	}

	return true;
}
static bool trigger_command(const struct device *i2s_dev_rx,
			    const struct device *i2s_dev_tx,
			    enum i2s_trigger_cmd cmd)
{
	int ret;

	if (i2s_dev_rx == i2s_dev_tx) {
		ret = i2s_trigger(i2s_dev_rx, I2S_DIR_BOTH, cmd);
		if (ret == 0) {
			return true;
		}
		/* -ENOSYS means that commands for the RX and TX streams need
		 * to be triggered separately.
		 */
		if (ret != -ENOSYS) {
			printk("Failed to trigger command %d: %d\n", cmd, ret);
			return false;
		}
	}

	ret = i2s_trigger(i2s_dev_tx, I2S_DIR_TX, cmd);
	if (ret < 0) {
		printk("Failed to trigger command %d on TX: %d\n", cmd, ret);
		return false;
	}

	ret = i2s_trigger(i2s_dev_rx, I2S_DIR_RX, cmd);
	if (ret < 0) {
		printk("Failed to trigger command %d on RX: %d\n", cmd, ret);
		return false;
	}
	return true;
}

int main(void)
{
	const struct device *const i2s_dev_rx = DEVICE_DT_GET(I2S_RX_NODE);
	const struct device *const i2s_dev_tx = DEVICE_DT_GET(I2S_TX_NODE);
	struct i2s_config config;

	if (!device_is_ready(i2s_dev_rx)) {
		printk("%s is not ready\n", i2s_dev_rx->name);
		return -1;
	}

	if (i2s_dev_rx != i2s_dev_tx && !device_is_ready(i2s_dev_tx)) {
		printk("%s is not ready\n", i2s_dev_tx->name);
		return -1;
	}

	config.word_size = SAMPLE_BIT_WIDTH;
	config.channels = NUMBER_OF_CHANNELS;
	config.format = I2S_FMT_DATA_FORMAT_I2S;
	config.options = I2S_OPT_BIT_CLK_MASTER | I2S_OPT_FRAME_CLK_MASTER;
	config.frame_clk_freq = SAMPLE_FREQUENCY;
	config.mem_slab = &mem_slab;
	config.block_size = BLOCK_SIZE;
	config.timeout = TIMEOUT;

	if (!config_streams(i2s_dev_rx, i2s_dev_tx, &config)) {
		return -1;
	}

	if (!tx_init(i2s_dev_rx, i2s_dev_tx)) {
		return -1;
	}

	if (!trigger_command(i2s_dev_rx, i2s_dev_tx,
				I2S_TRIGGER_START)) {
		return -1;
	}

	printk("Streams started\n");
	while (1) {

		void *mem_block;
		uint32_t block_size;
		int ret;

		ret = i2s_read(i2s_dev_rx, &mem_block, &block_size);

		if (ret < 0) {
			printk("Failed to read data: %d\n", ret);
			break;
		}

		//process_block_data(mem_block, SAMPLES_PER_BLOCK);
		ret = i2s_write(i2s_dev_tx, mem_block, block_size);
		if (ret < 0) {
			printk("Failed to write data: %d\n", ret);
			break;
		}
	}

	if (!trigger_command(i2s_dev_rx, i2s_dev_tx,
				I2S_TRIGGER_DROP)) {
		return -1;
	}

	printk("Streams stopped\n");

	return 0;
}

/*
 * Copyright (c) 2026 Alif Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/i2s.h>
#include <zephyr/sys/printk.h>

#if DT_NODE_EXISTS(DT_NODELABEL(i2s_rxtx))
#define I2S_RX_NODE DT_NODELABEL(i2s_rxtx)
#define I2S_TX_NODE I2S_RX_NODE
#elif DT_NODE_EXISTS(DT_NODELABEL(i2s2))
#define I2S_RX_NODE DT_NODELABEL(i2s2)
#define I2S_TX_NODE I2S_RX_NODE
#elif DT_NODE_EXISTS(DT_NODELABEL(i2s3))
#define I2S_RX_NODE DT_NODELABEL(i2s3)
#define I2S_TX_NODE I2S_RX_NODE
#else
#define I2S_RX_NODE DT_NODELABEL(i2s_rx)
#define I2S_TX_NODE DT_NODELABEL(i2s_tx)
#endif

#define SAMPLE_FREQUENCY    44100
#define SAMPLE_BIT_WIDTH    16
#define BYTES_PER_SAMPLE    sizeof(int16_t)
#define NUMBER_OF_CHANNELS  2
#define BLOCK_DURATION_MS   10
#define SAMPLES_PER_BLOCK   ((SAMPLE_FREQUENCY / (1000 / BLOCK_DURATION_MS)) * \
			      NUMBER_OF_CHANNELS)

#define INITIAL_BLOCKS      8
#define TIMEOUT             1000
#define RX_DEBUG_SAMPLES    4
#define LOOP_COUNT          100

#define BLOCK_SIZE  (BYTES_PER_SAMPLE * SAMPLES_PER_BLOCK)
#define BLOCK_COUNT (INITIAL_BLOCKS + 4)

K_MEM_SLAB_DEFINE_STATIC(mem_slab, BLOCK_SIZE, BLOCK_COUNT, 4);

static uint32_t tx_sequence;
static uint32_t rx_expected_sequence;

static void fill_tx_block(void *mem_block, uint32_t number_of_samples,
			  uint32_t sequence)
{
	int16_t *samples = mem_block;

	for (uint32_t i = 0; i < number_of_samples; ++i) {
		samples[i] = (int16_t)((sequence + i) & 0x7fff); /*wrap and +ive value */
	}
}

static bool inspect_rx_block(void *mem_block, uint32_t number_of_samples,
			     uint32_t block_index, uint32_t expected_sequence)
{
	int16_t *samples = mem_block, expected;
	uint32_t dump_count = number_of_samples < RX_DEBUG_SAMPLES ?
			      number_of_samples : RX_DEBUG_SAMPLES;

	int32_t mismatch_sample = -1;
	int16_t mismatch_expected = 0;
	int16_t mismatch_got = 0;

	for (uint32_t i = 0; i < number_of_samples; ++i) {
		expected =
			(int16_t)((expected_sequence + i) & 0x7fff); /*wrap and +ive value */

		/* check RX sample with generated pattern of Tx */
		if (samples[i] != expected) {
			mismatch_sample = (int32_t)i;
			mismatch_expected = expected;
			mismatch_got = samples[i];
			break;
		}
	}

	printk("RX block %u\n", block_index);
	printk("TX:");
	for (uint32_t i = 0; i < dump_count; ++i) {
		expected = (int16_t)((expected_sequence + i) & 0x7fff);
		printk(" %d", expected);
	}
	printk("\n");

	printk("RX:");
	for (uint32_t i = 0; i < dump_count; ++i) {
		printk(" %d", samples[i]);
	}
	printk("\n");

	if (mismatch_sample >= 0) {
		printk("Verification: FAIL at sample %ld expected=%d got=%d\n",
		       (long)mismatch_sample, mismatch_expected, mismatch_got);
		return false;
	}

	printk("Verification: PASS\n");
	return true;
}

static bool configure_streams(const struct device *i2s_dev_rx,
			      const struct device *i2s_dev_tx,
			      const struct i2s_config *config)
{
	int ret;

	if (i2s_dev_rx == i2s_dev_tx) {
		ret = i2s_configure(i2s_dev_rx, I2S_DIR_BOTH, config);
		if (ret == 0) {
			return true;
		}

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

static bool queue_tx_block(const struct device *i2s_dev_tx)
{
	void *mem_block;
	int ret;

	ret = k_mem_slab_alloc(&mem_slab, &mem_block, K_FOREVER);
	if (ret < 0) {
		printk("Failed to allocate TX block: %d\n", ret);
		return false;
	}

	fill_tx_block(mem_block, SAMPLES_PER_BLOCK, tx_sequence);
	tx_sequence += SAMPLES_PER_BLOCK;

	ret = i2s_write(i2s_dev_tx, mem_block, BLOCK_SIZE);
	if (ret < 0) {
		printk("Failed to write TX block: %d\n", ret);
		k_mem_slab_free(&mem_slab, mem_block);
		return false;
	}

	return true;
}

static bool prepare_tx_queue(const struct device *i2s_dev_tx)
{
	for (uint32_t i = 0; i < INITIAL_BLOCKS; ++i) {
		if (!queue_tx_block(i2s_dev_tx)) {
			printk("Failed to queue TX block %u\n", i);
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

		if (ret != -ENOSYS) {
			printk("Failed to trigger command %d: %d\n", cmd, ret);
			return false;
		}
	}

	ret = i2s_trigger(i2s_dev_rx, I2S_DIR_RX, cmd);
	if (ret < 0) {
		printk("Failed to trigger command %d on RX: %d\n", cmd, ret);
		return false;
	}

	ret = i2s_trigger(i2s_dev_tx, I2S_DIR_TX, cmd);
	if (ret < 0) {
		printk("Failed to trigger command %d on TX: %d\n", cmd, ret);
		return false;
	}

	return true;
}

int main(void)
{
	const struct device *const i2s_dev_rx = DEVICE_DT_GET(I2S_RX_NODE);
	const struct device *const i2s_dev_tx = DEVICE_DT_GET(I2S_TX_NODE);
	struct i2s_config config;

	printk("I2S full duplex sample\n");

	if (!device_is_ready(i2s_dev_rx)) {
		printk("%s is not ready\n", i2s_dev_rx->name);
		return 0;
	}

	if (i2s_dev_rx != i2s_dev_tx && !device_is_ready(i2s_dev_tx)) {
		printk("%s is not ready\n", i2s_dev_tx->name);
		return 0;
	}

	config.word_size = SAMPLE_BIT_WIDTH;
	config.channels = NUMBER_OF_CHANNELS;
	config.format = I2S_FMT_DATA_FORMAT_I2S;
	config.options = I2S_OPT_BIT_CLK_MASTER | I2S_OPT_FRAME_CLK_MASTER;
	config.frame_clk_freq = SAMPLE_FREQUENCY;
	config.mem_slab = &mem_slab;
	config.block_size = BLOCK_SIZE;
	config.timeout = TIMEOUT;

	if (!configure_streams(i2s_dev_rx, i2s_dev_tx, &config)) {
		return 0;
	}

	if (!prepare_tx_queue(i2s_dev_tx)) {
		return 0;
	}

	if (!trigger_command(i2s_dev_rx, i2s_dev_tx, I2S_TRIGGER_START)) {
		return 0;
	}

	printk("Sample setup: slab=%p block_size=%u bytes, block_count=%u, "
	       "samples_per_block=%u, sample_frequency=%u Hz\n",
	       (void *)&mem_slab, (uint32_t)BLOCK_SIZE, (uint32_t)BLOCK_COUNT,
	       (uint32_t)SAMPLES_PER_BLOCK, (uint32_t)SAMPLE_FREQUENCY);

	printk("Streams started\n");

	for (uint32_t block_index = 0; block_index < LOOP_COUNT; ++block_index) {
		void *mem_block;
		uint32_t block_size = BLOCK_SIZE;
		int ret;

		ret = i2s_read(i2s_dev_rx, &mem_block, &block_size);
		if (ret < 0) {
			printk("Failed to read data: %d\n", ret);
			break;
		}

		if (!queue_tx_block(i2s_dev_tx)) {
			k_mem_slab_free(&mem_slab, mem_block);
			break;
		}

		if (!inspect_rx_block(mem_block, block_size / BYTES_PER_SAMPLE,
				      block_index, rx_expected_sequence)) {
			k_mem_slab_free(&mem_slab, mem_block);
			break;
		}
		rx_expected_sequence += SAMPLES_PER_BLOCK;
		k_mem_slab_free(&mem_slab, mem_block);
	}

	(void)trigger_command(i2s_dev_rx, i2s_dev_tx, I2S_TRIGGER_DROP);

	return 0;
}

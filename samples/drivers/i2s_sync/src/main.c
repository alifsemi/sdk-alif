/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/device.h>
#include <zephyr/audio/codec.h>
#include <drivers/i2s_sync.h>

LOG_MODULE_REGISTER(main);

#define AUDIO_FRAME_LEN_MS   5
#define I2S_BLOCK_COUNT      8

#define CODEC_NODE DT_ALIAS(audio_codec)
static const struct device *codec_dev = DEVICE_DT_GET(CODEC_NODE);

#define I2S_NODE DT_ALIAS(i2s_bus)
static const struct device *i2s_dev = DEVICE_DT_GET(I2S_NODE);

static volatile uint32_t tx_ctr;
static volatile uint32_t rx_ctr;
static volatile uint32_t err_ctr;
static bool trigger_tx;
static struct k_mem_slab i2s_mem;
static uint8_t *i2s_mem_buffer;

K_MSGQ_DEFINE(i2s_msgq, sizeof(void *), I2S_BLOCK_COUNT, 1);

static void finish_rx(void *rx_block)
{
	if (rx_block == NULL) {
		LOG_ERR("No RX block to finish");
		return;
	}

	/* Push completed RX block to queue so it can be sent back out */
	int ret = k_msgq_put(&i2s_msgq, &rx_block, K_NO_WAIT);

	if (ret) {
		LOG_ERR("MSGQ put failed, err %d", ret);
	}
}

static void recv_next_block(const struct device *dev)
{
	void *rx_block = NULL;
	int ret = k_mem_slab_alloc(&i2s_mem, &rx_block, K_NO_WAIT);

	if (ret || !rx_block) {
		LOG_ERR("Could not allocate RX block");
		return;
	}

	i2s_sync_recv(dev, rx_block, i2s_mem.info.block_size);
}

static void send_next_block(const struct device *dev)
{
	void *tx_block = NULL;
	int ret = k_msgq_get(&i2s_msgq, &tx_block, K_NO_WAIT);

	if (ret || !tx_block) {
		trigger_tx = true;
		return;
	}

	i2s_sync_send(dev, tx_block, i2s_mem.info.block_size);
}

static void on_i2s_rx_complete(const struct device *dev,
			       enum i2s_sync_status status,
			       void *buffer)
{
	recv_next_block(dev);
	rx_ctr++;

	if (!buffer) {
		err_ctr++;
		goto trigger_tx;
	}

	if (status != I2S_SYNC_STATUS_OK) {
		err_ctr++;
		k_mem_slab_free(&i2s_mem, buffer);
		goto trigger_tx;
	}

	finish_rx(buffer);

trigger_tx:
	if (trigger_tx) {
		trigger_tx = false;
		send_next_block(dev);
	}
}

static void on_i2s_tx_complete(const struct device *dev,
			       enum i2s_sync_status status,
			       void *buffer)
{
	send_next_block(dev);
	tx_ctr++;

	if (buffer) {
		/* Free the completed TX block so it is available for RX */
		k_mem_slab_free(&i2s_mem, buffer);
	}

	if (status != I2S_SYNC_STATUS_OK) {
		err_ctr++;
	}
}

static int audio_memslab_allocate(uint32_t const sampling_freq,
				  uint32_t const channels,
				  uint32_t const sample_size)
{
	size_t block_size =
		sampling_freq * channels * (sample_size / 8) * AUDIO_FRAME_LEN_MS / 1000;

	if (!i2s_mem_buffer) {
		i2s_mem_buffer = malloc(block_size * I2S_BLOCK_COUNT);

		if (!i2s_mem_buffer) {
			LOG_ERR("Failed to allocate memory for I2S audio data");
			return -ENOMEM;
		}
	}

	/* Initialize mem slab for audio data */
	k_mem_slab_init(&i2s_mem, i2s_mem_buffer, block_size, I2S_BLOCK_COUNT);

	return 0;
}

int main(void)
{
	LOG_INF("I2S sync demo starting");

	if (!device_is_ready(codec_dev)) {
		LOG_ERR("WM8904 codec is not ready");
		return -1;
	}

	/* Stop codec output if started during init */
	audio_codec_stop_output(codec_dev);

	/* Configure codec */
	struct audio_codec_cfg codec_cfg = {
		.dai_type = AUDIO_DAI_TYPE_I2S,
		.dai_cfg = {
			.i2s = {
				.word_size = AUDIO_PCM_WIDTH_16_BITS,
				.channels = 2,
				.format = I2S_FMT_DATA_FORMAT_I2S,
				.options = 0,
				.frame_clk_freq = 48000,
				.mem_slab = NULL,
				.block_size = 0,
				.timeout = 0,
			},
		},
	};

	if (!device_is_ready(i2s_dev)) {
		LOG_ERR("I2S driver is not ready");
		return -1;
	}

	struct i2s_sync_config i2s_cfg;

	if (i2s_sync_get_config(i2s_dev, &i2s_cfg)) {
		LOG_ERR("Failed to get I2S config");
		return -EIO;
	}

	i2s_cfg.channel_count = codec_cfg.dai_cfg.i2s.channels;
	i2s_cfg.sample_rate = codec_cfg.dai_cfg.i2s.frame_clk_freq;
	i2s_cfg.bit_depth = codec_cfg.dai_cfg.i2s.word_size;

	if (i2s_sync_configure(i2s_dev, &i2s_cfg)) {
		LOG_ERR("Failed to configure I2S");
		return -EIO;
	}

	audio_codec_start_output(codec_dev);

	i2s_sync_register_cb(i2s_dev, I2S_DIR_RX, on_i2s_rx_complete);
	i2s_sync_register_cb(i2s_dev, I2S_DIR_TX, on_i2s_tx_complete);

	while (1) {
		/* Clear msgq and reset mem slab ready to start */
		k_msgq_purge(&i2s_msgq);
		audio_memslab_allocate(i2s_cfg.sample_rate, i2s_cfg.channel_count,
				       i2s_cfg.bit_depth);
		trigger_tx = true;

		/* Start receiving data on I2S interface */
		recv_next_block(i2s_dev);

		for (int i = 0; i < 6; i++) {
			k_sleep(K_SECONDS(5));
			LOG_INF("Blocks received %u, sent %u, errors %u", rx_ctr, tx_ctr, err_ctr);
		}

		/* Every 30 seconds, stop I2S for 5 seconds to demonstrate disable API */
		i2s_sync_disable(i2s_dev, I2S_DIR_BOTH);
		LOG_INF("Stopping I2S for 5 seconds");
		k_sleep(K_SECONDS(5));
		err_ctr = 0;
		tx_ctr = 0;
		rx_ctr = 0;
	}
}

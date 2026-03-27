/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <errno.h>

#include <zephyr/audio/codec.h>
#include <zephyr/drivers/i2s.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>

#define I2S_CODEC_TX DT_ALIAS(i2s_codec_tx)

#define SAMPLE_FREQUENCY CONFIG_SAMPLE_FREQ
#define SAMPLE_BIT_WIDTH 24U
#define CHANNEL_COUNT 2U

/*
 * The DesignWare I2S driver uses 32-bit containers for samples wider than
 * 16 bits, so we stream the 24-bit clip as stereo 32-bit words.
 */
#define BYTES_PER_CHANNEL_SAMPLE sizeof(uint32_t)
#define FRAMES_PER_BLOCK 256U
#define BLOCK_SIZE (FRAMES_PER_BLOCK * CHANNEL_COUNT * BYTES_PER_CHANNEL_SAMPLE)
#define BLOCK_COUNT 8U
#define TIMEOUT_MS 2000U

#define TONE_HZ 1000U
#define TONE_HIGH_LEVEL 0x500000U
#define TONE_LOW_LEVEL 0xB00000U
#define TONE_TOGGLE_FRAMES MAX(1U, SAMPLE_FREQUENCY / (TONE_HZ * 2U))
#define TONE_TOTAL_FRAMES (SAMPLE_FREQUENCY * 2U)

K_MEM_SLAB_DEFINE_STATIC(tx_mem_slab, BLOCK_SIZE, BLOCK_COUNT, 4);

static int configure_codec(const struct device *codec_dev)
{
	struct audio_codec_cfg audio_cfg = {0};
	int ret;

	audio_cfg.dai_route = AUDIO_ROUTE_PLAYBACK;
	audio_cfg.dai_type = AUDIO_DAI_TYPE_I2S;
	audio_cfg.dai_cfg.i2s.word_size = SAMPLE_BIT_WIDTH;
	audio_cfg.dai_cfg.i2s.channels = CHANNEL_COUNT;
	audio_cfg.dai_cfg.i2s.format = I2S_FMT_DATA_FORMAT_I2S;
	audio_cfg.dai_cfg.i2s.options = I2S_OPT_FRAME_CLK_MASTER;
	audio_cfg.dai_cfg.i2s.frame_clk_freq = SAMPLE_FREQUENCY;
	audio_cfg.dai_cfg.i2s.mem_slab = &tx_mem_slab;
	audio_cfg.dai_cfg.i2s.block_size = BLOCK_SIZE;
	audio_cfg.dai_cfg.i2s.timeout = TIMEOUT_MS;

	ret = audio_codec_configure(codec_dev, &audio_cfg);
	if (ret < 0) {
		printk("Failed to configure codec: %d\n", ret);
		return ret;
	}

	audio_codec_start_output(codec_dev);

	return 0;
}

static int configure_i2s_tx(const struct device *i2s_dev)
{
	struct i2s_config i2s_cfg = {
		.word_size = SAMPLE_BIT_WIDTH,
		.channels = CHANNEL_COUNT,
		.format = I2S_FMT_DATA_FORMAT_I2S,
		.options = I2S_OPT_FRAME_CLK_MASTER | I2S_OPT_BIT_CLK_MASTER,
		.frame_clk_freq = SAMPLE_FREQUENCY,
		.mem_slab = &tx_mem_slab,
		.block_size = BLOCK_SIZE,
		.timeout = TIMEOUT_MS,
	};
	int ret;

	ret = i2s_configure(i2s_dev, I2S_DIR_TX, &i2s_cfg);
	if (ret < 0) {
		printk("Failed to configure I2S TX: %d\n", ret);
		return ret;
	}

	return 0;
}

static size_t fill_tone_block(uint32_t *tx_block, size_t sample_offset)
{
	const size_t frames = MIN((size_t)FRAMES_PER_BLOCK,
				 TONE_TOTAL_FRAMES - sample_offset);

	for (size_t i = 0; i < frames; ++i) {
		const size_t frame_index = sample_offset + i;
		const bool high = (((frame_index / TONE_TOGGLE_FRAMES) & 0x1U) == 0U);
		const uint32_t sample = high ? TONE_HIGH_LEVEL : TONE_LOW_LEVEL;

		tx_block[2U * i] = sample;
		tx_block[(2U * i) + 1U] = sample;
	}

	return frames;
}

static int play_tone_sample(const struct device *i2s_dev)
{
	size_t sample_offset = 0U;
	bool started = false;

	while (sample_offset < TONE_TOTAL_FRAMES) {
		void *tx_block;
		size_t frames;
		size_t bytes;
		int ret;

		ret = k_mem_slab_alloc(&tx_mem_slab, &tx_block, K_FOREVER);
		if (ret < 0) {
			printk("Failed to allocate TX block: %d\n", ret);
			return ret;
		}

		frames = fill_tone_block((uint32_t *)tx_block, sample_offset);
		bytes = frames * CHANNEL_COUNT * BYTES_PER_CHANNEL_SAMPLE;

		ret = i2s_write(i2s_dev, tx_block, bytes);
		if (ret < 0) {
			k_mem_slab_free(&tx_mem_slab, tx_block);
			printk("Failed to queue TX block: %d\n", ret);
			return ret;
		}

		if (!started) {
			ret = i2s_trigger(i2s_dev, I2S_DIR_TX, I2S_TRIGGER_START);
			if (ret < 0) {
				printk("Failed to start I2S TX: %d\n", ret);
				(void)i2s_trigger(i2s_dev, I2S_DIR_TX, I2S_TRIGGER_DROP);
				return ret;
			}

			started = true;
		}

		sample_offset += frames;
	}

	return i2s_trigger(i2s_dev, I2S_DIR_TX, I2S_TRIGGER_DRAIN);
}

int main(void)
{
	const struct device *const i2s_dev_codec = DEVICE_DT_GET(I2S_CODEC_TX);
	const struct device *const codec_dev = DEVICE_DT_GET(DT_NODELABEL(audio_codec));
	int ret;

	printk("Playing tone sample at %u Hz\n", SAMPLE_FREQUENCY);

	if (!device_is_ready(i2s_dev_codec)) {
		printk("%s is not ready\n", i2s_dev_codec->name);
		return -ENODEV;
	}

	if (!device_is_ready(codec_dev)) {
		printk("%s is not ready\n", codec_dev->name);
		return -ENODEV;
	}

	ret = configure_codec(codec_dev);
	if (ret < 0) {
		return ret;
	}

	k_msleep(1000);

	ret = configure_i2s_tx(i2s_dev_codec);
	if (ret < 0) {
		audio_codec_stop_output(codec_dev);
		return ret;
	}

	ret = play_tone_sample(i2s_dev_codec);
	if (ret < 0) {
		audio_codec_stop_output(codec_dev);
		printk("Tone sample playback failed: %d\n", ret);
		return ret;
	}

	audio_codec_stop_output(codec_dev);
	printk("Tone sample playback complete\n");

	return 0;
}

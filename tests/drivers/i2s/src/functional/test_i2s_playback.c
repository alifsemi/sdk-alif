/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */
#include <zephyr/ztest.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/i2s.h>
#include <zephyr/audio/codec.h>
#include <zephyr/sys/util.h>
#include <string.h>
#include "test_i2s.h"
#include "test_i2s_common.h"

LOG_MODULE_REGISTER(i2s_playback_test, LOG_LEVEL_INF);

#define PLAY_BLOCK_COUNT  4

static char __aligned(WB_UP(4)) __unused
	_play_slab_buf[PLAY_BLOCK_COUNT * WB_UP(BLOCK_SIZE)];
static struct k_mem_slab __unused play_slab;

static int __unused play_slab_init(void)
{
	return k_mem_slab_init(&play_slab, _play_slab_buf,
			       WB_UP(BLOCK_SIZE), PLAY_BLOCK_COUNT);
}

static bool __unused triggerI2STxCommand(const struct device *i2s_dev_tx,
				enum i2s_trigger_cmd cmd)
{
	int ret;

	ret = i2s_trigger(i2s_dev_tx, I2S_DIR_TX, cmd);
	if (ret < 0) {
		printk("Failed to trigger command %d on TX: %d\n", cmd, ret);
		return false;
	}

	return true;
}

#if IS_ENABLED(CONFIG_I2S_PLAY_HELLO)

#define PLAY_THREAD_STACK_SIZE  1024
#define PLAY_THREAD_PRIORITY    5
#define PLAY_DURATION_MS        10000U
#define PLAY_JOIN_TIMEOUT_MS    2000U

static K_THREAD_STACK_DEFINE(play_thread_stack, PLAY_THREAD_STACK_SIZE);
static struct k_thread play_thread_data;

/* Semaphore: test waits until playback thread has successfully started TX */
static K_SEM_DEFINE(play_started_sem, 0, 1);

/* Stop request flag set by the test to ask the playback thread to exit */
static atomic_t play_stop_request;

static void play_thread_fn(void *arg1, void *arg2, void *arg3)
{
	const struct device *i2s_dev_tx = (const struct device *)arg1;
	const uint8_t *const buf = (const uint8_t *)hello_samples_24bit_48khz;
	const size_t buf_len = sizeof(hello_samples_24bit_48khz);
	size_t offset = 0U;
	bool started = false;
	int ret;

	ARG_UNUSED(arg2);
	ARG_UNUSED(arg3);

	while (atomic_get(&play_stop_request) == 0) {
		void *mem_block;
		size_t copy_len;

		ret = k_mem_slab_alloc(&play_slab, &mem_block, K_MSEC(10));
		if (ret < 0) {
			k_sleep(K_MSEC(1));
			continue;
		}

		copy_len = MIN(BLOCK_SIZE, buf_len - offset);
		memcpy(mem_block, &buf[offset], copy_len);
		if (copy_len < BLOCK_SIZE) {
			memcpy((uint8_t *)mem_block + copy_len, buf,
			       BLOCK_SIZE - copy_len);
			offset = BLOCK_SIZE - copy_len;
		} else {
			offset += copy_len;
			if (offset >= buf_len) {
				offset = 0U;
			}
		}

		ret = i2s_write(i2s_dev_tx, mem_block, BLOCK_SIZE);
		if (ret < 0) {
			printk("Failed to write TX block: %d\n", ret);
			k_mem_slab_free(&play_slab, mem_block);
			break;
		}

		if (!started) {
			if (!triggerI2STxCommand(i2s_dev_tx,
						 I2S_TRIGGER_START)) {
				printk("Failed to start I2S TX\n");
				break;
			}
			started = true;
			k_sem_give(&play_started_sem);
		}
	}
}

#endif /* CONFIG_I2S_PLAY_HELLO */

#if IS_ENABLED(CONFIG_I2S_PLAY_HELLO_CODEC)

#define I2S_CODEC_TX_NODE  DT_ALIAS(i2s_codec_tx)

#define CODEC_PLAY_THREAD_STACK_SIZE  1024
#define CODEC_PLAY_THREAD_PRIORITY    5
#define CODEC_PLAY_DURATION_MS        10000U
#define CODEC_PLAY_JOIN_TIMEOUT_MS    2000U

static K_THREAD_STACK_DEFINE(codec_play_thread_stack, CODEC_PLAY_THREAD_STACK_SIZE);
static struct k_thread codec_play_thread_data;

static K_SEM_DEFINE(codec_play_started_sem, 0, 1);
static atomic_t codec_play_stop_request;

K_MEM_SLAB_DEFINE_STATIC(codec_play_slab, WB_UP(BLOCK_SIZE), PLAY_BLOCK_COUNT, 4);

static void codec_play_thread_fn(void *arg1, void *arg2, void *arg3)
{
	const struct device *i2s_dev = (const struct device *)arg1;
	const uint8_t *const buf = (const uint8_t *)hello_samples_24bit_48khz;
	const size_t buf_len = sizeof(hello_samples_24bit_48khz);
	size_t offset = 0U;
	bool started = false;
	int ret;

	ARG_UNUSED(arg2);
	ARG_UNUSED(arg3);

	while (atomic_get(&codec_play_stop_request) == 0) {
		void *mem_block;
		size_t copy_len;

		ret = k_mem_slab_alloc(&codec_play_slab, &mem_block, K_MSEC(10));
		if (ret < 0) {
			k_sleep(K_MSEC(1));
			continue;
		}

		copy_len = MIN(BLOCK_SIZE, buf_len - offset);
		memcpy(mem_block, &buf[offset], copy_len);
		if (copy_len < BLOCK_SIZE) {
			memcpy((uint8_t *)mem_block + copy_len, buf,
			       BLOCK_SIZE - copy_len);
			offset = BLOCK_SIZE - copy_len;
		} else {
			offset += copy_len;
			if (offset >= buf_len) {
				offset = 0U;
			}
		}

		ret = i2s_write(i2s_dev, mem_block, BLOCK_SIZE);
		if (ret < 0) {
			printk("Codec play: failed to write TX block: %d\n", ret);
			k_mem_slab_free(&codec_play_slab, mem_block);
			break;
		}

		if (!started) {
			ret = i2s_trigger(i2s_dev, I2S_DIR_TX, I2S_TRIGGER_START);
			if (ret < 0) {
				printk("Codec play: failed to start I2S TX: %d\n", ret);
				break;
			}
			started = true;
			k_sem_give(&codec_play_started_sem);
		}
	}
}

#endif /* CONFIG_I2S_PLAY_HELLO_CODEC */

static bool i2s_playback_predicate(const void *state)
{
	ARG_UNUSED(state);
	return true;
}

ZTEST_SUITE(i2s_functional, i2s_playback_predicate, NULL, NULL, NULL, NULL);

ZTEST(i2s_functional, test_PlayHelloSamples48k_TxOnly_10s)
{
#if IS_ENABLED(CONFIG_I2S_PLAY_HELLO)
	const struct device *const i2s_dev_tx = DEVICE_DT_GET(I2S_TX_NODE);
	struct i2s_config config;
	int ret;

	zassert_true(device_is_ready(i2s_dev_tx), "%s is not ready", i2s_dev_tx->name);

	config.word_size = SAMPLE_BIT_WIDTH;
	config.channels = NUMBER_OF_CHANNELS;
	config.format = I2S_FMT_DATA_FORMAT_I2S;
	config.options = I2S_OPT_BIT_CLK_MASTER | I2S_OPT_FRAME_CLK_MASTER;
	config.frame_clk_freq = SAMPLE_FREQUENCY;
	play_slab_init();

	config.mem_slab = &play_slab;
	config.block_size = BLOCK_SIZE;
	config.timeout = TIMEOUT;

	ret = i2s_configure(i2s_dev_tx, I2S_DIR_TX, &config);
	zassert_equal(ret, 0, "Failed to configure TX stream: %d", ret);

	(void)triggerI2STxCommand(i2s_dev_tx, I2S_TRIGGER_DROP);

	/* Reset state from any previous run so the test is re-runnable. */
	atomic_set(&play_stop_request, 0);
	k_sem_reset(&play_started_sem);

	/* Spawn background thread for bounded playback. */
	k_thread_create(&play_thread_data, play_thread_stack,
			K_THREAD_STACK_SIZEOF(play_thread_stack),
			play_thread_fn,
			(void *)i2s_dev_tx, NULL, NULL,
			PLAY_THREAD_PRIORITY, 0, K_NO_WAIT);
	k_thread_name_set(&play_thread_data, "i2s_play");

	/* Wait until TX has actually started before timing the run. */
	ret = k_sem_take(&play_started_sem, K_MSEC(2000));
	zassert_equal(ret, 0, "I2S TX did not start within timeout");

	LOG_INF("I2S TX playing hello samples for %u ms\n",
		 (unsigned int)PLAY_DURATION_MS);
	k_sleep(K_MSEC(PLAY_DURATION_MS));

	/* Ask the playback thread to stop, then drop the TX stream so any
	 * in-flight i2s_write() unblocks and queued blocks are released back
	 * to the slab.
	 */
	atomic_set(&play_stop_request, 1);
	(void)triggerI2STxCommand(i2s_dev_tx, I2S_TRIGGER_DROP);

	ret = k_thread_join(&play_thread_data, K_MSEC(PLAY_JOIN_TIMEOUT_MS));
	zassert_equal(ret, 0, "Playback thread did not exit in time: %d", ret);

	LOG_INF("I2S TX playback stopped cleanly\n");
#else
	LOG_INF("Skipping: CONFIG_TEST_I2S_PLAY_HELLO is disabled\n");
	ztest_test_skip();
#endif
}

ZTEST(i2s_functional, test_PlayHelloSamples48k_Codec_10s)
{
#if IS_ENABLED(CONFIG_I2S_PLAY_HELLO_CODEC)
	const struct device *const i2s_dev = DEVICE_DT_GET(I2S_CODEC_TX_NODE);
	const struct device *const codec_dev = DEVICE_DT_GET(DT_NODELABEL(audio_codec));
	struct audio_codec_cfg audio_cfg = {0};
	struct i2s_config i2s_cfg;
	int ret;

	zassert_true(device_is_ready(i2s_dev), "%s is not ready", i2s_dev->name);
	zassert_true(device_is_ready(codec_dev), "%s is not ready", codec_dev->name);

	/* Configure WM8904 codec for 48 kHz / 24-bit stereo playback. */
	audio_cfg.dai_route = AUDIO_ROUTE_PLAYBACK;
	audio_cfg.dai_type = AUDIO_DAI_TYPE_I2S;
	audio_cfg.dai_cfg.i2s.word_size = SAMPLE_BIT_WIDTH;
	audio_cfg.dai_cfg.i2s.channels = NUMBER_OF_CHANNELS;
	audio_cfg.dai_cfg.i2s.format = I2S_FMT_DATA_FORMAT_I2S;
	audio_cfg.dai_cfg.i2s.options = I2S_OPT_FRAME_CLK_MASTER;
	audio_cfg.dai_cfg.i2s.frame_clk_freq = SAMPLE_FREQUENCY;
	audio_cfg.dai_cfg.i2s.mem_slab = &codec_play_slab;
	audio_cfg.dai_cfg.i2s.block_size = BLOCK_SIZE;
	audio_cfg.dai_cfg.i2s.timeout = TIMEOUT;

	ret = audio_codec_configure(codec_dev, &audio_cfg);
	zassert_equal(ret, 0, "Failed to configure codec: %d", ret);

	audio_codec_start_output(codec_dev);

	/* Allow codec PLL to stabilise before clocking data. */
	k_sleep(K_MSEC(100));

	/* Configure I2S TX with matching settings. */
	i2s_cfg.word_size = SAMPLE_BIT_WIDTH;
	i2s_cfg.channels = NUMBER_OF_CHANNELS;
	i2s_cfg.format = I2S_FMT_DATA_FORMAT_I2S;
	i2s_cfg.options = I2S_OPT_BIT_CLK_MASTER | I2S_OPT_FRAME_CLK_MASTER;
	i2s_cfg.frame_clk_freq = SAMPLE_FREQUENCY;
	i2s_cfg.mem_slab = &codec_play_slab;
	i2s_cfg.block_size = BLOCK_SIZE;
	i2s_cfg.timeout = TIMEOUT;

	ret = i2s_configure(i2s_dev, I2S_DIR_TX, &i2s_cfg);
	zassert_equal(ret, 0, "Failed to configure I2S TX: %d", ret);

	(void)i2s_trigger(i2s_dev, I2S_DIR_TX, I2S_TRIGGER_DROP);

	/* Reset state so the test is re-runnable. */
	atomic_set(&codec_play_stop_request, 0);
	k_sem_reset(&codec_play_started_sem);

	/* Spawn background playback thread. */
	k_thread_create(&codec_play_thread_data, codec_play_thread_stack,
			K_THREAD_STACK_SIZEOF(codec_play_thread_stack),
			codec_play_thread_fn,
			(void *)i2s_dev, NULL, NULL,
			CODEC_PLAY_THREAD_PRIORITY, 0, K_NO_WAIT);
	k_thread_name_set(&codec_play_thread_data, "i2s_codec_play");

	/* Wait until TX has actually started before timing the run. */
	ret = k_sem_take(&codec_play_started_sem, K_MSEC(2000));
	zassert_equal(ret, 0, "I2S TX (codec) did not start within timeout");

	LOG_INF("I2S TX (codec) playing hello samples for %u ms\n",
		(unsigned int)CODEC_PLAY_DURATION_MS);
	k_sleep(K_MSEC(CODEC_PLAY_DURATION_MS));

	/* Ask the playback thread to stop and drop the TX stream. */
	atomic_set(&codec_play_stop_request, 1);
	(void)i2s_trigger(i2s_dev, I2S_DIR_TX, I2S_TRIGGER_DROP);

	ret = k_thread_join(&codec_play_thread_data, K_MSEC(CODEC_PLAY_JOIN_TIMEOUT_MS));
	zassert_equal(ret, 0, "Codec playback thread did not exit in time: %d", ret);

	audio_codec_stop_output(codec_dev);

	LOG_INF("I2S TX (codec) playback stopped cleanly\n");
#else
	LOG_INF("Skipping: CONFIG_I2S_PLAY_HELLO_CODEC is disabled\n");
	ztest_test_skip();
#endif
}

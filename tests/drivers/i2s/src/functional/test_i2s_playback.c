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
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/i2s.h>
#include <zephyr/sys/util.h>
#include <string.h>
#include "test_i2s.h"
#include "test_i2s_common.h"

#define PLAY_BLOCK_COUNT  4

static char __aligned(WB_UP(4))
	_play_slab_buf[PLAY_BLOCK_COUNT * WB_UP(BLOCK_SIZE)];
static struct k_mem_slab play_slab;

static int play_slab_init(void)
{
	return k_mem_slab_init(&play_slab, _play_slab_buf,
			       WB_UP(BLOCK_SIZE), PLAY_BLOCK_COUNT);
}

static bool triggerI2STxCommand(const struct device *i2s_dev_tx,
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

ZTEST_SUITE(i2s_functional, NULL, NULL, NULL, NULL, NULL);

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

	TC_PRINT("I2S TX playing hello samples for %u ms\n",
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

	TC_PRINT("I2S TX playback stopped cleanly\n");
#else
	TC_PRINT("Skipping: CONFIG_TEST_I2S_PLAY_HELLO is disabled\n");
	ztest_test_skip();
#endif
}

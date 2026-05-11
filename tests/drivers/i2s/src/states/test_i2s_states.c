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
#include <zephyr/drivers/gpio.h>
#include <string.h>
#include "test_i2s.h"
#include "test_i2s_common.h"


#if DT_NODE_EXISTS(DT_NODELABEL(i2s_rxtx))
#define I2S_RX_NODE  DT_NODELABEL(i2s_rxtx)
#define I2S_TX_NODE  I2S_RX_NODE
#else
#define I2S_RX_NODE  DT_NODELABEL(i2s_rx)
#define I2S_TX_NODE  DT_NODELABEL(i2s_tx)
#endif
#define enab 0
#define errorChecksEnabled 0

int Disable_reception = 1;


static bool config_streams(const struct device *i2s_dev_rx,
			      const struct device *i2s_dev_tx,
			      const struct i2s_config *config)
{
	int ret;

	if (i2s_dev_rx == i2s_dev_tx) {
		ret = i2s_configure(i2s_dev_rx, I2S_DIR_BOTH, config);
		if (ret == 0) {
			/* Driver fully supports DIR_BOTH; nothing more to do. */
			return true;
		}
		if ((ret != -ENOSYS) && (ret != -ENOTSUP)) {
			printk("I2S_DIR_BOTH configure failed: %d\n", ret);
			return false;
		}
		/* -ENOSYS / -ENOTSUP means RX and TX must be configured
		 * separately; fall through to the per-direction calls below.
		 */
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

static bool transmitInit44(const struct device *i2s_dev_rx,
			     const struct device *i2s_dev_tx,
			     const struct i2s_config *config)
{
	int ret;
	char *buf = (char *)&hello_samples_24bit_48khz;
	uint32_t size = sizeof(hello_samples_24bit_48khz) /
			sizeof(hello_samples_24bit_48khz[0]);
	int blocks;
	int offset = 0;
	void *mem_block;

	blocks = (size * 4) / BLOCK_SIZE44;

	for (int i = 0; i < blocks; ++i) {

		ret = k_mem_slab_alloc(&mem_slab, &mem_block, K_NO_WAIT);
		if (ret < 0) {
			printk("Failed to allocate TX block %d: %d\n", i, ret);
			return false;
		}
		memcpy(mem_block, &buf[offset], BLOCK_SIZE44);
		offset += BLOCK_SIZE44;
		ret = i2s_write(i2s_dev_tx, mem_block, BLOCK_SIZE44);
		if (ret < 0) {
			printk("Failed to write block %d: block_count %d %d\n",
			       i, BLOCK_COUNT, ret);
			k_mem_slab_free(&mem_slab, mem_block);
			return false;
		}
	}
	return true;
}

static bool transmitInit(const struct device *i2s_dev_rx,
			     const struct device *i2s_dev_tx,
			     const struct i2s_config *config)
{
	int ret;
	char *buf = (char *)&hello_samples_24bit_48khz;
	uint32_t size = sizeof(hello_samples_24bit_48khz) /
			sizeof(hello_samples_24bit_48khz[0]);
	int blocks;
	int offset = 0;
	void *mem_block;

	blocks = (size * 4) / BLOCK_SIZE;

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
			k_mem_slab_free(&mem_slab, mem_block);
			return false;
		}
	}

	return true;
}
static bool triggerI2SCommand(const struct device *i2s_dev_rx,
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
			zassert_equal(ret, -ENOSYS, "Failed to trigger command %d: %d\n", cmd, ret);
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


static int transmitData(const struct device *i2s_dev_rx,
			      const struct device *i2s_dev_tx,
			      const struct i2s_config *config)
{
	int i = 0;

	zassert_true(config_streams(i2s_dev_rx, i2s_dev_tx, config));
	zassert_true(transmitInit(i2s_dev_rx, i2s_dev_tx, config), "transition failed");
	zassert_true(triggerI2SCommand(i2s_dev_rx, i2s_dev_tx, I2S_TRIGGER_START));

	printk("Streams started\n");

	while (i < 5) {
		void *mem_block;
		uint32_t block_size;
		int ret;

		ret = i2s_read(i2s_dev_rx, &mem_block, &block_size);
		if (ret < 0) {
			printk("Failed to read data: %d\n", ret);
			break;
		}
		ret = i2s_write(i2s_dev_tx, mem_block, block_size);
		if (ret < 0) {
			printk("Failed to write data: %d\n", ret);
			break;
		}
		i++;
	}
	zassert_true(triggerI2SCommand(i2s_dev_rx, i2s_dev_tx, I2S_TRIGGER_DRAIN),
		     "Trigger drain failed");
	zassert_true(triggerI2SCommand(i2s_dev_rx, i2s_dev_tx, I2S_TRIGGER_DROP),
		     "Trigger command failed");
	printk("Streams stopped\n");

	return 0;
}

static int TransmitReceiveFunc(const struct device *i2s_dev_rx,
			      const struct device *i2s_dev_tx,
			      const struct i2s_config *config)
{
	uint32_t size = sizeof(hello_samples_24bit_48khz) /
			sizeof(hello_samples_24bit_48khz[0]);
	int blocks = (size * 4) / BLOCK_SIZE;

	zassert_true(config_streams(i2s_dev_rx, i2s_dev_tx, config));

	for (int i = 0; i < blocks; ++i) {
		void *mem_block;
		int ret = k_mem_slab_alloc(&mem_slab, &mem_block, K_NO_WAIT);

		if (ret < 0) {
			printk("Failed to allocate TX block %d: %d\n", i, ret);
			return false;
		}

		memset(mem_block, 0, BLOCK_SIZE);

		ret = i2s_write(i2s_dev_tx, mem_block, BLOCK_SIZE);
		if (ret < 0) {
			printk("Failed to write block %d: %d\n", i, ret);
			return false;
		}
	}
	zassert_true(triggerI2SCommand(i2s_dev_rx, i2s_dev_tx, I2S_TRIGGER_START));
	printk("Streams started\n");

	for (int i = 0; i < blocks; i++) {
		void *mem_block;
		uint32_t block_size;
		int ret;

		ret = i2s_read(i2s_dev_rx, &mem_block, &block_size);
		if (ret < 0) {
			printk("Failed to read data: %d\n", ret);
			return ret;
		}
		ret = i2s_write(i2s_dev_tx, mem_block, block_size);
		if (ret < 0) {
			printk("Failed to write data: %d\n", ret);
			return ret;
		}
	}
	zassert_true(triggerI2SCommand(i2s_dev_rx, i2s_dev_tx, I2S_TRIGGER_DROP),
		     "Trigger command failed");
	printk("Streams stopped\n");
	return 0;

}

static int TransmitReceiveFunc44(const struct device *i2s_dev_rx,
			      const struct device *i2s_dev_tx,
			      const struct i2s_config *config)
{
	uint32_t size = sizeof(hello_samples_24bit_48khz) /
			sizeof(hello_samples_24bit_48khz[0]);
	int blocks;

	blocks = (size * 4) / BLOCK_SIZE44;

	void *mem_block;

	zassert_true(config_streams(i2s_dev_rx, i2s_dev_tx, config));
	for (int i = 0; i < blocks; ++i) {
		void *mem_block;
		int ret = k_mem_slab_alloc(&mem_slab, &mem_block, K_NO_WAIT);

		if (ret < 0) {
			printk("Failed to allocate TX block %d: %d\n", i, ret);
			return false;
		}

		memset(mem_block, 0, BLOCK_SIZE44);

		ret = i2s_write(i2s_dev_tx, mem_block, BLOCK_SIZE44);
		if (ret < 0) {
			printk("Failed to write block %d: %d\n", i, ret);
			return false;
		}
	}

	zassert_true(triggerI2SCommand(i2s_dev_rx, i2s_dev_tx, I2S_TRIGGER_START));

	printk("Streams started\n");

	for (int i = 0; i < blocks; i++) {
		void *mem_block;
		uint32_t block_size;
		int ret;

		ret = i2s_read(i2s_dev_rx, &mem_block, &block_size);
		if (ret < 0) {
			printk("Failed to read data: %d\n", ret);
			(void)triggerI2SCommand(i2s_dev_rx, i2s_dev_tx,
						I2S_TRIGGER_DROP);
			return ret;
		}

		ret = i2s_write(i2s_dev_tx, mem_block, block_size);
		if (ret < 0) {
			printk("Failed to write data: %d\n", ret);
			(void)triggerI2SCommand(i2s_dev_rx, i2s_dev_tx,
						I2S_TRIGGER_DROP);
			return ret;
		}
	}

	zassert_true(triggerI2SCommand(i2s_dev_rx, i2s_dev_tx, I2S_TRIGGER_DROP),
		     "Trigger command failed");
	printk("Streams stopped\n");
	return 0;
}




ZTEST_SUITE(i2s_states, NULL, NULL, NULL, NULL, NULL);

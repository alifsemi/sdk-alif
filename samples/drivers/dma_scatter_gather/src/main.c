/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/*
 * DMA Scatter-Gather Test Application
 *
 * This application demonstrates DMA scatter-gather functionality by transferring
 * three separate blocks of data from source buffers to destination buffers.
 */

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(dma_sg_test, LOG_LEVEL_INF);

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/dma.h>
#include <zephyr/sys/atomic.h>
#include <string.h>
#include <errno.h>
#include <soc_common.h>

/* DMA device and channel configuration */
#define DMA_NODE    DT_ALIAS(test_dma)

#if !DT_NODE_EXISTS(DMA_NODE)
#error "test-dma devicetree alias is not defined; apply the alif-dma snippet"
#elif !DT_NODE_HAS_STATUS_OKAY(DMA_NODE)
#error "test-dma devicetree node is disabled; enable it in your overlay"
#endif

#define DMA_CHANNEL 0

/* Block sizes for scatter-gather transfer */
#define BLOCK1_SIZE 64
#define BLOCK2_SIZE 128
#define BLOCK3_SIZE 32

/* Source and destination buffers, aligned for DMA word size */
static uint8_t src1[BLOCK1_SIZE] __aligned(sizeof(uint32_t));
static uint8_t src2[BLOCK2_SIZE] __aligned(sizeof(uint32_t));
static uint8_t src3[BLOCK3_SIZE] __aligned(sizeof(uint32_t));

static uint8_t dst1[BLOCK1_SIZE] __aligned(sizeof(uint32_t));
static uint8_t dst2[BLOCK2_SIZE] __aligned(sizeof(uint32_t));
static uint8_t dst3[BLOCK3_SIZE] __aligned(sizeof(uint32_t));

/* Synchronization primitives for DMA completion */
static K_SEM_DEFINE(dma_done, 0, 1);
static atomic_t dma_error = ATOMIC_INIT(0);

/*
 * DMA completion callback function
 *
 * Called when DMA transfer completes or encounters an error.
 * Signals the main thread via semaphore.
 */
static void dma_callback(const struct device *dev, void *user_data, uint32_t channel, int status)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(user_data);

	if (status < 0) {
		LOG_ERR("DMA error on channel %d: %d", channel, status);
		atomic_set(&dma_error, status);
	} else {
		LOG_INF("DMA scatter-gather complete on channel %d", channel);
		atomic_set(&dma_error, 0);
	}
	k_sem_give(&dma_done);
}

/*
 * Fill a buffer with a pattern
 *
 * Fills the buffer with incrementing values starting from the given pattern.
 */
static void fill_buffer(uint8_t *buf, size_t size, uint8_t pattern)
{
	for (size_t i = 0; i < size; i++) {
		buf[i] = pattern + (uint8_t)i;
	}
}

/*
 * Clear destination buffers
 *
 * Sets all destination buffers to zero to ensure clean verification.
 */
static void clear_buffers(void)
{
	memset(dst1, 0, sizeof(dst1));
	memset(dst2, 0, sizeof(dst2));
	memset(dst3, 0, sizeof(dst3));
}

/*
 * Setup scatter-gather block configurations
 *
 * Creates a linked list of DMA block configurations for the three transfer blocks.
 * Each block specifies source/destination addresses, size, and address adjustment.
 */
static int setup_scatter_gather_blocks(struct dma_block_config *blocks)
{
	/* Block 3 (last in chain) */
	blocks[2] = (struct dma_block_config){
		.source_address = (uintptr_t)src3,
		.dest_address = (uintptr_t)dst3,
		.block_size = BLOCK3_SIZE,
		.source_addr_adj = DMA_ADDR_ADJ_INCREMENT,
		.dest_addr_adj = DMA_ADDR_ADJ_INCREMENT,
		.next_block = NULL,
	};

	/* Block 2 */
	blocks[1] = (struct dma_block_config){
		.source_address = (uintptr_t)src2,
		.dest_address = (uintptr_t)dst2,
		.block_size = BLOCK2_SIZE,
		.source_addr_adj = DMA_ADDR_ADJ_INCREMENT,
		.dest_addr_adj = DMA_ADDR_ADJ_INCREMENT,
		.next_block = &blocks[2],
	};

	/* Block 1 (head) */
	blocks[0] = (struct dma_block_config){
		.source_address = (uintptr_t)src1,
		.dest_address = (uintptr_t)dst1,
		.block_size = BLOCK1_SIZE,
		.source_addr_adj = DMA_ADDR_ADJ_INCREMENT,
		.dest_addr_adj = DMA_ADDR_ADJ_INCREMENT,
		.next_block = &blocks[1],
	};

	return 0;
}

/*
 * Configure and start DMA transfer
 *
 * Sets up DMA configuration with scatter-gather blocks and starts the transfer.
 */
static int configure_and_start_dma(const struct device *dma_dev,
				 struct dma_block_config *head_block)
{
	struct dma_config dma_cfg = {
		.channel_direction = MEMORY_TO_MEMORY,
		.source_data_size = 4,
		.dest_data_size = 4,
		.source_burst_length = 1,
		.dest_burst_length = 1,	/* burst=1 keeps alignment simple for this test */
		.dma_slot = 0,
		.block_count = 3,
		.head_block = head_block,
		.dma_callback = dma_callback,
		.user_data = NULL,
	};

	int ret = dma_config(dma_dev, DMA_CHANNEL, &dma_cfg);

	if (ret) {
		LOG_ERR("dma_config failed: %d", ret);
		return ret;
	}

	ret = dma_start(dma_dev, DMA_CHANNEL);
	if (ret) {
		LOG_ERR("dma_start failed: %d", ret);
		return ret;
	}

	return 0;
}

/*
 * Wait for DMA completion
 *
 * Blocks until DMA transfer completes or times out.
 */
static int wait_for_completion(void)
{
	int ret = k_sem_take(&dma_done, K_SECONDS(1));

	if (ret) {
		LOG_ERR("DMA timed out");
		return ret;
	}

	return atomic_get(&dma_error);
}

/*
 * Verify transfer results
 *
 * Compares source and destination buffers for each block to ensure correct transfer.
 */
static int verify_transfer(void)
{
	int pass = 1;

	if (memcmp(src1, dst1, BLOCK1_SIZE) != 0) {
		LOG_ERR("FAIL: block1 mismatch");
		pass = 0;
	} else {
		LOG_INF("PASS: block1 (%zu bytes)", (size_t)BLOCK1_SIZE);
	}

	if (memcmp(src2, dst2, BLOCK2_SIZE) != 0) {
		LOG_ERR("FAIL: block2 mismatch");
		pass = 0;
	} else {
		LOG_INF("PASS: block2 (%zu bytes)", (size_t)BLOCK2_SIZE);
	}

	if (memcmp(src3, dst3, BLOCK3_SIZE) != 0) {
		LOG_ERR("FAIL: block3 mismatch");
		pass = 0;
	} else {
		LOG_INF("PASS: block3 (%zu bytes)", (size_t)BLOCK3_SIZE);
	}

	return pass ? 0 : -EIO;
}

/*
 * Run scatter-gather test
 *
 * Main test function that sets up data, configures DMA, starts transfer,
 * waits for completion, and verifies results.
 */
static int run_scatter_gather_test(const struct device *dma_dev)
{
	static struct dma_block_config blocks[3];
	int ret;

	/* Setup test data */
	fill_buffer(src1, BLOCK1_SIZE, 0xA0);
	fill_buffer(src2, BLOCK2_SIZE, 0xB0);
	fill_buffer(src3, BLOCK3_SIZE, 0xC0);
	clear_buffers();

	/* Setup scatter-gather blocks */
	ret = setup_scatter_gather_blocks(blocks);
	if (ret) {
		return ret;
	}

	/* Configure and start DMA */
	ret = configure_and_start_dma(dma_dev, &blocks[0]);
	if (ret) {
		return ret;
	}

	LOG_INF("Starting scatter-gather: block1=%zu bytes, block2=%zu bytes, block3=%zu bytes",
		(size_t)BLOCK1_SIZE, (size_t)BLOCK2_SIZE, (size_t)BLOCK3_SIZE);

	/* Wait for completion */
	ret = wait_for_completion();
	if (ret) {
		dma_stop(dma_dev, DMA_CHANNEL);
		return ret;
	}

	/* Verify results */
	return verify_transfer();
}

/*
 * Main application entry point
 *
 * Initializes DMA device and runs the scatter-gather test.
 */
int main(void)
{
	const struct device *dma_dev = DEVICE_DT_GET(DMA_NODE);
	int ret;

	LOG_INF("DMA scatter-gather test starting");

	if (!device_is_ready(dma_dev)) {
		LOG_ERR("DMA device not ready");
		return -ENODEV;
	}

	ret = run_scatter_gather_test(dma_dev);
	if (ret == 0) {
		LOG_INF("All blocks passed");
	} else {
		LOG_ERR("Test failed");
	}

	return ret;
}

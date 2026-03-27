/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/**
 * @file main.c
 * @brief PL330 DMA user microcode sample — M2M memcpy
 *
 * Demonstrates:
 *  - Building a PL330 microcode program using dma_pl330_opcode.h helpers
 *  - Executing it via dma_pl330_start_with_mcode()
 *
 * Microcode transfers 1000 bytes using 1-byte beats, burst length 16:
 *   - Loop 62 × DMALD+DMAST  = 992 bytes
 *   - Tail  1 × DMALD+DMAST  =   8 bytes  (burst_len overridden to 7)
 *   - DMAWMB + DMASEV(ch) + DMAEND
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/dma.h>
#include <zephyr/drivers/dma/dma_pl330.h>
#include <zephyr/drivers/dma/dma_pl330_opcode.h>
#include <soc_memory_map.h>
#include <string.h>

/* DMA controller selected via the dma-dev alias in the board snippet:
 *   - RTSS-HE: dma2   (Ensemble HE, E1C HE, Balletto HE)
 *   - RTSS-HP: dma1   (Ensemble HP)
 * Build with: -S alif-dma-mcode
 */
#define DMA_NODE DT_ALIAS(dma_dev)
#define CHANNEL  0

#define XFER_LEN      1000U
#define BURST_LEN     16U    /* beats per burst */
#define MCODE_BUF_SIZE 256U  /* microcode buffer size in bytes */

static uint8_t src_buf[XFER_LEN];
static uint8_t dst_buf[XFER_LEN];
static uint8_t __aligned(4) mcode_buf[MCODE_BUF_SIZE];

static K_SEM_DEFINE(dma_done, 0, 1);

static void dma_callback(const struct device *dev, void *user_data,
			 uint32_t channel, int status)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(user_data);
	ARG_UNUSED(channel);
	ARG_UNUSED(status);

	k_sem_give(&dma_done);
}

/**
 * @brief Build a PL330 M2M memcpy microcode program.
 *
 * @param buf      Output buffer for generated microcode.
 * @param buf_size Capacity of buf in bytes.
 * @param src      Source address.
 * @param dst      Destination address.
 * @param len      Number of bytes to transfer.
 * @param event_id DMASEV event number (equals channel number).
 *
 * @return Number of bytes written on success, negative errno on error.
 */
static int build_mcode(uint8_t *buf, size_t buf_size,
		       const uint8_t *src, const uint8_t *dst,
		       uint32_t len, uint8_t event_id)
{
	struct dma_pl330_opcode_buf ob = { .buf = buf, .off = 0, .buf_size = buf_size };
	union dma_pl330_ccr ccr = { .value = 0 };
	uint32_t full_bursts = len / BURST_LEN;
	uint32_t residue     = len % BURST_LEN;
	uint32_t lp_start;
	struct dma_pl330_loop lp;

	/* CCR: 1-byte beats, burst_len = BURST_LEN-1, incrementing src+dst */
	ccr.b.src_inc        = 1;
	ccr.b.dst_inc        = 1;
	ccr.b.src_burst_size = 0;  /* log2(1 byte) */
	ccr.b.dst_burst_size = 0;
	ccr.b.src_burst_len  = BURST_LEN - 1;
	ccr.b.dst_burst_len  = BURST_LEN - 1;
	ccr.b.src_cache_ctrl = 0x2; /* CC_CCTRL_MODIFIABLE_VALUE */
	ccr.b.dst_cache_ctrl = 0x2; /* CC_CCTRL_MODIFIABLE_VALUE */
	ccr.b.dst_prot_ctrl  = 0;
	ccr.b.src_prot_ctrl  = 0;
	ccr.b.endian_swap    = DMA_PL330_SWAP_NONE;   /* no endian swap */

	/* Set CCR, SAR, DAR */
	if (!dma_pl330_gen_move(ccr.value, DMA_PL330_REG_CCR, &ob)) {
		return -ENOMEM;
	}
	if (!dma_pl330_gen_move(local_to_global(src), DMA_PL330_REG_SAR, &ob)) {
		return -ENOMEM;
	}
	if (!dma_pl330_gen_move(local_to_global(dst), DMA_PL330_REG_DAR, &ob)) {
		return -ENOMEM;
	}

	/* Main loop: full_bursts × BURST_LEN bytes */
	if (full_bursts > 0) {
		if (!dma_pl330_gen_loop(DMA_PL330_LC_0, full_bursts, &ob)) {
			return -ENOMEM;
		}
		lp_start = ob.off;
		if (!dma_pl330_gen_load(DMA_PL330_XFER_FORCE, &ob)) {
			return -ENOMEM;
		}
		if (!dma_pl330_gen_store(DMA_PL330_XFER_FORCE, &ob)) {
			return -ENOMEM;
		}
		lp = (struct dma_pl330_loop){
			.xfer_type = DMA_PL330_XFER_FORCE,
			.lc        = DMA_PL330_LC_0,
			.jump      = (uint8_t)(ob.off - lp_start),
			.nf        = true,
		};
		if (!dma_pl330_gen_loopend(&lp, &ob)) {
			return -ENOMEM;
		}
	}

	/* Tail: residue bytes using a single burst with adjusted burst_len */
	if (residue > 0) {
		ccr.b.src_burst_len = residue - 1;
		ccr.b.dst_burst_len = residue - 1;
		if (!dma_pl330_gen_move(ccr.value, DMA_PL330_REG_CCR, &ob)) {
			return -ENOMEM;
		}
		if (!dma_pl330_gen_load(DMA_PL330_XFER_FORCE, &ob)) {
			return -ENOMEM;
		}
		if (!dma_pl330_gen_store(DMA_PL330_XFER_FORCE, &ob)) {
			return -ENOMEM;
		}
	}

	/* Write barrier, send event, end */
	if (!dma_pl330_gen_wmb(&ob)) {
		return -ENOMEM;
	}
	if (!dma_pl330_gen_send_event(event_id, &ob)) {
		return -ENOMEM;
	}
	if (!dma_pl330_gen_end(&ob)) {
		return -ENOMEM;
	}

	return (int)ob.off;
}

int main(void)
{
	const struct device *dma_dev = DEVICE_DT_GET(DMA_NODE);
	struct dma_block_config blk = {
		.source_address  = (uint32_t)(uintptr_t)src_buf,
		.dest_address    = (uint32_t)(uintptr_t)dst_buf,
		.block_size      = XFER_LEN,
		.source_addr_adj = DMA_ADDR_ADJ_INCREMENT,
		.dest_addr_adj   = DMA_ADDR_ADJ_INCREMENT,
	};
	struct dma_config cfg = {
		.channel_direction    = MEMORY_TO_MEMORY,
		.source_data_size     = 1,
		.dest_data_size       = 1,
		.source_burst_length  = BURST_LEN,
		.dest_burst_length    = BURST_LEN,
		.dma_callback         = dma_callback,
		.complete_callback_en = 1,
		.head_block           = &blk,
	};
	int ret, mcode_len;

	if (!device_is_ready(dma_dev)) {
		printk("DMA device not ready\n");
		return -ENODEV;
	}

	/* Initialise source buffer, clear destination */
	for (int i = 0; i < XFER_LEN; i++) {
		src_buf[i] = (uint8_t)(i + 1);
	}
	memset(dst_buf, 0, sizeof(dst_buf));

	/* Configure channel — stores callback and direction; addresses are
	 * overridden by the microcode, so the values above are not used for
	 * the actual transfer.
	 */
	ret = dma_config(dma_dev, CHANNEL, &cfg);
	if (ret) {
		printk("dma_config failed: %d\n", ret);
		return ret;
	}

	/* Build microcode. Event ID equals the channel number. */
	mcode_len = build_mcode(mcode_buf, sizeof(mcode_buf),
				src_buf, dst_buf, XFER_LEN, CHANNEL);
	if (mcode_len < 0) {
		printk("microcode build failed: %d\n", mcode_len);
		return mcode_len;
	}

	printk("Microcode size: %d bytes\n", mcode_len);

	/* Start transfer using custom microcode */
	ret = dma_pl330_start_with_mcode(dma_dev, CHANNEL, mcode_buf, mcode_len);
	if (ret) {
		printk("dma_pl330_start_with_mcode failed: %d\n", ret);
		return ret;
	}

	/* Wait for completion (interrupt mode: semaphore is posted in callback) */
	k_sem_take(&dma_done, K_FOREVER);

	/* Verify */
	if (memcmp(src_buf, dst_buf, XFER_LEN) == 0) {
		printk("DMA memcpy PASS\n");
	} else {
		printk("DMA memcpy FAIL\n");
		return -EIO;
	}

	return 0;
}

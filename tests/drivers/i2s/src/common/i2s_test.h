/*
 * Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * i2s_test.h - Shared definitions, structs, and prototypes for the I2S test suite.
 */

#ifndef I2S_TEST_H
#define I2S_TEST_H

#include <zephyr/kernel.h>
#include <zephyr/drivers/i2s.h>
#include <zephyr/ztest.h>

/* -------------------------------------------------------------------------
 * Device node resolution
 * -------------------------------------------------------------------------
 */
#if DT_NODE_EXISTS(DT_NODELABEL(i2s_rxtx))
#define I2S_RX_NODE  DT_NODELABEL(i2s_rxtx)
#define I2S_TX_NODE  I2S_RX_NODE
#else
#define I2S_RX_NODE  DT_NODELABEL(i2s_rx)
#define I2S_TX_NODE  DT_NODELABEL(i2s_tx)
#endif

#define I2S_GOLDEN_NODE  DT_ALIAS(i2s_node0)

/* -------------------------------------------------------------------------
 * Buffer geometry
 * -------------------------------------------------------------------------
 */
#define I2S_CHANNELS         2U
#define I2S_FRAMES           32U
#define I2S_GOLDEN_VEC_LEN   I2S_FRAMES
#define I2S_MAX_WORD_BYTES   4U
#define I2S_BLOCK_SIZE       (I2S_FRAMES * I2S_CHANNELS * I2S_MAX_WORD_BYTES)
#define I2S_TIMEOUT_MS       500U

/* Aliases used by golden-vector helpers (match i2s_golden_test.h naming) */
#define I2S_GOLDEN_CHANNELS        I2S_CHANNELS
#define I2S_GOLDEN_SAMPLES_PER_CH  I2S_FRAMES
#define I2S_GOLDEN_FRAMES          I2S_FRAMES
#define I2S_GOLDEN_MAX_WORD_BYTES  I2S_MAX_WORD_BYTES
#define I2S_GOLDEN_BLOCK_SIZE      I2S_BLOCK_SIZE
#define I2S_GOLDEN_TIMEOUT_MS      I2S_TIMEOUT_MS
#define I2S_GOLDEN_NUM_TX_BLOCKS   6U
#define I2S_GOLDEN_NUM_RX_BLOCKS   6U

/* -------------------------------------------------------------------------
 * DW I2S register offsets (used by config tests)
 * -------------------------------------------------------------------------
 */
#define I2S_REG_IRER_OFFSET   0x04U
#define I2S_REG_CER_OFFSET    0x0CU
#define I2S_REG_CCR_OFFSET    0x10U
#define I2S_REG_RXFFR_OFFSET  0x14U
#define I2S_REG_RER_OFFSET    0x28U
#define I2S_REG_RCR_OFFSET    0x30U
#define I2S_REG_TCR_OFFSET    0x34U
#define I2S_REG_RFCR_OFFSET   0x48U
#define I2S_REG_TFCR_OFFSET   0x4CU
#define I2S_REG_RFF_OFFSET    0x50U
#define I2S_REG_DMACR_OFFSET  0x200U

#define I2S_CCR_WSS_POS       3U
#define I2S_CCR_WSS_MASK      (0x3U << I2S_CCR_WSS_POS)
#define I2S_DMACR_RX_EN_BIT   BIT(16)
#define I2S_DMACR_TX_EN_BIT   BIT(17)

/* -------------------------------------------------------------------------
 * Golden vector entry — one per (bit_depth) test case
 * -------------------------------------------------------------------------
 */
struct i2s_golden_tc {
	const char     *name;
	uint32_t        sample_rate;
	uint8_t         word_size;
	const uint32_t *vec_L;
	const uint32_t *vec_R;
};

/* Pre-computed sine golden vectors (MSB-justified 32-bit, per channel)
 * defined in i2s_golden_vectors.c
 */
extern const uint32_t i2s_golden_12bit_L[I2S_GOLDEN_VEC_LEN];
extern const uint32_t i2s_golden_12bit_R[I2S_GOLDEN_VEC_LEN];
extern const uint32_t i2s_golden_16bit_L[I2S_GOLDEN_VEC_LEN];
extern const uint32_t i2s_golden_16bit_R[I2S_GOLDEN_VEC_LEN];
extern const uint32_t i2s_golden_20bit_L[I2S_GOLDEN_VEC_LEN];
extern const uint32_t i2s_golden_20bit_R[I2S_GOLDEN_VEC_LEN];
extern const uint32_t i2s_golden_24bit_L[I2S_GOLDEN_VEC_LEN];
extern const uint32_t i2s_golden_24bit_R[I2S_GOLDEN_VEC_LEN];
extern const uint32_t i2s_golden_32bit_L[I2S_GOLDEN_VEC_LEN];
extern const uint32_t i2s_golden_32bit_R[I2S_GOLDEN_VEC_LEN];

/* Full test matrix declared in i2s_golden_vectors.c */
extern const struct i2s_golden_tc i2s_golden_matrix[];
extern const size_t                i2s_golden_matrix_len;

/* Memory slabs for golden-vector tests (defined in i2s_test_common.c) */
extern struct k_mem_slab g_tx_slab;
extern struct k_mem_slab g_rx_slab;

/* -------------------------------------------------------------------------
 * Helper function prototypes (implemented in i2s_test_common.c)
 * -------------------------------------------------------------------------
 */
const struct device *i2s_rx_dev(void);
const struct device *i2s_tx_dev(void);
uintptr_t i2s_rx_base(void);
uintptr_t i2s_tx_base(void);

int  i2s_golden_configure(const struct device *dev, enum i2s_dir dir,
			  uint32_t rate, uint8_t word_size);
int  i2s_golden_tx_write(const struct device *dev, uint8_t word_size,
			 const uint32_t *vec_L, const uint32_t *vec_R);
int  i2s_golden_rx_verify(const struct device *dev, uint8_t word_size,
			  const uint32_t *vec_L, const uint32_t *vec_R);
void i2s_golden_stop(const struct device *dev);
int  i2s_golden_run(const struct device *dev, uint32_t rate, uint8_t word_size,
		    const uint32_t *vec_L, const uint32_t *vec_R);

/* -------------------------------------------------------------------------
 * Helpers for negative, stress, and performance.
 * Implementations live in i2s_test_common.c.
 * I2S_CHANNELS and I2S_FRAMES are defined above.
 * -------------------------------------------------------------------------
 */
#define I2S_MAX_FRAMES       64U
#define I2S_RATE_HZ          48000U
#define I2S_XFER_TIMEOUT_MS  200

#ifdef CONFIG_I2S_DW_TX_BLOCK_COUNT
#define I2S_TX_QUEUE CONFIG_I2S_DW_TX_BLOCK_COUNT
#else
#define I2S_TX_QUEUE 10
#endif

#ifdef CONFIG_I2S_DW_RX_BLOCK_COUNT
#define I2S_RX_QUEUE CONFIG_I2S_DW_RX_BLOCK_COUNT
#else
#define I2S_RX_QUEUE 10
#endif

#define I2S_SLAB_COUNT (MAX(I2S_TX_QUEUE, I2S_RX_QUEUE) + 4)
#define I2S_MAX_BLOCK  (I2S_MAX_FRAMES * I2S_CHANNELS * 4U)

extern struct k_mem_slab i2s_tx_slab;
extern struct k_mem_slab i2s_rx_slab;

struct i2s_tx_feed {
	const struct device *dev;
	size_t size;
	volatile bool run;
	volatile int fail;
	volatile uint32_t writes;
};

static inline const struct device *i2s_dev(void)
{
	return DEVICE_DT_GET(I2S_GOLDEN_NODE);
}

static inline size_t i2s_sample_bytes(uint8_t word_size)
{
	return word_size <= 16U ? 2U : 4U;
}

static inline size_t i2s_block_bytes(uint32_t frames, uint8_t word_size)
{
	return (size_t)frames * I2S_CHANNELS * i2s_sample_bytes(word_size);
}

uint32_t i2s_tx_used(void);
uint32_t i2s_rx_used(void);
bool i2s_slabs_full(void);

void i2s_setup_cfg(struct i2s_config *cfg, struct k_mem_slab *slab,
		  uint32_t rate, uint8_t word_size, uint8_t channels,
		  size_t block_size, int32_t timeout,
		  i2s_fmt_t format, i2s_opt_t options);
void i2s_setup_default(struct i2s_config *cfg, struct k_mem_slab *slab);
void i2s_stream_reset(const struct device *dev);
int i2s_block_write(const struct device *dev, size_t size, uint8_t pattern);
int i2s_wait_ready(const struct device *dev, enum i2s_dir dir,
		  const struct i2s_config *cfg, int32_t timeout_ms);
int i2s_force_tx_error(const struct device *dev);
void i2s_tx_feed_fn(void *p1, void *p2, void *p3);

#endif /* I2S_TEST_H */

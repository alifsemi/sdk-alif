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

#endif /* I2S_TEST_H */

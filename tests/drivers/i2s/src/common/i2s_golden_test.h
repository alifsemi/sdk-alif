/*
 * Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * i2s_golden_test.h - Shared definitions, golden vector declarations,
 *                     and helper prototypes for the I2S master-only test suite.
 *
 * Constraints this suite is designed under:
 *   - TX master-only (no slave mode)
 *   - No full-duplex (I2S_DIR_BOTH not used)
 *   - Internal hardware loopback (I2S_OPT_LOOPBACK) used for RX verification
 *   - External GPIO loopback supported via CONFIG_I2S_GOLDEN_GPIO_LOOPBACK
 */

#ifndef I2S_GOLDEN_TEST_H
#define I2S_GOLDEN_TEST_H

#include <zephyr/kernel.h>
#include <zephyr/drivers/i2s.h>
#include <zephyr/ztest.h>

/* -------------------------------------------------------------------------
 * DT node aliases
 * Boards must define i2s_node0 (TX/RX master port) in their board overlay.
 * -------------------------------------------------------------------------
 */
#define I2S_GOLDEN_NODE   DT_ALIAS(i2s_node0)

/* -------------------------------------------------------------------------
 * Buffer geometry
 *   CHANNELS        : stereo (L + R)
 *   SAMPLES_PER_CH  : number of sample frames per block
 *   MAX_WORD_BYTES  : widest word = 32-bit => 4 bytes
 *   BLOCK_SIZE      : bytes per transfer block (worst-case 32-bit)
 * -------------------------------------------------------------------------
 */
#define I2S_GOLDEN_CHANNELS         2U
#define I2S_GOLDEN_SAMPLES_PER_CH   32U
#define I2S_GOLDEN_FRAMES           I2S_GOLDEN_SAMPLES_PER_CH
#define I2S_GOLDEN_MAX_WORD_BYTES   4U
#define I2S_GOLDEN_BLOCK_SIZE \
	(I2S_GOLDEN_FRAMES * I2S_GOLDEN_CHANNELS * I2S_GOLDEN_MAX_WORD_BYTES)

/* -------------------------------------------------------------------------
 * Memory slabs
 *   NUM_BLOCKS: enough for pipeline depth (pre-queued + in-flight + drain)
 * -------------------------------------------------------------------------
 */
#define I2S_GOLDEN_NUM_TX_BLOCKS    6U
#define I2S_GOLDEN_NUM_RX_BLOCKS    6U

extern struct k_mem_slab g_tx_slab;
extern struct k_mem_slab g_rx_slab;

/* -------------------------------------------------------------------------
 * Timeout for i2s_read / sem_take (ms)
 * -------------------------------------------------------------------------
 */
#define I2S_GOLDEN_TIMEOUT_MS       2000U

/* -------------------------------------------------------------------------
 * Golden vector entry — one entry per (bit_depth, channel) combination.
 *   samples[] holds SAMPLES_PER_CH 32-bit words, MSB-justified so the same
 *   array is valid for all word widths after right-shifting by (32 - width).
 * -------------------------------------------------------------------------
 */
#define I2S_GOLDEN_VEC_LEN   I2S_GOLDEN_SAMPLES_PER_CH

/* Pre-computed sine golden vectors (MSB-justified 32-bit, stereo interleaved)
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

/* -------------------------------------------------------------------------
 * Test matrix descriptor — one entry drives one ZTEST case
 * -------------------------------------------------------------------------
 */
struct i2s_golden_tc {
	const char     *name;          /* human-readable e.g. "8kHz_16bit"     */
	uint32_t        sample_rate;   /* Hz                                    */
	uint8_t         word_size;     /* bits: 12, 16, 20, 24, 32             */
	const uint32_t *vec_L;         /* golden L-channel (MSB-justified 32b) */
	const uint32_t *vec_R;         /* golden R-channel (MSB-justified 32b) */
};

/* Full matrix declared in i2s_golden_vectors.c, used by test_i2s_tx_golden.c */
extern const struct i2s_golden_tc i2s_golden_matrix[];
extern const size_t                i2s_golden_matrix_len;

/* -------------------------------------------------------------------------
 * Helper function prototypes (implemented in i2s_test_common.c)
 * -------------------------------------------------------------------------
 */

/**
 * @brief Configure one I2S direction (TX or RX) as master.
 *
 * @param dev        I2S device handle
 * @param dir        I2S_DIR_TX or I2S_DIR_RX
 * @param rate       sample rate in Hz
 * @param word_size  bits per sample (12/16/20/24/32)
 * @return 0 on success, negative errno otherwise
 */
int i2s_golden_configure(const struct device *dev, enum i2s_dir dir,
			 uint32_t rate, uint8_t word_size);

/**
 * @brief Write one block of interleaved stereo golden data to the TX queue.
 *
 * Allocates from g_tx_slab, fills L/R channels from vec_L/vec_R (MSB-
 * justified, shifted to word_size), then calls i2s_write().
 *
 * @param dev        I2S device handle
 * @param word_size  bits per sample
 * @param vec_L      L-channel golden vector (MSB-justified 32-bit)
 * @param vec_R      R-channel golden vector (MSB-justified 32-bit)
 * @return 0 on success, negative errno otherwise
 */
int i2s_golden_tx_write(const struct device *dev, uint8_t word_size,
			const uint32_t *vec_L, const uint32_t *vec_R);

/**
 * @brief Read one block from the RX queue and verify against golden vectors.
 *
 * Reads via i2s_read(), then compares each sample against the expected
 * golden value. Prints the first mismatch with word index, channel, expected
 * and actual values.
 *
 * @param dev        I2S device handle
 * @param word_size  bits per sample
 * @param vec_L      expected L-channel golden vector (MSB-justified 32-bit)
 * @param vec_R      expected R-channel golden vector (MSB-justified 32-bit)
 * @return 0 if all samples match, -EIO on mismatch, other negative on error
 */
int i2s_golden_rx_verify(const struct device *dev, uint8_t word_size,
			 const uint32_t *vec_L, const uint32_t *vec_R);

/**
 * @brief Gracefully stop both TX and RX streams and drop queued data.
 *
 * @param dev  I2S device handle
 */
void i2s_golden_stop(const struct device *dev);

/**
 * @brief Run a complete golden-vector TX→loopback→RX→verify sequence.
 *
 * Configures TX and RX as masters, pre-queues blocks, starts TX then RX,
 * reads back and verifies, then stops both streams.
 *
 * @param dev        I2S device handle
 * @param rate       sample rate in Hz
 * @param word_size  bits per sample
 * @param vec_L      L-channel golden vector
 * @param vec_R      R-channel golden vector
 * @return 0 on pass, negative on failure/skip
 */
int i2s_golden_run(const struct device *dev, uint32_t rate, uint8_t word_size,
		   const uint32_t *vec_L, const uint32_t *vec_R);

#endif /* I2S_GOLDEN_TEST_H */

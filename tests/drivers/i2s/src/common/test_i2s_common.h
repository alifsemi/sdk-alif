/*
 * Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#ifndef _I2S_Common
#define _I2S_Common

#if DT_NODE_EXISTS(DT_NODELABEL(i2s_rxtx))
#define I2S_RX_NODE  DT_NODELABEL(i2s_rxtx)
#define I2S_TX_NODE  I2S_RX_NODE
#else
#define I2S_RX_NODE  DT_NODELABEL(i2s_rx)
#define I2S_TX_NODE  DT_NODELABEL(i2s_tx)
#endif

#define SAMPLE_FREQUENCY      48000  /* 48 kHz */
#define SAMPLE_FREQUENCY8     8000
#define SAMPLE_FREQUENCY16    16000
#define SAMPLE_FREQUENCY32    32000
#define SAMPLE_FREQUENCY44    44100
#define SAMPLE_FREQUENCY88    88200
#define SAMPLE_FREQUENCY96    96000


#define SAMPLE_BIT_WIDTH    24
#define BYTES_PER_SAMPLE    sizeof(uint32_t)
#define NUMBER_OF_CHANNELS  2

/* Such block length provides an echo with the delay of 100 ms. */
#define SAMPLES_PER_BLOCK   ((SAMPLE_FREQUENCY / 10) * NUMBER_OF_CHANNELS)
#define SAMPLES_PER_BLOCK8   ((SAMPLE_FREQUENCY8 / 10) * NUMBER_OF_CHANNELS)
#define SAMPLES_PER_BLOCK16   ((SAMPLE_FREQUENCY16 / 10) * NUMBER_OF_CHANNELS)
#define SAMPLES_PER_BLOCK32  ((SAMPLE_FREQUENCY32 / 10) * NUMBER_OF_CHANNELS)
#define SAMPLES_PER_BLOCK44   ((SAMPLE_FREQUENCY44 / 10) * NUMBER_OF_CHANNELS)
#define SAMPLES_PER_BLOCK88  ((SAMPLE_FREQUENCY88  / 10) * NUMBER_OF_CHANNELS)
#define SAMPLES_PER_BLOCK96   ((SAMPLE_FREQUENCY96 / 10) * NUMBER_OF_CHANNELS)
#define INITIAL_BLOCKS      4
#define TIMEOUT             1000

#define BLOCK_SIZE  (BYTES_PER_SAMPLE * SAMPLES_PER_BLOCK)
#define BLOCK_SIZE96  (BYTES_PER_SAMPLE * SAMPLES_PER_BLOCK96)
#define BLOCK_SIZE88  (BYTES_PER_SAMPLE * SAMPLES_PER_BLOCK88)
#define BLOCK_SIZE44  (BYTES_PER_SAMPLE * SAMPLES_PER_BLOCK44)
#define BLOCK_SIZE32  (BYTES_PER_SAMPLE * SAMPLES_PER_BLOCK32)
#define BLOCK_SIZE16  (BYTES_PER_SAMPLE * SAMPLES_PER_BLOCK)
#define BLOCK_COUNT (10)

#define SAMPLE_NO	32
#define FRAME_CLK_FREQ	8000
#define TEST_I2S_STATE_RUNNING_NEG_REPEAT_COUNT  5

extern int16_t data_l[SAMPLE_NO];
extern int16_t data_r[SAMPLE_NO];

#define I2S_DEV_NAME_RX "I2S_0"
#ifdef CONFIG_I2S_TEST_SEPARATE_DEVICES
#define I2S_DEV_NAME_TX "I2S_1"
#else
#define I2S_DEV_NAME_TX "I2S_0"
#endif
#define BLOCK_SIZE_D (2 * sizeof(data_l))

#define NUM_RX_BLOCKS	4
#define NUM_TX_BLOCKS	4



extern struct k_mem_slab mem_slab;
extern struct k_mem_slab rx_mem_slab;
extern struct k_mem_slab tx_mem_slab;

int tx_block_write(const struct device *dev_i2s, int att, int err);
int rx_block_read(const struct device *dev_i2s, int att);
int configure_stream(const struct device *dev_i2s, enum i2s_dir dir);

#endif

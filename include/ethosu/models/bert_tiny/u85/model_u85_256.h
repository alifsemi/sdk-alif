/**
 * @brief BERT-Tiny Model for ARM Ethos-U85 NPU
 *
 * Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef BERT_TINY_MODEL_U85_256_H
#define BERT_TINY_MODEL_U85_256_H

#include <stddef.h>
#include <stdint.h>
#include <zephyr/kernel.h>

/* Tensor arena size - adjusted per core DTCM constraints
 * HP: 700KB (fits in 1MB DTCM with stacks/heap)
 * HE: 128KB (fits in 256KB DTCM)
 */
#if defined(CONFIG_BOARD_ALIF_E8_DK_AE822FA0E5597XX0_RTSS_HE)
#define TENSOR_ARENA_SIZE 131072      /* 128KB for HE */
#elif defined(CONFIG_BOARD_ALIF_E8_DK_AE822FA0E5597XX0_RTSS_HP)
#define TENSOR_ARENA_SIZE 716800      /* 700KB for HP */
#else
#define TENSOR_ARENA_SIZE 716800      /* 700KB default */
#endif

/* Model name */
extern const char modelName[];

/* Tensor arena size constant */
extern const size_t tensorArenaSize;

/* Network model data - aligned for NPU DMA access */
extern unsigned char networkModelData[];

/* Model data size in bytes */
extern const size_t networkModelDataSize;

#endif /* BERT_TINY_MODEL_U85_256_H */

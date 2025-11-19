/**
 * @brief BERT-Tiny Sample Output Data (INT8 Quantized)
 *
 * Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef BERT_TINY_OUTPUT_H
#define BERT_TINY_OUTPUT_H

#include <stddef.h>
#include <stdint.h>

/* Expected output data array - aligned for NPU DMA access */
extern unsigned char expectedOutputData[];

/* Expected output data size in bytes */
extern const size_t expectedOutputDataSize;

#endif /* BERT_TINY_OUTPUT_H */

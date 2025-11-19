/*
 * BERT-Tiny Sample Input Data (INT8 Quantized)
 * Model: bert_tiny_ethos_u85
 * Format: Following ARM Ethos-U reference implementation
 */

#ifndef BERT_TINY_INPUT_H
#define BERT_TINY_INPUT_H

#include <stddef.h>
#include <stdint.h>

/* Input data array - aligned for NPU DMA access */
extern unsigned char inputData[];

/* Input data size in bytes */
extern const size_t inputDataSize;

#endif /* BERT_TINY_INPUT_H */

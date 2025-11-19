/*
 * BERT-Tiny Sample Output Data (INT8 Quantized)
 * Model: bert_tiny_ethos_u85
 * Format: Following ARM Ethos-U reference implementation
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

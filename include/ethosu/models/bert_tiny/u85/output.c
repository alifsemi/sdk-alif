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

#include "output.h"

unsigned char expectedOutputData[] __attribute__((aligned(16), section(".rodata.tflm_output"))) = {
    0xf3, 0xa0, 0xf6, 0x8d, 0xdf, 0x80, 0xbd, 0x4c, 0x80, 0xb2, 0x80, 0x9c, 0x80, 0x80, 0x80, 0xbb, 0x80, 0xd4, 0x80, 0x96, 0x84, 0x99, 0x86, 0xef, 0xe9, 0xad, 0x80, 0x80, 0x89, 0x93, 0xa1, 0x80
};

const size_t expectedOutputDataSize = sizeof(expectedOutputData);

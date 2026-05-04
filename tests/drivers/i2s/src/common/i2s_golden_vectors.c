/*
 * Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * i2s_golden_vectors.c - Pre-computed golden test vectors for I2S validation.
 *
 * All vectors are one period of a sine wave at full-scale amplitude,
 * 32 samples per channel, stored MSB-justified in a 32-bit word.
 *
 * L-channel: fundamental frequency (1 cycle / 32 samples)
 * R-channel: double frequency      (2 cycles / 32 samples)
 *
 * Bit depths covered: 12, 16, 20, 24, 32
 */

#include "i2s_test.h"

/* =========================================================================
 * 12-bit golden vectors
 *   full-scale peak = 2^11 - 1 = 2047
 *   MSB-justified shift = 32 - 12 = 20
 * =========================================================================
 */
const uint32_t i2s_golden_12bit_L[I2S_GOLDEN_VEC_LEN] = {
	0x00000000U, 0x19300000U, 0x30480000U, 0x434C0000U,
	0x4FA00000U, 0x573C0000U, 0x5A4C0000U, 0x59180000U,
	0x53680000U, 0x499C0000U, 0x3C040000U, 0x2C340000U,
	0x1A700000U, 0x07E00000U, 0xF5800000U, 0xE3580000U,
	0xD1400000U, 0xC0780000U, 0xB1880000U, 0xA5100000U,
	0x9B600000U, 0x94880000U, 0x91640000U, 0x91E80000U,
	0x95A80000U, 0x9CC40000U, 0xA72C0000U, 0xB4600000U,
	0xC3E00000U, 0xD4800000U, 0xE5780000U, 0xF6C00000U,
};

const uint32_t i2s_golden_12bit_R[I2S_GOLDEN_VEC_LEN] = {
	0x00000000U, 0x30480000U, 0x53680000U, 0x5A4C0000U,
	0x43400000U, 0x19300000U, 0xE6380000U, 0xBBD00000U,
	0xAC980000U, 0xBBD00000U, 0xE6380000U, 0x19300000U,
	0x43400000U, 0x5A4C0000U, 0x53680000U, 0x30480000U,
	0x00000000U, 0xCFB80000U, 0xAC980000U, 0xA5B40000U,
	0xBCC00000U, 0xE6D00000U, 0x19300000U, 0x43400000U,
	0x5A4C0000U, 0x53680000U, 0x30480000U, 0x00000000U,
	0xCFB80000U, 0xAC980000U, 0xA5B40000U, 0xBCC00000U,
};

/* =========================================================================
 * 16-bit golden vectors
 *   full-scale peak = 2^15 - 1 = 32767
 *   MSB-justified shift = 32 - 16 = 16
 * =========================================================================
 */
const uint32_t i2s_golden_16bit_L[I2S_GOLDEN_VEC_LEN] = {
	0x00000000U, 0x18F80000U, 0x30FB0000U, 0x471C0000U,
	0x5A820000U, 0x6A6D0000U, 0x76410000U, 0x7D890000U,
	0x7FFF0000U, 0x7D890000U, 0x76410000U, 0x6A6D0000U,
	0x5A820000U, 0x471C0000U, 0x30FB0000U, 0x18F80000U,
	0x00000000U, 0xE7080000U, 0xCF050000U, 0xB8E40000U,
	0xA57E0000U, 0x95930000U, 0x89BF0000U, 0x82770000U,
	0x80010000U, 0x82770000U, 0x89BF0000U, 0x95930000U,
	0xA57E0000U, 0xB8E40000U, 0xCF050000U, 0xE7080000U,
};

const uint32_t i2s_golden_16bit_R[I2S_GOLDEN_VEC_LEN] = {
	0x00000000U, 0x30FB0000U, 0x5A820000U, 0x76410000U,
	0x7FFF0000U, 0x76410000U, 0x5A820000U, 0x30FB0000U,
	0x00000000U, 0xCF050000U, 0xA57E0000U, 0x89BF0000U,
	0x80010000U, 0x89BF0000U, 0xA57E0000U, 0xCF050000U,
	0x00000000U, 0x30FB0000U, 0x5A820000U, 0x76410000U,
	0x7FFF0000U, 0x76410000U, 0x5A820000U, 0x30FB0000U,
	0x00000000U, 0xCF050000U, 0xA57E0000U, 0x89BF0000U,
	0x80010000U, 0x89BF0000U, 0xA57E0000U, 0xCF050000U,
};

/* =========================================================================
 * 20-bit golden vectors
 *   full-scale peak = 2^19 - 1 = 524287
 *   MSB-justified shift = 32 - 20 = 12
 * =========================================================================
 */
const uint32_t i2s_golden_20bit_L[I2S_GOLDEN_VEC_LEN] = {
	0x00000000U, 0x18F8B000U, 0x30FBC000U, 0x471CC000U,
	0x5A827000U, 0x6A6D9000U, 0x76412000U, 0x7D89D000U,
	0x7FFFF000U, 0x7D89D000U, 0x76412000U, 0x6A6D9000U,
	0x5A827000U, 0x471CC000U, 0x30FBC000U, 0x18F8B000U,
	0x00000000U, 0xE7075000U, 0xCF044000U, 0xB8E34000U,
	0xA57D9000U, 0x95927000U, 0x89BEE000U, 0x82763000U,
	0x80001000U, 0x82763000U, 0x89BEE000U, 0x95927000U,
	0xA57D9000U, 0xB8E34000U, 0xCF044000U, 0xE7075000U,
};

const uint32_t i2s_golden_20bit_R[I2S_GOLDEN_VEC_LEN] = {
	0x00000000U, 0x30FBC000U, 0x5A827000U, 0x76412000U,
	0x7FFFF000U, 0x76412000U, 0x5A827000U, 0x30FBC000U,
	0x00000000U, 0xCF044000U, 0xA57D9000U, 0x89BEE000U,
	0x80001000U, 0x89BEE000U, 0xA57D9000U, 0xCF044000U,
	0x00000000U, 0x30FBC000U, 0x5A827000U, 0x76412000U,
	0x7FFFF000U, 0x76412000U, 0x5A827000U, 0x30FBC000U,
	0x00000000U, 0xCF044000U, 0xA57D9000U, 0x89BEE000U,
	0x80001000U, 0x89BEE000U, 0xA57D9000U, 0xCF044000U,
};

/* =========================================================================
 * 24-bit golden vectors
 *   full-scale peak = 2^23 - 1 = 8388607
 *   MSB-justified shift = 32 - 24 = 8
 * =========================================================================
 */
const uint32_t i2s_golden_24bit_L[I2S_GOLDEN_VEC_LEN] = {
	0x00000000U, 0x18F8B000U, 0x30FB6200U, 0x471CCA00U,
	0x5A82799AU, 0x6A6D98A0U, 0x7641AF00U, 0x7D8A8D00U,
	0x7FFFFF00U, 0x7D8A8D00U, 0x7641AF00U, 0x6A6D98A0U,
	0x5A82799AU, 0x471CCA00U, 0x30FB6200U, 0x18F8B000U,
	0x00000000U, 0xE7074F00U, 0xCF049E00U, 0xB8E33600U,
	0xA57D8700U, 0x95926760U, 0x89BE5100U, 0x82757300U,
	0x80000100U, 0x82757300U, 0x89BE5100U, 0x95926760U,
	0xA57D8700U, 0xB8E33600U, 0xCF049E00U, 0xE7074F00U,
};

const uint32_t i2s_golden_24bit_R[I2S_GOLDEN_VEC_LEN] = {
	0x00000000U, 0x30FB6200U, 0x5A82799AU, 0x7641AF00U,
	0x7FFFFF00U, 0x7641AF00U, 0x5A82799AU, 0x30FB6200U,
	0x00000000U, 0xCF049E00U, 0xA57D8700U, 0x89BE5100U,
	0x80000100U, 0x89BE5100U, 0xA57D8700U, 0xCF049E00U,
	0x00000000U, 0x30FB6200U, 0x5A82799AU, 0x7641AF00U,
	0x7FFFFF00U, 0x7641AF00U, 0x5A82799AU, 0x30FB6200U,
	0x00000000U, 0xCF049E00U, 0xA57D8700U, 0x89BE5100U,
	0x80000100U, 0x89BE5100U, 0xA57D8700U, 0xCF049E00U,
};

/* =========================================================================
 * 32-bit golden vectors
 *   full-scale peak = 2^31 - 1 = 2147483647
 *   MSB-justified shift = 0
 * =========================================================================
 */
const uint32_t i2s_golden_32bit_L[I2S_GOLDEN_VEC_LEN] = {
	0x00000000U, 0x18F8B83CU, 0x30FBC54DU, 0x471CECE7U,
	0x5A82799AU, 0x6A6D98A4U, 0x7641AF3DU, 0x7D8A8D3EU,
	0x7FFFFFFFU, 0x7D8A8D3EU, 0x7641AF3DU, 0x6A6D98A4U,
	0x5A82799AU, 0x471CECE7U, 0x30FBC54DU, 0x18F8B83CU,
	0x00000000U, 0xE7074BC4U, 0xCF043AB3U, 0xB8E31319U,
	0xA57D8666U, 0x9592675CU, 0x89BE50C3U, 0x827572C2U,
	0x80000001U, 0x827572C2U, 0x89BE50C3U, 0x9592675CU,
	0xA57D8666U, 0xB8E31319U, 0xCF043AB3U, 0xE7074BC4U,
};

const uint32_t i2s_golden_32bit_R[I2S_GOLDEN_VEC_LEN] = {
	0x00000000U, 0x30FBC54DU, 0x5A82799AU, 0x7641AF3DU,
	0x7FFFFFFFU, 0x7641AF3DU, 0x5A82799AU, 0x30FBC54DU,
	0x00000000U, 0xCF043AB3U, 0xA57D8666U, 0x89BE50C3U,
	0x80000001U, 0x89BE50C3U, 0xA57D8666U, 0xCF043AB3U,
	0x00000000U, 0x30FBC54DU, 0x5A82799AU, 0x7641AF3DU,
	0x7FFFFFFFU, 0x7641AF3DU, 0x5A82799AU, 0x30FBC54DU,
	0x00000000U, 0xCF043AB3U, 0xA57D8666U, 0x89BE50C3U,
	0x80000001U, 0x89BE50C3U, 0xA57D8666U, 0xCF043AB3U,
};

/* =========================================================================
 * Test matrix: 8 sample rates × 5 bit depths = 40 entries
 * =========================================================================
 */
const struct i2s_golden_tc i2s_golden_matrix[] = {
	{ .name = "8kHz_12bit",    .sample_rate =   8000U, .word_size = 12U,
	  .vec_L = i2s_golden_12bit_L, .vec_R = i2s_golden_12bit_R },
	{ .name = "8kHz_16bit",    .sample_rate =   8000U, .word_size = 16U,
	  .vec_L = i2s_golden_16bit_L, .vec_R = i2s_golden_16bit_R },
	{ .name = "8kHz_20bit",    .sample_rate =   8000U, .word_size = 20U,
	  .vec_L = i2s_golden_20bit_L, .vec_R = i2s_golden_20bit_R },
	{ .name = "8kHz_24bit",    .sample_rate =   8000U, .word_size = 24U,
	  .vec_L = i2s_golden_24bit_L, .vec_R = i2s_golden_24bit_R },
	{ .name = "8kHz_32bit",    .sample_rate =   8000U, .word_size = 32U,
	  .vec_L = i2s_golden_32bit_L, .vec_R = i2s_golden_32bit_R },

	{ .name = "16kHz_12bit",   .sample_rate =  16000U, .word_size = 12U,
	  .vec_L = i2s_golden_12bit_L, .vec_R = i2s_golden_12bit_R },
	{ .name = "16kHz_16bit",   .sample_rate =  16000U, .word_size = 16U,
	  .vec_L = i2s_golden_16bit_L, .vec_R = i2s_golden_16bit_R },
	{ .name = "16kHz_20bit",   .sample_rate =  16000U, .word_size = 20U,
	  .vec_L = i2s_golden_20bit_L, .vec_R = i2s_golden_20bit_R },
	{ .name = "16kHz_24bit",   .sample_rate =  16000U, .word_size = 24U,
	  .vec_L = i2s_golden_24bit_L, .vec_R = i2s_golden_24bit_R },
	{ .name = "16kHz_32bit",   .sample_rate =  16000U, .word_size = 32U,
	  .vec_L = i2s_golden_32bit_L, .vec_R = i2s_golden_32bit_R },

	{ .name = "32kHz_12bit",   .sample_rate =  32000U, .word_size = 12U,
	  .vec_L = i2s_golden_12bit_L, .vec_R = i2s_golden_12bit_R },
	{ .name = "32kHz_16bit",   .sample_rate =  32000U, .word_size = 16U,
	  .vec_L = i2s_golden_16bit_L, .vec_R = i2s_golden_16bit_R },
	{ .name = "32kHz_20bit",   .sample_rate =  32000U, .word_size = 20U,
	  .vec_L = i2s_golden_20bit_L, .vec_R = i2s_golden_20bit_R },
	{ .name = "32kHz_24bit",   .sample_rate =  32000U, .word_size = 24U,
	  .vec_L = i2s_golden_24bit_L, .vec_R = i2s_golden_24bit_R },
	{ .name = "32kHz_32bit",   .sample_rate =  32000U, .word_size = 32U,
	  .vec_L = i2s_golden_32bit_L, .vec_R = i2s_golden_32bit_R },

	{ .name = "44k1Hz_12bit",  .sample_rate =  44100U, .word_size = 12U,
	  .vec_L = i2s_golden_12bit_L, .vec_R = i2s_golden_12bit_R },
	{ .name = "44k1Hz_16bit",  .sample_rate =  44100U, .word_size = 16U,
	  .vec_L = i2s_golden_16bit_L, .vec_R = i2s_golden_16bit_R },
	{ .name = "44k1Hz_20bit",  .sample_rate =  44100U, .word_size = 20U,
	  .vec_L = i2s_golden_20bit_L, .vec_R = i2s_golden_20bit_R },
	{ .name = "44k1Hz_24bit",  .sample_rate =  44100U, .word_size = 24U,
	  .vec_L = i2s_golden_24bit_L, .vec_R = i2s_golden_24bit_R },
	{ .name = "44k1Hz_32bit",  .sample_rate =  44100U, .word_size = 32U,
	  .vec_L = i2s_golden_32bit_L, .vec_R = i2s_golden_32bit_R },

	{ .name = "48kHz_12bit",   .sample_rate =  48000U, .word_size = 12U,
	  .vec_L = i2s_golden_12bit_L, .vec_R = i2s_golden_12bit_R },
	{ .name = "48kHz_16bit",   .sample_rate =  48000U, .word_size = 16U,
	  .vec_L = i2s_golden_16bit_L, .vec_R = i2s_golden_16bit_R },
	{ .name = "48kHz_20bit",   .sample_rate =  48000U, .word_size = 20U,
	  .vec_L = i2s_golden_20bit_L, .vec_R = i2s_golden_20bit_R },
	{ .name = "48kHz_24bit",   .sample_rate =  48000U, .word_size = 24U,
	  .vec_L = i2s_golden_24bit_L, .vec_R = i2s_golden_24bit_R },
	{ .name = "48kHz_32bit",   .sample_rate =  48000U, .word_size = 32U,
	  .vec_L = i2s_golden_32bit_L, .vec_R = i2s_golden_32bit_R },

	{ .name = "88k2Hz_12bit",  .sample_rate =  88200U, .word_size = 12U,
	  .vec_L = i2s_golden_12bit_L, .vec_R = i2s_golden_12bit_R },
	{ .name = "88k2Hz_16bit",  .sample_rate =  88200U, .word_size = 16U,
	  .vec_L = i2s_golden_16bit_L, .vec_R = i2s_golden_16bit_R },
	{ .name = "88k2Hz_20bit",  .sample_rate =  88200U, .word_size = 20U,
	  .vec_L = i2s_golden_20bit_L, .vec_R = i2s_golden_20bit_R },
	{ .name = "88k2Hz_24bit",  .sample_rate =  88200U, .word_size = 24U,
	  .vec_L = i2s_golden_24bit_L, .vec_R = i2s_golden_24bit_R },
	{ .name = "88k2Hz_32bit",  .sample_rate =  88200U, .word_size = 32U,
	  .vec_L = i2s_golden_32bit_L, .vec_R = i2s_golden_32bit_R },

	{ .name = "96kHz_12bit",   .sample_rate =  96000U, .word_size = 12U,
	  .vec_L = i2s_golden_12bit_L, .vec_R = i2s_golden_12bit_R },
	{ .name = "96kHz_16bit",   .sample_rate =  96000U, .word_size = 16U,
	  .vec_L = i2s_golden_16bit_L, .vec_R = i2s_golden_16bit_R },
	{ .name = "96kHz_20bit",   .sample_rate =  96000U, .word_size = 20U,
	  .vec_L = i2s_golden_20bit_L, .vec_R = i2s_golden_20bit_R },
	{ .name = "96kHz_24bit",   .sample_rate =  96000U, .word_size = 24U,
	  .vec_L = i2s_golden_24bit_L, .vec_R = i2s_golden_24bit_R },
	{ .name = "96kHz_32bit",   .sample_rate =  96000U, .word_size = 32U,
	  .vec_L = i2s_golden_32bit_L, .vec_R = i2s_golden_32bit_R },

	{ .name = "192kHz_12bit",  .sample_rate = 192000U, .word_size = 12U,
	  .vec_L = i2s_golden_12bit_L, .vec_R = i2s_golden_12bit_R },
	{ .name = "192kHz_16bit",  .sample_rate = 192000U, .word_size = 16U,
	  .vec_L = i2s_golden_16bit_L, .vec_R = i2s_golden_16bit_R },
	{ .name = "192kHz_20bit",  .sample_rate = 192000U, .word_size = 20U,
	  .vec_L = i2s_golden_20bit_L, .vec_R = i2s_golden_20bit_R },
	{ .name = "192kHz_24bit",  .sample_rate = 192000U, .word_size = 24U,
	  .vec_L = i2s_golden_24bit_L, .vec_R = i2s_golden_24bit_R },
	{ .name = "192kHz_32bit",  .sample_rate = 192000U, .word_size = 32U,
	  .vec_L = i2s_golden_32bit_L, .vec_R = i2s_golden_32bit_R },
};

const size_t i2s_golden_matrix_len = ARRAY_SIZE(i2s_golden_matrix);

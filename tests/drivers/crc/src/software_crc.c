/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <stdbool.h>
#include "software_crc.h"
#include <zephyr/sys/util.h>

typedef struct crc_config_t {
	int id;
	int degree;
	unsigned long poly;
} crc_config_t;

#define CRC_START_32 0xFFFFFFFFL

static const crc_config_t crc_config[] = {
	{CRC32_PRIME, 32, CRC32_POLY_NORMAL},
	{CRC32C_PRIME, 32, CRC32C_HW_POLY},
	{CRC16_802_15_4, 16, CRC16_CCITT_HW_POLY},
	{CRC16_ALT, 16, CRC16_ALT_HW_POLY},
	{CRC8_PRIME, 8, CRC8_HW_POLY},
};

static unsigned long crc_table[256] = {0};

static const crc_config_t *find_config(int id)
{
	size_t i;

	for (i = 0; i < ARRAY_SIZE(crc_config); i++) {
		if (crc_config[i].id == id) {
			return &crc_config[i];
		}
	}
	return NULL;
}

/*---------------------------------------------------------------------------*/
/* Table-driven version */
/*---------------------------------------------------------------------------*/
static unsigned long generate_mask(int degree)
{
	unsigned long half = (1ul << (degree / 2)) - 1;

	return half << (degree / 2) | half;
}

static void generate_crc_table(const crc_config_t *config)
{
	int i, j;
	unsigned long bit, crc;
	unsigned long high_bit = (1ul << (config->degree - 1));
	unsigned long mask = generate_mask(config->degree);

	for (i = 0; i < 256; i++) {
		crc = (unsigned long)i << (config->degree - 8);
		for (j = 0; j < 8; j++) {
			bit = crc & high_bit;
			crc <<= 1;
			if (bit) {
				crc ^= config->poly;
			}
		}
		crc_table[i] = crc & mask;
	}
}

/*****************************************************************************/
/* gen_crc - Return the CRC value for the data using the given CRC algorithm */
/* int id : identifies the CRC algorithm */
/* char *data : the data */
/* size_t len : the size of the data */
/*****************************************************************************/
unsigned long gen_crc(int id, const unsigned char *data, size_t len)
{
	const crc_config_t *config = find_config(id);
	unsigned long crc = 0;
	unsigned long mask;
	size_t i;

	if (config == NULL) {
		return 0U;
	}

	mask = generate_mask(config->degree);

	if ((id == CRC32_PRIME) || (id == CRC32C_PRIME)) {
		crc = 0xFFFFFFFFUL;
	}

	generate_crc_table(config);
	for (i = 0; i < len; i++) {
		unsigned int datum = data[i];

		/*
		 * This loop handles 16-bit chars when we compile on 16-bit
		 * machines.
		 */
		int n;

		for (n = 0; n < (CHAR_BIT / 8); n++) {
			unsigned long octet = ((datum >> (8 * n)) & 0xff);
			unsigned long term1 = (crc << 8);
			int idx =
				((crc >> (config->degree - 8)) & 0xff) ^ octet;

			crc = term1 ^ crc_table[idx];
		}
	}
	return crc & mask;
}

/*****************************************************************************/
/* 32 Bit CRC */
/*****************************************************************************/
static void init_crc32_tab(unsigned long CRC_POLY_32);
static uint32_t crc_tab32[256];

/*
 * uint32_t crc_32(const unsigned char *input_str, size_t num_bytes,
 *                  unsigned long CRC_POLY_32);
 *
 * The function crc_32() calculates in one pass the common 32 bit CRC value for
 * a byte string that is passed to the function together with a parameter
 * indicating the length.
 */
uint32_t crc_32(const unsigned char *input_str, size_t num_bytes,
		unsigned long CRC_POLY_32)
{

	uint32_t crc;
	uint32_t tmp;
	uint32_t long_c;
	const unsigned char *ptr;
	size_t a;

	init_crc32_tab(CRC_POLY_32);
	crc = CRC_START_32;
	ptr = input_str;
	if (ptr != NULL) {
		for (a = 0; a < num_bytes; a++) {
			long_c = 0x000000FFL & (uint32_t)*ptr;
			tmp = crc ^ long_c;
			crc = (crc >> 8) ^ crc_tab32[tmp & 0xff];
			ptr++;
		}
	}
	crc ^= 0xffffffffL;
	return crc & 0xffffffffL;
}

/*
 * static void init_crc32_tab(unsigned long CRC_POLY_32);
 *
 * For optimal speed, the CRC32 calculation uses a table with pre-calculated
 * bit patterns which are used in the XOR operations in the program. This table
 * is generated once, the first time the CRC update routine is called.
 */
static void init_crc32_tab(unsigned long CRC_POLY_32)
{
	uint32_t i;
	uint32_t j;
	uint32_t crc;

	for (i = 0; i < 256; i++) {
		crc = i;
		for (j = 0; j < 8; j++) {
			if (crc & 0x00000001L) {
				crc = (crc >> 1) ^ CRC_POLY_32;
			} else {
				crc = crc >> 1;
			}
		}
		crc_tab32[i] = crc;
	}
}

uint32_t crc32_update_msb(uint32_t crc, uint8_t data, uint32_t poly)
{
	crc ^= ((uint32_t)data << 24);

	for (int bit = 0; bit < 8; bit++) {
		if (crc & 0x80000000U) {
			crc = (crc << 1) ^ poly;
		} else {
			crc <<= 1;
		}
	}

	return crc;
}

uint32_t crc32_update_msb_word(uint32_t crc, const uint8_t *word, bool bit_swap,
			       bool byte_swap, uint32_t poly)
{
	for (int i = 0; i < 4; i++) {
		const int src = byte_swap ? i : (3 - i);
		uint8_t byte = word[src];

		if (bit_swap) {
			byte = (uint8_t)bit_reflect_n(byte, 8);
		}

		crc = crc32_update_msb(crc, byte, poly);
	}

	return crc;
}

uint32_t crc32_sw_unaligned(uint32_t crc, const uint8_t *input, uint32_t length,
			    uint32_t poly)
{
	uint32_t poly_refl = bit_reflect_32(poly);

	for (uint32_t byte_idx = 0; byte_idx < length; byte_idx++) {
		uint8_t data = input[byte_idx];

		for (uint32_t bit_idx = 0; bit_idx < 8; bit_idx++) {
			uint32_t check_bit = (crc ^ data) & 1U;

			crc >>= 1;
			if (check_bit) {
				crc ^= poly_refl;
			}
			data >>= 1;
		}
	}

	return ~crc;
}

uint32_t crc32_driver_reference(const uint8_t *data, size_t len, uint32_t seed,
				bool bit_swap, bool byte_swap, bool reflect,
				bool invert, uint32_t poly_normal,
				uint32_t poly_reflected)
{
	uint32_t crc = seed;
	uint32_t aligned_len = len - (len % 4);
	uint32_t unaligned_len = len % 4;
	const uint8_t *tail = data + aligned_len;

	ARG_UNUSED(poly_reflected);

	/* --- Aligned phase: feed LE32 words (MSB-first engine) --- */
	for (uint32_t off = 0; off < aligned_len; off += 4) {
		crc = crc32_update_msb_word(crc, &data[off], bit_swap,
					    byte_swap, poly_normal);
	}

	/*
	 * HW applies reflect + invert to CRC_OUT automatically.
	 * Apply the same transforms so we match the value that
	 * crc_calculate_32bit_unaligned_sw receives as input.
	 */
	if (reflect) {
		crc = bit_reflect_32(crc);
	}
	if (invert) {
		crc = ~crc;
	}

	/* --- Unaligned tail: replicate driver SW fallback --- */
	if (unaligned_len > 0) {
		if (invert) {
			crc = ~crc;
		}
		if (!reflect) {
			crc = bit_reflect_32(crc);
		}

		crc = crc32_sw_unaligned(crc, tail, unaligned_len, poly_normal);

		if (!reflect) {
			crc = bit_reflect_32(crc);
		}
		if (!invert) {
			crc = ~crc;
		}
	}

	return crc;
}

uint32_t crc32_full_reference(const uint8_t *data, size_t len)
{
	return crc32_driver_reference(data, len, 0xFFFFFFFFU, true, true, true,
				      true, CRC32_POLY_NORMAL,
				      CRC32_POLY_REFLECTED);
}

uint32_t crc32c_full_reference(const uint8_t *data, size_t len)
{
	return crc32_driver_reference(data, len, 0xFFFFFFFFU, true, true, true,
				      true, CRC32C_HW_POLY,
				      CRC32C_SW_POLY_REFL);
}

uint32_t custom_poly_crc(const uint8_t *data, size_t len, uint32_t poly,
			 uint32_t bits)
{
	uint32_t crc = 0; /* Seed 0, no invert */
	uint32_t mask = (bits == 32U) ? UINT32_MAX : ((1U << bits) - 1U);

	for (size_t i = 0; i < len; i++) {
		crc ^= (uint32_t)data[i] << (bits - 8U);
		for (int j = 0; j < 8; j++) {
			if (crc & BIT(bits - 1U)) {
				crc = (crc << 1) ^ poly;
			} else {
				crc <<= 1;
			}
			crc &= mask;
		}
	}

	return crc;
}


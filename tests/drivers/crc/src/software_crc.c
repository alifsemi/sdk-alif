// SPDX-License-Identifier: Apache-2.0

/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include "software_crc.h"
#include <zephyr/sys/util.h>

typedef struct crc_config_t {
	int id;
	int degree;
	unsigned long poly;
} crc_config_t;

#define		CRC_START_32		0xFFFFFFFFL

static struct crc_config_t crc_config[] = {
	{CRC32_PRIME, 32, 0x04C11DB7},
	{CRC16_802_15_4, 16, 0x1021},
	{CRC16_ALT, 16, 0x8005},
	{CRC8_PRIME, 8, 0x07}
};

static unsigned long crc_table[256] = { 0 };

const crc_config_t *find_config(int id)
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
unsigned long generate_mask(int degree)
{
	unsigned long half = (1ul << (degree / 2)) - 1;

	return half << (degree / 2) | half;
}

void generate_crc_table(const crc_config_t *config)
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
		return 0;
	}

	mask = generate_mask(config->degree);

	if (id == CRC32_PRIME) {
		crc = 0xFFFFFFFF;
	} else if (id == CRC16_802_15_4) {
		crc = 0;
	} else if (id == CRC8_PRIME) {
		crc = 0;
	} else if (id == CRC16_ALT) {
		crc = 0;
	}

	generate_crc_table(config);
	for (i = 0; i < len; i++) {
		unsigned int datum = data[i];

/*--------------------------------------------------------------------*/
/* This loop handles 16-bit chars when we compile on 16-bit machines. */
/*--------------------------------------------------------------------*/
		int n;

		for (n = 0; n < (CHAR_BIT / 8); n++) {
			unsigned long octet = ((datum >> (8 * n)) & 0xff);
			unsigned long term1 = (crc << 8);
			int idx = ((crc >> (config->degree - 8)) & 0xff) ^
				   octet;

			crc = term1 ^ crc_table[idx];
		}
	}
	return crc & mask;
}

/*****************************************************************************/
/* 32 Bit CRC */
/*****************************************************************************/
static void init_crc32_tab(unsigned long CRC_POLY_32);
static uint32_t	 crc_tab32[256];

/*
 * uint32_t crc_32( const unsigned char *input_str, size_t num_bytes );
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
 * static void init_crc32_tab( void );
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

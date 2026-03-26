/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */
#ifndef SOFTWARE_CRC_H_
#define SOFTWARE_CRC_H_

#include <stddef.h>
#include <stdint.h>

#include <zephyr/device.h>
#include <zephyr/drivers/crc.h>
#include <zephyr/kernel.h>
#include <zephyr/ztest.h>

/* Software reference algorithm identifiers used by gen_crc(). */
#define CRC32_PRIME 0
#define CRC16_802_15_4 1
#define CRC16_ALT 2
#define CRC8_PRIME 3
#define CRC32C_PRIME 4

/*
 * Keep this compatibility alias so local test edits that use CRC32C as an ID
 * continue to build. The canonical software-reference ID is CRC32C_PRIME.
 */
#define CRC32C CRC32C_PRIME

#define CRC_TRUE 1
#define CRC_FALSE 0

#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif

/* Standard hardware polynomials used by the software reference model. */
#define CRC8_HW_POLY 0x07UL
#define CRC16_CCITT_HW_POLY 0x1021UL
#define CRC16_HW_POLY 0x8005UL

/* Keep the existing test-facing name for the non-CCITT 16-bit polynomial. */
#define CRC16_ALT_HW_POLY CRC16_HW_POLY

/* Additional polynomials used by the extended test matrix. */
#define CRC16_CUSTOM_TEST_POLY 0xA001UL
#define CRC8_CUSTOM_TEST_POLY 0x9BUL
#define CRC32C_HW_POLY 0x1EDC6F41UL
#define CRC32C_SW_POLY_REFL 0x82F63B78UL
#define CRC32_CUSTOM_HW_POLY 0x2CEEA6C8UL
#define CRC32_CUSTOM_SW_POLY_REFL 0x13657734UL
#define CRC32_POLY_NORMAL 0x04C11DB7UL
#define CRC32_POLY_REFLECTED 0xEDB88320UL

enum test_crc_hw_algo {
	TEST_CRC_HW_ALGO_CRC8_CCITT = 0,
	TEST_CRC_HW_ALGO_CRC16 = 1,
	TEST_CRC_HW_ALGO_CRC16_CCITT = 2,
	TEST_CRC_HW_ALGO_CRC32 = 3,
	TEST_CRC_HW_ALGO_CRC32C = 4,
};

/*
 * Test-side runtime algorithm selection and common device validation.
 *
 * The current driver still fixes its algorithm from devicetree at init time,
 * so the test suite mirrors the first byte of the driver runtime data in
 * order to exercise multiple algorithms from one image.
 */
void crc_test_force_hw_algo(enum test_crc_hw_algo algo);
void crc_test_verify_device(void);

/* Shared test input data (defined in main.c). */
extern const uint8_t crc_test_data[];
extern const size_t crc_test_data_len;

/* CRC device handle (defined in main.c). */
extern const struct device *const crc_dev;

/* Shared CRC check string "123456789" - canonical CRC test vector. */
extern const uint8_t crc_check_string[];

#define CRC_CHECK_STRING_LEN 9

/* Bit-reflect helpers (defined in main.c). */
uint32_t bit_reflect_n(uint32_t input, uint32_t width);
uint32_t bit_reflect_32(uint32_t input);

/* CRC32 driver-path reference helpers. */
uint32_t crc32_update_msb(uint32_t crc, uint8_t data, uint32_t poly);
uint32_t crc32_update_msb_word(uint32_t crc, const uint8_t *word, bool bit_swap,
			       bool byte_swap, uint32_t poly);
uint32_t crc32_sw_unaligned(uint32_t crc, const uint8_t *input, uint32_t length,
			    uint32_t poly);
uint32_t crc32_driver_reference(const uint8_t *data, size_t len, uint32_t seed,
				bool bit_swap, bool byte_swap, bool reflect,
				bool invert, uint32_t poly_normal,
				uint32_t poly_reflected);

/* Convenience functions for standard CRC32 configurations. */
uint32_t crc32_full_reference(const uint8_t *data, size_t len);
uint32_t crc32c_full_reference(const uint8_t *data, size_t len);

unsigned long gen_crc(int id, const unsigned char *data, size_t len);

uint32_t crc_32(const unsigned char *input_str, size_t num_bytes,
		unsigned long crc_poly);

uint32_t custom_poly_crc(const uint8_t *data, size_t len, uint32_t poly,
			 uint32_t bits);

#endif /* SOFTWARE_CRC_H_ */

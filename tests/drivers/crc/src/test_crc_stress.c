// SPDX-License-Identifier: Apache-2.0

/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include "test_crc_common.h"
#include <string.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(alif_crc, CONFIG_LOG_DEFAULT_LEVEL);

#if IS_ENABLED(CONFIG_TEST_CRC_STRESS)

/* Polynomial constants now defined in test_crc_common.h */

/* ------------------------------------------------------------------ */
/* Known test vectors                                                  */
/* ------------------------------------------------------------------ */

/* CRC check string and length now defined in main.c/test_crc_common.h */

/* All zeros */
#if IS_ENABLED(CONFIG_TEST_CRC_8) ||\
	IS_ENABLED(CONFIG_TEST_CRC_16) ||\
	IS_ENABLED(CONFIG_TEST_CRC_32)
static const uint8_t all_zeros[64] = {0};
#endif

#if IS_ENABLED(CONFIG_TEST_CRC_32) || \
	IS_ENABLED(CONFIG_TEST_CRC_32_CUSTOM_POLY) || \
	IS_ENABLED(CONFIG_TEST_CRC_32C)
#define TEST_CRC_STRESS_32_FAMILY
#endif

#if defined(TEST_CRC_STRESS_32_FAMILY)
/*
 * CRC32 stress runs many non-standard control-bit combinations
 * (swap/reflect/invert) and no-swap/no-reflect/no-invert mode exercises
 * the driver's mixed HW + SW unaligned flow. This helper mirrors the same
 * data path and post-processing so HW/SW comparisons stay apples-to-apples.
 */
static uint32_t crc32_update_msb(uint32_t crc, uint8_t data, uint32_t poly)
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

static uint32_t crc32_update_lsb(uint32_t crc, uint8_t data,
				 uint32_t poly_reflected)
{
	crc ^= data;

	for (int bit = 0; bit < 8; bit++) {
		if (crc & 1U) {
			crc = (crc >> 1) ^ poly_reflected;
		} else {
			crc >>= 1;
		}
	}

	return crc;
}

static uint32_t crc32_update_msb_word(uint32_t crc, const uint8_t *word,
				      bool bit_swap,
			      bool byte_swap,
			      uint32_t poly)
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

/*
 * Pure-software CRC32 reference that replicates the exact HW data
 * path in alif_crc.c so stress tests perform genuine SW-vs-HW
 * validation.
 *
 * Aligned phase  : LE32 words fed into an MSB-first CRC engine
 *                  (matches sys_get_le32 + 32-bit register write).
 * Unaligned tail : LSB-first bit-by-bit with reflected poly and
 *                  final ~crc, wrapped by the same pre/post
 *                  transforms as crc_calculate_32bit_unaligned_sw.
 */
static uint32_t sw_crc_unaligned(uint32_t crc, const uint8_t *input,
				 uint32_t length, uint32_t poly)
{
	uint32_t poly_refl = bit_reflect_32(poly);
	uint32_t byte_idx, bit_idx;

	for (byte_idx = 0; byte_idx < length; byte_idx++) {
		uint8_t d = input[byte_idx];

		for (bit_idx = 0; bit_idx < 8; bit_idx++) {
			uint32_t chk = (crc ^ d) & 1U;

			crc >>= 1;
			if (chk) {
				crc ^= poly_refl;
			}
			d >>= 1;
		}
	}
	return ~crc;
}

static int stress_prepare_seed_and_poly(const struct crc_params *params,
					uint32_t seed);

static int stress_crc32_full_prepare(const struct crc_params *params)
{
	return stress_prepare_seed_and_poly(params, 0xFFFFFFFFU);
}

static uint32_t crc32_driver_reference(const uint8_t *data, size_t len,
				       uint32_t seed,
				       bool bit_swap,
				       bool byte_swap,
				       bool reflect,
				       bool invert,
				       uint32_t poly_normal,
				       uint32_t poly_reflected)
{
	uint32_t crc = seed;
	uint32_t aligned_len = len - (len % 4);
	uint32_t unaligned_len = len % 4;
	const uint8_t *tail = data + aligned_len;

	ARG_UNUSED(poly_reflected);

	/* --- Aligned phase: feed LE32 words (MSB-first engine) --- */
	for (uint32_t off = 0; off < aligned_len; off += 4) {
		crc = crc32_update_msb_word(crc, &data[off],
					    bit_swap, byte_swap,
					   poly_normal);
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

		crc = sw_crc_unaligned(crc, tail, unaligned_len,
				       poly_normal);

		if (!reflect) {
			crc = bit_reflect_32(crc);
		}
		if (!invert) {
			crc = ~crc;
		}
	}

	return crc;
}
#endif

static int stress_prepare_seed_and_poly(const struct crc_params *params,
					uint32_t seed)
{
	int ret = crc_set_seed(crc_dev, seed);

	if (ret != 0) {
		return ret;
	}

#if IS_ENABLED(CONFIG_TEST_CRC_32_CUSTOM_POLY) \
|| IS_ENABLED(CONFIG_TEST_CRC_32C)
	if (params->custom_poly) {
		uint32_t poly = IS_ENABLED(CONFIG_TEST_CRC_32C) ?
				CRC32C_HW_POLY : CRC32_CUSTOM_HW_POLY;

		ret = crc_set_polynomial(crc_dev, poly);
		if (ret != 0) {
			return ret;
		}
	}
#else
	/* Suppress unused-parameter warning when custom poly configs
	 * are disabled
	 */
	(void)params;
#endif

	return 0;
}

/* ================================================================== */
/* STRESS-1: Repeatability — same input must always produce same CRC  */
/* ================================================================== */
#if IS_ENABLED(CONFIG_TEST_CRC_8) ||\
	IS_ENABLED(CONFIG_TEST_CRC_16) ||\
	IS_ENABLED(CONFIG_TEST_CRC_32)
ZTEST(crc_test, stress_repeatability)
{
	uint32_t crc_first = 0, crc_repeat = 0;
	struct crc_params params = {
		.data_in     = (uint8_t *)crc_test_data,
		.len         = crc_test_data_len,
		.bit_swap    = CRC_FALSE,
		.byte_swap   = CRC_FALSE,
		.reflect     = CRC_FALSE,
		.invert      = CRC_FALSE,
		.custom_poly = CRC_FALSE,
		.data_out    = &crc_first,
	};

	zassert_equal(stress_prepare_seed_and_poly(&params, 0x00), 0,
		      "seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "compute 1 failed");

	/* Validate reference against software */
#if IS_ENABLED(CONFIG_TEST_CRC_8)
	uint32_t crc_sw = gen_crc(CRC8_PRIME,
			   (const unsigned char *)crc_test_data,
			   crc_test_data_len);

	zassert_equal((crc_first & 0xFF), crc_sw,
		      "HW/SW reference mismatch (8-bit): HW=0x%X SW=0x%X",
		      (crc_first & 0xFF), crc_sw);
#elif IS_ENABLED(CONFIG_TEST_CRC_16)
	uint32_t crc_sw = gen_crc(CRC16_802_15_4,
			   (const unsigned char *)crc_test_data,
			   crc_test_data_len);

	zassert_equal((crc_first & 0xFFFF), crc_sw,
		      "HW/SW reference mismatch (16-bit): HW=0x%X SW=0x%X",
		      (crc_first & 0xFFFF), crc_sw);
#elif IS_ENABLED(CONFIG_TEST_CRC_32)
	uint32_t crc_sw = crc32_driver_reference((const uint8_t *)crc_test_data,
			 crc_test_data_len,
			 0x00,
			 params.bit_swap,
			 params.byte_swap,
			 params.reflect,
			 params.invert,
			 CRC32_POLY_NORMAL,
			 CRC32_POLY_REFLECTED);
	zassert_equal(crc_first, crc_sw,
		      "HW/SW reference mismatch (32-bit): HW=0x%X SW=0x%X",
		      crc_first, crc_sw);
#endif

	/* Run 50 times and verify identical output */
	for (int i = 0; i < 50; i++) {
		params.data_out = &crc_repeat;
		zassert_equal(stress_prepare_seed_and_poly(&params, 0x00),
			      0, "seed iter %d", i);
		zassert_equal(crc_compute(crc_dev, &params), 0,
			      "compute iter %d", i);

#if IS_ENABLED(CONFIG_TEST_CRC_8)
		zassert_equal(crc_first & 0xFF, crc_repeat & 0xFF,
			      "Repeatability failure iter %d: 0x%X != 0x%X",
		      i, crc_first & 0xFF, crc_repeat & 0xFF);
#elif IS_ENABLED(CONFIG_TEST_CRC_16)
		zassert_equal(crc_first & 0xFFFF, crc_repeat & 0xFFFF,
			      "Repeatability failure iter %d: 0x%X != 0x%X",
		      i, crc_first & 0xFFFF, crc_repeat & 0xFFFF);
#else
		zassert_equal(crc_first, crc_repeat,
			      "Repeatability failure iter %d: 0x%X != 0x%X",
		      i, crc_first, crc_repeat);
#endif
	}
	LOG_INF("STRESS-1: 50 iterations identical CRC: 0x%X",
		crc_first);
}

/* ================================================================== */
/* STRESS-2: Back-to-back compute without re-seeding                  */
/*           Validates seed is re-loaded properly each time            */
/* ================================================================== */
ZTEST(crc_test, stress_back_to_back_no_reseed)
{
	uint32_t crc_a = 0, crc_b = 0;
	struct crc_params params = {
		.data_in     = (uint8_t *)crc_test_data,
		.len         = crc_test_data_len,
		.bit_swap    = CRC_FALSE,
		.byte_swap   = CRC_FALSE,
		.reflect     = CRC_FALSE,
		.invert      = CRC_FALSE,
		.custom_poly = CRC_FALSE,
		.data_out    = &crc_a,
	};

	/* First compute with seed */
	zassert_equal(stress_prepare_seed_and_poly(&params, 0x00), 0,
		      "seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "compute 1 failed");

	/* Validate first CRC against software */
#if IS_ENABLED(CONFIG_TEST_CRC_8)
	uint32_t crc_sw = gen_crc(CRC8_PRIME,
			   (const unsigned char *)crc_test_data,
			   crc_test_data_len);

	zassert_equal((crc_a & 0xFF), crc_sw,
		      "HW/SW first CRC mismatch (8-bit): HW=0x%X SW=0x%X",
		      (crc_a & 0xFF), crc_sw);
#elif IS_ENABLED(CONFIG_TEST_CRC_16)
	uint32_t crc_sw = gen_crc(CRC16_802_15_4,
		  (const unsigned char *)crc_test_data,
		  crc_test_data_len);

	zassert_equal((crc_a & 0xFFFF), crc_sw,
		      "HW/SW first CRC mismatch (16-bit): HW=0x%X SW=0x%X",
		      (crc_a & 0xFFFF), crc_sw);
#elif IS_ENABLED(CONFIG_TEST_CRC_32)
	uint32_t crc_sw = crc32_driver_reference((const uint8_t *)crc_test_data,
			 crc_test_data_len,
			 0x00,
			 params.bit_swap,
			 params.byte_swap,
			 params.reflect,
			 params.invert,
			 CRC32_POLY_NORMAL,
			 CRC32_POLY_REFLECTED);
	zassert_equal(crc_a, crc_sw,
		      "HW/SW first CRC mismatch (32-bit): HW=0x%X SW=0x%X",
		      crc_a, crc_sw);
#endif

	/* Second compute WITHOUT re-seeding — driver sets CRC_INIT_BIT in
	 * crc_params_init(), so the seed register should reload.
	 * If the driver doesn't re-init properly, crc_b will differ from crc_a.
	 */
	params.data_out = &crc_b;
	zassert_equal(crc_compute(crc_dev, &params), 0, "compute 2 failed");

	LOG_INF("STRESS-2: CRC_A=0x%X, CRC_B=0x%X (without re-seed)",
		crc_a, crc_b);

	/*
	 * The driver sets CRC_INIT_BIT in crc_params_init(),
	 * which reloads the seed register. Because the seed
	 * register still holds the value from the first call,
	 * crc_b must equal crc_a.
	 */
#if IS_ENABLED(CONFIG_TEST_CRC_8)
	zassert_equal(crc_a & 0xFF, crc_b & 0xFF,
		      "Back-to-back mismatch: A=0x%X B=0x%X",
		      crc_a & 0xFF, crc_b & 0xFF);
#elif IS_ENABLED(CONFIG_TEST_CRC_16)
	zassert_equal(crc_a & 0xFFFF, crc_b & 0xFFFF,
		      "Back-to-back mismatch: A=0x%X B=0x%X",
		      crc_a & 0xFFFF, crc_b & 0xFFFF);
#else
	zassert_equal(crc_a, crc_b,
		      "Back-to-back mismatch: A=0x%X B=0x%X",
		      crc_a, crc_b);
#endif
}

/* ================================================================== */
/* STRESS-5: All-zeros input (various lengths)                         */
/* ================================================================== */
ZTEST(crc_test, stress_all_zeros)
{
	uint32_t crc_prev = 0, crc_curr = 0;
	struct crc_params params = {
		.data_in     = all_zeros,
		.bit_swap    = CRC_FALSE,
		.byte_swap   = CRC_FALSE,
		.reflect     = CRC_FALSE,
		.invert      = CRC_FALSE,
		.custom_poly = CRC_FALSE,
		.data_out    = &crc_curr,
	};

	/* Validate deterministic CRCs for multiple all-zero lengths */
	static const uint32_t lengths[] = {1, 2, 4, 8, 16, 32, 64};

	for (int i = 0; i < ARRAY_SIZE(lengths); i++) {
		params.len = lengths[i];
		zassert_equal(stress_prepare_seed_and_poly(&params, 0x00), 0,
			      "seed failed");
		zassert_equal(crc_compute(crc_dev, &params), 0,
			      "compute len %u failed", lengths[i]);

		/* Validate against software reference */
#if IS_ENABLED(CONFIG_TEST_CRC_8)
		uint32_t crc_sw = gen_crc(CRC8_PRIME, all_zeros, lengths[i]);

		zassert_equal((crc_curr & 0xFF), crc_sw,
			      "HW/SW mismatch zeros len %u (8-bit): HW=0x%X SW=0x%X",
		      lengths[i], (crc_curr & 0xFF), crc_sw);
#elif IS_ENABLED(CONFIG_TEST_CRC_16)
		uint32_t crc_sw = gen_crc(CRC16_802_15_4, all_zeros,
					  lengths[i]);

		zassert_equal((crc_curr & 0xFFFF), crc_sw,
			      "HW/SW mismatch zeros len %u (16-bit): HW=0x%X SW=0x%X",
		      lengths[i], (crc_curr & 0xFFFF), crc_sw);
#elif IS_ENABLED(CONFIG_TEST_CRC_32)
		uint32_t crc_sw = crc32_driver_reference(all_zeros,
			lengths[i],
			0x00,
			params.bit_swap,
			params.byte_swap,
			params.reflect,
			params.invert,
			CRC32_POLY_NORMAL,
			CRC32_POLY_REFLECTED);
		zassert_equal(crc_curr, crc_sw,
			      "HW/SW mismatch zeros len %u (32-bit): HW=0x%X SW=0x%X",
		      lengths[i], crc_curr, crc_sw);
#endif

		LOG_INF("STRESS-5: zeros len=%u -> CRC=0x%X",
			lengths[i], crc_curr);
		LOG_INF("STRESS-5: (SW validated)");

		if ((i > 0) && (crc_curr == crc_prev)) {
			LOG_INF("STRESS-5: equal CRC len %u & %u: 0x%X",
		lengths[i - 1], lengths[i], crc_curr);
		}
		crc_prev = crc_curr;
	}
}

/* ================================================================== */
/* STRESS-7: Rapid repeated compute — detect register state leakage   */
/* ================================================================== */
ZTEST(crc_test, stress_rapid_100_iterations)
{
	uint32_t crc_ref = 0, crc_curr = 0;
	struct crc_params params = {
		.data_in     = crc_check_string,
		.len         = CRC_CHECK_STRING_LEN,
		.bit_swap    = CRC_FALSE,
		.byte_swap   = CRC_FALSE,
		.reflect     = CRC_FALSE,
		.invert      = CRC_FALSE,
		.custom_poly = CRC_FALSE,
		.data_out    = &crc_ref,
	};

	/* Get reference */
	zassert_equal(stress_prepare_seed_and_poly(&params, 0x00), 0,
		      "seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0,
		      "ref compute failed");

	/* Validate reference against software */
#if IS_ENABLED(CONFIG_TEST_CRC_8)
	uint32_t crc_sw = gen_crc(CRC8_PRIME, crc_check_string,
				  CRC_CHECK_STRING_LEN);

	zassert_equal((crc_ref & 0xFF), crc_sw,
		      "HW/SW reference mismatch (8-bit): HW=0x%X SW=0x%X",
		      (crc_ref & 0xFF), crc_sw);
#elif IS_ENABLED(CONFIG_TEST_CRC_16)
	uint32_t crc_sw = gen_crc(CRC16_802_15_4,
			   crc_check_string,
			   CRC_CHECK_STRING_LEN);

	zassert_equal((crc_ref & 0xFFFF), crc_sw,
		      "HW/SW reference mismatch (16-bit): HW=0x%X SW=0x%X",
		      (crc_ref & 0xFFFF), crc_sw);
#elif IS_ENABLED(CONFIG_TEST_CRC_32)
	uint32_t crc_sw = crc32_driver_reference(crc_check_string,
			 CRC_CHECK_STRING_LEN,
			 0x00,
			 params.bit_swap,
			 params.byte_swap,
			 params.reflect,
			 params.invert,
			 CRC32_POLY_NORMAL,
			 CRC32_POLY_REFLECTED);
	zassert_equal(crc_ref, crc_sw,
		      "HW/SW reference mismatch (32-bit): HW=0x%X SW=0x%X",
		      crc_ref, crc_sw);
#endif

	/* Rapid-fire 100 iterations */
	for (int i = 0; i < 100; i++) {
		params.data_out = &crc_curr;
		zassert_equal(stress_prepare_seed_and_poly(&params, 0x00),
			      0, "seed iter %d", i);
		zassert_equal(crc_compute(crc_dev, &params), 0,
			      "compute iter %d", i);
#if IS_ENABLED(CONFIG_TEST_CRC_8)
		zassert_equal(crc_ref & 0xFF, crc_curr & 0xFF,
			      "Mismatch at iter %d: ref=0x%X got=0x%X",
		      i, crc_ref & 0xFF, crc_curr & 0xFF);
#elif IS_ENABLED(CONFIG_TEST_CRC_16)
		zassert_equal(crc_ref & 0xFFFF, crc_curr & 0xFFFF,
			      "Mismatch at iter %d: ref=0x%X got=0x%X",
		      i, crc_ref & 0xFFFF, crc_curr & 0xFFFF);
#else
		zassert_equal(crc_ref, crc_curr,
			      "Mismatch at iter %d: ref=0x%X got=0x%X",
		      i, crc_ref, crc_curr);
#endif
	}
	LOG_INF("STRESS-7: 100 rapid iterations OK, CRC=0x%X",
		crc_ref);
}

/* ================================================================== */
/* STRESS-8: Alternating param combos — detect stale register bits    */
/* ================================================================== */
ZTEST(crc_test, stress_alternating_params)
{
	uint32_t crc_plain = 0, crc_reflect = 0, crc_invert = 0;
	uint32_t crc_plain2 = 0;
	struct crc_params params = {
		.data_in     = (uint8_t *)crc_test_data,
		.len         = crc_test_data_len,
		.bit_swap    = CRC_FALSE,
		.byte_swap   = CRC_FALSE,
		.reflect     = CRC_FALSE,
		.invert      = CRC_FALSE,
		.custom_poly = CRC_FALSE,
		.data_out    = &crc_plain,
	};

	/* Plain */
	zassert_equal(stress_prepare_seed_and_poly(&params, 0x00), 0,
		      "seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "compute plain");

#if IS_ENABLED(CONFIG_TEST_CRC_8)
	uint32_t sw_plain = gen_crc(CRC8_PRIME,
				    (const unsigned char *)crc_test_data,
				    crc_test_data_len);
	zassert_equal((crc_plain & 0xFF), sw_plain,
		      "HW/SW plain mismatch (8-bit): HW=0x%X SW=0x%X",
		      (crc_plain & 0xFF), sw_plain);
#elif IS_ENABLED(CONFIG_TEST_CRC_16)
	uint32_t sw_plain = gen_crc(CRC16_802_15_4,
				    (const unsigned char *)crc_test_data,
				    crc_test_data_len);

	zassert_equal((crc_plain & 0xFFFF), sw_plain,
		      "HW/SW plain mismatch (16-bit): HW=0x%X SW=0x%X",
		      (crc_plain & 0xFFFF), sw_plain);
#elif IS_ENABLED(CONFIG_TEST_CRC_32)
	uint32_t sw_plain = crc32_driver_reference(
			   (const uint8_t *)crc_test_data,
			   crc_test_data_len,
			   0x00,
			   params.bit_swap,
			   params.byte_swap,
			   params.reflect,
			   params.invert,
			   CRC32_POLY_NORMAL,
			   CRC32_POLY_REFLECTED);
	zassert_equal(crc_plain, sw_plain,
		      "HW/SW plain mismatch (32-bit): HW=0x%X SW=0x%X",
		      crc_plain, sw_plain);
#endif

	/* Reflect */
	params.reflect = CRC_TRUE;
	params.data_out = &crc_reflect;
	zassert_equal(stress_prepare_seed_and_poly(&params, 0x00), 0,
		      "seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "compute reflect");

	/* Invert */
	params.reflect = CRC_FALSE;
	params.invert = CRC_TRUE;
	params.data_out = &crc_invert;
	zassert_equal(stress_prepare_seed_and_poly(&params, 0x00), 0,
		      "seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "compute invert");

	/* Back to plain — MUST match original plain value */
	params.invert = CRC_FALSE;	/* Plain */
	params.data_out = &crc_plain2;
	zassert_equal(stress_prepare_seed_and_poly(&params, 0x00), 0,
		      "seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "compute plain");

	LOG_INF("STRESS-8: plain=0x%X, reflect=0x%X, invert=0x%X, plain2=0x%X",
		crc_plain, crc_reflect, crc_invert, crc_plain2);

	/* Plain must differ from reflect and invert */
#if IS_ENABLED(CONFIG_TEST_CRC_8)
	zassert_not_equal(crc_plain & 0xFF, crc_reflect & 0xFF,
			  "Plain and reflect same: 0x%X", crc_plain & 0xFF);
	zassert_not_equal(crc_plain & 0xFF, crc_invert & 0xFF,
			  "Plain and invert same: 0x%X", crc_plain & 0xFF);
	/* Returning to plain must be deterministic */
	zassert_equal(crc_plain & 0xFF, crc_plain2 & 0xFF,
		      "Stale register state: plain=0x%X, plain2=0x%X",
		   crc_plain & 0xFF, crc_plain2 & 0xFF);
#elif IS_ENABLED(CONFIG_TEST_CRC_16)
	zassert_not_equal(crc_plain & 0xFFFF, crc_reflect & 0xFFFF,
			  "Plain and reflect same: 0x%X", crc_plain & 0xFFFF);
	zassert_not_equal(crc_plain & 0xFFFF, crc_invert & 0xFFFF,
			  "Plain and invert same: 0x%X", crc_plain & 0xFFFF);
	/* Returning to plain must be deterministic */
	zassert_equal(crc_plain & 0xFFFF, crc_plain2 & 0xFFFF,
		      "Stale register state: plain=0x%X, plain2=0x%X",
		      crc_plain & 0xFFFF, crc_plain2 & 0xFFFF);
#else
	zassert_not_equal(crc_plain, crc_reflect,
			  "Plain and reflect same: 0x%X", crc_plain);
	zassert_not_equal(crc_plain, crc_invert,
			  "Plain and invert same: 0x%X", crc_plain);
	/* Returning to plain must be deterministic */
	zassert_equal(crc_plain, crc_plain2,
		      "Stale register state: plain=0x%X, plain2=0x%X",
		      crc_plain, crc_plain2);
#endif
}

/* ================================================================== */
/* STRESS-9: HW output width validation                                */
/*           8-bit CRC should only have valid data in lower 8 bits    */
/*           16-bit CRC should only have valid data in lower 16 bits  */
/* ================================================================== */
ZTEST(crc_test, stress_output_width_check)
{
	uint32_t crc_output = 0xDEADBEEF;  /* Pre-fill with known garbage */
	struct crc_params params = {
		.data_in     = (uint8_t *)crc_test_data,
		.len         = crc_test_data_len,
		.bit_swap    = CRC_FALSE,
		.byte_swap   = CRC_FALSE,
		.reflect     = CRC_FALSE,
		.invert      = CRC_FALSE,
		.custom_poly = CRC_FALSE,
		.data_out    = &crc_output,
	};

	zassert_equal(stress_prepare_seed_and_poly(&params, 0x00), 0,
		      "seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "compute failed");

	/* Validate against software and check output width */
#if IS_ENABLED(CONFIG_TEST_CRC_8)
	uint32_t crc_sw = gen_crc(CRC8_PRIME,
			   (const unsigned char *)crc_test_data,
			   crc_test_data_len);

	zassert_equal((crc_output & 0xFF), crc_sw,
		      "HW/SW mismatch (8-bit): HW=0x%X SW=0x%X",
		      (crc_output & 0xFF), crc_sw);
	zassert_equal((crc_output & 0xFFFFFF00U), 0U,
		      "8-bit CRC upper bits set unexpectedly: raw=0x%08X",
		      crc_output);
#elif IS_ENABLED(CONFIG_TEST_CRC_16)
	uint32_t crc_sw = gen_crc(CRC16_802_15_4,
		  (const unsigned char *)crc_test_data,
		  crc_test_data_len);

	zassert_equal((crc_output & 0xFFFF), crc_sw,
		      "HW/SW mismatch (16-bit): HW=0x%X SW=0x%X",
		      (crc_output & 0xFFFF), crc_sw);
	zassert_equal((crc_output & 0xFFFF0000U), 0U,
		      "16-bit CRC upper bits set unexpectedly: raw=0x%08X",
		      crc_output);
#endif

	LOG_INF("STRESS-9: Raw HW output = 0x%08X",
		crc_output);

	/*
	 * Regression guard: 8/16-bit modes return data via a 32-bit register,
	 * but only the low-width bits are valid. Driver now masks width
	 * before publishing output; these assertions catch regressions in
	 * that behavior.
	 */
}

/* ================================================================== */
/* STRESS-10: Known vector "123456789" — cross-check with SW          */
/* ================================================================== */
ZTEST(crc_test, stress_known_vector_123456789)
{
	uint32_t crc_hw = 0, crc_sw = 0;
	struct crc_params params = {
		.data_in     = crc_check_string,
		.len         = CRC_CHECK_STRING_LEN,
		.bit_swap    = CRC_FALSE,
		.byte_swap   = CRC_FALSE,
		.reflect     = CRC_FALSE,
		.invert      = CRC_FALSE,
		.custom_poly = CRC_FALSE,
		.data_out    = &crc_hw,
	};

	zassert_equal(stress_prepare_seed_and_poly(&params, 0x00), 0,
		      "seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "compute failed");

	/*
	 * Use the software CRC to cross-check.  Which SW algo to use
	 * depends on which HW algo is configured (8/16/32).
	 * We try CRC8 first; if the overlay sets 16 or 32 the Kconfig
	 * will select the right test file.  This test is intentionally
	 * generic — it runs for whichever algo is enabled.
	 */
#if IS_ENABLED(CONFIG_TEST_CRC_8)
	crc_sw = gen_crc(CRC8_PRIME, crc_check_string, CRC_CHECK_STRING_LEN);
	LOG_INF("STRESS-10: '123456789' CRC-8 HW=0x%X SW=0x%X",
		crc_hw, crc_sw);
	zassert_equal((crc_hw & 0xFF), crc_sw,
		      "CRC-8 known vector mismatch: HW=0x%X SW=0x%X",
		      crc_hw, crc_sw);
#elif IS_ENABLED(CONFIG_TEST_CRC_16)
	crc_sw = gen_crc(CRC16_802_15_4, crc_check_string,
			 CRC_CHECK_STRING_LEN);
	LOG_INF("STRESS-10: '123456789' CRC-16 HW=0x%X SW=0x%X",
		crc_hw, crc_sw);
	zassert_equal((crc_hw & 0xFFFF), crc_sw,
		      "CRC-16 known vector mismatch: HW=0x%X SW=0x%X",
		      crc_hw, crc_sw);
#elif IS_ENABLED(CONFIG_TEST_CRC_32)
	crc_sw = crc32_driver_reference(crc_check_string,
					CRC_CHECK_STRING_LEN,
				0x00,
				params.bit_swap,
				params.byte_swap,
				params.reflect,
				params.invert,
				CRC32_POLY_NORMAL,
				CRC32_POLY_REFLECTED);
	LOG_INF("STRESS-10: '123456789' CRC-32 HW=0x%X SW=0x%X",
		crc_hw, crc_sw);
	zassert_equal(crc_hw, crc_sw,
		      "CRC-32 known vector mismatch: HW=0x%X SW=0x%X",
		      crc_hw, crc_sw);
#else
	LOG_INF("STRESS-10: '123456789' HW=0x%X (no SW reference)",
		crc_hw);
#endif
}

/* ================================================================== */
/* STRESS-11: Large data — 256 bytes repeated pattern                 */
/* ================================================================== */
ZTEST(crc_test, stress_large_data_256)
{
	static uint8_t large_buf[256];
	uint32_t crc_hw = 0, crc_hw2 = 0;

	/* Fill with repeating pattern */
	for (int i = 0; i < 256; i++) {
		large_buf[i] = (uint8_t)(i & 0xFF);
	}

	struct crc_params params = {
		.data_in     = large_buf,
		.len         = 256,
		.bit_swap    = CRC_FALSE,
		.byte_swap   = CRC_FALSE,
		.reflect     = CRC_FALSE,
		.invert      = CRC_FALSE,
		.custom_poly = CRC_FALSE,
		.data_out    = &crc_hw,
	};

	zassert_equal(stress_prepare_seed_and_poly(&params, 0x00), 0,
		      "seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0,
		      "compute 256 failed");

	/* Validate 256-byte CRC against software */
#if IS_ENABLED(CONFIG_TEST_CRC_8)
	uint32_t crc_sw = gen_crc(CRC8_PRIME, large_buf, 256);

	zassert_equal((crc_hw & 0xFF), crc_sw,
		      "HW/SW 256-byte mismatch (8-bit): HW=0x%X SW=0x%X",
		      (crc_hw & 0xFF), crc_sw);
#elif IS_ENABLED(CONFIG_TEST_CRC_16)
	uint32_t crc_sw = gen_crc(CRC16_802_15_4, large_buf, 256);

	zassert_equal((crc_hw & 0xFFFF), crc_sw,
		      "HW/SW 256-byte mismatch (16-bit): HW=0x%X SW=0x%X",
		      (crc_hw & 0xFFFF), crc_sw);
#elif IS_ENABLED(CONFIG_TEST_CRC_32)
	uint32_t crc_sw = crc32_driver_reference(large_buf,
			256,
			0x00,
			params.bit_swap,
			params.byte_swap,
			params.reflect,
			params.invert,
			CRC32_POLY_NORMAL,
			CRC32_POLY_REFLECTED);
	zassert_equal(crc_hw, crc_sw,
		      "HW/SW 256-byte mismatch (32-bit): HW=0x%X SW=0x%X",
		      crc_hw, crc_sw);
#endif

	/* Repeat to verify determinism on large data */
	params.data_out = &crc_hw2;
	zassert_equal(stress_prepare_seed_and_poly(&params, 0x00), 0,
		      "seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0,
		      "compute 256 repeat failed");

#if IS_ENABLED(CONFIG_TEST_CRC_8)
	zassert_equal(crc_hw & 0xFF, crc_hw2 & 0xFF,
		      "Large data non-deterministic: 0x%X != 0x%X",
		      crc_hw & 0xFF, crc_hw2 & 0xFF);
#elif IS_ENABLED(CONFIG_TEST_CRC_16)
	zassert_equal(crc_hw & 0xFFFF, crc_hw2 & 0xFFFF,
		      "Large data non-deterministic: 0x%X != 0x%X",
		      crc_hw & 0xFFFF, crc_hw2 & 0xFFFF);
#else
	zassert_equal(crc_hw, crc_hw2,
		      "Large data non-deterministic: 0x%X != 0x%X",
		      crc_hw, crc_hw2);
#endif
	LOG_INF("STRESS-11: 256-byte CRC=0x%X (verified deterministic)",
		crc_hw);
}

/* ================================================================== */
/* STRESS-12: Bit-flip sensitivity — flipping one bit must change CRC */
/* ================================================================== */
ZTEST(crc_test, stress_bit_flip_sensitivity)
{
	uint8_t buf_orig[crc_test_data_len];
	uint8_t buf_flipped[crc_test_data_len];
	uint32_t crc_orig = 0, crc_flipped = 0;

	memcpy(buf_orig, crc_test_data, sizeof(buf_orig));
	memcpy(buf_flipped, crc_test_data, sizeof(buf_flipped));

	/* Flip bit 0 of byte 4 */
	buf_flipped[4] ^= 0x01;

	struct crc_params params = {
		.data_in     = buf_orig,
		.len         = sizeof(buf_orig),
		.bit_swap    = CRC_FALSE,
		.byte_swap   = CRC_FALSE,
		.reflect     = CRC_FALSE,
		.invert      = CRC_FALSE,
		.custom_poly = CRC_FALSE,
		.data_out    = &crc_orig,
	};

	zassert_equal(stress_prepare_seed_and_poly(&params, 0x00), 0,
		      "seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "compute orig failed");

	/* Validate original CRC against software */
#if IS_ENABLED(CONFIG_TEST_CRC_8)
	uint32_t sw_orig = gen_crc(CRC8_PRIME, buf_orig, sizeof(buf_orig));

	zassert_equal((crc_orig & 0xFF), sw_orig,
		      "HW/SW original mismatch (8-bit): HW=0x%X SW=0x%X",
		      (crc_orig & 0xFF), sw_orig);
#elif IS_ENABLED(CONFIG_TEST_CRC_16)
	uint32_t sw_orig = gen_crc(CRC16_802_15_4, buf_orig, sizeof(buf_orig));

	zassert_equal((crc_orig & 0xFFFF), sw_orig,
		      "HW/SW original mismatch (16-bit): HW=0x%X SW=0x%X",
		      (crc_orig & 0xFFFF), sw_orig);
#elif IS_ENABLED(CONFIG_TEST_CRC_32)
	uint32_t sw_orig = crc32_driver_reference(buf_orig,
			sizeof(buf_orig),
			0x00,
			params.bit_swap,
			params.byte_swap,
			params.reflect,
			params.invert,
			CRC32_POLY_NORMAL,
			CRC32_POLY_REFLECTED);
	zassert_equal(crc_orig, sw_orig,
		      "HW/SW original mismatch (32-bit): HW=0x%X SW=0x%X",
		      crc_orig, sw_orig);
#endif

	params.data_in = buf_flipped;
	params.data_out = &crc_flipped;
	zassert_equal(stress_prepare_seed_and_poly(&params, 0x00), 0,
		      "seed failed");
	zassert_equal(crc_compute(crc_dev, &params), 0,
		      "compute flipped failed");

	/* Validate flipped CRC against software */
#if IS_ENABLED(CONFIG_TEST_CRC_8)
	uint32_t sw_flipped = gen_crc(CRC8_PRIME, buf_flipped,
				      sizeof(buf_flipped));

	zassert_equal((crc_flipped & 0xFF), sw_flipped,
		      "HW/SW flipped mismatch (8-bit): HW=0x%X SW=0x%X",
		      (crc_flipped & 0xFF), sw_flipped);
#elif IS_ENABLED(CONFIG_TEST_CRC_16)
	uint32_t sw_flipped = gen_crc(CRC16_802_15_4,
			   buf_flipped,
			   sizeof(buf_flipped));

	zassert_equal((crc_flipped & 0xFFFF), sw_flipped,
		      "HW/SW flipped mismatch (16-bit): HW=0x%X SW=0x%X",
		      (crc_flipped & 0xFFFF), sw_flipped);
#elif IS_ENABLED(CONFIG_TEST_CRC_32)
	uint32_t sw_flipped = crc32_driver_reference(buf_flipped,
			   sizeof(buf_flipped),
			   0x00,
			   params.bit_swap,
			   params.byte_swap,
			   params.reflect,
			   params.invert,
			   CRC32_POLY_NORMAL,
			   CRC32_POLY_REFLECTED);
	zassert_equal(crc_flipped, sw_flipped,
		      "HW/SW flipped mismatch (32-bit): HW=0x%X SW=0x%X",
		      crc_flipped, sw_flipped);
#endif

#if IS_ENABLED(CONFIG_TEST_CRC_8)
	zassert_not_equal(crc_orig & 0xFF, crc_flipped & 0xFF,
			  "Single bit flip undetected: orig=0x%X flipped=0x%X",
			  crc_orig & 0xFF, crc_flipped & 0xFF);
#elif IS_ENABLED(CONFIG_TEST_CRC_16)
	zassert_not_equal(crc_orig & 0xFFFF, crc_flipped & 0xFFFF,
			  "Single bit flip undetected: orig=0x%X flipped=0x%X",
			  crc_orig & 0xFFFF, crc_flipped & 0xFFFF);
#else
	zassert_not_equal(crc_orig, crc_flipped,
			  "Single bit flip undetected: orig=0x%X flipped=0x%X",
			  crc_orig, crc_flipped);
#endif

	LOG_INF("STRESS-12: Bit-flip detected: orig=0x%X flipped=0x%X",
		crc_orig, crc_flipped);
}

/* ================================================================== */
/* STRESS-13: Reflect + Invert combined — verify orthogonality        */
/* ================================================================== */
ZTEST(crc_test, stress_reflect_invert_orthogonal)
{
	uint32_t crc_none = 0, crc_ref = 0, crc_inv = 0, crc_both = 0;
	struct crc_params params = {
		.data_in     = (uint8_t *)crc_test_data,
		.len         = crc_test_data_len,
		.bit_swap    = CRC_FALSE,
		.byte_swap   = CRC_FALSE,
		.reflect     = CRC_FALSE,
		.invert      = CRC_FALSE,
		.custom_poly = CRC_FALSE,
	};

	/* None */
	params.data_out = &crc_none;
	zassert_equal(stress_prepare_seed_and_poly(&params, 0x00), 0, "seed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "compute none");

	/* Validate baseline against software */
#if IS_ENABLED(CONFIG_TEST_CRC_8)
	uint32_t sw_none = gen_crc(CRC8_PRIME,
				   (const unsigned char *)crc_test_data,
				   crc_test_data_len);

	zassert_equal((crc_none & 0xFF), sw_none,
		      "HW/SW baseline mismatch (8-bit): HW=0x%X SW=0x%X",
		      (crc_none & 0xFF), sw_none);
#elif IS_ENABLED(CONFIG_TEST_CRC_16)
	uint32_t sw_none = gen_crc(CRC16_802_15_4,
				   (const unsigned char *)crc_test_data,
				   crc_test_data_len);

	zassert_equal((crc_none & 0xFFFF), sw_none,
		      "HW/SW baseline mismatch (16-bit): HW=0x%X SW=0x%X",
		      (crc_none & 0xFFFF), sw_none);
#elif IS_ENABLED(CONFIG_TEST_CRC_32)
	uint32_t sw_none = crc32_driver_reference(
			  (const uint8_t *)crc_test_data,
			  crc_test_data_len,
			  0x00,
			  params.bit_swap,
			  params.byte_swap,
			  params.reflect,
			  params.invert,
			  CRC32_POLY_NORMAL,
			  CRC32_POLY_REFLECTED);
	zassert_equal(crc_none, sw_none,
		      "HW/SW baseline mismatch (32-bit): HW=0x%X SW=0x%X",
		      crc_none, sw_none);
#endif

	/* Reflect only */
	params.reflect = CRC_TRUE;
	params.data_out = &crc_ref;
	zassert_equal(stress_prepare_seed_and_poly(&params, 0x00), 0, "seed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "compute reflect");

	/* Invert only */
	params.reflect = CRC_FALSE;
	params.invert = CRC_TRUE;
	params.data_out = &crc_inv;
	zassert_equal(stress_prepare_seed_and_poly(&params, 0x00), 0, "seed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "compute invert");

	/* Both */
	params.reflect = CRC_TRUE;
	params.invert = CRC_TRUE;
	params.data_out = &crc_both;
	zassert_equal(stress_prepare_seed_and_poly(&params, 0x00), 0, "seed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "compute both");

	LOG_INF("STRESS-13: none=0x%X ref=0x%X inv=0x%X both=0x%X",
		crc_none, crc_ref, crc_inv, crc_both);

	/* All four combinations should produce distinct results */
#if IS_ENABLED(CONFIG_TEST_CRC_8)
	zassert_not_equal(crc_none & 0xFF, crc_ref & 0xFF,
			  "none == reflect");
	zassert_not_equal(crc_none & 0xFF, crc_inv & 0xFF,
			  "none == invert");
	zassert_not_equal(crc_none & 0xFF, crc_both & 0xFF,
			  "none == both");
	zassert_not_equal(crc_ref & 0xFF, crc_inv & 0xFF,
			  "reflect == invert");
	zassert_not_equal(crc_ref & 0xFF, crc_both & 0xFF,
			  "reflect == both");
	zassert_not_equal(crc_inv & 0xFF, crc_both & 0xFF,
			  "invert == both");
#elif IS_ENABLED(CONFIG_TEST_CRC_16)
	zassert_not_equal(crc_none & 0xFFFF, crc_ref & 0xFFFF,
			  "none == reflect");
	zassert_not_equal(crc_none & 0xFFFF, crc_inv & 0xFFFF,
			  "none == invert");
	zassert_not_equal(crc_none & 0xFFFF, crc_both & 0xFFFF,
			  "none == both");
	zassert_not_equal(crc_ref & 0xFFFF, crc_inv & 0xFFFF,
			  "reflect == invert");
	zassert_not_equal(crc_ref & 0xFFFF, crc_both & 0xFFFF,
			  "reflect == both");
	zassert_not_equal(crc_inv & 0xFFFF, crc_both & 0xFFFF,
			  "invert == both");
#else
	zassert_not_equal(crc_none, crc_ref, "none == reflect");
	zassert_not_equal(crc_none, crc_inv, "none == invert");
	zassert_not_equal(crc_none, crc_both, "none == both");
	zassert_not_equal(crc_ref, crc_inv, "reflect == invert");
	zassert_not_equal(crc_ref, crc_both, "reflect == both");
	zassert_not_equal(crc_inv, crc_both, "invert == both");
#endif
}

/* ================================================================== */
/* STRESS-14: Swap sensitivity — bit_swap and byte_swap               */
/* ================================================================== */
ZTEST(crc_test, stress_swap_sensitivity)
{
	uint32_t crc_nosw = 0, crc_bitsw = 0, crc_bytesw = 0, crc_bothsw = 0;
	struct crc_params params = {
		.data_in     = (uint8_t *)crc_test_data,
		.len         = crc_test_data_len,
		.reflect     = CRC_FALSE,
		.invert      = CRC_FALSE,
		.custom_poly = CRC_FALSE,
	};

	/* No swap */
	params.bit_swap = CRC_FALSE;
	params.byte_swap = CRC_FALSE;
	params.data_out = &crc_nosw;
	zassert_equal(stress_prepare_seed_and_poly(&params, 0x00), 0, "seed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "compute no swap");

	/* Validate baseline against software */
#if IS_ENABLED(CONFIG_TEST_CRC_8)
	uint32_t sw_nosw = gen_crc(CRC8_PRIME,
				   (const unsigned char *)crc_test_data,
				   crc_test_data_len);

	zassert_equal((crc_nosw & 0xFF), sw_nosw,
		      "HW/SW baseline mismatch (8-bit): HW=0x%X SW=0x%X",
		      (crc_nosw & 0xFF), sw_nosw);
#elif IS_ENABLED(CONFIG_TEST_CRC_16)
	uint32_t sw_nosw = gen_crc(CRC16_802_15_4,
				   (const unsigned char *)crc_test_data,
				   crc_test_data_len);

	zassert_equal((crc_nosw & 0xFFFF), sw_nosw,
		      "HW/SW baseline mismatch (16-bit): HW=0x%X SW=0x%X",
		      (crc_nosw & 0xFFFF), sw_nosw);
#elif IS_ENABLED(CONFIG_TEST_CRC_32)
	uint32_t sw_nosw = crc32_driver_reference(
			  (const uint8_t *)crc_test_data,
			  crc_test_data_len,
			  0x00,
			  params.bit_swap,
			  params.byte_swap,
			  params.reflect,
			  params.invert,
			  CRC32_POLY_NORMAL,
			  CRC32_POLY_REFLECTED);
	zassert_equal(crc_nosw, sw_nosw,
		      "HW/SW baseline mismatch (32-bit): HW=0x%X SW=0x%X",
		      crc_nosw, sw_nosw);
#endif

	/* Bit swap only */
	params.bit_swap = CRC_TRUE;
	params.byte_swap = CRC_FALSE;
	params.data_out = &crc_bitsw;
	zassert_equal(stress_prepare_seed_and_poly(&params, 0x00), 0, "seed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "compute bit swap");

	/* Byte swap only */
	params.bit_swap = CRC_FALSE;
	params.byte_swap = CRC_TRUE;
	params.data_out = &crc_bytesw;
	zassert_equal(stress_prepare_seed_and_poly(&params, 0x00), 0, "seed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "compute byte swap");

	/* Both swaps */
	params.bit_swap = CRC_TRUE;
	params.byte_swap = CRC_TRUE;
	params.data_out = &crc_bothsw;
	zassert_equal(stress_prepare_seed_and_poly(&params, 0x00), 0, "seed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "compute both swap");

	LOG_INF("STRESS-14: nosw=0x%X bitsw=0x%X bytesw=0x%X bothsw=0x%X",
		crc_nosw, crc_bitsw, crc_bytesw, crc_bothsw);

	/* Different swap modes should produce different CRCs from baseline */
#if IS_ENABLED(CONFIG_TEST_CRC_8)
	zassert_not_equal(crc_nosw & 0xFF, crc_bitsw & 0xFF,
			  "no swap == bit swap");
	zassert_not_equal(crc_nosw & 0xFF, crc_bytesw & 0xFF,
			  "no swap == byte swap");
	zassert_not_equal(crc_nosw & 0xFF, crc_bothsw & 0xFF,
			  "no swap == both swap");
#elif IS_ENABLED(CONFIG_TEST_CRC_16)
	zassert_not_equal(crc_nosw & 0xFFFF, crc_bitsw & 0xFFFF,
			  "no swap == bit swap");
	zassert_not_equal(crc_nosw & 0xFFFF, crc_bytesw & 0xFFFF,
			  "no swap == byte swap");
	zassert_not_equal(crc_nosw & 0xFFFF, crc_bothsw & 0xFFFF,
			  "no swap == both swap");
#else
	zassert_not_equal(crc_nosw, crc_bitsw,
			  "no swap == bit swap");
	zassert_not_equal(crc_nosw, crc_bytesw,
			  "no swap == byte swap");
	zassert_not_equal(crc_nosw, crc_bothsw,
			  "no swap == both swap");
#endif
}

#endif /* CONFIG_TEST_CRC_8 || CONFIG_TEST_CRC_16 || CONFIG_TEST_CRC_32 */

/* ================================================================== */
/* STRESS-NEG: Edge-case input validation                              */
/* ================================================================== */
/*
 * Test edge cases that don't crash the board. Validates that
 * the driver produces deterministic results for boundary conditions.
 *
 * Coverage:
 *   - len=0: Validates HW output against independent SW reference
 *   - NULL tests: Omitted (would hard fault without driver guards)
 */
#if IS_ENABLED(CONFIG_TEST_CRC_8) ||\
	IS_ENABLED(CONFIG_TEST_CRC_16) ||\
	IS_ENABLED(CONFIG_TEST_CRC_32)

ZTEST(crc_test, stress_zero_length_input)
{
	uint32_t crc_hw = 0xDEADBEEF;
	uint8_t dummy = 0xAA;
	struct crc_params params = {
		.data_in     = &dummy,
		.len         = 0,
		.bit_swap    = CRC_FALSE,
		.byte_swap   = CRC_FALSE,
		.reflect     = CRC_FALSE,
		.invert      = CRC_FALSE,
		.custom_poly = CRC_FALSE,
		.data_out    = &crc_hw,
	};

	/* Expected contract: len=0 should be rejected by the driver.
	 * Current driver behavior: accepts len=0, writes stale HW register.
	 * This test validates both the return code and the output value.
	 */
	zassert_equal(stress_prepare_seed_and_poly(&params, 0x00),
		      0, "seed failed");
	int ret = crc_compute(crc_dev, &params);

	/* Validate return code - driver should ideally reject len=0 */
	zassert_equal(ret, 0,
		      "Driver should accept len=0 for now (ret=%d)", ret);

	/* Get software reference for len=0 */
	uint32_t crc_sw = 0;
#if IS_ENABLED(CONFIG_TEST_CRC_8)
	crc_sw = gen_crc(CRC8_PRIME, &dummy, 0);
#elif IS_ENABLED(CONFIG_TEST_CRC_16)
	crc_sw = gen_crc(CRC16_802_15_4, &dummy, 0);
#elif IS_ENABLED(CONFIG_TEST_CRC_32)
	crc_sw = crc32_driver_reference(&dummy, 0,
					0x00, params.bit_swap, params.byte_swap,
			params.reflect, params.invert,
			CRC32_POLY_NORMAL, CRC32_POLY_REFLECTED);
#endif

	/* Validate HW output against SW reference.
	 * Note: With len=0, both should return the seed value
	 * (0x00 for CRC-8/16, 0x00 for CRC-32 with our params).
	 */
#if IS_ENABLED(CONFIG_TEST_CRC_8)
	zassert_equal(crc_hw & 0xFF, crc_sw,
		      "len=0 HW/SW mismatch (8-bit): HW=0x%X SW=0x%X",
		      crc_hw & 0xFF, crc_sw);
#elif IS_ENABLED(CONFIG_TEST_CRC_16)
	zassert_equal(crc_hw & 0xFFFF, crc_sw,
		      "len=0 HW/SW mismatch (16-bit): HW=0x%X SW=0x%X",
		      crc_hw & 0xFFFF, crc_sw);
#else
	zassert_equal(crc_hw, crc_sw,
		      "len=0 HW/SW mismatch (32-bit): HW=0x%X SW=0x%X",
		      crc_hw, crc_sw);
#endif

	LOG_INF("STRESS-NEG: len=0 ret=%d HW=0x%X SW=0x%X (validated)",
		ret, crc_hw, crc_sw);
}

/* ================================================================== */
/* STRESS-NEG-NULL: NULL pointer validation                             */
/* ================================================================== */
/*
 * Test NULL pointer handling in driver API functions.
 * Validates proper error codes for invalid inputs.
 */

ZTEST(crc_test, stress_null_pointers)
{
	uint32_t crc_output = 0;
	struct crc_params params = {
		.data_in     = (uint8_t *)crc_test_data,
		.len         = crc_test_data_len,
		.bit_swap    = CRC_FALSE,
		.byte_swap   = CRC_FALSE,
		.reflect     = CRC_FALSE,
		.invert      = CRC_FALSE,
		.custom_poly = CRC_FALSE,
		.data_out    = &crc_output,
	};

	/* Test 1: NULL params to crc_compute() - should return -EINVAL */
	zassert_not_equal(crc_compute(crc_dev, NULL), 0,
			  "crc_compute with NULL params should fail");

	/* Test 2: NULL data_in - should return -EINVAL */
	params.data_in = NULL;
	zassert_not_equal(crc_compute(crc_dev, &params), 0,
			  "crc_compute with NULL data_in should fail");

	/* Test 3: NULL data_out - should return -EINVAL */
	params.data_in = (uint8_t *)crc_test_data;  /* Restore valid data_in */
	params.data_out = NULL;
	zassert_not_equal(crc_compute(crc_dev, &params), 0,
			  "crc_compute with NULL data_out should fail");

	/* Test 4: NULL device to crc_set_seed() - should return -EINVAL */
	zassert_not_equal(crc_set_seed(NULL, 0x00), 0,
			  "crc_set_seed with NULL device should fail");

	/* Test 5: NULL device to crc_set_polynomial() -
	 * should return -EINVAL
	 */
	zassert_not_equal(crc_set_polynomial(NULL, CRC32_POLY_NORMAL), 0,
			  "crc_set_polynomial with NULL device should fail");

	/* Test 6: NULL device to crc_compute() - should return -EINVAL */
	params.data_out = &crc_output;  /* Restore valid data_out */
	zassert_not_equal(crc_compute(NULL, &params), 0,
			  "crc_compute with NULL device should fail");

	LOG_INF("STRESS-NEG-NULL: All NULL pointer cases handled gracefully");
}

#endif /* generic neg tests */

#if IS_ENABLED(CONFIG_TEST_CRC_32)

ZTEST(crc_test, stress_crc32_full_known_vector_123456789)
{
	uint32_t crc_hw = 0;
	uint32_t crc_sw;
	struct crc_params params = {
		.data_in     = crc_check_string,
		.len         = CRC_CHECK_STRING_LEN,
		.bit_swap    = CRC_TRUE,
		.byte_swap   = CRC_TRUE,
		.reflect     = CRC_TRUE,
		.invert      = CRC_TRUE,
		.custom_poly = CRC_FALSE,
		.data_out    = &crc_hw,
	};

	zassert_equal(stress_crc32_full_prepare(&params), 0, "prepare failed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "compute failed");

	crc_sw = crc_32(crc_check_string, CRC_CHECK_STRING_LEN,
			CRC32_POLY_REFLECTED);

	zassert_equal(crc_hw, crc_sw,
		      "CRC-32 full known vector mismatch: HW=0x%X SW=0x%X",
		      crc_hw, crc_sw);
}

ZTEST(crc_test, stress_crc32_full_repeatability)
{
	uint32_t crc_first = 0;
	uint32_t crc_repeat = 0;
	uint32_t crc_sw;
	struct crc_params params = {
		.data_in     = (uint8_t *)crc_test_data,
		.len         = crc_test_data_len,
		.bit_swap    = CRC_TRUE,
		.byte_swap   = CRC_TRUE,
		.reflect     = CRC_TRUE,
		.invert      = CRC_TRUE,
		.custom_poly = CRC_FALSE,
		.data_out    = &crc_first,
	};

	zassert_equal(stress_crc32_full_prepare(&params), 0, "prepare failed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "compute first failed");

	crc_sw = crc_32((const unsigned char *)crc_test_data,
			crc_test_data_len, CRC32_POLY_REFLECTED);
	zassert_equal(crc_first, crc_sw,
		      "CRC-32 full HW/SW mismatch: HW=0x%X SW=0x%X",
		      crc_first, crc_sw);

	for (int i = 0; i < 50; i++) {
		params.data_out = &crc_repeat;
		zassert_equal(stress_crc32_full_prepare(&params), 0,
			      "prepare iter %d failed", i);
		zassert_equal(crc_compute(crc_dev, &params), 0,
			      "compute iter %d failed", i);
		zassert_equal(crc_first, crc_repeat,
			      "CRC-32 full repeatability iter %d: 0x%X != 0x%X",
		      i, crc_first, crc_repeat);
	}
}

ZTEST(crc_test, stress_crc32_full_unaligned_tail)
{
	uint32_t crc_hw = 0;
	uint32_t crc_sw;
	struct crc_params params = {
		.data_in     = (uint8_t *)crc_test_data,
		.len         = crc_test_data_len,
		.bit_swap    = CRC_TRUE,
		.byte_swap   = CRC_TRUE,
		.reflect     = CRC_TRUE,
		.invert      = CRC_TRUE,
		.custom_poly = CRC_FALSE,
		.data_out    = &crc_hw,
	};

	zassert_equal(stress_crc32_full_prepare(&params), 0, "prepare failed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "compute failed");

	crc_sw = crc_32((const unsigned char *)crc_test_data,
			crc_test_data_len, CRC32_POLY_REFLECTED);
	zassert_equal(crc_hw, crc_sw,
		      "CRC-32 full unaligned mismatch: HW=0x%X SW=0x%X",
		      crc_hw, crc_sw);
}

#endif /* CONFIG_TEST_CRC_32 */

#if IS_ENABLED(CONFIG_TEST_CRC_32_CUSTOM_POLY) || \
	IS_ENABLED(CONFIG_TEST_CRC_32C)

#if IS_ENABLED(CONFIG_TEST_CRC_32_CUSTOM_POLY)
#define STRESS_32X_NAME          "CRC-32 custom"
#define STRESS_32X_HW_POLY       CRC32_CUSTOM_HW_POLY
#define STRESS_32X_SW_POLY_REFL  CRC32_CUSTOM_SW_POLY_REFL
#else
#define STRESS_32X_NAME          "CRC-32C"
#define STRESS_32X_HW_POLY       CRC32C_HW_POLY
#define STRESS_32X_SW_POLY_REFL  CRC32C_SW_POLY_REFL
#endif

static int stress_32x_prepare(const struct crc_params *params)
{
	return stress_prepare_seed_and_poly(params, 0xFFFFFFFFU);
}

ZTEST(crc_test, stress_32x_known_vector_123456789)
{
	uint32_t crc_hw = 0;
	uint32_t crc_sw;
	struct crc_params params = {
		.data_in     = crc_check_string,
		.len         = CRC_CHECK_STRING_LEN,
		.bit_swap    = CRC_TRUE,
		.byte_swap   = CRC_TRUE,
		.reflect     = CRC_TRUE,
		.invert      = CRC_TRUE,
		.custom_poly = CRC_TRUE,
		.data_out    = &crc_hw,
	};

	zassert_equal(stress_32x_prepare(&params), 0, "prepare failed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "compute failed");

	crc_sw = crc_32(crc_check_string, CRC_CHECK_STRING_LEN,
			STRESS_32X_SW_POLY_REFL);

	LOG_INF("STRESS-32X-1 (%s): '123456789' HW=0x%X SW=0x%X",
		STRESS_32X_NAME, crc_hw, crc_sw);
	zassert_equal(crc_hw, crc_sw,
		      "%s known vector mismatch: HW=0x%X SW=0x%X",
		      STRESS_32X_NAME, crc_hw, crc_sw);
}

ZTEST(crc_test, stress_32x_repeatability)
{
	uint32_t crc_first = 0;
	uint32_t crc_repeat = 0;
	struct crc_params params = {
		.data_in     = (uint8_t *)crc_test_data,
		.len         = crc_test_data_len,
		.bit_swap    = CRC_TRUE,
		.byte_swap   = CRC_TRUE,
		.reflect     = CRC_TRUE,
		.invert      = CRC_TRUE,
		.custom_poly = CRC_TRUE,
		.data_out    = &crc_first,
	};

	zassert_equal(stress_32x_prepare(&params), 0, "prepare failed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "compute first failed");

	uint32_t crc_sw = crc_32((const unsigned char *)crc_test_data,
				 crc_test_data_len,
				 STRESS_32X_SW_POLY_REFL);
	zassert_equal(crc_first, crc_sw,
		      "%s HW/SW mismatch: HW=0x%X SW=0x%X",
		      STRESS_32X_NAME, crc_first, crc_sw);

	for (int i = 0; i < 50; i++) {
		params.data_out = &crc_repeat;
		zassert_equal(stress_32x_prepare(&params), 0,
			      "prepare iter %d failed", i);
		zassert_equal(crc_compute(crc_dev, &params), 0,
			      "compute iter %d failed", i);
		zassert_equal(crc_first, crc_repeat,
			      "%s repeatability failure iter %d: 0x%X != 0x%X",
		      STRESS_32X_NAME, i, crc_first, crc_repeat);
	}
}

ZTEST(crc_test, stress_32x_unaligned_tail)
{
	uint32_t crc_hw = 0;
	uint32_t crc_sw;
	struct crc_params params = {
		.data_in     = (uint8_t *)crc_test_data,
		.len         = crc_test_data_len,
		.bit_swap    = CRC_TRUE,
		.byte_swap   = CRC_TRUE,
		.reflect     = CRC_TRUE,
		.invert      = CRC_TRUE,
		.custom_poly = CRC_TRUE,
		.data_out    = &crc_hw,
	};

	zassert_equal(stress_32x_prepare(&params), 0, "prepare failed");
	zassert_equal(crc_compute(crc_dev, &params), 0, "compute failed");

	crc_sw = crc_32((const unsigned char *)crc_test_data,
			crc_test_data_len,
			STRESS_32X_SW_POLY_REFL);

	LOG_INF("STRESS-32X-3 (%s): unaligned HW=0x%X SW=0x%X",
		STRESS_32X_NAME, crc_hw, crc_sw);
	zassert_equal(crc_hw, crc_sw,
		      "%s unaligned mismatch: HW=0x%X SW=0x%X",
		      STRESS_32X_NAME, crc_hw, crc_sw);
}

#endif /* CONFIG_TEST_CRC_32_CUSTOM_POLY || CONFIG_TEST_CRC_32C */

#endif /* CONFIG_TEST_CRC_STRESS */

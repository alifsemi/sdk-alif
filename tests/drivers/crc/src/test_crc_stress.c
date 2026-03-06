/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include "software_crc.h"

#include <string.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(alif_crc_test);

#if IS_ENABLED(CONFIG_TEST_CRC_STRESS)

static void crc_stress_before(void *fixture)
{
	ARG_UNUSED(fixture);

	crc_test_verify_device();
}

#if !IS_ENABLED(CONFIG_TEST_CRC_8) && !IS_ENABLED(CONFIG_TEST_CRC_16) &&       \
	!IS_ENABLED(CONFIG_TEST_CRC_16_ALT) &&                                 \
	!IS_ENABLED(CONFIG_TEST_CRC_32) && !IS_ENABLED(CONFIG_TEST_CRC_32C)
#error "CONFIG_TEST_CRC_STRESS requires at least one CRC algorithm test"
#endif

/*
 * The stress suite is runtime-driven so a single image can exercise every
 * enabled CRC algorithm. For 8-bit and 16-bit modes the public API only
 * exposes the native hardware algorithms, while 32-bit modes additionally
 * exercise the custom-polynomial path.
 */
struct stress_runtime_cfg {
	const char *name;
	enum test_crc_hw_algo hw_algo;
	int sw_id;
	uint8_t bits;
	uint32_t mask;
	uint32_t poly_normal;
	uint32_t poly_reflected;
	bool use_custom_poly;
};

static const struct stress_runtime_cfg stress_runtime_cfgs[] = {
#if IS_ENABLED(CONFIG_TEST_CRC_8)
	{
		.name = "crc8-ccitt",
		.hw_algo = TEST_CRC_HW_ALGO_CRC8_CCITT,
		.sw_id = CRC8_PRIME,
		.bits = 8U,
		.mask = 0xFFU,
		.use_custom_poly = false,
	},
#endif
#if IS_ENABLED(CONFIG_TEST_CRC_16)
	{
		.name = "crc16-ccitt",
		.hw_algo = TEST_CRC_HW_ALGO_CRC16_CCITT,
		.sw_id = CRC16_802_15_4,
		.bits = 16U,
		.mask = 0xFFFFU,
		.use_custom_poly = false,
	},
#endif
#if IS_ENABLED(CONFIG_TEST_CRC_16_ALT)
	{
		.name = "crc16",
		.hw_algo = TEST_CRC_HW_ALGO_CRC16,
		.sw_id = CRC16_ALT,
		.bits = 16U,
		.mask = 0xFFFFU,
		.use_custom_poly = false,
	},
#endif
#if IS_ENABLED(CONFIG_TEST_CRC_32)
	{
		.name = "crc32",
		.hw_algo = TEST_CRC_HW_ALGO_CRC32,
		.sw_id = -1,
		.bits = 32U,
		.mask = 0xFFFFFFFFU,
		.poly_normal = CRC32_CUSTOM_HW_POLY,
		.poly_reflected = CRC32_CUSTOM_SW_POLY_REFL,
		.use_custom_poly = true,
	},
#endif
#if IS_ENABLED(CONFIG_TEST_CRC_32C)
	{
		.name = "crc32c",
		.hw_algo = TEST_CRC_HW_ALGO_CRC32C,
		.sw_id = -1,
		.bits = 32U,
		.mask = 0xFFFFFFFFU,
		.poly_normal = CRC32_CUSTOM_HW_POLY,
		.poly_reflected = CRC32_CUSTOM_SW_POLY_REFL,
		.use_custom_poly = true,
	},
#endif
};

static const uint8_t all_zeros[64] = {0};

static uint32_t stress_sw_reference(const struct stress_runtime_cfg *cfg,
				    const uint8_t *data, size_t len)
{
	if (cfg->bits == 32U) {
		return crc32_driver_reference(
			data, len, 0x00000000U, false, false, false, false,
			cfg->poly_normal, cfg->poly_reflected);
	}

	return gen_crc(cfg->sw_id, data, len);
}

static uint32_t
stress_apply_output_transforms(const struct stress_runtime_cfg *cfg,
			       uint32_t crc, bool reflect, bool invert)
{
	if (reflect) {
		crc = bit_reflect_n(crc, cfg->bits);
	}

	if (invert) {
		crc ^= cfg->mask;
	}

	return crc & cfg->mask;
}

static uint32_t stress_sw_reference_with_params(
	const struct stress_runtime_cfg *cfg, const uint8_t *data, size_t len,
	uint32_t seed, bool bit_swap, bool byte_swap, bool reflect, bool invert)
{
	if (cfg->bits == 32U) {
		return crc32_driver_reference(
			data, len, seed, bit_swap, byte_swap, reflect, invert,
			cfg->poly_normal, cfg->poly_reflected);
	}

	zassert_equal(seed, 0U, "%s seed model not implemented", cfg->name);
	zassert_false(bit_swap || byte_swap,
		      "%s swap model not implemented for %u-bit CRC", cfg->name,
		      cfg->bits);

	return stress_apply_output_transforms(
		cfg, gen_crc(cfg->sw_id, data, len), reflect, invert);
}

static void stress_select_runtime_algo(const struct stress_runtime_cfg *cfg,
				       struct crc_params *params)
{
	crc_test_force_hw_algo(cfg->hw_algo);
	params->custom_poly = cfg->use_custom_poly;
}

static int stress_prepare_seed_and_poly(const struct stress_runtime_cfg *cfg,
					struct crc_params *params,
					uint32_t seed)
{
	int ret;

	stress_select_runtime_algo(cfg, params);

	ret = crc_set_seed(crc_dev, seed);
	if (ret != 0) {
		return ret;
	}

	if (cfg->use_custom_poly) {
		ret = crc_set_polynomial(crc_dev, cfg->poly_normal);
	}

	return ret;
}

#define STRESS_RUN_FOR_EACH_CFG(fn)                                            \
	do {                                                                   \
		for (size_t cfg_idx = 0;                                       \
		     cfg_idx < ARRAY_SIZE(stress_runtime_cfgs); cfg_idx++) {   \
			fn(&stress_runtime_cfgs[cfg_idx]);                     \
		}                                                              \
	} while (false)

/* ================================================================== */
/* STRESS-1: Repeatability — same input must always produce same CRC  */
/* ================================================================== */
static void stress_repeatability_for_cfg(const struct stress_runtime_cfg *cfg)
{
	uint32_t crc_first = 0U;
	uint32_t crc_repeat = 0U;
	uint32_t crc_sw;
	struct crc_params params = {
		.data_in = (uint8_t *)crc_test_data,
		.len = crc_test_data_len,
		.bit_swap = CRC_FALSE,
		.byte_swap = CRC_FALSE,
		.reflect = CRC_FALSE,
		.invert = CRC_FALSE,
		.custom_poly = CRC_FALSE,
		.data_out = &crc_first,
	};

	zassert_equal(stress_prepare_seed_and_poly(cfg, &params, 0x00U), 0,
		      "%s seed failed", cfg->name);
	zassert_equal(crc_compute(crc_dev, &params), 0,
		      "%s first compute failed", cfg->name);

	crc_sw = stress_sw_reference(cfg, crc_test_data, crc_test_data_len);
	zassert_equal(crc_first & cfg->mask, crc_sw,
		      "%s HW/SW mismatch: HW=0x%X SW=0x%X", cfg->name,
		      crc_first & cfg->mask, crc_sw);

	for (int i = 0; i < 50; i++) {
		params.data_out = &crc_repeat;
		zassert_equal(stress_prepare_seed_and_poly(cfg, &params, 0x00U),
			      0, "%s seed iter %d failed", cfg->name, i);
		zassert_equal(crc_compute(crc_dev, &params), 0,
			      "%s compute iter %d failed", cfg->name, i);
		zassert_equal(crc_first & cfg->mask, crc_repeat & cfg->mask,
			      "%s repeat mismatch iter %d: 0x%X != 0x%X",
			      cfg->name, i, crc_first & cfg->mask,
			      crc_repeat & cfg->mask);
	}

	LOG_INF("STRESS-1 [%s]: 50 iterations identical CRC=0x%X", cfg->name,
		crc_first & cfg->mask);
}

ZTEST(crc_stress_tests, test_stress_repeatability)
{
	STRESS_RUN_FOR_EACH_CFG(stress_repeatability_for_cfg);
}

/* ================================================================== */
/* STRESS-2: Back-to-back compute without re-seeding                  */
/* ================================================================== */
static void
stress_back_to_back_no_reseed_for_cfg(const struct stress_runtime_cfg *cfg)
{
	uint32_t crc_a = 0U;
	uint32_t crc_b = 0U;
	uint32_t crc_sw;
	struct crc_params params = {
		.data_in = (uint8_t *)crc_test_data,
		.len = crc_test_data_len,
		.bit_swap = CRC_FALSE,
		.byte_swap = CRC_FALSE,
		.reflect = CRC_FALSE,
		.invert = CRC_FALSE,
		.custom_poly = CRC_FALSE,
		.data_out = &crc_a,
	};

	zassert_equal(stress_prepare_seed_and_poly(cfg, &params, 0x00U), 0,
		      "%s seed failed", cfg->name);
	zassert_equal(crc_compute(crc_dev, &params), 0, "%s compute A failed",
		      cfg->name);

	crc_sw = stress_sw_reference(cfg, crc_test_data, crc_test_data_len);
	zassert_equal(crc_a & cfg->mask, crc_sw,
		      "%s HW/SW mismatch: HW=0x%X SW=0x%X", cfg->name,
		      crc_a & cfg->mask, crc_sw);

	params.data_out = &crc_b;
	zassert_equal(crc_compute(crc_dev, &params), 0, "%s compute B failed",
		      cfg->name);
	zassert_equal(crc_a & cfg->mask, crc_b & cfg->mask,
		      "%s back-to-back mismatch: A=0x%X B=0x%X", cfg->name,
		      crc_a & cfg->mask, crc_b & cfg->mask);

	LOG_INF("STRESS-2 [%s]: A=0x%X B=0x%X", cfg->name, crc_a & cfg->mask,
		crc_b & cfg->mask);
}

ZTEST(crc_stress_tests, test_stress_back_to_back_no_reseed)
{
	STRESS_RUN_FOR_EACH_CFG(stress_back_to_back_no_reseed_for_cfg);
}

/* ================================================================== */
/* STRESS-5: All-zeros input (various lengths)                        */
/* ================================================================== */
static void stress_all_zeros_for_cfg(const struct stress_runtime_cfg *cfg)
{
	static const uint32_t lengths[] = {1U, 2U, 4U, 8U, 16U, 32U, 64U};
	uint32_t crc_prev = 0U;
	uint32_t crc_curr = 0U;
	uint32_t crc_sw;
	struct crc_params params = {
		.data_in = all_zeros,
		.len = 0U,
		.bit_swap = CRC_FALSE,
		.byte_swap = CRC_FALSE,
		.reflect = CRC_FALSE,
		.invert = CRC_FALSE,
		.custom_poly = CRC_FALSE,
		.data_out = &crc_curr,
	};

	for (size_t i = 0; i < ARRAY_SIZE(lengths); i++) {
		params.len = lengths[i];
		zassert_equal(stress_prepare_seed_and_poly(cfg, &params, 0x00U),
			      0, "%s seed len %u failed", cfg->name,
			      lengths[i]);
		zassert_equal(crc_compute(crc_dev, &params), 0,
			      "%s compute len %u failed", cfg->name,
			      lengths[i]);

		crc_sw = stress_sw_reference(cfg, all_zeros, lengths[i]);
		zassert_equal(crc_curr & cfg->mask, crc_sw,
			      "%s zeros len %u mismatch: HW=0x%X SW=0x%X",
			      cfg->name, lengths[i], crc_curr & cfg->mask,
			      crc_sw);

		if ((i > 0U) && (crc_curr == crc_prev)) {
			LOG_INF("STRESS-5 [%s]: equal CRC len %u and %u",
				cfg->name, lengths[i - 1U], lengths[i]);
		}

		LOG_INF("STRESS-5 [%s]: zeros len=%u CRC=0x%X", cfg->name,
			lengths[i], crc_curr & cfg->mask);
		crc_prev = crc_curr;
	}
}

ZTEST(crc_stress_tests, test_stress_all_zeros)
{
	STRESS_RUN_FOR_EACH_CFG(stress_all_zeros_for_cfg);
}

/* ================================================================== */
/* STRESS-7: Rapid repeated compute — detect register state leakage   */
/* ================================================================== */
static void
stress_rapid_100_iterations_for_cfg(const struct stress_runtime_cfg *cfg)
{
	uint32_t crc_ref = 0U;
	uint32_t crc_curr = 0U;
	uint32_t crc_sw;
	struct crc_params params = {
		.data_in = (uint8_t *)crc_check_string,
		.len = CRC_CHECK_STRING_LEN,
		.bit_swap = CRC_FALSE,
		.byte_swap = CRC_FALSE,
		.reflect = CRC_FALSE,
		.invert = CRC_FALSE,
		.custom_poly = CRC_FALSE,
		.data_out = &crc_ref,
	};

	zassert_equal(stress_prepare_seed_and_poly(cfg, &params, 0x00U), 0,
		      "%s seed failed", cfg->name);
	zassert_equal(crc_compute(crc_dev, &params), 0,
		      "%s reference compute failed", cfg->name);

	crc_sw = stress_sw_reference(cfg, crc_check_string,
				     CRC_CHECK_STRING_LEN);
	zassert_equal(crc_ref & cfg->mask, crc_sw,
		      "%s HW/SW mismatch: HW=0x%X SW=0x%X", cfg->name,
		      crc_ref & cfg->mask, crc_sw);

	for (int i = 0; i < 100; i++) {
		params.data_out = &crc_curr;
		zassert_equal(stress_prepare_seed_and_poly(cfg, &params, 0x00U),
			      0, "%s seed iter %d failed", cfg->name, i);
		zassert_equal(crc_compute(crc_dev, &params), 0,
			      "%s compute iter %d failed", cfg->name, i);
		zassert_equal(crc_ref & cfg->mask, crc_curr & cfg->mask,
			      "%s rapid mismatch iter %d: 0x%X != 0x%X",
			      cfg->name, i, crc_ref & cfg->mask,
			      crc_curr & cfg->mask);
	}

	LOG_INF("STRESS-7 [%s]: 100 rapid iterations OK, CRC=0x%X", cfg->name,
		crc_ref & cfg->mask);
}

ZTEST(crc_stress_tests, test_stress_rapid_100_iterations)
{
	STRESS_RUN_FOR_EACH_CFG(stress_rapid_100_iterations_for_cfg);
}

/* ================================================================== */
/* STRESS-8: Alternating param combos — detect stale register bits    */
/* ================================================================== */
static void
stress_alternating_params_for_cfg(const struct stress_runtime_cfg *cfg)
{
	uint32_t crc_plain = 0U;
	uint32_t crc_reflect = 0U;
	uint32_t crc_invert = 0U;
	uint32_t crc_plain2 = 0U;
	uint32_t sw_plain;
	uint32_t sw_reflect;
	uint32_t sw_invert;
	struct crc_params params = {
		.data_in = (uint8_t *)crc_test_data,
		.len = crc_test_data_len,
		.bit_swap = CRC_FALSE,
		.byte_swap = CRC_FALSE,
		.reflect = CRC_FALSE,
		.invert = CRC_FALSE,
		.custom_poly = CRC_FALSE,
		.data_out = &crc_plain,
	};

	zassert_equal(stress_prepare_seed_and_poly(cfg, &params, 0x00U), 0,
		      "%s plain seed failed", cfg->name);
	zassert_equal(crc_compute(crc_dev, &params), 0,
		      "%s plain compute failed", cfg->name);

	sw_plain = stress_sw_reference(cfg, crc_test_data, crc_test_data_len);
	zassert_equal(crc_plain & cfg->mask, sw_plain,
		      "%s plain HW/SW mismatch: HW=0x%X SW=0x%X", cfg->name,
		      crc_plain & cfg->mask, sw_plain);

	params.reflect = CRC_TRUE;
	params.data_out = &crc_reflect;
	zassert_equal(stress_prepare_seed_and_poly(cfg, &params, 0x00U), 0,
		      "%s reflect seed failed", cfg->name);
	zassert_equal(crc_compute(crc_dev, &params), 0,
		      "%s reflect compute failed", cfg->name);
	sw_reflect = stress_sw_reference_with_params(cfg, crc_test_data,
						     crc_test_data_len, 0x00U,
						     false, false, true, false);
	zassert_equal(crc_reflect & cfg->mask, sw_reflect,
		      "%s reflect HW/SW mismatch: HW=0x%X SW=0x%X", cfg->name,
		      crc_reflect & cfg->mask, sw_reflect);

	params.reflect = CRC_FALSE;
	params.invert = CRC_TRUE;
	params.data_out = &crc_invert;
	zassert_equal(stress_prepare_seed_and_poly(cfg, &params, 0x00U), 0,
		      "%s invert seed failed", cfg->name);
	zassert_equal(crc_compute(crc_dev, &params), 0,
		      "%s invert compute failed", cfg->name);
	sw_invert = stress_sw_reference_with_params(cfg, crc_test_data,
						    crc_test_data_len, 0x00U,
						    false, false, false, true);
	zassert_equal(crc_invert & cfg->mask, sw_invert,
		      "%s invert HW/SW mismatch: HW=0x%X SW=0x%X", cfg->name,
		      crc_invert & cfg->mask, sw_invert);

	params.invert = CRC_FALSE;
	params.data_out = &crc_plain2;
	zassert_equal(stress_prepare_seed_and_poly(cfg, &params, 0x00U), 0,
		      "%s plain2 seed failed", cfg->name);
	zassert_equal(crc_compute(crc_dev, &params), 0,
		      "%s plain2 compute failed", cfg->name);

	zassert_not_equal(crc_plain & cfg->mask, crc_reflect & cfg->mask,
			  "%s plain and reflect matched: 0x%X", cfg->name,
			  crc_plain & cfg->mask);
	zassert_not_equal(crc_plain & cfg->mask, crc_invert & cfg->mask,
			  "%s plain and invert matched: 0x%X", cfg->name,
			  crc_plain & cfg->mask);
	zassert_equal(crc_plain & cfg->mask, crc_plain2 & cfg->mask,
		      "%s stale register state: 0x%X != 0x%X", cfg->name,
		      crc_plain & cfg->mask, crc_plain2 & cfg->mask);

	LOG_INF("STRESS-8 [%s]: p=0x%X r=0x%X i=0x%X p2=0x%X", cfg->name,
		crc_plain & cfg->mask, crc_reflect & cfg->mask,
		crc_invert & cfg->mask, crc_plain2 & cfg->mask);
}

ZTEST(crc_stress_tests, test_stress_alternating_params)
{
	STRESS_RUN_FOR_EACH_CFG(stress_alternating_params_for_cfg);
}

/* ================================================================== */
/* STRESS-9: HW output width validation                               */
/* ================================================================== */
static void
stress_output_width_check_for_cfg(const struct stress_runtime_cfg *cfg)
{
	uint32_t crc_output = 0xDEADBEEFU;
	uint32_t crc_sw;
	struct crc_params params = {
		.data_in = (uint8_t *)crc_test_data,
		.len = crc_test_data_len,
		.bit_swap = CRC_FALSE,
		.byte_swap = CRC_FALSE,
		.reflect = CRC_FALSE,
		.invert = CRC_FALSE,
		.custom_poly = CRC_FALSE,
		.data_out = &crc_output,
	};

	zassert_equal(stress_prepare_seed_and_poly(cfg, &params, 0x00U), 0,
		      "%s seed failed", cfg->name);
	zassert_equal(crc_compute(crc_dev, &params), 0, "%s compute failed",
		      cfg->name);

	crc_sw = stress_sw_reference(cfg, crc_test_data, crc_test_data_len);
	zassert_equal(crc_output & cfg->mask, crc_sw,
		      "%s HW/SW mismatch: HW=0x%X SW=0x%X", cfg->name,
		      crc_output & cfg->mask, crc_sw);
	zassert_equal(crc_output & ~cfg->mask, 0U,
		      "%s upper bits set unexpectedly: raw=0x%08X", cfg->name,
		      crc_output);

	LOG_INF("STRESS-9 [%s]: raw HW output=0x%08X", cfg->name, crc_output);
}

ZTEST(crc_stress_tests, test_stress_output_width_check)
{
	bool ran = false;

	for (size_t cfg_idx = 0; cfg_idx < ARRAY_SIZE(stress_runtime_cfgs);
	     cfg_idx++) {
		const struct stress_runtime_cfg *cfg =
			&stress_runtime_cfgs[cfg_idx];

		if (cfg->bits == 32U) {
			continue;
		}

		ran = true;
		stress_output_width_check_for_cfg(cfg);
	}

	if (!ran) {
		ztest_test_skip();
	}
}

/* ================================================================== */
/* STRESS-10: Known vector "123456789" — cross-check with SW          */
/* ================================================================== */
static void
stress_known_vector_123456789_for_cfg(const struct stress_runtime_cfg *cfg)
{
	uint32_t crc_hw = 0U;
	uint32_t crc_sw;
	struct crc_params params = {
		.data_in = crc_check_string,
		.len = CRC_CHECK_STRING_LEN,
		.bit_swap = CRC_FALSE,
		.byte_swap = CRC_FALSE,
		.reflect = CRC_FALSE,
		.invert = CRC_FALSE,
		.custom_poly = CRC_FALSE,
		.data_out = &crc_hw,
	};

	zassert_equal(stress_prepare_seed_and_poly(cfg, &params, 0x00U), 0,
		      "%s seed failed", cfg->name);
	zassert_equal(crc_compute(crc_dev, &params), 0, "%s compute failed",
		      cfg->name);

	crc_sw = stress_sw_reference(cfg, crc_check_string,
				     CRC_CHECK_STRING_LEN);
	zassert_equal(crc_hw & cfg->mask, crc_sw,
		      "%s known vector mismatch: HW=0x%X SW=0x%X", cfg->name,
		      crc_hw & cfg->mask, crc_sw);

	LOG_INF("STRESS-10 [%s]: '123456789' HW=0x%X SW=0x%X", cfg->name,
		crc_hw & cfg->mask, crc_sw);
}

ZTEST(crc_stress_tests, test_stress_known_vector_123456789)
{
	STRESS_RUN_FOR_EACH_CFG(stress_known_vector_123456789_for_cfg);
}

/* ================================================================== */
/* STRESS-11: Large data — 256 bytes repeated pattern                 */
/* ================================================================== */
static void stress_large_data_256_for_cfg(const struct stress_runtime_cfg *cfg)
{
	static uint8_t large_buf[256];
	uint32_t crc_hw = 0U;
	uint32_t crc_hw2 = 0U;
	uint32_t crc_sw;
	struct crc_params params = {
		.data_in = large_buf,
		.len = sizeof(large_buf),
		.bit_swap = CRC_FALSE,
		.byte_swap = CRC_FALSE,
		.reflect = CRC_FALSE,
		.invert = CRC_FALSE,
		.custom_poly = CRC_FALSE,
		.data_out = &crc_hw,
	};

	for (size_t i = 0; i < ARRAY_SIZE(large_buf); i++) {
		large_buf[i] = (uint8_t)(i & 0xFFU);
	}

	zassert_equal(stress_prepare_seed_and_poly(cfg, &params, 0x00U), 0,
		      "%s seed failed", cfg->name);
	zassert_equal(crc_compute(crc_dev, &params), 0,
		      "%s first 256-byte compute failed", cfg->name);

	crc_sw = stress_sw_reference(cfg, large_buf, sizeof(large_buf));
	zassert_equal(crc_hw & cfg->mask, crc_sw,
		      "%s 256-byte mismatch: HW=0x%X SW=0x%X", cfg->name,
		      crc_hw & cfg->mask, crc_sw);

	params.data_out = &crc_hw2;
	zassert_equal(stress_prepare_seed_and_poly(cfg, &params, 0x00U), 0,
		      "%s repeat seed failed", cfg->name);
	zassert_equal(crc_compute(crc_dev, &params), 0,
		      "%s repeat 256-byte compute failed", cfg->name);
	zassert_equal(crc_hw & cfg->mask, crc_hw2 & cfg->mask,
		      "%s non-deterministic 256-byte CRC: 0x%X != 0x%X",
		      cfg->name, crc_hw & cfg->mask, crc_hw2 & cfg->mask);

	LOG_INF("STRESS-11 [%s]: 256-byte CRC=0x%X", cfg->name,
		crc_hw & cfg->mask);
}

ZTEST(crc_stress_tests, test_stress_large_data_256)
{
	STRESS_RUN_FOR_EACH_CFG(stress_large_data_256_for_cfg);
}

/* ================================================================== */
/* STRESS-12: Bit-flip sensitivity — flipping one bit must change CRC */
/* ================================================================== */
static void
stress_bit_flip_sensitivity_for_cfg(const struct stress_runtime_cfg *cfg)
{
	uint8_t buf_orig[crc_test_data_len];
	uint8_t buf_flipped[crc_test_data_len];
	uint32_t crc_orig = 0U;
	uint32_t crc_flipped = 0U;
	uint32_t sw_orig;
	uint32_t sw_flipped;
	struct crc_params params = {
		.data_in = buf_orig,
		.len = sizeof(buf_orig),
		.bit_swap = CRC_FALSE,
		.byte_swap = CRC_FALSE,
		.reflect = CRC_FALSE,
		.invert = CRC_FALSE,
		.custom_poly = CRC_FALSE,
		.data_out = &crc_orig,
	};

	memcpy(buf_orig, crc_test_data, sizeof(buf_orig));
	memcpy(buf_flipped, crc_test_data, sizeof(buf_flipped));
	buf_flipped[4] ^= 0x01U;

	zassert_equal(stress_prepare_seed_and_poly(cfg, &params, 0x00U), 0,
		      "%s seed failed", cfg->name);
	zassert_equal(crc_compute(crc_dev, &params), 0,
		      "%s original compute failed", cfg->name);

	sw_orig = stress_sw_reference(cfg, buf_orig, sizeof(buf_orig));
	zassert_equal(crc_orig & cfg->mask, sw_orig,
		      "%s original HW/SW mismatch: HW=0x%X SW=0x%X", cfg->name,
		      crc_orig & cfg->mask, sw_orig);

	params.data_in = buf_flipped;
	params.data_out = &crc_flipped;
	zassert_equal(stress_prepare_seed_and_poly(cfg, &params, 0x00U), 0,
		      "%s flipped seed failed", cfg->name);
	zassert_equal(crc_compute(crc_dev, &params), 0,
		      "%s flipped compute failed", cfg->name);

	sw_flipped = stress_sw_reference(cfg, buf_flipped, sizeof(buf_flipped));
	zassert_equal(crc_flipped & cfg->mask, sw_flipped,
		      "%s flipped HW/SW mismatch: HW=0x%X SW=0x%X", cfg->name,
		      crc_flipped & cfg->mask, sw_flipped);
	zassert_not_equal(crc_orig & cfg->mask, crc_flipped & cfg->mask,
			  "%s bit flip undetected: 0x%X == 0x%X", cfg->name,
			  crc_orig & cfg->mask, crc_flipped & cfg->mask);

	LOG_INF("STRESS-12 [%s]: orig=0x%X flipped=0x%X", cfg->name,
		crc_orig & cfg->mask, crc_flipped & cfg->mask);
}

ZTEST(crc_stress_tests, test_stress_bit_flip_sensitivity)
{
	STRESS_RUN_FOR_EACH_CFG(stress_bit_flip_sensitivity_for_cfg);
}

/* ================================================================== */
/* STRESS-13: Reflect + Invert combined — verify orthogonality        */
/* ================================================================== */
static void
stress_reflect_invert_orthogonal_for_cfg(const struct stress_runtime_cfg *cfg)
{
	uint32_t crc_none = 0U;
	uint32_t crc_ref = 0U;
	uint32_t crc_inv = 0U;
	uint32_t crc_both = 0U;
	uint32_t sw_none;
	uint32_t sw_ref;
	uint32_t sw_inv;
	uint32_t sw_both;
	struct crc_params params = {
		.data_in = (uint8_t *)crc_test_data,
		.len = crc_test_data_len,
		.bit_swap = CRC_FALSE,
		.byte_swap = CRC_FALSE,
		.reflect = CRC_FALSE,
		.invert = CRC_FALSE,
		.custom_poly = CRC_FALSE,
		.data_out = &crc_none,
	};

	zassert_equal(stress_prepare_seed_and_poly(cfg, &params, 0x00U), 0,
		      "%s none seed failed", cfg->name);
	zassert_equal(crc_compute(crc_dev, &params), 0,
		      "%s none compute failed", cfg->name);

	sw_none = stress_sw_reference(cfg, crc_test_data, crc_test_data_len);
	zassert_equal(crc_none & cfg->mask, sw_none,
		      "%s baseline HW/SW mismatch: HW=0x%X SW=0x%X", cfg->name,
		      crc_none & cfg->mask, sw_none);

	params.reflect = CRC_TRUE;
	params.data_out = &crc_ref;
	zassert_equal(stress_prepare_seed_and_poly(cfg, &params, 0x00U), 0,
		      "%s reflect seed failed", cfg->name);
	zassert_equal(crc_compute(crc_dev, &params), 0,
		      "%s reflect compute failed", cfg->name);
	sw_ref = stress_sw_reference_with_params(cfg, crc_test_data,
						 crc_test_data_len, 0x00U,
						 false, false, true, false);
	zassert_equal(crc_ref & cfg->mask, sw_ref,
		      "%s reflect HW/SW mismatch: HW=0x%X SW=0x%X", cfg->name,
		      crc_ref & cfg->mask, sw_ref);

	params.reflect = CRC_FALSE;
	params.invert = CRC_TRUE;
	params.data_out = &crc_inv;
	zassert_equal(stress_prepare_seed_and_poly(cfg, &params, 0x00U), 0,
		      "%s invert seed failed", cfg->name);
	zassert_equal(crc_compute(crc_dev, &params), 0,
		      "%s invert compute failed", cfg->name);
	sw_inv = stress_sw_reference_with_params(cfg, crc_test_data,
						 crc_test_data_len, 0x00U,
						 false, false, false, true);
	zassert_equal(crc_inv & cfg->mask, sw_inv,
		      "%s invert HW/SW mismatch: HW=0x%X SW=0x%X", cfg->name,
		      crc_inv & cfg->mask, sw_inv);

	params.reflect = CRC_TRUE;
	params.invert = CRC_TRUE;
	params.data_out = &crc_both;
	zassert_equal(stress_prepare_seed_and_poly(cfg, &params, 0x00U), 0,
		      "%s both seed failed", cfg->name);
	zassert_equal(crc_compute(crc_dev, &params), 0,
		      "%s both compute failed", cfg->name);
	sw_both = stress_sw_reference_with_params(cfg, crc_test_data,
						  crc_test_data_len, 0x00U,
						  false, false, true, true);
	zassert_equal(crc_both & cfg->mask, sw_both,
		      "%s reflect+invert HW/SW mismatch: HW=0x%X SW=0x%X",
		      cfg->name, crc_both & cfg->mask, sw_both);

	zassert_not_equal(crc_none & cfg->mask, crc_ref & cfg->mask,
			  "%s none == reflect", cfg->name);
	zassert_not_equal(crc_none & cfg->mask, crc_inv & cfg->mask,
			  "%s none == invert", cfg->name);
	zassert_not_equal(crc_none & cfg->mask, crc_both & cfg->mask,
			  "%s none == both", cfg->name);
	zassert_not_equal(crc_ref & cfg->mask, crc_inv & cfg->mask,
			  "%s reflect == invert", cfg->name);
	zassert_not_equal(crc_ref & cfg->mask, crc_both & cfg->mask,
			  "%s reflect == both", cfg->name);
	zassert_not_equal(crc_inv & cfg->mask, crc_both & cfg->mask,
			  "%s invert == both", cfg->name);

	LOG_INF("STRESS-13 [%s]: none=0x%X ref=0x%X inv=0x%X both=0x%X",
		cfg->name, crc_none & cfg->mask, crc_ref & cfg->mask,
		crc_inv & cfg->mask, crc_both & cfg->mask);
}

ZTEST(crc_stress_tests, test_stress_reflect_invert_orthogonal)
{
	STRESS_RUN_FOR_EACH_CFG(stress_reflect_invert_orthogonal_for_cfg);
}

/* ================================================================== */
/* STRESS-14: Swap sensitivity — bit_swap and byte_swap               */
/* ================================================================== */
static void
stress_swap_sensitivity_for_cfg(const struct stress_runtime_cfg *cfg)
{
	uint32_t crc_nosw = 0U;
	uint32_t crc_bitsw = 0U;
	uint32_t crc_bytesw = 0U;
	uint32_t crc_bothsw = 0U;
	uint32_t sw_nosw;
	uint32_t sw_bitsw = 0U;
	uint32_t sw_bytesw = 0U;
	uint32_t sw_bothsw = 0U;
	struct crc_params params = {
		.data_in = (uint8_t *)crc_test_data,
		.len = crc_test_data_len,
		.bit_swap = CRC_FALSE,
		.byte_swap = CRC_FALSE,
		.reflect = CRC_FALSE,
		.invert = CRC_FALSE,
		.custom_poly = CRC_FALSE,
		.data_out = &crc_nosw,
	};

	zassert_equal(stress_prepare_seed_and_poly(cfg, &params, 0x00U), 0,
		      "%s no-swap seed failed", cfg->name);
	zassert_equal(crc_compute(crc_dev, &params), 0,
		      "%s no-swap compute failed", cfg->name);

	sw_nosw = stress_sw_reference(cfg, crc_test_data, crc_test_data_len);
	zassert_equal(crc_nosw & cfg->mask, sw_nosw,
		      "%s baseline HW/SW mismatch: HW=0x%X SW=0x%X", cfg->name,
		      crc_nosw & cfg->mask, sw_nosw);

	params.bit_swap = CRC_TRUE;
	params.byte_swap = CRC_FALSE;
	params.data_out = &crc_bitsw;
	zassert_equal(stress_prepare_seed_and_poly(cfg, &params, 0x00U), 0,
		      "%s bit-swap seed failed", cfg->name);
	zassert_equal(crc_compute(crc_dev, &params), 0,
		      "%s bit-swap compute failed", cfg->name);

	params.bit_swap = CRC_FALSE;
	params.byte_swap = CRC_TRUE;
	params.data_out = &crc_bytesw;
	zassert_equal(stress_prepare_seed_and_poly(cfg, &params, 0x00U), 0,
		      "%s byte-swap seed failed", cfg->name);
	zassert_equal(crc_compute(crc_dev, &params), 0,
		      "%s byte-swap compute failed", cfg->name);

	params.bit_swap = CRC_TRUE;
	params.byte_swap = CRC_TRUE;
	params.data_out = &crc_bothsw;
	zassert_equal(stress_prepare_seed_and_poly(cfg, &params, 0x00U), 0,
		      "%s both-swap seed failed", cfg->name);
	zassert_equal(crc_compute(crc_dev, &params), 0,
		      "%s both-swap compute failed", cfg->name);

	if (cfg->bits == 32U) {
		sw_bitsw = stress_sw_reference_with_params(
			cfg, crc_test_data, crc_test_data_len, 0x00U, true,
			false, false, false);
		sw_bytesw = stress_sw_reference_with_params(
			cfg, crc_test_data, crc_test_data_len, 0x00U, false,
			true, false, false);
		sw_bothsw = stress_sw_reference_with_params(
			cfg, crc_test_data, crc_test_data_len, 0x00U, true,
			true, false, false);
		zassert_equal(crc_bitsw & cfg->mask, sw_bitsw,
			      "%s bit-swap HW/SW mismatch: HW=0x%X SW=0x%X",
			      cfg->name, crc_bitsw & cfg->mask, sw_bitsw);
		zassert_equal(crc_bytesw & cfg->mask, sw_bytesw,
			      "%s byte-swap HW/SW mismatch: HW=0x%X SW=0x%X",
			      cfg->name, crc_bytesw & cfg->mask, sw_bytesw);
		zassert_equal(crc_bothsw & cfg->mask, sw_bothsw,
			      "%s both-swap HW/SW mismatch: HW=0x%X SW=0x%X",
			      cfg->name, crc_bothsw & cfg->mask, sw_bothsw);
	}

	zassert_not_equal(crc_nosw & cfg->mask, crc_bitsw & cfg->mask,
			  "%s no swap == bit swap", cfg->name);
	zassert_not_equal(crc_nosw & cfg->mask, crc_bytesw & cfg->mask,
			  "%s no swap == byte swap", cfg->name);
	zassert_not_equal(crc_nosw & cfg->mask, crc_bothsw & cfg->mask,
			  "%s no swap == both swap", cfg->name);

	LOG_INF("STRESS-14 [%s]: nosw=0x%X bitsw=0x%X bytesw=0x%X both=0x%X",
		cfg->name, crc_nosw & cfg->mask, crc_bitsw & cfg->mask,
		crc_bytesw & cfg->mask, crc_bothsw & cfg->mask);
}

ZTEST(crc_stress_tests, test_stress_swap_sensitivity)
{
	STRESS_RUN_FOR_EACH_CFG(stress_swap_sensitivity_for_cfg);
}

/* ================================================================== */
/* STRESS-NEG: Edge-case input validation                             */
/* ================================================================== */
static void
stress_zero_length_input_for_cfg(const struct stress_runtime_cfg *cfg)
{
	uint32_t crc_hw = 0xDEADBEEFU;
	uint8_t dummy = 0xAAU;
	int ret;
	struct crc_params params = {
		.data_in = &dummy,
		.len = 0U,
		.bit_swap = CRC_FALSE,
		.byte_swap = CRC_FALSE,
		.reflect = CRC_FALSE,
		.invert = CRC_FALSE,
		.custom_poly = CRC_FALSE,
		.data_out = &crc_hw,
	};

	zassert_equal(stress_prepare_seed_and_poly(cfg, &params, 0x00U), 0,
		      "%s seed failed", cfg->name);

	ret = crc_compute(crc_dev, &params);
	zassert_equal(ret, -EINVAL,
		      "%s len=0 should fail with -EINVAL (ret=%d)", cfg->name,
		      ret);

	LOG_INF("STRESS-NEG [%s]: len=0 rejected with %d", cfg->name, ret);
}

ZTEST(crc_stress_tests, test_stress_zero_length_input)
{
	STRESS_RUN_FOR_EACH_CFG(stress_zero_length_input_for_cfg);
}

/* ================================================================== */
/* STRESS-NEG-NULL: Safe NULL field validation                        */
/* ================================================================== */
static void stress_null_pointers_for_cfg(const struct stress_runtime_cfg *cfg)
{
	uint32_t crc_output = 0U;
	struct crc_params params = {
		.data_in = (uint8_t *)crc_test_data,
		.len = crc_test_data_len,
		.bit_swap = CRC_FALSE,
		.byte_swap = CRC_FALSE,
		.reflect = CRC_FALSE,
		.invert = CRC_FALSE,
		.custom_poly = CRC_FALSE,
		.data_out = &crc_output,
	};

	/*
	 * Kernel-mode callers hit the inline wrappers in crc.h directly, so
	 * passing a NULL device or NULL params pointer would fault before the
	 * driver gets a chance to validate them. Only NULL payload members are
	 * safe to exercise from this test binary.
	 */
	stress_select_runtime_algo(cfg, &params);

	params.data_in = NULL;
	zassert_equal(crc_compute(crc_dev, &params), -EINVAL,
		      "%s NULL data_in should fail with -EINVAL", cfg->name);

	params.data_in = (uint8_t *)crc_test_data;
	params.data_out = NULL;
	zassert_equal(crc_compute(crc_dev, &params), -EINVAL,
		      "%s NULL data_out should fail with -EINVAL", cfg->name);

	LOG_INF("STRESS-NEG-NULL [%s]: safe NULL field checks passed",
		cfg->name);
}

ZTEST(crc_stress_tests, test_stress_null_pointers)
{
	STRESS_RUN_FOR_EACH_CFG(stress_null_pointers_for_cfg);
}

ZTEST_SUITE(crc_stress_tests, NULL, NULL, crc_stress_before, NULL, NULL);

#endif /* CONFIG_TEST_CRC_STRESS */

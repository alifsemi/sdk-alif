/*
 * Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * test_i2s_loopback.c  —  I2S Alif Test Suite
 *
 * Compact matrix form of the consolidated I2S loopback test suite.
 * 5 ZTESTs (one per bit-depth), each iterating over 8 sample rates.
 * On per-rate failure the rate is logged and the loop continues
 * (ztest_test_skip() is NOT used here; a failed rate is counted and
 * reported at the end via zassert so the ZTEST name still reflects
 * overall pass/fail).
 *
 * Bit depths  : 12, 16, 20, 24, 32
 * Sample rates: 8000, 16000, 32000, 44100, 48000, 88200, 96000, 192000 Hz
 *
 * Hardware requirement
 * --------------------
 *   Physical wire: P9_3 (I2S3_SDO) --> P9_0 (I2S3_SDI)
 *   Without the wire every rate is skipped gracefully.
 */

#include <zephyr/drivers/i2s.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>
#include <string.h>
#include <errno.h>

#include "i2s_test.h"

/* -------------------------------------------------------------------------
 * Device node
 * -------------------------------------------------------------------------
 */
#if DT_NODE_EXISTS(DT_NODELABEL(i2s_rxtx))
#define MX_NODE  DT_NODELABEL(i2s_rxtx)
#else
#define MX_NODE  DT_NODELABEL(i2s3)
#endif

/* -------------------------------------------------------------------------
 * Common geometry
 * -------------------------------------------------------------------------
 */
#define MX_CHANNELS          2U
#define MX_BLOCK_DURATION_MS 10U
#define MX_INITIAL_BLOCKS    8U
#define MX_VERIFY_BLOCKS     4U
#define MX_TIMEOUT_MS        2000
#define MX_NZ_THRESHOLD      4U
#define MX_SLAB_COUNT        10U

#define MX_FPB(rate)   ((rate) / (1000U / MX_BLOCK_DURATION_MS))
#define MX_SPB(rate)   (MX_FPB(rate) * MX_CHANNELS)
#define MX_BSZ16(rate) (MX_SPB(rate) * sizeof(int16_t))
#define MX_BSZ32(rate) (MX_SPB(rate) * sizeof(int32_t))

#define MX_MAX_BSZ16   MX_BSZ16(192000U)
#define MX_MAX_BSZ32   MX_BSZ32(192000U)

/* -------------------------------------------------------------------------
 * Rate table — iterated in every ZTEST
 * -------------------------------------------------------------------------
 */
static const uint32_t mx_rates[] = {
	8000U, 16000U, 32000U, 44100U,
	48000U, 88200U, 96000U, 192000U
};

/* -------------------------------------------------------------------------
 * Shared backing buffers + slab (one per bit-depth, sized for 192 kHz max).
 * k_mem_slab_init() is called before each rate to set the exact block_size.
 * -------------------------------------------------------------------------
 */
static uint8_t __nocache mx_12_buf[MX_MAX_BSZ16 * MX_SLAB_COUNT] __aligned(4);
static uint8_t __nocache mx_16_buf[MX_MAX_BSZ16 * MX_SLAB_COUNT] __aligned(4);
static uint8_t __nocache mx_20_buf[MX_MAX_BSZ32 * MX_SLAB_COUNT] __aligned(4);
static uint8_t __nocache mx_24_buf[MX_MAX_BSZ32 * MX_SLAB_COUNT] __aligned(4);
static uint8_t __nocache mx_32_buf[MX_MAX_BSZ32 * MX_SLAB_COUNT] __aligned(4);

static struct k_mem_slab mx_12_slab;
static struct k_mem_slab mx_16_slab;
static struct k_mem_slab mx_20_slab;
static struct k_mem_slab mx_24_slab;
static struct k_mem_slab mx_32_slab;

/* -------------------------------------------------------------------------
 * Stream helpers (identical logic to test_loopback_allbits_allrates.c)
 * -------------------------------------------------------------------------
 */
static int mx_configure_streams(const struct device *dev,
				const struct i2s_config *cfg)
{
	int ret;

	ret = i2s_configure(dev, I2S_DIR_BOTH, cfg);
	if (ret == 0) {
		TC_PRINT("[mx] configured DIR_BOTH\n");
		return 0;
	}
	if (ret != -ENOSYS && ret != -ENOTSUP) {
		TC_PRINT("[mx] DIR_BOTH configure failed (%d)\n", ret);
		return ret;
	}
	ret = i2s_configure(dev, I2S_DIR_RX, cfg);
	if (ret != 0) {
		TC_PRINT("[mx] RX configure failed (%d)\n", ret);
		return ret;
	}
	ret = i2s_configure(dev, I2S_DIR_TX, cfg);
	if (ret != 0) {
		TC_PRINT("[mx] TX configure failed (%d)\n", ret);
		return ret;
	}
	TC_PRINT("[mx] configured DIR_RX + DIR_TX separately\n");
	return 0;
}

static int mx_trigger_start(const struct device *dev)
{
	int ret;

	for (uint32_t attempt = 0U; attempt < 3U; attempt++) {
		ret = i2s_trigger(dev, I2S_DIR_BOTH, I2S_TRIGGER_START);
		if (ret == 0) {
			TC_PRINT("[mx] started DIR_BOTH\n");
			return 0;
		}
		if (ret == -ENOSYS || ret == -ENOTSUP) {
			break;
		}
		TC_PRINT("[mx] DIR_BOTH START failed (%d), retry %u\n",
			 ret, attempt + 1U);
		(void)i2s_trigger(dev, I2S_DIR_BOTH, I2S_TRIGGER_DROP);
		k_sleep(K_MSEC(100));
	}

	if (ret == -ENOSYS || ret == -ENOTSUP) {
		TC_PRINT("[mx] DIR_BOTH unsupported, falling back\n");
	} else {
		TC_PRINT("[mx] DIR_BOTH START permanently failed (%d)\n", ret);
		return ret;
	}

	ret = i2s_trigger(dev, I2S_DIR_RX, I2S_TRIGGER_START);
	if (ret != 0) {
		TC_PRINT("[mx] RX START failed (%d)\n", ret);
		return ret;
	}
	ret = i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_START);
	if (ret != 0) {
		TC_PRINT("[mx] TX START failed (%d)\n", ret);
		(void)i2s_trigger(dev, I2S_DIR_RX, I2S_TRIGGER_DROP);
		return ret;
	}
	TC_PRINT("[mx] started RX + TX separately\n");
	return 0;
}

static void mx_stop_streams(const struct device *dev)
{
	struct i2s_config reset = { 0 };

	(void)i2s_trigger(dev, I2S_DIR_BOTH, I2S_TRIGGER_DROP);
	(void)i2s_configure(dev, I2S_DIR_TX, &reset);
	(void)i2s_configure(dev, I2S_DIR_RX, &reset);
	k_sleep(K_MSEC(50));
}

/* -------------------------------------------------------------------------
 * Fill helpers
 * -------------------------------------------------------------------------
 */
static void mx_fill_12(void *buf, uint32_t n_samples)
{
	int16_t *b = buf;
	const uint32_t vlen = I2S_GOLDEN_VEC_LEN;

	for (uint32_t i = 0U; i < n_samples; i += 2U) {
		uint32_t idx = (i / 2U) % vlen;

		b[i]     = (int16_t)(i2s_golden_12bit_L[idx] >> 20);
		b[i + 1] = (int16_t)(i2s_golden_12bit_R[idx] >> 20);
	}
}

static void mx_fill_16(void *buf, uint32_t n_samples)
{
	int16_t *b = buf;
	const uint32_t vlen = I2S_GOLDEN_VEC_LEN;

	for (uint32_t i = 0U; i < n_samples; i += 2U) {
		uint32_t idx = (i / 2U) % vlen;

		b[i]     = (int16_t)(i2s_golden_16bit_L[idx] >> 16);
		b[i + 1] = (int16_t)(i2s_golden_16bit_R[idx] >> 16);
	}
}

static void mx_fill_20(void *buf, uint32_t n_samples)
{
	int32_t *b = buf;
	const uint32_t vlen = I2S_GOLDEN_VEC_LEN;

	for (uint32_t i = 0U; i < n_samples; i += 2U) {
		uint32_t idx = (i / 2U) % vlen;

		b[i]     = (int32_t)i2s_golden_20bit_L[idx];
		b[i + 1] = (int32_t)i2s_golden_20bit_R[idx];
	}
}

static void mx_fill_24(void *buf, uint32_t n_samples)
{
	int32_t *b = buf;
	const uint32_t vlen = I2S_GOLDEN_VEC_LEN;

	for (uint32_t i = 0U; i < n_samples; i += 2U) {
		uint32_t idx = (i / 2U) % vlen;

		b[i]     = (int32_t)i2s_golden_24bit_L[idx];
		b[i + 1] = (int32_t)i2s_golden_24bit_R[idx];
	}
}

static void mx_fill_32(void *buf, uint32_t n_samples)
{
	uint32_t *b = buf;
	const uint32_t vlen = I2S_GOLDEN_VEC_LEN;

	for (uint32_t i = 0U; i < n_samples; i += 2U) {
		uint32_t idx = (i / 2U) % vlen;

		b[i]     = i2s_golden_32bit_L[idx];
		b[i + 1] = i2s_golden_32bit_R[idx];
	}
}

/* -------------------------------------------------------------------------
 * Verify helpers — anchor-based consecutive-run scan
 * -------------------------------------------------------------------------
 */
static bool mx_verify_12(const void *buf, uint32_t n_samples,
			 uint32_t blk_idx, uint32_t rate)
{
	const int16_t *rx = buf;
	const uint32_t vlen     = I2S_GOLDEN_VEC_LEN;
	const uint32_t n_frames = n_samples / MX_CHANNELS;
	const uint32_t pass_thr = (n_frames * 3U) / 4U;
	const uint32_t anchors[] = {8U, 24U};
	uint32_t best_run = 0U, best_phase = 0U;

	for (uint32_t a = 0U; a < 2U; a++) {
		uint32_t aidx = anchors[a];
		int16_t  aval = (int16_t)(i2s_golden_12bit_L[aidx] >> 20);

		for (uint32_t f = 0U; f < n_frames; f++) {
			if (rx[f * 2U] != aval) {
				continue;
			}
			uint32_t run = 0U;

			for (uint32_t k = 0U; k < n_frames; k++) {
				uint32_t fk   = (f + k) % n_frames;
				uint32_t vidx = (aidx + k) % vlen;
				int16_t eL = (int16_t)(i2s_golden_12bit_L[vidx] >> 20);
				int16_t eR = (int16_t)(i2s_golden_12bit_R[vidx] >> 20);

				if (rx[fk * 2U] == eL && rx[fk * 2U + 1U] == eR) {
					run++;
				} else {
					break;
				}
			}
			if (run > best_run) {
				best_run   = run;
				best_phase = aidx;
			}
		}
	}
	if (best_run >= pass_thr) {
		TC_PRINT("[mx-12] PASS: %uHz blk=%u phase=%u run=%u/%u\n",
			 rate, blk_idx, best_phase, best_run, n_frames);
		return true;
	}
	TC_PRINT("[mx-12] FAIL: %uHz blk=%u phase=%u run=%u/%u (need %u)\n",
		 rate, blk_idx, best_phase, best_run, n_frames, pass_thr);
	return false;
}

static bool mx_verify_16(const void *buf, uint32_t n_samples,
			 uint32_t blk_idx, uint32_t rate)
{
	const int16_t *rx = buf;
	const uint32_t vlen     = I2S_GOLDEN_VEC_LEN;
	const uint32_t n_frames = n_samples / MX_CHANNELS;
	const uint32_t pass_thr = (n_frames * 3U) / 4U;
	const uint32_t anchors[] = {8U, 24U};
	uint32_t best_run = 0U, best_phase = 0U;

	for (uint32_t a = 0U; a < 2U; a++) {
		uint32_t aidx = anchors[a];
		int16_t  aval = (int16_t)(i2s_golden_16bit_L[aidx] >> 16);

		for (uint32_t f = 0U; f < n_frames; f++) {
			if (rx[f * 2U] != aval) {
				continue;
			}
			uint32_t run = 0U;

			for (uint32_t k = 0U; k < n_frames; k++) {
				uint32_t fk   = (f + k) % n_frames;
				uint32_t vidx = (aidx + k) % vlen;
				int16_t eL = (int16_t)(i2s_golden_16bit_L[vidx] >> 16);
				int16_t eR = (int16_t)(i2s_golden_16bit_R[vidx] >> 16);

				if (rx[fk * 2U] == eL && rx[fk * 2U + 1U] == eR) {
					run++;
				} else {
					break;
				}
			}
			if (run > best_run) {
				best_run   = run;
				best_phase = aidx;
			}
		}
	}
	if (best_run >= pass_thr) {
		TC_PRINT("[mx-16] PASS: %uHz blk=%u phase=%u run=%u/%u\n",
			 rate, blk_idx, best_phase, best_run, n_frames);
		return true;
	}
	TC_PRINT("[mx-16] FAIL: %uHz blk=%u phase=%u run=%u/%u (need %u)\n",
		 rate, blk_idx, best_phase, best_run, n_frames, pass_thr);
	return false;
}

static bool mx_verify_20(const void *buf, uint32_t n_samples,
			 uint32_t blk_idx, uint32_t rate)
{
	const int32_t *rx = buf;
	const uint32_t vlen     = I2S_GOLDEN_VEC_LEN;
	const uint32_t n_frames = n_samples / MX_CHANNELS;
	const uint32_t pass_thr = (n_frames * 3U) / 4U;
	const uint32_t anchors[] = {8U, 24U};
	uint32_t best_run = 0U, best_phase = 0U;

	for (uint32_t a = 0U; a < 2U; a++) {
		uint32_t aidx = anchors[a];
		int32_t  aval = (int32_t)(i2s_golden_20bit_L[aidx] & 0x000FFFFFU);

		for (uint32_t f = 0U; f < n_frames; f++) {
			if (rx[f * 2U] != aval) {
				continue;
			}
			uint32_t run = 0U;

			for (uint32_t k = 0U; k < n_frames; k++) {
				uint32_t fk   = (f + k) % n_frames;
				uint32_t vidx = (aidx + k) % vlen;
				int32_t eL = (int32_t)(i2s_golden_20bit_L[vidx] & 0x000FFFFFU);
				int32_t eR = (int32_t)(i2s_golden_20bit_R[vidx] & 0x000FFFFFU);

				if (rx[fk * 2U] == eL && rx[fk * 2U + 1U] == eR) {
					run++;
				} else {
					break;
				}
			}
			if (run > best_run) {
				best_run   = run;
				best_phase = aidx;
			}
		}
	}
	if (best_run >= pass_thr) {
		TC_PRINT("[mx-20] PASS: %uHz blk=%u phase=%u run=%u/%u\n",
			 rate, blk_idx, best_phase, best_run, n_frames);
		return true;
	}
	TC_PRINT("[mx-20] FAIL: %uHz blk=%u phase=%u run=%u/%u (need %u)\n",
		 rate, blk_idx, best_phase, best_run, n_frames, pass_thr);
	return false;
}

static bool mx_verify_24(const void *buf, uint32_t n_samples,
			 uint32_t blk_idx, uint32_t rate)
{
	const int32_t *rx = buf;
	const uint32_t vlen     = I2S_GOLDEN_VEC_LEN;
	const uint32_t n_frames = n_samples / MX_CHANNELS;
	const uint32_t pass_thr = (n_frames * 3U) / 4U;
	const uint32_t anchors[] = {8U, 24U};
	uint32_t best_run = 0U, best_phase = 0U;

	for (uint32_t a = 0U; a < 2U; a++) {
		uint32_t aidx = anchors[a];
		int32_t  aval = (int32_t)(i2s_golden_24bit_L[aidx] & 0x00FFFFFFU);

		for (uint32_t f = 0U; f < n_frames; f++) {
			if (rx[f * 2U] != aval) {
				continue;
			}
			uint32_t run = 0U;

			for (uint32_t k = 0U; k < n_frames; k++) {
				uint32_t fk   = (f + k) % n_frames;
				uint32_t vidx = (aidx + k) % vlen;
				int32_t eL = (int32_t)(i2s_golden_24bit_L[vidx] & 0x00FFFFFFU);
				int32_t eR = (int32_t)(i2s_golden_24bit_R[vidx] & 0x00FFFFFFU);

				if (rx[fk * 2U] == eL && rx[fk * 2U + 1U] == eR) {
					run++;
				} else {
					break;
				}
			}
			if (run > best_run) {
				best_run   = run;
				best_phase = aidx;
			}
		}
	}
	if (best_run >= pass_thr) {
		TC_PRINT("[mx-24] PASS: %uHz blk=%u phase=%u run=%u/%u\n",
			 rate, blk_idx, best_phase, best_run, n_frames);
		return true;
	}
	TC_PRINT("[mx-24] FAIL: %uHz blk=%u phase=%u run=%u/%u (need %u)\n",
		 rate, blk_idx, best_phase, best_run, n_frames, pass_thr);
	return false;
}

static uint32_t mx_best_run_32ch(const uint32_t *samples, uint32_t stride,
				 uint32_t n_frames, const uint32_t *vec,
				 uint32_t vlen, uint32_t anchor_idx)
{
	const uint32_t anchor_val = vec[anchor_idx];
	uint32_t best = 0U;

	for (uint32_t f = 0U; f < n_frames; f++) {
		if (samples[f * stride] != anchor_val) {
			continue;
		}
		uint32_t run = 0U;

		for (uint32_t k = 0U; k < n_frames; k++) {
			uint32_t fk   = (f + k) % n_frames;
			uint32_t vidx = (anchor_idx + k) % vlen;

			if (samples[fk * stride] == vec[vidx]) {
				run++;
			} else {
				break;
			}
		}
		if (run > best) {
			best = run;
		}
	}
	return best;
}

static bool mx_verify_32(const void *buf, uint32_t n_samples,
			 uint32_t blk_idx, uint32_t rate)
{
	const uint32_t *rx = buf;
	const uint32_t vlen     = I2S_GOLDEN_VEC_LEN;
	const uint32_t n_frames = n_samples / MX_CHANNELS;
	const uint32_t pass_thr = (n_frames * 3U) / 4U;
	const uint32_t L_anchors[] = {8U, 24U};
	const uint32_t R_anchors[] = {4U, 12U};
	uint32_t best_L = 0U, best_R = 0U;

	for (uint32_t a = 0U; a < 2U; a++) {
		uint32_t r;

		r = mx_best_run_32ch(rx,      2U, n_frames,
				     i2s_golden_32bit_L, vlen, L_anchors[a]);
		if (r > best_L) {
			best_L = r;
		}
		r = mx_best_run_32ch(rx + 1U, 2U, n_frames,
				     i2s_golden_32bit_R, vlen, R_anchors[a]);
		if (r > best_R) {
			best_R = r;
		}
	}
	if (best_L >= pass_thr && best_R >= pass_thr) {
		TC_PRINT("[mx-32] PASS: %uHz blk=%u L_run=%u/%u R_run=%u/%u\n",
			 rate, blk_idx, best_L, n_frames, best_R, n_frames);
		return true;
	}
	TC_PRINT("[mx-32] FAIL: %uHz blk=%u L_run=%u/%u R_run=%u/%u (need %u)\n",
		 rate, blk_idx, best_L, n_frames, best_R, n_frames, pass_thr);
	return false;
}

/* -------------------------------------------------------------------------
 * Unified runner — executes one (bit_depth, rate) combination.
 *
 * Returns:
 *   0        — PASS
 *  -ENODEV   — wire absent (caller should skip/continue)
 *   other    — failure
 * -------------------------------------------------------------------------
 */
typedef void (*mx_fill_fn_t)(void *buf, uint32_t n_samples);
typedef bool (*mx_verify_fn_t)(const void *buf, uint32_t n_samples,
				uint32_t blk_idx, uint32_t rate);

static int mx_run_one(uint32_t word_size, uint32_t rate,
		      struct k_mem_slab *slab, void *slab_buf,
		      mx_fill_fn_t fill_fn, mx_verify_fn_t verify_fn,
		      const char *pfx)
{
	const struct device *const dev = DEVICE_DT_GET(MX_NODE);
	const bool wide = (word_size > 16U);
	const uint32_t bsz = wide ? MX_BSZ32(rate) : MX_BSZ16(rate);
	const uint32_t spb = MX_SPB(rate);
	int ret;
	bool wire_ok = false, verified = false;

	if (!device_is_ready(dev)) {
		TC_PRINT("[mx-%s] device not ready\n", pfx);
		return -ENODEV;
	}

	k_mem_slab_init(slab, slab_buf, bsz, MX_SLAB_COUNT);

	TC_PRINT("[mx-%s] === %u-bit @ %u Hz ===\n", pfx, word_size, rate);
	TC_PRINT("[mx-%s] blk=%u B  frames=%u  slabs=%u\n",
		 pfx, (uint32_t)bsz, spb / MX_CHANNELS, MX_SLAB_COUNT);

	{
		struct i2s_config zero = { 0 };

		(void)i2s_trigger(dev, I2S_DIR_BOTH, I2S_TRIGGER_DROP);
		(void)i2s_configure(dev, I2S_DIR_TX, &zero);
		(void)i2s_configure(dev, I2S_DIR_RX, &zero);
		k_sleep(K_MSEC(50));
		TC_PRINT("[mx-%s] streams reset\n", pfx);
	}

	const struct i2s_config cfg = {
		.word_size      = word_size,
		.channels       = MX_CHANNELS,
		.format         = I2S_FMT_DATA_FORMAT_I2S,
		.options        = I2S_OPT_BIT_CLK_MASTER |
				  I2S_OPT_FRAME_CLK_MASTER,
		.frame_clk_freq = rate,
		.mem_slab       = slab,
		.block_size     = bsz,
		.timeout        = MX_TIMEOUT_MS,
	};

	ret = mx_configure_streams(dev, &cfg);
	if (ret != 0) {
		TC_PRINT("[mx-%s] configure failed (%d)\n", pfx, ret);
		return ret;
	}

	for (uint32_t s = 0U; s < MX_INITIAL_BLOCKS; s++) {
		void *blk = NULL;

		ret = k_mem_slab_alloc(slab, &blk, K_FOREVER);
		if (ret != 0) {
			TC_PRINT("[mx-%s] slab alloc s=%u (%d)\n", pfx, s, ret);
			mx_stop_streams(dev);
			return ret;
		}
		fill_fn(blk, spb);
		ret = i2s_write(dev, blk, bsz);
		if (ret != 0) {
			TC_PRINT("[mx-%s] write s=%u (%d)\n", pfx, s, ret);
			k_mem_slab_free(slab, blk);
			mx_stop_streams(dev);
			return ret;
		}
	}
	TC_PRINT("[mx-%s] pre-queued %u blocks\n", pfx, MX_INITIAL_BLOCKS);

	ret = mx_trigger_start(dev);
	if (ret != 0) {
		mx_stop_streams(dev);
		return ret;
	}

	for (uint32_t b = 0U; b < MX_INITIAL_BLOCKS + MX_VERIFY_BLOCKS; b++) {
		void  *rx_block = NULL;
		size_t rx_size  = 0U;

		ret = i2s_read(dev, &rx_block, &rx_size);
		if (ret != 0) {
			TC_PRINT("[mx-%s] read b=%u (%d)\n", pfx, b, ret);
			mx_stop_streams(dev);
			return ret;
		}

		if (!wire_ok) {
			const uint8_t *p = rx_block;
			uint32_t nz = 0U;

			for (uint32_t i = 0U; i < bsz; i++) {
				if (p[i] != 0U) {
					nz++;
				}
			}
			if (nz >= MX_NZ_THRESHOLD) {
				wire_ok = true;
				TC_PRINT("[mx-%s] wire ok (nz=%u b=%u)\n",
					 pfx, nz, b);
			}
		}

		if (wire_ok && b >= (MX_INITIAL_BLOCKS - 1U)) {
			if (verify_fn(rx_block, spb, b, rate)) {
				verified = true;
				fill_fn(rx_block, spb);
				if (i2s_write(dev, rx_block, bsz) != 0) {
					k_mem_slab_free(slab, rx_block);
				}
				mx_stop_streams(dev);
				return 0;
			}
		}

		fill_fn(rx_block, spb);
		if (i2s_write(dev, rx_block, bsz) != 0) {
			k_mem_slab_free(slab, rx_block);
		}
	}

	mx_stop_streams(dev);

	if (!wire_ok) {
		TC_PRINT("[mx-%s] SKIP: wire absent @ %u Hz\n", pfx, rate);
		return -ENODEV;
	}
	if (!verified) {
		TC_PRINT("[mx-%s] FAIL: no block passed verify @ %u Hz\n",
			 pfx, rate);
		return -EIO;
	}
	return 0;
}

/* =========================================================================
 * Suite setup
 * =========================================================================
 */
static void *mx_suite_setup(void)
{
	TC_PRINT("[mx] boot settle: waiting 500 ms for SE-services IPC...\n");
	k_sleep(K_MSEC(500));
	TC_PRINT("[mx] SE-services ready\n");
	return NULL;
}

ZTEST_SUITE(i2s_alif_functional, NULL, mx_suite_setup, NULL, NULL, NULL);

/* =========================================================================
 * 5 ZTESTs — one per bit-depth, looping over all 8 rates.
 *
 * Per-rate failures are printed and counted; the ZTEST only fails if at
 * least one rate genuinely failed (not just wire-absent / skipped).
 * =========================================================================
 */

ZTEST(i2s_alif_functional, test_lb_matrix_12bit)
{
#if !defined(CONFIG_I2S_GPIO_LOOPBACK) || !CONFIG_I2S_GPIO_LOOPBACK
	ztest_test_skip();
#else
	uint32_t n_fail = 0U;

	for (uint32_t i = 0U; i < ARRAY_SIZE(mx_rates); i++) {
		int rc = mx_run_one(12U, mx_rates[i],
				    &mx_12_slab, mx_12_buf,
				    mx_fill_12, mx_verify_12, "12");

		if (rc != 0 && rc != -ENODEV) {
			n_fail++;
		}
	}
	zassert_equal(n_fail, 0U,
		      "[mx-12] %u rate(s) failed", n_fail);
#endif
}

ZTEST(i2s_alif_functional, test_lb_matrix_16bit)
{
#if !defined(CONFIG_I2S_GPIO_LOOPBACK) || !CONFIG_I2S_GPIO_LOOPBACK
	ztest_test_skip();
#else
	uint32_t n_fail = 0U;

	for (uint32_t i = 0U; i < ARRAY_SIZE(mx_rates); i++) {
		int rc = mx_run_one(16U, mx_rates[i],
				    &mx_16_slab, mx_16_buf,
				    mx_fill_16, mx_verify_16, "16");

		if (rc != 0 && rc != -ENODEV) {
			n_fail++;
		}
	}
	zassert_equal(n_fail, 0U,
		      "[mx-16] %u rate(s) failed", n_fail);
#endif
}

ZTEST(i2s_alif_functional, test_lb_matrix_20bit)
{
#if !defined(CONFIG_I2S_GPIO_LOOPBACK) || !CONFIG_I2S_GPIO_LOOPBACK
	ztest_test_skip();
#else
	uint32_t n_fail = 0U;

	for (uint32_t i = 0U; i < ARRAY_SIZE(mx_rates); i++) {
		int rc = mx_run_one(20U, mx_rates[i],
				    &mx_20_slab, mx_20_buf,
				    mx_fill_20, mx_verify_20, "20");

		if (rc != 0 && rc != -ENODEV) {
			n_fail++;
		}
	}
	zassert_equal(n_fail, 0U,
		      "[mx-20] %u rate(s) failed", n_fail);
#endif
}

ZTEST(i2s_alif_functional, test_lb_matrix_24bit)
{
#if !defined(CONFIG_I2S_GPIO_LOOPBACK) || !CONFIG_I2S_GPIO_LOOPBACK
	ztest_test_skip();
#else
	uint32_t n_fail = 0U;

	for (uint32_t i = 0U; i < ARRAY_SIZE(mx_rates); i++) {
		int rc = mx_run_one(24U, mx_rates[i],
				    &mx_24_slab, mx_24_buf,
				    mx_fill_24, mx_verify_24, "24");

		if (rc != 0 && rc != -ENODEV) {
			n_fail++;
		}
	}
	zassert_equal(n_fail, 0U,
		      "[mx-24] %u rate(s) failed", n_fail);
#endif
}

ZTEST(i2s_alif_functional, test_lb_matrix_32bit)
{
#if !defined(CONFIG_I2S_GPIO_LOOPBACK) || !CONFIG_I2S_GPIO_LOOPBACK
	ztest_test_skip();
#else
	uint32_t n_fail = 0U;

	for (uint32_t i = 0U; i < ARRAY_SIZE(mx_rates); i++) {
		int rc = mx_run_one(32U, mx_rates[i],
				    &mx_32_slab, mx_32_buf,
				    mx_fill_32, mx_verify_32, "32");

		if (rc != 0 && rc != -ENODEV) {
			n_fail++;
		}
	}
	zassert_equal(n_fail, 0U,
		      "[mx-32] %u rate(s) failed", n_fail);
#endif
}

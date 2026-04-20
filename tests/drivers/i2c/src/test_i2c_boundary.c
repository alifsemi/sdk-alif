/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/*
 * I2C Boundary Test Suite
 * -----------------------
 * Focuses on NORMAL-PATH edge cases that commonly expose driver/hardware
 * defects. Error-path cases (NACK, bus recovery, invalid addr) are covered
 * by the fault-injection suite and intentionally NOT duplicated here.
 *
 * Every transfer in this suite is followed by:
 *   1. Data integrity check  (validate_target_rx / validate_controller_rx)
 *   2. Slave-callback contract check (i2c_test_assert_target_contract)
 *
 * The contract check is the primary driver-bug detector: it locks down
 * exactly which slave callbacks fire, how many times, and with what
 * cumulative byte count. Any ISR state-machine regression trips it
 * before data corruption can mask the defect.
 *
 * Targets:
 *   1. DW I2C FIFO threshold boundaries (TX/RX FIFO depth = 32).
 *   2. Single-byte read/write paths (distinct ISR code path).
 *   3. Zero-length probe (write 0 bytes — address-ACK only).
 *   4. RESTART (write-then-read) boundary sizes.
 *   5. Back-to-back zero-gap transfers (STOP->START race).
 */

#include "test_i2c.h"
LOG_MODULE_REGISTER(alif_i2c_boundary, LOG_LEVEL_INF);

/* DW I2C controller FIFO depth (IP-default on Alif Ensemble/Balletto). */
#define DW_FIFO_DEPTH	  32U

/* Max transfer size used across tests. Must fit within BUFF_PERF (128). */
#define BOUNDARY_MAX	  64U

/* Back-to-back iteration count — enough to catch intermittent state races. */
#define BACK_TO_BACK_ITER 32U

struct i2c_boundary_ctx {
	const struct device *controller;
};

static struct i2c_boundary_ctx boundary_ctx;


static uint8_t boundary_tx[BOUNDARY_MAX];
static uint8_t boundary_rx[BOUNDARY_MAX];
static uint8_t boundary_tgt_tx[BOUNDARY_MAX];

static void boundary_fill_tx(size_t len)
{
	for (size_t i = 0; i < len; i++) {
		boundary_tx[i] = (uint8_t)(0x40U + i);
	}
}

static void boundary_fill_tgt_tx(size_t len)
{
	for (size_t i = 0; i < len; i++) {
		boundary_tgt_tx[i] = (uint8_t)(0x80U ^ (uint8_t)i);
	}
}

/**
 * @brief Sweep write sizes across the FIFO depth boundary.
 *
 * Exercises the DW I2C TX FIFO threshold, TX_EMPTY refill, and final-STOP
 * path. Critical sizes chosen to straddle N=32 so that off-by-one bugs in
 * the refill/drain logic surface deterministically. After each transfer
 * the slave-callback contract is asserted to catch ISR state regressions.
 */
ZTEST(i2c_boundary_suite, test_fifo_boundary)
{
	const struct device *controller = boundary_ctx.controller;
	static const size_t sizes[] = {
		1U, 2U,
		DW_FIFO_DEPTH - 1U, DW_FIFO_DEPTH, DW_FIFO_DEPTH + 1U,
		(DW_FIFO_DEPTH * 2U) - 1U, DW_FIFO_DEPTH * 2U,
	};
	int ret;

	for (size_t i = 0; i < ARRAY_SIZE(sizes); i++) {
		size_t size = sizes[i];

		zassert_true(size <= BOUNDARY_MAX, "size %zu exceeds buffer",
			     size);

		boundary_fill_tx(size);
		i2c_test_prime_buffers(NULL, 0U, size);

		ret = i2c_write(controller, boundary_tx, size, TGT_I2C_ADDR);
		zassert_ok(ret, "write size %zu failed: %d", size, ret);

		ret = validate_target_rx(boundary_tx, size);
		zassert_ok(ret, "RX mismatch size %zu: %d", size, ret);

		i2c_test_assert_target_contract(&i2c_test_ctx,
					       I2C_TRANSMIT_ONLY, size, 0U);
	}

	LOG_INF("FIFO boundary sweep passed");
}

/**
 * @brief Sweep read sizes across the RX FIFO depth boundary.
 *
 * Pure reads exercise RX_FULL threshold handling and the master's
 * CMD-register read-request loop. Single-byte reads often use a distinct
 * code path in DW I2C and must be validated explicitly.
 */
ZTEST(i2c_boundary_suite, test_read_size_boundary)
{
	const struct device *controller = boundary_ctx.controller;
	static const size_t sizes[] = {
		1U, 2U,
		DW_FIFO_DEPTH - 1U, DW_FIFO_DEPTH, DW_FIFO_DEPTH + 1U,
		BOUNDARY_MAX,
	};
	int ret;

	for (size_t i = 0; i < ARRAY_SIZE(sizes); i++) {
		size_t size = sizes[i];

		boundary_fill_tgt_tx(size);
		i2c_test_prime_buffers(boundary_tgt_tx, size, 0U);
		poison_buffer(boundary_rx, size, POISON_RX);

		ret = i2c_read(controller, boundary_rx, size, TGT_I2C_ADDR);
		zassert_ok(ret, "read size %zu failed: %d", size, ret);

		ret = validate_controller_rx(boundary_tgt_tx, boundary_rx,
					     size);
		zassert_ok(ret, "read data mismatch size %zu: %d", size, ret);

		i2c_test_assert_target_contract(&i2c_test_ctx,
					       I2C_RECEIVE_ONLY, 0U, size);
	}

	LOG_INF("Read size boundary sweep passed");
}

/**
 * @brief Zero-length probe.
 *
 * Writing zero bytes is a protocol-legal address probe: START + addr+W +
 * (N)ACK + STOP. No data phase. Many drivers crash or hang on this
 * because their transfer loop assumes len > 0. On a registered slave the
 * probe must succeed with write_requested==1 and stop==1 (no byte
 * callbacks).
 */
ZTEST(i2c_boundary_suite, test_zero_length_probe)
{
	const struct device *controller = boundary_ctx.controller;
	int ret;

	i2c_test_prime_buffers(NULL, 0U, 0U);

	ret = i2c_write(controller, NULL, 0U, TGT_I2C_ADDR);
	zassert_ok(ret, "zero-length probe failed: %d", ret);

	/*
	 * Contract: write_requested fires on address match even with no data.
	 * write_received_count must be 0 (no byte callbacks).
	 */
	i2c_test_assert_target_contract(&i2c_test_ctx,
				       I2C_TRANSMIT_ONLY, 0U, 0U);

	/* Bus must be usable immediately after the probe. */
	const size_t follow_len = 4U;

	boundary_fill_tx(follow_len);
	i2c_test_prime_buffers(NULL, 0U, follow_len);

	ret = i2c_write(controller, boundary_tx, follow_len, TGT_I2C_ADDR);
	zassert_ok(ret, "write after zero-len probe failed: %d", ret);

	ret = validate_target_rx(boundary_tx, follow_len);
	zassert_ok(ret, "RX mismatch after probe: %d", ret);

	i2c_test_assert_target_contract(&i2c_test_ctx,
				       I2C_TRANSMIT_ONLY, follow_len, 0U);

	LOG_INF("Zero-length probe + recovery passed");
}

/**
 * @brief Sweep write-then-read (RESTART) sizes.
 *
 * i2c_write_read issues write, repeated-START, then read. Exercises:
 *   - master CMD register RESTART bit handling,
 *   - slave write_requested -> read_requested transition,
 *   - ISR state reset between phases.
 *
 * The 2x2 direction matrix (W->R, W->W, R->W, R->R) is covered across
 * this test and the hardware-aware suite. W->W and R->R are limited by
 * the DW IP and Zephyr master API respectively and are documented there.
 */
ZTEST(i2c_boundary_suite, test_restart_write_read)
{
	const struct device *controller = boundary_ctx.controller;
	static const struct { size_t w; size_t r; } combos[] = {
		{1U,		   1U},
		{1U,		   DW_FIFO_DEPTH},
		{DW_FIFO_DEPTH,	   1U},
		{DW_FIFO_DEPTH - 1U, DW_FIFO_DEPTH + 1U},
		{DW_FIFO_DEPTH + 1U, DW_FIFO_DEPTH - 1U},
		{DW_FIFO_DEPTH,	   DW_FIFO_DEPTH},
	};
	int ret;

	for (size_t i = 0; i < ARRAY_SIZE(combos); i++) {
		size_t w = combos[i].w;
		size_t r = combos[i].r;

		boundary_fill_tx(w);
		boundary_fill_tgt_tx(r);
		i2c_test_prime_buffers(boundary_tgt_tx, r, w);
		poison_buffer(boundary_rx, r, POISON_RX);

		ret = i2c_write_read(controller, TGT_I2C_ADDR,
				     boundary_tx, w, boundary_rx, r);
		zassert_ok(ret, "write_read w=%zu r=%zu failed: %d",
			   w, r, ret);

		ret = validate_target_rx(boundary_tx, w);
		zassert_ok(ret, "target RX mismatch w=%zu r=%zu: %d",
			   w, r, ret);

		ret = validate_controller_rx(boundary_tgt_tx, boundary_rx, r);
		zassert_ok(ret, "controller RX mismatch w=%zu r=%zu: %d",
			   w, r, ret);

		i2c_test_assert_target_contract(&i2c_test_ctx,
					       I2C_TRANSMIT_RECEIVE, w, r);
	}

	LOG_INF("RESTART write_read boundary sweep passed");
}

/**
 * @brief Back-to-back writes with no inter-transfer delay.
 *
 * Stresses STOP -> next START transition: the driver must reset its state
 * machine, re-arm the slave ISR, and clear latched interrupt bits before
 * the next transfer. State races here typically manifest as:
 *   - write_requested_count != 1 (stale START_DET)
 *   - stop_count != 1 (missed STOP_DET)
 *   - -EBUSY on subsequent transfer
 *
 * All three are caught by the contract check at every iteration.
 */
ZTEST(i2c_boundary_suite, test_back_to_back_transfers)
{
	const struct device *controller = boundary_ctx.controller;
	const size_t size = 1U;
	int ret;

	boundary_fill_tx(size);

	for (uint32_t i = 0; i < BACK_TO_BACK_ITER; i++) {
		i2c_test_prime_buffers(NULL, 0U, size);

		ret = i2c_write(controller, boundary_tx, size, TGT_I2C_ADDR);
		zassert_ok(ret, "back-to-back iter %u failed: %d", i, ret);

		ret = validate_target_rx(boundary_tx, size);
		zassert_ok(ret, "back-to-back iter %u RX mismatch: %d",
			   i, ret);

		i2c_test_assert_target_contract(&i2c_test_ctx,
					       I2C_TRANSMIT_ONLY, size, 0U);
	}

	LOG_INF("Back-to-back %u-iter stress passed", BACK_TO_BACK_ITER);
}

static void *i2c_boundary_suite_setup(void)
{
	const struct device *controller = DEVICE_DT_GET(I2C_CONTROLLER);
	const struct device *target = DEVICE_DT_GET(I2C_TARGET);
	int ret;

	zassert_true(device_is_ready(controller),
		     "Controller I2C device not ready");
	zassert_true(device_is_ready(target),
		     "Target I2C device not ready");

	boundary_ctx.controller = controller;

	i2c_test_ctx.controller_dev = controller;
	i2c_test_ctx.target_dev = target;
	i2c_test_reset_runtime_config(&i2c_test_ctx);
	i2c_test_ctx.target_registered = false;

	ret = i2c_configure(i2c_test_ctx.controller_dev, i2c_test_ctx.i2c_cfg);
	zassert_ok(ret, "Controller I2C configuration failed: %d", ret);

	LOG_INF("I2C boundary test suite setup completed");

	return &i2c_test_ctx;
}

static void i2c_boundary_before(void *fixture)
{
	struct i2c_test_ctx *ctx = fixture;

	i2c_test_reset_runtime_config(ctx);
	register_target_i2c();
}

static void i2c_boundary_after(void *fixture)
{
	struct i2c_test_ctx *ctx = fixture;
	int ret;

	/* Unregister target after each test to clean up state */
	if (ctx->target_registered) {
		ret = i2c_target_unregister(ctx->target_dev, &i2c_tcfg);
		if (ret == -ENOTSUP) {
			LOG_WRN("I2C target mode not supported");
		} else if (ret != 0) {
			LOG_ERR("i2c_target_unregister failed: %d", ret);
		}
		ctx->target_registered = false;
	}
}

ZTEST_SUITE(i2c_boundary_suite, NULL, i2c_boundary_suite_setup,
	    i2c_boundary_before, i2c_boundary_after, NULL);

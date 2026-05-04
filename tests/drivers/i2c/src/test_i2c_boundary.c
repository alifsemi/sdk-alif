/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/*
 * I2C boundary test suite: FIFO thresholds, single-byte paths,
 * zero-length probes, RESTART sizes, and back-to-back transfers.
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

/*
 * Async-capable transfer helpers
 */

static int boundary_xfer_write(const uint8_t *data, size_t len, bool async)
{
	struct i2c_msg msg = {
		.buf = (uint8_t *)data,
		.len = len,
		.flags = I2C_MSG_WRITE | I2C_MSG_STOP,
	};
	return i2c_do_xfer(boundary_ctx.controller, &msg, 1,
			   TGT_I2C_ADDR, async);
}

static int boundary_xfer_read(uint8_t *data, size_t len, bool async)
{
	struct i2c_msg msg = {
		.buf = data,
		.len = len,
		.flags = I2C_MSG_READ | I2C_MSG_STOP,
	};
	return i2c_do_xfer(boundary_ctx.controller, &msg, 1,
			   TGT_I2C_ADDR, async);
}

static int boundary_xfer_write_read(const uint8_t *tx, size_t tx_len,
				    uint8_t *rx, size_t rx_len, bool async)
{
	struct i2c_msg msgs[2];

	msgs[0].buf = (uint8_t *)tx;
	msgs[0].len = tx_len;
	msgs[0].flags = I2C_MSG_WRITE | I2C_MSG_RESTART;
	msgs[1].buf = rx;
	msgs[1].len = rx_len;
	msgs[1].flags = I2C_MSG_READ | I2C_MSG_STOP;
	return i2c_do_xfer(boundary_ctx.controller, msgs, 2,
			   TGT_I2C_ADDR, async);
}

/*
 * Scenarios
 */

static int sc_fifo_boundary(bool async)
{
	static const size_t sizes[] = {
		1U, 2U,
		DW_FIFO_DEPTH - 1U, DW_FIFO_DEPTH, DW_FIFO_DEPTH + 1U,
		(DW_FIFO_DEPTH * 2U) - 1U, DW_FIFO_DEPTH * 2U,
	};
	int fail = 0;

	for (size_t i = 0; i < ARRAY_SIZE(sizes); i++) {
		size_t size = sizes[i];
		int ret;

		if (size > BOUNDARY_MAX) {
			LOG_ERR("size %zu exceeds buffer", size);
			fail++;
			continue;
		}

		boundary_fill_tx(size);
		i2c_test_prime_buffers(NULL, 0U, size);

		ret = boundary_xfer_write(boundary_tx, size, async);
		if (ret != 0) {
			LOG_ERR("write size %zu failed: %d", size, ret);
			fail++;
			continue;
		}

		ret = validate_target_rx(boundary_tx, size);
		if (ret != 0) {
			LOG_ERR("RX mismatch size %zu: %d", size, ret);
			fail++;
		}
	}

	TC_PRINT("  FIFO boundary : %s (%d)\n", fail ? "FAIL" : "OK", fail);
	return fail;
}

static int sc_read_size_boundary(bool async)
{
	static const size_t sizes[] = {
		1U, 2U,
		DW_FIFO_DEPTH - 1U, DW_FIFO_DEPTH, DW_FIFO_DEPTH + 1U,
		BOUNDARY_MAX,
	};
	int fail = 0;

	for (size_t i = 0; i < ARRAY_SIZE(sizes); i++) {
		size_t size = sizes[i];
		int ret;

		boundary_fill_tgt_tx(size);
		i2c_test_prime_buffers(boundary_tgt_tx, size, 0U);
		poison_buffer(boundary_rx, size, POISON_RX);

		ret = boundary_xfer_read(boundary_rx, size, async);
		if (ret != 0) {
			LOG_ERR("read size %zu failed: %d", size, ret);
			fail++;
			continue;
		}

		ret = validate_controller_rx(boundary_tgt_tx, boundary_rx, size);
		if (ret != 0) {
			LOG_ERR("read data mismatch size %zu: %d", size, ret);
			fail++;
		}
	}

	TC_PRINT("  read boundary : %s (%d)\n", fail ? "FAIL" : "OK", fail);
	return fail;
}

static int sc_zero_length_probe(bool async)
{
	int ret;
	int fail = 0;
	const size_t follow_len = 4U;

	i2c_test_prime_buffers(NULL, 0U, 0U);

	ret = boundary_xfer_write(NULL, 0U, async);
	if (ret != 0) {
		LOG_ERR("zero-length probe failed: %d", ret);
		fail++;
	}

	/* Bus must be usable immediately after the probe. */
	boundary_fill_tx(follow_len);
	i2c_test_prime_buffers(NULL, 0U, follow_len);

	ret = boundary_xfer_write(boundary_tx, follow_len, async);
	if (ret != 0) {
		LOG_ERR("write after zero-len probe failed: %d", ret);
		fail++;
	} else {
		ret = validate_target_rx(boundary_tx, follow_len);
		if (ret != 0) {
			LOG_ERR("RX mismatch after probe: %d", ret);
			fail++;
		}
	}

	TC_PRINT("  zero-len probe : %s (%d)\n", fail ? "FAIL" : "OK", fail);
	return fail;
}

static int sc_restart_write_read(bool async)
{
	static const struct { size_t w; size_t r; } combos[] = {
		{1U,		   1U},
		{1U,		   DW_FIFO_DEPTH},
		{DW_FIFO_DEPTH,	   1U},
		{DW_FIFO_DEPTH - 1U, DW_FIFO_DEPTH + 1U},
		{DW_FIFO_DEPTH + 1U, DW_FIFO_DEPTH - 1U},
		{DW_FIFO_DEPTH,	   DW_FIFO_DEPTH},
	};
	int fail = 0;

	for (size_t i = 0; i < ARRAY_SIZE(combos); i++) {
		size_t w = combos[i].w;
		size_t r = combos[i].r;
		int ret;

		boundary_fill_tx(w);
		boundary_fill_tgt_tx(r);
		i2c_test_prime_buffers(boundary_tgt_tx, r, w);
		poison_buffer(boundary_rx, r, POISON_RX);

		ret = boundary_xfer_write_read(boundary_tx, w,
					       boundary_rx, r, async);
		if (ret != 0) {
			LOG_ERR("write_read w=%zu r=%zu failed: %d", w, r, ret);
			fail++;
			continue;
		}

		ret = validate_target_rx(boundary_tx, w);
		if (ret != 0) {
			LOG_ERR("target RX mismatch w=%zu r=%zu: %d", w, r, ret);
			fail++;
			continue;
		}

		ret = validate_controller_rx(boundary_tgt_tx, boundary_rx, r);
		if (ret != 0) {
			LOG_ERR("controller RX mismatch w=%zu r=%zu: %d",
				w, r, ret);
			fail++;
		}
	}

	TC_PRINT("  restart wr/rd : %s (%d)\n", fail ? "FAIL" : "OK", fail);
	return fail;
}

static int sc_back_to_back(bool async)
{
	const size_t size = 1U;
	int fail = 0;

	boundary_fill_tx(size);

	for (uint32_t i = 0; i < BACK_TO_BACK_ITER; i++) {
		int ret;

		i2c_test_prime_buffers(NULL, 0U, size);

		ret = boundary_xfer_write(boundary_tx, size, async);
		if (ret != 0) {
			LOG_ERR("back-to-back iter %u failed: %d", i, ret);
			fail++;
			continue;
		}

		ret = validate_target_rx(boundary_tx, size);
		if (ret != 0) {
			LOG_ERR("back-to-back iter %u RX mismatch: %d", i, ret);
			fail++;
		}
	}

	TC_PRINT("  back-to-back  : %s (%d)\n", fail ? "FAIL" : "OK", fail);
	return fail;
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
	i2c_test_async_init();
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

ZTEST(i2c_boundary_suite, test_boundary_fifo_sync)
{
	int fail = sc_fifo_boundary(false);

	zassert_equal(fail, 0, "FIFO boundary sync: %d failure(s)", fail);
}

#if IS_ENABLED(CONFIG_I2C_CALLBACK)
ZTEST(i2c_boundary_suite, test_boundary_fifo_async)
{
	int fail = sc_fifo_boundary(true);

	zassert_equal(fail, 0, "FIFO boundary async: %d failure(s)", fail);
}
#endif

ZTEST(i2c_boundary_suite, test_boundary_read_sync)
{
	int fail = sc_read_size_boundary(false);

	zassert_equal(fail, 0, "Read boundary sync: %d failure(s)", fail);
}

#if IS_ENABLED(CONFIG_I2C_CALLBACK)
ZTEST(i2c_boundary_suite, test_boundary_read_async)
{
	int fail = sc_read_size_boundary(true);

	zassert_equal(fail, 0, "Read boundary async: %d failure(s)", fail);
}
#endif

ZTEST(i2c_boundary_suite, test_boundary_zero_len_sync)
{
	int fail = sc_zero_length_probe(false);

	zassert_equal(fail, 0, "Zero-len probe sync: %d failure(s)", fail);
}

#if IS_ENABLED(CONFIG_I2C_CALLBACK)
ZTEST(i2c_boundary_suite, test_boundary_zero_len_async)
{
	int fail = sc_zero_length_probe(true);

	zassert_equal(fail, 0, "Zero-len probe async: %d failure(s)", fail);
}
#endif

ZTEST(i2c_boundary_suite, test_boundary_restart_sync)
{
	int fail = sc_restart_write_read(false);

	zassert_equal(fail, 0, "Restart WR-RD sync: %d failure(s)", fail);
}

#if IS_ENABLED(CONFIG_I2C_CALLBACK)
ZTEST(i2c_boundary_suite, test_boundary_restart_async)
{
	int fail = sc_restart_write_read(true);

	zassert_equal(fail, 0, "Restart WR-RD async: %d failure(s)", fail);
}
#endif

ZTEST(i2c_boundary_suite, test_boundary_back2back_sync)
{
	int fail = sc_back_to_back(false);

	zassert_equal(fail, 0, "Back-to-back sync: %d failure(s)", fail);
}

#if IS_ENABLED(CONFIG_I2C_CALLBACK)
ZTEST(i2c_boundary_suite, test_boundary_back2back_async)
{
	int fail = sc_back_to_back(true);

	zassert_equal(fail, 0, "Back-to-back async: %d failure(s)", fail);
}
#endif

ZTEST_SUITE(i2c_boundary_suite, NULL, i2c_boundary_suite_setup,
	    i2c_boundary_before, i2c_boundary_after, NULL);

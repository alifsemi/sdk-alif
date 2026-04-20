/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include "test_i2c.h"
LOG_MODULE_REGISTER(alif_i2c_stress, LOG_LEVEL_INF);
#include <zephyr/sys/util.h>

/* Kconfig-driven stress configuration */
#ifdef CONFIG_I2C_ENHANCED_STRESS
#define STRESS_LOOP_COUNT      CONFIG_I2C_STRESS_LOOPS
#define STRESS_TIMEOUT_MS      CONFIG_I2C_STRESS_TIMEOUT
#define STRESS_ERROR_INJECTION CONFIG_I2C_ERROR_INJECTION
#elif defined(CONFIG_I2C_STRESS_TESTS)
#define STRESS_LOOP_COUNT      1000U
#define STRESS_TIMEOUT_MS      300U
#define STRESS_ERROR_INJECTION 0
#endif

#define STRESS_WRITE_LEN_LARGE  64U
#define STRESS_WRITE_LEN_MEDIUM 32U
#define STRESS_WRITE_LEN_SMALL  16U
#define STRESS_READ_LEN         16U
#define STRESS_PARTIAL_READ_LEN 8U

/* Buffer mode aware size definitions */
#ifdef CONFIG_I2C_TARGET_BUFFER_MODE
#define STRESS_WRITE_LEN_BOUNDARY CONFIG_I2C_TAR_DATA_BUF_MAX_LEN
#define STRESS_WRITE_LEN_OVERSIZE (CONFIG_I2C_TAR_DATA_BUF_MAX_LEN + 16U)
#define STRESS_BUFFER_SIZE        CONFIG_I2C_TAR_DATA_BUF_MAX_LEN

/* Ensure driver buffer size does not exceed test buffer capacity */
BUILD_ASSERT(CONFIG_I2C_TAR_DATA_BUF_MAX_LEN <= BUFF_PERF,
	     "CONFIG_I2C_TAR_DATA_BUF_MAX_LEN exceeds BUFF_PERF");
#else
#define STRESS_WRITE_LEN_BOUNDARY BUFF_PERF
#define STRESS_WRITE_LEN_OVERSIZE (BUFF_PERF + 32U)
#define STRESS_BUFFER_SIZE        BUFF_PERF
#endif

/* Enhanced stress context with bug detection metrics */
struct i2c_stress_ctx {
	const struct device *controller_dev;
	const struct device *target_dev;
	bool target_registered;

	uint8_t controller_tx[BUFF_PERF];
	uint8_t controller_rx[BUFF_PERF];
	uint8_t target_tx[BUFF_PERF];
	uint8_t target_rx[BUFF_PERF];

	/* Buffer mode aware tracking */
	uint32_t buffer_size;

	uint32_t target_rx_count;
	uint32_t target_tx_index;

	uint32_t write_requested_count;
	uint32_t write_received_count;
	uint32_t read_requested_count;
	uint32_t read_processed_count;
	uint32_t stop_count;
	uint32_t stop_rx_snapshot;

	bool overflow_guard_hit;

	/* Bug detection metrics */
	uint32_t timing_violations;
	uint32_t data_corruption;
	uint32_t driver_errors;
	uint32_t hardware_errors;

#ifdef CONFIG_I2C_TARGET_BUFFER_MODE
	uint32_t buf_write_received_count;
	uint32_t buf_read_requested_count;
	uint32_t buf_last_write_len;
	uint32_t buf_read_reply_len;
	uint8_t *buf_read_ptr;
#endif
};

static struct i2c_stress_ctx stress_ctx;

#define validate_data_match i2c_validate_data_match

/* Enhanced timing validation for bug detection */
static int validate_timing_bounds(int64_t start_us, int64_t end_us,
				  uint32_t max_expected_us)
{
	int64_t duration = end_us - start_us;

	if (duration > max_expected_us) {
		stress_ctx.timing_violations++;
		LOG_ERR("Timing violation: %lldus (max %uus)",
			duration, max_expected_us);
		return -ETIME;
	}
	return 0;
}

/* Configurable stress loop with error injection */
static __unused int run_stress_loop(const struct device *dev,
				    const uint8_t *tx_data, size_t tx_len,
				    uint8_t *rx_data, size_t rx_len,
				    uint32_t iterations)
{
	int failures = 0;

	for (uint32_t i = 0; i < iterations; i++) {
		int ret;
		int timing_ret;
		int64_t start_us;
		int64_t end_us;

#if STRESS_ERROR_INJECTION
		if ((i % 10U) == 0U) {
			k_usleep(1U);
		}
#endif

		start_us = k_uptime_get() * 1000;

		if ((tx_len > 0U) && (rx_len > 0U)) {
			ret = i2c_write_read(dev, TGT_I2C_ADDR, tx_data, tx_len,
				     rx_data, rx_len);
		} else if (tx_len > 0U) {
			ret = i2c_write(dev, tx_data, tx_len, TGT_I2C_ADDR);
		} else {
			ret = i2c_read(dev, rx_data, rx_len, TGT_I2C_ADDR);
		}

		end_us = k_uptime_get() * 1000;

		if (ret != 0) {
			failures++;
			stress_ctx.driver_errors++;
			LOG_ERR("Stress iteration %u failed: %d", i, ret);
			continue;
		}

		timing_ret = validate_timing_bounds(start_us, end_us,
					    STRESS_TIMEOUT_MS * 1000U);
		if (timing_ret != 0) {
			failures++;
		}

		if ((tx_len > 0U) && (rx_len > 0U)) {
			ret = i2c_validate_data_match(tx_data, rx_data, tx_len,
					"stress loop");
			if (ret != 0) {
				failures++;
				stress_ctx.data_corruption++;
			}
		}
	}

	return failures;
}

/*
 * Generate a deterministic pattern with values <= 0x7F.
 * This helps distinguish real payload from synthetic errno-like bytes
 * (typically high-byte values from signed error casts).
 */
static void fill_pattern_u7(uint8_t *buf, size_t len, uint8_t seed)
{
	for (size_t i = 0; i < len; i++) {
		buf[i] = (uint8_t)((seed + (i * 13U) + (i >> 1U)) & 0x7FU);
	}
}

static void reset_runtime_counters(void)
{
	stress_ctx.target_rx_count = 0U;
	stress_ctx.target_tx_index = 0U;

	stress_ctx.write_requested_count = 0U;
	stress_ctx.write_received_count = 0U;
	stress_ctx.read_requested_count = 0U;
	stress_ctx.read_processed_count = 0U;
	stress_ctx.stop_count = 0U;
	stress_ctx.stop_rx_snapshot = 0U;

	stress_ctx.overflow_guard_hit = false;

	/* Reset error counters to prevent cross-test contamination */
	stress_ctx.driver_errors = 0U;
	stress_ctx.timing_violations = 0U;

#ifdef CONFIG_I2C_TARGET_BUFFER_MODE
	stress_ctx.buf_write_received_count = 0U;
	stress_ctx.buf_read_requested_count = 0U;
	stress_ctx.buf_last_write_len = 0U;
	stress_ctx.buf_read_reply_len = 0U;
	stress_ctx.buf_read_ptr = stress_ctx.target_tx;
#endif

	memset(stress_ctx.target_rx, 0, sizeof(stress_ctx.target_rx));
	memset(stress_ctx.controller_rx, 0, sizeof(stress_ctx.controller_rx));
}

/* Target callback implementations */

static int cb_write_requested(struct i2c_target_config *config)
{
	ARG_UNUSED(config);

	stress_ctx.write_requested_count++;

	/*
	 * Reset only at first write-request of a transfer.
	 * Multiple write_requested() in the same transfer
	 * should be caught by checks, not used to erase
	 * already captured data.
	 */
	if (stress_ctx.write_requested_count == 1U) {
		stress_ctx.target_rx_count = 0U;
	}

	return 0;
}

#ifndef CONFIG_I2C_TARGET_BUFFER_MODE
static int cb_write_received(struct i2c_target_config *config, uint8_t val)
{
	ARG_UNUSED(config);

	if (stress_ctx.target_rx_count < BUFF_PERF) {
		stress_ctx.target_rx[stress_ctx.target_rx_count++] = val;
	} else {
		stress_ctx.overflow_guard_hit = true;
	}

	stress_ctx.write_received_count++;

	return 0;
}

static int cb_read_requested(struct i2c_target_config *config, uint8_t *val)
{
	ARG_UNUSED(config);

	if (stress_ctx.target_tx_index >= BUFF_PERF) {
		stress_ctx.target_tx_index = 0U;
	}

	*val = stress_ctx.target_tx[stress_ctx.target_tx_index++];
	stress_ctx.read_requested_count++;

	return 0;
}

static int cb_read_processed(struct i2c_target_config *config, uint8_t *val)
{
	ARG_UNUSED(config);

	/*
	 * Zephyr I2C target API contract (see include/zephyr/drivers/i2c.h):
	 *   read_requested : invoked ONCE per read phase, supplies the FIRST
	 *                    byte the target sends to the controller.
	 *   read_processed : invoked for every SUBSEQUENT byte the controller
	 *                    ACKs, also through *val.
	 *
	 * This callback therefore MUST fill *val from the target TX buffer,
	 * otherwise a compliant driver will read stale / undefined data after
	 * the first byte.
	 */
	if (stress_ctx.target_tx_index >= BUFF_PERF) {
		stress_ctx.target_tx_index = 0U;
	}

	*val = stress_ctx.target_tx[stress_ctx.target_tx_index++];
	stress_ctx.read_processed_count++;
	return 0;
}
#else
static void cb_buf_write_received(struct i2c_target_config *config,
				    uint8_t *data_buf, uint32_t len)
{
	ARG_UNUSED(config);

	uint32_t accepted_len = MIN(len, stress_ctx.buffer_size);
	uint32_t copy_len = MIN(accepted_len,
				 (uint32_t)ARRAY_SIZE(stress_ctx.target_rx));

	memcpy(stress_ctx.target_rx, data_buf, copy_len);
	stress_ctx.target_rx_count = copy_len;
	stress_ctx.buf_last_write_len = copy_len;
	stress_ctx.buf_write_received_count++;

	if (accepted_len > copy_len) {
		stress_ctx.overflow_guard_hit = true;
	}
}

static int cb_buf_read_requested(struct i2c_target_config *config,
				 uint8_t **ptr, uint32_t *len)
{
	ARG_UNUSED(config);

	*ptr = stress_ctx.target_tx;
	stress_ctx.buf_read_requested_count++;

	if (stress_ctx.buf_read_reply_len == 0U) {
		stress_ctx.buf_read_reply_len =
			MIN(stress_ctx.buffer_size,
		    (uint32_t)CONFIG_I2C_TAR_DATA_BUF_MAX_LEN);
	}

	*len = stress_ctx.buf_read_reply_len;
	return 0;
}
#endif

static int cb_stop(struct i2c_target_config *config)
{
	ARG_UNUSED(config);

	stress_ctx.stop_count++;
	stress_ctx.stop_rx_snapshot = stress_ctx.target_rx_count;
	return 0;
}

static const struct i2c_target_callbacks i2c_t_cb_stress = {
	.write_requested = cb_write_requested,
	.stop = cb_stop,
#ifdef CONFIG_I2C_TARGET_BUFFER_MODE
	.buf_write_received = cb_buf_write_received,
	.buf_read_requested = cb_buf_read_requested,
#else
	.write_received = cb_write_received,
	.read_requested = cb_read_requested,
	.read_processed = cb_read_processed,
#endif
};

static struct i2c_target_config i2c_tcfg_stress = {
	.address = TGT_I2C_ADDR,
	.flags = 0,
	.callbacks = &i2c_t_cb_stress,
};

/* Transfer + registration helpers */

static int register_target_device(void)
{
	int ret;
	uint32_t master_cfg = I2C_MODE_CONTROLLER |
			      I2C_SPEED_SET(I2C_SPEED_FAST);
	uint32_t target_cfg = I2C_SPEED_SET(I2C_SPEED_FAST);

	ret = i2c_configure(stress_ctx.controller_dev, master_cfg);
	if (ret != 0) {
		LOG_ERR("I2C controller configure failed: %d", ret);
		return ret;
	}

	ret = i2c_configure(stress_ctx.target_dev, target_cfg);
	if (ret != 0) {
		LOG_ERR("I2C target configure failed: %d", ret);
		return ret;
	}

	ret = i2c_target_register(stress_ctx.target_dev, &i2c_tcfg_stress);
	if (ret == 0) {
		stress_ctx.target_registered = true;
	}

	return ret;
}

static void unregister_target_device(void)
{
	int ret;

	if (!stress_ctx.target_registered) {
		return;
	}

	ret = i2c_target_unregister(stress_ctx.target_dev, &i2c_tcfg_stress);
	/* ENOTSUP means operation not supported */
	if (ret == -ENOTSUP) {
		LOG_WRN("I2C target mode not supported");
	} else {
		zassert_equal(ret, 0, "i2c_target_unregister failed: %d", ret);
	}

	stress_ctx.target_registered = false;
}

/*
 * Reconfigure the target clock for a new frequency without
 * touching i2c_target_register() — the stress suite installed its own
 * callback table (i2c_tcfg_stress) in i2c_stress_before(), and calling
 * the shared register_target_speed_i2c() here would overwrite it with
 * the common i2c_tcfg and leave every stress_ctx counter at zero.
 */
static void stress_reconfigure_target_speed(uint32_t i2c_cfg)
{
	int ret;

	if (!stress_ctx.target_registered || stress_ctx.target_dev == NULL) {
		return;
	}

	ret = i2c_configure(stress_ctx.target_dev, i2c_cfg);
	zassert_ok(ret, "Target re-configure failed: %d", ret);
}

static int controller_write_transfer(const uint8_t *tx_buf, uint32_t len)
{
	struct i2c_msg msg = {
		.buf = (uint8_t *)tx_buf,
		.len = (uint16_t)len,
		.flags = I2C_MSG_WRITE | I2C_MSG_STOP,
	};

	return i2c_transfer(stress_ctx.controller_dev, &msg, 1, TGT_I2C_ADDR);
}

static int controller_read_transfer(uint8_t *rx_buf, uint32_t len)
{
	struct i2c_msg msg = {
		.buf = rx_buf,
		.len = (uint16_t)len,
		.flags = I2C_MSG_READ | I2C_MSG_STOP,
	};

	return i2c_transfer(stress_ctx.controller_dev, &msg, 1, TGT_I2C_ADDR);
}

static int controller_write_read_transfer(const uint8_t *tx_buf, uint32_t tx_len,
				      uint8_t *rx_buf, uint32_t rx_len)
{
	struct i2c_msg msgs[2] = {
		{
			.buf = (uint8_t *)tx_buf,
			.len = (uint16_t)tx_len,
			.flags = I2C_MSG_WRITE,
		},
		{
			.buf = rx_buf,
			.len = (uint16_t)rx_len,
			.flags = I2C_MSG_READ | I2C_MSG_RESTART | I2C_MSG_STOP,
		},
	};

	return i2c_transfer(stress_ctx.controller_dev, msgs, ARRAY_SIZE(msgs),
			    TGT_I2C_ADDR);
}

/* Test setup/teardown */

static void *i2c_stress_setup(void)
{
	stress_ctx.controller_dev = DEVICE_DT_GET(I2C_CONTROLLER);
	stress_ctx.target_dev = DEVICE_DT_GET(I2C_TARGET);
	stress_ctx.target_registered = false;

	zassert_true(device_is_ready(stress_ctx.controller_dev),
		     "Controller I2C device not ready");
	zassert_true(device_is_ready(stress_ctx.target_dev),
		     "Target I2C device not ready");

	return &stress_ctx;
}

static void i2c_stress_before(void *fixture)
{
	int ret;

	ARG_UNUSED(fixture);

	reset_runtime_counters();

	/* Initialize buffer size based on mode */
	stress_ctx.buffer_size = STRESS_BUFFER_SIZE;

	fill_pattern_u7(stress_ctx.target_tx, BUFF_PERF, 0x21U);

	ret = register_target_device();
	/* ENOTSUP means operation not supported */
	if (ret == -ENOTSUP) {
		ztest_test_skip();
	}

	zassert_equal(ret, 0, "i2c_target_register failed: %d", ret);
}

static void i2c_stress_after(void *fixture)
{
	ARG_UNUSED(fixture);
	unregister_target_device();
}

static void i2c_stress_teardown(void *fixture)
{
	i2c_stress_after(fixture);

	/* Validate no driver errors accumulated across entire stress test suite */
	zassert_equal(stress_ctx.driver_errors, 0U,
		      "Stress test suite accumulated %u driver errors",
		      stress_ctx.driver_errors);
	zassert_equal(stress_ctx.timing_violations, 0U,
		      "Stress test suite accumulated %u timing violations",
		      stress_ctx.timing_violations);
}

/* Core robustness tests */

ZTEST(i2c_stress, test_write_large)
{
	struct i2c_test_freq_desc freqs[I2C_TEST_MAX_FREQS];
	int freq_count;
	int ret;

	i2c_test_skip_if_no_freqs();
	freq_count = i2c_test_get_enabled_freqs(freqs, ARRAY_SIZE(freqs));

	for (int f = 0; f < freq_count; f++) {
		const uint32_t tx_len = STRESS_WRITE_LEN_LARGE;
		uint32_t expected_len;
		int match_ret;

		TC_PRINT("Stress large write at %s (%u Hz)\n",
			 freqs[f].name, freqs[f].freq_hz);

		ret = i2c_test_configure_controller_freq(stress_ctx.controller_dev,
						     &i2c_test_ctx, &freqs[f]);
		if (ret == -ENOTSUP) {
			TC_PRINT("Skipping %s: not supported\n", freqs[f].name);
			continue;
		}
		zassert_ok(ret, "Failed to configure %s: %d", freqs[f].name, ret);

		stress_reconfigure_target_speed(I2C_SPEED_SET(freqs[f].zephyr_speed));

		reset_runtime_counters();
	fill_pattern_u7(stress_ctx.controller_tx, tx_len, 0x35U);

	ret = controller_write_transfer(stress_ctx.controller_tx, tx_len);
	zassert_equal(ret, 0, "Controller write transfer failed: %d", ret);
	zassert_equal(stress_ctx.write_requested_count, 1U,
		      "write_requested count %u",
		      stress_ctx.write_requested_count);
	zassert_equal(stress_ctx.stop_count, 1U, "stop callback count %u",
		      stress_ctx.stop_count);
	zassert_false(stress_ctx.overflow_guard_hit,
		      "Target RX overflow guard was triggered");

#ifdef CONFIG_I2C_TARGET_BUFFER_MODE
	expected_len = MIN(tx_len, stress_ctx.buffer_size);
	zassert_equal(stress_ctx.buf_write_received_count, 1U,
		     "buf_write_received count %u",
		      stress_ctx.buf_write_received_count);
	zassert_equal(stress_ctx.buf_last_write_len, expected_len,
		      "buf_last_write_len %u, expected %u",
		      stress_ctx.buf_last_write_len,
		      expected_len);
#else
	expected_len = tx_len;
	zassert_equal(stress_ctx.write_received_count, expected_len,
		      "write_received count %u, expected %u",
		      stress_ctx.write_received_count,
		      expected_len);
	zassert_equal(stress_ctx.stop_rx_snapshot, expected_len,
		      "stop saw %u bytes, expected %u",
		      stress_ctx.stop_rx_snapshot, expected_len);
#endif

	zassert_equal(stress_ctx.target_rx_count, expected_len,
		      "target_rx_count %u, expected %u",
		      stress_ctx.target_rx_count, expected_len);
	match_ret = validate_data_match(stress_ctx.controller_tx,
					stress_ctx.target_rx,
					expected_len,
					"Large Controller TX -> Target RX");
	zassert_equal(match_ret, 0, "large write payload mismatch");
	}
}

ZTEST(i2c_stress, test_write_byte_exact)
{
	struct i2c_test_freq_desc freqs[I2C_TEST_MAX_FREQS];
	int freq_count;
	int ret;

	i2c_test_skip_if_no_freqs();
	freq_count = i2c_test_get_enabled_freqs(freqs, ARRAY_SIZE(freqs));

	for (int f = 0; f < freq_count; f++) {
		const uint32_t tx_len = 1U;
		uint32_t expected_len;
		int match_ret;

		TC_PRINT("Stress single byte at %s (%u Hz)\n",
			 freqs[f].name, freqs[f].freq_hz);

		ret = i2c_test_configure_controller_freq(stress_ctx.controller_dev,
						     &i2c_test_ctx, &freqs[f]);
		if (ret == -ENOTSUP) {
			TC_PRINT("Skipping %s: not supported\n", freqs[f].name);
			continue;
		}
		zassert_ok(ret, "Failed to configure %s: %d", freqs[f].name, ret);

		stress_reconfigure_target_speed(I2C_SPEED_SET(freqs[f].zephyr_speed));

		reset_runtime_counters();
	fill_pattern_u7(stress_ctx.controller_tx, tx_len, 0x22U);

	ret = controller_write_transfer(stress_ctx.controller_tx, tx_len);
	zassert_equal(ret, 0, "Controller write transfer failed: %d", ret);
	zassert_equal(stress_ctx.write_requested_count, 1U,
		      "write_requested count %u",
		      stress_ctx.write_requested_count);
	zassert_equal(stress_ctx.stop_count, 1U, "stop count %u",
		      stress_ctx.stop_count);

#ifdef CONFIG_I2C_TARGET_BUFFER_MODE
	expected_len = MIN(tx_len, (uint32_t)CONFIG_I2C_TAR_DATA_BUF_MAX_LEN);
	zassert_equal(stress_ctx.buf_write_received_count, 1U,
		     "buf_write_received count %u",
		      stress_ctx.buf_write_received_count);
#else
	expected_len = tx_len;
	zassert_equal(stress_ctx.write_received_count, expected_len,
		      "write_received count %u, expected %u",
		      stress_ctx.write_received_count,
		      expected_len);
#endif

	zassert_equal(stress_ctx.target_rx_count, expected_len,
		      "target_rx_count %u, expected %u",
		      stress_ctx.target_rx_count, expected_len);
	match_ret = validate_data_match(stress_ctx.controller_tx,
					stress_ctx.target_rx,
					expected_len,
					"Single-byte Controller TX -> Target RX");
	zassert_equal(match_ret, 0, "single-byte write payload mismatch");
	}
}

ZTEST(i2c_stress, test_write_len_sweep)
{
	struct i2c_test_freq_desc freqs[I2C_TEST_MAX_FREQS];
	int freq_count;
	int ret;

	i2c_test_skip_if_no_freqs();
	freq_count = i2c_test_get_enabled_freqs(freqs, ARRAY_SIZE(freqs));

	for (int f = 0; f < freq_count; f++) {
		static const uint8_t len_cases[] = {1U, 2U, 3U, 4U, 7U, 8U, 15U,
					    16U, 31U, 32U, 63U, 64U};

		TC_PRINT("Stress len sweep at %s (%u Hz)\n",
			 freqs[f].name, freqs[f].freq_hz);

		ret = i2c_test_configure_controller_freq(stress_ctx.controller_dev,
						     &i2c_test_ctx, &freqs[f]);
		if (ret == -ENOTSUP) {
			TC_PRINT("Skipping %s: not supported\n", freqs[f].name);
			continue;
		}
		zassert_ok(ret, "Failed to configure %s: %d", freqs[f].name, ret);

		stress_reconfigure_target_speed(I2C_SPEED_SET(freqs[f].zephyr_speed));

		for (uint32_t i = 0U; i < ARRAY_SIZE(len_cases); i++) {
			const uint32_t tx_len = len_cases[i];
			uint32_t expected_len;
			int match_ret;

			reset_runtime_counters();
		fill_pattern_u7(stress_ctx.controller_tx, tx_len,
				(uint8_t)(0x22U + i));

		ret = controller_write_transfer(stress_ctx.controller_tx, tx_len);
		zassert_equal(ret, 0,
			     "Case %u (len=%u): write failed: %d", i,
			     tx_len, ret);
		zassert_equal(stress_ctx.write_requested_count, 1U,
			      "Case %u (len=%u): write_requested count %u",
			       i, tx_len, stress_ctx.write_requested_count);
		zassert_equal(stress_ctx.stop_count, 1U,
			      "Case %u (len=%u): stop count %u",
			       i, tx_len, stress_ctx.stop_count);
		zassert_false(stress_ctx.overflow_guard_hit,
			      "Case %u (len=%u): overflow guard triggered",
			       i, tx_len);

#ifdef CONFIG_I2C_TARGET_BUFFER_MODE
		expected_len = MIN(tx_len,
				   (uint32_t)CONFIG_I2C_TAR_DATA_BUF_MAX_LEN);
		zassert_equal(stress_ctx.buf_write_received_count, 1U,
			      "Case %u (len=%u): buf_write count %u", i, tx_len,
			      stress_ctx.buf_write_received_count);
		zassert_equal(stress_ctx.buf_last_write_len, expected_len,
			      "Case %u (len=%u): buf_last_write_len %u",
			      i, tx_len, stress_ctx.buf_last_write_len);
#else
		expected_len = tx_len;
		zassert_equal(stress_ctx.write_received_count, expected_len,
			      "Case %u (len=%u): write_received count %u",
			      i, tx_len, stress_ctx.write_received_count);
		zassert_equal(stress_ctx.stop_rx_snapshot, expected_len,
			      "Case %u (len=%u): stop saw %u bytes", i, tx_len,
			      stress_ctx.stop_rx_snapshot);
#endif

		zassert_equal(stress_ctx.target_rx_count, expected_len,
			      "Case %u (len=%u): target_rx_count %u", i, tx_len,
			      stress_ctx.target_rx_count);
		match_ret =
			validate_data_match(stress_ctx.controller_tx,
					    stress_ctx.target_rx,
					    expected_len,
					    "Len-sweep Controller TX -> Target RX");
		zassert_equal(match_ret, 0,
			      "Case %u (len=%u): payload mismatch",
			      i, tx_len);
		}
	}
}

ZTEST(i2c_stress, test_write_repeat_100x32)
{
	struct i2c_test_freq_desc freqs[I2C_TEST_MAX_FREQS];
	int freq_count;
	int ret;

	i2c_test_skip_if_no_freqs();
	freq_count = i2c_test_get_enabled_freqs(freqs, ARRAY_SIZE(freqs));

	for (int f = 0; f < freq_count; f++) {
		const uint32_t tx_len = STRESS_WRITE_LEN_MEDIUM;
		uint32_t expected_len;
		int match_ret;

		TC_PRINT("Stress 100x32 at %s (%u Hz)\n",
			 freqs[f].name, freqs[f].freq_hz);

		ret = i2c_test_configure_controller_freq(stress_ctx.controller_dev,
						     &i2c_test_ctx, &freqs[f]);
		if (ret == -ENOTSUP) {
			TC_PRINT("Skipping %s: not supported\n", freqs[f].name);
			continue;
		}
		zassert_ok(ret, "Failed to configure %s: %d", freqs[f].name, ret);

		stress_reconfigure_target_speed(I2C_SPEED_SET(freqs[f].zephyr_speed));

		reset_runtime_counters();
		fill_pattern_u7(stress_ctx.controller_tx, tx_len, 0x10U);

		ret = controller_write_transfer(stress_ctx.controller_tx, tx_len);
		zassert_equal(ret, 0, "Controller write transfer failed: %d", ret);
		zassert_equal(stress_ctx.write_requested_count, 1U,
			      "write_requested count %u",
			      stress_ctx.write_requested_count);
		zassert_equal(stress_ctx.stop_count, 1U, "stop count %u",
			      stress_ctx.stop_count);

#ifdef CONFIG_I2C_TARGET_BUFFER_MODE
		expected_len = MIN(tx_len, (uint32_t)CONFIG_I2C_TAR_DATA_BUF_MAX_LEN);
		zassert_equal(stress_ctx.buf_write_received_count, 1U,
			      "buf_write_received count %u",
			      stress_ctx.buf_write_received_count);
		zassert_equal(stress_ctx.buf_last_write_len, expected_len,
			      "buf_last_write_len %u, expected %u",
			      stress_ctx.buf_last_write_len,
			      expected_len);
#else
		expected_len = tx_len;
		zassert_equal(stress_ctx.write_received_count, expected_len,
			      "write_received count %u, expected %u",
			      stress_ctx.write_received_count,
			      expected_len);
#endif

		zassert_equal(stress_ctx.target_rx_count, expected_len,
			      "target_rx_count %u, expected %u",
			      stress_ctx.target_rx_count, expected_len);

		match_ret = validate_data_match(stress_ctx.controller_tx,
					stress_ctx.target_rx,
					expected_len,
					"Controller TX -> Target RX");
		zassert_equal(match_ret, 0, "write payload mismatch");

		for (uint32_t i = 1U; i < STRESS_LOOP_COUNT; i++) {
			reset_runtime_counters();
			fill_pattern_u7(stress_ctx.controller_tx, tx_len,
					(uint8_t)(0x10U + i));

			ret = controller_write_transfer(stress_ctx.controller_tx, tx_len);
			zassert_equal(ret, 0,
				      "Iter %u: controller write failed: %d", i, ret);
			zassert_equal(stress_ctx.write_requested_count, 1U,
				      "Iter %u: write_requested count %u", i,
				      stress_ctx.write_requested_count);
			zassert_equal(stress_ctx.stop_count, 1U,
				      "Iter %u: stop count %u", i, stress_ctx.stop_count);

#ifdef CONFIG_I2C_TARGET_BUFFER_MODE
			zassert_equal(stress_ctx.buf_write_received_count, 1U,
				      "Iter %u: buf_write_received count %u", i,
				      stress_ctx.buf_write_received_count);
			zassert_equal(stress_ctx.buf_last_write_len, expected_len,
				      "Iter %u: buf_write len %u, exp %u", i,
				      stress_ctx.buf_last_write_len,
				      expected_len);
#else
			zassert_equal(stress_ctx.write_received_count, expected_len,
				      "Iter %u: write_received %u, exp %u", i,
				      stress_ctx.write_received_count, expected_len);
#endif

			zassert_equal(stress_ctx.target_rx_count, expected_len,
				      "Iter %u: target_rx_count %u, expected %u", i,
				      stress_ctx.target_rx_count, expected_len);

			match_ret = validate_data_match(stress_ctx.controller_tx,
							stress_ctx.target_rx,
							expected_len,
							"Controller TX -> Target RX");
			zassert_equal(match_ret, 0,
				      "Iter %u: payload mismatch", i);
		}
	}
}

ZTEST(i2c_stress, test_write_boundary_128)
{
	struct i2c_test_freq_desc freqs[I2C_TEST_MAX_FREQS];
	int freq_count;
	int ret;

	i2c_test_skip_if_no_freqs();
	freq_count = i2c_test_get_enabled_freqs(freqs, ARRAY_SIZE(freqs));

	for (int f = 0; f < freq_count; f++) {
		const uint32_t tx_len = STRESS_WRITE_LEN_BOUNDARY;
		uint32_t expected_len;
		int match_ret;

		TC_PRINT("Stress boundary at %s (%u Hz)\n",
			 freqs[f].name, freqs[f].freq_hz);

		ret = i2c_test_configure_controller_freq(stress_ctx.controller_dev,
						     &i2c_test_ctx, &freqs[f]);
		if (ret == -ENOTSUP) {
			TC_PRINT("Skipping %s: not supported\n", freqs[f].name);
			continue;
		}
		zassert_ok(ret, "Failed to configure %s: %d", freqs[f].name, ret);

		stress_reconfigure_target_speed(I2C_SPEED_SET(freqs[f].zephyr_speed));

		reset_runtime_counters();
		fill_pattern_u7(stress_ctx.controller_tx, tx_len, 0x2AU);

		ret = controller_write_transfer(stress_ctx.controller_tx, tx_len);
		zassert_equal(ret, 0, "Controller write transfer failed: %d", ret);
		zassert_equal(stress_ctx.write_requested_count, 1U,
			      "write_requested called %u times, expected 1",
			      stress_ctx.write_requested_count);
		zassert_equal(stress_ctx.stop_count, 1U,
			      "stop callback count %u, expected 1",
			      stress_ctx.stop_count);

#ifdef CONFIG_I2C_TARGET_BUFFER_MODE
		expected_len = MIN(tx_len, (uint32_t)CONFIG_I2C_TAR_DATA_BUF_MAX_LEN);
		zassert_equal(stress_ctx.buf_write_received_count, 1U,
			      "buf_write_received called %u times, expected 1",
			      stress_ctx.buf_write_received_count);
		zassert_equal(stress_ctx.buf_last_write_len, expected_len,
			      "buf_write_received len %u, expected %u",
			      stress_ctx.buf_last_write_len,
			      expected_len);
#else
		expected_len = tx_len;
		zassert_equal(stress_ctx.write_received_count, expected_len,
			      "write_received count %u, expected %u",
			      stress_ctx.write_received_count, expected_len);
		zassert_equal(stress_ctx.stop_rx_snapshot, expected_len,
			      "stop callback saw %u bytes, expected %u",
			      stress_ctx.stop_rx_snapshot, expected_len);
#endif

		zassert_equal(stress_ctx.target_rx_count, expected_len,
			      "target_rx_count %u, expected %u",
			      stress_ctx.target_rx_count, expected_len);

		match_ret = validate_data_match(stress_ctx.controller_tx,
				stress_ctx.target_rx, expected_len,
						"Boundary Controller TX -> Target RX");
		zassert_equal(match_ret, 0, "Boundary write payload mismatch");
	}
}

#ifndef CONFIG_I2C_TARGET_BUFFER_MODE
ZTEST(i2c_stress, test_write_oversize)
{
	struct i2c_test_freq_desc freqs[I2C_TEST_MAX_FREQS];
	int freq_count;
	int ret;

	i2c_test_skip_if_no_freqs();
	freq_count = i2c_test_get_enabled_freqs(freqs, ARRAY_SIZE(freqs));

	for (int f = 0; f < freq_count; f++) {
		static uint8_t tx_oversize[STRESS_WRITE_LEN_OVERSIZE];
		const uint32_t tx_len = STRESS_WRITE_LEN_OVERSIZE;
		int match_ret;

		TC_PRINT("Stress oversize at %s (%u Hz)\n",
			 freqs[f].name, freqs[f].freq_hz);

		ret = i2c_test_configure_controller_freq(stress_ctx.controller_dev,
						     &i2c_test_ctx, &freqs[f]);
		if (ret == -ENOTSUP) {
			TC_PRINT("Skipping %s: not supported\n", freqs[f].name);
			continue;
		}
		zassert_ok(ret, "Failed to configure %s: %d", freqs[f].name, ret);

		stress_reconfigure_target_speed(I2C_SPEED_SET(freqs[f].zephyr_speed));

		reset_runtime_counters();
		fill_pattern_u7(tx_oversize, tx_len, 0x33U);

		ret = controller_write_transfer(tx_oversize, tx_len);
		zassert_equal(ret, 0, "Controller write transfer failed: %d", ret);
		zassert_equal(stress_ctx.write_requested_count, 1U,
			      "write_requested called %u times, expected 1",
			      stress_ctx.write_requested_count);
		zassert_equal(stress_ctx.write_received_count, tx_len,
			      "write_received count %u, expected %u",
			      stress_ctx.write_received_count, tx_len);
		zassert_equal(stress_ctx.target_rx_count, BUFF_PERF,
			      "target_rx_count %u, expected %u",
			      stress_ctx.target_rx_count, BUFF_PERF);
		zassert_equal(stress_ctx.stop_count, 1U,
			      "stop callback count %u, expected 1",
			      stress_ctx.stop_count);
		zassert_equal(stress_ctx.stop_rx_snapshot, BUFF_PERF,
			      "stop callback saw %u bytes, expected %u",
			      stress_ctx.stop_rx_snapshot, BUFF_PERF);
		zassert_true(stress_ctx.overflow_guard_hit,
			     "Expected overflow_guard_hit for oversize write");

		match_ret = validate_data_match(tx_oversize,
						stress_ctx.target_rx,
						BUFF_PERF,
						"Oversize Mst TX -> Tar RX (clamped)");
		zassert_equal(match_ret, 0,
			      "Oversize write payload mismatch in captured window");
	}
}

ZTEST(i2c_stress, test_read_repeat_100)
{
	struct i2c_test_freq_desc freqs[I2C_TEST_MAX_FREQS];
	int freq_count;
	int ret;

	i2c_test_skip_if_no_freqs();
	freq_count = i2c_test_get_enabled_freqs(freqs, ARRAY_SIZE(freqs));

	for (int f = 0; f < freq_count; f++) {
		const uint32_t rx_len = STRESS_READ_LEN;

		TC_PRINT("Stress read 100x at %s (%u Hz)\n",
			 freqs[f].name, freqs[f].freq_hz);

		ret = i2c_test_configure_controller_freq(stress_ctx.controller_dev,
						     &i2c_test_ctx, &freqs[f]);
		if (ret == -ENOTSUP) {
			TC_PRINT("Skipping %s: not supported\n", freqs[f].name);
			continue;
		}
		zassert_ok(ret, "Failed to configure %s: %d", freqs[f].name, ret);

		stress_reconfigure_target_speed(I2C_SPEED_SET(freqs[f].zephyr_speed));

		for (uint32_t i = 0U; i < STRESS_LOOP_COUNT; i++) {
			int match_ret;

			reset_runtime_counters();
			fill_pattern_u7(stress_ctx.target_tx, BUFF_PERF,
					(uint8_t)(0x40U + i));

			ret = controller_read_transfer(stress_ctx.controller_rx, rx_len);
			zassert_equal(ret, 0,
				      "Iter %u: controller read failed: %d", i, ret);
			/*
			 * Zephyr I2C target API: read_requested fires exactly
			 * once per read phase; read_processed fires for every
			 * subsequent byte. Some drivers also fire
			 * read_processed for the final byte that the controller
			 * NACKs, so accept >= rx_len - 1.
			 */
			zassert_equal(stress_ctx.read_requested_count, 1U,
				      "Iter %u: read_requested %u, exp 1", i,
				      stress_ctx.read_requested_count);
			zassert_true(stress_ctx.read_processed_count >= rx_len - 1U,
				     "Iter %u: read_processed %u, exp >=%u", i,
				     stress_ctx.read_processed_count, rx_len - 1U);
			zassert_equal(stress_ctx.stop_count, 1U,
				      "Iter %u: stop count %u", i, stress_ctx.stop_count);

			match_ret = validate_data_match(stress_ctx.target_tx,
				stress_ctx.controller_rx, rx_len,
							"Target TX -> Controller RX");
			zassert_equal(match_ret, 0,
				      "Iter %u: read payload mismatch", i);
		}
	}
}

ZTEST(i2c_stress, test_wr_callback_bal)
{
	struct i2c_test_freq_desc freqs[I2C_TEST_MAX_FREQS];
	int freq_count;
	int ret;

	i2c_test_skip_if_no_freqs();
	freq_count = i2c_test_get_enabled_freqs(freqs, ARRAY_SIZE(freqs));

	for (int f = 0; f < freq_count; f++) {
		const uint32_t tx_len = STRESS_WRITE_LEN_SMALL;
		const uint32_t rx_len = STRESS_READ_LEN;
		int match_ret;

		TC_PRINT("Stress WR callback at %s (%u Hz)\n",
			 freqs[f].name, freqs[f].freq_hz);

		ret = i2c_test_configure_controller_freq(stress_ctx.controller_dev,
						     &i2c_test_ctx, &freqs[f]);
		if (ret == -ENOTSUP) {
			TC_PRINT("Skipping %s: not supported\n", freqs[f].name);
			continue;
		}
		zassert_ok(ret, "Failed to configure %s: %d", freqs[f].name, ret);

		stress_reconfigure_target_speed(I2C_SPEED_SET(freqs[f].zephyr_speed));

		reset_runtime_counters();
		fill_pattern_u7(stress_ctx.controller_tx, tx_len, 0x44U);
		fill_pattern_u7(stress_ctx.target_tx, BUFF_PERF, 0x55U);

		ret = controller_write_transfer(stress_ctx.controller_tx, tx_len);
		zassert_equal(ret, 0, "Write transfer failed: %d", ret);
		zassert_equal(stress_ctx.write_requested_count, 1U,
			      "write_requested count %u, expected 1",
			      stress_ctx.write_requested_count);
		zassert_equal(stress_ctx.write_received_count, tx_len,
			      "write_received count %u, expected %u",
			      stress_ctx.write_received_count, tx_len);
		zassert_equal(stress_ctx.read_requested_count, 0U,
			      "read_requested count %u, expected 0",
			      stress_ctx.read_requested_count);
		zassert_equal(stress_ctx.stop_count, 1U, "stop count %u, expected 1",
			      stress_ctx.stop_count);

		ret = controller_read_transfer(stress_ctx.controller_rx, rx_len);
		zassert_equal(ret, 0, "Read transfer failed: %d", ret);
		zassert_equal(stress_ctx.write_requested_count, 1U,
			      "write_requested count changed to %u during read",
			      stress_ctx.write_requested_count);
		zassert_equal(stress_ctx.write_received_count, tx_len,
			      "write_received count changed to %u during read",
			      stress_ctx.write_received_count);
		/*
		 * Zephyr I2C target API: read_requested is invoked once per
		 * read phase, read_processed for each byte after that. Allow
		 * read_processed to optionally fire for the NACK'd last byte
		 * (driver-dependent), hence the >= rx_len - 1 bound.
		 */
		zassert_equal(stress_ctx.read_requested_count, 1U,
			      "read_requested count %u, expected 1",
			      stress_ctx.read_requested_count);
		zassert_true(stress_ctx.read_processed_count >= rx_len - 1U,
			     "read_processed count %u, expected >=%u",
			     stress_ctx.read_processed_count, rx_len - 1U);
		zassert_equal(stress_ctx.stop_count, 2U, "stop count %u, expected 2",
			      stress_ctx.stop_count);

		match_ret = validate_data_match(stress_ctx.controller_tx,
						stress_ctx.target_rx, tx_len,
						"Write phase Controller TX -> Target RX");
		zassert_equal(match_ret, 0, "Write phase payload mismatch");

		match_ret = validate_data_match(stress_ctx.target_tx,
						stress_ctx.controller_rx, rx_len,
						"Read phase Target TX -> Controller RX");
		zassert_equal(match_ret, 0, "Read phase payload mismatch");
	}
}
#else
ZTEST(i2c_stress, test_buf_write_max)
{
	struct i2c_test_freq_desc freqs[I2C_TEST_MAX_FREQS];
	int freq_count;
	int ret;

	i2c_test_skip_if_no_freqs();
	freq_count = i2c_test_get_enabled_freqs(freqs, ARRAY_SIZE(freqs));

	for (int f = 0; f < freq_count; f++) {
		const uint32_t tx_len =
			MIN((uint32_t)BUFF_PERF,
			    (uint32_t)CONFIG_I2C_TAR_DATA_BUF_MAX_LEN + 16U);
		const uint32_t expected_len = MIN(tx_len,
					    (uint32_t)CONFIG_I2C_TAR_DATA_BUF_MAX_LEN);
		int match_ret;

		TC_PRINT("Stress buf write max at %s (%u Hz)\n",
			 freqs[f].name, freqs[f].freq_hz);

		ret = i2c_test_configure_controller_freq(stress_ctx.controller_dev,
						     &i2c_test_ctx, &freqs[f]);
		if (ret == -ENOTSUP) {
			TC_PRINT("Skipping %s: not supported\n", freqs[f].name);
			continue;
		}
		zassert_ok(ret, "Failed to configure %s: %d", freqs[f].name, ret);

		stress_reconfigure_target_speed(I2C_SPEED_SET(freqs[f].zephyr_speed));

		reset_runtime_counters();
		fill_pattern_u7(stress_ctx.controller_tx, tx_len, 0x52U);

		ret = controller_write_transfer(stress_ctx.controller_tx, tx_len);
		zassert_equal(ret, 0, "Controller write over-max transfer failed: %d", ret);
		zassert_equal(stress_ctx.buf_write_received_count, 1U,
			      "buf_write_received called %u times",
			      stress_ctx.buf_write_received_count);
		zassert_equal(stress_ctx.buf_last_write_len, expected_len,
			      "buf_write_received len %u, expected %u",
			      stress_ctx.buf_last_write_len,
			      expected_len);
		zassert_equal(stress_ctx.target_rx_count, expected_len,
			      "target_rx_count %u, expected %u",
			      stress_ctx.target_rx_count, expected_len);
		zassert_false(stress_ctx.overflow_guard_hit,
			      "Target RX overflow guard was triggered");

		match_ret = validate_data_match(stress_ctx.controller_tx,
						stress_ctx.target_rx, expected_len,
						"Controller TX -> Target RX");
		zassert_equal(match_ret, 0, "Truncated buffer-mode payload mismatch");
	}
}

ZTEST(i2c_stress, test_buf_read_partial)
{
	struct i2c_test_freq_desc freqs[I2C_TEST_MAX_FREQS];
	int freq_count;
	int ret;

	i2c_test_skip_if_no_freqs();
	freq_count = i2c_test_get_enabled_freqs(freqs, ARRAY_SIZE(freqs));

	for (int f = 0; f < freq_count; f++) {
		uint8_t rx_first[STRESS_PARTIAL_READ_LEN];
		uint8_t rx_second[STRESS_PARTIAL_READ_LEN];
		int match_ret;

		TC_PRINT("Stress buf read partial at %s (%u Hz)\n",
			 freqs[f].name, freqs[f].freq_hz);

		ret = i2c_test_configure_controller_freq(stress_ctx.controller_dev,
						     &i2c_test_ctx, &freqs[f]);
		if (ret == -ENOTSUP) {
			TC_PRINT("Skipping %s: not supported\n", freqs[f].name);
			continue;
		}
		zassert_ok(ret, "Failed to configure %s: %d", freqs[f].name, ret);

		stress_reconfigure_target_speed(I2C_SPEED_SET(freqs[f].zephyr_speed));

		/* Deliberately provide a larger target buffer than read length. */
		reset_runtime_counters();
		fill_pattern_u7(stress_ctx.target_tx, BUFF_PERF, 0x63U);
		stress_ctx.buf_read_reply_len =
			MIN((uint32_t)BUFF_PERF,
			    (uint32_t)CONFIG_I2C_TAR_DATA_BUF_MAX_LEN);
		stress_ctx.buf_read_ptr = stress_ctx.target_tx;

		ret = controller_read_transfer(rx_first, STRESS_PARTIAL_READ_LEN);
		zassert_equal(ret, 0, "First partial read failed: %d", ret);

		ret = controller_read_transfer(rx_second, STRESS_PARTIAL_READ_LEN);
		zassert_equal(ret, 0, "Second partial read failed: %d", ret);

		/*
		 * Robust expectation:
		 * each independent read transaction should trigger
		 * buf_read_requested().
		 */
		zassert_equal(stress_ctx.buf_read_requested_count, 2U,
			      "buf_read_requested called %u times, expected 2",
			      stress_ctx.buf_read_requested_count);
		zassert_equal(stress_ctx.stop_count, 2U,
			      "stop callback count %u, expected 2",
			      stress_ctx.stop_count);

		match_ret = validate_data_match(stress_ctx.target_tx, rx_first,
						STRESS_PARTIAL_READ_LEN,
						"Target TX -> First RX");
		zassert_equal(match_ret, 0, "First partial read payload mismatch");

		match_ret = validate_data_match(stress_ctx.target_tx, rx_second,
						STRESS_PARTIAL_READ_LEN,
						"Target TX -> Second RX");
		zassert_equal(match_ret, 0, "Second partial read payload mismatch");
	}
}

ZTEST(i2c_stress, test_buf_write_reset)
{
	struct i2c_test_freq_desc freqs[I2C_TEST_MAX_FREQS];
	int freq_count;
	int ret;

	i2c_test_skip_if_no_freqs();
	freq_count = i2c_test_get_enabled_freqs(freqs, ARRAY_SIZE(freqs));

	for (int f = 0; f < freq_count; f++) {
		const uint32_t tx_len =
			MIN(STRESS_WRITE_LEN_SMALL,
			    (uint32_t)CONFIG_I2C_TAR_DATA_BUF_MAX_LEN);
		int match_ret;

		TC_PRINT("Stress buf write reset at %s (%u Hz)\n",
			 freqs[f].name, freqs[f].freq_hz);

		ret = i2c_test_configure_controller_freq(stress_ctx.controller_dev,
						     &i2c_test_ctx, &freqs[f]);
		if (ret == -ENOTSUP) {
			TC_PRINT("Skipping %s: not supported\n", freqs[f].name);
			continue;
		}
		zassert_ok(ret, "Failed to configure %s: %d", freqs[f].name, ret);

		stress_reconfigure_target_speed(I2C_SPEED_SET(freqs[f].zephyr_speed));

		reset_runtime_counters();
		fill_pattern_u7(stress_ctx.controller_tx, tx_len, 0x26U);

		ret = controller_write_transfer(stress_ctx.controller_tx, tx_len);
		zassert_equal(ret, 0, "First buffer-mode write failed: %d", ret);
		zassert_equal(stress_ctx.write_requested_count, 1U,
			      "write_requested count %u, expected 1",
			      stress_ctx.write_requested_count);
		zassert_equal(stress_ctx.buf_write_received_count, 1U,
			      "buf_write_received count %u, expected 1",
			      stress_ctx.buf_write_received_count);
		zassert_equal(stress_ctx.stop_count, 1U, "stop count %u, expected 1",
			      stress_ctx.stop_count);

		fill_pattern_u7(stress_ctx.controller_tx, tx_len, 0x6AU);
		ret = controller_write_transfer(stress_ctx.controller_tx, tx_len);
		zassert_equal(ret, 0, "Second buffer-mode write failed: %d", ret);
		zassert_equal(stress_ctx.write_requested_count, 2U,
			      "write_requested count %u, expected 2",
			      stress_ctx.write_requested_count);
		zassert_equal(stress_ctx.buf_write_received_count, 2U,
			      "buf_write_received count %u, expected 2",
			      stress_ctx.buf_write_received_count);
		zassert_equal(stress_ctx.stop_count, 2U, "stop count %u, expected 2",
			      stress_ctx.stop_count);
		zassert_equal(stress_ctx.buf_last_write_len, tx_len,
			      "buf_last_write_len %u, expected %u",
			      stress_ctx.buf_last_write_len, tx_len);
		zassert_equal(stress_ctx.target_rx_count, tx_len,
			      "target_rx_count %u, expected %u",
			      stress_ctx.target_rx_count, tx_len);

		match_ret = validate_data_match(stress_ctx.controller_tx,
						stress_ctx.target_rx, tx_len,
						"Second write Controller TX -> Target RX");
		zassert_equal(match_ret, 0,
			      "Second buffer-mode write payload mismatch");
	}
}

#endif

ZTEST(i2c_stress, test_wr_restart_integrity)
{
	struct i2c_test_freq_desc freqs[I2C_TEST_MAX_FREQS];
	int freq_count;
	int ret;

	i2c_test_skip_if_no_freqs();
	freq_count = i2c_test_get_enabled_freqs(freqs, ARRAY_SIZE(freqs));

	for (int f = 0; f < freq_count; f++) {
		const uint32_t tx_len = STRESS_PARTIAL_READ_LEN;
		const uint32_t rx_len = STRESS_READ_LEN;
		uint32_t expected_write_len;
		int match_ret;

		TC_PRINT("Stress WR restart at %s (%u Hz)\n",
			 freqs[f].name, freqs[f].freq_hz);

		ret = i2c_test_configure_controller_freq(stress_ctx.controller_dev,
						     &i2c_test_ctx, &freqs[f]);
		if (ret == -ENOTSUP) {
			TC_PRINT("Skipping %s: not supported\n", freqs[f].name);
			continue;
		}
		zassert_ok(ret, "Failed to configure %s: %d", freqs[f].name, ret);

		stress_reconfigure_target_speed(I2C_SPEED_SET(freqs[f].zephyr_speed));

		reset_runtime_counters();
		fill_pattern_u7(stress_ctx.controller_tx, tx_len, 0x4BU);
		fill_pattern_u7(stress_ctx.target_tx, BUFF_PERF, 0x5CU);

		TC_PRINT("=== RESTART CONDITION CHECK ===\n");
		ret = controller_write_read_transfer(stress_ctx.controller_tx, tx_len,
						stress_ctx.controller_rx,
						 rx_len);

		TC_PRINT("transfer returned %d\n", ret);
		zassert_equal(ret, 0,
			      "Write-read repeated-start transfer failed: %d", ret);

		/* Validate repeated-start write phase before comparing payload. */
#ifdef CONFIG_I2C_TARGET_BUFFER_MODE
		expected_write_len = MIN(tx_len,
				       (uint32_t)CONFIG_I2C_TAR_DATA_BUF_MAX_LEN);
		zassert_equal(stress_ctx.buf_write_received_count, 1U,
			      "buf_write %u, exp 1 - missed restart",
			      stress_ctx.buf_write_received_count);
		zassert_equal(stress_ctx.buf_last_write_len, expected_write_len,
			      "buf_write_received len %u, expected %u",
			      stress_ctx.buf_last_write_len,
			      expected_write_len);
#else
		expected_write_len = tx_len;
		zassert_equal(stress_ctx.write_requested_count, 1U,
			      "write_requested count %u, expected 1",
			      stress_ctx.write_requested_count);
		zassert_equal(stress_ctx.write_received_count, expected_write_len,
			      "write_received %u, exp %u - missed restart",
			      stress_ctx.write_received_count, expected_write_len);
		/*
		 * Zephyr I2C target API: read_processed fires for every byte
		 * after the first one delivered via read_requested. Allow
		 * read_processed to optionally fire for the NACK'd last byte
		 * (driver-dependent), hence the >= rx_len - 1 bound.
		 */
		zassert_true(stress_ctx.read_processed_count >= rx_len - 1U,
			     "read_processed count %u, expected >=%u",
			     stress_ctx.read_processed_count, rx_len - 1U);
		zassert_equal(stress_ctx.stop_rx_snapshot, expected_write_len,
			      "stop callback saw %u bytes, expected %u",
			      stress_ctx.stop_rx_snapshot, expected_write_len);
#endif

		zassert_equal(stress_ctx.target_rx_count, expected_write_len,
			      "target_rx_count %u, expected %u",
			      stress_ctx.target_rx_count, expected_write_len);

		match_ret = validate_data_match(stress_ctx.controller_tx,
						stress_ctx.target_rx,
						expected_write_len,
						"Restart write phase data integrity");
		zassert_equal(match_ret, 0,
			      "Restart write phase data integrity failed");

		match_ret = validate_data_match(stress_ctx.target_tx,
						stress_ctx.controller_rx, rx_len,
						"Restart read phase data integrity");
		zassert_equal(match_ret, 0, "Restart read phase data integrity failed");

		/* Validate proper transaction sequencing */
		zassert_equal(stress_ctx.write_requested_count, 1U,
			      "write_requested count %u, expected 1",
			      stress_ctx.write_requested_count);
		zassert_equal(stress_ctx.stop_count, 1U, "stop count %u, expected 1",
			      stress_ctx.stop_count);
	}
}

ZTEST_SUITE(i2c_stress, NULL, i2c_stress_setup, i2c_stress_before,
	    i2c_stress_after, i2c_stress_teardown);

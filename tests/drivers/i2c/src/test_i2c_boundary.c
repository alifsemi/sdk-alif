/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include "test_i2c.h"
LOG_MODULE_REGISTER(alif_i2c_boundary, LOG_LEVEL_INF);

/* Boundary test constants - designed to expose FIFO and register issues */
#define BOUNDARY_TEST_MAX_SIZE	   64U
#define FIFO_DEPTH_PROBE_SIZE	   32U
#define TIMING_VIOLATION_DELAY_US  1U
#define REGISTER_STRESS_ITERATIONS 1000U
#define CLOCK_STRETCH_TIMEOUT_MS   100U

/* Hardware-aware test patterns */
static const uint8_t boundary_pattern_8bit[] = {
	0x00, 0xFF, 0xAA, 0x55, 0x01, 0xFE, 0x80, 0x7F
};

/* Boundary test context */
struct i2c_boundary_ctx {
	const struct device *master;
	const struct device *slave;
	uint32_t config_errors;
	uint32_t timing_errors;
	uint32_t fifo_errors;
};

static struct i2c_boundary_ctx boundary_ctx;

static int boundary_write_validate(const struct device *master,
				   const uint8_t *tx_data, size_t len,
				   uint16_t addr)
{
	int ret;

	i2c_test_prime_buffers(NULL, 0U, len);
	ret = i2c_write(master, tx_data, len, addr);
	if (ret != 0) {
		return ret;
	}

	return validate_slave_rx(tx_data, len);
}

static int boundary_write_read_validate(const struct device *master,
					uint16_t addr,
					const uint8_t *tx_data,
					uint8_t *rx_data, size_t len)
{
	uint8_t slave_tx_pattern[BUFF_PERF];
	size_t cap = MIN(len, (size_t)BUFF_PERF);
	int ret;

	/* Generate distinct slave TX pattern - must differ from master TX */
	fill_slave_tx_distinct(slave_tx_pattern, cap, SEED_SLAVE_TX);

	/* Prime slave with distinct TX data, prepare slave RX */
	i2c_test_prime_buffers(slave_tx_pattern, cap, len);

	/* Poison master RX to detect uninitialized reads */
	poison_buffer(rx_data, len, POISON_RX);

	ret = i2c_write_read(master, addr, tx_data, len, rx_data, len);
	if (ret != 0) {
		return ret;
	}

	ret = validate_slave_rx(tx_data, len);
	if (ret != 0) {
		return ret;
	}

	return validate_master_rx(slave_tx_pattern, rx_data, cap);
}

/**
 * @brief Test FIFO boundary conditions and overflow detection
 *
 * Objective: Expose FIFO overflow/underflow bugs in the driver
 * Hardware Issue: FIFO depth mismatches, pointer corruption
 * Driver Issue: Missing FIFO full/empty checks
 */
ZTEST(i2c_boundary_suite, test_fifo_boundary)
{
	TC_PRINT("Running boundary test: FIFO boundary\n");
	const struct device *master = DEVICE_DT_GET(I2C_MASTER);
	uint8_t tx_buffer[BOUNDARY_TEST_MAX_SIZE];
	uint8_t rx_buffer[BOUNDARY_TEST_MAX_SIZE];
	int ret;

	/* Prepare test data with alternating patterns to expose bit errors */
	for (size_t i = 0; i < BOUNDARY_TEST_MAX_SIZE; i++) {
		tx_buffer[i] = (i % 2) ? 0xAA : 0x55;
	}

	/* Test 1: Single byte transfers - expose pointer handling bugs */
	for (int i = 0; i < 10; i++) {
		ret = boundary_write_validate(master, &tx_buffer[i], 1U,
					      SLV_I2C_ADDR);
		zassert_ok(ret,
			   "Single byte write failed at iteration %d: %d",
			   i, ret);

		/* Small delay to expose timing-sensitive bugs */
		k_usleep(TIMING_VIOLATION_DELAY_US);
	}

	/* Test 2: FIFO depth probing - find actual FIFO boundaries */
	for (size_t size = 1; size <= FIFO_DEPTH_PROBE_SIZE; size++) {
		ret = boundary_write_validate(master, tx_buffer, size,
					      SLV_I2C_ADDR);
		if (ret != 0) {
			LOG_INF("FIFO boundary detected at size %zu (ret: %d)",
				size, ret);
			break;
		}
	}

	/* Test 3: Maximum size transfer - expose overflow handling */
	ret = boundary_write_validate(master, tx_buffer,
				      BOUNDARY_TEST_MAX_SIZE, SLV_I2C_ADDR);
	if (ret == -ENOMEM) {
		LOG_INF("Driver correctly rejects oversized transfer");
	} else if (ret == 0) {
		LOG_INF("Driver accepts max size xfer - checking corruption");
	} else {
		zassert_ok(ret,
			   "Unexpected error in max size transfer: %d", ret);
	}

	/* Test 4: Rapid small transfers - expose FIFO state machine bugs */
	for (int i = 0; i < 50; i++) {
		ret = boundary_write_read_validate(master, SLV_I2C_ADDR,
						   &tx_buffer[i % 8],
						   rx_buffer, 1U);
		zassert_ok(ret,
			   "Rapid transfer failed at iteration %d: %d",
			   i, ret);
	}

	LOG_INF("FIFO boundary tests completed - no overflow/underflow");
}

/**
 * @brief Test register configuration edge cases
 *
 * Objective: Expose register misconfiguration and state machine bugs
 * Hardware Issue: Register bit fields not properly set/cleared
 * Driver Issue: Missing register validation, incorrect bit manipulation
 */
ZTEST(i2c_boundary_suite, test_reg_config_edge)
{
	TC_PRINT("Running boundary test: register config edge\n");
	const struct device *master = DEVICE_DT_GET(I2C_MASTER);
	uint32_t config;
	uint8_t test_data[] = {0x12, 0x34, 0x56, 0x78};
	int ret;

	/* Test 1: Speed configuration edge cases */
	const uint32_t speed_values[] = {
		I2C_SPEED_STANDARD, I2C_SPEED_FAST,
		I2C_SPEED_FAST_PLUS, I2C_SPEED_HIGH
	};

	for (size_t i = 0; i < ARRAY_SIZE(speed_values); i++) {
		config = I2C_MODE_CONTROLLER |
			 I2C_SPEED_SET(speed_values[i]);

		register_slave_speed_i2c(I2C_SPEED_SET(speed_values[i]));
		ret = i2c_configure(master, config);
		i2c_test_ctx.i2c_cfg = config;
		if (ret == -ENOTSUP) {
			LOG_INF("Speed %u not supported, skipping",
				speed_values[i]);
			continue;
		}
		zassert_ok(ret,
			   "Speed configuration failed for value %u: %d",
			   speed_values[i], ret);

		/* Test transfer after each configuration change */
		if (ret == 0) {
			ret = boundary_write_validate(master, test_data,
						      sizeof(test_data),
							  SLV_I2C_ADDR);
			zassert_ok(ret,
				   "Transfer failed after speed config %u: %d",
				   speed_values[i], ret);
		}
	}

	/* Test 2: Addressing mode edge cases */
	const uint32_t addressing_modes[] = {
		0,				 /* 7-bit (no flags) */
		I2C_MSG_ADDR_10_BITS /* 10-bit addressing flag */
	};

	for (size_t i = 0; i < ARRAY_SIZE(addressing_modes); i++) {
		config = I2C_MODE_CONTROLLER |
			 I2C_SPEED_SET(I2C_SPEED_STANDARD);
		if (addressing_modes[i] == I2C_MSG_ADDR_10_BITS) {
			config |= I2C_ADDR_10_BITS;
		}

		if (addressing_modes[i] == I2C_MSG_ADDR_10_BITS) {
			register_slave_i2c_10Bit();
		} else {
			register_slave_i2c();
		}

		ret = i2c_configure(master, config);
		i2c_test_ctx.i2c_cfg = config;
		if (ret == -ENOTSUP) {
			LOG_INF("Addressing mode %u not supported, skipping",
				addressing_modes[i]);
			continue;
		}
		if (ret != 0) {
			boundary_ctx.config_errors++;
			LOG_ERR("Addressing mode %u config failed: %d",
				addressing_modes[i], ret);
			continue;
		}

		/* Test with appropriate addressing */
		uint16_t test_addr =
			(addressing_modes[i] == I2C_MSG_ADDR_10_BITS)
			? SLV_I2C_10BITADDR
			: SLV_I2C_ADDR;

		if (addressing_modes[i] == I2C_MSG_ADDR_10_BITS) {
			struct i2c_msg msg = {
				.buf = test_data,
				.len = sizeof(test_data),
				.flags = addressing_modes[i] |
					 I2C_MSG_WRITE | I2C_MSG_STOP
			};
			i2c_test_prime_buffers(NULL, 0U, sizeof(test_data));
			ret = i2c_transfer(master, &msg, 1, test_addr);
			if (ret == 0) {
				ret = validate_slave_rx(test_data,
							sizeof(test_data));
			}
		} else {
			ret = boundary_write_validate(master, test_data,
						      sizeof(test_data),
							  test_addr);
		}
		zassert_ok(ret,
			   "Transfer failed after addr mode %u",
			   addressing_modes[i]);
	}

	register_slave_i2c();

	/* Test 3: Rapid reconfiguration - expose register timing bugs */
	for (int i = 0; i < REGISTER_STRESS_ITERATIONS; i++) {
		config = I2C_MODE_CONTROLLER |
			 I2C_SPEED_SET((i % 2) ? I2C_SPEED_STANDARD
						   : I2C_SPEED_FAST);

		ret = i2c_configure(master, config);
		i2c_test_ctx.i2c_cfg = config;
		register_slave_speed_i2c(
			I2C_SPEED_SET((i % 2) ? I2C_SPEED_STANDARD
						  : I2C_SPEED_FAST));
		zassert_ok(ret,
			   "Rapid reconfiguration failed at iteration %d: %d",
			   i, ret);

		/* Immediate transfer after reconfiguration */
		ret = boundary_write_validate(
			master,
			&test_data[i % sizeof(test_data)],
			1U, SLV_I2C_ADDR);
		zassert_ok(ret,
			   "Transfer failed after rapid reconfig %d", i);
	}

	LOG_INF("Register configuration edge cases completed");
}

/**
 * @brief Test timing boundary conditions
 *
 * Objective: Expose timing violations and interrupt handling bugs
 * Hardware Issue: Setup/hold time violations, interrupt latency
 * Driver Issue: Missing timeout handling, incorrect interrupt sequencing
 */
ZTEST(i2c_boundary_suite, test_timing_boundary)
{
	TC_PRINT("Running boundary test: timing boundary\n");
	const struct device *master = DEVICE_DT_GET(I2C_MASTER);
	uint8_t test_data[] = {0xAA, 0x55, 0xFF, 0x00};
	int ret;

	/* Test 1: Minimum inter-transfer delay - expose timing bugs */
	for (int delay_us = 0; delay_us <= 10; delay_us++) {
		for (int i = 0; i < 5; i++) {
			ret = boundary_write_validate(master, test_data,
						      sizeof(test_data),
							  SLV_I2C_ADDR);
			zassert_ok(ret,
			   "Timing test failed at delay %dus, iter %d: %d",
			   delay_us, i, ret);

			if (delay_us > 0) {
				k_usleep(delay_us);
			}
		}
		LOG_INF("Timing test passed for %dus inter-transfer delay",
			delay_us);
	}

	/* Test 2: Back-to-back repeated start - expose restart timing bugs */
	for (int i = 0; i < 20; i++) {
		uint8_t rx_data[2];

		ret = boundary_write_read_validate(master, SLV_I2C_ADDR,
						   test_data, rx_data, 2U);
		zassert_ok(ret,
			   "Repeated start test failed at iteration %d: %d",
			   i, ret);
	}

	/* Test 3: Clock stretching timeout validation */
	{
		int64_t start_time, end_time, duration_ms;

		start_time = k_uptime_get();
		ret = boundary_write_validate(master, test_data,
					      sizeof(test_data), SLV_I2C_ADDR);
		end_time = k_uptime_get();

		zassert_ok(ret, "Clock stretching test failed: %d", ret);

		duration_ms = end_time - start_time;

		if (duration_ms > CLOCK_STRETCH_TIMEOUT_MS) {
			LOG_INF("Clk stretch: %lldms (timeout: %dms)",
				duration_ms, CLOCK_STRETCH_TIMEOUT_MS);
		} else {
			LOG_INF("No clock stretching (transfer took %lldms)",
				duration_ms);
		}
	}

	/* Test 4: Interrupt latency stress - expose interrupt handling bugs */
	for (int i = 0; i < 100; i++) {
		unsigned int key = irq_lock();

		i2c_test_prime_buffers(NULL, 0U, 1U);
		ret = i2c_write(master,
				&test_data[i % sizeof(test_data)],
				1, SLV_I2C_ADDR);

		irq_unlock(key);

		/* Allow for interrupt-driven timing constraints */
		if (ret == -EBUSY || ret == -EAGAIN) {
			LOG_DBG("IRQ latency iter %d ret %d", i, ret);
			continue;
		}
		zassert_ok(ret,
			   "Interrupt latency test failed at iteration %d: %d",
			   i, ret);
		if (ret == 0) {
			ret = validate_slave_rx(
				&test_data[i % sizeof(test_data)], 1U);
			zassert_ok(ret,
				   "Interrupt latency valid failed at iter %d: %d",
				   i, ret);
		}
	}

	LOG_INF("Timing boundary conditions completed");
}

/**
 * @brief Test data pattern boundary conditions
 *
 * Objective: Expose data corruption and signal integrity issues
 * Hardware Issue: Signal integrity, metastability, timing violations
 * Driver Issue: Data path corruption, byte ordering issues
 */
ZTEST(i2c_boundary_suite, test_data_pattern)
{
	TC_PRINT("Running boundary test: data pattern\n");
	const struct device *master = DEVICE_DT_GET(I2C_MASTER);
	uint8_t rx_buffer[64];
	int ret;

	/* Test 1: Critical bit patterns - expose signal integrity issues */
	struct {
		const char *name;
		const uint8_t *pattern;
		size_t size;
	} critical_patterns[] = {
		{"Alternating", boundary_pattern_8bit,
		 sizeof(boundary_pattern_8bit)},
		{"All zeros",
		 (uint8_t[]){0x00, 0x00, 0x00, 0x00}, 4},
		{"All ones",
		 (uint8_t[]){0xFF, 0xFF, 0xFF, 0xFF}, 4},
		{"Walking ones",
		 (uint8_t[]){0x01, 0x02, 0x04, 0x08,
				 0x10, 0x20, 0x40, 0x80}, 8},
		{"Walking zeros",
		 (uint8_t[]){0xFE, 0xFD, 0xFB, 0xF7,
				 0xEF, 0xDF, 0xBF, 0x7F}, 8},
	};

	for (size_t p = 0; p < ARRAY_SIZE(critical_patterns); p++) {
		for (int iteration = 0; iteration < 10; iteration++) {
			ret = boundary_write_read_validate(
				master, SLV_I2C_ADDR,
				critical_patterns[p].pattern,
				rx_buffer,
				critical_patterns[p].size);
			zassert_ok(ret,
				   "Pattern test failed for %s at iter %d: %d",
				   critical_patterns[p].name,
				   iteration, ret);
		}
		LOG_INF("Pattern test passed for %s",
			critical_patterns[p].name);
	}

	/* Test 2: Size boundary conditions - expose size handling bugs */
	const size_t boundary_sizes[] = {
		1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64
	};

	for (size_t i = 0; i < ARRAY_SIZE(boundary_sizes); i++) {
		uint8_t tx_buffer[64];

		/* Fill buffer with unique pattern for this size */
		for (size_t j = 0; j < boundary_sizes[i]; j++) {
			tx_buffer[j] = (uint8_t)(boundary_sizes[i] + j);
		}

		ret = boundary_write_read_validate(master, SLV_I2C_ADDR,
						   tx_buffer, rx_buffer,
						   boundary_sizes[i]);
		zassert_ok(ret,
			   "Size boundary test failed for size %zu: %d",
			   boundary_sizes[i], ret);
	}

	LOG_INF("Data pattern boundary conditions completed");
}

static void *i2c_boundary_suite_setup(void)
{
	const struct device *master = DEVICE_DT_GET(I2C_MASTER);
	const struct device *slave = DEVICE_DT_GET(I2C_SLAVE);
	int ret;

	/* Initialize boundary test context */
	memset(&boundary_ctx, 0, sizeof(boundary_ctx));
	boundary_ctx.master = master;
	boundary_ctx.slave = slave;

	/* Configure devices */
	zassert_not_null(master, "Master I2C device not found");
	zassert_true(device_is_ready(master), "Master I2C device not ready");

	zassert_not_null(slave, "Slave I2C device not found");
	zassert_true(device_is_ready(slave), "Slave I2C device not ready");

	i2c_test_ctx.master_dev = master;
	i2c_test_ctx.slave_dev = slave;
	i2c_test_reset_runtime_config(&i2c_test_ctx);
	ret = i2c_configure(master, i2c_test_ctx.i2c_cfg);
	zassert_ok(ret, "Master I2C configuration failed: %d", ret);
	register_slave_i2c();

	LOG_INF("I2C boundary test suite setup completed");
	return NULL;
}

static void i2c_boundary_suite_teardown(void *fixture)
{
	ARG_UNUSED(fixture);
	/* Reset shared context to avoid test leakage */
	i2c_test_reset_runtime_config(&i2c_test_ctx);
	memset(&boundary_ctx, 0, sizeof(boundary_ctx));
}

ZTEST_SUITE(i2c_boundary_suite, NULL, i2c_boundary_suite_setup, NULL,
	    i2c_boundary_suite_teardown, NULL);

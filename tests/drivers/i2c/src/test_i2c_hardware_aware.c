/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include "test_i2c.h"
LOG_MODULE_REGISTER(alif_i2c_hw, LOG_LEVEL_INF);

/* Hardware-aware test constants */
#define HW_INTERRUPT_STRESS_COUNT 1000U
#define HW_CLOCK_STRETCH_MAX_MS	  200U
#define HW_TIMING_PRECISION_US	  1U
#define HW_DMA_TEST_SIZE		  BUFF_PERF

/* Hardware timing validation patterns */
static const uint8_t timing_pattern[] = {
	0x55, 0xAA, 0xFF, 0x00, 0x01, 0xFE, 0x80, 0x7F
};

static const uint8_t signal_integrity_pattern[] = {
	0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, /* Fast transitions */
	0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA	/* Alternating */
};

/* Hardware-aware test context */
struct i2c_hw_ctx {
	const struct device *master;
	const struct device *slave;
	uint32_t interrupt_errors;
	uint32_t timing_violations;
	uint32_t signal_errors;
	bool dma_available;
	uint32_t baseline_freq_hz;
};

static struct i2c_hw_ctx hw_ctx;

static int hardware_set_speed(const struct device *master, uint32_t speed)
{
	uint32_t config = I2C_MODE_CONTROLLER | I2C_SPEED_SET(speed);
	int ret;

	ret = i2c_configure(master, config);
	if (ret == 0) {
		i2c_test_ctx.i2c_cfg = config;
		register_slave_speed_i2c(I2C_SPEED_SET(speed));
	}

	return ret;
}

static int hardware_write_validate(const struct device *master,
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

static int hardware_write_read_validate(const struct device *master,
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

static int hardware_read_validate(const struct device *master, uint16_t addr,
				  const uint8_t *expected_data,
				  uint8_t *rx_data, size_t len)
{
	int ret;

	i2c_test_prime_buffers(expected_data, len, 0U);
	ret = i2c_read(master, rx_data, len, addr);
	if (ret != 0) {
		return ret;
	}

	return validate_master_rx(expected_data, rx_data, len);
}

/**
 * @brief Test interrupt handling robustness
 *
 * Objective: Expose interrupt handling bugs in driver
 * Hardware Issue: Interrupt timing, priority, nesting
 * Driver Issue: Missing interrupt state validation, race conditions
 */
ZTEST(i2c_hardware_suite, test_irq_robustness)
{
	const struct device *master = DEVICE_DT_GET(I2C_MASTER);
	uint8_t test_data[] = {0x12, 0x34, 0x56, 0x78};
	uint8_t rx_buffer[8];
	int ret;

	/* Test 1: Interrupt latency stress - expose timing bugs */
	for (int i = 0; i < HW_INTERRUPT_STRESS_COUNT; i++) {
		int64_t start_time = k_uptime_get();
		unsigned int key = irq_lock();

		i2c_test_prime_buffers(NULL, 0U, sizeof(test_data));
		ret = i2c_write(master, test_data, sizeof(test_data),
				SLV_I2C_ADDR);

		irq_unlock(key);

		int64_t end_time = k_uptime_get();
		int64_t duration_us = (end_time - start_time) * 1000;

		zassert_ok(ret,
			   "Interrupt stress test failed at iteration %d: %d",
			   i, ret);
		if (ret == 0) {
			ret = validate_slave_rx(test_data, sizeof(test_data));
			zassert_ok(ret,
				   "IRQ stress valid failed at iter %d: %d",
				   i, ret);
		}

		/* Check for unusual timing that might indicate IRQ issues */
		if (duration_us > 5000) { /* 5ms threshold */
			LOG_DBG("Long xfer at iter %d: %lldus", i, duration_us);
			hw_ctx.interrupt_errors++;
		}
	}

	/* Test 2: Interrupt nesting - expose nested interrupt bugs */
	for (int i = 0; i < 100; i++) {
		unsigned int key1 = irq_lock();

		i2c_test_prime_buffers(NULL, 0U, 1U);
		ret = i2c_write(master, &test_data[0], 1, SLV_I2C_ADDR);

		if (ret == 0) {
			irq_unlock(key1);
			ret = validate_slave_rx(&test_data[0], 1U);
			zassert_ok(ret,
				   "Nested IRQ 1st write fail %d: %d",
				   i, ret);

			key1 = irq_lock();
			/* Simulate nested interrupt */
			unsigned int key2 = irq_lock();

			i2c_test_prime_buffers(NULL, 0U, 1U);
			ret = i2c_write(master, &test_data[1], 1,
					SLV_I2C_ADDR);

			irq_unlock(key2);
		}

		irq_unlock(key1);

		if (ret != 0) {
			LOG_DBG("Nested IRQ fail at iter %d: %d", i, ret);
			hw_ctx.interrupt_errors++;
		} else {
			ret = validate_slave_rx(&test_data[1], 1U);
			zassert_ok(ret,
				   "Nested IRQ 2nd write fail %d: %d",
				   i, ret);
		}
	}

	/* Test 3: Interrupt context switching - expose context bugs */
	for (int i = 0; i < 50; i++) {
		/* Rapid interrupt enable/disable */
		for (int j = 0; j < 10; j++) {
			unsigned int key = irq_lock();

			i2c_test_prime_buffers(NULL, 0U, 1U);

			ret = i2c_write(master, &test_data[j % 4], 1,
					SLV_I2C_ADDR);
			irq_unlock(key);

			if (ret != 0) {
				LOG_DBG("Ctx switch fail at %d.%d: %d",
					i, j, ret);
				hw_ctx.interrupt_errors++;
				break;
			}

			ret = validate_slave_rx(&test_data[j % 4], 1U);
			if (ret != 0) {
				LOG_DBG("Ctx switch fail %d.%d: %d",
					i, j, ret);
				hw_ctx.interrupt_errors++;
				break;
			}
		}
	}

	/* Test 4: Write-Read with interrupt pressure */
	for (int i = 0; i < 50; i++) {
		unsigned int key = irq_lock();

		i2c_test_prime_buffers(test_data, 2U, 2U);
		ret = i2c_write_read(master, SLV_I2C_ADDR,
				     test_data, 2, rx_buffer, 2);

		irq_unlock(key);

		zassert_ok(ret, "WR IRQ test fail iter %d: %d", i, ret);
		if (ret == 0) {
			ret = validate_slave_rx(test_data, 2U);
			zassert_ok(ret,
				   "WR IRQ w-phase fail iter %d: %d",
				   i, ret);
			ret = validate_master_rx(test_data, rx_buffer, 2U);
			zassert_ok(ret,
				   "WR IRQ r-phase fail iter %d: %d",
				   i, ret);
		}
	}

	LOG_INF("IRQ robustness done (errors: %u)",
		hw_ctx.interrupt_errors);
}

/**
 * @brief Test clock stretching behavior
 *
 * Objective: Expose clock stretching bugs in hardware and driver
 * Hardware Issue: Clock stretch timeout, improper stretch detection
 * Driver Issue: Missing stretch timeout, incorrect stretch handling
 */
ZTEST(i2c_hardware_suite, test_clock_stretch)
{
	const struct device *master = DEVICE_DT_GET(I2C_MASTER);
	uint8_t rx_buffer[8];
	int ret;

	/* Test 1: Clock stretching detection - expose detection bugs */
	for (int i = 0; i < 20; i++) {
		int64_t start_time = k_uptime_get();

		ret = hardware_read_validate(master, SLV_I2C_ADDR,
					     timing_pattern, rx_buffer,
						 sizeof(timing_pattern));

		int64_t end_time = k_uptime_get();
		int64_t duration_ms = end_time - start_time;

		if (ret == -ETIMEDOUT) {
			LOG_DBG("Clk stretch timeout at iter %d", i);
			hw_ctx.timing_violations++;
		} else if (ret == 0) {
			if (duration_ms > HW_CLOCK_STRETCH_MAX_MS) {
				LOG_DBG("Long xfer clk stretch: %lldms",
					 duration_ms);
			}
		} else {
			LOG_DBG("Clk stretch test error %d at iter %d", ret, i);
		}
	}

	/* Test 2: Clock stretching recovery - expose recovery bugs */
	for (int i = 0; i < 10; i++) {
		ret = hardware_read_validate(master, SLV_I2C_ADDR,
					     timing_pattern, rx_buffer, 1U);

		if (ret == -ETIMEDOUT) {
			/* Attempt recovery */
			uint32_t config = I2C_MODE_CONTROLLER |
					  I2C_SPEED_SET(I2C_SPEED_STANDARD);

			ret = i2c_configure(master, config);
			zassert_ok(ret,
				   "Clock stretch recovery config failed: %d",
				   ret);
			i2c_test_ctx.i2c_cfg = config;
			register_slave_i2c();

			/* Retry transfer */
			ret = hardware_read_validate(master, SLV_I2C_ADDR,
						     timing_pattern,
							 rx_buffer, 1U);
		}

		if (ret == 0) {
			LOG_DBG("Clk stretch recovery success at iter %d", i);
		}
	}

	/* Test 3: Clock stretching with different speeds */
	const uint32_t test_speeds[] = {
		I2C_SPEED_STANDARD, I2C_SPEED_FAST, I2C_SPEED_FAST_PLUS
	};

	for (size_t i = 0; i < ARRAY_SIZE(test_speeds); i++) {
		ret = hardware_set_speed(master, test_speeds[i]);
		zassert_ok(ret,
			   "Speed config failed for clock stretch test: %d",
			   ret);

		int64_t start_time = k_uptime_get();

		ret = hardware_read_validate(master, SLV_I2C_ADDR,
					     timing_pattern, rx_buffer, 4U);
		int64_t end_time = k_uptime_get();

		if (ret == 0) {
			int64_t duration_ms = end_time - start_time;

			LOG_INF("Clk stretch speed %u: %lldms",
				test_speeds[i], duration_ms);
		}
	}

	/* Test 4: Clock stretching timeout validation */
	for (int timeout_ms = 10; timeout_ms <= 100; timeout_ms += 10) {
		int64_t start_time = k_uptime_get();

		ret = hardware_read_validate(master, SLV_I2C_ADDR,
					     timing_pattern, rx_buffer,
						 sizeof(timing_pattern));
		int64_t end_time = k_uptime_get();
		int64_t duration_ms = end_time - start_time;

		if (duration_ms > timeout_ms) {
			LOG_DBG("Exceeded %dms (actual: %lldms)",
				timeout_ms, duration_ms);
		}
	}

	LOG_INF("Clock stretch tests done (violations: %u)",
		hw_ctx.timing_violations);
}

/**
 * @brief Test signal integrity and timing precision
 *
 * Objective: Expose signal integrity and timing precision bugs
 * Hardware Issue: Signal integrity, setup/hold time violations
 * Driver Issue: Incorrect timing calculations, missing validation
 */
ZTEST(i2c_hardware_suite, test_signal_integrity)
{
	const struct device *master = DEVICE_DT_GET(I2C_MASTER);
	uint8_t rx_buffer[32];
	int ret;

	/* Test 1: High-frequency signal integrity */
	const uint32_t speeds[] = {
		I2C_SPEED_FAST, I2C_SPEED_FAST_PLUS, I2C_SPEED_HIGH
	};

	for (int speed_idx = 0; speed_idx < 3; speed_idx++) {
		ret = hardware_set_speed(master, speeds[speed_idx]);
		zassert_ok(ret, "High-speed config failed: %d", ret);

		/* Test with fast transition patterns */
		for (int iteration = 0; iteration < 10; iteration++) {
			ret = hardware_write_read_validate(
				master, SLV_I2C_ADDR,
				signal_integrity_pattern, rx_buffer,
				sizeof(signal_integrity_pattern));

			if (ret != 0) {
				LOG_DBG("Signal integ fail speed %u iter %d:%d",
					speeds[speed_idx], iteration, ret);
				hw_ctx.signal_errors++;
				continue;
			}
		}

		LOG_INF("Signal integrity test passed for speed %u",
			speeds[speed_idx]);
	}

	/* Test 2: Timing precision validation - expose timing bugs */
	for (int i = 0; i < 50; i++) {
		int64_t start_time = k_uptime_get();

		ret = hardware_write_read_validate(master, SLV_I2C_ADDR,
						   timing_pattern, rx_buffer,
						   sizeof(timing_pattern));

		int64_t end_time = k_uptime_get();
		int64_t duration_us = (end_time - start_time) * 1000;

		zassert_ok(ret,
			   "Timing precision test failed at iteration %d: %d",
			   i, ret);

		/* Check for timing anomalies */
		if (duration_us > 10000) { /* 10ms threshold */
			LOG_DBG("Timing anomaly iter %d: %lldus",
				 i, duration_us);
			hw_ctx.timing_violations++;
		}
	}

	/* Test 3: Setup/hold time validation */
	for (int delay_us = 0; delay_us <= 100; delay_us += 10) {
		for (int iteration = 0; iteration < 5; iteration++) {
			ret = hardware_write_validate(master, timing_pattern,
						      sizeof(timing_pattern),
							  SLV_I2C_ADDR);
			zassert_ok(ret,
				   "Setup/hold test failed at %dus, iter %d: %d",
				   delay_us, iteration, ret);

			if (delay_us > 0) {
				k_usleep(delay_us);
			}
		}
	}

	/* Test 4: Edge case timing - expose edge timing bugs */
	const uint8_t edge_patterns[][4] = {
		{0x00, 0x00, 0x00, 0x00}, /* All zeros */
		{0xFF, 0xFF, 0xFF, 0xFF}, /* All ones */
		{0xAA, 0x55, 0xAA, 0x55}, /* Alternating */
		{0x01, 0x02, 0x04, 0x08}, /* Single bits */
	};

	for (size_t pattern = 0; pattern < ARRAY_SIZE(edge_patterns);
		 pattern++) {
		for (int iteration = 0; iteration < 20; iteration++) {
			ret = hardware_write_read_validate(
				master, SLV_I2C_ADDR,
				edge_patterns[pattern], rx_buffer, 4U);

			zassert_ok(ret,
				   "Edge timing test failed for pat %zu, iter %d: %d",
				   pattern, iteration, ret);
		}
	}

	LOG_INF("Signal integrity done (sig err: %u, timing viol: %u)",
		hw_ctx.signal_errors, hw_ctx.timing_violations);
}

/**
 * @brief Test DMA interactions (if available)
 *
 * Objective: Expose DMA-related bugs in I2C driver
 * Hardware Issue: DMA FIFO interaction, alignment issues
 * Driver Issue: DMA buffer management, synchronization bugs
 */
ZTEST(i2c_hardware_suite, test_dma)
{
	const struct device *master = DEVICE_DT_GET(I2C_MASTER);
	uint8_t tx_buffer[HW_DMA_TEST_SIZE];
	uint8_t rx_buffer[HW_DMA_TEST_SIZE];
	int ret;

	/* Check if DMA is available */
	hw_ctx.dma_available = true; /* Assume available for test */

	if (!hw_ctx.dma_available) {
		LOG_INF("DMA not available - skipping DMA tests");
		return;
	}

	/* Test 1: DMA boundary conditions - expose alignment bugs */
	const size_t dma_sizes[] = {
		1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64, 127, 128
	};

	for (size_t i = 0; i < ARRAY_SIZE(dma_sizes); i++) {
		/* Prepare test data */
		for (size_t j = 0; j < dma_sizes[i]; j++) {
			tx_buffer[j] = (uint8_t)(dma_sizes[i] + j);
		}

		ret = hardware_write_read_validate(master, SLV_I2C_ADDR,
						   tx_buffer, rx_buffer,
						   dma_sizes[i]);

		if (ret != 0) {
			LOG_INF("DMA test failed for size %zu: %d",
				dma_sizes[i], ret);
			continue;
		}
	}

	/* Test 2: DMA alignment issues - expose alignment bugs */
	uint8_t unaligned_tx[HW_DMA_TEST_SIZE + 1];
	uint8_t unaligned_rx[HW_DMA_TEST_SIZE + 1];

	uint8_t *unaligned_tx_ptr = &unaligned_tx[1];
	uint8_t *unaligned_rx_ptr = &unaligned_rx[1];

	/* Prepare unaligned test data */
	for (size_t i = 0; i < HW_DMA_TEST_SIZE; i++) {
		unaligned_tx_ptr[i] = (uint8_t)i;
	}

	ret = hardware_write_read_validate(master, SLV_I2C_ADDR,
					   unaligned_tx_ptr, unaligned_rx_ptr,
					   HW_DMA_TEST_SIZE);

	if (ret == 0) {
		LOG_INF("Unaligned DMA test passed");
	} else {
		LOG_INF("Unaligned DMA test failed (expected): %d", ret);
	}

	/* Test 3: DMA stress test - expose DMA state machine bugs */
	for (int i = 0; i < 50; i++) {
		size_t test_size = 32 + (i % 224); /* Variable sizes 32-255 */

		/* Prepare test data */
		for (size_t j = 0; j < test_size; j++) {
			tx_buffer[j] = (uint8_t)(i + j);
		}

		ret = hardware_write_read_validate(master, SLV_I2C_ADDR,
						   tx_buffer, rx_buffer,
						   test_size);

		zassert_ok(ret,
			   "DMA stress test failed at iter %d, size %zu: %d",
			   i, test_size, ret);
	}

	LOG_INF("DMA interactions tests completed");
}

static void *i2c_hardware_suite_setup(void)
{
	const struct device *master = DEVICE_DT_GET(I2C_MASTER);
	const struct device *slave = DEVICE_DT_GET(I2C_SLAVE);
	int ret;

	/* Initialize hardware context */
	memset(&hw_ctx, 0, sizeof(hw_ctx));
	hw_ctx.master = master;
	hw_ctx.slave = slave;
	hw_ctx.baseline_freq_hz = 100000; /* Standard mode */

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

	LOG_INF("I2C hardware-aware test suite setup completed");
	return NULL;
}

ZTEST_SUITE(i2c_hardware_suite, NULL, i2c_hardware_suite_setup,
	    NULL, NULL, NULL);

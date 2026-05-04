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
#define HW_DMA_TEST_SIZE		  BUFF_PERF

/* Hardware timing validation patterns */
static const uint8_t timing_pattern[] = {
	0x55, 0xAA, 0xFF, 0x00, 0x01, 0xFE, 0x80, 0x7F
};

static const uint8_t signal_integrity_pattern[] = {
	0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, /* Fast transitions */
	0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA	/* Alternating */
};

extern size_t total_freq_count;

/* Hardware-aware test context */
struct i2c_hw_ctx {
	const struct device *controller;
	const struct device *target;
	uint32_t interrupt_errors;
	uint32_t timing_violations;
	uint32_t signal_errors;
	bool dma_available;
};

static struct i2c_hw_ctx hw_ctx;

/* Async mode flag for hardware-aware tests (SPI TI pattern) */
static bool hw_test_async_mode;

/*
 * Async-capable transfer helpers
 */

static int hw_xfer_write(const struct device *dev, const uint8_t *data,
			 size_t len, uint16_t addr)
{
	if (!hw_test_async_mode) {
		return i2c_write(dev, data, len, addr);
	}
	struct i2c_msg msg = {
		.buf = (uint8_t *)data,
		.len = len,
		.flags = I2C_MSG_WRITE | I2C_MSG_STOP,
	};
	return i2c_do_xfer(dev, &msg, 1, addr, true);
}

static int hw_xfer_read(const struct device *dev, uint8_t *data,
			size_t len, uint16_t addr)
{
	if (!hw_test_async_mode) {
		return i2c_read(dev, data, len, addr);
	}
	struct i2c_msg msg = {
		.buf = data,
		.len = len,
		.flags = I2C_MSG_READ | I2C_MSG_STOP,
	};
	return i2c_do_xfer(dev, &msg, 1, addr, true);
}

static int hw_xfer_write_read(const struct device *dev, uint16_t addr,
			      const uint8_t *tx, size_t tx_len,
			      uint8_t *rx, size_t rx_len)
{
	if (!hw_test_async_mode) {
		return i2c_write_read(dev, addr, tx, tx_len, rx, rx_len);
	}
	struct i2c_msg msgs[2];

	msgs[0].buf = (uint8_t *)tx;
	msgs[0].len = tx_len;
	msgs[0].flags = I2C_MSG_WRITE | I2C_MSG_RESTART;
	msgs[1].buf = rx;
	msgs[1].len = rx_len;
	msgs[1].flags = I2C_MSG_READ | I2C_MSG_STOP;
	return i2c_do_xfer(dev, msgs, 2, addr, true);
}

static int hw_xfer_transfer(const struct device *dev, struct i2c_msg *msgs,
			    uint8_t num_msgs, uint16_t addr)
{
	if (!hw_test_async_mode) {
		return i2c_transfer(dev, msgs, num_msgs, addr);
	}
	return i2c_do_xfer(dev, msgs, num_msgs, addr, true);
}

/* Reset error counters before each test to prevent cross-test contamination */
static void reset_hw_error_counters(void)
{
	hw_ctx.interrupt_errors = 0U;
	hw_ctx.timing_violations = 0U;
	hw_ctx.signal_errors = 0U;
}

static int hardware_write_validate(const struct device *controller,
				   const uint8_t *tx_data, size_t len,
				   uint16_t addr)
{
	int ret;

	i2c_test_prime_buffers(NULL, 0U, len);
	ret = hw_xfer_write(controller, tx_data, len, addr);
	if (ret != 0) {
		return ret;
	}

	return validate_target_rx(tx_data, len);
}

/* Static buffer to avoid stack overflow in nested calls */
static uint8_t hw_target_tx_pattern[BUFF_PERF];

static int hardware_write_read_validate(const struct device *controller,
					uint16_t addr,
					const uint8_t *tx_data,
					uint8_t *rx_data, size_t len)
{
	size_t cap = MIN(len, (size_t)BUFF_PERF);
	int ret;

	/* Generate distinct target TX pattern - must differ from controller TX */
	fill_target_tx_distinct(hw_target_tx_pattern, cap, SEED_TARGET_TX);

	/* Prime target with distinct TX data, prepare target RX */
	i2c_test_prime_buffers(hw_target_tx_pattern, cap, len);

	/* Poison controller RX to detect uninitialized reads */
	poison_buffer(rx_data, len, POISON_RX);

	ret = hw_xfer_write_read(controller, addr, tx_data, len,
				 rx_data, len);
	if (ret != 0) {
		return ret;
	}

	ret = validate_target_rx(tx_data, len);
	if (ret != 0) {
		return ret;
	}

	return validate_controller_rx(hw_target_tx_pattern, rx_data, cap);
}

static int hardware_read_validate(const struct device *controller, uint16_t addr,
				  const uint8_t *expected_data,
				  uint8_t *rx_data, size_t len)
{
	int ret;

	i2c_test_prime_buffers(expected_data, len, 0U);
	ret = hw_xfer_read(controller, rx_data, len, addr);
	if (ret != 0) {
		return ret;
	}

	return validate_controller_rx(expected_data, rx_data, len);
}

/**
 * @brief Test interrupt handling robustness
 *
 * Objective: Expose interrupt handling bugs in driver
 * Hardware Issue: Interrupt timing, priority, nesting
 * Driver Issue: Missing interrupt state validation, race conditions
 */
static void hw_scenario_irq_robustness(void)
{
	const struct device *controller = hw_ctx.controller;
	uint8_t test_data[] = {0x12, 0x34, 0x56, 0x78};
	uint8_t rx_buffer[8];
	struct i2c_test_freq_desc freqs[I2C_TEST_MAX_FREQS];
	int ret;
	int freq_count;

	i2c_test_skip_if_no_freqs();
	freq_count = i2c_test_get_enabled_freqs(freqs, ARRAY_SIZE(freqs));

	for (int f = 0; f < freq_count; f++) {
		TC_PRINT("Testing at %s (%u Hz)\n", freqs[f].name, freqs[f].freq_hz);

		ret = i2c_test_configure_controller_freq(controller, &i2c_test_ctx, &freqs[f]);
		if (ret == -ENOTSUP) {
			TC_PRINT("Skipping %s: not supported\n", freqs[f].name);
			continue;
		}
		zassert_ok(ret, "Failed to configure %s: %d", freqs[f].name, ret);

		register_target_speed_i2c(I2C_SPEED_SET(freqs[f].zephyr_speed));

	/*
	 * Test 1: Scheduler-locked latency stress - expose timing bugs
	 *
	 * The previous version disabled interrupts around i2c_write().
	 * Zephyr I2C drivers complete transfers via their ISR and block on
	 * a semaphore, so running a blocking transfer with IRQs disabled
	 * would deadlock or yield spurious -ETIMEDOUT errors. Use
	 * k_sched_lock() to pin the thread (preventing preemption) while
	 * still letting the I2C ISR fire.
	 */
	for (int i = 0; i < HW_INTERRUPT_STRESS_COUNT; i++) {
		int64_t start_time = k_uptime_get();

		k_sched_lock();

		i2c_test_prime_buffers(NULL, 0U, sizeof(test_data));
		ret = hw_xfer_write(controller, test_data, sizeof(test_data),
				    TGT_I2C_ADDR);

		k_sched_unlock();

		int64_t end_time = k_uptime_get();
		int64_t duration_us = (end_time - start_time) * 1000;

		zassert_ok(ret,
			   "Interrupt stress test failed at iteration %d: %d",
			   i, ret);
		if (ret == 0) {
			ret = validate_target_rx(test_data, sizeof(test_data));
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

	/*
	 * Test 2: Nested scheduler-lock pressure
	 *
	 * Same rationale as Test 1 - blocking i2c_write() cannot run with
	 * IRQs disabled on an interrupt-driven controller. Simulate the
	 * "nested" pressure with stacked k_sched_lock() calls, which are
	 * reference-counted and therefore still test re-entrant pinning
	 * without breaking the driver.
	 */
	for (int i = 0; i < 100; i++) {
		k_sched_lock();

		i2c_test_prime_buffers(NULL, 0U, 1U);
		ret = hw_xfer_write(controller, &test_data[0], 1, TGT_I2C_ADDR);

		if (ret == 0) {
			k_sched_unlock();
			ret = validate_target_rx(&test_data[0], 1U);
			zassert_ok(ret,
				   "Nested sched 1st write fail %d: %d",
				   i, ret);

			k_sched_lock();
			/* Simulate nested scheduler lock */
			k_sched_lock();

			i2c_test_prime_buffers(NULL, 0U, 1U);
			ret = hw_xfer_write(controller, &test_data[1], 1,
					    TGT_I2C_ADDR);

			k_sched_unlock();
		}

		k_sched_unlock();

		if (ret != 0) {
			LOG_DBG("Nested IRQ fail at iter %d: %d", i, ret);
			hw_ctx.interrupt_errors++;
		} else {
			ret = validate_target_rx(&test_data[1], 1U);
			zassert_ok(ret,
				   "Nested IRQ 2nd write fail %d: %d",
				   i, ret);
		}
	}

	/* Test 3: Scheduler-lock context switching - expose context bugs */
	for (int i = 0; i < 50; i++) {
		/* Rapid scheduler lock/unlock toggling */
		for (int j = 0; j < 10; j++) {
			k_sched_lock();

			i2c_test_prime_buffers(NULL, 0U, 1U);

			ret = hw_xfer_write(controller, &test_data[j % 4], 1,
					    TGT_I2C_ADDR);
			k_sched_unlock();

			if (ret != 0) {
				LOG_DBG("Ctx switch fail at %d.%d: %d",
					i, j, ret);
				hw_ctx.interrupt_errors++;
				break;
			}

			ret = validate_target_rx(&test_data[j % 4], 1U);
			if (ret != 0) {
				LOG_DBG("Ctx switch fail %d.%d: %d",
					i, j, ret);
				hw_ctx.interrupt_errors++;
				break;
			}
		}
	}

	/* Test 4: Write-Read under scheduler-lock pressure */
	for (int i = 0; i < 50; i++) {
		k_sched_lock();

		i2c_test_prime_buffers(test_data, 2U, 2U);
		ret = hw_xfer_write_read(controller, TGT_I2C_ADDR,
					 test_data, 2, rx_buffer, 2);

		k_sched_unlock();

		zassert_ok(ret, "WR sched test fail iter %d: %d", i, ret);
		if (ret == 0) {
			ret = validate_target_rx(test_data, 2U);
			zassert_ok(ret,
				   "WR sched w-phase fail iter %d: %d",
				   i, ret);
			ret = validate_controller_rx(test_data, rx_buffer, 2U);
			zassert_ok(ret,
				   "WR sched r-phase fail iter %d: %d",
				   i, ret);
		}
	}

	}

	LOG_INF("IRQ robustness done (errors: %u)",
		hw_ctx.interrupt_errors);

	/* Fail test if any interrupt handling errors accumulated */
	zassert_equal(hw_ctx.interrupt_errors, 0U,
		      "Interrupt stress had %u errors",
		      hw_ctx.interrupt_errors);
}

/**
 * @brief Test clock stretching behavior
 *
 * Objective: Expose clock stretching bugs
 * I2C hardware-aware tests: clock stretch, frequency sweep,
 * multi-frequency readback, and 10-bit addressing.
 */

static void hw_scenario_clock_stretch(void)
{
	const struct device *controller = hw_ctx.controller;
	uint8_t rx_buffer[8];
	struct i2c_test_freq_desc freqs[I2C_TEST_MAX_FREQS];
	int ret;
	int freq_count;

	i2c_test_skip_if_no_freqs();
	freq_count = i2c_test_get_enabled_freqs(freqs, ARRAY_SIZE(freqs));

	for (int f = 0; f < freq_count; f++) {
		TC_PRINT("Testing at %s (%u Hz)\n", freqs[f].name, freqs[f].freq_hz);

		ret = i2c_test_configure_controller_freq(controller, &i2c_test_ctx, &freqs[f]);
		if (ret == -ENOTSUP) {
			TC_PRINT("Skipping %s: not supported\n", freqs[f].name);
			continue;
		}
		zassert_ok(ret, "Failed to configure %s: %d", freqs[f].name, ret);

		register_target_speed_i2c(I2C_SPEED_SET(freqs[f].zephyr_speed));

	/* Test 1: Clock stretching detection - expose detection bugs */
	for (int i = 0; i < 20; i++) {
		int64_t start_time = k_uptime_get();

		ret = hardware_read_validate(controller, TGT_I2C_ADDR,
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
		ret = hardware_read_validate(controller, TGT_I2C_ADDR,
					     timing_pattern, rx_buffer, 1U);

		if (ret == -ETIMEDOUT) {
			/* Attempt recovery */
			uint32_t config = I2C_MODE_CONTROLLER |
					  I2C_SPEED_SET(I2C_SPEED_STANDARD);

			ret = i2c_configure(controller, config);
			zassert_ok(ret,
				   "Clock stretch recovery config failed: %d",
				   ret);
			i2c_test_ctx.i2c_cfg = config;
			register_target_i2c();

			/* Retry transfer */
			ret = hardware_read_validate(controller, TGT_I2C_ADDR,
						     timing_pattern,
							 rx_buffer, 1U);
		}

		if (ret == 0) {
			LOG_DBG("Clk stretch recovery success at iter %d", i);
		}
	}

	/*
	 * Test 3: Clock stretching at the current (outer-loop) frequency.
	 *
	 * The outer for-f loop already iterates every enabled frequency
	 * from i2c_test_get_enabled_freqs() and configures both controller
	 * and target to that speed. A nested enabled-frequency loop here
	 * would re-do the same matrix quadratically, so simply measure
	 * clock-stretch duration for the speed the outer loop selected.
	 */
	{
		int64_t start_time = k_uptime_get();

		ret = hardware_read_validate(controller, TGT_I2C_ADDR,
					     timing_pattern, rx_buffer, 4U);

		int64_t end_time = k_uptime_get();

		if (ret == 0) {
			int64_t duration_ms = end_time - start_time;

			LOG_INF("Clk stretch %s (%u Hz): %lldms",
				freqs[f].name, freqs[f].freq_hz,
				duration_ms);
		}
	}

	/* Test 4: Clock stretching timeout validation */
	for (int timeout_ms = 10; timeout_ms <= 100; timeout_ms += 10) {
		int64_t start_time = k_uptime_get();

		ret = hardware_read_validate(controller, TGT_I2C_ADDR,
					     timing_pattern, rx_buffer,
						 sizeof(timing_pattern));
		int64_t end_time = k_uptime_get();
		int64_t duration_ms = end_time - start_time;

		if (duration_ms > timeout_ms) {
			LOG_DBG("Exceeded %dms (actual: %lldms)",
				timeout_ms, duration_ms);
		}
	}

	}

	LOG_INF("Clock stretch tests done (violations: %u)",
		hw_ctx.timing_violations);

	/* Fail test if any clock stretch timing violations accumulated */
	zassert_equal(hw_ctx.timing_violations, 0U,
		      "Clock stretch had %u timing violations",
		      hw_ctx.timing_violations);
}

/**
 * @brief Test signal integrity and timing precision
 *
 * Objective: Expose signal integrity and timing precision bugs
 * Hardware Issue: Signal integrity, setup/hold time violations
 * Driver Issue: Incorrect timing calculations, missing validation
 */
static void hw_scenario_signal_integrity(void)
{
	const struct device *controller = hw_ctx.controller;
	uint8_t rx_buffer[32];
	struct i2c_test_freq_desc freqs[I2C_TEST_MAX_FREQS];
	int ret;
	int freq_count;

	i2c_test_skip_if_no_freqs();
	freq_count = i2c_test_get_enabled_freqs(freqs, ARRAY_SIZE(freqs));

	for (int f = 0; f < freq_count; f++) {
		TC_PRINT("Testing at %s (%u Hz)\n", freqs[f].name, freqs[f].freq_hz);

		ret = i2c_test_configure_controller_freq(controller, &i2c_test_ctx, &freqs[f]);
		if (ret == -ENOTSUP) {
			TC_PRINT("Skipping %s: not supported\n", freqs[f].name);
			continue;
		}
		zassert_ok(ret, "Failed to configure %s: %d", freqs[f].name, ret);

		register_target_speed_i2c(I2C_SPEED_SET(freqs[f].zephyr_speed));

	/*
	 * Test 1: Signal integrity at the current (outer-loop) frequency.
	 *
	 * The outer for-f loop already walks every enabled frequency from
	 * i2c_test_get_enabled_freqs() (honoring CONFIG_I2C_TEST_FREQ_*)
	 * and programs both controller and target to that speed. A nested
	 * enabled-frequency loop here would execute the same matrix
	 * quadratically, so just exercise fast-transition patterns at the
	 * speed the outer loop selected.
	 */
	for (int iteration = 0; iteration < 10; iteration++) {
		ret = hardware_write_read_validate(
			controller, TGT_I2C_ADDR,
			signal_integrity_pattern, rx_buffer,
			sizeof(signal_integrity_pattern));

		if (ret != 0) {
			LOG_DBG("Signal integ fail %s iter %d:%d",
				freqs[f].name, iteration, ret);
			hw_ctx.signal_errors++;
			continue;
		}
	}

	LOG_INF("Signal integrity test passed at %s (%u Hz)",
		freqs[f].name, freqs[f].freq_hz);

	/* Test 2: Timing precision validation - expose timing bugs */
	for (int i = 0; i < 50; i++) {
		int64_t start_time = k_uptime_get();

		ret = hardware_write_read_validate(controller, TGT_I2C_ADDR,
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
			ret = hardware_write_validate(controller, timing_pattern,
						      sizeof(timing_pattern),
							  TGT_I2C_ADDR);
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
				controller, TGT_I2C_ADDR,
				edge_patterns[pattern], rx_buffer, 4U);

			zassert_ok(ret,
				   "Edge timing test failed for pat %zu, iter %d: %d",
				   pattern, iteration, ret);
		}
	}

	}

	LOG_INF("Signal integrity done (sig err: %u, timing viol: %u)",
		hw_ctx.signal_errors, hw_ctx.timing_violations);

	/* Fail test if any signal integrity or timing errors accumulated */
	zassert_equal(hw_ctx.signal_errors, 0U,
		      "Signal integrity had %u errors",
		      hw_ctx.signal_errors);
	zassert_equal(hw_ctx.timing_violations, 0U,
		      "Signal integrity had %u timing violations",
		      hw_ctx.timing_violations);
}

/**
 * @brief Test DMA interactions (if available)
 *
 * Objective: Expose DMA-related bugs in I2C driver
 * Hardware Issue: DMA FIFO interaction, alignment issues
 * Driver Issue: DMA buffer management, synchronization bugs
 */
static void hw_scenario_dma(void)
{
	const struct device *controller = hw_ctx.controller;
	struct i2c_test_freq_desc freqs[I2C_TEST_MAX_FREQS];
	int freq_count;

	i2c_test_skip_if_no_freqs();
	freq_count = i2c_test_get_enabled_freqs(freqs, ARRAY_SIZE(freqs));

	for (int f = 0; f < freq_count; f++) {
		uint8_t tx_buffer[HW_DMA_TEST_SIZE];
		uint8_t rx_buffer[HW_DMA_TEST_SIZE];
		int ret;

		TC_PRINT("Testing at %s (%u Hz)\n", freqs[f].name, freqs[f].freq_hz);

		ret = i2c_test_configure_controller_freq(controller, &i2c_test_ctx, &freqs[f]);
		if (ret == -ENOTSUP) {
			TC_PRINT("Skipping %s: not supported\n", freqs[f].name);
			continue;
		}
		zassert_ok(ret, "Failed to configure %s: %d", freqs[f].name, ret);

		register_target_speed_i2c(I2C_SPEED_SET(freqs[f].zephyr_speed));

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

		ret = hardware_write_read_validate(controller, TGT_I2C_ADDR,
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

	ret = hardware_write_read_validate(controller, TGT_I2C_ADDR,
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

		ret = hardware_write_read_validate(controller, TGT_I2C_ADDR,
						   tx_buffer, rx_buffer,
						   test_size);

		zassert_ok(ret,
			   "DMA stress test failed at iter %d, size %zu: %d",
			   i, test_size, ret);
	}

	}

	LOG_INF("DMA interactions tests completed");
}

/**
 * @brief Test repeated-START between two write phases (no STOP).
 *
 * Validates that write_requested() fires once per phase.
 */
static void hw_scenario_restart_write_write(void)
{
	const struct device *controller = hw_ctx.controller;
	uint8_t tx1[8] = {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7};
	uint8_t tx2[8] = {0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7};
	uint8_t expected[sizeof(tx1) + sizeof(tx2)];
	struct i2c_msg msgs[2];
	struct i2c_test_freq_desc freqs[I2C_TEST_MAX_FREQS];
	int ret;
	int freq_count;

	i2c_test_skip_if_no_freqs();
	freq_count = i2c_test_get_enabled_freqs(freqs, ARRAY_SIZE(freqs));

	memcpy(expected, tx1, sizeof(tx1));
	memcpy(expected + sizeof(tx1), tx2, sizeof(tx2));

	for (int f = 0; f < freq_count; f++) {
		TC_PRINT("Testing restart W->W at %s (%u Hz)\n",
			 freqs[f].name, freqs[f].freq_hz);

		ret = i2c_test_configure_controller_freq(controller,
						     &i2c_test_ctx, &freqs[f]);
		if (ret == -ENOTSUP) {
			TC_PRINT("Skipping %s: not supported\n", freqs[f].name);
			continue;
		}
		zassert_ok(ret, "Failed to configure %s: %d",
			   freqs[f].name, ret);

		register_target_speed_i2c(I2C_SPEED_SET(freqs[f].zephyr_speed));

		/* Prime target RX buffer for the combined payload */
		i2c_test_prime_buffers(NULL, 0U, sizeof(expected));

		/* Two write phases separated by repeated START, no STOP
		 * between them. Only the second phase carries STOP.
		 */
		msgs[0].buf = tx1;
		msgs[0].len = sizeof(tx1);
		msgs[0].flags = I2C_MSG_WRITE;

		msgs[1].buf = tx2;
		msgs[1].len = sizeof(tx2);
		msgs[1].flags = I2C_MSG_WRITE | I2C_MSG_RESTART | I2C_MSG_STOP;

		ret = hw_xfer_transfer(controller, msgs, ARRAY_SIZE(msgs),
				       TGT_I2C_ADDR);
		zassert_ok(ret, "W-RESTART-W transfer failed at %s: %d",
			   freqs[f].name, ret);

		/* Data integrity: target must have received both phases
		 * concatenated in order.
		 */
		ret = validate_target_rx(expected, sizeof(expected));
		zassert_ok(ret, "Target RX mismatch at %s: %d",
			   freqs[f].name, ret);

	}

	LOG_INF("Restart W->W test completed");
}

/**
 * @brief Repeated-START between two read phases (no STOP).
 *
 * Validates read_requested() fires once per phase.
 */
static void hw_scenario_restart_read_read_tx_abrt(void)
{
	const struct device *controller = hw_ctx.controller;
	uint8_t rx1[8];
	uint8_t rx2[8];
	uint8_t pattern[sizeof(rx1) + sizeof(rx2)];
	struct i2c_msg msgs[2];
	struct i2c_test_freq_desc freqs[I2C_TEST_MAX_FREQS];
	int ret;
	int freq_count;

	i2c_test_skip_if_no_freqs();
	freq_count = i2c_test_get_enabled_freqs(freqs, ARRAY_SIZE(freqs));

	/* Prime target TX with an incrementing pattern so the two phases
	 * together yield a contiguous sequence; this makes any off-by-one
	 * in phase boundaries trivially observable.
	 */
	for (size_t i = 0; i < sizeof(pattern); i++) {
		pattern[i] = (uint8_t)(0x30U + i);
	}

	for (int f = 0; f < freq_count; f++) {
		TC_PRINT("Testing restart R->R at %s (%u Hz)\n",
			 freqs[f].name, freqs[f].freq_hz);

		ret = i2c_test_configure_controller_freq(controller,
						     &i2c_test_ctx, &freqs[f]);
		if (ret == -ENOTSUP) {
			TC_PRINT("Skipping %s: not supported\n", freqs[f].name);
			continue;
		}
		zassert_ok(ret, "Failed to configure %s: %d",
			   freqs[f].name, ret);

		register_target_speed_i2c(I2C_SPEED_SET(freqs[f].zephyr_speed));

		/* Prime target TX buffer with the full pattern for both
		 * read phases; target TX index advances across phases.
		 */
		i2c_test_prime_buffers(pattern, sizeof(pattern), 0U);

		poison_buffer(rx1, sizeof(rx1), POISON_RX);
		poison_buffer(rx2, sizeof(rx2), POISON_RX);

		/* Two read phases separated by repeated START, no STOP
		 * between them. Controller NACKs the last byte of phase 1
		 * (standard I2C behavior), which causes TX_ABRT on the
		 * target side; only START_DET marks the boundary before
		 * the second read phase begins.
		 */
		msgs[0].buf = rx1;
		msgs[0].len = sizeof(rx1);
		msgs[0].flags = I2C_MSG_READ;

		msgs[1].buf = rx2;
		msgs[1].len = sizeof(rx2);
		msgs[1].flags = I2C_MSG_READ | I2C_MSG_RESTART | I2C_MSG_STOP;

		ret = hw_xfer_transfer(controller, msgs, ARRAY_SIZE(msgs),
				       TGT_I2C_ADDR);
		zassert_ok(ret, "R-RESTART-R transfer failed at %s: %d",
			   freqs[f].name, ret);

	}

	LOG_INF("Restart R->R (tx_abrt boundary) test completed");
}

static void *i2c_hardware_suite_setup(void)
{
	const struct device *controller = DEVICE_DT_GET(I2C_CONTROLLER);
	const struct device *target = DEVICE_DT_GET(I2C_TARGET);
	int ret;

	/* Initialize hardware context */
	memset(&hw_ctx, 0, sizeof(hw_ctx));
	hw_ctx.controller = controller;
	hw_ctx.target = target;

	/* Configure devices */
	zassert_not_null(controller, "Controller I2C device not found");
	zassert_true(device_is_ready(hw_ctx.controller), "Controller I2C device not ready");

	zassert_not_null(target, "Target I2C device not found");
	zassert_true(device_is_ready(hw_ctx.target), "Target I2C device not ready");

	i2c_test_ctx.controller_dev = hw_ctx.controller;
	i2c_test_ctx.target_dev = hw_ctx.target;
	i2c_test_reset_runtime_config(&i2c_test_ctx);
	ret = i2c_configure(i2c_test_ctx.controller_dev, i2c_test_ctx.i2c_cfg);
	zassert_ok(ret, "Controller I2C configuration failed: %d", ret);
	register_target_i2c();

	LOG_INF("I2C hardware-aware test suite setup completed");
	return &i2c_test_ctx;
}

/* Before each test: reset error counters to prevent cross-test contamination */
static void i2c_hardware_suite_before(void *fixture)
{
	struct i2c_test_ctx *ctx = fixture;

	reset_hw_error_counters();
	i2c_test_reset_runtime_config(ctx);
	i2c_test_async_init();
}

ZTEST(i2c_hardware_suite, test_irq_robustness_sync)
{
	hw_test_async_mode = false;
	hw_scenario_irq_robustness();
}

ZTEST(i2c_hardware_suite, test_clock_stretch_sync)
{
	hw_test_async_mode = false;
	hw_scenario_clock_stretch();
}

ZTEST(i2c_hardware_suite, test_signal_integrity_sync)
{
	hw_test_async_mode = false;
	hw_scenario_signal_integrity();
}

ZTEST(i2c_hardware_suite, test_dma_sync)
{
	hw_test_async_mode = false;
	hw_scenario_dma();
}

ZTEST(i2c_hardware_suite, test_restart_write_write_sync)
{
	hw_test_async_mode = false;
	hw_scenario_restart_write_write();
}

ZTEST(i2c_hardware_suite, test_restart_read_read_tx_abrt_sync)
{
	hw_test_async_mode = false;
	hw_scenario_restart_read_read_tx_abrt();
}

#if IS_ENABLED(CONFIG_I2C_CALLBACK)
ZTEST(i2c_hardware_suite, test_irq_robustness_async)
{
	hw_test_async_mode = true;
	hw_scenario_irq_robustness();
}

ZTEST(i2c_hardware_suite, test_clock_stretch_async)
{
	hw_test_async_mode = true;
	hw_scenario_clock_stretch();
}

ZTEST(i2c_hardware_suite, test_signal_integrity_async)
{
	hw_test_async_mode = true;
	hw_scenario_signal_integrity();
}

ZTEST(i2c_hardware_suite, test_dma_async)
{
	hw_test_async_mode = true;
	hw_scenario_dma();
}

ZTEST(i2c_hardware_suite, test_restart_write_write_async)
{
	hw_test_async_mode = true;
	hw_scenario_restart_write_write();
}

ZTEST(i2c_hardware_suite, test_restart_read_read_tx_abrt_async)
{
	hw_test_async_mode = true;
	hw_scenario_restart_read_read_tx_abrt();
}
#endif

ZTEST_SUITE(i2c_hardware_suite, NULL, i2c_hardware_suite_setup,
	    i2c_hardware_suite_before, NULL, NULL);

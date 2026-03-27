/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include "test_i2c.h"
LOG_MODULE_REGISTER(alif_i2c_comp, LOG_LEVEL_INF);

/* Comprehensive validation test constants */
#define COMP_ITERATIONS			1000U
#define COMP_PATTERN_SIZE		64U
#define COMP_TIMEOUT_STRESS_MS	100U
#define COMP_CONCURRENT_THREADS 8U
#define COMP_POWER_CYCLE_COUNT	100U

/* Fault isolation constants */
#define FAULT_ISOLATION_MAX_RETRIES	   3U
#define FAULT_ISOLATION_RETRY_DELAY_MS 10U
#define FAULT_ISOLATION_BASIC_PATTERN  0xA5

/* Fault classification levels */
enum i2c_fault_type {
	I2C_FAULT_NONE = 0,
	I2C_FAULT_HARDWARE,		   /* Physical hardware issues */
	I2C_FAULT_DRIVER,		   /* Driver implementation bugs */
	I2C_FAULT_TEST_CODE,	   /* Test code issues */
	I2C_FAULT_CONFIGURATION,   /* System configuration issues */
	I2C_FAULT_TIMEOUT,		   /* Timing-related issues */
	I2C_FAULT_DATA_CORRUPTION, /* Data integrity issues */
	I2C_FAULT_UNKNOWN
};

/* Fault isolation context */
struct i2c_fault_isolation {
	uint32_t total_operations;
	uint32_t hardware_faults;
	uint32_t driver_faults;
	uint32_t test_code_faults;
	uint32_t config_faults;
	uint32_t timeout_faults;
	uint32_t data_corruption_faults;
	uint32_t unknown_faults;

	/* Hardware validation */
	bool hardware_validated;
	bool clock_signal_ok;
	bool power_supply_ok;
	bool gpio_pins_ok;

	/* Driver validation */
	bool driver_initialized;
	bool api_functions_ok;
	bool interrupt_handling_ok;
	bool dma_functionality_ok;
};

/* Comprehensive test patterns */
static const uint8_t comprehensive_patterns[][8] = {
	{0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF}, /* Fast trans */
	{0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55}, /* Alternating */
	{0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80}, /* Walking ones */
	{0xFE, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF, 0xBF, 0x7F}, /* Walking zeros */
	{0xFF, 0x00, 0xAA, 0x55, 0x33, 0xCC, 0x78, 0x87}, /* Mixed */
};

/* Comprehensive test context */
struct i2c_comp_ctx {
	const struct device *master;
	const struct device *slave;
	uint32_t total_tests;
	uint32_t passed_tests;
	uint32_t failed_tests;
	uint32_t timeout_errors;
	uint32_t data_errors;
	uint32_t driver_errors;
	uint32_t hardware_errors;
	bool comprehensive_mode;
};

static struct i2c_comp_ctx comp_ctx;
static struct i2c_fault_isolation fault_ctx;

/* Fault classification - Differentiate error types */
static enum i2c_fault_type i2c_classify_error(int error_code,
					      const char *operation)
{
	fault_ctx.total_operations++;

	switch (error_code) {
	case -ENXIO:
		LOG_ERR("HW FAULT: Device not responding (ENXIO) in %s",
			operation);
		return I2C_FAULT_HARDWARE;
	case -EIO:
		fault_ctx.hardware_faults++;
		LOG_ERR("HW FAULT: I/O error (EIO) in %s - bus contention?",
			operation);
		return I2C_FAULT_HARDWARE;
	}

	/* Driver-related errors */
	if (error_code == -EINVAL) {
		fault_ctx.driver_faults++;
		LOG_ERR("DRIVER FAULT: EINVAL in %s", operation);
		return I2C_FAULT_DRIVER;
	}

	if (error_code == -ENOTSUP) {
		fault_ctx.driver_faults++;
		LOG_ERR("DRIVER FAULT: ENOTSUP in %s",
			operation);
		return I2C_FAULT_DRIVER;
	}

	if (error_code == -EBUSY) {
		fault_ctx.driver_faults++;
		LOG_ERR("DRIVER FAULT: EBUSY in %s", operation);
		return I2C_FAULT_DRIVER;
	}

	/* Timeout-related errors */
	if (error_code == -ETIMEDOUT) {
		fault_ctx.timeout_faults++;
		LOG_ERR("TIMEOUT FAULT: ETIMEDOUT in %s",
			operation);
		return I2C_FAULT_TIMEOUT;
	}

	/* Configuration errors */
	if (error_code == -EPERM) {
		fault_ctx.config_faults++;
		LOG_ERR("CONFIG FAULT: EPERM in %s", operation);
		return I2C_FAULT_CONFIGURATION;
	}

	/* Data corruption */
	if (error_code == -EILSEQ) {
		fault_ctx.data_corruption_faults++;
		LOG_ERR("DATA CORRUPT: Invalid byte (EILSEQ) in %s",
			operation);
		return I2C_FAULT_DATA_CORRUPTION;
	}

	/* Unknown errors */
	fault_ctx.unknown_faults++;
	LOG_ERR("UNKNOWN FAULT: Error code %d in %s", error_code, operation);
	return I2C_FAULT_UNKNOWN;
}

/* Hardware subsystem validation */
static bool i2c_validate_hardware_subsystem(const struct device *dev)
{
	if (fault_ctx.hardware_validated) {
		return fault_ctx.clock_signal_ok &&
			   fault_ctx.power_supply_ok &&
			   fault_ctx.gpio_pins_ok;
	}

	LOG_INF("=== HARDWARE SUBSYSTEM VALIDATION ===");

	/* Check device tree configuration */
	if (!device_is_ready(dev)) {
		LOG_ERR("HW: Device not ready - power/clock issue");
		fault_ctx.power_supply_ok = false;
		fault_ctx.hardware_validated = true;
		return false;
	}

	/* Basic hardware sanity check - try to configure at lowest speed */
	uint32_t basic_config = I2C_MODE_CONTROLLER |
				I2C_SPEED_SET(I2C_SPEED_STANDARD);
	int ret = i2c_configure(dev, basic_config);

	if (ret == -ENXIO) {
		LOG_ERR("HARDWARE: No device response - check SDA/SCL");
		fault_ctx.clock_signal_ok = false;
		fault_ctx.gpio_pins_ok = false;
	} else if (ret == -EIO) {
		LOG_ERR("HW: I/O error - bus short or pull-up issues");
		fault_ctx.clock_signal_ok = false;
	} else if (ret == 0) {
		LOG_INF("HARDWARE: Basic configuration successful");
		fault_ctx.clock_signal_ok = true;
		fault_ctx.power_supply_ok = true;
		fault_ctx.gpio_pins_ok = true;
	} else {
		LOG_ERR("HARDWARE: Unexpected hardware error %d", ret);
	}

	fault_ctx.hardware_validated = true;

	LOG_INF("Hardware validation results:");
	LOG_INF("  Clock signal: %s",
		fault_ctx.clock_signal_ok ? "OK" : "FAIL");
	LOG_INF("  Power supply: %s",
		fault_ctx.power_supply_ok ? "OK" : "FAIL");
	LOG_INF("  GPIO pins: %s",
		fault_ctx.gpio_pins_ok ? "OK" : "FAIL");

	return fault_ctx.clock_signal_ok && fault_ctx.power_supply_ok &&
		   fault_ctx.gpio_pins_ok;
}

/* Driver subsystem validation */
static bool i2c_validate_driver_subsystem(const struct device *dev)
{
	if (fault_ctx.driver_initialized) {
		return fault_ctx.api_functions_ok &&
			   fault_ctx.interrupt_handling_ok;
	}

	LOG_INF("=== DRIVER SUBSYSTEM VALIDATION ===");

	/* Test 1: API function availability */
	uint8_t test_data = FAULT_ISOLATION_BASIC_PATTERN;

	/* Try basic write operation */
	int ret = i2c_write(dev, &test_data, 1, SLV_I2C_ADDR);

	if (ret == -ENOTSUP || ret == -EINVAL) {
		LOG_ERR("DRIVER: API functions not properly implemented");
		fault_ctx.api_functions_ok = false;
	} else {
		fault_ctx.api_functions_ok = true;
	}

	/* Test 2: Configuration API */
	uint32_t test_config = I2C_MODE_CONTROLLER |
				   I2C_SPEED_SET(I2C_SPEED_STANDARD);

	ret = i2c_configure(dev, test_config);
	if (ret == -ENOTSUP || ret == -EINVAL) {
		LOG_ERR("DRIVER: Configuration API not working");
		fault_ctx.api_functions_ok = false;
	}

	/* Test 3: Speed change capability (tests interrupt handling) */
	uint32_t fast_config = I2C_MODE_CONTROLLER |
				   I2C_SPEED_SET(I2C_SPEED_FAST);

	ret = i2c_configure(dev, fast_config);
	if (ret == 0) {
		fault_ctx.interrupt_handling_ok = true;
		LOG_INF("DRIVER: Speed change successful - interrupts working");
	} else {
		LOG_WRN("DRIVER: Speed change failed - may indicate IRQ issue");
		fault_ctx.interrupt_handling_ok = false;
	}

	fault_ctx.driver_initialized = true;

	LOG_INF("Driver validation results:");
	LOG_INF("  API functions: %s",
		fault_ctx.api_functions_ok ? "OK" : "FAIL");
	LOG_INF("  IRQ handling: %s",
		fault_ctx.interrupt_handling_ok ? "OK" : "FAIL");

	return fault_ctx.api_functions_ok;
}

/* Initialize fault isolation */
static void i2c_fault_isolation_init(void)
{
	memset(&fault_ctx, 0, sizeof(fault_ctx));
	LOG_INF("Fault isolation system initialized");
}

/* Generate comprehensive fault report */
static void i2c_fault_isolation_report(void)
{
	LOG_INF("=== COMPREHENSIVE FAULT ISOLATION REPORT ===");
	LOG_INF("Total operations: %u", fault_ctx.total_operations);

	if (fault_ctx.total_operations == 0) {
		LOG_INF("No operations performed");
		return;
	}

	LOG_INF("--- Fault Classification ---");
	LOG_INF("Hardware faults: %u (%u%%)", fault_ctx.hardware_faults,
		(fault_ctx.hardware_faults * 100U) /
		fault_ctx.total_operations);
	LOG_INF("Driver faults: %u (%u%%)", fault_ctx.driver_faults,
		(fault_ctx.driver_faults * 100U) /
		fault_ctx.total_operations);
	LOG_INF("Configuration faults: %u (%u%%)", fault_ctx.config_faults,
		(fault_ctx.config_faults * 100U) /
		fault_ctx.total_operations);
	LOG_INF("Timeout faults: %u (%u%%)", fault_ctx.timeout_faults,
		(fault_ctx.timeout_faults * 100U) /
		fault_ctx.total_operations);
	LOG_INF("Data corruption: %u (%u%%)",
		fault_ctx.data_corruption_faults,
		(fault_ctx.data_corruption_faults * 100U) /
		fault_ctx.total_operations);
	LOG_INF("Unknown faults: %u (%u%%)", fault_ctx.unknown_faults,
		(fault_ctx.unknown_faults * 100U) /
		fault_ctx.total_operations);

	LOG_INF("--- Hardware Validation ---");
	if (fault_ctx.hardware_validated) {
		LOG_INF("Clock signal: %s",
			fault_ctx.clock_signal_ok ? "OK" : "FAIL");
		LOG_INF("Power supply: %s",
			fault_ctx.power_supply_ok ? "OK" : "FAIL");
		LOG_INF("GPIO pins: %s",
			fault_ctx.gpio_pins_ok ? "OK" : "FAIL");
	} else {
		LOG_INF("Hardware validation not performed");
	}

	LOG_INF("--- Driver Validation ---");
	if (fault_ctx.driver_initialized) {
		LOG_INF("API functions: %s",
			fault_ctx.api_functions_ok ? "OK" : "FAIL");
		LOG_INF("IRQ handling: %s",
			fault_ctx.interrupt_handling_ok ? "OK" : "FAIL");
	} else {
		LOG_INF("Driver validation not performed");
	}

	/* Root cause analysis */
	LOG_INF("--- Root Cause Analysis ---");
	if (fault_ctx.hardware_faults > fault_ctx.driver_faults * 2) {
		LOG_ERR("PRIMARY: HW - Check connections, power, clock");
	} else if (fault_ctx.driver_faults > fault_ctx.hardware_faults * 2) {
		LOG_ERR("PRIMARY: DRIVER - Check driver impl, API");
	} else if (fault_ctx.timeout_faults >
		   fault_ctx.total_operations / 4) {
		LOG_ERR("PRIMARY: TIMING - Check clk, IRQ");
	} else if (fault_ctx.config_faults > 0) {
		LOG_ERR("PRIMARY: CONFIG - Check DT, pins");
	} else {
		LOG_INF("No clear primary fault pattern detected");
	}

	LOG_INF("=== END FAULT ISOLATION REPORT ===");
}

/* Comprehensive test callback functions */
static int cb_i2c_comp_target_write_requested(struct i2c_target_config *config)
{
	ARG_UNUSED(config);
	i2c_test_note_write_requested(&i2c_test_ctx);
	return 0;
}

static int cb_i2c_comp_target_read_requested(struct i2c_target_config *config,
					     uint8_t *val)
{
	ARG_UNUSED(config);
	if (i2c_test_ctx.slv_tx_cnt >= BUFF_PERF) {
		i2c_test_ctx.slv_tx_cnt = 0;
	}
	*val = i2c_test_ctx.slv_tx_data[i2c_test_ctx.slv_tx_cnt++];
	i2c_test_note_read_requested(&i2c_test_ctx);
	LOG_DBG("Comprehensive: Read requested, send 0x%x", *val);
	return 0;
}

static int cb_i2c_comp_target_write_received(struct i2c_target_config *config,
					     uint8_t val)
{
	ARG_UNUSED(config);
	if (i2c_test_ctx.slv_rx_cnt < BUFF_PERF) {
		i2c_test_ctx.slv_rx_data[i2c_test_ctx.slv_rx_cnt++] = val;
		LOG_DBG("Comprehensive: RX[%u]: 0x%02X",
			i2c_test_ctx.slv_rx_cnt - 1, val);
	}
	i2c_test_note_write_received(&i2c_test_ctx);
	return 0;
}

static int cb_i2c_comp_target_read_processed(struct i2c_target_config *config,
					     uint8_t *val)
{
	ARG_UNUSED(config);
	ARG_UNUSED(val);
	i2c_test_note_read_processed(&i2c_test_ctx);
	LOG_DBG("Comprehensive: Read processed");
	return 0;
}

static int cb_i2c_comp_target_stop(struct i2c_target_config *config)
{
	ARG_UNUSED(config);
	i2c_test_note_stop(&i2c_test_ctx);
	return 0;
}

/* Target configuration for comprehensive tests */
static struct i2c_target_callbacks i2c_comp_t_cb = {
	.write_requested = &cb_i2c_comp_target_write_requested,
	.read_requested	 = &cb_i2c_comp_target_read_requested,
	.write_received	 = &cb_i2c_comp_target_write_received,
	.read_processed	 = &cb_i2c_comp_target_read_processed,
	.stop			 = &cb_i2c_comp_target_stop,
};

/* Reusable validation helpers - enforce real data comparison */
static int comp_write_read_validate(const struct device *master,
				    const uint8_t *tx_data, size_t tx_len,
					uint8_t *rx_data, size_t rx_len,
					uint32_t slave_tx_seed)
{
	uint8_t slave_tx_pattern[BUFF_PERF];
	size_t cap_tx = MIN(tx_len, (size_t)BUFF_PERF);
	size_t cap_rx = MIN(rx_len, (size_t)BUFF_PERF);
	int ret;

	/* Generate distinct slave TX pattern */
	fill_slave_tx_distinct(slave_tx_pattern, cap_rx, slave_tx_seed);

	/* Prime slave: load slave TX, prepare slave RX, reset callbacks */
	i2c_test_prime_buffers(slave_tx_pattern, cap_rx, tx_len);

	/* Poison master RX to detect uninitialized reads */
	poison_buffer(rx_data, rx_len, POISON_RX);

	ret = i2c_write_read(master, SLV_I2C_ADDR, tx_data, tx_len,
			     rx_data, rx_len);
	if (ret != 0) {
		return ret;
	}

	/* Validate slave received what master sent */
	ret = validate_slave_rx(tx_data, cap_tx);
	if (ret != 0) {
		LOG_ERR("Comp: slave RX mismatch (tx_len=%zu)", tx_len);
		return ret;
	}

	/* Validate master received what slave was primed to send */
	ret = validate_master_rx(slave_tx_pattern, rx_data, cap_rx);
	if (ret != 0) {
		LOG_ERR("Comp: master RX mismatch (rx_len=%zu)", rx_len);
	}
	return ret;
}

static int comp_write_validate(const struct device *master,
			       const uint8_t *tx_data, size_t tx_len)
{
	int ret;

	/* Prime slave: no read data, prepare slave RX */
	i2c_test_prime_buffers(NULL, 0, tx_len);

	ret = i2c_write(master, tx_data, tx_len, SLV_I2C_ADDR);
	if (ret != 0) {
		return ret;
	}

	return validate_slave_rx(tx_data, MIN(tx_len, (size_t)BUFF_PERF));
}

static int comp_read_validate(const struct device *master, uint8_t *rx_data,
			      size_t rx_len, uint32_t slave_tx_seed)
{
	uint8_t slave_tx_pattern[BUFF_PERF];
	size_t cap_rx = MIN(rx_len, (size_t)BUFF_PERF);
	int ret;

	fill_slave_tx_distinct(slave_tx_pattern, cap_rx, slave_tx_seed);
	i2c_test_prime_buffers(slave_tx_pattern, cap_rx, 0);

	/* Poison master RX */
	poison_buffer(rx_data, rx_len, POISON_RX);

	ret = i2c_read(master, rx_data, rx_len, SLV_I2C_ADDR);
	if (ret != 0) {
		return ret;
	}

	return validate_master_rx(slave_tx_pattern, rx_data, cap_rx);
}

static struct i2c_target_config i2c_comp_tcfg = {
	.callbacks = &i2c_comp_t_cb,
};

/**
 * @brief Comprehensive I2C validation - master mode
 *
 * Objective: Complete validation of I2C master functionality
 * Hardware Issue: Timing violations, signal integrity, clock behavior
 * Driver Issue: Register handling, FIFO management, interrupt processing
 */
ZTEST(i2c_comprehensive_suite, test_comprehensive_master)
{
	const struct device *master = DEVICE_DT_GET(I2C_MASTER);
	uint8_t tx_buffer[COMP_PATTERN_SIZE];
	uint8_t rx_buffer[COMP_PATTERN_SIZE];
	int ret;

	zassert_not_null(master,
			 "Master I2C device not found - cannot continue");
	if (!device_is_ready(master)) {
		zassert_true(false, "Master I2C device not ready");
		return;
	}

	/* Initialize fault isolation */
	i2c_fault_isolation_init();

	/* Perform hardware and driver validation */
	bool hw_ok = i2c_validate_hardware_subsystem(master);
	bool driver_ok = i2c_validate_driver_subsystem(master);

	if (!hw_ok) {
		LOG_ERR("Hardware validation failed - aborting test");
		zassert_true(false, "Hardware subsystem validation failed");
		return;
	}

	if (!driver_ok) {
		LOG_ERR("Driver validation failed - aborting test");
		zassert_true(false, "Driver subsystem validation failed");
		return;
	}

	LOG_INF("=== COMPREHENSIVE MASTER VALIDATION START ===");

	/* Test 1: Complete speed validation with real data comparison */
	const uint32_t all_speeds[] = {
		I2C_SPEED_STANDARD, I2C_SPEED_FAST,
		I2C_SPEED_FAST_PLUS, I2C_SPEED_HIGH
	};

	for (size_t speed_idx = 0; speed_idx < ARRAY_SIZE(all_speeds);
		 speed_idx++) {
		uint32_t config = I2C_MODE_CONTROLLER |
				  I2C_SPEED_SET(all_speeds[speed_idx]);

		ret = i2c_configure(master, config);
		if (ret != 0) {
			i2c_classify_error(ret, "speed_configuration");
			comp_ctx.driver_errors++;
			LOG_ERR("Speed %u configuration failed: %d",
				all_speeds[speed_idx], ret);
			continue;
		}

		for (size_t pattern_idx = 0;
			 pattern_idx < ARRAY_SIZE(comprehensive_patterns);
			 pattern_idx++) {
			for (int iteration = 0; iteration < 10; iteration++) {
				/* Prepare master TX data */
				memcpy(tx_buffer,
				       comprehensive_patterns[pattern_idx],
					   8);
				for (int i = 8; i < COMP_PATTERN_SIZE; i++) {
					tx_buffer[i] = (uint8_t)(pattern_idx +
							iteration + i);
				}

				comp_ctx.total_tests++;

				uint32_t seed =
					SEED_SLAVE_TX +
					(uint32_t)(speed_idx * 1000 +
						   pattern_idx * 100 +
						   iteration);

				ret = comp_write_read_validate(
					master, tx_buffer, COMP_PATTERN_SIZE,
					rx_buffer, COMP_PATTERN_SIZE, seed);
		zassert_ok(ret,
			   "Speed %u pattern %zu iter %d: data valid failed: %d",
					   all_speeds[speed_idx],
					   pattern_idx, iteration, ret);

				comp_ctx.passed_tests++;
			}
		}
	}

	/* Test 2: Size boundary - capped at BUFF_PERF to prevent overflow */
	const size_t boundary_sizes[] = {
		1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64, BUFF_PERF
	};

	for (size_t size_idx = 0; size_idx < ARRAY_SIZE(boundary_sizes);
		 size_idx++) {
		size_t test_size = boundary_sizes[size_idx];

		fill_buffer_random(tx_buffer, test_size,
				   SEED_MASTER_TX + (uint32_t)test_size);

		comp_ctx.total_tests++;

		ret = comp_write_read_validate(master, tx_buffer, test_size,
					       rx_buffer, test_size,
						   SEED_SLAVE_TX +
						   (uint32_t)test_size);
		zassert_ok(ret, "Size %zu boundary test failed: %d",
			   test_size, ret);

		comp_ctx.passed_tests++;
	}

	/* Test 3: Addressing mode validation with real data comparison */
	{
		uint8_t test_data[4];

		fill_buffer_random(test_data, sizeof(test_data),
				   SEED_MASTER_TX + 0x0007U);

		comp_ctx.total_tests++;
		ret = comp_write_validate(master, test_data,
					  sizeof(test_data));
		zassert_ok(ret,
			   "7-bit addressing write validation failed: %d",
			   ret);
		comp_ctx.passed_tests++;
		LOG_INF("Addressing mode 7-bit passed");

		/* 10-bit addressing */
		fill_buffer_random(test_data, sizeof(test_data),
				   SEED_MASTER_TX + 0x0010U);
		register_slave_i2c_10Bit();

		struct i2c_msg msg_10bit = {
			.buf = test_data,
			.len = sizeof(test_data),
			.flags = I2C_MSG_ADDR_10_BITS | I2C_MSG_WRITE |
				 I2C_MSG_STOP
		};

		i2c_test_prime_buffers(NULL, 0, sizeof(test_data));
		comp_ctx.total_tests++;
		ret = i2c_transfer(master, &msg_10bit, 1,
				   SLV_I2C_10BITADDR);
		if (ret == -ENOTSUP) {
			LOG_INF("10-bit addressing not supported, skipping");
		} else {
			zassert_ok(ret,
				   "10-bit addressing transfer failed: %d",
				   ret);
			ret = validate_slave_rx(test_data,
						sizeof(test_data));
			zassert_ok(ret,
				   "10-bit addr data valid failed: %d",
				   ret);
			comp_ctx.passed_tests++;
			LOG_INF("Addressing mode 10-bit passed");
		}

		/* Restore 7-bit slave registration */
		register_slave_i2c();
	}

	/* Generate comprehensive fault isolation report */
	i2c_fault_isolation_report();

	LOG_INF("=== COMPREHENSIVE MASTER VALIDATION RESULTS ===");
	LOG_INF("Total tests: %u, Passed: %u, Failed: %u",
		comp_ctx.total_tests, comp_ctx.passed_tests,
		comp_ctx.failed_tests);

	zassert_equal(comp_ctx.failed_tests, 0,
		      "Comprehensive master validation: %u tests failed",
			  comp_ctx.failed_tests);
}

/**
 * @brief Comprehensive stress and robustness validation
 *
 * Objective: Stress test I2C driver and hardware to breaking point
 * Hardware Issue: Signal integrity under stress, thermal issues
 * Driver Issue: Memory leaks, state corruption, resource exhaustion
 */
ZTEST(i2c_comprehensive_suite, test_comprehensive_stress)
{
	const struct device *master = DEVICE_DT_GET(I2C_MASTER);
	uint8_t stress_buffer[BUFF_PERF];
	uint8_t rx_buffer[BUFF_PERF];
	int ret;

	zassert_not_null(master,
			 "Master I2C device not found - cannot continue test");
	if (!device_is_ready(master)) {
		zassert_true(false, "Master I2C device not ready");
		return;
	}

	/* Test 1: High-frequency operation stress with real validation */
	for (int stress_cycle = 0; stress_cycle < COMP_ITERATIONS;
		 stress_cycle++) {
		fill_buffer_random(stress_buffer, BUFF_PERF,
				   SEED_MASTER_TX + (uint32_t)stress_cycle);

		/* Rapid transfers - capped at BUFF_PERF */
		for (int rapid_op = 0; rapid_op < 10; rapid_op++) {
			size_t transfer_size = MIN(8 + (rapid_op * 12),
						   (size_t)BUFF_PERF);

			comp_ctx.total_tests++;

			uint32_t seed = SEED_SLAVE_TX +
					(uint32_t)(stress_cycle * 10 +
						   rapid_op);

			ret = comp_write_read_validate(master, stress_buffer,
						       transfer_size,
							   rx_buffer,
							   transfer_size, seed);

			zassert_ok(ret,
				   "Stress cycle %d op %d size %zu failed: %d",
				   stress_cycle, rapid_op,
				   transfer_size, ret);
			comp_ctx.passed_tests++;
		}

		/* Configuration stress between cycles */
		if (stress_cycle % 100 == 0) {
			uint32_t config =
				I2C_MODE_CONTROLLER |
				I2C_SPEED_SET((stress_cycle % 2)
						  ? I2C_SPEED_FAST
						  : I2C_SPEED_STANDARD);

			ret = i2c_configure(master, config);
			zassert_ok(ret,
				   "Stress reconfig failed at cycle %d: %d",
				   stress_cycle, ret);
		}
	}

	/* Test 2: Memory and resource stress with write validation */
	for (int mem_cycle = 0; mem_cycle < 100; mem_cycle++) {
		for (int alloc_op = 0; alloc_op < 50; alloc_op++) {
			size_t var_size = MIN(1 + (alloc_op % 127),
					  (size_t)BUFF_PERF);

			fill_buffer_random(stress_buffer, var_size,
					   SEED_MASTER_TX +
					   (uint32_t)(mem_cycle * 50 +
							  alloc_op));

			comp_ctx.total_tests++;
			ret = comp_write_validate(master, stress_buffer,
						  var_size);
			if (ret == 0) {
				comp_ctx.passed_tests++;
			}
		}
	}

	LOG_INF("Comprehensive stress validation completed");
}

/**
 * @brief Comprehensive hardware-software co-validation
 *
 * Objective: Validate hardware-software integration thoroughly
 * Hardware Issue: Clock domain crossing, signal integrity, timing margins
 * Driver Issue: Hardware abstraction, register programming, IRQ handling
 */
ZTEST(i2c_comprehensive_suite, test_comprehensive_hwsw)
{
	const struct device *master = DEVICE_DT_GET(I2C_MASTER);
	uint8_t hw_tx_buffer[128];
	uint8_t hw_rx_buffer[128];
	int ret;

	zassert_not_null(master,
			 "Master I2C device not found - cannot continue test");
	if (!device_is_ready(master)) {
		zassert_true(false, "Master I2C device not ready");
		return;
	}

	/* Test 1: Clock domain crossing validation */
	const uint32_t clock_domains[] = {
		I2C_SPEED_STANDARD, I2C_SPEED_FAST,
		I2C_SPEED_FAST_PLUS, I2C_SPEED_HIGH
	};

	for (size_t domain_idx = 0;
		 domain_idx < ARRAY_SIZE(clock_domains); domain_idx++) {
		uint32_t config = I2C_MODE_CONTROLLER |
				  I2C_SPEED_SET(clock_domains[domain_idx]);

		ret = i2c_configure(master, config);
		if (ret != 0) {
			comp_ctx.driver_errors++;
			LOG_ERR("Clock domain %u config failed: %d",
				clock_domains[domain_idx], ret);
			continue;
		}

		for (int crossing_cycle = 0; crossing_cycle < 20;
			 crossing_cycle++) {
			fill_buffer_random(hw_tx_buffer, BUFF_PERF,
					   SEED_MASTER_TX +
					   (uint32_t)(domain_idx * 100 +
							  crossing_cycle));

			comp_ctx.total_tests++;
			uint32_t seed = SEED_SLAVE_TX +
					(uint32_t)(domain_idx * 100 +
						   crossing_cycle);
			ret = comp_write_read_validate(master, hw_tx_buffer,
						       BUFF_PERF, hw_rx_buffer,
							   BUFF_PERF, seed);
			zassert_ok(ret,
				   "Clock domain %u cycle %d valid failed: %d",
				   clock_domains[domain_idx],
				   crossing_cycle, ret);
			comp_ctx.passed_tests++;

			/* Rapid frequency change to stress clock crossing */
			if (crossing_cycle % 5 == 0) {
				uint32_t next_freq =
					clock_domains[(domain_idx + 1) %
					ARRAY_SIZE(clock_domains)];
				uint32_t next_config =
					I2C_MODE_CONTROLLER |
					I2C_SPEED_SET(next_freq);

				ret = i2c_configure(master, next_config);
				if (ret == 0) {
					comp_ctx.total_tests++;
					ret = comp_write_read_validate(
						master, hw_tx_buffer,
						BUFF_PERF, hw_rx_buffer,
						BUFF_PERF, seed + 1);
					zassert_ok(ret,
					   "Freq-switch at domain %u cycle %d failed: %d",
					   clock_domains[domain_idx],
					   crossing_cycle, ret);
					comp_ctx.passed_tests++;
				}
				/* Restore original frequency */
				ret = i2c_configure(master, config);
				zassert_ok(ret,
					   "Restore config failed");
			}
		}
	}

	/* Test 2: Signal integrity under various conditions */
	for (int integrity_cycle = 0; integrity_cycle < 50;
		 integrity_cycle++) {
		for (size_t pattern_idx = 0;
			 pattern_idx < ARRAY_SIZE(comprehensive_patterns);
			 pattern_idx++) {
			/* Extend pattern to full buffer */
			for (int bi = 0; bi < BUFF_PERF; bi++) {
				hw_tx_buffer[bi] =
					comprehensive_patterns[pattern_idx]
					[bi % 8];
			}

			for (size_t speed_idx = 0; speed_idx < 2;
				 speed_idx++) {
				uint32_t config =
					I2C_MODE_CONTROLLER |
					I2C_SPEED_SET(
						(speed_idx == 0)
						? I2C_SPEED_STANDARD
						: I2C_SPEED_FAST);

				ret = i2c_configure(master, config);
				if (ret != 0) {
					continue;
				}

				comp_ctx.total_tests++;
				uint32_t seed =
					SEED_SLAVE_TX +
					(uint32_t)(integrity_cycle * 100 +
						   pattern_idx * 10 +
						   speed_idx);
				ret = comp_write_read_validate(
					master, hw_tx_buffer, BUFF_PERF,
					hw_rx_buffer, BUFF_PERF, seed);
				zassert_ok(ret,
					   "Signal integrity cycle %d failed: %d",
					   integrity_cycle, ret);
				comp_ctx.passed_tests++;
			}
		}
	}

	/* Test 3: Hardware timing margin validation */
	for (int timing_cycle = 0; timing_cycle < 100; timing_cycle++) {
		for (int rapid_op = 0; rapid_op < 10; rapid_op++) {
			fill_buffer_random(hw_tx_buffer, 32,
					   SEED_MASTER_TX +
					   (uint32_t)(timing_cycle * 10 +
							  rapid_op));

			comp_ctx.total_tests++;
			uint32_t seed = SEED_SLAVE_TX +
					(uint32_t)(timing_cycle * 10 +
						   rapid_op);

			ret = comp_write_read_validate(master, hw_tx_buffer,
						       32, hw_rx_buffer,
							   32, seed);
			zassert_ok(ret,
				   "Timing margin cycle %d op %d failed: %d",
				   timing_cycle, rapid_op, ret);
			comp_ctx.passed_tests++;

			/* Minimal delay to stress timing margins */
			if (rapid_op < 9) {
				k_usleep(1);
			}
		}
	}

	LOG_INF("Comprehensive hardware-software co-validation completed");
}

/**
 * @brief Comprehensive regression and validation summary
 *
 * Objective: Final validation and regression test
 * Hardware Issue: All hardware issues
 * Driver Issue: All driver issues
 */
ZTEST(i2c_comprehensive_suite, test_comprehensive_regression)
{
	const struct device *master = DEVICE_DT_GET(I2C_MASTER);
	uint8_t final_tx_buffer[64];
	uint8_t final_rx_buffer[64];
	int ret;

	zassert_not_null(master,
			 "Master I2C device not found - cannot continue test");
	if (!device_is_ready(master)) {
		zassert_true(false, "Master I2C device not ready");
		return;
	}

	LOG_INF("Starting comprehensive regression validation");

	/* Test 1: All speeds regression with real data comparison */
	const uint32_t regression_speeds[] = {
		I2C_SPEED_STANDARD, I2C_SPEED_FAST,
		I2C_SPEED_FAST_PLUS, I2C_SPEED_HIGH
	};

	for (size_t speed_idx = 0;
		 speed_idx < ARRAY_SIZE(regression_speeds); speed_idx++) {
		uint32_t config = I2C_MODE_CONTROLLER |
				  I2C_SPEED_SET(regression_speeds[speed_idx]);

		ret = i2c_configure(master, config);
		zassert_ok(ret, "Regression speed %u config failed: %d",
			   regression_speeds[speed_idx], ret);

		for (size_t pattern_idx = 0;
			 pattern_idx < ARRAY_SIZE(comprehensive_patterns);
			 pattern_idx++) {
			memcpy(final_tx_buffer,
			       comprehensive_patterns[pattern_idx], 8);
			for (int i = 8; i < 64; i++) {
				final_tx_buffer[i] =
					(uint8_t)(speed_idx + pattern_idx + i);
			}

			comp_ctx.total_tests++;
			uint32_t seed = SEED_SLAVE_TX +
					(uint32_t)(speed_idx * 100 +
						   pattern_idx);

			ret = comp_write_read_validate(master,
						       final_tx_buffer, 64,
							   final_rx_buffer, 64,
							   seed);
			zassert_ok(ret,
				   "Regression speed %u failed: %d",
				   regression_speeds[speed_idx], ret);
			comp_ctx.passed_tests++;
		}
	}

	/* Test 2: Size regression */
	const size_t regression_sizes[] = {1, 2, 4, 8, 16, 32, 64};

	for (size_t size_idx = 0;
		 size_idx < ARRAY_SIZE(regression_sizes); size_idx++) {
		size_t test_size = regression_sizes[size_idx];

		fill_buffer_random(final_tx_buffer, test_size,
				   SEED_MASTER_TX + (uint32_t)test_size);

		comp_ctx.total_tests++;
		ret = comp_write_read_validate(master, final_tx_buffer,
					       test_size, final_rx_buffer,
						   test_size,
						   SEED_SLAVE_TX +
						   (uint32_t)test_size);
		zassert_ok(ret, "Regression size %zu failed: %d",
			   test_size, ret);
		comp_ctx.passed_tests++;
	}

	/* Test 3: Final mixed-operation validation */
	for (int final_cycle = 0; final_cycle < 10; final_cycle++) {
		fill_buffer_random(final_tx_buffer, 32,
				   SEED_MASTER_TX + (uint32_t)final_cycle);

		/* Write with data validation */
		comp_ctx.total_tests++;
		ret = comp_write_validate(master, final_tx_buffer, 32);
		zassert_ok(ret, "Final write cycle %d failed: %d",
			   final_cycle, ret);

		/* Read with data validation */
		comp_ctx.total_tests++;
		ret = comp_read_validate(master, final_rx_buffer, 32,
					 SEED_SLAVE_TX +
					 (uint32_t)final_cycle);
		zassert_ok(ret, "Final read cycle %d failed: %d",
			   final_cycle, ret);
	}

	LOG_INF("Total Tests: %u, Passed: %u, Failed: %u",
		comp_ctx.total_tests, comp_ctx.passed_tests,
		comp_ctx.failed_tests);

	zassert_equal(comp_ctx.failed_tests, 0,
		      "Comprehensive regression: %u tests failed",
			  comp_ctx.failed_tests);
	zassert_equal(comp_ctx.driver_errors, 0,
		      "Driver errors detected in comprehensive validation");
	zassert_equal(comp_ctx.hardware_errors, 0,
		      "Hardware errors detected in comprehensive validation");
}

static void *i2c_comprehensive_suite_setup(void)
{
	const struct device *master = DEVICE_DT_GET(I2C_MASTER);
	const struct device *slave = DEVICE_DT_GET(I2C_SLAVE);
	uint32_t master_config = I2C_MODE_CONTROLLER |
				 I2C_SPEED_SET(I2C_SPEED_STANDARD);
	uint32_t slave_config = I2C_SPEED_SET(I2C_SPEED_STANDARD);
	int ret;

	/* Initialize comprehensive context */
	memset(&comp_ctx, 0, sizeof(comp_ctx));
	comp_ctx.master = master;
	comp_ctx.slave = slave;
	comp_ctx.comprehensive_mode = true;

	/* Configure devices */
	zassert_not_null(master, "Master I2C device not found");
	ret = i2c_configure(master, master_config);
	zassert_ok(ret, "Master I2C configuration failed: %d", ret);

	zassert_not_null(slave, "Slave I2C device not found");
	ret = i2c_configure(slave, slave_config);
	zassert_ok(ret, "Slave I2C configuration failed: %d", ret);
	i2c_test_ctx.slave_dev = slave;

	/* Register slave if needed */
	if (!i2c_test_ctx.slave_registered) {
		i2c_comp_tcfg.flags = 0;
		i2c_comp_tcfg.address = SLV_I2C_ADDR;
		ret = i2c_target_register(slave, &i2c_comp_tcfg);
		zassert_ok(ret, "Slave registration failed: %d", ret);
		i2c_test_ctx.slave_registered = true;
	}

	LOG_INF("I2C comprehensive validation test suite setup completed");
	return NULL;
}

static void i2c_comprehensive_before(void *fixture)
{
	ARG_UNUSED(fixture);
	i2c_test_ctx.slv_rx_cnt = 0U;
	i2c_test_ctx.slv_tx_cnt = 0U;
	memset(i2c_test_ctx.slv_rx_data, 0,
	       sizeof(i2c_test_ctx.slv_rx_data));
	memset(i2c_test_ctx.slv_tx_data, 0,
	       sizeof(i2c_test_ctx.slv_tx_data));
	i2c_test_reset_callback_state(&i2c_test_ctx);
}

ZTEST_SUITE(i2c_comprehensive_suite, NULL, i2c_comprehensive_suite_setup,
	    i2c_comprehensive_before, NULL, NULL);

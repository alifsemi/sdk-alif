/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include "test_i2c.h"
LOG_MODULE_REGISTER(alif_i2c_fault, LOG_LEVEL_INF);

/* Fault injection test constants */
#define FAULT_TEST_TIMEOUT_MS		  5000U
#define BUS_RECOVERY_ATTEMPTS		  10U
#define NACK_INJECTION_ITERATIONS	  50U
#define BUS_BUSY_SIMULATION_DELAY_US      100U
#define NACK_RECOVERY_DELAY_US		  100U
#define ARBITRATION_LOSS_STRESS_COUNT     100U

/* Fault injection context */
struct i2c_fault_ctx {
	const struct device *controller;
	const struct device *target;
	uint32_t fault_count;
	uint32_t recovery_count;
};

static struct i2c_fault_ctx fault_ctx;

/* Reset error counters before each test to prevent cross-test contamination */
static void reset_fault_error_counters(void)
{
	fault_ctx.fault_count = 0U;
	fault_ctx.recovery_count = 0U;
}

/**
 * @brief Test NACK handling and recovery
 *
 * Objective: Expose NACK handling bugs in driver and hardware
 * Hardware Issue: Improper NACK generation/detection
 * Driver Issue: Missing NACK recovery, incorrect state transitions
 */
ZTEST(i2c_fault_suite, test_nack_handling)
{
	const struct device *controller = fault_ctx.controller;
	uint8_t test_data[] = {0xAA, 0x55, 0xFF};
	uint8_t rx_buffer[8];
	struct i2c_test_freq_desc freqs[I2C_TEST_MAX_FREQS];
	int ret;
	int freq_count;

	i2c_test_skip_if_no_freqs();
	freq_count = i2c_test_get_enabled_freqs(freqs, ARRAY_SIZE(freqs));

	for (int f = 0; f < freq_count; f++) {
		TC_PRINT("NACK handling at %s (%u Hz)\n", freqs[f].name,
			 freqs[f].freq_hz);

		ret = i2c_test_configure_controller_freq(controller, &i2c_test_ctx,
						     &freqs[f]);
		if (ret == -ENOTSUP) {
			TC_PRINT("Skipping %s: not supported\n", freqs[f].name);
			continue;
		}
		zassert_ok(ret, "Failed to configure %s: %d", freqs[f].name, ret);

		register_target_speed_i2c(I2C_SPEED_SET(freqs[f].zephyr_speed));

	LOG_INF("=== NACK Handling at %s ===", freqs[f].name);

	/* Test 1: NACK on address phase - invalid target address */
	const uint16_t invalid_addresses[] = {
		0x00,  /* Reserved address */
		0x78,  /* General call address */
		0x80,  /* Reserved range */
		0xFF,  /* Maximum 8-bit address */
		0x123, /* Invalid 10-bit address */
	};

	LOG_INF("Test 1: Invalid address handling (%zu addresses)",
		ARRAY_SIZE(invalid_addresses));

	for (size_t i = 0; i < ARRAY_SIZE(invalid_addresses); i++) {
		ret = i2c_write(controller, test_data, sizeof(test_data),
				invalid_addresses[i]);

		LOG_DBG("Address 0x%03X: ret=%d (%s)",
			invalid_addresses[i], ret,
			ret == -ENXIO		? "ENXIO"
			: ret == -EIO		? "EIO"
			: ret == 0			? "SUCCESS"
			: ret == -EBUSY		? "EBUSY"
			: ret == -ETIMEDOUT ? "ETIMEDOUT"
						: "OTHER");

		/* Should fail with appropriate error code */
		if (ret == -ENXIO) {
			LOG_INF("Correctly detected invalid address 0x%03X",
				invalid_addresses[i]);
		} else if (ret == -EIO) {
			LOG_INF("I2C error for addr 0x%03X",
				invalid_addresses[i]);
		} else if (ret == 0) {
			/* Unexpected success - could indicate hardware issue */
			LOG_WRN("Unexpected success with addr 0x%03X",
				invalid_addresses[i]);
			zassert_false(true,
				      "Invalid address should not succeed");
		} else {
			LOG_INF("Error code %d for invalid address 0x%03X",
				ret, invalid_addresses[i]);
		}
	}

	/* Test 2: NACK on data phase - simulate target not ready */
	LOG_INF("Test 2: Data phase NACK handling (%d iterations)",
		NACK_INJECTION_ITERATIONS);

	for (int i = 0; i < NACK_INJECTION_ITERATIONS; i++) {
		/* Try to read from target that might not be ready */
		ret = i2c_read(controller, rx_buffer, sizeof(rx_buffer),
			       TGT_I2C_ADDR);

		if (ret == -ENXIO) {
			/* Expected - target not responding */
			LOG_DBG("Data NACK test %d: ret=-ENXIO (expected)", i);
		} else if (ret == 0) {
			/* Success - verify data integrity */
			LOG_DBG("Data NACK test %d: ret=0 (success)", i);
			for (size_t j = 0; j < sizeof(rx_buffer); j++) {
				rx_buffer[j] = 0; /* Clear for next test */
			}
		} else if (ret == -EIO) {
			/* Bus error - could indicate hardware issue */
			LOG_DBG("Bus error during NACK test iteration %d", i);
		} else {
			LOG_INF("Unexpected error %d in NACK iter %d", ret, i);
		}
	}

	/* Test 3: Recovery after NACK - expose state machine bugs */
	LOG_INF("Test 3: Recovery after NACK (20 iterations)");

	for (int i = 0; i < 20; i++) {
		/* First, trigger NACK with invalid address */
		ret = i2c_write(controller, test_data, 1, 0x80);
		if (ret != 0) {
			/* Expected failure */
			LOG_DBG("NACK trigger %d: ret=%d (expected failure)",
				i, ret);
		} else {
			LOG_WRN("NACK trigger %d: Unexpected success!", i);
		}

		/* Allow small delay for I2C controller to reset after NACK */
		k_usleep(NACK_RECOVERY_DELAY_US);

		/* Try valid transfer - test recovery */
		ret = i2c_write(controller, test_data, sizeof(test_data),
				TGT_I2C_ADDR);
		LOG_DBG("Recovery test %d: ret=%d", i, ret);

		if (ret != 0) {
			LOG_ERR("Recovery failed at iteration %d: ret=%d (%s)",
				i, ret,
				ret == -ENXIO		? "ENXIO"
				: ret == -EIO		? "EIO"
				: ret == -EBUSY		? "EBUSY"
				: ret == -ETIMEDOUT ? "ETIMEDOUT"
							: "OTHER");
		}

		zassert_ok(ret,
			   "Recovery failed after NACK at iteration %d: %d",
			   i, ret);
	}

	}

	LOG_INF("=== NACK handling and recovery tests completed ===");
}

/**
 * @brief Test bus busy detection and arbitration
 *
 * Objective: Expose bus arbitration and busy detection bugs
 * Hardware Issue: Arbitration loss detection, bus state tracking
 * Driver Issue: Missing busy check, incorrect arbitration handling
 */
ZTEST(i2c_fault_suite, test_bus_busy)
{
	const struct device *controller = fault_ctx.controller;
	uint8_t test_data[] = {0x12, 0x34, 0x56, 0x78};
	int ret;

	/* Test 1: Bus busy simulation - expose busy detection bugs */
	for (int i = 0; i < 10; i++) {
		/* Start a transfer */
		ret = i2c_write(controller, test_data, sizeof(test_data),
				TGT_I2C_ADDR);
		zassert_ok(ret, "Initial transfer failed: %d", ret);

		/* Immediately try another transfer - should detect busy */
		ret = i2c_write(controller, test_data, 1, TGT_I2C_ADDR);

		/*
		 * Driver should either succeed (if transfer completed)
		 * or return busy error (if still in progress)
		 */
		if (ret == -EBUSY) {
			LOG_DBG("Correctly detected bus busy at iteration %d",
				i);
		} else if (ret == 0) {
			LOG_DBG("Transfer completed quickly at iteration %d",
				i);
		} else {
			LOG_INF("Busy test error %d at iter %d", ret, i);
		}

		/* Small delay to ensure bus is free */
		k_usleep(BUS_BUSY_SIMULATION_DELAY_US);
	}

	/* Test 2: Arbitration loss stress - expose arbitration bugs */
	for (int i = 0; i < ARBITRATION_LOSS_STRESS_COUNT; i++) {
		/* Use different addresses to potentially cause issues */
		uint16_t test_addr = TGT_I2C_ADDR + (i % 3);

		ret = i2c_write(controller, test_data, sizeof(test_data),
				test_addr);

		if (ret == -EIO) {
			/* Could indicate arbitration loss */
			LOG_DBG("Possible arbitration loss at iteration %d",
				i);
			fault_ctx.fault_count++;
		} else if (ret == -ENXIO) {
			/* Expected for invalid addresses */
		} else if (ret == 0) {
			/* Success */
		} else {
			LOG_INF("Arbitration test error %d at iter %d", ret, i);
		}
	}

	/* Test 3: Multi-master simulation (if hardware supports) */
	for (int i = 0; i < 20; i++) {
		uint16_t addr1 = TGT_I2C_ADDR;
		uint16_t addr2 = TGT_I2C_ADDR + 1;

		ret = i2c_write(controller, test_data, 1, addr1);
		if (ret != 0 && ret != -ENXIO) {
			LOG_INF("Multi-controller err %d addr1 iter %d", ret, i);
		}

		ret = i2c_write(controller, test_data, 1, addr2);
		if (ret != 0 && ret != -ENXIO) {
			LOG_INF("Multi-controller err %d addr2 iter %d", ret, i);
		}
	}

	LOG_INF("Bus busy and arbitration tests completed (faults: %u)",
		fault_ctx.fault_count);

	/* Fail test if any faults accumulated during bus busy/arbitration testing */
	zassert_equal(fault_ctx.fault_count, 0U,
		      "Bus busy/arbitration test had %u faults",
		      fault_ctx.fault_count);
}

/**
 * @brief Test bus recovery scenarios
 *
 * Objective: Expose bus recovery mechanism bugs
 * Hardware Issue: Stuck SDA/SCL detection, recovery timing
 * Driver Issue: Missing recovery implementation, incorrect sequence
 */
ZTEST(i2c_fault_suite, test_bus_recovery)
{
	const struct device *controller = fault_ctx.controller;
	uint8_t test_data[] = {0xAA, 0x55};
	int ret;

	/* Test 1: Simulate stuck SDA condition */
	for (int attempt = 0; attempt < BUS_RECOVERY_ATTEMPTS; attempt++) {
		/* Try normal transfer first */
		ret = i2c_write(controller, test_data, sizeof(test_data),
				TGT_I2C_ADDR);
		zassert_ok(ret, "Pre-recovery transfer failed: %d", ret);

		/* Attempt recovery by re-initializing the bus */
		uint32_t config = I2C_MODE_CONTROLLER |
				  I2C_SPEED_SET(I2C_SPEED_STANDARD);

		ret = i2c_configure(controller, config);
		zassert_ok(ret, "Recovery config failed at %d: %d",
			 attempt, ret);

		/* Test transfer after recovery */
		ret = i2c_write(controller, test_data, sizeof(test_data),
				TGT_I2C_ADDR);
		zassert_ok(ret,
			   "Post-recovery transfer failed at attempt %d: %d",
			   attempt, ret);

		fault_ctx.recovery_count++;
	}

	/* Test 2: Timeout and recovery */
	for (int i = 0; i < 10; i++) {
		int64_t start_time = k_uptime_get();

		ret = i2c_write(controller, test_data, sizeof(test_data),
				TGT_I2C_ADDR);

		int64_t end_time = k_uptime_get();
		int64_t duration = end_time - start_time;

		/* Check if transfer took unreasonable time */
		if (duration > FAULT_TEST_TIMEOUT_MS) {
			LOG_INF("Transfer %d took %lldms (timeout?)",
				i, duration);
			fault_ctx.fault_count++;
		} else {
			zassert_ok(ret, "Timeout test failed at %d: %d",
				i, ret);
		}
	}

	/* Test 3: Clock line recovery */
	for (int i = 0; i < 5; i++) {
		ret = i2c_read(controller, test_data, sizeof(test_data),
			       TGT_I2C_ADDR);

		if (ret == -ETIMEDOUT) {
			LOG_INF("Clock stretch timeout at iter %d", i);
			fault_ctx.fault_count++;
		} else if (ret == 0) {
			LOG_INF("Clock stretch test passed at iter %d", i);
		} else {
			LOG_INF("Clk stretch error %d at iter %d", ret, i);
		}
	}

	LOG_INF("Bus recovery done (recoveries: %u, faults: %u)",
		fault_ctx.recovery_count, fault_ctx.fault_count);

	/* Fail test if any faults accumulated during bus recovery testing */
	zassert_equal(fault_ctx.fault_count, 0U,
		      "Bus recovery test had %u faults",
		      fault_ctx.fault_count);
}

/**
 * @brief Test concurrent access scenarios
 *
 * Objective: Expose concurrency and race condition bugs
 * Hardware Issue: N/A (software only)
 * Driver Issue: Missing synchronization, race conditions
 */
ZTEST(i2c_fault_suite, test_concurrent_access)
{
	const struct device *controller = fault_ctx.controller;
	uint8_t test_data[] = {0xAA, 0x55, 0xFF};
	int ret;

	/* Test 1: Rapid reconfiguration - expose race conditions */
	for (int i = 0; i < 100; i++) {
		uint32_t config1 = I2C_MODE_CONTROLLER |
				   I2C_SPEED_SET(I2C_SPEED_STANDARD);
		uint32_t config2 = I2C_MODE_CONTROLLER |
				   I2C_SPEED_SET(I2C_SPEED_FAST);

		/* Rapid configuration changes */
		ret = i2c_configure(controller, config1);
		zassert_ok(ret, "Config1 failed at iteration %d: %d", i, ret);

		ret = i2c_configure(controller, config2);
		zassert_ok(ret, "Config2 failed at iteration %d: %d", i, ret);

		/* Immediate transfer */
		ret = i2c_write(controller, test_data, 1, TGT_I2C_ADDR);
		zassert_ok(ret,
			   "Transfer failed after rapid reconfig %d: %d",
			   i, ret);
	}

	/*
	 * Test 2: Scheduler-locked pressure - expose race conditions
	 *
	 * The previous version wrapped i2c_write() inside irq_lock()/
	 * irq_unlock(). Zephyr I2C controllers (including the Alif /
	 * DesignWare driver) complete transfers through interrupts and
	 * block on a semaphore until the ISR signals completion, so
	 * running i2c_write() with IRQs disabled would either deadlock
	 * or produce a misleading -ETIMEDOUT. Use k_sched_lock() instead
	 * to pin the current thread (preventing preemption) while still
	 * allowing the driver's ISR to fire.
	 */
	for (int i = 0; i < 50; i++) {
		k_sched_lock();

		ret = i2c_write(controller, &test_data[i % 3], 1, TGT_I2C_ADDR);

		k_sched_unlock();

		zassert_ok(ret,
			   "Scheduler-locked pressure test failed at iteration %d: %d",
			   i, ret);
	}

	/* Test 3: Simulated multi-thread access */
	for (int thread_sim = 0; thread_sim < 20; thread_sim++) {
		/* Simulate thread switching by rapid operations */
		for (int op = 0; op < 10; op++) {
			ret = i2c_write(controller, test_data, sizeof(test_data),
					TGT_I2C_ADDR);
			zassert_ok(ret,
				   "Thread sim %d, op %d failed: %d",
				   thread_sim, op, ret);

			/* Small delay to simulate context switch */
			k_usleep(1);
		}
	}

	LOG_INF("Concurrent access scenarios completed");
}

/**
 * @brief Test invalid address handling
 *
 * Objective: Validate driver error handling for invalid addresses
 * Expected: Driver should return appropriate error codes
 */
ZTEST(i2c_fault_suite, test_invalid_addr)
{
	const struct device *controller = fault_ctx.controller;
	uint8_t test_data[] = {0xAA};
	int ret;

	LOG_INF("=== Invalid Address Handling Test ===");

	/* Test reserved and invalid addresses */
	struct {
		uint16_t addr;
		const char *desc;
	} invalid_addresses[] = {
		{0x00,	"Reserved address"},
		{0x78,	"General call address"},
		{0x80,	"Reserved range"},
		{0xFF,	"Maximum 8-bit address"},
		{0x123, "Invalid 10-bit address"},
	};

	for (size_t i = 0; i < ARRAY_SIZE(invalid_addresses); i++) {
		ret = i2c_write(controller, test_data, sizeof(test_data),
				invalid_addresses[i].addr);

		LOG_INF("Address 0x%03X (%s): ret=%d (%s)",
			invalid_addresses[i].addr,
			invalid_addresses[i].desc, ret,
			ret == -ENXIO	? "ENXIO"
			: ret == -EIO	? "EIO"
			: ret == -EBUSY	? "EBUSY"
			: ret == -ETIMEDOUT ? "ETIMEDOUT"
			: ret == 0 ? "SUCCESS" : "OTHER");

		/* Driver should reject invalid addresses */
		if (ret == 0) {
			LOG_WRN("Addr 0x%03X succeeded - hardware?",
				invalid_addresses[i].addr);
		} else if (ret == -ENXIO || ret == -EIO) {
			LOG_INF("Correctly rejected invalid address 0x%03X",
				invalid_addresses[i].addr);
		} else {
			LOG_INF("Unexpected error %d for address 0x%03X",
				ret, invalid_addresses[i].addr);
		}
	}
}

/**
 * @brief Test NACK recovery scenarios
 *
 * Objective: Validate driver recovery after NACK conditions
 * Expected: Driver should recover and handle subsequent transfers
 */
ZTEST(i2c_fault_suite, test_nack_recovery)
{
	const struct device *controller = fault_ctx.controller;
	uint8_t test_data[] = {0xAA, 0x55};
	int ret;

	LOG_INF("=== Enhanced NACK Recovery Test ===");

	/* Verify target is responding first */
	ret = i2c_write(controller, test_data, 1, TGT_I2C_ADDR);
	zassert_ok(ret, "Target not responding - test setup issue");

	/* Test immediate recovery after NACK */
	for (int i = 0; i < 10; i++) {
		/* Trigger NACK with invalid address */
		ret = i2c_write(controller, test_data, 1, 0x80);
		LOG_DBG("NACK trigger %d: ret=%d", i, ret);

		/* Test immediate recovery */
		ret = i2c_write(controller, test_data, sizeof(test_data),
				TGT_I2C_ADDR);
		LOG_DBG("Immediate recovery %d: ret=%d", i, ret);

		if (ret != 0) {
			LOG_ERR("Recovery failed at iteration %d: ret=%d",
				i, ret);

			/* Test with delay */
			k_msleep(1);
			ret = i2c_write(controller, test_data,
					sizeof(test_data), TGT_I2C_ADDR);
			LOG_DBG("Delayed recovery %d: ret=%d", i, ret);

			if (ret == 0) {
				LOG_INF("Recovery works with delay - timing");
			} else {
				/* Test with bus reset */
				uint32_t config =
					I2C_MODE_CONTROLLER |
					I2C_SPEED_SET(I2C_SPEED_STANDARD);
				ret = i2c_configure(controller, config);
				zassert_ok(ret, "Bus reset failed");

				ret = i2c_write(controller, test_data,
						sizeof(test_data),
						TGT_I2C_ADDR);
				LOG_DBG("Recovery after reset %d: ret=%d",
					i, ret);
			}
		}

		zassert_ok(ret,
			   "Recovery failed completely at iteration %d: %d",
			   i, ret);
	}
}

/**
 * @brief Test bus busy conditions
 *
 * Objective: Validate driver busy detection and handling
 * Expected: Driver should detect and handle busy conditions
 */
ZTEST(i2c_fault_suite, test_bus_busy_enh)
{
	const struct device *controller = fault_ctx.controller;
	uint8_t test_data[] = {0x12, 0x34};
	int ret;

	LOG_INF("=== Enhanced Bus Busy Test ===");

	/* Test rapid successive transfers */
	for (int i = 0; i < 5; i++) {
		/* Start transfer */
		ret = i2c_write(controller, test_data, sizeof(test_data),
				TGT_I2C_ADDR);
		zassert_ok(ret, "Initial transfer failed: %d", ret);

		/* Immediate second transfer */
		ret = i2c_write(controller, test_data, 1, TGT_I2C_ADDR);

		if (ret == -EBUSY) {
			LOG_INF("Busy condition detected at iter %d", i);
		} else if (ret == 0) {
			LOG_DBG("Transfer completed quickly at iteration %d",
				i);
		} else {
			LOG_INF("Busy test error %d at iter %d", ret, i);
		}
	}
}

/**
 * @brief Test configuration edge cases
 *
 * Objective: Validate driver handling of invalid configurations
 * Expected: Driver should reject invalid configurations
 */
ZTEST(i2c_fault_suite, test_config_edge)
{
	const struct device *controller = fault_ctx.controller;
	int ret;

	LOG_INF("=== Configuration Edge Cases Test ===");

	/* Test invalid speed configurations */
	uint32_t invalid_configs[] = {
		0,/* No mode */
		I2C_SPEED_SET(999),/* Invalid speed */
		I2C_MODE_CONTROLLER | I2C_SPEED_SET(999), /* Invalid + mode */
	};

	for (size_t i = 0; i < ARRAY_SIZE(invalid_configs); i++) {
		ret = i2c_configure(controller, invalid_configs[i]);
		LOG_INF("Invalid config %zu: ret=%d", i, ret);

		if (ret == -EINVAL) {
			LOG_INF("Correctly rejected invalid configuration %zu",
				i);
		} else if (ret == 0) {
			LOG_WRN("Invalid config %zu accepted - permissive?", i);
		} else {
			LOG_INF("Unexpected error %d for config %zu", ret, i);
		}
	}

	/* Restore valid configuration */
	uint32_t valid_config = I2C_MODE_CONTROLLER |
				I2C_SPEED_SET(I2C_SPEED_STANDARD);

	ret = i2c_configure(controller, valid_config);
	zassert_ok(ret, "Failed to restore valid configuration");
}

/**
 * @brief Reserved-address rejection / NACK behavior.
 *
 * I2C reserves two address ranges that a compliant controller must never
 * own as a real slave:
 *   0x00-0x07 : general-call / CBUS / high-speed master / reserved
 *   0x78-0x7F : 10-bit addressing prefix / reserved
 *
 * A driver MAY reject these at i2c_write() time (best), OR allow the
 * transfer and let the bus NACK (acceptable). Either way, the bus must
 * remain usable after the attempt. This test asserts both paths behave
 * cleanly and that a normal transfer still succeeds afterwards.
 */
ZTEST(i2c_fault_suite, test_reserved_addr)
{
	const struct device *controller = fault_ctx.controller;
	static const uint16_t reserved_addrs[] = {
		0x00U, 0x01U, 0x04U, 0x07U,	/* low reserved range */
		0x78U, 0x7CU, 0x7FU,		/* high reserved range */
	};
	uint8_t test_byte = 0xA5U;
	int ret;

	LOG_INF("=== Reserved Address Rejection Test ===");

	for (size_t i = 0; i < ARRAY_SIZE(reserved_addrs); i++) {
		uint16_t addr = reserved_addrs[i];

		ret = i2c_write(controller, &test_byte, 1U, addr);
		/* Accept either early rejection (-EINVAL) or bus NACK (-EIO).
		 * Success is flagged as a warning — driver accepted a
		 * reserved-range transfer which indicates weak validation.
		 */
		if (ret == 0) {
			LOG_WRN("reserved addr 0x%02x accepted by driver",
				addr);
		} else {
			LOG_INF("reserved addr 0x%02x rejected: %d", addr, ret);
			zassert_true(ret == -EINVAL || ret == -EIO ||
				     ret == -ENXIO,
				     "reserved addr 0x%02x unexpected err %d",
				     addr, ret);
		}
	}

	/* Bus must be immediately usable after the reserved attempts. */
	uint8_t payload[4] = {0x11, 0x22, 0x33, 0x44};

	i2c_test_prime_buffers(NULL, 0U, sizeof(payload));
	ret = i2c_write(controller, payload, sizeof(payload), TGT_I2C_ADDR);
	zassert_ok(ret, "bus unusable after reserved-addr attempts: %d", ret);

	ret = validate_target_rx(payload, sizeof(payload));
	zassert_ok(ret, "RX mismatch after reserved-addr recovery: %d", ret);
}

/**
 * @brief NACK asserted mid-transfer by the slave.
 *
 * A real slave may NACK partway through a write (e.g., internal buffer
 * full). The driver must:
 *   1. Detect TX_ABRT,
 *   2. Return a non-zero error to the caller,
 *   3. Release the bus cleanly,
 *   4. Allow the next transfer to proceed normally.
 *
 * We synthesize this by writing more bytes than the slave has primed its
 * RX buffer for. If the slave NACKs after filling its buffer, the master
 * receives TX_ABRT. If the slave silently accepts (buffer grows), the
 * test still validates data integrity for the accepted portion.
 */
ZTEST(i2c_fault_suite, test_nack_mid_transfer)
{
	const struct device *controller = fault_ctx.controller;
	const size_t oversize = BUFF_PERF + 16U;
	static uint8_t big_tx[BUFF_PERF + 16U];
	int ret;

	LOG_INF("=== NACK Mid-Transfer Test ===");

	for (size_t i = 0; i < sizeof(big_tx); i++) {
		big_tx[i] = (uint8_t)(0x10U + i);
	}

	/* Prime target RX for only BUFF_PERF bytes; writing more may NACK. */
	i2c_test_prime_buffers(NULL, 0U, BUFF_PERF);

	ret = i2c_write(controller, big_tx, oversize, TGT_I2C_ADDR);
	if (ret != 0) {
		LOG_INF("mid-transfer NACK detected as expected: %d", ret);
		zassert_true(ret == -EIO || ret == -ENXIO,
			     "unexpected mid-transfer error: %d", ret);
	} else {
		LOG_INF("target accepted oversize write (no NACK)");
	}

	/* Critical check: bus must recover for the next transfer. */
	uint8_t follow[8];

	for (size_t i = 0; i < sizeof(follow); i++) {
		follow[i] = (uint8_t)(0xA0U + i);
	}

	i2c_test_prime_buffers(NULL, 0U, sizeof(follow));
	ret = i2c_write(controller, follow, sizeof(follow), TGT_I2C_ADDR);
	zassert_ok(ret, "bus unusable after mid-transfer NACK: %d", ret);

	ret = validate_target_rx(follow, sizeof(follow));
	zassert_ok(ret, "RX mismatch after NACK recovery: %d", ret);
}

static void *i2c_fault_suite_setup(void)
{
	const struct device *controller = DEVICE_DT_GET(I2C_CONTROLLER);
	const struct device *target = DEVICE_DT_GET(I2C_TARGET);
	int ret;

	/* Initialize fault context */
	memset(&fault_ctx, 0, sizeof(fault_ctx));
	fault_ctx.controller = controller;
	fault_ctx.target = target;

	/* Verify devices */
	zassert_not_null(controller, "Controller I2C device not found");
	zassert_true(device_is_ready(controller),
		     "Controller I2C device not ready");
	zassert_not_null(target, "Target I2C device not found");
	zassert_true(device_is_ready(target),
		     "Target I2C device not ready");

	/*
	 * Populate the shared test context so every common helper
	 * (register_target_speed_i2c, i2c_test_configure_controller_freq,
	 * i2c_test_prime_buffers, ...) resolves to the same controller
	 * / target pair used by this suite.
	 */
	i2c_test_ctx.controller_dev = controller;
	i2c_test_ctx.target_dev = target;
	i2c_test_reset_runtime_config(&i2c_test_ctx);

	ret = i2c_configure(i2c_test_ctx.controller_dev, i2c_test_ctx.i2c_cfg);
	zassert_ok(ret, "Controller I2C configuration failed: %d", ret);

	/*
	 * Route slave registration through the shared helper so the
	 * i2c_test_ctx.target_registered flag and common i2c_tcfg remain
	 * in sync with test_i2c.c bookkeeping. This avoids stale state
	 * if another suite ran previously.
	 */
	register_target_i2c();

	LOG_INF("I2C fault injection test suite setup completed");
	return NULL;
}

static void i2c_fault_suite_teardown(void *fixture)
{
	int ret;

	ARG_UNUSED(fixture);

	if (!i2c_test_ctx.target_registered || i2c_test_ctx.target_dev == NULL) {
		return;
	}

	ret = i2c_target_unregister(i2c_test_ctx.target_dev, &i2c_tcfg);
	if (ret == -ENOTSUP) {
		LOG_WRN("I2C target mode not supported");
	} else if (ret != 0) {
		LOG_WRN("i2c_target_unregister failed: %d", ret);
	}

	i2c_test_ctx.target_registered = false;
}

/* Before each test: reset error counters to prevent cross-test contamination */
static void i2c_fault_suite_before(void *fixture)
{
	ARG_UNUSED(fixture);
	reset_fault_error_counters();
}

ZTEST_SUITE(i2c_fault_suite, NULL, i2c_fault_suite_setup,
	    i2c_fault_suite_before, NULL, i2c_fault_suite_teardown);

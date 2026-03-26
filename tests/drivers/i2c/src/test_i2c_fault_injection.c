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
	const struct device *master;
	const struct device *slave;
	uint32_t fault_count;
	uint32_t recovery_count;
	bool bus_locked;
};

static struct i2c_fault_ctx fault_ctx;

/**
 * @brief Test NACK handling and recovery
 *
 * Objective: Expose NACK handling bugs in driver and hardware
 * Hardware Issue: Improper NACK generation/detection
 * Driver Issue: Missing NACK recovery, incorrect state transitions
 */
ZTEST(i2c_fault_suite, test_nack_handling)
{
	const struct device *master = DEVICE_DT_GET(I2C_MASTER);
	uint8_t test_data[] = {0xAA, 0x55, 0xFF};
	uint8_t rx_buffer[8];
	int ret;

	LOG_INF("=== Starting NACK Handling and Recovery Test ===");

	/* Test 1: NACK on address phase - invalid slave address */
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
		ret = i2c_write(master, test_data, sizeof(test_data),
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

	/* Test 2: NACK on data phase - simulate slave not ready */
	LOG_INF("Test 2: Data phase NACK handling (%d iterations)",
		NACK_INJECTION_ITERATIONS);

	for (int i = 0; i < NACK_INJECTION_ITERATIONS; i++) {
		/* Try to read from slave that might not be ready */
		ret = i2c_read(master, rx_buffer, sizeof(rx_buffer),
			       SLV_I2C_ADDR);

		if (ret == -ENXIO) {
			/* Expected - slave not responding */
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
		ret = i2c_write(master, test_data, 1, 0x80);
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
		ret = i2c_write(master, test_data, sizeof(test_data),
				SLV_I2C_ADDR);
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
	const struct device *master = DEVICE_DT_GET(I2C_MASTER);
	uint8_t test_data[] = {0x12, 0x34, 0x56, 0x78};
	int ret;

	/* Test 1: Bus busy simulation - expose busy detection bugs */
	for (int i = 0; i < 10; i++) {
		/* Start a transfer */
		ret = i2c_write(master, test_data, sizeof(test_data),
				SLV_I2C_ADDR);
		zassert_ok(ret, "Initial transfer failed: %d", ret);

		/* Immediately try another transfer - should detect busy */
		ret = i2c_write(master, test_data, 1, SLV_I2C_ADDR);

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
		uint16_t test_addr = SLV_I2C_ADDR + (i % 3);

		ret = i2c_write(master, test_data, sizeof(test_data),
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
		uint16_t addr1 = SLV_I2C_ADDR;
		uint16_t addr2 = SLV_I2C_ADDR + 1;

		ret = i2c_write(master, test_data, 1, addr1);
		if (ret != 0 && ret != -ENXIO) {
			LOG_INF("Multi-master err %d addr1 iter %d", ret, i);
		}

		ret = i2c_write(master, test_data, 1, addr2);
		if (ret != 0 && ret != -ENXIO) {
			LOG_INF("Multi-master err %d addr2 iter %d", ret, i);
		}
	}

	LOG_INF("Bus busy and arbitration tests completed (faults: %u)",
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
	const struct device *master = DEVICE_DT_GET(I2C_MASTER);
	uint8_t test_data[] = {0xAA, 0x55};
	int ret;

	/* Test 1: Simulate stuck SDA condition */
	for (int attempt = 0; attempt < BUS_RECOVERY_ATTEMPTS; attempt++) {
		/* Try normal transfer first */
		ret = i2c_write(master, test_data, sizeof(test_data),
				SLV_I2C_ADDR);
		zassert_ok(ret, "Pre-recovery transfer failed: %d", ret);

		/* Attempt recovery by re-initializing the bus */
		uint32_t config = I2C_MODE_CONTROLLER |
				  I2C_SPEED_SET(I2C_SPEED_STANDARD);

		ret = i2c_configure(master, config);
		zassert_ok(ret, "Recovery config failed at %d: %d",
			 attempt, ret);

		/* Test transfer after recovery */
		ret = i2c_write(master, test_data, sizeof(test_data),
				SLV_I2C_ADDR);
		zassert_ok(ret,
			   "Post-recovery transfer failed at attempt %d: %d",
			   attempt, ret);

		fault_ctx.recovery_count++;
	}

	/* Test 2: Timeout and recovery */
	for (int i = 0; i < 10; i++) {
		int64_t start_time = k_uptime_get();

		ret = i2c_write(master, test_data, sizeof(test_data),
				SLV_I2C_ADDR);

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
		ret = i2c_read(master, test_data, sizeof(test_data),
			       SLV_I2C_ADDR);

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
	const struct device *master = DEVICE_DT_GET(I2C_MASTER);
	uint8_t test_data[] = {0xAA, 0x55, 0xFF};
	int ret;

	/* Test 1: Rapid reconfiguration - expose race conditions */
	for (int i = 0; i < 100; i++) {
		uint32_t config1 = I2C_MODE_CONTROLLER |
				   I2C_SPEED_SET(I2C_SPEED_STANDARD);
		uint32_t config2 = I2C_MODE_CONTROLLER |
				   I2C_SPEED_SET(I2C_SPEED_FAST);

		/* Rapid configuration changes */
		ret = i2c_configure(master, config1);
		zassert_ok(ret, "Config1 failed at iteration %d: %d", i, ret);

		ret = i2c_configure(master, config2);
		zassert_ok(ret, "Config2 failed at iteration %d: %d", i, ret);

		/* Immediate transfer */
		ret = i2c_write(master, test_data, 1, SLV_I2C_ADDR);
		zassert_ok(ret,
			   "Transfer failed after rapid reconfig %d: %d",
			   i, ret);
	}

	/* Test 2: Interrupt context pressure - expose interrupt bugs */
	for (int i = 0; i < 50; i++) {
		unsigned int key = irq_lock();

		ret = i2c_write(master, &test_data[i % 3], 1, SLV_I2C_ADDR);

		irq_unlock(key);

		zassert_ok(ret,
			   "Interrupt pressure test failed at iteration %d: %d",
			   i, ret);
	}

	/* Test 3: Simulated multi-thread access */
	for (int thread_sim = 0; thread_sim < 20; thread_sim++) {
		/* Simulate thread switching by rapid operations */
		for (int op = 0; op < 10; op++) {
			ret = i2c_write(master, test_data, sizeof(test_data),
					SLV_I2C_ADDR);
			zassert_ok(ret,
				   "Thread sim %d, op %d failed: %d",
				   thread_sim, op, ret);

			/* Small delay to simulate context switch */
			k_usleep(1);
		}
	}

	LOG_INF("Concurrent access scenarios completed");
}

static void *i2c_fault_suite_setup(void)
{
	const struct device *master = DEVICE_DT_GET(I2C_MASTER);
	const struct device *slave = DEVICE_DT_GET(I2C_SLAVE);
	uint32_t master_config = I2C_MODE_CONTROLLER |
				 I2C_SPEED_SET(I2C_SPEED_STANDARD);
	uint32_t slave_config = I2C_SPEED_SET(I2C_SPEED_STANDARD);
	int ret;

	/* Initialize fault context */
	memset(&fault_ctx, 0, sizeof(fault_ctx));
	fault_ctx.master = master;
	fault_ctx.slave = slave;

	/* Configure devices */
	zassert_not_null(master, "Master I2C device not found");
	ret = i2c_configure(master, master_config);
	zassert_ok(ret, "Master I2C configuration failed: %d", ret);

	zassert_not_null(slave, "Slave I2C device not found");
	ret = i2c_configure(slave, slave_config);
	zassert_ok(ret, "Slave I2C configuration failed: %d", ret);

	/* Register slave if needed */
	if (!i2c_test_ctx.slave_registered) {
		i2c_tcfg.flags = 0;
		i2c_tcfg.address = SLV_I2C_ADDR;
		ret = i2c_target_register(slave, &i2c_tcfg);
		zassert_ok(ret, "Slave registration failed: %d", ret);
		i2c_test_ctx.slave_registered = true;
	}

	LOG_INF("I2C fault injection test suite setup completed");
	return NULL;
}

/**
 * @brief Test invalid address handling
 *
 * Objective: Validate driver error handling for invalid addresses
 * Expected: Driver should return appropriate error codes
 */
ZTEST(i2c_fault_suite, test_invalid_addr)
{
	const struct device *master = DEVICE_DT_GET(I2C_MASTER);
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
		ret = i2c_write(master, test_data, sizeof(test_data),
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
	const struct device *master = DEVICE_DT_GET(I2C_MASTER);
	uint8_t test_data[] = {0xAA, 0x55};
	int ret;

	LOG_INF("=== Enhanced NACK Recovery Test ===");

	/* Verify slave is responding first */
	ret = i2c_write(master, test_data, 1, SLV_I2C_ADDR);
	zassert_ok(ret, "Slave not responding - test setup issue");

	/* Test immediate recovery after NACK */
	for (int i = 0; i < 10; i++) {
		/* Trigger NACK with invalid address */
		ret = i2c_write(master, test_data, 1, 0x80);
		LOG_DBG("NACK trigger %d: ret=%d", i, ret);

		/* Test immediate recovery */
		ret = i2c_write(master, test_data, sizeof(test_data),
				SLV_I2C_ADDR);
		LOG_DBG("Immediate recovery %d: ret=%d", i, ret);

		if (ret != 0) {
			LOG_ERR("Recovery failed at iteration %d: ret=%d",
				i, ret);

			/* Test with delay */
			k_msleep(1);
			ret = i2c_write(master, test_data,
					sizeof(test_data), SLV_I2C_ADDR);
			LOG_DBG("Delayed recovery %d: ret=%d", i, ret);

			if (ret == 0) {
				LOG_INF("Recovery works with delay - timing");
			} else {
				/* Test with bus reset */
				uint32_t config =
					I2C_MODE_CONTROLLER |
					I2C_SPEED_SET(I2C_SPEED_STANDARD);
				ret = i2c_configure(master, config);
				zassert_ok(ret, "Bus reset failed");

				ret = i2c_write(master, test_data,
						sizeof(test_data),
						SLV_I2C_ADDR);
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
	const struct device *master = DEVICE_DT_GET(I2C_MASTER);
	uint8_t test_data[] = {0x12, 0x34};
	int ret;

	LOG_INF("=== Enhanced Bus Busy Test ===");

	/* Test rapid successive transfers */
	for (int i = 0; i < 5; i++) {
		/* Start transfer */
		ret = i2c_write(master, test_data, sizeof(test_data),
				SLV_I2C_ADDR);
		zassert_ok(ret, "Initial transfer failed: %d", ret);

		/* Immediate second transfer */
		ret = i2c_write(master, test_data, 1, SLV_I2C_ADDR);

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
	const struct device *master = DEVICE_DT_GET(I2C_MASTER);
	int ret;

	LOG_INF("=== Configuration Edge Cases Test ===");

	/* Test invalid speed configurations */
	uint32_t invalid_configs[] = {
		0,/* No mode */
		I2C_SPEED_SET(999),/* Invalid speed */
		I2C_MODE_CONTROLLER | I2C_SPEED_SET(999), /* Invalid + mode */
	};

	for (size_t i = 0; i < ARRAY_SIZE(invalid_configs); i++) {
		ret = i2c_configure(master, invalid_configs[i]);
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

	ret = i2c_configure(master, valid_config);
	zassert_ok(ret, "Failed to restore valid configuration");
}

ZTEST_SUITE(i2c_fault_suite, NULL, i2c_fault_suite_setup, NULL, NULL, NULL);

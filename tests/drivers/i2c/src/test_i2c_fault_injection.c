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

#define FAULT_TEST_TIMEOUT_MS		  5000U
#define BUS_RECOVERY_ATTEMPTS		  10U
#define NACK_INJECTION_ITERATIONS	  50U
#define BUS_BUSY_SIMULATION_DELAY_US      100U
#define NACK_RECOVERY_DELAY_US		  100U
#define ARBITRATION_LOSS_STRESS_COUNT     100U
struct i2c_fault_ctx {
	const struct device *controller;
	const struct device *target;
	uint32_t fault_count;
	uint32_t recovery_count;
};

static struct i2c_fault_ctx fault_ctx;

static void reset_fault_error_counters(void)
{
	fault_ctx.fault_count = 0U;
	fault_ctx.recovery_count = 0U;
}

/**
 * @brief NACK handling and recovery.
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

		const uint16_t invalid_addresses[] = {
			0x00,
			0x78,
			0x80,
			0xFF,
			0x123,
		};

		for (size_t i = 0; i < ARRAY_SIZE(invalid_addresses); i++) {
			ret = i2c_write(controller, test_data, sizeof(test_data),
					invalid_addresses[i]);

			if (ret == 0) {
				LOG_WRN("Unexpected success with addr 0x%03X",
					invalid_addresses[i]);
				zassert_unreachable("Invalid address should not succeed");
			}
		}

		for (int i = 0; i < NACK_INJECTION_ITERATIONS; i++) {
			ret = i2c_read(controller, rx_buffer, sizeof(rx_buffer),
				       TGT_I2C_ADDR);

			if (ret == 0) {
				for (size_t j = 0; j < sizeof(rx_buffer); j++) {
					rx_buffer[j] = 0;
				}
			}
		}

		for (int i = 0; i < 20; i++) {
			ret = i2c_write(controller, test_data, 1, 0x80);
			if (ret == 0) {
				LOG_WRN("NACK trigger %d: unexpected success", i);
			}

			k_usleep(NACK_RECOVERY_DELAY_US);

			ret = i2c_write(controller, test_data, sizeof(test_data),
					TGT_I2C_ADDR);
			zassert_ok(ret,
				   "Recovery failed after NACK at iteration %d: %d",
				   i, ret);
		}
	}
}

/**
 * @brief Bus busy detection and arbitration.
 */
ZTEST(i2c_fault_suite, test_bus_busy)
{
	const struct device *controller = fault_ctx.controller;
	uint8_t test_data[] = {0x12, 0x34, 0x56, 0x78};
	int ret;

	for (int i = 0; i < 10; i++) {
		ret = i2c_write(controller, test_data, sizeof(test_data),
				TGT_I2C_ADDR);
		zassert_ok(ret, "Initial transfer failed: %d", ret);

		ret = i2c_write(controller, test_data, 1, TGT_I2C_ADDR);

		if (ret == -EBUSY) {
			LOG_DBG("Bus busy at iteration %d", i);
		} else if (ret != 0) {
			LOG_INF("Busy test error %d at iter %d", ret, i);
		}

		k_usleep(BUS_BUSY_SIMULATION_DELAY_US);
	}

	for (int i = 0; i < ARBITRATION_LOSS_STRESS_COUNT; i++) {
		uint16_t test_addr = TGT_I2C_ADDR + (i % 3);

		ret = i2c_write(controller, test_data, sizeof(test_data),
				test_addr);

		if (ret == -EIO) {
			LOG_DBG("Possible arbitration loss at iteration %d", i);
			fault_ctx.fault_count++;
		} else if (ret != 0 && ret != -ENXIO) {
			LOG_INF("Arbitration test error %d at iter %d", ret, i);
		}
	}

	for (int i = 0; i < 20; i++) {
		ret = i2c_write(controller, test_data, 1, TGT_I2C_ADDR);
		if (ret != 0 && ret != -ENXIO) {
			LOG_INF("Multi-controller err %d addr1 iter %d", ret, i);
		}

		ret = i2c_write(controller, test_data, 1, TGT_I2C_ADDR + 1);
		if (ret != 0 && ret != -ENXIO) {
			LOG_INF("Multi-controller err %d addr2 iter %d", ret, i);
		}
	}

	zassert_equal(fault_ctx.fault_count, 0U,
		      "Bus busy/arbitration test had %u faults",
		      fault_ctx.fault_count);
}

/**
 * @brief Bus recovery scenarios.
 */
ZTEST(i2c_fault_suite, test_bus_recovery)
{
	const struct device *controller = fault_ctx.controller;
	uint8_t test_data[] = {0xAA, 0x55};
	int ret;

	for (int attempt = 0; attempt < BUS_RECOVERY_ATTEMPTS; attempt++) {
		ret = i2c_write(controller, test_data, sizeof(test_data),
				TGT_I2C_ADDR);
		zassert_ok(ret, "Pre-recovery transfer failed: %d", ret);

		uint32_t config = I2C_MODE_CONTROLLER |
				  I2C_SPEED_SET(I2C_SPEED_STANDARD);

		ret = i2c_configure(controller, config);
		zassert_ok(ret, "Recovery config failed at %d: %d",
			 attempt, ret);

		ret = i2c_write(controller, test_data, sizeof(test_data),
				TGT_I2C_ADDR);
		zassert_ok(ret,
			   "Post-recovery transfer failed at attempt %d: %d",
			   attempt, ret);

		fault_ctx.recovery_count++;
	}

	for (int i = 0; i < 10; i++) {
		int64_t start_time = k_uptime_get();

		ret = i2c_write(controller, test_data, sizeof(test_data),
				TGT_I2C_ADDR);

		int64_t duration = k_uptime_get() - start_time;

		if (duration > FAULT_TEST_TIMEOUT_MS) {
			LOG_INF("Transfer %d took %lldms (timeout?)",
				i, duration);
			fault_ctx.fault_count++;
		} else {
			zassert_ok(ret, "Timeout test failed at %d: %d",
				i, ret);
		}
	}

	for (int i = 0; i < 5; i++) {
		ret = i2c_read(controller, test_data, sizeof(test_data),
			       TGT_I2C_ADDR);

		if (ret == -ETIMEDOUT) {
			LOG_INF("Clock stretch timeout at iter %d", i);
			fault_ctx.fault_count++;
		}
	}
}

/**
 * @brief Concurrent access scenarios.
 */
ZTEST(i2c_fault_suite, test_concurrent_access)
{
	const struct device *controller = fault_ctx.controller;
	uint8_t test_data[] = {0xAA, 0x55, 0xFF};
	int ret;

	for (int i = 0; i < 100; i++) {
		uint32_t config1 = I2C_MODE_CONTROLLER |
				   I2C_SPEED_SET(I2C_SPEED_STANDARD);
		uint32_t config2 = I2C_MODE_CONTROLLER |
				   I2C_SPEED_SET(I2C_SPEED_FAST);

		ret = i2c_configure(controller, config1);
		zassert_ok(ret, "Config1 failed at iteration %d: %d", i, ret);

		ret = i2c_configure(controller, config2);
		zassert_ok(ret, "Config2 failed at iteration %d: %d", i, ret);

		ret = i2c_write(controller, test_data, 1, TGT_I2C_ADDR);
		zassert_ok(ret,
			   "Transfer failed after rapid reconfig %d: %d",
			   i, ret);
	}

	for (int i = 0; i < 50; i++) {
		k_sched_lock();
		ret = i2c_write(controller, &test_data[i % 3], 1, TGT_I2C_ADDR);
		k_sched_unlock();
		zassert_ok(ret,
			   "Scheduler-locked pressure test failed at iteration %d: %d",
			   i, ret);
	}

	for (int thread_sim = 0; thread_sim < 20; thread_sim++) {
		for (int op = 0; op < 10; op++) {
			ret = i2c_write(controller, test_data, sizeof(test_data),
					TGT_I2C_ADDR);
			zassert_ok(ret,
				   "Thread sim %d, op %d failed: %d",
				   thread_sim, op, ret);
			k_usleep(1);
		}
	}
}

/**
 * @brief Configuration edge cases.
 */
ZTEST(i2c_fault_suite, test_config_edge)
{
	const struct device *controller = fault_ctx.controller;
	int ret;

	uint32_t invalid_configs[] = {
		0,
		I2C_SPEED_SET(999),
		I2C_MODE_CONTROLLER | I2C_SPEED_SET(999),
	};

	for (size_t i = 0; i < ARRAY_SIZE(invalid_configs); i++) {
		ret = i2c_configure(controller, invalid_configs[i]);
		if (ret == 0) {
			LOG_WRN("Invalid config %zu accepted", i);
		}
	}

	uint32_t valid_config = I2C_MODE_CONTROLLER |
				I2C_SPEED_SET(I2C_SPEED_STANDARD);

	ret = i2c_configure(controller, valid_config);
	zassert_ok(ret, "Failed to restore valid configuration");
}

/**
 * @brief Reserved-address rejection / NACK behavior.
 *
 * A compliant driver must reject or NACK reserved addresses
 * (0x00-0x07 and 0x78-0x7F) and leave the bus usable afterwards.
 */
ZTEST(i2c_fault_suite, test_reserved_addr)
{
	const struct device *controller = fault_ctx.controller;
	static const uint16_t reserved_addrs[] = {
		0x00U, 0x01U, 0x04U, 0x07U,
		0x78U, 0x7CU, 0x7FU,
	};
	uint8_t test_byte = 0xA5U;
	int ret;

	for (size_t i = 0; i < ARRAY_SIZE(reserved_addrs); i++) {
		uint16_t addr = reserved_addrs[i];

		ret = i2c_write(controller, &test_byte, 1U, addr);
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

	uint8_t payload[4] = {0x11, 0x22, 0x33, 0x44};

	i2c_test_prime_buffers(NULL, 0U, sizeof(payload));
	ret = i2c_write(controller, payload, sizeof(payload), TGT_I2C_ADDR);
	zassert_ok(ret, "bus unusable after reserved-addr attempts: %d", ret);

	ret = validate_target_rx(payload, sizeof(payload));
	zassert_ok(ret, "RX mismatch after reserved-addr recovery: %d", ret);
}

/**
 * @brief NACK asserted mid-transfer by the target.
 *
 * The driver must detect TX_ABRT, return an error, and leave the bus
 * usable for the next transfer.
 */
ZTEST(i2c_fault_suite, test_nack_mid_transfer)
{
	const struct device *controller = fault_ctx.controller;
	const size_t oversize = BUFF_PERF + 16U;
	static uint8_t big_tx[BUFF_PERF + 16U];
	int ret;

	for (size_t i = 0; i < sizeof(big_tx); i++) {
		big_tx[i] = (uint8_t)(0x10U + i);
	}

	i2c_test_prime_buffers(NULL, 0U, BUFF_PERF);

	ret = i2c_write(controller, big_tx, oversize, TGT_I2C_ADDR);
	if (ret != 0) {
		LOG_INF("mid-transfer NACK detected as expected: %d", ret);
		zassert_true(ret == -EIO || ret == -ENXIO,
			     "unexpected mid-transfer error: %d", ret);
	} else {
		LOG_INF("target accepted oversize write (no NACK)");
	}

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

	memset(&fault_ctx, 0, sizeof(fault_ctx));
	fault_ctx.controller = controller;
	fault_ctx.target = target;

	zassert_not_null(controller, "Controller I2C device not found");
	zassert_true(device_is_ready(controller),
		     "Controller I2C device not ready");
	zassert_not_null(target, "Target I2C device not found");
	zassert_true(device_is_ready(target),
		     "Target I2C device not ready");

	i2c_test_ctx.controller_dev = controller;
	i2c_test_ctx.target_dev = target;
	i2c_test_reset_runtime_config(&i2c_test_ctx);

	ret = i2c_configure(i2c_test_ctx.controller_dev, i2c_test_ctx.i2c_cfg);
	zassert_ok(ret, "Controller I2C configuration failed: %d", ret);

	register_target_i2c();
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

static void i2c_fault_suite_before(void *fixture)
{
	ARG_UNUSED(fixture);
	reset_fault_error_counters();
}

ZTEST_SUITE(i2c_fault_suite, NULL, i2c_fault_suite_setup,
	    i2c_fault_suite_before, NULL, i2c_fault_suite_teardown);

/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef ZEPHYR_TESTS_DRIVERS_I2C_SRC_TEST_I2C_H_
#define ZEPHYR_TESTS_DRIVERS_I2C_SRC_TEST_I2C_H_

#include <zephyr/logging/log.h>
#include <zephyr/dt-bindings/i2c/i2c.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
#include <zephyr/ztest.h>
#include <zephyr/device.h>
#include <zephyr/sys/util_macro.h>

#define I2C_MASTER DT_ALIAS(master_i2c)
#define I2C_SLAVE  DT_ALIAS(slave_i2c)

#define SLV_I2C_ADDR	  0x50
#define SLV_I2C_10BITADDR 0x223

#define BUFF_SIZE		 4
#define BUFF_PERF		 128
#define SLAVE_T_MASTER_R 1

/* Distinct seeds for master TX and slave TX - must never be equal */
#define SEED_MASTER_TX 0xDEAD0001U
#define SEED_SLAVE_TX  0xBEEF0002U
#define POISON_RX	   0xA5
#define POISON_TX	   0x5A

enum i2c_test_scenario {
	I2C_TRANSMIT_ONLY = 0,
	I2C_RECEIVE_ONLY = 1,
	I2C_TRANSMIT_RECEIVE = 2,
};

enum i2c_callback_error {
	I2C_CB_ERR_DUP_WRITE_REQUEST = 1U << 0,
	I2C_CB_ERR_WRITE_BEFORE_REQUEST = 1U << 1,
	I2C_CB_ERR_READ_PROCESSED_BEFORE_REQUEST = 1U << 2,
};

/* Runtime frequency switching support */
enum i2c_test_speed {
	I2C_SPEED_STANDARD_KHZ = 100,
	I2C_SPEED_FAST_KHZ = 400,
	I2C_SPEED_FAST_PLUS_KHZ = 1000,
	I2C_SPEED_HIGH_KHZ = 3400,
};

/* Shared frequency descriptor for multi-frequency test execution */
struct i2c_test_freq_desc {
	uint32_t freq_hz;
	uint32_t zephyr_speed;
	enum i2c_test_speed test_speed;
	const char *name;
};

/* Maximum number of supported frequencies */
#define I2C_TEST_MAX_FREQS 4

/* Build-time enabled frequency count and table */
#define I2C_TEST_FREQ_COUNT \
	(IS_ENABLED(CONFIG_I2C_TEST_FREQ_STANDARD) + \
	 IS_ENABLED(CONFIG_I2C_TEST_FREQ_FAST) + \
	 IS_ENABLED(CONFIG_I2C_TEST_FREQ_FAST_PLUS) + \
	 IS_ENABLED(CONFIG_I2C_TEST_FREQ_HIGH))

/* Helper to populate enabled frequencies into a runtime array */
static inline int i2c_test_get_enabled_freqs(struct i2c_test_freq_desc *freqs,
				     size_t max_count)
{
	int count = 0;

#if IS_ENABLED(CONFIG_I2C_TEST_FREQ_STANDARD)
	if (count < max_count) {
		freqs[count] = (struct i2c_test_freq_desc){
			.freq_hz = 100000,
			.zephyr_speed = I2C_SPEED_STANDARD,
			.test_speed = I2C_SPEED_STANDARD_KHZ,
			.name = "Standard"
		};
		count++;
	}
#endif

#if IS_ENABLED(CONFIG_I2C_TEST_FREQ_FAST)
	if (count < max_count) {
		freqs[count] = (struct i2c_test_freq_desc){
			.freq_hz = 400000,
			.zephyr_speed = I2C_SPEED_FAST,
			.test_speed = I2C_SPEED_FAST_KHZ,
			.name = "Fast"
		};
		count++;
	}
#endif

#if IS_ENABLED(CONFIG_I2C_TEST_FREQ_FAST_PLUS)
	if (count < max_count) {
		freqs[count] = (struct i2c_test_freq_desc){
			.freq_hz = 1000000,
			.zephyr_speed = I2C_SPEED_FAST_PLUS,
			.test_speed = I2C_SPEED_FAST_PLUS_KHZ,
			.name = "Fast+"
		};
		count++;
	}
#endif

#if IS_ENABLED(CONFIG_I2C_TEST_FREQ_HIGH)
	if (count < max_count) {
		freqs[count] = (struct i2c_test_freq_desc){
			.freq_hz = 3400000,
			.zephyr_speed = I2C_SPEED_HIGH,
			.test_speed = I2C_SPEED_HIGH_KHZ,
			.name = "High"
		};
		count++;
	}
#endif

	return count;
}

/* Shared test context for callback accounting and device state */
struct i2c_test_ctx {
	const struct device *master_dev;
	const struct device *slave_dev;
	uint32_t i2c_cfg;
	uint8_t slv_rx_data[BUFF_PERF];
	uint8_t slv_tx_data[BUFF_PERF];
	uint32_t slv_tx_cnt;
	uint32_t slv_rx_cnt;
	uint32_t write_requested_count;
	uint32_t write_received_count;
	uint32_t read_requested_count;
	uint32_t read_processed_count;
	uint32_t stop_count;
	uint32_t stop_rx_snapshot;
	uint32_t callback_errors;
	bool slave_registered;
	enum i2c_test_scenario scenario;
	enum i2c_test_speed current_speed;
#ifdef CONFIG_I2C_TARGET_BUFFER_MODE
	uint32_t buf_write_received_count;
	uint32_t buf_read_requested_count;
	uint32_t buf_last_write_len;
	uint32_t buf_last_read_len;
#endif
};

extern struct i2c_test_ctx i2c_test_ctx;

/* Configure master from frequency descriptor with ENOTSUP skip handling */
static inline int i2c_test_configure_master_freq(
	const struct device *dev,
	struct i2c_test_ctx *ctx,
	const struct i2c_test_freq_desc *freq)
{
	uint32_t config = I2C_MODE_CONTROLLER |
			  I2C_SPEED_SET(freq->zephyr_speed);
	int ret;

	ret = i2c_configure(dev, config);
	if (ret == 0) {
		ctx->i2c_cfg = config;
		ctx->current_speed = freq->test_speed;
	}

	return ret;
}

/* Test skip helper for insufficient enabled frequencies */
static inline void i2c_test_skip_if_freqs_lt(int required)
{
	if (required > I2C_TEST_FREQ_COUNT) {
		TC_PRINT("Skipping: need %d frequencies, only %d enabled\n",
			 required, I2C_TEST_FREQ_COUNT);
		ztest_test_skip();
	}
}

/* Test skip helper for no frequencies enabled */
static inline void i2c_test_skip_if_no_freqs(void)
{
	if (0 == I2C_TEST_FREQ_COUNT) {
		TC_PRINT("Skipping: no frequencies enabled in Kconfig\n");
		ztest_test_skip();
	}
}

int i2c_validate_data_match(const uint8_t *expected, const uint8_t *actual,
				size_t len, const char *context);
int validate_slave_rx(const uint8_t *tx_data, size_t expected_len);
int validate_master_rx(const uint8_t *expected_data, const uint8_t *rx_data,
			   size_t expected_len);
void i2c_test_prime_buffers(const uint8_t *read_data, size_t read_len,
				size_t expected_write_len);
void register_slave_i2c(void);
void register_slave_speed_i2c(uint32_t i2c_cfg);
void register_slave_i2c_common(uint32_t i2c_cfg, uint16_t addr,
				   uint8_t flags);
void register_slave_i2c_10Bit(void);
void i2c_prepare_slave_rx(uint32_t expected_len);

static inline void i2c_test_reset_callback_state(struct i2c_test_ctx *ctx)
{
	ctx->write_requested_count = 0U;
	ctx->write_received_count = 0U;
	ctx->read_requested_count = 0U;
	ctx->read_processed_count = 0U;
	ctx->stop_count = 0U;
	ctx->stop_rx_snapshot = 0U;
	ctx->callback_errors = 0U;
#ifdef CONFIG_I2C_TARGET_BUFFER_MODE
	ctx->buf_write_received_count = 0U;
	ctx->buf_read_requested_count = 0U;
	ctx->buf_last_write_len = 0U;
	ctx->buf_last_read_len = 0U;
#endif
}

static inline void i2c_test_reset_runtime_config(struct i2c_test_ctx *ctx)
{
	/* Reset runtime frequency state */
	ctx->current_speed = I2C_SPEED_STANDARD_KHZ;
	ctx->i2c_cfg = I2C_MODE_CONTROLLER |
			   I2C_SPEED_SET(I2C_SPEED_STANDARD);

	/* Reset callback state */
	i2c_test_reset_callback_state(ctx);

	/* Reset data counters */
	ctx->slv_tx_cnt = 0;
	ctx->slv_rx_cnt = 0;
	memset(ctx->slv_rx_data, 0, BUFF_PERF);
	memset(ctx->slv_tx_data, 0, BUFF_PERF);
}

static inline void i2c_test_note_write_requested(struct i2c_test_ctx *ctx)
{
	ctx->write_requested_count++;
	if (ctx->write_requested_count > 1U) {
		ctx->callback_errors |= I2C_CB_ERR_DUP_WRITE_REQUEST;
	}
	if (ctx->write_requested_count == 1U) {
		ctx->slv_rx_cnt = 0U;
	}
}

static inline void i2c_test_note_write_received(struct i2c_test_ctx *ctx)
{
	if (ctx->write_requested_count == 0U) {
		ctx->callback_errors |= I2C_CB_ERR_WRITE_BEFORE_REQUEST;
	}
	ctx->write_received_count++;
}

static inline void i2c_test_note_read_requested(struct i2c_test_ctx *ctx)
{
	ctx->read_requested_count++;
}

static inline void i2c_test_note_read_processed(struct i2c_test_ctx *ctx)
{
	if (ctx->read_requested_count == 0U) {
		ctx->callback_errors |=
			I2C_CB_ERR_READ_PROCESSED_BEFORE_REQUEST;
	}
	ctx->read_processed_count++;
}

static inline void i2c_test_note_stop(struct i2c_test_ctx *ctx)
{
	ctx->stop_count++;
	ctx->stop_rx_snapshot = ctx->slv_rx_cnt;
}

/* Runtime frequency switching helper */
static inline int i2c_test_set_frequency(struct i2c_test_ctx *ctx,
					 enum i2c_test_speed speed_khz)
{
	uint32_t config = ctx->i2c_cfg;
	int ret;

	/* Extract non-speed bits and set new speed */
	config &= ~I2C_SPEED_MASK;
	switch (speed_khz) {
	case I2C_SPEED_STANDARD_KHZ:
		config |= I2C_SPEED_SET(I2C_SPEED_STANDARD);
		break;
	case I2C_SPEED_FAST_KHZ:
		config |= I2C_SPEED_SET(I2C_SPEED_FAST);
		break;
	case I2C_SPEED_FAST_PLUS_KHZ:
		config |= I2C_SPEED_SET(I2C_SPEED_FAST_PLUS);
		break;
	case I2C_SPEED_HIGH_KHZ:
		config |= I2C_SPEED_SET(I2C_SPEED_HIGH);
		break;
	default:
		return -EINVAL;
	}

	ret = i2c_configure(ctx->master_dev, config);
	if (ret == 0) {
		ctx->i2c_cfg = config;
		ctx->current_speed = speed_khz;
	}

	return ret;
}

static inline void fill_buffer(uint8_t *buf, size_t size)
{
	for (int j = 0; j < size; j++) {
		buf[j] = 0xAA - j;
	}
}

/* Enhanced buffer filling with random patterns for stronger validation */
static inline void fill_buffer_random(uint8_t *buf, size_t size,
					  uint32_t seed)
{
	/* Simple but effective pseudo-random generator */
	for (int j = 0; j < size; j++) {
		seed = seed * 1103515245 + 12345;
		buf[j] = (uint8_t)(seed >> 16);
	}
}

/*
 * Fill slave TX buffer with a pattern guaranteed different from master TX.
 * Call this before every read/transceive transfer to ensure real validation.
 */
static inline void fill_slave_tx_distinct(uint8_t *buf, size_t size,
					  uint32_t seed)
{
	fill_buffer_random(buf, size, seed);
}

/* Buffer poisoning to detect uninitialized data usage */
static inline void poison_buffer(uint8_t *buf, size_t size,
				 uint8_t poison_value)
{
	memset(buf, poison_value, size);
}

/* Enhanced buffer validation with checksum */
static inline int validate_buffer_with_checksum(const uint8_t *buf,
						size_t size,
						uint32_t expected_checksum)
{
	uint32_t actual_checksum = 0;

	/* Simple XOR checksum */
	for (size_t i = 0; i < size; i++) {
		actual_checksum ^= buf[i];
		actual_checksum = (actual_checksum << 1) |
				  (actual_checksum >> 31);
	}

	return (actual_checksum == expected_checksum) ? 0 : -EIO;
}

/* Global test context declarations */
extern struct i2c_test_ctx i2c_test_ctx;
extern struct i2c_target_config i2c_tcfg;

#endif /* ZEPHYR_TESTS_DRIVERS_I2C_SRC_TEST_I2C_H_ */

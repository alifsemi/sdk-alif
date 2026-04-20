/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include "test_i2c.h"
#include "zephyr/drivers/i2c.h"
#include "zephyr/linker/section_tags.h"
#include "zephyr/toolchain.h"
#include <zephyr/pm/device.h>
LOG_MODULE_REGISTER(alif_i2c_test, LOG_LEVEL_INF);

/* Expected target receive buffer size */
static uint32_t tgt_rx_expected = BUFF_SIZE;

/* Global test context instance */
struct i2c_test_ctx i2c_test_ctx;

static void prepare_target_rx(uint32_t expected_len);

void i2c_prepare_target_rx(uint32_t expected_len)
{
	prepare_target_rx(expected_len);
}

int i2c_validate_data_match(const uint8_t *expected, const uint8_t *actual,
				size_t len, const char *context)
{
	/* Enhanced validation with detailed diagnostics */
	if (len == 0) {
		LOG_DBG("%s: zero-length validation", context);
		return 0;
	}

	for (size_t i = 0; i < len; i++) {
		if (expected[i] != actual[i]) {
			LOG_ERR("%s mismatch at idx %zu: exp 0x%02X got 0x%02X",
				context, i, expected[i], actual[i]);

			/* Detect poison pattern - indicates
			 * uninitialized data usage
			 */
			if (actual[i] == 0x5A || actual[i] == 0xA5) {
				LOG_ERR("%s: UNINIT (poison 0x%02X) idx %zu",
					context, actual[i], i);
			}
			return -EIO;
		}
	}
	LOG_DBG("%s: %zu bytes validated successfully", context, len);
	return 0;
}

/**
 * @brief Prepare target TX/RX buffers and reset callback state for a transfer.
 *
 * This function sets up the test environment before each I2C transfer:
 * 1. Resets the target TX counter so the target starts from buffer index 0
 * 2. Poisons the target TX buffer with 0x5A to detect if the controller reads
 *    uninitialized data (indicates target callback failure)
 * 3. Copies expected read data into the target TX buffer (for controller-read ops)
 * 4. Prepares the target RX buffer and resets all callback counters
 *
 * The poisoning pattern (0x5A = 'Z') is chosen to be visually distinct
 * in hex dumps and unlikely to appear in valid test data.
 *
 * @param read_data Data to copy into target TX buffer (for controller-read transfers)
 * @param read_len Length of read_data
 * @param expected_write_len Expected bytes from controller-write (for RX validation)
 */
void i2c_test_prime_buffers(const uint8_t *read_data, size_t read_len,
				size_t expected_write_len)
{
	size_t copy_len = MIN(read_len, sizeof(i2c_test_ctx.tgt_tx_data));
	size_t i;

	i2c_test_ctx.tgt_tx_cnt = 0U;

	/*
	 * Poison buffer with 0x5A ('Z') to detect uninitialized reads.
	 * If the controller receives 0x5A bytes, the target's read_requested
	 * callback failed to populate the buffer before the transfer.
	 */
	poison_buffer(i2c_test_ctx.tgt_tx_data,
			  sizeof(i2c_test_ctx.tgt_tx_data), 0x5A);

	if ((read_data != NULL) && (copy_len > 0U)) {
		for (i = 0U; i < copy_len; i++) {
			i2c_test_ctx.tgt_tx_data[i] = read_data[i];
		}
	}

	prepare_target_rx(expected_write_len);
	i2c_test_reset_callback_state(&i2c_test_ctx);
}

static void prepare_target_rx(uint32_t expected_len)
{
	tgt_rx_expected = MIN(expected_len, (uint32_t)BUFF_PERF);
#ifdef CONFIG_I2C_TARGET_BUFFER_MODE
	tgt_rx_expected = MIN(tgt_rx_expected,
				  (uint32_t)CONFIG_I2C_TAR_DATA_BUF_MAX_LEN);
#endif

	/* Poison RX buffer to detect uninitialized data usage */
	poison_buffer(i2c_test_ctx.tgt_rx_data,
			  sizeof(i2c_test_ctx.tgt_rx_data), 0xA5);
	i2c_test_ctx.tgt_rx_cnt = 0U;
}

int validate_target_rx(const uint8_t *tx_data, size_t expected_len)
{
	int ret;
	size_t actual_expected = MIN(expected_len, (size_t)tgt_rx_expected);

	if (i2c_test_ctx.tgt_rx_cnt != actual_expected) {
		LOG_ERR("I2C target RX count %u, expected %zu",
			i2c_test_ctx.tgt_rx_cnt, actual_expected);
		return -EIO;
	}

	ret = i2c_validate_data_match(tx_data, i2c_test_ctx.tgt_rx_data,
					  actual_expected,
					  "I2C Controller TX -> Target RX");
	if (ret != 0) {
		return ret;
	}

	LOG_DBG("I2C controller TX and target RX data match");
	return 0;
}

int validate_controller_rx(const uint8_t *expected_data, const uint8_t *rx_data,
			   size_t expected_len)
{
	int ret;

	ret = i2c_validate_data_match(expected_data, rx_data, expected_len,
					  "I2C Target TX -> Controller RX");
	if (ret != 0) {
		return ret;
	}

	LOG_DBG("I2C target TX and controller RX data match");
	return 0;
}

/* Target callback functions */
static int cb_i2c_target_write_requested(struct i2c_target_config *config)
{
	ARG_UNUSED(config);
	i2c_test_note_write_requested(&i2c_test_ctx);
	return 0;
}

static int cb_i2c_target_stop(struct i2c_target_config *config)
{
	ARG_UNUSED(config);
	i2c_test_note_stop(&i2c_test_ctx);
	return 0;
}

#ifndef CONFIG_I2C_TARGET_BUFFER_MODE
/*
 * Zephyr I2C target API contract (include/zephyr/drivers/i2c.h):
 *   read_requested : invoked EXACTLY ONCE at the start of a master-read
 *                    phase; supplies the FIRST byte through *val.
 *   read_processed : invoked for EVERY SUBSEQUENT byte the master ACKs;
 *                    supplies the NEXT byte through *val.
 *
 * Both callbacks MUST populate *val from the same target-TX buffer using
 * a single monotonically advancing index (tgt_tx_cnt). Failing to set
 * *val in read_processed leaves stale/undefined data on the bus and is
 * a driver-contract violation.
 */
static int cb_i2c_target_read_requested(struct i2c_target_config *config,
					uint8_t *val)
{
	ARG_UNUSED(config);
	if (i2c_test_ctx.tgt_tx_cnt >= BUFF_PERF) {
		i2c_test_ctx.tgt_tx_cnt = 0;
	}
	*val = i2c_test_ctx.tgt_tx_data[i2c_test_ctx.tgt_tx_cnt++];
	i2c_test_note_read_requested(&i2c_test_ctx);
	LOG_DBG("read_requested: first byte 0x%02X (idx 0)", *val);
	return 0;
}

static int cb_i2c_target_write_received(struct i2c_target_config *config,
					uint8_t val)
{
	ARG_UNUSED(config);
	if (i2c_test_ctx.tgt_rx_cnt < tgt_rx_expected) {
		i2c_test_ctx.tgt_rx_data[i2c_test_ctx.tgt_rx_cnt++] = val;
		LOG_DBG("Target RX [%u]: 0x%02X (total: %u)",
			i2c_test_ctx.tgt_rx_cnt - 1, val,
			i2c_test_ctx.tgt_rx_cnt);
	} else {
		LOG_DBG("Ignoring extra target RX byte 0x%02X", val);
	}
	i2c_test_note_write_received(&i2c_test_ctx);
	return 0;
}

static int cb_i2c_target_read_processed(struct i2c_target_config *config,
					uint8_t *val)
{
	ARG_UNUSED(config);
	if (i2c_test_ctx.tgt_tx_cnt >= BUFF_PERF) {
		i2c_test_ctx.tgt_tx_cnt = 0;
	}
	*val = i2c_test_ctx.tgt_tx_data[i2c_test_ctx.tgt_tx_cnt++];
	i2c_test_note_read_processed(&i2c_test_ctx);
	LOG_DBG("read_processed: next byte 0x%02X (idx %u)", *val,
		i2c_test_ctx.tgt_tx_cnt - 1U);
	return 0;
}
#endif

#ifdef CONFIG_I2C_TARGET_BUFFER_MODE
static int cb_i2c_target_buf_read_requested(struct i2c_target_config *config,
						uint8_t **val, uint32_t *len)
{
	ARG_UNUSED(config);

	i2c_test_note_read_requested(&i2c_test_ctx);
	i2c_test_ctx.buf_read_requested_count++;
	i2c_test_ctx.buf_last_read_len =
		MIN((uint32_t)BUFF_PERF,
			(uint32_t)CONFIG_I2C_TAR_DATA_BUF_MAX_LEN);
	*val = i2c_test_ctx.tgt_tx_data;
	*len = i2c_test_ctx.buf_last_read_len;

	return 0;
}

static void cb_i2c_target_buf_write_received(
	struct i2c_target_config *config,
	uint8_t *data_buf,
	uint32_t len)
{
	uint32_t copy_len;

	ARG_UNUSED(config);

	copy_len = MIN(len, tgt_rx_expected);
	memcpy(i2c_test_ctx.tgt_rx_data, data_buf, copy_len);
	i2c_test_ctx.tgt_rx_cnt = copy_len;
	i2c_test_ctx.buf_write_received_count++;
	i2c_test_ctx.buf_last_write_len = copy_len;
	i2c_test_note_write_received(&i2c_test_ctx);
}
#endif

static const struct i2c_target_callbacks i2c_t_cb = {
	.write_requested = &cb_i2c_target_write_requested,
#ifdef CONFIG_I2C_TARGET_BUFFER_MODE
	.buf_read_requested = &cb_i2c_target_buf_read_requested,
	.buf_write_received = &cb_i2c_target_buf_write_received,
#else
	.read_requested = &cb_i2c_target_read_requested,
	.write_received = &cb_i2c_target_write_received,
	.read_processed = &cb_i2c_target_read_processed,
#endif
	.stop = &cb_i2c_target_stop,
};

struct i2c_target_config i2c_tcfg = {
	.callbacks = &i2c_t_cb,
};

/*
 * Function will send a set of predefined data
 * 0xAA 0XAB 0xAC 0xAD from i2c master to i2c Slave
 * also the master will read one byte data 0x60 from the target.
 * Predefined data values are randomly chosen, just to verify
 * write and read capabilities of the i2c instances.
 * Function will continuously print the write and read datas.
 */

int start_i2c_transceive(void)
{
	uint8_t tx_data[BUFF_SIZE];
	uint8_t rx_data[BUFF_SIZE];
	int ret;
	struct i2c_msg msgs[2];
	const struct device *const i2c_controller_dev = i2c_test_ctx.controller_dev;

	i2c_test_ctx.tgt_tx_cnt = 0;
	prepare_target_rx(BUFF_SIZE);

	/* Use current context config, ensure controller mode */
	if ((i2c_test_ctx.i2c_cfg & I2C_MODE_CONTROLLER) == 0U) {
		i2c_test_ctx.i2c_cfg |= I2C_MODE_CONTROLLER;
	}

	ret = i2c_configure(i2c_controller_dev, i2c_test_ctx.i2c_cfg);
	zassert_equal(ret, 0, "I2C controller config failed");

	/* Use distinct random patterns: controller TX != target TX */
	fill_buffer_random(tx_data, BUFF_SIZE, SEED_CONTROLLER_TX);
	fill_target_tx_distinct(i2c_test_ctx.tgt_tx_data, BUFF_SIZE,
				   SEED_TARGET_TX);
	i2c_test_reset_callback_state(&i2c_test_ctx);

	/* Poison controller RX to detect uninitialized reads */
	poison_buffer(rx_data, BUFF_SIZE, POISON_RX);

	/* Setup i2c messages */
	msgs[0].buf = tx_data;
	msgs[0].len = BUFF_SIZE;
	msgs[0].flags = I2C_MSG_WRITE | I2C_MSG_STOP;

	msgs[1].buf = rx_data;
	msgs[1].len = BUFF_SIZE;
	msgs[1].flags = I2C_MSG_READ | I2C_MSG_STOP;

	/* Writing data from Controller to Target */
	LOG_DBG("Starting transfer - RX count: %u", i2c_test_ctx.tgt_rx_cnt);
	ret = i2c_transfer(i2c_controller_dev, &msgs[0], 1, TGT_I2C_ADDR);
	zassert_equal(ret, 0, "I2C write transfer failed: %d", ret);
	LOG_DBG("Transfer completed - RX count: %u", i2c_test_ctx.tgt_rx_cnt);

	ret = validate_target_rx(tx_data, BUFF_SIZE);
	if (ret) {
		return ret;
	}
	i2c_test_assert_target_contract(&i2c_test_ctx, I2C_TRANSMIT_ONLY,
				       BUFF_SIZE, 0U);

	/* Reading data from Target by controller */
	i2c_test_reset_callback_state(&i2c_test_ctx);
	ret = i2c_transfer(i2c_controller_dev, &msgs[1], 1, TGT_I2C_ADDR);
	zassert_equal(ret, 0, "I2C read transfer failed: %d", ret);

	ret = validate_controller_rx(i2c_test_ctx.tgt_tx_data, rx_data, BUFF_SIZE);
	if (ret) {
		return ret;
	}
	i2c_test_assert_target_contract(&i2c_test_ctx, I2C_RECEIVE_ONLY,
				       0U, BUFF_SIZE);

	return 0;
}

int start_i2c_transmit(void)
{
	uint8_t tx_data[BUFF_SIZE];
	int ret;
	struct i2c_msg msgs[1];
	const struct device *const i2c_controller_dev = i2c_test_ctx.controller_dev;

	/* Use current context config, ensure controller mode */
	if ((i2c_test_ctx.i2c_cfg & I2C_MODE_CONTROLLER) == 0U) {
		i2c_test_ctx.i2c_cfg |= I2C_MODE_CONTROLLER;
	}
	i2c_test_ctx.tgt_tx_cnt = 0;
	prepare_target_rx(BUFF_SIZE);

	ret = i2c_configure(i2c_controller_dev, i2c_test_ctx.i2c_cfg);
	zassert_equal(ret, 0, "I2C controller config failed");

	/* Use distinct random pattern for controller TX */
	fill_buffer_random(tx_data, BUFF_SIZE, SEED_CONTROLLER_TX);
	i2c_test_reset_callback_state(&i2c_test_ctx);

	msgs[0].buf = tx_data;
	msgs[0].len = BUFF_SIZE;
	msgs[0].flags = I2C_MSG_WRITE | I2C_MSG_STOP;

	/* Writing data from Controller to Target */
	LOG_DBG("Starting transfer - RX count: %u", i2c_test_ctx.tgt_rx_cnt);
	ret = i2c_transfer(i2c_controller_dev, &msgs[0], 1, TGT_I2C_ADDR);
	zassert_equal(ret, 0, "I2C write transfer failed: %d", ret);

	ret = validate_target_rx(tx_data, BUFF_SIZE);
	if (ret) {
		return ret;
	}
	i2c_test_assert_target_contract(&i2c_test_ctx, I2C_TRANSMIT_ONLY,
				       BUFF_SIZE, 0U);

	return 0;
}

int start_i2c_receive(void)
{
	uint8_t rx_data[BUFF_SIZE];
	int ret;
	struct i2c_msg msgs[1];
	const struct device *const i2c_controller_dev = i2c_test_ctx.controller_dev;

	/* Use current context config, ensure controller mode */
	if ((i2c_test_ctx.i2c_cfg & I2C_MODE_CONTROLLER) == 0U) {
		i2c_test_ctx.i2c_cfg |= I2C_MODE_CONTROLLER;
	}
	i2c_test_ctx.tgt_tx_cnt = 0;
	ret = i2c_configure(i2c_controller_dev, i2c_test_ctx.i2c_cfg);
	zassert_equal(ret, 0, "I2C controller config failed");

	/* Use distinct random pattern for target TX */
	fill_target_tx_distinct(i2c_test_ctx.tgt_tx_data, BUFF_SIZE,
				   SEED_TARGET_TX);

	/* Poison controller RX to detect uninitialized reads */
	poison_buffer(rx_data, BUFF_SIZE, POISON_RX);

	msgs[0].buf = rx_data;
	msgs[0].len = BUFF_SIZE;
	msgs[0].flags = I2C_MSG_READ | I2C_MSG_STOP;

	/* Reading data from Target by controller */
	i2c_test_reset_callback_state(&i2c_test_ctx);
	ret = i2c_transfer(i2c_controller_dev, &msgs[0], 1, TGT_I2C_ADDR);
	zassert_equal(ret, 0, "I2C read transfer failed: %d", ret);

	ret = validate_controller_rx(i2c_test_ctx.tgt_tx_data, rx_data, BUFF_SIZE);
	if (ret) {
		return ret;
	}
	i2c_test_assert_target_contract(&i2c_test_ctx, I2C_RECEIVE_ONLY,
				       0U, BUFF_SIZE);

	return 0;
}

int start_10bit_i2c_transfer(void)
{
	uint8_t tx_data[BUFF_SIZE];
	uint8_t rx_data[BUFF_SIZE];
	int ret;
	struct i2c_msg msgs[2];
	const struct device *const i2c_controller_dev = i2c_test_ctx.controller_dev;

	i2c_test_ctx.tgt_tx_cnt = 0;
	prepare_target_rx(BUFF_SIZE);

	/* Use current context config, ensure controller mode and
	 * 10-bit addressing
	 */
	if ((i2c_test_ctx.i2c_cfg & I2C_MODE_CONTROLLER) == 0U) {
		i2c_test_ctx.i2c_cfg |= I2C_MODE_CONTROLLER;
	}
	i2c_test_ctx.i2c_cfg |= I2C_ADDR_10_BITS;

	ret = i2c_configure(i2c_controller_dev, i2c_test_ctx.i2c_cfg);
	zassert_equal(ret, 0, "I2C controller config failed");

	/* Use distinct random patterns: controller TX != target TX */
	fill_buffer_random(tx_data, BUFF_SIZE, SEED_CONTROLLER_TX);
	fill_target_tx_distinct(i2c_test_ctx.tgt_tx_data, BUFF_SIZE,
				   SEED_TARGET_TX);
	i2c_test_reset_callback_state(&i2c_test_ctx);

	/* Poison controller RX to detect uninitialized reads */
	poison_buffer(rx_data, BUFF_SIZE, POISON_RX);

	msgs[0].buf = tx_data;
	msgs[0].len = BUFF_SIZE;
	msgs[0].flags = I2C_MSG_WRITE | I2C_MSG_STOP | I2C_MSG_ADDR_10_BITS;

	msgs[1].buf = rx_data;
	msgs[1].len = BUFF_SIZE;
	msgs[1].flags = I2C_MSG_READ | I2C_MSG_STOP | I2C_MSG_ADDR_10_BITS;

	/* Writing data from Controller to Target */
	ret = i2c_transfer(i2c_controller_dev, &msgs[0], 1, TGT_I2C_10BITADDR);
	zassert_equal(ret, 0, "10-bit I2C write failed: %d", ret);

	ret = validate_target_rx(tx_data, BUFF_SIZE);
	if (ret) {
		return ret;
	}
	i2c_test_assert_target_contract(&i2c_test_ctx, I2C_TRANSMIT_ONLY,
				       BUFF_SIZE, 0U);

	/* Reading data from Target by controller */
	i2c_test_reset_callback_state(&i2c_test_ctx);
	ret = i2c_transfer(i2c_controller_dev, &msgs[1], 1, TGT_I2C_10BITADDR);
	zassert_equal(ret, 0, "10-bit I2C read failed: %d", ret);

	ret = validate_controller_rx(i2c_test_ctx.tgt_tx_data, rx_data, BUFF_SIZE);
	if (ret) {
		return ret;
	}
	i2c_test_assert_target_contract(&i2c_test_ctx, I2C_RECEIVE_ONLY,
				       0U, BUFF_SIZE);

	return 0;
}

/*
 * Registering the i2c instance as target
 * passing the i2c_target_config structure with call back apis
 */

void register_target_i2c_common(uint32_t i2c_cfg, uint16_t addr, uint8_t flags)
{
	int ret;
	const struct device *const i2c_target_dev = i2c_test_ctx.target_dev;

	/* Check if target is already registered and unregister properly */
	if (i2c_test_ctx.target_registered && i2c_test_ctx.target_dev) {
		LOG_DBG("Target already registered, unregistering first");
		ret = i2c_target_unregister(i2c_test_ctx.target_dev,
					&i2c_tcfg);
		if (ret != 0) {
			LOG_WRN("Target unregister failed: %d", ret);
		}

		i2c_test_ctx.target_registered = false;
		/* Allow time for unregistration to complete */
		k_msleep(10);
	}

	i2c_tcfg.flags = flags;
	i2c_tcfg.address = addr;

	ret = i2c_configure(i2c_target_dev, i2c_cfg);
	zassert_equal(ret, 0, "I2C config failed");

	ret = i2c_target_register(i2c_target_dev, &i2c_tcfg);
	/* ENOTSUP means operation not supported */
	if (ret == -ENOTSUP) {
		LOG_WRN("I2C target mode not supported");
		ztest_test_skip();
	} else {
		zassert_equal(ret, 0, "i2c_target_register failed: %d", ret);
	}

	if (ret == 0) {
		i2c_test_ctx.target_dev = i2c_target_dev;
		i2c_test_ctx.target_registered = true;
		LOG_DBG("Target registered successfully");
	}
}

void register_target_i2c(void)
{
	register_target_i2c_common(I2C_SPEED_SET(I2C_SPEED_STANDARD),
				  TGT_I2C_ADDR, 0);
}

void register_target_speed_i2c(uint32_t i2c_cfg)
{
	register_target_i2c_common(i2c_cfg, TGT_I2C_ADDR, 0);
}

void register_target_i2c_10Bit(void)
{
	register_target_i2c_common(I2C_SPEED_SET(I2C_SPEED_STANDARD),
				  TGT_I2C_10BITADDR,
				  I2C_TARGET_FLAGS_ADDR_10_BITS);
}

/*
 * Function will send a set of predefined data
 * 0xAA 0XAB 0xAC 0xAD from i2c master to i2c Slave
 * also the master will read one byte data 0x60 from the target.
 * Predefined data values are randomly chosen, just to verify
 * write and read capabilities of the i2c instances.
 * Function will continuously print the write and read datas.
 */

static void test_start_i2c_transmit(void)
{
	struct i2c_test_freq_desc freqs[I2C_TEST_MAX_FREQS];
	int ret;
	int freq_count;
	int i;

	i2c_test_skip_if_no_freqs();
	freq_count = i2c_test_get_enabled_freqs(freqs, ARRAY_SIZE(freqs));

	for (i = 0; i < freq_count; i++) {
		TC_PRINT("Testing TX at %s (%u Hz)\n", freqs[i].name,
			 freqs[i].freq_hz);

		ret = i2c_test_configure_controller_freq(i2c_test_ctx.controller_dev,
						     &i2c_test_ctx, &freqs[i]);
		if (ret == -ENOTSUP) {
			TC_PRINT("Skipping %s: not supported\n", freqs[i].name);
			continue;
		}
		zassert_equal(ret, 0, "Failed to configure %s: %d",
			      freqs[i].name, ret);

		register_target_speed_i2c(I2C_SPEED_SET(freqs[i].zephyr_speed));
		ret = start_i2c_transmit();
		zassert_equal(ret, 0, "I2C transmit failed at %s: %d",
			      freqs[i].name, ret);
	}
}

/*
 * function : test_start_i2c_7Bit_transfer
 * Function to test controller target transceive (write-then-read) using 7-bit
 * addressing at all enabled frequencies. Covers the classic I2C XCV pattern.
 */
static void test_start_i2c_7Bit_transfer(void)
{
	struct i2c_test_freq_desc freqs[I2C_TEST_MAX_FREQS];
	int ret;
	int freq_count;
	int i;

	i2c_test_skip_if_no_freqs();
	freq_count = i2c_test_get_enabled_freqs(freqs, ARRAY_SIZE(freqs));

	for (i = 0; i < freq_count; i++) {
		TC_PRINT("Testing 7-bit at %s (%u Hz)\n", freqs[i].name,
			 freqs[i].freq_hz);

		ret = i2c_test_configure_controller_freq(i2c_test_ctx.controller_dev,
						     &i2c_test_ctx, &freqs[i]);
		if (ret == -ENOTSUP) {
			TC_PRINT("Skipping %s: not supported\n", freqs[i].name);
			continue;
		}
		zassert_equal(ret, 0, "Failed to configure %s: %d",
			      freqs[i].name, ret);

		register_target_speed_i2c(I2C_SPEED_SET(freqs[i].zephyr_speed));
		ret = start_i2c_transceive();
		zassert_equal(ret, 0, "7-bit I2C transfer failed at %s: %d",
			      freqs[i].name, ret);
	}
}

/*
 * function : test_start_i2c_10Bit_transfer
 * Function to test controller and target with 10 bit addressing modes at all enabled
 * frequencies
 */
static void test_start_i2c_10Bit_transfer(void)
{
	struct i2c_test_freq_desc freqs[I2C_TEST_MAX_FREQS];
	int ret;
	int freq_count;
	int i;

	i2c_test_skip_if_no_freqs();
	freq_count = i2c_test_get_enabled_freqs(freqs, ARRAY_SIZE(freqs));

	for (i = 0; i < freq_count; i++) {
		TC_PRINT("Testing 10-bit at %s (%u Hz)\n", freqs[i].name,
			 freqs[i].freq_hz);

		ret = i2c_test_configure_controller_freq(i2c_test_ctx.controller_dev,
						     &i2c_test_ctx, &freqs[i]);
		if (ret == -ENOTSUP) {
			TC_PRINT("Skipping %s: not supported\n", freqs[i].name);
			continue;
		}
		zassert_equal(ret, 0, "Failed to configure %s: %d",
			      freqs[i].name, ret);

		/* 10-bit uses 10-bit target registration */
		register_target_i2c_10Bit();
		ret = start_10bit_i2c_transfer();
		zassert_equal(ret, 0, "10-bit I2C transfer failed at %s: %d",
			      freqs[i].name, ret);
	}
}

/*
 * function : test_set_config
 * Function to test set config
 */
static void test_set_config(void)
{
	int ret;
	const struct device *const i2c_dev = i2c_test_ctx.controller_dev;
	uint32_t i2c_cfg = I2C_MODE_CONTROLLER |
			   I2C_SPEED_SET(I2C_SPEED_STANDARD);

	ret = i2c_configure(i2c_dev, i2c_cfg);
	zassert_equal(ret, 0, "I2C config failed");
}

/*
 * function : test_get_config
 * Function to test get_config
 */
static void test_get_config(void)
{
	uint32_t i2c_cfg_tmp;
	int ret = 0;
	uint32_t i2c_cfg;
	const struct device *const i2c_controller_dev = i2c_test_ctx.controller_dev;

	i2c_cfg = I2C_MODE_CONTROLLER | I2C_SPEED_SET(I2C_SPEED_FAST);

	ret = i2c_configure(i2c_controller_dev, i2c_cfg);
	/* ENOTSUP means operation not supported */
	if (ret == -ENOTSUP) {
		LOG_WRN("get configuration not implemented");
		ztest_test_skip();
	} else {
		zassert_equal(ret, 0, "I2C configure failed");
	}

	ret = i2c_get_config(i2c_controller_dev, &i2c_cfg_tmp);
	/* ENOTSUP means operation not supported */
	if (ret == -ENOTSUP) {
		LOG_WRN("get configuration not implemented");
		ztest_test_skip();
	} else {
		zassert_equal(ret, 0, "I2C get config failed");
	}

	/* Check if the retrieved configuration matches expected */
	if (i2c_cfg != i2c_cfg_tmp) {
		LOG_ERR("I2C get_config invalid: expected %d, got %d",
			i2c_cfg, i2c_cfg_tmp);
		ztest_test_fail();
	}
}

ZTEST(i2c, test_node0)
{
	const struct device *const i2c_dev = i2c_test_ctx.controller_dev;

	zassert_true(device_is_ready(i2c_dev), "I2C device is not ready");
}

ZTEST(i2c, test_get_config)
{
	test_get_config();
}

ZTEST(i2c, test_set_config)
{
	test_set_config();
}

ZTEST(i2c, test_node1)
{
	const struct device *const i2c_dev = i2c_test_ctx.target_dev;

	zassert_true(device_is_ready(i2c_dev), "I2C device is not ready");
}

ZTEST(i2c, test_7bit_addr)
{
	test_start_i2c_7Bit_transfer();
}

#ifndef CONFIG_TEST_LPI2C
ZTEST(i2c, test_10bit_addr)
{
	test_start_i2c_10Bit_transfer();
}
#endif

ZTEST(i2c, test_tx)
{
	test_start_i2c_transmit();
}

ZTEST(i2c, test_nack_recovery)
{
	struct i2c_test_freq_desc freqs[I2C_TEST_MAX_FREQS];
	const struct device *const i2c_controller_dev = i2c_test_ctx.controller_dev;
	uint8_t tx_data[BUFF_SIZE];
	int ret;
	int freq_count;
	int i;

	i2c_test_skip_if_no_freqs();
	freq_count = i2c_test_get_enabled_freqs(freqs, ARRAY_SIZE(freqs));

	for (i = 0; i < freq_count; i++) {
		TC_PRINT("Testing NACK recovery at %s (%u Hz)\n",
			 freqs[i].name, freqs[i].freq_hz);

		ret = i2c_test_configure_controller_freq(i2c_controller_dev,
						     &i2c_test_ctx, &freqs[i]);
		if (ret == -ENOTSUP) {
			TC_PRINT("Skipping %s: not supported\n", freqs[i].name);
			continue;
		}
		zassert_equal(ret, 0, "Failed to configure %s: %d",
			      freqs[i].name, ret);

		register_target_speed_i2c(I2C_SPEED_SET(freqs[i].zephyr_speed));
		fill_buffer(tx_data, sizeof(tx_data));

		ret = i2c_write(i2c_controller_dev, tx_data, sizeof(tx_data),
				TGT_I2C_ADDR + 1U);
		zassert_not_equal(ret, 0,
				  "invalid target address unexpectedly acknowledged");

		/* Allow controller to recover from NACK state */
		k_msleep(10);

		ret = start_i2c_transmit();
		zassert_equal(ret, 0,
			      "controller did not recover after NACK at %s",
			      freqs[i].name);
	}
}

ZTEST(i2c, test_mrx_stx)
{
	struct i2c_test_freq_desc freqs[I2C_TEST_MAX_FREQS];
	int ret;
	int freq_count;
	int i;

	i2c_test_skip_if_no_freqs();
	freq_count = i2c_test_get_enabled_freqs(freqs, ARRAY_SIZE(freqs));

	for (i = 0; i < freq_count; i++) {
		TC_PRINT("Testing MRX/STX at %s (%u Hz)\n",
			 freqs[i].name, freqs[i].freq_hz);

		ret = i2c_test_configure_controller_freq(i2c_test_ctx.controller_dev,
						     &i2c_test_ctx, &freqs[i]);
		if (ret == -ENOTSUP) {
			TC_PRINT("Skipping %s: not supported\n", freqs[i].name);
			continue;
		}
		zassert_equal(ret, 0, "Failed to configure %s: %d",
			      freqs[i].name, ret);

		register_target_speed_i2c(I2C_SPEED_SET(freqs[i].zephyr_speed));
		ret = start_i2c_receive();
		zassert_equal(ret, 0, "Receive failed at %s", freqs[i].name);
	}
}

static void *i2c_suite_setup(void)
{
	i2c_test_ctx.controller_dev = DEVICE_DT_GET(I2C_CONTROLLER);
	i2c_test_ctx.target_dev = DEVICE_DT_GET(I2C_TARGET);
	i2c_test_ctx.target_registered = false;
	return &i2c_test_ctx;
}

static void i2c_before(void *fixture)
{
	struct i2c_test_ctx *ctx = fixture;

	i2c_test_reset_runtime_config(ctx);
}

static void i2c_after(void *fixture)
{
	struct i2c_test_ctx *ctx = fixture;
	int ret;

	if (!ctx->target_registered) {
		return;
	}

	ret = i2c_target_unregister(ctx->target_dev, &i2c_tcfg);
	/* ENOTSUP means operation not supported */
	if (ret == -ENOTSUP) {
		LOG_WRN("I2C target mode not supported");
	} else {
		zassert_equal(ret, 0,
				  "i2c_target_unregister failed: %d", ret);
	}

	ctx->target_registered = false;
}

ZTEST_SUITE(i2c, NULL, i2c_suite_setup, i2c_before, i2c_after, NULL);

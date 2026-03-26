/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include "test_i2c.h"
#include <zephyr/pm/device.h>
LOG_MODULE_REGISTER(alif_i2c_test, LOG_LEVEL_INF);

/* Expected slave receive buffer size */
static uint32_t slv_rx_expected = BUFF_SIZE;

/* Global test context instance */
struct i2c_test_ctx i2c_test_ctx;

static void prepare_slave_rx(uint32_t expected_len);

void i2c_prepare_slave_rx(uint32_t expected_len)
{
	prepare_slave_rx(expected_len);
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

void i2c_test_prime_buffers(const uint8_t *read_data, size_t read_len,
				size_t expected_write_len)
{
	size_t copy_len = MIN(read_len, sizeof(i2c_test_ctx.slv_tx_data));
	size_t i;

	i2c_test_ctx.slv_tx_cnt = 0U;

	/* Poison buffer first to detect uninitialized data usage */
	poison_buffer(i2c_test_ctx.slv_tx_data,
			  sizeof(i2c_test_ctx.slv_tx_data), 0x5A);

	if ((read_data != NULL) && (copy_len > 0U)) {
		for (i = 0U; i < copy_len; i++) {
			i2c_test_ctx.slv_tx_data[i] = read_data[i];
		}
	}

	prepare_slave_rx(expected_write_len);
	i2c_test_reset_callback_state(&i2c_test_ctx);
}

static void prepare_slave_rx(uint32_t expected_len)
{
	slv_rx_expected = MIN(expected_len, (uint32_t)BUFF_PERF);
#ifdef CONFIG_I2C_TARGET_BUFFER_MODE
	slv_rx_expected = MIN(slv_rx_expected,
				  (uint32_t)CONFIG_I2C_TAR_DATA_BUF_MAX_LEN);
#endif

	/* Poison RX buffer to detect uninitialized data usage */
	poison_buffer(i2c_test_ctx.slv_rx_data,
			  sizeof(i2c_test_ctx.slv_rx_data), 0xA5);
	i2c_test_ctx.slv_rx_cnt = 0U;
}

int validate_slave_rx(const uint8_t *tx_data, size_t expected_len)
{
	int ret;
	size_t actual_expected = MIN(expected_len, (size_t)slv_rx_expected);

	if (i2c_test_ctx.slv_rx_cnt != actual_expected) {
		LOG_ERR("I2C slave RX count %u, expected %zu",
			i2c_test_ctx.slv_rx_cnt, actual_expected);
		return -EIO;
	}

	ret = i2c_validate_data_match(tx_data, i2c_test_ctx.slv_rx_data,
					  actual_expected,
					  "I2C Master TX -> Slave RX");
	if (ret != 0) {
		return ret;
	}

	LOG_DBG("I2C master TX and slave RX data match");
	return 0;
}

int validate_master_rx(const uint8_t *expected_data, const uint8_t *rx_data,
			   size_t expected_len)
{
	int ret;

	ret = i2c_validate_data_match(expected_data, rx_data, expected_len,
					  "I2C Slave TX -> Master RX");
	if (ret != 0) {
		return ret;
	}

	LOG_DBG("I2C slave TX and master RX data match");
	return 0;
}

static uint32_t speed_from_bus_freq(uint32_t freq_hz)
{
	switch (freq_hz) {
	case 100000:
		return I2C_SPEED_STANDARD;
	case 400000:
		return I2C_SPEED_FAST;
	case 1000000:
		return I2C_SPEED_FAST_PLUS;
	case 3400000:
		return I2C_SPEED_HIGH;
	default:
		return I2C_SPEED_STANDARD;
	}
}

static uint32_t master_speed_from_dt(void)
{
	uint32_t speed = I2C_SPEED_STANDARD;

#if DT_NODE_HAS_PROP(I2C_MASTER, clock_frequency)
	speed = speed_from_bus_freq(DT_PROP(I2C_MASTER, clock_frequency));
#endif
	return speed;
}

static uint32_t slave_speed_from_dt(void)
{
	uint32_t speed = I2C_SPEED_STANDARD;

#if DT_NODE_HAS_PROP(I2C_SLAVE, clock_frequency)
	speed = speed_from_bus_freq(DT_PROP(I2C_SLAVE, clock_frequency));
#endif
	return speed;
}

static int configure_bus_frequency_runtime(uint32_t expected_hz)
{
	enum i2c_test_speed runtime_speed;

	switch (expected_hz) {
	case 100000:
		runtime_speed = I2C_SPEED_STANDARD_KHZ;
		break;
	case 400000:
		runtime_speed = I2C_SPEED_FAST_KHZ;
		break;
	case 1000000:
		runtime_speed = I2C_SPEED_FAST_PLUS_KHZ;
		break;
	case 3400000:
		runtime_speed = I2C_SPEED_HIGH_KHZ;
		break;
	default:
		return -EINVAL;
	}

	return i2c_test_set_frequency(&i2c_test_ctx, runtime_speed);
}

static void skip_if_bus_freq_ne(uint32_t expected_hz)
{
	int ret;

	ret = configure_bus_frequency_runtime(expected_hz);
	if (ret == -ENOTSUP) {
		LOG_INF("Skipping: %u Hz not supported", expected_hz);
		ztest_test_skip();
	}

	zassert_equal(ret, 0,
			  "failed to configure %u Hz runtime bus speed: %d",
			  expected_hz, ret);
}

/* Slave call back functions */
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
static int cb_i2c_target_read_requested(struct i2c_target_config *config,
					uint8_t *val)
{
	ARG_UNUSED(config);
	if (i2c_test_ctx.slv_tx_cnt >= BUFF_PERF) {
		i2c_test_ctx.slv_tx_cnt = 0;
	}
	*val = i2c_test_ctx.slv_tx_data[i2c_test_ctx.slv_tx_cnt++];
	i2c_test_note_read_requested(&i2c_test_ctx);
	LOG_DBG("Read requested from Master and send 0x%x from slave", *val);
	return 0;
}

static int cb_i2c_target_write_received(struct i2c_target_config *config,
					uint8_t val)
{
	ARG_UNUSED(config);
	if (i2c_test_ctx.slv_rx_cnt < slv_rx_expected) {
		i2c_test_ctx.slv_rx_data[i2c_test_ctx.slv_rx_cnt++] = val;
		LOG_DBG("Slave RX [%u]: 0x%02X (total: %u)",
			i2c_test_ctx.slv_rx_cnt - 1, val,
			i2c_test_ctx.slv_rx_cnt);
	} else {
		LOG_DBG("Ignoring extra slave RX byte 0x%02X", val);
	}
	i2c_test_note_write_received(&i2c_test_ctx);
	return 0;
}

static int cb_i2c_target_read_processed(struct i2c_target_config *config,
					uint8_t *val)
{
	ARG_UNUSED(config);
	ARG_UNUSED(val);
	i2c_test_note_read_processed(&i2c_test_ctx);
	LOG_DBG("Read processed_cb called");
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
	*val = i2c_test_ctx.slv_tx_data;
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

	copy_len = MIN(len, slv_rx_expected);
	memcpy(i2c_test_ctx.slv_rx_data, data_buf, copy_len);
	i2c_test_ctx.slv_rx_cnt = copy_len;
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
 * also the master will read one byte data 0x60 from the slave.
 * Predefined data values are randomly chosen, just to verify
 * write and read capabilities of the i2c instances.
 * Function will continuously print the write and read datas.
 */

int start_i2c_transceive(void)
{
	uint8_t tx_data[BUFF_SIZE];
	uint8_t rx_data[BUFF_SIZE];
	uint32_t master_cfg;
	int ret;
	struct i2c_msg msgs[2];
	const struct device *const i2c_master_dev =
		(i2c_test_ctx.master_dev != NULL)
		? i2c_test_ctx.master_dev
		: DEVICE_DT_GET(I2C_MASTER);

	i2c_test_ctx.slv_tx_cnt = 0;
	prepare_slave_rx(BUFF_SIZE);

	master_cfg = i2c_test_ctx.i2c_cfg;
	if ((master_cfg & I2C_MODE_CONTROLLER) == 0U) {
		master_cfg = I2C_MODE_CONTROLLER |
				 I2C_SPEED_SET(master_speed_from_dt());
	}
	i2c_test_ctx.i2c_cfg = master_cfg;

	zassert_true(device_is_ready(i2c_master_dev),
			 "i2c: Master Device is not ready");
	ret = i2c_configure(i2c_master_dev, master_cfg);
	zassert_equal(ret, 0, "I2C master config failed");

	/* Use distinct random patterns: master TX != slave TX */
	fill_buffer_random(tx_data, BUFF_SIZE, SEED_MASTER_TX);
	fill_slave_tx_distinct(i2c_test_ctx.slv_tx_data, BUFF_SIZE,
				   SEED_SLAVE_TX);
	i2c_test_reset_callback_state(&i2c_test_ctx);

	/* Poison master RX to detect uninitialized reads */
	poison_buffer(rx_data, BUFF_SIZE, POISON_RX);

	/* Setup i2c messages */
	msgs[0].buf = tx_data;
	msgs[0].len = BUFF_SIZE;
	msgs[0].flags = I2C_MSG_WRITE | I2C_MSG_STOP;

	msgs[1].buf = rx_data;
	msgs[1].len = BUFF_SIZE;
	msgs[1].flags = I2C_MSG_READ | I2C_MSG_STOP;

	/* Writing data from Master to Slave */
	LOG_DBG("Starting transfer - RX count: %u", i2c_test_ctx.slv_rx_cnt);
	ret = i2c_transfer(i2c_master_dev, &msgs[0], 1, SLV_I2C_ADDR);
	zassert_equal(ret, 0, "I2C write transfer failed: %d", ret);
	LOG_DBG("Transfer completed - RX count: %u", i2c_test_ctx.slv_rx_cnt);

	ret = validate_slave_rx(tx_data, BUFF_SIZE);
	if (ret) {
		return ret;
	}

	/* Reading data from Slave by master */
	i2c_test_reset_callback_state(&i2c_test_ctx);
	ret = i2c_transfer(i2c_master_dev, &msgs[1], 1, SLV_I2C_ADDR);
	zassert_equal(ret, 0, "I2C read transfer failed: %d", ret);

	ret = validate_master_rx(i2c_test_ctx.slv_tx_data, rx_data, BUFF_SIZE);
	if (ret) {
		return ret;
	}

	return 0;
}

int start_i2c_transmit(void)
{
	uint8_t tx_data[BUFF_SIZE];
	int ret;
	uint32_t master_cfg;
	struct i2c_msg msgs[1];
	const struct device *const i2c_master_dev =
		(i2c_test_ctx.master_dev != NULL)
		? i2c_test_ctx.master_dev
		: DEVICE_DT_GET(I2C_MASTER);

	master_cfg = i2c_test_ctx.i2c_cfg;
	if ((master_cfg & I2C_MODE_CONTROLLER) == 0U) {
		master_cfg = I2C_MODE_CONTROLLER |
				 I2C_SPEED_SET(master_speed_from_dt());
	}
	i2c_test_ctx.i2c_cfg = master_cfg;
	i2c_test_ctx.slv_tx_cnt = 0;
	prepare_slave_rx(BUFF_SIZE);

	zassert_true(device_is_ready(i2c_master_dev),
			 "i2c: Master Device is not ready");
	ret = i2c_configure(i2c_master_dev, master_cfg);
	zassert_equal(ret, 0, "I2C master config failed");

	/* Use distinct random pattern for master TX */
	fill_buffer_random(tx_data, BUFF_SIZE, SEED_MASTER_TX);
	i2c_test_reset_callback_state(&i2c_test_ctx);

	msgs[0].buf = tx_data;
	msgs[0].len = BUFF_SIZE;
	msgs[0].flags = I2C_MSG_WRITE | I2C_MSG_STOP;

	/* Writing data from Master to Slave */
	LOG_DBG("Starting transfer - RX count: %u", i2c_test_ctx.slv_rx_cnt);
	ret = i2c_transfer(i2c_master_dev, &msgs[0], 1, SLV_I2C_ADDR);
	zassert_equal(ret, 0, "I2C write transfer failed: %d", ret);

	ret = validate_slave_rx(tx_data, BUFF_SIZE);
	if (ret) {
		return ret;
	}

	return 0;
}

int start_i2c_receive(void)
{
	uint8_t rx_data[BUFF_SIZE];
	int ret;
	uint32_t master_cfg;
	struct i2c_msg msgs[1];
	const struct device *const i2c_master_dev =
		(i2c_test_ctx.master_dev != NULL)
		? i2c_test_ctx.master_dev
		: DEVICE_DT_GET(I2C_MASTER);

	zassert_true(device_is_ready(i2c_master_dev),
			 "i2c: Master Device is not ready");

	master_cfg = i2c_test_ctx.i2c_cfg;
	if ((master_cfg & I2C_MODE_CONTROLLER) == 0U) {
		master_cfg = I2C_MODE_CONTROLLER |
				 I2C_SPEED_SET(master_speed_from_dt());
	}
	i2c_test_ctx.i2c_cfg = master_cfg;
	i2c_test_ctx.slv_tx_cnt = 0;
	ret = i2c_configure(i2c_master_dev, master_cfg);
	zassert_equal(ret, 0, "I2C master config failed");

	/* Use distinct random pattern for slave TX */
	fill_slave_tx_distinct(i2c_test_ctx.slv_tx_data, BUFF_SIZE,
				   SEED_SLAVE_TX);

	/* Poison master RX to detect uninitialized reads */
	poison_buffer(rx_data, BUFF_SIZE, POISON_RX);

	msgs[0].buf = rx_data;
	msgs[0].len = BUFF_SIZE;
	msgs[0].flags = I2C_MSG_READ | I2C_MSG_STOP;

	/* Reading data from Slave by master */
	i2c_test_reset_callback_state(&i2c_test_ctx);
	ret = i2c_transfer(i2c_master_dev, &msgs[0], 1, SLV_I2C_ADDR);
	zassert_equal(ret, 0, "I2C read transfer failed: %d", ret);

	return validate_master_rx(i2c_test_ctx.slv_tx_data, rx_data, BUFF_SIZE);
}

int start_10bit_i2c_transfer(void)
{
	uint8_t tx_data[BUFF_SIZE];
	uint8_t rx_data[BUFF_SIZE];
	uint32_t master_cfg;
	int ret;
	struct i2c_msg msgs[2];
	const struct device *const i2c_master_dev =
		(i2c_test_ctx.master_dev != NULL)
		? i2c_test_ctx.master_dev
		: DEVICE_DT_GET(I2C_MASTER);

	i2c_test_ctx.slv_tx_cnt = 0;
	prepare_slave_rx(BUFF_SIZE);

	master_cfg = i2c_test_ctx.i2c_cfg;
	if ((master_cfg & I2C_MODE_CONTROLLER) == 0U) {
		master_cfg = I2C_MODE_CONTROLLER |
				 I2C_SPEED_SET(master_speed_from_dt());
	}
	master_cfg |= I2C_ADDR_10_BITS;
	i2c_test_ctx.i2c_cfg = master_cfg;
	zassert_true(device_is_ready(i2c_master_dev),
			 "i2c: Master Device is not ready");
	ret = i2c_configure(i2c_master_dev, master_cfg);
	zassert_equal(ret, 0, "I2C master config failed");

	/* Use distinct random patterns: master TX != slave TX */
	fill_buffer_random(tx_data, BUFF_SIZE, SEED_MASTER_TX);
	fill_slave_tx_distinct(i2c_test_ctx.slv_tx_data, BUFF_SIZE,
				   SEED_SLAVE_TX);
	i2c_test_reset_callback_state(&i2c_test_ctx);

	/* Poison master RX to detect uninitialized reads */
	poison_buffer(rx_data, BUFF_SIZE, POISON_RX);

	msgs[0].buf = tx_data;
	msgs[0].len = BUFF_SIZE;
	msgs[0].flags = I2C_MSG_WRITE | I2C_MSG_STOP | I2C_MSG_ADDR_10_BITS;

	msgs[1].buf = rx_data;
	msgs[1].len = BUFF_SIZE;
	msgs[1].flags = I2C_MSG_READ | I2C_MSG_STOP | I2C_MSG_ADDR_10_BITS;

	/* Writing data from Master to Slave */
	ret = i2c_transfer(i2c_master_dev, &msgs[0], 1, SLV_I2C_10BITADDR);
	zassert_equal(ret, 0, "10-bit I2C write failed: %d", ret);

	ret = validate_slave_rx(tx_data, BUFF_SIZE);
	if (ret) {
		return ret;
	}

	/* Reading data from Slave by master */
	i2c_test_reset_callback_state(&i2c_test_ctx);
	ret = i2c_transfer(i2c_master_dev, &msgs[1], 1, SLV_I2C_10BITADDR);
	zassert_equal(ret, 0, "10-bit I2C read failed: %d", ret);

	ret = validate_master_rx(i2c_test_ctx.slv_tx_data, rx_data, BUFF_SIZE);
	if (ret) {
		return ret;
	}

	return 0;
}

/*
 * Registering the i2c instance as slave
 * passing the i2c_target_config structure with call back apis
 */

void register_slave_i2c_common(uint32_t i2c_cfg, uint16_t addr, uint8_t flags)
{
	int ret;
	const struct device *const i2c_slave_dev =
		(i2c_test_ctx.slave_dev)
		? i2c_test_ctx.slave_dev
		: DEVICE_DT_GET(I2C_SLAVE);

	zassert_true(device_is_ready(i2c_slave_dev),
			 "i2c: Slave Device is not ready");

	/* Check if slave is already registered and unregister properly */
	if (i2c_test_ctx.slave_registered && i2c_test_ctx.slave_dev) {
		LOG_DBG("Slave already registered, unregistering first");
		ret = i2c_target_unregister(i2c_test_ctx.slave_dev,
						&i2c_tcfg);
		if (ret != 0) {
			LOG_WRN("Slave unregister failed: %d", ret);
		}

		i2c_test_ctx.slave_registered = false;
		/* Allow time for unregistration to complete */
		k_msleep(10);
	}

	/* Reset test context before new registration */
	i2c_test_ctx.slv_rx_cnt = 0;
	i2c_test_ctx.slv_tx_cnt = 0;
	memset(i2c_test_ctx.slv_rx_data, 0, BUFF_PERF);
	memset(i2c_test_ctx.slv_tx_data, 0, BUFF_PERF);
	i2c_test_reset_callback_state(&i2c_test_ctx);

	i2c_tcfg.flags = flags;
	i2c_tcfg.address = addr;

	ret = i2c_configure(i2c_slave_dev, i2c_cfg);
	zassert_equal(ret, 0, "I2C config failed");

	ret = i2c_target_register(i2c_slave_dev, &i2c_tcfg);
	/* ENOTSUP means operation not supported */
	if (ret == -ENOTSUP) {
		LOG_WRN("I2C target mode not supported");
		ztest_test_skip();
	} else {
		zassert_equal(ret, 0, "i2c_target_register failed: %d", ret);
	}

	if (ret == 0) {
		i2c_test_ctx.slave_dev = i2c_slave_dev;
		i2c_test_ctx.slave_registered = true;
		LOG_DBG("Slave registered successfully");
	}
}

void register_slave_i2c(void)
{
	register_slave_i2c_common(I2C_SPEED_SET(slave_speed_from_dt()),
				  SLV_I2C_ADDR, 0);
}

void register_slave_speed_i2c(uint32_t i2c_cfg)
{
	register_slave_i2c_common(i2c_cfg, SLV_I2C_ADDR, 0);
}

void register_slave_i2c_10Bit(void)
{
	register_slave_i2c_common(I2C_SPEED_SET(slave_speed_from_dt()),
				  SLV_I2C_10BITADDR,
				  I2C_TARGET_FLAGS_ADDR_10_BITS);
}

/* Unused transmit function removed - functionality covered in other tests */

/*
 * function : test_start_i2c_transmit
 * Function to test master slave transmit
 */

static void test_start_i2c_transmit(void)
{
	int ret;

	register_slave_i2c();
	ret = start_i2c_transmit();
}

/*
 * function : test_start_i2c_transceive
 * Function to test master slave transmit
 */

static void test_start_i2c_transceive(void)
{
	int ret;

	register_slave_i2c();
	ret = start_i2c_transceive();
}

/*
 * function : test_start_i2c_7Bit_transfer
 * Function to test master slave with 7 bit addressing mode
 */
static void test_start_i2c_7Bit_transfer(void)
{
	int ret;

	register_slave_i2c();
	ret = start_i2c_transceive();
}

/*
 * function : test_start_i2c_10Bit_transfer
 * Function to test master and slave
 * with 10 bit addressing modes
 */
static void test_start_i2c_10Bit_transfer(void)
{
	int ret;

	register_slave_i2c_10Bit();
	ret = start_10bit_i2c_transfer();
}

/*
 * function : test_set_config
 * Function to test set config
 */
static void test_set_config(void)
{
	int ret;
	const struct device *const i2c_dev = DEVICE_DT_GET(I2C_MASTER);
	uint32_t i2c_cfg = I2C_MODE_CONTROLLER |
			   I2C_SPEED_SET(I2C_SPEED_STANDARD);

	zassert_true(device_is_ready(i2c_dev), "I2C device is not ready");
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
	const struct device *const i2c_master_dev = DEVICE_DT_GET(I2C_MASTER);

	zassert_true(device_is_ready(i2c_master_dev),
			 "i2c: Master Device is not ready");

	i2c_cfg = I2C_MODE_CONTROLLER | I2C_SPEED_SET(I2C_SPEED_FAST);

	ret = i2c_configure(i2c_master_dev, i2c_cfg);
	/* ENOTSUP means operation not supported */
	if (ret == -ENOTSUP) {
		LOG_WRN("get configuration not implemented");
		ztest_test_skip();
	} else {
		zassert_equal(ret, 0, "I2C configure failed");
	}

	ret = i2c_get_config(i2c_master_dev, &i2c_cfg_tmp);
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

/*
 * function : test_standard_bus_speed
 * Function to test standard_bus_speed
 */
static void test_standard_bus_mode(void)
{
	int ret;

	skip_if_bus_freq_ne(100000);
	register_slave_speed_i2c(I2C_SPEED_SET(I2C_SPEED_STANDARD));
	ret = start_i2c_transceive();
}

/*
 * function : test_fast_bus_speed
 * Function to test fast_bus_speed
 */
static void test_fast_bus_mode(void)
{
	int ret;

	/* Reset runtime config before each test */
	i2c_test_reset_runtime_config(&i2c_test_ctx);

	skip_if_bus_freq_ne(400000);
	register_slave_speed_i2c(I2C_SPEED_SET(I2C_SPEED_FAST));
	ret = start_i2c_transceive();
}

/*
 * function : test_fast_plus_bus_speed
 * function to test fast_speed plus bus mode
 */
static void test_fast_plus_bus_mode(void)
{
	int ret;

	/* Reset runtime config before each test */
	i2c_test_reset_runtime_config(&i2c_test_ctx);

	skip_if_bus_freq_ne(1000000);
	register_slave_speed_i2c(I2C_SPEED_SET(I2C_SPEED_FAST_PLUS));
	ret = start_i2c_transceive();
}

/*
 * function : test_high_speed_bus_mode
 * function to test high_speed bus mode
 */
static void test_high_speed_bus_mode(void)
{
	int ret;

	/* Reset runtime config before each test */
	i2c_test_reset_runtime_config(&i2c_test_ctx);

	skip_if_bus_freq_ne(3400000);
	register_slave_speed_i2c(I2C_SPEED_SET(I2C_SPEED_HIGH));
	ret = start_i2c_transceive();
}

static void *i2c_suite_setup(void)
{
	i2c_test_ctx.master_dev = DEVICE_DT_GET(I2C_MASTER);
	i2c_test_ctx.slave_dev = DEVICE_DT_GET(I2C_SLAVE);
	i2c_test_ctx.i2c_cfg = I2C_MODE_CONTROLLER |
				I2C_SPEED_SET(master_speed_from_dt());
	i2c_test_ctx.slave_registered = false;
	return &i2c_test_ctx;
}

static void i2c_before(void *fixture)
{
	struct i2c_test_ctx *ctx = fixture;

	ctx->slv_rx_cnt = 0;
	ctx->slv_tx_cnt = 0;
	ctx->i2c_cfg = I2C_MODE_CONTROLLER |
			   I2C_SPEED_SET(master_speed_from_dt());
	memset(ctx->slv_tx_data, 0, BUFF_PERF);
	memset(ctx->slv_rx_data, 0, BUFF_PERF);
	i2c_test_reset_callback_state(ctx);
}

static void i2c_after(void *fixture)
{
	struct i2c_test_ctx *ctx = fixture;
	int ret;

	if (!ctx->slave_registered) {
		return;
	}

	ret = i2c_target_unregister(ctx->slave_dev, &i2c_tcfg);
	/* ENOTSUP means operation not supported */
	if (ret == -ENOTSUP) {
		LOG_WRN("I2C target mode not supported");
	} else {
		zassert_equal(ret, 0,
				  "i2c_target_unregister failed: %d", ret);
	}

	ctx->slave_registered = false;
}

static void i2c_teardown(void *fixture)
{
	i2c_after(fixture);
}

ZTEST(i2c, test_node0)
{
	const struct device *const i2c_dev = DEVICE_DT_GET(I2C_MASTER);

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
	const struct device *const i2c_dev = DEVICE_DT_GET(I2C_SLAVE);

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

ZTEST(i2c, test_xcv)
{
	test_start_i2c_transceive();
}

ZTEST(i2c, test_std)
{
	test_standard_bus_mode();
}

ZTEST(i2c, test_fast)
{
	test_fast_bus_mode();
}

ZTEST(i2c, test_fast_plus)
{
	test_fast_plus_bus_mode();
}

ZTEST(i2c, test_high)
{
	test_high_speed_bus_mode();
}

ZTEST(i2c, test_nack_recovery)
{
	const struct device *const i2c_master_dev = DEVICE_DT_GET(I2C_MASTER);
	uint8_t tx_data[BUFF_SIZE];
	int ret;

	register_slave_i2c();
	fill_buffer(tx_data, sizeof(tx_data));

	ret = i2c_write(i2c_master_dev, tx_data, sizeof(tx_data),
			SLV_I2C_ADDR + 1U);
	zassert_not_equal(ret, 0,
			  "invalid target address unexpectedly acknowledged");

	/* Allow controller to recover from NACK state */
	k_msleep(10);

	ret = start_i2c_transmit();
	zassert_equal(ret, 0, "controller did not recover after NACK");
}

#if SLAVE_T_MASTER_R
ZTEST(i2c, test_mrx_stx)
{
	register_slave_i2c();
	zassert_equal(start_i2c_receive(), 0, "Receive failed");
}
#endif

ZTEST(i2c, test_runtime_freq)
{
	int ret;
	const uint32_t speeds_hz[] = {100000, 400000, 1000000, 3400000};

	for (size_t i = 0; i < ARRAY_SIZE(speeds_hz); i++) {
		TC_PRINT("Testing runtime frequency %uHz\n", speeds_hz[i]);

		ret = configure_bus_frequency_runtime(speeds_hz[i]);
		if (ret == -ENOTSUP) {
			TC_PRINT("Skip unsupported freq %uHz\n",
				 speeds_hz[i]);
			continue;
		}

		zassert_equal(ret, 0,
				  "Failed to configure runtime freq %uHz: %d",
				  speeds_hz[i], ret);
		register_slave_speed_i2c(
			I2C_SPEED_SET(speed_from_bus_freq(speeds_hz[i])));

		ret = start_i2c_transceive();
		zassert_equal(ret, 0,
				  "Transmission stopped due to error at %uHz",
				  speeds_hz[i]);
	}
}

ZTEST_SUITE(i2c, NULL, i2c_suite_setup, i2c_before, i2c_after,
		i2c_teardown);

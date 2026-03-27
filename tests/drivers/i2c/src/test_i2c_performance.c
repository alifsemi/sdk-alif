/*
 * Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/*
 * I2C Performance Test Suite
 *
 * This test suite validates I2C performance characteristics including
 *
 * Test Scenarios:
 * - Standard speed (100kHz) transfers
 * - Fast speed (400kHz) transfers
 * - Fast Plus speed (1MHz) transfers
 * - High speed (3.4MHz) transfers
 * - Bidirectional transfers
 * - Repeated transfer operations
 */

#include "test_i2c.h"
LOG_MODULE_REGISTER(alif_i2c_perf, LOG_LEVEL_INF);

/**
 * @brief Performance measurement context
 *
 * Stores timing and cycle information for performance analysis
 */
struct i2c_perf_ctx {
	uint32_t rep;		   /* Repetition count */
	uint32_t m_start_cyc;  /* Master start cycle */
	uint32_t m_end_cyc;	   /* Master end cycle */
	uint32_t s_start_cyc;  /* Slave start cycle */
	uint32_t s_end_cyc;	   /* Slave end cycle */
	uint32_t m_start_time; /* Master start time */
	uint32_t m_end_time;   /* Master end time */
	uint32_t s_start_time; /* Slave start time */
	uint32_t s_end_time;   /* Slave end time */
};

static struct i2c_perf_ctx perf_ctx;

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
	uint32_t freq = DT_PROP(I2C_MASTER, clock_frequency);

	speed = speed_from_bus_freq(freq);
#endif

	return speed;
}

static void skip_if_bus_freq_ne(uint32_t expected_hz)
{
	int ret;
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
		LOG_INF("Skipping test: %u Hz bus speed is not supported",
			expected_hz);
		ztest_test_skip();
		return;
	}

	ret = i2c_test_set_frequency(&i2c_test_ctx, runtime_speed);
	if (ret == -ENOTSUP) {
		LOG_INF("Skipping: %u Hz not supported", expected_hz);
		ztest_test_skip();
	}

	zassert_equal(ret, 0,
			  "failed to configure %u Hz runtime bus speed: %d",
			  expected_hz, ret);
}

/*
 * Slave callback functions - static to avoid
 * linker clashes with test_i2c.c.
 */
static int cb_i2c_perf_target_write_requested(struct i2c_target_config *config)
{
	ARG_UNUSED(config);
	i2c_test_ctx.slv_tx_cnt = 0;
	i2c_test_note_write_requested(&i2c_test_ctx);
	LOG_DBG("Write requested - counters reset (TX: %u, RX: %u)",
		i2c_test_ctx.slv_tx_cnt, i2c_test_ctx.slv_rx_cnt);
	return 0;
}

static int cb_i2c_perf_target_read_requested(struct i2c_target_config *config,
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

static int cb_i2c_perf_target_write_received(struct i2c_target_config *config,
						 uint8_t val)
{
	ARG_UNUSED(config);
	if (i2c_test_ctx.slv_rx_cnt < BUFF_PERF) {
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

static int cb_i2c_perf_target_read_processed(struct i2c_target_config *config,
						 uint8_t *val)
{
	ARG_UNUSED(config);
	ARG_UNUSED(val);
	i2c_test_note_read_processed(&i2c_test_ctx);
	LOG_DBG("Read processed_cb called");
	return 0;
}

static int cb_i2c_target_stop(struct i2c_target_config *config)
{
	ARG_UNUSED(config);
	i2c_test_note_stop(&i2c_test_ctx);
	return 0;
}

#ifdef CONFIG_I2C_TARGET_BUFFER_MODE
static int cb_i2c_perf_target_buf_read_requested(
	struct i2c_target_config *config, uint8_t **val, uint32_t *len)
{
	ARG_UNUSED(config);

	i2c_test_ctx.slv_tx_cnt = 0;
	*len = BUFF_PERF;

	/* Use the pre-filled slv_tx_data - not a hardcoded pattern */
	*val = i2c_test_ctx.slv_tx_data;

	return 0;
}

static void cb_i2c_perf_target_buf_write_received(
	struct i2c_target_config *config, uint8_t *data_buf, uint32_t len)
{
	uint32_t idx;

	ARG_UNUSED(config);

	LOG_INF("Received %d bytes from master: ", len);
	for (idx = 0; idx < len; idx++) {
		/* Bounds checking to prevent buffer overflow */
		if (i2c_test_ctx.slv_rx_cnt < BUFF_PERF) {
			i2c_test_ctx.slv_rx_data[i2c_test_ctx.slv_rx_cnt++] =
				data_buf[idx];
			LOG_INF("0x%x ", data_buf[idx]);
		} else {
			LOG_DBG("Ignoring extra slave RX byte 0x%x",
				data_buf[idx]);
		}
	}
}
#endif

static const struct i2c_target_callbacks i2c_t_cb_perf = {
	.write_requested = &cb_i2c_perf_target_write_requested,
	.read_processed	 = &cb_i2c_perf_target_read_processed,

#ifdef CONFIG_I2C_TARGET_BUFFER_MODE
	.buf_read_requested	 = &cb_i2c_perf_target_buf_read_requested,
	.buf_write_received	 = &cb_i2c_perf_target_buf_write_received,
#else
	.read_requested	 = &cb_i2c_perf_target_read_requested,
	.write_received	 = &cb_i2c_perf_target_write_received,
#endif

	.stop = &cb_i2c_target_stop,
};

static struct i2c_target_config i2c_tcfg_perf = {
	.callbacks = &i2c_t_cb_perf,
};

/**
 * @brief Perform comprehensive I2C transfer with performance measurement
 *
 * This function executes a complete bidirectional I2C transfer:
 * 1. Master writes data to slave
 * 2. Master reads data from slave
 * 3. Measures timing and cycle performance
 * 4. Validates data integrity
 *
 * @return 0 on success, negative error code on failure
 */
int start_i2c_transfer(void)
{
	uint8_t tx_data[BUFF_PERF];
	uint8_t rx_data[BUFF_PERF];
	int ret = 0;
	struct i2c_msg msgs[2];
	size_t transfer_len = BUFF_PERF;
	const struct device *const i2c_master_dev = DEVICE_DT_GET(I2C_MASTER);

	/* Configure I2C controller for device tree frequency */
	i2c_test_ctx.i2c_cfg = I2C_MODE_CONTROLLER |
				I2C_SPEED_SET(master_speed_from_dt());

	zassert_true(device_is_ready(i2c_master_dev),
			 "I2C master device not ready");

	ret = i2c_configure(i2c_master_dev, i2c_test_ctx.i2c_cfg);
	if (ret != 0) {
		LOG_ERR("I2C master configuration failed: %d", ret);
		return ret;
	}

	/* Use distinct random patterns: master TX != slave TX */
	fill_slave_tx_distinct(i2c_test_ctx.slv_tx_data, BUFF_PERF,
				   SEED_SLAVE_TX);

	/* Poison master RX to detect uninitialized reads */
	poison_buffer(rx_data, BUFF_PERF, POISON_RX);

	/* Setup I2C messages for write operation */
	msgs[0].buf = tx_data;
	msgs[0].len = BUFF_PERF;
	msgs[0].flags = I2C_MSG_WRITE | I2C_MSG_STOP;

	/* Setup I2C messages for read operation */
	msgs[1].buf = rx_data;
	msgs[1].len = BUFF_PERF;
	msgs[1].flags = I2C_MSG_READ | I2C_MSG_STOP;

	/* Start master write operation timing */
	perf_ctx.m_start_cyc = k_cycle_get_32();
	perf_ctx.m_start_time = k_uptime_get();
	i2c_prepare_slave_rx(BUFF_PERF);
	i2c_test_reset_callback_state(&i2c_test_ctx);

	/* Perform master to slave write operation */
	ret = i2c_transfer(i2c_master_dev, &msgs[0], 1, SLV_I2C_ADDR);
	zassert_equal(ret, 0, "I2C write transfer failed: %d", ret);

	ret = validate_slave_rx(tx_data, BUFF_PERF);
	if (ret != 0) {
		LOG_ERR("Perf: Master TX -> Slave RX validation failed: %d",
			ret);
		return ret;
	}
	LOG_DBG("I2C Master TX -> Slave RX data validated");

	/* End master write operation timing */
	perf_ctx.m_end_time = k_uptime_get();
	perf_ctx.m_end_cyc = k_cycle_get_32();

	/* Start slave read operation timing */
	perf_ctx.s_start_time = k_uptime_get();
	perf_ctx.s_start_cyc = k_cycle_get_32();
	i2c_test_reset_callback_state(&i2c_test_ctx);

	/* Perform master to slave read operation */
	ret = i2c_transfer(i2c_master_dev, &msgs[1], 1, SLV_I2C_ADDR);
	zassert_equal(ret, 0, "I2C read transfer failed: %d", ret);

	/* End slave read operation timing */
	perf_ctx.s_end_time = k_uptime_get();
	perf_ctx.s_end_cyc = k_cycle_get_32();

	/* Validate read data integrity */
	ret = validate_master_rx(i2c_test_ctx.slv_tx_data, rx_data,
				 transfer_len);
	if (ret != 0) {
		LOG_ERR("Perf: Slave TX -> Master RX validation failed: %d",
			ret);
		return ret;
	}
	LOG_DBG("I2C Slave TX -> Master RX data validated");

	/* Log performance metrics */
	LOG_INF("Master TX -> Slave: ret=%d, time=%u ms, cyc=%u", ret,
		perf_ctx.m_end_time - perf_ctx.m_start_time,
		perf_ctx.m_end_cyc - perf_ctx.m_start_cyc);
	LOG_INF("Slave TX -> Master: ret=%d, time=%u ms, cyc=%u", ret,
		perf_ctx.s_end_time - perf_ctx.s_start_time,
		perf_ctx.s_end_cyc - perf_ctx.s_start_cyc);

	return ret;
}

static void test_start_i2c_transfer(void)
{
	int ret;

	ret = start_i2c_transfer();
	zassert_equal(ret, 0, "I2C transfer failed: %d", ret);
}

/**
 * @brief Test master I2C transmit and receive operation
 */
static void test_start_i2c_transceive(void)
{
	int ret;

	ret = start_i2c_transfer();
	zassert_equal(ret, 0, "I2C transceive failed: %d", ret);
}

static void test_standard_bus_mode(enum i2c_test_scenario scenario)
{
	const struct device *const i2c_master_dev = DEVICE_DT_GET(I2C_MASTER);

	/* Reset runtime config before each test */
	i2c_test_reset_runtime_config(&i2c_test_ctx);

	skip_if_bus_freq_ne(100000);
	zassert_true(device_is_ready(i2c_master_dev),
			 "i2c: Master Device is not ready.\n");

	i2c_test_ctx.scenario = scenario;

	i2c_test_ctx.i2c_cfg = I2C_SPEED_SET(I2C_SPEED_STANDARD);
	register_slave_speed_i2c(i2c_test_ctx.i2c_cfg);

	switch (i2c_test_ctx.scenario) {
	case I2C_TRANSMIT_ONLY:
		test_start_i2c_transfer();
		break;
	case I2C_TRANSMIT_RECEIVE:
		test_start_i2c_transceive();
		break;
	default:
		break;
	}
}

/* Function to test fast bus speed */
static void test_fast_bus_mode(enum i2c_test_scenario scenario)
{
	const struct device *const i2c_master_dev = DEVICE_DT_GET(I2C_MASTER);

	/* Reset runtime config before each test */
	i2c_test_reset_runtime_config(&i2c_test_ctx);

	skip_if_bus_freq_ne(400000);
	zassert_true(device_is_ready(i2c_master_dev),
			 "i2c: Master Device is not ready.\n");

	i2c_test_ctx.scenario = scenario;

	i2c_test_ctx.i2c_cfg = I2C_SPEED_SET(I2C_SPEED_FAST);
	register_slave_speed_i2c(i2c_test_ctx.i2c_cfg);

	switch (i2c_test_ctx.scenario) {
	case I2C_TRANSMIT_ONLY:
		test_start_i2c_transfer();
		break;
	case I2C_TRANSMIT_RECEIVE:
		test_start_i2c_transceive();
		break;
	default:
		break;
	}
}

/* Function to test fast-plus bus mode */
static void test_fast_plus_bus_mode(enum i2c_test_scenario scenario)
{
	const struct device *const i2c_master_dev = DEVICE_DT_GET(I2C_MASTER);

	/* Reset runtime config before each test */
	i2c_test_reset_runtime_config(&i2c_test_ctx);

	skip_if_bus_freq_ne(1000000);
	zassert_true(device_is_ready(i2c_master_dev),
			 "i2c: Master Device is not ready.\n");

	i2c_test_ctx.scenario = scenario;

	i2c_test_ctx.i2c_cfg = I2C_SPEED_SET(I2C_SPEED_FAST_PLUS);
	register_slave_speed_i2c(i2c_test_ctx.i2c_cfg);

	switch (i2c_test_ctx.scenario) {
	case I2C_TRANSMIT_ONLY:
		test_start_i2c_transfer();
		break;
	case I2C_TRANSMIT_RECEIVE:
		test_start_i2c_transceive();
		break;
	default:
		break;
	}
}

/*
 * function : test_high_speed_bus_mode
 * function to test high_speed bus mode
 */
static void test_high_speed_bus_mode(enum i2c_test_scenario scenario)
{
	const struct device *const i2c_master_dev = DEVICE_DT_GET(I2C_MASTER);

	/* Reset runtime config before each test */
	i2c_test_reset_runtime_config(&i2c_test_ctx);

	skip_if_bus_freq_ne(3400000);
	zassert_true(device_is_ready(i2c_master_dev),
			 "i2c: Master Device is not ready.\n");

	i2c_test_ctx.scenario = scenario;

	i2c_test_ctx.i2c_cfg = I2C_SPEED_SET(I2C_SPEED_HIGH);
	register_slave_speed_i2c(i2c_test_ctx.i2c_cfg);

	switch (i2c_test_ctx.scenario) {
	case I2C_TRANSMIT_ONLY:
		test_start_i2c_transfer();
		break;
	case I2C_TRANSMIT_RECEIVE:
		test_start_i2c_transceive();
		break;
	default:
		break;
	}
}

static void *i2c_perf_suite_setup(void)
{
	i2c_test_ctx.slave_dev = DEVICE_DT_GET(I2C_SLAVE);
	i2c_test_ctx.slave_registered = false;
	return &i2c_test_ctx;
}

static void i2c_perf_before(void *fixture)
{
	struct i2c_test_ctx *ctx = fixture;

	perf_ctx.rep = 0;
	i2c_test_ctx.i2c_cfg = I2C_SPEED_SET(I2C_SPEED_STANDARD);
	ctx->slave_registered = false;
	ctx->slv_tx_cnt = 0;
	ctx->slv_rx_cnt = 0;
	memset(ctx->slv_rx_data, 0, BUFF_PERF);
	memset(ctx->slv_tx_data, 0, BUFF_PERF);
	i2c_test_reset_callback_state(ctx);
}

static void i2c_perf_after(void *fixture)
{
	struct i2c_test_ctx *ctx = fixture;
	int ret;

	if (!ctx->slave_registered) {
		return;
	}

	ret = i2c_target_unregister(ctx->slave_dev, &i2c_tcfg_perf);
	/* ENOTSUP means operation not supported */
	if (ret == -ENOTSUP) {
		LOG_WRN("I2C target mode not supported");
	} else {
		zassert_equal(ret, 0,
				  "i2c_target_unregister failed: %d", ret);
	}

	ctx->slave_registered = false;
}

static void i2c_perf_teardown(void *fixture)
{
	i2c_perf_after(fixture);
}

ZTEST(i2c_perf, test_tx_std)
{
	test_standard_bus_mode(I2C_TRANSMIT_ONLY);
}

ZTEST(i2c_perf, test_tx_fast)
{
	test_fast_bus_mode(I2C_TRANSMIT_ONLY);
}

ZTEST(i2c_perf, test_tx_fast_plus)
{
	test_fast_plus_bus_mode(I2C_TRANSMIT_ONLY);
}

ZTEST(i2c_perf, test_tx_high)
{
	test_high_speed_bus_mode(I2C_TRANSMIT_ONLY);
}

ZTEST(i2c_perf, test_xcv_std)
{
	test_standard_bus_mode(I2C_TRANSMIT_RECEIVE);
}

ZTEST(i2c_perf, test_xcv_fast)
{
	test_fast_bus_mode(I2C_TRANSMIT_RECEIVE);
}

ZTEST(i2c_perf, test_xcv_fast_plus)
{
	test_fast_plus_bus_mode(I2C_TRANSMIT_RECEIVE);
}

ZTEST(i2c_perf, test_xcv_high)
{
	perf_ctx.rep = 1;
	test_high_speed_bus_mode(I2C_TRANSMIT_RECEIVE);
	perf_ctx.rep = 0;
}

ZTEST(i2c_perf, test_xcv_std_rep)
{
	perf_ctx.rep = 1;
	test_standard_bus_mode(I2C_TRANSMIT_RECEIVE);
	perf_ctx.rep = 0;
}

ZTEST(i2c_perf, test_xcv_fast_rep)
{
	perf_ctx.rep = 1;
	test_fast_bus_mode(I2C_TRANSMIT_RECEIVE);
	perf_ctx.rep = 0;
}

ZTEST(i2c_perf, test_tx_rx_std)
{
	test_standard_bus_mode(I2C_TRANSMIT_RECEIVE);
}

ZTEST(i2c_perf, test_tx_rx_fast)
{
	test_fast_bus_mode(I2C_TRANSMIT_RECEIVE);
}

ZTEST(i2c_perf, test_tx_rx_fast_plus)
{
	test_fast_plus_bus_mode(I2C_TRANSMIT_RECEIVE);
}

ZTEST(i2c_perf, test_tx_rx_high)
{
	perf_ctx.rep = 1;
	test_high_speed_bus_mode(I2C_TRANSMIT_RECEIVE);
	perf_ctx.rep = 0;
}

ZTEST(i2c_perf, test_tx_rx_std_repeat)
{
	perf_ctx.rep = 1;
	test_standard_bus_mode(I2C_TRANSMIT_RECEIVE);
	perf_ctx.rep = 0;
}

ZTEST(i2c_perf, test_tx_rx_fast_repeat)
{
	perf_ctx.rep = 1;
	test_fast_bus_mode(I2C_TRANSMIT_RECEIVE);
	perf_ctx.rep = 0;
}

ZTEST_SUITE(i2c_perf, NULL, i2c_perf_suite_setup, i2c_perf_before,
		i2c_perf_after, i2c_perf_teardown);

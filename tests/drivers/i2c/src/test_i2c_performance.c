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
	uint32_t rep;          /* Repetition count */
	uint32_t ctrl_start_cyc;  /* Controller start cycle */
	uint32_t ctrl_end_cyc;    /* Controller end cycle */
	uint32_t tgt_start_cyc;   /* Target start cycle */
	uint32_t tgt_end_cyc;     /* Target end cycle */
	uint32_t ctrl_start_time; /* Controller start time */
	uint32_t ctrl_end_time;   /* Controller end time */
	uint32_t tgt_start_time;  /* Target start time */
	uint32_t tgt_end_time;    /* Target end time */
};

static struct i2c_perf_ctx perf_ctx;

/*
 * Target callback functions - static to avoid
 * linker clashes with test_i2c.c.
 */
static int cb_i2c_perf_target_write_requested(struct i2c_target_config *config)
{
	ARG_UNUSED(config);
	i2c_test_ctx.tgt_tx_cnt = 0;
	i2c_test_note_write_requested(&i2c_test_ctx);
	LOG_DBG("Write requested - counters reset (TX: %u, RX: %u)",
		i2c_test_ctx.tgt_tx_cnt, i2c_test_ctx.tgt_rx_cnt);
	return 0;
}

/*
 * Zephyr I2C target API contract:
 *   read_requested : invoked ONCE at the start of a controller-read phase;
 *                    supplies the FIRST byte via *val.
 *   read_processed : invoked for EVERY SUBSEQUENT byte;
 *                    supplies the NEXT byte via *val.
 * Both must share a single monotonically advancing TX index.
 */
static int cb_i2c_perf_target_read_requested(struct i2c_target_config *config,
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

static int cb_i2c_perf_target_write_received(struct i2c_target_config *config,
						     uint8_t val)
{
	ARG_UNUSED(config);
	if (i2c_test_ctx.tgt_rx_cnt < BUFF_PERF) {
		i2c_test_ctx.tgt_rx_data[i2c_test_ctx.tgt_rx_cnt++] = val;
		LOG_DBG("Target RX [%u]: 0x%02X (total: %u)",
			i2c_test_ctx.tgt_rx_cnt - 1, val,
			i2c_test_ctx.tgt_rx_cnt);
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
	if (i2c_test_ctx.tgt_tx_cnt >= BUFF_PERF) {
		i2c_test_ctx.tgt_tx_cnt = 0;
	}
	*val = i2c_test_ctx.tgt_tx_data[i2c_test_ctx.tgt_tx_cnt++];
	i2c_test_note_read_processed(&i2c_test_ctx);
	LOG_DBG("read_processed: next byte 0x%02X (idx %u)", *val,
		i2c_test_ctx.tgt_tx_cnt - 1U);
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
			    struct i2c_target_config *config,
			    uint8_t **val, uint32_t *len)
{
	ARG_UNUSED(config);

	i2c_test_ctx.tgt_tx_cnt = 0;
	*len = MIN((uint32_t)BUFF_PERF,
		   (uint32_t)CONFIG_I2C_TAR_DATA_BUF_MAX_LEN);

	/* Use the pre-filled tgt_tx_data - not a hardcoded pattern */
	*val = i2c_test_ctx.tgt_tx_data;

	return 0;
}

static void cb_i2c_perf_target_buf_write_received(
			     struct i2c_target_config *config,
			     uint8_t *data_buf,
			     uint32_t len)
{
	uint32_t idx;

	ARG_UNUSED(config);

	LOG_INF("Received %d bytes from master: ", len);
	for (idx = 0; idx < len; idx++) {
		/* Bounds checking to prevent buffer overflow */
		if (i2c_test_ctx.tgt_rx_cnt < BUFF_PERF) {
			i2c_test_ctx.tgt_rx_data[i2c_test_ctx.tgt_rx_cnt++] =
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
#ifdef CONFIG_I2C_TARGET_BUFFER_MODE
	.buf_read_requested = &cb_i2c_perf_target_buf_read_requested,
	.buf_write_received = &cb_i2c_perf_target_buf_write_received,
#else
	.read_requested = &cb_i2c_perf_target_read_requested,
	.read_processed = &cb_i2c_perf_target_read_processed,
	.write_received = &cb_i2c_perf_target_write_received,
#endif
	.stop = &cb_i2c_target_stop,
};

static struct i2c_target_config i2c_tcfg_perf = {
	.callbacks = &i2c_t_cb_perf,
};

static void register_perf_target_speed_i2c(uint32_t i2c_cfg)
{
	int ret;
	const struct device *const i2c_target_dev = i2c_test_ctx.target_dev;

	if (i2c_test_ctx.target_registered && i2c_test_ctx.target_dev) {
		ret = i2c_target_unregister(i2c_test_ctx.target_dev,
						&i2c_tcfg_perf);
		if (ret != 0) {
			LOG_WRN("Perf target unregister failed: %d", ret);
		}

		i2c_test_ctx.target_registered = false;
		k_msleep(10);
	}

	i2c_test_ctx.tgt_rx_cnt = 0;
	i2c_test_ctx.tgt_tx_cnt = 0;
	memset(i2c_test_ctx.tgt_rx_data, 0, BUFF_PERF);
	memset(i2c_test_ctx.tgt_tx_data, 0, BUFF_PERF);
	i2c_test_reset_callback_state(&i2c_test_ctx);

	i2c_tcfg_perf.flags = 0;
	i2c_tcfg_perf.address = TGT_I2C_ADDR;

	ret = i2c_configure(i2c_target_dev, i2c_cfg);
	zassert_equal(ret, 0, "I2C slave config failed");

	ret = i2c_target_register(i2c_target_dev, &i2c_tcfg_perf);
	if (ret == -ENOTSUP) {
		LOG_WRN("I2C target mode not supported");
		ztest_test_skip();
	} else {
		zassert_equal(ret, 0, "i2c_target_register failed: %d", ret);
	}

	if (ret == 0) {
		i2c_test_ctx.target_dev = i2c_target_dev;
		i2c_test_ctx.target_registered = true;
	}
}

/**
 * @brief Perform comprehensive I2C transfer with performance measurement
 *
 * This function executes a complete bidirectional I2C transfer:
 * 1. Master writes data to slave
 * 2. Master reads data from slave
 * 3. Measures timing and cycle performance
 * 4. Validates data integrity
 *
 * Uses the current runtime frequency from i2c_test_ctx.
 *
 * @return 0 on success, negative error code on failure
 */
/* Performance test transfer modes */
enum i2c_perf_mode {
	I2C_PERF_TX_ONLY,   /* Master write to slave only */
	I2C_PERF_RX_ONLY,   /* Master read from slave only */
	I2C_PERF_TXRX       /* Write then read (full transceive) */
};

/**
 * @brief Perform I2C transfer operation for performance testing
 *
 * Supports three modes:
 * - I2C_PERF_TX_ONLY: Master write to slave, measures TX performance
 * - I2C_PERF_RX_ONLY: Master read from slave, measures RX performance
 * - I2C_PERF_TXRX:    Write then read, measures full transceive
 *
 * @param mode Transfer mode (TX_ONLY, RX_ONLY, or TXRX)
 * @return 0 on success, negative error code on failure
 */
int start_i2c_transfer_mode(enum i2c_perf_mode mode)
{
	uint8_t tx_data[BUFF_PERF];
	uint8_t rx_data[BUFF_PERF];
	int ret = 0;
	struct i2c_msg msgs[2];
	size_t transfer_len = BUFF_PERF;
	const struct device *const i2c_controller_dev = i2c_test_ctx.controller_dev;

	ret = i2c_configure(i2c_controller_dev, i2c_test_ctx.i2c_cfg);
	if (ret != 0) {
		LOG_ERR("I2C master configuration failed: %d", ret);
		return ret;
	}

	/* Use distinct random patterns: master TX != slave TX */
	fill_buffer_random(tx_data, BUFF_PERF, SEED_CONTROLLER_TX);
	fill_target_tx_distinct(i2c_test_ctx.tgt_tx_data, BUFF_PERF,
			       SEED_TARGET_TX);

	/* Setup I2C messages for write operation */
	msgs[0].buf = tx_data;
	msgs[0].len = BUFF_PERF;
	msgs[0].flags = I2C_MSG_WRITE | I2C_MSG_STOP;

	/* Setup I2C messages for read operation */
	msgs[1].buf = rx_data;
	msgs[1].len = BUFF_PERF;
	msgs[1].flags = I2C_MSG_READ | I2C_MSG_STOP;

	/* TX phase (for TX_ONLY or TXRX modes) */
	if (mode == I2C_PERF_TX_ONLY || mode == I2C_PERF_TXRX) {
		/* Poison controller RX to detect uninitialized reads */
		poison_buffer(rx_data, BUFF_PERF, POISON_RX);

		perf_ctx.ctrl_start_cyc = k_cycle_get_32();
		perf_ctx.ctrl_start_time = k_uptime_get();
		i2c_prepare_target_rx(BUFF_PERF);
		i2c_test_reset_callback_state(&i2c_test_ctx);

		ret = i2c_transfer(i2c_controller_dev, &msgs[0], 1, TGT_I2C_ADDR);
		zassert_equal(ret, 0, "I2C write transfer failed: %d", ret);

		ret = validate_target_rx(tx_data, BUFF_PERF);
		if (ret != 0) {
			LOG_ERR("Perf: Controller TX -> Target RX validation failed: %d",
				 ret);
			return ret;
		}
		i2c_test_assert_target_contract(&i2c_test_ctx,
					       I2C_TRANSMIT_ONLY,
					       BUFF_PERF, 0U);
		LOG_DBG("I2C Controller TX -> Target RX data validated");

		perf_ctx.ctrl_end_time = k_uptime_get();
		perf_ctx.ctrl_end_cyc = k_cycle_get_32();
	}

	/* RX phase (for RX_ONLY or TXRX modes) */
	if (mode == I2C_PERF_RX_ONLY || mode == I2C_PERF_TXRX) {
		/* Poison controller RX to detect uninitialized reads */
		poison_buffer(rx_data, BUFF_PERF, POISON_RX);

		perf_ctx.tgt_start_time = k_uptime_get();
		perf_ctx.tgt_start_cyc = k_cycle_get_32();
		i2c_test_reset_callback_state(&i2c_test_ctx);

		ret = i2c_transfer(i2c_controller_dev, &msgs[1], 1, TGT_I2C_ADDR);
		zassert_equal(ret, 0, "I2C read transfer failed: %d", ret);

		ret = validate_controller_rx(i2c_test_ctx.tgt_tx_data, rx_data,
					transfer_len);
		if (ret != 0) {
			LOG_ERR("Perf: Target TX -> Controller RX failed: %d",
				ret);
			return ret;
		}
		i2c_test_assert_target_contract(&i2c_test_ctx,
					       I2C_RECEIVE_ONLY,
					       0U, transfer_len);
		LOG_DBG("I2C Target TX -> Controller RX data validated");

		perf_ctx.tgt_end_time = k_uptime_get();
		perf_ctx.tgt_end_cyc = k_cycle_get_32();
	}

	/* Log performance metrics */
	if (mode == I2C_PERF_TX_ONLY || mode == I2C_PERF_TXRX) {
		LOG_INF("Controller TX -> Target: ret=%d, time=%u ms, cyc=%u", ret,
			perf_ctx.ctrl_end_time - perf_ctx.ctrl_start_time,
			perf_ctx.ctrl_end_cyc - perf_ctx.ctrl_start_cyc);
	}
	if (mode == I2C_PERF_RX_ONLY || mode == I2C_PERF_TXRX) {
		LOG_INF("Target TX -> Controller: ret=%d, time=%u ms, cyc=%u", ret,
			perf_ctx.tgt_end_time - perf_ctx.tgt_start_time,
			perf_ctx.tgt_end_cyc - perf_ctx.tgt_start_cyc);
	}

	return ret;
}

/*
 * Frequency-neutral scenario tests that loop through enabled frequencies
 */

/* Scenario: Single transmit operation at all enabled frequencies */
static void test_perf_tx_scenario(void)
{
	struct i2c_test_freq_desc freqs[I2C_TEST_MAX_FREQS];
	int ret;
	int freq_count;
	int i;

	i2c_test_skip_if_no_freqs();
	freq_count = i2c_test_get_enabled_freqs(freqs, ARRAY_SIZE(freqs));

	for (i = 0; i < freq_count; i++) {
		TC_PRINT("Perf TX at %s (%u Hz)\n",
			 freqs[i].name, freqs[i].freq_hz);

		ret = i2c_test_configure_controller_freq(i2c_test_ctx.controller_dev,
						     &i2c_test_ctx, &freqs[i]);
		if (ret == -ENOTSUP) {
			TC_PRINT("Skipping %s: not supported\n", freqs[i].name);
			continue;
		}
		zassert_equal(ret, 0, "Failed to configure %s: %d",
			      freqs[i].name, ret);

		register_perf_target_speed_i2c(
			I2C_SPEED_SET(freqs[i].zephyr_speed));
		ret = start_i2c_transfer_mode(I2C_PERF_TX_ONLY);
		zassert_equal(ret, 0, "I2C TX failed at %s: %d",
			      freqs[i].name, ret);
	}
}

/* Scenario: Transceive operation at all enabled frequencies */
static void test_perf_xcv_scenario(void)
{
	struct i2c_test_freq_desc freqs[I2C_TEST_MAX_FREQS];
	int ret;
	int freq_count;
	int i;

	i2c_test_skip_if_no_freqs();
	freq_count = i2c_test_get_enabled_freqs(freqs, ARRAY_SIZE(freqs));

	for (i = 0; i < freq_count; i++) {
		TC_PRINT("Perf XCV at %s (%u Hz)\n",
			 freqs[i].name, freqs[i].freq_hz);

		ret = i2c_test_configure_controller_freq(i2c_test_ctx.controller_dev,
						     &i2c_test_ctx, &freqs[i]);
		if (ret == -ENOTSUP) {
			TC_PRINT("Skipping %s: not supported\n", freqs[i].name);
			continue;
		}
		zassert_equal(ret, 0, "Failed to configure %s: %d",
			      freqs[i].name, ret);

		register_perf_target_speed_i2c(
			I2C_SPEED_SET(freqs[i].zephyr_speed));
		ret = start_i2c_transfer_mode(I2C_PERF_TXRX);
		zassert_equal(ret, 0, "I2C transceive failed at %s: %d",
			      freqs[i].name, ret);
	}
}

/* Scenario: Speed reconfiguration between adjacent enabled frequencies */
static void test_perf_speed_reconfig_scenario(void)
{
	struct i2c_test_freq_desc freqs[I2C_TEST_MAX_FREQS];
	int ret;
	int freq_count;
	int i;

	i2c_test_skip_if_freqs_lt(2);
	freq_count = i2c_test_get_enabled_freqs(freqs, ARRAY_SIZE(freqs));

	for (i = 0; i < freq_count - 1; i++) {
		TC_PRINT("Perf reconfig %s -> %s\n",
			freqs[i].name, freqs[i + 1].name);

		/* First frequency */
		ret = i2c_test_configure_controller_freq(i2c_test_ctx.controller_dev,
						     &i2c_test_ctx, &freqs[i]);
		if (ret == -ENOTSUP) {
			TC_PRINT("Skipping %s: not supported\n", freqs[i].name);
			continue;
		}
		zassert_equal(ret, 0, "Failed to configure %s: %d",
			      freqs[i].name, ret);

		register_perf_target_speed_i2c(
			I2C_SPEED_SET(freqs[i].zephyr_speed));
		ret = start_i2c_transfer_mode(I2C_PERF_TXRX);
		zassert_equal(ret, 0, "Transfer failed at %s: %d",
			      freqs[i].name, ret);

		/* Switch to next frequency */
		ret = i2c_test_configure_controller_freq(i2c_test_ctx.controller_dev,
						     &i2c_test_ctx,
					     &freqs[i + 1]);
		if (ret == -ENOTSUP) {
			TC_PRINT("Skipping %s: not supported\n",
				 freqs[i + 1].name);
			continue;
		}
		zassert_equal(ret, 0, "Failed to configure %s: %d",
			      freqs[i + 1].name, ret);

		register_perf_target_speed_i2c(
			I2C_SPEED_SET(freqs[i + 1].zephyr_speed));
		ret = start_i2c_transfer_mode(I2C_PERF_TXRX);
		zassert_equal(ret, 0, "Transfer failed at %s: %d",
			      freqs[i + 1].name, ret);
	}
}

static void *i2c_perf_suite_setup(void)
{
	i2c_test_ctx.controller_dev = DEVICE_DT_GET(I2C_CONTROLLER);
	i2c_test_ctx.target_dev = DEVICE_DT_GET(I2C_TARGET);
	i2c_test_ctx.i2c_cfg = I2C_MODE_CONTROLLER |
			       I2C_SPEED_SET(I2C_SPEED_STANDARD);
	i2c_test_ctx.target_registered = false;
	return &i2c_test_ctx;
}

static void i2c_perf_before(void *fixture)
{
	struct i2c_test_ctx *ctx = fixture;

	perf_ctx.rep = 0;
	i2c_test_reset_runtime_config(ctx);
}

static void i2c_perf_after(void *fixture)
{
	struct i2c_test_ctx *ctx = fixture;
	int ret;

	if (!ctx->target_registered) {
		return;
	}

	ret = i2c_target_unregister(ctx->target_dev, &i2c_tcfg_perf);
	/* ENOTSUP means operation not supported */
	if (ret == -ENOTSUP) {
		LOG_WRN("I2C target mode not supported");
	} else {
		zassert_equal(ret, 0, "i2c_target_unregister failed: %d", ret);
	}

	ctx->target_registered = false;
}

/* Frequency-neutral performance scenario tests */

ZTEST(i2c_perf, test_tx_all_freqs)
{
	test_perf_tx_scenario();
}

ZTEST(i2c_perf, test_xcv_all_freqs)
{
	test_perf_xcv_scenario();
}

ZTEST(i2c_perf, test_speed_reconfig)
{
	test_perf_speed_reconfig_scenario();
}

ZTEST_SUITE(i2c_perf, NULL, i2c_perf_suite_setup, i2c_perf_before,
	    i2c_perf_after, NULL);

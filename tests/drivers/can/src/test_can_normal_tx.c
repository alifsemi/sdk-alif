/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/*
 * CAN Normal Mode TX Test Suite
 *
 * Self-contained tests that exercise TX paths only.
 * Requires a CAN analyzer on the bus to ACK frames.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/can.h>
#include <zephyr/ztest.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(alif_can_normal_tx, LOG_LEVEL_INF);

/* Test bitrates (override via Kconfig) */
#define TEST_BITRATE_NOMINAL      CONFIG_CAN_TEST_BITRATE_NOMINAL
#define TEST_SAMPLE_POINT_NOMINAL CONFIG_CAN_TEST_SAMPLE_POINT_NOMINAL
#define TEST_BITRATE_DATA         CONFIG_CAN_TEST_BITRATE_DATA
#define TEST_SAMPLE_POINT_DATA    CONFIG_CAN_TEST_SAMPLE_POINT_DATA

/* Timeouts */
#define TEST_SEND_TIMEOUT_MS      2000

/* Test CAN IDs */
#define TEST_STD_ID_TX            0x5A5U
#define TEST_EXT_ID_TX            0x01FF5A5AU

/* Test frames */
static const struct can_frame test_tx_std_frame = {
	.flags = 0,
	.id    = TEST_STD_ID_TX,
	.dlc   = 8,
	.data  = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08}
};

static const struct can_frame test_tx_ext_frame = {
	.flags = CAN_FRAME_IDE,
	.id    = TEST_EXT_ID_TX,
	.dlc   = 4,
	.data  = {0xDE, 0xAD, 0xBE, 0xEF}
};

static const struct can_frame test_tx_rtr_std_frame = {
	.flags = CAN_FRAME_RTR,
	.id    = TEST_STD_ID_TX,
	.dlc   = 8,
	.data  = {0}
};

static const struct can_frame test_tx_rtr_ext_frame = {
	.flags = CAN_FRAME_RTR | CAN_FRAME_IDE,
	.id    = TEST_EXT_ID_TX,
	.dlc   = 4,
	.data  = {0}
};

static const struct can_frame test_tx_zero_dlc_frame = {
	.flags = 0,
	.id    = TEST_STD_ID_TX,
	.dlc   = 0,
	.data  = {0}
};

#ifdef CONFIG_CAN_FD_MODE
static const struct can_frame test_tx_fd_frame = {
	.flags = CAN_FRAME_FDF | CAN_FRAME_BRS,
	.id    = TEST_STD_ID_TX,
	.dlc   = 0x0FU,
	.data  = {
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
		0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
		0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
		0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
		0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
		0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
		0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F
	}
};

static const struct can_frame test_tx_fd_no_brs_frame = {
	.flags = CAN_FRAME_FDF,
	.id    = TEST_STD_ID_TX,
	.dlc   = 0x0DU,
	.data  = {
		0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
		0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
		0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7,
		0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF
	}
};
#endif

static const struct device *can_dev;
static struct k_sem tx_sem;

static void tx_callback(const struct device *dev, int error, void *user_data)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(user_data);

	zassert_equal(error, 0, "TX callback reported error %d", error);
	k_sem_give(&tx_sem);
}

static void *tx_suite_setup(void)
{
	struct can_timing timing = {0};
	can_mode_t cap;
	int err;

	can_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_canbus));
	zassert_true(device_is_ready(can_dev), "CAN device not ready");

	k_sem_init(&tx_sem, 0, 10);

	err = can_get_capabilities(can_dev, &cap);
	zassert_equal(err, 0, "get_capabilities failed (err %d)", err);

	(void)can_stop(can_dev);

	err = can_calc_timing(can_dev, &timing, TEST_BITRATE_NOMINAL,
			      TEST_SAMPLE_POINT_NOMINAL);
	zassert_equal(err, 0, "calc_timing failed (err %d)", err);

	err = can_set_timing(can_dev, &timing);
	zassert_equal(err, 0, "set_timing failed (err %d)", err);

#ifdef CONFIG_CAN_FD_MODE
	if ((cap & CAN_MODE_FD) != 0) {
		err = can_calc_timing_data(can_dev, &timing, TEST_BITRATE_DATA,
					   TEST_SAMPLE_POINT_DATA);
		zassert_equal(err, 0, "calc_timing_data failed (err %d)", err);

		err = can_set_timing_data(can_dev, &timing);
		zassert_equal(err, 0, "set_timing_data failed (err %d)", err);
	}
#endif

	return NULL;
}

static void tx_suite_before(void *fixture)
{
	ARG_UNUSED(fixture);
	can_mode_t mode = CAN_MODE_NORMAL;
	int err;

#ifdef CONFIG_CAN_FD_MODE
	can_mode_t cap;

	err = can_get_capabilities(can_dev, &cap);
	zassert_equal(err, 0, "get_capabilities failed (err %d)", err);

	if ((cap & CAN_MODE_FD) != 0) {
		mode |= CAN_MODE_FD;
	}
#endif

	(void)can_stop(can_dev);

	err = can_set_mode(can_dev, mode);
	zassert_equal(err, 0, "set_mode failed (err %d)", err);

	err = can_start(can_dev);
	zassert_equal(err, 0, "can_start failed (err %d)", err);
}

static void send_and_wait(const struct can_frame *frame)
{
	int err;

	k_sem_reset(&tx_sem);

	err = can_send(can_dev, frame, K_MSEC(TEST_SEND_TIMEOUT_MS),
		       tx_callback, NULL);
	zassert_equal(err, 0, "can_send failed (err %d)", err);

	err = k_sem_take(&tx_sem, K_MSEC(TEST_SEND_TIMEOUT_MS));
	zassert_equal(err, 0,
		      "TX timeout - no ACK from bus (is CANalyzer connected?)");
}

static void assert_post_tx_state(void)
{
	struct can_bus_err_cnt err_cnt;
	enum can_state state;
	int err;

	err = can_get_state(can_dev, &state, &err_cnt);
	zassert_equal(err, 0, "get_state failed (err %d)", err);
	zassert_equal(state, CAN_STATE_ERROR_ACTIVE,
		      "Expected ERROR_ACTIVE after TX, got state %d", state);
	zassert_equal(err_cnt.tx_err_cnt, 0, "TX error counter non-zero after TX");
	zassert_equal(err_cnt.rx_err_cnt, 0, "RX error counter non-zero after TX");
}

ZTEST(can_normal_tx, test_capabilities)
{
	can_mode_t cap;
	int err;

	err = can_get_capabilities(can_dev, &cap);
	zassert_equal(err, 0, "get_capabilities failed (err %d)", err);

	LOG_INF("CAN capabilities: 0x%08X", cap);
}

ZTEST(can_normal_tx, test_send_std_classic)
{
	send_and_wait(&test_tx_std_frame);
	assert_post_tx_state();
	LOG_INF("Classic STD frame sent OK");
}

ZTEST(can_normal_tx, test_send_ext_classic)
{
	send_and_wait(&test_tx_ext_frame);
	assert_post_tx_state();
	LOG_INF("Classic EXT frame sent OK");
}

ZTEST(can_normal_tx, test_send_rtr_std)
{
	send_and_wait(&test_tx_rtr_std_frame);
	assert_post_tx_state();
	LOG_INF("Standard RTR frame sent OK");
}

ZTEST(can_normal_tx, test_send_rtr_ext)
{
	send_and_wait(&test_tx_rtr_ext_frame);
	assert_post_tx_state();
	LOG_INF("Extended RTR frame sent OK");
}

ZTEST(can_normal_tx, test_send_zero_dlc)
{
	send_and_wait(&test_tx_zero_dlc_frame);
	assert_post_tx_state();
	LOG_INF("Zero-DLC frame sent OK");
}

#ifdef CONFIG_CAN_FD_MODE
ZTEST(can_normal_tx, test_send_fd)
{
	can_mode_t cap;
	int err;

	err = can_get_capabilities(can_dev, &cap);
	zassert_equal(err, 0, "get_capabilities failed (err %d)", err);
	if ((cap & CAN_MODE_FD) == 0) {
		ztest_test_skip();
	}

	send_and_wait(&test_tx_fd_frame);
	assert_post_tx_state();
	LOG_INF("CAN FD+BRS frame sent OK");
}

ZTEST(can_normal_tx, test_send_fd_no_brs)
{
	can_mode_t cap;
	int err;

	err = can_get_capabilities(can_dev, &cap);
	zassert_equal(err, 0, "get_capabilities failed (err %d)", err);
	if ((cap & CAN_MODE_FD) == 0) {
		ztest_test_skip();
	}

	send_and_wait(&test_tx_fd_no_brs_frame);
	assert_post_tx_state();
	LOG_INF("CAN FD no-BRS frame sent OK");
}
#endif

ZTEST(can_normal_tx, test_back_to_back_tx)
{
	int err;
	int i;

	k_sem_reset(&tx_sem);

	for (i = 0; i < 10; i++) {
		err = can_send(can_dev, &test_tx_std_frame,
			       K_MSEC(TEST_SEND_TIMEOUT_MS),
			       tx_callback, NULL);
		zassert_equal(err, 0, "can_send failed iter %d (err %d)",
			      i, err);
	}

	for (i = 0; i < 10; i++) {
		err = k_sem_take(&tx_sem, K_MSEC(TEST_SEND_TIMEOUT_MS));
		zassert_equal(err, 0,
			      "TX timeout iter %d - no ACK from bus", i);
	}

	assert_post_tx_state();
	LOG_INF("Back-to-back TX (10 frames) OK");
}

ZTEST(can_normal_tx, test_bus_recovery_api)
{
	can_mode_t mode = CAN_MODE_NORMAL | CAN_MODE_MANUAL_RECOVERY;
	int err;

#ifdef CONFIG_CAN_FD_MODE
	can_mode_t cap;

	err = can_get_capabilities(can_dev, &cap);
	zassert_equal(err, 0, "get_capabilities failed (err %d)", err);
	if ((cap & CAN_MODE_FD) != 0) {
		mode |= CAN_MODE_FD;
	}
#endif

	err = can_stop(can_dev);
	zassert_equal(err, 0, "can_stop failed (err %d)", err);

	err = can_recover(can_dev, K_NO_WAIT);
	zassert_equal(err, -ENETDOWN,
		      "expected -ENETDOWN while stopped, got %d", err);

	err = can_set_mode(can_dev, mode);
	zassert_equal(err, 0, "set manual recovery mode failed (err %d)", err);
	zassert_equal(can_get_mode(can_dev), mode, "mode mismatch after set_mode");

	err = can_start(can_dev);
	zassert_equal(err, 0, "can_start failed (err %d)", err);

	/* Not bus-off: driver returns success and does not run recovery. */
	err = can_recover(can_dev, K_MSEC(100));
	zassert_equal(err, 0, "can_recover failed while error-active (err %d)", err);
}

ZTEST(can_normal_tx, test_error_state_active)
{
	struct can_bus_err_cnt err_cnt;
	enum can_state state;
	int err;

	err = can_get_state(can_dev, &state, &err_cnt);
	zassert_equal(err, 0, "get_state failed (err %d)", err);

	zassert_equal(state, CAN_STATE_ERROR_ACTIVE,
		      "Expected ERROR_ACTIVE, got state %d", state);
	zassert_equal(err_cnt.tx_err_cnt, 0, "TX error counter should be 0");
	zassert_equal(err_cnt.rx_err_cnt, 0, "RX error counter should be 0");
}

ZTEST(can_normal_tx, test_stats_accessors)
{
	uint32_t bit_errors = can_stats_get_bit_errors(can_dev);
	uint32_t bit0_errors = can_stats_get_bit0_errors(can_dev);
	uint32_t bit1_errors = can_stats_get_bit1_errors(can_dev);
	uint32_t stuff_errors = can_stats_get_stuff_errors(can_dev);
	uint32_t crc_errors = can_stats_get_crc_errors(can_dev);
	uint32_t form_errors = can_stats_get_form_errors(can_dev);
	uint32_t ack_errors = can_stats_get_ack_errors(can_dev);
	uint32_t rx_overruns = can_stats_get_rx_overruns(can_dev);

	LOG_INF("stats: bit=%u bit0=%u bit1=%u stuff=%u crc=%u form=%u ack=%u ovr=%u",
		bit_errors, bit0_errors, bit1_errors, stuff_errors,
		crc_errors, form_errors, ack_errors, rx_overruns);
}

ZTEST_SUITE(can_normal_tx, NULL, tx_suite_setup, tx_suite_before, NULL, NULL);

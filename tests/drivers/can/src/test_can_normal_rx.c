/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/*
 * CAN Normal Mode RX Test Suite
 *
 * Each test installs its own RX filter, blocks on a semaphore until the
 * expected frame arrives, validates it, then removes the filter.
 * No shared msgq, no cross-test contention.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/can.h>
#include <zephyr/ztest.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(alif_can_normal_rx, LOG_LEVEL_INF);

/* Test bitrates (override via Kconfig) */
#define TEST_BITRATE_NOMINAL      CONFIG_CAN_TEST_BITRATE_NOMINAL
#define TEST_SAMPLE_POINT_NOMINAL CONFIG_CAN_TEST_SAMPLE_POINT_NOMINAL
#define TEST_BITRATE_DATA         CONFIG_CAN_TEST_BITRATE_DATA
#define TEST_SAMPLE_POINT_DATA    CONFIG_CAN_TEST_SAMPLE_POINT_DATA

/* Timeouts: 0 = block forever (manual testing) */
#define TEST_RECV_TIMEOUT_MS      CONFIG_CAN_TEST_RX_TIMEOUT_MS

/* Test CAN IDs */
#define TEST_STD_ID_RX            0x123U
#define TEST_EXT_ID_RX            0x18ABCDEFU

/* Expected payloads from CANalyzer */
#define CAN_RX_PAYLOAD_LEN        8U
static const uint8_t can_rx_expected_data[CAN_RX_PAYLOAD_LEN] = {
	0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22
};

static const struct device *can_dev;
static struct k_sem rx_sem;
static struct can_frame rx_frame;

static void rx_callback(const struct device *dev, struct can_frame *frame,
			void *user_data)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(user_data);

	LOG_INF("RX callback: id=0x%08X, dlc=%u, flags=0x%02X",
		frame->id, frame->dlc, frame->flags);

	rx_frame = *frame;
	k_sem_give(&rx_sem);
}

/**
 * @brief Block until a frame matching @p filter arrives.
 *
 * Installs the filter, waits on rx_sem, validates basic fields,
 * then removes the filter.  Returns 0 on success, -EAGAIN on timeout.
 */
static int recv_frame(const struct can_filter *filter,
		      struct can_frame *frame)
{
	int filter_id;
	int err;

	filter_id = can_add_rx_filter(can_dev, rx_callback, NULL, filter);
	zassert_true(filter_id >= 0, "add_rx_filter failed (err %d)", filter_id);

	err = k_sem_take(&rx_sem, TEST_RECV_TIMEOUT_MS ?
			 K_MSEC(TEST_RECV_TIMEOUT_MS) : K_FOREVER);

	can_remove_rx_filter(can_dev, filter_id);

	if (err == -EAGAIN) {
		return -EAGAIN;
	}
	zassert_equal(err, 0, "sem_take failed (err %d)", err);

	*frame = rx_frame;
	return 0;
}

static void *rx_suite_setup(void)
{
	struct can_timing timing = {0};
	can_mode_t cap;
	int err;

	can_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_canbus));
	zassert_true(device_is_ready(can_dev), "CAN device not ready");

	k_sem_init(&rx_sem, 0, 1);

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

static void rx_suite_before(void *fixture)
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

static void rx_suite_teardown(void *fixture)
{
	ARG_UNUSED(fixture);
}

/**
 * @brief Receive a standard frame from the external CANalyzer.
 */
ZTEST(can_normal_rx, test_receive_std_from_CANalyzer)
{
	struct can_filter filter = {
		.flags = 0,
		.id    = TEST_STD_ID_RX,
		.mask  = CAN_STD_ID_MASK
	};
	struct can_frame frame;
	int err;

	LOG_INF("=== Waiting for standard frame ===");
	LOG_INF("CANalyzer -> ID 0x%03X, DLC=8, data=AA BB CC DD EE FF 11 22",
		TEST_STD_ID_RX);

	err = recv_frame(&filter, &frame);
	if (err == -EAGAIN) {
		ztest_test_skip();
	}

	zassert_equal(frame.id, TEST_STD_ID_RX,
		      "RX ID mismatch: expected 0x%03X, got 0x%08X",
		      TEST_STD_ID_RX, frame.id);
	zassert_equal(frame.dlc, CAN_RX_PAYLOAD_LEN,
		      "RX DLC mismatch: expected %u, got %u",
		      CAN_RX_PAYLOAD_LEN, frame.dlc);
	zassert_mem_equal(frame.data, can_rx_expected_data,
			CAN_RX_PAYLOAD_LEN, "RX data mismatch");

	LOG_INF("Standard frame received OK");
}

/**
 * @brief Receive an extended frame from the external CANalyzer.
 */
ZTEST(can_normal_rx, test_receive_ext_from_CANalyzer)
{
	static const uint8_t ext_expected_data[4] = {0xDE, 0xAD, 0xBE, 0xEF};
	struct can_filter filter = {
		.flags = CAN_FILTER_IDE,
		.id    = TEST_EXT_ID_RX,
		.mask  = CAN_EXT_ID_MASK
	};
	struct can_frame frame;
	int err;

	LOG_INF("=== Waiting for extended frame ===");
	LOG_INF("CANalyzer -> ID 0x%08X, DLC=4, data=DE AD BE EF",
		TEST_EXT_ID_RX);

	err = recv_frame(&filter, &frame);
	if (err == -EAGAIN) {
		ztest_test_skip();
	}

	zassert_equal(frame.id, TEST_EXT_ID_RX,
		      "RX ID mismatch: expected 0x%08X, got 0x%08X",
		      TEST_EXT_ID_RX, frame.id);
	zassert_equal(frame.dlc, 4U,
		      "RX DLC mismatch: expected 4, got %u", frame.dlc);
	zassert_mem_equal(frame.data, ext_expected_data, 4U,
			"RX data mismatch");

	LOG_INF("Extended frame received OK");
}

/**
 * @brief Receive an RTR frame from the external CANalyzer.
 */
ZTEST(can_normal_rx, test_receive_rtr_from_CANalyzer)
{
	struct can_filter filter = {
		.flags = 0,
		.id    = TEST_STD_ID_RX,
		.mask  = CAN_STD_ID_MASK
	};
	struct can_frame frame;
	int err;

	LOG_INF("=== Waiting for RTR frame ===");
	LOG_INF("CANalyzer -> RTR frame ID 0x%03X, DLC=8", TEST_STD_ID_RX);

	err = recv_frame(&filter, &frame);
	if (err == -EAGAIN) {
		ztest_test_skip();
	}

	zassert_equal(frame.id, TEST_STD_ID_RX,
		      "RX ID mismatch: expected 0x%03X, got 0x%08X",
		      TEST_STD_ID_RX, frame.id);
	zassert_equal(frame.dlc, 8U,
		      "RX DLC mismatch: expected 8, got %u", frame.dlc);
	zassert_true((frame.flags & CAN_FRAME_RTR) != 0,
		     "RX frame is not an RTR frame");

	LOG_INF("RTR frame received OK");
}

#ifdef CONFIG_CAN_FD_MODE
/**
 * @brief Receive a CAN FD frame from the external CANalyzer.
 */
ZTEST(can_normal_rx, test_receive_fd_from_CANalyzer)
{
	static const uint8_t fd_expected_data[64] = {
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
		0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
		0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
		0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
		0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
		0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
		0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F
	};
	can_mode_t cap;
	struct can_filter filter = {
		.flags = CAN_FILTER_IDE,
		.id    = TEST_EXT_ID_RX,
		.mask  = CAN_EXT_ID_MASK
	};
	struct can_frame frame;
	int err;

	err = can_get_capabilities(can_dev, &cap);
	zassert_equal(err, 0, "get_capabilities failed (err %d)", err);

	if ((cap & CAN_MODE_FD) == 0) {
		ztest_test_skip();
	}

	LOG_INF("=== Waiting for CAN FD frame ===");
	LOG_INF("CANalyzer -> FD+BRS frame ID 0x%08X, DLC=15, data=00..3F",
		TEST_EXT_ID_RX);

	err = recv_frame(&filter, &frame);
	if (err == -EAGAIN) {
		ztest_test_skip();
	}

	zassert_equal(frame.id, TEST_EXT_ID_RX,
		      "RX ID mismatch: expected 0x%08X, got 0x%08X",
		      TEST_EXT_ID_RX, frame.id);
	zassert_equal(frame.dlc, 0x0FU,
		      "RX DLC mismatch: expected 15, got %u", frame.dlc);
	zassert_true((frame.flags & CAN_FRAME_FDF) != 0,
		     "RX frame is not an FD frame");
	zassert_true((frame.flags & CAN_FRAME_BRS) != 0,
		     "RX frame does not have BRS set");
	zassert_mem_equal(frame.data, fd_expected_data, 64U,
			"RX data mismatch");

	LOG_INF("CAN FD frame received OK");
}
#endif /* CONFIG_CAN_FD_MODE */

ZTEST_SUITE(can_normal_rx, NULL, rx_suite_setup, rx_suite_before,
	    rx_suite_teardown, NULL);

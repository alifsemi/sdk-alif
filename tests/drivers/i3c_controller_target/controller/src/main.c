/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include "i3c_protocol.h"

#include <string.h>

#include <zephyr/drivers/i3c.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/ztest.h>

LOG_MODULE_REGISTER(i3c_controller, LOG_LEVEL_INF);

static const struct device *bus;
static struct i3c_device_desc *peer;
static uint8_t peer_bcr;
static bool ibi_ready;
static uint8_t seq;
static struct k_sem ibi_sem;
static uint8_t ibi_buf[8];
static uint8_t ibi_len;
static uint32_t peer_caps;
static uint16_t peer_max_payload = 32U;
static uint8_t peer_hdr;

static int ibi_cb(struct i3c_device_desc *target, struct i3c_ibi_payload *payload)
{
	ARG_UNUSED(target);
	ibi_len = 0;
	if (payload != NULL) {
		ibi_len = payload->payload_len;
		if (ibi_len > sizeof(ibi_buf)) {
			ibi_len = sizeof(ibi_buf);
		}
		memcpy(ibi_buf, payload->payload, ibi_len);
	}
	k_sem_give(&ibi_sem);
	return 0;
}

static void require_peer(void)
{
	if (peer == NULL || peer->dynamic_addr == 0U) {
		ztest_test_skip();
	}
}

static void require_ibi(void)
{
	require_peer();
	if (!ibi_ready || (peer_bcr & I3C_BCR_IBI_REQUEST_CAPABLE) == 0U) {
		ztest_test_skip();
	}
}

static uint8_t next_seq(void)
{
	seq++;
	if (seq == 0U) {
		seq = 1U;
	}
	return seq;
}

static void refresh_peer(void)
{
	peer = i3c_test_find_peer(bus);
	if (peer != NULL) {
		peer->ibi_cb = ibi_cb;
		peer_bcr = peer->bcr;
	}
}

/* DAA can pair our descriptor with another device's address. Accept it only
 * after GETPID returns the test PID.
 */
static bool peer_pid_matches(void)
{
	uint8_t raw[6];
	uint64_t pid;
	int ret;

	if (peer == NULL || peer->dynamic_addr == 0U) {
		return false;
	}

	ret = i3c_test_getpid(bus, peer->dynamic_addr, raw);
	if (ret != 0) {
		LOG_WRN("GETPID DA 0x%02x failed: %d", peer->dynamic_addr, ret);
		return false;
	}

	pid = sys_get_be48(raw);
	if (pid != I3C_TEST_PID) {
		LOG_WRN("DA 0x%02x is PID 0x%llx, not the test target", peer->dynamic_addr,
			(unsigned long long)pid);
		return false;
	}

	return true;
}

static int enable_ibi(void)
{
	int ret;

	if (peer == NULL) {
		return -ENODEV;
	}
	peer->ibi_cb = ibi_cb;
	ret = i3c_ibi_enable(peer);
	ibi_ready = (ret == 0);
	return ret;
}

static int exchange_mode(uint8_t opcode, uint8_t sequence, const uint8_t *data, uint8_t len,
			 uint8_t resp_len, struct i3c_test_frame *resp, bool want_ibi,
			 uint8_t hdr_mode)
{
	struct i3c_test_frame req = {
		.opcode = opcode,
		.sequence = sequence,
		.status = I3C_TEST_STS_OK,
		.length = len,
	};
	uint8_t tx[I3C_TEST_FRAME_MAX];
	uint8_t rx[I3C_TEST_FRAME_MAX];
	int n;
	int expect;
	int ret;

	if (len > 0U) {
		memcpy(req.payload, data, len);
	}
	n = i3c_test_frame_pack(&req, tx, sizeof(tx));
	if (n < 0) {
		return n;
	}

	while (k_sem_take(&ibi_sem, K_NO_WAIT) == 0) {
	}

	ret = i3c_test_write_mode(peer, tx, (size_t)n, hdr_mode);
	if (ret != 0) {
		return ret;
	}

	if (want_ibi) {
		ret = k_sem_take(&ibi_sem, K_MSEC(500));
		if (ret != 0) {
			return ret;
		}
		/* The target queues its private-read response after IBI completion. */
		k_msleep(1);
	} else {
		k_msleep(20);
	}

	expect = I3C_TEST_FRAME_HDR + resp_len + 1;
	ret = i3c_test_read_mode(peer, rx, (size_t)expect, hdr_mode);
	if (ret != 0) {
		return ret;
	}

	return i3c_test_frame_parse(rx, (size_t)expect, resp);
}

static int exchange(uint8_t opcode, uint8_t sequence, const uint8_t *data, uint8_t len,
		    uint8_t resp_len, struct i3c_test_frame *resp, bool want_ibi)
{
	return exchange_mode(opcode, sequence, data, len, resp_len, resp, want_ibi, 0U);
}

static int command(uint8_t opcode, const uint8_t *data, uint8_t len, uint8_t resp_len,
		   struct i3c_test_frame *resp, bool want_ibi)
{
	return exchange(opcode, next_seq(), data, len, resp_len, resp, want_ibi);
}

static int read_peer_caps(void)
{
	struct i3c_test_frame resp;
	int ret = command(I3C_TEST_OP_CAPS, NULL, 0, I3C_TEST_CAPS_LEN, &resp, false);

	if (ret != 0) {
		return ret;
	}
	if (resp.status != I3C_TEST_STS_OK || resp.length != I3C_TEST_CAPS_LEN ||
	    resp.payload[0] != I3C_TEST_FRAME_VERSION) {
		return -EPROTO;
	}
	peer_caps = sys_get_le32(&resp.payload[1]);
	peer_max_payload = sys_get_le16(&resp.payload[5]);
	peer_hdr = resp.payload[11];
	LOG_INF("Target caps 0x%08x payload %u HDR 0x%02x", peer_caps, peer_max_payload, peer_hdr);
	return 0;
}

static void *suite_setup(void)
{
	int ret;

	k_sem_init(&ibi_sem, 0, 1);
	bus = i3c_test_bus();
	zassert_true(bus != NULL, "zephyr,i3c is not set");
	zassert_true(device_is_ready(bus), "I3C controller not ready");

	refresh_peer();
	if (peer_pid_matches()) {
		LOG_INF("Target already discovered at DA 0x%02x", peer->dynamic_addr);
	} else {
		int64_t start;

		/* The test PID has no dynamic address in the devicetree device list.
		 * Broadcast ENEC allows that target to raise hot-join, then an IBI
		 * after the controller assigns an address.
		 */
		ret = i3c_test_enec_broadcast(bus, I3C_CCC_EVT_ALL);
		if (ret != 0) {
			LOG_WRN("Broadcast ENEC failed: %d", ret);
		} else {
			LOG_INF("Test PID is not an addressed devicetree entry; broadcast ENEC");
		}

		start = k_uptime_get();
		LOG_INF("Wait up to %d ms for target hot-join", CONFIG_I3C_TEST_DISCOVER_WAIT_MS);
		do {
			refresh_peer();
			if (peer_pid_matches()) {
				break;
			}

			k_msleep(200);
		} while ((k_uptime_get() - start) < CONFIG_I3C_TEST_DISCOVER_WAIT_MS);
	}

	if (!peer_pid_matches()) {
		LOG_WRN("PID 0x%llx was not assigned an address", (unsigned long long)I3C_TEST_PID);
		peer = NULL;
		return NULL;
	}

	LOG_INF("Peer DA 0x%02x BCR 0x%02x", peer->dynamic_addr, peer->bcr);
	ret = enable_ibi();
	if (ret != 0) {
		LOG_WRN("IBI enable failed: %d", ret);
	} else {
		LOG_INF("Wait up to %d ms for target ready IBI", CONFIG_I3C_TEST_DISCOVER_WAIT_MS);
		ret = k_sem_take(&ibi_sem, K_MSEC(CONFIG_I3C_TEST_DISCOVER_WAIT_MS));
		if (ret != 0) {
			LOG_WRN("Target ready IBI not received: %d", ret);
		}
	}
	ret = read_peer_caps();
	if (ret != 0) {
		LOG_WRN("Target capability query failed: %d", ret);
	}
	return NULL;
}

static void suite_teardown(void *fixture)
{
	struct i3c_test_frame resp;

	ARG_UNUSED(fixture);
	if (peer != NULL && peer->dynamic_addr != 0U) {
		(void)command(I3C_TEST_OP_DONE, NULL, 0, 0, &resp, false);
		(void)i3c_ibi_disable(peer);
	}
}

ZTEST_SUITE(i3c_controller, NULL, suite_setup, NULL, NULL, suite_teardown);

ZTEST(i3c_controller, test_controller_ready)
{
	zassert_true(device_is_ready(bus), "I3C controller not ready");
}

ZTEST(i3c_controller, test_capabilities)
{
	require_peer();
	zassert_true((peer_caps & I3C_TEST_CAP_BULK) != 0U, "bulk capability missing");
	zassert_true(peer_max_payload > 0U && peer_max_payload <= I3C_TEST_PAYLOAD_MAX,
		     "payload limit %u", peer_max_payload);
}

ZTEST(i3c_controller, test_null_and_zero_length)
{
	uint8_t byte = 0x11;
	struct i3c_device_desc desc = {
		.bus = bus,
		.dynamic_addr = 0x09,
	};

	struct i3c_test_frame oversized = {
		.length = I3C_TEST_PAYLOAD_MAX + 1U,
	};
	uint8_t packed[8];

	zassert_equal(i3c_test_write(NULL, &byte, 1), -ENODEV, "NULL descriptor");
	zassert_equal(i3c_test_write(&desc, NULL, 1), -EINVAL, "NULL buffer");
	zassert_equal(i3c_test_write(&desc, &byte, 0), -EINVAL, "zero length");
	zassert_equal(i3c_test_read(&desc, NULL, 1), -EINVAL, "NULL read buffer");
	zassert_equal(i3c_test_frame_pack(&oversized, packed, sizeof(packed)), -EINVAL,
		      "oversized frame");
}

ZTEST(i3c_controller, test_invalid_address)
{
	uint8_t pid[6];
	int ret = i3c_test_getpid(bus, 0x00, pid);

	zassert_true(ret < 0, "GETPID to DA 0x00 returned %d", ret);
	ret = i3c_test_getpid(bus, 0x7F, pid);
	zassert_true(ret < 0, "GETPID to DA 0x7F returned %d", ret);
}

/* Bosch BMI323 provisioned ID from reg <0x69 0x0770 0x10431000>. */
#define BMI323_PID (((uint64_t)0x0770 << 32) | 0x10431000ULL)
#define BMI323_CHIP_ID 0x43U

ZTEST(i3c_controller, test_discover_one)
{
	int count;

	require_peer();
	count = i3c_test_assigned_count(bus);
	zassert_true(count >= 1, "assigned count %d", count);
	zassert_true(peer->dynamic_addr >= 0x08 && peer->dynamic_addr <= 0x7D,
		     "DA 0x%02x out of range", peer->dynamic_addr);
	zassert_equal(peer->pid, I3C_TEST_PID, "descriptor PID");
	LOG_INF("Addressed I3C devices: %d, test target DA 0x%02x", count, peer->dynamic_addr);
}

/* The BMI323 is another devicetree child. Bus init assigns it. This test reads
 * it directly. The hot-join and broadcast ENEC path remains only for the test
 * target PID when that devicetree entry has no dynamic address.
 */
ZTEST(i3c_controller, test_bmi323_child)
{
	const struct i3c_device_id id = {
		.pid = BMI323_PID,
	};
	struct i3c_device_desc *sensor = i3c_device_find(bus, &id);
	uint8_t raw[4];
	int ret;

	if (sensor == NULL || sensor->dynamic_addr == 0U) {
		ztest_test_skip();
	}

	ret = i3c_burst_read(sensor, 0x00, raw, sizeof(raw));
	zassert_ok(ret, "BMI323 chip-id read: %d", ret);
	/* I3C reads two dummy bytes ahead of the register word. */
	zassert_equal(raw[2], BMI323_CHIP_ID, "BMI323 chip id 0x%02x", raw[2]);
	LOG_INF("BMI323 DA 0x%02x PID 0x%llx chip 0x%02x", sensor->dynamic_addr,
		(unsigned long long)sensor->pid, raw[2]);
	if (peer != NULL && peer->dynamic_addr != 0U) {
		LOG_INF("Test target DA 0x%02x PID 0x%llx", peer->dynamic_addr,
			(unsigned long long)peer->pid);
	}
}

ZTEST(i3c_controller, test_pid_bcr_dcr)
{
	uint8_t raw[6];
	uint8_t bcr = 0;
	uint8_t dcr = 0;
	uint64_t pid;
	int ret;

	require_peer();
	ret = i3c_test_getpid(bus, peer->dynamic_addr, raw);
	zassert_ok(ret, "GETPID failed: %d", ret);
	pid = sys_get_be48(raw);
	zassert_equal(pid, I3C_TEST_PID, "GETPID 0x%llx", (unsigned long long)pid);

	ret = i3c_test_getbcr(bus, peer->dynamic_addr, &bcr);
	zassert_ok(ret, "GETBCR failed: %d", ret);
	peer_bcr = bcr;
	zassert_equal(bcr, peer->bcr, "BCR mismatch desc 0x%02x ccc 0x%02x", peer->bcr, bcr);

	ret = i3c_test_getdcr(bus, peer->dynamic_addr, &dcr);
	zassert_ok(ret, "GETDCR failed: %d", ret);
	zassert_equal(dcr, I3C_TEST_DCR, "DCR 0x%02x", dcr);
}

ZTEST(i3c_controller, test_rstdaa_rediscover)
{
	uint8_t previous;
	int ret;

	require_peer();
	previous = peer->dynamic_addr;
	ret = i3c_test_discover(bus);
	zassert_true(ret >= 0, "rediscover failed: %d", ret);
	refresh_peer();
	require_peer();
	zassert_true(peer->dynamic_addr != 0U, "DA cleared after DAA");
	LOG_INF("DA 0x%02x -> 0x%02x", previous, peer->dynamic_addr);
	(void)enable_ibi();
}

ZTEST(i3c_controller, test_status_and_lengths)
{
	uint16_t status = 0;
	uint16_t mrl = 0;
	uint16_t mwl = 0;
	uint16_t original_mwl;
	int ret;

	require_peer();
	ret = i3c_test_getstatus(bus, peer->dynamic_addr, &status);
	zassert_ok(ret, "GETSTATUS failed: %d", ret);

	ret = i3c_test_getmrl(bus, peer->dynamic_addr, &mrl);
	zassert_ok(ret, "GETMRL failed: %d", ret);
	zassert_true(mrl > 0U, "MRL is zero");

	ret = i3c_test_getmwl(bus, peer->dynamic_addr, &original_mwl);
	zassert_ok(ret, "initial GETMWL failed: %d", ret);
	ret = i3c_test_setmwl(bus, peer->dynamic_addr, 128);
	zassert_ok(ret, "SETMWL failed: %d", ret);
	ret = i3c_test_getmwl(bus, peer->dynamic_addr, &mwl);
	zassert_ok(ret, "GETMWL failed: %d", ret);
	zassert_equal(mwl, 128, "MWL 0x%04x", mwl);
	ret = i3c_test_setmwl(bus, peer->dynamic_addr, original_mwl);
	zassert_ok(ret, "restore MWL failed: %d", ret);
}

ZTEST(i3c_controller, test_enec_disec)
{
	int ret;

	require_peer();
	ret = i3c_test_enec(bus, peer->dynamic_addr, I3C_CCC_EVT_INTR);
	if (ret == -ENOTSUP) {
		ztest_test_skip();
	}
	zassert_ok(ret, "ENEC failed: %d", ret);

	ret = i3c_test_disec(bus, peer->dynamic_addr, I3C_CCC_EVT_INTR);
	zassert_ok(ret, "DISEC failed: %d", ret);

	ret = i3c_test_enec_broadcast(bus, I3C_CCC_EVT_INTR);
	zassert_ok(ret, "broadcast ENEC failed: %d", ret);
	ret = i3c_test_disec_broadcast(bus, I3C_CCC_EVT_INTR);
	zassert_ok(ret, "broadcast DISEC failed: %d", ret);
	(void)enable_ibi();
}

ZTEST(i3c_controller, test_optional_ccc)
{
	uint8_t caps[4] = {0};
	int caps_ret;
	int rst_ret;
	bool exercised = false;

	require_peer();
	caps_ret = i3c_test_getcaps(bus, peer->dynamic_addr, caps, sizeof(caps));
	if (caps_ret != -ENOTSUP && caps_ret != -ENXIO) {
		zassert_ok(caps_ret, "GETCAPS failed: %d", caps_ret);
		exercised = true;
	}

	rst_ret = i3c_test_rstact(bus, peer->dynamic_addr, 0x00);
	if (rst_ret != -ENOTSUP && rst_ret != -ENXIO) {
		zassert_ok(rst_ret, "RSTACT failed: %d", rst_ret);
		exercised = true;
	}

	if (!exercised) {
		ztest_test_skip();
	}
}

static void expect_echo(const uint8_t *data, uint8_t len)
{
	struct i3c_test_frame resp;
	int ret = command(I3C_TEST_OP_ECHO, data, len, len, &resp, false);

	zassert_ok(ret, "echo %u failed: %d", len, ret);
	zassert_equal(resp.status, I3C_TEST_STS_OK, "echo status %u", resp.status);
	zassert_equal(resp.length, len, "echo length");
	zassert_mem_equal(resp.payload, data, len, "echo payload");
}

ZTEST(i3c_controller, test_echo_lengths)
{
	static const uint8_t sizes[] = {1, 4, 16, 32};

	require_peer();
	for (size_t i = 0; i < ARRAY_SIZE(sizes); i++) {
		uint8_t buf[I3C_TEST_PAYLOAD_MAX];

		for (uint8_t n = 0; n < sizes[i]; n++) {
			buf[n] = (uint8_t)(0x20 + n + i);
		}
		expect_echo(buf, sizes[i]);
	}
}

ZTEST(i3c_controller, test_bulk_throughput)
{
	struct i3c_test_frame resp;
	uint8_t data[I3C_TEST_PAYLOAD_MAX];
	uint32_t bytes = 0;
	uint16_t chunk;
	int64_t start;
	int64_t elapsed;
	int ret;

	require_peer();
	if ((peer_caps & I3C_TEST_CAP_BULK) == 0U) {
		ztest_test_skip();
	}
	chunk = MIN((uint16_t)CONFIG_I3C_TEST_BULK_CHUNK, peer_max_payload);
	zassert_true(chunk > 0U, "bulk chunk is zero");
	for (uint16_t i = 0; i < chunk; i++) {
		data[i] = (uint8_t)(i ^ 0xA5U);
	}

	start = k_uptime_get();
	while (bytes < CONFIG_I3C_TEST_BULK_BYTES) {
		uint16_t len = MIN(chunk, CONFIG_I3C_TEST_BULK_BYTES - bytes);

		data[0] = (uint8_t)bytes;
		ret = command(I3C_TEST_OP_BULK, data, len, len, &resp, false);
		zassert_ok(ret, "bulk at %u: %d", bytes, ret);
		zassert_equal(resp.status, I3C_TEST_STS_OK, "bulk status %u", resp.status);
		zassert_mem_equal(resp.payload, data, len, "bulk data at %u", bytes);
		bytes += len;
	}
	elapsed = MAX(k_uptime_get() - start, 1);
	LOG_INF("bulk %u bytes in %lld ms: %llu kbit/s", bytes, elapsed,
		((uint64_t)bytes * 8U) / (uint64_t)elapsed);
}

ZTEST(i3c_controller, test_hdr_modes)
{
	struct i3c_config_controller cfg = {0};
	struct i3c_test_frame resp;
	/* Seven protocol bytes plus 17 payload bytes gives an even DDR length. */
	uint8_t payload[17];
	uint8_t ddr = I3C_TEST_MODE_HDR_DDR;
	uint8_t sdr = I3C_TEST_MODE_SDR;
	int ret;

	require_peer();
	if (!IS_ENABLED(CONFIG_I3C_TEST_ENABLE_HDR)) {
		ztest_test_skip();
	}
	ret = i3c_config_get_controller(bus, &cfg);
	zassert_ok(ret, "controller config: %d", ret);
	if ((cfg.supported_hdr & I3C_MSG_HDR_DDR) == 0U ||
	   (peer_hdr & I3C_MSG_HDR_DDR) == 0U) {
		ztest_test_skip();
	}
	for (size_t n = 0; n < sizeof(payload); n++) {
		payload[n] = (uint8_t)n;
	}

	ret = command(I3C_TEST_OP_MODE, &ddr, 1, 1, &resp, false);
	zassert_ok(ret, "enter HDR-DDR: %d", ret);
	zassert_equal(resp.status, I3C_TEST_STS_OK, "enter HDR-DDR status %u", resp.status);
	LOG_INF("HDR-DDR SCL 12500000 Hz");

	ret = exchange_mode(I3C_TEST_OP_BULK, next_seq(), payload, sizeof(payload),
			    sizeof(payload), &resp, false, I3C_MSG_HDR_DDR);
	zassert_ok(ret, "HDR-DDR transfer: %d", ret);
	zassert_equal(resp.status, I3C_TEST_STS_OK, "HDR-DDR status %u", resp.status);
	zassert_mem_equal(resp.payload, payload, sizeof(payload), "HDR-DDR data");

	ret = exchange_mode(I3C_TEST_OP_MODE, next_seq(), &sdr, 1, 1, &resp, false,
			    I3C_MSG_HDR_DDR);
	zassert_ok(ret, "return to SDR: %d", ret);
	zassert_equal(resp.status, I3C_TEST_STS_OK, "return to SDR status %u", resp.status);
}

ZTEST(i3c_controller, test_sdr_speeds)
{
	static const struct {
		uint32_t hz;
		const char *name;
	} rates[] = {
		{12500000U, "SDR0"},
		{8000000U, "SDR1"},
		{6000000U, "SDR2"},
		{4000000U, "SDR3"},
		{2000000U, "SDR4"},
	};
	struct i3c_config_controller cfg = {0};
	uint32_t original;
	uint8_t payload[] = {0x53, 0x44, 0x52};
	int ret;

	require_peer();
	ret = i3c_config_get_controller(bus, &cfg);
	zassert_ok(ret, "controller config: %d", ret);
	original = cfg.scl.i3c;
	for (size_t i = 0; i < ARRAY_SIZE(rates); i++) {
		cfg.scl.i3c = rates[i].hz;
		ret = i3c_configure_controller(bus, &cfg);
		if (ret == -ENOTSUP) {
			LOG_INF("%s %u Hz is not supported", rates[i].name, rates[i].hz);
			continue;
		}
		zassert_ok(ret, "configure %s %u Hz: %d", rates[i].name, rates[i].hz, ret);
		expect_echo(payload, sizeof(payload));
		LOG_INF("%s echo at %u Hz", rates[i].name, rates[i].hz);
	}
	cfg.scl.i3c = original;
	ret = i3c_configure_controller(bus, &cfg);
	zassert_ok(ret, "restore %u Hz: %d", original, ret);
}

ZTEST(i3c_controller, test_fill_read_overwrite)
{
	struct i3c_test_frame resp;
	uint8_t first[] = {0x10, 0x11, 0x12, 0x13};
	uint8_t second[] = {0xAA, 0xBB};
	int ret;

	require_peer();
	ret = command(I3C_TEST_OP_FILL, first, sizeof(first), 0, &resp, false);
	zassert_ok(ret, "fill failed: %d", ret);
	zassert_equal(resp.status, I3C_TEST_STS_OK, "fill status");

	ret = command(I3C_TEST_OP_READ, NULL, 0, sizeof(first), &resp, false);
	zassert_ok(ret, "read failed: %d", ret);
	zassert_mem_equal(resp.payload, first, sizeof(first), "stored data");

	ret = command(I3C_TEST_OP_FILL, second, sizeof(second), 0, &resp, false);
	zassert_ok(ret, "overwrite failed: %d", ret);
	ret = command(I3C_TEST_OP_READ, NULL, 0, sizeof(second), &resp, false);
	zassert_ok(ret, "read after overwrite failed: %d", ret);
	zassert_equal(resp.length, sizeof(second), "overwrite length");
	zassert_mem_equal(resp.payload, second, sizeof(second), "overwrite data");
}

ZTEST(i3c_controller, test_bad_checksum_and_opcode)
{
	struct i3c_test_frame resp;
	uint8_t tx[I3C_TEST_FRAME_MAX];
	struct i3c_test_frame req = {
		.opcode = I3C_TEST_OP_ECHO,
		.sequence = next_seq(),
		.length = 1,
		.payload = {0x42},
	};
	int n;
	int ret;

	require_peer();
	n = i3c_test_frame_pack(&req, tx, sizeof(tx));
	zassert_true(n > 0, "pack");
	tx[n - 1] ^= 0x5A;
	ret = i3c_test_write(peer, tx, (size_t)n);
	zassert_ok(ret, "bad frame write: %d", ret);
	k_msleep(20);
	ret = i3c_test_read(peer, tx, I3C_TEST_FRAME_HDR + 1);
	zassert_ok(ret, "checksum response read: %d", ret);
	zassert_ok(i3c_test_frame_parse(tx, I3C_TEST_FRAME_HDR + 1, &resp), "parse");
	zassert_equal(resp.status, I3C_TEST_STS_CHECKSUM, "status %u", resp.status);

	ret = command(0x7E, NULL, 0, 0, &resp, false);
	zassert_ok(ret, "unknown opcode exchange: %d", ret);
	zassert_equal(resp.status, I3C_TEST_STS_OPCODE, "opcode status %u", resp.status);
}

ZTEST(i3c_controller, test_stale_sequence)
{
	struct i3c_test_frame resp;
	uint8_t data[] = {0x01};
	uint8_t sequence;
	int ret;

	require_peer();
	sequence = next_seq();
	ret = exchange(I3C_TEST_OP_ECHO, sequence, data, 1, 1, &resp, false);
	zassert_ok(ret, "first echo: %d", ret);
	zassert_equal(resp.status, I3C_TEST_STS_OK, "first status");

	ret = exchange(I3C_TEST_OP_ECHO, sequence, data, 1, 0, &resp, false);
	zassert_ok(ret, "replay exchange: %d", ret);
	zassert_equal(resp.status, I3C_TEST_STS_SEQUENCE, "replay status %u", resp.status);
}

static void expect_ibi(const uint8_t *data, uint8_t len)
{
	struct i3c_test_frame resp;
	int ret;

	ret = command(I3C_TEST_OP_IBI, data, len, 1, &resp, true);
	zassert_ok(ret, "IBI %u failed: %d", len, ret);
	zassert_equal(resp.status, I3C_TEST_STS_OK, "IBI status %u", resp.status);
	/* BCR[2] is read-only on this target and reset to 0, so an accepted
	 * IBI is the address only. The controller queue length is then 0.
	 */
	if ((peer_bcr & I3C_BCR_IBI_PAYLOAD_HAS_DATA_BYTE) == 0U) {
		zassert_equal(ibi_len, 0U, "IBI length %u", ibi_len);
		return;
	}
	zassert_equal(ibi_len, len, "IBI length %u", ibi_len);
	zassert_mem_equal(ibi_buf, data, len, "IBI payload");
}

ZTEST(i3c_controller, test_ibi_payloads)
{
	static const uint8_t p1[] = {0xA5};
	static const uint8_t p2[] = {0xA5, 0x01};
	static const uint8_t p3[] = {0xA5, 0x01, 0x02};
	static const uint8_t p4[] = {0xA5, 0x01, 0x02, 0x03};
	static const uint8_t p5[] = {0xA5, 0x01, 0x02, 0x03, 0x04};

	require_ibi();
	expect_ibi(p1, sizeof(p1));
	expect_ibi(p2, sizeof(p2));
	expect_ibi(p3, sizeof(p3));
	expect_ibi(p4, sizeof(p4));
	expect_ibi(p5, sizeof(p5));
}

ZTEST(i3c_controller, test_ibi_two_then_transfer)
{
	static const uint8_t first[] = {0x11, 0x22};
	static const uint8_t second[] = {0x33, 0x44, 0x55};
	uint8_t echo[] = {0x66, 0x77};

	require_ibi();
	expect_ibi(first, sizeof(first));
	expect_ibi(second, sizeof(second));
	expect_echo(echo, sizeof(echo));
	expect_ibi(first, sizeof(first));
}

ZTEST(i3c_controller, test_ibi_disabled_then_enabled)
{
	struct i3c_test_frame resp;
	uint8_t payload[] = {0xA5, 0x5A};
	int ret;

	require_ibi();
	ret = i3c_ibi_disable(peer);
	zassert_ok(ret, "IBI disable: %d", ret);
	ibi_ready = false;

	ret = command(I3C_TEST_OP_IBI, payload, sizeof(payload), 1, &resp, false);
	zassert_ok(ret, "rejected IBI exchange: %d", ret);
	zassert_equal(resp.status, I3C_TEST_STS_IBI_REJECTED, "status %u", resp.status);

	ret = enable_ibi();
	zassert_ok(ret, "IBI re-enable: %d", ret);
	expect_ibi(payload, sizeof(payload));
}

ZTEST(i3c_controller, test_ibi_after_rstdaa)
{
	static const uint8_t payload[] = {0xB1};
	int ret;

	require_ibi();
	ret = i3c_test_discover(bus);
	zassert_true(ret > 0, "rediscover %d", ret);
	refresh_peer();
	require_peer();
	ret = enable_ibi();
	zassert_ok(ret, "IBI enable after DAA: %d", ret);
	expect_ibi(payload, sizeof(payload));
}

ZTEST(i3c_controller, test_stress_and_counters)
{
	struct i3c_test_frame resp;
	uint8_t data[8];
	uint16_t echo_ok;
	uint16_t ibi_ok;
	uint16_t errors;
	int ret;

	require_peer();
	for (uint8_t i = 0; i < sizeof(data); i++) {
		data[i] = i;
	}

	for (int i = 0; i < CONFIG_I3C_TEST_STRESS_LOOPS; i++) {
		data[0] = (uint8_t)i;
		expect_echo(data, sizeof(data));
		if (ibi_ready && (i % 10) == 0) {
			uint8_t ibi_payload[] = {0xC0, (uint8_t)i};

			expect_ibi(ibi_payload, sizeof(ibi_payload));
		}
		if ((i % 25) == 0) {
			uint8_t raw[6];

			ret = i3c_test_getpid(bus, peer->dynamic_addr, raw);
			zassert_ok(ret, "interleaved GETPID: %d", ret);
		}
	}

	ret = command(I3C_TEST_OP_COUNTERS, NULL, 0, I3C_TEST_COUNTERS_LEN, &resp, false);
	zassert_ok(ret, "counters: %d", ret);
	zassert_equal(resp.status, I3C_TEST_STS_OK, "counter status");
	echo_ok = sys_get_le16(&resp.payload[2]);
	ibi_ok = sys_get_le16(&resp.payload[4]);
	errors = sys_get_le16(&resp.payload[6]);
	zassert_true(echo_ok >= CONFIG_I3C_TEST_STRESS_LOOPS, "echo count %u", echo_ok);
	zassert_equal(errors, 0, "target errors %u", errors);
	if (ibi_ready) {
		zassert_true(ibi_ok > 0U, "no IBI counted");
	}
	LOG_INF("stress echo=%u ibi=%u", echo_ok, ibi_ok);
}

ZTEST(i3c_controller, test_zz_controller_role_handoff)
{
	struct i3c_test_frame resp;
	struct i3c_test_frame req_frame = {
		.opcode = I3C_TEST_OP_HANDOFF,
		.sequence = next_seq(),
		.status = I3C_TEST_STS_OK,
		.length = 1,
		.payload = {I3C_TEST_HANDOFF_REQUEST},
	};
	struct i3c_ibi role_request = {
		.ibi_type = I3C_IBI_CONTROLLER_ROLE_REQUEST,
	};
	struct i3c_config_controller cfg = {0};
	uint8_t tx[I3C_TEST_FRAME_MAX];
	uint8_t action;
	int64_t deadline;
	int n;
	int ret;

	require_peer();
	if (!IS_ENABLED(CONFIG_I3C_TEST_HANDOFF) || (peer_caps & I3C_TEST_CAP_HANDOFF) == 0U) {
		ztest_test_skip();
	}

	action = I3C_TEST_HANDOFF_REJECT;
	ret = command(I3C_TEST_OP_HANDOFF, &action, 1, 0, &resp, false);
	zassert_ok(ret, "handoff reject: %d", ret);
	zassert_equal(resp.status, I3C_TEST_STS_OK, "handoff reject status");
	action = I3C_TEST_HANDOFF_ACCEPT;
	ret = command(I3C_TEST_OP_HANDOFF, &action, 1, 0, &resp, false);
	zassert_ok(ret, "handoff accept: %d", ret);
	zassert_equal(resp.status, I3C_TEST_STS_OK, "handoff accept status");

	n = i3c_test_frame_pack(&req_frame, tx, sizeof(tx));
	zassert_true(n > 0, "handoff frame pack: %d", n);
	ret = i3c_test_write(peer, tx, n);
	zassert_ok(ret, "handoff request write: %d", ret);

	deadline = k_uptime_get() + 2000;
	do {
		ret = i3c_config_get_controller(bus, &cfg);
		zassert_ok(ret, "role config: %d", ret);
		if (cfg.is_secondary) {
			break;
		}
		k_msleep(10);
	} while (k_uptime_get() < deadline);
	zassert_true(cfg.is_secondary, "target did not become active controller");

	ret = i3c_target_controller_handoff(bus, true);
	zassert_ok(ret, "accept handback: %d", ret);
	ret = i3c_ibi_raise(bus, &role_request);
	zassert_ok(ret, "request handback: %d", ret);
	deadline = k_uptime_get() + 2000;
	do {
		ret = i3c_config_get_controller(bus, &cfg);
		zassert_ok(ret, "handback config: %d", ret);
		if (!cfg.is_secondary) {
			break;
		}
		k_msleep(10);
	} while (k_uptime_get() < deadline);
	zassert_false(cfg.is_secondary, "controller role was not restored");
}

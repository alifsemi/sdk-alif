/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <string.h>

#include <zephyr/drivers/i3c.h>
#include <zephyr/drivers/i3c/target_device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/sys/byteorder.h>

#include "i3c_protocol.h"
#include "i3c_target_service.h"

LOG_MODULE_REGISTER(i3c_target_service, LOG_LEVEL_INF);

static const struct device *i3c_dev;
static struct i3c_target_config target_cfg;
static uint8_t rx_buf[I3C_TEST_FRAME_MAX];
static uint16_t rx_len;
static struct k_sem rx_sem;
static uint8_t stored[I3C_TEST_PAYLOAD_MAX];
static uint8_t stored_len;
static bool have_seq;
static uint8_t last_seq;
static uint8_t active_mode;
static struct i3c_test_counters stats;
static struct k_mutex stats_lock;
static atomic_t session_active;

static K_THREAD_STACK_DEFINE(server_stack, 2048);
static struct k_thread server_thread;

static void session_reset(void)
{
	memset(stored, 0, sizeof(stored));
	stored_len = 0;
	have_seq = false;
	active_mode = I3C_TEST_MODE_SDR;
	atomic_clear(&session_active);
	k_mutex_lock(&stats_lock, K_FOREVER);
	memset(&stats, 0, sizeof(stats));
	k_mutex_unlock(&stats_lock);
}

static void count_error(void)
{
	k_mutex_lock(&stats_lock, K_FOREVER);
	stats.errors++;
	k_mutex_unlock(&stats_lock);
}

static void reply(const struct i3c_test_frame *resp)
{
	uint8_t tx[I3C_TEST_FRAME_MAX];
	int n = i3c_test_frame_pack(resp, tx, sizeof(tx));
	int ret;
	uint8_t hdr_mode = 0U;

	if (n < 0) {
		count_error();
		return;
	}

	switch (active_mode) {
	case I3C_TEST_MODE_HDR_DDR:
		hdr_mode = I3C_MSG_HDR_DDR;
		break;
	case I3C_TEST_MODE_HDR_TSP:
		hdr_mode = I3C_MSG_HDR_TSP;
		break;
	case I3C_TEST_MODE_HDR_TSL:
		hdr_mode = I3C_MSG_HDR_TSL;
		break;
	default:
		break;
	}
	ret = i3c_target_tx_write(i3c_dev, tx, (uint16_t)n, hdr_mode);
	if (ret != n) {
		LOG_ERR("TX write failed: %d", ret);
		count_error();
	} else {
		k_mutex_lock(&stats_lock, K_FOREVER);
		stats.bytes_tx += (uint32_t)n;
		k_mutex_unlock(&stats_lock);
	}
}

static uint32_t target_capabilities(struct i3c_config_target *cfg)
{
	uint32_t caps = I3C_TEST_CAP_BULK;

	if ((cfg->bcr & I3C_BCR_IBI_REQUEST_CAPABLE) != 0U) {
		caps |= I3C_TEST_CAP_IBI;
	}
	if ((cfg->supported_hdr & I3C_MSG_HDR_DDR) != 0U) {
		caps |= I3C_TEST_CAP_HDR_DDR;
	}
	if ((cfg->supported_hdr & I3C_MSG_HDR_TSP) != 0U) {
		caps |= I3C_TEST_CAP_HDR_TSP;
	}
	if ((cfg->supported_hdr & I3C_MSG_HDR_TSL) != 0U) {
		caps |= I3C_TEST_CAP_HDR_TSL;
	}
	if (IS_ENABLED(CONFIG_I3C_TEST_HANDOFF) &&
	    I3C_BCR_DEVICE_ROLE(cfg->bcr) == I3C_BCR_DEVICE_ROLE_I3C_CONTROLLER_CAPABLE) {
		int ret = i3c_target_controller_handoff(i3c_dev, false);

		if (ret == 0) {
			caps |= I3C_TEST_CAP_HANDOFF;
		}
	}
	return caps;
}

static uint8_t ibi_status(int ret)
{
	switch (ret) {
	case 0:
		return I3C_TEST_STS_OK;
	case -EAGAIN:
		return I3C_TEST_STS_IBI_REJECTED;
	case -EACCES:
		return I3C_TEST_STS_IBI_REJECTED;
	case -EINVAL:
		return I3C_TEST_STS_IBI_INVALID;
	default:
		return I3C_TEST_STS_IO;
	}
}

static void handle_frame(const struct i3c_test_frame *req, int parse_ret)
{
	struct i3c_test_frame resp = {
		.opcode = req->opcode,
		.sequence = req->sequence,
		.status = I3C_TEST_STS_OK,
	};

	atomic_set(&session_active, 1);
	k_mutex_lock(&stats_lock, K_FOREVER);
	stats.commands++;
	k_mutex_unlock(&stats_lock);

	if (parse_ret == -EBADMSG) {
		resp.status = I3C_TEST_STS_CHECKSUM;
		reply(&resp);
		return;
	}
	if (parse_ret != 0) {
		count_error();
		return;
	}

	if (have_seq && req->sequence == last_seq && req->opcode != I3C_TEST_OP_RESET) {
		resp.status = I3C_TEST_STS_SEQUENCE;
		reply(&resp);
		return;
	}
	have_seq = true;
	last_seq = req->sequence;

	switch (req->opcode) {
	case I3C_TEST_OP_ECHO:
		resp.length = req->length;
		memcpy(resp.payload, req->payload, req->length);
		k_mutex_lock(&stats_lock, K_FOREVER);
		stats.echo_ok++;
		k_mutex_unlock(&stats_lock);
		break;
	case I3C_TEST_OP_BULK:
		resp.length = req->length;
		memcpy(resp.payload, req->payload, req->length);
		k_mutex_lock(&stats_lock, K_FOREVER);
		stats.bulk_frames++;
		k_mutex_unlock(&stats_lock);
		break;
	case I3C_TEST_OP_FILL:
		stored_len = req->length;
		memcpy(stored, req->payload, req->length);
		break;
	case I3C_TEST_OP_READ:
		resp.length = stored_len;
		memcpy(resp.payload, stored, stored_len);
		break;
	case I3C_TEST_OP_STATUS:
		resp.length = 1;
		resp.payload[0] = stored_len;
		break;
	case I3C_TEST_OP_IBI: {
		struct i3c_ibi ibi = {
			.ibi_type = I3C_IBI_TARGET_INTR,
			.payload = (uint8_t *)req->payload,
			.payload_len = req->length,
		};
		int ret;

		ret = i3c_ibi_raise(i3c_dev, &ibi);
		if (ret == 0) {
			k_mutex_lock(&stats_lock, K_FOREVER);
			stats.ibi_ok++;
			k_mutex_unlock(&stats_lock);
		}
		resp.status = ibi_status(ret);
		resp.length = 1;
		resp.payload[0] = resp.status;
		reply(&resp);
		return;
	}
	case I3C_TEST_OP_RESET:
		session_reset();
		break;
	case I3C_TEST_OP_COUNTERS: {
		struct i3c_test_counters copy;

		k_mutex_lock(&stats_lock, K_FOREVER);
		copy = stats;
		k_mutex_unlock(&stats_lock);
		sys_put_le16(copy.commands, &resp.payload[0]);
		sys_put_le16(copy.echo_ok, &resp.payload[2]);
		sys_put_le16(copy.ibi_ok, &resp.payload[4]);
		sys_put_le16(copy.errors, &resp.payload[6]);
		sys_put_le32(copy.bytes_rx, &resp.payload[8]);
		sys_put_le32(copy.bytes_tx, &resp.payload[12]);
		sys_put_le32(copy.bulk_frames, &resp.payload[16]);
		sys_put_le16(copy.handoffs, &resp.payload[20]);
		sys_put_le16(0, &resp.payload[22]);
		resp.length = I3C_TEST_COUNTERS_LEN;
		break;
	}
	case I3C_TEST_OP_CAPS: {
		struct i3c_config_target cfg = {0};
		uint32_t caps;
		int ret = i3c_config_get_target(i3c_dev, &cfg);

		if (ret != 0) {
			resp.status = I3C_TEST_STS_IO;
			break;
		}
		caps = target_capabilities(&cfg);
		resp.payload[0] = I3C_TEST_FRAME_VERSION;
		sys_put_le32(caps, &resp.payload[1]);
		sys_put_le16(I3C_TEST_PAYLOAD_MAX, &resp.payload[5]);
		sys_put_le16(cfg.max_read_len, &resp.payload[7]);
		sys_put_le16(cfg.max_write_len, &resp.payload[9]);
		resp.payload[11] = cfg.supported_hdr;
		resp.payload[12] = active_mode;
		resp.payload[13] = 0U;
		resp.length = I3C_TEST_CAPS_LEN;
		break;
	}
	case I3C_TEST_OP_MODE: {
		uint8_t requested;
		uint8_t previous = active_mode;

		if (req->length != 1U) {
			resp.status = I3C_TEST_STS_LENGTH;
			break;
		}
		requested = req->payload[0];
		if (requested > I3C_TEST_MODE_HDR_TSL) {
			resp.status = I3C_TEST_STS_UNSUPPORTED;
			break;
		}
		resp.length = 1;
		resp.payload[0] = requested;
		reply(&resp);
		active_mode = requested;
		LOG_INF("Transfer mode %u -> %u", previous, requested);
		return;
	}
	case I3C_TEST_OP_HANDOFF: {
		struct i3c_ibi request = {
			.ibi_type = I3C_IBI_CONTROLLER_ROLE_REQUEST,
		};
		int ret;

		if (req->length != 1U) {
			resp.status = I3C_TEST_STS_LENGTH;
			break;
		}
		switch (req->payload[0]) {
		case I3C_TEST_HANDOFF_QUERY: {
			struct i3c_config_controller cfg = {0};

			ret = i3c_config_get_controller(i3c_dev, &cfg);
			resp.length = 1;
			resp.payload[0] = (ret == 0 && !cfg.is_secondary) ? 1U : 0U;
			break;
		}
		case I3C_TEST_HANDOFF_ACCEPT:
			ret = i3c_target_controller_handoff(i3c_dev, true);
			resp.status = (ret == 0) ? I3C_TEST_STS_OK : I3C_TEST_STS_UNSUPPORTED;
			break;
		case I3C_TEST_HANDOFF_REJECT:
			ret = i3c_target_controller_handoff(i3c_dev, false);
			resp.status = (ret == 0) ? I3C_TEST_STS_OK : I3C_TEST_STS_UNSUPPORTED;
			break;
		case I3C_TEST_HANDOFF_REQUEST:
			/*
			 * Queue the acknowledgment while this device is still a
			 * target. The role request may immediately transfer bus
			 * ownership, after which target TX is no longer available.
			 */
			resp.status = I3C_TEST_STS_OK;
			reply(&resp);
			ret = i3c_target_controller_handoff(i3c_dev, true);
			if (ret == 0) {
				ret = i3c_ibi_raise(i3c_dev, &request);
			}
			if (ret == 0) {
				k_mutex_lock(&stats_lock, K_FOREVER);
				stats.handoffs++;
				k_mutex_unlock(&stats_lock);
			} else {
				LOG_WRN("Controller-role request failed: %d", ret);
			}
			return;
		default:
			resp.status = I3C_TEST_STS_UNSUPPORTED;
			break;
		}
		break;
	}
	case I3C_TEST_OP_DONE:
		reply(&resp);
		session_reset();
		LOG_INF("Controller session complete; service remains available");
		return;
	default:
		resp.status = I3C_TEST_STS_OPCODE;
		break;
	}

	reply(&resp);
}

static void server_thread_fn(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	for (;;) {
		uint8_t local[I3C_TEST_FRAME_MAX];
		uint16_t n;
		unsigned int key;
		struct i3c_test_frame frame = {0};
		int ret;

		k_sem_take(&rx_sem, K_FOREVER);
		key = irq_lock();
		n = MIN(rx_len, sizeof(local));
		memcpy(local, rx_buf, n);
		rx_len = 0;
		irq_unlock(key);
		if (n == 0U) {
			continue;
		}

		ret = i3c_test_frame_parse(local, n, &frame);
		k_mutex_lock(&stats_lock, K_FOREVER);
		stats.bytes_rx += n;
		k_mutex_unlock(&stats_lock);
		handle_frame(&frame, ret);
	}
}

static int write_requested_cb(struct i3c_target_config *config)
{
	unsigned int key;

	ARG_UNUSED(config);
	key = irq_lock();
	rx_len = 0;
	irq_unlock(key);
	return 0;
}

static int write_received_cb(struct i3c_target_config *config, uint8_t val)
{
	unsigned int key;
	int ret = 0;

	ARG_UNUSED(config);
	key = irq_lock();
	if (rx_len < sizeof(rx_buf)) {
		rx_buf[rx_len++] = val;
	} else {
		ret = -ENOMEM;
	}
	irq_unlock(key);
	return ret;
}

static int read_requested_cb(struct i3c_target_config *config, uint8_t *val)
{
	ARG_UNUSED(config);
	if (val != NULL) {
		*val = 0;
	}
	return 0;
}

static int read_processed_cb(struct i3c_target_config *config, uint8_t *val)
{
	ARG_UNUSED(config);
	if (val != NULL) {
		*val = 0;
	}
	return 0;
}

static int stop_cb(struct i3c_target_config *config)
{
	ARG_UNUSED(config);
	k_sem_give(&rx_sem);
	return 0;
}

static const struct i3c_target_callbacks target_cbs = {
	.write_requested_cb = write_requested_cb,
	.write_received_cb = write_received_cb,
	.read_requested_cb = read_requested_cb,
	.read_processed_cb = read_processed_cb,
	.stop_cb = stop_cb,
};

int i3c_target_service_start(void)
{
	struct i3c_config_target hw = {
		.enabled = true,
		.static_addr = I3C_TEST_STATIC_ADDR,
		.pid = I3C_TEST_PID,
		.pid_random = false,
		.bcr = I3C_TEST_BCR,
		.dcr = I3C_TEST_DCR,
		.max_read_len = I3C_TEST_MAX_LEN,
		.max_write_len = I3C_TEST_MAX_LEN,
	};
	struct i3c_config_target got = {0};
	int ret;

	i3c_dev = i3c_test_bus();
	if (i3c_dev == NULL || !device_is_ready(i3c_dev)) {
		LOG_ERR("zephyr,i3c is not ready");
		return -ENODEV;
	}

	k_mutex_init(&stats_lock);
	k_sem_init(&rx_sem, 0, 4);
	session_reset();
	ret = i3c_configure_target(i3c_dev, &hw);
	if (ret != 0) {
		LOG_ERR("Configure target failed: %d", ret);
		return ret;
	}

	target_cfg.address = I3C_TEST_STATIC_ADDR;
	target_cfg.callbacks = &target_cbs;
	ret = i3c_target_register(i3c_dev, &target_cfg);
	if (ret != 0) {
		LOG_ERR("Register target failed: %d", ret);
		return ret;
	}

	ret = i3c_config_get_target(i3c_dev, &got);
	if (ret != 0 || got.pid != I3C_TEST_PID || got.dcr != I3C_TEST_DCR) {
		LOG_ERR("Target identity readback failed: %d", ret);
		return ret != 0 ? ret : -EIO;
	}

	k_thread_create(&server_thread, server_stack, K_THREAD_STACK_SIZEOF(server_stack),
			server_thread_fn, NULL, NULL, NULL, K_PRIO_PREEMPT(7), 0, K_NO_WAIT);
	k_thread_name_set(&server_thread, "i3c_target");
	LOG_INF("Target service ready PID 0x%012llx BCR 0x%02x MRL/MWL %u",
		(unsigned long long)got.pid, got.bcr, got.max_read_len);
	return 0;
}

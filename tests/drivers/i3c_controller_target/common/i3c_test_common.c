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

#include <zephyr/drivers/i3c/ccc.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/byteorder.h>

LOG_MODULE_REGISTER(i3c_test_common, LOG_LEVEL_INF);

const struct device *i3c_test_bus(void)
{
#if DT_HAS_CHOSEN(zephyr_i3c)
	return DEVICE_DT_GET(DT_CHOSEN(zephyr_i3c));
#else
	return NULL;
#endif
}

static uint8_t frame_checksum(const uint8_t *buf, size_t len)
{
	uint8_t sum = 0;

	for (size_t i = 0; i < len; i++) {
		sum ^= buf[i];
	}

	return sum;
}

int i3c_test_frame_pack(const struct i3c_test_frame *frame, uint8_t *buf, size_t buf_len)
{
	size_t total;

	if (frame == NULL || buf == NULL || frame->length > I3C_TEST_PAYLOAD_MAX) {
		return -EINVAL;
	}

	total = I3C_TEST_FRAME_HDR + frame->length + 1U;
	if (buf_len < total) {
		return -ENOSPC;
	}

	buf[0] = I3C_TEST_FRAME_MAGIC;
	buf[1] = I3C_TEST_FRAME_VERSION;
	buf[2] = frame->opcode;
	buf[3] = frame->sequence;
	buf[4] = frame->status;
	buf[5] = frame->length;
	if (frame->length > 0U) {
		memcpy(&buf[I3C_TEST_FRAME_HDR], frame->payload, frame->length);
	}
	buf[total - 1U] = frame_checksum(buf, total - 1U);

	return (int)total;
}

int i3c_test_frame_parse(const uint8_t *buf, size_t len, struct i3c_test_frame *frame)
{
	size_t total;

	if (buf == NULL || frame == NULL) {
		return -EINVAL;
	}

	memset(frame, 0, sizeof(*frame));
	if (len < I3C_TEST_FRAME_HDR + 1U) {
		return -EINVAL;
	}
	if (buf[0] != I3C_TEST_FRAME_MAGIC || buf[1] != I3C_TEST_FRAME_VERSION) {
		return -EINVAL;
	}

	frame->opcode = buf[2];
	frame->sequence = buf[3];
	frame->status = buf[4];
	frame->length = buf[5];
	if (frame->length > I3C_TEST_PAYLOAD_MAX) {
		return -EMSGSIZE;
	}

	total = I3C_TEST_FRAME_HDR + frame->length + 1U;
	if (len != total) {
		return -EINVAL;
	}
	if (frame->length > 0U) {
		memcpy(frame->payload, &buf[I3C_TEST_FRAME_HDR], frame->length);
	}
	if (buf[total - 1U] != frame_checksum(buf, total - 1U)) {
		return -EBADMSG;
	}

	return 0;
}

int i3c_test_assigned_count(const struct device *ctrl)
{
	struct i3c_device_desc *desc = NULL;
	int count = 0;

	if (ctrl == NULL) {
		return -EINVAL;
	}

	I3C_BUS_FOR_EACH_I3CDEV(ctrl, desc) {
		if (desc->dynamic_addr != 0U) {
			count++;
		}
	}

	return count;
}

struct i3c_device_desc *i3c_test_find_peer(const struct device *ctrl)
{
	struct i3c_device_desc *desc = NULL;

	if (ctrl == NULL) {
		return NULL;
	}

	I3C_BUS_FOR_EACH_I3CDEV(ctrl, desc) {
		if (desc->pid == I3C_TEST_PID && desc->dynamic_addr != 0U) {
			return desc;
		}
	}

	return NULL;
}

int i3c_test_discover(const struct device *ctrl)
{
	int ret;

	if (ctrl == NULL) {
		return -EINVAL;
	}

	ret = i3c_ccc_do_rstdaa_all(ctrl);
	if (ret < 0) {
		LOG_ERR("RSTDAA failed: %d", ret);
		return ret;
	}

	k_msleep(50);

	ret = i3c_do_daa(ctrl);
	if (ret < 0 && ret != -ENODEV) {
		LOG_ERR("DAA failed: %d", ret);
		return ret;
	}

	k_msleep(20);
	return i3c_test_assigned_count(ctrl);
}

int i3c_test_write(struct i3c_device_desc *desc, const uint8_t *buf, size_t len)
{
	struct i3c_msg msg;

	if (buf == NULL || len == 0U) {
		return -EINVAL;
	}
	if (desc == NULL || desc->dynamic_addr == 0U) {
		return -ENODEV;
	}

	memset(&msg, 0, sizeof(msg));
	msg.buf = (uint8_t *)buf;
	msg.len = len;
	msg.flags = I3C_MSG_WRITE | I3C_MSG_STOP;

	return i3c_transfer(desc, &msg, 1);
}

int i3c_test_read(struct i3c_device_desc *desc, uint8_t *buf, size_t len)
{
	struct i3c_msg msg;

	if (buf == NULL || len == 0U) {
		return -EINVAL;
	}
	if (desc == NULL || desc->dynamic_addr == 0U) {
		return -ENODEV;
	}

	memset(&msg, 0, sizeof(msg));
	msg.buf = buf;
	msg.len = len;
	msg.flags = I3C_MSG_READ | I3C_MSG_STOP;

	return i3c_transfer(desc, &msg, 1);
}

int i3c_test_write_mode(struct i3c_device_desc *desc, const uint8_t *buf, size_t len,
			uint8_t hdr_mode)
{
	struct i3c_msg msg = {0};

	if (buf == NULL || len == 0U) {
		return -EINVAL;
	}
	if (desc == NULL || desc->dynamic_addr == 0U) {
		return -ENODEV;
	}
	msg.buf = (uint8_t *)buf;
	msg.len = len;
	msg.flags = I3C_MSG_WRITE | I3C_MSG_STOP;
	msg.hdr_mode = hdr_mode;
	if (hdr_mode != 0U) {
		msg.flags |= I3C_MSG_HDR;
	}
	return i3c_transfer(desc, &msg, 1);
}

int i3c_test_read_mode(struct i3c_device_desc *desc, uint8_t *buf, size_t len, uint8_t hdr_mode)
{
	struct i3c_msg msg = {0};

	if (buf == NULL || len == 0U) {
		return -EINVAL;
	}
	if (desc == NULL || desc->dynamic_addr == 0U) {
		return -ENODEV;
	}
	msg.buf = buf;
	msg.len = len;
	msg.flags = I3C_MSG_READ | I3C_MSG_STOP;
	msg.hdr_mode = hdr_mode;
	if (hdr_mode != 0U) {
		msg.flags |= I3C_MSG_HDR;
	}
	return i3c_transfer(desc, &msg, 1);
}

static int ccc_read(const struct device *ctrl, uint8_t id, uint8_t da, uint8_t *data, size_t len)
{
	struct i3c_ccc_target_payload target = {
		.addr = da,
		.rnw = 1,
		.data = data,
		.data_len = len,
	};
	struct i3c_ccc_payload ccc = {
		.ccc.id = id,
		.targets.payloads = &target,
		.targets.num_targets = 1,
	};

	return i3c_do_ccc(ctrl, &ccc);
}

int i3c_test_getpid(const struct device *ctrl, uint8_t da, uint8_t pid[6])
{
	if (pid == NULL) {
		return -EINVAL;
	}

	return ccc_read(ctrl, I3C_CCC_GETPID, da, pid, 6);
}

int i3c_test_getbcr(const struct device *ctrl, uint8_t da, uint8_t *bcr)
{
	if (bcr == NULL) {
		return -EINVAL;
	}

	return ccc_read(ctrl, I3C_CCC_GETBCR, da, bcr, 1);
}

int i3c_test_getdcr(const struct device *ctrl, uint8_t da, uint8_t *dcr)
{
	if (dcr == NULL) {
		return -EINVAL;
	}

	return ccc_read(ctrl, I3C_CCC_GETDCR, da, dcr, 1);
}

int i3c_test_getstatus(const struct device *ctrl, uint8_t da, uint16_t *status)
{
	uint8_t data[2];
	int ret;

	if (status == NULL) {
		return -EINVAL;
	}

	ret = ccc_read(ctrl, I3C_CCC_GETSTATUS, da, data, sizeof(data));
	if (ret >= 0) {
		*status = sys_get_be16(data);
	}

	return ret;
}

int i3c_test_getmrl(const struct device *ctrl, uint8_t da, uint16_t *mrl)
{
	uint8_t data[2];
	int ret;

	if (mrl == NULL) {
		return -EINVAL;
	}

	ret = ccc_read(ctrl, I3C_CCC_GETMRL, da, data, sizeof(data));
	if (ret >= 0) {
		*mrl = sys_get_be16(data);
	}

	return ret;
}

int i3c_test_getmwl(const struct device *ctrl, uint8_t da, uint16_t *mwl)
{
	uint8_t data[2];
	int ret;

	if (mwl == NULL) {
		return -EINVAL;
	}

	ret = ccc_read(ctrl, I3C_CCC_GETMWL, da, data, sizeof(data));
	if (ret >= 0) {
		*mwl = sys_get_be16(data);
	}

	return ret;
}

int i3c_test_setmwl(const struct device *ctrl, uint8_t da, uint16_t mwl)
{
	uint8_t data[2];
	struct i3c_ccc_target_payload target = {
		.addr = da,
		.rnw = 0,
		.data = data,
		.data_len = sizeof(data),
	};
	struct i3c_ccc_payload ccc = {
		.ccc.id = I3C_CCC_SETMWL(false),
		.targets.payloads = &target,
		.targets.num_targets = 1,
	};

	sys_put_be16(mwl, data);
	return i3c_do_ccc(ctrl, &ccc);
}

static int events_direct(const struct device *ctrl, uint8_t id, uint8_t da, uint8_t events)
{
	struct i3c_ccc_target_payload target = {
		.addr = da,
		.rnw = 0,
		.data = &events,
		.data_len = 1,
	};
	struct i3c_ccc_payload ccc = {
		.ccc.id = id,
		.targets.payloads = &target,
		.targets.num_targets = 1,
	};

	return i3c_do_ccc(ctrl, &ccc);
}

int i3c_test_enec(const struct device *ctrl, uint8_t da, uint8_t events)
{
	return events_direct(ctrl, I3C_CCC_ENEC(false), da, events);
}

int i3c_test_disec(const struct device *ctrl, uint8_t da, uint8_t events)
{
	return events_direct(ctrl, I3C_CCC_DISEC(false), da, events);
}

static int events_broadcast(const struct device *ctrl, uint8_t id, uint8_t events)
{
	struct i3c_ccc_payload ccc = {
		.ccc.id = id,
		.ccc.data = &events,
		.ccc.data_len = 1,
	};

	return i3c_do_ccc(ctrl, &ccc);
}

int i3c_test_enec_broadcast(const struct device *ctrl, uint8_t events)
{
	return events_broadcast(ctrl, I3C_CCC_ENEC(true), events);
}

int i3c_test_disec_broadcast(const struct device *ctrl, uint8_t events)
{
	return events_broadcast(ctrl, I3C_CCC_DISEC(true), events);
}

int i3c_test_getcaps(const struct device *ctrl, uint8_t da, uint8_t *caps, size_t len)
{
	if (caps == NULL || len == 0U) {
		return -EINVAL;
	}

	return ccc_read(ctrl, I3C_CCC_GETCAPS, da, caps, len);
}

int i3c_test_rstact(const struct device *ctrl, uint8_t da, uint8_t action)
{
	struct i3c_ccc_target_payload target = {
		.addr = da,
		.rnw = 0,
		.data = &action,
		.data_len = 1,
	};
	struct i3c_ccc_payload ccc = {
		.ccc.id = I3C_CCC_RSTACT(false),
		.targets.payloads = &target,
		.targets.num_targets = 1,
	};

	return i3c_do_ccc(ctrl, &ccc);
}

int i3c_test_rstdaa(const struct device *ctrl)
{
	return i3c_ccc_do_rstdaa_all(ctrl);
}

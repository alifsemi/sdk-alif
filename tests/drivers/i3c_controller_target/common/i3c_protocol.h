/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef I3C_PROTOCOL_H
#define I3C_PROTOCOL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/device.h>
#include <zephyr/drivers/i3c.h>

#if defined(CONFIG_I3C_TEST_TARGET)
#define I3C_TEST_IS_TARGET 1
#elif defined(CONFIG_I3C_TEST_CONTROLLER)
#define I3C_TEST_IS_TARGET 0
#else
#error "Select CONFIG_I3C_TEST_CONTROLLER or CONFIG_I3C_TEST_TARGET"
#endif

/* 15-bit manufacturer ID 0x0001, fixed part ID 0x00000001. */
#define I3C_TEST_MFG_ID       0x0001U
#define I3C_TEST_PART_ID      0x00000001U
#define I3C_TEST_PID          (((uint64_t)I3C_TEST_MFG_ID << 33) | I3C_TEST_PART_ID)
#define I3C_TEST_STATIC_ADDR  0x50U
#define I3C_TEST_DYNAMIC_ADDR 0x09U
#define I3C_TEST_DCR          0x00U
#define I3C_TEST_BCR                                                                               \
	(I3C_BCR_IBI_REQUEST_CAPABLE | I3C_BCR_IBI_PAYLOAD_HAS_DATA_BYTE |                         \
	 FIELD_PREP(I3C_BCR_DEVICE_ROLE_MASK, I3C_BCR_DEVICE_ROLE_I3C_CONTROLLER_CAPABLE))
/* This DesignWare target keeps an 8-bit MRL/MWL. 256 is stored as 255. */
#define I3C_TEST_MAX_LEN 255U

#define I3C_TEST_FRAME_MAGIC   0xA5U
#define I3C_TEST_FRAME_VERSION 0x02U
/* 255-byte MRL/MWL minus the six-byte header and checksum. */
#define I3C_TEST_PAYLOAD_MAX   248U
#define I3C_TEST_FRAME_HDR     6U
#define I3C_TEST_FRAME_MAX     (I3C_TEST_FRAME_HDR + I3C_TEST_PAYLOAD_MAX + 1U)

#define I3C_TEST_CAP_IBI     BIT(0)
#define I3C_TEST_CAP_HDR_DDR BIT(1)
#define I3C_TEST_CAP_HDR_TSP BIT(2)
#define I3C_TEST_CAP_HDR_TSL BIT(3)
#define I3C_TEST_CAP_HANDOFF BIT(4)
#define I3C_TEST_CAP_BULK    BIT(5)

#define I3C_TEST_MODE_SDR     0U
#define I3C_TEST_MODE_HDR_DDR 1U
#define I3C_TEST_MODE_HDR_TSP 2U
#define I3C_TEST_MODE_HDR_TSL 3U

#define I3C_TEST_HANDOFF_QUERY   0U
#define I3C_TEST_HANDOFF_ACCEPT  1U
#define I3C_TEST_HANDOFF_REJECT  2U
#define I3C_TEST_HANDOFF_REQUEST 3U

enum i3c_test_opcode {
	I3C_TEST_OP_ECHO = 1,
	I3C_TEST_OP_FILL = 2,
	I3C_TEST_OP_READ = 3,
	I3C_TEST_OP_STATUS = 4,
	I3C_TEST_OP_IBI = 5,
	I3C_TEST_OP_RESET = 6,
	I3C_TEST_OP_COUNTERS = 7,
	I3C_TEST_OP_DONE = 8,
	I3C_TEST_OP_CAPS = 9,
	I3C_TEST_OP_BULK = 10,
	I3C_TEST_OP_MODE = 11,
	I3C_TEST_OP_HANDOFF = 12,
};

enum i3c_test_status {
	I3C_TEST_STS_OK = 0,
	I3C_TEST_STS_CHECKSUM = 1,
	I3C_TEST_STS_OPCODE = 2,
	I3C_TEST_STS_LENGTH = 3,
	I3C_TEST_STS_SEQUENCE = 4,
	I3C_TEST_STS_IBI_REJECTED = 5,
	I3C_TEST_STS_IBI_NO_ADDR = 6,
	I3C_TEST_STS_IBI_INVALID = 7,
	I3C_TEST_STS_IO = 8,
	I3C_TEST_STS_UNSUPPORTED = 9,
};

struct i3c_test_frame {
	uint8_t opcode;
	uint8_t sequence;
	uint8_t status;
	uint8_t length;
	uint8_t payload[I3C_TEST_PAYLOAD_MAX];
};

struct i3c_test_counters {
	uint16_t commands;
	uint16_t echo_ok;
	uint16_t ibi_ok;
	uint16_t errors;
	uint32_t bytes_rx;
	uint32_t bytes_tx;
	uint32_t bulk_frames;
	uint16_t handoffs;
	uint16_t reserved;
};

#define I3C_TEST_COUNTERS_LEN 24U
#define I3C_TEST_CAPS_LEN     14U

const struct device *i3c_test_bus(void);

int i3c_test_frame_pack(const struct i3c_test_frame *frame, uint8_t *buf, size_t buf_len);

int i3c_test_frame_parse(const uint8_t *buf, size_t len, struct i3c_test_frame *frame);

int i3c_test_discover(const struct device *ctrl);

struct i3c_device_desc *i3c_test_find_peer(const struct device *ctrl);

int i3c_test_assigned_count(const struct device *ctrl);

int i3c_test_write(struct i3c_device_desc *desc, const uint8_t *buf, size_t len);

int i3c_test_read(struct i3c_device_desc *desc, uint8_t *buf, size_t len);

int i3c_test_write_mode(struct i3c_device_desc *desc, const uint8_t *buf, size_t len,
			uint8_t hdr_mode);

int i3c_test_read_mode(struct i3c_device_desc *desc, uint8_t *buf, size_t len, uint8_t hdr_mode);

int i3c_test_getpid(const struct device *ctrl, uint8_t da, uint8_t pid[6]);

int i3c_test_getbcr(const struct device *ctrl, uint8_t da, uint8_t *bcr);

int i3c_test_getdcr(const struct device *ctrl, uint8_t da, uint8_t *dcr);

int i3c_test_getstatus(const struct device *ctrl, uint8_t da, uint16_t *status);

int i3c_test_getmrl(const struct device *ctrl, uint8_t da, uint16_t *mrl);

int i3c_test_getmwl(const struct device *ctrl, uint8_t da, uint16_t *mwl);

int i3c_test_setmwl(const struct device *ctrl, uint8_t da, uint16_t mwl);

int i3c_test_enec(const struct device *ctrl, uint8_t da, uint8_t events);

int i3c_test_disec(const struct device *ctrl, uint8_t da, uint8_t events);

int i3c_test_enec_broadcast(const struct device *ctrl, uint8_t events);

int i3c_test_disec_broadcast(const struct device *ctrl, uint8_t events);

int i3c_test_getcaps(const struct device *ctrl, uint8_t da, uint8_t *caps, size_t len);

int i3c_test_rstact(const struct device *ctrl, uint8_t da, uint8_t action);

int i3c_test_rstdaa(const struct device *ctrl);

#endif

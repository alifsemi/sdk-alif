/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 * Cross-core wire contract for the MHU doorbell sample.
 *
 * The doorbell carries a 32-bit value (a shared-SRAM address); the data it
 * points at is a struct shared_msg. Every participant -- M55-HE, M55-HP and
 * the A32/Linux side -- MUST agree on this layout and on the magic values,
 * so it lives in one shared header rather than being duplicated per test
 * case. The Linux test application mirrors this exact layout.
 *
 * Shared memory layout (struct shared_msg):
 *   [0x00] magic      - identifies the writer (see MAGIC_* below)
 *   [0x04] msg_id     - incrementing message counter
 *   [0x08] data_len   - number of payload bytes (max MAX_PAYLOAD)
 *   [0x0C] data[240]  - payload
 *   [0xFC] checksum   - sum of payload bytes
 */

#ifndef MHU_DOORBELL_MSG_H
#define MHU_DOORBELL_MSG_H

#include <stdint.h>

#define MAX_PAYLOAD 240

/* Magic values identifying which core wrote the block. */
#define MAGIC_HE  0xCAFE0E00   /* Written by M55-HE  */
#define MAGIC_HP  0xCAFE0F00   /* Written by M55-HP  */
#define MAGIC_A32 0xA32F055E   /* Written by A32     */
#define MAGIC_M55 0xF055EA32   /* Written by M55 (A32 protocol) */

struct shared_msg {
	uint32_t magic;
	uint32_t msg_id;
	uint32_t data_len;
	uint8_t  data[MAX_PAYLOAD];
	uint32_t checksum;
};

#endif /* MHU_DOORBELL_MSG_H */

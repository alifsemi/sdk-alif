/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef UNALIGNED_GET
struct __attribute__((packed, aligned(1))) T_UINT32_READ {
	uint32_t v;
};
#define UNALIGNED_GET(addr) (((const struct T_UINT32_READ *)(const void *)(addr))->v)
#endif

#include "mac154app.h"
#include "mac154_err.h"

#include "ahi_msg_lib.h"

/*AHI message definitions*/
#define TL_HEADER_LEN      9
#define TL_AHI_CMD_LEN     80
#define TL_AHI_PAYLOAD_LEN 127
#define TX_BUFFER_LEN      (TL_AHI_PAYLOAD_LEN + TL_HEADER_LEN + TL_AHI_CMD_LEN)
#define AHI_KE_MSG_TYPE    0x10

/*AHI header handling*/
#define MSG_TYPE(p_msg)     (p_msg[0])
#define MSG_LENGTH(p_msg)   ((p_msg[8] << 8) + p_msg[7])
#define MSG_COMMAND(p_msg)  ((p_msg[2] << 8) + p_msg[1])
#define MSG_SRC_TASK(p_msg) ((p_msg[6] << 8) + p_msg[5])
#define MSG_DST_TASK(p_msg) ((p_msg[4] << 8) + p_msg[3])

static enum alif_mac154_status_code alif_ahi_msg_status_convert(uint16_t status)
{
	if (status == MAC154_ERR_NO_ERROR) {
		return ALIF_MAC154_STATUS_OK;
	} else if (status == MAC154_ERR_NO_ANSWER) {
		return ALIF_MAC154_STATUS_NO_ACK;
	} else if (status == MAC154_ERR_RADIO_CHANNEL_IN_USE) {
		return ALIF_MAC154_STATUS_CHANNEL_ACCESS_FAILURE;
	} else if (status == MAC154_ERR_HARDWARE_FAILURE) {
		return ALIF_MAC154_STATUS_HW_FAILED;
	} else if (status == MAC154_ERR_RADIO_ISSUE) {
		return ALIF_MAC154_STATUS_HW_FAILED;
	} else if (status == MAC154_ERR_UNKNOWN_COMMAND) {
		return ALIF_MAC154_STATUS_SW_FAILED;
	} else if (status == MAC154_ERR_SFTWR_FAILURE_TX || status == MAC154_ERR_SFTWR_FAILURE_RX ||
		   status == MAC154_ERR_SFTWR_FAILURE_ED) {
		return ALIF_MAC154_STATUS_INVALID_STATE;
	} else if (status == MAC154_ERR_UNKNOWN_COMMAND) {
		return ALIF_MAC154_STATUS_COMM_FAILURE;
	}
	return ALIF_MAC154_STATUS_FAILED;
}

static void *alif_ahi_msg_header_validate(struct msg_buf *p_msg, uint16_t cmd, int msg_size)
{
	if (!p_msg || p_msg->msg_len < TL_HEADER_LEN + msg_size) {
		return NULL;
	}
	if (MSG_TYPE(p_msg->msg) != AHI_KE_MSG_TYPE) {
		return NULL;
	}
	if (MSG_COMMAND(p_msg->msg) != cmd || MSG_LENGTH(p_msg->msg) < msg_size) {
		return NULL;
	}
	return &p_msg->msg[TL_HEADER_LEN];
}

static void *alif_ahi_msg_header_write(struct msg_buf *p_msg, uint16_t cmd_length,
				       uint16_t data_length)
{
	p_msg->msg[0] = AHI_KE_MSG_TYPE;

	/* Command ID */
	p_msg->msg[1] = (MAC154APP_CMD & 0xFF);
	p_msg->msg[2] = (MAC154APP_CMD & 0xFF00) >> 8;

	/* Destination task */
	p_msg->msg[3] = (TASK_ID_MAC154APP & 0xFF);
	p_msg->msg[4] = (TASK_ID_MAC154APP & 0xFF00) >> 8;

	/* Source task (for routing response)*/
	p_msg->msg[5] = (TASK_ID_AHI & 0xFF);
	p_msg->msg[6] = (TASK_ID_AHI & 0xFF00) >> 8;

	/* Length of following data */
	p_msg->msg[7] = ((cmd_length + data_length) & 0xFF);
	p_msg->msg[8] = ((cmd_length + data_length) & 0xFF00) >> 8;
	p_msg->msg_len = TL_HEADER_LEN;

	/*update the structure length*/
	p_msg->msg_len += cmd_length + data_length;

	return &p_msg->msg[9];
}

enum alif_mac154_status_code alif_ahi_msg_tx_start_resp_1_1_0(struct msg_buf *p_msg, uint8_t *p_ctx,
							      int8_t *p_rssi, uint64_t *p_timestamp,
							      uint8_t *p_ack, uint8_t *p_ack_len)
{
	mac154app_tx_single_cmp_evt_t *p_cmd_resp;

	p_cmd_resp = alif_ahi_msg_header_validate(p_msg, MAC154APP_CMP_EVT,
						  sizeof(mac154app_tx_single_cmp_evt_t));

	if (!p_cmd_resp) {
		return ALIF_MAC154_STATUS_COMM_FAILURE;
	}
	if (p_ctx) {
		*p_ctx = p_cmd_resp->dummy;
	}
	if (p_cmd_resp->cmd_code != MAC154APP_TX_SINGLE) {
		return ALIF_MAC154_STATUS_INVALID_MESSAGE;
	}
	if (p_cmd_resp->status != MAC154_ERR_NO_ERROR) {
		return alif_ahi_msg_status_convert(p_cmd_resp->status);
	}
	if (p_rssi) {
		*p_rssi = p_cmd_resp->ack_rssi;
	}
	if (p_timestamp) {
		*p_timestamp = ((uint64_t)UNALIGNED_GET(&p_cmd_resp->ack_timestamp_h) << 32) +
			       UNALIGNED_GET(&p_cmd_resp->ack_timestamp_l);
	}
	if (p_ack_len) {
		*p_ack_len = p_cmd_resp->length;
	}
	if (p_ack) {
		memcpy(p_ack, p_cmd_resp->ack_msg_begin, p_cmd_resp->length);
	}

	return ALIF_MAC154_STATUS_OK;
}

enum alif_mac154_status_code alif_ahi_msg_mem_dbg_resp(struct msg_buf *p_msg, uint8_t *p_ctx,
						       uint32_t *p_value)
{
	mac154app_dbg_rw_mem_cmp_evt_t *p_cmd_resp;

	p_cmd_resp = alif_ahi_msg_header_validate(p_msg, MAC154APP_CMP_EVT,
						  sizeof(mac154app_dbg_rw_mem_cmp_evt_t));

	if (!p_cmd_resp) {
		return ALIF_MAC154_STATUS_COMM_FAILURE;
	}
	if (p_ctx) {
		*p_ctx = p_cmd_resp->dummy;
	}
	if (p_cmd_resp->status != MAC154_ERR_NO_ERROR) {
		return alif_ahi_msg_status_convert(p_cmd_resp->status);
	}
	if (p_value) {
		*p_value = p_cmd_resp->data;
	}
	return ALIF_MAC154_STATUS_OK;
}

enum alif_mac154_status_code alif_ahi_msg_mem_reg_resp(struct msg_buf *p_msg, uint8_t *p_ctx,
						       uint32_t *p_value)
{
	mac154app_dbg_rw_reg_cmp_evt_t *p_cmd_resp;

	p_cmd_resp = alif_ahi_msg_header_validate(p_msg, MAC154APP_CMP_EVT,
						  sizeof(mac154app_dbg_rw_reg_cmp_evt_t));

	if (!p_cmd_resp) {
		return ALIF_MAC154_STATUS_COMM_FAILURE;
	}
	if (p_ctx) {
		*p_ctx = p_cmd_resp->dummy;
	}
	if (p_cmd_resp->status != MAC154_ERR_NO_ERROR) {
		return alif_ahi_msg_status_convert(p_cmd_resp->status);
	}
	if (p_value) {
		*p_value = p_cmd_resp->data;
	}
	return ALIF_MAC154_STATUS_OK;
}

void alif_ahi_msg_dbg_mem(struct msg_buf *p_msg, uint16_t ctx, uint8_t write, uint32_t address,
			  uint32_t value)
{
	struct mac154app_dbg_rw_mem_cmd *p_cmd;

	p_msg->rsp_event = MAC154APP_CMP_EVT;
	p_msg->rsp_msg = MAC154APP_DBG_RW_MEM;

	p_cmd = alif_ahi_msg_header_write(p_msg, sizeof(struct mac154app_dbg_rw_mem_cmd), 0);

	p_cmd->cmd_code = MAC154APP_DBG_RW_MEM;
	p_cmd->dummy = ctx;
	p_cmd->write = write;
	p_cmd->addr = address;
	p_cmd->data = value;
}

void alif_ahi_msg_dbg_reg(struct msg_buf *p_msg, uint16_t ctx, uint8_t write, uint32_t address,
			  uint32_t value)
{
	struct mac154app_dbg_rw_reg_cmd *p_cmd;

	p_msg->rsp_event = MAC154APP_CMP_EVT;
	p_msg->rsp_msg = MAC154APP_DBG_RW_REG;

	p_cmd = alif_ahi_msg_header_write(p_msg, sizeof(struct mac154app_dbg_rw_reg_cmd), 0);

	p_cmd->cmd_code = MAC154APP_DBG_RW_REG;
	p_cmd->dummy = ctx;
	p_cmd->write = write;
	p_cmd->addr = address;
	p_cmd->data = value;
}

void alif_ahi_msg_csl_long_id_find(struct msg_buf *p_msg, uint16_t ctx, uint8_t *p_extended_address)
{
	mac154app_id_set_cmd_t *p_cmd;

	p_msg->rsp_event = MAC154APP_CMP_EVT;
	p_msg->rsp_msg = MAC154APP_CSL_LONG_ID_FIND;

	p_cmd = alif_ahi_msg_header_write(p_msg, sizeof(mac154app_id_set_cmd_t), 0);

	p_cmd->cmd_code = MAC154APP_CSL_LONG_ID_FIND;
	p_cmd->dummy = ctx;
	p_cmd->value_h = (p_extended_address[7] << 24) + (p_extended_address[6] << 16) +
			 (p_extended_address[5] << 8) + p_extended_address[4];
	p_cmd->value_l = (p_extended_address[3] << 24) + (p_extended_address[2] << 16) +
			 (p_extended_address[1] << 8) + p_extended_address[0];
}

void alif_ahi_msg_csl_long_id_insert(struct msg_buf *p_msg, uint16_t ctx,
				     uint8_t *p_extended_address)
{
	mac154app_id_set_cmd_t *p_cmd;

	p_msg->rsp_event = MAC154APP_CMP_EVT;
	p_msg->rsp_msg = MAC154APP_CSL_LONG_ID_INSERT;

	p_cmd = alif_ahi_msg_header_write(p_msg, sizeof(mac154app_id_set_cmd_t), 0);

	p_cmd->cmd_code = MAC154APP_CSL_LONG_ID_INSERT;
	p_cmd->dummy = ctx;
	p_cmd->value_h = (p_extended_address[7] << 24) + (p_extended_address[6] << 16) +
			 (p_extended_address[5] << 8) + p_extended_address[4];
	p_cmd->value_l = (p_extended_address[3] << 24) + (p_extended_address[2] << 16) +
			 (p_extended_address[1] << 8) + p_extended_address[0];
}

void alif_ahi_msg_csl_long_id_remove(struct msg_buf *p_msg, uint16_t ctx,
				     uint8_t *p_extended_address)
{
	mac154app_id_set_cmd_t *p_cmd;

	p_msg->rsp_event = MAC154APP_CMP_EVT;
	p_msg->rsp_msg = MAC154APP_CSL_LONG_ID_REMOVE;

	p_cmd = alif_ahi_msg_header_write(p_msg, sizeof(mac154app_id_set_cmd_t), 0);

	p_cmd->cmd_code = MAC154APP_CSL_LONG_ID_REMOVE;
	p_cmd->dummy = ctx;
	p_cmd->value_h = (p_extended_address[7] << 24) + (p_extended_address[6] << 16) +
			 (p_extended_address[5] << 8) + p_extended_address[4];
	p_cmd->value_l = (p_extended_address[3] << 24) + (p_extended_address[2] << 16) +
			 (p_extended_address[1] << 8) + p_extended_address[0];
}

void alif_ahi_msg_csl_short_id_find(struct msg_buf *p_msg, uint16_t ctx, uint16_t short_id)
{
	mac154app_id_set_cmd_t *p_cmd;

	p_msg->rsp_event = MAC154APP_CMP_EVT;
	p_msg->rsp_msg = MAC154APP_CSL_SHORT_ID_FIND;

	p_cmd = alif_ahi_msg_header_write(p_msg, sizeof(mac154app_id_set_cmd_t), 0);

	p_cmd->cmd_code = MAC154APP_CSL_SHORT_ID_FIND;
	p_cmd->dummy = ctx;
	p_cmd->value_h = 0;
	p_cmd->value_l = short_id;
}

void alif_ahi_msg_csl_short_id_insert(struct msg_buf *p_msg, uint16_t ctx, uint16_t short_id)
{
	mac154app_id_set_cmd_t *p_cmd;

	p_msg->rsp_event = MAC154APP_CMP_EVT;
	p_msg->rsp_msg = MAC154APP_CSL_SHORT_ID_INSERT;

	p_cmd = alif_ahi_msg_header_write(p_msg, sizeof(mac154app_id_set_cmd_t), 0);

	p_cmd->cmd_code = MAC154APP_CSL_SHORT_ID_INSERT;
	p_cmd->dummy = ctx;
	p_cmd->value_h = 0;
	p_cmd->value_l = short_id;
}

void alif_ahi_msg_csl_short_id_remove(struct msg_buf *p_msg, uint16_t ctx, uint16_t short_id)
{
	mac154app_id_set_cmd_t *p_cmd;

	p_msg->rsp_event = MAC154APP_CMP_EVT;
	p_msg->rsp_msg = MAC154APP_CSL_LONG_ID_REMOVE;

	p_cmd = alif_ahi_msg_header_write(p_msg, sizeof(mac154app_id_set_cmd_t), 0);

	p_cmd->cmd_code = MAC154APP_CSL_LONG_ID_REMOVE;
	p_cmd->dummy = ctx;
	p_cmd->value_h = 0;
	p_cmd->value_l = short_id;
}

void alif_ahi_msg_csl_period_set(struct msg_buf *p_msg, uint16_t ctx, uint16_t period)
{
	mac154app_id_set_cmd_t *p_cmd;

	p_msg->rsp_event = MAC154APP_CMP_EVT;
	p_msg->rsp_msg = MAC154APP_CSL_PERIOD_SET;

	p_cmd = alif_ahi_msg_header_write(p_msg, sizeof(mac154app_id_set_cmd_t), 0);

	p_cmd->cmd_code = MAC154APP_CSL_PERIOD_SET;
	p_cmd->dummy = ctx;
	p_cmd->value_h = 0;
	p_cmd->value_l = period;
}

void alif_ahi_msg_csl_period_get(struct msg_buf *p_msg, uint16_t ctx)
{
	mac154app_id_get_cmd_t *p_cmd;

	p_msg->rsp_event = MAC154APP_CMP_EVT;
	p_msg->rsp_msg = MAC154APP_CSL_PERIOD_GET;

	p_cmd = alif_ahi_msg_header_write(p_msg, sizeof(mac154app_id_get_cmd_t), 0);

	p_cmd->cmd_code = MAC154APP_CSL_PERIOD_GET;
	p_cmd->dummy = ctx;
}

void alif_ahi_msg_config_header_ie_csl_reduced(struct msg_buf *p_msg, uint16_t ctx,
					       uint16_t csl_period, uint16_t csl_phase)
{
	mac154app_config_header_ie_csl_reduced_cmd_t *p_cmd;

	p_msg->rsp_event = MAC154APP_CMP_EVT;
	p_msg->rsp_msg = MAC154APP_CONF_CSL_IE_HEADER_REDUCED;

	p_cmd = alif_ahi_msg_header_write(p_msg,
					  sizeof(mac154app_config_header_ie_csl_reduced_cmd_t), 0);

	p_cmd->cmd_code = MAC154APP_CONF_CSL_IE_HEADER_REDUCED;
	p_cmd->dummy = ctx;
	p_cmd->csl_period = csl_period;
	p_cmd->csl_phase = csl_phase;
}

void alif_ahi_msg_config_header_ie_csl_full(struct msg_buf *p_msg, uint16_t ctx,
					    uint16_t csl_period, uint16_t csl_phase,
					    uint16_t csl_rendezvous_time)
{
	mac154app_config_header_ie_csl_full_cmd_t *p_cmd;

	p_msg->rsp_event = MAC154APP_CMP_EVT;
	p_msg->rsp_msg = MAC154APP_CONF_CSL_IE_HEADER_FULL;

	p_cmd = alif_ahi_msg_header_write(p_msg, sizeof(mac154app_config_header_ie_csl_full_cmd_t),
					  0);

	p_cmd->cmd_code = MAC154APP_CONF_CSL_IE_HEADER_FULL;
	p_cmd->dummy = ctx;
	p_cmd->csl_period = csl_period;
	p_cmd->csl_phase = csl_phase;
	p_cmd->csl_rendezvous_time = csl_rendezvous_time;
}

void alif_ahi_msg_config_rx_slot(struct msg_buf *p_msg, uint16_t ctx, uint32_t start,
				 uint16_t duration, uint8_t channel)
{
	mac154app_config_rx_slot_cmd_t *p_cmd;

	p_msg->rsp_event = MAC154APP_CMP_EVT;
	p_msg->rsp_msg = MAC154APP_CONF_RX_SLOT;

	p_cmd = alif_ahi_msg_header_write(p_msg, sizeof(mac154app_config_rx_slot_cmd_t), 0);

	p_cmd->cmd_code = MAC154APP_CONF_RX_SLOT;
	p_cmd->dummy = ctx;
	p_cmd->start = start;
	p_cmd->duration = duration;
	p_cmd->channel = channel;
}

void alif_ahi_msg_frame_counter_update(struct msg_buf *p_msg, uint16_t ctx, uint32_t frame_counter)
{
	mac154app_id_set_cmd_t *p_cmd;

	p_msg->rsp_event = MAC154APP_CMP_EVT;
	p_msg->rsp_msg = MAC154APP_FRAME_COUNTER_UPDATE;

	p_cmd = alif_ahi_msg_header_write(p_msg, sizeof(mac154app_id_set_cmd_t), 0);

	p_cmd->cmd_code = MAC154APP_FRAME_COUNTER_UPDATE;
	p_cmd->dummy = ctx;
	p_cmd->value_h = 0;
	p_cmd->value_l = frame_counter;
}

enum alif_mac154_status_code alif_ahi_msg_csl_phase_resp(struct msg_buf *p_msg, uint8_t *p_ctx,
							 uint64_t *p_timestamp,
							 uint16_t *p_csl_phase)
{
	mac154app_get_csl_phase_cmp_evt_t *p_cmd_resp;

	p_cmd_resp = alif_ahi_msg_header_validate(p_msg, MAC154APP_CMP_EVT,
						  sizeof(mac154app_get_csl_phase_cmp_evt_t));

	if (!p_cmd_resp) {
		return ALIF_MAC154_STATUS_COMM_FAILURE;
	}
	if (p_ctx) {
		*p_ctx = p_cmd_resp->dummy;
	}
	if (p_cmd_resp->cmd_code != MAC154APP_CSL_PHASE_GET) {
		return ALIF_MAC154_STATUS_INVALID_MESSAGE;
	}
	if (p_ctx) {
		*p_ctx = p_cmd_resp->dummy;
	}
	if (p_cmd_resp->status != MAC154_ERR_NO_ERROR) {
		return alif_ahi_msg_status_convert(p_cmd_resp->status);
	}
	if (p_csl_phase) {
		*p_csl_phase = p_cmd_resp->csl_phase;
	}
	if (p_timestamp) {
		*p_timestamp = ((uint64_t)p_cmd_resp->value_h << 32) + p_cmd_resp->value_l;
	}

	return ALIF_MAC154_STATUS_OK;
}

enum alif_mac154_status_code alif_ahi_msg_header_ie_csl_full_resp(struct msg_buf *p_msg,
								  uint8_t *p_ctx)
{
	mac154app_config_header_ie_csl_full_cmp_evt_t *p_cmd_resp;

	p_cmd_resp = alif_ahi_msg_header_validate(
		p_msg, MAC154APP_CMP_EVT, sizeof(mac154app_config_header_ie_csl_full_cmp_evt_t));

	if (!p_cmd_resp) {
		return ALIF_MAC154_STATUS_COMM_FAILURE;
	}
	if (p_ctx) {
		*p_ctx = p_cmd_resp->dummy;
	}
	if (p_cmd_resp->cmd_code != MAC154APP_CONF_CSL_IE_HEADER_FULL) {
		return ALIF_MAC154_STATUS_INVALID_MESSAGE;
	}
	return alif_ahi_msg_status_convert(p_cmd_resp->status);
}

enum alif_mac154_status_code alif_ahi_msg_header_ie_csl_reduced_resp(struct msg_buf *p_msg,
								     uint8_t *p_ctx)
{
	mac154app_config_header_ie_csl_reduced_cmp_evt_t *p_cmd_resp;

	p_cmd_resp = alif_ahi_msg_header_validate(
		p_msg, MAC154APP_CMP_EVT, sizeof(mac154app_config_header_ie_csl_reduced_cmp_evt_t));

	if (!p_cmd_resp) {
		return ALIF_MAC154_STATUS_COMM_FAILURE;
	}
	if (p_ctx) {
		*p_ctx = p_cmd_resp->dummy;
	}
	if (p_cmd_resp->cmd_code != MAC154APP_CONF_CSL_IE_HEADER_REDUCED) {
		return ALIF_MAC154_STATUS_INVALID_MESSAGE;
	}
	return alif_ahi_msg_status_convert(p_cmd_resp->status);
}

enum alif_mac154_status_code alif_ahi_msg_config_rx_slot_resp(struct msg_buf *p_msg, uint8_t *p_ctx)
{
	mac154app_config_rx_slot_cmp_evt_t *p_cmd_resp;

	p_cmd_resp = alif_ahi_msg_header_validate(p_msg, MAC154APP_CMP_EVT,
						  sizeof(mac154app_config_rx_slot_cmp_evt_t));

	if (!p_cmd_resp) {
		return ALIF_MAC154_STATUS_COMM_FAILURE;
	}
	if (p_ctx) {
		*p_ctx = p_cmd_resp->dummy;
	}
	if (p_cmd_resp->cmd_code != MAC154APP_CONF_RX_SLOT) {
		return ALIF_MAC154_STATUS_INVALID_MESSAGE;
	}
	return alif_ahi_msg_status_convert(p_cmd_resp->status);
}

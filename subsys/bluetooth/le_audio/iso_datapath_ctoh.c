/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <stdlib.h>
#include "gapi_isooshm.h"
#include "iso_datapath_ctoh.h"

LOG_MODULE_REGISTER(iso_datapath_ctoh, CONFIG_BLE_AUDIO_LOG_LEVEL);

struct iso_datapath_ctoh {
	gapi_isooshm_dp_t dp;
	struct sdu_queue *sdu_queue;
	gapi_isooshm_sdu_buf_t *current_sdu;
	uint32_t sdu_overrun_count;
	uint32_t sdu_recvd_count;
	uint16_t last_sdu_seq;
	bool awaiting_buffer;
};

static void finish_last_sdu(struct iso_datapath_ctoh *datapath)
{
	if (datapath->current_sdu) {
		int ret = k_msgq_put(&datapath->sdu_queue->msgq, (void *)&datapath->current_sdu,
				     K_NO_WAIT);

		if (ret) {
			datapath->sdu_overrun_count++;
		}
		datapath->current_sdu = NULL;
		datapath->sdu_recvd_count++;
	}
}

static void recv_next_sdu(struct iso_datapath_ctoh *datapath)
{
	int ret = k_mem_slab_alloc(&datapath->sdu_queue->slab, (void **)&datapath->current_sdu,
				   K_NO_WAIT);

	if (ret) {
		datapath->awaiting_buffer = true;
		return;
	}

	/* Set max size of SDU */
	datapath->current_sdu->sdu_len = datapath->sdu_queue->item_size;

	uint16_t err = gapi_isooshm_dp_set_buf(&datapath->dp, datapath->current_sdu);

	if (err) {
		LOG_ERR("Failed to set next ISO buffer, err %u", err);
		datapath->awaiting_buffer = true;
		k_mem_slab_free(&datapath->sdu_queue->slab, (void **)&datapath->current_sdu);
	}
}

static void on_dp_transfer_complete(gapi_isooshm_dp_t *dp, gapi_isooshm_sdu_buf_t *buf)
{
	struct iso_datapath_ctoh *datapath = CONTAINER_OF(dp, struct iso_datapath_ctoh, dp);

	finish_last_sdu(datapath);
	recv_next_sdu(datapath);
}

struct iso_datapath_ctoh *iso_datapath_ctoh_create(uint8_t stream_lid, struct sdu_queue *sdu_queue)
{
	if (sdu_queue == NULL) {
		LOG_ERR("Invalid parameter");
		return NULL;
	}

	struct iso_datapath_ctoh *datapath =
		(struct iso_datapath_ctoh *)calloc(sizeof(struct iso_datapath_ctoh), 1);

	if (datapath == NULL) {
		LOG_ERR("Failed to allocate data path");
		return NULL;
	}

	datapath->sdu_queue = sdu_queue;

	uint16_t ret = gapi_isooshm_dp_init(&datapath->dp, on_dp_transfer_complete);

	if (ret != GAP_ERR_NO_ERROR) {
		LOG_ERR("Failed to init datapath with err %u", ret);
		iso_datapath_ctoh_delete(datapath);
		return NULL;
	}

	ret = gapi_isooshm_dp_bind(&datapath->dp, stream_lid, GAPI_DP_DIRECTION_OUTPUT);
	if (ret != GAP_ERR_NO_ERROR) {
		LOG_ERR("Failed to bind datapath with err %u", ret);
		iso_datapath_ctoh_delete(datapath);
		return NULL;
	}

	/* Flag that datapath is waiting for first available buffer */
	datapath->awaiting_buffer = true;

	return datapath;
}

void iso_datapath_ctoh_notify_sdu_done(void *datapath, uint32_t timestamp, uint16_t sdu_seq)
{
	struct iso_datapath_ctoh *iso_dp = (struct iso_datapath_ctoh *)datapath;

	if (iso_dp->awaiting_buffer) {
		iso_dp->awaiting_buffer = false;
		recv_next_sdu(iso_dp);
	}
}

int iso_datapath_ctoh_delete(struct iso_datapath_ctoh *datapath)
{
	if (datapath == NULL) {
		return -EINVAL;
	}

	gapi_isooshm_dp_unbind(&datapath->dp, NULL);
	free(datapath);

	return 0;
}
